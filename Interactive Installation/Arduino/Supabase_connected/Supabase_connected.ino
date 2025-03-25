#include <WiFi.h>
#include <HTTPClient.h>
#include "LIS3DHTR.h"
#include <Wire.h>

// Sensor object
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

const String deviceID = "plant_3"; // CHANGE WHEN CHANGIN ARDUINOS

// WiFi Credentials
const char* ssid = "";  
const char* password = "";



// Supabase API
const char* supabaseURL = "";  
const char* supabaseApiKey = "";

// Sensor Variables
float lastMagnitude = 0.0;
int sensorPin = A6;
int sensorValue = 0;
int soilMoisture = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting...");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    int timeout = 30; 
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
        Serial.println("WiFi disconnected, attempting to reconnect...");
        WiFi.disconnect();  // Ensure clean state
        WiFi.begin(ssid, password);
        int timeout = 10;  // 10 seconds timeout
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(1000);
            Serial.print(".");
            timeout--;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Reconnected to WiFi!");
        } else {
            Serial.println("Reconnection failed!");
        }
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
    soilMoisture = map(sensorValue, 0, 4096, 0, 2); // Mapped moisture value

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