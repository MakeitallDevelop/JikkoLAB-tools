#include "dialogflow.h"

int DialogflowBlock::wav_play_size;
int DialogflowBlock::total_send_bytes;
bool DialogflowBlock::wav_rcv_done;

SemaphoreHandle_t DialogflowBlock::tx_done;
SemaphoreHandle_t DialogflowBlock::rx_done;
SemaphoreHandle_t DialogflowBlock::ws_task_done;
SemaphoreHandle_t DialogflowBlock::sound_player_state;

RingbufHandle_t DialogflowBlock::ws_buf_handle;
RingbufHandle_t DialogflowBlock::wav_play_handle;
    
// 시스템 단일 오브젝트는 전역으로 선언 
static WiFiMulti WiFiMulti;
static esp_websocket_client_handle_t ws_client;
WiFiSTAClass WiFiSTA;

/* 
 *  웹소켓 이벤트 핸들러
 *  웹소켓으로 데이터 수신한경우 Call되는 함수 
 */
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  esp_websocket_event_data_t *ws_data = (esp_websocket_event_data_t *)event_data;
  DialogflowBlock* df = static_cast<DialogflowBlock *> (handler_args);
  
  if (ws_data->op_code == 0x08)  // 1. 웹소켓 연결종료 처리 
  {
    Serial.println("server closed connection");
  } 
  else if (ws_data->op_code == 0x09 or ws_data->op_code == 0x0A)   // 2. 웹소켓 Ping-Pong 처리.
  {
    return;
  }
  else if(ws_data->op_code == 0x01)  // 3. 문자열 데이터 처리 
  {  
     int rcv_data_len = ws_data->data_len;
     rcv_data_len = min(rcv_data_len,256);
     
     if(ws_data->data_ptr[0] == 'E')  // 3.1 END, 음성데이터 송신종료
     {
        df->wav_rcv_done = true;
     }
     else if(ws_data->data_ptr[0] == 'W') // 3.2 WAV : 음성데이터 전송시작 
     {
        df->wav_play_size = 0;
        Serial.printf("▶"); 
        df -> wav_rcv_done = false;
     }
     else if(ws_data->data_ptr[0] == 'I') // 3.3 INTENT 인텐트 이름 전송 
     {
        memcpy(df->rcv_intent,ws_data->data_ptr+2,rcv_data_len-2);
        df->rcv_intent[rcv_data_len-2] = 0x00;
        df->is_rcv_new_intent = true;
     }
     else if(ws_data->data_ptr[0] == 'R')  // 3.4 RESPONSE 인텐트 응답 전송 
     {
        memcpy(df->rcv_response,ws_data->data_ptr+2,rcv_data_len-2);
        df->rcv_response[rcv_data_len-2] = 0x00;
        df->is_rcv_new_response = true;
     }
    else if(ws_data->data_ptr[0] == 'S')  // 3.5 STR, 문자열 전송
     {
        memcpy(df->rcv_cmd,ws_data->data_ptr+2,rcv_data_len-2);
        df->rcv_cmd[rcv_data_len-2] = 0x00;
        df->is_rcv_new_cmd = true;
     }
  }
  else if (ws_data->op_code == 0x02)  // 4. 바이너리 데이터 수신 
  { 
    if(df->wav_play_handle != NULL) // 링버퍼 존재할경우 
    {
      int rcv_data_len = ws_data->data_len;
      // 링버퍼에 WAV 파일 쓰기
        UBaseType_t res = xRingbufferSend(df->wav_play_handle , ws_data->data_ptr, rcv_data_len, pdMS_TO_TICKS(4));
        if (res != pdTRUE)  // 링버퍼 쓰기 에러 
          Serial.print("X");
        else 
        {
          df->wav_play_size += rcv_data_len;
          Serial.print(int(xRingbufferGetCurFreeSize(df->wav_play_handle)*10/MAX_WAV_SIZE));
        }
    }
    else // 링버퍼 INIT 되지 않았을경우 
    {
      Serial.printf("rcv bytes but, wav_play_handle is not initialized, ignore! \n");
    }
  }
}

