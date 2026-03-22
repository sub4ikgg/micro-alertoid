#ifndef RESOURCE_H
#define RESOURCE_H

#include <Arduino.h>

void checkResourceAvailability();
void writeResourceConf(String url, int expectedCode, int checkInterval);
int getResourceCheckInterval();

#endif
