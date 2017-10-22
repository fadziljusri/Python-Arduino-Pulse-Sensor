#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>



#include <Wire.h>
#include <Event.h>
#include <Timer.h>

LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
Timer t;

#define pulsePin A0

//  VARIABLES
int rate[10];                    
unsigned long sampleCounter = 0; 
unsigned long lastBeatTime = 0;  
unsigned long lastTime = 0, N;
unsigned int BPM = 0;                        //  holds the heart rate value, derived every beat, from averaging previous 10 IBI values
int IBI = 0;                        //  holds the time interval between beats
int P = 512;
int T = 512;
int thresh = 512;  
int amp = 100;                   
unsigned int Signal;                         //  holds the analog signal data straight from the sensor
boolean Pulse = false;
boolean firstBeat = true;          
boolean secondBeat = true;
boolean QS = false;                 //  check Pulse is found and BPM is updated
int i = 0;

void setup() {
  Serial.begin(9600);
//  t.every(2000, pulseDetected);
  lcd.begin(16,2); // sixteen characters across - 2 lines
  lcd.backlight();
//
//  
//  for(int i=5; i>0; i--){
//    lcd.clear();
//    lcd.setCursor(0,0);
//    lcd.print("Start in ");
//    lcd.print(String(i) +"s");
//    delay(1000);
//  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("S.D.F.A.F");
  lcd.setCursor(0,1);
  lcd.print(String(BPM) + " bpm");
}

void loop() {
  t.update();
//  i++;
  if (QS == true) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("S.D.F.A.F.");
    lcd.setCursor(0,1);
    lcd.print(String(BPM) + " bpm");
//    Serial.println("BPM: "+ String(Signal));
    Serial.print(Signal);
    Serial.print(",");
    Serial.println(BPM);
    QS = false;
  } else if (millis() >= (lastTime + 2)) {
    Serial.print(Signal);
    Serial.print(",");
    Serial.println(BPM);
    readPulse(); 
    lastTime = millis();
  }
//  delay(1000);
}


void readPulse() {

  Signal = analogRead(pulsePin);              
  sampleCounter += 2;                           
  int N = sampleCounter - lastBeatTime;   

  detectSetHighLow();

  if (N > 250) {  
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3) )
      pulseDetected();
  }

  if (Signal < thresh && Pulse == true) {  
    Pulse = false;
    amp = P - T;
    thresh = amp / 2 + T;  
    P = thresh;
    T = thresh;
  }

  if (N > 2500) {
    thresh = 512;
    P = 512;
    T = 512;
    lastBeatTime = sampleCounter;
    firstBeat = true;            
    secondBeat = true;           
  }

}

void detectSetHighLow() {

  if (Signal < thresh && N > (IBI / 5) * 3) {
    if (Signal < T) {                       
      T = Signal;                         
    }
  }

  if (Signal > thresh && Signal > P) {    
    P = Signal;                           
  }                                       

}

void pulseDetected() {
  Pulse = true;                           
  IBI = sampleCounter - lastBeatTime;     
  lastBeatTime = sampleCounter;           

  if (firstBeat) {                       
    firstBeat = false;                 
    return;                            
  }
  if (secondBeat) {                    
    secondBeat = false;                
    for (int i = 0; i <= 9; i++) {   
      rate[i] = IBI;
    }
  }

  word runningTotal = 0;                   

  for (int i = 0; i <= 8; i++) {          
    rate[i] = rate[i + 1];            
    runningTotal += rate[i];          
  }

  rate[9] = IBI;                      
  runningTotal += rate[9];            
  runningTotal /= 10;                 
  BPM = 60000 / runningTotal;         
  QS = true;                              
}

