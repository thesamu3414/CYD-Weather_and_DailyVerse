#include "weather.h"
#include "weather_icons.h"
#include <WiFiClientSecure.h>
//#include "Free_Fonts.h"

#include "token.h"  // Add this line

bool weHaveWeatherInfo = false;

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
    
    fullURL.replace("lat=&", "lat=" + String(LAT_CITY) + "&");
    fullURL.replace("lon=&", "lon=" + String(LONG_CITY) + "&");
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
        WiFiClient* stream = http.getStreamPtr(); // Get the response payload
        Serial.println("Response:");
        Serial.println(*stream); // Print the response

        if(curr)
        {
          WeatherCurrent& current =
            WeatherCurrent::getInstance();
          getSuccess = current.parseJson(stream);
        }
        else if (forec)
        {
          WeatherForecast& forecast =
            WeatherForecast::getInstance();
          getSuccess = forecast.parseJson(stream);
        }
        else if (oneCall)
        {
          Weather_OneCall_3_0& oneCall_3_0 =
            Weather_OneCall_3_0::getInstance();
          getSuccess = oneCall_3_0.parseJson(stream);
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

  weHaveWeatherInfo = getSuccess;

  return getSuccess;
}

// One Call API 3.0
bool Weather_OneCall_3_0::parseJson(WiFiClient* stream) 
{
    DynamicJsonDocument doc(32768);  // Larger size for full forecast
    DeserializationError error = deserializeJson(doc, *stream);

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
        condition.r_id = w["id"];
        condition.r_main = w["main"].as<String>();
        condition.r_description = w["description"].as<String>();
        condition.r_icon = w["icon"].as<String>();

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
          condition.r_id = w["id"];
          condition.r_main = w["main"].as<String>();
          condition.r_description = w["description"].as<String>();
          condition.r_icon = w["icon"].as<String>();

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
          condition.r_id = w["id"];
          condition.r_main = w["main"].as<String>();
          condition.r_description = w["description"].as<String>();
          condition.r_icon = w["icon"].as<String>();

          entry.weather.push_back(condition);
        }

        daily.push_back(entry);
      }
    }

    printSummary();
    
    return true;
}


