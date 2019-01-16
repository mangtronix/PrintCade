// Send joystick characters over serial - emulates PrintCade hardware

void setup(){
  Serial.begin(9600);
}    

void loop(){
  Serial.write('L');
   delay(500);
  Serial.write('R');
  delay(500);
  Serial.write('B');
  delay(500);

  delay(1000);
}
