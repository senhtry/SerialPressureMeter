#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <avr/wdt.h>
#include "SerialPressureMeter.h"

#define ANALOGPIN 0
#define PERIOD 100
#define DATACOLUMN 3
#define RANGEMAX 6000
#define RANGEMIN 0
#define SDCHIPSELECT 10
#define THRESHOLD 3000
#define LOGFILE "datalog.csv"

RTC_DS1307 rtc;
String dataFileName;

void setup()
{
    // Initilize serial port
    while (!Serial)
        ;
    Serial.begin(115200);

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
    Serial.print("RTC initialized, current time: ");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(getTimeStamp());

    // Initialize SD card
    Serial.println("Initializing SD card ...");
    if (!SD.begin(SDCHIPSELECT))
    {
        Serial.println("SD card failed, or not present");
        while (1)
            ;
    }
    Serial.println("SD card initialized.");
    dataFileName = LOGFILE;

    // Watchdog
    wdt_enable(WDTO_1S);
}

void loop()
{
    // Reset watchdog
    wdt_reset();

    // Get sensor value
    double datas[DATACOLUMN];
    int _sensorValue = analogRead(ANALOGPIN);

    datas[0] = analogToPressure(_sensorValue);
    datas[1] = _sensorValue;
    if (datas[0] >= THRESHOLD)
        datas[2] = 1;
    else
        datas[2] = 0;

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
        // Serial.println(message);
    }
    else
    {
        Serial.println("Error opening file.");
    }
}

double analogToPressure(int value)
{
    double result = RANGEMIN + ((RANGEMAX - RANGEMIN) / 1023.00) * (double)value;
    return result;
}

String getTimeStamp()
{
    String timeStamp = "";
    DateTime now = rtc.now();
    timeStamp += now.year();
    timeStamp += "-";
    timeStamp += now.month();
    timeStamp += "-";
    timeStamp += now.day();
    timeStamp += ",";
    timeStamp += now.hour();
    timeStamp += ":";
    timeStamp += now.minute();
    timeStamp += ":";
    timeStamp += now.second();
    return timeStamp;
}