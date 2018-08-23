#include <LiquidCrystal.h>
#include <IRremote.h>

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);  //The ports plugged into LCD
const int irReceiverPin = 2;  //The port of the infrared receiver
int input = 0; //The integer input based on infrared
IRrecv irrecv(irReceiverPin);
decode_results results;

int currentPos = 0;
int row = 0;
int rowView = 0;
char paper[16][16];
int keyCodes[31] = {-1756, -2963, -24658, -25059, -4942, -4537, 11430, 11029, -434, -29, //ROW 1
                    -10324, -10725, -32414, -32009, -16042, -16443, 6034, 6439, -3856, //ROW 2
                    10796, 9589, 27168, 26767, 25286, 25691, 15396,                      //ROW 3
                    -26693, 14995, -6694, -6289
};
char keys[31] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', //ROW 1
                 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',       //ROW 2
                 'z', 'x', 'c', 'v', 'b', 'n', 'm',                 //ROW 3
                 ' ', ',', '.', '?'
};

//UP: 9678
//DOWN: 9277
//BACKSPACE: -4257

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn();
  lcd.begin(16,2);
}
void loop() {
   if (irrecv.decode(&results)) {
    Serial.print("IR_Code: ");
    Serial.println(results.value, HEX);
    input = results.value;
    Serial.println(input);
    irrecv.resume();

  }

  if (input == 0 || input == -1) {
    //Nothing
    //BACKSPACE
  } else if (input == -4257) {
    input = 0;
    //Remove number
    if (currentPos > 0) {
       paper[row][currentPos-1] = (char) 0;
       currentPos--;
       updateDisplay(row, currentPos);
    //Remove number and go up one row
    } else {
      if (row != 0) {
        row--;
        currentPos = 15;
        paper[row][currentPos] = (char) 0;
        updateDisplay(row, currentPos);
      }
    }
    //SCROLL DOWN
  } else if (input == 9277) {
    input = 0;
    if (rowView == row-1) {
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.display();
      rowView++;
      updateDisplay(rowView, currentPos);
    } else if (rowView < row) {
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.display();
      rowView++;
      updateDisplay(rowView, 16);
    }
   
    //SCROLL UP
  } else if (input == 9678) {
    input = 0;
    if (rowView >= 2) {
      rowView--;
      updateDisplay(rowView, 16);
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

void enterKey(char key) {
    if (currentPos == 16) {
      currentPos = 0;
      row++;
      rowView++;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.display();
    }
    paper[row][currentPos] = key;
    updateDisplay(row, currentPos);
    currentPos++;
}

void updateDisplay(int r, int pos) {
  //Updating display
    if (r == 0 && row == 0) {
      lcd.setCursor(0,0);
      for (int i = 0; i < pos+1; i ++) {
        lcd.print(paper[r][i]);
      }
      lcd.setCursor(0,1);  //Display currentPos
      lcd.display();
    } else {
      lcd.setCursor(0,0);
      for (int i = 0; i < 16; i ++) {
        lcd.print(paper[r-1][i]);
      }
      lcd.setCursor(0,1);
      for (int i = 0; i < pos+1; i ++) {
        lcd.print(paper[r][i]);
      }
      lcd.display();
    }
}

