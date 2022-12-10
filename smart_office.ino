#include <WiFi.h>
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WIFI
const char *ssid = "Dhairya";
const char *password = "le2garib";

// Adafruit MQTT
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "Dhairya_Bhatt"
#define MQTT_PASS "aio_qPJU06A1oTR3zGQQSMT1oV4aHdPf"
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
Adafruit_MQTT_Subscribe office_light = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/office-light");
Adafruit_MQTT_Subscribe door_status = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/door-status");
Adafruit_MQTT_Subscribe fire_sensor = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/feeds/fire-sensor");
Adafruit_MQTT_Subscribe *subscription;

// Fire Related
#define SMOKE_SENSOR 34
#define SENSOR_THRESHOLD 500
#define EXHAUST 2
#define WARNING_LIGHT 12
#define BUZZER 14
int freq = 2000;
int channel = 0;
int resolution = 8;

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}
void initMQTT()
{
    mqtt.subscribe(&office_light);
    mqtt.subscribe(&door_status);
    mqtt.subscribe(&fire_sensor);
}
 void initFire()
 {
     pinMode(EXHAUST, OUTPUT);
     pinMode(WARNING_LIGHT, OUTPUT);
     ledcSetup(channel, freq, resolution);
     ledcAttachPin(BUZZER, channel);
     ledcWriteTone(channel, 2000);
 }
void setup()
{
    Serial.begin(115200);
    initWiFi();
    initMQTT();
    initFire();
}
void doorOperate()
{
    int pos;
    for (pos = 0; pos <= SERVO_ANGLE; pos += 1)
    {
        myservo.write(pos);
        delay(15);
    }
    delay(2000);
    for (pos = SERVO_ANGLE; pos >= 0; pos -= 1)
    {
        myservo.write(pos);
        delay(15);
    }
}
 void fireAlarm(char *status)
 {
     if (strcmp(status, "ON"))
     {
         digitalWrite(EXHAUST, HIGH);
         digitalWrite(WARNING_LIGHT, HIGH);
         delay(100);
         digitalWrite(WARNING_LIGHT, LOW);
         delay(100);
         Serial.println("Door Open");
         if(flag == 1)
         {
         for (int pos = 0; pos <= SERVO_ANGLE; pos += 1)
         {
             myservo.write(pos);
             delay(15);
         }
         }
             ledcWriteTone(channel, 500);
             delay(500);
             ledcWriteTone(channel, 3000);
             delay(500);
     }
     else
     {
         digitalWrite(EXHAUST, LOW);
         digitalWrite(WARNING_LIGHT, LOW);
         Serial.println("Door Close");
         for (int pos = SERVO_ANGLE; pos >= 0; pos -= 1)
         {
             myservo.write(pos);
             delay(15);
         }
             ledcWriteTone(channel, 0);
     }
 }
// void smokeDetector()
// {
//     //    int smoke = analogRead(SMOKE_SENSOR);
//     //    if (smoke > SENSOR_THRESHOLD)
//     //        fireAlarm("ON");
//     //    else
//     //        fireAlarm("OFF");
// }
void MQTT_connect()
{
    int8_t ret;
    // Stop if already connected.
    if (mqtt.connected())
        return;
    Serial.print("Connecting to MQTT... ");
    uint8_t retries = 10;
    while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
    {
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 2 seconds...");
        mqtt.disconnect();
        delay(2000); // wait 2 seconds
        if (--retries == 0)
            // basically die and wait for WDT to reset me
            while (1)
                ;
    }
    Serial.println("MQTT Connected!");
}
void loop()
{
    loopRFID();
    if (flag == 0)
         {
             digitalWrite(WARNING_LIGHT, HIGH);
             delay(100);
             digitalWrite(WARNING_LIGHT, LOW);
             delay(100);
         }
    MQTT_connect();
      while ((subscription = mqtt.readSubscription(5000)))
    {
        // smokeDetector();
         if (subscription == &fire_sensor)
         {
             Serial.println((char *)fire_sensor.lastread);
             if (strcmp((char *)fire_sensor.lastread, "ON"))
             {
                 fireAlarm("ON");
                 flag = 0;
             }
             else
             {
                 flag = 1;
                 fireAlarm("OFF");
             }
         }
    }
    // ping the server to keep the mqtt connection alive
    if (!mqtt.ping())
        mqtt.disconnect();
}
