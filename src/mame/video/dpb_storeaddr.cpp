// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_storeaddr.cpp
    DPB-7000/1 - Store Address Card

***************************************************************************/

#include "emu.h"
#include "dpb_storeaddr.h"

#define VERBOSE (1)
#include "logmacro.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(DPB7000_STOREADDR, dpb7000_storeaddr_card_device, "dpb_storeaddr", "Quantel DPB-7000 Store Address Card")


//-------------------------------------------------
//  dpb7000_storeaddr_card_device - constructor
//-------------------------------------------------

dpb7000_storeaddr_card_device::dpb7000_storeaddr_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DPB7000_STOREADDR, tag, owner, clock)
	, m_bb_base(nullptr)
	, m_bc_base(nullptr)
	, m_bd_base(nullptr)
	, m_protx_base(nullptr)
	, m_proty_base(nullptr)
	, m_bb_out(0)
	, m_bc_out(0)
	, m_bd_out(0)
	, m_protx(false)
	, m_proty(false)
	, m_rhscr(0)
	, m_rvscr(0)
	, m_rzoom(0)
	, m_fld_sel(0)
	, m_orig_cx_stripe_addr(0)
	, m_orig_cx_stripe_num(0)
	, m_orig_cy_addr(0)
	, m_cx_stripe_addr(0)
	, m_cx_stripe_num(0)
	, m_cy_addr(0)
	, m_s_type(0)
	, m_cen(false)
	, m_cxd(false)
	, m_cxen(false)
	, m_cxld(false)
	, m_cxck(false)
	, m_cxod(false)
	, m_cxoen(false)
	, m_cyd(false)
	, m_cyen(false)
	, m_cyld(false)
	, m_cyck(false)
	, m_cyod(false)
	, m_cyoen(false)
	, m_clrc(false)
	, m_selvideo(false)
	, m_creq(false)
	, m_cread(false)
	, m_prot_a(false)
	, m_prot_b(false)
	, m_x_prom(*this, "x_prom")
	, m_protx_prom(*this, "protx_prom")
	, m_proty_prom(*this, "proty_prom")
	, m_blanking_pal(*this, "blanking_pal")
{
}

ROM_START( dpb7000_storeaddr )
	ROM_REGION(0xc00, "x_prom", 0)
	ROM_LOAD("pb-032-17425b-bbb.bin", 0x000, 0x400, CRC(2051a6e4) SHA1(3bd8a9015e77b034a94fe072a9753649b76f9f69))
	ROM_LOAD("pb-032-17425b-bcb.bin", 0x400, 0x400, CRC(01aaa6f7) SHA1(e31bff0c68f74996368443bfb58a3524a838f270))
	ROM_LOAD("pb-032-17425b-bdb.bin", 0x800, 0x400, CRC(20e2fb9e) SHA1(c4c77ec02ab6d3a1a28edf5543e57235a64a9d8d))

	ROM_REGION(0xc00, "protx_prom", 0)
	ROM_LOAD("pb-032-17425b-deb.bin", 0x000, 0x400, CRC(faeb44dd) SHA1(3eaf981245824332d216e97095bdc02ff04e4800))

	ROM_REGION(0xc00, "proty_prom", 0)
	ROM_LOAD("pb-032-17425b-edb.bin", 0x000, 0x400, CRC(183bfdc0) SHA1(175b052948e4e4a9421d8913479e7531b7e5f03c))

	ROM_REGION(0x10000, "blanking_pal", 0)
	ROM_LOAD("pb-032-17425b-igb.bin", 0x00000, 0x10000, CRC(cdd80590) SHA1(fecb64695b61e8ec740af1480240088d5447688d))
ROM_END

const tiny_rom_entry *dpb7000_storeaddr_card_device::device_rom_region() const
{
	return ROM_NAME( dpb7000_storeaddr );
}

