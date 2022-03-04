#include "Arduino.h"
#include "Dimmer.h"
Dimmer::Dimmer(int ledPin, int minVal, int maxVal, unsigned long dimTemp,  unsigned long dimDelay) {
      pinMode(ledPin, OUTPUT);
      _ledPin = ledPin;
      _dimTemp = dimTemp;
      _dimDelay = dimDelay;
      _max = maxVal;
      _min = minVal;
      distance = 100 - (_min * 100 / _max); //90;
      onePCent = (float)_max / 100;
    }


void Dimmer::Start(bool trigUp, bool trigDown, bool state) {
      //startLoop = millis();
      if (trigUp == true) {
        t1 = millis();
      }

      if (state == true) {
        if (millis() - t1 >  _dimDelay) {

          if (dim == false) {
            tDim = millis();
            dim = true;
          }

          float time = (float)_dimTemp / (float)onePCent;
          newCount = (float)(distance * (millis() - tDim) / time);

          if (newCount > 0) {
            tDim = millis();
            if (maxReach == true) {
              count = count - newCount;
              //Serial.println(count);
            } else {
              count = count + newCount;
              //Serial.println(count);
            }

            if (count <= _min) {
              count = _min;
              maxReach = false;
              Serial.println(1);
            }
            if (count >= _max) {
              count = _max;
              maxReach = true;
              Serial.println(0);
            }
          }

        }
      }

      if (trigDown == true ) {
        dim = false;
        if (millis() - t1 < _dimDelay) {
          if (count > 0) {
            lastLedState = count;
            count = 0;
          } else {
            count = lastLedState;
          }
        }
      }
      ledState = count;
      analogWrite(_ledPin, count);
      lastButtonState = state;
      //while ((millis() - startLoop) < loopInterval) {}

    }

void Dimmer::ReadSerial(String comand) {
      String com = comand.substring(0, 3);                            // take the first 3 char
      if (com == "off") {
        count = 0;                                                    // turn off
        Serial.println("count 0");
      }
      if (com == "on") {                                              // turn on
        count = _max;
      }
      if (com == "dim") {                                             // set dimmer value in percentual
        int c = comand.substring(4, 7).toInt() * _max / 100;
        if (c > _max) {
          Serial.println("WARNING: max passed");
          count = _max;
        } else {
          count = c;
        }
      }
    }