bool WeatherForecast::parseJson(WiFiClient* stream) 
{
    DynamicJsonDocument doc(8192);  // Larger size for full forecast
    DeserializationError error = deserializeJson(doc, *stream);

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

bool WeatherCurrent::parseJson(WiFiClient* stream) 
{
    DynamicJsonDocument doc(8192);  // Larger size for full forecast
    DeserializationError error = deserializeJson(doc, *stream);

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
      Serial.printf("    icon: %s \n\n", current.weather[0].r_icon.c_str());
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
      Serial.printf("    icon: %s \n\n", hourly[0].weather[0].r_icon.c_str());
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

void unixToHHMM(int &unixTime, short &timezoneOffset, int &hours, int &minutes) {
    int localTime = unixTime + timezoneOffset;
    hours   = (localTime % 86400) / 3600;
    minutes = (localTime % 3600) / 60;

    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
}

int drawTime(int x, int y, int hour, int minutes, int font_size)
{
  char buffer[6];
  memset(buffer,0,sizeof(buffer));

  tft.setTextColor(0xFFFF);
  tft.setTextSize(font_size);

  // hour
  sprintf(buffer, " %02d", hour);
  tft.drawString(buffer, x, y);
  int hourTextWidth = tft.textWidth(buffer);
  //// "h""
  tft.setTextSize(2);
  tft.drawString("h", x + hourTextWidth -1, y+7);
  hourTextWidth += tft.textWidth("h")-1; // Update total width

  //// colon
  //tft.setTextSize(2);
  //tft.drawString(":", x + hourTextWidth -3, y+5);
  //hourTextWidth += tft.textWidth(":")-5; // Update total width
  //// minutes
  //tft.setTextSize(font_size);
  //sprintf(buffer, "%02d", minutes);
  //tft.drawString(buffer, x + hourTextWidth, y);
  //hourTextWidth += tft.textWidth(buffer);

  return hourTextWidth;
}

int drawTemperature(int x, int y, int degrees, int decimals, int font_size)
{
  char buffer[6];
  memset(buffer,0,sizeof(buffer));

  tft.setTextSize(font_size);
  tft.setTextColor(0x03E0);

  x += 2;
  y += 2 + tft.fontHeight();

  // int part
  if (degrees >= 10) {sprintf(buffer, "%d", degrees);}
  else {sprintf(buffer, " %d", degrees);} // Add space for alignment if single digit
  tft.drawString(buffer, x, y);
  int tempTextWidth = tft.textWidth(buffer);
  int tempTextHeight = tft.fontHeight();
  // decimal dot
  tft.setTextSize(font_size-1);
  tft.drawString(".", x + tempTextWidth-3, y + 5);
  tempTextWidth += tft.textWidth(".")-3; // Update total width
  // decimal part
  memset(buffer,0,sizeof(buffer));
  tft.setTextSize(font_size);
  sprintf(buffer, "%d", decimals);
  tft.drawString(buffer, x + tempTextWidth, y);
  tempTextWidth += tft.textWidth(buffer); // Update total width
  
  // degrees character
  tft.setTextSize(font_size-2);
  tft.drawString("o", x + tempTextWidth, y - 3);
  tempTextWidth += tft.textWidth(buffer); // Update total width

  return tempTextWidth;
}


int margin = 5;
int weatherScreen_Y = 71; // horizontal line separating clock and weather info
int temp_X = 5;
int temp_Y = weatherScreen_Y + margin;

void drawWeatherForecast_Hourly()
{
  Weather_OneCall_3_0& oneCall = Weather_OneCall_3_0::getInstance();
 
  char buffer[10];
  memset(buffer,0,sizeof(buffer));
  
  int temp_font_size = 7;
  int hourforecast_font_size = 3;
  

  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(1);

  tft.setTextSize(temp_font_size);
  int charHeigth = tft.fontHeight();

  int forecast_X = temp_X + 4;
  int forecast_Y = temp_Y + charHeigth + 13;
  
  tft.drawLine(temp_X-3, forecast_Y - 13 , temp_X-3, 240, 0xFFFF);

  int step = 3;
  for (int i = 1; i <= 1 + 3*step; i += step)
  {  // Hour -------------------------------
    int hour, minutes;
    unixToHHMM(oneCall.hourly[i].dt, oneCall.timezone_offset, hour, minutes);
    int hourTextWidth = drawTime(forecast_X, forecast_Y, hour, minutes, hourforecast_font_size);

    // temperature ------------------------
    int intPart     = (int)oneCall.hourly[i].temp;
    int decimalPart = (int)((oneCall.hourly[i].temp - intPart) * 10); 
    int tempTextWidth = drawTemperature(forecast_X, forecast_Y, intPart, decimalPart, hourforecast_font_size);

    // Weather icon ------------------------
    int iconCode = getWeatherIcon_equivalent(oneCall.hourly[i].weather[0].r_icon);
    if(iconCode >= 0)
    {
      tft.drawBitmap(forecast_X + 20, 240 - 40, wea_icon_allArray[iconCode].bitmap, 30, 30, 0x0000, wea_icon_allArray[iconCode].color);
    }

    tft.drawLine(forecast_X + tempTextWidth + 6, forecast_Y - 13 , forecast_X + tempTextWidth + 6, 240, 0xFFFF);
    forecast_X += tempTextWidth + 9;
  }
}

void drawWeather()
{
  Weather_OneCall_3_0& oneCall = Weather_OneCall_3_0::getInstance();

  int temp_font_size = 7;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(temp_font_size);
  tft.setTextFont(1);
  int charHeigth = tft.fontHeight();

  char buffer[10];
  memset(buffer,0,sizeof(buffer));

  // Print black weather screen part
  tft.fillRect(0,weatherScreen_Y, 320, 240, TFT_BLACK);

  tft.drawLine(0, weatherScreen_Y , 340, weatherScreen_Y, 0xFFFF);
  tft.drawLine(0, temp_Y + charHeigth, 340, temp_Y + charHeigth, 0xFFFF);

  // temperature ------------------------
  int intPart     = (int)oneCall.current.temp;
  int decimalPart = (int)((oneCall.current.temp - intPart) * 10); 
  int feelslike   = (int)(oneCall.current.feels_like - oneCall.current.temp);


  tft.setTextColor(0x25C4);
  tft.setTextSize(temp_font_size);
  // int part
  sprintf(buffer, "%d", intPart);
  tft.drawString(buffer, temp_X, temp_Y);
  int tempTextWidth = tft.textWidth(buffer);
  int tempTextHeight = tft.fontHeight();
  // decimal dot
  tft.setTextSize(temp_font_size-2);
  tft.drawString(".", temp_X + tempTextWidth-3, temp_Y + charHeigth - tft.fontHeight()-3);
  tempTextWidth += tft.textWidth(".")-3; // Update total width
  // decimal part
  memset(buffer,0,sizeof(buffer));
  tft.setTextSize(temp_font_size);
  sprintf(buffer, "%d", decimalPart);
  tft.drawString(buffer, temp_X + tempTextWidth, temp_Y);
  tempTextWidth += tft.textWidth(buffer); // Update total width

  // degrees character
  tft.setTextSize(3);
  tft.drawString("o", temp_X + tempTextWidth, temp_Y-5);

  // feels like part
  if (feelslike > 0.1 || feelslike < -0.1) // Only show if difference is significant
  {
    memset(buffer,0,sizeof(buffer));
    tft.setTextSize(2);
    if(feelslike > 0) 
    {
      tft.setTextColor(0xFDA0);
      sprintf(buffer, "+%d", feelslike);
      tft.drawString(buffer, temp_X + tempTextWidth, temp_Y + charHeigth - tft.fontHeight() - 5);

    }
    else
    {
      tft.setTextColor(0x867D);
      sprintf(buffer, "%d", feelslike);
      tft.drawString(buffer, temp_X + tempTextWidth, temp_Y + charHeigth - tft.fontHeight() - 5);
    }
  }
  tempTextWidth += tft.textWidth(buffer); // Update total width

  // Weather icon
  int iconCode = getWeatherIcon_equivalent(oneCall.current.weather[0].r_icon);
  if(iconCode >= 0)
  {
    tft.drawBitmap(tempTextWidth + 5, temp_Y, wea_icon_allArray[iconCode].bitmap, 30, 30, 0x0000, wea_icon_allArray[iconCode].color);
  }

  memset(buffer,0,sizeof(buffer));
  
  // Humudity ---------------------------
  tft.setTextColor(0x5FA);
  tft.setTextSize(temp_font_size);
  sprintf(buffer, "%d", oneCall.current.humidity);
  tft.drawString(buffer, 340 - tft.textWidth(buffer) - tft.textWidth("."), temp_Y);
  int humTextWidth = tft.textWidth(".");
  // percentage character
  tft.setTextSize(3);
  tft.drawString("%", 340 - humTextWidth, temp_Y + charHeigth - tft.fontHeight() - 3);

  drawWeatherForecast_Hourly();
}

void drawWeatherInfo(const bool &forceRequest)
{
    if(forceRequest || !weHaveWeatherInfo) 
    {
      Serial.println("weather::drawWeatherInfo - requesting OW forecast.");
      requestWeather();
    }

    drawWeather();
}