void dpb7000_storeaddr_card_device::device_start()
{
	save_item(NAME(m_bb_out));
	save_item(NAME(m_bc_out));
	save_item(NAME(m_bd_out));
	save_item(NAME(m_protx));
	save_item(NAME(m_proty));

	save_item(NAME(m_rhscr));
	save_item(NAME(m_rvscr));
	save_item(NAME(m_rzoom));
	save_item(NAME(m_fld_sel));
	save_item(NAME(m_orig_cx_stripe_addr));
	save_item(NAME(m_orig_cx_stripe_num));
	save_item(NAME(m_orig_cy_addr));
	save_item(NAME(m_cx_stripe_addr));
	save_item(NAME(m_cx_stripe_num));
	save_item(NAME(m_cy_addr));

	save_item(NAME(m_s_type));

	save_item(NAME(m_cen));

	save_item(NAME(m_cxd));
	save_item(NAME(m_cxen));
	save_item(NAME(m_cxld));
	save_item(NAME(m_cxck));
	save_item(NAME(m_cxod));
	save_item(NAME(m_cxoen));

	save_item(NAME(m_cyd));
	save_item(NAME(m_cyen));
	save_item(NAME(m_cyld));
	save_item(NAME(m_cyck));
	save_item(NAME(m_cyod));
	save_item(NAME(m_cyoen));

	save_item(NAME(m_clrc));
	save_item(NAME(m_selvideo));
	save_item(NAME(m_creq));
	save_item(NAME(m_cread));
}

void dpb7000_storeaddr_card_device::device_reset()
{
	m_rhscr = 0;
	m_rvscr = 0;
	m_rzoom = 0;
	m_fld_sel = 0;
	m_orig_cx_stripe_addr = 0;
	m_orig_cx_stripe_num = 0;
	m_orig_cy_addr = 0;
	m_cx_stripe_addr = 0;
	m_cx_stripe_num = 0;
	m_cy_addr = 0;

	m_s_type = 0;

	m_cen = false;

	m_cxd = false;
	m_cxen = false;
	m_cxld = false;
	m_cxck = false;
	m_cxod = false;
	m_cxoen = false;

	m_cyd = false;
	m_cyen = false;
	m_cyld = false;
	m_cyck = false;
	m_cyod = false;
	m_cyoen = false;

	m_clrc = false;
	m_selvideo = false;
	m_creq = false;
	m_cread = false;

	m_bb_base = m_x_prom->base() + 0x000;
	m_bc_base = m_x_prom->base() + 0x400;
	m_bd_base = m_x_prom->base() + 0x800;
	m_protx_base = m_protx_prom->base();
	m_proty_base = m_proty_prom->base();

	m_bb_out = 0;
	m_bc_out = 0;
	m_bd_out = 0;
	m_protx = false;
	m_proty = false;
}

