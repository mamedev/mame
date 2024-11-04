// license:BSD-3-Clause
// copyright-holders:hap
/*

Hitachi HMCS40 MCU family cores

References:
- 1985 #AP1 Hitachi 4-bit Single-Chip Microcomputer Data Book
- 1988 HMCS400 Series Handbook (note: *400 is a newer MCU series, with similarities)
- opcode decoding by Tatsuyuki Satoh, Olivier Galibert, Kevin Horton, Lord Nightmare
  (verified a while later after new documentation was found)

TODO:
- Which opcodes block interrupt on next cycle? LPU is obvious, and Gakken Crazy Kong
  (VFD tabletop game) locks up if CAL doesn't do it. Maybe BR? But that's a
  dangerous assumption since tight infinite loops wouldn't work right anymore.

*/

#include "emu.h"
#include "hmcs40.h"
#include "hmcs40d.h"

#define IS_PMOS 0
#define IS_CMOS ~0


//-------------------------------------------------
//  device types
//-------------------------------------------------

// HMCS42/C/CL, 28 pins, 22 I/O lines, (512+32)x10 ROM, 32x4 RAM, no B or SPY register
//DEFINE_DEVICE_TYPE(HD38702, hd38702_device, "hd38702", "Hitachi HD38702") // PMOS
//DEFINE_DEVICE_TYPE(HD44700, hd44700_device, "hd44700", "Hitachi HD44700") // CMOS
//DEFINE_DEVICE_TYPE(HD44708, hd44708_device, "hd44708", "Hitachi HD44708") // CMOS, low-power

// HMCS43/C/CL, 42 pins, 32 I/O lines, (1024+64)x10 ROM, 80x4 RAM
DEFINE_DEVICE_TYPE(HD38750, hd38750_device, "hd38750", "Hitachi HD38750") // PMOS
DEFINE_DEVICE_TYPE(HD38755, hd38755_device, "hd38755", "Hitachi HD38755") // ceramic filter oscillator type
DEFINE_DEVICE_TYPE(HD44750, hd44750_device, "hd44750", "Hitachi HD44750") // CMOS
DEFINE_DEVICE_TYPE(HD44758, hd44758_device, "hd44758", "Hitachi HD44758") // CMOS, low-power

// HMCS44A/C/CL, 42 pins, 32 I/O lines, (2048+128)x10 ROM, 160x4 RAM
DEFINE_DEVICE_TYPE(HD38800, hd38800_device, "hd38800", "Hitachi HD38800") // PMOS
DEFINE_DEVICE_TYPE(HD38805, hd38805_device, "hd38805", "Hitachi HD38805") // ceramic filter oscillator type
DEFINE_DEVICE_TYPE(HD44801, hd44801_device, "hd44801", "Hitachi HD44801") // CMOS
DEFINE_DEVICE_TYPE(HD44808, hd44808_device, "hd44808", "Hitachi HD44808") // CMOS, low-power

// HMCS45A/C/CL, 54 pins(QFP) or 64 pins(DIP), 44 I/O lines, (2048+128)x10 ROM, 160x4 RAM
DEFINE_DEVICE_TYPE(HD38820, hd38820_device, "hd38820", "Hitachi HD38820") // PMOS
DEFINE_DEVICE_TYPE(HD38825, hd38825_device, "hd38825", "Hitachi HD38825") // ceramic filter oscillator type
DEFINE_DEVICE_TYPE(HD44820, hd44820_device, "hd44820", "Hitachi HD44820") // CMOS
DEFINE_DEVICE_TYPE(HD44828, hd44828_device, "hd44828", "Hitachi HD44828") // CMOS, low-power

// HMCS46C/CL, 42 pins, 32 I/O lines, 4096x10 ROM, 256x4 RAM (no PMOS version exists)
DEFINE_DEVICE_TYPE(HD44840, hd44840_device, "hd44840", "Hitachi HD44840") // CMOS
DEFINE_DEVICE_TYPE(HD44848, hd44848_device, "hd44848", "Hitachi HD44848") // CMOS, low-power

