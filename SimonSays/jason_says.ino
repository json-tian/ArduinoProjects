int base = 8;
int num = 3;
//Red-8 Green-9 Blue-10

#include <LiquidCrystal.h>
#include <IRremote.h>

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);  //The ports plugged into LCD
const int irReceiverPin = 2;  //The port of the infrared receiver
int input; //The integer input based on infrared
IRrecv irrecv(irReceiverPin);
decode_results results;

int level = 1;  //Current level
int moveNum = 0;  //number of moves the player made
int gameState = 0; //0-menu 1-memorizing 2-playing 3-ending
int pattern[100];  //Stores light keys, up to 99 levels or sequences

void setup()
{
  for (int i = base; i < base + num; i ++) {
    pinMode(i, OUTPUT); //set port ‘i’ as an output port
  }
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

  switch (gameState) {
      case 0:
        //START BUTTON
        if (input == -1756) {
          gameState = 1;
          level = 1;
          createLevel(level);
        } else {
           notStarted();
        }
        break;
      case 1:
        //Empty - do not receive user input  
        break;
      case 2:
        if (input == -2963)
          makeMove(8);
        else if (input == -24658)
          makeMove(9);
        else if (input == -25059)
          makeMove(10);
        break;
      case 3:
        break;
  }

  //START: -1756
  //RED: -2963
  //GREEN: -24658
  //BLUE: -25059
  delay(10);
}

void notStarted() {
    //Updating display
    lcd.setCursor(0,0);
    lcd.print("|| Jason Says ||");
    lcd.setCursor(0,1);  //Display position
    lcd.print("- Press Start! -");
    lcd.display();

    //Turning lights off one at a time
    for (int i = base; i < base + num; i ++) {
      digitalWrite(i, LOW);
      delay(200);
    }
    
    lcd.setCursor(0,1);  //Display position
    lcd.print("                ");
    lcd.display();

    //Turning lights on one at a time
    for (int i = base; i < base + num; i ++) {
      digitalWrite(i, HIGH);
      delay(200);
    }
}

void createLevel(int k) {
  turnLightsOff();

  //Updating display
  lcd.setCursor(0,0);
  lcd.print("-- Get Ready! --");
  lcd.setCursor(0,1);  //Display position
  lcd.print("--- Level ");
  lcd.print(k);
  lcd.print(" ---");
  lcd.display();

  delay(3000);

  //Add an extra pattern randomly
  randomSeed(analogRead(0));
  int temp = random(8, 11);
  pattern[level-1] = temp;

  //Replay the entire sequence for the player
  for (int j = 0; j < level; j ++) {
    digitalWrite(pattern[j], HIGH);
    delay(1000);
    digitalWrite(pattern[j], LOW);
    delay(200);

    //TESTING IN CONSOLE
    //Serial.println(pattern[j]);
  }

  gameState = 2;
  playLevel();
}

void playLevel() {
  //Updating display
  lcd.setCursor(0,0);
  lcd.print("-- Your Turn! --");
  lcd.setCursor(0,1);  //Display position
  lcd.print("----------------");
  lcd.display();
}

void makeMove(int k) {
  turnLightsOff();
  delay(50);
    //If the user gets it correct
    if (pattern[moveNum] == k) {
      digitalWrite(k, HIGH);
      moveNum++;
    } else {
      //Gameover, resets game
      gameState = 0;
      lcd.setCursor(0,0);
      lcd.print("--- You Lose ---");
      lcd.setCursor(0,1);  //Display position
      lcd.print("-- Try Again! --");
      lcd.display();
      moveNum = 0;
      delay(3000);
    }
    //If the player has finished the level
    if (level == moveNum) {
      gameState = 3;
      levelFinish();
    }
}

void levelFinish() {
  turnLightsOn();

  //Update display
  lcd.setCursor(0,0);
  lcd.print("Congratulations!");
  lcd.setCursor(0,1);
  lcd.print("-- Get Ready! --");
  lcd.display();

  level++;
  delay(3000);
  
  turnLightsOff();
  
  gameState = 1;
  moveNum = 0;
  createLevel(level);
}

//Turns the lights on
void turnLightsOn() {
  for (int i = base; i < base + num; i ++) {
      digitalWrite(i, HIGH);
  }
}

//Turns the lights off
void turnLightsOff() {
  for (int i = base; i < base + num; i ++) {
      digitalWrite(i, LOW);
  }
}
