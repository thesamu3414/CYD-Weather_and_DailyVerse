#include "weather.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
//#include "Free_Fonts.h"

#include "token.h"  // Add this line

bool firstWeatherRequest = true;

bool requestOpenWeather(bool curr, bool forec, bool oneCall)
{
  bool getSuccess;
    // Make an HTTP GET request
  if (WiFi.status() == WL_CONNECTED) { // Check if connected to Wi-Fi
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate verification (useful for development)

    HTTPClient http;
    String fullURL;

    if(curr)
    {
      Serial.printf("weather::requestOpenWeather - CURRENT:\n");
      fullURL = URL_OPENWEATHER_CURRENT_WEATH;
    }
    else if (forec)
    {
      Serial.printf("weather::requestOpenWeather - FORECAST:\n");
      fullURL = URL_OPENWEATHER_FOREC_METRIC_5CNT;
    }
    else if (oneCall)
    {
      Serial.printf("weather::requestOpenWeather - ONE CALL 3.0:\n");
      fullURL = URL_OPENWEATHER_ONE_CALL_3;
    }
    
    fullURL.replace("lat=&", "lat=" + String(LAT_GIJON) + "&");
    fullURL.replace("lon=&", "lon=" + String(LONG_GIJON) + "&");
    fullURL.replace("appid=", "appid=" + String(API_KEY_OPENWEATHER));

    Serial.printf("weather::requestOpenWeather - fullURL: ");
    Serial.println(fullURL);

    http.begin(client, fullURL); // Initialize HTTPClient with URL
    
    int httpCode = http.GET(); // Perform GET request

    // Check HTTP response code
    if (httpCode > 0) {
      Serial.printf("HTTP GET Code: %d\n", httpCode);
      
      if (httpCode == HTTP_CODE_OK) // If response code is 200
      {
        String payload = http.getString(); // Get the response payload
        Serial.println("Response:");
        Serial.println(payload); // Print the response

        if(curr)
        {
          WeatherCurrent& current =
            WeatherCurrent::getInstance();
          current.parseJson(payload);
        }
        else if (forec)
        {
          WeatherForecast& forecast =
            WeatherForecast::getInstance();
          forecast.parseJson(payload);
        }
        else if (oneCall)
        {
          Weather_OneCall_3_0& oneCall_3_0 =
            Weather_OneCall_3_0::getInstance();
          oneCall_3_0.parseJson(payload);
        }

      } else {
        Serial.printf("Unexpected HTTP code: %d\n", httpCode);
        String response = http.getString(); // Print server's response
        Serial.println(response);

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

// One Call API 3.0
bool Weather_OneCall_3_0::parseJson(const String& jsonString) 
{
    DynamicJsonDocument doc(8192);  // Larger size for full forecast
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return false;
    }

    this->clear();

    Serial.println(" Weather_OneCall_3_0::parseJson - 1");

    r_lat = doc["lat"];
    r_long = doc["lon"];
    timezone = doc["timezone"].as<String>();
    timezone_offset = doc["timezone_offset"];

    // CURRENT
    if (doc.containsKey("current"))
    {
      exists_current = true;

      JsonObject currentJSON = doc["current"];
      
      current.dt = currentJSON["dt"];
      current.sunrise = currentJSON["sunrise"];
      current.sunset = currentJSON["sunset"];
      current.temp = currentJSON["temp"];
      current.feels_like = currentJSON["feels_like"];
      current.pressure = currentJSON["pressure"];
      current.humidity = currentJSON["humidity"];
      current.dew_point = currentJSON["dew_point"];
      current.uvi = currentJSON["uvi"];
      current.clouds = currentJSON["clouds"];
      current.visibility = currentJSON["visibility"];
      current.wind_speed = currentJSON["wind_speed"];
      current.wind_deg = currentJSON["wind_deg"];
      current.wind_gust = currentJSON["wind_gust"];

      current.rain_exists = currentJSON.containsKey("rain");
      if(current.rain_exists) 
      {
        current.rain.r_one_hour = currentJSON["rain"]["1h"];
      }

      current.snow_exists = currentJSON.containsKey("snow");
      if(current.rain_exists)
      {
        current.snow.r_one_hour = currentJSON["snow"]["1h"];
      }
      
      // Weather conditions
      JsonArray weatherArray = currentJSON["weather"];
      for (JsonObject w : weatherArray) {
        weather_descrip condition;
        condition.r_id = w["weather"]["r_id"];
        condition.r_main = w["weather"]["r_main"].as<String>();
        condition.r_description = w["weather"]["r_description"].as<String>();
        condition.r_icon = w["weather"]["r_icon"].as<String>();

        current.weather.push_back(condition);
      }
    }

    // Parse HOURLY entries
    if (doc.containsKey("hourly"))
    {
      exists_hourly = true;

      JsonArray listArray = doc["hourly"];
      Serial.println("  Weather_OneCall_3_0::parseJson - HOURLY listArray loop");
      int it = 1;
      for (JsonObject item : listArray)
      {
        Serial.print(" Weather_OneCall_3_0::parseJson - \t iter: ");
        Serial.println(it);
        it++;
        weather_main_3_0_hourly entry;

        entry.dt = item["dt"];
        
        entry.temp        = item["temp"];
        entry.feels_like  = item["feels_like"];
        entry.pressure    = item["pressure"];
        entry.humidity    = item["humidity"];
        entry.dew_point   = item["dew_point"];
        entry.uvi         = item["uvi"];
        entry.clouds      = item["clouds"];
        entry.visibility  = item["visibility"];
        entry.wind_speed  = item["wind_speed"];
        entry.wind_deg    = item["wind_deg"];
        entry.wind_gust   = item["wind_gust"];

        entry.pop         = item["pop"];

        entry.rain_exists = item.containsKey("rain_exists");
        if(entry.rain_exists) 
        {
          entry.rain.r_one_hour = item["rain"]["1h"];
        }

        entry.snow_exists = item.containsKey("snow_exists");
        if(entry.rain_exists)
        {
          entry.snow.r_one_hour = item["snow"]["1h"];
        }

        // Weather conditions
        JsonArray weatherArray = item["weather"];
        for (JsonObject w : weatherArray) {
          weather_descrip condition;
          condition.r_id = w["weather"]["r_id"];
          condition.r_main = w["weather"]["r_main"].as<String>();
          condition.r_description = w["weather"]["r_description"].as<String>();
          condition.r_icon = w["weather"]["r_icon"].as<String>();

          entry.weather.push_back(condition);
        }

        hourly.push_back(entry);
      }
    }

    // Parse DAILY entries
    if (doc.containsKey("daily"))
    {
      exists_daily = true;

      JsonArray listArray = doc["daily"];
      Serial.println("  Weather_OneCall_3_0::parseJson - DAILY listArray loop");
      int it = 1;
      for (JsonObject item : listArray)
      {
        Serial.print(" Weather_OneCall_3_0::parseJson - \t iter: ");
        Serial.println(it);
        it++;
        weather_main_3_0_daily entry;

        entry.dt = item["dt"];

        entry.sunrise     = item["sunrise"];
        entry.sunset      = item["sunset"];
        entry.moonrise    = item["moonrise"];
        entry.moonset     = item["moonset"];
        entry.moon_phase  = item["moon_phase"];
        entry.summary     = item["summary"].as<String>();

        entry.temp.morn   = item["temp"]["morn"];
        entry.temp.day    = item["temp"]["day"];
        entry.temp.eve    = item["temp"]["eve"];
        entry.temp.night  = item["temp"]["night"];
        entry.temp.min    = item["temp"]["min"];
        entry.temp.max    = item["temp"]["max"];

        entry.feels_like.morn  = item["feels_like"]["morn"];
        entry.feels_like.day   = item["feels_like"]["day"];
        entry.feels_like.eve   = item["feels_like"]["eve"];
        entry.feels_like.night = item["feels_like"]["night"];

        entry.pressure    = item["pressure"];
        entry.humidity    = item["humidity"];
        entry.dew_point   = item["dew_point"];
        entry.uvi         = item["uvi"];
        entry.clouds      = item["clouds"];
        entry.wind_speed  = item["wind_speed"];
        entry.wind_deg    = item["wind_deg"];
        entry.wind_gust   = item["wind_gust"];

        entry.pop         = item["pop"];

        entry.rain_exists = item.containsKey("rain");
        if(entry.rain_exists) 
        {
          entry.rain = item["rain"];
        }

        entry.snow_exists = item.containsKey("snow");
        if(entry.rain_exists)
        {
          entry.snow = item["snow"];
        }

        // Weather conditions
        JsonArray weatherArray = item["weather"];
        for (JsonObject w : weatherArray) {
          weather_descrip condition;
          condition.r_id = w["weather"]["r_id"];
          condition.r_main = w["weather"]["r_main"].as<String>();
          condition.r_description = w["weather"]["r_description"].as<String>();
          condition.r_icon = w["weather"]["r_icon"].as<String>();

          entry.weather.push_back(condition);
        }

        daily.push_back(entry);
      }
    }

    printSummary();
    
    return true;
}


bool WeatherForecast::parseJson(const String& jsonString) 
{
    DynamicJsonDocument doc(8192);  // Larger size for full forecast
    DeserializationError error = deserializeJson(doc, jsonString);

    r_fc_temp_min = 50.0;
    r_fc_temp_max = 0.0;

    if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return false;
    }

    r_cnt = doc["cnt"];

    Serial.println(" WeatherForecast::parseJson - 1");

    // Parse city data
    JsonObject cityObj = doc["city"];
    r_city.r_name = cityObj["name"].as<String>();
    r_city.r_lat = cityObj["coord"]["lat"];
    r_city.r_lon = cityObj["coord"]["lon"];
    r_city.r_country = cityObj["country"].as<String>();
    Serial.println(" WeatherForecast::parseJson - 2");
    r_city.r_population = cityObj["population"];
    r_city.r_timezone = cityObj["timezone"];
    r_city.r_sunrise = cityObj["sunrise"];
    r_city.r_sunset = cityObj["sunset"];

    // Parse forecast entries
    JsonArray listArray = doc["list"];
    Serial.println(" WeatherForecast::parseJson - listArray loop");
    int it = 1;
    for (JsonObject item : listArray)
    {
        Serial.print(" WeatherForecast::parseJson - \t iter: ");
        Serial.println(it);
        it++;
        WeatherElement entry;

        entry.r_dt = item["dt"];
        entry.r_dt_txt = item["dt_txt"].as<String>();

        // Main data
        JsonObject mainObj = item["main"];
        entry.r_main.r_temp = mainObj["temp"];
        entry.r_main.r_feels_like = mainObj["feels_like"];
        entry.r_main.r_temp_min = mainObj["temp_min"];
        entry.r_main.r_temp_max = mainObj["temp_max"];

        if(entry.r_main.r_temp_min < r_fc_temp_min) {r_fc_temp_min = entry.r_main.r_temp_min;}
        if(entry.r_main.r_temp_max > r_fc_temp_max) {r_fc_temp_max = entry.r_main.r_temp_max;}

        entry.r_main.r_pressure = mainObj["pressure"];
        entry.r_main.r_sea_level = mainObj["sea_level"];
        entry.r_main.r_grnd_level = mainObj["grnd_level"];
        entry.r_main.r_humidity = mainObj["humidity"];

        // Weather conditions
        JsonArray weatherArray = item["weather"];
        for (JsonObject w : weatherArray) {
            weather_descrip condition;
            condition.r_id = w["id"];
            condition.r_main = w["main"].as<String>();
            condition.r_description = w["description"].as<String>();
            condition.r_icon = w["icon"].as<String>();
            entry.r_weather.push_back(condition);
        }

        // Clouds
        entry.r_clouds.r_all = item["clouds"]["all"];

        // Wind
        JsonObject windObj = item["wind"];
        entry.r_wind.r_speed = windObj["speed"];
        entry.r_wind.r_deg = windObj["deg"];

        // Other data
        entry.r_visibility = item["visibility"];
        entry.r_pop = item["pop"];
        entry.r_sys.r_pod = item["sys"]["pod"].as<String>();

        r_list.push_back(entry);
    }

    printSummary();
    
    return true;
}

bool WeatherCurrent::parseJson(const String& jsonString) 
{
    DynamicJsonDocument doc(8192);  // Larger size for full forecast
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      return false;
    }

    // Parse city data
    JsonObject cityObj = doc["city"];
    r_city.r_name = doc["name"].as<String>();
    r_city.r_lat = doc["coord"]["lat"];
    r_city.r_lon = doc["coord"]["lon"];
    r_city.r_country = doc["sys"]["country"].as<String>();
    r_city.r_timezone = doc["sys"]["timezone"];
    r_city.r_sunrise = doc["sys"]["r_sunrise"];
    r_city.r_sunset = doc["sys"]["r_sunset"];

    Serial.println(" WeatherCurrent::parseJson - 2");

    r_data.r_dt = doc["dt"];

    // Main data
    JsonObject mainObj = doc["main"];
    r_data.r_main.r_temp = mainObj["temp"];
    r_data.r_main.r_feels_like = mainObj["feels_like"];
    r_data.r_main.r_temp_min = mainObj["temp_min"];
    r_data.r_main.r_temp_max = mainObj["temp_max"];

    r_data.r_main.r_pressure = mainObj["pressure"];
    r_data.r_main.r_sea_level = mainObj["sea_level"];
    r_data.r_main.r_grnd_level = mainObj["grnd_level"];
    r_data.r_main.r_humidity = mainObj["humidity"];

    // Weather conditions
    JsonArray weatherArray = doc["weather"];
    for (JsonObject w : weatherArray) {
        weather_descrip condition;
        condition.r_id = w["id"];
        condition.r_main = w["main"].as<String>();
        condition.r_description = w["description"].as<String>();
        condition.r_icon = w["icon"].as<String>();
        r_data.r_weather.push_back(condition);
    }

    // Clouds
    r_data.r_clouds.r_all = doc["clouds"]["all"];

    Serial.println(" WeatherCurrent::parseJson - 2");
    
    // Wind
    JsonObject windObj = doc["wind"];
    r_data.r_wind.r_speed = windObj["speed"];
    r_data.r_wind.r_deg = windObj["deg"];

    // Other data
    r_data.r_visibility = doc["visibility"];

    printSummary();

    return true;
}

// Helper: Get forecast for specific index
WeatherElement* WeatherForecast::getEntry(int index)
{
  if (!dataLoaded) {
        Serial.println("Warning: Data not loaded yet!");
        return nullptr;
  }

  if (index >= 0 && index < r_list.size()) {
        return &r_list[index];
  }
  return nullptr;
}

void Weather_OneCall_3_0::printSummary()
{
    Serial.println("=== Weather One Call 3.0 ===");
    Serial.printf("Lat: %.2f Long: %.2f \n", r_lat, r_long);
    Serial.printf("Timezone: %s Offset: %d \n", timezone.c_str(), timezone_offset);

    if(exists_current)
    {
      Serial.println("Current data exists");
      Serial.print("[");
      Serial.print(dateTime(current.dt,LOCAL_TIME, "Y-m-d H:i:s"));
      Serial.println("]");

      Serial.printf("    Temp: %.1f°C (feels like %.1f°C)\n", 
                  current.temp, current.feels_like);
      Serial.print("    Weather: ");
      Serial.println(getCurrentWeatherDescription());
      Serial.printf("    Humidity: %d%%\n", current.humidity);
      Serial.printf("    Wind: %.1f m/s\n\n", current.wind_speed);
      Serial.printf("    Rain?: %d \n\n", current.rain_exists);
      Serial.printf("    Snow?: %d \n\n", current.snow_exists);
    }

    if(exists_hourly)
    {
      Serial.println("Hourly data exists.");
      Serial.printf("Num Entries: %d \n", hourly.size());
      Serial.println("Showing first entry.");
      Serial.print("[");
      Serial.print(dateTime(hourly[0].dt, "Y-m-d H:i:s"));
      Serial.println("]");

      Serial.printf("    Temp: %.1f°C (feels like %.1f °C)\n", 
                  hourly[0].temp, hourly[0].feels_like);
      Serial.print("    Weather: ");
      Serial.println(getHourlyWeatherDescription(0));
      Serial.printf("    Humidity: %d%%\n", hourly[0].humidity);
      Serial.printf("    Wind: %.1f m/s\n\n", hourly[0].wind_speed);
      Serial.printf("    Pop: %.1f m/s\n\n", hourly[0].pop);
      Serial.printf("    Rain?: %d \n\n", hourly[0].rain_exists);
      Serial.printf("    Snow?: %d \n\n", hourly[0].snow_exists);
    }

    if(exists_daily)
    {
      Serial.println("Daily data exists.");
      Serial.printf("Num Entries: %d \n", daily.size());
      Serial.println("Showing first entry.");
      Serial.print("[");
      Serial.print(dateTime(daily[0].dt, "Y-m-d H:i:s"));
      Serial.println("]");

      Serial.printf("    Temp: %.1f°C (feels like %.1f°C)\n", 
                  daily[0].temp.day, daily[0].feels_like.day);
      Serial.print("    Weather: ");
      Serial.println(getDailyWeatherDescription(0));
      Serial.printf("    Humidity: %d%%\n", daily[0].humidity);
      Serial.printf("    Wind: %.1f m/s\n\n", daily[0].wind_speed);
      Serial.printf("    Pop: %.1f m/s\n\n", daily[0].pop);
      Serial.printf("    Rain?: %d \n\n", daily[0].rain_exists);
      Serial.printf("    Snow?: %d \n\n", daily[0].snow_exists);
    }
}

// Helper: Print summary
void WeatherForecast::printSummary()
{
    Serial.println("=== Weather Forecast ===");
    Serial.printf("City: %s, %s\n", r_city.r_name.c_str(), r_city.r_country.c_str());
    Serial.printf("Forecast entries: %d\n\n", r_cnt);

    for (int i = 0; i < r_list.size(); i++) {
        WeatherElement& entry = r_list[i];
        Serial.printf("[%d] %s\n", i, entry.r_dt_txt.c_str());
        Serial.printf("    Temp: %.1f°C (feels like %.1f°C)\n", 
                    entry.r_main.r_temp, entry.r_main.r_feels_like);
        Serial.printf("    Weather: %s\n", entry.getWeatherDescription().c_str());
        Serial.printf("    Humidity: %d%%\n", entry.r_main.r_humidity);
        Serial.printf("    Wind: %.1f m/s\n\n", entry.r_wind.r_speed);
    }
}

void WeatherCurrent::printSummary()
{
    Serial.println("=== Current Weather ===");
    Serial.printf("City: %s, %s\n", r_city.r_name.c_str(), r_city.r_country.c_str());

        Serial.printf("%s \n", dateTime(r_data.r_dt, "Y-m-d H:i:s") );
        Serial.printf("    Temp: %.1f°C (feels like %.1f°C)\n", 
                    r_data.r_main.r_temp, r_data.r_main.r_feels_like);
        Serial.printf("    Weather: %s\n", r_data.getWeatherDescription().c_str());
        Serial.printf("    Humidity: %d%%\n", r_data.r_main.r_humidity);
        Serial.printf("    Wind: %.1f m/s\n\n", r_data.r_wind.r_speed);

}

void requestWeather()
{
  // bools -> 1 = current, 2=forecast, 3=OneCallApi3.0
  //requestOpenWeather(false,false,false);
  //requestOpenWeather(false,false,false);
  requestOpenWeather(false,false,true);
}

void drawWeather()
{
  Weather_OneCall_3_0& oneCall =
          Weather_OneCall_3_0::getInstance();

  int width = tft.width();
  int height = tft.height();
  int margin = 5;

  int temp_X = 10;
  int temp_Y = 210;

  tft.setTextDatum(TL_DATUM);
  tft.setFreeFont(&Orbitron_Light_24);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  int charWidth = tft.textWidth("B");
  int charHeigth = tft.fontHeight();
  int textY = 70;

  char buffer[10];
  memset(buffer,0,sizeof(buffer));

  // Print black weather part screen
  tft.fillRect(0,240 - tft.fontHeight(), 160, tft.fontHeight(), TFT_BLACK);
  Serial.print("weather::drawWeather() - temp: ");
  Serial.println(oneCall.current.temp);

  oneCall.printSummary();

  // temperature
  sprintf(buffer, "%.1f", oneCall.current.temp);
  tft.drawString(buffer, temp_X, temp_Y - tft.fontHeight());
  // degrees character
  tft.setTextSize(1);
  tft.drawString("o", temp_X + charWidth * 3.2, temp_Y + 10 - charHeigth);

  

}

void drawWeatherInfo()
{
    if(firstWeatherRequest)
    {
      firstWeatherRequest = false;

      Serial.println("weather::drawWeatherInfo - FIRST TIME requesting OW forecast.");
      requestWeather();
    }
    else if (myTZ.minute() == 30 || myTZ.minute() == 0) 
    {
      Serial.println("weather::drawWeatherInfo - requesting OW forecast.");
      requestWeather();
    }

    drawWeather();
}