// HMCS47A/C/CL, 54 pins(QFP) or 64 pins(DIP), 44 I/O lines, 4096x10 ROM, 256x4 RAM
DEFINE_DEVICE_TYPE(HD38870, hd38870_device, "hd38870", "Hitachi HD38870") // PMOS
DEFINE_DEVICE_TYPE(HD44860, hd44860_device, "hd44860", "Hitachi HD44860") // CMOS
DEFINE_DEVICE_TYPE(HD44868, hd44868_device, "hd44868", "Hitachi HD44868") // CMOS, low-power

// LCD-III, 64 pins, HMCS44C core, LCDC with 4 commons and 32 segments
//DEFINE_DEVICE_TYPE(HD44790, hd44790_device, "hd44790", "Hitachi HD44790") // CMOS
//DEFINE_DEVICE_TYPE(HD44795, hd44795_device, "hd44795", "Hitachi HD44795") // CMOS, low-power

// LCD-IV, 64 pins, HMCS46C core, LCDC with 4 commons and 32 segments
//DEFINE_DEVICE_TYPE(HD613901, hd613901_device, "hd613901", "Hitachi HD613901") // CMOS


//-------------------------------------------------
//  constructor
//-------------------------------------------------

hmcs40_cpu_device::hmcs40_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int family, u16 polarity, int stack_levels, int pcwidth, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 16, prgwidth, -1, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_pcwidth(pcwidth),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth),
	m_family(family),
	m_polarity(polarity),
	m_stack_levels(stack_levels),
	m_read_r(*this, polarity & 0xf),
	m_write_r(*this),
	m_read_d(*this, polarity),
	m_write_d(*this)
{ }

hmcs40_cpu_device::~hmcs40_cpu_device() { }


hmcs43_cpu_device::hmcs43_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity) :
	hmcs40_cpu_device(mconfig, type, tag, owner, clock, HMCS43_FAMILY, polarity, 3 /* stack levels */, 10 /* pc width */, 11 /* prg width */, address_map_constructor(FUNC(hmcs43_cpu_device::program_1k), this), 7 /* data width */, address_map_constructor(FUNC(hmcs43_cpu_device::data_80x4), this))
{ }

hd38750_device::hd38750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs43_cpu_device(mconfig, HD38750, tag, owner, clock, IS_PMOS)
{ }
hd38755_device::hd38755_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs43_cpu_device(mconfig, HD38755, tag, owner, clock, IS_PMOS)
{ }
hd44750_device::hd44750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs43_cpu_device(mconfig, HD44750, tag, owner, clock, IS_CMOS)
{ }
hd44758_device::hd44758_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs43_cpu_device(mconfig, HD44758, tag, owner, clock, IS_CMOS)
{ }


hmcs44_cpu_device::hmcs44_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity) :
	hmcs40_cpu_device(mconfig, type, tag, owner, clock, HMCS44_FAMILY, polarity, 4, 11, 12, address_map_constructor(FUNC(hmcs44_cpu_device::program_2k), this), 8, address_map_constructor(FUNC(hmcs44_cpu_device::data_160x4), this))
{ }

hd38800_device::hd38800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs44_cpu_device(mconfig, HD38800, tag, owner, clock, IS_PMOS)
{ }
hd38805_device::hd38805_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs44_cpu_device(mconfig, HD38805, tag, owner, clock, IS_PMOS)
{ }
hd44801_device::hd44801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs44_cpu_device(mconfig, HD44801, tag, owner, clock, IS_CMOS)
{ }
hd44808_device::hd44808_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs44_cpu_device(mconfig, HD44808, tag, owner, clock, IS_CMOS)
{ }


hmcs45_cpu_device::hmcs45_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity) :
	hmcs40_cpu_device(mconfig, type, tag, owner, clock, HMCS45_FAMILY, polarity, 4, 11, 12, address_map_constructor(FUNC(hmcs45_cpu_device::program_2k), this), 8, address_map_constructor(FUNC(hmcs45_cpu_device::data_160x4), this))
{ }

