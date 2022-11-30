#include <splash.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_I2CRegister.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>

#include <Adafruit_AS7341.h>

#define DATA_PTS    60
#define RED_LED     5
#define IR_LED      9
#define light_max   18000
#define NUM_RVALS   6

#define SCREEN_WIDTH   128 // OLED display width, in pixels
#define SCREEN_HEIGHT  32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AS7341 as7341;

unsigned long  time;
uint16_t       readings[12];
float          rvalues[NUM_RVALS];

int    rval_count      = 0;
bool   rval_count_full = false;

bool   waiting = true;

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
  as7341.enableLED(false);
  as7341.setATIME(29);
  as7341.setASTEP(599);
  //as7341.setASTEP(74);
  as7341.setGain(AS7341_GAIN_512X);

  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  pinMode(IR_LED, OUTPUT);
  digitalWrite(IR_LED, LOW);
  //pinMode(4, OUTPUT);

  //display.display();
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

void updateDisplay(double spo2,double rval, uint16_t red_min, uint16_t red_max, uint16_t nir_min, uint16_t nir_max){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 2);
  display.cp437(true);
  /*display.print("SpO2: ");*/
  /*display.println(spo2);*/
  display.print("RVal: ");
  display.println(rval);
  display.print("Red: ");
  display.print(red_max);
  display.print("/");
  display.println(red_min);
  display.print("NIR: ");
  display.print(nir_max);
  display.print("/");
  display.println(nir_min);
  display.display();
}
void printDisplay(char line[]){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 4);
  display.cp437(true);
  display.println(line);
  display.display();
}
void loop() {
  time = millis();
  // collect data set
  uint16_t ir_min = light_max;
  uint16_t ir_max = 0;
  uint16_t red_min = light_max;
  uint16_t red_max = 0;
  
  
  // turn on LED
  setLED(IR_LED,1);
  setLED(RED_LED,1);

  for (int i = 0; i < DATA_PTS; i++){
    // read ir led light levels
    if (!as7341.readAllChannels(readings)){
      Serial.println("Error reading channels!");
      return;
    }
    // check if no light was absorbed
    if (readings[AS7341_CHANNEL_630nm_F7] == light_max || readings[AS7341_CHANNEL_NIR] == light_max){
      printDisplay("No finger detected");
      waiting = true;
      rval_count_full = false;
      rval_count = 0;
      delay(1000);
      return;
    }
    // display collecting data if nothing else to display
    if (waiting){
      printDisplay("Collecting data...");
      waiting = false;
      
      return;
    }

    // set min/max for both wavelengths
    if(readings[AS7341_CHANNEL_630nm_F7] > red_max){
      red_max = readings[AS7341_CHANNEL_630nm_F7];
    }
    if(readings[AS7341_CHANNEL_630nm_F7] < red_min){
      red_min = readings[AS7341_CHANNEL_630nm_F7];
    }
    if(readings[AS7341_CHANNEL_NIR] > ir_max){
      ir_max = readings[AS7341_CHANNEL_NIR];
    }
    if(readings[AS7341_CHANNEL_NIR] < ir_min){
      ir_min = readings[AS7341_CHANNEL_NIR];
    }
  }
  // turn off LED
  setLED(IR_LED,0);
  setLED(RED_LED,0);

  // measure time elapsed since start of data collection
  time = millis() - time;
  // output read rate in reads per second
  Serial.print("Read Rate : ");
  float rps = (float)DATA_PTS / ((float)time/ (float)1000);
  Serial.println(rps);

  // analyze data
  /*uint16_t red_avg = 0;*/
  /*uint16_t ir_avg = 0;*/
  /*for (int i = 0; i < DATA_PTS; i++){*/
    /*red_avg += readings[AS7341_CHANNEL_630nm_F7] / DATA_PTS;*/
    /*ir_avg += readings[AS7341_CHANNEL_NIR] / DATA_PTS;*/
  /*}*/

  

  float a_red_max = ((float)light_max / (float)red_max);
  float a_red_min = ((float)light_max / (float)red_min);
  float a_ir_max  = ((float)light_max / (float)ir_max);
  float a_ir_min  = ((float)light_max / (float)ir_min);
  //float rval = log(a_red_max/a_red_min) / log(a_ir_max/a_ir_min);
  // calc r value
  float rval = ((float)red_min/(float)red_max) / ((float)ir_min/(float)ir_max);
  
  rvalues[rval_count] = rval;
  rval_count = (rval_count + 1) % NUM_RVALS;
  if (rval_count == 0){
    rval_count_full = true;
  }

  // calculate average r value
  float rval_avg = 0;
  if (rval_count_full){
    for (int i = 0; i < NUM_RVALS; i++){
      rval_avg += rvalues[i];
    }
    rval_avg /= NUM_RVALS;
  }
  else {
    for (int i = 0; i < rval_count; i++){
      rval_avg += rvalues[i];
    }
    rval_avg /= rval_count;
  }
  // convert to value that is easier to read
  rval_avg = (1.0 - rval_avg) * 100;
  //rval_avg = rval_avg * 100;
  
  // output r value components
  Serial.print("(");
  Serial.print((float)a_red_min);
  Serial.print(" / ");
  Serial.print((float)a_red_max);
  Serial.print(") / (");
  Serial.print((float)a_ir_min);
  Serial.print(" / ");
  Serial.print((float)a_ir_max);
  Serial.println(")");
  Serial.print("rval:");
  Serial.println(rval_avg);

  // calc spo2 from rval
  //float spo2 = rval * .7 * -.3333 + 1.27;
  //Serial.print("spo2: ");
  //Serial.println(spo2);
  updateDisplay(0, rval_avg, red_min, red_max, ir_min, ir_max);

}
