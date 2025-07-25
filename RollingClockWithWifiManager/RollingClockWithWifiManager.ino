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

void loop()
{
    baseProjectLoop();

    //checkAndDrawTouch();

    unsigned long currentTime = millis();

    if (first)
    {
        Serial.println("Entering first time");
        drawRollingClock();
        drawBibleVerseScreen();

        wDay =  myTZ.weekday();
        first = false;
    }
    else if (currentTime - lastUpdateTime >= updateInterval)
    {
        if (minuteChanged())
        {
            Serial.println("Minute change");
            drawRollingClock();
        }
        else if (dayChanged())
        {
            Serial.println("Day change");
            daychanged = true;
            //drawdailyVerse();
        }
    }

    checkTouchForNavigation();
    /*
    Serial.print(" outside - Free heap memory: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");*/
}