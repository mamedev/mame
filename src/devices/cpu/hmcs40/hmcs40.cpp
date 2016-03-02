// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family cores

  References:
  - 1985 #AP1 Hitachi 4-bit Single-Chip Microcomputer Data Book
  - 1988 HMCS400 Series Handbook (note: *400 is a newer MCU series, with similarities)
  - opcode decoding by Tatsuyuki Satoh, Olivier Galibert, Kevin Horton, Lord Nightmare

*/

#define IS_PMOS 0
#define IS_CMOS ~0

#include "hmcs40.h"
#include "debugger.h"


// MCU types

// HMCS42/C/CL
//const device_type HD38702 = &device_creator<hd38702_device>; // PMOS, 28 pins, 22 I/O lines, (512+32)x10 ROM, 32x4 RAM, no B or SPY register
//const device_type HD44700 = &device_creator<hd44700_device>; // CMOS version
//const device_type HD44708 = &device_creator<hd44708_device>; // CMOS version, low-power

// HMCS43/C/CL
const device_type HD38750 = &device_creator<hd38750_device>; // PMOS, 42 pins, 32 I/O lines, (1024+64)x10 ROM, 80x4 RAM
const device_type HD38755 = &device_creator<hd38755_device>; // ceramic filter oscillator type
const device_type HD44750 = &device_creator<hd44750_device>; // CMOS version
const device_type HD44758 = &device_creator<hd44758_device>; // CMOS version, low-power

// HMCS44A/C/CL
const device_type HD38800 = &device_creator<hd38800_device>; // PMOS, 42 pins, 32 I/O lines, (2048+128)x10 ROM, 160x4 RAM
const device_type HD38805 = &device_creator<hd38805_device>; // ceramic filter oscillator type
const device_type HD44801 = &device_creator<hd44801_device>; // CMOS version
const device_type HD44808 = &device_creator<hd44808_device>; // CMOS version, low-power

// HMCS45A/C/CL
const device_type HD38820 = &device_creator<hd38820_device>; // PMOS, 54 pins(QFP) or 64 pins(DIP), 44 I/O lines, (2048+128)x10 ROM, 160x4 RAM
const device_type HD38825 = &device_creator<hd38825_device>; // ceramic filter oscillator type
const device_type HD44820 = &device_creator<hd44820_device>; // CMOS version
const device_type HD44828 = &device_creator<hd44828_device>; // CMOS version, low-power

// HMCS46C/CL (no PMOS version exists)
//const device_type HD44840 = &device_creator<hd44840_device>; // CMOS, 42 pins, 32 I/O lines, 4096x10 ROM, 256x4 RAM
//const device_type HD44848 = &device_creator<hd44848_device>; // CMOS, low-power

// HMCS47A/C/CL
//const device_type HD38870 = &device_creator<hd38870_device>; // PMOS, 54 pins(QFP) or 64 pins(DIP), 44 I/O lines, 4096x10 ROM, 256x4 RAM
//const device_type HD44860 = &device_creator<hd44860_device>; // CMOS version
//const device_type HD44868 = &device_creator<hd44868_device>; // CMOS version, low-power


// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 16, hmcs40_cpu_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0780, 0x07bf) AM_ROM // patterns on page 30
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_2k, AS_PROGRAM, 16, hmcs40_cpu_device)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0f40, 0x0fbf) AM_ROM // patterns on page 61,62
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_80x4, AS_DATA, 8, hmcs40_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
	AM_RANGE(0x40, 0x4f) AM_RAM AM_MIRROR(0x30)
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_160x4, AS_DATA, 8, hmcs40_cpu_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
	AM_RANGE(0x80, 0x8f) AM_RAM AM_MIRROR(0x30)
	AM_RANGE(0xc0, 0xcf) AM_RAM AM_MIRROR(0x30)
ADDRESS_MAP_END


// device definitions
hmcs43_cpu_device::hmcs43_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT16 polarity, const char *shortname)
	: hmcs40_cpu_device(mconfig, type, name, tag, owner, clock, HMCS40_FAMILY_HMCS43, polarity, 3 /* stack levels */, 10 /* pc width */, 11 /* prg width */, ADDRESS_MAP_NAME(program_1k), 7 /* data width */, ADDRESS_MAP_NAME(data_80x4), shortname, __FILE__)
{ }

