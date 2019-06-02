#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>
#include <DHT_U.h>
#define Input A1                      //The pin for detecting Light Intensity
#define RTCgnd A2                     //Pin of RTC ground
#define RTC5V A3                      //Pin of RTC 5V
#define DHT_PIN 2                     //Pin of DHT
#define WorkingCondition 3            //Pin for sending system state to Wemos
#define val_onoff 4                   //Pin for sending valve state to Wemos
#define Led_Low 5                     //Pin for switching on or off of the low level leds
#define Led_Med 6                     //Pin for switching on or off of the medium level leds
#define Led_High 7                    //Pin for switching on or off of the high level leds
#define valve_pin 8                   //Pin for switching on or off of the water valve
#define low 3                         //The light intensity of switching leds to low mode
#define moderate 6                    //The light intensity of switching leds to medium mode
#define high 9                        //The light intensity of switching leds to high mode
#define Coefficient 0.04092           //The coefficient for calculating the light intensity from photoresistor
#define DHTTYPE DHT11                 //The type of the DHT
DHT dht(DHT_PIN, DHTTYPE);
RTC_DS1307 rtc;                       //The type of the real time clock
DateTime current;
const char weekdays[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int valvenum = 1;               //The number of the water valve
const int timenum = 2;                //On and sleep modes of the system
const int on = 1;
const int off = 0;
int times[valvenum][timenum];         //Buffer for storing the on and sleep time for the system
int valve[valvenum];                  //Support for controlling multiple valves
int currenttime;
int Analog_Data;
float energy;
float humidity;
float temperature;
String tmp;
bool Working_State;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("System Activated:");
  Serial.print("\n");
  Serial.println("Please connect all components to the defined pins before uploading the code.");
  
  valve[0] = valve_pin;               //Assign the pin for the valve
  digitalWrite(RTC5V, HIGH);          //Provide 5V voltage to the RTC component
  digitalWrite(RTCgnd, LOW);          //Provide ground for the RTC component
  pinMode (RTC5V, OUTPUT);
  pinMode (RTCgnd, OUTPUT);
  pinMode(Led_Low, OUTPUT);           //Output the condition for high LEDs to WeMos D1 Mini
  pinMode(Led_Med, OUTPUT);           //Output the condition for medium LEDs to WeMos D1 Mini
  pinMode(Led_High, OUTPUT);          //Output the condition for low LEDs to WeMos D1 Mini
  pinMode(WorkingCondition,OUTPUT);   //Output the condition for the system to WeMos D1 Mini
  pinMode(val_onoff,OUTPUT);          //Output the condition for the valve to WeMos D1 Mini
  pinMode(valve[0], OUTPUT);          //Output the condition for the valve to valve and WeMos D1 Mini
/*Default setting for system working period from 0:00 to 23:59*/
  for(int i = 0; i < valvenum; i++){  
    for(int j = 0; j < timenum; j++)
    {
      if(j == 0)
      {
        times[i][j] = 23*60+59;
      }
      else
      {
        times[i][j] = 0;
      }
    }
  }
  
  Wire.begin(); 
  rtc.begin();
  dht.begin();
}

