#include "Arduino.h"
#include "PassoPasso.h"

PassoPasso::PassoPasso(int buttonPin, int ledPin){
    PassoPasso::State(buttonPin, ledPin);
}

void PassoPasso::State(int buttonPin, int ledPin){
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);
    _buttonPin = buttonPin;
    _ledPin = ledPin;
}

void PassoPasso::Start(){
    buttonPress = digitalRead(_buttonPin);

    if (buttonPress != lastButtonState) {
    lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (buttonPress != buttonState) {
            buttonState = buttonPress;
            if (buttonState == HIGH) {
                ledState = !ledState;
            }
        }
    } 
   digitalWrite(_ledPin, ledState);
   lastButtonState = buttonPress;
}