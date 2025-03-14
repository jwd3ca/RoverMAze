// RoverMAze.ino
// Use RoverC to attempt to solve a maze
// another attempt to push to github

// the turn_left() subroutine fails in this code

/* pseudo code:
      while(1)
        move forward until wall
        look left, look right, choose direction to go
        repeat
        
  */

#include <M5StickCPlus.h>  // w/o the 'Plus', Lcd stuff is broken
#include <NewPing.h>
#include <M5_RoverC.h>
#include <Wire.h>
#include <millisDelay.h>

#define TRIG_PIN 33       // Trig pin connected to GPIO 33 (Grove port)
#define ECHO_PIN 32       // Echo pin connected to GPIO 32 (Grove port)
#define MAX_DISTANCE 200  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

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
int distance = 0;
int fd = 0;
int ld = 0;
int rd = 0;

// Motor speed and turn duration
const int turnSpeed = 50;               // Adjust as necessary (range: -100 to 100)
const unsigned long turnDuration = 10;  // Duration in milliseconds (adjust based on empirical testing)

// have const here so it is easy to find and change
const unsigned long DELAY_TIME = 3000;  // in mS (3sec)
millisDelay MyDelay;                   // the delay object

const int left_angle = 35;
const int forward_angle = 110;
const int right_angle = 170;
const int stop_distance = 60;

//==================================================================
void update_sprite(int value, int x, int y) {
  char String_buffer[128];

  snprintf(String_buffer, sizeof(String_buffer), "%d\n", value);
  sprite.setTextColor(fill_color);
  sprite.setCursor(x, y);
  sprite.print(String_buffer);
}

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

  // Initialize RoverC
  roverc.begin(&Wire);  // Pass the Wire instance
  roverc.setServoAngle(1, forward_angle);
}

//==================================================================
void loop() {
  /* pseudo code:
      while(1)
        move forward until wall
        stop
        look left, look right, choose direction to go
        repeat
        
  */

  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  show_volts();

  // three_step(left_angle, forward_angle, right_angle);

  roverc.setServoAngle(1, forward_angle);

  while ((fd = look_forward()) > stop_distance) {
    forward();
  }

  stop();
  // Add a delay before the next action
  MyDelay.start(500);
  while (!MyDelay.justFinished()) {
  }

  rd = look_right();
  // Add a delay before the next action
  MyDelay.start(1000);
  while (!MyDelay.justFinished()) {
  }
  // Serial.println("3 seconds have elapsed!");

  ld = look_left();
  // Add a delay before the next action
  MyDelay.start(1000);
  while (!MyDelay.justFinished()) {
  }

  Serial.printf("rd = %d, ld = %d\n", rd, ld);

  if (rd < ld) { // righthand wall is closer, go left
    Serial.println("turning left");

    roverc.setSpeed(0, 0, -turnSpeed);  // left turn
    MyDelay.start(200);
    while (!MyDelay.justFinished()) {
      ;
    }
    roverc.setSpeed(0, 0, 0);  // stop
    // Add a delay before the next action
    MyDelay.start(2000);
    while (!MyDelay.justFinished()) {
      ;
    }

  } 
   if (rd > ld) { // lefthand wall is closer, go right
    
    Serial.println("turning right");
    roverc.setSpeed(0, 0, turnSpeed);  // right turn
    MyDelay.start(200);
    while (!MyDelay.justFinished()) {
      ;
    }
    roverc.setSpeed(0, 0, 0);  // stop

    // Add a delay before the next action
    MyDelay.start(2000);
    while (!MyDelay.justFinished()) {
      ;
    }
  }
  
  // update_sprite(distance, 20, 20);
  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
}

//-----------------------------------------------------------------------------------
void show_volts() {
  // loads the sprite but doesn't display it
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

//-----------------------------------------------------------------------------------
void turn_left() {
  // NOTE: the accuracy of the turn depends on turnSpeed, turnDuration and surface traction
  // 50 and 200 work well on my desk

  // Start the left turn
  roverc.setSpeed(0, 0, -turnSpeed);  // left turn
  MyDelay.start(200);
  while (!MyDelay.justFinished()) {
    ;
  }
  roverc.setSpeed(0, 0, 0);  // stop
}

//-----------------------------------------------------------------------------------
void turn_right() {
  // NOTE: the accuracy of the turn depends on turnSpeed, turnDuration and surface traction
  // 50 and 200 work well on my desk

  // Start the right turn
  roverc.setSpeed(0, 0, turnSpeed);  // right turn
  MyDelay.start(200);
  while (!MyDelay.justFinished()) {
    ;
  }
  roverc.setSpeed(0, 0, 0);  // stop
}

//-----------------------------------------------------------------------------------
void forward() {
  roverc.setSpeed(0, topspeed, 0);
}

//-----------------------------------------------------------------------------------
void backward() {
  roverc.setSpeed(0, -topspeed, 0);
}

//-----------------------------------------------------------------------------------
void do180() {
  roverc.setSpeed(0, 0, topspeed);
}

//-----------------------------------------------------------------------------------
void stop() {
  Serial.println("stopping");
  roverc.setSpeed(0, 0, 0);
}


//-----------------------------------------------------------------------------------
int look_left() {
  Serial.println("looking left()");

  roverc.setServoAngle(1, left_angle);  // rotate the motor counter-clockwise
  delay(1000);
  ld = sonar.ping_cm();
  Serial.printf("ld: %d\n", ld);
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(ld, 20, 20);
  show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  return (ld);
}

//------------------------------------------------------------
int look_right() {
  Serial.println("looking right()");

  roverc.setServoAngle(1, right_angle);  // rotate the motor counter-clockwise
  delay(1000);
  rd = sonar.ping_cm();
  Serial.printf("rd: %d\n", rd);
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(rd, 20, 20);
  show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  return (rd);
}

//------------------------------------------------------------
int look_forward() {
  Serial.println("\nin looking forward()");

  roverc.setServoAngle(1, forward_angle);  // rotate the motor counter-clockwise
  delay(1000);
  fd = sonar.ping_cm();
  Serial.printf("fd: %d\n", fd);
  
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(fd, 20, 20);
  show_volts();
  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(500);
  return (fd);
}

//------------------------------------------------------------
void three_step(int a, int b, int c) {
  int servoAngle;

  Serial.printf("in three_step() with %d %d %d\n", a, b, c);

  roverc.setServoAngle(1, a);  // rotate the motor counter-clockwise
  ld = sonar.ping_cm();
  Serial.printf("ld: %d\n", ld);
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(ld, 20, 20);
  show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)

  roverc.setServoAngle(1, b);  // rotate the motor counter-clockwise
  fd = sonar.ping_cm();
  Serial.printf("fd: %d\n", fd);

  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(fd, 20, 20);
  show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)

  roverc.setServoAngle(1, c);  // rotate the motor counter-clockwise
  rd = sonar.ping_cm();
  Serial.printf("rd: %d\n", rd);

  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(rd, 20, 20);
  show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)
}

