// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami IC 056230 (LANC)

***************************************************************************/

#include "emu.h"
#include "k056230.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(K056230, k056230_device, "k056230", "K056230 LANC")

//-------------------------------------------------
//  k056230_device - constructor
//-------------------------------------------------

k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K056230, tag, owner, clock)
	, m_is_thunderh(0)
	, m_cpu(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056230_device::device_start()
{
	save_item(NAME(m_ram));
}


READ8_MEMBER(k056230_device::read)
{
	switch (offset)
	{
		case 0:     // Status register
		{
			return 0x08;
		}
	}

//  logerror("k056230_r: %d %s\n", offset, machine().describe_context());

	return 0;
}

TIMER_CALLBACK_MEMBER(k056230_device::network_irq_clear)
{
	if (m_cpu)
		m_cpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
}


WRITE8_MEMBER(k056230_device::write)
{
	switch(offset)
	{
		case 0:     // Mode register
		{
			break;
		}
		case 1:     // Control register
		{
			if(data & 0x20)
			{
				// Thunder Hurricane breaks otherwise...
				if (!m_is_thunderh)
				{
					if (m_cpu)
						m_cpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);

					machine().scheduler().timer_set(attotime::from_usec(10), timer_expired_delegate(FUNC(k056230_device::network_irq_clear), this));
				}
			}
//          else
//              m_cpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
			break;
		}
		case 2:     // Sub ID register
		{
			break;
		}
	}
//  logerror("k056230_w: %d, %02X at %08X\n", offset, data, machine().describe_context());
}

READ32_MEMBER(k056230_device::lanc_ram_r)
{
	//logerror("LANC_RAM_r: %08X, %08X %s\n", offset, mem_mask, machine().describe_context());
	return m_ram[offset & 0x7ff];
}

WRITE32_MEMBER(k056230_device::lanc_ram_w)
{
	//logerror("LANC_RAM_w: %08X, %08X, %08X %s\n", data, offset, mem_mask, machine().describe_context());
	COMBINE_DATA(m_ram + (offset & 0x7ff));
}
