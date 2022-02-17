#ifndef Jikko_H
#define Jikko_H


int getMultiBTData1(String str, int number);
int getMultiBTData2(String str, int number);
int getMultiBTData3(String str, int number);
void pingpongGameInit(int userBoard, int myBoard);
void shootGameInit(int life, int speed);
void pingpongGame();
void shootGame();
float getDust(int DPin, int APin);
void wifiInit(String ssid, String pw);
void wifiStart();

#endif