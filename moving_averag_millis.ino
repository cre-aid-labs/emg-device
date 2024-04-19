#define NUM_SAMPLES 10  // Number of samples to consider for moving average
#define EMG_PIN A2
#define ENC_A 3
#define ENC_B 4
#define ENC_SW 2
#define CONV_FACTOR 0.537
//int samples[NUM_SAMPLES];  // Array to store sample values


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128    // OLED display width in pixels
#define SCREEN_HEIGHT 64    // OLED display height in pixels

int currentIndex = 0;  // Current index in the samples array
float movingAverage = 0;  // Variable to store the moving average
long mytime;
long previousmillis = 0;
int i;
int stage = 0;
long encoder_counter = 0;
float baseline_sig = 0;
float max_sig = 0;
float threshold = 0;
bool toggle = false;

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float movAvg();
float toMicroVolts(float sig);
void baselineRecordStep();
void maxPowerRecordStep();
void displayData();

void setup() {

  Serial.begin(9600);
  display.begin(0x3C, true);  // Initialize with the I2C address of the OLED
  display.clearDisplay();     // Clear the buffer

  // You can uncomment the below line to rotate the display if needed
  // display.setRotation(2);

  attachInterrupt(digitalPinToInterrupt(ENC_A), encoderChAInterrupt, RISING);

  // Write text on the display
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 2);
     // Output the buffer to the OLED

}

void loop() {
  Serial.println(digitalRead(ENC_SW));
  switch(stage) {
    case 0:
      baselineRecordStep();
      break;
    case 1:
      maxPowerRecordStep();
      break;
    case 2:
      setThresholdStep();
      break;
    case 3:
      displayData();
      break;
  }
}

float movAvg() {
  float avg = 0;
  for(i = 0; i < NUM_SAMPLES; i++) {
    avg += (analogRead(EMG_PIN) / 1023.0);
  }
  return avg / NUM_SAMPLES;
}

float toMicroVolts(float sig) {
  return sig / CONV_FACTOR;
}

void baselineRecordStep() {
  display.clearDisplay();
  display.setCursor(0, 2);
  display.print("Relax your muscles");
  display.setCursor(20, 20);
  display.print("Voltage: ");
  float avg = movAvg();
  display.print(avg);
  display.print(" uV");
  display.display();
  if(digitalRead(ENC_SW) == LOW && toggle == false) {
    toggle = true;
    previousmillis = millis();
    baseline_sig = avg;
    stage = 1;
  }
}

void maxPowerRecordStep() {
  if(millis() - previousmillis > 1000) {
    toggle = false;
  }
  display.clearDisplay();
  display.setCursor(0, 2);
  display.print("Flex your muscles");
  display.setCursor(20, 20);
  display.print("Voltage: ");
  float avg = movAvg();
  display.print(avg);
  display.print(" uV");
  display.display();
  if(digitalRead(ENC_SW) == LOW && toggle == false) {
    toggle = true;
    previousmillis = millis();
    max_sig = avg;
    stage = 2;
  }
}

void setThresholdStep() {
  if(millis() - previousmillis > 1000) {
    toggle = false;
  }
  display.clearDisplay();
  display.setCursor(0, 2);
  display.print("Set threshold percentage: ");
  display.setCursor(20, 20);
  display.print("Threshold: ");
  float thr = 0.1 * (abs(encoder_counter) % 9) + 0.1;
  display.print(thr * 100.0);
  display.print(" %");
  display.display();
  if(digitalRead(ENC_SW) == LOW && toggle == false) {
    toggle = true;
    previousmillis = millis();
    threshold = thr;
    stage = 3;
  }
}

void displayData() {
  display.clearDisplay();
  display.setCursor(0, 2);
  display.print("Voltage: ");
  float avg = movAvg();
  display.print(avg);
  display.print(" uV");
  if(avg > (max_sig * threshold)) {
    display.setCursor(20, 20);
    display.print("THRESHOLD!");
  }
  display.display();
}

void encoderChAInterrupt() {
  if(digitalRead(ENC_B) == HIGH) {
    encoder_counter++;
  } else {
    encoder_counter--;
  }
}
