#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 0

#if DEBUG
  #define LOG(x) Serial.println(x)
#else
  #define LOG(x)
#endif

#endif
