// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * nsc810.c
 *
 *  Created on: 10/03/2014
 *
 *  TODO:
 *    - 128 byte RAM
 *    - other timer modes (only mode 1 - event counter - is implemented currently)
 *    - port bit set/clear
 *    - and lots of other stuff
 */

#include "nsc810.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

#define LOG (1)

const device_type NSC810 = &device_creator<nsc810_device>;

nsc810_device::nsc810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NSC810, "National Semiconductor NSC810", tag, owner, clock, "nsc810", __FILE__),
	m_portA_r(*this),
	m_portB_r(*this),
	m_portC_r(*this),
	m_portA_w(*this),
	m_portB_w(*this),
	m_portC_w(*this),
	m_timer0_out(*this),
	m_timer1_out(*this)
{
}

void nsc810_device::device_start()
{
	m_portA_r.resolve_safe(0);
	m_portB_r.resolve_safe(0);
	m_portC_r.resolve_safe(0);
	m_portA_w.resolve_safe();
	m_portB_w.resolve_safe();
	m_portC_w.resolve_safe();
	m_timer0_out.resolve_safe();
	m_timer1_out.resolve_safe();

	m_portA_w(0);
	m_portB_w(0);
	m_portC_w(0);
	m_timer0_out(0);
	m_timer1_out(0);

	m_timer0 = timer_alloc(TIMER0_CLOCK);
	m_timer1 = timer_alloc(TIMER1_CLOCK);
}

void nsc810_device::device_reset()
{
	m_portA_latch = 0;
	m_portB_latch = 0;
	m_portC_latch = 0;
	m_ddrA = 0;
	m_ddrB = 0;
	m_ddrC = 0;
	m_mode = 0;
	m_timer0_mode = 0;
	m_timer1_mode = 0;
	m_timer0_counter = 0;
	m_timer1_counter = 0;
	m_timer0_running = false;
	m_timer1_running = false;
	m_ramselect = false;
}

void nsc810_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER0_CLOCK:
		m_timer0_counter--;
		if((m_timer0_mode & 0x07) == 0x01 || (m_timer0_mode & 0x07) == 0x02)
		{
			if(m_timer0_counter == 0)
			{
				m_timer0_out(ASSERT_LINE);
				m_timer0_counter = m_timer0_base;
				if(LOG) logerror("NSC810 '%s': Timer 0 output set\n",tag());
			}
		}
		break;
	case TIMER1_CLOCK:
		m_timer1_counter--;
		if((m_timer1_mode & 0x07) == 0x01 || (m_timer1_mode & 0x07) == 0x02)
		{
			if(m_timer1_counter == 0)
			{
				m_timer1_out(ASSERT_LINE);
				m_timer1_counter = m_timer1_base;
				if(LOG) logerror("NSC810 '%s': Timer 1 output set\n",tag());
			}
		}
		break;
	}
}

