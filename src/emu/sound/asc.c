/***************************************************************************

    asc.c

    Apple Sound Chip (ASC) 344S0063
    Enhanced Apple Sound Chip (EASC) 343S1063
    
    Emulation by R. Belmont

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "asc.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  static_set_type - configuration helper to set
//  the chip type
//-------------------------------------------------

void asc_device_config::static_set_type(device_config *device, int type)
{
	asc_device_config *asc = downcast<asc_device_config *>(device);
	asc->m_type = type;
}

//-------------------------------------------------
//  asc_device_config - constructor
//-------------------------------------------------

asc_device_config::asc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "ASC", tag, owner, clock),
	  device_config_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *asc_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(asc_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *asc_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, asc_device(machine, *this));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  asc_device - constructor
//-------------------------------------------------

asc_device::asc_device(running_machine &_machine, const asc_device_config &config)
	: device_t(_machine, config),
	  device_sound_interface(_machine, config, *this),
	  m_config(config),
	  m_chip_type(m_config.m_type)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void asc_device::device_start()
{
	// create the stream
	m_stream = stream_create(this, 0, 2, 22257, this, static_stream_generate);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void asc_device::device_reset()
{
	stream_update(m_stream);
}

//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

STREAM_UPDATE( asc_device::static_stream_generate )
{
	reinterpret_cast<asc_device *>(param)->stream_generate(inputs, outputs, samples);
}

void asc_device::stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

UINT8 asc_device::read(UINT16 offset)
{
	if (offset < 0x400)
	{
		return fifo_a[offset];
	}
	else if (offset < 0x800)
	{
		return fifo_b[offset-0x400];
	}
	else
	{
		switch (offset)
		{
			case 0x800:	// VERSION
				switch (m_chip_type)
				{
					case ASC_TYPE_ASC:
						return 0;

					case ASC_TYPE_V8:
						return 0xe8;

					case ASC_TYPE_SONORA:
						return 0xbc;

					default:
						return 0;
				}
				break;

			case 0x804:	// FIFO Interrupt Status
				if (m_chip_type == ASC_TYPE_V8)
				{
					return 3;
				}
				break;

			default:
				break;
		}
	}

	return regs[offset-0x800];
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

void asc_device::write(UINT16 offset, UINT8 data)
{
	if (offset < 0x400)
	{
		fifo_a[offset] = data;
	}
	else if (offset < 0x800)
	{
		fifo_b[offset-0x400] = data;
	}
	else
	{
		regs[offset-0x800] = data;
	}
}

const device_type ASC = asc_device_config::static_alloc_device_config;