hd38820_device::hd38820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs45_cpu_device(mconfig, HD38820, tag, owner, clock, IS_PMOS)
{ }
hd38825_device::hd38825_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs45_cpu_device(mconfig, HD38825, tag, owner, clock, IS_PMOS)
{ }
hd44820_device::hd44820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs45_cpu_device(mconfig, HD44820, tag, owner, clock, IS_CMOS)
{ }
hd44828_device::hd44828_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs45_cpu_device(mconfig, HD44828, tag, owner, clock, IS_CMOS)
{ }


hmcs46_cpu_device::hmcs46_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity) :
	hmcs40_cpu_device(mconfig, type, tag, owner, clock, HMCS46_FAMILY, polarity, 4, 12, 12, address_map_constructor(FUNC(hmcs46_cpu_device::program_2k), this), 8, address_map_constructor(FUNC(hmcs46_cpu_device::data_256x4), this))
{ }

hd44840_device::hd44840_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs46_cpu_device(mconfig, HD44840, tag, owner, clock, IS_CMOS)
{ }
hd44848_device::hd44848_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs46_cpu_device(mconfig, HD44848, tag, owner, clock, IS_CMOS)
{ }


hmcs47_cpu_device::hmcs47_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity) :
	hmcs40_cpu_device(mconfig, type, tag, owner, clock, HMCS47_FAMILY, polarity, 4, 12, 12, address_map_constructor(FUNC(hmcs47_cpu_device::program_2k), this), 8, address_map_constructor(FUNC(hmcs47_cpu_device::data_256x4), this))
{ }

hd38870_device::hd38870_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs47_cpu_device(mconfig, HD38870, tag, owner, clock, IS_PMOS)
{ }
hd44860_device::hd44860_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs47_cpu_device(mconfig, HD44860, tag, owner, clock, IS_CMOS)
{ }
hd44868_device::hd44868_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs47_cpu_device(mconfig, HD44868, tag, owner, clock, IS_CMOS)
{ }


//-------------------------------------------------
//  initialization
//-------------------------------------------------

void hmcs40_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;
	m_pcmask = (1 << m_pcwidth) - 1;

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_op = 0;
	m_prev_op = 0;
	m_i = 0;
	m_eint_line = 0;
	m_halt = 0;
	m_prescaler = 0;
	m_block_int = false;

	m_pc = 0;
	m_prev_pc = 0;
	m_pc_upper = 0;
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
	save_item(NAME(m_prescaler));
	save_item(NAME(m_block_int));

	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_pc_upper));
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
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_s).formatstr("%2s").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%04X"); // 1
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 2
	state_add(++m_state_count, "B", m_b).formatstr("%01X"); // 3
	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 4
	state_add(++m_state_count, "SPX", m_spx).formatstr("%01X"); // 5
	state_add(++m_state_count, "Y", m_y).formatstr("%01X"); // 6
	state_add(++m_state_count, "SPY", m_spy).formatstr("%01X"); // 7

	state_add(++m_state_count, "S", m_s).formatstr("%01X").noshow(); // 8
	state_add(++m_state_count, "C", m_c).formatstr("%01X").noshow(); // 9

	set_icountptr(m_icount);
}

void hmcs40_cpu_device::device_reset()
{
	m_pc = m_pcmask;
	m_prev_op = m_op = 0;

	// clear interrupts
	m_cf = 0;
	m_ie = 0;
	m_iri = m_irt = 0;
	m_if[0] = m_if[1] = m_tf = 1;

	// all I/O ports set to input
	reset_io();

	// HMCS46/47 R70 set to 1 (already the default on CMOS devices)
	if (m_family == HMCS46_FAMILY || m_family == HMCS47_FAMILY)
		m_r[7] |= 1;
}


//-------------------------------------------------
//  disasm
//-------------------------------------------------

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

std::unique_ptr<util::disasm_interface> hmcs40_cpu_device::create_disassembler()
{
	return std::make_unique<hmcs40_disassembler>();
}


