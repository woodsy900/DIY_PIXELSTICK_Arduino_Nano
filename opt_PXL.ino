// Adafruit NeoPixel - Version: 1.1.6
#include <Adafruit_NeoPixel.h>

// Adafruit_SSD1306-1.1.2 - Version: Latest 
#include <Adafruit_SSD1306.h>

// SD - Version: Latest 
#include <SD.h>


  
// Pin assignments for the Arduino (Make changes to these if you use different Pins)
#define SDssPin 53                        // SD card CS pin
byte NPPin = 31;                           // Data Pin for the NeoPixel LED Strip
byte AuxButton = 44;                       // Aux Select Button Pin
byte AuxButtonGND = 45;                    // Aux Select Button Ground Pin
byte g = 0;                                // Variable for the Green Value
byte b = 0;                                // Variable for the Blue Value
byte r = 0;                                // Variable for the Red Value

// Intial Variable declarations and assignments (Make changes to these if you want to change defaults)
#define STRIP_LENGTH 144                  // Set the number of LEDs the LED Strip
byte frameDelay = 15;                      // default for the frame delay 
int frameBlankDelay = 0;                  // Default Frame blank delay of 0
byte menuItem = 1;                         // Variable for current main menu selection
byte initDelay = 0;                        // Variable for delay between button press and start of light sequence
byte repeat = 0;                           // Variable to select auto repeat (until select button is pressed again)
int repeatDelay = 0;                      // Variable for delay between repeats
int updateMode = 0;                       // Variable to keep track of update Modes
byte repeatTimes = 1;                      // Variable to keep track of number of repeats
byte brightness = 90;                      // Variable and default for the Brightness of the strip
byte interuptPressed = 0;                  // variable to keep track if user wants to interupt display session.
int loopCounter = 0;                       // count loops as a means to avoid extra presses of keys
boolean cycleAllImages = false;            //cycle through all images
boolean cycleAllImagesOneshot = false;
int cycleImageCount= 0;
// Other program variable declarations, assignments, and initializations
byte x;

//OLED details
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define LOGO16_Gdisplay_HEIGHT 16 
#define LOGO16_Gdisplay_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };
  
#define SSD1306_displayHEIGHT 64
#if (SSD1306_displayHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Declaring the two LED Strips and pin assignments to each 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LENGTH, NPPin, NEO_GRB + NEO_KHZ800);

// Variable assignments for the Keypad
int adc_key_val[5] ={ 0, 331, 509, 145, 744 };  
byte NUM_KEYS = 5;
int adc_key_in;
char key=-1;
char oldkey=-1;

// SD Card Variables and assignments
File root;
File dataFile;
String m_CurrentFilename = "";
byte m_FileIndex = 0;
byte m_NumberOfFiles = 0;
String m_FileNames[10];                  
long buffer[STRIP_LENGTH];

#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

