// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Alex W. Jackson
/****************************************************************************

    NEC V25/V35 special function registers and internal data area access

****************************************************************************/

#include "emu.h"
#include "v25.h"
#include "v25priv.ipp"

void v25_common_device::ida_sfr_map(address_map &map)
{
	map(0x000, 0x0ff).ram().share("internal_ram");
	map(0x100, 0x100).rw(FUNC(v25_common_device::p0_r), FUNC(v25_common_device::p0_w));
	map(0x101, 0x101).w(FUNC(v25_common_device::pm0_w));
	map(0x102, 0x102).w(FUNC(v25_common_device::pmc0_w));
	map(0x108, 0x108).rw(FUNC(v25_common_device::p1_r), FUNC(v25_common_device::p1_w));
	map(0x109, 0x109).w(FUNC(v25_common_device::pm1_w));
	map(0x10a, 0x10a).w(FUNC(v25_common_device::pmc1_w));
	map(0x110, 0x110).rw(FUNC(v25_common_device::p2_r), FUNC(v25_common_device::p2_w));
	map(0x111, 0x111).w(FUNC(v25_common_device::pm2_w));
	map(0x112, 0x112).w(FUNC(v25_common_device::pmc2_w));
	map(0x138, 0x138).r(FUNC(v25_common_device::pt_r));
	map(0x13b, 0x13b).w(FUNC(v25_common_device::pmt_w));
	map(0x140, 0x140).rw(FUNC(v25_common_device::intm_r), FUNC(v25_common_device::intm_w));
	map(0x144, 0x146).rw(FUNC(v25_common_device::ems_r), FUNC(v25_common_device::ems_w));
	map(0x14c, 0x14c).rw(FUNC(v25_common_device::exic0_r), FUNC(v25_common_device::exic0_w));
	map(0x14d, 0x14d).rw(FUNC(v25_common_device::exic1_r), FUNC(v25_common_device::exic1_w));
	map(0x14e, 0x14e).rw(FUNC(v25_common_device::exic2_r), FUNC(v25_common_device::exic2_w));
	map(0x165, 0x165).rw(FUNC(v25_common_device::srms0_r), FUNC(v25_common_device::srms0_w));
	map(0x166, 0x166).rw(FUNC(v25_common_device::stms0_r), FUNC(v25_common_device::stms0_w));
	map(0x168, 0x168).rw(FUNC(v25_common_device::scm0_r), FUNC(v25_common_device::scm0_w));
	map(0x169, 0x169).rw(FUNC(v25_common_device::scc0_r), FUNC(v25_common_device::scc0_w));
	map(0x16a, 0x16a).rw(FUNC(v25_common_device::brg0_r), FUNC(v25_common_device::brg0_w));
	map(0x16b, 0x16b).r(FUNC(v25_common_device::sce0_r));
	map(0x16c, 0x16c).rw(FUNC(v25_common_device::seic0_r), FUNC(v25_common_device::seic0_w));
	map(0x16d, 0x16d).rw(FUNC(v25_common_device::sric0_r), FUNC(v25_common_device::sric0_w));
	map(0x16e, 0x16e).rw(FUNC(v25_common_device::stic0_r), FUNC(v25_common_device::stic0_w));
	map(0x175, 0x175).rw(FUNC(v25_common_device::srms1_r), FUNC(v25_common_device::srms1_w));
	map(0x176, 0x176).rw(FUNC(v25_common_device::stms1_r), FUNC(v25_common_device::stms1_w));
	map(0x178, 0x178).rw(FUNC(v25_common_device::scm1_r), FUNC(v25_common_device::scm1_w));
	map(0x179, 0x179).rw(FUNC(v25_common_device::scc1_r), FUNC(v25_common_device::scc1_w));
	map(0x17a, 0x17a).rw(FUNC(v25_common_device::brg1_r), FUNC(v25_common_device::brg1_w));
	map(0x17b, 0x17b).r(FUNC(v25_common_device::sce1_r));
	map(0x17c, 0x17c).rw(FUNC(v25_common_device::seic1_r), FUNC(v25_common_device::seic1_w));
	map(0x17d, 0x17d).rw(FUNC(v25_common_device::sric1_r), FUNC(v25_common_device::sric1_w));
	map(0x17e, 0x17e).rw(FUNC(v25_common_device::stic1_r), FUNC(v25_common_device::stic1_w));
	map(0x180, 0x181).rw(FUNC(v25_common_device::tm0_r), FUNC(v25_common_device::tm0_w));
	map(0x182, 0x183).rw(FUNC(v25_common_device::md0_r), FUNC(v25_common_device::md0_w));
	map(0x188, 0x189).rw(FUNC(v25_common_device::tm1_r), FUNC(v25_common_device::tm1_w));
	map(0x18a, 0x18b).rw(FUNC(v25_common_device::md1_r), FUNC(v25_common_device::md1_w));
	map(0x190, 0x190).w(FUNC(v25_common_device::tmc0_w));
	map(0x191, 0x191).w(FUNC(v25_common_device::tmc1_w));
	map(0x194, 0x196).rw(FUNC(v25_common_device::tmms_r), FUNC(v25_common_device::tmms_w));
	map(0x19c, 0x19c).rw(FUNC(v25_common_device::tmic0_r), FUNC(v25_common_device::tmic0_w));
	map(0x19d, 0x19d).rw(FUNC(v25_common_device::tmic1_r), FUNC(v25_common_device::tmic1_w));
	map(0x19e, 0x19e).rw(FUNC(v25_common_device::tmic2_r), FUNC(v25_common_device::tmic2_w));
	map(0x1e1, 0x1e1).rw(FUNC(v25_common_device::rfm_r), FUNC(v25_common_device::rfm_w));
	map(0x1e8, 0x1e9).rw(FUNC(v25_common_device::wtc_r), FUNC(v25_common_device::wtc_w));
	map(0x1ea, 0x1ea).rw(FUNC(v25_common_device::flag_r), FUNC(v25_common_device::flag_w));
	map(0x1eb, 0x1eb).rw(FUNC(v25_common_device::prc_r), FUNC(v25_common_device::prc_w));
	map(0x1ec, 0x1ec).rw(FUNC(v25_common_device::tbic_r), FUNC(v25_common_device::tbic_w));
	map(0x1ef, 0x1ef).r(FUNC(v25_common_device::irqs_r));
	map(0x1fc, 0x1fc).r(FUNC(v25_common_device::ispr_r));
	map(0x1ff, 0x1ff).rw(FUNC(v25_common_device::idb_r), FUNC(v25_common_device::idb_w));
}

