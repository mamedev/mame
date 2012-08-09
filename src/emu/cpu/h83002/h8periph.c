/***************************************************************************

 h8periph.c: Implementation of Hitachi H8/3002 on-board MCU functions.

 Original by The_Author & DynaChicken for the ZiNc emulator.

 Rewritten for MAME to use MAME timers and other facilities by R. Belmont

****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h8.h"
#include "h8priv.h"

#define H8_REG_START	(0x00ffff10)

// timer registers
#define TSTR	(0x60)
#define TIER0	(0x66)
#define TIER1   (0x70)
#define TIER2   (0x7a)
#define TIER3   (0x84)
#define TIER4   (0x94)
#define TSR0	(0x67)
#define TSR1	(0x71)
#define TSR2	(0x7b)
#define TSR3	(0x85)
#define TSR4	(0x95)
#define TCR0	(0x64)
#define TCR1	(0x6e)
#define TCR2	(0x78)
#define TCR3	(0x82)
#define TCR4	(0x92)

static const UINT8 tsr[5] = { TSR0, TSR1, TSR2, TSR3, TSR4 };
static const UINT8 tier[5] = { TIER0, TIER1, TIER2, TIER3, TIER4 };
static const UINT8 tcr[5] = { TCR0, TCR1, TCR2, TCR3, TCR4 };
static const int tscales[4] = { 1, 2, 4, 8 };

extern void h8_3002_InterruptRequest(h83xx_state *h8, UINT8 source, UINT8 state);

static void h8itu_timer_expire(h83xx_state *h8, int which)
{
	h8->timer[which]->adjust(attotime::never);
	h8->h8TCNT[which] = 0;
	h8->per_regs[tsr[which]] |= 4;
	// interrupt on overflow ?
	if(h8->per_regs[tier[which]] & 4)
	{
		h8_3002_InterruptRequest(h8, 26 + 4*which, 1);
	}
}

static TIMER_CALLBACK( h8itu_timer_0_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_timer_expire(h8, 0);
}

static TIMER_CALLBACK( h8itu_timer_1_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_timer_expire(h8, 1);
}

static TIMER_CALLBACK( h8itu_timer_2_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_timer_expire(h8, 2);
}

static TIMER_CALLBACK( h8itu_timer_3_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_timer_expire(h8, 3);
}

static TIMER_CALLBACK( h8itu_timer_4_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_timer_expire(h8, 4);
}

static void h8_itu_refresh_timer(h83xx_state *h8, int tnum)
{
	int ourTCR = 0;
	int ourTVAL = 0;
	attotime period;

	ourTCR = h8->per_regs[tcr[tnum]];
	ourTVAL = h8->h8TCNT[tnum];

	period = attotime::from_hz(h8->device->unscaled_clock()) * tscales[ourTCR & 3] * (65536 - ourTVAL);

	if (ourTCR & 4)
	{
		logerror("H8/3002: Timer %d is using an external clock.  Unsupported!\n", tnum);
	}

	h8->timer[tnum]->adjust(period);
}

static void h8_itu_sync_timers(h83xx_state *h8, int tnum)
{
	int ourTCR = 0;
	attotime cycle_time, cur;
	UINT16 ratio;

	ourTCR = h8->per_regs[tcr[tnum]];

	// get the time per unit
	cycle_time = attotime::from_hz(h8->device->unscaled_clock()) * tscales[ourTCR & 3];
	cur = h8->timer[tnum]->elapsed();

	ratio = cur.as_double() / cycle_time.as_double();

	h8->h8TCNT[tnum] = ratio;
}

UINT8 h8_itu_read8(h83xx_state *h8, UINT8 reg)
{
	UINT8 val;

	switch(reg)
	{
	case 0x60:
		val = h8->h8TSTR;
		break;
	case 0x68:
		h8_itu_sync_timers(h8, 0);
		val = h8->h8TCNT[0]>>8;
		break;
	case 0x69:
		h8_itu_sync_timers(h8, 0);
		val = h8->h8TCNT[0]&0xff;
		break;
	case 0x72:
		h8_itu_sync_timers(h8, 1);
		val = h8->h8TCNT[1]>>8;
		break;
	case 0x73:
		h8_itu_sync_timers(h8, 1);
		val = h8->h8TCNT[1]&0xff;
		break;
	case 0x7c:
		h8_itu_sync_timers(h8, 2);
		val = h8->h8TCNT[2]>>8;
		break;
	case 0x7d:
		h8_itu_sync_timers(h8, 2);
		val = h8->h8TCNT[2]&0xff;
		break;
	case 0x86:
		h8_itu_sync_timers(h8, 3);
		val = h8->h8TCNT[3]>>8;
		break;
	case 0x87:
		h8_itu_sync_timers(h8, 3);
		val = h8->h8TCNT[3]&0xff;
		break;
	default:
		val = h8->per_regs[reg];
		break;
	}


	return val;
}

void h8_itu_write8(h83xx_state *h8, UINT8 reg, UINT8 val)
{
	h8->per_regs[reg] = val;
	switch(reg)
	{
	case 0x60:
		if ((val & 1) && !(h8->h8TSTR & 1))
		{
			h8_itu_refresh_timer(h8, 0);
		}
		if ((val & 2) && !(h8->h8TSTR & 2))
		{
			h8_itu_refresh_timer(h8, 1);
		}
		if ((val & 4) && !(h8->h8TSTR & 4))
		{
			h8_itu_refresh_timer(h8, 2);
		}
		if ((val & 8) && !(h8->h8TSTR & 8))
		{
			h8_itu_refresh_timer(h8, 3);
		}
		if ((val & 0x10) && !(h8->h8TSTR & 0x10))
		{
			h8_itu_refresh_timer(h8, 4);
		}
		h8->h8TSTR = val;
		break;
	case 0x68:
		h8->h8TCNT[0] = (val<<8) | (h8->h8TCNT[0] & 0xff);
		if (h8->h8TSTR & 1)
		{
			h8_itu_refresh_timer(h8, 0);
		}
		break;
	case 0x69:
		h8->h8TCNT[0] = (val) | (h8->h8TCNT[0] & 0xff00);
		if (h8->h8TSTR & 1)
		{
			h8_itu_refresh_timer(h8, 0);
		}
		break;
	case 0x72:
		h8->h8TCNT[1] = (val<<8) | (h8->h8TCNT[1] & 0xff);
		if (h8->h8TSTR & 2)
		{
			h8_itu_refresh_timer(h8, 1);
		}
		break;
	case 0x73:
		h8->h8TCNT[1] = (val) | (h8->h8TCNT[1] & 0xff00);
		if (h8->h8TSTR & 2)
		{
			h8_itu_refresh_timer(h8, 1);
		}
		break;
	case 0x7c:
		h8->h8TCNT[2] = (val<<8) | (h8->h8TCNT[2] & 0xff);
		if (h8->h8TSTR & 4)
		{
			h8_itu_refresh_timer(h8, 2);
		}
		break;
	case 0x7d:
		h8->h8TCNT[2] = (val) | (h8->h8TCNT[2] & 0xff00);
		if (h8->h8TSTR & 4)
		{
			h8_itu_refresh_timer(h8, 2);
		}
		break;
	case 0x86:
		h8->h8TCNT[3] = (val<<8) | (h8->h8TCNT[3] & 0xff);
		if (h8->h8TSTR & 8)
		{
			h8_itu_refresh_timer(h8, 3);
		}
		break;
	case 0x87:
		h8->h8TCNT[3] = (val) | (h8->h8TCNT[3] & 0xff00);
		if (h8->h8TSTR & 8)
		{
			h8_itu_refresh_timer(h8, 3);
		}
		break;
	case 0x96:
		h8->h8TCNT[4] = (val<<8) | (h8->h8TCNT[4] & 0xff);
		if (h8->h8TSTR & 0x10)
		{
			h8_itu_refresh_timer(h8, 4);
		}
		break;
	case 0x97:
		h8->h8TCNT[4] = (val) | (h8->h8TCNT[4] & 0xff00);
		if (h8->h8TSTR & 0x10)
		{
			h8_itu_refresh_timer(h8, 4);
		}
		break;
	default:
		break;
	}
}

#ifdef UNUSED_FUNCTION
UINT8 h8_debugger_itu_read8(UINT8 reg)
{
	UINT8 val;
	val = 0;
	return val;
}
#endif


static UINT8 h8_ISR_r(h83xx_state *h8)
{
	UINT8 res = 0;

	int i;
	for (i = 0; i < 6; i++)
		if (h8->irq_req[0] & (1 << (12+i)))	res |= (1 << i);

	return res;
}

static void h8_ISR_w(h83xx_state *h8, UINT8 val)
{
	int i;
	for (i = 0; i < 6; i++)
		if ((~val) & (1 << i))	h8->irq_req[0] &= ~(1 << (12+i));
}


UINT8 h8_register_read8(h83xx_state *h8, UINT32 address)
{
	UINT8 val;
	UINT8 reg;

	address &= 0xffffff;

	reg = address & 0xff;

	if(reg >= 0x60 && reg <= 0x9f)
	{
		return h8_itu_read8(h8, reg);
	}
	else
	{
		switch(reg)
		{
		case 0xb4: // serial port A status
			val = h8->per_regs[reg];
			val |= 0xc4;		// transmit finished, receive ready, no errors
			break;
		case 0xb5: // serial port A receive
			val = h8->io->read_byte(H8_SERIAL_0);
			break;
		case 0xbc: // serial port B status
			val = h8->per_regs[reg];
			val |= 0xc4;		// transmit finished, receive ready, no errors
			break;
		case 0xbd: // serial port B receive
			val = h8->io->read_byte(H8_SERIAL_1);
			break;
		case 0xe0:
			val = h8->io->read_byte(H8_ADC_0_H);
			break;
		case 0xe1:
			val = h8->io->read_byte(H8_ADC_0_L);
			break;
		case 0xe2:
			val = h8->io->read_byte(H8_ADC_1_H);
			break;
		case 0xe3:
			val = h8->io->read_byte(H8_ADC_1_L);
			break;
		case 0xe4:
			val = h8->io->read_byte(H8_ADC_2_H);
			break;
		case 0xe5:
			val = h8->io->read_byte(H8_ADC_2_L);
			break;
		case 0xe6:
			val = h8->io->read_byte(H8_ADC_3_H);
			break;
		case 0xe7:
			val = h8->io->read_byte(H8_ADC_3_L);
			break;
		case 0xe8:		// adc status
			val = 0x80;
			break;
		case 0xc7:  		// port 4 data
			val = h8->io->read_byte(H8_PORT_4);
			break;
		case 0xcb:  		// port 6 data
			val = h8->io->read_byte(H8_PORT_6);
			break;
		case 0xce:		// port 7 data
			val = h8->io->read_byte(H8_PORT_7);
			break;
		case 0xcf:		// port 8 data
			val = h8->io->read_byte(H8_PORT_8);
			break;
		case 0xd2:		// port 9 data
			val = h8->io->read_byte(H8_PORT_9);
			break;
		case 0xd3:		// port a data
			val = h8->io->read_byte(H8_PORT_A);
			break;
		case 0xd6:		// port b data
			val = h8->io->read_byte(H8_PORT_B);
			break;
		case 0xf6:
			val = h8_ISR_r(h8);
			break;

		default:
			val = h8->per_regs[reg];
			break;
		}
	}

	return val;
}

void h8_register_write8(h83xx_state *h8, UINT32 address, UINT8 val)
{
	UINT8 reg;

	address &= 0xffffff;

	reg = address & 0xff;

	if(reg >= 0x60 && reg <= 0x9f)
	{
		h8_itu_write8(h8, reg, val);
	}

	switch (reg)
	{
		case 0xb3:	// serial 0 send
			h8->io->write_byte(H8_SERIAL_0, val);
			h8_3002_InterruptRequest(h8, 54, 1);
			h8_3002_InterruptRequest(h8, 55, 1);
			break;
		case 0xbb:	// serial 1 send
			h8->io->write_byte(H8_SERIAL_1, val);
			h8_3002_InterruptRequest(h8, 58, 1);
			h8_3002_InterruptRequest(h8, 59, 1);
			break;
		case 0xc7:
			h8->io->write_byte(H8_PORT_4, val);
			break;
		case 0xcb:  		// port 6 data
			h8->io->write_byte(H8_PORT_6, val);
			break;
		case 0xce:		// port 7 data
			h8->io->write_byte(H8_PORT_7, val);
			break;
		case 0xcf:		// port 8 data
			h8->io->write_byte(H8_PORT_8, val);
			break;
		case 0xd2:		// port 9 data
			h8->io->write_byte(H8_PORT_9, val);
			break;
		case 0xd3:		// port a data
			h8->io->write_byte(H8_PORT_A, val);
			break;
		case 0xd6:		// port b data
			h8->io->write_byte(H8_PORT_B, val);
			break;
		case 0xf6:
			h8_ISR_w(h8, val);
			break;
	}

	h8->per_regs[reg] = val;
}

static void h8_3007_itu_refresh_timer(h83xx_state *h8, int tnum)
{
	attotime period;
	int ourTCR = h8->per_regs[0x68+(tnum*8)];

	period = attotime::from_hz(h8->device->unscaled_clock()) * tscales[ourTCR & 3];

	if (ourTCR & 4)
	{
		logerror("H8/3007: Timer %d is using an external clock.  Unsupported!\n", tnum);
	}

	h8->timer[tnum]->adjust(period);
}

static void h8itu_3007_timer_expire(h83xx_state *h8, int tnum)
{
	int base = 0x68 + (tnum*8);
	UINT16 count;

	count = (h8->per_regs[base + 0x2]<<8) | h8->per_regs[base + 0x3];
	count++;

	//logerror("h8/3007 timer %d count = %04x\n",tnum,count);

	// GRA match
	if ((h8->per_regs[base + 0x1] & 0x03) && (count == ((h8->per_regs[base + 0x4]<<8) | h8->per_regs[base + 0x5])))
	{
		if ((h8->per_regs[base + 0x0] & 0x60) == 0x20)
		{
			//logerror("h8/3007 timer %d GRA match, restarting\n",tnum);
			count = 0;
			h8_3007_itu_refresh_timer(h8, tnum);
		}
		else
		{
			//logerror("h8/3007 timer %d GRA match, stopping\n",tnum);
			h8->timer[tnum]->adjust(attotime::never);
		}

		h8->per_regs[0x64] |= 1<<tnum;
		if(h8->per_regs[0x64] & (4<<tnum))	// interrupt enable
		{
			//logerror("h8/3007 timer %d GRA INTERRUPT\n",tnum);
			h8_3002_InterruptRequest(h8, 24+tnum*4, 1);
		}
	}
	// GRB match
	if ((h8->per_regs[base + 0x1] & 0x30) && (count == ((h8->per_regs[base + 0x6]<<8) | h8->per_regs[base + 0x7])))
	{
		if ((h8->per_regs[base + 0x0] & 0x60) == 0x40)
		{
			//logerror("h8/3007 timer %d GRB match, restarting\n",tnum);
			count = 0;
			h8_3007_itu_refresh_timer(h8, tnum);
		}
		else
		{
			//logerror("h8/3007 timer %d GRB match, stopping\n",tnum);
			h8->timer[tnum]->adjust(attotime::never);
		}

		h8->per_regs[0x65] |= 1<<tnum;
		if(h8->per_regs[0x65] & (4<<tnum))	// interrupt enable
		{
			//logerror("h8/3007 timer %d GRB INTERRUPT\n",tnum);
			h8_3002_InterruptRequest(h8, 25+tnum*4, 1);
		}
	}
	// Overflow
	if (((h8->per_regs[base + 0x1] & 0x33) == 0) && (count == 0))
	{
		//logerror("h8/3007 timer %d OVF match, restarting\n",tnum);
		h8->per_regs[0x66] |= 1<<tnum;
		if(h8->per_regs[0x66] & (4<<tnum))	// interrupt enable
		{
			//logerror("h8/3007 timer %d OVF INTERRUPT\n",tnum);
			h8_3002_InterruptRequest(h8, 26+tnum*4, 1);
		}
	}

	h8->per_regs[base + 0x2] = count >> 8;
	h8->per_regs[base + 0x3] = count & 0xff;
}

static TIMER_CALLBACK( h8itu_3007_timer_0_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_3007_timer_expire(h8, 0);
}

static TIMER_CALLBACK( h8itu_3007_timer_1_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_3007_timer_expire(h8, 1);
}

static TIMER_CALLBACK( h8itu_3007_timer_2_cb )
{
	h83xx_state *h8 = (h83xx_state *)ptr;

	h8itu_3007_timer_expire(h8, 2);
}

UINT8 h8_3007_itu_read8(h83xx_state *h8, UINT8 reg)
{
	UINT8 val;

	switch(reg)
	{
	case 0x60:
		val = h8->h8TSTR | 0xf8;
		break;
	default:
		val = h8->per_regs[reg];
		break;
	}

	return val;
}

void h8_3007_itu_write8(h83xx_state *h8, UINT8 reg, UINT8 val)
{
	logerror("%06x: h8/3007 reg %02x = %02x\n",h8->pc,reg,val);
	h8->per_regs[reg] = val;
	switch(reg)
	{
	case 0x60:
		if ((val & 1) && !(h8->h8TSTR & 1))
		{
			h8_3007_itu_refresh_timer(h8, 0);
		}
		if ((val & 2) && !(h8->h8TSTR & 2))
		{
			h8_3007_itu_refresh_timer(h8, 1);
		}
		if ((val & 4) && !(h8->h8TSTR & 4))
		{
			h8_3007_itu_refresh_timer(h8, 2);
		}
		h8->h8TSTR = val;
		break;
	default:
		break;
	}
}

UINT8 h8_3007_register_read8(h83xx_state *h8, UINT32 address)
{
	UINT8 val;
	UINT8 reg;

	address &= 0xffffff;

	reg = address & 0xff;

	if(reg >= 0x60 && reg <= 0x7f)
	{
		return h8_3007_itu_read8(h8, reg);
	}
	else
	{
		switch(reg)
		{
		case 0xb4: // serial port A status
			val = h8->per_regs[reg];
			val |= 0xc4;		// transmit finished, receive ready, no errors
			break;
		case 0xb5: // serial port A receive
			val = h8->io->read_byte(H8_SERIAL_0);
			break;
		case 0xbc: // serial port B status
			val = h8->per_regs[reg];
			val |= 0xc4;		// transmit finished, receive ready, no errors
			break;
		case 0xbd: // serial port B receive
			val = h8->io->read_byte(H8_SERIAL_1);
			break;
		case 0xe0:
			val = h8->io->read_byte(H8_ADC_0_H);
			break;
		case 0xe1:
			val = h8->io->read_byte(H8_ADC_0_L);
			break;
		case 0xe2:
			val = h8->io->read_byte(H8_ADC_1_H);
			break;
		case 0xe3:
			val = h8->io->read_byte(H8_ADC_1_L);
			break;
		case 0xe4:
			val = h8->io->read_byte(H8_ADC_2_H);
			break;
		case 0xe5:
			val = h8->io->read_byte(H8_ADC_2_L);
			break;
		case 0xe6:
			val = h8->io->read_byte(H8_ADC_3_H);
			break;
		case 0xe7:
			val = h8->io->read_byte(H8_ADC_3_L);
			break;
		case 0xe8:		// adc status
			val = 0x80;
			break;

		case 0xd3:  		// port 4 data
			val = h8->io->read_byte(H8_PORT_4);
			break;
		case 0xd5:  		// port 6 data
			val = h8->io->read_byte(H8_PORT_6);
			break;
		case 0xd6:		// port 7 data
			val = h8->io->read_byte(H8_PORT_7);
			break;
		case 0xd7:		// port 8 data
			val = h8->io->read_byte(H8_PORT_8);
			break;
		case 0xd8:		// port 9 data
			val = h8->io->read_byte(H8_PORT_9);
			break;
		case 0xd9:		// port a data
			val = h8->io->read_byte(H8_PORT_A);
			break;
		case 0xda:		// port b data
			val = h8->io->read_byte(H8_PORT_B);
			break;
		default:
			val = h8->per_regs[reg];
			break;
		}
	}

	return val;
}

void h8_3007_register_write8(h83xx_state *h8, UINT32 address, UINT8 val)
{
	UINT8 reg;

	address &= 0xffffff;

	reg = address & 0xff;

	h8->per_regs[reg] = val;

	if(reg >= 0x60 && reg <= 0x7f)
	{
		h8_3007_itu_write8(h8, reg, val);
	}
	else
	{
		switch (reg)
		{
			case 0xb3:
				h8->io->write_byte(H8_SERIAL_0, val);
				break;
			case 0xbb:
				h8->io->write_byte(H8_SERIAL_1, val);
				break;
			case 0xd3:
				h8->io->write_byte(H8_PORT_4, val);
				break;
			case 0xd5:		// port 6 data
				h8->io->write_byte(H8_PORT_6, val);
				break;
			case 0xd6:		// port 7 data
				h8->io->write_byte(H8_PORT_7, val);
				break;
			case 0xd7:		// port 8 data
				h8->io->write_byte(H8_PORT_8, val);
				break;
			case 0xd8:		// port 9 data
				h8->io->write_byte(H8_PORT_9, val);
				break;
			case 0xd9:		// port a data
				h8->io->write_byte(H8_PORT_A, val);
				break;
			case 0xda:		// port b data
				h8->io->write_byte(H8_PORT_B, val);
				break;
		}
	}
}

UINT8 h8_3007_register1_read8(h83xx_state *h8, UINT32 address)
{
	switch (address)
	{
		case 0xfee012:	return h8->per_regs[0xF2];	// SYSCR
		case 0xfee016:	return h8_ISR_r(h8);		// ISR
		case 0xfee018:	return h8->per_regs[0xF8];	// IPRA
	}

	logerror("cpu '%s' (PC=%08X): unmapped I/O(1) byte read from %08X\n",h8->device->tag(),h8->pc,address);
	return 0;
}

void h8_3007_register1_write8(h83xx_state *h8, UINT32 address, UINT8 val)
{
	switch (address)
	{
		case 0xfee012:	h8->per_regs[0xF2] = val;	return;	// SYSCR
		case 0xfee016:	h8_ISR_w(h8, val);		return;	// ISR
		case 0xfee018:	h8->per_regs[0xF8] = val;	return;	// IPRA
	}
	logerror("cpu '%s' (PC=%08X): unmapped I/O(1) byte write to %08X = %02X\n",h8->device->tag(),h8->pc,address,val);
}

void h8_3007_itu_init(h83xx_state *h8)
{
	h8->timer[0] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_3007_timer_0_cb), h8);
	h8->timer[1] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_3007_timer_1_cb), h8);
	h8->timer[2] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_3007_timer_2_cb), h8);

	h8_itu_reset(h8);
}

void h8_itu_init(h83xx_state *h8)
{
	h8->timer[0] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_timer_0_cb), h8);
	h8->timer[1] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_timer_1_cb), h8);
	h8->timer[2] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_timer_2_cb), h8);
	h8->timer[3] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_timer_3_cb), h8);
	h8->timer[4] = h8->device->machine().scheduler().timer_alloc(FUNC(h8itu_timer_4_cb), h8);

	h8_itu_reset(h8);
}

void h8_itu_reset(h83xx_state *h8)
{
	int i;

	// stop all the timers
	for (i=0; i<5; i++)
		h8->timer[i]->adjust(attotime::never);
}
