#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdint.h>

extern uint8_t const RelayIndexValve;

extern uint8_t const RelayIndexCueShutter;
extern uint8_t const RelayIndexReleaseShutter;

// Time that the shutter will be released for; that is, the amount of
// time that the shutter button will be held fully down before letting
// go.
extern unsigned long const ShutterReleaseTimeMillis;

// Time to cue the camera (hold the button "half down") before
// releasing the shutter.
extern unsigned long const ShutterPrepareTimeMillis;

#endif
