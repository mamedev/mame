// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    Peripherals and improvements by R. Belmont and hap

*/

#include "emu.h"
#include "debugger.h"
#include "mn10200.h"

#define log_write(...)
#define log_event(...)

enum mn10200_flag
{
	FLAG_ZF  = 0x0001, // zero flag
	FLAG_NF  = 0x0002, // negative flag
	FLAG_CF  = 0x0004, // carry flag
	FLAG_VF  = 0x0008, // overflow flag
	FLAG_ZX  = 0x0010, // extended zero flag
	FLAG_NX  = 0x0020, // extended negative flag
	FLAG_CX  = 0x0040, // extended carry flag
	FLAG_VX  = 0x0080, // extended overflow flag
	FLAG_IM0 = 0x0100, // interrupt mask
	FLAG_IM1 = 0x0200, // "
	FLAG_IM2 = 0x0400, // "
	FLAG_IE  = 0x0800, // interrupt enable
	FLAG_S0  = 0x1000, // software bit
	FLAG_S1  = 0x2000, // software bit
	FLAG_D14 = 0x4000, // ?
	FLAG_D15 = 0x8000  // ?
};


const device_type MN1020012A = &device_creator<mn1020012a_device>;

// internal memory maps
static ADDRESS_MAP_START( mn1020012a_internal_map, AS_PROGRAM, 16, mn10200_device )
	AM_RANGE(0x00fc00, 0x00ffff) AM_READWRITE8(io_control_r, io_control_w, 0xffff)
ADDRESS_MAP_END


// device definitions
mn1020012a_device::mn1020012a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mn10200_device(mconfig, MN1020012A, "MN1020012A", tag, owner, clock, ADDRESS_MAP_NAME(mn1020012a_internal_map), "mn1020012a", __FILE__)
{ }


// disasm
void mn10200_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "S=%d irq=%s im=%d %c%c%c%c %c%c%c%c",
				(m_psw >> 12) & 3,
				m_psw & FLAG_IE ? "on " : "off",
				(m_psw >> 8) & 7,
				m_psw & FLAG_VX ? 'V' : '-',
				m_psw & FLAG_CX ? 'C' : '-',
				m_psw & FLAG_NX ? 'N' : '-',
				m_psw & FLAG_ZX ? 'Z' : '-',
				m_psw & FLAG_VF ? 'v' : '-',
				m_psw & FLAG_CF ? 'c' : '-',
				m_psw & FLAG_NF ? 'n' : '-',
				m_psw & FLAG_ZF ? 'z' : '-');
			break;
	}
}

offs_t mn10200_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( mn10200 );
	return CPU_DISASSEMBLE_NAME(mn10200)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	MN10200_PC = 0,
	MN10200_PSW,
	MN10200_MDR,
	MN10200_D0,
	MN10200_D1,
	MN10200_D2,
	MN10200_D3,
	MN10200_A0,
	MN10200_A1,
	MN10200_A2,
	MN10200_A3,
	MN10200_NMICR,
	MN10200_IAGR
};

void mn10200_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	// resolve callbacks
	m_read_port0.resolve_safe(0xff);
	m_read_port1.resolve_safe(0xff);
	m_read_port2.resolve_safe(0xff);
	m_read_port3.resolve_safe(0xff);
	m_read_port4.resolve_safe(0xff);

	m_write_port0.resolve_safe();
	m_write_port1.resolve_safe();
	m_write_port2.resolve_safe();
	m_write_port3.resolve_safe();
	m_write_port4.resolve_safe();

	// init and register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_d));
	save_item(NAME(m_a));
	save_item(NAME(m_psw));
	save_item(NAME(m_mdr));

	// interrupts
	memset(&m_icrl, 0, sizeof(m_icrl));
	memset(&m_icrh, 0, sizeof(m_icrh));

	save_item(NAME(m_nmicr));
	save_item(NAME(m_iagr));
	save_item(NAME(m_extmdl));
	save_item(NAME(m_extmdh));
	save_item(NAME(m_icrl));
	save_item(NAME(m_icrh));
	save_item(NAME(m_possible_irq));

	// timers
	m_sysclock_base = attotime::from_hz(unscaled_clock() / 2);

	for (int tmr = 0; tmr < MN10200_NUM_TIMERS_8BIT; tmr++)
	{
		m_timer_timers[tmr] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mn10200_device::simple_timer_cb), this));
		m_timer_timers[tmr]->adjust(attotime::never, tmr);
	}

	for (int i = 0; i < MN10200_NUM_TIMERS_8BIT; i++)
	{
		m_simple_timer[i].mode = 0;
		m_simple_timer[i].base = 0;
		m_simple_timer[i].cur = 0;

		save_item(NAME(m_simple_timer[i].mode), i);
		save_item(NAME(m_simple_timer[i].base), i);
		save_item(NAME(m_simple_timer[i].cur), i);
	}

	for (int i = 0; i < MN10200_NUM_PRESCALERS; i++)
	{
		m_prescaler[i].mode = 0;
		m_prescaler[i].base = 0;
		m_prescaler[i].cur = 0;

		save_item(NAME(m_prescaler[i].mode), i);
		save_item(NAME(m_prescaler[i].base), i);
		save_item(NAME(m_prescaler[i].cur), i);
	}

	// dma
	for (int i = 0; i < 8; i++)
	{
		m_dma[i].adr = 0;
		m_dma[i].count = 0;
		m_dma[i].iadr = 0;
		m_dma[i].ctrll = 0;
		m_dma[i].ctrlh = 0;
		m_dma[i].irq = 0;

		save_item(NAME(m_dma[i].adr), i);
		save_item(NAME(m_dma[i].count), i);
		save_item(NAME(m_dma[i].iadr), i);
		save_item(NAME(m_dma[i].ctrll), i);
		save_item(NAME(m_dma[i].ctrlh), i);
		save_item(NAME(m_dma[i].irq), i);
	}

	// serial
	for (int i = 0; i < 2; i++)
	{
		m_serial[i].ctrll = 0;
		m_serial[i].ctrlh = 0;
		m_serial[i].buf = 0;

		save_item(NAME(m_serial[i].ctrll), i);
		save_item(NAME(m_serial[i].ctrlh), i);
		save_item(NAME(m_serial[i].buf), i);
	}

	// ports
	m_p4 = 0xf;

	save_item(NAME(m_pplul));
	save_item(NAME(m_ppluh));
	save_item(NAME(m_p3md));
	save_item(NAME(m_p4));

	for (int i = 0; i < 4; i++)
	{
		m_port[i].out = 0;
		m_port[i].dir = 0;

		save_item(NAME(m_port[i].out), i);
		save_item(NAME(m_port[i].dir), i);
	}

	// register for debugger
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



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mn10200_device::device_reset()
{
	change_pc(0x80000);
	m_psw = 0;

	// note: officially, these registers are reset to 'undefined'
	memset(m_d, 0, sizeof(m_d));
	memset(m_a, 0, sizeof(m_a));
	m_mdr = 0;

	// reset internal peripherals
	m_nmicr = 0;
	m_iagr = 0;
	write_mem16(0xfc00, 0x8000);
	write_mem16(0xfc02, 0x0737);

	// need to clear them twice since some rely on the value of others
	for (int i = 0; i < 2; i++)
		for (int address = 0xfc04; address < 0x10000; address++)
			write_mem16(address, 0);

	m_possible_irq = false;
}



//-------------------------------------------------
//  interrupts
//-------------------------------------------------

void mn10200_device::take_irq(int level, int group)
{
	m_cycles -= 7;

	write_mem24(m_a[3] - 4, m_pc);
	write_mem16(m_a[3] - 6, m_psw);
	m_a[3] -= 6;
	change_pc(0x80008);
	m_psw = (m_psw & 0xf0ff) | (level << 8);
	m_iagr = group;
}

