#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// for debugging porpuses
// String ex_response = "{\"success\":{\"total\":1},\"contents\":{\"id\":\"U24_S1Qxu1J87ke9Y3kSrweF\",\"testament\":\"Old Testament\",\"book\":\"Micah\",\"bookid\":33,\"chapter\":2,\"verse\":\"In that day shall one take up a parable against you, and lament with a doleful lamentation, and say, We be utterly spoiled: he has changed the portion of my people: how has he removed it from me! turning away he has divided our fields.\",\"title\":\"Bible Verse of the day\",\"category\":\"vod\",\"date\":\"2025-01-28\"},\"copyright\":{\"url\":\"https://quotes.rest\",\"year\":\"2025\"}}";

String verse_text, book;
int chapter, verse_num;

bool weHaveVerse;

// split the verse into lines that fit the TFT screen (chunksize) and get the length of the longest one
void splitStringIntoChunks(String &inputString, int chunkSize, char* buffer, int &chunkNumber, int &maxLineLength) 
{
  inputString.replace("\n"," ");

  int startIndex = 0;
  int spaceIndex;
  String currentChunk = "";

  while (startIndex < inputString.length()) 
  {
    spaceIndex = inputString.indexOf(' ', startIndex);  // Find the next space

    // If there's no space, we've reached the end of the sentence
    if (spaceIndex == -1) {
      spaceIndex = inputString.length();
    }

    // Extract word
    String word = inputString.substring(startIndex, spaceIndex);

    // If adding the word would exceed the chunk size, print the current chunk and start a new one
    if (currentChunk.length() + word.length() > chunkSize) {
      // Convert the current chunk to const char* and print or display it
      const char* chunkCStr = currentChunk.c_str();

      if(currentChunk.length() > maxLineLength)
        maxLineLength = currentChunk.length();

      sprintf(buffer, "%s%s\n", buffer, currentChunk.c_str());

      // Reset the current chunk and start a new one
      currentChunk = word;
      chunkNumber += 1;
    } else {
      // Otherwise, append the word to the current chunk
      if (currentChunk.length() > 0) {
        currentChunk += ' ';  // Add space between words
      }
      currentChunk += word;
    }

    startIndex = spaceIndex + 1;  // Move past the space or to the end of the string
  }

  // Print the last chunk if any
  if (currentChunk.length() > 0) {
    const char* chunkCStr = currentChunk.c_str();

    sprintf(buffer, "%s%s\n", buffer, currentChunk.c_str());
  }
/*
  Serial.println("Final buffer: ");
  Serial.print(buffer);
  Serial.print("Max length: ");
  Serial.println(maxLineLength);
*/
}

// Draw the unexpected HTTP Code in the bottom right corner
// of the TFT.
void drawUnexpectedCode(int unexpectedCode)
{
  int width = tft.width();
  int height = tft.height();
  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_RED);

  char buffer[50];
  memset(buffer, 0, sizeof(buffer));

  // If we dont have a verse saved, draw error screen.
  // if we DO have a verse, draw that verse and the http error code on the bottom
  if(!weHaveVerse)
  {
    tft.fillRect(0, SCREEN_Y_TIMEDATE, 320, height - 110, TFT_BLACK);

    sprintf(buffer, "Error requesting verse. Unexpected HTTP Code.");
    tft.drawString("Unexpected HTTP Code", 90, 135);
    tft.drawString("Error requesting verse", 90, 150);
  }

  tft.fillRect(160,240 - tft.fontHeight(), 160, tft.fontHeight(), TFT_BLACK);

  sprintf(buffer, "HttpCode: %d", unexpectedCode);
  tft.drawString(buffer, tft.width() - tft.textWidth("B") * strlen(buffer), 240 - tft.fontHeight());

  Serial.println("Unexpected Code.");
  Serial.print("---Buffer lenght: ");
  Serial.print(strlen(buffer));
  Serial.print(", X: ");
  Serial.println(tft.width() - tft.textWidth("B") * strlen(buffer));
}

bool extractVerse(String api_response)
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, api_response);

  if(error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  
  book = doc["random_verse"]["book_id"] | "";
  chapter = doc["random_verse"]["chapter"] | 0;
  verse_num = doc["random_verse"]["verse"] | 0;
  verse_text = doc["random_verse"]["text"] | "";

  Serial.print("Free heap memory: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");

  return true;
}

