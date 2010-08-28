/***************************************************************************

    Konami IC 033906 (PCI bridge)

***************************************************************************/

#include "emu.h"
#include "k033906.h"
#include "video/voodoo.h"
#include "devhelpr.h"


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(k033906, "Konami 033906")

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k033906_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const k033906_interface *intf = reinterpret_cast<const k033906_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<k033906_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		m_voodoo_tag = NULL;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type K033906 = k033906_device_config::static_alloc_device_config;

//-------------------------------------------------
//  k033906_device - constructor
//-------------------------------------------------

k033906_device::k033906_device(running_machine &_machine, const k033906_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k033906_device::device_start()
{
	m_voodoo = m_machine.device(m_config.m_voodoo_tag);

	m_reg = auto_alloc_array(&m_machine, UINT32, 256);
	m_ram = auto_alloc_array(&m_machine, UINT32, 32768);

	m_reg_set = 0;

	state_save_register_device_item_pointer(this, 0, m_reg, 256);
	state_save_register_device_item_pointer(this, 0, m_ram, 32768);
	state_save_register_device_item(this, 0, m_reg_set);
}


WRITE_LINE_DEVICE_HANDLER_TRAMPOLINE(k033906, k033906_set_reg)
{
	m_reg_set = state & 1;
}

UINT32 k033906_device::k033906_reg_r(int reg)
{
	switch (reg)
	{
		case 0x00:		return 0x0001121a;			// PCI Vendor ID (0x121a = 3dfx), Device ID (0x0001 = Voodoo)
		case 0x02:		return 0x04000000;			// Revision ID
		case 0x04:		return m_reg[0x04];			// memBaseAddr
		case 0x0f:		return m_reg[0x0f];			// interrupt_line, interrupt_pin, min_gnt, max_lat

		default:
			fatalerror("%s: k033906_reg_r: %08X", cpuexec_describe_context(&m_machine), reg);
	}
	return 0;
}

void k033906_device::k033906_reg_w(int reg, UINT32 data)
{
	switch (reg)
	{
		case 0x00:
			break;

		case 0x01:		// command register
			break;

		case 0x04:		// memBaseAddr
		{
			if (data == 0xffffffff)
			{
				m_reg[0x04] = 0xff000000;
			}
			else
			{
				m_reg[0x04] = data & 0xff000000;
			}
			break;
		}

		case 0x0f:		// interrupt_line, interrupt_pin, min_gnt, max_lat
		{
			m_reg[0x0f] = data;
			break;
		}

		case 0x10:		// initEnable
		{
			voodoo_set_init_enable(m_voodoo, data);
			break;
		}

		case 0x11:		// busSnoop0
		case 0x12:		// busSnoop1
			break;

		case 0x38:		// ???
			break;

		default:
			fatalerror("%s:K033906_w: %08X, %08X", cpuexec_describe_context(&m_machine), data, reg);
	}
}

READ32_DEVICE_HANDLER_TRAMPOLINE(k033906, k033906_r)
{
	if(m_reg_set)
	{
		return k033906_reg_r(offset);
	}
	else
	{
		return m_ram[offset];
	}
}

WRITE32_DEVICE_HANDLER_TRAMPOLINE(k033906, k033906_w)
{
	if(m_reg_set)
	{
		k033906_reg_w(offset, data);
	}
	else
	{
		m_ram[offset] = data;
	}
}
