#include <XPT2046_Touchscreen.h>
// A library for interfacing with the touch screen
//
// Can be installed from the library manager (Search for "XPT2046")
//https://github.com/PaulStoffregen/XPT2046_Touchscreen

// ----------------------------
// Touch Screen pins
// ----------------------------

// The CYD touch uses some non default
// SPI pins

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// ----------------------------

// Max and min values for remapping 
// ts coordinates
#define TS_XMAX 3946
#define TS_XMIN 142
#define TS_YMAX 3895
#define TS_YMIN 196

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

void startTouchScreen()
{
    // Start the SPI for the touch screen and init the TS library
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);
}

int maxX=0;
int minX=4000;
int maxY=0;
int minY=4000;

void printTouchToSerial(TS_Point p) {
  Serial.print("Pressure = ");
  Serial.print(p.z);
  Serial.print(", x = ");
  Serial.print(p.x);
  Serial.print(", y = ");
  Serial.print(p.y);
  Serial.println();

  Serial.print("               ");
  Serial.print(", maxX = ");
  Serial.print(maxX);
  Serial.print(", minX = ");
  Serial.print(minX);
  Serial.println();

  Serial.print("               ");
  Serial.print(", maxY = ");
  Serial.print(maxY);
  Serial.print(", minY = ");
  Serial.print(minY);
  Serial.println();
}

void printTouchToDisplay(TS_Point p) {

  // Clear screen first
  //tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int x = 0;
  int y = 0;
  int fontSize = 1;
  tft.setTextSize(1);

  String temp = "Pressure = " + String(p.z);
  //tft.drawCentreString(temp, x, y, fontSize);

  //y += 16;
  temp += ", X = " + String(p.x);
  //tft.drawCentreString(temp, x, y, fontSize);

  if (p.x > maxX) {maxX = p.x;}
  else if (p.x < minX) {minX = p.x;}

  if (p.y > maxY) {maxY = p.y;}
  else if (p.y < minY) {minY = p.y;}

  //y += 16;
  temp += ", Y = " + String(p.y);
  tft.drawString(temp, x, y, fontSize);

  // draw an "+" where the touch was made
  tft.drawString("+", p.x, p.y, fontSize);

}

void checkAndDrawTouch()
{
  if (ts.tirqTouched() && ts.touched())
  {
      TS_Point p = ts.getPoint();
      p.x = map(p.x,TS_XMIN,TS_XMAX,0,340);
      p.y = map(p.y,TS_YMIN,TS_YMAX,0,240);

      printTouchToSerial(p);
      printTouchToDisplay(p);
      delay(50);
  }
}