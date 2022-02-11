#include "StefanMLibrery.h"

ButtonSignalFilter::ButtonSignalFilter() {}

ButtonSignalFilter::ButtonSignalFilter(int pin)
{
  ButtonSignalFilter::State(pin);
}

void ButtonSignalFilter::State(int pin)
{
  pinMode(pin, INPUT);
  _pin = pin;
}

bool ButtonSignalFilter::Loop()
{
  _buttonState = digitalRead(_pin);

  if (_buttonState != _lastState)
  {
    tIn = millis();
  }
  else
  {
    if ((millis() - tIn) > _debounceDelay)
    {
      if (_buttonState != _lastStatePin)
      {

        if (_buttonState == LOW)
        {
          trigUp = false;
          trigDown = true;
        }
        else
        {
          trigUp = true;
          trigDown = false;
        }
      }
      else
      {
        trigUp = false;
        trigDown = false;
      }
      state = _buttonState;
      _lastStatePin = _buttonState;
    }
  }
  _lastState = _buttonState;
  return state;
}

//=======================================================================================

Dimmer::Dimmer(int id, int bPin, int ledPin, int minVal, int maxVal, unsigned long dimTemp, unsigned long dimDelay)
{
  bfilter.State(bPin);
  pinMode(ledPin, OUTPUT);
  _ledPin = ledPin;
  _dimTemp = dimTemp;
  _dimDelay = dimDelay;
  _max = maxVal;
  _min = minVal;
  distance = 100 - (_min * 100 / _max); //90;
  onePCent = (float)_max / 100;
  _id = id;
  Start();
}

void Dimmer::Start()
{
  bfilter.Loop();
  if (bfilter.trigUp == true)
  {
    t1 = millis();
  }

  if (bfilter.state == true)
  {
    if (millis() - t1 > _dimDelay)
    {

      if (dim == false)
      {
        tDim = millis();
        dim = true;
      }

      float time = (float)_dimTemp / (float)onePCent;
      newCount = (float)(distance * (millis() - tDim) / time);

      if (newCount > 0)
      {
        tDim = millis();
        if (maxReach == true)
        {
          count = count - newCount;
          //Serial.println(count);
        }
        else
        {
          count = count + newCount;
          //Serial.println(count);
        }

        if (count <= _min)
        {
          count = _min;
          maxReach = false;
          //Serial.println(1);
        }
        if (count >= _max)
        {
          count = _max;
          maxReach = true;
          //Serial.println(0);
        }
      }
    }
  }

  if (bfilter.trigDown == true)
  {
    dim = false;
    if (millis() - t1 < _dimDelay)
    {
      if (count > 0)
      {
        lastLedState = count;
        count = 0;
      }
      else
      {
        count = lastLedState;
      }
    }
  }
  ledState = count;
  analogWrite(_ledPin, count);
  lastButtonState = bfilter.state;
  //while ((millis() - startLoop) < loopInterval) {}
}

void Dimmer::ReadSerial(String line)
{
  String com = line.substring(0, 3); // take the first 3 char

  if (com == "off")
  {
    count = 0;
  }
  if (com == "on")
  {
    count = _max;
  }
  if (com == "dim")
  {
    int c = line.substring(4, 7).toInt() * _max / 100;
    if (c > _max)
    {
      Serial.println("WARNING: max passed");
      count = _max;
    }
    else
    {
      count = c;
    }
  }
  if (com == "get")
  {
    int c = count / _max * 100;
    Serial.println("id " + String(_id) + " " + String(count/_max*100, 0));
  }
  // }
}

//================================================================

void Orchestrator::Start(Dimmer dimmers[])
{

  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n'); // take the incoming message til find the ENTER button
    int id = line.substring(0, 1).toInt();
    String com = line.substring(2, 9);
    for (int i = 0; i < sizeof(dimmers); i++)
    {
      if (dimmers[i]._id == id)
      {
        dimmers[i].ReadSerial(com);
      }
    }
  }

  for (int i = 0; i < sizeof(dimmers); i++)
  {
    dimmers[i].Start();
  }
}
