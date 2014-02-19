
// include the library code:
#include <LiquidCrystal595.h>
#include <Keypad.h>

// datapin, latchpin, clockpin, num_lines
LiquidCrystal595 lcd(2,3,4);

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns

char keys[ROWS][COLS] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};

byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {11, 10, 9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() 
{
  Serial.begin(9600);
  
  lcd.begin(16,2);
  
  //keypad.addEventListener(keypadEvent);
}

void loop() 
{
    char key = keypad.getKey();

    if (key) {
        Serial.println(key);
        lcd.print(key);
    }
}

void keypadEvent(KeypadEvent key)
{
  lcd.setCursor(0,1);
  lcd.print(keypad.getKey());
  Serial.println(keypad.getKey());
}
