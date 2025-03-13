#include "LIS3DHTR.h"
#include <Wire.h>


LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

float lastMagnitude = 0.0;

int sensorPin = A6;
int sensorValue = 0;
int soilMoisture = 0;

void setup() {
    Serial.begin(115200);
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

  
}

void loop() {
    // Read accelerometer values
    float accelX = LIS.getAccelerationX();
    float accelY = LIS.getAccelerationY();
    float accelZ = LIS.getAccelerationZ();

    // Compute total acceleration magnitude
    float magnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    // Compute change in acceleration magnitude
    float changeMagnitude = abs(magnitude - lastMagnitude);
    float movementLimit = 0.05;

    bool movementDetected = (changeMagnitude > movementLimit);

    lastMagnitude = magnitude;

    //soil moisture sensor
    sensorValue = analogRead(sensorPin);
    soilMoisture = map(sensorValue, 0, 2400, 0, 2);

    String category = "";

   if (soilMoisture == 0) {
        if (movementDetected) category = "Dry & Movement";
        else category = "Dry & No Movement";
    } else if (soilMoisture == 1) {
        if (movementDetected) category = "OK & Movement";
        else category = "OK & No Movement";
    } else if (soilMoisture == 2) {
        if (movementDetected) category = "Wet & Movement";
        else category = "Wet & No Movement";
    }

    // Print the category
    Serial.println(category);
    Serial.println(sensorValue);

    delay(2000);
}