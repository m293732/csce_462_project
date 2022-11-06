#include <splash.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_I2CRegister.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>

#include <Adafruit_AS7341.h>

#define DATA_PTS 12
#define RED_LED 5
#define IR_LED 6

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AS7341 as7341;



void setup() {
  // Wait for communication with the host computer serial monitor
  Serial.begin(9600);

  while (!Serial) {
    Serial.println("Could not find serial");
    delay(1);
  }

  if (!as7341.begin()){
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1) { delay(10); }
  }
  Serial.println("Configuring AS7341");
  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);

  //pinMode(RED_LED, OUTPUT)
  //pinMode(IR_LED, OUTPUT)

  display.display();
  //delay(2000); // Pause for 2 seconds

  

}
void setLED(int state){
  
}
void printReadings(const uint16_t* readings){
  // print a few wavelength ranges
    Serial.print("ADC2/F7 630nm : ");
    Serial.println(readings[8]);
    Serial.print("ADC5/NIR      : ");
    Serial.println(readings[11]);
}
void analyzeData(const uint16_t** data){
  
}
void updateDisplay(int hr, int spo2){
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);
  display.print("Heart Rate: ");
  display.println(hr);
  display.print("SpO2: ");
  display.println(spo2);
  display.display();
}
void loop() {
  uint16_t readings[12];
  uint16_t data[DATA_PTS][2];
  
  // collect data set
  for (int i = 0; i < DATA_PTS; i++){
    // turn on LEDs
    setLED(1);
    
    // Read all channels at the same time and store in as7341 object
    if (!as7341.readAllChannels(readings)){
      Serial.println("Error reading all channels!");
      return;
    }
    
    // turn off LEDs
    setLED(0);

    printReadings(readings);
    
    data[i][0] = readings[8];  // 630 nm
    data[i][1] = readings[11]; // NIR
    delay(10);
  }
  updateDisplay(1,1);

}
