 #include <Arduino.h>
#include <driver/i2s.h>
#include "config.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"

#include "esp_websocket_client.h"
#include "SPIFFS.h"

#include <WiFi.h>
#include <WiFiMulti.h>


#define CHUNKSIZE 512
#define RECORD_TIME 6
#define MINWAVSIZE 16000* 4* RECORD_TIME

class DialogflowBlock
{
private:
    // 1. FreeRtos 핸들러 
    QueueHandle_t m_i2s_queue;

    // 2. 함수
    //static void WS_Sender(void * parameter);
    bool WS_reset();
    static void WS_Sender(void * parameter);
    static void SoundPlayTask(void * parameter);
    StaticRingbuffer_t *wav_play_struct;
    uint8_t *wav_play_storage;
    const char* wifi_ssid ;
    const char* wifi_password ;
    const char* server_ip ;
    int server_port;
    bool inited;
    bool ws_state;
    
public:
    DialogflowBlock();
    ~ DialogflowBlock();

    static bool wav_rcv_done;
    static int wav_play_size;
    static int total_send_bytes;

    static SemaphoreHandle_t tx_done;
    static SemaphoreHandle_t rx_done;
    static SemaphoreHandle_t ws_task_done;
    static SemaphoreHandle_t sound_player_state;
    static RingbufHandle_t ws_buf_handle;
    static RingbufHandle_t wav_play_handle;

    char* rcv_cmd;
    char* rcv_intent;
    char* rcv_response;
    bool is_rcv_new_cmd;
    bool is_rcv_new_intent;
    bool is_rcv_new_response;
   
    void init(const char* ssid ,const char* password , const char  * ip , int port);
    bool check_sound_play();
    bool check_websocket();
    
    char* get_cmd();
    char* get_intent();
    char* get_response();
    
    bool SpeechRec();
    
   
};