READ8_MEMBER(nsc810_device::read)
{
	UINT8 res = 0xff;

	if(m_ramselect)
	{
		// TODO: 128 byte RAM access
	}
	else
	{
		// Register access
		switch(offset & 0x1f)
		{
		case REG_PORTA:
			res = m_portA_latch &= m_ddrA;
			res |= (m_portA_r() & ~m_ddrA);
			//if(LOG) logerror("NSC810 '%s': Port A data read %02x\n",tag(),res);
			break;
		case REG_PORTB:
			res = m_portB_latch &= m_ddrB;
			res |= (m_portB_r() & ~m_ddrB);
			//if(LOG) logerror("NSC810 '%s': Port B data read %02x\n",tag(),res);
			break;
		case REG_PORTC:
			res = m_portC_latch &= m_ddrC;
			res |= (m_portC_r() & ~m_ddrC);
			//if(LOG) logerror("NSC810 '%s': Port C data read %02x\n",tag(),res);
			break;
		case REG_MODE_TIMER0:
			res = m_timer0_mode;
			break;
		case REG_MODE_TIMER1:
			res = m_timer1_mode;
			break;
		case REG_TIMER0_LOW:
			res = m_timer0_counter & 0xff;
			if((m_timer0_mode & 0x07) == 0x01 || (m_timer0_mode & 0x07) == 0x02)
			{
				m_timer0_out(CLEAR_LINE);
				if(LOG) logerror("NSC810 '%s': Timer 0 output reset\n",tag());
			}
			break;
		case REG_TIMER0_HIGH:
			res = m_timer0_counter >> 8;
			if((m_timer0_mode & 0x07) == 0x01 || (m_timer0_mode & 0x07) == 0x02)
			{
				m_timer0_out(CLEAR_LINE);
				if(LOG) logerror("NSC810 '%s': Timer 0 output reset\n",tag());
			}
			break;
		case REG_TIMER1_LOW:
			res = m_timer1_counter & 0xff;
			if((m_timer1_mode & 0x07) == 0x01 || (m_timer1_mode & 0x07) == 0x02)
			{
				m_timer1_out(0);
				if(LOG) logerror("NSC810 '%s': Timer 1 output reset\n",tag());
			}
			break;
		case REG_TIMER1_HIGH:
			res = m_timer1_counter >> 8;
			if((m_timer1_mode & 0x07) == 0x01 || (m_timer1_mode & 0x07) == 0x02)
			{
				m_timer1_out(0);
				if(LOG) logerror("NSC810 '%s': Timer 1 output reset\n",tag());
			}
			break;
		default:
			if(LOG) logerror("NSC810 '%s': unused port %02x read\n",tag(),offset);
		}
	}
	return res;
}

