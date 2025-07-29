
enum ScreenState {
  SCREEN_WEATHER,
  SCREEN_BIBLE_VERSE
};

// integer tracking what screen is up
ScreenState currentScreen = SCREEN_BIBLE_VERSE; 

// Navigation arrows coordinates
//
struct Point {
    int x;
    int y;
};

// left arrow
Point Lp1 = {7,  125};  // center vertex
Point Lp2 = {22, 135};  // top vertex
Point Lp3 = {22, 115};  // bottom vertex

// right arrow
//  using height() because at the time that the code passes through here
//  the screen hasnt rotated yet (which does in displaySetup())
Point Rp1 = {tft.height() - 7,  125};
Point Rp2 = {tft.height() - 22, 135};
Point Rp3 = {tft.height() - 22, 115};


void drawNavigationRightArrow() 
{  
    //tft.drawRect(Rp3.x - 4, Rp3.y - 4, 18, 18, TFT_WHITE);

    // Right Arrow
    tft.fillTriangle(Rp1.x, Rp1.y, Rp2.x, Rp2.y, Rp3.x, Rp3.y, TFT_WHITE);

    Serial.print("Draw Right arrow. Rp1: ");
    Serial.print(Rp1.x);
    Serial.print(", ");
    Serial.println(Rp1.y);

}

void drawNavigationLeftArrow() 
{  
  // Left Arrow
  tft.fillTriangle(Lp1.x, Lp1.y, Lp2.x, Lp2.y, Lp3.x, Lp3.y, TFT_WHITE);
}


void drawNavigationArrows() {
  // Left Arrow
  drawNavigationLeftArrow();
  
  // Right Arrow
  drawNavigationRightArrow();
}

void drawWeatherScreen() {
  //tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 90, 320, tft.height() - 110, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Weather Today", 110, 115);

  // draw weather data...
  
  //drawNavigationArrows();
}

void drawBibleVerseScreen() {
  //tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, SCREEN_Y_TIMEDATE, 320, tft.height() - 70, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);

  drawdailyVerse();

  //drawNavigationArrows();
}

void navigateLeft() {
  if (currentScreen == SCREEN_BIBLE_VERSE) 
  {
    currentScreen = SCREEN_WEATHER;
    drawWeatherScreen();
  }
  else if (currentScreen == SCREEN_WEATHER)
  {
    currentScreen = SCREEN_BIBLE_VERSE;
    drawBibleVerseScreen();
  }
}

void navigateRight() {
  if (currentScreen == SCREEN_WEATHER) {
    currentScreen = SCREEN_BIBLE_VERSE;
    drawBibleVerseScreen();
  }
  else if (currentScreen == SCREEN_BIBLE_VERSE) 
  {
    currentScreen = SCREEN_WEATHER;
    drawWeatherScreen();
  }
}

void checkTouchForNavigation() {
  if (ts.tirqTouched() && ts.touched()) {

    TS_Point p = getRemapedPoint(ts);

    printTouchToDisplay(p);

    // Bottom part of screen touched
    if (p.y > 90 ) {
      navigateRight();
    }

    delay(100);
  }
}
