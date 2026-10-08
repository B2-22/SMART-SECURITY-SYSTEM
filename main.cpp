#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "B2";
const char* WIFI_PASSWORD = "2005BBB222";

const char* THINGSPEAK_API_KEY = "Z7M89TJ3J4GA6BDI";

const int PIR_PIN = 23;
const int BUZZER_PIN = 22;
const int LED_PIN = 21;

bool motionDetected = false;
unsigned long lastUploadTime = 0;

const unsigned long uploadInterval = 16000;

void connectToWiFi()
{
    Serial.println();
    Serial.print("Connecting to Wi-Fi");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wi-Fi CONNECTED");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("Wi-Fi CONNECTION FAILED");
    }
}

void sendToThingSpeak(int motion, int alarm)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Wi-Fi disconnected");
        return;
    }

    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=";
    url += THINGSPEAK_API_KEY;
    url += "&field1=";
    url += String(motion);
    url += "&field2=";
    url += String(alarm);

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String response = http.getString();

        Serial.print("ThingSpeak response: ");
        Serial.println(response);
    }
    else
    {
        Serial.print("ThingSpeak error: ");
        Serial.println(httpCode);
    }

    http.end();
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(PIR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);

    Serial.println("==============================");
    Serial.println("SMART SECURITY SYSTEM");
    Serial.println("==============================");

    connectToWiFi();

    Serial.println("System Ready");
}

void loop()
{
    int motion = digitalRead(PIR_PIN);

    if (motion == HIGH)
    {
        motionDetected = true;

        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);

        Serial.println("MOTION DETECTED - ALARM ON");
    }
    else
    {
        motionDetected = false;

        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, HIGH);

        Serial.println("NO MOTION - ALARM OFF");
    }

    if (millis() - lastUploadTime >= uploadInterval)
    {
        lastUploadTime = millis();

        int alarmStatus = motionDetected ? 1 : 0;

        sendToThingSpeak(
            motionDetected ? 1 : 0,
            alarmStatus
        );
    }

    delay(500);
}