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

void setup()
{
    Serial.begin(115200);

    baseProjectSetup();
    // You will be fully connected by the time you are here

    rollingClockSetup(projectConfig.twentyFourHour, projectConfig.usDateFormat);
}

bool first = true;
int apiRequests = 0;

void loop()
{
    baseProjectLoop();

    if (first)
    {
        Serial.println("Entering first time");
        drawRollingClock();
        drawdailyVerse();

        wDay =  myTZ.weekday();
        first = false;
        apiRequests += 1;
        drawNumbApiRequests(apiRequests);
    }
    else if (minuteChanged())
    {
        Serial.println("Minute change");
        drawRollingClock();
    }
    else if (dayChanged())
    {
        Serial.println("Day change");
        apiRequests += 1;
        drawdailyVerse();
        drawNumbApiRequests(apiRequests);
    }

    /*
    Serial.print(" outside - Free heap memory: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");*/
    delay(1000);
}