/*
 * DialogflowBlock class 생성자
 */
DialogflowBlock::DialogflowBlock()
{
  rcv_intent = (char*)pvPortMalloc(256);
  rcv_intent[0] = 0x00;
  rcv_response = (char*)pvPortMalloc(256);
  rcv_response[0] = 0x00;
  rcv_cmd = (char*)pvPortMalloc(256);
  rcv_cmd[0] = 0x00;

  total_send_bytes = 0;
  wav_play_size = 0;
  wav_rcv_done = false;
  inited = false;
  is_rcv_new_cmd = false;
  is_rcv_new_intent = false;
  is_rcv_new_response = false;
  sound_player_state = xSemaphoreCreateCounting(1,0); 
  ws_state = false;
  ws_buf_handle  = xRingbufferCreate(8192, RINGBUF_TYPE_BYTEBUF);
  tx_done = xSemaphoreCreateBinary();               
  rx_done = xSemaphoreCreateBinary(); 
}

/*
 * DialogflowBlock class 소멸자
 */
DialogflowBlock:: ~DialogflowBlock()
{
  // 1. unset i2s
  Serial.print("[1] Unset I2S...");
  if (i2s_driver_uninstall(I2S_NUM_0) != ESP_OK)
  {
    Serial.printf("Failed deleting driver: %d\n");
  }

  esp_websocket_client_destroy(ws_client);

  vPortFree(rcv_intent);  
  vPortFree(rcv_response);
  vPortFree(rcv_cmd);

  vRingbufferDelete(wav_play_handle);
  free(wav_play_struct);
  free(wav_play_storage);
  vRingbufferDelete(ws_buf_handle);
  vSemaphoreDelete(tx_done);
  vSemaphoreDelete(rx_done);
}


/* 1. Dialogflow init

   [1] LED 핀 설정
   [2] I2S 설정
   [3] WIFI 설정 및 연결
   [4] Websocket 설정및 연결테스트
*/

void DialogflowBlock::init(const char* ssid ,const char* password , const char  * ip , int port)
{ 
  // 변수 업데이트 
  wifi_ssid  = ssid ;
  wifi_password = password ;
  server_ip = ip;
  server_port = port;

  // 1. set led pin
  pinMode(ONBOARD_LED, OUTPUT);

  // 2. set i2s
  if(Serial)
  {
    Serial.print("[1] Dialogflow start...");
  }
  else {
    Serial.begin(115200);
    Serial.print("[1] Dialogflow start...");
  }
  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_MIC_CHANNEL,
      .communication_format = (i2s_comm_format_t)0X02, //I2S_COMM_FORMAT_STAND_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = true,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 1
  };
  const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_MIC_SERIAL_CLOCK,      // LRCL in [SPH0645]
      .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,   // LRCL in [SPH0645]
      .data_out_num = I2S_SPEAKER_SERIAL_DATA, // DOUT in [SPH0645]
      .data_in_num = I2S_MIC_SERIAL_DATA       // DIN in  [Speeker]
  };

  esp_err_t err;
  REG_SET_BIT(  I2S_TIMING_REG(I2S_NUM_0),BIT(9)); 
  err = i2s_driver_install(I2S_NUM_0, &i2s_config,  4, &m_i2s_queue);
  REG_SET_BIT(  I2S_TIMING_REG(I2S_NUM_0),BIT(9)); 
  
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
  }
  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
  }
  Serial.print("Done! \n[2] Starting  Wifi");

  wav_play_struct = (StaticRingbuffer_t *)heap_caps_malloc(sizeof(StaticRingbuffer_t), MALLOC_CAP_DEFAULT);
  wav_play_storage = (uint8_t *)heap_caps_malloc(sizeof(uint8_t)*MAX_WAV_SIZE, MALLOC_CAP_DEFAULT);
  wav_play_handle = xRingbufferCreateStatic(MAX_WAV_SIZE,RINGBUF_TYPE_BYTEBUF, wav_play_storage, wav_play_struct);

  // 3. 와이파이 연결
  WiFi.mode(WIFI_MODE_STA);
  WiFiSTA.begin(wifi_ssid, wifi_password);
  while(!WiFiSTA.isConnected()) // 연결대기 
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("connected, ip:");
  Serial.println(WiFi.localIP());  // IP 출력 

  if(!this->WS_reset())
  {
    Serial.println("For now, skip Websocket. but need to reset server!");
  }

  if (inited == false)
  {
     xTaskCreate(SoundPlayTask, "SoundPlayTask", 2048, NULL, 20, NULL);
  }
  inited = true;
}