uint8_t v25_common_device::read_irqcontrol(int /*INTSOURCES*/ source, uint8_t priority)
{
	return  (((m_pending_irq & source)     ? 0x80 : 0x00)
			| ((m_unmasked_irq & source)   ? 0x00 : 0x40)
			| ((m_macro_service & source)  ? 0x20 : 0x00)
			| ((m_bankswitch_irq & source) ? 0x10 : 0x00)
			| priority);
}

void v25_common_device::write_irqcontrol(int /*INTSOURCES*/ source, uint8_t d)
{
	if (BIT(d, 7))
		m_pending_irq |= source;
	else
		m_pending_irq &= ~source;

	if (BIT(d, 6))
		m_unmasked_irq &= ~source;
	else
		m_unmasked_irq |= source;

	if (BIT(d, 5))
	{
		if ((m_macro_service & source) == 0)
			logerror("%06x: Warning: macro service function not implemented\n",PC());
		m_macro_service |= source;
	}
	else
		m_macro_service &= ~source;

	if (BIT(d, 4))
		m_bankswitch_irq |= source;
	else
		m_bankswitch_irq &= ~source;
}

uint8_t v25_common_device::p0_r()
{
	return m_p0_in();
}

void v25_common_device::p0_w(uint8_t d)
{
	m_p0_out(d);
}

void v25_common_device::pm0_w(uint8_t d)
{
	logerror("%06x: PM0 set to %02x\n", PC(), d);
}

void v25_common_device::pmc0_w(uint8_t d)
{
	logerror("%06x: PMC0 set to %02x\n", PC(), d);
}

