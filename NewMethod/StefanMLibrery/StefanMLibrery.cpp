#include "StefanMLibrery.h"

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
  analogWrite(_ledPin, _max);
  Start();
}

void Dimmer::Start()
{
  bfilter.Loop();             // start control loop for the button
  if (bfilter.trigUp == true) // if press the button
  {
    t1 = millis(); // take the time from the trig up
  }

  if (bfilter.state == true) // if the button is press
  {
    if (millis() - t1 > _dimDelay) // if pass 250ms from the last time and the button is still press start dim
    {

      if (dim == false)
      {
        tDim = millis();
        dim = true;
      }
      onePCent = (float)_max / 100;
      distance = 100 - (_min * 100 / _max);                    // distance to make from min to max
      float time = (float)_dimTemp / (float)onePCent;          // 1% of distance
      newCount = (float)(distance * (millis() - tDim) / time); // how to increment to arrive to max in set time

      if (newCount > 0)
      {
        tDim = millis(); // reset the time for the press button
        if (maxReach == true)
        {
          count = count - newCount; // apply the calculated count to cover the distance in time
        }
        else
        {
          count = count + newCount; // apply the calculated count to cover the distance in time
        }

        if (count <= _min) // if reach the min possible value set to min
        {
          count = _min;
          maxReach = false;
        }
        if (count >= _max) // is reach the max possible value the to max
        {
          count = _max;
          maxReach = true;
        }
      }
    }
  }

  if (bfilter.trigDown == true) // when the button is release
  {
    dim = false;
    if (millis() - t1 < _dimDelay) // is pass < than 250ms switch the led state
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
  analogWrite(_ledPin, count);     // apply the value brightness
  lastButtonState = bfilter.state; // save the last button state
  // while ((millis() - startLoop) < loopInterval) {}
}

//====================== SET MIN AND MAX TO LED FROM MODBUSS METHOD =============== 

void Dimmer::SetMinMax(int min, int max)
{
  int lastMax;
  int lastMin;

  if (min != _min && min != lastMin)
  {
    _min = min;
    lastMin = min;
  }
  if (max != _max && max != lastMax)
  {
    _max = max;
    lastMax = max;
  }

}


//====================== SET DIM FROM MODBUSS METHOD =============== 

int Dimmer::SetDim(int modDim)
{
  if (modDim <= _max && modDim > 0 && dim == false)
  {
    if (modDim != lastLedState)
    {
      count = modDim;
      lastLedState = modDim;
    }

    if (count != 0)
    {
      count = modDim;
      lastLedState = modDim;
    }
  }
  if (count != modDim)
  {
    return count;
  }
  else
  {
    return 0;
  }
}


//====================== TURN ON/OFF FROM MODBUSS METHOD =============== 
bool Dimmer::SetOnOff(int on, int off)
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
      return true;
  }
  if (!on && off)
  {
    if (count > 0)
    {
      lastLedState = count;
      count = 0;
    }
    return false;
  }
}


//======================== ORCHESTRATOR COSTRUCTOR =====================

Orchestrator::Orchestrator(int idDispositivo,int numeroDevice, int comPort, int itemNumber, int startCoil, int startHReg, int startIReg, int startDIReg, int startIDIReg)
{
  _deviceId = idDispositivo;
  _startCoil = startCoil;
  _startHReg = startHReg;
  _startIReg = startIReg;
  _startDIReg = startDIReg;
  _startIDIReg = startIDIReg;
  if (!ModbusRTUServer.begin(_deviceId, comPort))
  {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1)
      ;
  }
  ModbusRTUServer.configureCoils(startCoil, itemNumber * 2);
  ModbusRTUServer.configureHoldingRegisters(startHReg, itemNumber * 3);
  ModbusRTUServer.configureInputRegisters(startIReg, itemNumber * 3);
  ModbusRTUServer.configureDiscreteInputs(startDIReg, itemNumber);
  ModbusRTUServer.configureInputRegisters(_startIDIReg, itemNumber);
}

//========================= SERIAL MANAGER =============================

void Orchestrator::Start(Dimmer dimmers[])
{
  for (int i = 0; i < sizeof(dimmers); i++)
  {
    ModbusRTUServer.inputRegisterWrite(_startIDIReg + i, dimmers[i]._id);
  }
  
  ModbusRTUServer.poll();

  // SET MIN OR MAX
  int holdingIndex2 = _startHReg;
  int inputRegIDstart = _startIDIReg;
  for (int i = 0; i < sizeof(dimmers); i++)
  {
    int m = ModbusRTUServer.holdingRegisterRead(holdingIndex2);
    int M = ModbusRTUServer.holdingRegisterRead(holdingIndex2 + 1);
    dimmers[i].SetMinMax(m, M);
    ModbusRTUServer.inputRegisterWrite(inputIndex1, setDim);
    ModbusRTUServer.inputRegisterWrite(inputIndex1, setDim);
    holdingIndex2 += 3;
  }



  // PASSO PASSO DA MODBUS
  int coilIndex = _startCoil;
  int discretIndex = _startDIReg;
  for (int i = 0; i < sizeof(dimmers); i++)
  {
    int coilOn = ModbusRTUServer.coilRead(coilIndex);                     // read ON value from coil
    int coilOff = ModbusRTUServer.coilRead(coilIndex + 1);                // read OFF value from coil
    bool setIStatus = dimmers[i].SetOnOff(coilOn, coilOff);               // send the value to the Led Function and take if is on or off

    ModbusRTUServer.discreteInputWrite(discretIndex, setIStatus);

    if (coilOn)
    {
      ModbusRTUServer.coilWrite(coilIndex, 0);                            // set false the ON register for the next cicle
    }
    if (coilOff)
    {
      ModbusRTUServer.coilWrite(coilIndex + 1, 0);                        // set false the OFF register for the next cicle
    }
    coilIndex += 2;
    discretIndex += 1;
  }



  // DIM DA MODBUS
  int holdingIndex1 = _startHReg;
  int inputIndex1 = _startIReg;
  int discretIndex1 = _startDIReg;
  for (int i = 0; i < sizeof(dimmers); i++)
  {
    int mdim = (int)ModbusRTUServer.holdingRegisterRead(holdingIndex1+2);          // read the dim register
    int setDim = dimmers[i].SetDim(mdim);                                          // set the dim
    ModbusRTUServer.holdingRegisterWrite(holdingIndex1+2, 0);                      // set 0 the dim register

    ModbusRTUServer.inputRegisterWrite(inputIndex1, setDim);                   // update the user interface with brightness value
    if (setDim != 0)
    {
      ModbusRTUServer.discreteInputWrite(discretIndex1, true);                   // update the user interface led status
    }
    holdingIndex1 += 3;
    inputIndex1 += 1;
    discretIndex1 += 1;
  }
  holdingIndex1 = 0;
  inputIndex1 = 0;
  discretIndex1 = 0;




  // BUTTON LOOP START
  for (int i = 0; i < sizeof(dimmers); i++)
  {
    dimmers[i].Start();
  }
}
