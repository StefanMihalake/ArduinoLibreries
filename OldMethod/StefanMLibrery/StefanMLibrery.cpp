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
          Serial.print("set*");
          Serial.print(String(_id) + "*");
          Serial.println(String(count / _max * 100, 0));
        }
        else
        {
          count = count + newCount;
          Serial.print("set*");
          Serial.print(String(_id) + "*");
          Serial.println(String(count / _max * 100, 0));
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
        Serial.print("set*");
        Serial.print(String(_id) + "*");
        Serial.println(String(count / _max * 100, 0));
      }
      else
      {
        count = lastLedState;
        Serial.print("set*");
        Serial.print(String(_id) + "*");
        Serial.println(String(count / _max * 100, 0));
      }
    }
  }
  ledState = count;
  analogWrite(_ledPin, count);
  lastButtonState = bfilter.state;
  //while ((millis() - startLoop) < loopInterval) {}
}

void Dimmer::ReadSerial(String com, String dim)
{
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
    int c = dim.toInt() * _max / 100;
    if (c > _max)
    {
      //Serial.println("WARNING: max passed");
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
    Serial.println("get*" + String(_id) + "*" + String(count / _max * 100, 0));
  }
  // }
}

//================================================================

void Orchestrator::Start(Dimmer dimmers[])
{
  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n'); // take the incoming message til find the ENTER button
    SplitString(line, '*');
    if (first == "ids")
    {
      String idString = "ids*";
      for (int i = 0; i < sizeof(dimmers); i++)
      {
        idString += (String)dimmers[i]._id + "*";
      }
      idString = idString.substring(0, idString.length() - 1);
      Serial.println(idString);
    }
    else
    {
      int id = first.toInt();
      for (int i = 0; i < sizeof(dimmers); i++)
      {
        if (dimmers[i]._id == id)
        {
          dimmers[i].ReadSerial(second, third);
        }
      }
    }
  }

  for (int i = 0; i < sizeof(dimmers); i++)
  {
    dimmers[i].Start();
  }
}

void Orchestrator::SplitString(String str, char sep)
{
  int index = str.indexOf(sep);
  if (index >= 0)
  {
    first = str.substring(0, index);
    int index2 = str.indexOf(sep, index + 1);
    if (index2 != -1)
    {
      second = str.substring(index + 1, index2);
      third = str.substring(index2 + 1);
    }
    else
    {
      second = str.substring(index + 1);
      third = "";
    }
  }
  else
  {
    first = str;
    second = "";
    third = "";
  }
}