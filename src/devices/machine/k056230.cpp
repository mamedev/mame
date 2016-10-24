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
const device_type K056230 = &device_creator<k056230_device>;

//-------------------------------------------------
//  k056230_device - constructor
//-------------------------------------------------

k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K056230, "K056230 LANC", tag, owner, clock, "k056230", __FILE__)
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


uint8_t k056230_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	switch (offset)
	{
		case 0:     // Status register
		{
			return 0x08;
		}
	}

//  osd_printf_debug("k056230_r: %d at %08X\n", offset, space.device().safe_pc());

	return 0;
}

void k056230_device::network_irq_clear(void *ptr, int32_t param)
{
	if (m_cpu)
		m_cpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
}


void k056230_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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
//  osd_printf_debug("k056230_w: %d, %02X at %08X\n", offset, data, space.device().safe_pc());
}

uint32_t k056230_device::lanc_ram_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	//osd_printf_debug("LANC_RAM_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
	return m_ram[offset & 0x7ff];
}

void k056230_device::lanc_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//osd_printf_debug("LANC_RAM_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
	COMBINE_DATA(m_ram + (offset & 0x7ff));
}
