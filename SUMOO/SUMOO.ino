//#include "funkcje.h"
#include <SoftWire.h>
#include <vl53l0x_class.h>

//SILNIKI
#define L1 PB8
#define L2 PB9
#define R1 PB7
#define R2 PB6
#define L 0
#define R 1
int prevmili=millis();
//RGB
#define R PA3
#define G PA2
#define B PA1

//VL53L0X
#define SCL PB10
#define SDA PB11
#define XSHUTL PB12
#define XSHUTM PB14
#define XSHUTR PA8
#define GPIOL PB13
#define GPIOM PB15
#define GPIOR PA9

//IR
#define IRL PB0
#define IRR PB1

//BATERIA
#define BAT PA7

//START
#define START PB4

//PROGI
#define batThreshold 2137
#define irThreshold 1500
#define vlThreshold 600
#define halfPeriod 500

//FLAGI
bool rBlinking = false;
int rState = LOW;
int STATE = 0;
bool DIR = 1;
bool TIMEOUT = 0;
int ind=0;
int lLine;
int rLine;
int v= 50;
int timing=0;
bool strona=true;
//CZASY
uint32_t rPrevMillis = 0;
uint32_t prevMillis = 0;
int timeLeft = 0;
#define TURN 500;
#define SCAN 1000;
#define RETREAT 300;

SoftWire WIRE1(PB10, PB11, SOFT_FAST);
VL53L0X sensorL(&WIRE1, XSHUTL, GPIOL); //WIRE, XSHUT, INT
VL53L0X sensorM(&WIRE1, XSHUTM, GPIOM);
VL53L0X sensorR(&WIRE1, XSHUTR, GPIOR);

