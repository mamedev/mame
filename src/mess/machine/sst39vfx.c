/*

    SST Multi-Purpose Flash (MPF)

    (c) 2001-2007 Tim Schuerewegen

    SST39VF020  - 256 KByte
    SST39VF400A - 512 Kbyte

*/

#include "sst39vfx.h"

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct sst39vfx_t
{
	UINT8 *data;
	UINT32 size;
	UINT8 swap;
};

enum
{
	TYPE_SST39VF020,
	TYPE_SST39VF400A
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE sst39vfx_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SST39VF020 || device->type() == SST39VF400A);

	return (sst39vfx_t *) downcast<sst39vf020_device *>(device)->token();
}

INLINE const sst39vfx_config *get_config(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SST39VF020 || device->type() == SST39VF400A);

	return (const sst39vfx_config *)device->static_config();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void common_start(device_t *device, int device_type)
{
	sst39vfx_t *flash = get_token(device);
	const sst39vfx_config *config = get_config(device);

	_logerror( 0, ("sst39vfx_init (%d)\n", device_type));
	memset(flash, 0, sizeof(sst39vfx_t));
	switch (device_type)
	{
		case TYPE_SST39VF020  : flash->size = 256 * 1024; break;
		case TYPE_SST39VF400A : flash->size = 512 * 1024; break;
	}
	flash->data = auto_alloc_array(device->machine(), UINT8, flash->size);
#ifdef LSB_FIRST
	if (config->cpu_endianess != ENDIANNESS_LITTLE) flash->swap = config->cpu_datawidth / 8; else flash->swap = 0;
#else
	if (config->cpu_endianess != ENDIANNESS_BIG) flash->swap = config->cpu_datawidth / 8; else flash->swap = 0;
#endif

	state_save_register_item_pointer(device->machine(), "sst39vfx", device->tag(), 0, flash->data, flash->size);
	state_save_register_item(device->machine(), "sst39vfx", device->tag(), 0, flash->swap);
}


static DEVICE_START( sst39vf020 )
{
	common_start(device, TYPE_SST39VF020);
}

static DEVICE_START( sst39vf400a )
{
	common_start(device, TYPE_SST39VF400A);
}

UINT8* sst39vfx_get_base( device_t *device)
{
	sst39vfx_t *flash = get_token(device);
	return flash->data;
}

UINT32 sst39vfx_get_size( device_t *device)
{
	sst39vfx_t *flash = get_token(device);
	return flash->size;
}

/*
#define OFFSET_SWAP(offset,width) (offset & (~(width - 1))) | (width - 1 - (offset & (width - 1)))
*/

#ifdef UNUSED_FUNCTION
READ8_HANDLER( sst39vfx_r )
{
	_logerror( 1, ("sst39vfx_r (%08X)\n", offset));
	if (flash->swap) offset = OFFSET_SWAP( offset, flash->swap);
	return flash->data[offset];
}

WRITE8_HANDLER( sst39vfx_w )
{
	_logerror( 1, ("sst39vfx_w (%08X/%02X)\n", offset, data));
	if (flash->swap) offset = OFFSET_SWAP( offset, flash->swap);
	flash->data[offset] = data;
}
#endif

static void sst39vfx_swap( device_t *device)
{
	int i, j;
	UINT8 *base, temp[8];
	sst39vfx_t *flash = get_token(device);

	base = flash->data;
	for (i=0;i<flash->size;i+=flash->swap)
	{
		memcpy( temp, base, flash->swap);
		for (j=flash->swap-1;j>=0;j--) *base++ = temp[j];
	}
}

void sst39vfx_load(device_t *device, emu_file *file)
{
	sst39vfx_t *flash = get_token(device);

	_logerror( 0, ("sst39vfx_load (%p)\n", file));
	file->read(flash->data, flash->size);
	if (flash->swap) sst39vfx_swap(device);
}

void sst39vfx_save(device_t *device, emu_file *file)
{
	sst39vfx_t *flash = get_token(device);

	_logerror( 0, ("sst39vfx_save (%p)\n", file));
	if (flash->swap) sst39vfx_swap(device);
	file->write(flash->data, flash->size);
	if (flash->swap) sst39vfx_swap(device);
}

#if 0
NVRAM_HANDLER( sst39vfx )
{
	_logerror( 0, ("nvram_handler_sst39vfx (%p/%d)\n", file, read_or_write));
	if (read_or_write)
	{
		sst39vfx_save( file);
	}
	else
	{
		if (file)
		{
			sst39vfx_load( file);
		}
		else
		{
			memset( flash->data, 0xFF, flash->size);
		}
	}
}
#endif

const device_type SST39VF020 = &device_creator<sst39vf020_device>;

sst39vf020_device::sst39vf020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SST39VF020, "SST39VF020", tag, owner, clock)
{
	m_token = global_alloc_clear(sst39vfx_t);
}
sst39vf020_device::sst39vf020_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_clear(sst39vfx_t);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void sst39vf020_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sst39vf020_device::device_start()
{
	DEVICE_START_NAME( sst39vf020 )(this);
}


const device_type SST39VF400A = &device_creator<sst39vf400a_device>;

sst39vf400a_device::sst39vf400a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sst39vf020_device(mconfig, SST39VF400A, "SST39VF400A", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sst39vf400a_device::device_start()
{
	DEVICE_START_NAME( sst39vf400a )(this);
}
