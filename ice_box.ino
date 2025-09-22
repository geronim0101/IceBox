#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Pin and sensor type definitions
#define DHT_PIN1 A0 // sample
#define DHT_PIN2 A1 // cooler
#define DHT_PIN3 A2 // heater
#define DHT_TYPE DHT22
#define COOLER_FAN_PIN 3
#define HEATER_FAN_PIN 5

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DHT objects
DHT dht1(DHT_PIN1, DHT_TYPE);
DHT dht2(DHT_PIN2, DHT_TYPE);
DHT dht3(DHT_PIN3, DHT_TYPE);

// Fan states and thresholds
String cooler_fan_state = "OFF";
String heater_fan_state = "OFF";
float set_temp = 18.0; // default setpoint
float lower_temp_lim = set_temp - 0.5;
float higher_temp_lim = set_temp + 0.5;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
bool inKeypadMode = false;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {13, 12, 11, 10};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Screen cycle
int screenState = 0; // 0 = temp/humidity, 1 = fans/setpoint

void setup() {
  pinMode(COOLER_FAN_PIN, OUTPUT);
  pinMode(HEATER_FAN_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();

  dht1.begin();
  dht2.begin();
  dht3.begin();
  
  Serial.begin(9600);
  Serial.println("Time(ms),CoolerTemperature,CoolerHumidity,SampleTemperature,SampleHumidity,HeaterTemperature,HeaterHumidity,CoolerFanState,HeaterFanState");
}

void loop() {
  // ====== Keypad handling ======
  char key = keypad.getKey();
  static String input = "";
  
  if (key) {
    inKeypadMode = true; // pause sensor display

    if (key >= '0' && key <= '9') {
      input += key;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Temp:");
      lcd.setCursor(0,1);
      lcd.print(input);
    }
    else if (key == 'A') { // decimal point
      if (input.indexOf('.') == -1) { // only allow one '.'
        input += '.';
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set Temp:");
        lcd.setCursor(0,1);
        lcd.print(input);
      }
    }
    else if (key == '#') { // confirm
      if (input.length() > 0) {
        set_temp = input.toFloat();
        lower_temp_lim = set_temp - 0.5;
        higher_temp_lim = set_temp + 0.5;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Target set:");
        lcd.setCursor(0,1);
        lcd.print(set_temp,1); // show 1 decimal
        lcd.print(" C");
        delay(2000);
      }
      input = "";
      inKeypadMode = false;
    }
    else if (key == '*') { // cancel
      input = "";
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Entry cleared");
      delay(1000);
      inKeypadMode = false;
    }
  }

  // ====== Sensor reading every 5s ======
  static unsigned long lastRead = 0;
  unsigned long now = millis();

  if (!inKeypadMode && (now - lastRead >= 3000)) {
    lastRead = now;

    float temp_cooler = dht2.readTemperature();
    float hum_cooler  = dht2.readHumidity();
    float temp_sample = dht1.readTemperature();
    float hum_sample  = dht1.readHumidity();
    float temp_heater = dht3.readTemperature();
    float hum_heater  = dht3.readHumidity();

    // Validate readings
    if (isnan(temp_cooler) || isnan(hum_cooler) ||
        isnan(temp_sample) || isnan(hum_sample) ||
        isnan(temp_heater) || isnan(hum_heater)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sensor error");
      return;
    }

    // Fan control
    if (temp_sample >= higher_temp_lim) {
      digitalWrite(COOLER_FAN_PIN, HIGH);
      cooler_fan_state = "ON ";
      digitalWrite(HEATER_FAN_PIN, LOW);
      heater_fan_state = "OFF";
    } else if (temp_sample <= lower_temp_lim) {
      digitalWrite(COOLER_FAN_PIN, LOW);
      cooler_fan_state = "OFF";
      digitalWrite(HEATER_FAN_PIN, HIGH);
      heater_fan_state = "ON ";
    } else {
      digitalWrite(COOLER_FAN_PIN, LOW);
      cooler_fan_state = "OFF";
      digitalWrite(HEATER_FAN_PIN, LOW);
      heater_fan_state = "OFF";
    }

    // ====== LCD Screens ======
    lcd.clear();
    if (screenState == 0) {
      // --- Screen 1: Temperatures and Humidities ---
      lcd.setCursor(0,0);
      lcd.print("T:");
      lcd.print(round(temp_cooler)); lcd.print("/");
      lcd.print(round(temp_sample)); lcd.print("/");
      lcd.print(round(temp_heater)); lcd.print("C");

      lcd.setCursor(0,1);
      lcd.print("H:");
      lcd.print(round(hum_cooler)); lcd.print("/");
      lcd.print(round(hum_sample)); lcd.print("/");
      lcd.print(round(hum_heater)); lcd.print("%");

      screenState = 1; // switch next cycle
    }
    else {
      // --- Screen 2: Fan states + Setpoint ---
      lcd.setCursor(0,0);
      lcd.print("Fans:C=" + cooler_fan_state);
      lcd.setCursor(10,0);
      lcd.print("/H=" + heater_fan_state);

      lcd.setCursor(0,1);
      lcd.print("Set:");
      lcd.print(set_temp,1); // show 1 decimal
      lcd.print("C(+-0.5)");

      screenState = 0; // switch back next cycle
    }

    // ====== Serial logging ======
    Serial.print(millis()); Serial.print(",");
    Serial.print(temp_cooler); Serial.print(",");
    Serial.print(hum_cooler); Serial.print(",");
    Serial.print(temp_sample); Serial.print(",");
    Serial.print(hum_sample); Serial.print(",");
    Serial.print(temp_heater); Serial.print(",");
    Serial.print(hum_heater); Serial.print(",");
    Serial.print(cooler_fan_state); Serial.print(",");
    Serial.println(heater_fan_state);
  }
}
