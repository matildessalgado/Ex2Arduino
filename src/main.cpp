#include <Arduino.h>
#define ledPin 13

int sensorValue = 0;
int angulo = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  sensorValue = analogRead(A0);
  angulo = map(sensorValue, 0, 1023, 0, 360);
  if (angulo >= 180) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  Serial.println(angulo);
  delay(1);
}
