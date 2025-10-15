#include "LedControl.h"
// led matrix pinouts
LedControl lc = LedControl(12, 10, 11, 1);

// joystick pinouts
// const int SW_pin = 2;  // digital pin connected to switch output
const int X_pin = 0;   // analog pin connected to X output
const int Y_pin = 1;   // analog pin connected to Y output

// define the LED digit patterns, from 0 - 9
// 1 = LED on, 0 = LED off, in this order:
//                74HC595 pin     Q0,Q1,Q2,Q3,Q4,Q5,Q6,Q7 
//                Mapping to      a,b,c,d,e,f,g of Seven-Segment LED
byte seven_seg_digits[11] = { B11111100,  // = 0
                              B01100000,  // = 1
                              B11011010,  // = 2
                              B11110010,  // = 3
                              B01100110,  // = 4
                              B10110110,  // = 5
                              B10111110,  // = 6
                              B11100000,  // = 7
                              B11111110,  // = 8
                              B11100110,  // = 9
                              0           // = null
                             };

// connect to the ST_CP of 74HC595 (pin 3,latch pin)
int latchPin = 3;
// connect to the SH_CP of 74HC595 (pin 4, clock pin)
int clockPin = 4;
// connect to the DS of 74HC595 (pin 2)
int dataPin = 2;

// mini XY Library
struct XY {
  int X;
  int Y;
};

XY XYPlus(XY xy1, XY xy2) {
  XY xy = { xy1.X + xy2.X, xy1.Y + xy2.Y };
  return xy;
}

XY XYPlusMod(XY xy1, XY xy2, XY mod) {
  XY xy = { (xy1.X + xy2.X + mod.X) % mod.X, (xy1.Y + xy2.Y + mod.Y) % mod.Y };
  return xy;
}

XY XYRand(XY range) {
  XY xy = { random(range.X-1), random(range.Y-1) };
  return xy;
}

int XYdot(XY xy1, XY xy2) {
  return xy1.X * xy2.X + xy1.Y * xy2.Y;
}

bool XYIsZero(XY xy) {
  return xy.X == 0 && xy.Y == 0;
}

bool XYEQ(XY xy1, XY xy2) {
  return xy1.X == xy2.X && xy1.Y == xy2.Y;
}

void XYprint(XY xy) {
  Serial.print("(");
  Serial.print(xy.X);
  Serial.print(",");
  Serial.print(xy.Y);
  Serial.print(")");
}

int joystick_threshold = 0x100;

XY BoardSize = { 8, 8 };

// global gamestate
const byte snakeMaxLen = 32;
byte snakeLen;
XY snake[snakeMaxLen];
byte snakeHeadPointer;
byte snakeTailPointer;
XY snakeDir;
XY applePos;

int score;
int gametickLength;

void initializeGame() {
  updateScore(0);
  snakeLen = 3;
  snake[0] = { 3, 1 };
  snake[1] = { 3, 2 };
  snake[2] = { 3, 3 };
  snakeHeadPointer = 2;
  snakeTailPointer = 0;
  snakeDir = { 0, 1 };

  applePos = XYRand(BoardSize);
}

void setup() {
  Serial.begin(9600);

  //setup 7-seg
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // setup display
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  initializeGame();
  displaySnake();

  Serial.println("Setup Complete.");
}

// display a number on the digital segment display
void sevenSegWrite(byte digit) {
  // set the latchPin to low potential, before sending data
  digitalWrite(latchPin, LOW);
     
  // the original data (bit pattern)
  shiftOut(dataPin, clockPin, LSBFIRST, seven_seg_digits[digit]);  
 
  // set the latchPin to high potential, after sending data
  digitalWrite(latchPin, HIGH);
}

