// RoverMAze.ino
// Use RoverC to attempt to solve a maze
// another attempt to push to github


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

#define TRIG_PIN 33       // Trig pin connected to GPIO 33 (Grove port)
#define ECHO_PIN 32       // Echo pin connected to GPIO 32 (Grove port)
#define MAX_DISTANCE 200  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

#define THREESTEP

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
  show_volts();

#ifdef THREESTEP
  three_step(17, 105, 165);
#else
  distance = sonar.ping_cm();
  update_sprite(distance, 20, 20);

#endif

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
  forwardDist = sonar.ping_cm();
  Serial.printf("forwardDist: %d\n", forwardDist);
  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(forwardDist, 20, 20);
    show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)

  roverc.setServoAngle(1, b);  // rotate the motor counter-clockwise
  forwardDist = sonar.ping_cm();
  Serial.printf("forwardDist: %d\n", forwardDist);

  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(forwardDist, 20, 20);
    show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)

  roverc.setServoAngle(1, c);  // rotate the motor counter-clockwise
  forwardDist = sonar.ping_cm();
  Serial.printf("forwardDist: %d\n", forwardDist);

  sprite.fillSprite(TFT_BLACK);  // Clear once per frame
  update_sprite(forwardDist, 20, 20);
    show_volts();

  sprite.pushSprite(0, 0);  // Once per frame only, raw the sprite covering the whole screen
  delay(3000);              // keep rotating for 5 seconds (5000 milliseconds)
}
