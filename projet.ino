#include <LiquidCrystal.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
const int rs = 4, en = 5, d4 = 6, d5 = 7, d6 = 8, d7 = 9, pinBat = 14, btnBit0 = 16, btnBit1 = 15, btnEnable = 17;
LiquidCrystal lcd(4,5,6,7,8,9);
SoftwareSerial ss (3, 2);
TinyGPSPlus gps;
File myFile;
float tension;
float heure;
int second1 = 0, second2 = 0, difference = 0;
int time_current = 0, time_last = 0;
const int bit0 = 16;
const int bit1 = 15;
const int active = 17;
int state0 = 0;
int state1 = 0;
int statea = 0;
boolean enableM = false; //measure
boolean enableShow = false; //show
boolean enableT = false; //tension
boolean start = false;
double latitude = 0, longitude = 0;
double distance = 0;

void setup(){
  Serial.begin(4800);
  lcd.begin(8,2);
  ss.begin(4800);
  pinMode(bit0, INPUT);
  pinMode(bit1,INPUT);
  pinMode(active,INPUT);
  if (SD.begin()){
    Serial.println("SD card is ready to use.");
  }else{
    Serial.println("SD card initialization failed");
    return;
  }
  
  affichageTension();
}

void loop(){
  state0 = digitalRead(bit0);
  state1 = digitalRead(bit1);
  statea = digitalRead(active);
  if ((state0 == 0)&&(state1 == 0)&&(statea == 1)&&(enableM == false)){
    //if (SD.exists("test.csv")) SD.remove("test.csv");
    //second1 = gps.time.second() + gps.time.minute()*60 + gps.time.hour()*3600;
    enableM = true;
    enableShow = true;
    enableT = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("start!");
    start = false;
  }else if ((state0 == 1)&&(state1 == 0)&&(statea == 1)&&(enableM == true)){
    enableM = false;
    enableShow = false;
    enableT = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("stop!");
    second2 = gps.time.second() + gps.time.minute()*60 + gps.time.hour()*3600;
    if (second2 > second1) difference = second2 - second1;
    else difference = second2 + 24*3600 - second1;
    myFile = SD.open("test.csv",FILE_WRITE);
    if (myFile){
      myFile.print(difference);
      myFile.print("\n");
      myFile.print(distance);
      myFile.print("\n");
      distance = 0;
    }
    myFile.close();
    lcd.setCursor(0,1);
    lcd.print(difference/3600);
    lcd.print("h");
    difference = difference - 3600*(difference/3600);
    lcd.print (difference/60);
    lcd.print("m");
    difference = difference - 60*(difference/60);
    lcd.print(difference);
    lcd.print("s");
  }else if ((state0 == 0)&&(state1 == 1)&&(statea == 1)&&(enableM == true)&&(enableShow == false)){
    enableShow = true;
    enableT = false;
    lcd.clear();
  }else if ((state0 == 1)&&(state1 == 1)&&(statea == 1)){
    enableShow = false;
    enableT = true;
  }
  if (enableM){
    measure(enableShow);
  }
  if (enableT){
    affichageTension();
  }
  lcd.setCursor(0,0);
  lcd.print(enableM); lcd.print(enableT); lcd.print(enableShow);
  delay(500);
}

void affichageTension() {
  //calcul la tension
  tension = (float(analogRead(pinBat))/1023.0)*6.5;
  //si tension <= 4, GPS ne peut pas fonctionner
  lcd.clear();
  if(tension<=4){
    lcd.setCursor(0,0);
    lcd.print("faible");
    lcd.setCursor(0,1);
    lcd.print("tension");
  }else{//calcul 
    heure = 20 - (6.4-tension)/0.12;
    lcd.setCursor(0,0);
    lcd.print("can use:");
    lcd.setCursor(0,1);
    lcd.print(heure);
    lcd.print("h");
  }
}

void measure(int show) {
  myFile = SD.open("test.csv",FILE_WRITE);
  while (ss.available() > 0){
    if (gps.encode(ss.read())){
      if (gps.location.isValid()){
        if (gps.location.lat() != latitude || gps.location.lng() != longitude){
          Serial.print(gps.location.lat(),6);
          Serial.print(F(","));
          Serial.print(gps.location.lng(),6);
          Serial.print("\n");
          if (show){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(gps.location.lat(),6);
            lcd.setCursor(1,1);
            lcd.print(gps.location.lng(),6);
          }
          time_last = time_current;
          time_current = gps.time.centisecond() + gps.time.second()*100;
          if (start){
            if (time_current > time_last){
              distance = distance + (time_current - time_last) * gps.speed.mps();
            }
            else if (time_current < time_last) {
              distance = distance + (6000 + time_current - time_last) * gps.speed.mps();
            }
          }
          if (myFile){
            if (!start){
              second1 = gps.time.second() + gps.time.minute()*60 + gps.time.hour()*3600;
              myFile.print(gps.date.year());
              myFile.print(F("/"));
              myFile.print(gps.date.month());
              myFile.print(F("/"));
              myFile.print(gps.date.day());
              myFile.print(F(" "));
              if (gps.time.hour() < 10) myFile.print(F("0"));
              myFile.print(gps.time.hour());
              myFile.print(F(":"));
              if (gps.time.minute() < 10) myFile.print(F("0"));
              myFile.print(gps.time.minute());
              myFile.print(F(":"));
              if (gps.time.second() < 10) myFile.print(F("0"));
              myFile.print(gps.time.second());
              myFile.print("\n");
              start = true;
            }
            myFile.print(gps.location.lat(),6);
            myFile.print(F(","));
            myFile.print(gps.location.lng(),6);
            myFile.print(F(","));
            myFile.print(gps.speed.mps());
            myFile.print(F(","));
            myFile.print(gps.hdop.hdop());
            myFile.print(F(","));
            myFile.print(gps.time.centisecond() + gps.time.second()*100 + gps.time.minute()*6000 + gps.time.hour()*360000);
            myFile.print("\n");
          }
        }
        
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }else{
        Serial.println("Satellites aren't positioned");
      }
    }
  }
  myFile.close();
}
