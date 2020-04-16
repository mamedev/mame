// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#include "ncurses.h"

#if defined(_MSC_VER)
#include <windows.h>
#include <strsafe.h>
#include <varargs.h>

typedef struct tagTCnsoleData
{
	HANDLE hStdIn;
	HANDLE hStdOut;
	DWORD dwOriginalMode;
} TConsoleData;

static TConsoleData l_ConsoleData = {0};

static LPVOID lAllocMem(DWORD dwLen)
{
	return HeapAlloc(GetProcessHeap(), 0, dwLen);
}

static LPVOID lReAllocMem(LPVOID pvMem, DWORD dwLen)
{
	return HeapReAlloc(GetProcessHeap(), 0, pvMem, dwLen);
}

static VOID lFreeMem(LPVOID pvMem)
{
	HeapFree(GetProcessHeap(), 0, pvMem);
}
#endif

MExternC WINDOW* MCallingConvention initscr(void)
{
#if defined(_MSC_VER)
	l_ConsoleData.hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	l_ConsoleData.hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!GetConsoleMode(l_ConsoleData.hStdIn, &l_ConsoleData.dwOriginalMode) ||
		!SetConsoleMode(l_ConsoleData.hStdIn, ENABLE_PROCESSED_INPUT | ENABLE_WINDOW_INPUT))
	{
		l_ConsoleData.hStdIn = NULL;
		l_ConsoleData.hStdOut = NULL;

		return 0;
	}

	clear();

	return (WINDOW*)&l_ConsoleData;
#else
	return 0;
#endif
}

MExternC int MCallingConvention cbreak(void)
{
#if defined(_MSC_VER)
	return OK;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention noecho(void)
{
#if defined(_MSC_VER)
	return OK;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention clear(void)
{
#if defined(_MSC_VER)
	COORD Coord = {0, 0};
	DWORD dwCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	DWORD dwConsoleSize;

	if (!GetConsoleScreenBufferInfo(l_ConsoleData.hStdOut, &ConsoleScreenBufferInfo))
	{
		return ERR;
	}

	dwConsoleSize = ConsoleScreenBufferInfo.dwSize.X * ConsoleScreenBufferInfo.dwSize.Y;
	
	if (!FillConsoleOutputCharacter(l_ConsoleData.hStdOut, ' ', dwConsoleSize, Coord, &dwCharsWritten))
	{
		return ERR;
	}

	if (!GetConsoleScreenBufferInfo(l_ConsoleData.hStdOut, &ConsoleScreenBufferInfo))
	{
		return ERR;
	}

	if (!FillConsoleOutputAttribute(l_ConsoleData.hStdOut, ConsoleScreenBufferInfo.wAttributes, dwConsoleSize, Coord, &dwCharsWritten))
	{
		return ERR;
	}

	SetConsoleCursorPosition(l_ConsoleData.hStdOut, Coord);

	return OK;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention mvaddstr(int row, int col, const char* text)
{
#if defined(_MSC_VER)
	COORD Coord;

	Coord.X = col;
	Coord.Y = row;

	SetConsoleCursorPosition(l_ConsoleData.hStdOut, Coord);

	if (!WriteConsole(l_ConsoleData.hStdOut, text, lstrlen(text), NULL, NULL))
	{
		return ERR;
	}

	return OK;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention mvprintw(int row, int col, const char* format, ...)
{
#if defined(_MSC_VER)
	INT nBufferLen = lstrlenA(format) + 20;
	LPSTR pszBuffer = (LPSTR)lAllocMem(nBufferLen * sizeof(char));
	LPSTR pszNewBuffer;
	int result;
	va_list arguments;

	if (pszBuffer == NULL)
	{
		return ERR;
	}

	va_start(arguments, format);

	while (S_OK != StringCchVPrintfA(pszBuffer, nBufferLen, format, arguments))
	{
		nBufferLen += 20;

		pszNewBuffer = (LPSTR)lReAllocMem(pszBuffer, nBufferLen);

		if (pszNewBuffer == NULL)
		{
			lFreeMem(pszBuffer);

			return ERR;
		}

		pszBuffer = pszNewBuffer;
	}

	result = mvaddstr(row, col, pszBuffer);

	lFreeMem(pszBuffer);

	va_end(arguments);

	return result;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention getch(void)
{
#if defined(_MSC_VER)
	BOOL bQuit = FALSE;
	INPUT_RECORD InputRecord;
	DWORD dwNumberOfEventsRead;

	while (!bQuit)
	{
		if (ReadConsoleInput(l_ConsoleData.hStdIn, &InputRecord, 1, &dwNumberOfEventsRead))
		{
			if (dwNumberOfEventsRead > 0)
			{
				switch (InputRecord.EventType)
				{
					case KEY_EVENT:
						if (InputRecord.Event.KeyEvent.bKeyDown && InputRecord.Event.KeyEvent.uChar.AsciiChar != 0)
						{
							return InputRecord.Event.KeyEvent.uChar.AsciiChar;
						}
						break;
				}
			}
		}
		else
		{
			bQuit = TRUE;
		}
	}

	return ERR;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention delwin(WINDOW* win)
{
#if defined(_MSC_VER)
	SetConsoleMode(l_ConsoleData.hStdIn, l_ConsoleData.dwOriginalMode);

	l_ConsoleData.hStdIn = NULL;
	l_ConsoleData.hStdOut = NULL;

	return OK;
#else
	return ERR;
#endif
}

MExternC int MCallingConvention endwin(void)
{
#if defined(_MSC_VER)
	return OK;
#else
	return ERR;
#endif
}