void loop() {
  rtctest();
  getTime();
  Working_State = WorkingConditionCheck();
  detectIntensity();
  LEDfeedback(energy, Led_Low, Led_Med, Led_High, Working_State);
  getTemp();
  getHumidity();
  checksetting();
  ValveState(Working_State);
  delay(1000);
}
/*Confirm the rtc component is working appropriately, adjust the RTC component to the correct time,
and store time in the variable 'current'.*/
void rtctest()  {
  if (!rtc.isrunning()) {
    Serial.print("RTC currently is not working, please check with your circuit.\n");
    return;
  }
  Serial.print("RTC currently is working!\n");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  current = rtc.now();  
}
/*Output the time stored in the variable 'current'.
Eg: 30/5/2019 Thursday 13:15:29*/
void getTime() {
    Serial.print(current.day(), DEC); 
    Serial.print('/');
    Serial.print(current.month(), DEC);
    Serial.print('/');
    Serial.print(current.year(), DEC);
    Serial.print(" ");
    Serial.print(weekdays[current.dayOfTheWeek()]);
    Serial.print(" ");
    Serial.print(current.hour(), DEC);
    Serial.print(':');
    Serial.print(current.minute(), DEC);
    Serial.print(':');
    Serial.print(current.second(), DEC);
    Serial.println();
}
/*Checking the condition of the system, more functions are called if the system is in on mode.
Otherwise, the system just stays in the sleep mode.*/
bool WorkingConditionCheck() {
   currenttime = (current.hour()*60 + current.minute());    //Get the current time and stored it in the variable 'currenttime'.
/*Compare the current time with working hours set for system, if current time is in the period,
return true. Otherwise, return false.*/
  for(int i = 0; i < valvenum; i++){                                       
     if((currenttime >= times[i][on]) && (currenttime < times[i][off]))
     {
      digitalWrite(WorkingCondition,HIGH);
      return true;
     }
     else
     {
      digitalWrite(WorkingCondition, LOW);
      return false;
     }
  }
}
/*Read the current going through in the phtotresistor and calculate the light intensity in the environment.*/
void detectIntensity()  {
  Analog_Data = analogRead(Input);
  energy = Coefficient * Analog_Data;
}
/*Check the system in the on mode, adjust light intensity of the system according to the light intensity of the environment.*/
void LEDfeedback(float Energy, int pin_low, int pin_med, int pin_high, bool statecheck)  {
         Serial.print("Current light is in "); 
         if(statecheck == true)
         {
/*If the environment light intensity is low that 3, all LEDs in the system are working to provide 
sufficent light intensity for the plant.*/
           if(Energy < low){
             Serial.print("HIGH mode\tIntensity: ");
             Serial.println(Energy);
             digitalWrite(pin_low, HIGH);
             digitalWrite(pin_med, HIGH);
             digitalWrite(pin_high, HIGH);
            }
/*If the environment light intensity is grater that 3 but lower than 6, most of LEDs in the system are working to provide 
sufficent light intensity for the plant.*/
           else if(low <= Energy && Energy < moderate){
             Serial.print("MEDIUM mode\tIntensity: ");
             Serial.println(Energy);
             digitalWrite(pin_low, HIGH);
             digitalWrite(pin_med,HIGH);
             digitalWrite(pin_high, LOW);
            }
/*If the environment light intensity is grater that 6 but lower than 9, one third LEDs in the system are working to provide 
sufficent light intensity for the plant.*/
           else if(Energy >= moderate && Energy < high){
             Serial.print("LOW mode\tIntensity: ");
             Serial.println(Energy);
             digitalWrite(pin_low, HIGH);
             digitalWrite(pin_med, LOW);
             digitalWrite(pin_high, LOW);
            }
/*If the environment light intensity is grater 9, no LEDs in the system are working since the light intensity in the oustide is 
sufficent for the plant.*/
           else if(Energy >= high){
             Serial.print("NO LIGHT mode\tIntensity: ");
             Serial.println(Energy);
             digitalWrite(pin_low, LOW);
             digitalWrite(pin_med, LOW);
             digitalWrite(pin_high, LOW);
            } 
         }
/*If the current time is not in the working period for the system, the system closes all LEDs.*/
         else
         {
             Serial.println("OFF mode");
             digitalWrite(pin_low, LOW);
             digitalWrite(pin_med, LOW);
             digitalWrite(pin_high, LOW);
         }
}
/*Read the temperature of the environment from the DHT module and print it in the serial port.*/
void getTemp(){
    temperature = dht.readTemperature();
    if(isnan(temperature))
    {
      Serial.print("Failed to read temperature from DHT!\n");
      return;
    }
    
    Serial.print("Temperature:  ");
    Serial.print(temperature);
    Serial.print("°C\n");
    Serial.flush();
}
/*Read the humidity of the environment from the DHT module and print it in the serial port.*/
void getHumidity()  {
    humidity = dht.readHumidity();
    
    if(isnan(humidity))
    {
      Serial.print("Failed to read humidity from DHT!\n");
      return;
    }    
    Serial.print("Humidity:  ");
    Serial.print(humidity);
    Serial.print("%\n");
    Serial.flush();
}
/*Check if any input from the serial port, and take actions according to the input in the serial port.*/
void checksetting()
{
  if(Serial.available() > 0) {
    tmp = Serial.readString();
  }
/*If the first character of the input in the serial monitor is 'C', function 'CurrentSetting' is executed and
serial port outputs working period of the system.*/
  if(tmp[0] == 'C')
  {
    CurrentSetting();
    return;
  }
/*If the first character of the input in the serial monitor is 'M', function 'ModifySetting is executed and
working period of the system is changed.*/
  else if(tmp[0] == 'M')
  {
    ModifySetting();
    return;
  }
/*If the first character of the input in the serial monitor is not 'M' or 'C', 
no funtion is called and system stays in its original condition.*/
  else 
  {
    Serial.print("Invalid input is detected, please check with the format! Correct formats are shown below.\n");
    Serial.print("Eg: C, c, M1N1230, M0n1125, M2F1700 ,M0f0900\n");
    Serial.flush();
    return;
  }
}
/*Serial port outputs the working period of the system.*/
void CurrentSetting() {
  for(int i = 0; i < valvenum; i++){
    Serial.print("Valve ");
    Serial.print(i);
    Serial.print(" is set to be activated from ");
    Serial.print(times[i][on]/60);
    Serial.print(":");
    Serial.print(times[i][on]-(times[i][on]/60)*60);
    Serial.print(" to ");
    Serial.print(times[i][off]/60);
    Serial.print(":");
    Serial.print(times[i][off]-(times[i][off]/60)*60);
    Serial.println();
  }
  Serial.flush();
}
/*Read the remaining input from the serial port and modify the working period of the system.*/
void ModifySetting()  {
  int valnum = tmp[1];                                  //Read the valve pin
  char state = tmp[2];                                  // Read on or sleep
  if(state != 'N' && state != 'F' && state != 'n' && state != 'f'){
    Serial.println("Unable to set the on off time since the invalid input, please check with the format! Correct formats are shown below.\n");
    Serial.print("Eg: C, c, M1N1230, M0n1125, M2F1700 ,M0f0900\n");
    Serial.flush();
    return;
  }
  int sethour = (10*(tmp[3] - '0') + (tmp[4] - '0'));   //Store the hour to be set
  int setminute = (10*(tmp[5] - '0') + (tmp[6] - '0')); //Store the minute to be set
  int timetobeset = sethour*60 + setminute;             //Convert the time to be set into minutes
  if(state == 'N' || state == 'n')  {                   //Chenge new on mode time to the system if state indicate N or n
    times[valnum][on] = timetobeset;
  }
  else if(state == 'F' || state == 'f')                 //Chenge new off mode time to the system if state indicate F or f
  {
    times[valnum][off] = timetobeset;
  }
  else
  {
    Serial.println("Unknown error causing time setting failure!");  //No action performs for incorrect input
    return;
  }
  Serial.println();
/*The serial port outputs new working period of the system after executing current function
by calling function 'CurrentSetting'.*/
  Serial.println("The time setting has been modified, current setting will be displayed below");
  Serial.flush();
  CurrentSetting();
}
/*Check the system in the on mode, adjust valve condition of the system according to the humidity of the environment.*/
void ValveState(bool statecheck) {
  for(int i = 0; i < valvenum; i++){
    Serial.print("Valve ");
    Serial.print(i);
    Serial.print(" is currently in ");
    if(statecheck == true)
    {
      Serial.println(humidity);
/*If humidty of the environment is greater than 60, the valve stays in the off mode.*/
      if(humidity > 60)
      {
        Serial.print("OFF Mode!\n");
        digitalWrite(valve[i], LOW);
        digitalWrite(val_onoff, LOW);
      }
/*If humidty of the environment is less than 60, the valve stays in the on mode.*/
      else{
        Serial.print("ON Mode!\n");
        digitalWrite(valve[i], HIGH);
        digitalWrite(val_onoff, HIGH);
      }
    }
/*If the system is in sleep mode, the valve stays in the off mode.*/
    else
    {
      Serial.print("OFF Mode!\n");
      digitalWrite(valve[i], LOW);
      digitalWrite(val_onoff, LOW);
    }
  }
}