//-------------------------------------------------
//  internal memory maps
//-------------------------------------------------

/*

On HMCS42/43/44/45, only half of the ROM address range contains user-executable
code, there is up to 128 bytes of pattern data in the 2nd half. The 2nd half
also includes a couple of pages with factory test code by Hitachi, only executable
when MCU test mode is enabled externally (TEST pin). This data can still be accessed
with the P opcode.

On HMCS46/47, the 2nd half can be jumped to with a bank bit from R70. These MCUs
have 2 more banks with factory test code, but that part of the ROM is only accessible
under MCU test mode.

*/

void hmcs40_cpu_device::program_1k(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}

void hmcs40_cpu_device::program_2k(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}


void hmcs40_cpu_device::data_80x4(address_map &map)
{
	map(0x00, 0x3f).ram();
	map(0x40, 0x4f).ram().mirror(0x30);
}

void hmcs40_cpu_device::data_160x4(address_map &map)
{
	map(0x00, 0x7f).ram();
	map(0x80, 0x8f).ram().mirror(0x30);
	map(0xc0, 0xcf).ram().mirror(0x30);
}

void hmcs40_cpu_device::data_256x4(address_map &map)
{
	map(0x00, 0xff).ram();
}

device_memory_interface::space_config_vector hmcs40_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  i/o ports
//-------------------------------------------------

void hmcs40_cpu_device::reset_io()
{
	m_d = m_polarity;
	m_write_d(m_polarity);

	for (int i = 0; i < 8; i++)
		hmcs40_cpu_device::write_r(i, m_polarity);
}

u8 hmcs40_cpu_device::read_r(u8 index)
{
	index &= 7;
	u8 inp = m_read_r[index](index);

	if (m_polarity)
		return (inp & m_r[index]) & 0xf;
	else
		return (inp | m_r[index]) & 0xf;
}

void hmcs40_cpu_device::write_r(u8 index, u8 data)
{
	index &= 7;
	data &= 0xf;
	m_r[index] = data;
	m_write_r[index](index, data);
}

int hmcs40_cpu_device::read_d(u8 index)
{
	index &= 0xf;
	u16 inp = m_read_d(0, 1 << index);

	if (m_polarity)
		return BIT(inp & m_d, index);
	else
		return BIT(inp | m_d, index);
}

void hmcs40_cpu_device::write_d(u8 index, int state)
{
	index &= 0xf;
	u16 mask = 1 << index;

	m_d = (m_d & ~mask) | (state ? mask : 0);
	m_write_d(0, m_d, mask);
}

// HMCS43:
// R0 is input-only, R1 is i/o, R2,R3 are output-only, no R4-R7
// D0-D3 are i/o, D4-D15 are output-only

