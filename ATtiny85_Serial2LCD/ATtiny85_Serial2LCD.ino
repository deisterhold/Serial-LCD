//Import libraries that are being used
#include "LiquidCrystal595.h"  //LCD controlled with sift register
#include <SoftwareSerial.h>    //Control the lcd from a serial device
#include <EEPROM.h>            //Read and write settings that persist

// --- EEPROM ADDRESS DEFINITIONS
#define LCD_BACKLIGHT_ADDRESS 1  // EEPROM address for backlight setting
#define BAUD_ADDRESS 2  // EEPROM address for Baud rate setting
#define SPLASH_SCREEN_ADDRESS 3 // EEPROM address for splash screen on/off
#define ROWS_ADDRESS 4  // EEPROM address for number of rows
#define COLUMNS_ADDRESS 5  // EEPROM address for number of columns

// --- SPECIAL COMMAND DEFINITIONS
#define BACKLIGHT_COMMAND 128  // 0x80
#define SPECIAL_COMMAND 254 // 0xFE
#define BAUD_COMMAND 129  // 0x81

// --- ARDUINO PIN DEFINITIONS
uint8_t dataPin = 0;
uint8_t latchPin = 1;
uint8_t clockPin = 2;

char inKey;  // Character received from serial input
uint8_t Cursor = 0;  // Position of cursor, 0 is top left, (rows*columns)-1 is bottom right
uint8_t LCDOnOff = 1;  // 0 if LCD is off
uint8_t blinky = 0;  // Is 1 if blinky cursor is on
uint8_t underline = 0; // Is 1 if underline cursor is on
uint8_t splashScreenEnable = 1;  // 1 means splash screen is enabled
uint8_t rows = 2;  // Number rows, will be either 2 or 4
uint8_t columns = 16; // Number of columns, will be 16 or 20
uint8_t characters; // rows * columns

// initialize the LCD at pins defined above
LiquidCrystal595 lcd(dataPin, latchPin, clockPin);

// initialize the Serial port using the software serial library
SoftwareSerial input(3,4); //RX,TX

/* ----------------------------------------------------------
  In the setup() function, we'll read the previous baud, 
  screen size, backlight brightness, and splash screen state
  from EEPROM. Serial will be started at the proper baud, the
  LCD will be initialized, backlight turned on, and splash
  screen displayed (or not) according to the EEPROM states.
  ----------------------------------------------------------*/
void setup(){
  delay(200);
  // initialize the serial communications:
  setBaudRate(EEPROM.read(BAUD_ADDRESS));
  
  // Read rows and columns from EEPROM
  // Will default to 2x16, if not previously set
  rows = EEPROM.read(ROWS_ADDRESS);
  if (rows != 4)
    rows = 2;
  columns = EEPROM.read(COLUMNS_ADDRESS);
  if (columns != 20)
    columns = 16;
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(columns, rows);
  
  // Set up the backlight
  //EEPROM.write(LCD_BACKLIGHT_ADDRESS, true);
  //setBacklight(EEPROM.read(LCD_BACKLIGHT_ADDRESS));
  setBacklight(true);
  
  // Do splashscreen if set
  splashScreenEnable = EEPROM.read(SPLASH_SCREEN_ADDRESS);
  if (splashScreenEnable!=0)
  {
    if (columns == 16)
    {
      lcd.setCursor(0, 0);
      lcd.print("Serial LCD Kit ");
      lcd.setCursor(0, 1);
      lcd.print("Ready");
      delay(1000);
      lcd.clear();
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("   Serial LCD Kit ");
      lcd.setCursor(0, 2);
      lcd.print("   Ready");
      delay(1000);
      lcd.clear();
    }
  }
}

/*----------------------------------------------------------
  In loop(), we wait for a serial character to be 
  received. Once received, the character is checked against 
  all the special commands if it's not a special command the 
  character (or tab, line feed, etc.) is displayed. If it is 
  a special command another loop will be entered and we'll 
  again wait for any further characters that are needed to 
  finish the command.
  ----------------------------------------------------------*/
void loop()
{
    while (input.available() > 0) {
      inKey = input.read();
      // Check for special LCD command
      if ((inKey&0xFF) == SPECIAL_COMMAND)
        SpecialCommands();
      // Backlight control
      else if ((inKey&0xFF) == BACKLIGHT_COMMAND)
      {
        // Wait for the next character
        while(input.available() == 0)
          ;
        setBacklight(input.read());
      }
      // baud rate control
      else if ((inKey&0xFF) == BAUD_COMMAND)
      {
        // Wait for the next character
        while(input.available() == 0)
          ;
        setBaudRate(input.read());
      }
      // backspace
      else if (inKey == 8)
      {
        Cursor--;
        LCDDisplay(0x20);
        Cursor--;
      }
      // horizontal tab
      else if (inKey == 9)
        Cursor += 5;
      // line feed
      else if (inKey == 10)
        Cursor += columns - Cursor%columns;
      // carriage return
      else if (inKey == 13)
        Cursor += columns;
      // finally (since no special commad received), just display the received character
      else
        LCDDisplay(inKey);
    }
}

/* ----------------------------------------------------------
  SpecialCommands() is reached if SPECIAL_COMMAND is received.
  This function will wait for another character from the serial
  input and then perform the desired command. If a command is
  not recognized, nothing further happens and we jump back into
  loop().
  ----------------------------------------------------------*/
