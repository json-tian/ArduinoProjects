/*
Project: Custom Fish Feeder
Author: Jason Tian
Date: August 26th, 2018
*/

#include <Servo.h>
#include <LiquidCrystal.h>
#include <IRremote.h>

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);  //The ports plugged into LCD
const int irReceiverPin = 2;  //The port of the infrared receiver
IRrecv irrecv(irReceiverPin);
decode_results results;

Servo myservo;
int pos = 0;  //Tracking servo position

//Tracking button presses
const int buttonPin = 7;
int buttonState = 0;
int lastButtonState = 0;

int keypad[10] = {-10244, -11451, 32390, 31989, 23312, 22105, -25852, -26253, -6382, -5977};  //IR codes for keypad numbers

//IR codes for each button
int addTime = -2170;
int removeTime = -1765;
int off = -12060;
int feedNow = 22908;
int setSystemTime = 31386;
int viewTimes = 31791;
int down = -18179;
int up = -17778;

String timeInput[4] = {"_", "_", "_", "_"}; //Strings for user input
int systemTime = 0; //The current system time (in minutes)
int feedingTime[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; //The set feeding times (in minutes)  -1 = N/A

int feederState = 0;  //0-menu  1-addtime  2-removetime 3-setSystemTime 4-off 5-ViewTimes
int viewRow = 0;  //Keeping track of which row is being viewed in viewtimes

//Tracking how long programming is running to update system time
unsigned long startTime = millis();
unsigned long interval = 60000;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  lcd.begin(16,2);

  //Initializing the button
  pinMode(buttonPin, INPUT);
  
  showMenu();

  //Initializing servo
  myservo.attach(13);
  myservo.write(pos);
  myservo.detach();
}
void loop() {
  int input = -1; //The integer input based on infrared
  buttonState = digitalRead(buttonPin);
  
  //Receiving infrared input
  if (irrecv.decode(&results)) {
    Serial.print("IR_Code: ");
    Serial.println(results.value, HEX);
    input = results.value;
    Serial.println(input);
    irrecv.resume();
  }

  //Feed if button is pressed
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      feed();
    }
  }
  lastButtonState = buttonState;
  
  if (input == feedNow) {
    feed();
  } else if (input == off) {
    feederState = 4;
    updateOffScreen();
  } else if (input == setSystemTime) {
    feederState = 3;
    emptyInput();
    showSelection("Set System Time");
  } else if (input == addTime) {
    feederState = 1;
    emptyInput();
    showSelection("Add Feeding Time");
  } else if (input == up && feederState == 5) { //Scrolling up when viewing feeding times
    if (viewRow > 0) {
      viewRow--;
      viewFeedingTimes(viewRow, viewRow+1);
    }
  } else if (input == down && feederState == 5) { //Scrolling down when viewing feeding times
    int temp = 0;
    for (int i = 0; i < 10; i ++) {
      if (feedingTime[i] == -1) {
        temp = i;
        break;
      }
    }
    if (viewRow + 2 != temp) {
      viewRow++;
      viewFeedingTimes(viewRow, viewRow+1);
    }
  } else if (input == viewTimes) {
    feederState = 5;
    viewFeedingTimes(0, 1);
  } else if (input == removeTime) {
    removeAll();
  }
  for (int i = 0; i < 10; i ++) { //Detecting key presses
    if (input == keypad[i]) {
      keyPress(i);
      break;
    }
  }
  if (millis() - startTime >= interval) { //Checking if a minute has gone past
    unsigned long temp = 60000;
    startTime += temp;
    Serial.println(startTime);
    Serial.println(millis());
    systemTime += 1;
    if (systemTime == 1440)
      systemTime = 1;
    if (feederState == 4)
      updateOffScreen();

    for (int i = 0; i < 10; i ++) { //Checking if system time matches feeding times
      if (feedingTime[i] == systemTime)
        feed();
    }
  } 
  input == -1;
}

void keyPress(int num) {
  //If setting system time
  if (feederState == 3) {
    int index = checkCurrentDigit();
    timeInput[index] = num;
    showSelection("Set System Time");
    if (index == 3) {
      setSystem();
      showMessage("System Time Set");
    }
  //If setting feeding times
  } else if (feederState == 1) {
    int index = checkCurrentDigit();
    timeInput[index] = num;
    showSelection("Add Feeding Time");
    if (index == 3) {
      setFeedingTime();
      showMessage("Feeding Time Set");
    }
  }
}

