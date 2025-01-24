#include <HTTPClient.h>
#include <WiFiClientSecure.h>


void getWeekVerse()
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

    int httpCode = http.GET(); // Perform GET request

    // Check HTTP response code
    if (httpCode > 0) {
      Serial.printf("HTTP GET Code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) { // If response code is 200
        String payload = http.getString(); // Get the response payload
        Serial.println("Response:");
        Serial.println(payload); // Print the response
      } else {
        Serial.printf("Unexpected HTTP code: %d\n", httpCode);
        String response = http.getString(); // Print server's response
        Serial.println(response);
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Free resources
  } else {
    Serial.println("Wi-Fi not connected");
  }
}

void drawWeekVerse()
{
    getWeekVerse();
}