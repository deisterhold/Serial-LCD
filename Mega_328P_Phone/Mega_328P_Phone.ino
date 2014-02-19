/*
Arduino sketch that uses the SeeedStudio GPRS shield v1.4
 Includes basic functions to make calls, send text messages, and other basic functionality
 
 Need to create a GSM Class for this shield.
 Add Hardware interrupts for processing SMS messages and Phone calls
 
 By: Daniel Eisterhold
 31 Jan 2014
 */

#include <SoftwareSerial.h>
#include <String.h>


SoftwareSerial Phone(7, 8);

int latchPin = 10;
int clockPin = 12;
int dataPin = 11;

const boolean debug = true;
boolean shieldReady = false;

void setup()
{
  Phone.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate

  //Reserve memory for the strings that hold the phone number and text message
  String textMessage = "";
  String phoneNumber = "";
  textMessage.reserve(140);
  phoneNumber.reserve(10);
  //powerToggle();
  if(debug) Serial.println("Shield Powering Up...");
  delay(5000);
  if(debug) Serial.println("Shield Powered Up.");
}

void loop()
{
  //Add Stuff Here
  if(Phone.available() > 0)
  {
    processSerialData();
  }
}

void processSerialData()
{
  String serialData = "";
  while(Phone.available())
  {
    serialData += (char)Phone.read();
    if(serialData.length() == 64 || Phone.peek() == '\n')
    {
      break;
    }
  }
  serialData.trim();
  if(debug) Serial.println(serialData);
  if(serialData == "RING")
  {
    Serial.println("The Phone is ringing");
    answer_Call();
    delay(300);
    //end_Call();
  }
  else if (serialData == "NO CARRIER")
  {
    Serial.println("The Call has ended");
  }
  else if (serialData == "Call Ready")
  {
    shieldReady = true;
    if(debug) Serial.println("Shield Ready");
    if(debug) Serial.println("The Current time is " + getCurrentTime());
  }
}

//Sends a text message to the specified number with the included message
void sendText(String phoneNumber,String message)
{
  String command = "AT + CMGS = \"+1" + phoneNumber + "\"";
  Phone.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  Phone.println(command);//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  Phone.println(message);//the content of the message
  delay(100);
  Phone.write(0x1A);//the ASCII code of the ctrl+z is 26
  delay(100);
  Phone.println();
  delay(3000);
  if(debug) Serial.println("Sending message to: " + phoneNumber);
  if(debug) Serial.println("Message that was sent \"" + message + "\"");
}

//Dials the specified phone number
void make_Call(String phoneNumber)
{
  String command = "ATD + +1" + phoneNumber +";";
  Phone.println(command);//dial the number
  delay(100);
  Phone.println();
}

void end_Call()
{
  Phone.println("AT+HVOIC");
  delay(100);
  Phone.println();
}

void answer_Call()
{
  Phone.println("ATA");
  delay(100);
  Phone.println();
}

void setRingVolume(int volume)
{
  constrain(volume, 0, 4);
  String command = "AT+CRSL=" + String(volume);
  Phone.println(command);
  delay(100);
  Phone.println();  
}

void mute_Call(boolean muteStatus)
{
  if(muteStatus)
  {
    //Turns mute on
    Phone.println("AT+CMUT=1");
    delay(100);
    Phone.println();
  }
  else
  {
    //Turns mute off
    Phone.println("AT+CMUT=0");
    delay(100);
    Phone.println();
  }
}

String getCurrentTime()
{
  int year = 2000;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  String time = "";
  Phone.println("AT+CCLK?");
  delay(50);
  Phone.println();
  delay(25);
  Phone.readStringUntil('"');
  time = Phone.readStringUntil('"');
  return time;
}
void powerToggle()
{
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}

void setSignalMeter(uint8_t signalPercentage)
{
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  int signalStrength = map(signalPercentage, 0, 255, 1, 8);
  byte signal = 0;
  for(int bitPosition = 0; bitPosition < signalStrength; bitPosition++)
  {
    bitSet(signal, bitPosition);
  }
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, signal);   
  digitalWrite(latchPin, HIGH);
}
