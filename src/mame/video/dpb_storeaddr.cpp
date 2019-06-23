// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_storeaddr.cpp
    DPB-7000/1 - Store Address Card

    TODO:
    - Code is currently a more or less direct translation of the board
      schematic. It is highly inefficient, but accurate. An equally-
      accurate, but faster, version can be made once better understanding
      of the overall DPB-7000 system is had.

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
	, m_blanking_base(nullptr)
	, m_bb_out(0)
	, m_bc_out(0)
	, m_bd_out(0)
	, m_protx(false)
	, m_proty(false)
	, m_delay_timer(nullptr)
	, m_delay_step(0)
	, m_df_out(0)
	, m_ee_out(0)
	, m_addr(0)
	, m_rhscr(0)
	, m_rvscr(0)
	, m_rzoom(0)
	, m_fld_sel(0)
	, m_hzoom_count(0)
	, m_vzoom_count(0)
	, m_orig_cx_stripe_addr(0)
	, m_orig_cx_stripe_num(0)
	, m_orig_cy_addr(0)
	, m_cx_stripe_addr(0)
	, m_cx_stripe_num(0)
	, m_cy_addr(0)
	, m_rhscr_stripe_addr(0)
	, m_rhscr_stripe_num(0)
	, m_rvscr_counter(0)
	, m_rvscr_with_v0(0)
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
	, m_preread(false)
	, m_rreq_pending(false)
	, m_rreq_active(false)
	, m_creq_pending(false)
	, m_creq_active(false)
	, m_store_busy(false)
	, m_rvl(false)
	, m_rhr(false)
	, m_plt(false)
	, m_zb(false)
	, m_rppck(false)
	, m_rb(false)
	, m_pflag(false)
	, m_mxr(false)
	, m_ras(false)
	, m_cas(false)
	, m_laac(false)
	, m_t6(false)
	, m_clrw(false)
	, m_opstr(false)
	, m_cck_clear(false)
	, m_creq_sel(false)
	, m_write_active(false)
	, m_window_enable(false)
	, m_b26(false)
	, m_blank_d(false)
	, m_blank_a(false)
	, m_blank_b(false)
	, m_blank_q(0)
	, m_crc(false)
	, m_ipen(false)
	, m_ipsel(false)
	, m_rck(false)
	, m_ra(0)
	, m_opra(false)
	, m_opwa(false)
	, m_opwb(false)
	, m_cck(false)
	, m_csel(false)
	, m_ipsel_out(*this)
	, m_rck_out(*this)
	, m_ra_out(*this)
	, m_opra_out(*this)
	, m_oprb_out(*this)
	, m_blk_out(*this)
	, m_addr_out(*this)
	, m_r_busy_out(*this)
	, m_ras_out(*this)
	, m_cas_out(*this)
	, m_opwb_out(*this)
	, m_opstr_out(*this)
	, m_w_out(*this)
	, m_opwa_out(*this)
	, m_csel_out(*this)
	, m_cck_out(*this)
	, m_cbusy_out(*this)
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

	save_item(NAME(m_delay_step));

	save_item(NAME(m_df_in));
	save_item(NAME(m_df_out));
	save_item(NAME(m_ee_in));
	save_item(NAME(m_ee_out));

	save_item(NAME(m_dg_in));
	save_item(NAME(m_eg_in));
	save_item(NAME(m_fg_in));
	save_item(NAME(m_gg_in));

	save_item(NAME(m_rhscr));
	save_item(NAME(m_rvscr));
	save_item(NAME(m_rzoom));
	save_item(NAME(m_fld_sel));

	save_item(NAME(m_hzoom_count));
	save_item(NAME(m_vzoom_count));

	save_item(NAME(m_orig_cx_stripe_addr));
	save_item(NAME(m_orig_cx_stripe_num));
	save_item(NAME(m_orig_cy_addr));
	save_item(NAME(m_cx_stripe_addr));
	save_item(NAME(m_cx_stripe_num));
	save_item(NAME(m_cy_addr));

	save_item(NAME(m_rhscr_stripe_addr));
	save_item(NAME(m_rhscr_stripe_num));

	save_item(NAME(m_rvscr_counter));
	save_item(NAME(m_rvscr_with_v0));

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

	save_item(NAME(m_prot_a));
	save_item(NAME(m_prot_b));

	save_item(NAME(m_preread));
	save_item(NAME(m_rreq_pending));
	save_item(NAME(m_rreq_active));
	save_item(NAME(m_creq_pending));
	save_item(NAME(m_creq_active));
	save_item(NAME(m_store_busy));

	save_item(NAME(m_rvl));
	save_item(NAME(m_rhr));
	save_item(NAME(m_plt));
	save_item(NAME(m_zb));
	save_item(NAME(m_rppck));
	save_item(NAME(m_rb));
	save_item(NAME(m_pflag));

	save_item(NAME(m_mxr));
	save_item(NAME(m_ras));
	save_item(NAME(m_cas));
	save_item(NAME(m_store_busy));
	save_item(NAME(m_laac));
	save_item(NAME(m_t6));
	save_item(NAME(m_clrw));
	save_item(NAME(m_opstr));
	save_item(NAME(m_cck_clear));

	save_item(NAME(m_creq_sel));

	save_item(NAME(m_write_active));

	save_item(NAME(m_window_enable));
	save_item(NAME(m_b26));

	save_item(NAME(m_blank_d));
	save_item(NAME(m_blank_a));
	save_item(NAME(m_blank_b));
	save_item(NAME(m_blank_q));

	save_item(NAME(m_crc));
	save_item(NAME(m_ipen));

	save_item(NAME(m_ipsel));
	save_item(NAME(m_rck));
	save_item(NAME(m_ra));
	save_item(NAME(m_opra));
	save_item(NAME(m_opwa));
	save_item(NAME(m_opwb));
	save_item(NAME(m_clrw));
	save_item(NAME(m_cck));
	save_item(NAME(m_csel));

	m_ipsel_out.resolve_safe();
	m_rck_out.resolve_safe();
	m_ra_out.resolve_safe();
	m_opra_out.resolve_safe();
	m_oprb_out.resolve_safe();
	m_blk_out.resolve_safe();
	m_addr_out.resolve_safe();
	m_r_busy_out.resolve_safe();
	m_ras_out.resolve_safe();
	m_cas_out.resolve_safe();
	m_opwb_out.resolve_safe();
	m_opstr_out.resolve_safe();
	m_w_out.resolve_safe();
	m_opwa_out.resolve_safe();
	m_csel_out.resolve_safe();
	m_cck_out.resolve_safe();
	m_cbusy_out.resolve_safe();

	m_delay_timer = timer_alloc(DELAY_TIMER);
	m_delay_timer->adjust(attotime::never);
}

