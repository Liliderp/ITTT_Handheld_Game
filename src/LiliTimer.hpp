#pragma once
#include <Arduino.h>
/**
 *Timer used for the touch, the game duration itself and for the vibrator.
 Simple timer. 
*/
class LiliTimer
{
private:
    const char *_name;
    unsigned long _startTime;

public:
    LiliTimer(const char *s);
    ~LiliTimer();
    unsigned long start();
    void stop();
    unsigned long delta();
    unsigned long reset();
};
