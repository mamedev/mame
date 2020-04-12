// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#include "ncurses.h"

MExternC WINDOW* MCallingConvention initscr(void)
{
	return 0;
}

MExternC int MCallingConvention cbreak(void)
{
	return -1;
}

MExternC int MCallingConvention noecho(void)
{
	return -1;
}

MExternC int MCallingConvention clear(void)
{
	return -1;
}

MExternC int MCallingConvention mvaddstr(int row, int col, const char* text)
{
	return -1;
}

MExternC int MCallingConvention mvprintw(int row, int col,const char* format, ...)
{
	return -1;
}

MExternC int MCallingConvention getch(void)
{
	return -1;
}

MExternC int MCallingConvention delwin(WINDOW* win)
{
	return -1;
}

MExternC int MCallingConvention endwin(void)
{
	return -1;
}
