#include "dialogflow_BLE.h"

SemaphoreHandle_t DialogflowBlock_BLE::ws_task_done;
SemaphoreHandle_t DialogflowBlock_BLE::sound_player_state;
SemaphoreHandle_t DialogflowBlock_BLE::tx_done;
SemaphoreHandle_t DialogflowBlock_BLE::rx_done;

RingbufHandle_t DialogflowBlock_BLE::i2s_buf_handle ;
RingbufHandle_t DialogflowBlock_BLE::wav_play_handle;

BLEServer* DialogflowBlock_BLE::pServer;
BLEService* DialogflowBlock_BLE::pService;
BLECharacteristic* DialogflowBlock_BLE::p_message;
BLECharacteristic* DialogflowBlock_BLE::p_sound_tx;
BLECharacteristic* DialogflowBlock_BLE::p_sound_rx;
BLEAdvertising* DialogflowBlock_BLE::pAdvertising;

int DialogflowBlock_BLE::total_send_bytes;
int DialogflowBlock_BLE::wav_total_size;
bool DialogflowBlock_BLE::wav_rcv_done;
bool DialogflowBlock_BLE::inited;

/* =============================================================================
1. BLE 콜백
=================================================================================*/

  /* ---------- 1.A [BLE_MESSAGE] ----------  
    문자열 데이터 수신 BLE GATT Chariteristic
  */
  // 1.A.1 BLE_MESSAGE 에  DialogflowBlock 모듈 연결
  void BLE_MESSAGE_callback::apply_dflow(DialogflowBlock_BLE * DialogflowBlock_BLE)
  { 
    dflow = DialogflowBlock_BLE;
  }

  // 1.A.2 BLE_MESSAGE 데이터 수신시 처리하는 콜백 
  void BLE_MESSAGE_callback:: onWrite(BLECharacteristic *pCharacteristic) 
  {
    // 수신 데이터 c++ string에 저장
    std::string rxValue = pCharacteristic->getValue();
    const char *cstr = rxValue.c_str();
    int len_str = rxValue.length();
    
    // 최소길이3 , 최대길이 255 제한 
    if (len_str > 255) {len_str = 255;}
    if (len_str < 3)
      return;

    // 태그에 맞춰서 데이터 처리 
    if(cstr[0] == 'E')  // "E",  음성재생 종료 
    {
      dflow->wav_rcv_done = true;
    }
    else if(cstr[0] == 'W') // "W" 음성재생 시작 
    {
      std::string wav_len_str = rxValue.substr(2,len_str-2);
      dflow -> wav_total_size =  atoi(wav_len_str.c_str());
      Serial.printf("start rcv %d size wav \n▶",dflow -> wav_total_size);   
      dflow -> wav_rcv_done = false;
    }
    else if(cstr[0] == 'I')  // Intent 데이터 "I_default_fallback_intent"
    {
      strncpy(dflow->rcv_intent, cstr+2, len_str-2);
      dflow->rcv_intent[len_str-2] = 0x00;
      dflow-> is_rcv_new_intent = true;
    }
    else if(cstr[0] == 'R') // Response 데이터 "R_안녕하세요"
    {
      strncpy(dflow->rcv_response, cstr+2, len_str-2);
      dflow->rcv_response[len_str-2] = 0x00;
      dflow-> is_rcv_new_response = true;
    }
    else if(cstr[0] == 'S') // General String[command] 데이터 "S_stop"
    {
      strncpy(dflow->rcv_cmd, cstr+2, len_str-2);
      dflow->rcv_cmd[len_str-2] = 0x00;
      dflow-> is_rcv_new_cmd = true;
    }
    
  }

  /* ---------- 1.B [BLE_SOUND_RX] ----------  
    음성데이터 수신 BLE GATT Chariteristic
  */
  // 1.B.1 BLE_SOUND_RX 에  DialogflowBlock 모듈 연결
  void BLE_SOUND_RX_callback::apply_dflow(DialogflowBlock_BLE * DialogflowBlock_BLE)
  { 
    dflow = DialogflowBlock_BLE;
  }

  // 1.B.2 BLE_MESSAGE 데이터 수신시 처리하는 콜백 
  void BLE_SOUND_RX_callback:: onWrite(BLECharacteristic *pCharacteristic) 
  {
    /*
      1. 수신한 데이터 Byte 로 변환
      2. 수신데이터 음성재생 queue에 쓰기
    */
    std::string rxValue = pCharacteristic->getValue();
    uint8_t*rcv_data  = (uint8_t*)rxValue.c_str();
    int rcv_data_len = rxValue.length();
    
    if(dflow->wav_play_handle != NULL) // 링버퍼 존재할경우 
    {
      // 링버퍼에 WAV 파일 쓰기
        UBaseType_t res = xRingbufferSend(dflow->wav_play_handle , rcv_data, rcv_data_len, pdMS_TO_TICKS(4));
        if (res != pdTRUE)  // 링버퍼 쓰기 에러 
          Serial.print("X");
        else 
        {
          Serial.print(int(xRingbufferGetCurFreeSize(dflow->wav_play_handle)*10/MAX_WAV_SIZE));
        }
    }
    else // 링버퍼 INIT 되지 않았을경우 
    {
      Serial.printf("rcv bytes but, wav_play_handle is not initialized, ignore! \n");
    }
    return;
  }

  /* ---------- 1.C [BLE_SERVER] ----------  
    BLE 관리 SERVICE
  */
  // 1.C.1 BLE_SERVER 에  DialogflowBlock 모듈 연결 
  void BLE_SERVER_callback::apply_dflow(DialogflowBlock_BLE *DialogflowBlock_BLE)
  {
    dflow = DialogflowBlock_BLE;
  }

  // 1.C.2 BLE_SERVER 연결시 동작 콜백 
  void BLE_SERVER_callback::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) 
  {
    dflow->deviceConnected = 1;
    esp_ble_conn_update_params_t conn_params = {0};
    memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
    conn_params.latency = 0;
    conn_params.min_int = 6; // min_int = 6*1.25ms = 7.5ms
    conn_params.max_int = 8; // max_int = 8*1.25ms = 9ms
    conn_params.timeout = 20;  // timeout = 20*10ms = 200ms
    //start sent the update connection parameters to the peer device.
    esp_ble_gap_update_conn_params(&conn_params);
    Serial.println("connected");
   
    vTaskDelay(pdMS_TO_TICKS(200)); 
  };
  
  // 1.C.3 BLE_SERVER 연결 해제시 동작 콜백 
  void BLE_SERVER_callback::onDisconnect(BLEServer* pServer) 
  {
    dflow->deviceConnected = 0;
    vTaskDelay(pdMS_TO_TICKS(200)); 
    BLEDevice::startAdvertising();
  }




