/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    MAME conversion, timers, and IRQ controller by R. Belmont

*/

#include "emu.h"
#include "debugger.h"
#include "mn10200.h"

#define log_write(...)
#define log_event(...)

#define MEM_BYTE (0)
#define MEM_WORD (1)


const device_type MN10200 = &device_creator<mn10200_device>;


mn10200_device::mn10200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MN10200, "MN10200", tag, owner, clock, "mn10200", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_io_config("data", ENDIANNESS_LITTLE, 8, 8, 0)
{
}


UINT8 mn10200_device::mn102_read_byte(UINT32 address)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		return mn10200_r(address-0xfc00, MEM_BYTE);
	}

	return m_program->read_byte(address);
}

UINT16 mn10200_device::mn102_read_word(UINT32 address)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		return mn10200_r(address-0xfc00, MEM_WORD);
	}

	if (address & 1)
	{
		return m_program->read_byte(address) | (m_program->read_byte(address+1)<<8);
	}

	return m_program->read_word(address);
}

void mn10200_device::mn102_write_byte(UINT32 address, UINT8 data)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		mn10200_w(address-0xfc00, data, MEM_BYTE);
		return;
	}

	m_program->write_byte(address, data);
}

void mn10200_device::mn102_write_word(UINT32 address, UINT16 data)
{
	if (address >= 0xfc00 && address < 0x10000)
	{
		mn10200_w(address-0xfc00, data, MEM_WORD);
		return;
	}

	if (address & 1)
	{
		m_program->write_byte(address, data&0xff);
		m_program->write_byte(address+1, (data>>8)&0xff);
		return;
	}

	m_program->write_word(address, data);
}

INT32 mn10200_device::r24u(offs_t adr)
{
	return mn102_read_word(adr)|(mn102_read_byte(adr+2)<<16);
}

void mn10200_device::w24(offs_t adr, UINT32 val)
{
/*  if(adr == 0x4075aa || adr == 0x40689a || adr == 0x4075a2) {
        log_write("TRACE", adr, val, MEM_LONG);
    }*/
	mn102_write_byte(adr, val);
	mn102_write_byte(adr+1, val>>8);
	mn102_write_byte(adr+2, val>>16);
}

void mn10200_device::mn102_change_pc(UINT32 pc)
{
		m_pc = pc & 0xffffff;
}

void mn10200_device::mn102_take_irq(int level, int group)
{
	if(!(m_psw & 0x800))
	{
//      if (group != 8) printf("MN10200: Dropping irq L %d G %d pc=%x, a3=%x\n", level, group, m_pc, m_a[3]);
		return;
	}

//  if (group != 8) printf("MN10200: Taking irq L %d G %d pc=%x, a3=%x\n", level, group, m_pc, m_a[3]);

	m_a[3] -= 6;
	w24(m_a[3]+2, m_pc);
	mn102_write_word(m_a[3], m_psw);
	mn102_change_pc(0x80008);
	m_psw = (m_psw & 0xf0ff) | (level << 8);
	m_iagr = group << 1;
}

void mn10200_device::refresh_timer(int tmr)
{
	// enabled?
	if (m_simple_timer[tmr].mode & 0x80)
	{
		UINT8 source = (m_simple_timer[tmr].mode & 3);

		// source is a prescaler?
		if (source >= 2)
		{
			INT32 rate;

			// is prescaler enabled?
			if (m_prescaler[source-2].mode & 0x80)
			{
				// rate = (sysclock / prescaler) / our count
				rate = unscaled_clock() / m_prescaler[source-2].cycles;
				rate /= m_simple_timer[tmr].base;

				if (tmr != 8)   // HACK: timer 8 is run at 500 kHz by the Taito program for no obvious reason, which kills performance
					m_timer_timers[tmr]->adjust(attotime::from_hz(rate), tmr);
			}
			else
			{
				logerror("MN10200: timer %d using prescaler %d which isn't enabled!\n", tmr, source-2);
			}
		}
	}
	else    // disabled, so stop it
	{
		m_timer_timers[tmr]->adjust(attotime::never, tmr);
	}
}

void mn10200_device::timer_tick_simple(int tmr)
{
	m_simple_timer[tmr].cur--;

	// did we expire?
	if (m_simple_timer[tmr].cur == 0)
	{
		int group, irq_in_grp, level;

		// does timer auto-reload?  apparently.
		m_simple_timer[tmr].cur = m_simple_timer[tmr].base;

		// signal the cascade if we're not timer 9
		if (tmr < (MN10200_NUM_TIMERS_8BIT-1))
		{
			// is next timer enabled?
			if (m_simple_timer[tmr+1].mode & 0x80)
			{
				// is it's source "cascade"?
				if ((m_simple_timer[tmr+1].mode & 0x3) == 1)
				{
					// recurse!
					timer_tick_simple(tmr+1);
				}
			}
		}

		// interrupt from this timer if possible
		group = (tmr / 4);
		irq_in_grp = (tmr % 4);
		level = (m_icrh[group]>>4) & 0x7;

		// indicate interrupt pending
		m_icrl[group] |= (1 << (4 + irq_in_grp));

		// interrupt detect = pending AND enable
		m_icrl[group] |= (m_icrh[group]&0x0f) & (m_icrl[group]>>4);

		// is the result enabled?
		if (m_icrl[group] & (1 << irq_in_grp))
		{
//          printf("Timer %d IRQ! (Group %d in_grp %d ICRH %x ICRL %x\n", tmr, group, irq_in_grp, m_icrh[group], m_icrl[group]);
			// try to take it now
			mn102_take_irq(level, group + 1);
		}
	}
}

TIMER_CALLBACK_MEMBER( mn10200_device::simple_timer_cb )
{
	int tmr = param;

	// handle our expiring and also tick our cascaded children
	m_simple_timer[tmr].cur = 1;
	timer_tick_simple(tmr);

	// refresh this timer
	refresh_timer(tmr);
}

