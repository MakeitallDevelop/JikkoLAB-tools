#include "Arduino.h" 
#include "Jikko.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SPACESHIP_HEIGHT   16
#define SPACESHIP_WIDTH    16

Adafruit_SSD1306 display_p(128, 64, &Wire, -1);

int MY_LIVES = 5;
int MY_SPEED = 5;

static const unsigned char PROGMEM playerGraphic[] =
{ 0x00, 0x00, 0x10, 0x00, 0x28, 0x00, 0x24, 0x00, 0xC2, 0x00, 0xC1, 0x80, 0x20, 0x78, 0x10, 0x07,
0x10, 0x07, 0x20, 0x78, 0xC1, 0x80, 0xC2, 0x00, 0x24, 0x00, 0x28, 0x00, 0x10, 0x00, 0x00, 0x00 };

#define ENEMY_HEIGHT 8
#define ENEMY_WIDTH 8
static const unsigned char PROGMEM enemyGraphic[] =
{
  0x00, 0x3E, 0x64, 0xC7, 0xC7, 0x64, 0x3E, 0x00
};

static const unsigned char PROGMEM heartGraphic[] =
{
  0x38, 0x1C, 0x44, 0x22, 0x82, 0x41, 0x81, 0x81, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02,
  0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM coinGraphic[] =
{
  0x03, 0xC0, 0x0C, 0x30, 0x10, 0x08, 0x21, 0x04, 0x47, 0xC2, 0x45, 0x02, 0x85, 0x01, 0x87, 0xC1,
  0x81, 0x41, 0x81, 0x41, 0x47, 0xC2, 0x41, 0x02, 0x20, 0x04, 0x10, 0x08, 0x0C, 0x30, 0x03, 0xC0
};

static const unsigned char PROGMEM clockGraphic[] =
{
  0x03, 0xC0, 0x0C, 0x30, 0x10, 0x88, 0x20, 0x84, 0x40, 0x82, 0x40, 0x82, 0x80, 0x81, 0x80, 0x81,
  0x8F, 0x81, 0x80, 0x01, 0x40, 0x02, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x0C, 0x30, 0x03, 0xC0
};


// Pins that the joystick is using
#define joystick_x_pin           4
#define joystick_y_pin           13
#define joystick_SW_pin          14

// Make sure the player can't shoot more than (shotDelay / 1000) seconds
static unsigned long lastShot = 0; //ms
static unsigned long shotDelay;

// Variables to make sure the player stays inside the blue section of the OLED
#define MIN_Y 16
#define MAX_Y 50
#define MIN_X 0
#define MAX_X 112

class Player {
  private:
    int _x;
    int _y;
    int _score;
    int _lives;
  public:
    Player(int x, int y, int lives) {
      this->_x=x;
      this->_y=y;
      this->_lives=lives;
      this->_score = 0;
    }

    void Move(int dirX, int dirY) {
      _x += dirX;
      _y += dirY;
  
      _x = constrain(_x, MIN_X, MAX_X);
      _y = constrain(_y, MIN_Y, MAX_Y); 
    }

    void draw() {
      display_p.drawBitmap(_x, _y, playerGraphic, SPACESHIP_WIDTH, SPACESHIP_HEIGHT, WHITE);
    }

    int get_lives() {
      return _lives;
    }

    int get_score() {
      return _score;
    }

    int get_x() {
      return _x;
    }

    int get_y() {
      return _y;
    }

    void add_to_score(int val) {
      _score += val; 
    }

    void remove_lives(int val) {
      _lives -= val;
    }

//기본 세팅값
    void reset() {
      _lives = MY_LIVES;   //라이프 (수정가능하게 해주세요 1~9)
      _score = 0;   //점수
      _x = 0;       //X 시작위치
      _y = 64 / 2;  //Y 시작위치
    }
};


struct bounds {
  int x;
  int y;
  int w;
  int h;
};

struct bullet {
  int x;
  int y;
  int dir = 1;
  int moveSpeed;
  int maxX = 128;
  bool live;
  bounds boundingBox;

  void Draw() {
    display_p.drawCircle(x, y, 3, WHITE);
  }

  void Move() {
    if (x >=maxX) {
      live = false;
    }
    x += dir * moveSpeed;
    calc_bounds();
  }

  void calc_bounds() {
    boundingBox.x = x;
    boundingBox.y = y;
    boundingBox.w = ENEMY_WIDTH;
    boundingBox.h = ENEMY_HEIGHT;
  }
};

struct enemy {
  int x;
  int y;
  int dir = -1;
  int moveSpeed;
  bounds boundingBox;

  void Move() {
    x += dir * moveSpeed;
    calc_bounds();
  }

  void Draw() {
    display_p.drawBitmap(x, y, enemyGraphic, ENEMY_WIDTH, ENEMY_HEIGHT, WHITE);
  }

  void calc_bounds() {
    boundingBox.x = x;
    boundingBox.y = y;
    boundingBox.w = ENEMY_WIDTH;
    boundingBox.h = ENEMY_HEIGHT;
  }
};

const int maxBullets = 5;
const int maxEnemies = 13;

int bulletCount;
bullet bullets[maxBullets];

int enemyCount;
enemy enemies[maxEnemies];
int enemySpeed;
int enemySpawnChance;

Player player = Player(0, 64 / 2, MY_LIVES);
int i;
int j;

void initialSetup() {
  bulletCount = 0;
  enemyCount = 0;
  enemySpeed = 1;
  enemySpawnChance = 92;
  shotDelay = 600;
  lastShot = 0;

  for(i = 0; i < maxBullets; i++) {
    bullets[i].live = false;
  }
  
  for (i = 0; i < maxEnemies; i++) {
    enemies[i].x = 0;
    enemies[i].y = 64;
  }

}

void shootGameInit(int life, int speed)
{
    MY_LIVES = life;
    MY_SPEED = speed;
    initialSetup();
    player = Player(0, 64 / 2, MY_LIVES);
    pinMode(joystick_SW_pin, INPUT_PULLUP);

    display_p.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display_p.setTextColor(WHITE, BLACK);
    display_p.clearDisplay();
    display_p.setTextSize(1);
}

void defeatScreen() {
  unsigned long currentMillis;
  unsigned long previousMillis = 0;
  long interval = 2000;

  display_p.clearDisplay();
  display_p.setCursor(10, 0);
  display_p.setTextSize(1);
  display_p.println(F("Game Over"));

  display_p.setCursor(10, 15);
  display_p.print(F("Total Score: "));
  display_p.println(player.get_score());
  display_p.display();
  delay(2000);
  
  while(digitalRead(joystick_SW_pin) != 0) {
    currentMillis = millis();

    if (currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;
    }
  }
}

int readValue(int pin, int rangeLow, int rangeHigh, int newRangeLow, int newRangeHigh) {
  int val = analogRead(pin);
  // Deadzone of the joystick
  if (val >= 480 && val <= 530){
    val = 512;
  }
  return map(val, rangeLow, rangeHigh, newRangeLow, newRangeHigh);
}

// Bullet Things
void createNewBullet() {
  bullets[bulletCount].x = player.get_x() + SPACESHIP_WIDTH;
  bullets[bulletCount].y = player.get_y() + (SPACESHIP_HEIGHT / 2);
  bullets[bulletCount].moveSpeed = 5;
  bullets[bulletCount].live = true;
  bullets[bulletCount].calc_bounds();
  bulletCount++;
  if (bulletCount > maxBullets - 1)
    bulletCount = 0;
}

void createNewEnemy() {
  if (enemies[enemyCount].x > 0) {
    return;
  }
  enemies[enemyCount].x = 127;
  enemies[enemyCount].y = random(MIN_Y, MAX_Y);
  enemies[enemyCount].moveSpeed = enemySpeed;
  enemies[enemyCount].calc_bounds();
  enemyCount++;
  if (enemyCount > maxEnemies - 1)
    enemyCount = 0;
}

void moveEnemies() {
  for(int i = 0; i < maxEnemies; i++) {
    enemies[i].Move();
  }
}

void moveBullets() {
  for(i = 0; i < maxBullets; i++) {
    if (bullets[i].live){
      bullets[i].Move();
    }
  }
}

void drawStats(int num, int stat, int offsetLabel, int offsetStat) {
  switch(num) {
    case 0: 
      display_p.drawBitmap(offsetLabel, 0, coinGraphic, SPACESHIP_WIDTH, SPACESHIP_HEIGHT, WHITE);
      break;
    case 1: 
      display_p.drawBitmap(offsetLabel, 0, clockGraphic, SPACESHIP_WIDTH, SPACESHIP_HEIGHT, WHITE);
      break;
    case 2: 
      display_p.drawBitmap(offsetLabel, 0, heartGraphic, SPACESHIP_WIDTH, SPACESHIP_HEIGHT, WHITE);
      break;
    default: 
      display_p.drawBitmap(offsetLabel, 0, coinGraphic, SPACESHIP_WIDTH, SPACESHIP_HEIGHT, WHITE);
  }
  display_p.setCursor(offsetStat, 0);
  display_p.print(stat);
}

bounds getPlayerBounds() {
  bounds retBounds{player.get_x(), player.get_y(), SPACESHIP_WIDTH, SPACESHIP_HEIGHT};
  return retBounds;
}

// Collision checking
bool hasCollided(bounds boundsOne, bounds boundsTwo) {
  bool collided = false;
  int xMin1, xMax1, yMin1, yMax1;
  int xMin2, xMax2, yMin2, yMax2;

  xMin1 = boundsOne.x;
  xMax1 = boundsOne.x + boundsOne.w;
  yMin1 = boundsOne.y;
  yMax1 = boundsOne.y + boundsOne.h;

  xMin2 = boundsTwo.x;
  xMax2 = boundsTwo.x + boundsTwo.w;
  yMin2 = boundsTwo.y;
  yMax2 = boundsTwo.y + boundsTwo.h;

  return (xMin1 < xMax2 && xMin2 < xMax1 && yMin1 < yMax2 && yMin2 < yMax1);
}

// Rendering methods
void drawEnemies() {
  for(i = 0; i < maxEnemies; i++) {
    if (enemies[i].y > MIN_Y)
      enemies[i].Draw();
  }  
}

void drawBullets() {
  for(i = 0; i < maxBullets; i++) {
    if (bullets[i].live && bullets[i].x >= SPACESHIP_WIDTH) {
      bullets[i].Draw();
    }  
  }
}

void shootGame() {

  while (true) {
      display_p.clearDisplay();

      // Should spawn enemy?
      if (random(100) > enemySpawnChance){
        createNewEnemy();
      }


      // 유저 움직이기
      int newX;
      int newY;

      
      
      
      if(analogRead(joystick_x_pin) > 4000) {
          newX = MY_SPEED;
        }
      else {
      if(analogRead(joystick_x_pin) < 50) {
          newX = -MY_SPEED;
        }
      else
      {
          newX = 0;
        }
      }

      if(analogRead(joystick_y_pin) > 4000) {
          newY = MY_SPEED;
        }
      else {
      if(analogRead(joystick_y_pin) < 50) {
          newY = -MY_SPEED;
        }
      else
      {
          newY = 0;
        }
      }
 
      player.Move(newX, newY);

      if (digitalRead(joystick_SW_pin) == 0) {
        if (millis() >= lastShot) {
          lastShot = millis() + shotDelay;;
          createNewBullet();
        }
      }

      moveBullets();
      moveEnemies();

      // 충돌체크
      for(i = 0; i < maxEnemies; i++){
        if(enemies[i].x > 0) {
          if (hasCollided(enemies[i].boundingBox, getPlayerBounds())){
            enemies[i].x = 0;
            enemies[i].y = 0;
            player.remove_lives(1);
            if (player.get_lives() < 0) {
              defeatScreen();
              player.reset();
              initialSetup();
              continue;
            }
          }
        }
        
        for(j = 0; j < maxBullets; j++) {
          if(bullets[j].live && enemies[i].x > 0){
            if (hasCollided(bullets[j].boundingBox, enemies[i].boundingBox)){
              enemies[i].x = 0;
              enemies[i].y = 0;
              bullets[j].live = false;

              player.add_to_score(5);
              if (player.get_score() % 50 == 0) {
                enemySpeed++;
                enemySpawnChance -= 3;
                shotDelay -= 50;
              }
            }
          }
        }
      }

      // Render
      player.draw();
      drawBullets();
      drawEnemies();
      
      drawStats(0, player.get_score(), 0, 20);
      drawStats(1, enemySpeed, 52, 72);
      drawStats(2, player.get_lives(), 98, 118);
      display_p.drawLine(0, MIN_Y-1, 128-1, MIN_Y-1, WHITE);
      
      display_p.display();
      delay(50);
  }
}