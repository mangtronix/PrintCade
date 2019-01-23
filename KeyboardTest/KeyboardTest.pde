// Test the keyboard to see what happens when a key is held
//
// One way to view the console output:
// - File -> Export Application
// - change to PrintCade directory
// - $ ./keyboardtest.sh

void draw(){
  // Nada
}

void keyPressed(){
  println("key="+key + "\t" + "keyCode=" + keyCode + "\tint(key)=" + int(key));
}
