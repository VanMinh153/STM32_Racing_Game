#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

#define LCD_WIDTH 240
#define LCD_HEIGHT 320
// 1,2,3,5,8,13,21

enum Level_t {
//	LV1 = 0,
	LV2 = 30,
	LV3 = 80,
	LV4 = 150,
	LV5 = 250
};

enum Score_t {
	LV1_SCORE = 21,
	LV2_SCORE = 16,
	LV3_SCORE = 13,
	LV4_SCORE = 10,
	LV5_SCORE = 8
};


class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();
    void handleBackground();
    void handleSheep();
    void handleCar();
    void calcScore();
    void calcLevel();
    bool checkCrash();
    void setupJoystick();
    void getNewXY();
    void getCoordinate();

protected:
    int16_t localImageX;
    uint32_t tickCount;
    uint16_t score;
    uint8_t level;
    uint16_t _x, _y;
    uint16_t _xC, _yC;
    uint16_t X0, Y0, Xmin, Ymin;
    bool GameOver = false;
};

#endif // SCREEN2VIEW_HPP
