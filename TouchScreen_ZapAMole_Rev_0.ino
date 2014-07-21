/*
 Title: Zap-a-Mole Extreme for Touch Screen
 Author: Richard Kirkpatrick
 Date: 8 June 2014
 Description:
 Uses the Seeed Studio TFT Touch Screen (Version 1) with Arduino Uno to play the Zap-A-Mole game.
 Revision: 
  - @date 14 June 2014: 
    - Added high score functionality
    - Fixed bug with score not displaying for every time update
 
 Future Upgrades:
   - Make the Zap A Mole game into a class
   - Add a start up screen
   - Add more difficulty
*/

#include <stdint.h>
#include <TouchScreen.h> 
#include <TFT.h>
#include <TouchScreenGeometry.h> // Library for drawing shapes for the keypad
#include <TouchScreenStrings.h> // Library for drawing strings for the keypad
#include <TouchScreenButtons.h> // Library for drawing buttons for the keypad

#ifdef SEEEDUINO
  #define YP A2   // must be an analog pin, use "An" notation!
  #define XM A1   // must be an analog pin, use "An" notation!
  #define YM 14   // can be a digital pin, this is A0
  #define XP 17   // can be a digital pin, this is A3 
#endif

#ifdef MEGA
  #define YP A2   // must be an analog pin, use "An" notation!
  #define XM A1   // must be an analog pin, use "An" notation!
  #define YM 54   // can be a digital pin, this is A0
  #define XP 57   // can be a digital pin, this is A3 
#endif 

//Measured ADC values for (0,0) and (210-1,320-1)
//TS_MINX corresponds to ADC value when X = 0
//TS_MINY corresponds to ADC value when Y = 0
//TS_MAXX corresponds to ADC value when X = 240 -1
//TS_MAXY corresponds to ADC value when Y = 320 -1
#define TS_MINX 140
#define TS_MAXX 900

#define TS_MINY 120
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Pre-processor constants
#define heightButton 45
#define widthButton 45
#define widthTextBlock 98
#define heightTextBlock 35
#define widthReset 102
#define heightReset 20
#define widthStart 102
#define heightStart 20
#define font 1
#define noColumns 4
#define noRows 4
#define numButton 16

// Global constants
const int xminButton[] = {12, 69, 126, 183};  // x-min for buttons
const int xmaxButton[] = {57, 114, 171, 228}; // x-max for buttons
const int yminButton[] = {70, 125, 180, 235}; // y-min for buttons
const int ymaxButton[] = {115, 170, 225, 280}; // y-max for buttons
const int timeBlockCoord[] = {12, 110, 15, 50};     // xmin, xmax, ymin, ymax
const int scoreBlockCoord[] = {130, 228, 15, 50};  // xmin, xmax, ymin, ymax
const int resetButtonCoord[] = {126, 228, 290, 310};  // xmin, xmax, ymin, ymax
const int startButtonCoord[] = {12, 114, 290, 310}; // xmin, xmax, ymin, ymax

// Global instances 
Button button[16];
Button startButton;
Button resetButton;
TouchScreenString startText("START", 45, 297, font, WHITE);
TouchScreenString resetText("RESET", 160, 297, font, WHITE);

// Global variable
int highScore = 0;

void setup() 
{
  Tft.init();          // Initializes the TFT library
  screenSetup();
}

void loop() 
{
  // Intentionally left blank
}

// Draws the initial screen
void screenSetup() 
{   
  // Clear the screen
  eraseScreen();
  delay(250);
  
  // Set the coordinates sizes of the scoreblock, timeblock, start button, and reset button
  Rectangle scoreBlock(scoreBlockCoord[0], scoreBlockCoord[2], widthTextBlock, heightTextBlock);
  Rectangle timeBlock(timeBlockCoord[0], timeBlockCoord[2], widthTextBlock, heightTextBlock);
  startButton.setValues(startButtonCoord[0], resetButtonCoord[2], widthStart, heightStart);
  resetButton.setValues(resetButtonCoord[0], resetButtonCoord[2], widthReset, heightReset);
  
  // Set the text for the timeblock and score block
  TouchScreenString timeText("TIME", 45, 18, font, WHITE);
  TouchScreenString scoreText("SCORE", 158, 18, font, WHITE);

  // Sets the coordinates and sizes of the keypads
  for(int y = 0; y < 4; y++) {
    for(int x = 0; x < 4; x++) {
      button[noColumns * y + x].setValues(xminButton[x], yminButton[y], widthButton, heightButton);
    }
  }
  
  // Draws the time, score and reset blocks
  timeBlock.draw();
  scoreBlock.draw();
  startButton.draw();
  resetButton.draw();
   
  // Draws the text for the blocks
  timeText.drawText();
  scoreText.drawText();
  startText.drawText();
  resetText.drawText();
  
  // Draws the keypads
  for(int x = 0; x < 16; x++) 
    button[x].draw();
 
  // Setup the game
  gameSetup();
}

