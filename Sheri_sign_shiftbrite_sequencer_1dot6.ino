
/*
  Version 1.6  -  
  Standardized some naming schemes...
  Random = pick random colors
  Write = scribe with white, each element at a time, fade to a color
  Pop = set all elements, then write the array at once
  Flash = Write all pixels white, write the array, then write the color and write the array
  ___ pixels = apply this to some subset of pixels, one at a time
  ___ letters = perform this function, one letter at a time
  ___ word = perform this function on all of the pixels
  
  Installed a debug switch, using a resistor chain, and analog input.
  0 = run the regular pattern. 1-9 run a specific function as a loop.
  We'll use a CASE statement to define which function on which number.
  (And each function now calls out to the pseudo-console when called.)
  
  This uses the original ShiftBrite demo code to write the array.
  It's using the default ShiftBrite Shield pinouts, and you do
  need the extra power if you are using more than three ShiftBrites.
  
  The sign will write characters the way the tip of the pen draws
  while you write (left to write). This means that the pixels
  must be wired right to left, in the inverse order of drawing. */

//Define constants which should NEVER change (maybe use "const" instead?)
#define clockpin 13 // CI
#define enablepin 10 // EI
#define latchpin 9 // LI
#define datapin 11 // DI
#define NumLEDs 14   // how many LEDs are defined in each section below? 

//Define 'Globals' (variables available to all functions)
// first, for the LED array...
int LEDChannels[NumLEDs][3] = {0};
int SB_CommandMode;
int SB_RedCommand;
int SB_GreenCommand;
int SB_BlueCommand;

// next, for the variable we want to pass around...
int delaytime = 50;  // in milliseconds, between shifts...
int ipDelay = 50;  // inter-pixel delay...
int icDelay = 150;  // inter-character delay...
int nameDelay = 750; // delay between sequences...
int lpixel = 0;  //  Remember, lowest number will be the LAST in the chain!
int hpixel = 13;  // The highest number is clocked out last; the 1st in the chain!
int letter1l = 0;  // the first pixel in the first character... (S)
int letter1h = 2;  // the first pixel in the first character...
int letter2l = 3;  // the first pixel in the first character... (h)
int letter2h = 5;  // the first pixel in the first character...
int letter3l = 6;  // the first pixel in the first character... (e)
int letter3h = 8;  // the first pixel in the first character...
int letter4l = 9;  // the first pixel in the first character... (r)
int letter4h = 11;  // the first pixel in the first character...
int letter5l = 12;  // the first pixel in the first character... (i)
int letter5h = 15;  // the first pixel in the first character...

/* Prepare the colors array
 myBase is modulo 15, with R-G-B values, from brightest to dimmest
 0=white, 15=red, 30=yellow, 45=green, 60=blue, 75=teal, 90=purple
 value 105=black (all zeroes).
 myColorCode is modulo 3, defining one shade of a given color in R-G-B.
 You can use any modulo-3 number in the array, from 0 (white) to 105 (black).
 18=almost-brightest red, 51 is medium-green, 72 is dimmest-blue. */
int myBase;
int myColorCode;
int myColors[] = {
  1023, 1023, 1023,
  800, 800, 800,
  600, 600, 600,
  400, 400, 400,
  200, 200, 200,
  1023, 700, 700,
  1023, 300, 300,
  1023, 0, 0,
  700, 0, 0,
  300, 0, 0,
  1023, 1023, 700,
  1023, 1023, 300,
  1023, 1023, 0,
  700, 700, 0,
  300, 300, 0,
  700, 1023, 700,
  300, 1023, 300,
  0, 1023, 0,
  0, 700, 0,
  0, 300, 0,
  700, 1016, 200,
  300, 1008, 200,
  0, 1000, 200,
  0, 700, 130,
  0, 300, 60,
  700, 700, 1023,
  300, 300, 1023,
  0, 0, 1023,
  0, 0, 700,
  0, 0, 300,
  980, 700, 1023,
  940, 300, 1023,
  900, 0, 1023,
  600, 0, 700,
  200, 0, 300,
  0};

int lightPin = 3;   // Cadmium photo-cell (light detector)
int switchPin = 4;   /* thumbwheel switch and resistor chain connected to analog pin 3
                     outside leads to ground and +5V */
