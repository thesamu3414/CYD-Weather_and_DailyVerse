#include "LittleFS.h"

void showFilesInSystem()
{
    File root = LittleFS.open("/");

    File file = root.openNextFile();

    Serial.println("Files in system now:");
    while(file){
        Serial.print("-");
        Serial.print(file.name());
        Serial.print("\tSize: ");
        Serial.println(file.size()); 

        file = root.openNextFile();
    }

    root.close();
    file.close();

}