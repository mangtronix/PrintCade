// Read buttons and send over serial
//
// Sends e.g. R on right button down, then r when it comes up
//
// Michael Ang
// https://www.michaelang.com/project/printcade

const int BUTTON_PIN = A0;
const int RIGHT_PIN = A1;
const int LEFT_PIN = A2;

int firing = 0;

int buttonState, oldButtonState;
int leftState, oldLeftState;
int rightState, oldRightState;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

  buttonState = digitalRead(BUTTON_PIN);
  leftState = digitalRead(LEFT_PIN);
  rightState = digitalRead(RIGHT_PIN);

  oldButtonState = buttonState;
  oldLeftState = leftState;
  oldRightState = oldRightState;

  Serial.begin(9600); // set up Serial library at 9600 bps
}

void loop() {
  uint8_t i;

  buttonState = digitalRead(BUTTON_PIN);
  leftState = digitalRead(LEFT_PIN);
  rightState = digitalRead(RIGHT_PIN);

  if (buttonState != oldButtonState) {
    if (buttonState == LOW) {
      Serial.print("B");
      firing = 1;
    } else {
      Serial.print("b");
      firing = 0;
    }
    oldButtonState = buttonState;
  }

  if (leftState != oldLeftState) {
    if (leftState == LOW) {
      Serial.print("L");
    } else {
      Serial.print("l");
    }
    oldLeftState = leftState;
  }

  if (rightState != oldRightState) {
    if (rightState == LOW) {
      Serial.print("R");
    } else {
      Serial.print("r");
    }
    oldRightState = rightState;
  }
 
  delay(30); // This delay as well as the serial send time sets the minimum latency
}
