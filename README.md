# ESP32 CYD Rolling Clock with WiFi Manager

This project implements a rolling clock display on the ESP32 Cheap Yellow Display (CYD), enhanced with WiFi connectivity, weather information, daily Bible verses, and touch controls. It's based on the [Rolling Clock with Wifi Manager project](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/tree/main/Examples/Projects/RollingClockWithWifiManager) by Brian Lough.

## Features

- **Rolling Clock Display**: Animated rolling digits for time display
- **WiFi Manager**: Easy WiFi configuration via captive portal
- **Weather Integration**: Fetches and displays current weather data
- **Daily Bible Verses**: Shows inspirational Bible verses
- **Touch Screen Interface**: Interactive controls for settings and navigation
- **NTP Time Synchronization**: Accurate time keeping with internet sync
- **Customizable Display**: Support for different screens and layouts

## Hardware Requirements

- ESP32 Cheap Yellow Display (CYD) with ILI9341 TFT touchscreen
- USB cable for programming and power

## Software Requirements

- [PlatformIO](https://platformio.org/) IDE
- Arduino framework for ESP32

## Dependencies

The project uses the following libraries (automatically installed via PlatformIO):

- `khoih-prog/ESP_DoubleResetDetector@^1.3.2`
- `bblanchon/ArduinoJson@^6.21.3`
- `wnatth3/WiFiManager@^2.0.16-rc.2`
- `ropg/ezTime@^0.8.3`
- `bodmer/TFT_eSPI@^2.5.33`

## Installation

1. Clone this repository:
   ```
   git clone <repository-url>
   cd RollingClockWithWifiManager
   ```

2. Open the project in PlatformIO.

3. Prepare the `token.h` file.
    First rename `token.h.example` to `token.h`.
    Change the values inside:
        -   Wifi SSID and password, to yours.

3. Build and upload to your ESP32 CYD:
   ```
   pio run -t upload
   ```

4. Monitor the serial output:
   ```
   pio device monitor
   ```

## Usage

1. Power on the device.
2. If not configured, it will create a WiFi access point (e.g., "ESP32-Config").
3. Connect to the AP and configure your WiFi credentials.
4. The device will connect to the internet and display the rolling clock.
5. Use the touchscreen to navigate between different screens (clock, weather, verses, settings).

## Configuration

- WiFi settings: Configured via the built-in WiFi manager
- Weather API: Requires API key (configure in `token.h`)
- Bible verses: Fetched from an online service
- Display settings: Adjustable via touch interface

## Project Structure

- `RollingClockWithWifiManager.ino`: Main Arduino sketch
- `weather.h/cpp`: Weather data handling
- `bibleVerse.h`: Bible verse management
- `wifiManagerHandler.h`: WiFi configuration
- `Digit.h/cpp`: Rolling digit animation logic
- `screensMngr.h`: Screen management
- `touch.h`: Touch input handling
- `projectConfig.h`: Project configuration
- `constAndParam.h`: Constants and parameters

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests.

## License

This project is based on the original work by Brian Lough. Please check the original repository for licensing information.

## Acknowledgments

- [Brian Lough](https://github.com/witnessmenow) for the original Rolling Clock project
- ESP32 Cheap Yellow Display community
