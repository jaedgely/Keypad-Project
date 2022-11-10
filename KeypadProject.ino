//Jack Edgely 11/10/2022
#include <Password.h>                                     //Password library
#include <LiquidCrystal.h>                                //LCD library - NOT LiquidCrystal_I2C.h!!!!!!!!!!!!!!!!!!
#include <Keypad.h>                                       //Keypad library

//These are variables that can be changed
Password password = Password("12345");                    //1, 2, 3, 4, 5, 6, 7, 8, 9, 0, A, B, C, D, *, # 
LiquidCrystal lcd = LiquidCrystal(8, 9, 10, 11, 12, 13);  //Pins which the LCD is connected
const byte doorOpen = A0;                                 //Pin which the door sensor is connected
const byte doorAlarm = A1;                                //Pin which the alarm LED is connected
const byte systemLED = A2;                                //Pin which the system active LED is connected
char passwordLength = 5;                                  //Password length variable - must be equal to length of password class. E.g. Password("1234") --> passwordLength
unsigned long alarmGracePeriod = 5000;                    //How long between alarm being tripped and alarm going off

//These are variables you shouldn't change
char alarmTripped = 0;                                    //Since system is disabled, alarmTripped should be disabled.
char systemEnable = 0;                                    //System starts disabled  - You could change this, but why would you?
char inputLength = 0;                                     //Input length starts at 0 - Please don't touch, otherwise 1st password input will always be incorrect due to auto-evaluate function
const byte numRows = 4;                                   //Keypad rows and columns
const byte numClms = 4;                                   //Only reason to change this is if your keypad is not a 4x4
unsigned long alarmTimer = 0;                             //Needed for an if-loop case, don't touch, otherwise alarm LED will never go off

char keymap[numRows][numClms] = {{'1', '2', '3', 'A'},    //Initializes the keymap with the same layout as the physical keypad
                                {'4', '5', '6', 'B'},
				{'7', '8', '9', 'C'},
				{'*', '0', '#', 'D'}};

byte rowPins[numRows] = {7, 6, 5, 4 };                    //Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins in this order
byte clmPins[numClms] = {3, A3, A4, A5 };                 //Connect keypad COL0, COL1, COL2 and COL3 to these Arduino pins in this order
 
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, clmPins, numRows, numClms); 

void setup(){ 
  pinMode(doorOpen, INPUT);                               //Starts led, as well as telling the system which pins will be input and output
  pinMode(doorAlarm, OUTPUT);                             
  pinMode(systemLED, OUTPUT);
  lcd.begin(16,2);                                        
  lcd.setCursor(0,0);
  lcd.print("Enter Passcode:");
  lcd.setCursor(0,1);
  Serial.begin(9600); 
} 
void loop(){ 
  char keyInput = myKeypad.getKey();                      
  if (keyInput != NO_KEY){                               
    inputLength = inputLength + 1;                        //Keypad input increments the input length by +1, needed for password auto - evaluation
    lcd.print(keyInput);
    password.append(keyInput);                            //System counts the key input towards the password
    delay(100);                                           //Allows user to see final input - otherwise the code is too quick, wipes result and displays SYSTEM ARMED or equivalent
  }                                                       //100ms is fast enough for human eye to register and still feel seamless.
  if (inputLength == passwordLength){                   
  checkPassword();                                      
  }
  while (systemEnable == 1){                              //While system is enabled
    if (alarmTripped == 1){                               //If the alarm is tripped
      if (millis() - alarmTimer >= alarmGracePeriod){     //If the time elapsed has exceeded the grace period
        alarmTimer = 0;                                   //Reset timer
        digitalWrite(doorAlarm, HIGH);                    //Turn on LED
        break;
      }
    }
    if (digitalRead(doorAlarm) == LOW){                   //If the alarm is not active
      if (digitalRead(doorOpen) == LOW){                  //AND if the door is closed
       return(0);                                         //This if loop seems useless, but without it, unless the door is open, there can be no user input
      }
      if (digitalRead(doorOpen) == HIGH){                 //OR if  the door is open
        alarmTripped = 1;                                 //Set the alarm as tripped
        if (alarmTimer == 0){                             //If the timer has not been started
          alarmTimer = millis();                          //Set equal to current time
        }          
        if (alarmTimer != 0){                             //If the alarmTimer has been set
          return(0);                                      //Move on - necessary because if the door is open the program doesnt accept inputs
        }
      }
    }
  }  
}
void checkPassword(){                                     
  if (password.evaluate()){                               //If password is correct...
    if (systemEnable == 0){                               //AND if system is NOT ENABLED
      if (digitalRead(doorOpen) == LOW){                  //AND if door is CLOSED --> Arm system (doorOpen == 1 means door is open)
        password.reset();                                 
        lcd.clear();                                  
        lcd.setCursor(0,0);                           
        lcd.print("SYSTEM ARMED");   
        digitalWrite(systemLED, HIGH);               
        delay(3000);
        lcd.setCursor(0,0);
        lcd.print("Enter Passcode:");                 
        lcd.setCursor(0,1);                           
        inputLength = 0;                                  //Set inputLength back to 0, otherwise the password auto-evaluate only works once
        systemEnable = 1;                                 //Turn on the alarm system
        return(0);
      }
      if (digitalRead(doorOpen) == HIGH){                 //OR if door is OPEN
        password.reset();                                 //Door has to be closed for the alarm to be set... stops auto-triggering the alarm
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("NOT SECURED");
        delay(3000);
        lcd.setCursor(0,0);
        lcd.print("Enter Passcode:");
        lcd.setCursor(0,1);
        inputLength = 0;
        systemEnable = 0;                                 //Maybe delete this? If the system is on this if loop shouldn't fire
        return(0);
      }
    } 
    if (systemEnable == 1){                               //OR if the system is ENABLED
      password.reset();                                  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SYSTEM DISARMED");
      digitalWrite(doorAlarm, LOW);
      digitalWrite(systemLED, LOW);
      delay(3000);
      lcd.setCursor(0,0);
      lcd.print("Enter Passcode:");
      lcd.setCursor(0,1);
      inputLength = 0;
      systemEnable = 0;
      alarmTripped = 0;                                   //Set the alarm as not tripped. Needed for the delay between doorOpen = 1 and doorAlarm becoming HIGH
      return(0);
      }
    }
  else{                                                   //If password is incorrect
    password.reset();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("INCORRECT");
    delay(3000);
    lcd.setCursor(0,0);
    lcd.print("Enter Passcode:");
    lcd.setCursor(0,1);
    inputLength = 0;
  } 
}
