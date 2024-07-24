#include <Arduino.h>
#include <Notecard.h>
#include <NotecardPseudoSensor.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_Sensor.h>

int getSensorInterval();

using namespace blues;


#define usbSerial Serial
#define productUID "com.gmail.cthoman24:testing"

Notecard notecard;
NotecardPseudoSensor sensor(notecard);

// the setup function runs once when you press reset or power the board
void setup()
{
  delay(2500);
  usbSerial.begin(115200);

  notecard.begin();
  notecard.setDebugOutputStream(usbSerial);

  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  JAddBoolToObject(req, "sync", true); 
  notecard.sendRequest(req);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  float temperature = sensor.temp();
  float humidity = sensor.humidity();

  usbSerial.print("Temperature = ");
  usbSerial.print(temperature);
  usbSerial.println(" *C");
  usbSerial.print("Humidity = ");
  usbSerial.print(humidity);
  usbSerial.println(" %");

  J *req = notecard.newRequest("note.add");
  if (req != NULL)
  {
    JAddStringToObject(req, "file", "sensors.qo");
    JAddBoolToObject(req, "sync", true);
    J *body = JAddObjectToObject(req, "body");
    if (body)
    {
      JAddNumberToObject(body, "temp", temperature);
      JAddNumberToObject(body, "humidity", humidity);
    }
    notecard.sendRequest(req);
  }
  int sensorIntervalSeconds = getSensorInterval();
  usbSerial.print("Delaying ");
  usbSerial.print(sensorIntervalSeconds);
  usbSerial.println(" seconds");
  delay(sensorIntervalSeconds * 1000);
  
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(1000);                     // wait for a second
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
  delay(1000);
}

int getSensorInterval() {
  int sensorIntervalSeconds = 60;
  J *req = notecard.newRequest("env.get");
  if (req != NULL) {
      JAddStringToObject(req, "name", "reading_interval");
      J* rsp = notecard.requestAndResponse(req);
      int readingIntervalEnvVar = atoi(JGetString(rsp, "text"));
      if (readingIntervalEnvVar > 0) {
        sensorIntervalSeconds = readingIntervalEnvVar;
      }
      notecard.deleteResponse(rsp);
  }
  return sensorIntervalSeconds;
}