
/*
 created : 16 Dec 2014 
 modified: 17, 22, 23, 28, 30 Dec 2014
           4, 5, 6, 7, 8, 9, 10, 11, 30, 31 Jan 2015 
 */


//define Debug On 

#if defined Debug 
#warning "*** Compiling with Debug ***"
#endif

#include "Adafruit_WS2801.h"
#include "SPI.h" 

// define the number of zones (0 to 5)
#define numZones     6
// define the number of pixels in each zone
#define zone0Pixels 23 
#define zone1Pixels 13 
#define zone2Pixels 69 
#define zone3Pixels 39 
#define zone4Pixels 11 
#define zone5Pixels 14 

// TV run              5 / 6 -----------------------------------------------------------------------------+ 
// Glass Cabinet run   4 / 5 ----------------------------------------------------------------+            | 
// Dining Room run     3 / 4 ---------------------------------------------------+            |            |
// Hob Extractor run   2 / 3 --------------------------------------+            |            |            |
// Wall Cupboard run   1 / 2 -------------------------+            |            |            |            |
// Fridge / Cooker run 0 / 1 ------------+            |            |            |            |            |
//                                       |            |            |            |            |            |
// testing setup uint8_t

//warning "************* PINS set for TESTING Mode ************"
//const uint8_t clockPin  [] = {          A4,           8,          A2,          A3,          A0,          A5 } ;
//const uint8_t dataPin   [] = {           4,           5,           6,           7,          A1,           9 } ;
#warning "************* PINS set for LIVE Mode ************"
const uint8_t clockPin  [] = {          A0,          A1,          A2,          A3,          A4,          A5 } ;
const uint8_t dataPin   [] = {           4,           5,           6,           7,           8,           9 } ;

const uint8_t numPixels [] = { zone0Pixels, zone1Pixels, zone2Pixels, zone3Pixels, zone4Pixels, zone5Pixels } ;

// define single array to hold status of every pixel.
//   0 -> 22   String 0
//  23 -> 35   String 1
//  36 -> 104  String 2
// 105 -> 143  String 3
// 144 -> 154  String 4
// 155 -> 168  String 5

// physical x,y coordinates of each pixel used for multizone effects
const uint8_t pixelXY [169] [2] = 
  {
// String 0 = 23 pixels      Fridge / Cooker zone
    { 3, 0},{ 3, 1},{ 3, 2},{ 3, 3},{ 3, 4},{ 3, 5},{ 3, 6},{ 3, 7},{ 3, 8},{ 3, 9},
    { 3,10},{ 3,11},{ 3,12},{ 3,13},{ 3,14},{ 3,15},{ 3,16},{ 3,17},{ 3,18},{ 3,19},
    { 3,20},{ 3,21},{ 3,22},       
// String 1 = 13 pixels      High level Wall Cupboard zone
    { 0,34},{ 0,35},{ 0,36},{ 0,37},{ 0,38},{ 0,39},{ 0,40},{ 0,41},{ 0,42},{ 0,43},
    { 0,44},{ 0,45},{ 0,46},          
// String 2   = 69 pixels    Hob Extractor Zone
// String 2.1 = 19 pixels
    {26, 0},{26, 1},{26, 2},{26, 3},{26, 4},{26, 5},{26, 6},{26, 7},{26, 8},{26, 9},
    {26,10},{26,11},{26,12},{26,13},{26,14},{26,15},{26,16},{26,17},{26,18},
// String 2.2 = 11 pixels 
    {25,18},{24,18},{23,18},{22,18},{21,18},{20,18},{19,18},{18,18},{17,18},{16,18},
    {15,18},          
// String 2.3 =  9 pixels
    {14,19},{14,20},{14,21},{14,22},{14,23},{14,24},{14,25},{14,26},{14,27},
// String 2.4 = 11 pixels
    {15,28},{16,28},{17,28},{18,28},{19,28},{20,28},{21,28},{22,28},{23,28},{24,28},
    {25,28},
// String 2.5 = 19 pixels
    {26,28},{26,29},{26,30},{26,31},{26,32},{26,33},{26,34},{26,35},{26,36},{26,37},
    {26,38},{26,39},{26,40},{26,41},{26,42},{26,43},{26,44},{26,45},{26,46},
// String 3   = 39 pixels    Dining Room Zone
    {34, 8},{34, 9},{34,10},{34,11},{34,12},{34,13},{34,14},{34,15},{34,16},{34,17},
    {34,18},{34,19},{34,20},{34,21},{34,22},{34,23},{34,24},{34,25},{34,26},{34,27},
    {34,28},{34,29},{34,30},{34,31},{34,32},{34,33},{34,34},{34,35},{34,36},{34,37},
    {34,38},{34,39},{34,40},{34,41},{34,42},{34,43},{34,44},{34,45},{34,46},
// String 4   = 11 pixels    Glass Cabinet Zone
    {29,18},{28,18},{27,18},{26,18},{25,18},{24,18},{23,18},{22,18},{21,18},{20,18},
    {19,18},
// String 5   = 14 pixels    TV surround Zone
    {32,46},{31,46},{30,46},{29,46},{28,46},
    {28,46},{28,46},
    {28,46},{29,46},{30,46},{31,46},{32,46},
    {32,46},{32,46}          
  } ;

