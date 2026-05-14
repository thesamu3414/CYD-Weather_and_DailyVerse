/*******************************************************************
    /* === CYD ROLLING CLOCK ===
    This example shows a digital clock with a rolling effect as the digits change.
    Most of the code are borrowed from other examples. Thanks Internet!


    Brian:
    Took the rolling clock example and added the following to make it webflashable

    - Wifi manager for configuring
    - Double reset detector for entering config mode
    - Saving and loading config
    - NTP and Timezones

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include "token.h"
#include "genericBaseProject.h"
#include "RollingClockLogic.h"

#include "weather.h"
#include "bibleVerse.h"
#include "screensMngr.h"


void setup()
{
    Serial.begin(115200);

    baseProjectSetup();
    // You will be fully connected by the time you are here

    rollingClockSetup(projectConfig.twentyFourHour, projectConfig.usDateFormat);
}



unsigned long lastUpdateTime = 0;
const int updateInterval = 1000;
bool first = true;

void loop()
{
    baseProjectLoop();

    //checkAndDrawTouch();

    unsigned long currentTime = millis();

    if (first)
    {
        first = false;
        Serial.println("Entering first time");
        drawRollingClock();
        //drawBibleVerseScreen(true);
        drawWeatherScreen(true);

        wDay =  myTZ.weekday();
    }
    else if (currentTime - lastUpdateTime >= updateInterval)
    {
        if (minuteChanged() )
        {
            Serial.println("Minute change");
            drawRollingClock();

            if(currentScreen == SCREEN_WEATHER && 
                (myTZ.minute() == 30 || myTZ.minute() == 0))
            {
                //drawdailyVerse();
                drawWeatherScreen(true);
            }
        }
        else if (dayChanged())
        {
            Serial.println("Day change");
            if(currentScreen == SCREEN_BIBLE_VERSE)
            {
                //drawdailyVerse();
                drawBibleVerseScreen(true);
            }
            else if (currentScreen == SCREEN_WEATHER)
            {
                drawWeatherScreen(true);
            }
        }
    }

    checkTouchForNavigation();
    /*
    Serial.print(" outside - Free heap memory: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");*/
}