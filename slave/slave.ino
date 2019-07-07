#include <Adafruit_NeoPixel.h>
#define NEOPIN  6  // Pin6 Neopixel out
#define COLORBUTTON_P1 4  // Pin4 P1 color change
#define COLORBUTTON_P2 5  // Pin5 P2 color change
#define NUM_LEDS 72
#define SNAKESPEED 30000 //speed of assistance

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type:
//   NEO_GRB  Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB  Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIN, NEO_GRB);

//Different colors  - G R B
uint32_t colorP1 = strip.Color(20, 255, 40); // Change RGB color value here
uint32_t colorP2 = strip.Color(255, 10, 50); // Change RGB color value here
//state 0pregame 1game 2postgame
unsigned int p1, p2, p1Moves, p2Moves, val1, val2, snkassist, state;

// Array of pixels in order of animation - 72 in total:
int sine[] = {
7, 8, 9, 10, 11, 12, 13, 14, 15, 16,17, 18,
30, 29, 28, 27, 26, 25, 24, 47, 46, 45, 44, 43,
55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
67, 68, 69, 70, 71, 48, 49, 50, 51, 52, 53, 54,
42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 
19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6
};

//Set variables to initial values
void varInit() {
  state = 0; //pregame
  strip.setBrightness(40); // 40/255 brightness (about 15%)
  int i;
  for(i=0; i < 72; i++) {
    strip.setPixelColor(sine[(i) % 72], 0); // Clear path
    strip.show();
    delay(5);
  }
  //variables initial values
  p1=0; //position of p1
  p2=36; //position of p2
  val1 = 0;
  val2 = 0;
  p1Moves = 26;
  p2Moves = 26;
  snkassist = 0;
}

//Setup interrupts and button functionalities, start ring LEDs
void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels
  attachInterrupt(0, moveP1, RISING); //pin2 P1 move
  attachInterrupt(1,moveP2,RISING); //pin3 P2 move
  pinMode(COLORBUTTON_P1, INPUT);
  pinMode(COLORBUTTON_P2, INPUT);
  varInit();
}

//Loop
void loop() {
  if(state==0) { //pregame
    val1 = digitalRead(COLORBUTTON_P1);
    val2 = digitalRead(COLORBUTTON_P2);
    if(val1 == 1) {
      setP1Color();
    }
    if(val2 == 1) {
      setP2Color();
    }
    int i=0;
    for(i=0; i < 10; i++) { //display snakes
    strip.setPixelColor(sine[(p1-i+72) % 72], colorP1); // Draw 'head' pixel
    strip.setPixelColor(sine[(p2-i+72) % 72], colorP2); // Draw 'head' pixel
    strip.show();
    delay(10);
    }   
  }
  else if(state==1) { //game
    snkassist++;
    if(snkassist==SNAKESPEED) {
      if(p1Moves<p2Moves) {
        moveP1();
        moveP2();
      }
      else {
        moveP2();
        moveP1();
      }
      snkassist=0;
    }
  }
}

void setP1Color() {
  colorP1 = strip.Color(random(0, 255), random(0, 255), random(0, 255));
}
void setP2Color() {
  colorP2 = strip.Color(random(0, 255), random(0, 255), random(0, 255));  
}

void EndGame(int player) {
  state=2;
  int i=0, j=0;
  if(player == 1) { // player1 won
    for(i=0; i < 72; i++) {
      strip.setPixelColor(sine[(p1+i) % 72], colorP1); // Draw 'head' pixel
      strip.show();
      delay(10);
    }
    for(i=50; i < 30; i--) { 
      strip.setBrightness(i);
      strip.show();
      delay(50);
    }
    for(i=30; i < 50; i++) { 
      strip.setBrightness(i);
      strip.show();
      delay(50);
    }
    for(j=0; j<30; j++) {
      for(i=0; i < 72; i=i+2) {
        strip.setPixelColor(sine[(i) % 72], colorP1); // Draw 'head' pixel
        strip.setPixelColor(sine[(i+1) % 72], 0); // Draw 'head' pixel
      }
      strip.show();
      delay(30+(j*10));
      for(i=0; i < 72; i=i+2) {
        strip.setPixelColor(sine[(i+1) % 72], colorP1); // Draw 'head' pixel
        strip.setPixelColor(sine[(i) % 72], 0); // Draw 'head' pixel
      }
      strip.show();
      delay(30+(j*10));
    }
  }
  else {  // player2 won
    for(i=0; i < 72; i++) {
      strip.setPixelColor(sine[(p2+i) % 72], colorP2); // Draw 'head' pixel
      strip.show();
      delay(10);
    }
    for(i=50; i < 30; i--) { 
      strip.setBrightness(i);
      strip.show();
      delay(50);
    }
    for(i=30; i < 50; i++) { 
      strip.setBrightness(i);
      strip.show();
      delay(50);
    }
    for(j=0; j<30; j++) {
      for(i=0; i < 72; i=i+2) {
        strip.setPixelColor(sine[(i) % 72], colorP2); // Draw 'head' pixel
        strip.setPixelColor(sine[(i+1) % 72], 0); // Draw 'head' pixel
      }
      strip.show();
      delay(30+(j*10));
      for(i=0; i < 72; i=i+2) {
        strip.setPixelColor(sine[(i+1) % 72], colorP2); // Draw 'head' pixel
        strip.setPixelColor(sine[(i) % 72], 0); // Draw 'head' pixel
      }
      strip.show();
      delay(30+(j*10));
    } 
  }
  state=0;
  varInit();
}

void moveP1() {
  state=1;
  p1Moves++;
  p2Moves--;
  p1 = (p1+1) % 72;
  strip.setPixelColor(sine[p1 % 72], colorP1); // Draw 'head' pixel
  strip.setPixelColor(sine[(p1+62) % 72], 0); //clear tail
  strip.show();
  if(p2Moves == 0) {
    state=2;
    EndGame(1);
  }
}
void moveP2() {
  state=1;
  p2Moves++;
  p1Moves--;
  p2 = (p2+1) % 72; 
  strip.setPixelColor(sine[p2 % 72], colorP2); // Draw 'head' pixel
  strip.setPixelColor(sine[(p2+62) % 72], 0); //clear tail
  strip.show();
  if(p1Moves == 0) {
    state=2;
    EndGame(2);
  }
}
