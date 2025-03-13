#include <WiFi.h>
#include <HTTPClient.h>
#include "LIS3DHTR.h"
#include <Wire.h>

// Sensor object
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

// WiFi Credentials
// const char* ssid = "Antonia's Iphone";  
// const char* password = "parola123";

const char* ssid = "FabulousNet";  
const char* password = "25jan2022";
const String deviceID = "plant_1"; // CHANGE BEFORE RUNNING, plant_1, plant_2, plant_3

// const char* ssid = "Sunrise_4790874";   
// const char* password = "zsczhjay5gyFyxaV";

// Supabase API
const char* supabaseURL = "https://bhuvkendkwmmnbzkjxsr.supabase.co/rest/v1/sensors_values";  // Table name is sensors_values
const char* supabaseApiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImJodXZrZW5ka3dtbW5iemtqeHNyIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDExNzY3OTUsImV4cCI6MjA1Njc1Mjc5NX0.EKYOtg_1rExta0k4UkZGViJg3bup-a3uRrZI1Ih0bzM";

// Sensor Variables
float lastMagnitude = 0.0;
int sensorPin = A6;
int sensorValue = 0;
int soilMoisture = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int timeout = 30;  // 30 seconds timeout
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(1000);
        Serial.print(".");
        timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
    } else {
        Serial.println("Failed to connect to WiFi!");
        return;
    }

    //Accelerometer
    LIS.begin(WIRE, 0x19);
    if (!LIS) {
        Serial.println("LIS3DHTR not connected");
        while (1);
    }

    LIS.openTemp();
    delay(100);

    LIS.setFullScaleRange(LIS3DHTR_RANGE_4G);
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_10HZ);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, skipping data send.");
        return;
    }

    // Read Accelerometer values
    float accelX = LIS.getAccelerationX();
    float accelY = LIS.getAccelerationY();
    float accelZ = LIS.getAccelerationZ();

    // magnitude
    float magnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
    float changeMagnitude = abs(magnitude - lastMagnitude);
    float movementLimit = 0.05;
    bool movementDetected = (changeMagnitude > movementLimit);
    lastMagnitude = magnitude;

    // Read soil moisture
    sensorValue = analogRead(sensorPin); // Raw sensor value
    soilMoisture = map(sensorValue, 0, 2400, 0, 2); // Mapped moisture value

    // Values sum for sound
    float totalDataValue = magnitude + sensorValue;

    // Print values for debugging
    Serial.print("X: "); Serial.print(accelX, 4);
    Serial.print(" | Y: "); Serial.print(accelY, 4);
    Serial.print(" | Z: "); Serial.print(accelZ, 4);
    Serial.print(" | Magnitude: "); Serial.print(magnitude, 4);
    Serial.print(" | Change: "); Serial.print(changeMagnitude, 4);
    Serial.print(" | Soil Moisture (Raw): "); Serial.print(sensorValue);
    Serial.print(" | Soil Moisture (Mapped): "); Serial.print(soilMoisture);
    Serial.print(" | TotalDataValue: "); Serial.print(totalDataValue);
    Serial.print(" | Status: ");
    
    if (movementDetected) {
        Serial.println("Movement detected");
    } else {
        Serial.println("No movement");
    }

    // Send data to Supabase
    sendToSupabase(soilMoisture, movementDetected, totalDataValue);

    delay(5000);  
}

void sendToSupabase(int moisture, bool movement, float totalDataValue) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Skipping request, WiFi is disconnected.");
        return;
    }

    HTTPClient http;
    http.begin(supabaseURL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabaseApiKey);
    http.addHeader("Authorization", "Bearer " + String(supabaseApiKey));
    http.addHeader("Prefer", "return=minimal");  // Minimize response

    // JSON payload now includes totalDataValue
    String jsonPayload = 
      "{\"device_id\": \"" + deviceID + "\", \"values\": {" +
      "\"moisture\": " + String(moisture) + 
      ", \"movement\": " + (movement ? "true" : "false") + 
      ", \"totalDataValue\": " + String(totalDataValue) + " } }";

  
    Serial.print("Sending to Supabase: ");
    Serial.println(jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    http.end();
}