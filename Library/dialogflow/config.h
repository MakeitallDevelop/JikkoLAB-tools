
// 녹음 설정 
#define ONBOARD_LED 2  // 음성인식 시작시 ON 될 LED 핀번호 (HIGH면 켜짐), BUILT_IN_LED로 변경
#define MAX_WAV_SIZE 64000 // Byte

#define VOL 14  // 광역 볼륨레벨 조정
#define VOLUME_TH 21000   // 말하는중 이라고 판단할 최소 볼륨
#define LOW_VOL_CNT_TH 90 // 최소볼륨 이하일시 최대 대기회수, 최소볼륨 이하로 이 숫자이상 지나면 음성녹음종료 
#define MIN_SEND_RATIO 0.3 // 전치 보내는 시간(6초)중 최소한 음성인식 진행할 시간 비율 0.4 =40% = 6*0.4 = 2.4초

// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define USE_I2S_MIC_INPUT

// I2S Microphone Settings
/*
 * SCLK == Serial CLOCK
 * WS == WORD SELECT = LRCLK = LEFT_RIGHT_CLOCK
 * SD == SERIAL DATA == DATA_OUT OR DATA_IN
 */
// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
// #define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_26 
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_27
#define I2S_MIC_SERIAL_DATA GPIO_NUM_25 

// speaker settings
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_33
