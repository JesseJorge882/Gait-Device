/*                               Libraries                                      */
#include <Wire.h>
#include <ADXL345.h>

/*                              Declerations                                    */
#define FORCE_SENSOR_PIN A0 // FSR and 10K pulldown connected to A0
#define ADXL345 0x53 // ADXL345 Connection Set Up
#define GRAVITY 9.81 // Gravity in m/s^2

/*                               Functions                                      */


/*                               Variables                                      */
// Gait Speed
float accX, accY, accZ;
float SX;
float pX = 0.0;
float dX = 0.0;
float velX = 0.0; // Variable to store the velocity
float pvelX = 0.0;
float dispX = 0.0; // Variable to store the displacement
float distX = 0.0; // Variable to store the distance traveled
float Speed = 0.0; // Variable to store the speed

// Time
unsigned long previousTime = 0; // Variable to store the previous time
unsigned long currentTime = 0; // Variable to store the current time
unsigned long E_Time = 0; // Variable to store the change in time

// Swing Time
unsigned long startTime = 0; // Variable to store the start time
unsigned long stopTime = 0; // Variable to store the stop time
float swingTime = 0; // Variable to store the stop time
bool timing = false; // Flag to indicate if timing is active

// FSR07 States
int sensorState = 0; // Variable to track the state of the sensor
int previousState = 0; // Variable to track if the previous state was a press

// Step Count
int stepCount = 0;

// Stance Time
unsigned long S_Time = 0;
unsigned long stanceTimeS = 0;
unsigned long stanceTimeE = 0;
float stanceTime;

// Step Time
float stepTime;

// Step Length
float stepLength = 0;


void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  Wire.begin();

  // Accelerometer
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D);
  Wire.write(8);
  Wire.endTransmission();

  // Calibration
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);
  Wire.write(0);
  Wire.endTransmission();

  Wire.beginTransmission(ADXL345);
  Wire.write(0x1F);
  Wire.write(2);
  Wire.endTransmission();
  
  Wire.beginTransmission(ADXL345);
  Wire.write(0x20);
  Wire.write(4);
  Wire.endTransmission();

}

void loop()
{
  // Record Time
  currentTime = millis(); // Get the current time
  E_Time = (currentTime - previousTime); // / 1000.0; // Calculate the change in time


  // Read force sensor data
  sensorState = analogRead(FORCE_SENSOR_PIN);

  // Read acceleration data
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);
  
  accX = ( Wire.read() | Wire.read() << 8);
  accX = (accX/256);

  accY = ( Wire.read() | Wire.read() << 8);
  accY = (accY/256);

  accZ = ( Wire.read() | Wire.read() << 8);
  accZ = (accZ/256);

  // Low-pass Filter
  SX = 0.2 * accX + (1 - 0.2) * SX;

  // Stance Time Calculations
  if (sensorState > 800 && previousState < 500)
  {
    if (stanceTimeE != 0)
    {
      // Reset stance time variables if there was an incomplete stance phase
      stanceTimeS = 0;
      stanceTimeE = 0;
    }
    stanceTimeS = millis(); // Recorded TimeStamp e.g 10s
  }

  if (sensorState < 500 && previousState > 800)
  {
    if (stanceTimeS != 0)
    {
      stanceTimeE = millis(); // Recorded TimeStamp e.g 5s
      stanceTime = ((stanceTimeE - stanceTimeS) / 1000.0) + 0.2; // 0.2s for Toe-Off
      // Calculate the Duration e.g ((10 - 5) / 1000.0) + 0.2
    }
    else
    {
      // If stanceTimeS is not initialized, reset it
      stanceTimeS = 0;
      stanceTimeE = 0;
    }
    stepCount++; // Increment step count
  }

  // Reset Force Sensor Variables
  if (sensorState == 0 && previousState == 0)
  {
    stanceTimeS = 0;
    stanceTimeE = 0;
  }


  // Record new values
  pX = SX;
  SX = accX;

  dX = SX - pX;
  if (dX < 0)
  {
    dX = -dX;
  }


  // Calculates the displacement
  if (dX >= 0.01)
  {
    velX = (0.5 * dX) * GRAVITY * (E_Time / 1000.0); // Trapezoidal rule for velocity
    dispX += ((0.5 * (velX + pvelX)) * (E_Time / 1000.0)) * 1.5; // Trapezoidal rule for displacement

    pvelX = velX;
  }

  // If foot starts to leave the ground
  else if (SX > 0.2 || SX < -0.2)
  {
    if (!timing)
    {
      startTime = millis(); // Start timing
      timing = true;
    }
  }
  else
  {
    if (timing)
    {
      stopTime = millis(); // Stop timing
      timing = false;

      distX = abs(dispX); // Calculate the absolute distance traveled

      swingTime = (stopTime - startTime)/1000.0;
      Speed = distX / swingTime;

    }
  }
  
  // Step Time Calculations
  stepTime = swingTime / stepCount;

  // Calculate Step Length
  stepLength = (Speed * swingTime) / stepCount; // swingTime

  Serial.print("Time: ");
  Serial.print(E_Time/1000.0);

  Serial.print("s  Delta X: ");
  Serial.print(dX);
  Serial.print("  SX: ");
  Serial.print(SX);
  Serial.print("  prev SX: ");
  Serial.print(pX);

  //Serial.print("s  vel X: ");
  //Serial.print(velX);
  //Serial.print("  last vel X: ");
  //Serial.print(pvelX);

  Serial.print("  Disp X: ");
  Serial.print(dispX);
  Serial.print("m  Speed: ");
  Serial.print(Speed);
  Serial.println("m/s");

  previousTime = currentTime; // Update the previous time
  previousState = sensorState; // Update previous state

  delay(100); // Delay for stability
}
