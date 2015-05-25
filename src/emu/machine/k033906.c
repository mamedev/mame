// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami IC 033906 (PCI bridge)

***************************************************************************/

#include "emu.h"
#include "k033906.h"
#include "video/voodoo.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type K033906 = &device_creator<k033906_device>;

//-------------------------------------------------
//  k033906_device - constructor
//-------------------------------------------------

k033906_device::k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K033906, "K033906 PCI bridge", tag, owner, clock, "k033906", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k033906_device::device_start()
{
	m_voodoo = machine().device(m_voodoo_tag);

	m_reg_set = 0;

	save_item(NAME(m_reg));
	save_item(NAME(m_ram));
	save_item(NAME(m_reg_set));
}


WRITE_LINE_MEMBER(k033906_device::set_reg)
{
	m_reg_set = state & 1;
}

UINT32 k033906_device::reg_r(int reg)
{
	switch (reg)
	{
		case 0x00:      return 0x0001121a;          // PCI Vendor ID (0x121a = 3dfx), Device ID (0x0001 = Voodoo)
		case 0x02:      return 0x04000000;          // Revision ID
		case 0x04:      return m_reg[0x04];         // memBaseAddr
		case 0x0f:      return m_reg[0x0f];         // interrupt_line, interrupt_pin, min_gnt, max_lat

		default:
			fatalerror("%s: k033906_reg_r: %08X\n", machine().describe_context(), reg);
	}
	// never executed
	//return 0;
}

void k033906_device::reg_w(int reg, UINT32 data)
{
	switch (reg)
	{
		case 0x00:
			break;

		case 0x01:      // command register
			break;

		case 0x04:      // memBaseAddr
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

		case 0x0f:      // interrupt_line, interrupt_pin, min_gnt, max_lat
		{
			m_reg[0x0f] = data;
			break;
		}

		case 0x10:      // initEnable
		{
			voodoo_set_init_enable(m_voodoo, data);
			break;
		}

		case 0x11:      // busSnoop0
		case 0x12:      // busSnoop1
			break;

		case 0x38:      // ???
			break;

		default:
			fatalerror("%s:K033906_w: %08X, %08X\n", machine().describe_context(), data, reg);
	}
}

READ32_MEMBER(k033906_device::read)
{
	if (m_reg_set)
		return reg_r(offset);
	else
		return m_ram[offset];
}

WRITE32_MEMBER(k033906_device::write)
{
	if (m_reg_set)
		reg_w(offset, data);
	else
		m_ram[offset] = data;
}