hd38750_device::hd38750_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs43_cpu_device(mconfig, HD38750, "HD38750", tag, owner, clock, IS_PMOS, "hd38750")
{ }
hd38755_device::hd38755_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs43_cpu_device(mconfig, HD38755, "HD38755", tag, owner, clock, IS_PMOS, "hd38755")
{ }
hd44750_device::hd44750_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs43_cpu_device(mconfig, HD44750, "HD44750", tag, owner, clock, IS_CMOS, "hd44750")
{ }
hd44758_device::hd44758_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs43_cpu_device(mconfig, HD44758, "HD44758", tag, owner, clock, IS_CMOS, "hd44758")
{ }


hmcs44_cpu_device::hmcs44_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT16 polarity, const char *shortname)
	: hmcs40_cpu_device(mconfig, type, name, tag, owner, clock, HMCS40_FAMILY_HMCS44, polarity, 4, 11, 12, ADDRESS_MAP_NAME(program_2k), 8, ADDRESS_MAP_NAME(data_160x4), shortname, __FILE__)
{ }

hd38800_device::hd38800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs44_cpu_device(mconfig, HD38800, "HD38800", tag, owner, clock, IS_PMOS, "hd38800")
{ }
hd38805_device::hd38805_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs44_cpu_device(mconfig, HD38805, "HD38805", tag, owner, clock, IS_PMOS, "hd38805")
{ }
hd44801_device::hd44801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs44_cpu_device(mconfig, HD44801, "HD44801", tag, owner, clock, IS_CMOS, "hd44801")
{ }
hd44808_device::hd44808_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs44_cpu_device(mconfig, HD44808, "HD44808", tag, owner, clock, IS_CMOS, "hd44808")
{ }


hmcs45_cpu_device::hmcs45_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT16 polarity, const char *shortname)
	: hmcs40_cpu_device(mconfig, type, name, tag, owner, clock, HMCS40_FAMILY_HMCS45, polarity, 4, 11, 12, ADDRESS_MAP_NAME(program_2k), 8, ADDRESS_MAP_NAME(data_160x4), shortname, __FILE__)
{ }

hd38820_device::hd38820_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs45_cpu_device(mconfig, HD38820, "HD38820", tag, owner, clock, IS_PMOS, "hd38820")
{ }
hd38825_device::hd38825_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs45_cpu_device(mconfig, HD38825, "HD38825", tag, owner, clock, IS_PMOS, "hd38825")
{ }
hd44820_device::hd44820_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs45_cpu_device(mconfig, HD44820, "HD44820", tag, owner, clock, IS_CMOS, "hd44820")
{ }
hd44828_device::hd44828_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hmcs45_cpu_device(mconfig, HD44828, "HD44828", tag, owner, clock, IS_CMOS, "hd44828")
{ }


// disasm
void hmcs40_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c",
				m_c ? 'C':'c',
				m_s ? 'S':'s'
			);
			break;

		default: break;
	}
}

offs_t hmcs40_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(hmcs40);
	return CPU_DISASSEMBLE_NAME(hmcs40)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	HMCS40_PC=1, HMCS40_A, HMCS40_B,
	HMCS40_X, HMCS40_SPX, HMCS40_Y, HMCS40_SPY
};

