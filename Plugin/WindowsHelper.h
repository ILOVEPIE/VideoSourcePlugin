#pragma once

#include "OBSApi.h"
#include <windowsx.h>

#define Button_IsChecked(hwnd) Button_GetCheck(hwnd) ? true : false

#define Slider_SetRange(hwndCtl, posMin, posMax) ((int)(DWORD)SNDMSG((hwndCtl), TBM_SETRANGE, (WPARAM)(int)(1), (LPARAM)MAKELONG(posMin, posMax)))
#define Slider_GetPos(hwndCtl) ((int)(DWORD)SNDMSG((hwndCtl), TBM_GETPOS, (WPARAM)(int)(0), (LPARAM)(int)(0)))
#define Slider_SetPos(hwndCtl, pos) ((int)(DWORD)SNDMSG((hwndCtl), TBM_SETPOS, (WPARAM)(int)(1), (LPARAM)pos))