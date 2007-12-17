/***************************************************************************

    debugcmt.h

    Debugger code-comment management functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DEBUGCMT_H__
#define __DEBUGCMT_H__

#define DEBUG_COMMENT_MAX_LINE_LENGTH (128)	/* 128 characters per comment - 50 are displayed at once */
#define DEBUG_COMMENT_MAX_NUM (0x10000)		/* 64k comments should be good for awhile */

/* init and exit */
int debug_comment_init(running_machine *machine);

/* load and save */
int debug_comment_save(void);
int debug_comment_load(void);

/* comment interface functions */
int debug_comment_add(int cpu_num, offs_t addr, const char *comment, rgb_t color, UINT32 c_crc);
int debug_comment_remove(int cpu_num, offs_t addr, UINT32 c_crc);

const char *debug_comment_get_text(int cpu_num, offs_t addr, UINT32 c_crc);
int debug_comment_get_count(int cpu_num);
UINT32 debug_comment_get_change_count(int cpu_num);
UINT32 debug_comment_all_change_count(void);

/* local functionality */
UINT32 debug_comment_get_opcode_crc32(offs_t address);	/* pull a crc for the opcode at a given address */
void debug_comment_dump(int cpu_num, offs_t addr);		/* dump all (or a single) comment to the command line */

#endif
