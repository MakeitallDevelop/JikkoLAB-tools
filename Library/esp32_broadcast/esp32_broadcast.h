#ifndef esp_now_h         
#define esp_now_h

#include <esp_now.h>   //mASTER
#include <WiFi.h>
#include <Arduino.h>

uint8_t broadcastAddress[7] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
typedef struct struct_message { int id; int d1; int d2; int d3; int d4;} struct_message;
struct_message SendData;
struct_message ReciveData;

int my_id = 0;
boolean recive_ok = false;
boolean esp_debug = false;


int esp_read_id(){
    return ReciveData.id;
}

void esp_debug_mode(boolean _en){
    esp_debug = _en;
}

int esp_read(int _number){
  int rt = 0;
  switch(_number){
    case 0: rt = ReciveData.d1;
                 ReciveData.d1 = 0;
    break;

    case 1: rt = ReciveData.d2;
                 ReciveData.d2 = 0;
    break;

    case 2: rt = ReciveData.d3;
                 ReciveData.d3 = 0;
    break;

    case 3: rt = ReciveData.d4;
                 ReciveData.d4= 0;
    break;
    
  }
  return rt;
  
}




boolean esp_available(int _id){
  if(recive_ok == true){ //외부서 왔어요
      if(ReciveData.id == _id){
          recive_ok = false;
          return true;
      }
  }
  return false;
}


boolean esp_available(){
  if(recive_ok == true){
      recive_ok = false;
      return true;
  }
  return false;
}


void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    char macStr[18];
    
    
    memcpy(&ReciveData, incomingData, sizeof(ReciveData));
    if(esp_debug == true){
      Serial.print("Packet received from: ");
          snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                   mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
      Serial.println(macStr);
      Serial.printf("Board ID %u: %u bytes\n", ReciveData.id, len);
      
      Serial.printf("d1 value: %d \n", ReciveData.d1);
      Serial.printf("d2 value: %d \n", ReciveData.d2);
      Serial.printf("d3 value: %d \n", ReciveData.d3);
      Serial.printf("d4 value: %d \n", ReciveData.d4);
      Serial.println();
    }
    recive_ok = true;
    
    
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    //Serial.print("\r\nLast Packet Send Status:\t");
    //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void esp_send(int _d1){
  SendData.id = my_id;
  SendData.d1 = _d1;
  SendData.d2 = 0;
  SendData.d3 = 0;
  SendData.d4 = 0;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &SendData, sizeof(SendData));
  if (result != ESP_OK) {
    Serial.println("Error ESP32 Send");
  }
}

void esp_send(int _d1, int _d2){
  SendData.id = my_id;
  SendData.d1 = _d1;
  SendData.d2 = _d2;
  SendData.d3 = 0;
  SendData.d4 = 0;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &SendData, sizeof(SendData));
  if (result != ESP_OK) {
    Serial.println("Error ESP32 Send");
  }
}

void esp_send(int _d1, int _d2, int _d3){
  SendData.id = my_id;
  SendData.d1 = _d1;
  SendData.d2 = _d2;
  SendData.d3 = _d3;
  SendData.d4 = 0;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &SendData, sizeof(SendData));
  if (result != ESP_OK) {
    Serial.println("Error ESP32 Send");
  }
}

void esp_send(int _d1, int _d2, int _d3 , int _d4){
  SendData.id = my_id;
  SendData.d1 = _d1;
  SendData.d2 = _d2;
  SendData.d3 = _d3;
  SendData.d4 = _d4;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &SendData, sizeof(SendData));
  if (result != ESP_OK) {
    Serial.println("Error ESP32 Send");
  }
}




void esp_broadcast_init( int _id, int ch ){
    WiFi.mode(WIFI_STA);
    my_id = _id;
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
    }

    esp_now_register_recv_cb(OnDataRecv);
    //---------------------
    esp_now_register_send_cb(OnDataSent); //보내는 인터럽트
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = ch;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }

}


    







#endif
