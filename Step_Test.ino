//
// SUCCESS
//

// Libraries
#include <Wire.h>
#include <ADXL345.h>

// Define the pin for the force sensor
#define FORCE_SENSOR_PIN A0 // FSR and 10K pulldown connected to A0

// NOT IN USE
const int threshold = 100; // Define the threshold for considering a press

// States
int sensorState = 0; // Variable to track the state of the sensor
int previousState = 0; // Variable to track if the previous state was a press

// Variable to count steps
int stepCount = 0;

void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  Wire.begin();
}

void loop()
{
  // Read the state of the force sensor
  sensorState = analogRead(FORCE_SENSOR_PIN);

  // Check if the sensor is pressed
  if (sensorState > 800 && previousState < 100)
  {
    stepCount++; // Increment step count

    // Print step count
    Serial.print("Step Count: ");
    Serial.println(stepCount);

    delay(50); // Delay to debounce
  }

  
  Serial.print("Sensor State: ");
  Serial.println(sensorState);
  Serial.print("Previous State: ");
  Serial.println(previousState);
  

  previousState = sensorState; // Update previous state
  delay(500);
}