void mn10200_device::check_irq()
{
	if (!m_nmicr && !(m_psw & FLAG_IE))
		return;

	int level = m_psw >> 8 & 7;
	int group = 0;

	// find highest valid level
	for (int i = 1; i < 11; i++)
	{
		if (m_icrl[i] >> 4 & m_icrh[i] & 0xf && (m_icrh[i] >> 4 & 7) < level)
		{
			level = m_icrh[i] >> 4 & 7;
			group = i;
		}
	}

	// take interrupt
	if (m_nmicr && group)
		take_irq(level, 0);
	else if (m_nmicr)
		take_irq(0, 0);
	else if (group)
		take_irq(level, group);

	return;
}

void mn10200_device::check_ext_irq()
{
	for (int i = 0; i < 4; i++)
	{
		// active irq at low or high? (not edge triggered)
		if ((m_p4 >> i & 1) == (m_extmdl >> (i * 2) & 3))
			m_icrl[8] |= (1 << (4 + i));
	}

	m_possible_irq = true;
}

void mn10200_device::execute_set_input(int irqnum, int state)
{
	// take an external IRQ
	assert(((UINT32)irqnum) < MN10200_MAX_EXT_IRQ);

	int pin = state ? 0 : 1;
	int old = m_p4 >> irqnum & 1;
	bool active = false;

	switch (m_extmdl >> (irqnum * 2) & 3)
	{
		// 'L' level
		case 0:
			active = (pin == 0);
			break;

		// 'H' level
		case 1:
			active = (pin == 1);
			break;

		// falling edge
		case 2:
			active = (pin == 0 && old == 1);
			break;

		// rising edge
		case 3:
			active = (pin == 1 && old == 0);
			break;
	}

	m_p4 &= ~(1 << irqnum);
	m_p4 |= pin << irqnum;

	if (active)
	{
		m_icrl[8] |= (1 << (4 + irqnum));
		m_possible_irq = true;
	}
}



//-------------------------------------------------
//  timers
//-------------------------------------------------

int mn10200_device::timer_tick_simple(int tmr)
{
	int next = tmr + 1;

	// is it a cascaded timer, and enabled?
	if (next < MN10200_NUM_TIMERS_8BIT && m_simple_timer[next].mode & 0x83 && (m_simple_timer[next].mode & 0x83) == 0x81)
	{
		// did it underflow?
		if (--m_simple_timer[next].cur == 0xff)
		{
			// cascaded underflow?
			if (timer_tick_simple(next) != 2)
			{
				m_simple_timer[next].cur = m_simple_timer[next].base;
				return 1;
			}
		}

		return 2;
	}
	else
	{
		// trigger irq
		m_icrl[1 + (tmr >> 2)] |= (1 << (4 + (tmr & 3)));
		m_possible_irq = true;

		return 0;
	}
}

void mn10200_device::refresh_timer(int tmr)
{
	// 0: external pin
	// 1: cascaded (handled elsewhere)
	// 2: prescaler 0
	// 3: prescaler 1
	int p = m_simple_timer[tmr].mode & 1;

	// enabled, and source is prescaler?
	if ((m_simple_timer[tmr].mode & 0x82) == 0x82 && m_prescaler[p].mode & 0x80)
	{
		attotime period = m_sysclock_base * (m_prescaler[p].base + 1) * (m_simple_timer[tmr].cur + 1);
		m_timer_timers[tmr]->adjust(period, tmr);
	}
	else
	{
		m_timer_timers[tmr]->adjust(attotime::never, tmr);
	}
}

void mn10200_device::refresh_all_timers()
{
	for (int tmr = 0; tmr < MN10200_NUM_TIMERS_8BIT; tmr++)
		refresh_timer(tmr);
}

TIMER_CALLBACK_MEMBER( mn10200_device::simple_timer_cb )
{
	int tmr = param;

	// handle our expiring and also tick our cascaded children
	if (timer_tick_simple(tmr) == 2)
		m_simple_timer[tmr].cur = 0xff; // cascaded and no underflow occurred
	else
		m_simple_timer[tmr].cur = m_simple_timer[tmr].base;

	// refresh this timer
	refresh_timer(tmr);
}



//-------------------------------------------------
//  opcode helpers
//-------------------------------------------------

void mn10200_device::illegal(UINT8 prefix, UINT8 op)
{
	logerror("MN10200: illegal opcode %x %x @ PC=%x\n", prefix, op, m_pc);
	m_nmicr |= 2;
}

UINT32 mn10200_device::do_add(UINT32 a, UINT32 b, UINT32 c)
{
	UINT32 r = (a & 0xffffff) + (b & 0xffffff) + c;

	m_psw &= 0xff00;
	if ((a^r) & (b^r) & 0x00800000)
		m_psw |= FLAG_VX;
	if (r & 0x01000000)
		m_psw |= FLAG_CX;
	if (r & 0x00800000)
		m_psw |= FLAG_NX;
	if ((r & 0x00ffffff) == 0)
		m_psw |= FLAG_ZX;
	if ((a^r) & (b^r) & 0x00008000)
		m_psw |= FLAG_VF;
	if (((a & 0xffff) + (b & 0xffff) + c) & 0x00010000)
		m_psw |= FLAG_CF;
	if (r & 0x00008000)
		m_psw |= FLAG_NF;
	if ((r & 0x0000ffff) == 0)
		m_psw |= FLAG_ZF;

	return r;
}

UINT32 mn10200_device::do_sub(UINT32 a, UINT32 b, UINT32 c)
{
	UINT32 r = (a & 0xffffff) - (b & 0xffffff) - c;

	m_psw &= 0xff00;
	if ((a^b) & (a^r) & 0x00800000)
		m_psw |= FLAG_VX;
	if (r & 0x01000000)
		m_psw |= FLAG_CX;
	if (r & 0x00800000)
		m_psw |= FLAG_NX;
	if ((r & 0x00ffffff) == 0)
		m_psw |= FLAG_ZX;
	if ((a^b) & (a^r) & 0x00008000)
		m_psw |= FLAG_VF;
	if (((a & 0xffff) - (b & 0xffff) - c) & 0x00010000)
		m_psw |= FLAG_CF;
	if (r & 0x00008000)
		m_psw |= FLAG_NF;
	if ((r & 0x0000ffff) == 0)
		m_psw |= FLAG_ZF;

	return r;
}

void mn10200_device::test_nz16(UINT16 v)
{
	m_psw &= 0xfff0;
	if (v & 0x8000)
		m_psw |= FLAG_NF;
	if (v == 0)
		m_psw |= FLAG_ZF;
}

void mn10200_device::do_jsr(UINT32 to, UINT32 ret)
{
	m_a[3] -= 4;
	write_mem24(m_a[3], ret);
	change_pc(to);
}

void mn10200_device::do_branch(int condition)
{
	if (condition)
	{
		m_cycles -= 1;
		change_pc(m_pc + (INT8)read_arg8(m_pc));
	}
}



//-------------------------------------------------
//  execute loop
//-------------------------------------------------

