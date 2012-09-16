#pragma once

#ifndef __SHARC_H__
#define __SHARC_H__


#define SHARC_INPUT_FLAG0		3
#define SHARC_INPUT_FLAG1		4
#define SHARC_INPUT_FLAG2		5
#define SHARC_INPUT_FLAG3		6

enum SHARC_BOOT_MODE 
{
	BOOT_MODE_EPROM,
	BOOT_MODE_HOST,
	BOOT_MODE_LINK,
	BOOT_MODE_NOBOOT
};

struct sharc_config {
	SHARC_BOOT_MODE boot_mode;
};

extern void sharc_set_flag_input(device_t *device, int flag_num, int state);

extern void sharc_external_iop_write(device_t *device, UINT32 address, UINT32 data);
extern void sharc_external_dma_write(device_t *device, UINT32 address, UINT64 data);

DECLARE_LEGACY_CPU_DEVICE(ADSP21062, adsp21062);

extern UINT32 sharc_dasm_one(char *buffer, offs_t pc, UINT64 opcode);

#endif /* __SHARC_H__ */
