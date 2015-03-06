// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family cores

  References:
  - 1985 #AP1 Hitachi 4-bit Single-Chip Microcomputer Data Book
  - 1988 HMCS400 Series Handbook (note: *400 is a newer MCU series, with similarities)

*/

enum
{
	FAMILY_HMCS42 = 0,
	FAMILY_HMCS43,
	FAMILY_HMCS44,
	FAMILY_HMCS45,
	FAMILY_HMCS46,
	FAMILY_HMCS47,
};

#define IS_CMOS true
#define IS_PMOS false

#include "hmcs40.h"
#include "debugger.h"

#include "hmcs40op.inc"

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
	AM_RANGE(0x0400, 0x043f) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_2k, AS_PROGRAM, 16, hmcs40_cpu_device)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x087f) AM_ROM
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
hmcs43_cpu_device::hmcs43_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_cmos, const char *shortname)
	: hmcs40_cpu_device(mconfig, type, name, tag, owner, clock, FAMILY_HMCS43, is_cmos, 3, 10, 11, ADDRESS_MAP_NAME(program_1k), 7, ADDRESS_MAP_NAME(data_80x4), shortname, __FILE__)
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


hmcs44_cpu_device::hmcs44_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_cmos, const char *shortname)
	: hmcs40_cpu_device(mconfig, type, name, tag, owner, clock, FAMILY_HMCS44, is_cmos, 4, 11, 12, ADDRESS_MAP_NAME(program_2k), 8, ADDRESS_MAP_NAME(data_160x4), shortname, __FILE__)
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


hmcs45_cpu_device::hmcs45_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_cmos, const char *shortname)
	: hmcs40_cpu_device(mconfig, type, name, tag, owner, clock, FAMILY_HMCS45, is_cmos, 4, 11, 12, ADDRESS_MAP_NAME(program_2k), 8, ADDRESS_MAP_NAME(data_160x4), shortname, __FILE__)
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
void hmcs40_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c",
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
	
	UINT8 defval = (m_is_cmos) ? 0xf : 0;
	m_read_r0.resolve_safe(defval);
	m_read_r1.resolve_safe(defval);
	m_read_r2.resolve_safe(defval);
	m_read_r3.resolve_safe(defval);
	m_read_r4.resolve_safe(defval);
	m_read_r5.resolve_safe(defval);
	m_read_r6.resolve_safe(defval);
	m_read_r7.resolve_safe(defval);
	
	m_write_r0.resolve_safe();
	m_write_r1.resolve_safe();
	m_write_r2.resolve_safe();
	m_write_r3.resolve_safe();
	m_write_r4.resolve_safe();
	m_write_r5.resolve_safe();
	m_write_r6.resolve_safe();
	m_write_r7.resolve_safe();

	m_read_d.resolve_safe((m_is_cmos) ? 0xffff : 0);
	m_write_d.resolve_safe();

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_op = 0;
	m_prev_op = 0;
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
	memset(m_r, 0, sizeof(m_r));
	m_d = 0;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
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
	
	// clear i/o
	m_d = (m_is_cmos) ? 0xffff : 0;
	for (int i = 0; i < 16; i++)
		hmcs40_cpu_device::write_d(i, (m_is_cmos) ? 1 : 0);
	
	for (int i = 0; i < 8; i++)
		hmcs40_cpu_device::write_r(i, (m_is_cmos) ? 0xf : 0);
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
	while (m_icount > 0)
	{
		m_icount--;
		
		// LPU is handled 1 cycle later
		if ((m_prev_op & 0x3e0) == 0x340)
			m_pc = ((m_page << 6) | (m_pc & 0x3f)) & m_pcmask;

		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;
		
		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_word(m_pc << 1) & 0x3ff;
		increment_pc();
	}
}
