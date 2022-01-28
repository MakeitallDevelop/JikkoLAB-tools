#include "Arduino.h"

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define REST 0

const int pacmanMelody[] PROGMEM = {
    // Pacman
    // Score available at https://musescore.com/user/85429/scores/107109
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8,
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8,
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8,
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8,
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8,
    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
    NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
    NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

    NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, //2
    NOTE_FS5, -16, NOTE_DS5, 8, NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32,
    NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8};

const int takeonmeMelody[] PROGMEM = {
    NOTE_FS5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_B4,
    8,
    REST,
    8,
    NOTE_B4,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    NOTE_GS5,
    8,
    NOTE_GS5,
    8,
    NOTE_A5,
    8,
    NOTE_B5,
    8,
    NOTE_A5,
    8,
    NOTE_A5,
    8,
    NOTE_A5,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_D5,
    8,
    REST,
    8,
    NOTE_FS5,
    8,
    REST,
    8,
    NOTE_FS5,
    8,
    REST,
    8,
    NOTE_FS5,
    8,
    NOTE_E5,
    8,
    NOTE_E5,
    8,
    NOTE_FS5,
    8,
    NOTE_E5,
    8,
    NOTE_FS5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_B4,
    8,
    REST,
    8,
    NOTE_B4,
    8,
    REST,
    8,
    NOTE_E5,
    8,

    REST,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    NOTE_GS5,
    8,
    NOTE_GS5,
    8,
    NOTE_A5,
    8,
    NOTE_B5,
    8,
    NOTE_A5,
    8,
    NOTE_A5,
    8,
    NOTE_A5,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_D5,
    8,
    REST,
    8,
    NOTE_FS5,
    8,
    REST,
    8,
    NOTE_FS5,
    8,
    REST,
    8,
    NOTE_FS5,
    8,
    NOTE_E5,
    8,
    NOTE_E5,
    8,
    NOTE_FS5,
    8,
    NOTE_E5,
    8,
    NOTE_FS5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_B4,
    8,
    REST,
    8,
    NOTE_B4,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    REST,
    8,
    NOTE_E5,
    8,
    NOTE_GS5,
    8,
    NOTE_GS5,
    8,
    NOTE_A5,
    8,
    NOTE_B5,
    8,
};

const int cannonMelody[] PROGMEM = {

    NOTE_D4,
    4,
    NOTE_FS4,
    8,
    NOTE_G4,
    8,
    NOTE_A4,
    4,
    NOTE_FS4,
    8,
    NOTE_G4,
    8,
    NOTE_A4,
    4,
    NOTE_B3,
    8,
    NOTE_CS4,
    8,
    NOTE_D4,
    8,
    NOTE_E4,
    8,
    NOTE_FS4,
    8,
    NOTE_G4,
    8,
    NOTE_FS4,
    4,
    NOTE_D4,
    8,
    NOTE_E4,
    8,
    NOTE_FS4,
    4,
    NOTE_FS3,
    8,
    NOTE_G3,
    8,
    NOTE_A3,
    8,
    NOTE_G3,
    8,
    NOTE_FS3,
    8,
    NOTE_G3,
    8,
    NOTE_A3,
    2,
    NOTE_G3,
    4,
    NOTE_B3,
    8,
    NOTE_A3,
    8,
    NOTE_G3,
    4,
    NOTE_FS3,
    8,
    NOTE_E3,
    8,
    NOTE_FS3,
    4,
    NOTE_D3,
    8,
    NOTE_E3,
    8,
    NOTE_FS3,
    8,
    NOTE_G3,
    8,
    NOTE_A3,
    8,
    NOTE_B3,
    8,

    NOTE_G3,
    4,
    NOTE_B3,
    8,
    NOTE_A3,
    8,
    NOTE_B3,
    4,
    NOTE_CS4,
    8,
    NOTE_D4,
    8,
    NOTE_A3,
    8,
    NOTE_B3,
    8,
    NOTE_CS4,
    8,
    NOTE_D4,
    8,
    NOTE_E4,
    8,
    NOTE_FS4,
    8};

