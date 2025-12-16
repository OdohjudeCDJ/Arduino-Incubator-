#include "DHT.h"         // Library in order to work with the DHT 11 Sensor.
#include <LCD-I2C.h>     // Library in order to work with the Liquid Crystal Display.
#include <Wire.h>        // Library in order to enable proper i2c communication.
#include <ESP32Servo.h>  // Library in order to work with the Servo Motor.

#define DHTPIN 25      // Output pin of the DHT 11 sensor
#define DHTTYPE DHT11  // using DHT 1

DHT dht(DHTPIN, DHTTYPE);  //create dht11 instance (object)
LCD_I2C lcd(0x27, 16, 2);  //create Liquid crystal display instance (object)
Servo myservo;             //create servo instance (object)


/*________DECLARATION OF VARIABLES______*/
double temperature, humidity;  // stores readings of temperature and humidity
int TEMP_CONTROL = 17;         // pins that controls temperature
int HUMD_CONTROL = 18;         // pins that controls humidity
int servo_pin = 16;
int alarm_pin = 4;


const unsigned long interval = 60000;  //intervals of turning the egg which is 10800000  milliseconds (3 hours)
unsigned long prev_time = 0;
typedef int my_special_int;
my_special_int deceive = 1;

void Print_temp_and_humd_values_to_lcd() {
  //print temperature and humidity to lcd
  lcd.setCursor(1, 0);
  lcd.print("Temp: ");
  lcd.setCursor(7, 0);
  lcd.print(temperature);
  lcd.setCursor(11, 0);
  lcd.print("*C");

  lcd.setCursor(1, 1);
  lcd.print("Humd: ");
  lcd.setCursor(7, 1);
  lcd.print(humidity);
  lcd.setCursor(11, 1);
  lcd.print("%");
}

//___________Regulate-Temperature____________________________//
void Regulate_temperature() {
  if (temperature <= 36.5) {
    //temperature is lower than 36.5°c so we raise it high
    digitalWrite(TEMP_CONTROL, HIGH);
  }

  if (temperature >= 38.0) {
    //temperature is going higher than required temperature so we turn it low
    digitalWrite(TEMP_CONTROL, LOW);
  }
}

//________________Regulate-humidity__________________________//
void Regulate_humidity() {
  if (humidity <= 50) {
    //humidity is below desired 50% so we turn on the humidifier
    digitalWrite(HUMD_CONTROL, HIGH);
  }

  if (humidity >= 65) {
    //humidity is higher then maximum level required so we turn off the hmidifier
    digitalWrite(HUMD_CONTROL, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  lcd.begin(&Wire);

  pinMode(alarm_pin, OUTPUT);
  pinMode(TEMP_CONTROL, OUTPUT);
  pinMode(HUMD_CONTROL, OUTPUT);

  digitalWrite(alarm_pin, HIGH);
  //Home page display
  lcd.display();
  lcd.setCursor(3, 0);
  lcd.print("Welcome to");
  lcd.setCursor(4, 1);
  lcd.print("Jay_Tech");
  lcd.backlight();

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);  // standard 50 hz servo
  myservo.attach(servo_pin, 500, 2500);

  for (int p = 90; p >= 0; p -= 1) {
    myservo.write(p);  //ensures the servo is at position angle 0
    delay(25);
  }


  delay(2000);
}

void loop() {
  unsigned long time = millis();  //Program has started, so we start to measure the time from here.

  humidity = dht.readHumidity();        //Read humidity and store value in humidity variable
  temperature = dht.readTemperature();  //Read temperature and store value in temperature variable

  //___________________Temp-and-humd-display-part___________________//

  // Check if any readings failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("SENSOR");
    lcd.setCursor(5, 1);
    lcd.print("Error!");

    //since we couldn't take sensor reading turn on alarm.
    digitalWrite(alarm_pin, HIGH);
    return;
  } else {
    lcd.clear();  //clear screen and print the values you've read and stored
    Print_temp_and_humd_values_to_lcd();

    //turn off alarm if error has been rectified.
    digitalWrite(alarm_pin, LOW);
    //print temperature and humidity to serial monitor
    // Serial.print(F("Humidity: "));
    // Serial.print(humidity);
    // Serial.print(F("%  Temperature: "));
    // Serial.print(temperature);
    // Serial.println(F("°C "));
  }

  //___________________________________control-part________________________________________//

  //code to maintain temperature between 36.5
  if ((temperature <= 36.5) || (temperature >= 38.0)) {
    Regulate_temperature();
  }

  //code to control humidity
  if ((humidity <= 50) || (humidity >= 65)) {
    Regulate_humidity();
  }


  // ______________________________Automatic-Egg-Turning__________________________________//
  int count;  // variable to hold an value to trigger next action.
  if (time - prev_time >= interval) {
    prev_time = time;  //reset prev_time to accurately calculate the next interval
    count = 12;        //this is just the value that gets initialized only when 3 hours has elsaped.
  } else {
    count = 0;
  }

  // this statement can only be true if 3 hours has elasped since from start time.
  if (count == 12) {
    if (myservo.read() == -1)  //if servo is at angle 0
    {
      for (int i = 0; i <= 90; i += 1) {
        count = 0;         //reset count so that it get re-initialized only when 3 hours elaspes
        myservo.write(i);  //move servo to angle 90
        delay(25);
      }
    } else if (myservo.read() == 88)  //if servo is at angle 90
    {
      for (int j = 90; j >= 0; j -= 1) {
        count = 0;         //reset count so that it get re-initialized only when 3 hours elaspes
        myservo.write(j);  //move servo to angle 0
        delay(25);
      }
    }
    count = 0;
  }
}

