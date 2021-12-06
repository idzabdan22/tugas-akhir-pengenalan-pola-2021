#include "HX711.h"
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

//Pin Define
#define LOADCELL_DOUT_PIN  2
#define LOADCELL_SCK_PIN  3
#define SERVO_ATAS 11
#define SERVO_BAWAH 12
#define S0_CS 6
#define S1_CS 7
#define S2_CS 8
#define S3_CS 9
#define OUT_CS 5

// Class object init
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo_bot, myServo_top;

//Calibration State
//Load Cell
float calibration_factor = 360;
//Color Sensor
int redMin = 76;
int greenMin = 86;
int blueMin = 55;
int redMax = 320;
int greenMax = 348;
int blueMax = 233;

//Variables for Color Pulse Width Measurements
int redPW = 0;
int greenPW = 0;
int bluePW = 0;

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

byte average = 0;
char output;

byte state = 0;
static unsigned long t_awal;
static unsigned long t_setelah;
static unsigned long treshold = 2000;

void send_data(int red, int green, int blue, int w){
  delay(1000);
  Serial.println('s');
  String data = "";
  data = (String(w) + "," + String(red) + "," + String(green) + "," + String(blue));
  delay(1000);
  Serial.println(data);
  Serial.flush();
}

void reset_all(){
  redValue = 0;
  greenValue = 0;
  blueValue = 0;
  myServo_bot.write(90);
  myServo_top.write(00);
}

void process_bottom_servo(char output){   
  delay(1000);
  (output == 'K') ? myServo_bot.write(90 - 45) : myServo_bot.write(90 + 45);
  delay(500);
}

void process_top_servo(){
  delay(1000);
  myServo_top.write(150);
  delay(500);
  myServo_top.write(00);
}

char receive_data(){
  String x;
  char jenis;
  while(!Serial.available());
  x = Serial.readString();
  (x == "S") ? jenis = 'S' : jenis = 'K';
  return jenis;
}

void readRGB(){
  for (int i = 0; i < 5; i++)
  {
    redPW = getRedPW();
    redValue = map(redPW, redMin, redMax, 255, 0);
    if(redValue < 0){
      redValue = 0;
    }
    delay(500);

    // Read Green Pulse Width
    greenPW = getGreenPW();
    greenValue = map(greenPW, greenMin, greenMax, 255, 0);
    if(greenValue < 0){
      greenValue = 0;
    }
    delay(500);

    // Read Blue Pulse Width
    bluePW = getBluePW();
    blueValue = map(bluePW, blueMin, blueMax, 255, 0);
    if(blueValue < 0){
      blueValue = 0;
    }
    delay(500);  

    redValue += redValue;
    greenValue += greenValue;
    blueValue += blueValue;
  }
  Serial.print("Red = ");
  Serial.print(redValue/5);
  Serial.print(" - Green = ");
  Serial.print(greenValue/5);
  Serial.print(" - Blue = ");
  Serial.println(blueValue/5);

  redValue /= 5;
  greenValue /= 5;
  blueValue /= 5;
}

int cekWeight(){
  int berat = 0;
  scale.set_scale(calibration_factor);
  berat = scale.get_units();
  return berat;
}

int readWeight(){
  average = 0;
  scale.set_scale(calibration_factor);
  for(int i = 0; i < 5; i++){
    average += scale.get_units();
  }
  average /= 5;
  return average;
}

void setup_LCD(){
  lcd.begin();
  lcd.backlight();
}

void setup_loadCell(){
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.println("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}

void setup_colorSensor(){
  // Set S0 - S3 as outputs
  pinMode(S0_CS, OUTPUT);
  pinMode(S1_CS, OUTPUT);
  pinMode(S2_CS, OUTPUT);
  pinMode(S3_CS, OUTPUT);
  // Set Pulse Width scaling to 20%
  digitalWrite(S0_CS,HIGH);
  digitalWrite(S1_CS,LOW);
  // Set Sensor output as input
  pinMode(OUT_CS, INPUT);
}

int getGreenPW() {
  // Set sensor to read Green only
  digitalWrite(S2_CS,HIGH);
  digitalWrite(S3_CS,HIGH);
  int PW;
  PW = pulseIn(OUT_CS, LOW);
  return PW;
}

int getBluePW() {
  // Set sensor to read Blue only
  digitalWrite(S2_CS,LOW);
  digitalWrite(S3_CS,HIGH);
  int PW;
  PW = pulseIn(OUT_CS, LOW);
  return PW;
}

int getRedPW() {
  // Set sensor to read Red only
  digitalWrite(S2_CS,LOW);
  digitalWrite(S3_CS,LOW);
  int PW;
  PW = pulseIn(OUT_CS, LOW);
  return PW;
}

void FSM(){
  t_setelah = millis() - t_awal;
  switch(state){
    case 0:
      Serial.println("Put object on top of load cell");
      if(cekWeight() > 2 && t_setelah > treshold * 3){
        Serial.println("Please dont move object while processing...");
        state = 1;
        t_awal = millis();   
      }
      break;

    case 1:      
      if(t_setelah > treshold && cekWeight() > 2){
        Serial.println("Begin reading weight...");
        Serial.print("Reading weight...");
        int w = readWeight();
        Serial.print("Weight: ");
        Serial.print(w);
        state = 2;
        t_awal = millis();
      }
      break;

    case 2:
      if(t_setelah > treshold && cekWeight() > 2){
        Serial.println("Begin reading color...");
        Serial.println("Reading color...\n");
        Serial.print("Color:\n");
        readRGB();
        state = 3;
        t_awal = millis();
      }
      break;

    case 3:
      if(t_setelah > treshold){ 
        Serial.println("Begin classification...");
        send_data(redValue, greenValue, blueValue, average);
        output = receive_data();
        state = 4;
        t_awal = millis();
      }

    case 4:
      if((t_setelah > treshold) && (output == 'K' || output == 'S')){
        Serial.print("Servo Bottom move...");
        process_bottom_servo(output);
        state = 5;
        t_awal = millis();
      }

    case 5:
      if(t_setelah > treshold){
        Serial.print("Servo Top move...");
        process_top_servo();
        state = 6;
        t_awal = millis();
      }

    case 6:
      if(t_setelah > treshold * 2){
        Serial.print("End of process...");
        if(cekWeight() < 3){
          reset_all();
          state = 0;
          t_awal = millis();
        }
      }
      break;
    }
  } 

void setup(){
  Serial.begin(9600);
  Serial.setTimeout(1);
  while(!Serial.available()) {
    Serial.println("waiting to start...");
    delay(1000);
  }
  Serial.println("Setting up...");
  delay(1000);
  setup_loadCell();
  Serial.println("Load Cell OK...");
  delay(1000);
  setup_colorSensor();
  Serial.println("Color Sensor OK...");
  delay(1000);
  setup_LCD();
  Serial.println("LCD OK...");
  t_awal = millis();
  myServo_bot.attach(12);
  myServo_top.attach(11);
  myServo_top.write(00);
  Serial.println("All set, Ready!");
}

void loop(){
  FSM();
}
