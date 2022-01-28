#include "Arduino.h" 
#include "Jikko.h"

float prevVal;

int getMultiBTData1(String str, int number) {
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