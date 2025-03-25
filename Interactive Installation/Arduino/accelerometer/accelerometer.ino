#include "LIS3DHTR.h"
#include <Wire.h>

LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

float lastMagnitude = 0.0;

void setup() {
    Serial.begin(115200);
    while (!Serial) { };

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
}

void loop() {
    // Read acceleration values 
    float accelX = LIS.getAccelerationX();
    float accelY = LIS.getAccelerationY();
    float accelZ = LIS.getAccelerationZ();

    // Calculate total acceleration magnitude
    float magnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    // Change in acceleration magnitude
    float changeMagnitude = abs(magnitude - lastMagnitude);

    float movementLimit = 0.08;  

    // Print the acceleration values
    Serial.print("X: ");
    Serial.print(accelX, 4);  // Print with 4 decimal places
    Serial.print(" | Y: ");
    Serial.print(accelY, 4);
    Serial.print(" | Z: ");
    Serial.print(accelZ, 4);
    Serial.print(" | Magnitude: ");
    Serial.print(magnitude, 4);
    Serial.print(" | Change: ");
    Serial.print(changeMagnitude, 4);
    Serial.print(" | Status: ");
    
    if (changeMagnitude > movementLimit) {
        Serial.println("Movement detected");
    } else {
        Serial.println("No movement");
    }

    lastMagnitude = magnitude;

    delay(300);  // Adjust delay for a smoother or faster reading
}