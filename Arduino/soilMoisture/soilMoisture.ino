int sensorPin = A6;
int sensorValue = 0;
int soilMoisture = 0;

void setup() {
    Serial.begin(9600);
}
void loop() {
  
  sensorValue = analogRead(sensorPin);

  soilMoisture = map(sensorValue, 0, 4095, 0, 2);
  Serial.println(sensorValue);


  delay(1000);

}