void mn10200_device::execute_run()
{
	while (m_cycles > 0)
	{
	// internal peripheral, external pin, or prev instruction may have changed irq state
	while (m_possible_irq)
	{
		m_possible_irq = false;
		check_irq();
	}

	debugger_instruction_hook(this, m_pc);

	m_cycles -= 1;
	UINT8 op = read_arg8(m_pc);
	m_pc += 1;

	// main opcodes
	switch (op)
	{
		// mov dm, (an)
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			write_mem16(m_a[op>>2&3], m_d[op&3]);
			break;

		// movb dm, (an)
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			write_mem8(m_a[op>>2&3], m_d[op&3]); // note: error in manual
			break;

		// mov (an), dm
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			m_d[op&3] = (INT16)read_mem16(m_a[op>>2&3]);
			break;

		// movbu (an), dm
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			m_d[op&3] = read_mem8(m_a[op>>2&3]);
			break;

		// mov dm, (d8, an)
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			write_mem16((m_a[op>>2&3] + (INT8)read_arg8(m_pc)), m_d[op&3]);
			m_pc += 1;
			break;

		// mov am, (d8, an)
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			m_cycles -= 1;
			write_mem24((m_a[op>>2&3] + (INT8)read_arg8(m_pc)), m_a[op&3]);
			m_pc += 1;
			break;

		// mov (d8, an), dm
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			m_d[op&3] = (INT16)read_mem16(m_a[op>>2&3] + (INT8)read_arg8(m_pc));
			m_pc += 1;
			break;

		// mov (d8, an), am
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_cycles -= 1;
			m_a[op&3] = read_mem24(m_a[op>>2&3] + (INT8)read_arg8(m_pc));
			m_pc += 1;
			break;

		// mov dn, dm
		case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
			m_d[op&3] = m_d[op>>2&3];
			break;

		// mov imm8, dn
		case 0x80: case 0x85: case 0x8a: case 0x8f:
			m_d[op&3] = (INT8)read_arg8(m_pc);
			m_pc += 1;
			break;

		// add dn, dm
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			m_d[op&3] = do_add(m_d[op&3], m_d[op>>2&3]);
			break;

		// sub dn, dm
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			m_d[op&3] = do_sub(m_d[op&3], m_d[op>>2&3]);
			break;

		// extx dn
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
			m_d[op&3] = (INT16)m_d[op&3];
			break;

		// extxu dn
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			m_d[op&3] = (UINT16)m_d[op&3];
			break;

		// extxb dn
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
			m_d[op&3] = (INT8)m_d[op&3];
			break;

		// extxbu dn
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			m_d[op&3] = (UINT8)m_d[op&3];
			break;

		// mov dn, (abs16)
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
			write_mem16(read_arg16(m_pc), m_d[op&3]);
			m_pc += 2;
			break;

		// movb dn, (abs16)
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			write_mem8(read_arg16(m_pc), m_d[op&3]);
			m_pc += 2;
			break;

		// mov (abs16), dn
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			m_d[op&3] = (INT16)read_mem16(read_arg16(m_pc));
			m_pc += 2;
			break;

		// movbu (abs16), dn
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			m_d[op&3] = read_mem8(read_arg16(m_pc));
			m_pc += 2;
			break;

		// add imm8, an
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			m_a[op&3] = do_add(m_a[op&3], (INT8)read_arg8(m_pc));
			m_pc += 1;
			break;

		// add imm8, dn
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			m_d[op&3] = do_add(m_d[op&3], (INT8)read_arg8(m_pc));
			m_pc += 1;
			break;

		// cmp imm8, dn
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
			do_sub(m_d[op&3], (INT8)read_arg8(m_pc));
			m_pc += 1;
			break;

		// mov imm16, an
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
			m_a[op&3] = read_arg16(m_pc);
			m_pc += 2;
			break;

		// blt label8
		case 0xe0:
			do_branch(((m_psw & (FLAG_NF|FLAG_VF)) == FLAG_NF) || ((m_psw & (FLAG_NF|FLAG_VF)) == FLAG_VF)); // (VF^NF)=1
			m_pc += 1;
			break;

		// bgt label8
		case 0xe1:
			do_branch(((m_psw & (FLAG_ZF|FLAG_NF|FLAG_VF)) == 0) || ((m_psw & (FLAG_ZF|FLAG_NF|FLAG_VF)) == (FLAG_NF|FLAG_VF))); // ((VF^NF)|ZF)=0
			m_pc += 1;
			break;

		// bge label8
		case 0xe2:
			do_branch(((m_psw & (FLAG_NF|FLAG_VF)) == 0) || ((m_psw & (FLAG_NF|FLAG_VF)) == (FLAG_NF|FLAG_VF))); // (VF^NF)=0
			m_pc += 1;
			break;

		// ble label8
		case 0xe3:
			do_branch((m_psw & FLAG_ZF) || ((m_psw & (FLAG_NF|FLAG_VF)) == FLAG_NF) || ((m_psw & (FLAG_NF|FLAG_VF)) == FLAG_VF)); // ((VF^NF)|ZF)=1
			m_pc += 1;
			break;

		// bcs label8
		case 0xe4:
			do_branch(m_psw & FLAG_CF); // CF=1
			m_pc += 1;
			break;

		// bhi label8
		case 0xe5:
			do_branch(!(m_psw & (FLAG_ZF|FLAG_CF))); // (CF|ZF)=0
			m_pc += 1;
			break;

		// bcc label8
		case 0xe6:
			do_branch(!(m_psw & FLAG_CF)); // CF=0
			m_pc += 1;
			break;

		// bls label8
		case 0xe7:
			do_branch(m_psw & (FLAG_ZF|FLAG_CF)); // (CF|ZF)=1
			m_pc += 1;
			break;

		// beq label8
		case 0xe8:
			do_branch(m_psw & FLAG_ZF); // ZF=1
			m_pc += 1;
			break;

		// bne label8
		case 0xe9:
			do_branch(!(m_psw & FLAG_ZF)); // ZF=0
			m_pc += 1;
			break;

		// bra label8
		case 0xea:
			do_branch();
			m_pc += 1;
			break;

		// rti
		case 0xeb:
			m_cycles -= 5;
			m_psw = read_mem16(m_a[3]);
			change_pc(read_mem24(m_a[3] + 2));
			m_a[3] += 6;
			m_possible_irq = true;
			break;

		// cmp imm16, an
		case 0xec: case 0xed: case 0xee: case 0xef:
			do_sub(m_a[op&3], read_arg16(m_pc));
			m_pc += 2;
			break;

		// nop
		case 0xf6:
			break;

		// mov imm16, dn
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
			m_d[op&3] = (INT16)read_arg16(m_pc);
			m_pc += 2;
			break;

		// jmp label16
		case 0xfc:
			m_cycles -= 1;
			change_pc(m_pc + 2 + (INT16)read_arg16(m_pc));
			break;

		// jsr label16
		case 0xfd:
			m_cycles -= 3;
			do_jsr(m_pc + 2 + (INT16)read_arg16(m_pc), m_pc + 2);
			break;

		// rts
		case 0xfe:
			m_cycles -= 4;
			change_pc(read_mem24(m_a[3]));
			m_a[3] += 4;
			break;

		default:
			illegal(0, op);
			m_possible_irq = true;
			break;



		// extended code f0 (2 bytes)
		case 0xf0:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;

		switch (op)
		{
		// jmp (an)
		case 0x00: case 0x04: case 0x08: case 0x0c:
			m_cycles -= 1;
			change_pc(m_a[op>>2&3]);
			break;

		// jsr (an)
		case 0x01: case 0x05: case 0x09: case 0x0d:
			m_cycles -= 3;
			do_jsr(m_a[op>>2&3], m_pc);
			break;

		// bset dm, (an)
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		{
			m_cycles -= 3;
			UINT8 v = read_mem8(m_a[op>>2&3]);
			test_nz16(v & m_d[op&3]);
			write_mem8(m_a[op>>2&3], v | m_d[op&3]);
			break;
		}

		// bclr dm, (an)
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			m_cycles -= 3;
			UINT8 v = read_mem8(m_a[op>>2&3]);
			test_nz16(v & m_d[op&3]);
			write_mem8(m_a[op>>2&3], v & ~m_d[op&3]);
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
			m_d[op&3] = (INT8)read_mem8(m_a[op>>2&3] + m_d[op>>4&3]);
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
			m_d[op&3] = read_mem8(m_a[op>>2&3] + m_d[op>>4&3]);
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
			write_mem8(m_a[op>>2&3] + m_d[op>>4&3], m_d[op&3]);
			break;

		default:
			illegal(0xf0, op);
			m_possible_irq = true;
			break;
		}
		break;



		// extended code f1 (2 bytes)
		case 0xf1:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;

		switch (op & 0xc0)
		{
		// mov (di, an), am
		case 0x00:
			m_cycles -= 1;
			m_a[op&3] = read_mem24(m_a[op>>2&3] + m_d[op>>4&3]);
			break;

		// mov (di, an), dm
		case 0x40:
			m_d[op&3] = (INT16)read_mem16(m_a[op>>2&3] + m_d[op>>4&3]);
			break;

		// mov am, (di, an)
		case 0x80:
			m_cycles -= 1;
			write_mem24(m_a[op>>2&3] + m_d[op>>4&3], m_a[op&3]);
			break;

		// mov dm, (di, an)
		case 0xc0:
			write_mem16(m_a[op>>2&3] + m_d[op>>4&3], m_d[op&3]);
			break;
		}
		break;



		// extended code f2 (2 bytes)
		case 0xf2:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;

		switch (op & 0xf0)
		{
		// add dm, an
		case 0x00:
			m_a[op&3] = do_add(m_a[op&3], m_d[op>>2&3]);
			break;

		// sub dm, an
		case 0x10:
			m_a[op&3] = do_sub(m_a[op&3], m_d[op>>2&3]);
			break;

		// cmp dm, an
		case 0x20:
			do_sub(m_a[op&3], m_d[op>>2&3]);
			break;

		// mov dm, an
		case 0x30:
			m_a[op&3] = m_d[op>>2&3];
			break;

		// add an, am
		case 0x40:
			m_a[op&3] = do_add(m_a[op&3], m_a[op>>2&3]);
			break;

		// sub an, am
		case 0x50:
			m_a[op&3] = do_sub(m_a[op&3], m_a[op>>2&3]);
			break;

		// cmp an, am
		case 0x60:
			do_sub(m_a[op&3], m_a[op>>2&3]);
			break;

		// mov an, am
		case 0x70:
			m_a[op&3] = m_a[op>>2&3];
			break;

		// addc dn, dm
		case 0x80:
		{
			UINT16 mask0 = ~FLAG_ZF | (m_psw & FLAG_ZF);
			m_d[op&3] = do_add(m_d[op&3], m_d[op>>2&3], (m_psw & FLAG_CF) ? 1 : 0);
			m_psw &= mask0; // ZF can only be set if it was set before the operation
			break;
		}

		// subc dn, dm
		case 0x90:
		{
			UINT16 mask0 = ~FLAG_ZF | (m_psw & FLAG_ZF);
			m_d[op&3] = do_sub(m_d[op&3], m_d[op>>2&3], (m_psw & FLAG_CF) ? 1 : 0);
			m_psw &= mask0; // ZF can only be set if it was set before the operation
			break;
		}

		// add an, dm
		case 0xc0:
			m_d[op&3] = do_add(m_d[op&3], m_a[op>>2&3]);
			break;

		// sub an, dm
		case 0xd0:
			m_d[op&3] = do_sub(m_d[op&3], m_a[op>>2&3]);
			break;

		// cmp an, dm
		case 0xe0:
			do_sub(m_d[op&3], m_a[op>>2&3]);
			break;

		// mov an, dm
		case 0xf0:
			m_d[op&3] = m_a[op>>2&3];
			break;

		default:
			illegal(0xf2, op);
			m_possible_irq = true;
			break;
		}
		break;



		// extended code f3 (2 bytes)
		case 0xf3:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;

		switch (op)
		{
		// and dn, dm
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			test_nz16(m_d[op&3] &= 0xff0000 | m_d[op>>2&3]);
			break;

		// or dn, dm
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			test_nz16(m_d[op&3] |= 0x00ffff & m_d[op>>2&3]);
			break;

		// xor dn, dm
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			test_nz16(m_d[op&3] ^= 0x00ffff & m_d[op>>2&3]);
			break;

		// rol dn
		case 0x30: case 0x31: case 0x32: case 0x33:
		{
			UINT32 d = m_d[op&3];
			test_nz16(m_d[op&3] = (d & 0xff0000) | ((d << 1) & 0x00fffe) | ((m_psw & FLAG_CF) ? 1 : 0));
			if (d & 0x8000)
				m_psw |= FLAG_CF;
			break;
		}

		// ror dn
		case 0x34: case 0x35: case 0x36: case 0x37:
		{
			UINT32 d = m_d[op&3];
			test_nz16(m_d[op&3] = (d & 0xff0000) | ((d >> 1) & 0x007fff) | ((m_psw & FLAG_CF) ? 0x8000 : 0));
			if (d & 1)
				m_psw |= FLAG_CF;
			break;
		}

		// asr dn
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		{
			UINT32 d = m_d[op&3];
			test_nz16(m_d[op&3] = (d & 0xff8000) | ((d >> 1) & 0x007fff));
			if (d & 1)
				m_psw |= FLAG_CF;
			break;
		}

		// lsr dn
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			UINT32 d = m_d[op&3];
			test_nz16(m_d[op&3] = (d & 0xff0000) | ((d >> 1) & 0x007fff));
			if (d & 1)
				m_psw |= FLAG_CF;
			break;
		}

		// mul dn, dm
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		{
			m_cycles -= 10;
			UINT32 res = ((INT16)m_d[op&3]) * ((INT16)m_d[op>>2&3]);
			m_d[op&3] = res & 0xffffff;
			m_psw &= 0xff00; // f4 is undefined
			if (res & 0x80000000)
				m_psw |= FLAG_NF;
			else if (res == 0)
				m_psw |= FLAG_ZF;
			m_mdr = res >> 16;
			break;
		}

		// mulu dn, dm
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		{
			m_cycles -= 10;
			UINT32 res = ((UINT16)m_d[op&3]) * ((UINT16)m_d[op>>2&3]);
			m_d[op&3] = res & 0xffffff;
			m_psw &= 0xff00; // f4 is undefined
			if (res & 0x80000000)
				m_psw |= FLAG_NF;
			else if (res == 0)
				m_psw |= FLAG_ZF;
			m_mdr = res >> 16;
			break;
		}

		// divu dn, dm
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		{
			UINT32 n, d, q, r;
			m_cycles -= 11;
			m_psw &= 0xff00; // f7 may be undefined

			n = (m_mdr << 16) | (UINT16)m_d[op&3];
			d = (UINT16)m_d[op>>2&3];
			if (d == 0)
			{
				// divide by 0
				m_psw |= FLAG_VF;
				break;
			}
			q = n / d;
			r = n % d;
			if (q >= 0x10000)
			{
				// overflow (Dm and MDR are undefined)
				m_psw |= FLAG_VF;
				break;
			}
			m_d[op&3] = q;
			m_mdr = r;
			if (q == 0)
				m_psw |= FLAG_ZF | FLAG_ZX;
			if (q & 0x8000)
				m_psw |= FLAG_NF;
			break;
		}

		// cmp dn, dm
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			do_sub(m_d[op&3], m_d[op>>2&3]);
			break;

		// mov dn, mdr
		case 0xc0: case 0xc4: case 0xc8: case 0xcc:
			m_mdr = m_d[op>>2&3];
			break;

		// ext dn
		case 0xc1: case 0xc5: case 0xc9: case 0xcd:
			m_cycles -= 1;
			m_mdr = (m_d[op>>2&3] & 0x8000) ? 0xffff : 0x0000;
			break;

		// mov dn, psw
		case 0xd0: case 0xd4: case 0xd8: case 0xdc:
			m_cycles -= 1;
			m_psw = m_d[op>>2&3];
			m_possible_irq = true;
			break;

		// mov mdr, dn
		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
			m_d[op&3] = m_mdr;
			break;

		// not dn
		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			test_nz16(m_d[op&3] ^= 0x00ffff);
			break;

		// mov psw, dn
		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
			m_d[op&3] = m_psw;
			break;

		default:
			illegal(0xf3, op);
			m_possible_irq = true;
			break;
		}
		break;



		// extended code f4 (5 bytes)
		case 0xf4:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;
			m_cycles -= 1;

		switch (op)
		{
		// mov dm, (d24, an)
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			write_mem16(read_arg24(m_pc) + m_a[op>>2&3], m_d[op&3]);
			break;

		// mov am, (d24, an)
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_cycles -= 1;
			write_mem24(read_arg24(m_pc) + m_a[op>>2&3], m_a[op&3]);
			break;

		// movb dm, (d24, an)
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			write_mem8(read_arg24(m_pc) + m_a[op>>2&3], m_d[op&3]);
			break;

		// movx dm, (d24, an)
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			m_cycles -= 1;
			write_mem24(read_arg24(m_pc) + m_a[op>>2&3], m_d[op&3]);
			break;

		// mov dn, (abs24)
		case 0x40: case 0x41: case 0x42: case 0x43:
			write_mem16(read_arg24(m_pc), m_d[op&3]);
			break;

		// movb dn, (abs24)
		case 0x44: case 0x45: case 0x46: case 0x47:
			write_mem8(read_arg24(m_pc), m_d[op&3]);
			break;

		// mov an, (abs24)
		case 0x50: case 0x51: case 0x52: case 0x53:
			m_cycles -= 1;
			write_mem24(read_arg24(m_pc), m_a[op&3]);
			break;

		// add imm24, dn
		case 0x60: case 0x61: case 0x62: case 0x63:
			m_d[op&3] = do_add(m_d[op&3], read_arg24(m_pc));
			break;

		// add imm24, an
		case 0x64: case 0x65: case 0x66: case 0x67:
			m_a[op&3] = do_add(m_a[op&3], read_arg24(m_pc));
			break;

		// sub imm24, dn
		case 0x68: case 0x69: case 0x6a: case 0x6b:
			m_d[op&3] = do_sub(m_d[op&3], read_arg24(m_pc));
			break;

		// sub imm24, an
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			m_a[op&3] = do_sub(m_a[op&3], read_arg24(m_pc));
			break;

		// mov imm24, dn
		case 0x70: case 0x71: case 0x72: case 0x73:
			m_d[op&3] = read_arg24(m_pc);
			break;

		// mov imm24, an
		case 0x74: case 0x75: case 0x76: case 0x77:
			m_a[op&3] = read_arg24(m_pc);
			break;

		// cmp imm24, dn
		case 0x78: case 0x79: case 0x7a: case 0x7b:
			do_sub(m_d[op&3], read_arg24(m_pc));
			break;

		// cmp imm24, an
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			do_sub(m_a[op&3], read_arg24(m_pc));
			break;

		// mov (d24, an), dm
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			m_d[op&3] = (INT16)read_mem16(m_a[op>>2&3] + read_arg24(m_pc));
			break;

		// movbu (d24, an), dm
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			m_d[op&3] = read_mem8(m_a[op>>2&3] + read_arg24(m_pc));
			break;

		// movb (d24, an), dm
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			m_d[op&3] = (INT8)read_mem8(m_a[op>>2&3] + read_arg24(m_pc));
			break;

		// movx (d24, an), dm
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			m_cycles -= 1;
			m_d[op&3] = read_mem24(m_a[op>>2&3] + read_arg24(m_pc));
			break;

		// mov (abs24), dn
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
			m_d[op&3] = (INT16)read_mem16(read_arg24(m_pc));
			break;

		// movb (abs24), dn
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			m_d[op&3] = (INT8)read_mem8(read_arg24(m_pc));
			break;

		// movbu (abs24), dn
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			m_d[op&3] = read_mem8(read_arg24(m_pc));
			break;

		// mov (abs24), an
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			m_cycles -= 1;
			m_a[op&3] = read_mem24(read_arg24(m_pc));
			break;

		// jmp label24
		case 0xe0:
			m_cycles -= 1;
			change_pc(m_pc + read_arg24(m_pc));
			break;

		// jsr label24
		case 0xe1:
			m_cycles -= 2;
			do_jsr(m_pc + read_arg24(m_pc), m_pc + 3);
			break;

		// mov (d24, an), am
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
			m_cycles -= 1;
			m_a[op&3] = read_mem24(m_a[op>>2&3] + read_arg24(m_pc));
			break;

		default:
			illegal(0xf4, op);
			m_possible_irq = true;
			break;
		}
		m_pc += 3;
		break;



		// extended code f5 (3 bytes)
		case 0xf5:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;

		switch (op)
		{
		// and imm8, dn
		case 0x00: case 0x01: case 0x02: case 0x03:
			test_nz16(m_d[op&3] &= 0xff0000 | read_arg8(m_pc));
			break;

		// btst imm8, dn
		case 0x04: case 0x05: case 0x06: case 0x07:
			test_nz16(m_d[op&3] & read_arg8(m_pc));
			break;

		// or imm8, dn
		case 0x08: case 0x09: case 0x0a: case 0x0b:
			test_nz16(m_d[op&3] |= read_arg8(m_pc));
			break;

		// addnf imm8, an
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			m_a[op&3] = m_a[op&3] + (INT8)read_arg8(m_pc);
			break;

		// movb dm, (d8, an)
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			write_mem8(m_a[op>>2&3] + (INT8)read_arg8(m_pc), m_d[op&3]);
			break;

		// movb (d8, an), dm
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			m_d[op&3] = (INT8)read_mem8(m_a[op>>2&3] + (INT8)read_arg8(m_pc));
			break;

		// movbu (d8, an), dm
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			m_d[op&3] = read_mem8(m_a[op>>2&3] + (INT8)read_arg8(m_pc));
			break;

		// movx dm, (d8, an)
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			m_cycles -= 1;
			write_mem24(m_a[op>>2&3] + (INT8)read_arg8(m_pc), m_d[op&3]);
			break;

		// movx (d8, an), dm
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_cycles -= 1;
			m_d[op&3] = read_mem24(m_a[op>>2&3] + (INT8)read_arg8(m_pc));
			break;

		// bltx label8
		case 0xe0:
			do_branch(((m_psw & (FLAG_NX|FLAG_VX)) == FLAG_NX) || ((m_psw & (FLAG_NX|FLAG_VX)) == FLAG_VX)); // (VX^NX)=1
			break;

		// bgtx label8
		case 0xe1:
			do_branch(((m_psw & (FLAG_ZX|FLAG_NX|FLAG_VX)) == 0) || ((m_psw & (FLAG_ZX|FLAG_NX|FLAG_VX)) == (FLAG_NX|FLAG_VX))); // ((VX^NX)|ZX)=0
			break;

		// bgex label8
		case 0xe2:
			do_branch(((m_psw & (FLAG_NX|FLAG_VX)) == 0) || ((m_psw & (FLAG_NX|FLAG_VX)) == (FLAG_NX|FLAG_VX))); // (VX^NX)=0
			break;

		// blex label8
		case 0xe3:
			do_branch((m_psw & FLAG_ZX) || ((m_psw & (FLAG_NX|FLAG_VX)) == FLAG_NX) || ((m_psw & (FLAG_NX|FLAG_VX)) == FLAG_VX)); // ((VX^NX)|ZX)=1
			break;

		// bcsx label8
		case 0xe4:
			do_branch(m_psw & FLAG_CX); // CX=1
			break;

		// bhix label8
		case 0xe5:
			do_branch(!(m_psw & (FLAG_ZX|FLAG_CX))); // (CX|ZX)=0
			break;

		// bccx label8
		case 0xe6:
			do_branch(!(m_psw & FLAG_CX)); // CX=0
			break;

		// blsx label8
		case 0xe7:
			do_branch(m_psw & (FLAG_ZX|FLAG_CX)); // (CX|ZX)=1
			break;

		// beqx label8
		case 0xe8:
			do_branch(m_psw & FLAG_ZX); // ZX=1
			break;

		// bnex label8
		case 0xe9:
			do_branch(!(m_psw & FLAG_ZX)); // ZX=0
			break;

		// bvcx label8
		case 0xec:
			do_branch(!(m_psw & FLAG_VX)); // VX=0
			break;

		// bvsx label8
		case 0xed:
			do_branch(m_psw & FLAG_VX); // VX=1
			break;

		// bncx label8
		case 0xee:
			do_branch(!(m_psw & FLAG_NX)); // NX=0
			break;

		// bnsx label8
		case 0xef:
			do_branch(m_psw & FLAG_NX); // NX=1
			break;

		// bvc label8
		case 0xfc:
			do_branch(!(m_psw & FLAG_VF)); // VF=0
			break;

		// bvs label8
		case 0xfd:
			do_branch(m_psw & FLAG_VF); // VF=1
			break;

		// bnc label8
		case 0xfe:
			do_branch(!(m_psw & FLAG_NF)); // NF=0
			break;

		// bns label8
		case 0xff:
			do_branch(m_psw & FLAG_NF); // NF=1
			break;

		default:
			illegal(0xf5, op);
			m_possible_irq = true;
			break;
		}
		m_pc += 1;
		break;



		// extended code f7 (4 bytes)
		case 0xf7:
			m_cycles -= 1;
			op = read_arg8(m_pc);
			m_pc += 1;

		switch (op)
		{
		// and imm16, dn
		case 0x00: case 0x01: case 0x02: case 0x03:
			test_nz16(m_d[op&3] &= 0xff0000 | read_arg16(m_pc));
			break;

		// btst imm16, dn
		case 0x04: case 0x05: case 0x06: case 0x07:
			test_nz16(m_d[op&3] & read_arg16(m_pc));
			break;

		// add imm16, an
		case 0x08: case 0x09: case 0x0a: case 0x0b:
			m_a[op&3] = do_add(m_a[op&3], (INT16)read_arg16(m_pc));
			break;

		// sub imm16, an
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			m_a[op&3] = do_sub(m_a[op&3], (INT16)read_arg16(m_pc));
			break;

		// and imm16, psw
		case 0x10:
			m_cycles -= 1;
			m_psw &= read_arg16(m_pc);
			break;

		// or imm16, psw
		case 0x14:
			m_cycles -= 1;
			m_psw |= read_arg16(m_pc);
			m_possible_irq = true;
			break;

		// add imm16, dn
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			m_d[op&3] = do_add(m_d[op&3], (INT16)read_arg16(m_pc));
			break;

		// sub imm16, dn
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_d[op&3] = do_sub(m_d[op&3], (INT16)read_arg16(m_pc));
			break;

		// mov an, (abs16)
		case 0x20: case 0x21: case 0x22: case 0x23:
			m_cycles -= 1;
			write_mem24(read_arg16(m_pc), m_a[op&3]);
			break;

		// mov (abs16), an
		case 0x30: case 0x31: case 0x32: case 0x33:
			m_cycles -= 1;
			m_a[op&3] = read_mem24(read_arg16(m_pc));
			break;

		// or imm16, dn
		case 0x40: case 0x41: case 0x42: case 0x43:
			test_nz16(m_d[op&3] |= read_arg16(m_pc));
			break;

		// cmp imm16, dn
		case 0x48: case 0x49: case 0x4a: case 0x4b:
			do_sub(m_d[op&3], (INT16)read_arg16(m_pc));
			break;

		// xor imm16, dn
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			test_nz16(m_d[op&3] ^= read_arg16(m_pc));
			break;

		// movbu (d16, an), dm
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			m_d[op&3] = read_mem8(m_a[op>>2&3] + (INT16)read_arg16(m_pc));
			break;

		// movx dm, (d16, an)
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			m_cycles -= 1;
			write_mem24(m_a[op>>2&3] + (INT16)read_arg16(m_pc), m_d[op&3]);
			break;

		// movx (d16, an), dm
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_cycles -= 1;
			m_d[op&3] = read_mem24(m_a[op>>2&3] + (INT16)read_arg16(m_pc));
			break;

		// mov dm, (d16, an)
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			write_mem16(m_a[op>>2&3] + (INT16)read_arg16(m_pc), m_d[op&3]);
			break;

		// movb dm, (d16, an)
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			write_mem8(m_a[op>>2&3] + (INT16)read_arg16(m_pc), m_d[op&3]);
			break;

		// mov am, (d16, an)
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			m_cycles -= 1;
			write_mem24(m_a[op>>2&3] + (INT16)read_arg16(m_pc), m_a[op&3]);
			break;

		// mov (d16, an), am
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			m_cycles -= 1;
			m_a[op&3] = read_mem24(m_a[op>>2&3] + (INT16)read_arg16(m_pc));
			break;

		// mov (d16, an), dm
		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
			m_d[op&3] = (INT16)read_mem16(m_a[op>>2&3] + (INT16)read_arg16(m_pc));
			break;

		// movb (d16, an), dm
		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
			m_d[op&3] = (INT8)read_mem8(m_a[op>>2&3] + (INT16)read_arg16(m_pc));
			break;

		default:
			illegal(0xf7, op);
			m_possible_irq = true;
			break;
		}
		m_pc += 2;
		break;

	} // end main switch

	} // end loop
}