/*
 *  웹소켓 연결 리셋함수
 */
bool DialogflowBlock::WS_reset()
{ 
  // 1. 웹소켓 연결 
  Serial.print("[3] Set Websocket...");

  if(esp_websocket_client_is_connected(ws_client))
  { 
    Serial.println("Already Connect");
    ws_state = true;
    return true;
  }
  
  const esp_websocket_client_config_t ws_cfg =
  {
    .host = server_ip,
    .port = server_port,
    .task_stack = 10000,
    .buffer_size = 8192,
    .pingpong_timeout_sec = 10000,
    .disable_pingpong_discon = true,
    .skip_cert_common_name_check = true,
    .keep_alive_idle = 3600,
    .keep_alive_interval =3600,
    .ping_interval_sec = 1,
  };

  // 이전에 웹소켓 연결이 있었을경우 메모리해제 
  if (ws_client != NULL)
  {
    esp_websocket_client_destroy(ws_client);
  }

  // 웹소켓 연결 시작 

  ws_client = esp_websocket_client_init(&ws_cfg); 
  esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_DATA, websocket_event_handler, (void *)this);
  esp_websocket_client_start(ws_client);

  // 연결될떄까지 대기
  for(int i = 0; i< 20; i++)
  {
    vTaskDelay(200 / portTICK_RATE_MS);
    Serial.print(".");
    if (esp_websocket_client_is_connected(ws_client))
    {
      Serial.print("Connect!");
      esp_websocket_client_send_text(ws_client, (const char *)"Connect", 8, 10000);
      ws_state = true;
      return true;
    }
  }
  Serial.println("Fail!");
  ws_state = false;
  return false;
} 


/* 
 *  버퍼로부터 데이터를 읽어서 웹소켓으로 전송하는 테스크
 */