void hmcs40_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;
	m_pcmask = (1 << m_pcwidth) - 1;

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hmcs40_cpu_device::simple_timer_cb), this));
	reset_prescaler();

	// resolve callbacks
	m_read_r0.resolve_safe(m_polarity & 0xf);
	m_read_r1.resolve_safe(m_polarity & 0xf);
	m_read_r2.resolve_safe(m_polarity & 0xf);
	m_read_r3.resolve_safe(m_polarity & 0xf);
	m_read_r4.resolve_safe(m_polarity & 0xf);
	m_read_r5.resolve_safe(m_polarity & 0xf);
	m_read_r6.resolve_safe(m_polarity & 0xf);
	m_read_r7.resolve_safe(m_polarity & 0xf);

	m_write_r0.resolve_safe();
	m_write_r1.resolve_safe();
	m_write_r2.resolve_safe();
	m_write_r3.resolve_safe();
	m_write_r4.resolve_safe();
	m_write_r5.resolve_safe();
	m_write_r6.resolve_safe();
	m_write_r7.resolve_safe();

	m_read_d.resolve_safe(m_polarity);
	m_write_d.resolve_safe();

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_op = 0;
	m_prev_op = 0;
	m_i = 0;
	m_eint_line = 0;
	m_halt = 0;
	m_pc = 0;
	m_prev_pc = 0;
	m_page = 0;
	m_a = 0;
	m_b = 0;
	m_x = 0;
	m_spx = 0;
	m_y = 0;
	m_spy = 0;
	m_s = 1;
	m_c = 0;
	m_tc = 0;
	m_cf = 0;
	m_ie = 0;
	m_iri = m_irt = 0;
	memset(m_if, 0, sizeof(m_if));
	m_tf = 0;
	memset(m_int, 0, sizeof(m_int));
	memset(m_r, 0, sizeof(m_r));
	m_d = 0;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_i));
	save_item(NAME(m_eint_line));
	save_item(NAME(m_halt));
	save_item(NAME(m_timer_halted_remain));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_page));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_x));
	save_item(NAME(m_spx));
	save_item(NAME(m_y));
	save_item(NAME(m_spy));
	save_item(NAME(m_s));
	save_item(NAME(m_c));
	save_item(NAME(m_tc));
	save_item(NAME(m_cf));
	save_item(NAME(m_ie));
	save_item(NAME(m_iri));
	save_item(NAME(m_irt));
	save_item(NAME(m_if));
	save_item(NAME(m_tf));
	save_item(NAME(m_int));

	save_item(NAME(m_r));
	save_item(NAME(m_d));

	// register state for debugger
	state_add(HMCS40_PC,  "PC",  m_pc).formatstr("%04X");
	state_add(HMCS40_A,   "A",   m_a).formatstr("%01X");
	state_add(HMCS40_B,   "B",   m_b).formatstr("%01X");
	state_add(HMCS40_X,   "X",   m_x).formatstr("%01X");
	state_add(HMCS40_SPX, "SPX", m_spx).formatstr("%01X");
	state_add(HMCS40_Y,   "Y",   m_y).formatstr("%01X");
	state_add(HMCS40_SPY, "SPY", m_spy).formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_s).formatstr("%2s").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hmcs40_cpu_device::device_reset()
{
	m_pc = m_pcmask;
	m_prev_op = m_op = 0;

	// clear interrupts
	m_cf = 0;
	m_ie = 0;
	m_iri = m_irt = 0;
	m_if[0] = m_if[1] = m_tf = 1;

	// clear i/o
	m_d = m_polarity;
	for (int i = 0; i < 16; i++)
		hmcs40_cpu_device::write_d(i, m_polarity);

	for (int i = 0; i < 8; i++)
		hmcs40_cpu_device::write_r(i, m_polarity & 0xf);
}



//-------------------------------------------------
//  i/o handling
//-------------------------------------------------

UINT8 hmcs40_cpu_device::read_r(int index)
{
	index &= 7;
	UINT8 inp = 0;

	switch (index)
	{
		case 0: inp = m_read_r0(index, 0xff); break;
		case 1: inp = m_read_r1(index, 0xff); break;
		case 2: inp = m_read_r2(index, 0xff); break;
		case 3: inp = m_read_r3(index, 0xff); break;
		case 4: inp = m_read_r4(index, 0xff); break;
		case 5: inp = m_read_r5(index, 0xff); break;
		case 6: inp = m_read_r6(index, 0xff); break;
		case 7: inp = m_read_r7(index, 0xff); break;
	}

	if (m_polarity)
		return (inp & m_r[index]) & 0xf;
	else
		return (inp | m_r[index]) & 0xf;
}

void hmcs40_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;
	data &= 0xf;
	m_r[index] = data;

	switch (index)
	{
		case 0: m_write_r0(index, data, 0xff); break;
		case 1: m_write_r1(index, data, 0xff); break;
		case 2: m_write_r2(index, data, 0xff); break;
		case 3: m_write_r3(index, data, 0xff); break;
		case 4: m_write_r4(index, data, 0xff); break;
		case 5: m_write_r5(index, data, 0xff); break;
		case 6: m_write_r6(index, data, 0xff); break;
		case 7: m_write_r7(index, data, 0xff); break;
	}
}

int hmcs40_cpu_device::read_d(int index)
{
	index &= 15;

	if (m_polarity)
		return (m_read_d(index, 0xffff) & m_d) >> index & 1;
	else
		return (m_read_d(index, 0xffff) | m_d) >> index & 1;
}

