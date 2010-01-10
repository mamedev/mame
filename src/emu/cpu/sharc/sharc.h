#pragma once

#ifndef __SHARC_H__
#define __SHARC_H__


#define SHARC_INPUT_FLAG0		3
#define SHARC_INPUT_FLAG1		4
#define SHARC_INPUT_FLAG2		5
#define SHARC_INPUT_FLAG3		6

typedef enum
{
	BOOT_MODE_EPROM,
	BOOT_MODE_HOST,
	BOOT_MODE_LINK,
	BOOT_MODE_NOBOOT
} SHARC_BOOT_MODE;

typedef struct {
	SHARC_BOOT_MODE boot_mode;
} sharc_config;

extern void sharc_set_flag_input(const device_config *device, int flag_num, int state);

extern void sharc_external_iop_write(const device_config *device, UINT32 address, UINT32 data);
extern void sharc_external_dma_write(const device_config *device, UINT32 address, UINT64 data);

CPU_GET_INFO( adsp21062 );
#define CPU_ADSP21062 CPU_GET_INFO_NAME( adsp21062 )

extern UINT32 sharc_dasm_one(char *buffer, offs_t pc, UINT64 opcode);

#endif /* __SHARC_H__ */