WRITE8_MEMBER(nsc810_device::write)
{
	UINT32 rate;

	if(m_ramselect)
	{
		// TODO: 128 byte RAM access
	}
	else
	{
		// Register access
		switch(offset & 0x1f)
		{
		case REG_PORTA:
			m_portA_latch = data & ~m_ddrA;
			m_portA_w((0xff & ~m_ddrA) | (data & m_ddrA));
			if(LOG) logerror("NSC810 '%s': Port A data write %02x\n",tag(),data);
			break;
		case REG_PORTB:
			m_portB_latch = data & ~m_ddrB;
			m_portB_w((0xff & ~m_ddrB) | (data & m_ddrB));
			if(LOG) logerror("NSC810 '%s': Port B data write %02x\n",tag(),data);
			break;
		case REG_PORTC:
			m_portC_latch = data & ~m_ddrC;
			m_portC_w((0xff & ~m_ddrC) | (data & m_ddrC));
			if(LOG) logerror("NSC810 '%s': Port C data write %02x\n",tag(),data);
			break;
		case REG_DDRA:
			m_ddrA = data;
			if(LOG) logerror("NSC810 '%s': Port A direction write %02x\n",tag(),data);
			break;
		case REG_DDRB:
			m_ddrB = data;
			if(LOG) logerror("NSC810 '%s': Port B direction write %02x\n",tag(),data);
			break;
		case REG_DDRC:
			m_ddrC = data;
			if(LOG) logerror("NSC810 '%s': Port C direction write %02x\n",tag(),data);
			break;
		case REG_MODE_DEF:
			if(LOG) logerror("NSC810 '%s': Mode Definition write %02x\n",tag(),data);
			break;
		case REG_PORTA_BITCLR:
			if(LOG) logerror("NSC810 '%s': Port A bit-clear write %02x\n",tag(),data);
			break;
		case REG_PORTB_BITCLR:
			if(LOG) logerror("NSC810 '%s': Port B bit-clear write %02x\n",tag(),data);
			break;
		case REG_PORTC_BITCLR:
			if(LOG) logerror("NSC810 '%s': Port C bit-clear write %02x\n",tag(),data);
			break;
		case REG_PORTA_BITSET:
			if(LOG) logerror("NSC810 '%s': Port A bit-set write %02x\n",tag(),data);
			break;
		case REG_PORTB_BITSET:
			if(LOG) logerror("NSC810 '%s': Port B bit-set write %02x\n",tag(),data);
			break;
		case REG_PORTC_BITSET:
			if(LOG) logerror("NSC810 '%s': Port C bit-set write %02x\n",tag(),data);
			break;
		case REG_TIMER0_LOW:
			m_timer0_base = (m_timer0_base & 0xff00) | data;
			m_timer0_counter = (m_timer0_counter & 0xff00) | data;
			if(LOG) logerror("NSC810 '%s': Timer 0 low-byte write %02x (base=%04x)\n",tag(),data,m_timer0_base);
			break;
		case REG_TIMER0_HIGH:
			m_timer0_base = (m_timer0_base & 0x00ff) | (data << 8);
			m_timer0_counter = (m_timer0_counter & 0x00ff) | (data << 8);
			if(LOG) logerror("NSC810 '%s': Timer 0 high-byte write %02x (base=%04x)\n",tag(),data,m_timer0_base);
			break;
		case REG_TIMER1_LOW:
			m_timer1_base = (m_timer1_base & 0xff00) | data;
			m_timer1_counter = (m_timer1_counter & 0xff00) | data;
			if(LOG) logerror("NSC810 '%s': Timer 1 low-byte write %02x (base=%04x)\n",tag(),data,m_timer1_base);
			break;
		case REG_TIMER1_HIGH:
			m_timer1_base = (m_timer1_base & 0x00ff) | (data << 8);
			m_timer1_counter = (m_timer1_counter & 0x00ff) | (data << 8);
			if(LOG) logerror("NSC810 '%s': Timer 1 high-byte write %02x (base=%04x)\n",tag(),data,m_timer1_base);
			break;
		case REG_TIMER0_STOP:
			m_timer0_running = false;
			m_timer0->reset();
			if(LOG) logerror("NSC810 '%s': Timer 0 Stop write %02x\n",tag(),data);
			break;
		case REG_TIMER0_START:
			if((m_timer0_mode & 0x07) != 0x00 && (m_timer0_mode & 0x07) != 0x07)
			{
				m_timer0_running = true;
				if(m_timer0_mode & 0x10)
					rate = m_timer0_clock / 64;
				else
					if(m_timer0_mode & 0x08)
						rate = m_timer0_clock / 2;
					else
						rate = m_timer0_clock;
				m_timer0->adjust(attotime::zero,0,attotime::from_hz(rate));
			}
			if(LOG) logerror("NSC810 '%s': Timer 0 Start write %02x\n",tag(),data);
			break;
		case REG_TIMER1_STOP:
			m_timer1_running = false;
			m_timer1->reset();
			if(LOG) logerror("NSC810 '%s': Timer 1 Stop write %02x\n",tag(),data);
			break;
		case REG_TIMER1_START:
			if((m_timer1_mode & 0x07) != 0x00 && (m_timer1_mode & 0x07) != 0x07)
			{
				m_timer1_running = true;
				// no /64 prescaler on timer 1
				if(m_timer0_mode & 0x08)
					rate = m_timer0_clock / 2;
				else
					rate = m_timer0_clock;
				m_timer1->adjust(attotime::zero,0,attotime::from_hz(rate));
			}
			if(LOG) logerror("NSC810 '%s': Timer 1 Start write %02x\n",tag(),data);
			break;
		case REG_MODE_TIMER0:
			m_timer0_mode = data;
			if(LOG) logerror("NSC810 '%s': Timer 0 Mode write %02x\n",tag(),data);
			break;
		case REG_MODE_TIMER1:
			m_timer1_mode = data;
			if(LOG) logerror("NSC810 '%s': Timer 1 Mode write %02x\n",tag(),data);
			break;
		default:
			logerror("NSC810 '%s': Unused register %02x write %02x\n",tag(),offset,data);
		}
	}
}
