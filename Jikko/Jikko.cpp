#include "Arduino.h" 
#include "Jikko.h"
#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

float prevVal;

//PINGPONG=================================

Adafruit_SSD1306 display_j(128, 64, &Wire, -1);

const int SW_pin = 14; // 다시 시작 번호
const int Y_pin = 13; // 보드 움직이는 번호

const unsigned long PADDLE_RATE = 5;  //보드 움직이는 속도
const unsigned long BALL_RATE = 1;  
 uint8_t PADDLE_HEIGHT_JIKKO = 15;   //보드길이 (수정 가능하게 해주세요)
 uint8_t PADDLE_HEIGHT_USER = 30;    //USER 보드길이 (수정 가능하게 해주세요)

int playerScore = 0;
int aiScore = 0;
int maxScore = 8;
bool resetBall = false;

void drawCourt();
void drawScore();

uint8_t ball_x = 64, ball_y = 32;      //공 시작위치
uint8_t ball_dir_x = 1, ball_dir_y = 1;
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 22;
uint8_t cpu_y = 26;

const uint8_t PLAYER_X = 105;
uint8_t player_y = 6;


// WIFI
Adafruit_SSD1306 display_w(128, 64, &Wire, -1);

//==========================================

void wifiInit(String ssid, String pw){
  
  WiFi.begin(ssid.c_str(), pw.c_str());
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);


  display_w.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display_w.setTextColor(WHITE, BLACK);
  display_w.clearDisplay();
  display_w.setTextSize(1);
  }
  configTime(32400, 0, "pool.ntp.org");

 struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return;
  }
  display_w.setCursor(0, 0);
  display_w.println(&timeinfo, "%A");
  display_w.setCursor(0, 20);
  display_w.println(&timeinfo, "%B %d %Y");
  display_w.setCursor(0, 40);
  display_w.println(&timeinfo, "%H:%M:%S");
  display_w.display();
  display_w.fillRect(0, 0, 128, 64, BLACK);
  WiFi.disconnect(true);
  
  WiFi.mode(WIFI_OFF);
}

void wifiStart()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return;
  }
  display_w.setCursor(0, 0);
  display_w.println(&timeinfo, "%A");
  display_w.setCursor(0, 20);
  display_w.println(&timeinfo, "%B %d %Y");
  display_w.setCursor(0, 40);
  display_w.println(&timeinfo, "%H:%M:%S");
  display_w.display();
  display_w.fillRect(0, 0, 128, 64, BLACK);
}

int getMultiBTData1(String str, int number)
{
  int i1 = str.toInt();

  return i1;
}

int getMultiBTData2(String str, int number) {
  int firstcom = str.indexOf(",");
  int strlength = str.length();

  String v1 = str.substring(0, firstcom);
  String v2 = str.substring(firstcom+1, strlength);
  
  int i1 = v1.toInt();
  int i2 = v2.toInt();

  if(number == 1) {
    return i1;
  }
  else if(number == 2) {
    return i2;
  }
}

int getMultiBTData3(String str, int number) {
  int firstcom = str.indexOf(",");
  int secondcom = str.indexOf(",", firstcom+1);
  int strlength = str.length();

  String v1 = str.substring(0, firstcom);
  String v2 = str.substring(firstcom+1, secondcom);
  String v3 = str.substring(secondcom+1, strlength);
  
  int i1 = v1.toInt();
  int i2 = v2.toInt();
  int i3 = v3.toInt();

  if(number == 1) {
    return i1;
  }
  else if(number == 2) {
    return i2;
  }
  else if(number == 3) {
    return i3;
  }
}

void drawCourt() {
    display_j.drawRect(0, 0, 128, 64, WHITE);
}
void drawScore() {
  display_j.setCursor(45, 4);
  display_j.println(aiScore);
  display_j.setCursor(75, 4);
  display_j.println(playerScore);
}

//게임 승패 판단
void gameOver(){ 
  display_j.fillScreen(BLACK);
  if(playerScore>aiScore)
  {
    display_j.setCursor(20,8);
    display_j.print("YOU WIN");
  }
  else {
    if(playerScore<aiScore) {
    display_j.setCursor(10,8);
    display_j.print("JIKKO WIN");    
  }
  else
   {    display_j.setCursor(45,8);
    display_j.print("DRAW");
      }
    }
  
 delay(200);
 display_j.display();
 delay(2000);
 aiScore = playerScore = 0;
  
unsigned long start = millis();
while(millis() - start < 2000);
ball_update = millis();    
paddle_update = ball_update;
resetBall=true;
}

