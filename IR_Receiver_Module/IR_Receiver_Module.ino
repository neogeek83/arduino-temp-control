//www.elegoo.com
//2016.06.13

#include "IRremote.h"

int receiver = 3; // Signal Pin of IR receiver to Arduino Digital Pin 3
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // Start the receiver

}/*--(end setup )---*/


void loop(){   /*----( LOOP: RUNS CONSTANTLY )----*/
  if (irrecv.decode(&results)){ // have we received an IR signal?
    Serial.println(translateIR()); 
    irrecv.resume(); // receive the next value
  }  
}/* --(end main loop )-- */

/*-----( Function )-----*/
String translateIR(){ // takes action based on IR code received

// describing Remote IR codes 
  switch(results.value) {
    //row 1
    case 0xFFA25D: return " PWR";
    case 0xFF629D: return " VOL+";
    case 0xFFE21D: return " FUNC/STOP";
  
    //row 2
    case 0xFF22DD: return " |<<";
    case 0xFF02FD: return " PLAY/PAUSE";
    case 0xFFC23D: return " >>|";
  
    //row 3
    case 0xFFE01F: return " DOWN";
    case 0xFFA857: return " VOL-";
    case 0xFF906F: return " UP";
  
    //row 4
    case 0xFF6897: return " 0";
    case 0xFF9867: return " EQ";
    case 0xFFB04F: return " ST/REPT";
  
    //row 5
    case 0xFF30CF: return " 1";
    case 0xFF18E7: return " 2";
    case 0xFF7A85: return " 3";
  
    //row 6
    case 0xFF10EF: return " 4";
    case 0xFF38C7: return " 5";
    case 0xFF5AA5: return " 6";
  
    //row 7
    case 0xFF42BD: return " 7";
    case 0xFF4AB5: return " 8";
    case 0xFF52AD: return " 9";
  
    //holding a button down results in a repeat code
    case 0xFFFFFFFF: return " REPEAT";

    default: 
      String retValue = "0x";
      retValue = retValue + String(results.value, HEX);
      return retValue;
  }// End Case

  delay(500); // Do not get immediate repeat


} //END translateIR