// The zones are numbered 0 - 5, so 6 in total 

uint8_t zoneStateChanged     [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // 1 = zone status changed, 0 it didn't
uint8_t zoneState            [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // 0 = OFF, 1 = turningOn, 2 = On , 3 = turningOff
uint8_t zoneColourOrPattern  [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // 0 = off, 01-29 colour, 31-59 zone patterns, 61-89 group patterns 
uint8_t zonePatternSpeed     [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // running pattern speed for each zone 0 = fastest, 99 =very slow 
uint8_t zonePatternSkips     [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // number of times left to skip the ++patternCount
uint8_t zonePatternCount     [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // How far into the pattern are we? 
uint8_t zoneBrightnessLevel  [ numZones ] = { 0, 0, 0, 0, 0, 0 } ;  // zone brightness 00 low -> 99 high 

// define each zone as a strip for control with the Adafruit routines  
Adafruit_WS2801 strip[] = {
    Adafruit_WS2801(numPixels[0], dataPin[0], clockPin[0], WS2801_GRB) , 
    Adafruit_WS2801(numPixels[1], dataPin[1], clockPin[1], WS2801_GRB) ,
    Adafruit_WS2801(numPixels[2], dataPin[2], clockPin[2], WS2801_GRB) , 
    Adafruit_WS2801(numPixels[3], dataPin[3], clockPin[3], WS2801_GRB) , 
    Adafruit_WS2801(numPixels[4], dataPin[4], clockPin[4], WS2801_GRB) ,
    Adafruit_WS2801(numPixels[5], dataPin[5], clockPin[5], WS2801_GRB) 
} ;

const uint8_t maxColours   = 10 ;   // the maximum number of colours defined  
const uint8_t maxZPatterns = 3 ;   // the maximum number of zone patterns defined  
const uint8_t maxGPatterns = 2 ;   // the maximum number of zone group patterns defined  

const uint8_t colourArray [maxColours][3] = {
                    {  0,  0,  0},   // 0  Black or Off  
                    {255,  0,  0},   // 1  Red
                    {255,255,  0},   // 2  Yellow
                    {  0,255,  0},   // 3  Green 
                    {  0,255,255},   // 4  Cyan
                    {  0,  0,255},   // 5  Blue
                    {255,  0,255},   // 6  Magenta 
                    {255,255,255},   // 7  White  
                    {192,192,255},   // 8  Cool White                    
                    {255,192,128}    // 9  Warm White
                   } ;

const int heartbeatLEDpin             = 13 ; // System Heartbeat - double flash every second

// set up slow and fast flash variables used in interrupt routine as volatile 
volatile unsigned int slowFlash        = 0 ;  // slow flash variable for LEDs
volatile unsigned int fastFlash        = 0 ;  // fast flash variable for LEDs

// status used for each Zone 
// Off        = 0 = all pixels are off in this zone
// On         = 1 = all pixels are on and are displaying a static colour OR continuous pattern is running (Rainbow etc) 

enum { Off, On } ;
enum { Red, Green, Blue } ;

// used to indicate 0 = x and y = 1 in array of physical pixel locations
enum { xPos, yPos } ;

String cmdLine    ;  // used to store the commands coming in on the serial port
String firstChar  ;  // used to store first character of a command for comparison agaist known commands

#if defined Debug 

// Used during testing to establish how many loops (frames) per second the arduino is capable of.
volatile unsigned int     intCount ;
         unsigned int     loopCount ;

#endif

         
uint8_t GPatternCount    = 0 ;  // 0 to 46 
uint8_t GPatternSpeed    = 0 ; // 0 = fast, 99 = very slow  (0 delay to 99 delay)
uint8_t GPatternSkips    = 0 ; // skips left before next patternloop increment
uint8_t maxGPatternCount = 46 ; // some patterns need 46, some need 255

         
// This code is run once at startup
void setup() {

  // Set up interrupt timer
  // Set Timer Clock Divider   
  // CK divided by 1024  - clock overflow runs 30.63725 times a second - every 32.64 ms
  TCCR2B = 0<<WGM22 | 1<<CS22 | 1<<CS21 | 1<<CS20; 
  // enable overflow interrupt - interrupt fires when clock overflows
  TIMSK2 = 1<<TOIE2;                               
  
  // set heartbeat LED pin as output 
  pinMode(heartbeatLEDpin , OUTPUT);  
  
  // Initialise LED pixel strings 0 to 5
  // All off at start-up  
  for (uint8_t i=0; i < numZones; i++) {
    strip[i].begin() ;
    strip[i].show()  ;
  }  

  // Open serial communications 
  Serial.begin(57600);
  // clear the buffer incase there is some start-up garbage in there 
  clearSerialBuffer() ;  

  #if defined Debug 
  // report the amount of free ram at Startup.  
  Serial.print   ("Free Ram : ") ;
  Serial.println (freeRam()) ;
  
  intCount  = 0 ; // zero the counter that increments every time the interrupt is fired
  loopCount = 0 ; // zero the counter that increments every time the main loop completes

  #endif   

  
}

// The main program loop
void loop() {
  
  // temporary code to check timing during debugging. 
  #if defined Debug 
  if (intCount >= 306) { 
    Serial.print("loops per 306 interrupts (10 secs) : ") ;
    Serial.print(loopCount) ;
    Serial.print("\tintCount : ") ;
    Serial.println(intCount) ;
    noInterrupts() ;
    intCount  = 0 ;
    loopCount = 0 ;
    interrupts() ;
  }
  #endif 

  // start of Serial processing section 
  // every loop : Check to see if serial data has arrived 

  if (Serial.available()) {
    // if data is in the buffer read one byte
    char s = Serial.read();
    // ignore CR if sent 
    if (s != '\r') {
      // Only NL is used as command terminator
      // if this byte is NL then we have a complete command
      if (s == '\n' ) {
        // take action based on command received
        String firstChar = cmdLine.substring(0,1) ;        
        if (firstChar == "S" || firstChar == "s") { 
          Serial.println ("OK");  // say ok
          #if defined Debug
          dumpTheLot () ;         // dump variables for debug 
          #endif
        } else if (firstChar == "N" || firstChar == "F" || firstChar == "n" || firstChar == "f" ) {
          Serial.print ("RCVD >") ;  // echo back what we received
          Serial.print (cmdLine) ;
          Serial.println ("< OK") ;
          //           1
          // 01234
          // Nzppssll  = N=ON  z=zone pp=pattern ss=patternspeed ll=brightness level 
          // Fz        = F=OFF z=zone 
          // S         = Status display

          int Czone                =  7 ; // set default zone to all ceiling zones 
          int CzoneColourOrPattern =  7 ; // default col/pattern = cool white
          int CzonePatternSpeed    = 10 ; // set default speed to 10 (0 = fastest, 99 = slowest) 
          int CzoneBrightnessLevel = 99 ; // set default brightness to full
          
          if (cmdLine.length() > 1)  Czone                 = cmdLine.substring(1,2).toInt() ; // zone is char 1
          if (cmdLine.length() > 3)  CzoneColourOrPattern  = cmdLine.substring(2,4).toInt() ; // colour or pattern is char 2 to 3
          if (cmdLine.length() > 5)  CzonePatternSpeed     = cmdLine.substring(4,6).toInt() ; // pattern speed is char 4 to 5
          if (cmdLine.length() > 7)  CzoneBrightnessLevel  = cmdLine.substring(6,8).toInt() ; // brightness level 0-99 
          #if defined Debug
          Serial.print ("Zone,Colour/Pattern,PatternSpeed ");
          Serial.print (Czone);
          Serial.print (",");
          Serial.print (CzoneColourOrPattern);
          Serial.print (",");
          Serial.print (CzonePatternSpeed);
          #endif
          // if Zone < 6 then we are controlling an individual string
          //
          // This section of code is only for setting flags when a new command
          // is received
          //
          uint8_t zoneStart = 0 ;
          uint8_t zoneEnd   = 0 ;
          
          switch (Czone) {
            case 6: // all Kitchen ceiling zones
              zoneStart = 0 ;
              zoneEnd   = 2 ;
              break ;
            case 7: // all ceiling zones
              zoneStart = 0 ;
              zoneEnd   = 3 ;
              break ;
            case 8: // all ceiling zones + Glass Cab
              zoneStart = 0 ;
              zoneEnd   = 4 ;
              break ;
            case 9:  // all zones 
              zoneStart = 0 ;
              zoneEnd   = 5 ;
              break ;
            default: 
              zoneStart = Czone ;
              zoneEnd   = Czone ;
          }
          #if defined Debug
          Serial.println () ;
          Serial.print ("Zone list :") ;
          #endif
          for (int thisZone = zoneStart ; thisZone <= zoneEnd ; thisZone++) {
            // set value for each zone or group of zones based on input
            // when a zone is Off and we want to turn it on :
            if ( 
                 ( firstChar == "N" || firstChar == "n" )  // if we have an "On" request (n/N)
                 &&           
                 ( CzoneColourOrPattern  > 0 ) // the pattern is not 0 (which is "Off") 
                 &&   
                 ( 
                   ( 
                     ( CzoneColourOrPattern < 30 ) 
                     && 
                     ( CzoneColourOrPattern <= maxColours ) 
                   ) // Colour request is valid OR
                   ||  
                   ( 
                     ( CzoneColourOrPattern > 30 ) 
                     && 
                     ( CzoneColourOrPattern < 60 ) 
                     && 
                     ((CzoneColourOrPattern-30) <= maxZPatterns)  
                   )  // Zone pattern request is valid
                   ||  
                   ( 
                     ( CzoneColourOrPattern > 60 ) 
                     && 
                     ( CzoneColourOrPattern < 90 ) 
                     && 
                     ((CzoneColourOrPattern-60) <= maxGPatterns)  
                   )  // Group pattern request is valid
                 )
               ) {  // then set zone values
              zoneState            [thisZone] = On ;
              zoneColourOrPattern  [thisZone] = CzoneColourOrPattern ;
              zoneBrightnessLevel  [thisZone] = CzoneBrightnessLevel ;
              if (CzoneColourOrPattern > 60) {  //  Group zone pattern 
                GPatternSpeed = CzonePatternSpeed ; //  so set group zone speed 
              } else if (CzoneColourOrPattern > 30) {  // else its an a individual zone pattern 
                zonePatternSpeed [thisZone] = CzonePatternSpeed ; // so set the zone speed
              }
              zoneStateChanged [thisZone] = true ;  // used to trigger count reset 
            }  

            // when a zone is On and we want to turn it off :       
            if (  firstChar == "F" || firstChar == "f" ) {  // request is to turn off
              zoneState            [thisZone] = Off ;
              zoneColourOrPattern  [thisZone] = 0 ; // set to zero as it is now off
              zonePatternSpeed     [thisZone] = 0 ; // set to zero as it is now off
              zoneStateChanged     [thisZone] = true ;  // used to run final colour set to 0
            }  
            #if defined Debug
              Serial.print ("\t") ;
              Serial.print (thisZone) ;
            #endif
          }

  
        } else {
          // Not a command we know about so do nothing
          // except repeat the command back to the sender
          Serial.print ("??? >");
          Serial.print (cmdLine);
          Serial.println ("<");
        }
        // clear this command ready for next one 
        cmdLine = "" ; 
      } else { 
        // if this byte isn't a NL character then 
        // just add it to the command string 
        cmdLine += s ;
      }
    }
  } // end of Serial processing section

  // Main code to process each zone on every loop 
  for (uint8_t thisZone=0; thisZone < numZones; thisZone++) {  // for each zone  
    if (zoneState [thisZone] == On) {                          // if the zone if "On"
      if (zoneColourOrPattern [thisZone] > 60) {               // and the pattern is a Group Zone pattern
        GpatternRoutine (thisZone) ;                           // run Group Pattern routine for this zone.
      } else if (zoneColourOrPattern [thisZone] > 30) {        // If the pattern is a indivual zone pattern
        ZpatternRoutine (thisZone) ;                           // run Zone Pattern routine
      } else {                                                 // must be a solid colour so
        colourRoutine   (thisZone) ;                           // run colour routine for this zone
      }
    } else {                                                   // otherwise the zonestate must be "Off"
      if (zoneStateChanged [thisZone] == true) {               // so if we just changed to "Off"
        colourRoutine (thisZone) ;                             // turn off the pixels in this zone
      }
    }    
  }   

  // display all strips *** I might stop display of "off" strips in the future
  for (uint8_t zoneNum=0; zoneNum < numZones; zoneNum++) strip[zoneNum].show()  ;
  //  }
  // increment the loop counter for group zones - this runs all the time
  if (++GPatternSkips > GPatternSpeed ) {        // is time to ++GPatternCount or are we still skipping it 
    GPatternSkips = 0 ;                          // reset the skip count to 0 
    if (++GPatternCount > maxGPatternCount ) GPatternCount = 0 ; // if GPatternCount has hit the end stop, reset it to 0
  } 
  #if defined Debug 
  ++loopCount ;   // used for debug timing
  #endif
} // main loop code ends here

// **********  sub routines  *********** 

void colourRoutine (uint8_t thisZone) {
  for (uint8_t thisPixel=0; thisPixel < numPixels[thisZone] ; thisPixel++) {
    strip[thisZone].setPixelColor(thisPixel, Colour(thisZone, 
       colourArray [zoneColourOrPattern [thisZone]] [Red], 
       colourArray [zoneColourOrPattern [thisZone]] [Green], 
       colourArray [zoneColourOrPattern [thisZone]] [Blue] ) 
    );
  }
  zoneStateChanged [thisZone] = false ; // stop this running all the time
}

// Zone pattern routine
void ZpatternRoutine (uint8_t thisZone) {
  uint8_t pattern = zoneColourOrPattern [thisZone] - 30 ;  // convert 31-59 to the actual pattern number of 1 - 29 
  if (zoneStateChanged [thisZone] == true) {  // if this is the first time for this pattern
    zoneStateChanged [thisZone] = false ;     // reset the flag and
    zonePatternCount [thisZone] = 0 ;         // set the counter to 0
  }
  switch (pattern) {    // based on the pattern number execute the relevant pattern code 
    case 1:  // pattern 1 / 31 : Rainbow spanning length of string
      for (uint8_t thisPixel = 0; thisPixel < numPixels[thisZone]; thisPixel++) {
        strip[thisZone].setPixelColor(thisPixel, Wheel( thisZone,  ((thisPixel * 256 / numPixels[thisZone] ) + zonePatternCount [thisZone]) % 256) );
      } ;
      if (++zonePatternSkips [thisZone] > zonePatternSpeed [thisZone] ) {  // increment skips until it's time to 
        zonePatternSkips [thisZone] = 0 ;                                  // start the count from 0 again
        if (++zonePatternCount [thisZone] > 255 ) zonePatternCount [thisZone] = 0 ;  // pointless as after 255 is 0 for uint8_t 
      }
    break ;
    case 2: // pattern 2 / 32 : white dot running length of string
      for (uint8_t thisPixel = 0; thisPixel < numPixels[thisZone]; thisPixel++) {
        if (thisPixel == zonePatternCount [thisZone]) { 
        strip[thisZone].setPixelColor(thisPixel, Colour (thisZone,255,255,255) );
        } else {
          strip[thisZone].setPixelColor(thisPixel, Colour (thisZone,0,0,0) );
        }
      } ;
      if (++zonePatternSkips [thisZone] > zonePatternSpeed [thisZone] ) {
        zonePatternSkips [thisZone] = 0 ; 
        if (++zonePatternCount [thisZone] >= numPixels[thisZone] ) zonePatternCount [thisZone] = 0 ;  // 
      }
    break ;
    case 3: // pattern 3 / 33 : white dot running length of string
      for (uint8_t thisPixel = 0; thisPixel < (numPixels[thisZone] / 2 + 1); thisPixel++) {
        if (thisPixel == zonePatternCount [thisZone]) { 
        strip[thisZone].setPixelColor(thisPixel, Colour (thisZone,255,255,255) );
        strip[thisZone].setPixelColor(numPixels[thisZone] - thisPixel -1, Colour (thisZone,255,255,255) );
        } else {
          strip[thisZone].setPixelColor(thisPixel, Colour (thisZone,0,0,0) );
          strip[thisZone].setPixelColor(numPixels[thisZone] - thisPixel -1, Colour (thisZone,0,0,0) );
        }
      } ;
      if (++zonePatternSkips [thisZone] > zonePatternSpeed [thisZone] ) {
        zonePatternSkips [thisZone] = 0 ; 
        if (++zonePatternCount [thisZone] >= (numPixels[thisZone] / 2 + 1) ) zonePatternCount [thisZone] = 0 ;  // 
      }
    break ;
  }
}

// Group (multizone) pattern routine
void GpatternRoutine (uint8_t thisZone) {
  uint8_t pattern = zoneColourOrPattern [thisZone] - 60 ;
  switch (pattern) {
    case 1: // pattern 1 / 61 rainbow spanning width of room (46 pixels)
      maxGPatternCount = 255 ; // this pattern uses a counter from 0 to 255
      for (uint8_t thisPixel = 0; thisPixel < numPixels[thisZone]; thisPixel++) {
        strip[thisZone].setPixelColor(thisPixel, Wheel(thisZone, ((pixelXY [ getPixelNum(thisZone, thisPixel) ] [xPos] * 256 / 46 ) + GPatternCount) % 256) );
      } ;
    break ;
    case 2: // pattern 2 / 62 rainbow spanning length of room (46 pixels)
      maxGPatternCount = 255 ; // this pattern uses a counter from 0 to 255
      for (uint8_t thisPixel = 0; thisPixel < numPixels[thisZone]; thisPixel++) {
        strip[thisZone].setPixelColor(thisPixel, Wheel(thisZone, ((pixelXY [ getPixelNum(thisZone, thisPixel) ] [yPos] * 256 / 46 ) + GPatternCount) % 256) );
      } ;
    break ;
  }
}

// clean out the serial buffer
void clearSerialBuffer() {
     while (Serial.available()) {
          Serial.read();
     }
}  

// given the zone and pixel, return an LED number from 0 to 168
uint8_t getPixelNum (uint8_t zone, uint8_t pixel) {
  uint8_t pixelNum = 0 ;
  for (uint8_t thisZone = 0; thisZone < zone ; thisZone++) {
    pixelNum += (numPixels[thisZone] ) ;
  }
  return pixelNum + pixel ;   
}


#if false
// Adafruit Colo(u)r Wipe example (Currently not called) 
void ColourWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip[1].numPixels(); i++) {
      strip[1].setPixelColor(i, c);
      strip[1].show();
      delay(wait);
  }
}
#endif


uint32_t Colour(uint8_t thisZone, uint8_t r, uint8_t g, uint8_t b)
{
  // Create a 24bit colour from the 3 8bit RGB levels suitable for input into ws2801 
  // Adapted from Adafruit example
  // I need to tidy this code and maybe also allow calibration of the colours
  // (Blue is a bit brighter than green and a lot brighter than red) 
  // 
  
  uint8_t rlev = (float) zoneBrightnessLevel [thisZone]  / 99 * r ;        // calibrated @ 100%
  uint8_t glev = (float) zoneBrightnessLevel [thisZone]  / 99 * g * 0.63 ; // calibrated @  63%
  uint8_t blev = (float) zoneBrightnessLevel [thisZone]  / 99 * b * 0.25 ; // calibrated @  25%
  
  // shift each colour into a unsigned long making a 24bit RGB code

  uint32_t c;  // ################################
  c = rlev;    // ########################<-red-->
  c <<= 8;     // ################<-red-->########
  c |= glev;   // ################<-red--><green=>
  c <<= 8;     // ########<-red--><green=>########
  c |= blev;   // ########<-red--><green-><-blue->
  return c;
}


// Receive zone and position in colour wheel (adapted from Adafruit example)
// The colours are a transition red -> yellow -> green -> cyan -> blue -> magenta -> back to red
uint32_t Wheel(uint8_t thisZone, byte WheelPos) {
  if (WheelPos < 85) {
    return Colour(thisZone, WheelPos * 3      , 255 - WheelPos * 3, 0                 );
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return Colour(thisZone, 255 - WheelPos * 3, 0                 , WheelPos * 3      );
  } else {
    WheelPos -= 170; 
    return Colour(thisZone, 0                 , WheelPos * 3      , 255 - WheelPos * 3);  }  
}

#if defined Debug 
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif

//void printHexString (String s) {
//  for (int i=0; i < s.length() ; i++){
//    Serial.print(s.charAt(i), HEX);
//    Serial.print(" ");
//  } 
//}  

ISR(TIMER2_OVF_vect) {
  // Start of Interrupt Routine
  // This runs ~30 times a second
  static int slowFlashCount = 15 ;   // timer counts for slow flashes  
  static int fastFlashCount = 3  ;   // timer counts for fast flashes  

  // System Heartbeat LED - double flash every second  
  digitalWrite(heartbeatLEDpin, (slowFlash && fastFlash)); 

  // start of main interrupt code
  // If addition ineterrupt code is required, place it here
  // end   of main ineterrupt code  
  
  // maintain flash count timers and flash variables 
  if (slowFlashCount) {
    --slowFlashCount ;
  } else {
    slowFlashCount = 15 ;
    slowFlash = !slowFlash ;
  }  

  if (fastFlashCount) {
    --fastFlashCount ;
  } else {
    fastFlashCount = 3 ;
    fastFlash = !fastFlash ;
  }
  
  #if defined Debug 
  ++intCount  ;
  #endif

}

#if defined Debug 
void dumpTheLot () {

  Serial.println ("Zone(), State, StateChange, ColPat, PattSpd, Brightness ") ;
  for (uint8_t zoneNum=0; zoneNum < numZones; zoneNum++) { 
    Serial.print ("Zone(") ;
    Serial.print (zoneNum) ;
    Serial.print ("), ") ;
    Serial.print (zoneState [zoneNum]) ;             Serial.print (", ") ;
    Serial.print (zoneStateChanged [zoneNum]) ;      Serial.print (", ") ;
    Serial.print (zoneColourOrPattern [zoneNum]) ;   Serial.print (", ") ;
    Serial.print (zonePatternSpeed [zoneNum]) ;      Serial.print (", ") ;
    Serial.print (zoneBrightnessLevel [zoneNum]) ;   Serial.print (", ") ;
    Serial.println () ;
  }
    Serial.print ("GPatternCount, GPatternSpeed :");
    Serial.print (GPatternCount);    
    Serial.print (", ");
    Serial.println (GPatternSpeed);

}
#endif