uint8_t v25_common_device::p1_r()
{
	// P1 is combined with the interrupt lines
	return ((m_p1_in() & 0xf0)
			| (m_nmi_state     ? 0x00 : 0x01)
			| (m_intp_state[0] ? 0x00 : 0x02)
			| (m_intp_state[1] ? 0x00 : 0x04)
			| (m_intp_state[2] ? 0x00 : 0x08));
}

void v25_common_device::p1_w(uint8_t d)
{
	// only the upper four bits of P1 can be used as output
	m_p1_out(d & 0xf0);
}

void v25_common_device::pm1_w(uint8_t d)
{
	logerror("%06x: PM1 set to %02x\n", PC(), d);
}

void v25_common_device::pmc1_w(uint8_t d)
{
	logerror("%06x: PMC1 set to %02x\n", PC(), d);
}

uint8_t v25_common_device::p2_r()
{
	return m_p2_in();
}

void v25_common_device::p2_w(uint8_t d)
{
	m_p2_out(d);
}

void v25_common_device::pm2_w(uint8_t d)
{
	logerror("%06x: PM2 set to %02x\n", PC(), d);
}

void v25_common_device::pmc2_w(uint8_t d)
{
	logerror("%06x: PMC2 set to %02x\n", PC(), d);
}

uint8_t v25_common_device::pt_r()
{
	return m_pt_in();
}

void v25_common_device::pmt_w(uint8_t d)
{
	logerror("%06x: PMT set to %02x\n", PC(), d);
}

uint8_t v25_common_device::intm_r()
{
	return m_intm;
}

void v25_common_device::intm_w(uint8_t d)
{
	logerror("%06x: INTM set to %02x\n", PC(), d & 0x55);
	m_intm = d & 0x55;
}

uint8_t v25_common_device::ems_r(offs_t a)
{
	return m_ems[a];
}

void v25_common_device::ems_w(offs_t a, uint8_t d)
{
	logerror("%06x: EMS%d set to %02x\n", PC(), a, d & 0xf7);
	m_ems[a] = d & 0xf7;
}

uint8_t v25_common_device::exic0_r()
{
	return read_irqcontrol(INTP0, m_priority_intp);
}

void v25_common_device::exic0_w(uint8_t d)
{
	write_irqcontrol(INTP0, d);
	m_priority_intp = d & 0x7;
}

uint8_t v25_common_device::exic1_r()
{
	return read_irqcontrol(INTP1, 7);
}

void v25_common_device::exic1_w(uint8_t d)
{
	write_irqcontrol(INTP1, d);
}

uint8_t v25_common_device::exic2_r()
{
	return read_irqcontrol(INTP2, 7);
}

void v25_common_device::exic2_w(uint8_t d)
{
	write_irqcontrol(INTP2, d);
}

uint8_t v25_common_device::srms0_r()
{
	return m_srms[0];
}

void v25_common_device::srms0_w(uint8_t d)
{
	logerror("%06x: SRMS0 set to %02x\n", PC(), d & 0xf7);
	m_srms[0] = d & 0xf7;
}

uint8_t v25_common_device::stms0_r()
{
	return m_stms[0];
}

void v25_common_device::stms0_w(uint8_t d)
{
	logerror("%06x: STMS0 set to %02x\n", PC(), d & 0xf7);
	m_stms[0] = d & 0xf7;
}

uint8_t v25_common_device::scm0_r()
{
	return m_scm[0];
}

void v25_common_device::scm0_w(uint8_t d)
{
	logerror("%06x: SCM0 set to %02x\n", PC(), d);
	m_scm[0] = d;
}

uint8_t v25_common_device::scc0_r()
{
	return m_scc[0];
}

void v25_common_device::scc0_w(uint8_t d)
{
	logerror("%06x: SCC0 prescaler set to %d\n", PC(), 2 << (d & 0x0f));
	m_scc[0] = d & 0x0f;
}

uint8_t v25_common_device::brg0_r()
{
	return m_brg[0];
}

void v25_common_device::brg0_w(uint8_t d)
{
	logerror("%06x: BRG0 divider set to %d\n", PC(), d);
	m_brg[0] = d;
}

uint8_t v25_common_device::sce0_r()
{
	if (!machine().side_effects_disabled())
		logerror("%06x: Warning: read back SCE0\n",PC());
	return m_sce[0];
}