void hmcs40_cpu_device::write_d(int index, int state)
{
	index &= 15;
	state = (state) ? 1 : 0;

	m_d = (m_d & ~(1 << index)) | state << index;
	m_write_d(index, m_d, 0xffff);
}

// HMCS43:
// R0 is input-only, R1 is i/o, R2,R3 are output-only, no R4-R7
// D0-D3 are i/o, D4-D15 are output-only

UINT8 hmcs43_cpu_device::read_r(int index)
{
	index &= 7;

	if (index >= 2)
		logerror("%s read from %s port R%d at $%04X\n", tag(), (index >= 4) ? "unknown" : "output", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs43_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;

	if (index != 0 && index < 4)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("%s ineffective write to port R%d = $%X at $%04X\n", tag(), index, data & 0xf, m_prev_pc);
}

int hmcs43_cpu_device::read_d(int index)
{
	index &= 15;

	if (index >= 4)
		logerror("%s read from output pin D%d at $%04X\n", tag(), index, m_prev_pc);

	return hmcs40_cpu_device::read_d(index);
}

// HMCS44:
// R0-R3 are i/o, R4,R5 are extra registers, no R6,R7
// D0-D15 are i/o

UINT8 hmcs44_cpu_device::read_r(int index)
{
	index &= 7;

	if (index >= 6)
		logerror("%s read from unknown port R%d at $%04X\n", tag(), index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs44_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;

	if (index < 6)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("%s ineffective write to port R%d = $%X at $%04X\n", tag(), index, data & 0xf, m_prev_pc);
}

// HMCS45:
// R0-R5 are i/o, R6 is output-only, no R7
// D0-D15 are i/o

UINT8 hmcs45_cpu_device::read_r(int index)
{
	index &= 7;

	if (index >= 6)
		logerror("%s read from %s port R%d at $%04X\n", tag(), (index == 7) ? "unknown" : "output", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs45_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;

	if (index != 7)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("%s ineffective write to port R%d = $%X at $%04X\n", tag(), index, data & 0xf, m_prev_pc);
}



//-------------------------------------------------
//  interrupt/timer handling
//-------------------------------------------------

void hmcs40_cpu_device::do_interrupt()
{
	m_icount--;
	push_stack();
	m_ie = 0;

	// line 0/1 for external interrupt, let's use 2 for t/c interrupt
	int line = (m_iri) ? m_eint_line : 2;

	// vector $3f, on page 0(timer/counter), or page 1(external)
	// external interrupt has priority over t/c interrupt
	m_pc = 0x3f | (m_iri ? 0x40 : 0);
	if (m_iri)
		m_iri = 0;
	else
		m_irt = 0;

	standard_irq_callback(line);
}

void hmcs40_cpu_device::execute_set_input(int line, int state)
{
	state = (state) ? 1 : 0;

	// halt/unhalt mcu
	if (line == HMCS40_INPUT_LINE_HLT && state != m_halt)
	{
		if (state)
		{
			m_timer_halted_remain = m_timer->remaining();
			m_timer->reset();
		}
		else
			m_timer->adjust(m_timer_halted_remain);

		m_halt = state;
		return;
	}

	if (line != 0 && line != 1)
		return;

	// external interrupt request on rising edge
	if (state && !m_int[line])
	{
		if (!m_if[line])
		{
			m_eint_line = line;
			m_iri = 1;
			m_if[line] = 1;
		}

		// clock tc if it is in counter mode
		if (m_cf && line == 1)
			increment_tc();
	}

	m_int[line] = state;
}

void hmcs40_cpu_device::reset_prescaler()
{
	// reset 6-bit timer prescaler
	attotime base = attotime::from_ticks(4 * 64, unscaled_clock());
	m_timer->adjust(base);
}

TIMER_CALLBACK_MEMBER( hmcs40_cpu_device::simple_timer_cb )
{
	// timer prescaler overflow
	if (!m_cf)
		increment_tc();

	reset_prescaler();
}

void hmcs40_cpu_device::increment_tc()
{
	// increment timer/counter
	m_tc = (m_tc + 1) & 0xf;

	// timer interrupt request on overflow
	if (m_tc == 0 && !m_tf)
	{
		m_irt = 1;
		m_tf = 1;
	}
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

inline void hmcs40_cpu_device::increment_pc()
{
	// PC lower bits is a LFSR identical to TI TMS1000
	UINT8 mask = 0x3f;
	UINT8 low = m_pc & mask;
	int fb = (low << 1 & 0x20) == (low & 0x20);

	if (low == (mask >> 1))
		fb = 1;
	else if (low == mask)
		fb = 0;

	m_pc = (m_pc & ~mask) | ((m_pc << 1 | fb) & mask);
}

void hmcs40_cpu_device::execute_run()
{
	// in HLT state, the internal clock is not running
	if (m_halt)
	{
		m_icount = 0;
		return;
	}

	while (m_icount > 0)
	{
		// LPU is handled 1 cycle later
		if ((m_prev_op & 0x3e0) == 0x340)
			m_pc = ((m_page << 6) | (m_pc & 0x3f)) & m_pcmask;

		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// check/handle interrupt, but not in the middle of a long jump
		if (m_ie && (m_iri || m_irt) && (m_prev_op & 0x3e0) != 0x340)
		{
			do_interrupt();
			if (m_icount <= 0)
				break;
		}

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_icount--;
		m_op = m_program->read_word(m_pc << 1) & 0x3ff;
		m_i = BITSWAP8(m_op,7,6,5,4,0,1,2,3) & 0xf; // reversed bit-order for 4-bit immediate param (except for XAMR, REDD, SEDD)
		increment_pc();

		// handle opcode
		switch (m_op)
		{
			/* 0x000 */

			case 0x000: case 0x001: case 0x002: case 0x003:
				op_xsp(); break;
			case 0x004: case 0x005: case 0x006: case 0x007:
				op_sem(); break;
			case 0x008: case 0x009: case 0x00a: case 0x00b:
				op_lam(); break;
			case 0x010: case 0x011: case 0x012: case 0x013: case 0x014: case 0x015: case 0x016: case 0x017:
			case 0x018: case 0x019: case 0x01a: case 0x01b: case 0x01c: case 0x01d: case 0x01e: case 0x01f:
				op_lmiiy(); break;
			case 0x020: case 0x021: case 0x022: case 0x023:
				op_lbm(); break;
			case 0x024:
				op_blem(); break;
			case 0x030:
				op_amc(); break;
			case 0x034:
				op_am(); break;
			case 0x03c:
				op_lta(); break;

			case 0x040:
				op_lxa(); break;
			case 0x045:
				op_das(); break;
			case 0x046:
				op_daa(); break;
			case 0x04c:
				op_rec(); break;
			case 0x04f:
				op_sec(); break;
			case 0x050:
				op_lya(); break;
			case 0x054:
				op_iy(); break;
			case 0x058:
				op_ayy(); break;
			case 0x060:
				op_lba(); break;
			case 0x064:
				op_ib(); break;
			case 0x070: case 0x071: case 0x072: case 0x073: case 0x074: case 0x075: case 0x076: case 0x077:
			case 0x078: case 0x079: case 0x07a: case 0x07b: case 0x07c: case 0x07d: case 0x07e: case 0x07f:
				op_lai(); break;

			case 0x080: case 0x081: case 0x082: case 0x083: case 0x084: case 0x085: case 0x086: case 0x087:
			case 0x088: case 0x089: case 0x08a: case 0x08b: case 0x08c: case 0x08d: case 0x08e: case 0x08f:
				op_ai(); break;
			case 0x090:
				op_sed(); break;
			case 0x094:
				op_td(); break;
			case 0x0a0:
				op_seif1(); break;
			case 0x0a1:
				op_secf(); break;
			case 0x0a2:
				op_seif0(); break;
			case 0x0a4:
				op_seie(); break;
			case 0x0a5:
				op_setf(); break;

			case 0x0c0: case 0x0c1: case 0x0c2: case 0x0c3: case 0x0c4: case 0x0c5: case 0x0c6: case 0x0c7:
				op_lar(); break;
			case 0x0d0: case 0x0d1: case 0x0d2: case 0x0d3: case 0x0d4: case 0x0d5: case 0x0d6: case 0x0d7:
			case 0x0d8: case 0x0d9: case 0x0da: case 0x0db: case 0x0dc: case 0x0dd: case 0x0de: case 0x0df:
				op_sedd(); break;
			case 0x0e0: case 0x0e1: case 0x0e2: case 0x0e3: case 0x0e4: case 0x0e5: case 0x0e6: case 0x0e7:
				op_lbr(); break;
			case 0x0f0: case 0x0f1: case 0x0f2: case 0x0f3: case 0x0f4: case 0x0f5: case 0x0f6: case 0x0f7:
			case 0x0f8: case 0x0f9: case 0x0fa: case 0x0fb: case 0x0fc: case 0x0fd: case 0x0fe: case 0x0ff:
				op_xamr(); break;


			/* 0x100 */

			case 0x110: case 0x111:
				op_lmaiy(); break;
			case 0x114: case 0x115:
				op_lmady(); break;
			case 0x118:
				op_lay(); break;
			case 0x120:
				op_or(); break;
			case 0x124:
				op_anem(); break;

			case 0x140: case 0x141: case 0x142: case 0x143: case 0x144: case 0x145: case 0x146: case 0x147:
			case 0x148: case 0x149: case 0x14a: case 0x14b: case 0x14c: case 0x14d: case 0x14e: case 0x14f:
				op_lxi(); break;
			case 0x150: case 0x151: case 0x152: case 0x153: case 0x154: case 0x155: case 0x156: case 0x157:
			case 0x158: case 0x159: case 0x15a: case 0x15b: case 0x15c: case 0x15d: case 0x15e: case 0x15f:
				op_lyi(); break;
			case 0x160: case 0x161: case 0x162: case 0x163: case 0x164: case 0x165: case 0x166: case 0x167:
			case 0x168: case 0x169: case 0x16a: case 0x16b: case 0x16c: case 0x16d: case 0x16e: case 0x16f:
				op_lbi(); break;
			case 0x170: case 0x171: case 0x172: case 0x173: case 0x174: case 0x175: case 0x176: case 0x177:
			case 0x178: case 0x179: case 0x17a: case 0x17b: case 0x17c: case 0x17d: case 0x17e: case 0x17f:
				op_lti(); break;

			case 0x1a0:
				op_tif1(); break;
			case 0x1a1:
				op_ti1(); break;
			case 0x1a2:
				op_tif0(); break;
			case 0x1a3:
				op_ti0(); break;
			case 0x1a5:
				op_ttf(); break;

			case 0x1c0: case 0x1c1: case 0x1c2: case 0x1c3: case 0x1c4: case 0x1c5: case 0x1c6: case 0x1c7:
			case 0x1c8: case 0x1c9: case 0x1ca: case 0x1cb: case 0x1cc: case 0x1cd: case 0x1ce: case 0x1cf:
			case 0x1d0: case 0x1d1: case 0x1d2: case 0x1d3: case 0x1d4: case 0x1d5: case 0x1d6: case 0x1d7:
			case 0x1d8: case 0x1d9: case 0x1da: case 0x1db: case 0x1dc: case 0x1dd: case 0x1de: case 0x1df:
			case 0x1e0: case 0x1e1: case 0x1e2: case 0x1e3: case 0x1e4: case 0x1e5: case 0x1e6: case 0x1e7:
			case 0x1e8: case 0x1e9: case 0x1ea: case 0x1eb: case 0x1ec: case 0x1ed: case 0x1ee: case 0x1ef:
			case 0x1f0: case 0x1f1: case 0x1f2: case 0x1f3: case 0x1f4: case 0x1f5: case 0x1f6: case 0x1f7:
			case 0x1f8: case 0x1f9: case 0x1fa: case 0x1fb: case 0x1fc: case 0x1fd: case 0x1fe: case 0x1ff:
				op_br(); break;


			/* 0x200 */

			case 0x200: case 0x201: case 0x202: case 0x203:
				op_tm(); break;
			case 0x204: case 0x205: case 0x206: case 0x207:
				op_rem(); break;
			case 0x208: case 0x209: case 0x20a: case 0x20b:
				op_xma(); break;
			case 0x210: case 0x211: case 0x212: case 0x213: case 0x214: case 0x215: case 0x216: case 0x217:
			case 0x218: case 0x219: case 0x21a: case 0x21b: case 0x21c: case 0x21d: case 0x21e: case 0x21f:
				op_mnei(); break;
			case 0x220: case 0x221: case 0x222: case 0x223:
				op_xmb(); break;
			case 0x224:
				op_rotr(); break;
			case 0x225:
				op_rotl(); break;
			case 0x230:
				op_smc(); break;
			case 0x234:
				op_alem(); break;
			case 0x23c:
				op_lat(); break;

			case 0x240:
				op_laspx(); break;
			case 0x244:
				op_nega(); break;
			case 0x24f:
				op_tc(); break;
			case 0x250:
				op_laspy(); break;
			case 0x254:
				op_dy(); break;
			case 0x258:
				op_syy(); break;
			case 0x260:
				op_lab(); break;
			case 0x267:
				op_db(); break;
			case 0x270: case 0x271: case 0x272: case 0x273: case 0x274: case 0x275: case 0x276: case 0x277:
			case 0x278: case 0x279: case 0x27a: case 0x27b: case 0x27c: case 0x27d: case 0x27e: case 0x27f:
				op_alei(); break;

			case 0x280: case 0x281: case 0x282: case 0x283: case 0x284: case 0x285: case 0x286: case 0x287:
			case 0x288: case 0x289: case 0x28a: case 0x28b: case 0x28c: case 0x28d: case 0x28e: case 0x28f:
				op_ynei(); break;
			case 0x290:
				op_red(); break;
			case 0x2a0:
				op_reif1(); break;
			case 0x2a1:
				op_recf(); break;
			case 0x2a2:
				op_reif0(); break;
			case 0x2a4:
				op_reie(); break;
			case 0x2a5:
				op_retf(); break;

			case 0x2c0: case 0x2c1: case 0x2c2: case 0x2c3: case 0x2c4: case 0x2c5: case 0x2c6: case 0x2c7:
				op_lra(); break;
			case 0x2d0: case 0x2d1: case 0x2d2: case 0x2d3: case 0x2d4: case 0x2d5: case 0x2d6: case 0x2d7:
			case 0x2d8: case 0x2d9: case 0x2da: case 0x2db: case 0x2dc: case 0x2dd: case 0x2de: case 0x2df:
				op_redd(); break;
			case 0x2e0: case 0x2e1: case 0x2e2: case 0x2e3: case 0x2e4: case 0x2e5: case 0x2e6: case 0x2e7:
				op_lrb(); break;


			/* 0x300 */
			case 0x320:
				op_comb(); break;
			case 0x324:
				op_bnem(); break;

			case 0x340: case 0x341: case 0x342: case 0x343: case 0x344: case 0x345: case 0x346: case 0x347:
			case 0x348: case 0x349: case 0x34a: case 0x34b: case 0x34c: case 0x34d: case 0x34e: case 0x34f:
			case 0x350: case 0x351: case 0x352: case 0x353: case 0x354: case 0x355: case 0x356: case 0x357:
			case 0x358: case 0x359: case 0x35a: case 0x35b: case 0x35c: case 0x35d: case 0x35e: case 0x35f:
				op_lpu(); break;
			case 0x360: case 0x361: case 0x362: case 0x363: case 0x364: case 0x365: case 0x366: case 0x367:
				op_tbr(); break;
			case 0x368: case 0x369: case 0x36a: case 0x36b: case 0x36c: case 0x36d: case 0x36e: case 0x36f:
				op_p(); break;

			case 0x3a4:
				op_rtni(); break;
			case 0x3a7:
				op_rtn(); break;

			case 0x3c0: case 0x3c1: case 0x3c2: case 0x3c3: case 0x3c4: case 0x3c5: case 0x3c6: case 0x3c7:
			case 0x3c8: case 0x3c9: case 0x3ca: case 0x3cb: case 0x3cc: case 0x3cd: case 0x3ce: case 0x3cf:
			case 0x3d0: case 0x3d1: case 0x3d2: case 0x3d3: case 0x3d4: case 0x3d5: case 0x3d6: case 0x3d7:
			case 0x3d8: case 0x3d9: case 0x3da: case 0x3db: case 0x3dc: case 0x3dd: case 0x3de: case 0x3df:
			case 0x3e0: case 0x3e1: case 0x3e2: case 0x3e3: case 0x3e4: case 0x3e5: case 0x3e6: case 0x3e7:
			case 0x3e8: case 0x3e9: case 0x3ea: case 0x3eb: case 0x3ec: case 0x3ed: case 0x3ee: case 0x3ef:
			case 0x3f0: case 0x3f1: case 0x3f2: case 0x3f3: case 0x3f4: case 0x3f5: case 0x3f6: case 0x3f7:
			case 0x3f8: case 0x3f9: case 0x3fa: case 0x3fb: case 0x3fc: case 0x3fd: case 0x3fe: case 0x3ff:
				op_cal(); break;


			default:
				op_illegal(); break;
		} /* big switch */
	}
}
