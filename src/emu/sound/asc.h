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

#define MDRV_ASC_ADD(_tag, _clock, _type, _irqf) \
	MDRV_DEVICE_ADD(_tag, ASC, _clock) \
	MDRV_ASC_TYPE(_type) \
	MDRV_IRQ_FUNC(_irqf)

#define MDRV_ASC_REPLACE(_tag, _clock, _type, _irqf) \
	MDRV_DEVICE_REPLACE(_tag, ASC, _clock) \
	MDRV_ASC_TYPE(_type) \
	MDRV_IRQ_FUNC(_irqf)

#define MDRV_ASC_TYPE(_type) \
	asc_device_config::static_set_type(device, _type); \

#define MDRV_IRQ_FUNC(_irqf) \
	asc_device_config::static_set_irqf(device, _irqf); \


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
	static void static_set_irqf(device_config *device, void (*irqf)(running_device *device, int state));

protected:
	// inline data
	UINT8			m_type;
	void (*m_irq_func)(running_device *device, int state);
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
	enum
	{
		R_VERSION = 0x800,
		R_MODE,
		R_CONTROL,
		R_FIFOMODE,
		R_FIFOSTAT,
		R_WTCONTROL,
		R_VOLUME,
		R_CLOCK,
		R_REG8,
		R_REG9,
		R_PLAYRECA,
		R_REGB,
		R_REGC,
		R_REGD,
		R_REGE,
		R_TEST
	};

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal callbacks
	static STREAM_UPDATE( static_stream_generate );
	virtual void stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	const asc_device_config &m_config;

	UINT8	m_chip_type;
	void (*m_irq_cb)(running_device *device, int state);
	sound_stream *m_stream;

	UINT8	m_fifo_a[0x400];
	UINT8	m_fifo_b[0x400];

	UINT8	m_regs[0x800];

	UINT32	m_phase[4], m_incr[4];

	int	m_fifo_a_rdptr, m_fifo_b_rdptr;
	int	m_fifo_a_wrptr, m_fifo_b_wrptr;
	int 	m_fifo_cap_a, m_fifo_cap_b;
};


// device type definition
extern const device_type ASC;


#endif /* __ASC_H__ */

