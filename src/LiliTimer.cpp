#include "LiliTimer.hpp"
/**
 *This is where the functions from the LiliTimer class are being implemented. 
*/
LiliTimer::LiliTimer(const char *name)
{
    this->_name = name;
}

LiliTimer::~LiliTimer()
{
}
void LiliTimer::stop()
{
    _startTime = 0;
}
unsigned long LiliTimer::start()
{
    _startTime = millis();
    return _startTime;
}
//Returns the difference in time.
unsigned long LiliTimer::delta()
{
    if (!_startTime)
        return 0;
    return millis() - _startTime;
}

unsigned long LiliTimer::reset()
{
    return start();
}