void mn10200_device::device_start()
{
	int tmr;

	m_pc = 0;
	m_d[0] = m_d[1] = m_d[2] = m_d[3] = 0;
	m_a[0] = m_a[1] = m_d[2] = m_d[3] = 0;
	m_nmicr = 0;
	m_iagr = 0;
	for( int i = 0; i < MN10200_NUM_IRQ_GROUPS; i++ )
	{
		m_icrl[i] = 0;
		m_icrh[i] = 0;
	}
	m_psw = 0;
	m_mdr = 0;
	for ( int i = 0; i < MN10200_NUM_TIMERS_8BIT; i++ )
	{
		m_simple_timer[i].mode = 0;
		m_simple_timer[i].base = 0;
		m_simple_timer[i].cur = 0;
	}
	for ( int i = 0; i < MN10200_NUM_PRESCALERS; i++ )
	{
		m_prescaler[i].cycles = 0;
		m_prescaler[i].mode = 0;
	}
	for ( int i = 0; i < 8; i++ )
	{
		m_dma[i].adr = 0;
		m_dma[i].count = 0;
		m_dma[i].iadr = 0;
		m_dma[i].ctrll = 0;
		m_dma[i].ctrlh = 0;
		m_dma[i].irq = 0;
	}
	for ( int i = 0; i < 2; i++ )
	{
		m_serial[i].ctrll = 0;
		m_serial[i].ctrlh = 0;
		m_serial[i].buf = 0;
	}
	for ( int i = 0; i < 8; i++ )
	{
		m_ddr[i] = 0;
	}

	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	save_item(NAME(m_pc));
	save_item(NAME(m_d));
	save_item(NAME(m_a));
	save_item(NAME(m_nmicr));
	save_item(NAME(m_iagr));
	save_item(NAME(m_icrl));
	save_item(NAME(m_icrh));
	save_item(NAME(m_psw));
	save_item(NAME(m_mdr));
	save_item(NAME(m_simple_timer[0].mode));
	save_item(NAME(m_simple_timer[0].base));
	save_item(NAME(m_simple_timer[0].cur));
	save_item(NAME(m_simple_timer[1].mode));
	save_item(NAME(m_simple_timer[1].base));
	save_item(NAME(m_simple_timer[1].cur));
	save_item(NAME(m_simple_timer[2].mode));
	save_item(NAME(m_simple_timer[2].base));
	save_item(NAME(m_simple_timer[2].cur));
	save_item(NAME(m_simple_timer[3].mode));
	save_item(NAME(m_simple_timer[3].base));
	save_item(NAME(m_simple_timer[3].cur));
	save_item(NAME(m_simple_timer[4].mode));
	save_item(NAME(m_simple_timer[4].base));
	save_item(NAME(m_simple_timer[4].cur));
	save_item(NAME(m_simple_timer[5].mode));
	save_item(NAME(m_simple_timer[5].base));
	save_item(NAME(m_simple_timer[5].cur));
	save_item(NAME(m_simple_timer[6].mode));
	save_item(NAME(m_simple_timer[6].base));
	save_item(NAME(m_simple_timer[6].cur));
	save_item(NAME(m_simple_timer[7].mode));
	save_item(NAME(m_simple_timer[7].base));
	save_item(NAME(m_simple_timer[7].cur));
	save_item(NAME(m_simple_timer[8].mode));
	save_item(NAME(m_simple_timer[8].base));
	save_item(NAME(m_simple_timer[8].cur));
	save_item(NAME(m_simple_timer[9].mode));
	save_item(NAME(m_simple_timer[9].base));
	save_item(NAME(m_simple_timer[9].cur));
	save_item(NAME(m_prescaler[0].cycles));
	save_item(NAME(m_prescaler[0].mode));
	save_item(NAME(m_prescaler[1].cycles));
	save_item(NAME(m_prescaler[1].mode));
	save_item(NAME(m_dma[0].adr));
	save_item(NAME(m_dma[0].count));
	save_item(NAME(m_dma[0].iadr));
	save_item(NAME(m_dma[0].ctrll));
	save_item(NAME(m_dma[0].ctrlh));
	save_item(NAME(m_dma[0].irq));
	save_item(NAME(m_dma[1].adr));
	save_item(NAME(m_dma[1].count));
	save_item(NAME(m_dma[1].iadr));
	save_item(NAME(m_dma[1].ctrll));
	save_item(NAME(m_dma[1].ctrlh));
	save_item(NAME(m_dma[1].irq));
	save_item(NAME(m_dma[2].adr));
	save_item(NAME(m_dma[2].count));
	save_item(NAME(m_dma[2].iadr));
	save_item(NAME(m_dma[2].ctrll));
	save_item(NAME(m_dma[2].ctrlh));
	save_item(NAME(m_dma[2].irq));
	save_item(NAME(m_dma[3].adr));
	save_item(NAME(m_dma[3].count));
	save_item(NAME(m_dma[3].iadr));
	save_item(NAME(m_dma[3].ctrll));
	save_item(NAME(m_dma[3].ctrlh));
	save_item(NAME(m_dma[3].irq));
	save_item(NAME(m_dma[4].adr));
	save_item(NAME(m_dma[4].count));
	save_item(NAME(m_dma[4].iadr));
	save_item(NAME(m_dma[4].ctrll));
	save_item(NAME(m_dma[4].ctrlh));
	save_item(NAME(m_dma[4].irq));
	save_item(NAME(m_dma[5].adr));
	save_item(NAME(m_dma[5].count));
	save_item(NAME(m_dma[5].iadr));
	save_item(NAME(m_dma[5].ctrll));
	save_item(NAME(m_dma[5].ctrlh));
	save_item(NAME(m_dma[5].irq));
	save_item(NAME(m_dma[6].adr));
	save_item(NAME(m_dma[6].count));
	save_item(NAME(m_dma[6].iadr));
	save_item(NAME(m_dma[6].ctrll));
	save_item(NAME(m_dma[6].ctrlh));
	save_item(NAME(m_dma[6].irq));
	save_item(NAME(m_dma[7].adr));
	save_item(NAME(m_dma[7].count));
	save_item(NAME(m_dma[7].iadr));
	save_item(NAME(m_dma[7].ctrll));
	save_item(NAME(m_dma[7].ctrlh));
	save_item(NAME(m_dma[7].irq));
	save_item(NAME(m_serial[0].ctrll));
	save_item(NAME(m_serial[0].ctrlh));
	save_item(NAME(m_serial[0].buf));
	save_item(NAME(m_serial[1].ctrll));
	save_item(NAME(m_serial[1].ctrlh));
	save_item(NAME(m_serial[1].buf));
	save_item(NAME(m_ddr));

	for (tmr = 0; tmr < MN10200_NUM_TIMERS_8BIT; tmr++)
	{
		m_timer_timers[tmr] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mn10200_device::simple_timer_cb), this));
		m_timer_timers[tmr]->adjust(attotime::never, tmr);
	}

	state_add( MN10200_PC,    "PC",    m_pc ).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_MDR,   "MDR",   m_mdr).formatstr("%04X");
	state_add( MN10200_D0,    "D0",    m_d[0]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_D1,    "D1",    m_d[1]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_D2,    "D2",    m_d[2]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_D3,    "D3",    m_d[3]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_A0,    "A0",    m_a[0]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_A1,    "A1",    m_a[1]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_A2,    "A2",    m_a[2]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_A3,    "A3",    m_a[3]).mask(0xffffff).formatstr("%06X");
	state_add( MN10200_NMICR, "MNICR", m_nmicr).formatstr("%02X");
	state_add( MN10200_IAGR,  "IAGR",  m_iagr).formatstr("%02X");

	state_add( STATE_GENPC, "GENPC", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_psw).formatstr("%26s").noshow();

	m_icountptr = &m_cycles;
}


void mn10200_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case CPUINFO_STR_FLAGS:
			string.printf( "S=%d irq=%s im=%d %c%c%c%c %c%c%c%c",
				(m_psw >> 12) & 3,
				m_psw & 0x0800 ? "on " : "off",
				(m_psw >> 8) & 7,
				m_psw & 0x0080 ? 'V' : '-',
				m_psw & 0x0040 ? 'C' : '-',
				m_psw & 0x0020 ? 'N' : '-',
				m_psw & 0x0010 ? 'Z' : '-',
				m_psw & 0x0008 ? 'v' : '-',
				m_psw & 0x0004 ? 'c' : '-',
				m_psw & 0x0002 ? 'n' : '-',
				m_psw & 0x0001 ? 'z' : '-');
			break;
	}
}


void mn10200_device::device_reset()
{
	int tmr, grp;

	memset(m_d, 0, sizeof(m_d));
	memset(m_a, 0, sizeof(m_a));
	m_pc = 0x80000;
	m_psw = 0;
	m_nmicr = 0;
	memset(m_icrl, 0, sizeof(m_icrl));
	memset(m_icrh, 0, sizeof(m_icrh));

	// reset all timers
	for (tmr = 0; tmr < MN10200_NUM_TIMERS_8BIT; tmr++)
	{
		m_simple_timer[tmr].mode = 0;
		m_simple_timer[tmr].cur = 0;
		m_simple_timer[tmr].base = 0;
		m_timer_timers[tmr]->adjust(attotime::never, tmr);
	}

	// clear all interrupt groups
	for (grp = 0; grp < MN10200_NUM_IRQ_GROUPS; grp++)
	{
		m_icrl[grp] = m_icrh[grp] = 0;
	}
}


void mn10200_device::unemul()
{
	fatalerror("MN10200: unknown opcode @ PC=%x\n", m_pc);
}

UINT32 mn10200_device::do_add(UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) + (b & 0xffffff);
	r16 = (a & 0xffff)   + (b & 0xffff);

	m_psw &= 0xff00;
	if((~(a^b)) & (a^r24) & 0x00800000)
	m_psw |= 0x80;
	if(r24 & 0x01000000)
	m_psw |= 0x40;
	if(r24 & 0x00800000)
	m_psw |= 0x20;
	if(!(r24 & 0x00ffffff))
	m_psw |= 0x10;
	if((~(a^b)) & (a^r16) & 0x00008000)
	m_psw |= 0x08;
	if(r16 & 0x00010000)
	m_psw |= 0x04;
	if(r16 & 0x00008000)
	m_psw |= 0x02;
	if(!(r16 & 0x0000ffff))
	m_psw |= 0x01;
	return r24 & 0xffffff;
}

UINT32 mn10200_device::do_addc(UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) + (b & 0xffffff);
	r16 = (a & 0xffff)   + (b & 0xffff);

	if(m_psw & 0x04) {
	r24++;
	r16++;
	}

	m_psw &= 0xff00;
	if((~(a^b)) & (a^r24) & 0x00800000)
	m_psw |= 0x80;
	if(r24 & 0x01000000)
	m_psw |= 0x40;
	if(r24 & 0x00800000)
	m_psw |= 0x20;
	if(!(r24 & 0x00ffffff))
	m_psw |= 0x10;
	if((~(a^b)) & (a^r16) & 0x00008000)
	m_psw |= 0x08;
	if(r16 & 0x00010000)
	m_psw |= 0x04;
	if(r16 & 0x00008000)
	m_psw |= 0x02;
	if(!(r16 & 0x0000ffff))
	m_psw |= 0x01;
	return r24 & 0xffffff;
}

UINT32 mn10200_device::do_sub(UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) - (b & 0xffffff);
	r16 = (a & 0xffff)   - (b & 0xffff);

	m_psw &= 0xff00;
	if((a^b) & (a^r24) & 0x00800000)
		m_psw |= 0x80;
	if(r24 & 0x01000000)
		m_psw |= 0x40;
	if(r24 & 0x00800000)
		m_psw |= 0x20;
	if(!(r24 & 0x00ffffff))
		m_psw |= 0x10;
	if((a^b) & (a^r16) & 0x00008000)
		m_psw |= 0x08;
	if(r16 & 0x00010000)
		m_psw |= 0x04;
	if(r16 & 0x00008000)
		m_psw |= 0x02;
	if(!(r16 & 0x0000ffff))
		m_psw |= 0x01;
	return r24 & 0xffffff;
}