int switchValue = 0;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void setup() {
 
   pinMode(A3, INPUT);  // Cadminum-Sulfide photo-cell resistor tied to 5-volts
   digitalWrite(A3, LOW);  // set pulldown on analog pin 3 
   pinMode(A4, INPUT);  // this is the tap on the resistor chain...
   int randomSeed(analogRead(0)); // give us a fairly random seed to start...
   pinMode(datapin, OUTPUT);   // should be 11, for the MaceTech ShiftBrite Shield
   pinMode(latchpin, OUTPUT);   // should be 9...
   pinMode(enablepin, OUTPUT);   // should be 10
   pinMode(clockpin, OUTPUT);  // should be 13...
   SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPR1)|(0<<SPR0);
   digitalWrite(latchpin, LOW);
   digitalWrite(enablepin, LOW);
   // Adding the serial code, to display which function is active
   Serial.begin(9600);    // setup the serial monitor
}

void loop() {
  int dial = 0;   // this is the variable for the switch setting
  int thumbwheel = analogRead(switchPin);   // read the input pin
   // Now, set "dial" based on some skew around the measured reading
  if (thumbwheel < 50) { dial = 1; } // expect 0, full ccw
  if (thumbwheel > 100 && thumbwheel < 150) { dial = 2; } // expect 113-115
  if (thumbwheel > 200 && thumbwheel < 250) { dial = 3; } // expect 229
  if (thumbwheel > 325 && thumbwheel < 375) { dial = 4; } // expect 340-341
  if (thumbwheel > 425 && thumbwheel < 475) { dial = 5; } // expect 452-454
  if (thumbwheel > 545 && thumbwheel < 585) { dial = 6; } // expect 564
  if (thumbwheel > 650 && thumbwheel < 700) { dial = 7; } // expect 677-678
  if (thumbwheel > 770 && thumbwheel < 820) { dial = 8; } // expect 790-791
  if (thumbwheel > 880 && thumbwheel < 930) { dial = 9; } // expect 906-908
  if (thumbwheel > 1000) { dial = 10; } // expect 1023, fully cw
  // All the way counter-clockwise is zero, one click cw is 1, etc.
  switch (dial) {
    case 1: 
      myBase = (random(1, 34) * 3);
      flashcolor (myBase, lpixel, hpixel);
      delay(nameDelay);
      break;
    case 2: 
      myBase = (random(1, 6) * 15);
      flashfade (myBase, lpixel, hpixel);
      delay(nameDelay);
      break;
    case 3:
      poppixels (120);
      delay(nameDelay);
      writeblack(lpixel, hpixel);
      break;
    case 4:
      popcolor (lpixel, hpixel);
      writeblack(lpixel, hpixel);
      delay(nameDelay);
      break;
    case 5:
      myBase = (random(1, 6) * 15);
      writecolor (myBase, lpixel, hpixel);
      delay(nameDelay);
      break;
    case 6:
      myBase = (random(1, 6) * 15);
      writefade (myBase, lpixel, hpixel);
      delay(nameDelay);
      break;
    case 7:
      randomletters (5, 15);  // loop count, inter-letter delay in ms
      delay(icDelay);
      break;
    default:
    // fully clockwise!
      NormalPattern ();
      break;
  }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void NormalPattern() {
Serial.println("NormalPatern 1");
myBase = (random(1, 6) * 15);
flashfade (myBase, lpixel, hpixel);
 delay(nameDelay);
Serial.println("NormalPatern 2");
myBase = (random(1, 6) * 15);
writefade (myBase, lpixel, hpixel);
 delay(nameDelay);
Serial.println("NormalPatern 3");
myBase = (random(1, 6) * 15);
writecolor (myBase, 0, 13);
 delay(nameDelay);
Serial.println("NormalPatern 4");
randomletters (5, 15);  // loop count, inter-letter delay in ms
 delay(icDelay);
Serial.println("NormalPatern 5");
myBase = (random(1, 6) * 15);
writecolor (myBase, 0, 13);
 delay(nameDelay);
Serial.println("NormalPatern 6");
myBase = (random(4, 34) * 3);
writecolor (myBase, 0, 6);
Serial.println("NormalPatern 7");
myBase = (random(1, 6) * 15);
flashfade (myBase, lpixel, hpixel);
 delay(nameDelay);
Serial.println("NormalPatern 8");
popcolor (lpixel, hpixel);
writeblack(lpixel, hpixel);
 delay(nameDelay);
Serial.println("NormalPatern 9");
myBase = (random(1, 34) * 3);
writecolor (myBase, 7, 13);
 delay(nameDelay);
Serial.println("NormalPatern 10");
myBase = (random(5, 34) * 3);
writecolor (myBase, 0, 6);
myBase = (random(5, 34) * 3);
writecolor (myBase, 7, 13);
 delay(nameDelay);
Serial.println("NormalPatern 11");
poppixels (120);
 delay(nameDelay);
writeblack (lpixel, hpixel);
 delay(nameDelay);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void flashcolor(int myBaseColor, int myLpixel, int myHpixel) {
  /* Write white to the array, then write a random color to the array
     suggesting int myBaseColor = (random(1, 34) * 3); */
Serial.println("flashcolor");
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = 1023;
    LEDChannels[pixel][1] = 1023;
    LEDChannels[pixel][2] = 1023;
  }
WriteLEDArray();
 delay(icDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor];
    LEDChannels[pixel][1] = myColors[myBaseColor + 1];
    LEDChannels[pixel][2] = myColors[myBaseColor + 2]
    ;
  }
WriteLEDArray();
delay(nameDelay);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void flashfade(int myBaseColor, int myLpixel, int myHpixel) {
/* We take in a base color, and a range of LEDs, likely the fulll string.
   We then write "white" to the array, then the brightest color to
   the the array, and then the second-brightest color to the array. 
   Then we pause for nameDelay, and then, we write the next dimmest
   color to the array, then the dimmest color, and then black
   At the end of the function, all elements in the range should be off.*/
Serial.println("flashfade");
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = 1023;
    LEDChannels[pixel][1] = 1023;
    LEDChannels[pixel][2] = 1023;
  }
WriteLEDArray();
 delay(ipDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor];
    LEDChannels[pixel][1] = myColors[myBaseColor + 1];
    LEDChannels[pixel][2] = myColors[myBaseColor + 2];
  }