void SpecialCommands()
{
  // Wait for the next character
  while(input.available() == 0);
  
  inKey = input.read();
  // Clear Display
  if (inKey == 1)
  {
    Cursor = 0;
    lcd.clear();
  }
  // Move cursor right one
  else if (inKey == 20)
    Cursor++;
  // Move cursor left one
  else if (inKey == 16)
    Cursor--;
  // Scroll right
  else if (inKey == 28)
    lcd.scrollDisplayRight();
  // Scroll left
  else if (inKey == 24)
    lcd.scrollDisplayLeft();
  // Turn display on
  else if ((inKey == 12)&&(LCDOnOff==0))
  {
    LCDOnOff = 1;
    lcd.display();
  }
  // Turn display off
  else if (inKey == 8)
  {
    LCDOnOff = 0;
    lcd.noDisplay();
  }
  // Underline Cursor on
  else if (inKey == 14)
  {
    underline = 1;
    blinky = 0;
    lcd.noBlink();
    lcd.cursor();
  }
  // Underline Cursor off
  else if ((inKey == 12)&&(underline==1))
  {
    underline = 0;
    lcd.noCursor();
  }
  // Blinking box cursor on
  else if (inKey == 13)
  {
    lcd.noCursor();
    lcd.blink();
    blinky = 1;
    underline = 0;
  }
  // Blinking box cursor off
  else if ((inKey == 12)&&(blinky=1))
  {
    blinky = 0;
    lcd.noBlink();
  }
  // Set Cursor position
  else if ((inKey&0xFF) == 128)
  {
    // Wait for the next character
    while(input.available() == 0)
      ;
    inKey = input.read();
    Cursor = inKey;
  }
  else if (inKey == 30)
  {
    if (splashScreenEnable)
      splashScreenEnable = 0;
    else
      splashScreenEnable = 1;
    EEPROM.write(SPLASH_SCREEN_ADDRESS, splashScreenEnable);
  }
  else if (inKey == 3)
  {
    // 20 columns
    columns = 20;
    EEPROM.write(COLUMNS_ADDRESS, columns);
    lcd.begin(columns, rows);
    Cursor = 0;
  }
  else if (inKey == 4)
  {
    // 16 columns
    columns = 16;
    EEPROM.write(COLUMNS_ADDRESS, columns);
    lcd.begin(columns, rows);
    Cursor = 0;
  }
  else if (inKey == 5)
  {
    // 4 lines
    rows = 4;
    EEPROM.write(ROWS_ADDRESS, rows);
    lcd.begin(columns, rows);
    Cursor = 0;
  }
  else if (inKey == 6)
  {
    // 2 lines
    rows = 2;
    EEPROM.write(ROWS_ADDRESS, rows);
    lcd.begin(columns, rows);
    Cursor = 0;
  }
}

/* ----------------------------------------------------------
  LCDDisplay() receives a single character and displays it
  depending on what value is in Cursor. We also do a bit of
  manipulation on Cursor, if it is beyond the screen size.
  Finally the Cursor is advanced one value, before the function
  is exited.
  ----------------------------------------------------------*/
void LCDDisplay(char character)
{
  int currentRow = 0;
  characters = rows * columns;
  
  // If Cursor is beyond screen size, get it right
  while (Cursor >= characters)
    Cursor -= characters;
  while (Cursor < 0)
    Cursor += characters;
  
  if (Cursor >= columns)
    currentRow = Cursor/columns;
    
  lcd.setCursor(Cursor%columns, currentRow);
  lcd.write(character);
  
  Cursor++;
}

/* ----------------------------------------------------------
  setBacklight() is called from SpecialCommands(). It receives
  a backlight setting between 0 and 255. The backlight is set
  accordingly (via analogWrite()). Before exit the new backlight
  value is written to EEPROM.
  ----------------------------------------------------------*/
void setBacklight(boolean backlightSetting)
{
  lcd.setLED1Pin(backlightSetting);
  lcd.print(" ");
  EEPROM.write(LCD_BACKLIGHT_ADDRESS, backlightSetting);
}

/* ----------------------------------------------------------
  setBaudRate() is called from SpecialCommands(). It receives
  a baud rate setting balue that should be between 0 and 10.
  The baud rate is then set accordingly, and the new value is
  written to EEPROM. If the EEPROM value hasn't been written
  before (255), this function will default to 9600. If the value
  is out of bounds 10<baud<255, no action is taken.
  ----------------------------------------------------------*/
void setBaudRate(uint8_t baudSetting)
{
  // If EEPROM is unwritten (0xFF), set it to 9600 by default
  if (baudSetting==255)
    baudSetting = 4;
    
  switch(baudSetting)
  {
    case 0:
      input.begin(300);
      break;
    case 1:
      input.begin(1200);
      break;
    case 2:
      input.begin(2400);
      break;
    case 3:
      input.begin(4800);
      break;
    case 4:
      input.begin(9600);
      break;
    case 5:
      input.begin(14400);
      break;
    case 6:
      input.begin(19200);
      break;
    case 7:
      input.begin(28800);
      break;
    case 8:
      input.begin(38400);
      break;
    case 9:
      input.begin(57600);
      break;
    case 10:
      input.begin(115200);
      break;
  }
  if ((baudSetting>=0)&&(baudSetting<=10))
    EEPROM.write(BAUD_ADDRESS, baudSetting);
}

