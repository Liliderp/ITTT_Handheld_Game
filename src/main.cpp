#define LGFX_AUTODETECT
#define LGFX_USE_V1
#define PIN_VIBRATOR 27

#include <LovyanGFX.hpp>
#include "config.hpp"
#include "LiliTimer.hpp"
#include "Level.hpp"
// 2d point
typedef struct vec2
{
    int x;
    int y;
} vec2;

// Representation of a square.
typedef struct square
{
    const vec2 point;
    uint32_t color;
    bool filled;
    bool touched;
} square;

typedef struct time_bar
{
    const vec2 pos;
    const vec2 dimensions;
    unsigned int pixels_filled;
} time_bar;
// Initial values the squares need to have.
static square SQUARES[] =
    {
        {{20, 80}, SQUARE_DEFAULT_COLOR, false, false},
        {{175, 80}, SQUARE_DEFAULT_COLOR, false, false},
        {{330, 80}, SQUARE_DEFAULT_COLOR, false, false},
};

static time_bar timeBar =
    {
        {TIME_BAR_X, TIME_BAR_Y},
        {TIME_BAR_WIDTH, 30},
        0,
};
// Menu scherm, game die afspeelt en game over scherm.
enum GAME_STATE
{
    GAME_STATE_MENU,
    GAME_STATE_RUNNING,
    GAME_STATE_DEAD,
};

static LGFX lcd;
static int32_t touched_x, touched_y;
LiliTimer vibratorTimer("vibratorTimer");
LiliTimer lvlTimer("lvlTimer");
LiliTimer touchInterruptTimer("touchTimer");
Level lvl;

unsigned int pixels_filled_old = 0;
GAME_STATE gameState = GAME_STATE_MENU;

void timeBarClear();
// Draws a square with background color to clear the screen.
void textClear()
{
    lcd.fillRect(TEXT_LEFT_X, TEXT_LEFT_Y, 100, 20, BACKGROUND_COLOR);
}

void textPrintLvl(unsigned int lvl)
{
    textClear();
    lcd.setFont(&Font4);
    lcd.setCursor(TEXT_LEFT_X, TEXT_LEFT_Y);
    lcd.printf("lvl: %d", lvl);
}

void textPrint(const char *s)
{
    textClear();
    lcd.setFont(&Font4);
    lcd.setCursor(TEXT_LEFT_X, TEXT_LEFT_Y);
    lcd.printf(s);
}
// Converts color values.
uint8_t color24to8(uint32_t color)
{
    return lcd.color16to8(lcd.color24to16(color));
}
// Draws the square and fills the square if the fill bool is true.
void squareDraw(vec2 point, bool fill, uint32_t color)
{
    if (fill)
    {
        lcd.fillRect(point.x, point.y, SQUARE_WIDTH, SQUARE_HEIGHT, color24to8(color));
    }
    else
    {
        squareDraw(point, true, BACKGROUND_COLOR);
        lcd.drawRect(point.x, point.y, SQUARE_WIDTH, SQUARE_HEIGHT, color24to8(color));
    }
}

void squareSetActive(unsigned int index)
{
    for (int i = 0; i < SQUARE_COUNT; i++)
    {
        if (i == index)
            SQUARES[i].filled = true;
        else
            SQUARES[i].filled = false;
    }
}
// Function for coloring the squares by touch.
void squareTouched(unsigned int index)
{
    // Loops through all the squares, checks if the filled square is touched.
    for (int i = 0; i < SQUARE_COUNT; i++)
    {
        // Based on which square you touched, sets color accordingly.
        if (i == index)
        {
            if (SQUARES[i].filled)
                SQUARES[i].color = SQUARE_TOUCHED_COLOR;
            else
                SQUARES[i].color = SQUARE_TOUCHED_WRONG_COLOR;
            SQUARES[i].touched = true;
        }
        // Sets all the other squares back to their default color.
        else
        {
            SQUARES[i].color = SQUARE_DEFAULT_COLOR;
            SQUARES[i].touched = false;
        }
    }
}