// Setup loop to get everything ready.  This is only run once at power on or reset
void setup() {
  
  pinMode(AuxButton, INPUT);
  digitalWrite(AuxButton,HIGH);
  pinMode(AuxButtonGND, OUTPUT);
  digitalWrite(AuxButtonGND,LOW);

  setupLEDs();
  setupSDcard();
  setupOLED();
  byte i;
  for (i = 0; i < m_NumberOfFiles; i++) {     // Loop through all of the files that have been identified on the memory card
    if(m_FileNames[i] == "STARTUP.BMP"){      // Look for a default file to show on initial bootup This file name could be changed to "STARTUP.BMP"
      SendFile(m_FileNames[i]);               // if the startup BMP was found, send it to the LED's
      ClearStrip(0);                          // Clear the strip
      break;                                  // Exit for loop, we found our default file
    }
  }
}

     
// The Main Loop for the program starts here... 
// This will loop endlessly looking for a key press to perform a function
void loop() {
  switch (menuItem) {
    case 1:
      display.begin(16,2);
      display.println( F("1:File Select   "));
      display.setCursor(0, 1);
      display.println(m_CurrentFilename);
      break;    
    case 2:
      display.begin(16,2);
      display.println( F("2:Brightness    "));
      display.setCursor(0, 1);
      display.println(brightness);
      if (brightness == 100) {
        display.setCursor(3, 1);
      }
      else {
        display.setCursor(2, 1);
      }
       display.println( F("%"));
      break;    
    case 3:
      display.begin(16,2);
      display.println( F("3:Init Delay    "));
      display.setCursor(0, 1);
      display.println(initDelay);    
      break;    
    case 4:
      display.begin(16,2);
      display.println( F("4:Frame Delay   "));
      display.setCursor(0, 1);
      display.println(frameDelay);    
      break;    
    case 5:
      display.begin(16,2);
      display.println( F("5:Repeat Times  "));
      display.setCursor(0, 1);
      display.println(repeatTimes);    
      break;    
    case 6:
      display.begin(16,2);
      display.println( F("6:Repeat Delay  "));
      display.setCursor(0, 1);
      display.println(repeatDelay);    
      break;    

  }
  
  int keypress = ReadKeypad();
  delay(50);
  
  if ((keypress == 4) ||(digitalRead(AuxButton) == LOW)) {    // The select key was pressed
    delay(initDelay);
    if (repeatTimes > 1) {
      for (byte x = repeatTimes; x > 0; x--) {
        SendFile(m_CurrentFilename);
        delay(repeatDelay);
      }
    }
    else {
      SendFile(m_CurrentFilename);
    }
    ClearStrip(0);
  }
  if (keypress == 0) {                    // The Right Key was Pressed
    switch (menuItem) { 
      case 1:                             // Select the Next File
         
        if (m_FileIndex < m_NumberOfFiles -1) {
          m_FileIndex++;
        }
        else {
          m_FileIndex = 0;                // On the last file so wrap round to the first file
        }
        DisplayCurrentFilename();
        break;
      case 2:                             // Adjust Brightness
         
        if (brightness < 100) {
          brightness+=1;
        }
        break;
      case 3:                             // Adjust Initial Delay + 1 second
         
        initDelay+=1000;
        break;
      case 4:                             // Adjust Frame Delay + 1 milliseconds 
         
        frameDelay+=1;
        break;
      case 5:                             // Adjust Repeat Times + 1
         
        repeatTimes+=1;
        break;
      case 6:                             // Adjust Repeat Delay + 100 milliseconds
         
        repeatDelay+=100;
        break;
    }
  }

  if (keypress == 3) {                    // The Left Key was Pressed
    switch (menuItem) {                   // Select the Previous File
      case 1:
         
        if (m_FileIndex > 0) {
          m_FileIndex--;
        }
        else {
          m_FileIndex = m_NumberOfFiles -1;    // On the last file so wrap round to the first file
        }
        DisplayCurrentFilename();
        delay(500);        
        break;
      case 2:                             // Adjust Brightness
         
        if (brightness > 1) {
          brightness-=1;
        }
        break;
      case 3:                             // Adjust Initial Delay - 1 second
         
        if (initDelay > 0) {
          initDelay-=1000;
        }
        break;
      case 4:                             // Adjust Frame Delay - 1 millisecond 
         
        if (frameDelay > 0) {
          frameDelay-=1;
        }
        break;
      case 5:                             // Adjust Repeat Times - 1
         
        if (repeatTimes > 1) {
          repeatTimes-=1;
        }
        break;
      case 6:                             // Adjust Repeat Delay - 100 milliseconds
         
        if (repeatDelay > 0) {
          repeatDelay-=100;
        }
        break;
    }    
  }


  if (( keypress == 1)) {                 // The up key was pressed
    if (menuItem == 1) {
      menuItem = 6;  
    }
    else {
      menuItem -= 1;
    }
  }
  if (( keypress == 2)) {                 // The down key was pressed
    if (menuItem == 6) {
      menuItem = 1;  
    }
    else {
      menuItem += 1;
    }
  }
}

void setupLEDs() {
  strip.begin();
  strip.show();
}

void setupOLED(){
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.display();     // show splashscreen
  delay(2000);
  display.clearDisplay(); // Make sure the display is cleared

}

void setupSDcard() {
  pinMode(SDssPin, OUTPUT);
 
  while (!SD.begin(SDssPin)) {
    display.println( F("SD init failed!"));
    delay(1000);
    display.clearDisplay();
    delay(500);
  }
  display.clearDisplay();
  display.println( F("SD init done."));
  delay(1000);
  root = SD.open("/");
  display.clearDisplay();
   display.println( F("Scanning files"));
  delay(500);
  GetFileNamesFromSD(root);
  isort(m_FileNames, m_NumberOfFiles);
  m_CurrentFilename = m_FileNames[0];
  DisplayCurrentFilename();
}
     