const int odetojoyMelody[] PROGMEM = {
    NOTE_E4, 4, NOTE_E4, 4, NOTE_F4, 4, NOTE_G4, 4, //1
    NOTE_G4, 4, NOTE_F4, 4, NOTE_E4, 4, NOTE_D4, 4,
    NOTE_C4, 4, NOTE_C4, 4, NOTE_D4, 4, NOTE_E4, 4,
    NOTE_E4, -4, NOTE_D4, 8, NOTE_D4, 2,

    NOTE_E4, 4, NOTE_E4, 4, NOTE_F4, 4, NOTE_G4, 4, //4
    NOTE_G4, 4, NOTE_F4, 4, NOTE_E4, 4, NOTE_D4, 4,
    NOTE_C4, 4, NOTE_C4, 4, NOTE_D4, 4, NOTE_E4, 4,
    NOTE_D4, -4, NOTE_C4, 8, NOTE_C4, 2,

    NOTE_D4, 4, NOTE_D4, 4, NOTE_E4, 4, NOTE_C4, 4, //8
    NOTE_D4, 4, NOTE_E4, 8, NOTE_F4, 8, NOTE_E4, 4, NOTE_C4, 4,
    NOTE_D4, 4, NOTE_E4, 8, NOTE_F4, 8, NOTE_E4, 4, NOTE_D4, 4,
    NOTE_C4, 4, NOTE_D4, 4, NOTE_G3, 2};

const int pinnkpantherMelody[] PROGMEM = {
    REST,
    2,
    REST,
    4,
    REST,
    8,
    NOTE_DS4,
    8,
    NOTE_E4,
    -4,
    REST,
    8,
    NOTE_FS4,
    8,
    NOTE_G4,
    -4,
    REST,
    8,
    NOTE_DS4,
    8,
    NOTE_E4,
    -8,
    NOTE_FS4,
    8,
    NOTE_G4,
    -8,
    NOTE_C5,
    8,
    NOTE_B4,
    -8,
    NOTE_E4,
    8,
    NOTE_G4,
    -8,
    NOTE_B4,
    8,
    NOTE_AS4,
    2,
    NOTE_A4,
    -16,
    NOTE_G4,
    -16,
    NOTE_E4,
    -16,
    NOTE_D4,
    -16,
    NOTE_E4,
    2,
    REST,
    4,
    REST,
    8,
    NOTE_DS4,
    4,

    NOTE_E4,
    -4,
    REST,
    8,
    NOTE_FS4,
    8,
    NOTE_G4,
    -4,
    REST,
    8,
    NOTE_DS4,
    8,
    NOTE_E4,
    -8,
    NOTE_FS4,
    8,
    NOTE_G4,
    -8,
    NOTE_C5,
    8,
    NOTE_B4,
    -8,
    NOTE_G4,
    8,
    NOTE_B4,
    -8,
    NOTE_E5,
    8,
    NOTE_DS5,
    1,
    NOTE_D5,
    2,
    REST,
    4,
    REST,
    8,
    NOTE_DS4,
    8,
    NOTE_E4,
    -4,
    REST,
    8,
    NOTE_FS4,
    8,
    NOTE_G4,
    -4,
    REST,
    8,
    NOTE_DS4,
    8,
    NOTE_E4,
    -8,
    NOTE_FS4,
    8,
    NOTE_G4,
    -8,
    NOTE_C5,
    8,
    NOTE_B4,
    -8,
    NOTE_E4,
    8,
    NOTE_G4,
    -8,
    NOTE_B4,
    8

};

