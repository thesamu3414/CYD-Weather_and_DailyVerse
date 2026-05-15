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
unsigned long lastScreenChangeTime = 0;
const int updateInterval = 2000;                   // 2 secs
const unsigned long screenChangeInterval = 300000; // 5 mins
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
        drawBibleVerseScreen(true);
        requestWeather();

        wDay =  myTZ.weekday();
    }
    else if (currentTime - lastUpdateTime >= updateInterval)
    {
        lastUpdateTime = currentTime;

        if (minuteChanged() )
        {
            Serial.println("Minute change");
            drawRollingClock();

            if(myTZ.minute() % 10 == 0)
            {
                if(currentScreen == SCREEN_WEATHER)
                {
                    drawWeatherScreen(true);
                }
                else
                {
                    requestWeather();
                }
            }
        }
        else if (dayChanged())
        {
            Serial.println("Day change");
            if(currentScreen == SCREEN_BIBLE_VERSE)
            {
                drawBibleVerseScreen(true);
                requestWeather();
            }
            else if (currentScreen == SCREEN_WEATHER)
            {
                drawWeatherScreen(true);
                getdailyVerse();
            }
        }
        if (currentTime - lastScreenChangeTime >= screenChangeInterval)
        {
            lastScreenChangeTime = currentTime;
            navigateRight();
        }
    }

    checkTouchForNavigation(lastScreenChangeTime);
    
    //Serial.print(" outside - Free heap memory: ");
    //Serial.print(ESP.getFreeHeap());
    //Serial.println(" bytes");
}