int squareGetTouched()
{
    for (int i = 0; i < SQUARE_COUNT; i++)
    {
        if (SQUARES[i].touched)
            return i;
    }
    return -1;
}
// Checks if there is collision with the squares.
void squareHandleCollision()
{
    for (int i = 0; i < SQUARE_COUNT; i++)
    {
        square sq = SQUARES[i];
        if (touched_x >= sq.point.x && touched_x <= sq.point.x + SQUARE_WIDTH)
            if (touched_y >= sq.point.y && touched_y <= sq.point.y + SQUARE_HEIGHT)
            {
                // Set the square to be touched.
                squareTouched(i);
                return;
            }
    }
}

void drawGame(bool reset_state)
{
    // Draw the squares.
    static int8_t old_states[3] = {-1, -1, -1};
    static unsigned int old_lvl = -1;
    // Resets all previous drawn graphics in the screen.
    if (reset_state)
    {
        old_states[0] = -1;
        old_states[1] = -1;
        old_states[2] = -1;
        old_lvl = -1;
        pixels_filled_old = 0;
        timeBarClear();
        return;
    }
    // Forloop draws the squares based on the change in state.
    for (int i = 0; i < SQUARE_COUNT; i++)
    {
        square sq = SQUARES[i];
        int8_t state = (sq.filled << 0) + (sq.touched << 1);

        if (sq.filled || sq.touched)
            squareDraw(sq.point, true, sq.color);
        else if (old_states[i] != state)
            squareDraw(sq.point, false, sq.color);
        old_states[i] = state;
    }

    // Draw the new time bar.
    for (int x = 0; x < timeBar.pixels_filled; x++)
    {
        lcd.fillRect(timeBar.pos.x + x, timeBar.pos.y, 1, timeBar.dimensions.y, color24to8(TIME_BAR_COLOR));
    }
    pixels_filled_old = timeBar.pixels_filled;
    // Draws level number.
    unsigned int tmp_lvl = lvl.getLevelnumber();
    if (tmp_lvl != old_lvl)
    {
        textPrintLvl(tmp_lvl);
        old_lvl = tmp_lvl;
    }
}
// Returns a bool from getTouch and it checks if we have collision with a square.
int checkTouch()
{
    int touched = lcd.getTouch(&touched_x, &touched_y);
    // If the game is running, check collision with squares.
    if (gameState == GAME_STATE_RUNNING)
        squareHandleCollision();
    touched_x = 0;
    touched_y = 0;
    return touched;
}

void timeBarClear()
{
    // Clear old timebar.
    lcd.fillRect(timeBar.pos.x, timeBar.pos.y, pixels_filled_old, timeBar.dimensions.y, BACKGROUND_COLOR);
    // Draw new timebar outline.
    lcd.drawRect(timeBar.pos.x, timeBar.pos.y, timeBar.dimensions.x, timeBar.dimensions.y, color24to8(TIME_BAR_COLOR));
}

void timeBarUpdate(unsigned long amount, unsigned long max_value)
{
    timeBar.pixels_filled = (amount * timeBar.dimensions.x) / max_value;
}

void gameReset()
{
    gameState = GAME_STATE_RUNNING;
    lvl.reset();
    lvlTimer.reset();
    lcd.clear(BACKGROUND_COLOR);
    // When given true to the drawGame function, resets all graphics.
    drawGame(true);
    // squareTouched -1 reset the touched squares to 0.
    squareTouched(-1);
}
// Checks if you touch the right square, if not then it's game over/dead.
void gameNextLevel()
{ // Go to next level if you touch right square.
    if (squareGetTouched() == lvl.getCurSquare())
        lvl.next();
    else
        gameState = GAME_STATE_DEAD;
    timeBarClear();
    // Sets up the next square that should be touched.
    squareSetActive(lvl.getCurSquare());
    // reset the touched square.
    squareTouched(-1);
}

void gameStart()
{
    lvlTimer.start();
    touchInterruptTimer.start();
}

