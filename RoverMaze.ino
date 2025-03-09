// MyRunningRoverC.ino Improvement on the example code
//roverc.setSpeed(int8_t x, int8_t y, int8_t z) (x,y and z axis)


/* pseudo code:
      while(1)
        find furthest wall
        rotate TOF towards wall
        calc stopDist
        move towards wall until stopDist

  crab_right();
  forward();
  crab_left();
  do180();
  // backward();
  stop();

  */


#include <M5StickCPlus.h>  // w/o the 'Plus', Lcd stuff is broken
#include <M5_RoverC.h>
#include <Wire.h>
#include "DFRobot_VL53L0X.h"
// Create an instance of the VL53L0X sensor
DFRobot_VL53L0X TOF;

#define TRIG_PIN 33  // Trig pin connected to GPIO 33 (Grove port)
#define ECHO_PIN 32  // Echo pin connected to GPIO 32 (Grove port)

// #define THREESTEP

int topspeed = 50;
int ms = 1000;
uint16_t fill_color;

M5_RoverC roverc;

// Specify sprite 160 x 128 pixels (needs 40Kbytes of RAM for 16 bit colour
TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);  // Create a sprite

int screenWidth = M5.Lcd.width();
int screenHeight = M5.Lcd.height();

int textSize = 3;

// Servo limits
const int minAngle = 15;
const int maxAngle = 160;
int servoAngle = minAngle;  // initialize this to minAngle!

//==================================================================
void setup() {
  M5.begin();
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  delay(500);

  // sprite.createSprite(screenHeight, screenWidth);  // Swap width & height for rotation
  sprite.createSprite(screenWidth, screenHeight);  // Create a full-screen sprite
  sprite.fillSprite(TFT_BLACK);                    // Fill the sprite with black
  sprite.setTextSize(textSize);
  sprite.setTextDatum(MC_DATUM);  // Center the text

  Serial.println("Initializing devices...");

  init_TOF();  // for looking aft Calling this breaks the servo at either 0 or 1!

  // Initialize RoverC
  roverc.begin(&Wire);  // Pass the Wire instance
}

//==================================================================
void update_sprite(int value, int x, int y) {
  char String_buffer[128];

  snprintf(String_buffer, sizeof(String_buffer), "%d\n", value);
  sprite.setTextColor(fill_color);
  sprite.setCursor(x, y);
  sprite.print(String_buffer);
}

//==================================================================
void loop() {

  sprite.fillSprite(TFT_BLACK);  // Clear once per frame

#ifdef THREESTEP
  three_step(17, 105, 165);

#else
  int fd = get_forwardDist();
  update_sprite(fd, 20, 20);

  int ad = get_aftDist();
  update_sprite(ad, 20, 60);

  show_volts();

#endif


  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
}


//==================================================================
int get_forwardDist() {
  int forwardDist = 0;
  long duration;
  float distance;

  // Send a 10µs pulse to trigger measurement
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pulse duration
  duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout 30ms

  // Convert to distance (cm)
  if (duration > 0) {
    distance = duration * 0.0343 / 2;
    // forwardDist = int(distance +0.5f);
    forwardDist = int(distance);
    Serial.printf("forwardDist: %d\n", forwardDist);
  } else {
    Serial.println("forwardDist out of range!");
  }

  delay(300);

  fill_color = YELLOW;
  return (forwardDist);
}

//-----------------------------------------------------------------------------------
int get_aftDist() {
  int aftDist = TOF.getDistance() / 10;
  Serial.printf("aftDist: %d cm\n", aftDist);
  delay(300);

  fill_color = GREEN;
  return (aftDist);
}

//-----------------------------------------------------------------------------------
void show_volts() {
  char String_buffer[128];
  float volts = M5.Axp.GetBatVoltage();

  if (volts <= 2.0) {
    fill_color = RED;
  } else if (volts >= 4.0) {
    fill_color = GREEN;
  } else {
    fill_color = YELLOW;
  }

  snprintf(String_buffer, sizeof(String_buffer), "%.2f V\n", volts);
  sprite.setTextColor(fill_color);
  sprite.setCursor(20, 180);
  sprite.print(String_buffer);
}

//==================================================================
void init_TOF() {  // aft sensor, edited to switch to DF_Robot_VL53L0X.h
  // this breaks the servo
  Serial.println("Initializing TOF VL53L0X using chassis connector...");
  Wire.begin(26, 0);  // SDA = 26,  SCL = 0 for the Grove connectors on RoverC chassis. Tested working if alone
  TOF.begin(0x50);    // Use 0x50 for aft sensor via chassis port. Works
  //Set to Back-to-back mode and high precision mode
  TOF.setMode(TOF.eContinuous, TOF.eHigh);
  //Laser rangefinder begins to work
  TOF.start();
  Serial.println("TOF VL53L0X initialized successfully");
}


//-----------------------------------------------------------------------------------
void display(int degrees) {
  // Clear the screen before displaying new number
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(60, 50);
  M5.Lcd.printf("%d", degrees);
}

//-----------------------------------------------------------------------------------

void crab_right() {
  roverc.setSpeed(topspeed, 0, 0);
  delay(ms);
}


void crab_left() {
  roverc.setSpeed(-topspeed, 0, 0);
  delay(ms);
}

void forward() {
  roverc.setSpeed(0, topspeed, 0);
  delay(ms);
}

void backward() {
  roverc.setSpeed(0, -topspeed, 0);
  delay(ms);
}

void do180() {
  roverc.setSpeed(0, 0, topspeed);
  delay(ms);
}

void stop() {
  roverc.setSpeed(0, 0, 0);
  char String_buffer[128];

  delay(ms);
}


void three_step(int a, int b, int c) {
  int servoAngle;
  int forwardDist;

  Serial.printf("in three_step() with %d %d %d\n", a, b, c);

  roverc.setServoAngle(1, a);  // rotate the motor counter-clockwise
  forwardDist = get_forwardDist();
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(forwardDist, 20, 20);
  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)

  roverc.setServoAngle(1, b);  // rotate the motor counter-clockwise
  forwardDist = get_forwardDist();
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(forwardDist, 20, 20);
  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)

  roverc.setServoAngle(1, c);  // rotate the motor counter-clockwise
  forwardDist = get_forwardDist();
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(forwardDist, 20, 20);
  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)
}
