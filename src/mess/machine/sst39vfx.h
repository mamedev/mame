/*

    SST Multi-Purpose Flash (MPF)

    (c) 2001-2007 Tim Schuerewegen

    SST39VF020  - 256 KByte
    SST39VF400A - 512 Kbyte

*/

#ifndef _SST39VFX_H_
#define _SST39VFX_H_

#include "emu.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sst39vfx_config sst39vfx_config;
struct _sst39vfx_config
{
	int cpu_datawidth;
	int cpu_endianess;
};

/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(SST39VF020, sst39vf020);

#define MCFG_SST39VF020_ADD(_tag,_cpu_datawidth,_cpu_endianess) \
	MCFG_DEVICE_ADD(_tag, SST39VF020, 0) \
	MCFG_DEVICE_CONFIG_DATA32(sst39vfx_config, cpu_datawidth, _cpu_datawidth) \
	MCFG_DEVICE_CONFIG_DATA32(sst39vfx_config, cpu_endianess, _cpu_endianess)

#define MCFG_SST39VF020_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

DECLARE_LEGACY_DEVICE(SST39VF400A, sst39vf400a);

#define MCFG_SST39VF400A_ADD(_tag,_cpu_datawidth,_cpu_endianess) \
	MCFG_DEVICE_ADD(_tag, SST39VF400A, 0) \
	MCFG_DEVICE_CONFIG_DATA32(sst39vfx_config, cpu_datawidth, _cpu_datawidth) \
	MCFG_DEVICE_CONFIG_DATA32(sst39vfx_config, cpu_endianess, _cpu_endianess)

#define MCFG_SST39VF400A_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
// get base/size
UINT8* sst39vfx_get_base( device_t *device );
UINT32 sst39vfx_get_size( device_t *device );

// read/write handler
#if 0
READ8_HANDLER( sst39vfx_r );
WRITE8_HANDLER( sst39vfx_w );
#endif

// load/save
void sst39vfx_load(device_t *device, emu_file *file);
void sst39vfx_save(device_t *device, emu_file *file);

// non-volatile ram handler
//NVRAM_HANDLER( sst39vfx );

#endif