UINT32 mn10200_device::do_subc(UINT32 a, UINT32 b)
{
	UINT32 r24, r16;
	r24 = (a & 0xffffff) - (b & 0xffffff);
	r16 = (a & 0xffff)   - (b & 0xffff);

	if(m_psw & 0x04) {
	r24--;
	r16--;
	}

	m_psw &= 0xff00;
	if(r24 >= 0x00800000 && r24 < 0xff800000)
		m_psw |= 0x80;
	if(r24 & 0x01000000)
		m_psw |= 0x40;
	if(r24 & 0x00800000)
		m_psw |= 0x20;
	if(!(r24 & 0x00ffffff))
		m_psw |= 0x10;
	if(r16 >= 0x00008000 && r16 < 0xffff8000)
		m_psw |= 0x08;
	if(r16 & 0x00010000)
		m_psw |= 0x04;
	if(r16 & 0x00008000)
		m_psw |= 0x02;
	if(!(r16 & 0x0000ffff))
		m_psw |= 0x01;
	return r24 & 0xffffff;
}

void mn10200_device::test_nz16(UINT16 v)
{
	m_psw &= 0xfff0;
	if(v & 0x8000)
		m_psw |= 2;
	if(!v)
		m_psw |= 1;
}

void mn10200_device::do_jsr(UINT32 to, UINT32 ret)
{
	mn102_change_pc(to & 0xffffff);
	m_a[3] -= 4;
	w24(m_a[3], ret);
}

#if 0
	log_event("MN102", "PSW change S=%d irq=%s im=%d %c%c%c%c %c%c%c%c",
		(psw >> 12) & 3,
		psw & 0x0800 ? "on" : "off",
		(psw >> 8) & 7,
		psw & 0x0080 ? 'V' : '-',
		psw & 0x0040 ? 'C' : '-',
		psw & 0x0020 ? 'N' : '-',
		psw & 0x0010 ? 'Z' : '-',
		psw & 0x0008 ? 'v' : '-',
		psw & 0x0004 ? 'c' : '-',
		psw & 0x0002 ? 'n' : '-',
		psw & 0x0001 ? 'z' : '-');
#endif

// take an external IRQ
void mn10200_device::execute_set_input(int irqnum, int state)
{
	int level = (m_icrh[7]>>4)&0x7;

//  printf("mn102_extirq: irq %d status %d G8 ICRL %x\n", irqnum, status, m_icrl[7]);

	// if interrupt is enabled, handle it
	if (state)
	{
		// indicate interrupt pending
		m_icrl[7] |= (1 << (4 + irqnum));

		// set interrupt detect = pending AND enable
		m_icrl[7] |= (m_icrh[7]&0x0f) & (m_icrl[7]>>4);

		// is the result enabled?
		if (m_icrl[7] & (1 << irqnum))
		{
			// try to take it now
			mn102_take_irq(level, 8);
		}
	}
}