WriteLEDArray();
 delay(ipDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor + 3];
    LEDChannels[pixel][1] = myColors[myBaseColor + 4];
    LEDChannels[pixel][2] = myColors[myBaseColor + 5];
  }
WriteLEDArray();
 delay(ipDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor + 6];
    LEDChannels[pixel][1] = myColors[myBaseColor + 7];
    LEDChannels[pixel][2] = myColors[myBaseColor + 8];
  }
WriteLEDArray();
 delay(ipDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor + 9];
    LEDChannels[pixel][1] = myColors[myBaseColor + 10];
    LEDChannels[pixel][2] = myColors[myBaseColor + 11];
  }
WriteLEDArray();
 delay(ipDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor + 12];
    LEDChannels[pixel][1] = myColors[myBaseColor + 13];
    LEDChannels[pixel][2] = myColors[myBaseColor + 14];
  }
WriteLEDArray();
 delay(nameDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = 0;
    LEDChannels[pixel][1] = 0;
    LEDChannels[pixel][2] = 0;
  }
WriteLEDArray();
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void poppixels(int count) {
/* Just tell me how many pixels I get to pop, and I'll randomly choose
  which pixel, and which color to make it.*/
Serial.println("poppixels");
for (int pixel = lpixel; pixel < hpixel + 1; pixel++)  
  {
    int myBaseColor = (random(6, 34) * 3);
    LEDChannels[pixel][0] = myColors[myBaseColor];
    LEDChannels[pixel][1] = myColors[myBaseColor + 1];
    LEDChannels[pixel][2] = myColors[myBaseColor + 2];
  }
WriteLEDArray();
for (int counter = 0; counter < count; counter++)
  {
     int myBaseColor = (random(4, 34) * 3);
     int pixel = random(lpixel, hpixel+1);
     LEDChannels[pixel][0] = myColors[myBaseColor];
     LEDChannels[pixel][1] = myColors[myBaseColor + 1];
     LEDChannels[pixel][2] = myColors[myBaseColor + 2];
     WriteLEDArray();
     delay(ipDelay);
  }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void popcolor(int myLpixel, int myHpixel) {
  // Write a random color to all pixels, then write the array
int myBaseColor = (random(1, 34) * 3);
Serial.print(myBaseColor);
Serial.println("\t popcolor");
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor];
    LEDChannels[pixel][1] = myColors[myBaseColor + 1];
    LEDChannels[pixel][2] = myColors[myBaseColor + 2];
  }
