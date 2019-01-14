// Main code by Michael Ang
// https://www.michaelang.com/project/printcade
//
// Hack hack stylez, was written during the 48 hour Art Hack Day at transmediale in Berlin

// We soldered to the switches in a retro joystick to read the joystick movement.
// The LED colours come from the graphics on the side of the joystick (classic 80s games).
// We use some Perlin noise to keep it fresh. The fire button on the joystick triggers
// flashes on the print head - we soldered to the existing ribbon cable running up to the
// print head. Haxx0r.

// Other code:
  // Adafruit Motor shield library
  // copyright Adafruit Industries LLC, 2009
  // this code is public domain, enjoy!

  // Perlin noise using the algorithm from http://freespace.virgin.net/hugo.elias/models/m_perlin.html
  // thanks to hugo elias


// Required libraries:
// Seems to require the OLD Adafruit Motor Library.
// Download the zip, then Sketch -> Include Library -> Add .ZIP library
// https://learn.adafruit.com/adafruit-motor-shield/library-install
// (I believe the motor shield we used was a gift from Phil Torrone back in the day, thanks!)
//
// TimerOne
// Sketch -> Include Library -> Manage Libraries -> search for 'TimerOne'

// Last tested with Arduino 1.8.5 and Arduino UNO
  
#include <AFMotor.h>

AF_DCMotor motor(3);
AF_DCMotor fan(2);

const int BUTTON_PIN = A0;
const int RIGHT_PIN = A1;
const int LEFT_PIN = A2;

int MOTOR_SPEED = 150;
int FAN_SPEED = 200;

int divisor = 3;

// Light strip code
#include <TimerOne.h>
int SDI = A4; //Red wire (not the red 5V wire!)
int CKI = A5; //Green wire>
int ticksPerShift = 10;
int ticksSinceShift = 10000;
int ticksPerRandom = 50;
int ticksSinceRandom = 10000;
#define STRIP_LENGTH 32 //32 LEDs on this strip
unsigned long strip_colors[STRIP_LENGTH];
int frameDelay = 10;
unsigned char RGB[3]; // current colour
unsigned long touchColors[5];
#include <math.h>

float x1,y1,x2,y2,x3,y3,persistence;
int octaves;

enum MODE {
  mode_perlin,
  mode_random,
  mode_single,
  mode_double,
  mode_static
};

MODE mode = mode_static;

int independence = 0;

// Colours
/*
Dig Dug Orange 214	1	1	
Medium blue 1	81	121	
Light blue 35	114	130	
Very light blue 147	147	127	
PacMan yellow  248	255	58		
Case yellow 253	244	2	
Road purple 25	5	41	
Off white 215	174	120	
Joystick red 210	0	0	
*/
const long unsigned int orange = 0x0178DC;
const long unsigned int medium_blue = 0x795101;
const long unsigned int light_blue = 0x827223;
const long unsigned int very_light_blue = 0x7F9393;
const long unsigned int yellow = 0x00F8F8;
const long unsigned int purple = 0x290519;
const long unsigned int off_white = 0x78AED7;
const long unsigned int red = 0x0000D2;

const long unsigned int colors[] = {
  orange,
  medium_blue,
  light_blue,
  very_light_blue,
  yellow,
  purple,
  red
};
int colors_max_index = 7;

int firing = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  
  // Serial.begin(9600);           // set up Serial library at 9600 bps
  // Serial.println("Motor test!");

  // turn on motor
  motor.setSpeed(MOTOR_SPEED);
  motor.run(RELEASE);
  
  fan.setSpeed(FAN_SPEED);
  fan.run(RELEASE);
  
  // Lights setup
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  
  //Clear out the array
  for(int x = 0 ; x < STRIP_LENGTH ; x++)
    strip_colors[x] = 0;
    
  randomSeed(analogRead(0));

  // Noise
  //persistence affects the degree to which the "finer" noise is seen
  persistence = 0.25;
  //octaves are the number of "layers" of noise that get computed
  octaves = 3;
  
  RGB[0] = 0;
  RGB[1] = 0;
  RGB[2] = 0;
  
  Timer1.initialize(1000); // microseconds
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
  
  //Pre-fill the color array with known values
  strip_colors[0] = orange;
  strip_colors[1] = medium_blue;
  strip_colors[2] = light_blue;
  strip_colors[3] = red;
  strip_colors[4] = yellow;
  strip_colors[5] = purple;
  post_frame(); //Push the current color frame to the strip
  
  mode = mode_random;
  
  // Serial.print("mang");
}

