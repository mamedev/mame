/***************************************************************************

    debugcmt.h

    Debugger code-comment management functions.

    Copyright Nicola Salmoria and the MAME Team.
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
int debug_comment_save(running_machine *machine);
int debug_comment_load(running_machine *machine);

/* comment interface functions */
int debug_comment_add(const device_config *device, offs_t addr, const char *comment, rgb_t color, UINT32 c_crc);
int debug_comment_remove(const device_config *device, offs_t addr, UINT32 c_crc);

const char *debug_comment_get_text(const device_config *device, offs_t addr, UINT32 c_crc);
int debug_comment_get_count(const device_config *device);
UINT32 debug_comment_get_change_count(const device_config *device);
UINT32 debug_comment_all_change_count(running_machine *machine);

/* local functionality */
UINT32 debug_comment_get_opcode_crc32(const device_config *device, offs_t address);	/* pull a crc for the opcode at a given address */
void debug_comment_dump(const device_config *device, offs_t addr);		/* dump all (or a single) comment to the command line */

#endif