WriteLEDArray();
delay(nameDelay);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void popcolorblack(int myLpixel, int myHpixel) {
  // Write a random color to all pixels, then write the array
int myBaseColor = (random(1, 34) * 3);
Serial.print(myBaseColor);
Serial.println("\t popcolor");
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = myColors[myBaseColor];
    LEDChannels[pixel][1] = myColors[myBaseColor + 1];
    LEDChannels[pixel][2] = myColors[myBaseColor + 2];
  }
WriteLEDArray();
delay(nameDelay);
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = 0;
    LEDChannels[pixel][1] = 0;
    LEDChannels[pixel][2] = 0;
  }
WriteLEDArray();
delay(nameDelay);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void writeblack(int myLpixel, int myHpixel) {
  // Just write one pixel at a time to black...
Serial.println("writeblack");
for (int pixel = myLpixel; pixel < myHpixel + 1; pixel++)  
  {
    LEDChannels[pixel][0] = 0;
    LEDChannels[pixel][1] = 0;
    LEDChannels[pixel][2] = 0;
    WriteLEDArray();
    delay(ipDelay);
  }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void writecolor(int myBaseColor, int myLpixel, int myHpixel) {
/* We take in a range of LEDs, likely the fulll string, and a base color.
   We then write "white" to the first pixel, then the brightest color to
   the pixel behind it, and then the second-brightest color to the pixel 
   behind that, until all pixels are at the medium-bright, solid color.
   (The sequence goes higher than the range, to make sure that the last
   trailing pixel get to the dimmer color...).
   At the end of the function, all elements in the range should be set
   to the all-solid color set by the base color.
   There is no name delay, presuming that this function is for writing
   a single letter (a subset of the string).*/
Serial.println("writecolor");
for (int active = myLpixel; active < (myHpixel + 3); active++)
  {
    if (active > (myLpixel - 1)  && active < (myHpixel+ 1))
    {
      LEDChannels[active][0] = 1023;
      LEDChannels[active][1] = 1023;
      LEDChannels[active][2] = 1023;
    }

    if (active > myLpixel  && active < (myHpixel+ 2))
    {
      LEDChannels[active-1][0] = myColors[myBaseColor + 3];
      LEDChannels[active-1][1] = myColors[myBaseColor + 4];
      LEDChannels[active-1][2] = myColors[myBaseColor + 5];
    }

    if (active > (myLpixel + 1)  && active < (myHpixel+ 3))
    {
      LEDChannels[active-2][0] = myColors[myBaseColor + 6];
      LEDChannels[active-2][1] = myColors[myBaseColor + 7];
      LEDChannels[active-2][2] = myColors[myBaseColor + 8];
    }
   WriteLEDArray();
   delay(ipDelay);
  } 
  delay(icDelay);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void writefade(int myBaseColor, int myLpixel, int myHpixel) {
/* We take in a range of LEDs, likely the full string, and a base color.
   We then write "white" to the first pixel, then the brightest color to
   the pixel behind it, and then the second-brightest color to the pixel 
   behind that, until all pixels are at the medium-bright, solid color.
   (The sequence goes higher than the range, to make sure that the last
   trailing pixel get to the dimmer color...) Then, we go back to the 
   beginning of the sequence, and dim them all to black using another 
   2-step trailing pixel method.
   At the end of the function, all elements in the range should be off.*/
Serial.println("writefade");
for (int active = myLpixel; active < (myHpixel + 3); active++)
  {
    if (active > (myLpixel - 1)  && active < (myHpixel+ 1))
    {
      LEDChannels[active][0] = 1023;
      LEDChannels[active][1] = 1023;
      LEDChannels[active][2] = 1023;
    }

    if (active > myLpixel  && active < (myHpixel+ 2))
    {
      LEDChannels[active-1][0] = myColors[myBaseColor + 3];
      LEDChannels[active-1][1] = myColors[myBaseColor + 4];
      LEDChannels[active-1][2] = myColors[myBaseColor + 5];
    }

    if (active > (myLpixel + 1)  && active < (myHpixel+ 3))
    {
      LEDChannels[active-2][0] = myColors[myBaseColor + 6];
      LEDChannels[active-2][1] = myColors[myBaseColor + 7];
      LEDChannels[active-2][2] = myColors[myBaseColor + 8];
    }
   WriteLEDArray();
   delay(ipDelay);
  } 
  delay(icDelay);

  for (int active = myLpixel; active < (myHpixel + 3); active++)
  {
    if (active > (myLpixel - 1)  && active < (myHpixel+ 1))
    {
      LEDChannels[active][0] = myColors[myBaseColor + 9];
      LEDChannels[active][1] = myColors[myBaseColor + 10];
      LEDChannels[active][2] = myColors[myBaseColor + 11];
    }

    if (active > myLpixel  && active < (myHpixel+ 2))
    {
      LEDChannels[active-1][0] = myColors[myBaseColor + 12];
      LEDChannels[active-1][1] = myColors[myBaseColor + 13];
      LEDChannels[active-1][2] = myColors[myBaseColor + 14];
    }

    if (active > (myLpixel + 1)  && active < (myHpixel+ 3))
    {
      LEDChannels[active-2][0] = myColors[105];
      LEDChannels[active-2][1] = myColors[105];
      LEDChannels[active-2][2] = myColors[105];
    }
   WriteLEDArray();
   delay(ipDelay);
  } 
  delay(icDelay);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void randomletters(int myCount, int myDelay) {
/* take each letter in sequence, pick a random color, send it to the
   writecolor function, until all letters have been written as many 
   times as requested.*/
Serial.println("randomletters");
for (int counter = 0; counter < myCount; counter++)
{
  Serial.print("randomletters pass ");
  Serial.println(counter);
  myBase = (random(5, 34) * 3);
  writecolor (myBase, letter1l, letter1h);
  delay(myDelay);
  myBase = (random(5, 34) * 3);
  writecolor (myBase, letter2l, letter2h);
  delay(myDelay);
  myBase = (random(5, 34) * 3);
  writecolor (myBase, letter3l, letter3h);
  delay(myDelay);
  myBase = (random(5, 34) * 3);
  writecolor (myBase, letter4l, letter4h);
  delay(myDelay);
  myBase = (random(5, 34) * 3);
  writecolor (myBase, letter5l, letter5h);
  delay(myDelay);
  }
Serial.println("randomletters is done.");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 The following code is from the original ShiftBrite demo code, by
Garret Mace, at http://docs.macetech.com/doku.php/shiftbrite_2.0
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SB_SendPacket() {
 
    if (SB_CommandMode == B01) {
     SB_RedCommand = 120;
     SB_GreenCommand = 100;
     SB_BlueCommand = 100;
    }
 
    SPDR = SB_CommandMode << 6 | SB_BlueCommand>>4;
    while(!(SPSR & (1<<SPIF)));
    SPDR = SB_BlueCommand<<4 | SB_RedCommand>>6;
    while(!(SPSR & (1<<SPIF)));
    SPDR = SB_RedCommand << 2 | SB_GreenCommand>>8;
    while(!(SPSR & (1<<SPIF)));
    SPDR = SB_GreenCommand;
    while(!(SPSR & (1<<SPIF)));
}
 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void WriteLEDArray() {
 
    SB_CommandMode = B00; // Write to PWM control registers
    for (int h = 0;h<NumLEDs;h++) {
	  SB_RedCommand = LEDChannels[h][0];
	  SB_GreenCommand = LEDChannels[h][1];
	  SB_BlueCommand = LEDChannels[h][2];
	  SB_SendPacket();
    }
 
    delayMicroseconds(15);
    digitalWrite(latchpin,HIGH); // latch data into registers
    delayMicroseconds(15);
    digitalWrite(latchpin,LOW);
 
    SB_CommandMode = B01; // Write to current control registers
    for (int z = 0; z < NumLEDs; z++) SB_SendPacket();
    delayMicroseconds(15);
    digitalWrite(latchpin,HIGH); // latch data into registers
    delayMicroseconds(15);
    digitalWrite(latchpin,LOW);
}

// * * * * * * * * * * < End of File > * * * * * * * * * * * * * * *

