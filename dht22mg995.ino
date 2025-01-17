#include <Servo.h>
#include <DHT.h>

// Pin definitions
#define DHT_PIN 2         // Pin connected to the DHT22 sensor data pin
#define SERVO_PIN 6       // Pin connected to the MG995 servo PWM signal
#define DHT_TYPE DHT22    // Define the type of DHT sensor

// Constants
const float TEMP_MIN = 32.55; // Minimum temperature threshold
const float TEMP_MAX = 32.65; // Maximum temperature threshold

// Objects
DHT dht(DHT_PIN, DHT_TYPE);
Servo windowServo;

// Variables
float currentTemp;
int windowState;

void setup() {
  Serial.begin(9600);
  dht.begin();
  windowState=0;//window intially closed
}

void loop() {
  // Read temperature from the DHT22 sensor
  currentTemp = dht.readTemperature();

  // Check if the reading is valid
  if (isnan(currentTemp)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return; // Skip the rest of the loop
  }

  Serial.print("Current Temperature: ");
  Serial.println(currentTemp);
  Serial.print("Window is ");
  if (windowState==0){
    Serial.println("Closed.");
  }else {
    Serial.println("Open.");
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

  delay(5000); // Wait before the next reading
}