uint32_t distanceL;
uint32_t distanceM;
uint32_t distanceR;
bool kontra=false;
void setup()
{
  //PWM
  
  pinMode(L1, PWM);
  pinMode(L2, PWM);
  pinMode(R1, PWM);
  pinMode(R2, PWM);
  //DIGITAL
  pinMode(PC13, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  //ANALOG

  //VL53L0X
  int status;
  Serial.begin(9600);
  WIRE1.begin();
  
  sensorL.VL53L0X_Off();
  sensorM.VL53L0X_Off();
  sensorR.VL53L0X_Off();

  status = sensorL.InitSensor(0x10);
  //sensorL.VL53L0X_Off();
  
  while(status)
  {
     digitalWrite(R,HIGH);
    digitalWrite(G,HIGH);
    digitalWrite(B,HIGH);
     delay(500);
     digitalWrite(R,LOW);
    delay(500);   
    Serial.println("L Init sensor_vl53l0x failed...");
  }
delay(150);
  status = sensorR.InitSensor(0x18);
  //sensorR.VL53L0X_Off();  
  
    while(status)
  {
    digitalWrite(R,HIGH);
    digitalWrite(G,HIGH);
    digitalWrite(B,HIGH);
    delay(500);
     digitalWrite(R,LOW);
    delay(500);  
    Serial.println("R Init sensor_vl53l0x failed...");
  }
  delay(150);
 status = sensorM.InitSensor(0x26);
  
    while(status)
  {
    digitalWrite(R,HIGH);
    digitalWrite(G,HIGH);
    digitalWrite(B,HIGH);
   delay(500);
     digitalWrite(R,LOW);
    delay(500);   
    Serial.println("M Init sensor_vl53l0x failed...");
  }

  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
  

  DIR = analogRead(IRL) % 2;
  digitalWrite(R, HIGH);
  digitalWrite(G, HIGH);
  digitalWrite(B, HIGH);
  while(millis()<5000){}
  power(L,0,70);
power(R,0,70);
delay(100);

}


/////////////////////////////////////////////////////////////////////////////////////////LOOP

void loop()
{
  int rng=350;
sensorL.GetDistance(&distanceL);
sensorM.GetDistance(&distanceM);
sensorR.GetDistance(&distanceR);
//lLine = analogRead(IRL);
//rLine = analogRead(IRL);
if(distanceL==0){
  distanceL=2600;
}
if(distanceM==0){
  distanceM=2600;
}
if(distanceR==0){
  distanceR=2600;
}


if(distanceM>rng&&distanceL>rng&&distanceR>rng){
int xd=millis();
  if(xd-timing>2000){
  kontra=true;
  timing=xd;
  }
  if(strona){
  power(L,0,100);
  power(R,1,100);
  }
  else{
  power(R,0,100);
  power(L,1,100);    
    }
  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
}

else if(distanceM<=rng){
  if(millis()-prevmili>3000){
      power(R,0,100);
    power(L,0,100);
    delay(400);
    prevmili=millis();
    }
    else{
  if(kontra){
  kontra=false;
  power(L,1,100);
  power(R,0,100);
  delay(200);
  }
  power(L,1,100);
  power(R,1,100);
  digitalWrite(R, LOW);
  digitalWrite(G, HIGH);
  digitalWrite(B, HIGH);
timing=millis();
    
    }
}
else if(distanceL<=rng){
  strona=true;
    if(kontra){
  kontra=false;
  power(L,1,100);
  power(R,0,100);
  delay(200);
    }
  power(L,1,40);
  power(R,1,100);
  digitalWrite(R, HIGH);
  digitalWrite(G, LOW);
  digitalWrite(B, HIGH);
  timing=millis();
}
else if(distanceR<=rng){
  strona=false;
    if(kontra){
  kontra=false;
  power(L,1,100);
  power(R,0,100);
  delay(200);
    }
  power(R,1,40);
  power(L,1,100);
  digitalWrite(R, HIGH);
  digitalWrite(G, HIGH);
  digitalWrite(B, LOW);
  timing=millis();
}
else{
  power(L,0,v);
  power(R,0,65);  
  digitalWrite(R, HIGH);
  digitalWrite(G, LOW);
  digitalWrite(B, HIGH);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////LOOP


void decide(bool IsWhiteL, bool IsWhiteR, bool InRangeL, bool InRangeM, bool InRangeR)
{
  switch(STATE)
  {
    case 1:
    digitalWrite(R, HIGH);
    digitalWrite(G, HIGH);
    digitalWrite(B, LOW);
      if(timeLeft > 0)
      {
        timeLeft = timeLeft - (millis() - prevMillis);
        if(InRangeL || InRangeM || InRangeR)
        {
          power(L, 1, 100);
          power(R, 1, 100);
          STATE = 3;
        }
        else
        {
          power(L, DIR, 100);
          power(R, !DIR, 100);
        }
      }
      else
      {
        timeLeft = 0;
        STATE = 2;
      }
      
      break;
    case 2:
    digitalWrite(R, HIGH);
    digitalWrite(G, LOW);
    digitalWrite(B, HIGH);
      if(InRangeL || InRangeM || InRangeR)
      {
        power(L, 1, 100);
        power(R, 1, 100);
        STATE = 3;
      }
      else
      {
        power(L, 1, 70);
        power(R, 1, 70);
      }
      break;
    case 3:
    digitalWrite(R, LOW);
    digitalWrite(G, LOW);
    digitalWrite(B, LOW);
      if(IsWhiteL)
      {
        power(L, 0, 100);
        power(R, 0, 100);
        DIR = R;
        STATE = 4;
        timeLeft = RETREAT;
      }
      if(IsWhiteL)
      {
        power(L, 0, 100);
        power(R, 0, 100);
        DIR = L;
        STATE = 4;
        timeLeft = RETREAT;
      }
      if(InRangeL && !InRangeM && !InRangeR)
      {
        power(L, 1, 0); 
        power(R, 1, 100);
        DIR = L;
      }
      else if(InRangeL && InRangeM && !InRangeR)
      {
        power(L, 1, 50); 
        power(R, 1, 100);
        DIR = L;
      }
      else if(!InRangeL && InRangeM && InRangeR)
      {
        power(L, 1, 100); 
        power(R, 1, 50);
        DIR = R;
      }
      else if(!InRangeL && !InRangeM && InRangeR)
      {
        power(L, 1, 100); 
        power(R, 1, 0);
        DIR = R;
      }
      else if(!InRangeL && !InRangeM && !InRangeR)
      {
        STATE = 1;
        power(L, 1, 0);
        power(R, 1, 0);
      }
      else
      {
        power(L, 1, 100);
        power(R, 1, 100);
      }
      break;
    case 4:
    digitalWrite(R, LOW);
    digitalWrite(G, HIGH);
    digitalWrite(B, HIGH);
      if(timeLeft > 0)
      {
        timeLeft = timeLeft - (millis() - prevMillis);
      }
      else
      {
        timeLeft = TURN;
        STATE = 5;
      }
      break;
    case 5:
    digitalWrite(R, LOW);
    digitalWrite(G, HIGH);
    digitalWrite(B, LOW);
      if(timeLeft > 0)
      {
        timeLeft = timeLeft - (millis() - prevMillis);
        power(L, DIR, 100);
        power(R, !DIR, 100);
      }
      else
      {
        timeLeft = SCAN;
        STATE = 1;
      }
      break;
    default:
    
      if(IsWhiteL)
      {
        digitalWrite(R, LOW);
        digitalWrite(G, HIGH);
        digitalWrite(B, HIGH);
        power(L, 1, 50);
        power(R, 0, 50);
      }
      else if(IsWhiteR)
      {
        digitalWrite(R, HIGH);
        digitalWrite(G, HIGH);
        digitalWrite(B, LOW);
        power(L, 0, 50);
        power(R, 1, 50);
      }
      else
      {
        digitalWrite(R, HIGH);
        digitalWrite(G, HIGH);
        digitalWrite(B, HIGH);
        power(L, 1, 50);
        power(R, 1, 50);
      }
      break;
  }
  /*if(InRangeL) 
  {
    digitalWrite(R, LOW);
    
  }
  else
  {
    digitalWrite(R, HIGH);
    
  }

  if(InRangeM) 
  {
    digitalWrite(G, LOW);
  }
  else 
  {
    digitalWrite(G, HIGH);
  }

  if(InRangeR) 
  {
    digitalWrite(B, LOW);
    
  }
  else 
  {
    digitalWrite(B, HIGH);
    
  }

  if(IsWhiteL || IsWhiteR) digitalWrite(PC13, HIGH);
  else digitalWrite(PC13, LOW);*/
}

//-------------------------------------------------------IN

bool inRangeL()
{
  //digitalWrite(B, LOW);
  sensorL.GetDistance(&distanceL);
  //digitalWrite(B, HIGH);
  if(vlThreshold > distanceL && distanceL != 0)
    return 1;
  else
    return 0;
}

bool inRangeM()
{
  //digitalWrite(G, LOW);
  sensorM.GetDistance(&distanceM);
  //digitalWrite(G, HIGH);
  if(vlThreshold > distanceM && distanceM != 0)
    return 1;
  else
    return 0;
}

bool inRangeR()
{
  //digitalWrite(R, LOW);
  sensorR.GetDistance(&distanceR);
  //digitalWrite(R, HIGH);
  if(vlThreshold > distanceR && distanceR != 0)
    return 1;
  else
    return 0;
}

bool isWhiteL()
{
  uint16_t valueL;
  valueL = analogRead(IRL);
  if(valueL < irThreshold) return 1;
  else return 0;
}

bool isWhiteR()
{
  uint16_t valueR;
  valueR = analogRead(IRR);
  if(valueR < irThreshold) return 1;
  else return 0;
}

void checkBattery()
{
  uint16_t batLevel;
  batLevel = analogRead(BAT);
  if(batLevel < batThreshold)
  {
    //redBlink();
  }
}

//----------------------------------------------------------OUT

void redBlink()
{
  if(rBlinking)
  {
    uint32_t currentMillis = millis();
    if (currentMillis - rPrevMillis >= halfPeriod) 
    {
      rPrevMillis = currentMillis;
      if (rState == LOW) 
      {
        rState = HIGH;
      } 
      else 
      {
        rState = LOW;
      }
      digitalWrite(R, rState);
    }
  }
}

void power(int side, bool direction, uint16_t power)  //Lewy, Prawy, 0-tył 1-przód, power 0 - 65535
{
  if(side == 0)
  {
    pwmWrite(L1, power*direction*65535/100);
    pwmWrite(L2, power*(!direction)*65535/100);
  }
  else
  {
    pwmWrite(R1, power*direction*65535/100);
    pwmWrite(R2, power*(!direction)*65535/100);
  }
}

/*void rgbOn(char color, int period)
{
  if(color == 'R')
  {
    isRedActive = true;
    periodR = period;
  }
  if(color == 'G')
  {
    isGreenActive = true;
    periodG = period;
  }
  if(color =='B')
  {
    isBlueActive = true;
    periodB = period;
  }
}

void rgbOn(char color)
{
  
}

void rgbOff(char color)
{
  
}*/
