#ifndef JIKKOCHATBOT_H
#define JIKKOCHATBOT_H

void playMelody(const int melody[], int melodySize, int tempo, int sec, int channel);
void playBuzzerMusic(int music, int sec, int channel);
void chatbotMotorTime1(float sec, int act, float lpwm, float rpwm);
void chatbotMove1(int num, float lpwm, float rpwm);
void chatbotMotorTime2(float sec, int act, float rate, float lpwm, float rpwm);
void chatbotMove2(int num, float rate, float lpwm, float rpwm);

#endif