const int harrypotterMelody[] PROGMEM = {
    REST,
    2,
    NOTE_D4,
    4,
    NOTE_G4,
    -4,
    NOTE_AS4,
    8,
    NOTE_A4,
    4,
    NOTE_G4,
    2,
    NOTE_D5,
    4,
    NOTE_C5,
    -2,
    NOTE_A4,
    -2,
    NOTE_G4,
    -4,
    NOTE_AS4,
    8,
    NOTE_A4,
    4,
    NOTE_F4,
    2,
    NOTE_GS4,
    4,
    NOTE_D4,
    -1,
    NOTE_D4,
    4,

    NOTE_G4,
    -4,
    NOTE_AS4,
    8,
    NOTE_A4,
    4, //10
    NOTE_G4,
    2,
    NOTE_D5,
    4,
    NOTE_F5,
    2,
    NOTE_E5,
    4,
    NOTE_DS5,
    2,
    NOTE_B4,
    4,
    NOTE_DS5,
    -4,
    NOTE_D5,
    8,
    NOTE_CS5,
    4,
    NOTE_CS4,
    2,
    NOTE_B4,
    4,
    NOTE_G4,
    -1,
    NOTE_AS4,
    4};

const int bearMelody[] PROGMEM = {NOTE_C5, 4, NOTE_C5, 8, NOTE_C5, 8, NOTE_C5, 4, NOTE_C5, 4,             //도도도도도 [곰 세마리가]
                                  NOTE_E5, 4, NOTE_G5, 8, NOTE_G5, 8, NOTE_E5, 4, NOTE_C5, 4,             //미솔솔미도 [한 집에 있어
                                  NOTE_G5, 8, NOTE_G5, 8, NOTE_E5, 4, NOTE_G5, 8, NOTE_G5, 8, NOTE_E5, 4, //솔솔미솔솔미 [아빠곰 엄마곰]
                                  NOTE_C5, 4, NOTE_C5, 4, NOTE_C5, 2,                                     //도도도 [애기곰]
                                  NOTE_G5, 4, NOTE_G5, 4, NOTE_E5, 4, NOTE_C5, 4,                         //솔솔미도 [아빠곰은]
                                  NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 2,                                     //솔솔솔 [뚱뚱해]
                                  NOTE_G5, 4, NOTE_G5, 4, NOTE_E5, 4, NOTE_C5, 4,                         //솔솔미도 [엄마곰은]
                                  NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 2,                                     //솔솔솔 [날씬해]
                                  NOTE_G5, 4, NOTE_G5, 4, NOTE_E5, 4, NOTE_C5, 4,                         //솔솔미도 [애기곰은]
                                  NOTE_G5, 8, NOTE_G5, 8, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 2,             //솔솔솔라솔 [너무귀여워]
                                  NOTE_C6, 4, NOTE_G5, 4, NOTE_C6, 4, NOTE_G5, 4,                         //도솔도솔 [으쓱으쓱]
                                  NOTE_E5, 4, NOTE_D5, 4, NOTE_C5, 2};                                    //미레도 [잘한다]}

const int circleMelody[] PROGMEM = {
    NOTE_E4, 12, NOTE_E4, 12, NOTE_E4, 12, NOTE_C4, 6, NOTE_D4, 16, NOTE_E4, 2,                         // 둥글게 둥글게
    NOTE_E4, 12, NOTE_E4, 12, NOTE_E4, 12, NOTE_C4, 6, NOTE_D4, 16, NOTE_E4, 2,                         // 둥글게 둥글게
    NOTE_F4, 6, NOTE_E4, 16, NOTE_F4, 6, NOTE_G4, 16, NOTE_A4, 6, NOTE_A4, 16, NOTE_G4, 6, NOTE_F4, 16, // 빙글빙글돌아가며
    NOTE_E4, 6, NOTE_E4, 16, NOTE_F4, 6, NOTE_F4, 16, NOTE_E4, 2,                                       // 춤을 춥시다
    NOTE_E4, 12, NOTE_E4, 12, NOTE_E4, 12, NOTE_C4, 6, NOTE_D4, 16, NOTE_E4, 2,                         // 손뼉을 치면서
    NOTE_E4, 12, NOTE_E4, 12, NOTE_E4, 12, NOTE_C4, 6, NOTE_D4, 16, NOTE_E4, 2,                         // 노래를 부르며
    NOTE_F4, 6, NOTE_E4, 16, NOTE_F4, 6, NOTE_B4, 16, NOTE_A4, 6, NOTE_G4, 16, NOTE_F4, 6, NOTE_G4, 16, // 랄라랄라즐거웁게
    NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 2,                                                                 // 춤추자
    NOTE_C5, 6, NOTE_C5, 16, NOTE_C5, 6, NOTE_C5, 16, NOTE_C5, 4, NOTE_B4, 4,                           // 링가링가 링가
    NOTE_A4, 6, NOTE_A4, 16, NOTE_A4, 6, NOTE_F4, 16, NOTE_E4, 2,                                       // 링가링가링
    NOTE_B4, 6, NOTE_B4, 16, NOTE_B4, 6, NOTE_B4, 16, NOTE_B4, 4, NOTE_C5, 4,                           // 링가링가 링가
};