void mn10200_device::execute_run()
{
	while(m_cycles > 0)
	{
		UINT8 opcode;

		debugger_instruction_hook(this, m_pc);

		opcode = mn102_read_byte(m_pc);
		switch(opcode) {
		// mov dm, (an)
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			m_cycles -= 1;
			mn102_write_word(m_a[(opcode>>2)&3], (UINT16)m_d[opcode & 3]);
			m_pc += 1;
			break;

		// movb dm, (an)
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_cycles -= 1;
			mn102_write_byte(m_a[(opcode>>2)&3], (UINT8)m_d[opcode & 3]);
			m_pc += 1;
			break;

		// mov (an), dm
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT16)mn102_read_word(m_a[(opcode>>2)&3]);
			m_pc += 1;
			break;

		// movbu (an), dm
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			m_cycles -= 1;
			m_d[opcode & 3] = mn102_read_byte(m_a[(opcode>>2)&3]);
			m_pc += 1;
			break;

		// mov dm, (d8, an)
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			m_cycles -= 1;
			mn102_write_word((m_a[(opcode>>2)&3]+(INT8)mn102_read_byte(m_pc+1)) & 0xffffff, (UINT16)m_d[opcode & 3]);
			m_pc += 2;
			break;

		// mov am, (d8, an)
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			m_cycles -= 2;
			w24((m_a[(opcode>>2)&3]+(INT8)mn102_read_byte(m_pc+1)) & 0xffffff, m_a[opcode & 3]);
			m_pc += 2;
			break;

		// mov (d8, an), dm
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT16)mn102_read_word((m_a[(opcode>>2) & 3] + (INT8)mn102_read_byte(m_pc+1)) & 0xffffff);
			m_pc += 2;
			break;

		// mov (d8, an), am
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_cycles -= 2;
			m_a[opcode & 3] = r24u((m_a[(opcode>>2) & 3] + (INT8)mn102_read_byte(m_pc+1)) & 0xffffff);
			m_pc += 2;
			break;

		// mov dm, dn
		case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
			m_cycles -= 1;
			m_d[opcode & 3] = m_d[(opcode>>2) & 3];
			m_pc += 1;
			break;

		// mov imm8, dn
		case 0x80: case 0x85: case 0x8a: case 0x8f:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT8)mn102_read_byte(m_pc+1);
			m_pc += 2;
			break;

		// add dn, dm
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			m_cycles -= 1;
			m_d[opcode & 3] = do_add(m_d[opcode & 3], m_d[(opcode >> 2) & 3]);
			m_pc += 1;
			break;

		// sub dn, dm
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			m_cycles -= 1;
			m_d[opcode & 3] = do_sub(m_d[opcode & 3], m_d[(opcode >> 2) & 3]);
			m_pc += 1;
			break;

		// extx dn
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT16)m_d[opcode & 3] & 0xffffff;
			m_pc += 1;
			break;

		// extxu dn
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			m_cycles -= 1;
			m_d[opcode & 3] = (UINT16)m_d[opcode & 3];
			m_pc += 1;
			break;

		// extxb dn
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT8)m_d[opcode & 3] & 0xffffff;
			m_pc += 1;
			break;

		// extxbu dn
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			m_cycles -= 1;
			m_d[opcode & 3] = (UINT8)m_d[opcode & 3];
			m_pc += 1;
			break;

		// mov dn, (imm16)
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
			m_cycles -= 2;
			mn102_write_word(mn102_read_word(m_pc+1), (UINT16)m_d[opcode & 3]);
			m_pc += 3;
			break;

		// movb dn, (imm16)
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			m_cycles -= 1;
			mn102_write_byte(mn102_read_word(m_pc+1), (UINT8)m_d[opcode & 3]);
			m_pc += 3;
			break;

		// mov (abs16), dn
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT16)mn102_read_word(mn102_read_word(m_pc+1));
			m_pc += 3;
			break;

		// movbu (abs16), dn
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			m_cycles -= 2;
			m_d[opcode & 3] = mn102_read_byte(mn102_read_word(m_pc+1));
			m_pc += 3;
			break;

		// add imm8, an
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			m_cycles -= 1;
			m_a[opcode & 3] = do_add(m_a[opcode & 3], (INT8)mn102_read_byte(m_pc+1));
			m_pc += 2;
			break;

		// add imm8, dn
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			m_cycles -= 1;
			m_d[opcode & 3] = do_add(m_d[opcode & 3], (INT8)mn102_read_byte(m_pc+1));
			m_pc += 2;
			break;

		// cmp imm8, dn
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
			m_cycles -= 1;
			do_sub(m_d[opcode & 3], (INT8)mn102_read_byte(m_pc+1));
			m_pc += 2;
			break;

		// mov imm16, an
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
			m_cycles -= 1;
			m_a[opcode & 3] = mn102_read_word(m_pc+1);
			m_pc += 3;
			break;

		// blt label8
		case 0xe0:
			if(((m_psw & 0x0a) == 2) || ((m_psw & 0x0a) == 8))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bgt label8
		case 0xe1:
			if(((m_psw & 0x0b) == 0) || ((m_psw & 0x0b) == 0xa))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bge label8
		case 0xe2:
			if(((m_psw & 0x0a) == 0) || ((m_psw & 0x0a) == 0xa))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// ble label8
		case 0xe3:
			if((m_psw & 0x01) || ((m_psw & 0x0a) == 2) || ((m_psw & 0x0a) == 8))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
			m_cycles -= 1;
			m_pc += 2;
			}
			break;

		// bcs label8
		case 0xe4:
			if(m_psw & 0x04)
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bhi label8
		case 0xe5:
			if(!(m_psw & 0x05))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bcc label8
		case 0xe6:
			if(!(m_psw & 0x04))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bls label8
		case 0xe7:
			if(m_psw & 0x05)
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// beq label8
		case 0xe8:
			if(m_psw & 0x01)
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bne label8
		case 0xe9:
			if(!(m_psw & 0x01))
			{
				m_cycles -= 2;
				mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			}
			else
			{
				m_cycles -= 1;
				m_pc += 2;
			}
			break;

		// bra label8
		case 0xea:
			m_cycles -= 2;
			mn102_change_pc(m_pc+2+(INT8)mn102_read_byte(m_pc+1));
			break;

		// rti
		case 0xeb:
			m_cycles -= 6;
			m_psw = mn102_read_word(m_a[3]);
			mn102_change_pc(r24u(m_a[3]+2));
			m_a[3] += 6;
			break;

		// cmp imm16, an
		case 0xec: case 0xed: case 0xee: case 0xef:
			m_cycles -= 1;
			do_sub(m_a[opcode & 3], mn102_read_word(m_pc+1));
			m_pc += 3;
			break;

		case 0xf0:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode) {
				// jmp (an)
				case 0x00: case 0x04: case 0x08: case 0x0c:
				m_cycles -= 3;
				mn102_change_pc(m_a[(opcode>>2) & 3]);
				break;

				// jsr (an)
				case 0x01: case 0x05: case 0x09: case 0x0d:
				m_cycles -= 5;
				do_jsr(m_a[(opcode>>2) & 3], m_pc+2);
				break;

				// bset dm, (an)
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: {
				UINT8 v;
				m_cycles -= 5;
				v = mn102_read_byte(m_a[(opcode>>2) & 3]);
				test_nz16(v & m_d[opcode & 3]);
				mn102_write_byte(m_a[(opcode>>2) & 3], v | m_d[opcode & 3]);
				m_pc += 2;
				break;
				}

				// bclr dm, (an)
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f: {
				UINT8 v;
				m_cycles -= 5;
				v = mn102_read_byte(m_a[(opcode>>2) & 3]);
				test_nz16(v & m_d[opcode & 3]);
				mn102_write_byte(m_a[(opcode>>2) & 3], v & ~m_d[opcode & 3]);
				m_pc += 2;
				break;
				}

				// movb (di, an), dm
				case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
				case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
				case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
				case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				m_cycles -= 2;
				m_d[opcode & 3] = (INT8)mn102_read_byte((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff);
				m_pc += 2;
				break;

				// movbu (di, an), dm
				case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
				case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				m_cycles -= 2;
				m_d[opcode & 3] = mn102_read_byte((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff);
				m_pc += 2;
				break;

				// movb dm, (di, an)
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
				case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
				case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
				case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
				case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
				m_cycles -= 2;
				mn102_write_byte((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff, m_d[opcode & 3]);
				m_pc += 2;
				break;

				default:
				unemul();
				break;
			}
			break;

		case 0xf1:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode>>6) {
				// mov (di, an), am
				case 0:
				m_cycles -= 3;
				m_a[opcode & 3] = r24u((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff);
				m_pc += 2;
				break;

				// mov (di, an), dm
				case 1:
				m_cycles -= 2;
				m_d[opcode & 3] = mn102_read_word((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff);
				m_pc += 2;
				break;

				// mov am, (di, an)
				case 2:
				m_cycles -= 3;
				w24((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff, m_a[opcode & 3]);
				m_pc += 2;
				break;

				// mov dm, (di, an)
				case 3:
				m_cycles -= 2;
				mn102_write_word((m_a[(opcode>>2) & 3] + m_d[(opcode>>4) & 3]) & 0xffffff, m_d[opcode & 3]);
				m_pc += 2;
				break;

				default:
				unemul();
				break;
				}
			break;

		case 0xf2:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode>>4) {
				// add dm, an
				case 0x0:
				m_cycles -= 2;
				m_a[opcode & 3] = do_add(m_a[opcode & 3], m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// sub dm, an
				case 0x1:
				m_cycles -= 2;
				m_a[opcode & 3] = do_sub(m_a[opcode & 3], m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// cmp dm, an
				case 0x2:
				m_cycles -= 2;
				do_sub(m_a[opcode & 3], m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// mov am, dn
				case 0x3:
				m_cycles -= 2;
				m_a[opcode & 3] = m_d[(opcode>>2) & 3];
				m_pc += 2;
				break;

				// add am, an
				case 0x4:
				m_cycles -= 2;
				m_a[opcode & 3] = do_add(m_a[opcode & 3], m_a[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// sub am, an
				case 0x5:
				m_cycles -= 2;
				m_a[opcode & 3] = do_sub(m_a[opcode & 3], m_a[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// cmp am, an
				case 0x6:
				m_cycles -= 2;
				do_sub(m_a[opcode & 3], m_a[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// mov am, an
				case 0x7:
				m_cycles -= 2;
				m_a[opcode & 3] = m_a[(opcode>>2) & 3];
				m_pc += 2;
				break;

				// addc dm, dn
				case 0x8:
				m_cycles -= 2;
				m_d[opcode & 3] = do_addc(m_d[opcode & 3], m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// subc dm, dn
				case 0x9:
				m_cycles -= 2;
				m_d[opcode & 3] = do_subc(m_d[opcode & 3], m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// add am, dn
				case 0xc:
				m_cycles -= 2;
				m_d[opcode & 3] = do_add(m_d[opcode & 3], m_a[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// sub am, dn
				case 0xd:
				m_cycles -= 2;
				m_d[opcode & 3] = do_sub(m_d[opcode & 3], m_a[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// cmp am, dn
				case 0xe:
				m_cycles -= 2;
				do_sub(m_d[opcode & 3], m_a[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// mov an, dm
				case 0xf:
				m_cycles -= 2;
				m_d[opcode & 3] = m_a[(opcode>>2) & 3];
				m_pc += 2;
				break;

				default:
				unemul();
				break;
				}
			break;

		case 0xf3:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode) {
				// and dm, dn
				case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] &= 0xff0000|m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// or dm, dn
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] |= 0x00ffff&m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// xor dm, dn
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] ^= 0x00ffff&m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// rol dn
				case 0x30: case 0x31: case 0x32: case 0x33: {
				UINT32 d = m_d[opcode & 3];
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] = (d & 0xff0000) | ((d << 1) & 0x00fffe) | ((m_psw & 0x04) ? 1 : 0));
				if(d & 0x8000)
				m_psw |= 0x04;
				m_pc += 2;
				break;
				}

				// ror dn
				case 0x34: case 0x35: case 0x36: case 0x37: {
				UINT32 d = m_d[opcode & 3];
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] = (d & 0xff0000) | ((d >> 1) & 0x007fff) | ((m_psw & 0x04) ? 0x8000 : 0));
				if(d & 1)
				m_psw |= 0x04;
				m_pc += 2;
				break;
				}

				// asr dn
				case 0x38: case 0x39: case 0x3a: case 0x3b: {
				UINT32 d = m_d[opcode & 3];
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] = (d & 0xff8000) | ((d >> 1) & 0x007fff));
				if(d & 1)
				m_psw |= 0x04;
				m_pc += 2;
				break;
				}

				// lsr dn
				case 0x3c: case 0x3d: case 0x3e: case 0x3f: {
				UINT32 d = m_d[opcode & 3];
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] = (d & 0xff0000) | ((d >> 1) & 0x007fff));
				if(d & 1)
				m_psw |= 0x04;
				m_pc += 2;
				break;
				}

				// mul dn, dm
				case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
				case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f: {
				UINT32 res;
				m_cycles -= 12;
				res = ((INT16)m_d[opcode & 3])*((INT16)m_d[(opcode>>2) & 3]);
				m_d[opcode & 3] = res & 0xffffff;
				m_psw &= 0xff00;
				if(res & 0x80000000)
				m_psw |= 2;
				if(!res)
				m_psw |= 1;
				m_mdr = res >> 16;
				m_pc += 2;
				break;
				}

				// mulu dn, dm
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: {
				UINT32 res;
				m_cycles -= 12;
				res = ((UINT16)m_d[opcode & 3])*((UINT16)m_d[(opcode>>2) & 3]);
				m_d[opcode & 3] = res & 0xffffff;
				m_psw &= 0xff00;
				if(res & 0x80000000)
				m_psw |= 2;
				if(!res)
				m_psw |= 1;
				m_mdr = res >> 16;
				m_pc += 2;
				break;
				}

				// divu dn, dm
				case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
				case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: {
				UINT32 n, d, q, r;
				m_cycles -= 13;
				m_pc += 2;
				m_psw &= 0xff00;

				n = (m_mdr<<16)|(UINT16)m_d[opcode & 3];
				d = (UINT16)m_d[(opcode>>2) & 3];
				if(!d) {
				m_psw |= 8;
				break;
				}
				q = n/d;
				r = n%d;
				if(q >= 0x10000) {
				m_psw |= 8;
				break;
				}
				m_d[opcode & 3] = q;
				m_mdr = r;
				if(!q)
				m_psw |= 0x11;
				if(q & 0x8000)
				m_psw |= 2;
				break;
				}

				// cmp dm, dn
				case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				m_cycles -= 2;
				do_sub(m_d[opcode & 3], m_d[(opcode>>2) & 3]);
				m_pc += 2;
				break;

				// mov mdr, dn
				case 0xc0: case 0xc4: case 0xc8: case 0xcc:
				m_cycles -= 2;
				m_mdr = m_d[(opcode>>2) & 3];
				m_pc += 2;
				break;

				// ext dn
				case 0xc1: case 0xc5: case 0xc9: case 0xcd:
				m_cycles -= 3;
				m_mdr = m_d[(opcode>>2) & 3] & 0x8000 ? 0xffff : 0x0000;
				m_pc += 2;
				break;

				// mov dn, psw
				case 0xd0: case 0xd4: case 0xd8: case 0xdc:
				m_cycles -= 3;
				m_psw = m_d[(opcode>>2) & 3];
				m_pc += 2;
				break;

				// mov dn, mdr
				case 0xe0: case 0xe1: case 0xe2: case 0xe3:
				m_cycles -= 2;
				m_d[opcode & 3] = m_mdr;
				m_pc += 2;
				break;

				// not dn
				case 0xe4: case 0xe5: case 0xe6: case 0xe7:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] ^= 0x00ffff);
				m_pc += 2;
				break;

				// mov psw, dn
				case 0xf0: case 0xf1: case 0xf2: case 0xf3:
				m_cycles -= 3;
				m_d[(opcode>>2) & 3] = m_psw;
				m_pc += 2;
				break;

				default:
				unemul();
				break;
				}
			break;

		case 0xf4:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode) {
				// mov dm, (abs24, an)
				case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				m_cycles -= 3;
				mn102_write_word((r24u(m_pc+2) + m_a[(opcode>>2) & 3]) & 0xffffff, m_d[opcode & 3]);
				m_pc += 5;
				break;

				// mov am, (abs24, an)
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				m_cycles -= 4;
				w24((r24u(m_pc+2) + m_a[(opcode>>2) & 3]) & 0xffffff, m_a[opcode & 3]);
				m_pc += 5;
				break;

				// movb dm, (abs24, an)
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				m_cycles -= 3;
				mn102_write_byte((r24u(m_pc+2) + m_a[(opcode>>2) & 3]) & 0xffffff, m_d[opcode & 3]);
				m_pc += 5;
				break;

				// movx dm, (abs24, an)
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				m_cycles -= 4;
				w24((r24u(m_pc+2) + m_a[(opcode>>2) & 3]) & 0xffffff, m_d[opcode & 3]);
				m_pc += 5;
				break;

				// mov dn, (abs24)
				case 0x40: case 0x41: case 0x42: case 0x43:
				m_cycles -= 3;
				mn102_write_word(r24u(m_pc+2), m_d[opcode & 3]);
				m_pc += 5;
				break;

				// movb dn, (abs24)
				case 0x44: case 0x45: case 0x46: case 0x47:
				m_cycles -= 3;
				mn102_write_byte(r24u(m_pc+2), m_d[opcode & 3]);
				m_pc += 5;
				break;

				// mov an, (abs24)
				case 0x50: case 0x51: case 0x52: case 0x53:
				m_cycles -= 4;
				w24(r24u(m_pc+2), m_a[opcode & 3]);
				m_pc += 5;
				break;

				// add abs24, dn
				case 0x60: case 0x61: case 0x62: case 0x63:
				m_cycles -= 3;
				m_d[opcode & 3] = do_add(m_d[opcode & 3], r24u(m_pc+2));
				m_pc += 5;
				break;

				// add abs24, an
				case 0x64: case 0x65: case 0x66: case 0x67:
				m_cycles -= 3;
				m_a[opcode & 3] = do_add(m_a[opcode & 3], r24u(m_pc+2));
				m_pc += 5;
				break;

				// sub abs24, dn
				case 0x68: case 0x69: case 0x6a: case 0x6b:
				m_cycles -= 3;
				m_d[opcode & 3] = do_sub(m_d[opcode & 3], r24u(m_pc+2));
				m_pc += 5;
				break;

				// sub abs24, an
				case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				m_cycles -= 3;
				m_a[opcode & 3] = do_sub(m_a[opcode & 3], r24u(m_pc+2));
				m_pc += 5;
				break;

				// mov imm24, dn
				case 0x70: case 0x71: case 0x72: case 0x73:
				m_cycles -= 3;
				m_d[opcode & 3] = r24u(m_pc+2);
				m_pc += 5;
				break;

				// mov imm24, an
				case 0x74: case 0x75: case 0x76: case 0x77:
				m_cycles -= 3;
				m_a[opcode & 3] = r24u(m_pc+2);
				m_pc += 5;
				break;

				// cmp abs24, dn
				case 0x78: case 0x79: case 0x7a: case 0x7b:
				m_cycles -= 3;
				do_sub(m_d[opcode & 3], r24u(m_pc+2));
				m_pc += 5;
				break;

				// cmp abs24, an
				case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				m_cycles -= 3;
				do_sub(m_a[opcode & 3], r24u(m_pc+2));
				m_pc += 5;
				break;

				// mov (abs24, an), dm
				case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				m_cycles -= 3;
				m_d[opcode & 3] = (INT16)mn102_read_word((m_a[(opcode>>2) & 3] + r24u(m_pc+2)) & 0xffffff);
				m_pc += 5;
				break;

				// movbu (abs24, an), dm
				case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				m_cycles -= 3;
				m_d[opcode & 3] = mn102_read_byte((m_a[(opcode>>2) & 3] + r24u(m_pc+2)) & 0xffffff);
				m_pc += 5;
				break;

				// movb (abs24, an), dm
				case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
				case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				m_cycles -= 3;
				m_d[opcode & 3] = (INT8)mn102_read_byte((m_a[(opcode>>2) & 3] + r24u(m_pc+2)) & 0xffffff);
				m_pc += 5;
				break;

				// movx (abs24, an), dm
				case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				m_cycles -= 4;
				m_d[opcode & 3] = r24u((m_a[(opcode>>2) & 3] + r24u(m_pc+2)) & 0xffffff);
				m_pc += 5;
				break;

				// mov (abs24), dn
				case 0xc0: case 0xc1: case 0xc2: case 0xc3:
				m_cycles -= 3;
				m_d[opcode & 3] = (INT16)mn102_read_word(r24u(m_pc+2));
				m_pc += 5;
				break;

				// movb (abs24), dn
				case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				m_cycles -= 3;
				m_d[opcode & 3] = (INT8)mn102_read_byte(r24u(m_pc+2));
				m_pc += 5;
				break;

				// movbu (abs24), dn
				case 0xc8: case 0xc9: case 0xca: case 0xcb:
				m_cycles -= 3;
				m_d[opcode & 3] = mn102_read_byte(r24u(m_pc+2));
				m_pc += 5;
				break;

				// mov (abs24), an
				case 0xd0: case 0xd1: case 0xd2: case 0xd3:
				m_cycles -= 4;
				m_a[opcode & 3] = r24u(r24u(m_pc+2));
				m_pc += 5;
				break;

				// jmp imm24
				case 0xe0:
				m_cycles -= 4;
				mn102_change_pc(m_pc+5+r24u(m_pc+2));
				break;

				// jsr label24
				case 0xe1:
				m_cycles -= 5;
				do_jsr(m_pc+5+r24u(m_pc+2), m_pc+5);
				break;


				// mov (abs24, an), am
				case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
				case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
				m_cycles -= 4;
				m_a[opcode & 3] = r24u((m_a[(opcode>>2) & 3] + r24u(m_pc+2)) & 0xffffff);
				m_pc += 5;
				break;

				default:
				unemul();
				break;
				}
			break;

		case 0xf5:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode) {
				// and imm8, dn
				case 0x00: case 0x01: case 0x02: case 0x03:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] &= 0xff0000|mn102_read_byte(m_pc+2));
				m_pc += 3;
				break;

				// btst imm8, dn
				case 0x04: case 0x05: case 0x06: case 0x07:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] & mn102_read_byte(m_pc+2));
				m_pc += 3;
				break;

				// or imm8, dn
				case 0x08: case 0x09: case 0x0a: case 0x0b:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] |= mn102_read_byte(m_pc+2));
				m_pc += 3;
				break;

				// addnf imm8, an
				case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				m_cycles -= 2;
				m_a[opcode & 3] = m_a[opcode & 3] +(INT8)mn102_read_byte(m_pc+2);
				m_pc += 3;
				break;

				// movb dm, (d8, an)
				case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				m_cycles -= 2;
				mn102_write_byte((m_a[(opcode>>2) & 3]+(INT8)mn102_read_byte(m_pc+2)) & 0xffffff, m_d[opcode & 3]);
				m_pc += 3;
				break;

				// movb (d8, an), dm
				case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				m_cycles -= 2;
				m_d[opcode & 3] = (INT8)mn102_read_byte((m_a[(opcode>>2) & 3]+(INT8)mn102_read_byte(m_pc+2)) & 0xffffff);
				m_pc += 3;
				break;

				// movbu (d8, an), dm
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				m_cycles -= 2;
				m_d[opcode & 3] = mn102_read_byte((m_a[(opcode>>2) & 3]+(INT8)mn102_read_byte(m_pc+2)) & 0xffffff);
				m_pc += 3;
				break;

				// movx dm, (d8, an)
				case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				m_cycles -= 3;
				w24((m_a[(opcode>>2) & 3]+(INT8)mn102_read_byte(m_pc+2)) & 0xffffff, m_d[opcode & 3]);
				m_pc += 3;
				break;

				// movx (d8, an), dm
				case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
				case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				m_cycles -= 3;
				m_d[opcode & 3] = r24u((m_a[(opcode>>2) & 3]+(INT8)mn102_read_byte(m_pc+2)) & 0xffffff);
				m_pc += 3;
				break;

				// bltx label8
				case 0xe0:
				if(((m_psw & 0xa0) == 0x20) || ((m_psw & 0xa0) == 0x80)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// bgtx label8
				case 0xe1:
				if(((m_psw & 0xb0) == 0) || ((m_psw & 0xb0) == 0xa0)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// bgex label8
				case 0xe2:
				if(((m_psw & 0xa0) == 0) || ((m_psw & 0xa0) == 0xa0)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// blex label8
				case 0xe3:
				if((m_psw & 0x10) || ((m_psw & 0xa0) == 0x20) || ((m_psw & 0xa0) == 0x80)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// bcsx label8
				case 0xe4:
				if(m_psw & 0x40) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// bhix label8
				case 0xe5:
				if(!(m_psw & 0x50)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// bccx label8
				case 0xe6:
				if(!(m_psw & 0x40)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// blsx label8
				case 0xe7:
				if(m_psw & 0x50) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// beqx label8
				case 0xe8:
				if(m_psw & 0x10) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				// bnex label8
				case 0xe9:
				if(!(m_psw & 0x10)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;


				// bnc label8
				case 0xfe:
				if(!(m_psw & 0x02)) {
				m_cycles -= 3;
				mn102_change_pc(m_pc+3+(INT8)mn102_read_byte(m_pc+2));
				} else {
				m_cycles -= 2;
				m_pc += 3;
				}
				break;

				default:
				unemul();
				break;
				}
			break;

		case 0xf7:
			opcode = mn102_read_byte(m_pc+1);
			switch(opcode) {
				// and imm16, dn
				case 0x00: case 0x01: case 0x02: case 0x03:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] &= 0xff0000|mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// btst imm16, dn
				case 0x04: case 0x05: case 0x06: case 0x07:
				m_cycles -= 2;
				test_nz16(m_d[opcode & 3] & mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// add imm16, an
				case 0x08: case 0x09: case 0x0a: case 0x0b:
				m_cycles -= 2;
				m_a[opcode & 3] = do_add(m_a[opcode & 3], (INT16)mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// sub imm16, an
				case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				m_cycles -= 2;
				m_a[opcode & 3] = do_sub(m_a[opcode & 3], (INT16)mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// and imm16, psw
				case 0x10:
				m_cycles -= 3;
				m_psw &= mn102_read_word(m_pc+2);
				m_pc += 4;
				break;

				// or imm16, psw
				case 0x14:
				m_cycles -= 3;
				m_psw |= mn102_read_word(m_pc+2);
				m_pc += 4;
				break;

				// add imm16, dn
				case 0x18: case 0x19: case 0x1a: case 0x1b:
				m_cycles -= 2;
				m_d[opcode & 3] = do_add(m_d[opcode & 3], (INT16)mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// sub imm16, dn
				case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				m_cycles -= 2;
				m_d[opcode & 3] = do_sub(m_d[opcode & 3], (INT16)mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// or imm16, dn
				case 0x40: case 0x41: case 0x42: case 0x43:
				m_cycles -= 3;
				test_nz16(m_d[opcode & 3] |= mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// cmp imm16, dn
				case 0x48: case 0x49: case 0x4a: case 0x4b:
				m_cycles -= 2;
				do_sub(m_d[opcode & 3], (INT16)mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// xor imm16, dn
				case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				m_cycles -= 3;
				test_nz16(m_d[opcode & 3] ^= mn102_read_word(m_pc+2));
				m_pc += 4;
				break;

				// mov dm, (imm16, an)
				case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				m_cycles -= 2;
				mn102_write_word((m_a[(opcode>>2)&3]+(INT16)mn102_read_word(m_pc+2)) & 0xffffff, (UINT16)m_d[opcode & 3]);
				m_pc += 4;
				break;

				// mov (imm16, an), dm
				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
				case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
				m_cycles -= 2;
				m_d[opcode & 3] = (INT16)mn102_read_word((m_a[(opcode>>2) & 3] + (INT16)mn102_read_byte(m_pc+2)) & 0xffffff) & 0xffffff;
				m_pc += 4;
				break;


				default:
				unemul();
				break;
				}
			break;

		// mov imm16, dn
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
			m_cycles -= 1;
			m_d[opcode & 3] = (INT16)mn102_read_word(m_pc+1);
			m_pc += 3;
			break;

		// jmp label16
		case 0xfc:
			m_cycles -= 2;
			mn102_change_pc(m_pc+3+(INT16)mn102_read_word(m_pc+1));
			break;

		// jsr label16
		case 0xfd:
			m_cycles -= 4;
			do_jsr(m_pc+3+(INT16)mn102_read_word(m_pc+1), m_pc+3);
			break;

		// rts
		case 0xfe:
			m_cycles -= 5;
			mn102_change_pc(r24u(m_a[3]));
			m_a[3] += 4;
			break;

		default:
			unemul();
			break;
		}
	}
}

static const char *const inames[10][4] = {
	{ "timer0", "timer1", "timer2", "timer3" },
	{ "timer4", "timer5", "timer6", "timer7" },
	{ "timer8", "timer9", "timer12a", "timer12b" },
	{ "timer10u", "timer10a", "timer10b", "?" },
	{ "timer11u", "timer11a", "timer11b", "?" },
	{ "dma0", "dma1", "dma2", "dma3" },
	{ "dma4", "dma5", "dma6", "dma7" },
	{ "X0", "X1", "X2", "X3" },
	{ "ser0tx", "ser0rx", "ser1tx", "ser1rx" },
	{ "key", "a/d", "?", "?" }
};

void mn10200_device::mn10200_w(UINT32 adr, UINT32 data, int type)
{
	if(type == MEM_WORD) {
	mn10200_w(adr, data & 0xff, MEM_BYTE);
	mn10200_w(adr+1, (data>>8) & 0xff, MEM_BYTE);
	return;
	}

	switch(adr) {
	case 0x000:
	if(data & 12) {
		log_event("CPU", "Stop request");
	}
	break;
	case 0x001:
	log_event("WATCHDOG", "Write %d", data>>7);
	break;

	case 0x002: case 0x003: // Memory control
	break;

	case 0x030: case 0x031: // Memory mode reg 0
	case 0x032: case 0x033: // Memory mode reg 1
	case 0x034: case 0x035: // Memory mode reg 2
	case 0x036: case 0x037: // Memory mode reg 3
	break;

	// Non-Maskable irqs
	case 0x040:
	m_nmicr = data & 6;
	break;
	case 0x041:
	break;

	// Maskable irq control
	case 0x042: case 0x044: case 0x046: case 0x048: case 0x04a:
	case 0x04c: case 0x04e: case 0x050: case 0x052: case 0x054:
	{
		// note: writes here ack interrupts
	m_icrl[((adr & 0x3f)>>1)-1] = data;
	}
	break;

	case 0x043: case 0x045: case 0x047: case 0x049: case 0x04b:
	case 0x04d: case 0x04f: case 0x051: case 0x053: case 0x055: {
	int irq = ((adr & 0x3f)>>1)-1;
#if 0
	if((m_icrh[irq] != data) && (data & 15)) {
		printf("MN10200: irq %d enabled, level=%x, enable= %s %s %s %s\n", irq+1, (data >> 4) & 7,
		data & 1 ? inames[irq][0] : "-",
		data & 2 ? inames[irq][1] : "-",
		data & 4 ? inames[irq][2] : "-",
		data & 8 ? inames[irq][3] : "-");
	}
	if((m_icrh[irq] != data) && !(data & 15)) {
		printf("MN10200: irq %d disabled\n", irq+1);
	}
#endif
	m_icrh[irq] = data;
	break;
	}

	case 0x056: {
//    const char *modes[4] = { "l", "h", "fall", "rise" };
//    log_event("MN102", "irq3=%s irq2=%s irq1=%s irq0=%s",
//        modes[(data >> 6) & 3], modes[(data >> 4) & 3], modes[(data >> 2) & 3], modes[data & 3]);
	break;
	}

	case 0x057: {
//    const char *modes[4] = { "l", "1", "fall", "3" };
//    log_event("MN102", "irq_ki=%s", modes[data & 3]);
	break;
	}

	case 0x100: case 0x101: // DRAM control reg
	case 0x102: case 0x103: // Refresh counter
	break;

	case 0x180: case 0x190: {
	int ser = (adr-0x180) >> 4;
//    const char *parity[8] = { "no", "1", "2", "3", "l", "h", "even", "odd" };
//    const char *source[4] = { "sbt0", "timer 8", "2", "timer 9" };
	m_serial[ser].ctrll = data;

//    log_event("MN102", "Serial %d length=%c, parity=%s, stop=%c, source=%s",
//        ser,
//        data & 0x80 ? '8' : '7', parity[(data >> 4) & 7],
//        data & 8 ? '2' : '1', source[data & 3]);
	break;
	}

	case 0x181: case 0x191: {
	int ser = (adr-0x180) >> 4;
	m_serial[ser].ctrlh = data;
//    log_event("MN102", "Serial %d transmit=%s, receive=%s, break=%s, proto=%s, order=%s",
//        ser,
//        data & 0x80 ? "on" : "off", data & 0x40 ? "on" : "off",
//        data & 0x20 ? "on" : "off", data & 8 ? "sync" : "async",
//        data & 2 ? "msb" : "lsb");
	break;
	}

	case 0x182: case 0x192: {
	int ser = (adr-0x180) >> 4;
	m_serial[ser].buf = data;
	log_event("MN102", "Serial %d buffer=%02x", ser, data);
	break;
	}

	case 0x1a0:
		log_event("MN102", "AN %s timer7=%s /%d %s %s",
			data & 0x80 ? "on" : "off", data & 0x40 ? "on" : "off",
			1 << ((data >> 2) & 3), data & 2 ? "continuous" : "single", data & 1 ? "one" : "multi");
		break;

	case 0x1a1:
		log_event("MN102", "AN chans=0-%d current=%d", (data >> 4) & 7, data & 7);
		break;

	case 0x210: case 0x211: case 0x212: case 0x213: case 0x214:
	case 0x215: case 0x216: case 0x217: case 0x218: case 0x219:
		m_simple_timer[adr-0x210].base = data + 1;
//      printf("MN10200: Timer %d value set %02x\n", adr-0x210, data);
		refresh_timer(adr-0x210);
		break;

	case 0x21a:
		m_prescaler[0].cycles = data+1;
//      printf("MN10200: Prescale 0 cycle count %d\n", data+1);
		break;

	case 0x21b:
		m_prescaler[1].cycles = data+1;
//      printf("MN10200: Prescale 1 cycle count %d\n", data+1);
		break;

	case 0x220: case 0x221: case 0x222: case 0x223: case 0x224:
	case 0x225: case 0x226: case 0x227: case 0x228: case 0x229:
	{
//      const char *source[4] = { "TMxIO", "cascade", "prescale 0", "prescale 1" };
		m_simple_timer[adr-0x220].mode = data;
//      printf("MN10200: Timer %d %s b6=%d, source=%s\n", adr-0x220, data & 0x80 ? "on" : "off", (data & 0x40) != 0, source[data & 3]);

		if (data & 0x40)
		{
//          printf("MN10200: loading timer %d\n", adr-0x220);
			m_simple_timer[adr-0x220].cur = m_simple_timer[adr-0x220].base;
		}
		refresh_timer(adr-0x220);
		break;
	}

	case 0x22a:
//      printf("MN10200: Prescale 0 %s, ps0bc %s\n",
//          data & 0x80 ? "on" : "off", data & 0x40 ? "-> ps0br" : "off");
		m_prescaler[0].mode = data;
		break;

	case 0x22b:
//      printf("MN10200: Prescale 1 %s, ps1bc %s\n",
//          data & 0x80 ? "on" : "off", data & 0x40 ? "-> ps1br" : "off");
		m_prescaler[1].mode = data;
		break;

	case 0x230: case 0x240: case 0x250:
	{
//      const char *modes[4] = { "single", "double", "ioa", "iob" };
//      const char *sources[8] = { "pres.0", "pres.1", "iob", "sysclk", "*4", "*1", "6", "7" };
//      printf("MN10200: Timer %d comp=%s on_1=%s on_match=%s phase=%s source=%s\n",
//          10 + ((adr-0x230) >> 4),
//          modes[data >> 6], data & 0x20 ? "cleared" : "not cleared",  data & 0x10 ? "cleared" : "not cleared",
//          data & 8 ? "tff" : "rsff", sources[data & 7]);
		break;
	}

	case 0x231: case 0x241: case 0x251:
	{
//      const char *modes[4] = { "up", "down", "up on ioa", "up on iob" };
//      printf("MN10200: Timer %d %s ff=%s op=%s ext_trig=%s %s\n",
//      10 + ((adr-0x230) >> 4),
//      data & 0x80 ? "enable" : "disable", data & 0x40 ? "operate" : "clear",
//      modes[(data >> 4) & 3], data & 2 ? "on" : "off", data & 1 ? "one-shot" : "repeat");

		break;
	}

	case 0x234: case 0x244: case 0x254:
	log_event("MN102", "Timer %d ca=--%02x", 10 + ((adr-0x230) >> 4), data);
	break;

	case 0x235: case 0x245: case 0x255:
	log_event("MN102", "Timer %d ca=%02x--", 10 + ((adr-0x230) >> 4), data);
	break;

	case 0x236: case 0x246: case 0x256:
	log_event("MN102", "Timer %d ca read trigger", 10 + ((adr-0x230) >> 4));
	break;

	case 0x237: case 0x247: case 0x257: break;

	case 0x238: case 0x248: case 0x258:
	log_event("MN102", "Timer %d cb=--%02x", 10 + ((adr-0x230) >> 4), data);
	break;

	case 0x239: case 0x249: case 0x259:
	log_event("MN102", "Timer %d cb=%02x--", 10 + ((adr-0x230) >> 4), data);
	break;

	case 0x23a: case 0x24a: case 0x25a:
	log_event("MN102", "Timer %d cb read trigger", 10 + ((adr-0x230) >> 4));
	break;

	case 0x23b: case 0x24b: case 0x25b: break;

	case 0x260: case 0x261: {
//    const char *mode[4] = { "sysbuf", "4-phase", "4-phase 1/2", "3" };
//    log_event("MN102", "Sync Output %c timing=%s out=%s dir=%s mode=%s",
//        adr == 0x261 ? 'B' : 'A',
//        data & 0x10 ? "12A" : "1", data & 8 ? "sync a" :"P13-10",
//        data & 4 ? "ccw" : "cw", mode[data & 3]);
	break;
	}

	case 0x262:
	log_event("MN102", "Sync Output buffer = %02x", data);
	break;

	case 0x264:
	m_io->write_byte(MN10200_PORT1, data);
	break;

	case 0x280: case 0x290: case 0x2a0: case 0x2b0: case 0x2c0: case 0x2d0: case 0x2e0: case 0x2f0: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].adr = (m_dma[dma].adr & 0x00ffff00) | data;
	logerror("MN10200: DMA %d adr=%06x\n", dma, m_dma[dma].adr);
	break;
	}

	case 0x281: case 0x291: case 0x2a1: case 0x2b1: case 0x2c1: case 0x2d1: case 0x2e1: case 0x2f1: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].adr = (m_dma[dma].adr & 0x00ff00ff) | (data << 8);
	logerror("MN10200: DMA %d adr=%06x\n", dma, m_dma[dma].adr);
	break;
	}

	case 0x282: case 0x292: case 0x2a2: case 0x2b2: case 0x2c2: case 0x2d2: case 0x2e2: case 0x2f2: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].adr = (m_dma[dma].adr & 0x0000ffff) | (data << 16);
	logerror("MN10200: DMA %d adr=%06x\n", dma, m_dma[dma].adr);
	break;
	}

	case 0x283: case 0x293: case 0x2a3: case 0x2b3: case 0x2c3: case 0x2d3: case 0x2e3: case 0x2f3:
	break;

	case 0x284: case 0x294: case 0x2a4: case 0x2b4: case 0x2c4: case 0x2d4: case 0x2e4: case 0x2f4: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].count = (m_dma[dma].count & 0x00ffff00) | data;
	logerror("MN10200: DMA %d count=%06x\n", dma, m_dma[dma].count);
	break;
	}

	case 0x285: case 0x295: case 0x2a5: case 0x2b5: case 0x2c5: case 0x2d5: case 0x2e5: case 0x2f5: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].count = (m_dma[dma].count & 0x00ff00ff) | (data << 8);
	logerror("MN10200: DMA %d count=%06x\n", dma, m_dma[dma].count);
	break;
	}

	case 0x286: case 0x296: case 0x2a6: case 0x2b6: case 0x2c6: case 0x2d6: case 0x2e6: case 0x2f6: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].count = (m_dma[dma].count & 0x0000ffff) | (data << 16);
	logerror("MN10200: DMA %d count=%06x\n", dma, m_dma[dma].count);
	break;
	}

	case 0x287: case 0x297: case 0x2a7: case 0x2b7: case 0x2c7: case 0x2d7: case 0x2e7: case 0x2f7:
	break;

	case 0x288: case 0x298: case 0x2a8: case 0x2b8: case 0x2c8: case 0x2d8: case 0x2e8: case 0x2f8: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].iadr = (m_dma[dma].iadr & 0xff00) | data;
	logerror("MN10200: DMA %d iadr=%03x\n", dma, m_dma[dma].iadr);
	break;
	}

	case 0x289: case 0x299: case 0x2a9: case 0x2b9: case 0x2c9: case 0x2d9: case 0x2e9: case 0x2f9: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].iadr = (m_dma[dma].iadr & 0x00ff) | ((data & 3) << 8);
	logerror("MN10200: DMA %d iadr=%03x\n", dma, m_dma[dma].iadr);
	break;
	}

	case 0x28a: case 0x29a: case 0x2aa: case 0x2ba: case 0x2ca: case 0x2da: case 0x2ea: case 0x2fa: {
	static const char *const trans[4] = { "M-IO", "M-M", "M-X1", "m-X2" };
	static const char *const start[32] = {
		"soft", "a/d", "ser0tx", "set0rx", "ser1tx", "ser1rx",
		"timer0", "timer1", "timer2", "timer3", "timer4", "timer5", "timer6", "timer7", "timer8", "timer9",
		"timer10u", "timer10a", "timer10b",
		"timer11u", "timer11a", "timer12b",
		"timer12a", "timer12b",
		"irq0", "irq1", "irq2", "irq3",
		"X0e", "X1e", "X0l", "X1l"
	};

	int dma = (adr-0x280) >> 4;
	m_dma[dma].ctrll = data;
	logerror("MN10200: DMA %d control ack=%s, trans=%s, start=%s\n",
			dma,
			data & 0x80 ? "level" : "pulse",
			trans[(data >> 5) & 3],
			start[data & 31]);
	break;
	}

	case 0x28b: case 0x29b: case 0x2ab: case 0x2bb: case 0x2cb: case 0x2db: case 0x2eb: case 0x2fb: {
	static const char *const tradr[4] = { "inc", "dec", "fixed", "reserved" };
	int dma = (adr-0x280) >> 4;
	m_dma[dma].ctrlh = data;
	logerror("MN10200: DMA %d control %s irq=%s %s %s dir=%s %s %s\n",
			dma,
			data & 0x80 ? "enable" : "disable",
			data & 0x40 ? "off" : "on",
			data & 0x20 ? "byte" : "word",
			data & 0x10 ? "burst" : "single",
			data & 0x08 ? "dst" : "src",
			data & 0x04 ? "continue" : "normal",
			tradr[data & 3]);
	break;
	}

	case 0x28c: case 0x29c: case 0x2ac: case 0x2bc: case 0x2cc: case 0x2dc: case 0x2ec: case 0x2fc: {
	int dma = (adr-0x280) >> 4;
	m_dma[dma].irq = data & 7;
	logerror("MN10200: DMA %d irq=%d\n", dma, data & 7);
	break;
	}

	case 0x28d: case 0x29d: case 0x2ad: case 0x2bd: case 0x2cd: case 0x2dd: case 0x2ed: case 0x2fd:
	break;

	case 0x3b0:
	log_event("MN102", "Pull-ups 0-7 = -%c%c%c%c%c%c%c",
			data & 0x40 ? '#' : '.',
			data & 0x20 ? '#' : '.',
			data & 0x10 ? '#' : '.',
			data & 0x08 ? '#' : '.',
			data & 0x04 ? '#' : '.',
			data & 0x02 ? '#' : '.',
			data & 0x01 ? '#' : '.');
	break;

	case 0x3b1:
	log_event("MN102", "Pull-ups 8-f = --%c%c%c%c%c%c",
			data & 0x20 ? '#' : '.',
			data & 0x10 ? '#' : '.',
			data & 0x08 ? '#' : '.',
			data & 0x04 ? '#' : '.',
			data & 0x02 ? '#' : '.',
			data & 0x01 ? '#' : '.');
	break;

	case 0x3b2:
	log_event("MN102", "Timer I/O 4-0 = %c%c%c%c%c",
			data & 0x10 ? 'o' : 'i',
			data & 0x08 ? 'o' : 'i',
			data & 0x04 ? 'o' : 'i',
			data & 0x02 ? 'o' : 'i',
			data & 0x01 ? 'o' : 'i');
	break;

	case 0x3b3:
	log_event("MN102", "Timer I/O 12b/a-10b/a = %c%c %c%c %c%c",
			data & 0x20 ? 'o' : 'i',
			data & 0x10 ? 'o' : 'i',
			data & 0x08 ? 'o' : 'i',
			data & 0x04 ? 'o' : 'i',
			data & 0x02 ? 'o' : 'i',
			data & 0x01 ? 'o' : 'i');
	break;

	case 0x3c0: // port 0 data
		m_io->write_byte(MN10200_PORT0, data);
		break;

	case 0x3c2: // port 2 data
		m_io->write_byte(MN10200_PORT2, data);
		break;

	case 0x3c3: // port 3 data
		m_io->write_byte(MN10200_PORT3, data);
		break;

	case 0x3e0: // port0 ddr
		m_ddr[0] = data;
		break;

	case 0x3e1: // port1 ddr
		m_ddr[1] = data;
		break;

	case 0x3e2: // port2 ddr
		m_ddr[2] = data;
		break;

	case 0x3e3: // port3 ddr
		m_ddr[3] = data;
		break;

	case 0x3f3:
