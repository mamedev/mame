#include "rf5c296.h"

// rf5c296 is very inaccurate at that point, it hardcodes the gnet config

const device_type RF5C296 = &device_creator<rf5c296_device>;

rf5c296_device::rf5c296_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PCCARD_SLOT, "PCCARD SLOT", tag, owner, clock, "pccard", __FILE__)
{
}

void rf5c296_device::device_start()
{
	m_pccard = machine().device<pccard_slot_device>(m_pccard_name);
}

void rf5c296_device::reg_w(ATTR_UNUSED UINT8 reg, UINT8 data)
{
	//  fprintf(stderr, "rf5c296_reg_w %02x, %02x (%s)\n", reg, data, machine().describe_context());
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

UINT8 rf5c296_device::reg_r(ATTR_UNUSED UINT8 reg)
{
	//  fprintf(stderr, "rf5c296_reg_r %02x (%s)\n", reg, machine().describe_context());
	return 0x00;
}

WRITE16_MEMBER(rf5c296_device::io_w)
{
	if(offset < 4)
	{
		m_pccard->write_memory(space, offset, data, mem_mask);
	}

	if(offset == 0x3e0/2)
	{
		if(ACCESSING_BITS_0_7)
			m_rf5c296_reg = data;
		if(ACCESSING_BITS_8_15)
			reg_w(m_rf5c296_reg, data >> 8);
	}
}

READ16_MEMBER(rf5c296_device::io_r)
{
	if(offset < 4)
	{
		return m_pccard->read_memory(space, offset, mem_mask);
	}

	offset *= 2;

	if(offset == 0x3e0/2)
	{
		UINT32 res = 0x0000;
		if(ACCESSING_BITS_0_7)
			res |= m_rf5c296_reg;
		if(ACCESSING_BITS_8_15)
			res |= reg_r(m_rf5c296_reg) << 8;
		return res;
	}

	return 0xffff;
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