void DialogflowBlock::WS_Sender( void * parameter)
{ 
  // 1. 시작신호 전송
  esp_websocket_client_send_text(ws_client, (const char *)"S", 1, portMAX_DELAY);

  // 2. wav 파일 헤더 전송
  typedef struct
  {
      // RIFF Header
      char riff_header[4]; // Contains "RIFF"
      int32_t wav_size = 16000*7*1*2 -8;;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
      char wave_header[4]; // Contains "WAVE"

      // Format Header
      char fmt_header[4]; // Contains "fmt " (includes trailing space)
      int32_t fmt_chunk_size = 16; // Should be 16 for PCM
      short audio_format = 1;; // Should be 1 for PCM. 3 for IEEE Float
      short num_channels = 1;;
      int32_t sample_rate = 16000;;
      int32_t byte_rate =  16000*1*2;;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
      short sample_alignment = 1*2;; // num_channels * Bytes Per Sample
      short bit_depth = 16;;        // Number of bits per sample

      // Data
      char data_header[4]; // Contains "data"
      int32_t data_bytes = 16000*7*1*2;;      // Number of bytes in data. Number of samples * num_channels * sample byte size
      // uint8_t bytes[]; // Remainder of wave file is bytes
  } wav_header_t;

  wav_header_t header;
  strncpy(header.riff_header, "RIFF", 4);
  strncpy(header.wave_header, "WAVE", 4);
  strncpy(header.fmt_header, "fmt ", 4);
  strncpy(header.data_header, "data", 4);
  esp_websocket_client_send_bin(ws_client, (const char*)&header, 44, 10000);

  int send_bytes = 0;         // 웹소켓으로 보낸 총 데이터 크기
  int send_size =  CHUNKSIZE; // 한번에 웹소켓으로 보낼 데이터 크기
  int record_exit_cnt = 0;    // 볼륨 공백 카운트 
  int print_cnt = 0;
  wav_play_size = 0;  // 웨이브 사이즈 변수 리셋 
  wav_rcv_done = false; 
  
  // 3. MINWAVSIZE 이상 데이터를 읽어서 전송할때까지 루프 계속
  // 녹음되어 버퍼에 저장되는  WAV 파일은 MINWAVSIZE 보다 약간 큼
  while (send_bytes < MINWAVSIZE )
  {
    // 4. 전송할 데이터 사이즈 갱신
    send_size = total_send_bytes - send_bytes; // 버퍼에 담긴 데이터크기 - 보낸 데이터 크기
    size_t rcv_bytes;
    
    // 5. 세마포어 확인후, 읽을 준비가 되있으면 (tx_done: True) 링버퍼로부터 데이터 읽기
    if(xSemaphoreTake(tx_done,10) == pdTRUE)
    { 
      int16_t minsample = 0;  // 볼륨 측정용 변수
      int16_t maxsample = 0;  // 볼륨 측정용 변수

      // 데이터 버퍼에서 읽기
      int32_t * rcv_item = (int32_t *)xRingbufferReceiveUpTo(ws_buf_handle , &rcv_bytes, pdMS_TO_TICKS(50),  send_size);
      
      // 7천 바이트 이상 보냈을 경우 LED ON
      if (send_bytes > 7000)
          digitalWrite(ONBOARD_LED,HIGH);

      // 읽기에 성공한경우
      if (rcv_item != NULL)  
      {   
        // 32비트 데이터를 16비트로 변환,
        // sph0645는 32비트로 녹음하나, Dialogflow는 16비트 wav 파일 처리가 기본
        int16_t* ws_data = (int16_t* )pvPortMalloc(rcv_bytes / 2);
        for (int i = 0; i < rcv_bytes / 4 ; i++)
        {
         ws_data[i] = rcv_item[i]>> VOL;  // 비트 시프트 연산은 음성 크기 조절을 위해서 들어감, 숫자커지면 소리 작아짐
         minsample = min(minsample, ws_data[i]);
         maxsample = max(maxsample, ws_data[i]);
        }
        vRingbufferReturnItem(ws_buf_handle, (void *)rcv_item);
        xSemaphoreGive(rx_done);
        
        //  웹소켓 데이터 전송
        /* 
         *  TODO : 데이터 전송 실패시 핸들링 방법 고민필요, 임시로  portMAX_DELAY 무한딜레이중
         *  구버전으로 바꾸면서 추가 에러 발생 
         *  https://github.com/espressif/esp-idf/issues/4316
         *  portMAX_DELAY -> 10000으로 수정 
        */
        esp_websocket_client_send_bin(ws_client, (const char*)ws_data, (int)rcv_bytes / 2, 10000);
        vPortFree(ws_data);
        // 볼륨이 VOLUME_TH 미만이면 카운트 증가 
        send_bytes += rcv_bytes;
        if(maxsample - minsample < VOLUME_TH)
          record_exit_cnt ++;
        else
          record_exit_cnt = 0;

        print_cnt ++;
        if (print_cnt % 10  ==0 and record_exit_cnt == 0)
            Serial.print("□");
        else if (print_cnt % 10  ==0 and record_exit_cnt != 0)
            Serial.print("■");
        
        //Serial.print(record_exit_cnt);
        //Serial.print(" ");
        //Serial.println(maxsample - minsample);
      }
      
      // 6. 볼륨이 sound_th 미만인 카운트가 LOW_VOL_CNT_TH 초과고, 60%이상 전송한경우 빠른 전송종료
      if(( record_exit_cnt >  LOW_VOL_CNT_TH) and (send_bytes > MINWAVSIZE*MIN_SEND_RATIO ))
      {
        Serial.println("R Done [0]");
        break;
      }
      // 7. 보낸 데이터가 읽은 데이터의 99% 이상이고, 버퍼에서 데이터를 못읽을경우, 전송종료
      else if (send_bytes > MINWAVSIZE and rcv_item == NULL)
      {
        Serial.println("R Done [1]");
        break;
      }
     
    }
    // 아직 세마포어가 준비가 안된경우 
    else 
    {  
      xSemaphoreGive(rx_done);  
      vTaskDelay(1 / portTICK_RATE_MS); // 1ms 대기
    }
    
  } // 루프종료
  
  // 8. 전송 종료 및 종료신호 전송
  digitalWrite(ONBOARD_LED,LOW);
  xSemaphoreGive(rx_done);
  esp_websocket_client_send_text(ws_client, (const char *)"D", 1, portMAX_DELAY);
  Serial.println("[5] finish send voice"); 
  xSemaphoreGive(ws_task_done);  // 세마포어 0 -> 1
  vTaskDelete( NULL );  
}
 

