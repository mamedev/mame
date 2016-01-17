// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    swim.c

    Implementation of the Apple SWIM FDC controller; used on (less)
    early Macs

*********************************************************************/

#include "machine/swim.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LOG_SWIM    0

enum
{
	SWIM_MODE_IWM,
	SWIM_MODE_SWIM,
	SWIM_MODE_SWIM2,
	SWIM_MODE_SWIM3
};


/***************************************************************************
    DEVICE
***************************************************************************/

const device_type SWIM = &device_creator<swim_device>;

//-------------------------------------------------
//  ctor
//-------------------------------------------------

swim_device::swim_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: applefdc_base_device(APPLEFDC_SWIM, mconfig, SWIM, "Apple SWIM (Steve Woz Integrated Machine)", tag, owner, clock, "swim", __FILE__)
{
}



//-------------------------------------------------
//  device_start - device-specific start
//-------------------------------------------------

void swim_device::device_start()
{
	// call inherited version
	applefdc_base_device::device_start();

	m_swim_mode         = SWIM_MODE_IWM;
	m_swim_magic_state  = 0x00;
	m_parm_offset       = 0x00;
	memset(m_ism_regs, 0, sizeof(m_ism_regs));
	memset(m_parms, 0, sizeof(m_parms));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void swim_device::device_reset()
{
	// call inherited version
	applefdc_base_device::device_reset();

	static UINT8 swim_default_parms[16] =
	{
		0x38, 0x18, 0x41, 0x2e, 0x2e, 0x18, 0x18, 0x1b,
		0x1b, 0x2f, 0x2f, 0x19, 0x19, 0x97, 0x1b, 0x57
	};

	for (int i = 0; i < 16; i++)
	{
		m_parms[i] = swim_default_parms[i];
	}

	m_swim_magic_state = 0;
	m_swim_mode = SWIM_MODE_IWM;
	m_parm_offset = 0;
}



//-------------------------------------------------
//  read - reads a byte from the FDC
//-------------------------------------------------

UINT8 swim_device::read(UINT8 offset)
{
	UINT8 result = 0;

	if (m_swim_mode == SWIM_MODE_IWM)
	{
		// IWM mode
		result = applefdc_base_device::read(offset);
	}
	else if (m_swim_mode >= SWIM_MODE_SWIM)
	{
		// reading parameter RAM?
		if ((offset & 7) == 3)
		{
			result = m_parms[m_parm_offset++];
			m_parm_offset &= 0xf;
		}
		else
		{
			result = m_ism_regs[offset&7];
		}

		if (LOG_SWIM)
			logerror("SWIM: read %02x from offset %x\n", result, offset & 7);
	}
	return result;
}



//-------------------------------------------------
//  write - write a byte to the FDC
//-------------------------------------------------

void swim_device::write(UINT8 offset, UINT8 data)
{
	if (m_swim_mode == SWIM_MODE_IWM)
	{
		// IWM mode
		applefdc_base_device::write(offset, data);
	}
	else if (m_swim_mode >= SWIM_MODE_SWIM)
	{
		if (LOG_SWIM)
			logerror("SWIM: write %02x to offset %x\n", data, offset & 7);

		switch (offset & 7)
		{
			case 2: // write CRC
				break;

			case 3: // write parameter
				m_parms[m_parm_offset++] = data;
				m_parm_offset &= 0xf;
				break;

			case 6: // write zeros to status (also zeroes parameter RAM pointer)
				m_ism_regs[6] &= ~data;
				m_parm_offset = 0;

				if (data == 0xf8)   // magic "revert to IWM" value
				{
					if (LOG_SWIM)
						logerror("SWIM: reverting to IWM\n");
					m_swim_mode = SWIM_MODE_IWM;
				}
				break;

			case 7: // write ones to status
				m_ism_regs[6] |= data;
				break;

			default:
				m_ism_regs[offset & 7] = data;
				break;

		}
	}
}



//-------------------------------------------------
//  iwm_modereg_w - changes the mode register
//-------------------------------------------------

void swim_device::iwm_modereg_w(UINT8 data)
{
	// SWIM mode is unlocked by writing 1/0/1/1 in a row to bit 6 (which is unused on IWM)
	// when SWIM mode engages, the IWM is disconnected from both the 68k and the drives,
	// and the ISM is substituted.

	switch (m_swim_magic_state)
	{
		case 0:
		case 2:
		case 3:
			if (data & 0x40)
			{
				m_swim_magic_state++;
			}
			else
			{
				m_swim_magic_state = 0;
			}
			break;
		case 1:
			if (!(data & 0x40))
			{
				m_swim_magic_state++;
			}
			else
			{
				m_swim_magic_state = 0;
			}
			break;
	}

	if (m_swim_magic_state == 4)
	{
		m_swim_magic_state = 0;
		m_swim_mode = SWIM_MODE_SWIM;
	}

	// call inherited version
	applefdc_base_device::iwm_modereg_w(data);
}
