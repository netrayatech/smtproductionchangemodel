#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
// #include <mDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
// #include <ESP8266mDNS.h>
#endif

// #include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
// #include <ArduinoJson.h>

#define FIREBASE_HOST "smtchangemodel-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyDs3v7PGtmQj_5F69YmlOhZrU-bhreqNX8"

char ssid[] = "DIV2 Customer";     // your network SSID (name)
char password[] = "s@tnusa8888"; // your network key
// char ssid[] = "GardivoHome";     // your network SSID (name)
// char password[] = "g4Rd1V0pAnc@Oy3"; // your network key

// Initialize Telegram BOT
// #define BOTtoken "5562094133:AAEeuNkwIxSiP7PfGQCmWT-Izv22NsaFGWs" // LIVE your Bot Token (Get from Botfather)
#define BOTtokenUAT "5333299283:AAHbB3vgtPuYnFoouU93J78vjY1PAKyqxFo" //UAT environment
// #define CHAT_ID "-1001788256455" // LIVE
#define CHAT_UAT "-631983788" //UAT

unsigned long changeModelTimer = 0; // the time the delay started
unsigned long delayEvery5Mins = 0;
int t1 = 0, t2 = 0;
double totalCM = 0.0;
unsigned long limitChangeTimeInMins = 30; // the time the delay started 30 mins
bool delayRunning = false; // true if still waiting for delay to finish
const long utcOffsetInSeconds = 25200;
String message = "";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtokenUAT, secured_client);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//Define FirebaseESP8266 data object
FirebaseData fbdo;
FirebaseJson json;

int pushBtnDigitalPin = D1; //Push Button D1
int sensorDigitalPin1 = D5; //Sensor Conveyor 1 D5
int sensorDigitalPin2 = D6; //Sensor Conveyor 2 D6
int buttonState;
int lastButtonState1 = LOW;
int startMount = LOW, endMount = LOW;

unsigned long lastDebounceTime1 = 0;
unsigned long debounceDelay = 50;

String timingStart = "", timingEnd = "";


String whichLine = "LINE#12";
String yellowLEDPath = "/smt-fact8/change-model/line12/yellowLED";
String redLEDPath = "/smt-fact8/change-model/line12/redLED";
String startDatePath = "/smt-fact8/change-model/line12/startDate";
String endDatePath = "/smt-fact8/change-model/line12/endDate";

int konfirmasiSensorStatus = 0;

int botRequestDelay = 500;
unsigned long lastTimeBotRan;

int L12Status = 0;