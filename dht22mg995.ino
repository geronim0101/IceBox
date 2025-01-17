#include <Servo.h>
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Pin definitions
#define DHT_PIN 2         // Pin connected to the DHT22 sensor data pin
#define SERVO_PIN 6       // Pin connected to the MG995 servo PWM signal
#define DHT_TYPE DHT22    // Define the type of DHT sensor

// Constants
const float TEMP_MIN = 32.0; // Minimum temperature threshold
const float TEMP_MAX = 33.0; // Maximum temperature threshold

// Objects
DHT dht(DHT_PIN, DHT_TYPE);
Servo windowServo;
LiquidCrystal_I2C lcd(0x27,20,4);

// Variables
float currentTemp;
float currentHum;
int windowState;

void setup() {
  Serial.begin(9600);
  dht.begin();
  windowState=0;//window intially closed
  lcd.begin(16, 2); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight
}

void loop() {
  // Read temperature from the DHT22 sensor
  currentTemp = dht.readTemperature();
  currentHum = dht.readHumidity();

  // Check if the reading is valid
  if (isnan(currentTemp) || isnan(currentHum)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return; // Skip the rest of the loop
  }

  Serial.print("Current Temperature: ");
  Serial.println(currentTemp);
  Serial.print("Current Humidity: ");
  Serial.println(currentHum);
  Serial.print("Window is ");

  if (windowState==0){
    Serial.println("Closed.");
  }else {
    Serial.println("Open.");
  }
  Serial.println("---------------------------------------");

  // Display data on LCD
  lcd.clear(); // Clear the display
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(currentTemp);
  lcd.setCursor(7, 0);
  lcd.print("C");

  lcd.setCursor(9, 0);
  lcd.print("H:");
  lcd.print(currentHum);
  lcd.setCursor(15, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Window is ");
  if (windowState == 0) {
    lcd.print("Close");
  } else {
    lcd.print("Open");
  }

  // Control the servo based on temperature range
  if (currentTemp < TEMP_MIN && windowState==0) {
    windowServo.attach(SERVO_PIN);
    Serial.println("Temperature is too low. Opening window...");
    windowServo.write(0); // Open window halfway (adjust angle as needed)
    delay(600);
    windowServo.detach();
    windowState=1;//window open

  } else if (currentTemp > TEMP_MAX && windowState==0) {
    windowServo.attach(SERVO_PIN);
    Serial.println("Temperature is too high. Opening window...");
    windowServo.write(0); // Open window halfway (adjust angle as needed)
    delay(600);
    windowServo.detach();
    windowState=1;//window open
  } else if (currentTemp < TEMP_MAX && currentTemp > TEMP_MIN && windowState==1) {
    windowServo.attach(SERVO_PIN);
    Serial.println("Temperature is within range. Closing window...");
    windowServo.write(180); // Close window
    delay(600);
    windowServo.detach();
    windowState=0; //window close
  }

  delay(3000); // Wait before the next reading
}
