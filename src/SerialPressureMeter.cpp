#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include "SerialPressureMeter.h"

#define ANALOGPIN 0
#define PERIOD 100
#define DATACOLUMN 2
#define RANGEMAX 10000
#define RANGEMIN 0
#define SDCHIPSELECT 10

RTC_DS1307 rtc;
String dataFileName = "";
DateTime lastTime;
long lastMillis;

void setup()
{
    // Initilize serial port
    while (!Serial)
        ;
    Serial.begin(19200);

    // Initialize DS1307 chip and set date and time
    Serial.println("Initializing RTC ...");
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC.");
        while (1)
            ;
    }
    if (!rtc.isrunning())
    {
        Serial.println("RTC is NOT running!");
    }
    DateTime now = rtc.now();
    Serial.print("RTC initialized, current time: ");
    Serial.println(now.timestamp());

    // Initialize SD card
    Serial.println("Initializing SD card ...");
    if (!SD.begin(SDCHIPSELECT))
    {
        Serial.println("SD card failed, or not present");
        while (1)
            ;
    }
    Serial.println("SD card initialized.");
    dataFileName = "test.csv";
}

void loop()
{
    double datas[DATACOLUMN];
    int _sensorValue = analogRead(ANALOGPIN);

    datas[0] = analogToPressure(_sensorValue);
    datas[1] = _sensorValue;

    sendToSerial(datas);
    logToSdcard(datas);
    delay(PERIOD);
}

void sendToSerial(double *datas)
{
    String message = "";
    message += "$";
    for (int i = 0; i < DATACOLUMN; i++)
    {
        message += String(datas[i]);
        if (i < DATACOLUMN - 1)
            message += " ";
    }
    message += ";";
    Serial.println(message);
}

double analogToPressure(int value)
{
    double result = RANGEMIN + ((RANGEMAX - RANGEMIN) / 1023.00) * (double)value;
    return result;
}

void logToSdcard(double *datas)
{
    String message = "";
    message += getTimeStamp();
    message += ",";
    for (int i = 0; i < DATACOLUMN; i++)
    {
        message += String(datas[i]);
        if (i < DATACOLUMN - 1)
            message += ",";
    }

    File dataFile = SD.open(dataFileName, FILE_WRITE);
    if (dataFile)
    {
        dataFile.println(message);
        dataFile.close();
    }
    else
    {
        Serial.println("Error opening file.");
    }
}

String getTimeStamp()
{
    String timeStamp = "";
    DateTime now = rtc.now();
    long currentMillis = millis();
    timeStamp += now.timestamp();
    timeStamp += ",";
    timeStamp += String(currentMillis - lastMillis);
    timeStamp += ",";
    lastMillis = currentMillis;
}