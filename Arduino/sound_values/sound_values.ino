#include "LIS3DHTR.h"
#include <Wire.h>

LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

// Accelerometer
float lastMagnitude = 0.0;

// Soil Moisture Sensor
int sensorPin = A6;
int sensorValue = 0;
int soilMoisture = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial) { };

    // Initialize Accelerometer
    LIS.begin(WIRE, 0x19);
    if (!LIS) {
        Serial.println("LIS3DHTR not connected");
        while (1);
    }

    LIS.openTemp();
    delay(100);
    LIS.setFullScaleRange(LIS3DHTR_RANGE_4G);  
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_10HZ);  

    Serial.println("LIS3DHTR Ready");

    // Initialize Soil Moisture Sensor
    pinMode(sensorPin, INPUT);
}

void loop() {
    // Read Accelerometer Values
    float accelX = LIS.getAccelerationX();
    float accelY = LIS.getAccelerationY();
    float accelZ = LIS.getAccelerationZ();

    // Calculate total acceleration magnitude
    float magnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    // Change in acceleration magnitude (for movement detection)
    float changeMagnitude = abs(magnitude - lastMagnitude);
    float movementLimit = 0.08;  
    bool movementDetected = (changeMagnitude > movementLimit);
    lastMagnitude = magnitude;

    // Read Soil Moisture Sensor
    sensorValue = analogRead(sensorPin); // Raw sensor value
    soilMoisture = map(sensorValue, 0, 4095, 0, 2); // Mapped soil moisture value

    // Create a combined variable (sum of all values)
    float totalDataValue = magnitude + changeMagnitude + sensorValue;

    // Print values to Serial Monitor
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

    delay(2000); // Adjust delay for smooth readings
}