// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpc.c  --  Williams WPC ASIC, used on Williams WPC pinball machines
 *
 *  Created on: 7/10/2013
 */

#include "wpc.h"

#define LOG_WPC (0)

const device_type WPCASIC = &device_creator<wpc_device>;

wpc_device::wpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig,WPCASIC,"Williams WPC ASIC",tag,owner,clock, "wpc", __FILE__),
		m_dmd_visiblepage(0),
		m_irq_cb(*this),
		m_firq_cb(*this),
		m_sounddata_r(*this),
		m_sounddata_w(*this),
		m_soundctrl_r(*this),
		m_soundctrl_w(*this),
		m_sounds11_w(*this),
		m_bank_w(*this),
		m_dmdbank_w(*this)
{
}


void wpc_device::device_start()
{
	// resolve callbacks
	m_irq_cb.resolve_safe();
	m_firq_cb.resolve_safe();
	m_sounddata_r.resolve_safe(0);
	m_sounddata_w.resolve_safe();
	m_soundctrl_r.resolve_safe(0);
	m_soundctrl_w.resolve_safe();
	m_sounds11_w.resolve_safe();
	m_bank_w.resolve_safe();
	m_dmdbank_w.resolve_safe();

	m_zc_timer = timer_alloc(TIMER_ZEROCROSS);
	m_zc_timer->adjust(attotime::from_hz(120),0,attotime::from_hz(120));
}

void wpc_device::device_reset()
{
	m_memprotect = 0;
	m_dmd_irqsrc = false;
	m_snd_irqsrc = false;
	m_alpha_pos = 0;
}

void wpc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_ZEROCROSS:
		m_zerocross = true;
		break;
	}
}

READ8_MEMBER(wpc_device::read)
{
	UINT8 ret = 0x00;
	char kbdrow[8];

	switch(offset)
	{
	case DMD_FIRQLINE:
		if(m_dmd_irqsrc)
			ret |= 0x80;
		break;
	case WPC_WATCHDOG:
		if(m_zerocross)
		{
			ret |= 0x80;
			m_irq_count = 0;
		}
		m_zerocross = false;
		break;
	case WPC_SWROWREAD:
		sprintf(kbdrow,":INP%X",m_switch_col);
		ret = ~ioport(kbdrow)->read();
//      for(x=0;x<8;x++)
//      {
//          if(m_switch_col & (1<<x))
//              ret = m_switches[3+x];
//      }
		break;
	case WPC_SWCOINDOOR:
		ret = ~ioport(":COIN")->read();
		//ret = m_switches[0];
		break;
	case WPC_DIPSWITCH:
		ret = ~ioport(":DIPS")->read();
		//ret = m_switches[1];
		break;
	case WPC_FLIPPERS:
		if(ioport(":FLIP"))  // just in case some non-fliptronics games tries to read here...
			ret = ~ioport(":FLIP")->read();
		else
			ret = 0x00;
		break;
	case WPC_SOUNDIF:
		ret = m_sounddata_r(space,0);
		break;
	case WPC_SOUNDBACK:
		ret = m_soundctrl_r(space,0);
		break;
	case WPC_FIRQSRC:
		if(m_snd_irqsrc)
			ret |= 0x80;
		break;
	case WPC_SHIFTADRH:
		ret = m_shift_addr_high + ((m_shift_addr_low + (m_shift_bit1 >> 3)) >> 8);
		break;
	case WPC_SHIFTADRL:
		ret = (m_shift_addr_low + (m_shift_bit1 >> 3)) & 0xff;
		break;
	case WPC_SHIFTBIT:
		ret = 1 << (m_shift_bit1 & 0x07);
		break;
	case WPC_SHIFTBIT2:
		ret = 1 << (m_shift_bit2 & 0x07);
		break;
	default:
		logerror("WPC: Unknown or unimplemented WPC register read from offset %02x\n",offset);
		break;
	}
	return ret;
}

WRITE8_MEMBER(wpc_device::write)
{
	switch(offset)
	{
	case DMD_PAGE3000:
		m_dmdbank_w(space,0,data & 0x0f);
		break;
	case DMD_PAGE3200:
		m_dmdbank_w(space,1,data & 0x0f);
		break;
	case DMD_PAGE3400:
		m_dmdbank_w(space,2,data & 0x0f);
		break;
	case DMD_PAGE3600:
		m_dmdbank_w(space,3,data & 0x0f);
		break;
	case DMD_PAGE3800:
		m_dmdbank_w(space,4,data & 0x0f);
		break;
	case DMD_PAGE3A00:
		m_dmdbank_w(space,5,data & 0x0f);
		break;
	case DMD_FIRQLINE:
		m_firq_cb(space,0);
		m_dmd_irqsrc = false;
		m_dmd_irqline = data;
		if(LOG_WPC) logerror("WPC: DMD FIRQ line set to %i\n",data);
		break;
	case DMD_VISIBLEPAGE:
		m_dmd_visiblepage = data;
		if(LOG_WPC) logerror("WPC: DMD Visible page set to %i\n",data);
		break;
	case WPC_ROMBANK:
		m_bank_w(space,0,data);
		if(LOG_WPC) logerror("WPC: ROM bank set to %02x\n",data);
		break;
	case WPC_ALPHAPOS:
		m_alpha_pos = data & 0x1f;
		break;
	case WPC_ALPHA1LO:
		m_alpha_data[m_alpha_pos] |= data;
		break;
	case WPC_ALPHA1HI:
		m_alpha_data[m_alpha_pos] |= (data << 8);
		break;
	case WPC_ALPHA2LO:
		m_alpha_data[20+m_alpha_pos] |= data;
		break;
	case WPC_ALPHA2HI:
		m_alpha_data[20+m_alpha_pos] |= (data << 8);
		break;
	case WPC_IRQACK:
		m_irq_cb(space,CLEAR_LINE);
		break;
	case WPC_WATCHDOG:
		if(data & 0x80)
		{
			m_irq_count++;
			m_irq_cb(space,CLEAR_LINE);
		}
		break;
	case WPC_SWCOLSELECT:
		m_switch_col = data;
		if(LOG_WPC) logerror("WPC: Switch column select %02x\n",data);
		break;
	case WPC_SOUNDIF:
		m_sounddata_w(space,0,data);
		break;
	case WPC_SOUNDBACK:
		m_soundctrl_w(space,0,data);
		break;
	case WPC_SOUNDS11:
		m_sounds11_w(space,0,data);
		break;
	case WPC_FIRQSRC:
		m_firq_cb(space,0);
		m_snd_irqsrc = false;
		break;
	case WPC_PROTMEMCTRL:
		if(m_memprotect == 0xb4)
		{  // may not be correct
			m_memprotect_mask = (((data & 0x01) << 3) |
								((data & 0x02) << 1) |
								((data & 0x04) >> 1) |
								((data & 0x08) >> 3) |
								(data & 0xf0)) + 0x10;
			m_memprotect_mask <<= 8;
		}
		break;
	case WPC_PROTMEM:
		m_memprotect = data;
		break;
	case WPC_SHIFTADRH:
		m_shift_addr_high = data;
		break;
	case WPC_SHIFTADRL:
		m_shift_addr_low = data;
		break;
	case WPC_SHIFTBIT:
		m_shift_bit1 = data;
		break;
	case WPC_SHIFTBIT2:
		m_shift_bit2 = data;
		break;
	default:
		logerror("WPC: Unknown or unimplemented WPC register write %02x to offset %02x\n",data,offset);
		break;
	}
}