XY pollMovementVec() {
  XY dPos = { 0, 0 };
  int xaxis = analogRead(X_pin) - 0x200;
  int yaxis = analogRead(Y_pin) - 0x200;

  if (abs(xaxis) > abs(yaxis) && abs(xaxis) > joystick_threshold) {
    dPos.X = (xaxis > 0) - (xaxis < 0);
  } else if (abs(xaxis) < abs(yaxis) && abs(yaxis) > joystick_threshold) {
    dPos.Y = (yaxis > 0) - (yaxis < 0);
  }

  return dPos;
}

void updateSnakeArr(bool lengthen = false) {
  XY head = snake[snakeHeadPointer];

  snakeHeadPointer += 1;
  snakeHeadPointer %= snakeMaxLen;
  snake[snakeHeadPointer] = XYPlusMod(head, snakeDir, BoardSize);

  if (!lengthen) {
    snakeTailPointer += 1;
    snakeTailPointer %= snakeMaxLen;
  } else {
    snakeLen += 1;
  }
}

void displaySnake() {
  for (int i = snakeTailPointer; i != (snakeHeadPointer + 1) % snakeMaxLen; i = (i + 1) % snakeMaxLen) {
    lc.setLed(0, snake[i].X, snake[i].Y, true);
  }
}

void displayApple(bool show = true) {
  lc.setLed(0, applePos.X, applePos.Y, show);
}

void printSnake() {
  Serial.println("Snake: ");
  Serial.print("tail: ");
  Serial.print(snakeTailPointer);
  Serial.print("; head: ");
  Serial.print(snakeHeadPointer);
  Serial.print("; [");
  for (int i = snakeTailPointer; i != (snakeHeadPointer + 1) % snakeMaxLen; i = (i + 1) % snakeMaxLen) {
    XYprint(snake[i]);
  }
  Serial.println("]");
}

int updateScore(int newspeed) {
  if (newspeed == 0) score = 0;
  sevenSegWrite(score % 10);
  gametickLength = 100 * exp(-score/10.0);
  Serial.print("Updated tickLength to");
  Serial.println(gametickLength);
}

bool applePosInSnake() {
  for (int i = snakeTailPointer; i != (snakeHeadPointer + 1) % snakeMaxLen; i = (i + 1) % snakeMaxLen) {
    if (XYEQ(snake[i], applePos)) return true;
  }
  return false;
}

int I = 0;
void loop() {
  // poll inputs every loop
  XY joystickInput = pollMovementVec();
  if (!XYIsZero(joystickInput) && XYdot(snakeDir, joystickInput) == 0) {
    snakeDir = joystickInput;
  }

  if (I % gametickLength == 0) { // only every gametick
    // game logic
    bool ateApple = false;
    bool dead = false;
    XY newHeadPos = XYPlusMod(snake[snakeHeadPointer], snakeDir, BoardSize);
    if (XYEQ(newHeadPos, applePos)) {
      // snake has eaten apple

      ateApple = true;
      score++;
      updateScore(score);

      // guarantee an apple outside of the snake
      applePos = XYRand(BoardSize);
      while (applePosInSnake()) {
        applePos = XYRand(BoardSize);
      }

      displayApple();
      delay(100);
      displayApple(false);
      delay(100);
    }

    for (int i = snakeTailPointer; i != (snakeHeadPointer + 1) % snakeMaxLen; i = (i + 1) % snakeMaxLen) {
      dead = dead || XYEQ(newHeadPos, snake[i]);
    }

    if (!dead) {
      updateSnakeArr(ateApple);
      // printSnake();
    }

    // rendering logic

    if (!dead) {
      lc.clearDisplay(0);
      displaySnake();
      displayApple();
    }
    else {
      for (int i = 0; i < 8; i++) {
        lc.setColumn(0, i, 0xFF);
      }
      for (int i = 0; i < 3; i++) {
        if (score > 10) {
          sevenSegWrite(score / 10);
          delay(500);
        }
        sevenSegWrite(score % 10);
        delay(500);
        sevenSegWrite(10); // Null
        delay(500);
      }

      delay(1000);
      initializeGame();
      displaySnake();
    }
  }

  delay(10);
  I++;
}