int ReadKeypad() {
  adc_key_in = analogRead(0);             // read the value from the sensor  
  digitalWrite(13, HIGH);  
  key = get_key(adc_key_in);              // convert into key press
     
  if (key != oldkey) {                    // if keypress is detected
    delay(100);                            // wait for debounce time
    adc_key_in = analogRead(0);           // read the value from the sensor  
    key = get_key(adc_key_in);            // convert into key press
    if (key != oldkey) {                  
      oldkey = key;
      if (key >=0){
        return key;
      }
    }
  }
  return key;
}
     
     

// Convert ADC value to key number
int get_key(unsigned int input) {
  int k;
  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {        
      return k;
    }
  }
  if (k >= NUM_KEYS)
    k = -1;                               // No valid key pressed
  return k;
}


void SendFile(String Filename) {
  char temp[14];
  Filename.toCharArray(temp,14);
     
  dataFile = SD.open(temp);
     
  // if the file is available send it to the LED's
  if (dataFile) {
    ReadTheFile();
    dataFile.close();
    if (interuptPressed >=3){
      //interuptPressed = 0;
      
      delay (100); // add a delay to prevent select button from starting sequence again.
    }
  }  
  else {
    display.clearDisplay();
    display.println( F("Error reading"));
    display.setCursor(4, 1);
    display.println( F("file"));
    delay(1000);
    display.clearDisplay();
    setupSDcard();
    return;
    }
  }



void DisplayCurrentFilename() {
  m_CurrentFilename = m_FileNames[m_FileIndex];
  display.setCursor(0, 1);
   display.println( F("                "));
  display.setCursor(0, 1);
   display.println(m_CurrentFilename);
}


     
void GetFileNamesFromSD(File dir) {
  byte fileCount = 0;
  String CurrentFilename = "";
  while(1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      m_NumberOfFiles = fileCount;
    entry.close();
      break;
    }
    else {
      if (entry.isDirectory()) {
        //GetNextFileName(root);
      }
      else {
        CurrentFilename = entry.name();
        if (CurrentFilename.endsWith(".bmp") || CurrentFilename.endsWith(".BMP") ) { //find files with our extension only
          if(CurrentFilename.startsWith("_")){      // mac sidecar files start with _ and should not be included, may be on card if written from Mac
          }else{
          m_FileNames[fileCount] = entry.name();
          fileCount++;
          }
        }
      }
    }
    entry.close();
  }
}
     
     

void latchanddelay(int dur) {
  strip.show();
  delay(dur);
}



void ClearStrip(int duration) {
  int x;
  for(x=0;x<STRIP_LENGTH;x++) {
    strip.setPixelColor(x, 0);
  }
  strip.show();
}


uint32_t readLong() {
  uint32_t retValue;
  byte incomingbyte;
     
  incomingbyte=readByte();
  retValue=(uint32_t)((byte)incomingbyte);
     
  incomingbyte=readByte();
  retValue+=(uint32_t)((byte)incomingbyte)<<8;
     
  incomingbyte=readByte();
  retValue+=(uint32_t)((byte)incomingbyte)<<16;
     
  incomingbyte=readByte();
  retValue+=(uint32_t)((byte)incomingbyte)<<24;
     
  return retValue;
}



uint16_t readInt() {
  byte incomingbyte;
  uint16_t retValue;
     
  incomingbyte=readByte();
  retValue+=(uint16_t)((byte)incomingbyte);
     
  incomingbyte=readByte();
  retValue+=(uint16_t)((byte)incomingbyte)<<8;
     
  return retValue;
}



int readByte() {
  int retbyte=-1;
  while(retbyte<0) retbyte= dataFile.read();
  return retbyte;
}


void getRGBwithGamma() {
 // g=gamma(readByte())/(101-brightness);
 // b=gamma(readByte())/(101-brightness);
 // r=gamma(readByte())/(101-brightness);
  
  g=gamma(readByte())*(brightness *0.01);          // Brian Heiland Revise.  Old formula
  b=gamma(readByte())*(brightness *0.01);          // reduced brightness 50% when brightness was changed from 
  r=gamma(readByte())*(brightness *0.01);          // 100 to 99 , by 50% brightness was nearly 0 This formula corrects this.
}