void setSystem() {
  //Setting new system time
  startTime = millis();
  systemTime = convertToMinutes();
}

void setFeedingTime() {
  //Setting new feeding time
  int temp = 0;
  for (int i = 0; i < 10; i ++) {
    if (feedingTime[i] == -1) {
      temp = i;
      break;
    }
  }
  feedingTime[temp] = convertToMinutes();
  
}

//Converts H:M time format to minutes
int convertToMinutes() {
  int temp = 0;
  temp += (timeInput[0].toInt() * 600);
  temp += (timeInput[1].toInt() * 60);
  temp += (timeInput[2].toInt() * 10);
  temp += timeInput[3].toInt();
  return temp;
}

//Checks the current digit that the user is going to input
int checkCurrentDigit() {
  for (int i = 0; i < 4; i ++) {
    if (timeInput[i] == "_") {
      return i; 
    }
  }
}

void showMenu() {
  //Show menu on LCD
  lcd.setCursor(0,0);
  lcd.print("-- FishFeeder --");
  lcd.setCursor(0,1);
  lcd.print(" By: Jason Tian ");
  lcd.display();
}

//Updates screen where user inputs
void showSelection(String message) {
  clearDisplay();
  lcd.setCursor(0,0);
  lcd.print(message);
  lcd.setCursor(0,1);
  lcd.print(timeInput[0]);
  lcd.print(timeInput[1]);
  lcd.print(":");
  lcd.print(timeInput[2]);
  lcd.print(timeInput[3]);
  lcd.display();
}

//Shows a completion message after setting system/feeding time
void showMessage(String message) {
  clearDisplay();
  lcd.setCursor(0,0);
  lcd.print(message);
  lcd.setCursor(0,1);
  lcd.print("Successfully!");
  lcd.display();
}

//Clearing the user inputs after finish/cancel
void emptyInput() {
  for (int i = 0; i < 4; i ++)
    timeInput[i] = "_";
}

//Clears Screen
void clearDisplay() {
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.display();
}

//Updates the system time screen (when the screen is off)
void updateOffScreen() {
  clearDisplay();

  //Converting system time to H:S
  int hours = systemTime / 60;
  int minutes = (systemTime - (hours * 60));
  
  //Updating LCD
  lcd.setCursor(0,0);
  lcd.print("System Time:    ");
  lcd.setCursor(0,1);
  if (hours < 10)
    lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10)
    lcd.print("0");
  lcd.print(minutes);
  lcd.display();
}

//Feeds the fish once
void feed() {
  myservo.attach(13);
  pos = 180;  //Spin to 180 degrees
  myservo.write(pos);
  delay(1000);
  
  //Slowly turn back to origin
  for (pos = 180; pos >= 0; pos --) {
    myservo.write(pos);
    delay(15);
  }
   myservo.detach();
}

//Removes all feeding times
void removeAll() {
  clearDisplay();
  for (int i = 0; i < 10; i ++) {
    feedingTime[i] = -1;
  }
  lcd.setCursor(0,0);
  lcd.print(" Feeding Times  ");
  lcd.setCursor(0,1);
  lcd.print("    Removed.    ");
  lcd.display();
}

//Displays an interactive list of feeding times that were set
void viewFeedingTimes(int one, int two) {
  clearDisplay();

  //If both feeding times are empty
  if (feedingTime[one] && feedingTime[two] == -1) {
    lcd.setCursor(0,0);
    lcd.print("NONE");
  } else {
    //Update the first row
    lcd.setCursor(0,0);
    convertToHm(one);
  }
  //If second time given exists, display on second row
  if (feedingTime[two] != -1) {
    lcd.setCursor(0,1);
    convertToHm(two);
  }
  lcd.display();
}

//Converts feedingTime minutes to H:M format and displays on LCD
void convertToHm(int index) {
  int hours = feedingTime[index] / 60;
  int minutes = feedingTime[index] - (hours * 60);
  if (hours < 10)
    lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10)
    lcd.print("0");
  lcd.print(minutes);
}

