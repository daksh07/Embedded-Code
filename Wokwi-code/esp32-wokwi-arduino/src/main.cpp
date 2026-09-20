#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() { Serial.begin(115200); pinMode(2, OUTPUT); } 

void loop() { 
  digitalWrite(2, HIGH); 
  Serial.println("LED ON"); 
  delay(1000); 
  digitalWrite(2, LOW); 
  Serial.println("LED OFF"); 
  delay(1000); 
} 