/***************************************************************************

    Konami IC 056230 (LANC)

***************************************************************************/

#include "emu.h"
#include "k056230.h"
#include "devhelpr.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type K056230 = &device_creator<k056230_device>;

//-------------------------------------------------
//  k056230_device - constructor
//-------------------------------------------------

k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K056230, "Konami 056230", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k056230_device::device_config_complete()
{
	// inherit a copy of the static data
	const k056230_interface *intf = reinterpret_cast<const k056230_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<k056230_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		m_cpu_tag = NULL;
		m_is_thunderh = false;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056230_device::device_start()
{
	if(m_cpu_tag)
	{
		m_cpu = machine().device(m_cpu_tag);
	}
	else
	{
		m_cpu = NULL;
	}

	m_ram = auto_alloc_array(machine(), UINT32, 0x2000);

	save_pointer(NAME(m_ram), 0x2000);
}


READ8_DEVICE_HANDLER_TRAMPOLINE(k056230, k056230_r)
{
	switch (offset)
	{
		case 0:     // Status register
		{
			return 0x08;
		}
	}

//  mame_printf_debug("k056230_r: %d at %08X\n", offset, space.device().safe_pc());

	return 0;
}

TIMER_CALLBACK( k056230_device::network_irq_clear_callback )
{
	reinterpret_cast<k056230_device*>(ptr)->network_irq_clear();
}

void k056230_device::network_irq_clear()
{
	if(m_cpu)
	{
		m_cpu->execute().set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
	}
}


WRITE8_DEVICE_HANDLER_TRAMPOLINE(k056230, k056230_w)
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
				if(!m_is_thunderh)
				{
					if(m_cpu)
					{
						m_cpu->execute().set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
					}
					machine().scheduler().timer_set(attotime::from_usec(10), FUNC(network_irq_clear_callback), 0, (void*)this);
				}
			}
//          else
//              k056230->cpu->execute().set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
			break;
		}
		case 2:     // Sub ID register
		{
			break;
		}
	}
//  mame_printf_debug("k056230_w: %d, %02X at %08X\n", offset, data, space.device().safe_pc());
}

READ32_DEVICE_HANDLER_TRAMPOLINE(k056230, lanc_ram_r)
{
	//mame_printf_debug("LANC_RAM_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
	return m_ram[offset & 0x7ff];
}

WRITE32_DEVICE_HANDLER_TRAMPOLINE(k056230, lanc_ram_w)
{
	//mame_printf_debug("LANC_RAM_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
	COMBINE_DATA(m_ram + (offset & 0x7ff));
}
