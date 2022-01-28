#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>
#include <driver/i2s.h>
#include "freertos/task.h"
#include "config.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"

#define CHUNKSIZE 512
#define RECORD_TIME 6
#define MINWAVSIZE 8000* 4* RECORD_TIME

class DialogflowBlock_BLE
{
private:
    // 1. FreeRtos 핸들러 
    QueueHandle_t m_i2s_queue;

    // 5. 기타
    bool exit_flag;
    int connid;
    StaticRingbuffer_t *wav_play_struct;
    uint8_t *wav_play_storage;
    
    // 함수 
    static void VoiceSender(void * parameter);
    static void SoundPlayTask(void * parameter);

public:
    DialogflowBlock_BLE();
    ~ DialogflowBlock_BLE();
    
    char* rcv_cmd;
    char* rcv_intent;
    char* rcv_response;
    bool is_rcv_new_cmd;
    bool is_rcv_new_intent;
    bool is_rcv_new_response;

    char* device_name;
    int deviceConnected;
    
    // 2. wav 재생 관련 변수 
    static SemaphoreHandle_t ws_task_done;
    static SemaphoreHandle_t sound_player_state;
    static RingbufHandle_t i2s_buf_handle ;
    static RingbufHandle_t wav_play_handle;
    static SemaphoreHandle_t tx_done;
    static SemaphoreHandle_t rx_done;

    // 4. BLE 관련 
    static BLEServer *pServer;
    static BLEService *pService;
    static BLECharacteristic *p_message;
    static BLECharacteristic *p_sound_tx;
    static BLECharacteristic *p_sound_rx;
    static BLEAdvertising *pAdvertising;
    
    static bool wav_rcv_done;
    static bool inited;
    static int wav_total_size;
    static int total_send_bytes;

    // 함수 
    void init(const char* d_name);
    bool check_sound_play();
    bool check_connect();
    bool check_websocket(); // 레거시 지원
    
    char* get_cmd();
    char* get_intent();
    char* get_response();
    
    bool SpeechRec();
    
};

class BLE_MESSAGE_callback: public BLECharacteristicCallbacks
{
  public:
     DialogflowBlock_BLE *dflow;
     void apply_dflow(DialogflowBlock_BLE* DialogflowBlock_BLE);
     void onWrite(BLECharacteristic *pCharacteristic);
};

class BLE_SOUND_RX_callback: public BLECharacteristicCallbacks
{
  public:
     DialogflowBlock_BLE *dflow;
     void apply_dflow(DialogflowBlock_BLE* DialogflowBlock_BLE);
     void onWrite(BLECharacteristic *pCharacteristic);
};

class BLE_SERVER_callback: public BLEServerCallbacks
{
  public:
     DialogflowBlock_BLE *dflow;
     void apply_dflow(DialogflowBlock_BLE* dialogflowblock);
     void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) ;
     void onDisconnect(BLEServer* pServer);
};