/*    log_event("MN102", "Port 3 bits 4=%s 3=%s 2=%s 1=%s 0=%s",
          data & 0x10 ? data & 0x40 ? "serial_1" : "tm9" : "p34",
          data & 0x08 ? data & 0x20 ? "serial_0" : "tm8" : "p33",
          data & 0x04 ? "tm7" : "p32",
          data & 0x04 ? "tm6" : "p31",
          data & 0x04 ? "tm5" : "p30");*/
	break;


	default:
	log_event("MN102", "internal_w %04x, %02x (%03x)", adr+0xfc00, data, adr);
	break;
	}
}

UINT32 mn10200_device::mn10200_r(UINT32 adr, int type)
{
	if(type == MEM_WORD)
	{
		return mn10200_r(adr, MEM_BYTE) | (mn10200_r(adr+1, MEM_BYTE) << 8);
	}

	switch(adr) {
	case 0x00e:
	return m_iagr;

	case 0x00f:
	return 0;

	case 0x042: case 0x044: case 0x046: case 0x048: case 0x04a:
	case 0x04c: case 0x04e: case 0x050: case 0x052: case 0x054:
	return m_icrl[((adr & 0x3f)>>1)-1];

	case 0x043: case 0x045: case 0x047: case 0x049: case 0x04b:
	case 0x04d: case 0x04f: case 0x051: case 0x053: case 0x055:
	return m_icrh[((adr & 0x3f)>>1)-1];

	case 0x056:
	return 0;

	// p4i1 = tms empty line
	case 0x057:
	return 0x20;

	case 0x180: case 0x190:
	return m_serial[(adr-0x180) >> 4].ctrll;

	case 0x181: case 0x191:
	return m_serial[(adr-0x180) >> 4].ctrlh;

	case 0x182: {
	static int zz;
	return zz++;
	}

	case 0x183:
		return 0x10;

	case 0x200: case 0x201: case 0x202: case 0x203: case 0x204:
	case 0x205: case 0x206: case 0x207: case 0x208: case 0x209:
//      printf("MN10200: timer %d value read = %d\n", adr-0x200, m_simple_timer[adr-0x200].cur);
		return m_simple_timer[adr-0x200].cur;

	case 0x264: // port 1 data
		return m_io->read_byte(MN10200_PORT1);

	case 0x28c: case 0x29c: case 0x2ac: case 0x2bc: case 0x2cc: case 0x2dc: case 0x2ec: case 0x2fc:
		{
			int dma = (adr-0x280) >> 4;
			return m_dma[dma].irq;
		}

	case 0x3c0: // port 0 data
		return m_io->read_byte(MN10200_PORT0);

	case 0x3c2: // port 2 data
		return m_io->read_byte(MN10200_PORT2);

	case 0x3c3: // port 3 data
		return m_io->read_byte(MN10200_PORT3);

	default:
	log_event("MN102", "internal_r %04x (%03x)", adr+0xfc00, adr);
	}

	return 0;
}


offs_t mn10200_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( mn10200 );
	return CPU_DISASSEMBLE_NAME(mn10200)(this, buffer, pc, oprom, opram, options);
}
