/***************************************************************************

    asc.h

    Apple Sound Chip (ASC) 344S0063
    Enhanced Apple Sound Chip (EASC) 343S1063

***************************************************************************/

#pragma once

#ifndef __ASC_H__
#define __ASC_H__

#include "streams.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// chip behavior types
enum
{
	ASC_TYPE_ASC = 0,	// original discrete Apple Sound Chip
	ASC_TYPE_EASC = 1,	// discrete Enhanced Apple Sound Chip
	ASC_TYPE_V8 = 2,	// Subset of ASC included in the V8 ASIC (LC/LCII)
	ASC_TYPE_EAGLE = 3,	// Subset of ASC included in the Eagle ASIC (Classic II)
	ASC_TYPE_SPICE = 4,	// Subset of ASC included in the Spice ASIC (Color Classic)
	ASC_TYPE_SONORA = 5,	// Subset of ASC included in the Sonora ASIC (LCIII)
	ASC_TYPE_VASP = 6,	// Subset of ASC included in the VASP ASIC  (IIvx/IIvi)
	ASC_TYPE_ARDBEG = 7	// Subset of ASC included in the Ardbeg ASIC (LC520)
};



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_ASC_ADD(_tag, _clock, _type) \
	MDRV_DEVICE_ADD(_tag, ASC, _clock) \
	MDRV_ASC_TYPE(_type)

#define MDRV_ASC_REPLACE(_tag, _clock, _type) \
	MDRV_DEVICE_REPLACE(_tag, ASC, _clock) \
	MDRV_ASC_TYPE(_type)

#define MDRV_ASC_TYPE(_type) \
	asc_device_config::static_set_type(device, _type); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> asc_device_config

class asc_device_config :	public device_config, public device_config_sound_interface
{
	friend class asc_device;

	// construction/destruction
	asc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_type(device_config *device, int type);

protected:
	// device_config overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// internal state
	const address_space_config  m_space_config;

	// inline data
	UINT8						m_type;
};



// ======================> asc_device

class asc_device : public device_t, public device_sound_interface
{
	friend class asc_device_config;

	// construction/destruction
	asc_device(running_machine &_machine, const asc_device_config &config);

public:
	UINT8 read(UINT16 offset);
	void write(UINT16 offset, UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal callbacks
	static STREAM_UPDATE( static_stream_generate );
	virtual void stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	const asc_device_config &m_config;

	UINT8	m_chip_type;
	sound_stream *m_stream;

	UINT8	fifo_a[0x400];
	UINT8	fifo_b[0x400];

	UINT8	regs[0x100];
};


// device type definition
extern const device_type ASC;


#endif /* __ASC_H__ */

