#pragma once
/**
 * Contains all the logic for the level.
 */
class Level
{
private:
    //The index of the current square that should be touched.
    unsigned int _currentSquare;
    //Time remaining before it's game over.
    unsigned int _timeRemaining;
    //Current level number.
    unsigned int _currentLevel;
    //Highscore of the current session.
    unsigned int _highscore;
public:
    Level();
    ~Level();
    //Generates the next level.
    void next();
    //Reset the level back to 0.
    void reset();
    unsigned int getCurSquare();
    unsigned int getTimeDuration();
    unsigned int getLevelnumber();
    //Updates the highschore and returns it.
    unsigned int getHighscore();
private:
    //Generates a random time, how higher the level, the shorter the time remaining.
    void _generateTime();
};