const int tigerMelody[] PROGMEM = {
    NOTE_C5, 2, NOTE_G4, 2, NOTE_E4, 8, NOTE_D4, 8, NOTE_E4, 8, NOTE_D4, 8, NOTE_E4, 4, NOTE_C4, 4,                         //도 솔 미레미레미도 [산중 호걸이라 하는]
    NOTE_G4, 4, NOTE_C4, 4, NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 8, NOTE_D4, 8, NOTE_E4, 8, NOTE_D4, 8, NOTE_E4, 4, NOTE_C4, 4, //솔도미솔 미레미레미도 [호랑님의 생일날이되어]
    NOTE_C5, 4, NOTE_C5, 4, NOTE_G4, 4, NOTE_G4, 4, NOTE_E4, 8, NOTE_D4, 8, NOTE_E4, 8, NOTE_D4, 8, NOTE_E4, 4, NOTE_C4, 4, // 도도 솔솔 미레미레미도 [각색짐승 공원에 모여]
    NOTE_G4, 4, NOTE_C4, 4, NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 4, NOTE_D4, 4, NOTE_C4, 2,                                     //솔도미솔 미레도 [무도회가 열렸네]
    NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 2, NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 2,                                                 // 라라라 라라라 [토끼는 춤추고]
    NOTE_G4, 4, NOTE_C5, 4, NOTE_C5, 2, NOTE_A4, 8, NOTE_G4, 8, NOTE_A4, 4, NOTE_G4, 2                                      //솔도도 라솔라솔 [여우는 바이올린]
};

const int rudolfMelody[] PROGMEM = {
    NOTE_G4, 8, NOTE_A4, 4, NOTE_G4, 8, NOTE_E4, 4, NOTE_C5, 4, NOTE_A4, 4, NOTE_G4, 2,          // 솔라솔미도라솔,  루돌프 사슴코는
    NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 4, NOTE_C5, 4, NOTE_B4, 2, REST, 4, // 솔라솔라솔도시 (0=쉼표),  매우 반짝이는 코
    NOTE_F4, 8, NOTE_G4, 4, NOTE_F4, 8, NOTE_D4, 4, NOTE_B4, 4, NOTE_A4, 4, NOTE_G4, 2,          // 파솔파레시라솔,  만일 네가 봤다면
    NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 4, NOTE_A4, 4, NOTE_E4, 2, REST, 4, // 솔라솔라솔라미 (0=쉼표),  불붙는다 했겠지
    NOTE_G4, 8, NOTE_A4, 4, NOTE_G4, 8, NOTE_E4, 4, NOTE_C5, 4, NOTE_A4, 4, NOTE_G4, 2,          // 솔라솔미도라솔 (0=쉼표),  다른 모든 사슴들
    NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 4, NOTE_C5, 4, NOTE_B4, 2, REST, 4, // 솔라솔라솔도시 (0=쉼표),  놀려대며 웃었네
};

