#include <Adafruit_AS7341.h>
#include <Adafruit_SSD1306.h>
void setup() {
  // Wait for communication with the host computer serial monitor
  Serial.begin(115200);

  while (!Serial) {
    delay(1);
  }

  if (!as7341.begin()){
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }

  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);

  // red LED
  pinMode(3, OUTPUT)
  pinMode(5, OUTPUT)

}

void loop() {
  // Read all channels at the same time and store in as7341 object
  if (!as7341.readAllChannels()){
    Serial.println("Error reading all channels!");
    return;
  }
  // print a few wavelength ranges
  Serial.print("ADC1/F6 590nm : ");
  Serial.println(readings[7]);
  Serial.print("ADC2/F7 630nm : ");
  Serial.println(readings[8]);
  Serial.print("ADC3/F8 680nm : ");
  Serial.println(readings[9]);
  Serial.print("ADC5/NIR      : ");
  Serial.println(readings[11]);
  

}
