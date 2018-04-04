#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include "ESP8266WiFi.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define ARDUINO_USD_CS 15 // Pin D8
#define LOG_FILE_PREFIX "gpslog"
#define MAX_LOG_FILES 100
#define LOG_FILE_SUFFIX "csv"
char logFileName[13];
#define LOG_COLUMN_COUNT 11
const String wigleHeaderFileFormat = "WigleWifi-1.4,appRelease=2.26,model=Feather,release=0.0.0,device=arduinoWardriving,display=3fea5e7,board=esp8266,brand=Adafruit";
char * log_col_names[LOG_COLUMN_COUNT] = {
  "MAC", "SSID", "AuthMode", "FirstSeen", "Channel", "RSSI", "CurrentLatitude", "CurrentLongitude", "AltitudeMeters", "AccuracyMeters", "Type"
};

#define LOG_RATE 500
unsigned long lastLog = 0;

static const int RXPin = 0, TXPin = 3;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus tinyGPS;
int display = 1;

#define SerialMonitor Serial

void setup() {
  SerialMonitor.begin(115200);
  ss.begin(GPSBaud);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Setting up SD card.");
  SerialMonitor.println("Setting up SD card.");
  delay(200);
  lcd.clear();
  if (!SD.begin(ARDUINO_USD_CS)) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Error initializing SD card.");
    SerialMonitor.println("Error initializing SD card.");
  }
  updateFileName();
  printHeader();
}
void lookForNetworks() {
    int n = WiFi.scanNetworks();
  if (n == 0) {
    SerialMonitor.println("no networks found");
  } else {
    for (uint8_t i = 1; i <= n; ++i) {
      if ((isOnFile(WiFi.BSSIDstr(i)) == -1) && (WiFi.channel(i) > 0) && (WiFi.channel(i) < 15)) { //Avoid erroneous channels
        File logFile = SD.open(logFileName, FILE_WRITE);
        SerialMonitor.println("New network found");
        logFile.print(WiFi.BSSIDstr(i));
        logFile.print(',');
        logFile.print(WiFi.SSID(i));
        logFile.print(',');
        logFile.print(getEncryption(i));
        logFile.print(',');
        logFile.print(tinyGPS.date.year());
        logFile.print('-');
        logFile.print(tinyGPS.date.month());
        logFile.print('-');
        logFile.print(tinyGPS.date.day());
        logFile.print(' ');
        logFile.print(tinyGPS.time.hour());
        logFile.print(':');
        logFile.print(tinyGPS.time.minute());
        logFile.print(':');
        logFile.print(tinyGPS.time.second());
        logFile.print(',');
        logFile.print(WiFi.channel(i));
        logFile.print(',');
        logFile.print(WiFi.RSSI(i));
        logFile.print(',');
        logFile.print(tinyGPS.location.lat(), 6);
        logFile.print(',');
        logFile.print(tinyGPS.location.lng(), 6);
        logFile.print(',');
        logFile.print(tinyGPS.altitude.meters(), 1);
        logFile.print(',');
        logFile.print((tinyGPS.hdop.value(), 1));
        logFile.print(',');
        logFile.print("WIFI");
        logFile.println();
        logFile.close();
      }
    }
  }
}
void loop() {
  SerialMonitor.print("GPS logged ");
  SerialMonitor.print(tinyGPS.location.lat(), 6);
  SerialMonitor.print(", ");
  SerialMonitor.println(tinyGPS.location.lng(), 6);
  if (tinyGPS.location.isValid()) {
    lookForNetworks();
  }
  if (display == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lat: ");
    lcd.print(tinyGPS.location.lat(), 6);
    lcd.setCursor(0, 1);
    lcd.print("Lon: ");
    lcd.print(tinyGPS.location.lng(), 6);
    display = 0;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Seen: ");
    lcd.print(countNetworks());
    lcd.setCursor(0, 1);
    lcd.print("networks");
    display = 1;
  }
  smartDelay(LOG_RATE);

  if (millis() > 5000 && tinyGPS.charsProcessed() < 10)
    SerialMonitor.println("No GPS data received: check wiring");
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      tinyGPS.encode(ss.read());
  } while (millis() - start < ms);
}

int countNetworks() {
  File netFile = SD.open(logFileName);
  int networks = 0;
  if (netFile) {
    while (netFile.available()) {
      netFile.readStringUntil('\n');
      networks++;
    }
    netFile.close();
    if (networks == 0) {
      return networks;
    } else {
      return (networks - 1); //Avoid header count
    }
  }
}

int isOnFile(String mac) {
  File netFile = SD.open(logFileName);
  String currentNetwork;
  if (netFile) {
    while (netFile.available()) {
      currentNetwork = netFile.readStringUntil('\n');
      if (currentNetwork.indexOf(mac) != -1) {
        SerialMonitor.println("The network was already found");
        netFile.close();
        return currentNetwork.indexOf(mac);
      }
    }
    netFile.close();
    return currentNetwork.indexOf(mac);
  }
}

void printHeader() {
  File logFile = SD.open(logFileName, FILE_WRITE);
  if (logFile) {
    int i = 0;
    logFile.println(wigleHeaderFileFormat);
    for (; i < LOG_COLUMN_COUNT; i++) {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1)
        logFile.print(',');
      else
        logFile.println();
    }
    logFile.close();
  }
}

void updateFileName() {
  int i = 0;
  for (; i < MAX_LOG_FILES; i++) {
    memset(logFileName, 0, strlen(logFileName));
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) {
      break;
    } else {
      SerialMonitor.print(logFileName);
      SerialMonitor.println(" exists");
    }
  }
  SerialMonitor.print("File name: ");
  SerialMonitor.println(logFileName);
}

String getEncryption(uint8_t network) {
  byte encryption = WiFi.encryptionType(network);
  switch (encryption) {
    case 2:
      return "[WPA-PSK-CCMP+TKIP][ESS]";
    case 5:
      return "[WEP][ESS]";
    case 4:
      return "[WPA2-PSK-CCMP+TKIP][ESS]";
    case 7:
      return "[ESS]";
    case 8:
      return "[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][ESS]";
  }
}