void dpb7000_storeaddr_card_device::device_reset()
{
	m_bb_out = 0;
	m_bc_out = 0;
	m_bd_out = 0;
	m_protx = false;
	m_proty = false;

	m_delay_timer->adjust(attotime::never);
	m_delay_step = 0;

	memset(m_df_in, 0, 2);
	m_df_in[1] = 0xc;
	m_df_out = 0;

	memset(m_ee_in, 0, 2);
	m_ee_in[1] = 0xc;
	m_ee_out = 0;

	memset(m_dg_in, 0, 2);
	memset(m_eg_in, 0, 2);
	memset(m_fg_in, 0, 2);
	memset(m_gg_in, 0, 2);
	m_addr = 0;

	m_rhscr = 0;
	m_rvscr = 0;
	m_rzoom = 0;
	m_fld_sel = 0;

	m_hzoom_count = 0;
	m_vzoom_count = 0;

	m_orig_cx_stripe_addr = 0;
	m_orig_cx_stripe_num = 0;
	m_orig_cy_addr = 0;
	m_cx_stripe_addr = 0;
	m_cx_stripe_num = 0;
	m_cy_addr = 0;

	m_rhscr_stripe_addr = 0;
	m_rhscr_stripe_num = 0;

	m_rvscr_counter = 0;
	m_rvscr_with_v0 = 0;

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

	m_prot_a = false;
	m_prot_b = false;

	m_preread = false;
	m_rreq_pending = false;
	m_rreq_active = false;
	m_creq_pending = false;
	m_creq_active = false;
	m_store_busy = false;

	m_rvl = false;
	m_rhr = false;
	m_plt = false;
	m_zb = false;
	m_rppck = false;
	m_rb = false;
	m_pflag = false;

	m_mxr = false;
	m_ras = false;
	m_cas = false;
	m_store_busy = false;
	m_laac = false;
	m_t6 = false;
	m_clrw = false;
	m_opstr = false;
	m_cck_clear = false;

	m_creq_sel = false;

	m_write_active = false;

	m_window_enable = false;
	m_b26 = false;

	m_blank_d = false;
	m_blank_a = false;
	m_blank_b = false;
	m_blank_q = 0;

	m_crc = false;
	m_ipen = false;

	m_ipsel = false;
	m_rck = false;
	m_ra = 0;
	m_opra = false;
	m_opwa = false;
	m_opwb = false;
	m_clrw = false;
	m_cck = false;
	m_csel = false;

	m_bb_base = m_x_prom->base() + 0x000;
	m_bc_base = m_x_prom->base() + 0x400;
	m_bd_base = m_x_prom->base() + 0x800;
	m_protx_base = m_protx_prom->base();
	m_proty_base = m_proty_prom->base();
	m_blanking_base = m_blanking_pal->base();
}