/* =============================================================================
2. DialogflowBlock 모듈 
=================================================================================*/

  /* ---------- 2.A [생성, 소멸자] ----------  
  */
  DialogflowBlock_BLE::DialogflowBlock_BLE()
  {
    rcv_intent = (char*)pvPortMalloc(256);
    rcv_intent[0] = 0x00;
    rcv_response = (char*)pvPortMalloc(256);
    rcv_response[0] = 0x00;
    rcv_cmd = (char*)pvPortMalloc(256);
    rcv_cmd[0] = 0x00;
    deviceConnected = 0;
    wav_total_size = 1000;
    wav_rcv_done = false; 
    inited = false; 
    tx_done = xSemaphoreCreateBinary();               
    rx_done = xSemaphoreCreateBinary(); 
  }

  DialogflowBlock_BLE:: ~DialogflowBlock_BLE(){

  }


  /* ---------- 2.B [모듈 Init 함수] ----------  

  1. 로깅용 하드웨어 시리얼 통신 시작 
  2. ble 표시용 디바이스 이름 저장 
  3. i2s 시작 
  4. 음성재생 태스크 메모리 할당 및 생성 
  5. ble 서비스 및 콜백 등록 
  6. ble advertise 시작 

  */
  void DialogflowBlock_BLE::init(const char* d_name)
  {
    if (inited)
    {
      Serial.print("[0] Already inited, Exit");
      return;
    }

    //  ------------- 1. 로깅용 하드웨어 시리얼 통신 시작   ------------- 
    if(!Serial)
    {
      Serial.begin(115200);
    }
    Serial.print("[1] Dialogflow BLE start...");

    // -------------- 2. ble 표시용 디바이스 이름 저장 -------------
    device_name = (char *) d_name;   

    // ------------- 3. I2S 시작  ------------- 
    const i2s_config_t i2s_config = {
          .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
          .sample_rate = 8000,
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
      Serial.println("[3] I2s Done, start setting BLE"); 

    // ------------- 4. 음성 재생 태스크 메모리 할당 및 테스크 실행   ---------------
    wav_play_struct = (StaticRingbuffer_t *)heap_caps_malloc(sizeof(StaticRingbuffer_t),
                                                             MALLOC_CAP_DEFAULT
                                                             );
    wav_play_storage = (uint8_t *)heap_caps_malloc(sizeof(uint8_t)*MAX_WAV_SIZE,
                                                     MALLOC_CAP_DEFAULT
                                                     );
    wav_play_handle = xRingbufferCreateStatic(MAX_WAV_SIZE,
                                              RINGBUF_TYPE_BYTEBUF, 
                                              wav_play_storage, 
                                              wav_play_struct
                                              );

    sound_player_state = xSemaphoreCreateCounting(1,0); 
    i2s_buf_handle = xRingbufferCreate(8192, RINGBUF_TYPE_BYTEBUF);                 
    xTaskCreate(SoundPlayTask,
                "SoundPlayTask",
                2048, NULL, 20, NULL
                );

    // ------------- 5. ble 서비스 및 콜백 등록  ---------------
    BLEDevice::init(device_name);
    BLEDevice::setMTU(517);
    BLEDevice::setPower(ESP_PWR_LVL_P9); 

    // 서버 및 서비스 등록 
    pServer = BLEDevice::createServer();

    pService = pServer->createService(SERVICE_UUID);


    // Characteristic 생성 
    p_message = pService->createCharacteristic(
                          MESSAGE_UUID,
                          BLECharacteristic::PROPERTY_WRITE
                          | BLECharacteristic::PROPERTY_WRITE_NR 
                          | BLECharacteristic::PROPERTY_READ 
                          | BLECharacteristic::PROPERTY_NOTIFY 
                        );

    p_sound_tx = pService->createCharacteristic(
                          SOUND_TX_UUID,
                          BLECharacteristic::PROPERTY_READ 
                        );

    p_sound_rx  = pService->createCharacteristic(
                        SOUND_RX_UUID,
                        BLECharacteristic::PROPERTY_WRITE
                        | BLECharacteristic::PROPERTY_WRITE_NR 
                      );

    // callback 생성 및 등록 
    BLE_SERVER_callback* callback_01 = new BLE_SERVER_callback();
    BLE_MESSAGE_callback* callback_02 = new BLE_MESSAGE_callback();
    BLE_SOUND_RX_callback* callback_03 = new BLE_SOUND_RX_callback();
    callback_01->apply_dflow(this);
    callback_02->apply_dflow(this);
    callback_03->apply_dflow(this);
  
    pServer -> setCallbacks(callback_01);                                        
    p_message -> setCallbacks(callback_02);
    p_sound_rx -> setCallbacks(callback_03);

    // ------------- 6. ble advertise 시작  ------------- 
    pService->start();
    
    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    connid = pServer->getConnId();
    inited = true;
  }

/* =============================================================================
3. DialogflowBlock 모듈 서브 테스크 
=================================================================================*/

  /* ---------- 3.A [음성 재생 TASK] ----------
    1. 초기 설정 및 메모리 할당
    2. 무한 루프 시작
    3. MAX_WAV_SIZE 이상 버퍼에 있을경우 재생작업 시작 
  */
  void DialogflowBlock_BLE::SoundPlayTask(void * parameter)
  {
    // ---------- 1. 초기 설정 및 메모리 할당 ----------
    int print_counter = 0;
    Serial.print("[WAV Player:0] : Start_sound_player");
    xSemaphoreGive(sound_player_state); // 카운팅 세마포어 0 -> 1
    int16_t empty[8] = {0,0,0,0,0,0,0,0};
    size_t bytesWrite = 0;
    int wating_size =BUFFER_RESERVE_RATIO * min( MAX_WAV_SIZE, wav_total_size);
    
    // ---------- 2. 무한 루프 시작  ----------
    while (1)
    { 
      // 3. wating_size 이상 버퍼에 있을때까지 대기작업 
      
      while ( (MAX_WAV_SIZE - xRingbufferGetCurFreeSize(wav_play_handle) ) < wating_size ){
          vTaskDelay(pdMS_TO_TICKS(50)); 
          print_counter ++;
          if( print_counter % 200 == 0 ){ 
            Serial.println("[WAV Player:0] : alive");
            i2s_write_expand(I2S_NUM_0, empty, 16, 16,32, &bytesWrite, pdMS_TO_TICKS(10));   //제로데이터 쓰기
          }
          if (wav_total_size  != 1000)
            wating_size =BUFFER_RESERVE_RATIO * min( MAX_WAV_SIZE, wav_total_size );
          else 
            wating_size =BUFFER_RESERVE_RATIO * MAX_WAV_SIZE;
      }

      // 4. 재생시작 
      xSemaphoreTake(sound_player_state,10); // 카운팅 세마포어 1 -> 0
      int play_bytes = 0;
      int timeout_cnt = 0;
      Serial.println("[WAV Player:1] :  start playing");
      while((play_bytes < wav_total_size - 17) or ( xRingbufferGetCurFreeSize(wav_play_handle)  > 16))
      {    
        size_t rcv_bytes;
        int16_t * rcv_item = (int16_t *)xRingbufferReceiveUpTo(wav_play_handle, &rcv_bytes, pdMS_TO_TICKS(20), 16); //링 버퍼에서 데이터 읽기
        
        if (rcv_item != NULL)  // 읽기 성공시 음성재생 
        {    
          if (rcv_bytes % 2 == 0) // 짝수개의 데이터를 읽었을때만, 음성 재생, (16비트->32비트 변환에는 짝수 데이터 필요)
          { 
            for(int i = 0; i < bytesWrite/2 ; i++ )
            {
              rcv_item[i] =  (int16_t)(rcv_item[i]* PLAY_VOL_UP); // 볼륨조절
            }
            i2s_write_expand(I2S_NUM_0, (uint8_t*)rcv_item,  rcv_bytes, 16,32, &bytesWrite,  pdMS_TO_TICKS(10));
          }
        play_bytes +=  rcv_bytes;
        vRingbufferReturnItem(wav_play_handle,(void *)rcv_item);
        timeout_cnt = 0; // 타임아웃 초기화 
        
        }
        else  // 읽기 실패시 일단 제로데이터 쓰기.
        {
            //Serial.printf("[WAV Player:2] : %d %d\n",play_bytes,wav_total_size);
            i2s_write_expand(I2S_NUM_0, empty, 16, 16,32, &bytesWrite, pdMS_TO_TICKS(10));    //제로데이터 쓰기
            timeout_cnt += 100; // 타임아웃 증가
            vTaskDelay(pdMS_TO_TICKS(20)); 
        }
        if ( timeout_cnt > 10000) // 타임아웃 1ms * 10000 = 10초 
        {
          Serial.printf("[WAV Player:3] : play wav Timeout!, %d/%d %d\n",play_bytes,wav_total_size,timeout_cnt);
          i2s_write_expand(I2S_NUM_0, empty, 16, 16,32, &bytesWrite, pdMS_TO_TICKS(10));   //제로데이터 쓰기
          break;
        }
      }

      // 재생 종료        
      wav_total_size = 1000;
      i2s_write_expand(I2S_NUM_0, empty, 16, 16,32, &bytesWrite, pdMS_TO_TICKS(10)); // 11. 음성 제로데이터 전송
      int discard_bytes = 0;

      // 12. 링버퍼 비우기 
      while(1){
        size_t rcv_bytes;
        uint8_t * rcv_item = (uint8_t *)xRingbufferReceiveUpTo(wav_play_handle, &rcv_bytes, pdMS_TO_TICKS(1), 100);  
        if (rcv_item == NULL) {break;}
        else {
          vRingbufferReturnItem(wav_play_handle,(void *)rcv_item);
          discard_bytes += rcv_bytes;
        }
      }        
      xSemaphoreGive(sound_player_state); // 카운팅 세마포어 0 -> 1
      Serial.printf("[WAV Player:4] : reset ring buffer discard %d byte go to idle \n",discard_bytes);
      
    }
    vRingbufferDelete(wav_play_handle);
    vTaskDelete( NULL );   
  }

  /* ---------- 3.A [음성 전송 TASK] ----------
    1. 초기 설정 및 메모리 할당
    2. 무한 루프 시작
    3. MAX_WAV_SIZE 이상 버퍼에 있을경우 재생작업 시작 
  */
  void DialogflowBlock_BLE::VoiceSender( void * parameter)
  { 
    // 1. wav 파일 헤더 전송
    typedef struct
    {
        // RIFF Header
        char riff_header[4]; // Contains "RIFF"
        int32_t wav_size = 8000*7*1*2 -8;;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
        char wave_header[4]; // Contains "WAVE"

        // Format Header
        char fmt_header[4]; // Contains "fmt " (includes trailing space)
        int32_t fmt_chunk_size = 16; // Should be 16 for PCM
        short audio_format = 1;; // Should be 1 for PCM. 3 for IEEE Float
        short num_channels = 1;;
        int32_t sample_rate = 8000;;
        int32_t byte_rate =  8000*1*2;;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
        short sample_alignment = 1*2;; // num_channels * Bytes Per Sample
        short bit_depth = 16;;        // Number of bits per sample

        // Data
        char data_header[4]; // Contains "data"
        int32_t data_bytes = 8000*7*1*2;;      // Number of bytes in data. Number of samples * num_channels * sample byte size
        // uint8_t bytes[]; // Remainder of wave file is bytes
    } wav_header_t;

    wav_header_t header;
    strncpy(header.riff_header, "RIFF", 4);
    strncpy(header.wave_header, "WAVE", 4);
    strncpy(header.fmt_header, "fmt ", 4);
    strncpy(header.data_header, "data", 4);
    p_sound_tx->setValue((uint8_t*)&header,44);
    p_sound_tx->indicate();

    int send_bytes = 0;         // 웹소켓으로 보낸 총 데이터 크기
    int send_size = 512; // 한번에 웹소켓으로 보낼 데이터 크기
    int record_exit_cnt = 0;    // 볼륨 공백 카운트 
    int print_cnt = 0;
    int block_counter = 0;
    wav_rcv_done = false; 

   Serial.println("[Voice Sender] Start!");
    // 2. MINWAVSIZE 이상 데이터를 읽어서 전송할때까지 루프 계속
    // 녹음되어 버퍼에 저장되는  WAV 파일은 MINWAVSIZE 보다 약간 큼
    while (send_bytes < MINWAVSIZE )
    {

      // 데이터 버퍼에서 읽기
      if(xSemaphoreTake(tx_done,10) == pdTRUE) 
      {
        size_t rcv_bytes = 0;
        int16_t minsample = 0;  // 볼륨 측정용 변수
        int16_t maxsample = 0;  // 볼륨 측정용 변수
        int32_t * rcv_item = (int32_t *)xRingbufferReceiveUpTo(i2s_buf_handle  , &rcv_bytes,pdMS_TO_TICKS(2000),  send_size);
        xSemaphoreGive(rx_done);
        
        // 7천 바이트 이상 보냈을 경우 LED ON
        if (send_bytes > 7000)
            digitalWrite(ONBOARD_LED,HIGH);
              
        // 읽기에 성공한경우
        if (  rcv_bytes != 0)  
        {   
          //  sph0645는 32비트로 녹음하나, Dialogflow는 16비트 wav 파일 처리가 기본, 32비트 데이터를 16비트로 변환,
          int ble_data_len = rcv_bytes / 4;
          int16_t ws_data[ble_data_len+1];
          ws_data[0] = block_counter;
          for (int i = 1; i < ble_data_len +1 ; i++)
          {
            ws_data[i] = rcv_item[i]>> VOL;  // 비트 시프트 연산은 음성 크기 조절을 위해서 들어감, 숫자커지면 소리 작아짐
            minsample = min(minsample, ws_data[i]);
            maxsample = max(maxsample, ws_data[i]);
            Serial.print(ws_data[i]);
          }
          p_sound_tx->setValue((uint8_t*)ws_data,int(rcv_bytes / 2));
          p_sound_tx->indicate();
          block_counter ++;
          vRingbufferReturnItem(i2s_buf_handle , (void *)rcv_item); //  데이터 반환
          //vPortFree(ws_data);                                     //  메모리 해제 
         
          send_bytes += rcv_bytes;
          print_cnt ++;
          
          if(maxsample - minsample < VOLUME_TH) { record_exit_cnt ++; }  // 볼륨이 VOLUME_TH 미만이면 종료 카운트 증가 
          else { record_exit_cnt = 0; }                                  // 볼륨이 VOLUME_TH 이상이면 종료 카운트 초기화
           
          if (print_cnt % 10  ==0 and record_exit_cnt == 0) { Serial.print("□"); }     // 전송상황, 볼륨과 함께 print
          else if (print_cnt % 10  ==0 and record_exit_cnt != 0) { Serial.print("■"); }
              
          // 전송종료 플래그 
          
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
        // 읽기 실패한경우 
        else { Serial.printf("read ring buffer fail %d \n", rcv_bytes); }
      }
      
 
      
      else
      {
        record_exit_cnt ++;
        print_cnt ++;
        if (print_cnt % 10  ==0) { Serial.printf("%d / 10000\n",xRingbufferGetCurFreeSize(i2s_buf_handle )); }
        if(record_exit_cnt >  LOW_VOL_CNT_TH)
        {
          Serial.println("R Done [2]");
          break;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
      }
    } // 루프종료
    char end_signal[4] = "end";
    p_sound_tx->setValue((uint8_t*)end_signal,4);
    p_sound_tx->indicate();
    //
    // 8. 전송 종료 및 종료신호 전송
    digitalWrite(ONBOARD_LED,LOW);
    Serial.println("[5] finish send voice"); 
    xSemaphoreGive(ws_task_done);  // 세마포어 0 -> 1
    vTaskDelete( NULL );  
  }

  /* ---------- 3.C [음성 녹음 함수] ----------  
  1. 모듈이 Init 되지 않았으면 에러메세지 출력 후 false 리턴
  2. 음성 플레이어 테스크가 재생중이면 대기  , 너무 길어지면 타임아웃
  3. wav 파일 헤더 생성
  4. 녹음시작
  */

  bool DialogflowBlock_BLE::SpeechRec()
  { 
    // ---------- 1. 모듈이 Init 되지 않았으면 에러메세지 출력 후 false 리턴  ---------- 
    if (!inited)
    {
      Serial.println("[Recoder:Abort] : Run speechRec() without init()");
      return false;
    }
    
    // ---------- 2. 음성 플레이어 테스크가 재생중이면 대기  , 너무 길어지면 타임아웃---------- 
    int timeout_cnt = 0;
    while ( check_sound_play() == true )  
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

    // 3. BLE 연결 확인후, 미연결시 종료 
    if (check_connect() == false)
    {
      Serial.println("[Recoder:Abort] : BLE Not connected");
      return false;
    }
    Serial.printf("[Recoder:2] BLE Sender Start ,memory %d Bytes \n",xPortGetFreeHeapSize());

    // 4. PC로 음성재생 신호 전송 
    char end_signal[4] = "REC";
    end_signal[3] = NULL;
    p_sound_tx->setValue((uint8_t*)end_signal,4);
    p_sound_tx->indicate();
    
    /*
     *    자체 녹음기능 비활성화 ( BLE로 데이터 송신 한계)
     * 
    ws_task_done = xSemaphoreCreateCounting(2,0); 
    uint8_t* i2sData = (uint8_t*)pvPortMalloc(CHUNKSIZE); 
    TaskHandle_t xHandle = NULL;
    xTaskCreate(VoiceSender, "VoiceSender", 20000, NULL, 15, &xHandle);
    xSemaphoreGive(rx_done); //세마포어 초기화
    xSemaphoreGive(tx_done);
    int fail_count = 0;
    int print_count = 0;
    
    // 4. 녹음 시작
    total_send_bytes = 0;  // 전체 읽은 데이터 바이트 수
    i2s_event_t evt;
    Serial.printf("[Recoder:3] : Recording Start left ,memory %d Bytes \n",xPortGetFreeHeapSize());
    while (total_send_bytes < MINWAVSIZE ) //MINWAVSIZE : 16000*4*6 : 6초간 녹음
    { 
      if (xQueueReceive(m_i2s_queue, &evt, 10) == pdPASS)  // i2s_q에 수신완료경우만 데이터 읽기,
      {
        if(xSemaphoreTake(rx_done,100) == pdTRUE)
        {
          size_t bytesRead = 0;
          i2s_read(I2S_NUM_0, i2sData,  CHUNKSIZE, &bytesRead, pdMS_TO_TICKS(10));
          if (bytesRead != 0)
          { 
            int left_space = xRingbufferGetCurFreeSize(i2s_buf_handle );
            if(left_space >= bytesRead ) 
            {
              xRingbufferSend(i2s_buf_handle  ,i2sData, bytesRead, pdMS_TO_TICKS(10));  // 링버퍼에 읽은 데이터 그대로 저장
              xSemaphoreGive(tx_done);
              total_send_bytes += bytesRead;  // 전체 WAV 데이터가 몇바이트인지 저장
              fail_count = 0;
              if (print_count % 10 == 0)
                Serial.printf("[Recoder:3-1] Send %d data\n",total_send_bytes) ;
            } 
            else 
            { 
              fail_count ++;
              xSemaphoreGive(tx_done); 
              if (print_count % 10 == 0)
                Serial.printf("[Recoder:3-1] Fail to Send %d data, %d left \n",bytesRead, left_space) ;  
            }
          }
        }
      }
      
      print_count++;          
      if ( uxSemaphoreGetCount(ws_task_done) > 0 )  // task 세마포어 체커 counting 세마포어가 0 초과이면 녹음 종료 
        break;
      else if (fail_count > 50)
      {
        Serial.printf("[Recoder:Abort] too many Fail Count \n") ;  
        break;
      }
    }

    // 5. 녹음 종료 LED OFF
    Serial.printf("[DEBUG:4] : Recording Stop ,left memory %d Bytes \n",xPortGetFreeHeapSize());
    digitalWrite(ONBOARD_LED,LOW);

    // 6. 링버퍼 비우기 
    int discard_bytes = 0;
    while(1)
    {
      size_t rcv_bytes;
      uint8_t * rcv_item = (uint8_t *)xRingbufferReceiveUpTo(i2s_buf_handle , &rcv_bytes, pdMS_TO_TICKS(1), 100);  
      if (rcv_item == NULL)
        break;
      else
        vRingbufferReturnItem(i2s_buf_handle ,(void *)rcv_item);
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
    
    // 7. 메모리 해제         
    vPortFree(i2sData);  
    vSemaphoreDelete(ws_task_done);
    Serial.printf("[Recoder:End] : left memory %d Bytes \n",xPortGetFreeHeapSize());
    return true;
    */
  }

/* =============================================================================
4. DialogflowBlock 모듈 기타 함수들
=================================================================================*/

char*  DialogflowBlock_BLE::get_cmd()
{
  /*
  if (gatts_if != NULL)
  {
    esp_ble_gatts_send_indicate( gatts_if ,123,123,4,'asdf');
  }*/
  is_rcv_new_cmd = false;
  return rcv_cmd;
}

char*  DialogflowBlock_BLE::get_intent()
{
  is_rcv_new_intent = false;
  return rcv_intent;
}

char*  DialogflowBlock_BLE::get_response()
{
  is_rcv_new_response = false;
  return rcv_response;
}

bool  DialogflowBlock_BLE::check_sound_play()
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

bool  DialogflowBlock_BLE::check_connect()
{
    if( deviceConnected == 1){
      Serial.println("[check_BLE, Connected!]");
      return true;
    }
    else{
      Serial.println("[check_BLE, Not Connected!]");
      BLEDevice::startAdvertising();
      return false;
    }
}

//레거시 지원
bool DialogflowBlock_BLE::check_websocket()
{
  return check_connect();
}
