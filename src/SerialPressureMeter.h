#ifndef SerialPressureMeter_h_
#define SerialPressureMeter_h_

void sendToSerial(double *);
void logToSdcard(double *);
double analogToPressure(int);
String getTimeStamp();

#endif