void dpb7000_storeaddr_card_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == DELAY_TIMER)
	{
		tick_delay_step();
	}
}

void dpb7000_storeaddr_card_device::tick_delay_step()
{
	m_delay_step++;
	switch (m_delay_step & 15)
	{
	case 1:
		ras_w(true);
		break;
	case 3:
		mxr_w(false);
		break;
	case 5:
		cas_w(true);
		break;
	case 6:
		t6_w(true);
		break;
	case 10:
		t6_w(false);
		laac_w(true);
		break;
	case 11:
		ras_w(false);
		break;
	case 12:
		cas_w(false);
		laac_w(false);
		break;
	case 13:
		clrw_w(true);
		break;
	case 14:
		clrw_w(false);
		break;
	case 15:
	default:
		// Do nothing
		break;
	}
}

void dpb7000_storeaddr_card_device::mxr_w(bool state)
{
	m_mxr = state;
	update_addr_select_outputs();
}

void dpb7000_storeaddr_card_device::opwb_w(bool state)
{
	const bool old = m_opwb;
	m_opwb = state;
	if (old != m_opwb)
	{
		m_opwb_out(!m_opwb);
		update_r_busy(m_ras);

		m_opwa_out(!(m_opwb || BIT(m_rhscr_stripe_num, 0)));
	}
}

void dpb7000_storeaddr_card_device::update_opwa()
{
	const bool old = m_opwa;
	m_opwa = !(m_opwb || BIT(m_rhscr_stripe_num, 0));
	if (old != m_opwa)
	{
		m_opwa_out(m_opwa);
		update_req_clears();
	}
}

void dpb7000_storeaddr_card_device::update_req_clears()
{
	if (m_laac)
	{
		if (m_opwa)
		{
			m_creq_pending = false;
			m_creq_active = false;
		}
		else
		{
			m_rreq_pending = false;
			m_rreq_active = false;
		}
	}
}

void dpb7000_storeaddr_card_device::update_opstr()
{
	const bool old = m_opstr;
	m_opstr = !(m_t6 && !m_write_active);
	if (old != m_opstr)
	{
		m_opstr_out(m_opstr);
	}
}

void dpb7000_storeaddr_card_device::update_cck()
{
	const bool old_clear = m_cck_clear;
	m_cck_clear = !(m_opwb && m_t6);

	if (old_clear != m_cck_clear)
	{
		const bool old_csel = m_csel;
		m_csel = BIT(m_cx_stripe_addr, 0);
		if (old_csel != m_csel)
		{
			m_csel_out(m_csel);
		}
	}

	const bool old = m_cck;
	m_cck = !m_cck_clear;
	if (old != m_cck)
	{
		m_cck_out(m_cck);
	}
}

void dpb7000_storeaddr_card_device::ras_w(bool state)
{
	const bool old_ras = m_ras;
	m_ras = state;
	if (old_ras != m_ras)
	{
		m_ras_out(!m_ras);
		update_r_busy(old_ras);
	}
}

void dpb7000_storeaddr_card_device::update_r_busy(bool old_ras)
{
	const bool old_r_busy = m_opwb || !old_ras;
	const bool r_busy = m_opwb || !m_ras;
	if (old_r_busy != r_busy)
	{
		m_r_busy_out(r_busy);
	}
}

void dpb7000_storeaddr_card_device::cas_w(bool state)
{
	const bool old_cas = m_cas;
	m_cas = state;
	if (old_cas != m_cas)
	{
		m_cas_out(!m_cas);
	}
}

