#include <Wire.h>
#include <ADXL345.h>
#include <SD.h>
#include <SPI.h>

#define FORCE_SENSOR_PIN A0
#define ADXL345 0x53 
#define GRAVITY 9.81
#define MAX_SAMPLES 20

float accX, accY, accZ, SX, pX, dX, velX, pvelX, dispX, distX, Speed;
unsigned long previousTime, currentTime, E_Time, startTime, startTime2, stopTime, stanceTimeS, stanceTimeE;
float swingTime = 0, stanceTime = 0, cadance, stepTime, stepLength;
int rawSense, sensorState, previousState, stepCount;
float speedBuffer[MAX_SAMPLES], swingTimeBuffer[MAX_SAMPLES], stanceTimeBuffer[MAX_SAMPLES], stepTimeBuffer[MAX_SAMPLES], stepLengthBuffer[MAX_SAMPLES];
int speedBufferIndex = 0, swingTimeBufferIndex = 0, stanceTimeBufferIndex = 0, stepTimeBufferIndex = 0, stepLengthBufferIndex = 0;
bool timing = false, sixtySecondsElapsed = false;
File dataFile;
int chipSelect = 10;

void writeDataToFile(float averageSpeed, float averageSwingTime, float averageStanceTime, float averageStepTime, float averageStepLength);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(ADXL345); Wire.write(0x2D); Wire.write(8); Wire.endTransmission();
  Wire.beginTransmission(ADXL345); Wire.write(0x1E); Wire.write(0); Wire.endTransmission();
  Wire.beginTransmission(ADXL345); Wire.write(0x1F); Wire.write(2); Wire.endTransmission();
  Wire.beginTransmission(ADXL345); Wire.write(0x20); Wire.write(4); Wire.endTransmission();
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed.");
    return;
  } else {
    Serial.println("SD Card is ready.");
  }
}

void loop() {
  currentTime = millis();
  E_Time = (currentTime - previousTime);

if ((sensorState > 800 && previousState > 800)) {
    if (startTime2 == 0) {
      startTime2 = currentTime;
    } else {
      if (currentTime - startTime2 >= 3000) {
        storeData();
        startTime2 = 0;
      }
    }
  } else {
    startTime2 = 0;
  }

  rawSense = analogRead(FORCE_SENSOR_PIN);
  sensorState = 0.2 * rawSense + (1 - 0.2) * rawSense;
  Wire.beginTransmission(ADXL345); Wire.write(0x32); Wire.endTransmission(false); Wire.requestFrom(ADXL345, 6, true);
  accX = ( Wire.read() | Wire.read() << 8); accX = (accX/256);
  accY = ( Wire.read() | Wire.read() << 8); accY = (accY/256);
  accZ = ( Wire.read() | Wire.read() << 8); accZ = (accZ/256);
  SX = 0.2 * accX + (1 - 0.2) * SX;

  if (sensorState > 600 && previousState < 550) { stanceTime = 0; stanceTimeS = millis(); }
  if (sensorState < 550 && previousState > 600) {
    stanceTimeE = millis();
    stanceTime = ((stanceTimeE - stanceTimeS) / 1000.0);
    stepCount++;
    addStanceTimeToBuffer(stanceTime);
  }
  if (sensorState < 550 && previousState > 600) { startTime = millis(); }
  if (sensorState > 600 && previousState < 550 && startTime != 0) { stopTime = millis(); // End swing timer
    swingTime = (stopTime - startTime) / 1000.0;
  }

  pX = SX; SX = accX; dX = SX - pX;
  if (dX < 0) dX = -dX;

  if (sensorState < 550) {
    velX = (0.5 * dX) * GRAVITY * (E_Time / 1000.0);
    dispX += ((0.5 * (velX + pvelX)) * (E_Time / 1000.0)) * 1.5;
    pvelX = velX;
  }
  if (sensorState > 550 && startTime != 0){
      distX = abs(dispX);
      Speed = distX / swingTime;
      startTime = 0;
      if (stepCount > 1) {
        stepTime = swingTime / stepCount;
        stepLength = ((Speed * swingTime) / stepCount) / 2;
        addStepTimeToBuffer(stepTime);
        addStepLengthToBuffer(stepLength);
      }
      addSpeedToBuffer(Speed);
      addSwingTimeToBuffer(swingTime);
  }
  previousTime = currentTime;
  previousState = sensorState;
  delay(100);
}

