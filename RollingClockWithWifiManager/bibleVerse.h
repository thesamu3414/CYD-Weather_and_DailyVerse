#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// for debugging porpuses
//String ex_response = "{\"success\": {\"total\": 1},\"contents\": {\"id\": \"Rj5RwO0mA6_tOSPJQFMagQeF\",\"testament\": \"Old Testament\",\"book\": \"Micah\",\"bookid\": 33,\"chapter\": 2,\"verse\": \"And they covet fields, and take them by violence; and houses, and take them away: so they oppress a man and his house, even a man and his heritage.\",\"title\": \"Bible Verse of the day\",\"category\": \"vod\",\"date\": \"2025-01-26\"},\"copyright\": {\"url\": \"https://quotes.rest\",\"year\": \"2025\"}}";

String verse, book;
int chapter;

// split the verse into lines that fit the TFT screen (chunksize) and get the length of the longest one
void splitStringIntoChunks(const String &inputString, int chunkSize, char* buffer, int &chunkNumber, int &maxLineLength) {
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

void extractVerse(String api_response)
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, api_response);

  if(error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  verse = doc["contents"]["verse"] | "";
  book = doc["contents"]["book"] | "";
  chapter = doc["contents"]["chapter"] | 0;

  Serial.print("Free heap memory: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
}

void getdailyVerse()
{
    // Make an HTTP GET request
  if (WiFi.status() == WL_CONNECTED) { // Check if connected to Wi-Fi
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate verification (useful for development)

    HTTPClient http;

    const String url = "https://quotes.rest/bible/vod.json?";
    http.begin(client, url); // Initialize HTTPClient with URL

    // Attach the API token as a header
    http.addHeader("Authorization", String("Bearer ") + apiToken);

    Serial.print("Authorization: ");
    Serial.println(String("Bearer ") + apiToken);

    // uncomment this line and the ex_response on the beggining of the file for debugging purposes
    //extractVerse(ex_response);
    ///*
    int httpCode = http.GET(); // Perform GET request

    // Check HTTP response code
    if (httpCode > 0) {
      Serial.printf("HTTP GET Code: %d\n", httpCode);
      
      if (httpCode == HTTP_CODE_OK) { // If response code is 200
        String payload = http.getString(); // Get the response payload
        Serial.println("Response:");
        Serial.println(payload); // Print the response

        extractVerse(payload);

      } else {
        Serial.printf("Unexpected HTTP code: %d\n", httpCode);
        String response = http.getString(); // Print server's response
        Serial.println(response);
      }
           
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Free resources*/
  } else {
    Serial.println("Wi-Fi not connected");
  }
}

void drawVerse()
{
  int width = tft.width();
  int height = tft.height();
  int margin = 5; // size of margins left at both sides of the text.

  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(2);
  tft.setTextSize(1);
  int charWidth = tft.textWidth("B");
  int charHeigth = tft.fontHeight();
  int textY = 80;

  // the chunksize will be the space left for the text divided by the width of a 
  // character:
  //        chunksize = (Text_space_width)/(charWidth)
  // where the Text_space_width will be the screen width minus the margin:
  //        Text_space_width = width - margin * 2

  int chunkSize = (width - margin * 2) / charWidth;

  char buffer[200];
  int lines = 0;
  int maxLineLen = 0;
  splitStringIntoChunks(verse, chunkSize, buffer, lines, maxLineLen);
  
  // convert buffer to string for drawing
  String verseInLines(buffer);

  // calculate new horiz margin:
  //    new_margin = margin + (screen_width - Length_biggest_line) * charwidth / 2
  margin += (width - (maxLineLen - 2) * charWidth);

  // calculate new vertical margin:
  //    new_vert_margin = ((screen_heigth - textY) - (charHeigth + 1 ) * lines )/2
  textY += ( (height - textY) - (charHeigth + 1) * (lines + 1) ) / 2;

  // draw sentences in different lines in the tft
  int startIndex = 0;
  int spaceIndex;

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

    tft.drawString(line, margin, textY + (charHeigth + 1) * i);
  }

  //tft.drawString("+---+", 150, 80);


  // ******** BOOK and CHAPTER
  tft.setTextDatum(TR_DATUM);
  tft.setTextFont(1);
  tft.setTextSize(1);

  char bookChap[50];

  sprintf(bookChap, "%s, %d.", book, chapter);


  //tft.fillRect(0, height / 2, 320, charHeigth * 2, TFT_SKYBLUE);

  tft.drawString(bookChap, width - margin, textY + (charHeigth + 1) * (lines + 1) + 3);
}

void drawdailyVerse()
{
    getdailyVerse();
    drawVerse();
}