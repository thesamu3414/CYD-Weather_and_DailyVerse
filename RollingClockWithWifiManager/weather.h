#ifndef WEATHER_H
#define WEATHER_H

#include <ArduinoJson.h>
#include <vector>
#include <ezTime.h>
#include <TFT_eSPI.h>
#include <HTTPClient.h>

extern Timezone myTZ;
extern TFT_eSPI tft;

struct clouds
{
    short r_all;
};

struct wind
{
    float r_speed;
    short r_deg;
};

struct precipitation
{
    float r_three_hours;
};

struct precipitation_3_0
{
    float r_one_hour;
};

struct Sys
{
    String r_pod; // "n" -> night, "d" -> day
};

struct weather_descrip
{
    short r_id;
    String r_main;
    String r_description;
    String r_icon;
};

struct weather_main
{
    float r_temp;
    float r_feels_like;
    float r_temp_min;
    float r_temp_max;
    short r_pressure;
    short r_sea_level;
    short r_grnd_level;
    short r_humidity;
};

struct weather_main_3_0_current
{
    int     dt;
    int     sunrise;
    int     sunset;
    float   temp;
    float   feels_like;
    int     pressure;
    int     humidity;
    float   dew_point;
    float   uvi;
    int     clouds;
    int     visibility;
    float   wind_speed;
    int     wind_deg;
    float   wind_gust;
    bool              rain_exists;
    precipitation_3_0 rain;
    bool              snow_exists;
    precipitation_3_0 snow;
    std::vector<weather_descrip> weather;
};

struct weather_main_3_0_hourly
{
    int               dt;
    float             temp;
    float             feels_like;
    int               pressure;
    int               humidity;
    float             dew_point;
    float             uvi;
    int               clouds;
    int               visibility;
    float             wind_speed;
    int               wind_deg;
    float             wind_gust;
    float             pop;
    bool              rain_exists;
    precipitation_3_0 rain;
    bool              snow_exists;
    precipitation_3_0 snow;
    std::vector<weather_descrip> weather;
};

struct daily_temp
{
    float morn;
    float day;
    float eve;
    float night;
    float min;
    float max;
};

struct daily_fells_like
{
    float morn;
    float day;
    float eve;
    float night;
};

struct weather_main_3_0_daily
{
    int                 dt;
    int                 sunrise;
    int                 sunset;
    int                 moonrise;
    int                 moonset;
    float               moon_phase;
    String              summary;
    daily_temp          temp;
    daily_fells_like    feels_like;
    int                 pressure; // hPa
    int                 humidity; // %
    float               dew_point;
    float               wind_speed;
    int                 wind_deg;
    float               wind_gust;
    int                 clouds;
    float               uvi;
    float               pop;
    bool                rain_exists;
    float               rain; // mm
    bool                snow_exists;
    float               snow; // mm
    std::vector<weather_descrip> weather;
};

class Weather_OneCall_3_0
{
private:
    Weather_OneCall_3_0() : dataLoaded(false){}
    Weather_OneCall_3_0(const Weather_OneCall_3_0&);             // No copy
    Weather_OneCall_3_0& operator=(const Weather_OneCall_3_0&);  // No assignment

    bool dataLoaded;

public:
    float   r_lat;
    float   r_long;
    String  timezone;
    short   timezone_offset;

    bool                     exists_current;
    weather_main_3_0_current current;

    bool                    exists_hourly;
    std::vector<weather_main_3_0_hourly> hourly;

    bool                   exists_daily;
    std::vector<weather_main_3_0_daily> daily;

    // Get the singleton instance
    static Weather_OneCall_3_0& getInstance() {
        static Weather_OneCall_3_0 instance;
        return instance;
    }

    // Parse from JSON
    bool parseJson(WiFiClient* stream);
    void printSummary();

    // Clear/reset data
    void clear() {
        exists_current = false;
        exists_hourly  = false;
        hourly.clear();
        exists_daily   = false;
        daily.clear();
        dataLoaded = false;
    }

    String getCurrentWeatherDescription()
    {
        if (current.weather.size() > 0)
        {
            return current.weather[0].r_description;
        }
        return "";
    }

    String getHourlyWeatherDescription(size_t entry)
    {
        if (hourly[entry].weather.size() > 0)
        {
            return hourly[entry].weather[0].r_description;
        }
        return "";
    }

    String getDailyWeatherDescription(size_t entry)
    {
        if (daily[entry].weather.size() > 0)
        {
            return daily[entry].weather[0].r_description;
        }
        return "";
    }

    // Check if data has been loaded
    bool isDataLoaded() const { return dataLoaded; }

};

class WeatherElement
{
    public:
        time_t r_dt;
        weather_main r_main;
        std::vector<weather_descrip> r_weather;
        clouds r_clouds;
        wind r_wind;
        short r_visibility;
        float r_pop;
        precipitation r_rain;
        precipitation r_snow;
        Sys r_sys;
        String r_dt_txt;

    
        String getWeatherDescription()
        {
            if (r_weather.size() > 0)
            {
                return r_weather[0].r_description;
            }
            return "";
        }
    
};

// City information
struct City {
  String r_name;
  float r_lat;
  float r_lon;
  String r_country;
  int r_population;
  int r_timezone;
  time_t r_sunrise;
  time_t r_sunset;
};

class WeatherForecast 
{
private:
    WeatherForecast() : r_cnt(0), dataLoaded(false){}
    WeatherForecast(const WeatherForecast&);             // No copy
    WeatherForecast& operator=(const WeatherForecast&);  // No assignment

    bool dataLoaded;

public:
    int r_cnt;
    std::vector<WeatherElement> r_list;
    float r_fc_temp_min; // minimum temp in the forecast
    float r_fc_temp_max; // maximum
    City r_city;

    // Get the singleton instance
    static WeatherForecast& getInstance() {
        static WeatherForecast instance;
        return instance;
    }

    // Parse from JSON
    bool parseJson(WiFiClient* stream);
    WeatherElement* getEntry(int index);
    void printSummary();

    // Clear/reset data
    void clear() {
        r_cnt = 0;
        r_list.clear();
        dataLoaded = false;
    }

    // Check if data has been loaded
    bool isDataLoaded() const { return dataLoaded; }

};

class WeatherCurrent
{
private:
    WeatherCurrent() : dataLoaded(false){}
    WeatherCurrent(const WeatherCurrent&);             // No copy
    WeatherCurrent& operator=(const WeatherCurrent&);  // No assignment

    bool dataLoaded;

public:
    WeatherElement r_data;
    City r_city;

    // Get the singleton instance
    static WeatherCurrent& getInstance() {
        static WeatherCurrent instance;
        return instance;
    }

    // Parse from JSON
    bool parseJson(WiFiClient* stream);
    WeatherElement& getData() {return r_data;};
    void printSummary();

    // Clear/reset data
    // TODO implement a way to empty r_data
    void clear() { dataLoaded = false;}

    // Check if data has been loaded
    bool isDataLoaded() const { return dataLoaded; }

};

bool extractWeatherInfo(String api_response);

bool requestOpenWeather(bool curr, bool forec, bool oneCall);

void requestWeather();

void drawWeatherInfo();

#endif