void addSpeedToBuffer(float speed) {
  speedBuffer[speedBufferIndex] = speed;
  speedBufferIndex = (speedBufferIndex + 1) % MAX_SAMPLES;
}

void addSwingTimeToBuffer(float swingTime) {
  swingTimeBuffer[swingTimeBufferIndex] = swingTime;
  swingTimeBufferIndex = (swingTimeBufferIndex + 1) % MAX_SAMPLES;
}

void addStanceTimeToBuffer(float stanceTime) {
  stanceTimeBuffer[stanceTimeBufferIndex] = stanceTime;
  stanceTimeBufferIndex = (stanceTimeBufferIndex + 1) % MAX_SAMPLES;
}

void addStepTimeToBuffer(float stepTime) {
  stepTimeBuffer[stepTimeBufferIndex] = stepTime;
  stepTimeBufferIndex = (stepTimeBufferIndex + 1) % MAX_SAMPLES;
}

void addStepLengthToBuffer(float stepLength) {
  stepLengthBuffer[stepLengthBufferIndex] = stepLength;
  stepLengthBufferIndex = (stepLengthBufferIndex + 1) % MAX_SAMPLES;
}

void storeData() {
  cadance = ((stepCount * 2) / (currentTime/1000.0)) * 60;
  float totalSpeed = 0, totalSwingTime = 0, totalStanceTime = 0, totalStepTime = 0, totalStepLength = 0;

  for (int i = 0; i < speedBufferIndex; i++) totalSpeed += speedBuffer[i];
  float averageSpeed = totalSpeed / speedBufferIndex;

  for (int i = 0; i < swingTimeBufferIndex; i++) totalSwingTime += swingTimeBuffer[i];
  float averageSwingTime = totalSwingTime / swingTimeBufferIndex;

  for (int i = 0; i < stanceTimeBufferIndex; i++) totalStanceTime += stanceTimeBuffer[i];
  float averageStanceTime = totalStanceTime / stanceTimeBufferIndex;

  for (int i = 0; i < stepTimeBufferIndex; i++) totalStepTime += stepTimeBuffer[i];
  float averageStepTime = totalStepTime / stepTimeBufferIndex;

  for (int i = 0; i < stepLengthBufferIndex; i++) totalStepLength += speedBuffer[i];
  float averageStepLength = totalStepLength / speedBufferIndex;

  writeDataToFile(averageSpeed, averageSwingTime, averageStanceTime, averageStepTime, averageStepLength);
}

void writeDataToFile(float averageSpeed, float averageSwingTime, float averageStanceTime, float averageStepTime, float averageStepLength) {
  
  stepCount * 2;
  dataFile = SD.open("gait.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("");
    dataFile.print("Cadance: "); dataFile.print(cadance); dataFile.println(" Steps/min");
    dataFile.print("Gait Speed: "); dataFile.print(averageSpeed); dataFile.println(" m/s");
    dataFile.print("Swing Time: "); dataFile.print(averageSwingTime); dataFile.println(" seconds");
    dataFile.print("Stance Time: "); dataFile.print(averageStanceTime); dataFile.println(" seconds");
    dataFile.print("Step Time: "); dataFile.print(averageStepTime); dataFile.println(" seconds");
    dataFile.print("Step Length: "); dataFile.print(averageStepLength); dataFile.println(" meters");
    dataFile.print("Step Count: "); dataFile.print(stepCount); dataFile.println("");
    dataFile.print("Time: "); dataFile.print(currentTime/1000.0); dataFile.println(" seconds");
    dataFile.close();
    Serial.println("Data written to file.");

  Serial.print("Cadance: "); Serial.print(cadance); Serial.println(" Steps/min");
  Serial.print("Gait Speed: "); Serial.print(averageSpeed); Serial.println(" m/s");
  Serial.print("Swing Time: "); Serial.print(averageSwingTime); Serial.println(" seconds");
  Serial.print("Stance Time: "); Serial.print(averageStanceTime); Serial.println(" seconds");
  Serial.print("Step Time: "); Serial.print(averageStepTime); Serial.println(" seconds");
  Serial.print("Step Length: "); Serial.print(averageStepLength); Serial.println(" meters");
  Serial.print("Step Count: "); Serial.print(stepCount); Serial.println("");
  Serial.print("Time: "); Serial.print(currentTime/1000.0); Serial.println(" seconds");
  } else {
    Serial.println("Error writing to file!");
  }
}
