//#define DEBUG


const int LED_ROT = 3;
const int LED_GRUEN = 4;
const int RELAIS = 2;
const int SWITCH_AUTO = 5;
const int SWITCH_ON = 6;

const int minVoltage = 20500; /* 22V läuft stabil */
const int minStartVoltage = 23000;  /* 24V läuft stabil */
const int VoltageOverloadDiff = 1500;
const double calibration = 1.2;

const int restartDelayMin = 30;

const int systemVoltage = 5000;
const int R1 = 100;
const int R2 = 330;
typedef enum 
{
  E_OFF,
  E_ON,
  E_AUTO
}mode_e;

typedef enum
{
  E_ERROR_OVERLOAD,
  E_ERROR_UNDERVOLTAGE
}errorcode_e;

mode_e lastMode = E_OFF;

double voltage;
int lastVoltage;

void CheckBatteryVoltage();


void setup() 
{
  pinMode(LED_ROT, OUTPUT);
  pinMode(LED_GRUEN, OUTPUT);
  pinMode(RELAIS, OUTPUT);
  pinMode(SWITCH_AUTO, INPUT);
  pinMode(SWITCH_ON, INPUT);
  
  digitalWrite(LED_ROT, 0);
  digitalWrite(LED_GRUEN, 0);
  digitalWrite(RELAIS, 0);
#ifdef DEBUG
  Serial.begin(9600);
#endif
}

void loop() 
{
  voltage = analogRead(0);
  voltage = (voltage / 1023)* systemVoltage * 10;
  voltage = voltage * calibration;
  
  if(HIGH == digitalRead(SWITCH_ON))
  {
    lastMode = E_ON;
    digitalWrite(LED_ROT, 0);
    digitalWrite(LED_GRUEN, 1);
    digitalWrite(RELAIS, 1);
#ifdef DEBUG
    Serial.println("Mode ON");
#endif
    delay(1000);
    return;
  }

  if(LOW == digitalRead(SWITCH_AUTO))
  {
    lastMode = E_OFF;
    digitalWrite(LED_ROT, 0);
    digitalWrite(LED_GRUEN, 0);
    digitalWrite(RELAIS, 0);
#ifdef DEBUG
    Serial.println("Mode OFF");
#endif
    delay(1000);
    return;
  }
#ifdef DEBUG
  Serial.print(voltage); 
  Serial.println(" mV");
  Serial.println("Mode AUTO");
#endif
  CheckOverload();
  CheckBatteryVoltage();
  lastMode = E_AUTO;
  //delay(100);
}

void CheckBatteryVoltage()
{
   if (5000 >= voltage)
  {
    digitalWrite(LED_ROT, 1);
    delay(100);
    digitalWrite(LED_ROT, 0);
    delay(100);
    return;
  }
  if (  (minStartVoltage <= voltage)
      ||((minVoltage <= voltage)&&(E_AUTO != lastMode)))
  {
    StartSolar();
  }
  if (minVoltage > voltage)
  {
    StopSolar(E_ERROR_UNDERVOLTAGE);
  }
}

void CheckOverload()
{
  if((lastVoltage-VoltageOverloadDiff) >= voltage)
  {
    StopSolar(E_ERROR_OVERLOAD);
  }
  lastVoltage = voltage;
}

void StartSolar()
{
  digitalWrite(LED_ROT, 0);
  digitalWrite(LED_GRUEN, 1);
  digitalWrite(RELAIS, 1);
}

void StopSolar(errorcode_e error)
{
  digitalWrite(LED_ROT, 1);
  digitalWrite(LED_GRUEN, 0);
  digitalWrite(RELAIS, 0);
  DelayMin(30, error);
}

void DelayMin (int Min, errorcode_e error )
{
  int ledCount = 0;
  for (int i=0; i<(600*Min); i++)
  {
    delay(100);
    if (  (0 == digitalRead(SWITCH_AUTO))
        ||(0 != digitalRead(SWITCH_ON)) )
     {
      return;
     }
     ledCount++;
     switch (error)
     {
      case E_ERROR_OVERLOAD:
        if(3 == ledCount)
        {
          digitalWrite(LED_ROT, 1);
        }
        if (4 == ledCount)
        {
          digitalWrite(LED_ROT, 0);
          ledCount = 0;
        }
      case E_ERROR_UNDERVOLTAGE:
        if(5 == ledCount)
        {
          digitalWrite(LED_ROT, 1);
          digitalWrite(LED_GRUEN, 0); 
        }
        if (10 == ledCount)
        {
          digitalWrite(LED_ROT, 0);
          digitalWrite(LED_GRUEN, 1);
          ledCount = 0;
        }
        
     }
  }
}

