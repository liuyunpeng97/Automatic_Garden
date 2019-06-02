#include <BlynkSimpleEsp8266.h> //Exteral application library
#include <Wire.h>
#include <DHTesp.h>
#define dht_pin D2              //Pin for reading data from DHT module
#define pin_low D3              //Pin for reading data from low pin in Arduino UNO board
#define pin_medium D4           //Pin for reading data from medium pin in Arduino UNO board
#define pin_high D5             //Pin for reading data from high pin in Arduino UNO board
#define val_state D6            //Pin for reading valve condition from Arduino UNO board
#define sys_state D7            //Pin for reading system state from Arduino UNO board
DHTesp dht;
WidgetLCD lcd(V9);
char auth[] = "5f903776947940769a01e25be8e4d978";
char ssid[] = "ABCDEF";         //Name of the WIFI connection
char pass[] = "xxxxxxxx";       //Password for the WIFI
int low = 0;
int medium = 0;
int high = 0;

void setup()
{
  
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  pinMode(pin_low,INPUT);
  pinMode(pin_medium,INPUT);
  pinMode(pin_high,INPUT);
  pinMode(val_state,INPUT);
  pinMode(sys_state,INPUT);
  dht.setup(dht_pin, DHTesp::DHT11);
}

void loop()
{
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  Blynk.run();
  senddht();
  sendlight();
  sendsys();
  sendval();
  delay(1000);
}
/*Read intensity data from the Arduino UNO board and upload the data to Blynk.*/
void sendlight()
{
  low = digitalRead(pin_low);
  medium = digitalRead(pin_medium);
  high  = digitalRead(pin_high);
/*Output the LEDs condition in the serial port.*/
  Serial.print("Low:");
  Serial.println(low);
  Serial.print("Medium:");
  Serial.println(medium);
  Serial.print("High:");
  Serial.println(high);
/*If the LEDs from the Arduino UNO board is in sleep mode or no light mode, 'LED: no light' of Blynk turns on.*/
  if(low == LOW && medium == low && high == low)
  {
    Blynk.virtualWrite(V2,1023);
    Blynk.virtualWrite(V3,0);
    Blynk.virtualWrite(V4,0);
    Blynk.virtualWrite(V5,0);
  }
/*If the LEDs condtion from the Arduino UNO board is in low mode, 'LED: low' of Blynk turns on.*/
  else if(low == HIGH && medium == LOW && medium == high)
  {
    Blynk.virtualWrite(V2,0);
    Blynk.virtualWrite(V3,1023);
    Blynk.virtualWrite(V4,0);
    Blynk.virtualWrite(V5,0);
  }
/*If the LEDs condtion from the Arduino UNO board is in medium mode, 'LED: medium' of Blynk turns on.*/
  else if(low == HIGH && medium == HIGH && high == LOW)
  {
    Blynk.virtualWrite(V2,0);
    Blynk.virtualWrite(V3,0);
    Blynk.virtualWrite(V4,1023);
    Blynk.virtualWrite(V5,0);
  }
/*If the LEDs condtion from the Arduino UNO board is in high mode, 'LED: high' of Blynk turns on.*/
else if(low == HIGH && medium == HIGH && high == HIGH)
  {
    Blynk.virtualWrite(V2,0);
    Blynk.virtualWrite(V3,0);
    Blynk.virtualWrite(V4,0);
    Blynk.virtualWrite(V5,1023);
  }
/*If the LEDs condtion from the Arduino UNO board is in unknown condition, all LEDs of Blynk turn on.*/
  else
  {
    Serial.println(low);
    Serial.println(medium);
    Serial.println(high);
    Blynk.virtualWrite(V2,1023);
    Blynk.virtualWrite(V3,1023);
    Blynk.virtualWrite(V4,1023);
    Blynk.virtualWrite(V5,1023);
  }
}
/*Read the humidity and temperature from DHT module and output them in the serial port and Blynk.*/
void senddht()
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  if(isnan(humidity) || isnan(temperature))
  {
    Serial.println("Unable to read data from the dht module!");
    return;
  }
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  Blynk.virtualWrite(V0,temperature);
  Blynk.virtualWrite(V1,humidity);
}
/*Read the system state from the Arduino UNO and upload it to Blynk.*/
void sendsys()
{
  lcd.clear();
  lcd.print(3,0,"SYSTEM STATE");
  if(digitalRead(D7) == 1)
  {
    lcd.print(8,1,"ON");
    return;
  }
  lcd.print(8,1,"OFF");
}
/*Read the valve condition and upload it to Blynk.*/
void sendval() {
  if(digitalRead(val_state) == 1)
  {
  Blynk.virtualWrite(V6,1023);
  return;
  }
  Blynk.virtualWrite(V6,0);

}