void dpb7000_storeaddr_card_device::laac_w(bool state)
{
	const bool old_laac = m_laac;
	m_laac = state;
	if (old_laac != m_laac)
	{
		update_req_clears();
	}
}

void dpb7000_storeaddr_card_device::t6_w(bool state)
{
	const bool old = m_t6;
	m_t6 = state;
	if (old != m_t6)
	{
		update_opstr();
		update_cck();
	}
}

void dpb7000_storeaddr_card_device::clrw_w(bool state)
{
	const bool old = m_clrw;
	m_clrw = state;
	if (old != m_clrw)
	{
		if (m_clrw)
		{
			opwb_w(false);
			m_write_active = false;
			update_opstr();
		}
	}
}

void dpb7000_storeaddr_card_device::reg_w(uint16_t data)
{
	switch ((data >> 12) & 7)
	{
		case 0:
		{
			LOG("%s: Store Address Card %d, set RHSCR: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			const uint16_t old = m_rhscr;
			m_rhscr = data & 0xfff;
			if (BIT(old, 0) != BIT(m_rhscr, 0))
			{
				update_rck();
			}
			break;
		}
		case 1:
			LOG("%s: Store Address Card %d, set RVSCR: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_rvscr = data & 0xfff;
			break;
		case 2:
		{
			LOG("%s: Store Address Card %d, set R ZOOM: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			const uint8_t old = m_rzoom;
			m_rzoom = data & 0xf;
			if (BIT(old, 0) != BIT(m_rzoom, 0))
			{
				update_rck();
			}
			break;
		}
		case 3:
			LOG("%s: Store Address Card %d, set FLDSEL: %03x\n", machine().describe_context(), m_s_type, data & 0xfff);
			m_fld_sel = data & 0xf;
			m_window_enable = BIT(m_fld_sel, 2);
			m_blank_b = !(m_window_enable && m_b26);
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
		const int8_t old_ca = m_cx_stripe_addr;
		const int16_t old_cn = m_cx_stripe_num;

		if (m_cxd)
		{
			m_cx_stripe_addr--;
			if (m_cx_stripe_addr == -1)
			{
				m_cx_stripe_addr = 9;
				m_cx_stripe_num--;
				m_crc = true;
				update_prot_proms();
			}
			else
			{
				m_crc = false;
			}
		}
		else
		{
			m_cx_stripe_addr++;
			if (m_cx_stripe_addr == 10)
			{
				m_cx_stripe_addr = 0;
				m_cx_stripe_num++;
				m_crc = true;
				update_prot_proms();
			}
			else
			{
				m_crc = false;
			}
		}

		if ((old_ca & 0x0e) != (m_cx_stripe_addr & 0x0e) && !m_rck)
		{
			m_ra &= 1;
			m_ra |= m_cx_stripe_addr & 0x0e;
			m_ra_out(m_ra);
		}

		const int16_t changed = old_cn ^ m_cx_stripe_num;
		if (changed & 0x30)
			update_addr_mux_inputs();
		else if (changed & 0x40)
			update_addr_mux_outputs();
		else
			update_addr_select_inputs();
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
		const int16_t old_cy = m_cy_addr;
		if (m_cyd)
		{
			m_cy_addr--;
		}
		else
		{
			m_cy_addr++;
		}

		const int16_t changed = old_cy ^ m_cy_addr;
		if (changed & 0x300)
			update_addr_mux_inputs();
		else
			update_addr_select_inputs();

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
	const bool old = m_clrc;
	m_clrc = (bool)state;
	if (old != m_clrc && !m_clrc)
	{
		m_creq_pending = false;
		m_creq_active = true;
	}
}

void dpb7000_storeaddr_card_device::selvideo_w(int state)
{
	const bool old = m_selvideo;
	m_selvideo = (bool)state;
	if (!old && m_selvideo && BIT(m_cx_stripe_addr, 3))
	{
		request_c_read();
	}
}

void dpb7000_storeaddr_card_device::creq_w(int state)
{
	const bool old = m_creq;
	m_creq = (bool)state;
	if (!old && m_creq)
	{
		request_c_read();
	}
}

void dpb7000_storeaddr_card_device::cr_w(int state)
{
	m_cread = (bool)state;
}

void dpb7000_storeaddr_card_device::request_r_read()
{
	m_rreq_pending = true;
	check_r_read();
}

void dpb7000_storeaddr_card_device::check_r_read()
{
	if (m_rreq_pending && !m_creq_active)
	{
		m_rreq_active = true;
		m_creq_sel = false;
		update_addr_select_outputs();
		check_cycle_start();
	}
}

void dpb7000_storeaddr_card_device::request_c_read()
{
	const bool old = m_creq_pending;
	m_creq_pending = true;
	if (!old && m_creq_pending)
	{
		m_cbusy_out(!m_creq_pending);
	}
	check_c_read();
}

void dpb7000_storeaddr_card_device::check_c_read()
{
	if (m_creq_pending && !m_rreq_active)
	{
		m_creq_active = true;
		m_creq_sel = true;
		update_addr_select_outputs();
		check_cycle_start();

		const bool old_write = m_write_active;
		m_write_active = !(m_protx || m_proty || m_cread || BIT(m_cy_addr, 10));
		if (old_write != m_write_active)
		{
			update_opstr();
			m_w_out(m_write_active);
		}

		opwb_w(true);
	}
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

void dpb7000_storeaddr_card_device::preread_w(int state)
{
	const bool old = m_preread;
	m_preread = (bool)state;
	if (!old && m_preread)
	{
		request_r_read();
	}
}

void dpb7000_storeaddr_card_device::check_cycle_start()
{
	if ((m_creq_active || m_rreq_active) && !m_store_busy)
	{
		m_store_busy = true;
		mxr_w(true);
		m_delay_timer->adjust(attotime::from_nsec(25), 0, attotime::from_nsec(25));
	}
}

void dpb7000_storeaddr_card_device::update_prot_proms()
{
	const uint16_t m_x_addr = ((uint8_t)m_cx_stripe_num)  | (m_prot_a ? 0x000 : 0x100) | (m_prot_b ? 0x000 : 0x200);
	const uint16_t m_y_addr = ((uint8_t)(m_cy_addr >> 2)) | (m_prot_a ? 0x000 : 0x100) | (m_prot_b ? 0x000 : 0x200);
	m_protx = BIT(m_protx_base[m_x_addr], 0);
	m_proty = BIT(m_proty_base[m_y_addr], 0);
}

void dpb7000_storeaddr_card_device::rvl_w(int state)
{
	m_rvl = (bool)state;
}

void dpb7000_storeaddr_card_device::rhr_w(int state)
{
	const bool old = m_rhr;
	m_rhr = (bool)state;
	if (old && !m_rhr)
	{
		if (!m_rhr)
		{
			m_rreq_pending = false;
			m_rreq_active = false;
		}

		if (!m_plt && !m_rhr)
		{
			m_rvscr_counter = 0;
		}

		const uint8_t old_vzoom = m_vzoom_count;
		if (!m_rvl || m_vzoom_count == 15)
		{
			m_vzoom_count = ~m_rzoom & 15;
		}
		else
		{
			m_vzoom_count++;
		}

		if (!m_rvl)
		{
			m_rvscr_counter = m_rvscr;
		}
		else if (m_zb || old_vzoom == 15)
		{
			const uint16_t old_counter = m_rvscr_counter;
			m_rvscr_counter++;

			if (((old_counter ^ m_rvscr_counter) & 0x3fc) != 0)
				update_blanking_pal();

			const int16_t changed = old_counter ^ m_rvscr_counter;
			if (changed & 0x300)
				update_addr_mux_inputs();
			else
				update_addr_select_inputs();
		}
		update_v0();
	}
}

void dpb7000_storeaddr_card_device::plt_w(int state)
{
	m_plt = (bool)state;
	update_rck();
}

void dpb7000_storeaddr_card_device::zb_w(int state)
{
	m_zb = (bool)state;
}

void dpb7000_storeaddr_card_device::rppck_w(int state)
{
	const bool old = m_rppck;
	m_rppck = (bool)state;
	if (old && !m_rppck)
	{
		if (!m_plt && !m_rppck)
		{
			m_rvscr_counter = 0;
		}

		const uint8_t old_hzoom = m_hzoom_count;
		if (!m_rhr || m_hzoom_count == 15)
		{
			m_hzoom_count = ~m_rzoom & 15;
		}

		m_blank_q <<= 1;
		if (m_blank_a && m_blank_b)
		{
			m_blank_q |= 1;
		}
		m_blk_out(BIT(m_blank_q, 7));

		if (m_zb || old_hzoom == 15)
		{
			const uint8_t old_addr = m_rhscr_stripe_addr;
			m_rhscr_stripe_addr++;
			if ((old_addr & 0x08) != (m_rhscr_stripe_addr & 0x08))
			{
				m_blank_a = !m_blank_d;
			}

			if (m_rhscr_stripe_addr == 10)
			{
				m_rhscr_stripe_addr = 0;

				const uint8_t old_stripe_num = m_rhscr_stripe_num;
				m_rhscr_stripe_num++;
				update_opwa();
				update_blanking_pal();
				if ((old_stripe_num & 1) != (m_rhscr_stripe_num & 1) && m_rck)
				{
					m_opra = BIT(m_rhscr_stripe_num, 0);
					m_opra_out(m_opra);
				}

				const int16_t changed = old_stripe_num ^ m_rhscr_stripe_num;
				if (changed & 0x30)
					update_addr_mux_inputs();
				else if (changed & 0x40)
					update_addr_mux_outputs();
				else
					update_addr_select_inputs();
			}

			const uint8_t old_ra = m_ra;
			if (m_rck)
			{
				m_ra = m_rhscr_stripe_addr;
			}
			else
			{
				m_ra &= ~1;
				m_ra |= BIT(m_rhscr_stripe_addr, 0);
			}

			if (old_ra != m_ra)
				m_ra_out(m_ra);
		}
	}
}

void dpb7000_storeaddr_card_device::rb_w(int state)
{
	m_rb = (bool)state;
}

void dpb7000_storeaddr_card_device::pflag_w(int state)
{
	const bool old = m_pflag;
	m_pflag = (bool)state;
	if (old != m_pflag)
		update_rck();
}

void dpb7000_storeaddr_card_device::b26_w(int state)
{
	m_b26 = (bool)state;
	m_blank_b = !(m_window_enable && m_b26);
}

void dpb7000_storeaddr_card_device::ipen_w(int state)
{
	const bool old_en = m_ipen;
	m_ipen = (bool)state;
	if (!old_en && m_ipen)
	{
		const bool old_sel = m_ipsel;
		m_ipsel = ((!m_crc || m_selvideo) != m_ipsel);
		if (old_sel != m_ipsel)
			m_ipsel_out(m_ipsel);
	}
}

void dpb7000_storeaddr_card_device::update_rck()
{
	const bool scroll0 = BIT(m_rhscr, 0);
	const bool zoom0 = BIT(m_rzoom, 0);
	const bool old_rck = m_rck;
	m_rck = (scroll0 && m_plt && !zoom0) != m_pflag;
	if (old_rck != m_rck)
	{
		m_rck_out(m_rck);
		m_oprb_out(m_rck);
		if (!old_rck && m_rck)
		{
			update_cck();
		}

		const uint8_t old_ra = m_ra;
		const bool old_opra = m_opra;
		const bool ah_sel = !m_rck;
		m_ra = (m_rhscr_stripe_addr & 1);
		if (ah_sel)
		{
			m_ra |= m_cx_stripe_addr & 0x0e;
			m_opra = false;
		}
		else
		{
			m_ra |= m_rhscr_stripe_addr & 0x0e;
			m_opra = BIT(m_rhscr_stripe_num, 0);
		}

		if (old_ra != m_ra)
			m_ra_out(m_ra);
		if (old_opra != m_opra)
			m_opra_out(m_opra);
	}
}

void dpb7000_storeaddr_card_device::update_v0()
{
	// FLDSEL1  VLSB  NAND1  FLDSEL0  V0
	// 0        0     1      0        1
	// 0        1     1      0        1
	// 1        0     1      0        1
	// 1        1     0      0        1
	// 0        0     1      1        0
	// 0        1     1      1        0
	// 1        0     1      1        0
	// 1        1     0      1        1
	const bool v0 = !BIT(m_fld_sel, 0) || (BIT(m_fld_sel, 1) && BIT(m_rvscr_counter, 0));
	m_rvscr_with_v0 = (m_rvscr_counter &~ 1) | (v0 ? 1 : 0);
}

void dpb7000_storeaddr_card_device::update_blanking_pal()
{
	uint16_t addr = (m_rhscr_stripe_num >> 4);
	addr |= (m_rvscr_counter & 0xfc) << 2;
	addr |= BIT(m_rhscr_stripe_num, 0) << 10;
	addr |= (m_rvscr_counter & 0x300) << 3;
	addr |= (m_rhscr_stripe_num & 0x0e) << 12;
	const uint8_t blanking = m_blanking_base[addr];
	m_blank_d = !(BIT(blanking, 0) && BIT(blanking, 1));
}

void dpb7000_storeaddr_card_device::update_addr_mux_inputs()
{
	m_df_in[0] = (m_cx_stripe_num & 0x30) >> 4;
	const uint8_t cy_df_bits = (m_cy_addr & 0x300) >> 8;
	m_df_in[0] |= cy_df_bits << 2;
	m_df_in[1] = cy_df_bits | 0xc;

	m_ee_in[0] = (m_rhscr_stripe_num & 0x30) >> 4;
	const uint8_t ry_ee_bits = (m_rvscr_counter & 0x300) >> 8;
	m_ee_in[0] |= ry_ee_bits << 2;
	m_ee_in[1] = ry_ee_bits | 0xc;

	update_addr_mux_outputs();
}

void dpb7000_storeaddr_card_device::update_addr_mux_outputs()
{
	m_df_out = m_df_in[BIT(m_cx_stripe_num, 6)];
	m_ee_out = m_ee_in[BIT(m_rhscr_stripe_num, 6)];

	update_addr_select_inputs();
}

void dpb7000_storeaddr_card_device::update_addr_select_inputs()
{
	m_dg_in[0] = BIT(m_rhscr_stripe_num, 0) | (BIT(m_ee_out,        0) << 1) | (BIT(m_cx_stripe_num, 0) << 2) | (BIT(m_df_out,  0) << 3);
	m_dg_in[1] = BIT(m_rhscr_stripe_num, 1) | (BIT(m_ee_out,        1) << 1) | (BIT(m_cx_stripe_num, 1) << 2) | (BIT(m_df_out,  1) << 3);
	m_eg_in[0] = BIT(m_rhscr_stripe_num, 2) | (BIT(m_rvscr_with_v0, 5) << 1) | (BIT(m_cx_stripe_num, 2) << 2) | (BIT(m_cy_addr, 5) << 3);
	m_eg_in[1] = BIT(m_rhscr_stripe_num, 3) | (BIT(m_rvscr_with_v0, 6) << 1) | (BIT(m_cx_stripe_num, 3) << 2) | (BIT(m_cy_addr, 6) << 3);
	m_fg_in[0] = BIT(m_rvscr_with_v0,    1) | (BIT(m_rvscr_with_v0, 7) << 1) | (BIT(m_cy_addr,       1) << 2) | (BIT(m_cy_addr, 7) << 3);
	m_fg_in[1] = BIT(m_rvscr_with_v0,    2) | (BIT(m_ee_out,        2) << 1) | (BIT(m_cy_addr,       2) << 2) | (BIT(m_df_out,  2) << 3);
	m_gg_in[0] = BIT(m_rvscr_with_v0,    3) | (BIT(m_ee_out,        3) << 1) | (BIT(m_cy_addr,       3) << 2) | (BIT(m_df_out,  3) << 3);
	m_gg_in[1] = BIT(m_rvscr_with_v0,    4) | (BIT(m_rvscr_with_v0, 0) << 1) | (BIT(m_cy_addr,       4) << 2) | (BIT(m_cy_addr, 0) << 3);

	update_addr_select_outputs();
}

void dpb7000_storeaddr_card_device::update_addr_select_outputs()
{
	const uint8_t sel = (m_mxr ? 1 : 0) | (m_creq_sel ? 2 : 0);
	const uint8_t old = m_addr;
	m_addr  = BIT(m_dg_in[0], sel);
	m_addr |= BIT(m_dg_in[1], sel) << 1;
	m_addr |= BIT(m_eg_in[0], sel) << 2;
	m_addr |= BIT(m_eg_in[1], sel) << 3;
	m_addr |= BIT(m_fg_in[0], sel) << 4;
	m_addr |= BIT(m_fg_in[1], sel) << 5;
	m_addr |= BIT(m_gg_in[0], sel) << 6;
	m_addr |= BIT(m_gg_in[1], sel) << 7;
	if (old != m_addr)
		m_addr_out(m_addr);
}
