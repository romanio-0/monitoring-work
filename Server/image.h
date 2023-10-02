#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <Windows.h>
#include <iostream>
#include <gdiplus.h>

#include "sup.h"

HBITMAP dataToHBmp(screenData *screen);

void saveScreen(screenData *screen, const wchar_t *path);

void printScreen(screenData *screen);

#endif //_IMAGE_H_