uint8_t v25_common_device::seic0_r()
{
	return read_irqcontrol(INTSER0, m_priority_ints0);
}

void v25_common_device::seic0_w(uint8_t d)
{
	// no macro service for error interrupt
	write_irqcontrol(INTSER0, d & 0xd0);
	m_priority_ints0 = d & 0x7;
}

uint8_t v25_common_device::sric0_r()
{
	return read_irqcontrol(INTSR0, m_priority_ints0);
}

void v25_common_device::sric0_w(uint8_t d)
{
	write_irqcontrol(INTSR0, d);
	m_priority_ints0 = d & 0x7;
}

uint8_t v25_common_device::stic0_r()
{
	return read_irqcontrol(INTST0, m_priority_ints0);
}

void v25_common_device::stic0_w(uint8_t d)
{
	write_irqcontrol(INTST0, d);
	m_priority_ints0 = d & 0x7;
}

uint8_t v25_common_device::srms1_r()
{
	return m_srms[1];
}

void v25_common_device::srms1_w(uint8_t d)
{
	logerror("%06x: SRMS1 set to %02x\n", PC(), d & 0xf7);
	m_srms[1] = d & 0xf7;
}

uint8_t v25_common_device::stms1_r()
{
	return m_stms[1];
}

void v25_common_device::stms1_w(uint8_t d)
{
	logerror("%06x: STMS1 set to %02x\n", PC(), d & 0xf7);
	m_stms[1] = d & 0xf7;
}

uint8_t v25_common_device::scm1_r()
{
	return m_scm[1];
}

void v25_common_device::scm1_w(uint8_t d)
{
	logerror("%06x: SCM1 set to %02x\n", PC(), d);
	m_scm[1] = d;
}

uint8_t v25_common_device::scc1_r()
{
	return m_scc[1];
}

void v25_common_device::scc1_w(uint8_t d)
{
	logerror("%06x: SCC1 prescaler set to %d\n", PC(), 2 << (d & 0x0f));
	m_scc[1] = d & 0x0f;
}

uint8_t v25_common_device::brg1_r()
{
	return m_brg[1];
}

void v25_common_device::brg1_w(uint8_t d)
{
	logerror("%06x: BRG1 divider set to %d\n", PC(), d);
	m_brg[1] = d;
}

uint8_t v25_common_device::sce1_r()
{
	if (!machine().side_effects_disabled())
		logerror("%06x: Warning: read back SCE1\n",PC());
	return m_sce[1];
}

uint8_t v25_common_device::seic1_r()
{
	return read_irqcontrol(INTSER1, m_priority_ints1);
}

void v25_common_device::seic1_w(uint8_t d)
{
	// no macro service for error interrupt
	write_irqcontrol(INTSER1, d & 0xd0);
	m_priority_ints1 = d & 0x7;
}

uint8_t v25_common_device::sric1_r()
{
	return read_irqcontrol(INTSR1, m_priority_ints1);
}

void v25_common_device::sric1_w(uint8_t d)
{
	write_irqcontrol(INTSR1, d);
	m_priority_ints1 = d & 0x7;
}

uint8_t v25_common_device::stic1_r()
{
	return read_irqcontrol(INTST1, m_priority_ints1);
}

void v25_common_device::stic1_w(uint8_t d)
{
	write_irqcontrol(INTST1, d);
	m_priority_ints1 = d & 0x7;
}

uint16_t v25_common_device::tm0_r()
{
	if (!machine().side_effects_disabled())
		logerror("%06x: Warning: read back TM0\n",PC());
	return m_TM0;
}

void v25_common_device::tm0_w(uint16_t d)
{
	m_TM0 = d;
}

uint16_t v25_common_device::md0_r()
{
	if (!machine().side_effects_disabled())
		logerror("%06x: Warning: read back MD0\n",PC());
	return m_MD0;
}

void v25_common_device::md0_w(uint16_t d)
{
	m_MD0 = d;
}

uint16_t v25_common_device::tm1_r()
{
	if (!machine().side_effects_disabled())
		logerror("%06x: Warning: read back TM1\n",PC());
	return m_TM1;
}