/*
 * 실제 메인문에서 실행하는 음성인식 함수 
 * 
 * [1] 웹소켓 테스크 실행해서 웹소켓 준비될때까지 대기 
 * [2] 웹소켓 준비되면 녹음시작 
 * [3] 전송 종료까지 대기 
 */
 bool DialogflowBlock::SpeechRec()
{
  if (!inited)
  {
    Serial.println("[Recoder:Abort] : Run speechRec() without init()");
    return false;
  }

  int timeout_cnt = 0;
  while ( check_sound_play() == true )  //  sound_player_state, 음성 플레이어 테스크가 재생중이면 대기 
  {
    vTaskDelay(pdMS_TO_TICKS(500)); // 10ms 대기
    timeout_cnt++;
    if(timeout_cnt > 20) // 500ms * 20 = 10초 이상 대기시 
    {
      Serial.println("[Recoder:Abort] : Sound Player is still running!()");
      return false;
    }
  }
  timeout_cnt = 0;

  // 1. 버퍼 메모리 할당
  Serial.printf("[Recoder:START] : left memory %d Bytes \n",xPortGetFreeHeapSize());


  ws_task_done = xSemaphoreCreateCounting(2,0); 
  uint8_t* i2sData = (uint8_t*)pvPortMalloc(CHUNKSIZE); 
  
  // 2. 웹소켓 연결 확인후 미연결시 재연결
  int reconnect_cnt = 1;
  while(!esp_websocket_client_is_connected(ws_client))
  { 
    Serial.printf("[1-A] Websocket Reset [%d/3] times.\n",reconnect_cnt );
    if(this->WS_reset() == true)
     break;
    else {
      reconnect_cnt++;
      if (reconnect_cnt > 3) {
        Serial.println("Fail to Reconnect, shutdown Speech Rec");
        return false;
      }
    }
  }

  // 3. 웹소켓 전송 테스크 실행
  Serial.printf("[Recoder:2] Websocket Start ,memory %d Bytes \n",xPortGetFreeHeapSize());
  TaskHandle_t xHandle = NULL;
  xTaskCreate(WS_Sender, "WS_Sender", 20000, NULL, 15, &xHandle);
  
  // 4. 녹음 실행
  Serial.printf("[Recoder:3] : Recording Start left ,memory %d Bytes \n",xPortGetFreeHeapSize());
  total_send_bytes = 0;  // 전체 읽은 데이터 바이트 수
  i2s_event_t evt;
  
  xSemaphoreGive(rx_done); //세마포어 초기화
  xSemaphoreGive(tx_done);
  
  // MINWAVSIZE : 16000*4*6 : 6초간 녹음
  while (total_send_bytes < MINWAVSIZE )
  { 
    if (xQueueReceive(m_i2s_queue, &evt, 10) == pdPASS)  // i2s_q에 수신완료경우만 데이터 읽기,
    {
      size_t bytesRead = 0;
      i2s_read(I2S_NUM_0, i2sData,  CHUNKSIZE, &bytesRead, 20);
      if (bytesRead != 0)
      { 
        if(xSemaphoreTake(rx_done,10) == pdTRUE ) 
        {
          xRingbufferSend(ws_buf_handle , i2sData, bytesRead, 20);  // 링버퍼에 읽은 데이터 그대로 저장
          total_send_bytes +=  bytesRead;  // 전체 WAV 데이터가 몇바이트인지 저장
        }
        xSemaphoreGive(tx_done);
      }
    }
    else
      vTaskDelay(1 / portTICK_RATE_MS);  // 1ms 휴식

    if ( uxSemaphoreGetCount(ws_task_done) > 0 )  // task 세마포어 체커 counting 세마포어가 0 초과이면 녹음 종료 
        break;
  }

  // 5. 녹음 종료 LED OFF
  Serial.printf("[DEBUG:4] : Recording Stop ,left memory %d Bytes \n",xPortGetFreeHeapSize());
  digitalWrite(ONBOARD_LED,LOW);

  // 6. 링버퍼 비우기 
  int discard_bytes = 0;
  while(1)
  {
    size_t rcv_bytes;
    uint8_t * rcv_item = (uint8_t *)xRingbufferReceiveUpTo(ws_buf_handle, &rcv_bytes, pdMS_TO_TICKS(1), 100);  
    if (rcv_item == NULL)
      break;
    else
      vRingbufferReturnItem(ws_buf_handle,(void *)rcv_item);
    discard_bytes += rcv_bytes;
  }
  Serial.printf("[Recoder:5] reset ring buffer discard %d bytes, Wait Player done\n",discard_bytes);

  // 7. 플레이어 종료까지 대기 
  vTaskDelay(pdMS_TO_TICKS(3000));  // 플레이어 시작 대기 
  timeout_cnt = 0;
  while ( uxSemaphoreGetCount(sound_player_state) == 0 )  //  sound_player_state, 음성 플레이어 테스크가 재생중이면 대기 
  {
    vTaskDelay(pdMS_TO_TICKS(10)); // 10ms 대기
    timeout_cnt++;
    if(timeout_cnt > 1000) // 10ms * 1000 = 10초 이상 대기시 
    {
      break;
    }
  }
  Serial.printf("[Recoder:] : Sound Player Done, wait %f sec!\n",float(timeout_cnt/100));
  
  // 7. z메모리 해제         
  vPortFree(i2sData);  

  vSemaphoreDelete(ws_task_done);
  Serial.printf("[Recoder:End] : left memory %d Bytes \n",xPortGetFreeHeapSize());
  return true;
}