void gameLoop()
{
    static bool touched = false;

    if (!touched && checkTouch())
    {
        touchInterruptTimer.start();
        touched = true;
    }
    if (lvlTimer.delta() > lvl.getTimeDuration() || touchInterruptTimer.delta() > LEVEL_TOUCH_TIME)
    {
        // Checks if sqaure has been touched.
        gameNextLevel();
        lvlTimer.reset();
        touchInterruptTimer.stop();
        touched = false;
    }
    // Update the timebar with the remaining time.
    timeBarUpdate(lvlTimer.delta(), lvl.getTimeDuration());
    squareSetActive(lvl.getCurSquare());
    // Draw all the graphics.
    drawGame(false);
}

// First draws the menu text, then just waits for touch input.
void gameMenu()
{
    static bool init = true;
    // If statement will only run once because of the init bool.
    if (init)
    { // Text for the start screen.
        lcd.clear(BACKGROUND_COLOR);
        lcd.setFont(&Font4);
        lcd.setCursor(START_TEXT_X, START_TEXT_Y);
        lcd.printf("Press any button to start.");
        init = false;
    }
    if (checkTouch())
    { // Checks if the screen has been touched, if yes, the game will start playing.
        gameState = GAME_STATE_RUNNING;
        init = true;
        delay(500);
        // Reset game values to 0.
        gameReset();
    }
}

void gameOver()
{
    static bool init = true;

    // Draws gameover text once, then waits for user input, resets the game.
    if (init)
    {

        lcd.clear(BACKGROUND_COLOR);
        lcd.setCursor(DIED_TEXT_X, DIED_TEXT_Y);

        lcd.setFont(&Font4);
        lcd.setTextColor(YOU_DIED_TEXT_COLOR);
        lcd.setTextSize(4, 4);
        lcd.print("YOU DIED");
        lcd.setTextSize(1, 1);

        lcd.setTextColor(HIGHSCORE_TEXT_COLOR);
        lcd.setFont(&Font4);
        lcd.setCursor(SCORE_TEXT_X, SCORE_TEXT_Y);
        lcd.printf("Score %u", lvl.getLevelnumber());
        lcd.setCursor(SCORE_TEXT_X - 20, SCORE_TEXT_Y + 30);
        lcd.printf("Highscore %u", lvl.getHighscore());

        lcd.setTextColor(0xFFFFFFU);
        lcd.setCursor(START_TEXT_X, START_TEXT_Y);
        lcd.printf("Press any button to restart.");

        init = false;
    }
    if (checkTouch())
    {
        init = true;
        delay(500);
        gameReset();
    }
}
// Based on the current gamestate either turn vibrator on or off.
void handleVibrator()
{
    // Save the old game state to detect the difference in state.
    static GAME_STATE stateOld = gameState;
    // If the vibrator has been on for more than 500ms, turn the vibrator off.
    if (vibratorTimer.delta() >= 500)
    {
        vibratorTimer.stop();
        digitalWrite(PIN_VIBRATOR, LOW);
        stateOld = gameState;
    }
    else if (gameState != stateOld)
    {
        if (gameState == GAME_STATE_DEAD)
        {
            digitalWrite(PIN_VIBRATOR, HIGH);
            vibratorTimer.start();
        }
        else
            digitalWrite(PIN_VIBRATOR, LOW);
        stateOld = gameState;
    }
}

void setup(void)
{ // Analog read to get a random value.
    randomSeed(analogRead(36));
    // Setup lcd library.
    lcd.init();
    // Landscapemode
    if (lcd.width() < lcd.height())
        lcd.setRotation(lcd.getRotation() ^ 1);
    // Pin is an output and not an input.
    pinMode(PIN_VIBRATOR, OUTPUT);
    // Initially turn vibrator off.
    digitalWrite(PIN_VIBRATOR, LOW);
}

void loop()
{
    // Checks current game state and handles accordingly.
    if (gameState == GAME_STATE_MENU)
        gameMenu();
    else if (gameState == GAME_STATE_RUNNING)
        gameLoop();
    else if (gameState == GAME_STATE_DEAD)
        gameOver();

    handleVibrator();
}
