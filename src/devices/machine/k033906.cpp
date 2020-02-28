// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami IC 033906 (PCI bridge)

***************************************************************************/

#include "emu.h"
#include "k033906.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(K033906, k033906_device, "k033906", "K033906 PCI bridge")

//-------------------------------------------------
//  k033906_device - constructor
//-------------------------------------------------

k033906_device::k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K033906, tag, owner, clock)
	, m_reg_set(0)
	, m_voodoo(*this, finder_base::DUMMY_TAG)
	, m_reg(nullptr)
	, m_ram(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k033906_device::device_start()
{
	m_reg_set = 0;
	m_reg = make_unique_clear<u32[]>(256);
	m_ram = make_unique_clear<u32[]>(32768);

	save_item(NAME(m_reg_set));
	save_pointer(NAME(m_reg), 256);
	save_pointer(NAME(m_ram), 32768);
}


WRITE_LINE_MEMBER(k033906_device::set_reg)
{
	m_reg_set = state & 1;
}

uint32_t k033906_device::reg_r(int reg)
{
	switch (reg)
	{
		case 0x00:      return 0x0001121a;          // PCI Vendor ID (0x121a = 3dfx), Device ID (0x0001 = Voodoo)
		case 0x02:      return 0x04000000;          // Revision ID
		case 0x04:      return m_reg[0x04];         // memBaseAddr
		case 0x0f:      return m_reg[0x0f];         // interrupt_line, interrupt_pin, min_gnt, max_lat

		default:
			fatalerror("%s: k033906_reg_r: %08X\n", machine().describe_context().c_str(), reg);
	}
	// never executed
	//return 0;
}

void k033906_device::reg_w(int reg, uint32_t data)
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
			m_voodoo->voodoo_set_init_enable(data);
			break;
		}

		case 0x11:      // busSnoop0
		case 0x12:      // busSnoop1
			break;

		case 0x38:      // ???
			break;

		default:
			fatalerror("%s:K033906_w: %08X, %08X\n", machine().describe_context().c_str(), data, reg);
	}
}

u32 k033906_device::read(offs_t offset)
{
	if (m_reg_set)
		return reg_r(offset);
	else
		return m_ram[offset];
}

void k033906_device::write(offs_t offset, u32 data)
{
	if (m_reg_set)
		reg_w(offset, data);
	else
		m_ram[offset] = data;
}