void v25_common_device::tm1_w(uint16_t d)
{
	m_TM1 = d;
}

uint16_t v25_common_device::md1_r()
{
	if (!machine().side_effects_disabled())
		logerror("%06x: Warning: read back MD1\n",PC());
	return m_MD1;
}

void v25_common_device::md1_w(uint16_t d)
{
	m_MD1 = d;
}

void v25_common_device::tmc0_w(uint8_t d)
{
	m_TMC0 = d;
	if (BIT(d, 0))   // oneshot mode
	{
		if (BIT(d, 7))
		{
			unsigned tmp = m_PCK * m_TM0 * (BIT(d, 6) ? 128 : 12);
			attotime time = clocks_to_attotime(tmp);
			m_timers[0]->adjust(time, INTTU0);
		}
		else
			m_timers[0]->adjust(attotime::never);

		if (BIT(d, 5))
		{
			unsigned tmp = m_PCK * m_MD0 * (BIT(d, 4) ? 128 : 12);
			attotime time = clocks_to_attotime(tmp);
			m_timers[1]->adjust(time, INTTU1);
		}
		else
			m_timers[1]->adjust(attotime::never);
	}
	else    // interval mode
	{
		if (BIT(d, 7))
		{
			unsigned tmp = m_PCK * m_MD0 * (BIT(d, 6) ? 128 : 6);
			attotime time = clocks_to_attotime(tmp);
			m_timers[0]->adjust(time, INTTU0, time);
			m_timers[1]->adjust(attotime::never);
			m_TM0 = m_MD0;
		}
		else
		{
			m_timers[0]->adjust(attotime::never);
			m_timers[1]->adjust(attotime::never);
		}
	}
}

void v25_common_device::tmc1_w(uint8_t d)
{
	m_TMC1 = d & 0xC0;
	if (BIT(d, 7))
	{
		unsigned tmp = m_PCK * m_MD1 * (BIT(d, 6) ? 128 : 6);
		attotime time = clocks_to_attotime(tmp);
		m_timers[2]->adjust(time, INTTU2, time);
		m_TM1 = m_MD1;
	}
	else
		m_timers[2]->adjust(attotime::never);
}

uint8_t v25_common_device::tmms_r(offs_t a)
{
	return m_tmms[a];
}

void v25_common_device::tmms_w(offs_t a, uint8_t d)
{
	logerror("%06x: TMMS%d set to %02x\n", PC(), a, d & 0xf7);
	m_tmms[a] = d & 0xf7;
}

uint8_t v25_common_device::tmic0_r()
{
	return read_irqcontrol(INTTU0, m_priority_inttu);
}

void v25_common_device::tmic0_w(uint8_t d)
{
	write_irqcontrol(INTTU0, d);
	m_priority_inttu = d & 0x7;
}

uint8_t v25_common_device::tmic1_r()
{
	return read_irqcontrol(INTTU1, 7);
}

void v25_common_device::tmic1_w(uint8_t d)
{
	write_irqcontrol(INTTU1, d);
}

uint8_t v25_common_device::tmic2_r()
{
	return read_irqcontrol(INTTU2, 7);
}

void v25_common_device::tmic2_w(uint8_t d)
{
	write_irqcontrol(INTTU2, d);
}

uint8_t v25_common_device::rfm_r()
{
	return m_RFM;
}

void v25_common_device::rfm_w(uint8_t d)
{
	m_RFM = d;
}

uint16_t v25_common_device::wtc_r()
{
	return m_WTC;
}

void v25_common_device::wtc_w(offs_t a, uint16_t d, uint16_t m)
{
	m_WTC = (m_WTC & ~m) | (d & m);
}

uint8_t v25_common_device::flag_r()
{
	return (m_F0 << 3) | (m_F1 << 5);
}

void v25_common_device::flag_w(uint8_t d)
{
	m_F0 = BIT(d, 3);
	m_F1 = BIT(d, 5);
}

