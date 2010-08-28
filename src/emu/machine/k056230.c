/***************************************************************************

    Konami IC 056230 (LANC)

***************************************************************************/

#include "emu.h"
#include "k056230.h"
#include "devhelpr.h"


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(k056230, "Konami 056230")

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k056230_device_config::device_config_complete()
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
		m_cpu = NULL;
		m_is_thunderh = 0;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type K0506230 = k056230_device_config::static_alloc_device_config;

//-------------------------------------------------
//  k056230_device - constructor
//-------------------------------------------------

k056230_device::k056230_device(running_machine &_machine, const k056230_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056230_device::device_start()
{
	if(m_config.m_cpu)
	{
		m_cpu = m_machine.device(m_config.m_cpu);
	}
	else
	{
		m_cpu = NULL;
	}

	m_is_thunderh = m_config.m_is_thunderh;

	m_ram = auto_alloc_array(&m_machine, UINT32, 0x2000);

	state_save_register_device_item_pointer(this, 0, m_ram, 0x2000);
}


READ8_DEVICE_HANDLER_TRAMPOLINE(k056230, k056230_r)
{
	switch (offset)
	{
		case 0:		// Status register
		{
			return 0x08;
		}
	}

//  mame_printf_debug("k056230_r: %d at %08X\n", offset, cpu_get_pc(space->cpu));

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
		cpu_set_input_line(m_cpu, INPUT_LINE_IRQ2, CLEAR_LINE);
	}
}


WRITE8_DEVICE_HANDLER_TRAMPOLINE(k056230, k056230_w)
{
	switch(offset)
	{
		case 0:		// Mode register
		{
			break;
		}
		case 1:		// Control register
		{
			if(data & 0x20)
			{
				// Thunder Hurricane breaks otherwise...
				if(!m_is_thunderh)
				{
					if(m_cpu)
					{
						cpu_set_input_line(m_cpu, INPUT_LINE_IRQ2, ASSERT_LINE);
					}
					timer_set(&m_machine, ATTOTIME_IN_USEC(10), (void*)this, 0, network_irq_clear_callback);
				}
			}
//          else
//              cpu_set_input_line(k056230->cpu, INPUT_LINE_IRQ2, CLEAR_LINE);
			break;
		}
		case 2:		// Sub ID register
		{
			break;
		}
	}
//  mame_printf_debug("k056230_w: %d, %02X at %08X\n", offset, data, cpu_get_pc(space->cpu));
}

READ32_DEVICE_HANDLER_TRAMPOLINE(k056230, lanc_ram_r)
{
	//mame_printf_debug("LANC_RAM_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
	return m_ram[offset & 0x7ff];
}

WRITE32_DEVICE_HANDLER_TRAMPOLINE(k056230, lanc_ram_w)
{
	//mame_printf_debug("LANC_RAM_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
	COMBINE_DATA(m_ram + (offset & 0x7ff));
}
