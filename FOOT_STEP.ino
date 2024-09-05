#define FORCE_SENSOR_PIN A0 // the FSR and 10K pulldown are connected to A0

// Define thresholds for foot on/off the ground
const int FOOT_ON_THRESHOLD = 400;
const int FOOT_OFF_THRESHOLD = 200;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  int analogReading = analogRead(FORCE_SENSOR_PIN);

  Serial.print("Force sensor reading = ");
  Serial.println(analogReading); // print the raw analog reading

  if (analogReading > FOOT_ON_THRESHOLD) // Foot on the ground
  {
    Serial.println(" -> Foot on the ground");
    Serial.println(" ");
  }
  if (analogReading > FOOT_OFF_THRESHOLD && analogReading < FOOT_ON_THRESHOLD)
  {
    Serial.println(" -> Foot HALF on the ground");
    Serial.println(" "); 
  }
  if (analogReading < FOOT_OFF_THRESHOLD) // Foot off the ground
  {
    Serial.println(" -> Foot off the ground");
    Serial.println(" ");
  }
  delay(250);
}
