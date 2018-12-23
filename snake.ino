#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define VRX_PIN A1
#define VRY_PIN A2
#define SW_PIN 13
#define EEPROM_RESET_PIN A0
#define DIN_PIN 12
#define CLK_PIN 11
#define CS_PIN 10
#define V0_PIN 9
#define RS_PIN 2
#define EN_PIN 3
#define D4_PIN 4
#define D5_PIN 5
#define D6_PIN 6
#define D7_PIN 7

const int matrixAmount = 1;

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, matrixAmount);
LiquidCrystal lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

int xAxis = 0;
int yAxis = 0;
bool pressedButton = 1; // Joystick's button is wired with a pullup resistor

int address = 0;
int addressIndex = 0;

String direction;

int snakeX[64]; 
int snakeY[64];

int speed = 300;
int speedRate = 1;

int snakeSize;

int foodX;
int foodY;

int score = 0;

bool inGame = 0;
bool ledState = 1;
 
unsigned long lastUpdate = 0;
unsigned long lastFoodBlink = 0;


void setup() {
  //Reseting the EEPROM's stored values if there is a resistor wired to A0 pin.
  /*for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.update(i, 0);
  }        */
  
  pinMode(VRX_PIN, INPUT);
  pinMode(VRY_PIN, INPUT);
  pinMode(SW_PIN, INPUT); 
   
  lc.shutdown(0, false);
  lc.setIntensity(0, 1); 
  lc.clearDisplay(0);
  startScreen();
    
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Press joystick");
  lcd.setCursor(0, 1);
  lcd.print("button to start!");
  pinMode(V0_PIN, OUTPUT);
  analogWrite(V0_PIN, 120);
    
  while (pressedButton == 1) {
    pressedButton = digitalRead(SW_PIN);
  }
  
  newGame();
}


void loop() {
  if (inGame) {
    if ((millis() - lastUpdate) >= speed){
      lastUpdate = millis(); 
         
      lc.clearDisplay(0); //Clear screen.

      lcd.setCursor(0, 0);
      lcd.print("Score: ");
      lcd.print(score);
      lcd.setCursor(0, 1);
      lcd.print("Speed rate: x");
      lcd.print(speedRate);
      
      xAxis = simple(analogRead(VRX_PIN));
      yAxis = simple(analogRead(VRY_PIN));
        
      if (yAxis > 5 && direction != "up") { 
        direction = "down";
      }
      if (yAxis < 3 && direction != "down") {
        direction = "up";
      }
      if (xAxis < 3 && direction != "left") {
        direction = "right";
      }
      if (xAxis > 5 && direction != "right") { 
        direction = "left";
      }
        
      moveSnake(direction);
        
      checkIfHitFood();
      checkIfHitSelf();
       
      drawSnake();
      drawFood();
        
    }
  }
}


void startScreen() {  
  bool newGame[8][85] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };
  
  for (int i = 0; i < 77; i++){
    for (int row = 0; row < 8; row++){
      for (int col = 0; col < 8; col++){
        lc.setLed(0, row, col, newGame[row][col+i]);
      }
    }
    delay(35);
  }
}


//This method returns a more simple value to work with when reading the analogs.
//Returns and int that is 0-8.
int simple(int num) {
  return (num * 9 / 1024);
}


//This method makes the snake move in the direction provided in the parameters.
void moveSnake(String dir) {
  for (int i=snakeSize-1; i>0; i--){ //Go through all the snake body parts (excluding the head), and set it to the previous position.
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1]; 
  }

  if(dir == "up") {
    if (snakeY[0] == 0) { //If snake tries to get out of bounds, teleport him on the opposite side.
      snakeY[0] = 7;
    }
    else {
      snakeY[0]--;
    }
  }
  else {
    if (dir == "down") {
      if (snakeY[0] == 7) {
        snakeY[0] = 0;
      }
      else {
        snakeY[0]++;
      }
    } 
    else {
      if (dir == "left") {
        if (snakeX[0] == 0) {
          snakeX[0] = 7;
        }
        else {
          snakeX[0]--;
        }
      } 
      else {
        if(dir == "right") {
          if(snakeX[0] == 7) {
           snakeX[0] = 0;
          } 
          else {
            snakeX[0]++;
          }
        }
      }
    }
  }
}


void drawSnake() {
  for (int i = 0; i < snakeSize; i++) {
    lc.setLed(0, snakeY[i], snakeX[i], true);
  }
}


void drawFood() {
  if((millis() - lastFoodBlink) >= 50){
    lastFoodBlink = millis();
    lc.setLed(0, foodY, foodX, ledState);
    ledState = 1-ledState;
  }
}
    

void newFood() {
  int newFoodX = random(0, 8);
  int newFoodY = random(0, 8);
  while(isSnake(newFoodX, newFoodY)){ //If the new food coordinates are in the snake, then keep trying until they're not.
    newFoodX = random(0, 8);
    newFoodY = random(0, 8);
  }
  lc.setLed(0, foodY, foodX, false);
  foodX = newFoodX;
  foodY = newFoodY;
}

 
void checkIfHitFood() {
  if(snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeSize++;
    score++;
    if(score%3 == 0 && score != 0) {
      speedRate++;
      speed = speed - 25; 
    }
    newFood(); //Create a new food at a new location.
  }
}


void checkIfHitSelf() {
  for(int i=1; i<snakeSize; i++){
    if(snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver(); //Call the gameOver() method.
    }
  }
}


//Checks if the given coordinates are part of the snake.
//Used when checking where to generate new food.
bool isSnake(int x, int y){
  for(int i=0; i<snakeSize-1; i++){
    if((x == snakeX[i]) && (y == snakeY[i])){
      return 1;
    }
  }
  return 0;
}


//This method sets all the variables for a new game.
void newGame() { 
    lcd.clear();
    pressedButton = 1;
    inGame = 1;
    score = 0;
    
    speedRate = 1;  
    speed = 300;  
    
    for(int i=0; i<64; i++) {
        snakeX[i] = -1;
        snakeY[i] = -1;
    }
  
    snakeX[0] = 4;
    snakeY[0] = 8;
    direction = "up";
    snakeSize = 1;
    newFood();
}


void gameOver() {
  inGame = 0; 
  bool gameOver[8][73] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}
  };
    
  for(int i = 0; i < 65; i++){
    for (int row = 0; row < 8; row++){
      for (int col = 0; col < 8; col++){
        lc.setLed(0, row, col, gameOver[row][col+i]);
      }
    }
    delay(35);
  }
   
  checkHighscore(score);
  printHighscore();
  while(pressedButton == 1) {
   pressedButton = digitalRead(SW_PIN);
  }
  
  newGame();
}


void checkHighscore(int score){
  int value;
  bool checked = 0;
  EEPROM.get(address, value);
  if (score > value && checked == 0) {
    value = score;
    checked = 1;
  }
  
  if (checked == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Highscore!!!!!!!!");
    lcd.setCursor(0,1);
    lcd.print(score);
    delay(3000);
  }
  
  EEPROM.update(address, (byte)value); 
  
}


void printHighscore() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Highscore:");
  lcd.setCursor(0,1);
  lcd.print("1.");
  lcd.print(EEPROM.read(address));
  delay(3000);
   
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Press joystick");
  lcd.setCursor(0, 1);
  lcd.print("button to start!");
}
