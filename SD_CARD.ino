#include <SD.h>
#include <SPI.h>

File myFile;

int pinCS = 10;

void setup()
{
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);

  if (SD.begin())
  {
    Serial.println("SD Card is ready");
  }
  else
  {
    Serial.println("SD Card Failed");
    return;
  }

  myFile = SD.open("test.txt", FILE_WRITE);

  if (myFile)
  {
    Serial.println("Writing to file...");

    myFile.println("Testing Text 1, 2, 3...");
    myFile.close();
    Serial.println("Done.");
  }
  else
  {
    Serial.println("Error.");
  }


  myFile = SD.open("test.txt");
  if (myFile)
  {
    Serial.println("Read: ");

    while(myFile.available())
    {
      Serial.write(myFile.read());
    }
    myFile.close();
  }
  else
  {
    Serial.println("Error Opening test.txt");
  }
}

void loop() 
{
  // put your main code here, to run repeatedly:

}
