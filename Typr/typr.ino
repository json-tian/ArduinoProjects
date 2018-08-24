//Typr - An Arduino typewriter!
//Author: Jason Tian
//August 23, 2018

#include <LiquidCrystal.h>
#include <IRremote.h>

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);  //The ports plugged into LCD
const int irReceiverPin = 2;  //The port of the infrared receiver
IRrecv irrecv(irReceiverPin);
decode_results results;

int currentPos = 0; //Current position in row
int row = 0;  //Number of rows
int rowView = 0;  //Row that is currently being viewed

const int displayWidth = 16;
const int displayHeight = 50;

char paper[displayWidth][displayHeight]; //Holding the user's text (maximum 50 lines)

//Key codes (from infrared)
int keyCodes[31] = {-1756, -2963, -24658, -25059, -4942, -4537, 11430, 11029, -434, -29,  //ROW 1
                    -10324, -10725, -32414, -32009, -16042, -16443, 6034, 6439, -3856,    //ROW 2
                    10796, 9589, 27168, 26767, 25286, 25691, 15396,                       //ROW 3
                    -26693, 14995, -6694, -6289                     
};

//Initializing keys
char keys[31] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',  //ROW 1
                 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',       //ROW 2
                 'z', 'x', 'c', 'v', 'b', 'n', 'm',                 //ROW 3
                 ' ', ',', '.', '?'
};

//Constants of infrared input
const int up = 9678;
const int down = 9277;
const int backspace = -4257;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  lcd.begin(16,2);

  for (int i = 0; i < displayWidth; i ++) {
    for (int j = 0; j < displayHeight; j ++) {
      paper[i][j] = ' ';
    }
  }
  
  //Intro screen
  lcd.setCursor(0,0);
  lcd.print("----  Typr  ----");
  lcd.setCursor(0,1);
  lcd.print(" By: Jason Tian ");
  lcd.display();
}

//Main Program Loop
void loop() {
  int input = -1; //The integer input based on infrared
  
  //Receiving infrared input
  if (irrecv.decode(&results)) {
    Serial.print("IR_Code: ");
    Serial.println(results.value, HEX);
    input = results.value;
    Serial.println(input);
    irrecv.resume();
  }

  if (input == -1) {
    //Nothing
    //Filter out empty infrared presses
  
  //BACKSPACE
  } else if (input == backspace) {
    input = 0;
    
    //Remove last key
    if (currentPos > 0) {
       paper[row][currentPos-1] = ' ';
       currentPos--;
       updateDisplay(row, currentPos);
       
    //Remove last key and move to previous row
    } else {
      if (row != 0) {
        row--;
        currentPos = displayWidth-1;
        paper[row][currentPos] = ' ';
        updateDisplay(row, currentPos);
      }
    }
    
  //SCROLL DOWN
  } else if (input == down) {
    input = 0;
    if (rowView < row) {
      clearBottomDisplay();
      rowView++;
      updateDisplay(rowView, displayWidth);
    }
   
  //SCROLL UP
  } else if (input == up) {
    input = 0;
    if (rowView >= 2) {
      rowView--;
      updateDisplay(rowView, displayWidth);
    }
    
  } else {
    for (int i = 0; i < sizeof(keyCodes); i ++) {
      if (keyCodes[i] == input) {
        enterKey(keys[i]);
        break;
      }
    }
    input = 0;
  }
  delay(10);
}

//When a letter is pressed
void enterKey(char key) {
  if (currentPos == displayWidth) {
    currentPos = 0;
    row++;
    rowView++;
    clearBottomDisplay();
  }
  paper[row][currentPos] = key;
  updateDisplay(row, currentPos);
  currentPos++;
}

//Updating LCD
void updateDisplay(int r, int pos) {
  //If just one row, print current on the first line
  if (row == 0) {
    lcd.setCursor(0,0);
    for (int i = 0; i < displayWidth; i ++) {
      lcd.print(paper[r][i]);
    }
    lcd.display();

    clearBottomDisplay();

  //Otherwise, print current on second line
  } else {
    //Printing first line
    lcd.setCursor(0,0);
    for (int i = 0; i < displayWidth; i ++) {
      lcd.print(paper[r-1][i]);
    }
    //Printing second line
    lcd.setCursor(0,1);
    for (int i = 0; i < displayWidth; i ++) {
      lcd.print(paper[r][i]);
    }
    lcd.display();
  }
}

//Clearing the second line display
void clearBottomDisplay() {
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.display();
}
