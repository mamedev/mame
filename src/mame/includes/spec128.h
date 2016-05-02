// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/spec128.h
 *
 ****************************************************************************/

#ifndef __SPEC128H__
#define __SPEC128H__

/* 128K machines take an extra 4 cycles per scan line - add this to retrace */
#define SPEC128_UNSEEN_LINES    15
#define SPEC128_RETRACE_CYCLES  52
#define SPEC128_CYCLES_PER_LINE 228

MACHINE_CONFIG_EXTERN( spectrum_128 );

#endif /* __SPEC128H__ */