const int starMelody[] PROGMEM = {
    NOTE_C4, 4, NOTE_C4, 4, NOTE_G4, 4, NOTE_G4, 4, NOTE_A4, 4, NOTE_A4, 4, NOTE_G4, 2, // 반짝반짝 작은별
    NOTE_F4, 4, NOTE_F4, 4, NOTE_E4, 4, NOTE_E4, 4, NOTE_D4, 4, NOTE_D4, 4, NOTE_C4, 2, // 아름답게 비치네
    NOTE_G4, 4, NOTE_G4, 4, NOTE_F4, 4, NOTE_F4, 4, NOTE_E4, 4, NOTE_E4, 4, NOTE_D4, 2, // 동쪽하늘 에서도
    NOTE_G4, 4, NOTE_G4, 4, NOTE_F4, 4, NOTE_F4, 4, NOTE_E4, 4, NOTE_E4, 4, NOTE_D4, 2, // 서쪽하늘 에서도
    NOTE_C4, 4, NOTE_C4, 4, NOTE_G4, 4, NOTE_G4, 4, NOTE_A4, 4, NOTE_A4, 4, NOTE_G4, 2, // 반짝반짝 작은별
    NOTE_F4, 4, NOTE_F4, 4, NOTE_E4, 4, NOTE_E4, 4, NOTE_D4, 4, NOTE_D4, 4, NOTE_C4, 2, // 아름답게 비치네

};

long previousMillis = 0;

void playMelody(const int melody[], int melodySize, int tempo, int sec, int channel)
{
    // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
    // there are two values per note (pitch and duration), so for each note there are four bytes
    int notes = melodySize / sizeof(melody[0]) / 2;

    // this calculates the duration of a whole note in ms (60s/tempo)*4 beats
    int wholenote = (60000 * 4) / tempo;

    int divider = 0, noteDuration = 0;
    long interval = sec * 1000;

    int buzzer = 6;

    previousMillis = millis();

    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2)
    {
        unsigned long currentMillis = millis();
        
        if (currentMillis - previousMillis < interval) {
          // calculates the duration of each note
          divider = pgm_read_word_near(&(melody[thisNote + 1]));
          if (divider > 0)
          {
              // regular note, just proceed
              noteDuration = (wholenote) / divider;
          }
          else if (divider < 0)
          {
              // dotted notes are represented with negative durations!!
              noteDuration = (wholenote) / abs(divider);
              noteDuration *= 1.5; // increases the duration in half for dotted notes
          }
          // we only play the note for 90% of the duration, leaving 10% as a pause
          ledcWriteTone(channel, pgm_read_word_near(&(melody[thisNote])));

          // Wait for the specief duration before playing the next note.
          delay(noteDuration);

          // stop the waveform generation before the next note.
          ledcWriteTone(channel, 0);
        }
        
        else {
          ledcWriteTone(channel, 0);
          break;
        }
        
    }

    

    // for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2)
    // {
    //     unsigned long currentMillis = millis();
    //     if (currentMillis - previousMillis > interval)
    //     {
    //         Serial.println("do");
    //         previousMillis = currentMillis;
    //         ledcWriteTone(channel, 0);
    //         break;
    //     }
    //     // calculates the duration of each note
    //     divider = pgm_read_word_near(&(melody[thisNote + 1]));
    //     if (divider > 0)
    //     {
    //         // regular note, just proceed
    //         noteDuration = (wholenote) / divider;
    //     }
    //     else if (divider < 0)
    //     {
    //         // dotted notes are represented with negative durations!!
    //         noteDuration = (wholenote) / abs(divider);
    //         noteDuration *= 1.5; // increases the duration in half for dotted notes
    //     }
    //     // we only play the note for 90% of the duration, leaving 10% as a pause
    //     ledcWriteTone(channel, pgm_read_word_near(&(melody[thisNote])));

    //     // Wait for the specief duration before playing the next note.
    //     delay(noteDuration);

    //     // stop the waveform generation before the next note.
    //     ledcWriteTone(channel, 0);
    // }
}

