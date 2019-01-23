// Test the keyboard to see what happens when a key is held
//
// One way to view the console output:
// - File -> Export Application
// - change to KeyboardTest directory
// - $ ./keyboardtest.sh

// Observed behaviour on OSX:
// When a key is held
// - keyPressed is called immediately
// - short delay
// - keyPressed is called, then repeated at an interval
// - if a new key is pressed, it takes precedence
//   - keyPressed for the new key is called
//   - short delay
//   - keyPressed for the new key is repeated while held
// - when any key is released, keyReleased is called for that key

void draw(){
  // Nada
}

void keyPressed(){
  println("key="+key + "\t" + "keyCode=" + keyCode + "\tint(key)=" + int(key));
}

void keyReleased(){
  println("Released " + key + " keyCode=" + keyCode);
}