void DialogflowBlock::SoundPlayTask(void * parameter)
{
  int print_counter = 0;

  Serial.print("[WAV Player:0] : Start_sound_player");
  xSemaphoreGive(sound_player_state); // 카운팅 세마포어 0 -> 1
  while (1)
  {
    if( (MAX_WAV_SIZE - xRingbufferGetCurFreeSize(wav_play_handle) )> 2000)
    {
      xSemaphoreTake(sound_player_state,10); // 카운팅 세마포어 1 -> 0
      // 재생 시작 
      int play_bytes = 0;
      int timeout_cnt = 0;
      Serial.println("[WAV Player:1] :  start playing");
      while(1) 
      {    
        // 1. 웨이브파일을 아직 수신중인데, 버퍼에 저장된 사이즈가 최소 사이즈 이상일경우 
        if(wav_play_size < 14000 and wav_rcv_done == false) 
        { 
          vTaskDelay(pdMS_TO_TICKS(1));
          timeout_cnt++;
        }
        // 2. 웨이브파일을 아직 수신중인데, 버퍼에 저장된 사이즈가 최소사이즈는 넘어서 재생은 진행중이지만 일시적으로 데이터가 부족해진경우 
        else if( play_bytes +16 > wav_play_size and wav_rcv_done == false)  
        {
          timeout_cnt++;
          vTaskDelay(pdMS_TO_TICKS(1)); 
        }
        // 3. 수신은 종료되었고 , 수신한 데이터를 거의(16바이트 이하만 남기고) 재생 완료했을경우 
        else if( play_bytes +16 > wav_play_size and wav_rcv_done == true) 
        { 
          Serial.printf("[WAV Player:3] : play wav done, %d/%d %d\n",play_bytes,wav_play_size,timeout_cnt);
          break;
        }
        // 4. 데이터 수신중이며, 버퍼에 최소 16바이트 이상의 읽을 데이터가 있는경우 
        else 
        {
          size_t rcv_bytes;
          //링 버퍼에서 데이터 읽기
          uint8_t * rcv_item = (uint8_t *)xRingbufferReceiveUpTo(wav_play_handle, &rcv_bytes, pdMS_TO_TICKS(20), 16);
          if (rcv_item != NULL)  // 읽기 성공시 음성재생 
          {    
            size_t bytesWrite = 0;
            if (rcv_bytes % 2 == 0) // 짝수개의 데이터를 읽었을때만, 음성 재생, (16비트->32비트 변환에는 짝수 데이터 필요)
            {
              i2s_write_expand(I2S_NUM_0, rcv_item,  rcv_bytes, 16,32, &bytesWrite,  pdMS_TO_TICKS(10));
            }
            play_bytes +=  rcv_bytes;
            vRingbufferReturnItem(wav_play_handle,(void *)rcv_item);
            timeout_cnt = 0; // 타임아웃 초기화 
          }
          else 
          {
              Serial.printf("[WAV Player:2] : %d %d\n",wav_play_size,xRingbufferGetCurFreeSize(wav_play_handle));
              size_t bytesWrite = 0;
              uint8_t empty[2] = {0X00,0x00};
              i2s_write_expand(I2S_NUM_0, empty, 2, 16,32, &bytesWrite, pdMS_TO_TICKS(10));
              timeout_cnt += 100;
          }
        }
  
        // 타임아웃
        if ( timeout_cnt > 10000) // 1ms * 10000 = 10초
        {
          Serial.printf("[WAV Player:3] : play wav Timeout!, %d/%d %d\n",play_bytes,wav_play_size,timeout_cnt);
          break;
        }
  
      }
  
      // 11. 음성 제로데이터 전송
      size_t bytesWrite = 0;
      uint8_t empty[2] = {0X00,0x00};
      i2s_write_expand(I2S_NUM_0, empty, 2, 16,32, &bytesWrite, pdMS_TO_TICKS(10));
      int discard_bytes = 0;

      // 12. 링버퍼 비우기 
      while(1)
      {
        size_t rcv_bytes;
        uint8_t * rcv_item = (uint8_t *)xRingbufferReceiveUpTo(wav_play_handle, &rcv_bytes, pdMS_TO_TICKS(1), 100);  
        if (rcv_item == NULL) 
        {
          break;
        }
        else
        {
          vRingbufferReturnItem(wav_play_handle,(void *)rcv_item);
          discard_bytes += rcv_bytes;
        }
      }
      
      xSemaphoreGive(sound_player_state); // 카운팅 세마포어 0 -> 1
      Serial.printf("[WAV Player:4] : reset ring buffer discard %d bytes, exit process \n",discard_bytes);
    }
    else
    {
      vTaskDelay(pdMS_TO_TICKS(50)); 
      print_counter ++;
      if( print_counter % 200 == 0 ) 
      { 
        Serial.println("[WAV Player:0] : alive");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1)); 
  }
  vTaskDelete( NULL );   
}

char*  DialogflowBlock::get_cmd()
{
  is_rcv_new_cmd = false;
  return rcv_cmd;
}

char*  DialogflowBlock::get_intent()
{
  is_rcv_new_intent = false;
  return rcv_intent;
}

char*  DialogflowBlock::get_response()
{
  is_rcv_new_response = false;
  return rcv_response;
}

bool  DialogflowBlock::check_sound_play()
{
    if (uxSemaphoreGetCount(sound_player_state) == 0)
    {
      Serial.println("[check_sound_play], sound playing!");
      return true;
    }
    else{
      Serial.println("[check_sound_play], sound ok ");
      return false;
    }
}

bool  DialogflowBlock::check_websocket()
{
    if(esp_websocket_client_is_connected(ws_client))
      ws_state = true;
    else 
      ws_state = false;
    Serial.printf("[check_websockt] %s \n",ws_state ? "connected!" : "not connected!");
    return ws_state;
}
