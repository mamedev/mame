// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6702 Mystery Device emulation

**********************************************************************/

#include "emu.h"
#include "mos6702.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS6702, mos6702_device, "mos6702", "MOS 6702")

const uint8_t mos6702_device::LEFTMOST[8] = { 1 << 5, 1 << 2, 1 << 6, 1 << 7, 1 << 0, 1 << 2, 1 << 4, 1 << 1 };

static constexpr uint8_t MAGIC = 0xd6;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6702_device - constructor
//-------------------------------------------------

mos6702_device::mos6702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS6702, tag, owner, clock),
	m_val(MAGIC),
	m_prevodd(1),
	m_wantodd(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6702_device::device_start()
{
	// state saving
	save_item(NAME(m_shift));
	save_item(NAME(m_val));
	save_item(NAME(m_prevodd));
	save_item(NAME(m_wantodd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6702_device::device_reset()
{
	for (int i = 0; i < 8; i++)
	{
		m_shift[i] = BIT(MAGIC | 1, i) ? LEFTMOST[i] : 0;
	}

	m_val = MAGIC;
	m_prevodd = 1;
	m_wantodd = 0;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos6702_device::read(offs_t offset)
{
	return m_val;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos6702_device::write(offs_t offset, uint8_t data)
{
	LOG("%s write %02x\n", machine().describe_context(), data);

	if (BIT(data, 0) != m_wantodd)
		return;

	if (m_wantodd)
	{
		uint8_t changed = m_prevodd ^ data;

		for (int i = 0; i < 8; i++)
		{
			uint16_t shift = m_shift[i];

			if (BIT(changed, i))
				shift ^= LEFTMOST[i];

			if (BIT(shift, 0))
			{
				m_val ^= 1 << i;
				shift |= LEFTMOST[i] << 1;
			}

			m_shift[i] = shift >> 1;
		}

		m_prevodd = data;
	}

	m_wantodd ^= 1;
}