u8 hmcs43_cpu_device::read_r(u8 index)
{
	index &= 7;

	if (index >= 2)
		logerror("read from %s port R%d @ $%04X\n", (index >= 4) ? "unknown" : "output", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs43_cpu_device::write_r(u8 index, u8 data)
{
	index &= 7;

	if (index != 0 && index < 4)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("ineffective write to port R%d = $%X @ $%04X\n", index, data & 0xf, m_prev_pc);
}

int hmcs43_cpu_device::read_d(u8 index)
{
	index &= 15;

	if (index >= 4)
		logerror("read from output pin D%d @ $%04X\n", index, m_prev_pc);

	return hmcs40_cpu_device::read_d(index);
}

// HMCS44:
// R0-R3 are i/o, R4,R5 are extra registers, no R6,R7
// D0-D15 are i/o

u8 hmcs44_cpu_device::read_r(u8 index)
{
	index &= 7;

	if (index >= 6)
		logerror("read from unknown port R%d @ $%04X\n", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs44_cpu_device::write_r(u8 index, u8 data)
{
	index &= 7;

	if (index < 6)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("ineffective write to port R%d = $%X @ $%04X\n", index, data & 0xf, m_prev_pc);
}

// HMCS45:
// R0-R5 are i/o, R6 is output-only, no R7
// D0-D15 are i/o

u8 hmcs45_cpu_device::read_r(u8 index)
{
	index &= 7;

	if (index >= 6)
		logerror("read from %s port R%d @ $%04X\n", (index == 7) ? "unknown" : "output", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs45_cpu_device::write_r(u8 index, u8 data)
{
	index &= 7;

	if (index != 7)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("ineffective write to port R%d = $%X @ $%04X\n", index, data & 0xf, m_prev_pc);
}

// HMCS46:
// R0-R3 are i/o, R4,R5,R7 are extra registers, no R6
// D0-D15 are i/o

u8 hmcs46_cpu_device::read_r(u8 index)
{
	index &= 7;

	if (index == 6)
		logerror("read from unknown port R%d @ $%04X\n", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}

void hmcs46_cpu_device::write_r(u8 index, u8 data)
{
	index &= 7;

	if (index != 6)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("ineffective write to port R%d = $%X @ $%04X\n", index, data & 0xf, m_prev_pc);
}

// HMCS47:
// R0-R5 are i/o, R6 is output-only, R7 is an extra register
// D0-D15 are i/o

u8 hmcs47_cpu_device::read_r(u8 index)
{
	index &= 7;

	if (index == 6)
		logerror("read from output port R%d @ $%04X\n", index, m_prev_pc);

	return hmcs40_cpu_device::read_r(index);
}


//-------------------------------------------------
//  interrupt/timer
//-------------------------------------------------

void hmcs40_cpu_device::take_interrupt()
{
	push_stack();
	m_ie = 0;

	// line 0/1 for external interrupt, let's use 2 for t/c interrupt
	int line = (m_iri) ? m_eint_line : 2;
	standard_irq_callback(line, m_pc);

	// vector $3f, on page 0(timer/counter), or page 1(external)
	// external interrupt has priority over t/c interrupt
	m_pc = 0x3f | (m_iri ? 0x40 : 0);
	if (m_iri)
		m_iri = 0;
	else
		m_irt = 0;

	m_prev_pc = m_pc;

	cycle();
}

void hmcs40_cpu_device::execute_set_input(int line, int state)
{
	state = state ? 1 : 0;

	// halt/unhalt mcu
	if (line == HMCS40_INPUT_LINE_HLT && state != m_halt)
	{
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
			clock_timer();
	}

	m_int[line] = state;
}

void hmcs40_cpu_device::clock_timer()
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

void hmcs40_cpu_device::clock_prescaler()
{
	m_prescaler = (m_prescaler + 1) & 0x3f;

	// timer prescaler overflow
	if (m_prescaler == 0 && !m_cf)
		clock_timer();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

inline void hmcs40_cpu_device::increment_pc()
{
	// PC lower bits is a LFSR identical to TI TMS1000
	u8 mask = 0x3f;
	u8 low = m_pc & mask;
	int fb = (low << 1 & 0x20) == (low & 0x20);

	if (low == (mask >> 1))
		fb = 1;
	else if (low == mask)
		fb = 0;

	m_pc = (m_pc & ~mask) | ((m_pc << 1 | fb) & mask);
}

void hmcs40_cpu_device::cycle()
{
	m_icount--;
	clock_prescaler();
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
		if ((m_prev_op & 0x7e0) == 0x340)
			m_pc = ((m_pc_upper << 6) | (m_pc & 0x3f)) & m_pcmask;

		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// check/handle interrupt
		if (m_ie && (m_iri || m_irt) && !m_block_int)
			take_interrupt();
		m_block_int = false;

		// fetch next opcode
		debugger_instruction_hook(m_pc);
		m_op = m_program->read_word(m_pc) & 0x3ff;
		m_i = bitswap<4>(m_op,0,1,2,3); // reversed bit-order for 4-bit immediate param (except for XAMR)
		increment_pc();
		cycle();

		// handle opcode
		switch (m_op & 0x3f0)
		{
			case 0x1c0: case 0x1d0: case 0x1e0: case 0x1f0: op_br(); break;
			case 0x3c0: case 0x3d0: case 0x3e0: case 0x3f0: op_cal(); break;
			case 0x340: case 0x350: op_lpu(); break;

			case 0x010: op_lmiiy(); break;
			case 0x070: op_lai(); break;
			case 0x080: op_ai(); break;
			case 0x0f0: op_xamr(); break;
			case 0x140: op_lxi(); break;
			case 0x150: op_lyi(); break;
			case 0x160: op_lbi(); break;
			case 0x170: op_lti(); break;
			case 0x210: op_mnei(); break;
			case 0x270: op_alei(); break;
			case 0x280: op_ynei(); break;

			default:
				switch (m_op & 0x3fc)
				{
			case 0x0c0: case 0x0c4: op_lar(); break;
			case 0x0e0: case 0x0e4: op_lbr(); break;
			case 0x2c0: case 0x2c4: op_lra(); break;
			case 0x2e0: case 0x2e4: op_lrb(); break;
			case 0x360: case 0x364: op_tbr(); break;
			case 0x368: case 0x36c: op_p(); break;

			case 0x000: op_xsp(); break;
			case 0x004: op_sem(); break;
			case 0x008: op_lam(); break;
			case 0x020: op_lbm(); break;
			case 0x0d0: op_sedd(); break;
			case 0x200: op_tm(); break;
			case 0x204: op_rem(); break;
			case 0x208: op_xma(); break;
			case 0x220: op_xmb(); break;
			case 0x2d0: op_redd(); break;

			default:
				switch (m_op)
				{
			case 0x024: op_blem(); break;
			case 0x030: op_amc(); break;
			case 0x034: op_am(); break;
			case 0x03c: op_lta(); break;
			case 0x040: op_lxa(); break;
			case 0x045: op_das(); break;
			case 0x046: op_daa(); break;
			case 0x04c: op_rec(); break;
			case 0x04f: op_sec(); break;
			case 0x050: op_lya(); break;
			case 0x054: op_iy(); break;
			case 0x058: op_ayy(); break;
			case 0x060: op_lba(); break;
			case 0x064: op_ib(); break;
			case 0x090: op_sed(); break;
			case 0x094: op_td(); break;
			case 0x0a0: op_seif1(); break;
			case 0x0a1: op_secf(); break;
			case 0x0a2: op_seif0(); break;
			case 0x0a4: op_seie(); break;
			case 0x0a5: op_setf(); break;

			case 0x110: case 0x111: op_lmaiy(); break;
			case 0x114: case 0x115: op_lmady(); break;
			case 0x118: op_lay(); break;
			case 0x120: op_or(); break;
			case 0x124: op_anem(); break;
			case 0x1a0: op_tif1(); break;
			case 0x1a1: op_ti1(); break;
			case 0x1a2: op_tif0(); break;
			case 0x1a3: op_ti0(); break;
			case 0x1a5: op_ttf(); break;

			case 0x224: op_rotr(); break;
			case 0x225: op_rotl(); break;
			case 0x230: op_smc(); break;
			case 0x234: op_alem(); break;
			case 0x23c: op_lat(); break;
			case 0x240: op_laspx(); break;
			case 0x244: op_nega(); break;
			case 0x24f: op_tc(); break;
			case 0x250: op_laspy(); break;
			case 0x254: op_dy(); break;
			case 0x258: op_syy(); break;
			case 0x260: op_lab(); break;
			case 0x267: op_db(); break;
			case 0x290: op_red(); break;
			case 0x2a0: op_reif1(); break;
			case 0x2a1: op_recf(); break;
			case 0x2a2: op_reif0(); break;
			case 0x2a4: op_reie(); break;
			case 0x2a5: op_retf(); break;

			case 0x320: op_comb(); break;
			case 0x324: op_bnem(); break;
			case 0x3a4: op_rtni(); break;
			case 0x3a7: op_rtn(); break;

			default: op_illegal(); break;
				}
				break; // 0x3ff

				}
				break; // 0x3fc

		} // 0x3f0
	}
}
