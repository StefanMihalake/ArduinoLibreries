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
  lastLedState = 255;
  distance = 100 - (_min * 100 / _max); // 90;
  onePCent = (float)_max / 100;
  _id = id;
  analogWrite(_ledPin, 255);
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
        }
        else
        {
          count = count + newCount;
        }

        if (count <= _min)
        {
          count = _min;
          maxReach = false;
        }
        if (count >= _max)
        {
          count = _max;
          maxReach = true;
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
  // while ((millis() - startLoop) < loopInterval) {}
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
      // Serial.println("WARNING: max passed");
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

void Dimmer::SetMinMax(int min, int max)
{
  if (min != 0)
  {
    _min = min;
  }
  if (max != 0)
  {
    _max = max;
  }
}

void Dimmer::SetDim(int dim)
{
  if (dim <= 255 && dim > 0 && dim != count)
  {
    lastLedState = dim;
    if (count != 0)
    {
      count = dim;
    }
  }
}

void Dimmer::ReadModBus(int on, int off)
{
  if (on && !off)
  {
    if (count > 0)
    {
      lastLedState = count;
    }
    else
    {
      count = lastLedState;
    }
  }
  else if (!on && off)
  {
    if (count > 0)
    {
      lastLedState = count;
      count = 0;
    }
  }
}

//======================== SPLIT METHOD ===============================

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

//======================== ORCHESTRATOR COSTRUCTOR =====================

Orchestrator::Orchestrator(int idDispositivo, int comPort, int itemNumber, int startCoil, int startReg)
{
  _startCoil = startCoil;
  _startReg = startReg;
  if (!ModbusRTUServer.begin(idDispositivo, comPort))
  {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1)
      ;
  }
  ModbusRTUServer.configureCoils(startCoil, itemNumber * 2);
  ModbusRTUServer.configureHoldingRegisters(startReg, itemNumber * 3);
  // ModbusRTUServer.coilWrite(1, 1);
  // ModbusRTUServer.coilWrite(3, 1);
}

//========================= SERIAL MANAGER =============================

void Orchestrator::Start(Dimmer dimmers[])
{
  ModbusRTUServer.poll();

  // int holdindex = 0;
  // for (int i = 0; i < sizeof(dimmers); i++)
  // {
  //   dimmers[i].SetMinMax(ModbusRTUServer.holdingRegisterRead(_startReg + i * 3 /*holdindex*/), ModbusRTUServer.holdingRegisterRead((_startReg + 1) + i * 3 /*holdindex*/));
  //   // holdindex += 3;
  // }
  // holdindex = 0;

  for (int i = 0; i < sizeof(dimmers); i++)
  {
    int cv1 = ModbusRTUServer.coilRead(i * 2);
    int cv2 = ModbusRTUServer.coilRead(i * 2 + 1);
    dimmers[i].ReadModBus(cv1, cv2);
    if (cv1)
    {
      ModbusRTUServer.coilWrite(i * 2, 0);
    }
    if (cv2)
    {
      ModbusRTUServer.coilWrite(i * 2 + 1, 0);
    }
  }

  int dim = (int)ModbusRTUServer.holdingRegisterRead(2);
  dimmers[0].SetDim(dim);

  int dim2 = (int)ModbusRTUServer.holdingRegisterRead(5);
  dimmers[1].SetDim(dim2);

  // if (Serial.available())
  // {
  //   String line = Serial.readStringUntil('\n'); // take the incoming message til find the ENTER button
  //   SplitString(line, '*');                     // spit the message in id and command and put them on public variable
  //   if (first == "ids")                         // if the first word is id that mean that is a ids request
  //   {
  //     String idString = "ids*";
  //     for (int i = 0; i < sizeof(dimmers); i++)
  //     {
  //       idString += (String)dimmers[i]._id + "*";
  //     }
  //     idString = idString.substring(0, idString.length() - 1);
  //     Serial.println(idString);
  //   }
  //   else
  //   {
  //     int id = first.toInt();
  //     for (int i = 0; i < sizeof(dimmers); i++)
  //     {
  //       if (dimmers[i]._id == id)
  //       {
  //         dimmers[i].ReadSerial(second, third);
  //       }
  //     }
  //   }
  // }

  for (int i = 0; i < sizeof(dimmers); i++)
  {
    dimmers[i].Start();
  }
}