bool getdailyVerse()
{
  bool getSuccess;
    // Make an HTTP GET request
  if (WiFi.status() == WL_CONNECTED) { // Check if connected to Wi-Fi
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate verification (useful for development)

    HTTPClient http;

    const String url = "https://bible-api.com/data/kjv/random/NT";
    http.begin(client, url); // Initialize HTTPClient with URL

    // Attach the API token as a header
    //http.addHeader("Authorization", String("Bearer ") + apiToken);

    //Serial.print("Authorization: ");
    //Serial.println(String("Bearer ") + apiToken);

    // uncomment this line and the ex_response on the beggining of the file for debugging purposes
    //extractVerse(ex_response);
    //drawUnexpectedCode(111); //costum code for knowing when the example verse is showing
    
    int httpCode = http.GET(); // Perform GET request

    //int httpCode = 200;

    // Check HTTP response code
    if (httpCode > 0) {
      Serial.printf("HTTP GET Code: %d\n", httpCode);
      
      if (httpCode == HTTP_CODE_OK) // If response code is 200
      {
        String payload = http.getString(); // Get the response payload
        Serial.println("Response:");
        Serial.println(payload); // Print the response

        if(extractVerse(payload))
        {
          // fill the right half of the bottom of the txt withblack background
          // to eliminate the htpp code drawing that is only for error codes
          tft.fillRect(160,240 - tft.fontHeight(), 160, tft.fontHeight(), TFT_BLACK);
          Serial.println("Filling BOTTOM RIGHT with black.");
          
          //extractVerse(ex_response); for debuggin
          weHaveVerse = true;
          getSuccess = true;
        }

      } else {
        Serial.printf("Unexpected HTTP code: %d\n", httpCode);
        String response = http.getString(); // Print server's response
        Serial.println(response);

        // draw example verse
        //extractVerse(ex_response);

        // draw unexpected code in display
        drawUnexpectedCode(httpCode);

        getSuccess = false;
      }
           
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
      getSuccess = false;
    }

    http.end(); // Free resources*/
  } else {
    Serial.println("Wi-Fi not connected");
    getSuccess = false;
  }

  return getSuccess;
}

void drawVerse()
{
  int width = tft.width();
  int height = tft.height();
  int margin = 5; // size of margins left at both sides of the text.

  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  int charWidth = tft.textWidth("B");
  int charHeigth = tft.fontHeight();
  int textY = SCREEN_Y_TIMEDATE;

  // the chunksize will be the space left for the text divided by the width of a 
  // character:
  //        chunksize = (Text_space_width)/(charWidth)
  // where the Text_space_width will be the screen width minus the margin:
  //        Text_space_width = width - margin * 2

  int chunkSize = (width - margin * 2) / charWidth;

  char buffer[400];
  memset(buffer, 0, sizeof(buffer));
  int lines = 0;
  int maxLineLen = 0;
  splitStringIntoChunks(verse_text, chunkSize, buffer, lines, maxLineLen);
  
  // convert buffer to string for drawing
  String verseInLines(buffer);

  // calculate new horiz margin:
  //    new_margin = margin + (screen_width - Length_biggest_line) * charwidth / 2
  margin += (width - (maxLineLen - 2) * charWidth);

  // calculate new vertical margin:
  //    new_vert_margin = ((screen_heigth - textY) - (charHeigth + 1 ) * lines )/2
  textY += ( (height - textY) - (charHeigth + 1) * (lines + 2) ) / 2;

  // draw sentences in different lines in the tft
  int startIndex = 0;
  int spaceIndex;

  tft.fillRect(0, SCREEN_Y_TIMEDATE, 320, height - 110, TFT_BLACK);

  tft.drawLine(0, SCREEN_Y_TIMEDATE , 340, SCREEN_Y_TIMEDATE, 0xFFFF);

  for (int i = 0; i<=lines; i++)
  {
    spaceIndex = verseInLines.indexOf('\n', startIndex);

    // If there's no endline, we've reached the end of the sentence
    if (spaceIndex == -1) {
      spaceIndex = verseInLines.length();
    }

    // Extract line
    String line = verseInLines.substring(startIndex, spaceIndex);  
    startIndex = spaceIndex + 1;  // Move past the space or to the end of the string

    // for debuggin
    //Serial.println(line); 

    tft.drawString(line, margin, textY + (charHeigth + 1) * i);
  }

  //tft.drawString("+---+", 150, 70);

  // ******** BOOK, CHAPTER and VERSE
  tft.setTextDatum(TR_DATUM);
  tft.setTextFont(1);
  tft.setTextSize(1);

  char bookChap[50];
  memset(bookChap, 0, sizeof(bookChap));

  sprintf(bookChap, "%s, %d:%d", book, chapter, verse_num);

  //tft.fillRect(0, height / 2, 320, charHeigth * 2, TFT_SKYBLUE);

  tft.drawString(bookChap, width - margin, textY + (charHeigth + 1) * (lines + 1) + 3);
}

// draw the number of requests done to the bible api
// in the bottom left corner of the tft. This is for
// monitoring purposes.
void drawNumbApiRequests(int numReqsts)
{
  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY);
  
  char buffer[50];
  memset(buffer, 0, sizeof(buffer));
  // fill the half of the bottom of the txt withblack background
  tft.fillRect(0,240 - tft.fontHeight(), 160, tft.fontHeight(), TFT_BLACK);

  sprintf(buffer, "Api Requests: %d", numReqsts);
  tft.drawString(buffer, 0, 240 - tft.fontHeight());
}

void drawdailyVerse(const bool &forceRequest)
{
  if(forceRequest)
  {
    // we try to get a new verse, if this fails and we have
    // one saved, draw that one and show error http code
    if(getdailyVerse() || weHaveVerse)
    {
      drawVerse();
    }
  }
  else{
    // if the day hasnt changed of it isnt the first time
    // we check if we have a verse already to prioritize that one
    //          if not, we try to get one or draw error http code (see getdailyVerse)
    if(weHaveVerse || getdailyVerse())
    {
      drawVerse();
    }
  }
}