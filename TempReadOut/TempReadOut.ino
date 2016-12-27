/*
 The LiquidCrystal library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface. -- Wire up using the below
   Better wiring diiagram(just use data pins defined below): http://www.instructables.com/id/How-to-use-an-LCD-displays-Arduino-Tutorial/step3/The-Circuit/

  The circuit:
 * LCD RS pin to digital pin 7
 * LCD Enable pin to digital pin 8
 * LCD D4 pin to digital pin 9
 * LCD D5 pin to digital pin 10
 * LCD D6 pin to digital pin 11
 * LCD D7 pin to digital pin 12
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K potentiometer:
 * Ends to +5V and ground
 * LCD VO pin (pin 3) to potentiometer center lead
 * 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#include <SimpleDHT.h>
// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int pinDHT11 = 2;
SimpleDHT11 dht11;


#include "IRremote.h"
int receiver = 3; // Signal Pin of IR receiver to Arduino Digital Pin 3
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'
int setTemp = 72;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  lcd.setCursor(0, 0); // setCursor(col:0-16,row:0-1)
  
  // read with raw sample data.
  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    lcd.print("Read DHT11 failed");
    return;
  }

  lcd.print("Temp: "); lcd.print((int)temperature); lcd.print("C (");
  temperature=(int)temperature*9/5+32;//convert to F
  lcd.print((int)temperature); lcd.print("F)");
  
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.print((int)humidity); lcd.print("% ");
  
   if (irrecv.decode(&results)){ // have we received an IR signal?
     String IRVal = translateIR();
     if ( IRVal == "VOL+" ) setTemp++;
     if ( IRVal == "VOL-" ) setTemp--;
     //irrecv.resume(); // receive the next value
   }
   lcd.print(setTemp);
   lcd.print("F");
  
  delay(500);// DHT11 sampling rate is 500kHZ.
}


String translateIR(){ // takes action based on IR code received

// describing Remote IR codes 
  switch(results.value) {
    //row 1
    case 0xFFA25D: return "PWR";
    case 0xFF629D: return "VOL+";
    case 0xFFE21D: return "FUNC/STOP";
  
    //row 2
    case 0xFF22DD: return "|<<";
    case 0xFF02FD: return "PLAY/PAUSE";
    case 0xFFC23D: return ">>|";
  
    //row 3
    case 0xFFE01F: return "DOWN";
    case 0xFFA857: return "VOL-";
    case 0xFF906F: return "UP";
  
    //row 4
    case 0xFF6897: return "0";
    case 0xFF9867: return "EQ";
    case 0xFFB04F: return "ST/REPT";
  
    //row 5
    case 0xFF30CF: return "1";
    case 0xFF18E7: return "2";
    case 0xFF7A85: return "3";
  
    //row 6
    case 0xFF10EF: return "4";
    case 0xFF38C7: return "5";
    case 0xFF5AA5: return "6";
  
    //row 7
    case 0xFF42BD: return "7";
    case 0xFF4AB5: return "8";
    case 0xFF52AD: return "9";
  
    //holding a button down results in a repeat code
    case 0xFFFFFFFF: return "REPEAT";

    default: 
      String retValue = "0x";
      retValue = retValue + String(results.value, HEX);
      return retValue;
  }// End Case

  delay(500); // Do not get immediate repeat


} //END translateIR
