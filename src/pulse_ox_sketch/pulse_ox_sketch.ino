#include <splash.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_I2CRegister.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>

#include <Adafruit_AS7341.h>

#define DATA_PTS 24
#define RED_LED 5
#define IR_LED 9
#define light_max 9000

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AS7341 as7341;
unsigned long time;
uint16_t readings[12];
uint16_t data[DATA_PTS][2];

bool waiting = true;

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
  as7341.setATIME(29);
  //as7341.setASTEP(599);
  as7341.setASTEP(299);
  as7341.setGain(AS7341_GAIN_512X);

  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  pinMode(IR_LED, OUTPUT);
  digitalWrite(IR_LED, LOW);
  //pinMode(4, OUTPUT);

  display.display();
  //delay(2000); // Pause for 2 seconds

  

}
void setLED(int led, int state){
  digitalWrite(led, state);
}
void printReadings(const uint16_t* readings){
  // print a few wavelength ranges
    Serial.print("630nm      : ");
    //Serial.println(1.00 - (float)readings[8] / (float)18000);
    Serial.println(readings[8]);
    Serial.print("ADC2/F8 680nm     : ");
    Serial.println(readings[9]);
    Serial.print("NIR        : ");
    //Serial.println(1.00 - (float)readings[11] / (float)18000);
    Serial.println(readings[11]);
}

void updateDisplay(double spo2,double rval, uint16_t red, uint16_t nir){
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 4);     // Start at top-left corner
  display.cp437(true);
  //display.print("Heart Rate: ");
  //display.println(hr);
  display.print("SpO2: ");
  display.println(spo2);
  display.print("RVal: ");
  display.println(rval);
  display.print("Light: ");
  display.print(red);
  display.print("/");
  display.println(nir);
  display.display();
}
void printDisplay(char line[]){
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 4);     // Start at top-left corner
  display.cp437(true);
  display.println(line);
  display.display();
}
void loop() {
  setLED(IR_LED,1);
  setLED(RED_LED,1);
  time = millis();
  // collect data set
  uint16_t ir_min = light_max;
  uint16_t ir_max = 0;
  uint16_t red_min = light_max;
  uint16_t red_max = 0;
  
  for (int i = 0; i < DATA_PTS; i++){
    
    // read ir led light levels
    setLED(IR_LED,1);
    delay(5);
    if (!as7341.readAllChannels(readings)){
      Serial.println("Error reading channels!");
      return;
    }
    setLED(IR_LED,0);
    data[i][1] = readings[11]; // NIR

    // read red led light levels
    setLED(RED_LED,1);
    delay(5);
    if (!as7341.readAllChannels(readings)){
      Serial.println("Error reading channels!");
      return;
    }
    setLED(RED_LED,0);
    data[i][0] = readings[8];  // 630 nm

    // check if no light was absorbed
    if (data[i][0] == light_max || data[i][1] == light_max){
      printDisplay("No finger detected");
      waiting = true;
      delay(100);
      return;
    }
    if (waiting){
      printDisplay("Collecting data...");
      waiting = false;
    }
    if(data[i][0] > red_max){
      red_max = data[i][0];
    }
    if(data[i][0] < red_min){
      red_min = data[i][0];
    }

    if(data[i][1] > ir_max){
      ir_max = data[i][1];
    }
    if(data[i][1] < ir_min){
      ir_min = data[i][1];
    }
    //updateDisplay(hr_avg, 1, data[i][0], data[i][1]);
    //delay(10);
  }
  
  time = millis() - time;
  Serial.print("Read Rate : ");
  double rps = (double)DATA_PTS / ((double)time/ (double)1000);
  Serial.println(rps);
  
  // analyze data
  uint16_t red_avg = 0;
  uint16_t ir_avg = 0;
  for (int i = 0; i < DATA_PTS; i++){
    red_avg += data[i][0] / DATA_PTS;
    ir_avg += data[i][1] / DATA_PTS;
  }

  Serial.print((float)red_min);
  Serial.print(" / ");
  Serial.print((float)red_max);
  Serial.print(" / ");
  Serial.print((float)ir_min);
  Serial.print(" / ");
  Serial.println((float)ir_max);
  float rval = ((float)red_min/(float)red_max) / ((float)ir_min/(float)ir_max);
  Serial.print("rval:");
  Serial.println(rval);
  float spo2 = rval * .7 * -.3333 + 1.27;

  Serial.print("spo2: ");
  Serial.println(spo2);
  updateDisplay(spo2, rval, red_avg, ir_avg);

}