//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

WRITE8_MEMBER(mn10200_device::io_control_w)
{
	switch (offset)
	{
		case 0x000:
			if(data & 12)
			{
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

		// irq control
		// group 0, NMI control
		case 0x040:
			m_nmicr &= data; // nmi ack
			break;
		case 0x041:
			break;

		// group 1-10, maskable irq control
		case 0x042: case 0x044: case 0x046: case 0x048: case 0x04a:
		case 0x04c: case 0x04e: case 0x050: case 0x052: case 0x054:
			m_icrl[(offset & 0x3f) >> 1] &= data; // irq ack

			// group 8, external irqs might still be active
			if (offset == 0x050)
				check_ext_irq();
			break;

		case 0x043: case 0x045: case 0x047: case 0x049: case 0x04b:
		case 0x04d: case 0x04f: case 0x051: case 0x053: case 0x055:
			m_icrh[(offset & 0x3f) >> 1] = data;
			m_possible_irq = true;
			break;

		// external irq control
		case 0x056:
			m_extmdl = data;
			check_ext_irq();
			break;
		case 0x057:
			m_extmdh = data & 3;
			break;

		case 0x100: case 0x101: // DRAM control reg
		case 0x102: case 0x103: // Refresh counter
			break;

		case 0x180: case 0x190:
		{
			int ser = (offset-0x180) >> 4;
			// const char *parity[8] = { "no", "1", "2", "3", "l", "h", "even", "odd" };
			// const char *source[4] = { "sbt0", "timer 8", "2", "timer 9" };
			m_serial[ser].ctrll = data;

			// log_event("MN102", "Serial %d length=%c, parity=%s, stop=%c, source=%s",
			//     ser,
			//     data & 0x80 ? '8' : '7', parity[(data >> 4) & 7],
			//     data & 8 ? '2' : '1', source[data & 3]);
			break;
		}

		case 0x181: case 0x191:
		{
			int ser = (offset-0x180) >> 4;
			m_serial[ser].ctrlh = data;
			// log_event("MN102", "Serial %d transmit=%s, receive=%s, break=%s, proto=%s, order=%s",
			//     ser,
			//     data & 0x80 ? "on" : "off", data & 0x40 ? "on" : "off",
			//     data & 0x20 ? "on" : "off", data & 8 ? "sync" : "async",
			//     data & 2 ? "msb" : "lsb");
			break;
		}

		case 0x182: case 0x192:
		{
			int ser = (offset-0x180) >> 4;
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

		// 8-bit timers
		// timer base
		case 0x210: case 0x211: case 0x212: case 0x213: case 0x214:
		case 0x215: case 0x216: case 0x217: case 0x218: case 0x219:
			m_simple_timer[offset & 0xf].base = data;
			break;

		// prescaler base
		case 0x21a: case 0x21b:
			m_prescaler[offset & 1].base = data;
			break;

		// timer mode
		case 0x220: case 0x221: case 0x222: case 0x223: case 0x224:
		case 0x225: case 0x226: case 0x227: case 0x228: case 0x229:
			m_simple_timer[offset & 0xf].mode = data & 0xc3;

			// reload
			if (data & 0x40)
				m_simple_timer[offset & 0xf].cur = m_simple_timer[offset & 0xf].base;

			refresh_timer(offset & 0xf);
			break;

		// prescaler mode
		case 0x22a: case 0x22b:
			m_prescaler[offset & 1].mode = data & 0xc0;

			// reload
			if (data & 0x40)
				m_prescaler[offset & 1].cur = m_prescaler[offset & 1].base;

			refresh_all_timers();
			break;

		case 0x230: case 0x240: case 0x250:
		{
			// const char *modes[4] = { "single", "double", "ioa", "iob" };
			// const char *sources[8] = { "pres.0", "pres.1", "iob", "sysclk", "*4", "*1", "6", "7" };
			// printf("MN10200: Timer %d comp=%s on_1=%s on_match=%s phase=%s source=%s\n",
			//     10 + ((offset-0x230) >> 4),
			//     modes[data >> 6], data & 0x20 ? "cleared" : "not cleared",  data & 0x10 ? "cleared" : "not cleared",
			//     data & 8 ? "tff" : "rsff", sources[data & 7]);
			break;
		}

		case 0x231: case 0x241: case 0x251:
		{
			// const char *modes[4] = { "up", "down", "up on ioa", "up on iob" };
			// printf("MN10200: Timer %d %s ff=%s op=%s ext_trig=%s %s\n",
			// 10 + ((offset-0x230) >> 4),
			// data & 0x80 ? "enable" : "disable", data & 0x40 ? "operate" : "clear",
			// modes[(data >> 4) & 3], data & 2 ? "on" : "off", data & 1 ? "one-shot" : "repeat");

			break;
		}

		case 0x234: case 0x244: case 0x254:
			log_event("MN102", "Timer %d ca=--%02x", 10 + ((offset-0x230) >> 4), data);
			break;

		case 0x235: case 0x245: case 0x255:
			log_event("MN102", "Timer %d ca=%02x--", 10 + ((offset-0x230) >> 4), data);
			break;

		case 0x236: case 0x246: case 0x256:
			log_event("MN102", "Timer %d ca read trigger", 10 + ((offset-0x230) >> 4));
			break;

		case 0x237: case 0x247: case 0x257:
			break;

		case 0x238: case 0x248: case 0x258:
			log_event("MN102", "Timer %d cb=--%02x", 10 + ((offset-0x230) >> 4), data);
			break;

		case 0x239: case 0x249: case 0x259:
			log_event("MN102", "Timer %d cb=%02x--", 10 + ((offset-0x230) >> 4), data);
			break;

		case 0x23a: case 0x24a: case 0x25a:
			log_event("MN102", "Timer %d cb read trigger", 10 + ((offset-0x230) >> 4));
			break;

		case 0x23b: case 0x24b: case 0x25b:
			break;

		case 0x260: case 0x261:
		{
			// const char *mode[4] = { "sysbuf", "4-phase", "4-phase 1/2", "3" };
			// log_event("MN102", "Sync Output %c timing=%s out=%s dir=%s mode=%s",
			//     offset == 0x261 ? 'B' : 'A',
			//     data & 0x10 ? "12A" : "1", data & 8 ? "sync a" :"P13-10",
			//     data & 4 ? "ccw" : "cw", mode[data & 3]);
			break;
		}

		case 0x262:
			log_event("MN102", "Sync Output buffer = %02x", data);
			break;

		case 0x280: case 0x290: case 0x2a0: case 0x2b0: case 0x2c0: case 0x2d0: case 0x2e0: case 0x2f0:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].adr = (m_dma[dma].adr & 0x00ffff00) | data;
			logerror("MN10200: DMA %d adr=%06x\n", dma, m_dma[dma].adr);
			break;
		}

		case 0x281: case 0x291: case 0x2a1: case 0x2b1: case 0x2c1: case 0x2d1: case 0x2e1: case 0x2f1:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].adr = (m_dma[dma].adr & 0x00ff00ff) | (data << 8);
			logerror("MN10200: DMA %d adr=%06x\n", dma, m_dma[dma].adr);
			break;
		}

		case 0x282: case 0x292: case 0x2a2: case 0x2b2: case 0x2c2: case 0x2d2: case 0x2e2: case 0x2f2:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].adr = (m_dma[dma].adr & 0x0000ffff) | (data << 16);
			logerror("MN10200: DMA %d adr=%06x\n", dma, m_dma[dma].adr);
			break;
		}

		case 0x283: case 0x293: case 0x2a3: case 0x2b3: case 0x2c3: case 0x2d3: case 0x2e3: case 0x2f3:
			break;

		case 0x284: case 0x294: case 0x2a4: case 0x2b4: case 0x2c4: case 0x2d4: case 0x2e4: case 0x2f4:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].count = (m_dma[dma].count & 0x00ffff00) | data;
			logerror("MN10200: DMA %d count=%06x\n", dma, m_dma[dma].count);
			break;
		}

		case 0x285: case 0x295: case 0x2a5: case 0x2b5: case 0x2c5: case 0x2d5: case 0x2e5: case 0x2f5:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].count = (m_dma[dma].count & 0x00ff00ff) | (data << 8);
			logerror("MN10200: DMA %d count=%06x\n", dma, m_dma[dma].count);
			break;
		}

		case 0x286: case 0x296: case 0x2a6: case 0x2b6: case 0x2c6: case 0x2d6: case 0x2e6: case 0x2f6:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].count = (m_dma[dma].count & 0x0000ffff) | (data << 16);
			logerror("MN10200: DMA %d count=%06x\n", dma, m_dma[dma].count);
			break;
		}

		case 0x287: case 0x297: case 0x2a7: case 0x2b7: case 0x2c7: case 0x2d7: case 0x2e7: case 0x2f7:
			break;

		case 0x288: case 0x298: case 0x2a8: case 0x2b8: case 0x2c8: case 0x2d8: case 0x2e8: case 0x2f8:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].iadr = (m_dma[dma].iadr & 0xff00) | data;
			logerror("MN10200: DMA %d iadr=%03x\n", dma, m_dma[dma].iadr);
			break;
		}

		case 0x289: case 0x299: case 0x2a9: case 0x2b9: case 0x2c9: case 0x2d9: case 0x2e9: case 0x2f9:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].iadr = (m_dma[dma].iadr & 0x00ff) | ((data & 3) << 8);
			logerror("MN10200: DMA %d iadr=%03x\n", dma, m_dma[dma].iadr);
			break;
		}

		case 0x28a: case 0x29a: case 0x2aa: case 0x2ba: case 0x2ca: case 0x2da: case 0x2ea: case 0x2fa:
		{
			static const char *const trans[4] = { "M-IO", "M-M", "M-X1", "m-X2" };
			static const char *const start[32] =
			{
				"soft", "a/d", "ser0tx", "set0rx", "ser1tx", "ser1rx",
				"timer0", "timer1", "timer2", "timer3", "timer4", "timer5", "timer6", "timer7", "timer8", "timer9",
				"timer10u", "timer10a", "timer10b",
				"timer11u", "timer11a", "timer12b",
				"timer12a", "timer12b",
				"irq0", "irq1", "irq2", "irq3",
				"X0e", "X1e", "X0l", "X1l"
			};

			int dma = (offset-0x280) >> 4;
			m_dma[dma].ctrll = data;
			logerror("MN10200: DMA %d control ack=%s, trans=%s, start=%s\n",
				dma,
				data & 0x80 ? "level" : "pulse",
				trans[(data >> 5) & 3],
				start[data & 31]);

			break;
		}

		case 0x28b: case 0x29b: case 0x2ab: case 0x2bb: case 0x2cb: case 0x2db: case 0x2eb: case 0x2fb:
		{
			static const char *const tradr[4] = { "inc", "dec", "fixed", "reserved" };
			int dma = (offset-0x280) >> 4;
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

		case 0x28c: case 0x29c: case 0x2ac: case 0x2bc: case 0x2cc: case 0x2dc: case 0x2ec: case 0x2fc:
		{
			int dma = (offset-0x280) >> 4;
			m_dma[dma].irq = data & 7;
			logerror("MN10200: DMA %d irq=%d\n", dma, data & 7);
			break;
		}

		case 0x28d: case 0x29d: case 0x2ad: case 0x2bd: case 0x2cd: case 0x2dd: case 0x2ed: case 0x2fd:
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

		// ports
		// pull-up control
		case 0x3b0:
			m_pplul = data & 0x7f;
			break;
		case 0x3b1:
			m_ppluh = data & 0x3f;
			break;

		// outputs
		case 0x3c0:
			m_port[0].out = data;
			m_write_port0(MN10200_PORT0, m_port[0].out | (m_port[0].dir ^ 0xff), 0xff);
			break;
		case 0x264:
			m_port[1].out = data;
			m_write_port1(MN10200_PORT1, m_port[1].out | (m_port[1].dir ^ 0xff), 0xff);
			break;
		case 0x3c2:
			m_port[2].out = data & 0x0f;
			m_write_port2(MN10200_PORT2, m_port[2].out | (m_port[2].dir ^ 0x0f), 0xff);
			break;
		case 0x3c3:
			m_port[3].out = data & 0x1f;
			m_write_port3(MN10200_PORT3, m_port[3].out | (m_port[3].dir ^ 0x1f), 0xff);
			break;

		// directions (0=input, 1=output)
		case 0x3e0:
			m_port[0].dir = data;
			break;
		case 0x3e1:
			m_port[1].dir = data;
			break;
		case 0x3e2:
			m_port[2].dir = data & 0x0f;
			break;
		case 0x3e3:
			m_port[3].dir = data & 0x1f;
			break;

		// port 3 output mode
		case 0x3f3:
			m_p3md = data & 0x7f;
			break;


		default:
			log_event("MN102", "internal_w %04x, %02x (%03x)", offset+0xfc00, data, adr);
			break;
	}
}


