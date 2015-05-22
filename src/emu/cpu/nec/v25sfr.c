// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Alex W. Jackson
/****************************************************************************

    NEC V25/V35 special function registers and internal ram access

****************************************************************************/

#include "emu.h"
#include "v25.h"
#include "v25priv.h"

UINT8 v25_common_device::read_irqcontrol(int /*INTSOURCES*/ source, UINT8 priority)
{
	return  (((m_pending_irq & source)     ? 0x80 : 0x00)
			| ((m_unmasked_irq & source)   ? 0x00 : 0x40)
			| ((m_bankswitch_irq & source) ? 0x10 : 0x00)
			| priority);
}

UINT8 v25_common_device::read_sfr(unsigned o)
{
	UINT8 ret;

	switch(o)
	{
		case 0x00: /* P0 */
			ret = m_io->read_byte(V25_PORT_P0);
			break;
		case 0x08: /* P1 */
			/* P1 is combined with the interrupt lines */
			ret = ((m_io->read_byte(V25_PORT_P1) & 0xF0)
					| (m_nmi_state     ? 0x00 : 0x01)
					| (m_intp_state[0] ? 0x00 : 0x02)
					| (m_intp_state[1] ? 0x00 : 0x04)
					| (m_intp_state[2] ? 0x00 : 0x08));
			break;
		case 0x10: /* P2 */
			ret = m_io->read_byte(V25_PORT_P2);
			break;
		case 0x38: /* PT */
			ret = m_io->read_byte(V25_PORT_PT);
			break;
		case 0x4C: /* EXIC0 */
			ret = read_irqcontrol(INTP0, m_priority_intp);
			break;
		case 0x4D: /* EXIC1 */
			ret = read_irqcontrol(INTP1, 7);
			break;
		case 0x4E: /* EXIC2 */
			ret = read_irqcontrol(INTP2, 7);
			break;
		case 0x9C: /* TMIC0 */
			ret = read_irqcontrol(INTTU0, m_priority_inttu);
			break;
		case 0x9D: /* TMIC1 */
			ret = read_irqcontrol(INTTU1, 7);
			break;
		case 0x9E: /* TMIC2 */
			ret = read_irqcontrol(INTTU2, 7);
			break;
		case 0xEA: /* FLAG */
			ret = ((m_F0 << 3) | (m_F1 << 5));
			break;
		case 0xEB: /* PRC */
			ret = (m_RAMEN ? 0x40 : 0);
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
			break;
		case 0xEC: /* TBIC */
			ret = read_irqcontrol(INTTB, 7);
			break;
		case 0xEF: /* IRQS */
			ret = m_IRQS;
			break;
		case 0xFC: /* ISPR */
			ret = m_ISPR;
			break;
		case 0xFF: /* IDB */
			ret = (m_IDB >> 12);
			break;
		default:
			logerror("%06x: Read from special function register %02x\n",PC(),o);
			ret = 0;
	}
	return ret;
}

UINT16 v25_common_device::read_sfr_word(unsigned o)
{
	UINT16 ret;

	switch(o)
	{
		case 0x80:  /* TM0 */
			logerror("%06x: Warning: read back TM0\n",PC());
			ret = m_TM0;
			break;
		case 0x82: /* MD0 */
			logerror("%06x: Warning: read back MD0\n",PC());
			ret = m_MD0;
			break;
		case 0x88:  /* TM1 */
			logerror("%06x: Warning: read back TM1\n",PC());
			ret = m_TM1;
			break;
		case 0x8A: /* MD1 */
			logerror("%06x: Warning: read back MD1\n",PC());
			ret = m_MD1;
			break;
		default:
			ret = (read_sfr(o) | (read_sfr(o+1) << 8));
	}
	return ret;
}

void v25_common_device::write_irqcontrol(int /*INTSOURCES*/ source, UINT8 d)
{
	if(d & 0x80)
		m_pending_irq |= source;
	else
		m_pending_irq &= ~source;

	if(d & 0x40)
		m_unmasked_irq &= ~source;
	else
		m_unmasked_irq |= source;

	if(d & 0x20)
		logerror("%06x: Warning: macro service function not implemented\n",PC());

	if(d & 0x10)
		m_bankswitch_irq |= source;
	else
		m_bankswitch_irq &= ~source;
}