void dpb7000_storeaddr_card_device::reg_w(uint16_t data)
{
	switch ((data >> 12) & 7)
	{
		case 0:
			LOG("%s: Store Address Card %d, set RHSCR: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_rhscr = data & 0xfff;
			break;
		case 1:
			LOG("%s: Store Address Card %d, set RVSCR: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_rvscr = data & 0xfff;
			break;
		case 2:
			LOG("%s: Store Address Card %d, set R ZOOM: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_rzoom = data & 0xfff;
			break;
		case 3:
			LOG("%s: Store Address Card %d, set FLDSEL: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_fld_sel = data & 0xfff;
			break;
		case 4:
			LOG("%s: Store Address Card %d, set CXPOS: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			set_cxpos(data & 0xfff);
			break;
		case 5:
			LOG("%s: Store Address Card %d, set CYPOS: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_orig_cy_addr = data & 0xfff;
			break;
		default:
			LOG("%s: Store Address Card %d, unknown register: %04x\n", machine().describe_context(), m_s_type, data);
			break;
	}
}

void dpb7000_storeaddr_card_device::set_cxpos(uint16_t data)
{
	const uint16_t prom_addr = (data >> 1) & 0x3ff;
	m_bb_out = m_bb_base[prom_addr];
	m_bc_out = m_bc_base[prom_addr];
	m_bd_out = m_bd_base[prom_addr];
	m_orig_cx_stripe_addr = BIT(data, 0) | (m_bb_out & 0xe);
	m_orig_cx_stripe_num = m_bc_out | (m_bd_out << 4);
}

void dpb7000_storeaddr_card_device::s_type_w(int state)
{
	m_s_type = state ? 2 : 1;
}

void dpb7000_storeaddr_card_device::cen_w(int state)
{
	m_cen = (bool)state;
}

void dpb7000_storeaddr_card_device::cxd_w(int state)
{
	m_cxd = (bool)state;
}

void dpb7000_storeaddr_card_device::cxen_w(int state)
{
	m_cxen = (bool)state;
}

void dpb7000_storeaddr_card_device::cxld_w(int state)
{
	m_cxld = (bool)state;
}

void dpb7000_storeaddr_card_device::cxck_w(int state)
{
	bool old = m_cxck;
	m_cxck = (bool)state;
	if (old && !m_cxck) // Inverted by 74LS240 AE
		tick_cxck();
}

void dpb7000_storeaddr_card_device::tick_cxck()
{
	if (m_cxld)
	{
		m_cx_stripe_addr = m_orig_cx_stripe_addr;
		m_cx_stripe_num = m_orig_cx_stripe_num;
	}
	else if (!m_cen && !m_cxen)
	{
		if (m_cxd)
		{
			m_cx_stripe_addr--;
			if (m_cx_stripe_addr == -1)
			{
				m_cx_stripe_addr = 9;
				m_cx_stripe_num--;
				update_prot_proms();
			}
		}
		else
		{
			m_cx_stripe_addr++;
			if (m_cx_stripe_addr == 10)
			{
				m_cx_stripe_addr = 0;
				m_cx_stripe_num++;
				update_prot_proms();
			}
		}
	}

	if (!m_cen && !m_cxoen)
	{
		if (m_cxod)
		{
			m_orig_cx_stripe_addr--;
			if (m_orig_cx_stripe_addr == -1)
			{
				m_orig_cx_stripe_addr = 9;
				m_orig_cx_stripe_num--;
			}
		}
		else
		{
			m_orig_cx_stripe_addr++;
			if (m_orig_cx_stripe_addr == 10)
			{
				m_orig_cx_stripe_addr = 0;
				m_orig_cx_stripe_num++;
			}
		}
	}
}

void dpb7000_storeaddr_card_device::cxod_w(int state)
{
	m_cxod = (bool)state;
}

void dpb7000_storeaddr_card_device::cxoen_w(int state)
{
	m_cxoen = (bool)state;
}

void dpb7000_storeaddr_card_device::cyd_w(int state)
{
	m_cyd = (bool)state;
}

void dpb7000_storeaddr_card_device::cyen_w(int state)
{
	m_cyen = (bool)state;
}

void dpb7000_storeaddr_card_device::cyld_w(int state)
{
	m_cyld = (bool)state;
}

void dpb7000_storeaddr_card_device::cyck_w(int state)
{
	bool old = m_cyck;
	m_cyck = (bool)state;
	if (old && !m_cyck) // Inverted by 74LS240 AE
		tick_cyck();
}

void dpb7000_storeaddr_card_device::tick_cyck()
{
	if (m_cyld)
	{
		m_cy_addr = m_orig_cy_addr;
	}
	else if (!m_cen && !m_cyen)
	{
		if (m_cyd)
		{
			m_cy_addr--;
		}
		else
		{
			m_cy_addr++;
		}
		update_prot_proms();
	}

	if (!m_cen && !m_cyoen)
	{
		if (m_cyod)
			m_orig_cy_addr--;
		else
			m_orig_cy_addr++;
	}
}

void dpb7000_storeaddr_card_device::cyod_w(int state)
{
	m_cyod = (bool)state;
}

void dpb7000_storeaddr_card_device::cyoen_w(int state)
{
	m_cyoen = (bool)state;
}

void dpb7000_storeaddr_card_device::clrc_w(int state)
{
	m_clrc = (bool)state;
}

void dpb7000_storeaddr_card_device::selvideo_w(int state)
{
	m_selvideo = (bool)state;
}

void dpb7000_storeaddr_card_device::creq_w(int state)
{
	m_creq = (bool)state;
}

void dpb7000_storeaddr_card_device::cr_w(int state)
{
	m_cread = (bool)state;
}

void dpb7000_storeaddr_card_device::prot_a_w(int state)
{
	const bool old = m_prot_a;
	m_prot_a = (bool)state;
	if (old != m_prot_b)
		update_prot_proms();
}

void dpb7000_storeaddr_card_device::prot_b_w(int state)
{
	const bool old = m_prot_b;
	m_prot_b = (bool)state;
	if (old != m_prot_b)
		update_prot_proms();
}

void dpb7000_storeaddr_card_device::update_prot_proms()
{
	const uint16_t m_x_addr = ((uint8_t)m_cx_stripe_num)  | (m_prot_a ? 0x000 : 0x100) | (m_prot_b ? 0x000 : 0x200);
	const uint16_t m_y_addr = ((uint8_t)(m_cy_addr >> 2)) | (m_prot_a ? 0x000 : 0x100) | (m_prot_b ? 0x000 : 0x200);
	m_protx = BIT(m_protx_base[m_x_addr], 0);
	m_proty = BIT(m_proty_base[m_y_addr], 0);
}