void playBuzzerMusic(int music, int sec, int channel)
{
    const int musicTempo[] = {105, 140, 100, 114, 120, 144, 114, 114, 114, 114, 114};

    switch (music)
    {
    case 0:
        playMelody(bearMelody, sizeof(bearMelody), musicTempo[music], sec, channel);
        break;
    case 1:
        playMelody(circleMelody, sizeof(circleMelody), musicTempo[music], sec, channel);
        break;
    case 2:
        playMelody(tigerMelody, sizeof(tigerMelody), musicTempo[music], sec, channel);
        break;
    case 3:
        playMelody(rudolfMelody, sizeof(rudolfMelody), musicTempo[music], sec, channel);
        break;
    case 4:
        playMelody(starMelody, sizeof(starMelody), musicTempo[music], sec, channel);
        break;
    case 5:
        playMelody(takeonmeMelody, sizeof(takeonmeMelody), musicTempo[music], sec, channel);
        break;
    case 6:
        playMelody(pacmanMelody, sizeof(pacmanMelody), musicTempo[music], sec, channel);
        break;
    case 7:
        playMelody(cannonMelody, sizeof(cannonMelody), musicTempo[music], sec, channel);
        break;
    case 8:
        playMelody(odetojoyMelody, sizeof(odetojoyMelody), musicTempo[music], sec, channel);
        break;
    case 9:
        playMelody(pinnkpantherMelody, sizeof(pinnkpantherMelody), musicTempo[music], sec, channel);
        break;
    case 10:
        playMelody(harrypotterMelody, sizeof(harrypotterMelody), musicTempo[music], sec, channel);
        break;
    }
}


long previousTime = 0;

void chatbotMove1(int num, float lpwm, float rpwm)
{
    switch (num)
    {
    case 0:
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(50);
        break;
    case 1:
        ledcWrite(0, lpwm); ledcWrite(1, 0); ledcWrite(2, rpwm); ledcWrite(3, 0);
        delay(50);
        break;
    case 2:
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, rpwm); ledcWrite(3, 0);
        delay(50);
        break;
    case 3:
        ledcWrite(0, lpwm); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(50);
        break;
    case 4:
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, rpwm); ledcWrite(3, 0);
        delay(250);
        ledcWrite(0, lpwm); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(250);
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(250);
        ledcWrite(0, lpwm); ledcWrite(1, 0); ledcWrite(2, rpwm); ledcWrite(3, 0);
        delay(250);
        break;
    case 5:
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(500);
        ledcWrite(0, lpwm); ledcWrite(1, 0); ledcWrite(2, rpwm); ledcWrite(3, 0);
        delay(500);
        break;
    case 6:
        ledcWrite(0, 0); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, 0);
        delay(500);
        break;
    case 7:
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, 0); ledcWrite(3, rpwm/(1.5));
        delay(500);
        break;
    case 8:
        ledcWrite(0, 0); ledcWrite(1, lpwm/(1.5)); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(500);
        break;
    }
}

void chatbotMotorTime1(float sec, int act, float lpwm, float rpwm)
{
    long interval = sec * 1000;
    previousTime = millis();
    unsigned long CurrentTime = millis();

    while(CurrentTime - previousTime < interval) {
      CurrentTime = millis();
      chatbotMove1(act, lpwm, rpwm);
    }

    ledcWrite(0, 0); 
    ledcWrite(1, 0); 
    ledcWrite(2, 0); 
    ledcWrite(3, 0);
}

void chatbotMove2(int num, float rate, float lpwm, float rpwm)
{
    switch (num)
    {
    case 0:
        ledcWrite(0, 0); ledcWrite(1, lpwm); ledcWrite(2, 0); ledcWrite(3, rpwm/(1 + (0.1 * rate)));
        delay(500);
        break;
    case 1:
        ledcWrite(0, 0); ledcWrite(1, lpwm/(1 + (0.1 * rate))); ledcWrite(2, 0); ledcWrite(3, rpwm);
        delay(500);
        break;
    }
}

void chatbotMotorTime2(float sec, int act, float rate, float lpwm, float rpwm)
{
    long interval = sec * 1000;
    previousTime = millis();
    unsigned long CurrentTime = millis();

    while(CurrentTime - previousTime < interval) {
      CurrentTime = millis();
      chatbotMove2(act, rate, lpwm, rpwm);
    }

    ledcWrite(0, 0); 
    ledcWrite(1, 0); 
    ledcWrite(2, 0); 
    ledcWrite(3, 0);
}