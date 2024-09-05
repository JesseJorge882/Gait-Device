//    FLAT FOOT DETECTION AND STANCE TIME

// Libraries
#include <Wire.h>
#include <SPI.h>

// Declerations
#define FORCE_SENSOR_PIN A0 // FSR and 10K pulldown connected to A0

// States
int sensorState = 0; // Variable to track the state of the sensor
int previousState = 0; // Variable to track if the previous state was a press

// Step Count
int stepCount = 0;

// Stance Time
unsigned long previousTime = 0;
unsigned long stanceTimeS = 0;
unsigned long stanceTimeE = 0;
float stanceTime;

// Swing Time
unsigned long swingTimeS = 0;
unsigned long swingTimeE = 0;
float swingTime;

// Step Time
float stepTime;


void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  Wire.begin();

  previousTime = millis();
}

void loop()
{

  // Read the state of the force sensor
  sensorState = analogRead(FORCE_SENSOR_PIN);

  // Current time
  unsigned long currentTime = millis();

  // Calculate Time Remaining
  unsigned long E_Time = (currentTime - previousTime) / 1000.0;

  int rawSense = analogRead(FORCE_SENSOR_PIN);
  sensorState = 0.2 * rawSense + (1 - 0.2) * rawSense;

  // Check if the sensor is pressed
  if (sensorState > 800 && previousState < 500) //<= >=
  {
    stanceTime = 0;
    stanceTimeS = millis(); // Recorded TimeStamp e.g 10s
  }

  if (sensorState < 550 && previousState > 600)
  {
    stanceTimeE = millis(); // Recorded TimeStamp e.g 5s
    stanceTime = ((stanceTimeE - stanceTimeS) / 1000.0) + 0.2; // Calculate the Duration

    stepCount++; // Increment step count
  }

  // NOT 100% WORKING
  // Check if the force sensor is low (swing)
  if (sensorState < 400 && previousState > 500)
  {
    swingTimeS = millis(); // Start swing timer
  }

  // Check if the force sensor transitions from low to high (swing ends)
  if (sensorState > 500 && previousState < 300 && swingTimeS != 0)
  {
    swingTimeE = millis(); // End swing timer
    swingTime = (swingTimeE - swingTimeS) / 1000.0; // Calculate swing time

    stepTime = swingTime / stepCount;
  }


  Serial.print("Sensor State: ");
  Serial.print(sensorState);
  Serial.print("  Previous State: ");
  Serial.print(previousState);
  Serial.print("  Stance Time: ");
  Serial.print(stanceTime);
  Serial.print("s");
  Serial.print("  Swing Time: ");
  Serial.print(swingTime);
  Serial.println("s");

  // Check if stanceTime exceeds 60 seconds
  if (E_Time >= 60)
  {
    // Print a message indicating that 60 seconds has elapsed
    Serial.println("60 seconds elapsed. Stopping the program.");

    // Exit the loop
    while (true)
    {
      // Do nothing, effectively halting the program
    }
  }

  previousState = sensorState; // Update previous state
  delay(100);
}