void v25_common_device::write_sfr(unsigned o, UINT8 d)
{
	int tmp;
	attotime time;

	static const int timebases[4] = { 10, 13, 16, 20 };
	static const int clocks[4] = { 2, 4, 8, 0 };

	switch(o)
	{
		case 0x00: /* P0 */
			m_io->write_byte(V25_PORT_P0, d);
			break;
		case 0x08: /* P1 */
			/* only the upper four bits of P1 can be used as output */
			m_io->write_byte(V25_PORT_P1, d & 0xF0);
			break;
		case 0x10: /* P2 */
			m_io->write_byte(V25_PORT_P2, d);
			break;
		case 0x4C: /* EXIC0 */
			write_irqcontrol(INTP0, d);
			m_priority_intp = d & 0x7;
			break;
		case 0x4D: /* EXIC1 */
			write_irqcontrol(INTP1, d);
			break;
		case 0x4E: /* EXIC2 */
			write_irqcontrol(INTP2, d);
			break;
		case 0x90: /* TMC0 */
			m_TMC0 = d;
			if(d & 1)   /* oneshot mode */
			{
				if(d & 0x80)
				{
					tmp = m_PCK * m_TM0 * ((d & 0x40) ? 128 : 12 );
					time = attotime::from_hz(unscaled_clock()) * tmp;
					m_timers[0]->adjust(time, INTTU0);
				}
				else
					m_timers[0]->adjust(attotime::never);

				if(d & 0x20)
				{
					tmp = m_PCK * m_MD0 * ((d & 0x10) ? 128 : 12 );
					time = attotime::from_hz(unscaled_clock()) * tmp;
					m_timers[1]->adjust(time, INTTU1);
				}
				else
					m_timers[1]->adjust(attotime::never);
			}
			else    /* interval mode */
			{
				if(d & 0x80)
				{
					tmp = m_PCK * m_MD0 * ((d & 0x40) ? 128 : 6 );
					time = attotime::from_hz(unscaled_clock()) * tmp;
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
			break;
		case 0x91: /* TMC1 */
			m_TMC1 = d & 0xC0;
			if(d & 0x80)
			{
				tmp = m_PCK * m_MD1 * ((d & 0x40) ? 128 : 6 );
				time = attotime::from_hz(unscaled_clock()) * tmp;
				m_timers[2]->adjust(time, INTTU2, time);
				m_TM1 = m_MD1;
			}
			else
				m_timers[2]->adjust(attotime::never);
			break;
		case 0x9C: /* TMIC0 */
			write_irqcontrol(INTTU0, d);
			m_priority_inttu = d & 0x7;
			break;
		case 0x9D: /* TMIC1 */
			write_irqcontrol(INTTU1, d);
			break;
		case 0x9E: /* TMIC2 */
			write_irqcontrol(INTTU2, d);
			break;
		case 0xEA: /* FLAG */
			m_F0 = ((d & 0x08) == 0x08);
			m_F1 = ((d & 0x20) == 0x20);
			break;
		case 0xEB: /* PRC */
			logerror("%06x: PRC set to %02x\n", PC(), d);
			m_RAMEN = ((d & 0x40) == 0x40);
			m_TB = timebases[(d & 0x0C) >> 2];
			m_PCK = clocks[d & 0x03];
			if (m_PCK == 0)
			{
				logerror("        Warning: invalid clock divider\n");
				m_PCK = 8;
			}
			tmp = m_PCK << m_TB;
			time = attotime::from_hz(unscaled_clock()) * tmp;
			m_timers[3]->adjust(time, INTTB, time);
			notify_clock_changed(); /* make device_execute_interface pick up the new clocks_to_cycles() */
			logerror("        Internal RAM %sabled\n", (m_RAMEN ? "en" : "dis"));
			logerror("        Time base set to 2^%d\n", m_TB);
			logerror("        Clock divider set to %d\n", m_PCK);
			break;
		case 0xEC: /* TBIC */
			/* time base interrupt doesn't support macro service, bank switching or priority control */
			write_irqcontrol(INTTB, d & 0xC0);
			break;
		case 0xFF: /* IDB */
			m_IDB = (d << 12) | 0xE00;
			logerror("%06x: IDB set to %02x\n",PC(),d);
			break;
		default:
			logerror("%06x: Wrote %02x to special function register %02x\n",PC(),d,o);
	}
}

void v25_common_device::write_sfr_word(unsigned o, UINT16 d)
{
	switch(o)
	{
		case 0x80:  /* TM0 */
			m_TM0 = d;
			break;
		case 0x82: /* MD0 */
			m_MD0 = d;
			break;
		case 0x88:  /* TM1 */
			m_TM1 = d;
			break;
		case 0x8A: /* MD1 */
			m_MD1 = d;
			break;
		default:
			write_sfr(o, d);
			write_sfr(o+1, d >> 8);
	}
}

UINT8 v25_common_device::v25_read_byte(unsigned a)
{
	if((a & 0xFFE00) == m_IDB || a == 0xFFFFF)
	{
		unsigned o = a & 0x1FF;

		if(m_RAMEN && o < 0x100)
			return m_ram.b[BYTE_XOR_LE(o)];

		if(o >= 0x100)
			return read_sfr(o-0x100);
	}

	return m_program->read_byte(a);
}

UINT16 v25_common_device::v25_read_word(unsigned a)
{
	if( a & 1 )
		return (v25_read_byte(a) | (v25_read_byte(a + 1) << 8));

	if((a & 0xFFE00) == m_IDB)
	{
		unsigned o = a & 0x1FF;

		if(m_RAMEN && o < 0x100)
			return m_ram.w[o/2];

		if(o >= 0x100)
			return read_sfr_word(o-0x100);
	}

	if(a == 0xFFFFE)    /* not sure about this - manual says FFFFC-FFFFE are "reserved" */
		return (m_program->read_byte(a) | (read_sfr(0xFF) << 8));

	return m_program->read_word(a);
}

void v25_common_device::v25_write_byte(unsigned a, UINT8 d)
{
	if((a & 0xFFE00) == m_IDB || a == 0xFFFFF)
	{
		unsigned o = a & 0x1FF;

		if(m_RAMEN && o < 0x100)
		{
			m_ram.b[BYTE_XOR_LE(o)] = d;
			return;
		}

		if(o >= 0x100)
		{
			write_sfr(o-0x100, d);
			return;
		}
	}

	m_program->write_byte(a, d);
}

void v25_common_device::v25_write_word(unsigned a, UINT16 d)
{
	if( a & 1 )
	{
		v25_write_byte(a, d);
		v25_write_byte(a + 1, d >> 8);
		return;
	}

	if((a & 0xFFE00) == m_IDB)
	{
		unsigned o = a & 0x1FF;

		if(m_RAMEN && o < 0x100)
		{
			m_ram.w[o/2] = d;
			return;
		}

		if(o >= 0x100)
		{
			write_sfr_word(o-0x100, d);
			return;
		}
	}

	if(a == 0xFFFFE)    /* not sure about this - manual says FFFFC-FFFFE are "reserved" */
	{
		m_program->write_byte(a, d);
		write_sfr(0xFF, d >> 8);
		return;
	}

	m_program->write_word(a, d);
}