void ReadTheFile() {
  #define MYBMP_BF_TYPE           0x4D42
  #define MYBMP_BF_OFF_BITS       54
  #define MYBMP_BI_SIZE           40
  #define MYBMP_BI_RGB            0L
  #define MYBMP_BI_RLE8           1L
  #define MYBMP_BI_RLE4           2L
  #define MYBMP_BI_BITFIELDS      3L

  uint16_t bmpType = readInt();
  uint32_t bmpSize = readLong();
  uint16_t bmpReserved1 = readInt();
  uint16_t bmpReserved2 = readInt();
  uint32_t bmpOffBits = readLong();
  bmpOffBits = 54;
     
  /* Check file header */
 
  /* Read info header */
  uint32_t imgSize = readLong();
  uint32_t imgWidth = readLong();
  uint32_t imgHeight = readLong();
  uint16_t imgPlanes = readInt();
  uint16_t imgBitCount = readInt();
  uint32_t imgCompression = readLong();
  uint32_t imgSizeImage = readLong();
  uint32_t imgXPelsPerMeter = readLong();
  uint32_t imgYPelsPerMeter = readLong();
  uint32_t imgClrUsed = readLong();
  uint32_t imgClrImportant = readLong();
   
  /* Check info header */

     
  byte displayWidth = imgWidth;
  if (imgWidth > STRIP_LENGTH) {
    displayWidth = STRIP_LENGTH;           //only display the number of led's we have
  }
     
     
  /* compute the line length */
  uint32_t lineLength = imgWidth * 3;
  if ((lineLength % 4) != 0)
    lineLength = (lineLength / 4 + 1) * 4;
    


    // Note:  
    // The x,r,b,g sequence below might need to be changed if your strip is displaying
    // incorrect colors.  Some strips use an x,r,b,g sequence and some use x,r,g,b
    // Change the order if needed to make the colors correct.
    
    for(byte y=imgHeight; y > 0; y--) {
      int bufpos=0; 
           if ((interuptPressed <= 3)&& (y <=(imgHeight -5))){     // if the interupt has not been pressed and we are working on column 5 of the image.  (look for no key presses until into 5th column of image)
            int keypress = ReadKeypad();              //Read the keypad, each successive read will be column+1 until 3 is reached.
            if ((keypress == 4) ||(digitalRead(AuxButton) == LOW)) {    // The select key was pressed or the Aux button was pressed
            interuptPressed +=1;                                        // user is trying to interupt display, increment count of frames with button held down.      
          }
         if (interuptPressed >=3){                                      // 3 or more frames have passed with button pressed
            cycleAllImagesOneshot = 0;
            ClearStrip(0);                                               // clear the strip
            break;                                                       // break out of the y for loop.
          }
        }   
      for(byte x=0; x < displayWidth; x++) {                              // Loop through the x  values of the image
        uint32_t offset = (MYBMP_BF_OFF_BITS + (((y-1)* lineLength) + (x*3))) ;  
        dataFile.seek(offset);
        getRGBwithGamma();
        strip.setPixelColor(x,r,b,g);        
      }
    latchanddelay(frameDelay);
    if(frameBlankDelay > 0){
      ClearStrip(0);
      delay(frameBlankDelay);
    }

    
        if (interuptPressed >=3){                                        // this code is unreachable clean up and remove when verified not needed Brian Heiland
         // ClearStrip(0);
         dataFile.close();
          break;
        }
    }
  }



// Sort the filenames in alphabetical order
void isort(String *filenames, byte n) {
  for (byte i = 1; i < n; ++i) {
    String j = filenames[i];
    byte k;
    for (k = i - 1; (k >= 0) && (j < filenames[k]); k--) {
      filenames[k + 1] = filenames[k];
    }
    filenames[k + 1] = j;
  }
}
     

   
PROGMEM const unsigned char gammaTable[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
  11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
  16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
  23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
  30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
  40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
  50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
  62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
  76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
  92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};
 
 
inline byte gamma(byte x) {
  return pgm_read_byte(&gammaTable[x]);
}