void loop() {
  uint8_t i;
  
  int buttonState = digitalRead(BUTTON_PIN);
  int leftState = digitalRead(LEFT_PIN);
  int rightState = digitalRead(RIGHT_PIN);
  
  if (buttonState == LOW) {
    // Serial.print("B");
    firing = 1;
    fan.run(FORWARD);
  } else {
    firing = 0;
    fan.run(RELEASE);
  }
  
  if (rightState == LOW) {
    // Serial.print("R");
    motor.run(BACKWARD);
    motor.setSpeed(MOTOR_SPEED);
  } else if (leftState == LOW) {
    // Serial.print("L");
    motor.run(FORWARD);
    motor.setSpeed(MOTOR_SPEED);
  } else {
    // Serial.print("S");
    motor.setSpeed(0);
    motor.run(RELEASE);
  }
  
  // Lights
    if (ticksSinceShift > ticksPerShift) {
      switch (mode) {
        case mode_random:
          if (ticksSinceRandom > ticksPerRandom) {
            addRandom();
            ticksSinceRandom = 0;
          }
          break;
          
        case mode_single:
          shiftColors();
          strip_colors[0] = touchColors[0];
          if (independence) {
            strip_colors[STRIP_LENGTH / 2] = touchColors[1];  
          }
          break;
          
        case mode_double:
          shiftColors();
          strip_colors[0] = touchColors[0];
          strip_colors[STRIP_LENGTH / 2] = touchColors[1];
          /*
          for (int i = 0; i < STRIP_LENGTH / 2; i++) {
            strip_colors[i] = touchColors[0];
            strip_colors[i + STRIP_LENGTH / 2] = touchColors[1];
          }
          */
          break;
          
        case mode_perlin:
          shiftColors();
          
          x1 = float(millis())/100.0f;
          y1 = 10.0f;
          x2 = float(millis())/100.0f;
          y2 = 11.0f;
          x3 = float(millis())/100.0f;
          y3 = 12.0f;
        
          //PerlinNoise2 results in a float between -1 and 1
          //below we convert to a n int between 0 and 255
          unsigned long color = int(PerlinNoise2(x1,y1,persistence,octaves)*128+128);
          color <<= 8;
          color += int(PerlinNoise2(x2,y2,persistence,octaves)*128+128);
          color <<= 8;
          color += int(PerlinNoise2(x3,y3,persistence,octaves)*128+128);
          strip_colors[0] = color;
          break; 
          
          /*
        case mode_static:
          // nada
          break; 
   */       

      }

      // XXX munge frame for button firing
      if (!firing) {
        strip_colors[STRIP_LENGTH - 1] = 0;
        strip_colors[STRIP_LENGTH - 2] = 0;  
      }

      post_frame(); //Push the current color frame to the strip
      ticksSinceShift = 0;
    }


  delay(100);
}
  
void oldloop() {
  uint8_t i;
  
  // Serial.print("tick");
  
  motor.run(FORWARD);
  for (i=0; i<255; i++) {
    motor.setSpeed(i);  
    delay(10);
 }
 
  for (i=255; i!=0; i--) {
    motor.setSpeed(i/divisor);  
    delay(10);
 }
  
  // Serial.print("tock");

  motor.run(BACKWARD);
  for (i=0; i<255; i++) {
    motor.setSpeed(i/divisor);  
    delay(10);
 }
 
  for (i=255; i!=0; i--) {
    motor.setSpeed(i/divisor);  
    delay(10);
 }
  

  // Serial.print("tech");
  motor.run(RELEASE);
  delay(1000);
}



// Lights
//Throws random colors down the strip array
void addRandom(void) {
  int x;
  
  //First, shuffle all the current colors down one spot on the strip
  for(x = (STRIP_LENGTH - 1) ; x > 0 ; x--)
    strip_colors[x] = strip_colors[x - 1];
    
  int index = random(colors_max_index);
  strip_colors[0] = colors[index];
    
  /*
  //Now form a new RGB color
  long new_color = 0;
  for(x = 0 ; x < 3 ; x++){
    new_color <<= 8;
    new_color |= random(0xFF); //Give me a number from 0 to 0xFF
    //new_color &= 0xFFFFF0; //Force the random number to just the upper brightness levels. It sort of works.
  }
  
  strip_colors[0] = new_color; //Add the new random color to the strip
  */
}

