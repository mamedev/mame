#ifndef _SHARC_H
#define _SHARC_H

#include "cpuintrf.h"

#define SHARC_INPUT_FLAG0		3
#define SHARC_INPUT_FLAG1		4
#define SHARC_INPUT_FLAG2		5
#define SHARC_INPUT_FLAG3		6

typedef enum {
	BOOT_MODE_EPROM,
	BOOT_MODE_HOST,
	BOOT_MODE_LINK,
	BOOT_MODE_NOBOOT
} SHARC_BOOT_MODE;

typedef struct {
	SHARC_BOOT_MODE boot_mode;
} sharc_config;

extern void sharc_set_flag_input(int flag_num, int state);

extern void sharc_external_iop_write(UINT32 address, UINT32 data);
extern void sharc_external_dma_write(UINT32 address, UINT64 data);

#if (HAS_ADSP21062)
void adsp21062_get_info(UINT32 state, cpuinfo *info);
#endif

#ifdef MAME_DEBUG
extern UINT32 sharc_dasm_one(char *buffer, offs_t pc, UINT64 opcode);
#endif

#endif /* _SHARC_H */
