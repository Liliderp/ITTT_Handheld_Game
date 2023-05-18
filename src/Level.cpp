#include "Level.hpp" //Relative path.
#include <Arduino.h> //Library die geinstalleerd is.
#include "config.hpp"
//Zorgt ervoor dat de moeilijkheidsgraad omhoog gaat gedurende het spelen.
void Level::_generateTime()
{
    int start_max = LEVEL_TIME_MAX - _currentLevel * 50;;
    int start_min = LEVEL_TIME_MAX - LEVEL_TIME_MIN -_currentLevel * 50;

    if (start_min <= LEVEL_TIME_MIN)
        start_min = LEVEL_TIME_MIN;
    if (start_max <= LEVEL_TIME_MIN * 2)
        start_max = LEVEL_TIME_MIN * 2;

    _timeRemaining = random(start_min, start_max);
    _timeRemaining -= (_timeRemaining % 50);
}

void Level::reset()
{
    _currentLevel = 0;
    next();
    _currentLevel = 0;
}

void Level::next()
{
    _currentSquare = random(0, 3);
    _generateTime();
    _currentLevel++;
}

unsigned int Level::getHighscore()
{
    if (_currentLevel > _highscore)
        _highscore = _currentLevel;
    return _highscore;
}


unsigned int Level::getCurSquare()
{
    return _currentSquare;
}

unsigned int Level::getTimeDuration()
{
    return _timeRemaining;
}

unsigned int Level::getLevelnumber()
{
    return _currentLevel;
}
Level::Level()
{
    next();
    _currentLevel = 0;
    _highscore = 0;
}

Level::~Level(){}