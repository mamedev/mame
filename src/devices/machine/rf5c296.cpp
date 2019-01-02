// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "rf5c296.h"

// rf5c296 is very inaccurate at that point, it hardcodes the gnet config

DEFINE_DEVICE_TYPE(RF5C296, rf5c296_device, "rf5c296", "RF5C296 PC Card controller")

rf5c296_device::rf5c296_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RF5C296, tag, owner, clock)
	, m_rf5c296_reg(0)
	, m_pccard(*this, finder_base::DUMMY_TAG)
{
}

void rf5c296_device::device_start()
{
}

void rf5c296_device::reg_w(ATTR_UNUSED uint8_t reg, uint8_t data)
{
	//  fprintf(stderr, "%s rf5c296_reg_w %02x, %02x\n", machine().describe_context().c_str(), reg, data);
	switch (reg)
	{
		// Interrupt and General Control Register
		case 0x03:
			// Check for card reset
			if (!(data & 0x40))
			{
				m_pccard->reset();
			}
			break;

		default:
			break;
	}
}

uint8_t rf5c296_device::reg_r(ATTR_UNUSED uint8_t reg)
{
	//  fprintf(stderr, "%s rf5c296_reg_r %02x\n", machine().describe_context().c_str(), reg);
	return 0x00;
}

WRITE16_MEMBER(rf5c296_device::io_w)
{
	/// TODO: find out if this should be done here.
	offset *= 2;
	if (mem_mask == 0xff00)
	{
		mem_mask >>= 8;
		data >>= 8;
		offset++;
	}

	switch(offset)
	{
	case 0x3e0:
		m_rf5c296_reg = data;
		break;

	case 0x3e1:
		reg_w(m_rf5c296_reg, data);
		break;

	default:
		m_pccard->write_memory(space, offset, data, mem_mask);
		break;
	}
}

READ16_MEMBER(rf5c296_device::io_r)
{
	/// TODO: find out if this should be done here.
	offset *= 2;
	int shift = 0;
	if (mem_mask == 0xff00)
	{
		shift = 8;
		mem_mask >>= 8;
		offset++;
	}

	uint16_t data;

	switch( offset )
	{
	case 0x3e0:
		data = m_rf5c296_reg;
		break;

	case 0x3e1:
		data = reg_r(m_rf5c296_reg);
		break;

	default:
		data = m_pccard->read_memory(space, offset, mem_mask);
		break;
	}

	return data << shift;
}

// Hardcoded to reach the pcmcia CIS

READ16_MEMBER(rf5c296_device::mem_r)
{
	return m_pccard->read_reg(space, offset, mem_mask);
}

WRITE16_MEMBER(rf5c296_device::mem_w)
{
	m_pccard->write_reg(space, offset, data, mem_mask);
}