void pingpongGameInit(int userBoard, int myBoard){

  PADDLE_HEIGHT_JIKKO=myBoard;
  PADDLE_HEIGHT_USER=userBoard;
  
  //SETUP====================
    display_j.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display_j.setTextColor(WHITE, BLACK);
    display_j.clearDisplay();
    display_j.setTextSize(1);

    unsigned long start = millis();
    pinMode(SW_pin, INPUT_PULLUP);
    display_j.clearDisplay();
    drawCourt();
    drawScore();  
    while(millis() - start < 2000);
    ball_update = millis();
    paddle_update = ball_update;
    //=============================

}


void pingpongGame(){
   bool update = false;
  unsigned long time = millis();

  static bool up_state = false;
  static bool down_state = false;

  if (resetBall)
  {
    if (playerScore == maxScore || aiScore == maxScore)
    {
      gameOver();
    }
    else
    {
      display_j.fillScreen(BLACK);
      drawScore();
      drawCourt();
      ball_x = random(62, 66);
      ball_y = random(30, 34);
      do
      {
        ball_dir_x = random(-1, 2);
      } while (ball_dir_x == 0);

      do
      {
        ball_dir_y = random(-1, 2);
      } while (ball_dir_y == 0);

      resetBall = false;
    }
  }

  if (time > ball_update)
  {
    uint8_t new_x = ball_x + ball_dir_x;
    uint8_t new_y = ball_y + ball_dir_y;

    // 벽에 닿았을 때
    if (new_x == 0 || new_x == 127)
    {

      if (new_x == 0)
      {
        playerScore += 1;
        display_j.fillScreen(BLACK);
        resetBall = true;
      }
      else if (new_x == 127)
      {
        aiScore += 1;
        display_j.fillScreen(BLACK);
        resetBall = true;
      }
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    // 벽에 닿았을 때
    if (new_y == 0 || new_y == 63)
    {
      ball_dir_y = -ball_dir_y;
      new_y += ball_dir_y + ball_dir_y;
    }

    // JIKKO PADDLE에 닿았을 때
    if (new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT_JIKKO)
    {
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    // USER PADDLE에 닿았을 때
    if (new_x == PLAYER_X && new_y >= player_y && new_y <= player_y + PADDLE_HEIGHT_USER)
    {
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    display_j.drawPixel(ball_x, ball_y, BLACK);
    display_j.drawPixel(new_x, new_y, WHITE);
    ball_x = new_x;
    ball_y = new_y;

    ball_update += BALL_RATE;

    update = true;
  }

  if (time > paddle_update)
  {
    paddle_update += PADDLE_RATE;

    // JIKKO paddle 움직이기
    display_j.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT_JIKKO, BLACK);
    const uint8_t half_paddle = PADDLE_HEIGHT_JIKKO >> 1;
    if (cpu_y + half_paddle > ball_y)
    {
      cpu_y -= 1;
    }
    if (cpu_y + half_paddle < ball_y)
    {
      cpu_y += 1;
    }
    if (cpu_y < 1)
      cpu_y = 1;
    if (cpu_y + PADDLE_HEIGHT_JIKKO > 63)
      cpu_y = 63 - PADDLE_HEIGHT_JIKKO;
    display_j.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT_JIKKO, WHITE);

    // USER paddle 움직이기
    display_j.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT_USER, BLACK);
    if (analogRead(Y_pin) < 4000)
    {
      player_y -= 1;
    }
    if (analogRead(Y_pin) > 50)
    {
      player_y += 1;
    }
    up_state = down_state = false;
    if (player_y < 1)
      player_y = 1;
    if (player_y + PADDLE_HEIGHT_USER > 63)
      player_y = 63 - PADDLE_HEIGHT_USER;
    display_j.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT_USER, WHITE);
  }
  update = true;

  if (update)
  {
    drawScore();
    display_j.display();
    if (digitalRead(SW_pin) == 0)
    {
      gameOver();
    }
  }
}

float getDust(int led, int sensor) {

  float dust;
  float sensitivity = 0.1;

  digitalWrite(led, LOW);
  delayMicroseconds(280);
  dust = analogRead(sensor);
  delayMicroseconds(40);
  digitalWrite(led, HIGH);
  delayMicroseconds(9680);

  dust = (0.143 * (dust * 0.0049) - 0.03) * 1000;
  float filteredVal = (prevVal * (1 - sensitivity)) + (dust * sensitivity);
  prevVal = filteredVal;
  dust = filteredVal;

  if (dust < 100) {
    dust = dust / 4;
  } else if (dust > 100 && dust < 200) {
    dust = (dust / 4) * 1.15;
  } else if (dust > 200 && dust < 300) {
    dust = (dust / 4) * 1.15 * 1.3;
  } else if (dust > 300 && dust < 400) {
    dust = (dust / 4) * 1.15 * 1.3 * 1.45;
  } else if (dust > 400 && dust < 500) {
    dust = (dust / 4) * 1.15 * 1.3 * 1.45 * 1.6;
  }

  return dust;

}