void shiftColors(void) {
  int x;
  
  //First, shuffle all the current colors down one spot on the strip
  for(x = (STRIP_LENGTH - 1) ; x > 0 ; x--) {
    strip_colors[x] = strip_colors[x - 1];
  }
}

//Takes the current strip color array and pushes it out
void post_frame (void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0 
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for 500us or more causes the IC to post the data.

  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    long this_led_color = strip_colors[LED_number]; //24 bits of color data

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)
      
      digitalWrite(CKI, LOW); //Only change data when clock is low
      
      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
      
      if(this_led_color & mask) 
        digitalWrite(SDI, HIGH);
      else
        digitalWrite(SDI, LOW);
  
      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKI, LOW);
  delayMicroseconds(500); //Wait for 500us to go into reset
}

char atoc(char* inputString) {
  return atoi((const char*)inputString);  
}

unsigned long colorFromRGB(unsigned char* aRGB) {
    unsigned long color = RGB[0];
    color <<= 8;
    color += RGB[1];
    color <<= 8;
    color += RGB[2];
    //color = RGB[2] + (RGB[1] << 8) + (RGB[0] << 16);
    return color;
}

unsigned long colorFromHex(char* hexString) {
    unsigned long color = 0;
    int temp;
    sscanf(&hexString[0], "%2X", &temp);
    color += temp;
    color <<= 8;
    sscanf(&hexString[2], "%2X", &temp);
    color += temp;
    color <<= 8;
    sscanf(&hexString[4], "%2X", &temp);
    color += temp;
    return color;
}

void timerIsr() {
  // Just increment
  ticksSinceShift++;
  
  ticksSinceRandom++;
  if (ticksSinceRandom > 5000) {
    ticksSinceRandom = 5000;  // clamp
  }
}

// Perlin noise

//using the algorithm from http://freespace.virgin.net/hugo.elias/models/m_perlin.html
// thanks to hugo elias
float Noise2(float x, float y)
{
  long noise;
  noise = x + y * 57;
  noise = pow(noise << 13,noise);
  return ( 1.0 - ( long(noise * (noise * noise * 15731L + 789221L) + 1376312589L) & 0x7fffffff) / 1073741824.0);
}

float SmoothNoise2(float x, float y)
{
  float corners, sides, center;
  corners = ( Noise2(x-1, y-1)+Noise2(x+1, y-1)+Noise2(x-1, y+1)+Noise2(x+1, y+1) ) / 16;
  sides   = ( Noise2(x-1, y)  +Noise2(x+1, y)  +Noise2(x, y-1)  +Noise2(x, y+1) ) /  8;
  center  =  Noise2(x, y) / 4;
  return (corners + sides + center);
}

float InterpolatedNoise2(float x, float y)
{
  float v1,v2,v3,v4,i1,i2,fractionX,fractionY;
  long longX,longY;

  longX = long(x);
  fractionX = x - longX;

  longY = long(y);
  fractionY = y - longY;

  v1 = SmoothNoise2(longX, longY);
  v2 = SmoothNoise2(longX + 1, longY);
  v3 = SmoothNoise2(longX, longY + 1);
  v4 = SmoothNoise2(longX + 1, longY + 1);

  i1 = Interpolate(v1 , v2 , fractionX);
  i2 = Interpolate(v3 , v4 , fractionX);

  return(Interpolate(i1 , i2 , fractionY));
}

float Interpolate(float a, float b, float x)
{
  //cosine interpolations
  return(CosineInterpolate(a, b, x));
}

float LinearInterpolate(float a, float b, float x)
{
  return(a*(1-x) + b*x);
}

float CosineInterpolate(float a, float b, float x)
{
  float ft = x * 3.1415927;
  float f = (1 - cos(ft)) * .5;

  return(a*(1-f) + b*f);
}

float PerlinNoise2(float x, float y, float persistance, int octaves)
{
  float frequency, amplitude;
  float total = 0.0;

  for (int i = 0; i <= octaves - 1; i++)
  {
    frequency = pow(2,i);
    amplitude = pow(persistence,i);

    total = total + InterpolatedNoise2(x * frequency, y * frequency) * amplitude;
  }

  return(total);
}
 