READ8_MEMBER(mn10200_device::io_control_r)
{
	switch (offset)
	{
		// active irq group
		case 0x00e:
			return m_iagr << 1;
		case 0x00f:
			return 0;

		// irq control
		// group 0, NMI control
		case 0x040:
			return m_nmicr;
		case 0x041:
			return 0;

		// group 1-10, maskable irq control
		case 0x042: case 0x044: case 0x046: case 0x048: case 0x04a:
		case 0x04c: case 0x04e: case 0x050: case 0x052: case 0x054:
			// low 4 bits are a mask of IEN and IRF
			return (m_icrl[(offset & 0x3f) >> 1] & 0xf0) | (m_icrl[(offset & 0x3f) >> 1] >> 4 & m_icrh[(offset & 0x3f) >> 1]);

		case 0x043: case 0x045: case 0x047: case 0x049: case 0x04b:
		case 0x04d: case 0x04f: case 0x051: case 0x053: case 0x055:
			return m_icrh[(offset & 0x3f) >> 1];

		// external irq control
		case 0x056:
			return m_extmdl;
		case 0x057:
			return m_extmdh | (m_p4 << 4);

		case 0x180: case 0x190:
			return m_serial[(offset-0x180) >> 4].ctrll;

		case 0x181: case 0x191:
			return m_serial[(offset-0x180) >> 4].ctrlh;

		case 0x182:
		{
			static int zz;
			return zz++;
		}

		case 0x183:
			return 0x10;

		// 8-bit timers
		// timer counter (not accurate)
		case 0x200: case 0x201: case 0x202: case 0x203: case 0x204:
		case 0x205: case 0x206: case 0x207: case 0x208: case 0x209:
			return m_simple_timer[offset & 0xf].cur;

		// prescaler counter (not accurate)
		case 0x20a: case 0x20b:
			return m_prescaler[offset & 1].cur;

		// timer base
		case 0x210: case 0x211: case 0x212: case 0x213: case 0x214:
		case 0x215: case 0x216: case 0x217: case 0x218: case 0x219:
			return m_simple_timer[offset & 0xf].base;

		// prescaler base
		case 0x21a: case 0x21b:
			return m_prescaler[offset & 1].base;

		// timer mode
		case 0x220: case 0x221: case 0x222: case 0x223: case 0x224:
		case 0x225: case 0x226: case 0x227: case 0x228: case 0x229:
			return m_simple_timer[offset & 0xf].mode;

		// prescaler mode
		case 0x22a: case 0x22b:
			return m_prescaler[offset & 1].mode;

		case 0x28c: case 0x29c: case 0x2ac: case 0x2bc: case 0x2cc: case 0x2dc: case 0x2ec: case 0x2fc:
		{
			int dma = (offset-0x280) >> 4;
			return m_dma[dma].irq;
		}

		// ports
		// pull-up control
		case 0x3b0:
			return m_pplul;
		case 0x3b1:
			return m_ppluh;

		// outputs
		case 0x3c0:
			return m_port[0].out;
		case 0x264:
			return m_port[1].out;
		case 0x3c2:
			return m_port[2].out;
		case 0x3c3:
			return m_port[3].out;

		// inputs
		case 0x3d0:
			return m_read_port0(MN10200_PORT0, 0xff) | m_port[0].dir;
		case 0x3d1:
			return m_read_port1(MN10200_PORT1, 0xff) | m_port[1].dir;
		case 0x3d2:
			return (m_read_port2(MN10200_PORT2, 0xff) & 0x0f) | m_port[2].dir;
		case 0x3d3:
			return (m_read_port3(MN10200_PORT3, 0xff) & 0x1f) | m_port[3].dir;

		// directions (0=input, 1=output)
		case 0x3e0:
			return m_port[0].dir;
		case 0x3e1:
			return m_port[1].dir;
		case 0x3e2:
			return m_port[2].dir;
		case 0x3e3:
			return m_port[3].dir;

		// port 3 output mode
		case 0x3f3:
			return m_p3md;


		default:
			log_event("MN102", "internal_r %04x (%03x)", offset+0xfc00, adr);
			break;
	}

	return 0;
}