// Initial game setup
void gameSetup()
{
  // Initial game values
  int gameTime = 60;             // Total game time
  int score = 0;                  // Holds the player score
  int tempo = 1000;               // Delay between button flashes
  int counterTempo;               // Used for the tempo
  int startPressed;               // Checks to see if user has pressed start button
  int newButton;                  // Randomly chosen button
  int oldButton = -1;             // Previously chosen button
  int userButton = -1;            // User's selected button
  
  // Prompt user to press start button
  Tft.fillRectangle(65, 55, 220, 15, BLACK);
  Tft.drawString("PRESS START!", 70, 60, font, WHITE);
  while (startPressed != 100)
    startPressed = getButton();

  // Start the countdown
  Tft.fillRectangle(65, 55, 100, 15, BLACK);
  countDown();
  timeDisplay(gameTime);
  
  // Display the score
  scoreDisplay(score);
  
  // Provide a random seed for the button selection
  randomSeed(analogRead(0));
  
  // Loop until timer = 0 or user enters RESET
  while (gameTime != 0 && userButton != 200) {  
    
    // Choose a random button
    newButton = chooseButton(oldButton, newButton); 
    // Store the previous randomly chosen button
    oldButton = newButton; 
    // Highlight the randomly chosen button
    button[newButton].setFillColor(GREEN);
    button[newButton].fill();
    // While the tempo counter != tempo
    counterTempo = 0;
    while (counterTempo < tempo) {
      // Get the the user input
      userButton = getButton();
      // If the user input = randomly chosen button 
      if (userButton == newButton) {
        // Display the score
        scoreDisplay(score);
        // Increment the score
        score++;
         // Un-highlight the chosen button
        button[newButton].setFillColor(BLACK);
        button[newButton].fill();
        button[newButton].draw();
        // Choose a random button
        newButton = chooseButton(oldButton, newButton); 
        // Store the previous randomly chosen button
        oldButton = newButton; 
        // Highlight the randomly chosen button
        button[newButton].setFillColor(GREEN);
        button[newButton].fill();
      }
      // Increment the counter
      counterTempo += 50;
      delay(50);
    }
    // Update the timer
    gameTime--;
    timeDisplay(gameTime);
    // Un-highlight the chosen button
    button[newButton].setFillColor(BLACK);
    button[newButton].fill();
    button[newButton].draw();
  }
  // Game over!
  if (score > highScore)
    highScore = score;
  gameOver();
}

// Determines which button was pressed
int getButton() 
{
  // A point objects holds x, y, and z coordinates
  Point p = ts.getPoint(); //
  p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
  p.y = map(p.y, TS_MINY,TS_MAXY, 320, 0);
  
  int buttonNumber = -1; // Resets the button value
  int i = -1; int j = -1; // Used for buttonDisplay().  Resets the values for each loop.
  
  // Gets the button number.
  if (p.z > ts.pressureThreshhold) {
    for(int y = 0; y < noRows; y++) {
      for(int x = 0; x < noColumns; x++) {
        if ((p.x > xminButton[x] && p.x < xmaxButton[x]) && (p.y > yminButton[y] && p.y < ymaxButton[y])) 
          buttonNumber = noColumns * y + x;
       else if ((p.x > resetButtonCoord[0] && p.x < resetButtonCoord[1]) 
                && (p.y > resetButtonCoord[2] && p.y < resetButtonCoord[3])) { 
          resetGame(); 
          return 200;
       } else if ((p.x > startButtonCoord[0] && p.x < startButtonCoord[1]) 
                && (p.y > startButtonCoord[2] && p.y < startButtonCoord[3])) {
          startGame();
          return 100;
        }
      }
    }
  }
 
  // Displays the pressed button
  if (buttonNumber != -1) 
    button[buttonNumber].buttonDisplay(); 
  
  return buttonNumber;
}

// Resets the game
void resetGame() 
{
  resetButton.buttonDisplay();
  resetText.textButtonDisplay();
  screenSetup();
}

// Starts the game
void startGame() 
{
  startButton.buttonDisplay();
  startText.textButtonDisplay();
}

// Begins the countdown
void countDown() 
{
  Tft.fillRectangle(45, 30, 50, 20, BLACK);
  Tft.drawString("3", 55, 35, 2, WHITE);
  delay(1000);
  Tft.drawString("3", 55, 35, 2, BLACK);
  Tft.drawString("2", 55, 35, 2, WHITE);
  delay(1000);
  Tft.drawString("2", 55, 35, 2, BLACK);
  Tft.drawString("1", 55, 35, 2, WHITE);
  delay(1000);
  Tft.drawString("1", 55, 35, 2, BLACK);
  Tft.drawString("GO!", 50, 35, 2, WHITE);
  delay(1000);
  Tft.fillRectangle(45, 30, 50, 20, BLACK);
  delay(100);
}

// Game over!
void gameOver() 
{
  Tft.fillRectangle(65, 55, 220, 15, BLACK);
  Tft.drawString("GAME OVER!", 70, 60, font, WHITE);
  delay(1000);
  Tft.fillRectangle(65, 55, 220, 15, BLACK);
  Tft.drawString("High Score: ", 70, 60, font, WHITE);
  String str = String(highScore);
  char cstr[4];
  str.toCharArray(cstr, 4);
  Tft.drawString(cstr, 170, 60, font, WHITE);
  delay(3000);
  eraseScreen();
  screenSetup(); 
}

// Displays the time to the screen
void timeDisplay(int gameTime) 
{
  Tft.fillRectangle(45, 30, 50, 20, BLACK);
  String str = String(gameTime);
  char cstr[4];
  str.toCharArray(cstr, 4);
  Tft.drawString(cstr, 45, 35, 2, WHITE);
}

// Displays the score to the screen
void scoreDisplay(int score) 
{
  Tft.fillRectangle(158, 30, 50, 20, BLACK);
  String str = String(score);
  char cstr[4];
  str.toCharArray(cstr, 4);
  Tft.drawString(cstr, 158, 35, 2, WHITE);
}

// Chooses a random button to highlight
int chooseButton(int oldButton, int newButton) 
{
  while (oldButton == newButton) 
    newButton = random(0, 16);   
  return newButton;
}

// Erases the entire screen
void eraseScreen()
{
  Tft.fillRectangle(0, 0, 239, 319, BLACK);
}



