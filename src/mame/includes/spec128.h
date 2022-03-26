// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/spec128.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_SPEC128_H
#define MAME_INCLUDES_SPEC128_H

#define X1_128_AMSTRAD  35'469'000       // Main clock (Amstrad 128K model, +2A?)
#define X1_128_SINCLAIR 35.469_MHz_XTAL  // Main clock (Sinclair 128K model)

/* 128K machines take an extra 4 cycles per scan line - add this to retrace */
#define SPEC128_UNSEEN_LINES    15
#define SPEC128_RETRACE_CYCLES  52
#define SPEC128_CYCLES_PER_LINE 228

#endif // MAME_INCLUDES_SPEC128_H