uint8_t v25_common_device::prc_r()
{
	uint8_t ret = m_RAMEN ? 0x40 : 0;

	switch (m_TB)
	{
	case 10:
		break;
	case 13:
		ret |= 0x04;
		break;
	case 16:
		ret |= 0x08;
		break;
	case 20:
		ret |= 0x0C;
		break;
	}

	switch (m_PCK)
	{
	case 2:
		break;
	case 4:
		ret |= 0x01;
		break;
	case 8:
		ret |= 0x02;
		break;
	}

	return ret;
}

void v25_common_device::prc_w(uint8_t d)
{
	static const int timebases[4] = { 10, 13, 16, 20 };
	static const int clocks[4] = { 2, 4, 8, 0 };

	logerror("%06x: PRC set to %02x\n", PC(), d);
	m_RAMEN = ((d & 0x40) == 0x40);
	m_TB = timebases[(d & 0x0C) >> 2];
	m_PCK = clocks[d & 0x03];
	if (m_PCK == 0)
	{
		logerror("        Warning: invalid clock divider\n");
		m_PCK = 8;
	}

	unsigned tmp = m_PCK << m_TB;
	attotime time = clocks_to_attotime(tmp);
	m_timers[3]->adjust(time, INTTB, time);
	notify_clock_changed(); // make device_execute_interface pick up the new clocks_to_cycles()

	logerror("        Internal RAM %sabled\n", (m_RAMEN ? "en" : "dis"));
	logerror("        Time base set to 2^%d\n", m_TB);
	logerror("        Clock divider set to %d\n", m_PCK);
}

uint8_t v25_common_device::tbic_r()
{
	return read_irqcontrol(INTTB, 7);
}

void v25_common_device::tbic_w(uint8_t d)
{
	// time base interrupt doesn't support macro service, bank switching or priority control
	write_irqcontrol(INTTB, d & 0xC0);
}

uint8_t v25_common_device::irqs_r()
{
	return m_IRQS;
}

uint8_t v25_common_device::ispr_r()
{
	return m_ISPR;
}

uint8_t v25_common_device::idb_r()
{
	return m_IDB >> 12;
}

void v25_common_device::idb_w(uint8_t d)
{
	m_IDB = (d << 12) | 0xe00;
}

uint8_t v25_common_device::v25_read_byte(unsigned a)
{
	if (((a & 0xffe00) == m_IDB && (m_RAMEN || BIT(a, 8))) || a == 0xfffff)
		return m_data.read_byte(a & 0x1ff);
	else
		return m_program->read_byte(a);
}

uint16_t v25_common_device::v25_read_word(unsigned a)
{
	if (BIT(a, 0))
		return (v25_read_byte(a) | (v25_read_byte(a + 1) << 8));

	// not sure about this - manual says FFFFC-FFFFE are "reserved"
	if (a == 0xffffe)
		return (m_program->read_byte(a) | (m_data.read_byte(0x1ff) << 8));
	else if ((a & 0xffe00) == m_IDB && (m_RAMEN || BIT(a, 8)))
		return m_data.read_word(a & 0x1ff);
	else
		return m_program->read_word(a);
}

void v25_common_device::v25_write_byte(unsigned a, uint8_t d)
{
	if (((a & 0xffe00) == m_IDB && (m_RAMEN || BIT(a, 8))) || a == 0xfffff)
		m_data.write_byte(a & 0x1ff, d);
	else
		m_program->write_byte(a, d);
}

void v25_common_device::v25_write_word(unsigned a, uint16_t d)
{
	if (BIT(a, 0))
	{
		v25_write_byte(a, d);
		v25_write_byte(a + 1, d >> 8);
		return;
	}

	// not sure about this - manual says FFFFC-FFFFE are "reserved"
	if (a == 0xffffe)
	{
		m_program->write_byte(a, d);
		m_data.write_byte(0x1ff, d >> 8);
	}
	else if ((a & 0xffe00) == m_IDB && (m_RAMEN || BIT(a, 8)))
		m_data.write_word(a & 0x1ff, d);
	else
		m_program->write_word(a, d);
}

bool v25_common_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	if (spacenum == AS_PROGRAM && intention != TR_FETCH && (((address & 0xffe00) == m_IDB && (m_RAMEN || BIT(address, 8))) || address == 0xfffff))
	{
		address &= 0x1ff;
		target_space = &m_data.space();
	}
	else
		target_space = &space(spacenum);
	return true;
}
