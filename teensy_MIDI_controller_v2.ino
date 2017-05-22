/* some code designed for using the Teensy 3 as a USB MIDI controller
v2.0, 16 May 2017
by Yann Seznec www.yannseznec.com

no copyright or anything, use however you want

remember to select MIDI as your USB Type in the Tools menu

this should also work with other Teensy boards, apart from the "touch" pins

things that are kind of dumb in this code:
- the touch threshold is hard coded (int touchThreshold)
- touch pins only send midi note on/off with no velocity change
- no system for sending touch pin as CC

new things:
- easily assignable CC numbers
- added outputs for controlling things via midi - send midi signals to the Teensy to
controls lights or motors or what have you
- regular MIDI support in addition to USB MIDI (not fully tested but should work)

TODO:
- add pitch wheel support somehow? 
- touch pin CC 

NOTES:
If you want to use regular MIDI using din connectors, keep the Rx and Tx pins free. On Teensy LC
and Teensy 3 (and up) these are pins 0 and 1.
Check this page for more info on how to wire this up:
https://www.pjrc.com/teensy/td_libs_MIDI.html
*/

#include <MIDI.h>
// the MIDI channel number to send messages
const int channel = 1;
//

int const numPins = 6; //  number of analog inputs for CC
int currentVal[numPins];
int newVal[numPins];
int analogPins[] = {  
  14,15,16,17,18,19   // which analog pins to use
};
int analogPinsCC[] = {  
  50,51,52,53,54,55   // which CC to use
};


int const numDigPins = 5; // number of digital pins to send note values
int currentDig[numDigPins];
int digitalpin[] = {
  5,6,7,8,9,   // which digital pins to use for sending note values
};
int digitalpitch[] = {
  48,50,51,53,55,60}; // which midi notes to send from the digitalpins pins



int const numDigPinsCC = 3; // number of digital pins to send CC (0 or 127)
int currentDigCC[numDigPinsCC];
int digPinsCC[] = {
   2,3,4 // which digital pins to use for sending CC
};
int digitalPinsCC[] = {
  30,31,32,33,34,35
};



int const numTouchPins = 0; // number of pins to use as touchpins, sending note values
int touch[numTouchPins];
int touchon[numTouchPins];
int touchpin[] = {
  2}; // which digital pins to use as touch pins
  int touchpitch[] = {
  }; // which midi notes to send from the touch pins
  int touchThreshold = 2000;
  int touchMax = 5000; 
  
int const numOutputs = 0; // number of pins to use as outputs
int outs[numOutputs];
int outPins[] = {
 10 }; // which digital pins to use as out pins
  int outputpitch[] = {
 48 }; // which midi notes to use for sending the outputs






void setup() {
  MIDI.begin(4);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  for (int i = 0; i < numPins; i++) {
    pinMode(analogPins[i], INPUT_PULLUP);
  }
  
    for (int i = 0; i < numDigPins; i++) {
    pinMode(digitalpin[i], INPUT_PULLUP);
  }
  
    for (int i = 0; i < numDigPinsCC; i++) {
    pinMode(digPinsCC[i], INPUT_PULLUP);
  }
  
      for (int i = 0; i < numTouchPins; i++) {
    pinMode(touchpin[i], INPUT_PULLUP);
  }
  
  
    for (int i = 0; i < numOutputs; i++) {
    pinMode(outPins[i], OUTPUT);
  }
  
  


  Serial.begin(38400);

}

void loop() {

//  touchpads COMMENT OUT IF USING TEENSY++ or TEENSY 2
  for (int i = 0; i < numTouchPins; i++) {
   touch[i] = touchRead(touchpin[i]); 

    if (touch[i] > touchThreshold && touchon[i] == 0) {
      usbMIDI.sendNoteOn(touchpitch[i], 100, channel); 
      MIDI.sendNoteOn(touchpitch[i], 100, channel); 
     touchon[i] = 1;
    }
    if (touch[i] < touchThreshold && touchon[i] == 1) {
      usbMIDI.sendNoteOff(touchpitch[i], 100, channel);
      MIDI.sendNoteOff(touchpitch[i], 100, channel);
      touchon[i] = 0;
    }

  }

 

// digital pins sending notes
  for (int i = 0; i < numDigPins; i++) {
    if (digitalRead(digitalpin[i]) == 1 && currentDig[i] == 0) {
      usbMIDI.sendNoteOff(digitalpitch[i], 100, channel); 
      MIDI.sendNoteOff(digitalpitch[i], 100, channel); 
      currentDig[i] = 1;
    }  
    if (digitalRead(digitalpin[i]) == 0  && currentDig[i] == 1) {
      usbMIDI.sendNoteOn(digitalpitch[i], 100, channel);
      MIDI.sendNoteOn(digitalpitch[i], 100, channel);
      currentDig[i] = 0;
    }  
  }

// digital pins sending CC (0 or 127)

  for (int i = 0; i < numDigPinsCC; i++) {
    if (digitalRead(digPinsCC[i]) == 1 && currentDigCC[i] == 0) {
      usbMIDI.sendControlChange(digitalPinsCC[i], 0, channel); 
      MIDI.sendControlChange(digitalPinsCC[i], 0, channel); 
      currentDigCC[i] = 1;
    }  
    if (digitalRead(digPinsCC[i]) == 0  && currentDigCC[i] == 1) {
      usbMIDI.sendControlChange(digitalPinsCC[i], 127, channel);
      MIDI.sendControlChange(digitalPinsCC[i], 127, channel);
      currentDigCC[i] = 0;
    }  
  }

// analog pins

  for (int i = 0; i < numPins; i++) {

    newVal[i] = analogRead(analogPins[i]);

    if (abs(newVal[i] - currentVal[i])>3) {
      usbMIDI.sendControlChange(analogPinsCC[i], newVal[i]>>3, channel); 
      MIDI.sendControlChange(analogPinsCC[i], newVal[i]>>3, channel); 
      currentVal[i] = newVal[i];
    }  
  }
  
// output
  
  // i think if you remove these last two lines everything breaks and things are sad and people cry
  while (usbMIDI.read()); // read and discard any incoming MIDI messages
   delay(5); 
}

 void OnNoteOn(byte channel, byte note, byte velocity) {
   for (int i = 0; i < numOutputs; i++) {
     
     if (note == outputpitch[i]) {
      digitalWrite(outPins[i], LOW);
     }
  }
     
}

void OnNoteOff(byte channel, byte note, byte velocity) {
   for (int i = 0; i < numOutputs; i++) {
      if (note == outputpitch[i]) {
      digitalWrite(outPins[i], HIGH);
      }
  }
}





