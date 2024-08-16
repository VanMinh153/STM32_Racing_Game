#include <gui/screen2_screen/Screen2View.hpp>

#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include "main.h"

Screen2View::Screen2View()
{
	tickCount = 0;
	score = 0;
	level = 1;
	Xmin = 1550;
	Ymin = 1550;
	Unicode::snprintf(textScoreBuffer, TEXTSCORE_SIZE, "%d", score);
	Unicode::snprintf(textLevelBuffer, TEXTLEVEL_SIZE, "%d", level);
	invalidate();

	setupJoystick();
	char mess[40];
	sprintf(mess, "#Setup %d,%d - %d,%d\n", X0, Y0, Xmin, Ymin);
	HAL_UART_Transmit(&huart1, (uint8_t *) mess, strlen(mess), 10);
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

	localImageX = presenter->GetImageX();
    imgCar.setX(localImageX);
    imgSheep.setX(15);
    imgSheep.setY(0);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
    presenter->UpdateImageX(localImageX);
}

void Screen2View::handleTickEvent()
{
	Screen2ViewBase::handleTickEvent();
	tickCount++;
	if (GameOver)
	{
		osDelay(1);
		return;
	}

	calcScore();
	calcLevel();
	handleBackground();
	handleSheep();
	handleCar();
	checkCrash();
	if (GameOver == true)
	{
		textGameOver.setVisible(true);
		HAL_UART_Transmit(&huart1, (uint8_t *) "Game Over!\n", strlen("Game Over!\n"), 10);
	}
	invalidate();
}

void Screen2View::handleBackground()
{
	uint8_t speed, t;
	if (level == 1) speed = 15;
	else if (level == 2) speed = 10;
	else speed = 5;

	t = tickCount % speed;
	if (t % (speed/5) == 0) t = t / (speed/5);
	else return;

	switch (t)
	{
	case 0:
		track0.setVisible(true);
		track4.setVisible(false);
		break;
	case 1:
		track1.setVisible(true);
		track0.setVisible(false);
		break;
	case 2:
		track2.setVisible(true);
		track1.setVisible(false);
		break;
	case 3:
		track3.setVisible(true);
		track2.setVisible(false);
		break;
	case 4:
		track4.setVisible(true);
		track3.setVisible(false);
		break;
	default:
		break;
	}
}

void Screen2View::handleSheep()
{
	const uint8_t height = imgSheep.getHeight();
	uint8_t speed = level;
	int16_t x, y;
	x = imgSheep.getX();
	y = imgSheep.getY();

	y += speed;

	if (y >= LCD_HEIGHT)
	{
		y = -height;
		x += 60;
		if (x > LCD_WIDTH) x = 15;
		imgSheep.setX(x);
	}

	imgSheep.setY(y);
}

void Screen2View::handleCar()
{
	int16_t x = imgCar.getX();
	int16_t y = imgCar.getY();
	getCoordinate();
	if (abs(_xC - x) < 5 && abs(_yC - y) < 5)
		return;
	if (abs(_xC - x) > 20 || abs(_yC - y) > 20)
		getCoordinate();

	int16_t dx, dy;
	uint8_t t = 0;
	dx = _xC - x;
	dy = _yC - y;
	t = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	t /= 10;
	dx /= t;
	dy /= t;

	while (t > 0)
	{
		t -= 1;
		x += dx;
		y += dy;
		imgCar.setXY(x, y);
		imgCar.invalidate();
		if (checkCrash() == true)
			return;
	}
	x = _xC;
	y = _yC;
	imgCar.setXY(x, y);
}

void Screen2View::calcScore()
{
	uint8_t t;
	switch (level) {
	case 1:
		t = LV1_SCORE;
		break;
	case 2:
		t = LV2_SCORE;
		break;
	case 3:
		t = LV3_SCORE;
		break;
	case 4:
		t = LV4_SCORE;
		break;
	case 5:
		t = LV5_SCORE;
		break;
	default:
		return;
		break;
	}

	if (tickCount % t == 0)
	{
		score++;
		Unicode::snprintf(textScoreBuffer, TEXTSCORE_SIZE, "%d", score);
	}
}

void Screen2View::calcLevel()
{
	bool flag = true;
	switch (score) {
	case LV2:
		level = 2;
		break;
	case LV3:
		level = 3;
		break;
	case LV4:
		level = 4;
		break;
	case LV5:
		level = 5;
		break;
	default:
		flag = 0;
		break;
	}

	if (flag == true)
		Unicode::snprintf(textLevelBuffer, TEXTLEVEL_SIZE, "%d", level);
}

bool Screen2View::checkCrash()
{
	Rect car = imgCar.getRect();
	Rect sheep = imgSheep.getRect();
	touchgfx::Rect r = car & sheep;

	if (  (r.width > 6 && r.height > 3)
	  && !(r.width + r.height < 20))
	{
		GameOver = true;
		return true;
	}
	return false;
}

void Screen2View::setupJoystick()
{
	uint32_t joystick;
	for (uint8_t i = 0; i < 16; i++)
		osMessageQueueGet(Queue01Handle, &joystick, NULL, 0);

	uint32_t x0, y0;
	const uint8_t ntest = 100;
	x0 = y0 = 0;

	for (uint8_t i = 0; i < ntest; i++)
	{
		getNewXY();
		x0 += _x;
		y0 += _y;
	}

	X0 = x0 / ntest;
	Y0 = y0 / ntest;
}

void Screen2View::getNewXY()
{
	uint32_t joystick;
	osMessageQueueGet(Queue01Handle, &joystick, NULL, osWaitForever);
	_y = joystick >> 16;
	_x = joystick & 0xFFFF;

	if (_x < Xmin) Xmin = _x;
	if (_y < Ymin) Ymin = _y;
}

void Screen2View::getCoordinate()
{
	const uint16_t Xmax = 4095;
	const uint16_t Ymax = 4095;
	const uint16_t width = (LCD_WIDTH - imgCar.getWidth()) / 2;
	const uint16_t height = 50;
	float t;
	getNewXY();

	if (_x < X0)
	{
		t = (float)(_x - Xmin)/(X0 - Xmin);
		_xC = (int) (t*width);
	}
	else if (_x > X0)
	{
		t = (float)(_x - X0)/(Xmax - X0);
		_xC = (int) (t*width) + width;
	}

	if (_y < Y0)
	{
		t = (float)(_y - Ymin)/(Y0 - Ymin);
		_yC = (int) (t*height);
	}
	else if (_y > Y0)
	{
		t = (float)(_y - Y0)/(Ymax - Y0);
		_yC = (int) (t*height) + height;
	}
	_yC = LCD_HEIGHT - imgCar.getHeight() - 1 - _yC;
}
