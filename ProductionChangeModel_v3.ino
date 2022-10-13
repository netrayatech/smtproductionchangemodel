#include "config.h"

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/L12ON") {
      Serial.println("BENAR L12ON");
      L12Status = 1;
      konfirmasiSensorStatus = 1;
      delayRunning = false;
      startMount = LOW;
      endMount = HIGH;
      delayEvery5Mins = millis();
      changeModelTimer = 0;

      t2 = millis();
      timingEnd = getTiming();

      totalCM = ((t2 - t1) / 60) / 1000;

      message = String(whichLine) + String(" CM:\n\nStart at " + timingStart);

      message += String("\nEnd at " + timingEnd);

      if (totalCM > 0)
        message += String("\n\nTotal CM Time: ") + String(totalCM, 2) + String(" mins");

      Serial.println(message);

      bot.sendMessage(CHAT_UAT, message, "");

      delay(100);
      Firebase.setString(fbdo, yellowLEDPath, "OFF");
      Firebase.setString(fbdo, redLEDPath, "OFF");
      Firebase.setString(fbdo, endDatePath, timingEnd);

      json.setFloatDigits(2);
      json.add("line", whichLine);
      json.add("startDate", timingStart);
      json.add("endDate", timingEnd);
      json.add("totalCM", (float)totalCM);
      json.add("reportGenerated", "N");

      Serial.printf("Push json... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/smt-report/change-model", &json) ? "ok" : fbdo.errorReason().c_str());
    }

    if (text == "/L12OFF") {
      Serial.println("SALAH L12OFF");
      L12Status = 0;
      konfirmasiSensorStatus = 0;
      delayRunning = true;

      bot.sendMessage(CHAT_UAT, from_name + " menjawab Sensor Terdeteksi object lain, Change Model lanjut", "");
    }
  }
}

void handleButtonPressed() {
  if (startMount == LOW) {  //To ignore if the push button pressed second time
    int readingPushBtn = digitalRead(pushBtnDigitalPin);

    if (readingPushBtn != lastButtonState1) {
      lastDebounceTime1 = millis();
    }

    if ((millis() - lastDebounceTime1) > debounceDelay) {
      if (readingPushBtn != buttonState) {
        buttonState = readingPushBtn;

        if (buttonState == HIGH) {  //If push button pressed
          message = String("");
          t1 = millis();
          startMount = HIGH;
          endMount = LOW;
          delayRunning = true;
          timingStart = getTiming();
          konfirmasiSensorStatus = 0;
          message = String(whichLine) + String(" CM\nStart at " + timingStart);
          // bot.sendMessage(CHAT_ID, message, "");
          bot.sendMessage(CHAT_UAT, message, "");
          Serial.println(message);
          Firebase.setString(fbdo, startDatePath, timingStart);
        }
      }
    }

    lastButtonState1 = readingPushBtn;
  }

  //------------------------------------------------

  if (startMount == HIGH && endMount == LOW) {  //Only if push button already press, system will go to check sensor
    int sensorState1 = digitalRead(sensorDigitalPin1);
    delay(50);
    int sensorState2 = digitalRead(sensorDigitalPin2);
    delay(50);

    if ((sensorState1 == HIGH || sensorState2 == HIGH)  //Sensor is detected object/pcb
        && konfirmasiSensorStatus == 0) {               //Confirmation message to Telegram if the object detected is really PCB or human hand
      konfirmasiSensorStatus = 1;
      String welcomeMSG = "Sensor terdeteksi PCB.\n";
      welcomeMSG += "Apakah benar sudah selesai Change Model?\n\n";
      welcomeMSG += "/L12ON : BENAR, " + whichLine + " sudah SELESAI.\n";
      welcomeMSG += "/L12OFF : SALAH, " + whichLine + " terdeteksi object lain.\n";
      bot.sendMessage(CHAT_UAT, welcomeMSG, "Markdown");
    }
  }
}

void setup() {
  Serial.begin(115200);

  secured_client.setInsecure();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");

  String localIPAddr = WiFi.localIP().toString().c_str();
  Serial.println(localIPAddr);

  // ArduinoOTA.onStart([]() {
  //   Serial.println("Start");
  // });
  // ArduinoOTA.onEnd([]() {
  //   Serial.println("\nEnd");
  // });
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  // });
  // ArduinoOTA.onError([](ota_error_t error) {
  //   Serial.printf("Error[%u]: ", error);
  //   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  //   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  //   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  //   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  //   else if (error == OTA_END_ERROR) Serial.println("End Failed");
  // });
  // ArduinoOTA.begin();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  pinMode(pushBtnDigitalPin, INPUT);  //Push Button
  pinMode(sensorDigitalPin1, INPUT);  //Sensor Conveyor 1
  pinMode(sensorDigitalPin2, INPUT);  //Sensor Conveyor 2

  message = String(whichLine) + String(" CM is UP and RUNNING.\n") + String("IP addr : ") + String(localIPAddr);
  message += String("\nTime: ") + String(getTiming());
  // bot.sendMessage(CHAT_ID, message, "");
  bot.sendMessage(CHAT_UAT, message, "");

  delay(100);
  timeClient.begin();
  delayEvery5Mins = millis();  // start delay

  delayRunning = false;

  Firebase.setString(fbdo, yellowLEDPath, "OFF");
  Firebase.setString(fbdo, redLEDPath, "OFF");
}

void loop() {

  handleButtonPressed();

  if (delayRunning && ((millis() - delayEvery5Mins) >= 300000)) {  //Loop every 5 mins
    delayEvery5Mins = millis();
    changeModelTimer = changeModelTimer + 5;

    if (changeModelTimer > limitChangeTimeInMins) {  // More than 30 mins
      message = String(whichLine) + String(" CM sudah berjalan ") + String(changeModelTimer) + String(" menit, ")
                + String("dan sudah telat ") + String(changeModelTimer - limitChangeTimeInMins) + String(" menit.");

      Serial.println(message);

      bot.sendMessage(CHAT_UAT, message, "");
    } else if (changeModelTimer >= (limitChangeTimeInMins - 10) && changeModelTimer <= limitChangeTimeInMins) {  //in range of 20 - 30 mins
      message = String(whichLine) + String(" sudah berjalan ") + String(changeModelTimer) + String(" menit.\n");

      if (limitChangeTimeInMins - changeModelTimer > 0)
        message += String("Waktu tersisa ") + String(limitChangeTimeInMins - changeModelTimer) + String(" menit.\n");
      else message += String("Waktu sudah habis.");

      // bot.sendMessage(CHAT_ID, message, "");
      bot.sendMessage(CHAT_UAT, message, "");

      Serial.println(message);

      if (changeModelTimer == (limitChangeTimeInMins - 10)) {
        Firebase.setString(fbdo, yellowLEDPath, "ON");
        Firebase.setString(fbdo, redLEDPath, "OFF");
      } else if (changeModelTimer == limitChangeTimeInMins) {
        Firebase.setString(fbdo, yellowLEDPath, "OFF");
        Firebase.setString(fbdo, redLEDPath, "ON");
      }
    }
  }

  if (startMount == HIGH && endMount == LOW && konfirmasiSensorStatus == 1) {
    if (millis() > lastTimeBotRan + botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }

      lastTimeBotRan = millis();
    }
  }
}

String getTiming() {
  timeClient.update();

  String formatTime = timeClient.getFormattedTime();
  if (formatTime.length() > 5) {
    formatTime = formatTime.substring(0, 5);
  }

  return formatTime;
}