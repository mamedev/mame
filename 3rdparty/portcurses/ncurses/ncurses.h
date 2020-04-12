// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#if !defined(NCURSES_H)
#define NCURSES_H

#if defined(__cplusplus)
#define MExternC extern "C"
#else
#define MExternC
#endif

#if defined(_MSC_VER )
#define MCallingConvention __cdecl
#else
#define MCallingConvention
#endif

typedef void WINDOW;

MExternC WINDOW* MCallingConvention initscr(void);

MExternC int MCallingConvention cbreak(void);

MExternC int MCallingConvention noecho(void);

MExternC int MCallingConvention clear(void);

MExternC int MCallingConvention mvaddstr(int row, int col, const char* text);

MExternC int MCallingConvention mvprintw(int row, int col, const char* format, ...);

MExternC int MCallingConvention getch(void);

MExternC int MCallingConvention delwin(WINDOW* win);

MExternC int MCallingConvention endwin(void);

#endif /* NCURSES_H */
