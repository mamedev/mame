// license:BSD-3-Clause
// copyright-holders:hap
/*

Hitachi HMCS400 MCU family cores

It's the successor to HMCS40, it was mainly used in consumer electronics, not
much in games.

Compared to HMCS40, it accepts a higher clock speed, and it has more versatile
peripherals, like a serial interface. The opcodes were mostly kept the same.
They added an extra RAM addressing mode, and interrupt-related opcodes were
removed (interrupt flags are via memory-mapped I/O).

TODO:
- add serial interface
- current I/O ports are hardcoded for HMS402/4/8, which will need to be changed
  when other MCU types are added
- do the LAW/LWA opcodes not work on early revisions of HMCS400? the 1988 user
  manual warns that the W register is write-only, and that there is no efficient
  way to save this register when using interrupts
- what happens when accessing ROM/RAM out of address range? Hitachi documentation
  says 'unused', but maybe it's mirrored?

*/

#include "emu.h"
#include "hmcs400.h"
#include "hmcs400d.h"


//-------------------------------------------------
//  device types
//-------------------------------------------------

// C = standard
// CL = low-power
// AC = high-speed

// HMCS408, HMCS414, HMCS424 have a mask option for the system clock divider
// rev 2 apparently added LAW/LWA opcodes?

// HMCS402C/CL/AC, 64 pins DP-64S or FP-64, 2Kx10 ROM, 160x4 RAM
DEFINE_DEVICE_TYPE(HD614022, hd614022_device, "hd614022", "Hitachi HD614022") // C, rev 2
DEFINE_DEVICE_TYPE(HD614023, hd614023_device, "hd614023", "Hitachi HD614023") // C, rev 1
DEFINE_DEVICE_TYPE(HD614025, hd614025_device, "hd614025", "Hitachi HD614025") // CL, rev 2
DEFINE_DEVICE_TYPE(HD614026, hd614026_device, "hd614026", "Hitachi HD614026") // CL, rev 1
DEFINE_DEVICE_TYPE(HD614028, hd614028_device, "hd614028", "Hitachi HD614028") // AC, rev 2
DEFINE_DEVICE_TYPE(HD614029, hd614029_device, "hd614029", "Hitachi HD614029") // AC, rev 1

// HMCS404C/CL/AC, 64 pins DP-64S or FP-64, 4Kx10 ROM, 256x4 RAM
DEFINE_DEVICE_TYPE(HD614042, hd614042_device, "hd614042", "Hitachi HD614042") // C, rev 2
DEFINE_DEVICE_TYPE(HD614043, hd614043_device, "hd614043", "Hitachi HD614043") // C, rev 1
DEFINE_DEVICE_TYPE(HD614045, hd614045_device, "hd614045", "Hitachi HD614045") // CL, rev 2
DEFINE_DEVICE_TYPE(HD614046, hd614046_device, "hd614046", "Hitachi HD614046") // CL, rev 1
DEFINE_DEVICE_TYPE(HD614048, hd614048_device, "hd614048", "Hitachi HD614048") // AC, rev 2
DEFINE_DEVICE_TYPE(HD614049, hd614049_device, "hd614049", "Hitachi HD614049") // AC, rev 1

// HMCS408C/CL/AC, 64 pins DP-64S or FP-64, 8Kx10 ROM, 512x4 RAM
DEFINE_DEVICE_TYPE(HD614080, hd614080_device, "hd614080", "Hitachi HD614080") // C, rev 2
DEFINE_DEVICE_TYPE(HD614081, hd614081_device, "hd614081", "Hitachi HD614081") // C, rev 1
DEFINE_DEVICE_TYPE(HD614085, hd614085_device, "hd614085", "Hitachi HD614085") // CL, rev 2
DEFINE_DEVICE_TYPE(HD614086, hd614086_device, "hd614086", "Hitachi HD614086") // CL, rev 1
DEFINE_DEVICE_TYPE(HD614088, hd614088_device, "hd614088", "Hitachi HD614088") // AC, rev 2
DEFINE_DEVICE_TYPE(HD614089, hd614089_device, "hd614089", "Hitachi HD614089") // AC, rev 1


//-------------------------------------------------
//  constructor
//-------------------------------------------------

hmcs400_cpu_device::hmcs400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size) :
	cpu_device(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_LITTLE, 16, 14, -1, address_map_constructor(FUNC(hmcs400_cpu_device::program_map), this)),
	m_data_config("data", ENDIANNESS_LITTLE, 8, 10, 0, address_map_constructor(FUNC(hmcs400_cpu_device::data_map), this)),
	m_ram(*this, "ram%u", 0U),
	m_nvram_defval(0),
	m_nvram_battery(true),
	m_rom_size(rom_size),
	m_ram_size(ram_size),
	m_has_div(false),
	m_divider(8),
	m_read_r(*this, 0),
	m_write_r(*this),
	m_read_d(*this, 0),
	m_write_d(*this),
	m_stop_cb(*this)
{
	// disable nvram by default (set to true if MCU is battery-backed when in stop mode)
	nvram_enable_backup(false);
}

hmcs400_cpu_device::~hmcs400_cpu_device() { }


hmcs402_cpu_device::hmcs402_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x800, 96)
{ }

hd614022_device::hd614022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614022, tag, owner, clock)
{ }
hd614023_device::hd614023_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614023, tag, owner, clock)
{ }
hd614025_device::hd614025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614025, tag, owner, clock)
{ }
hd614026_device::hd614026_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614026, tag, owner, clock)
{ }
hd614028_device::hd614028_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614028, tag, owner, clock)
{ }
hd614029_device::hd614029_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614029, tag, owner, clock)
{ }


hmcs404_cpu_device::hmcs404_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x1000, 192)
{ }

hd614042_device::hd614042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614042, tag, owner, clock)
{ }
hd614043_device::hd614043_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614043, tag, owner, clock)
{ }
hd614045_device::hd614045_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614045, tag, owner, clock)
{ }
hd614046_device::hd614046_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614046, tag, owner, clock)
{ }
hd614048_device::hd614048_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614048, tag, owner, clock)
{ }
hd614049_device::hd614049_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614049, tag, owner, clock)
{ }


hmcs408_cpu_device::hmcs408_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x2000, 448)
{
	m_has_div = true;
}

hd614080_device::hd614080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614080, tag, owner, clock)
{ }
hd614081_device::hd614081_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614081, tag, owner, clock)
{ }
hd614085_device::hd614085_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614085, tag, owner, clock)
{ }
hd614086_device::hd614086_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614086, tag, owner, clock)
{ }
hd614088_device::hd614088_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614088, tag, owner, clock)
{ }
hd614089_device::hd614089_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614089, tag, owner, clock)
{ }


//-------------------------------------------------
//  initialization
//-------------------------------------------------

void hmcs400_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_sp = 0;
	m_op = 0;
	m_param = 0;
	m_i = 0;

	m_a = 0;
	m_b = 0;
	m_w = 0;
	m_x = 0;
	m_spx = 0;
	m_y = 0;
	m_spy = 0;
	m_st = 0;
	m_ca = 0;
	m_standby = false;
	m_stop = false;

	memset(m_r, 0, sizeof(m_r));
	memset(m_r_mask, 0, sizeof(m_r_mask));
	m_d = 0;
	m_d_mask = 0;

	m_int_line[0] = m_int_line[1] = 1;
	m_irq_flags = 0;
	m_pmr = 0;
	m_prescaler = 0;
	m_timer_mode[0] = m_timer_mode[1] = 0;
	m_timer_div[0] = m_timer_div[1] = 0;
	m_timer_count[0] = m_timer_count[1] = 0;
	m_timer_load = 0;
	m_timer_b_low = 0;

	// register for savestates
	save_item(NAME(m_nvram_battery));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_sp));
	save_item(NAME(m_op));

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_w));
	save_item(NAME(m_x));
	save_item(NAME(m_spx));
	save_item(NAME(m_y));
	save_item(NAME(m_spy));
	save_item(NAME(m_st));
	save_item(NAME(m_ca));
	save_item(NAME(m_standby));
	save_item(NAME(m_stop));

	save_item(NAME(m_r));
	save_item(NAME(m_r_mask));
	save_item(NAME(m_d));
	save_item(NAME(m_d_mask));

	save_item(NAME(m_int_line));
	save_item(NAME(m_irq_flags));
	save_item(NAME(m_pmr));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_timer_mode));
	save_item(NAME(m_timer_div));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_load));
	save_item(NAME(m_timer_b_low));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_st).formatstr("%2s").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%04X"); // 1
	state_add(++m_state_count, "SP", m_sp).formatstr("%03X"); // 2
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 3
	state_add(++m_state_count, "B", m_b).formatstr("%01X"); // 4
	state_add(++m_state_count, "W", m_w).formatstr("%01X"); // 5
	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 6
	state_add(++m_state_count, "SPX", m_spx).formatstr("%01X"); // 7
	state_add(++m_state_count, "Y", m_y).formatstr("%01X"); // 8
	state_add(++m_state_count, "SPY", m_spy).formatstr("%01X"); // 9

	state_add(++m_state_count, "ST", m_st).formatstr("%01X").noshow(); // 10
	state_add(++m_state_count, "CA", m_ca).formatstr("%01X").noshow(); // 11

	set_icountptr(m_icount);
}

void hmcs400_cpu_device::device_reset()
{
	m_pc = 0;
	m_sp = 0x3ff;
	m_st = 1;
	m_standby = false;
	m_stop = false;
	m_stop_cb(0);

	// clear peripherals
	m_irq_flags = 0xaaa8; // IM=1, IF=0, IE=0
	m_pmr = 0;

	m_prescaler = 0;
	tm_w(0, 0, 0xf);
	tm_w(1, 0, 0xf);
	m_timer_count[0] = m_timer_count[1] = 0;
	m_timer_load = 0;
	m_timer_b_low = 0;

	// all I/O ports set to input
	reset_io();
}


//-------------------------------------------------
//  disasm
//-------------------------------------------------

void hmcs400_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c",
				m_ca ? 'C':'c',
				m_st ? 'S':'s'
			);
			break;

		default: break;
	}
}

std::unique_ptr<util::disasm_interface> hmcs400_cpu_device::create_disassembler()
{
	return std::make_unique<hmcs400_disassembler>();
}


//-------------------------------------------------
//  internal memory maps
//-------------------------------------------------

void hmcs400_cpu_device::program_map(address_map &map)
{
	map.unmap_value_high();
	map(0, m_rom_size - 1).rom();
}

void hmcs400_cpu_device::data_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x003).rw(FUNC(hmcs400_cpu_device::irq_control_r), FUNC(hmcs400_cpu_device::irq_control_w));
	map(0x004, 0x004).w(FUNC(hmcs400_cpu_device::pmr_w));
	map(0x008, 0x009).w(FUNC(hmcs400_cpu_device::tm_w));
	map(0x00a, 0x00a).rw(FUNC(hmcs400_cpu_device::tcbl_r), FUNC(hmcs400_cpu_device::tlrl_w));
	map(0x00b, 0x00b).rw(FUNC(hmcs400_cpu_device::tcbu_r), FUNC(hmcs400_cpu_device::tlru_w));

	map(0x020, 0x020 + m_ram_size - 1).ram().share(m_ram[0]);
	map(0x3c0, 0x3ff).ram().share(m_ram[1]); // stack
}

device_memory_interface::space_config_vector hmcs400_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  nvram
//-------------------------------------------------

bool hmcs400_cpu_device::nvram_write(util::write_stream &file)
{
	// if it's currently not battery-backed, don't save at all
	if (!m_nvram_battery)
		return true;

	std::error_condition err;
	size_t actual;

	// main RAM and stack area
	for (auto & ram : m_ram)
	{
		std::tie(err, actual) = write(file, &ram[0], ram.bytes());
		if (err)
			return false;
	}

	return true;
}

bool hmcs400_cpu_device::nvram_read(util::read_stream &file)
{
	std::error_condition err;
	size_t actual;

	// main RAM and stack area
	for (auto & ram : m_ram)
	{
		std::tie(err, actual) = read(file, &ram[0], ram.bytes());
		if (err || (ram.bytes() != actual))
			return false;
	}

	return true;
}

void hmcs400_cpu_device::nvram_default()
{
	if (!nvram_backup_enabled())
		return;

	// default nvram from mytag:nvram region if it exists
	memory_region *region = memregion("nvram");
	if (region != nullptr)
	{
		const u32 total = m_ram[0].bytes() + m_ram[1].bytes();
		if (region->bytes() != total)
			fatalerror("%s: Wrong region size (expected 0x%x, found 0x%x)", region->name(), total, region->bytes());

		u32 offset = 0;
		for (auto & ram : m_ram)
		{
			std::copy_n(&region->as_u8(offset), ram.bytes(), &ram[0]);
			offset += ram.bytes();
		}
	}
	else
	{
		for (auto & ram : m_ram)
			std::fill_n(&ram[0], ram.bytes(), m_nvram_defval);
	}
}


//-------------------------------------------------
//  i/o ports
//-------------------------------------------------

void hmcs400_cpu_device::reset_io()
{
	// D4-D15 are high-voltage
	m_d_mask = m_d = 0x000f;
	m_write_d(m_d_mask);

	for (int i = 0; i < 11; i++)
	{
		// R0-R2 and RA are high-voltage
		u8 mask = (i >= 3 && i <= 9) ? 0xf : 0;

		m_r_mask[i] = m_r[i] = mask;
		m_write_r[i](i, mask, 0xf);
	}
}

u8 hmcs400_cpu_device::read_r(u8 index)
{
	// reads from write-only or non-existent ports are invalid
	const bool write_only = (index == 0 || (index >= 6 && index <= 8));
	if (write_only || index > 10)
	{
		logerror("read from %s port R%X @ $%04X\n", write_only ? "output" : "unknown", index, m_prev_pc);
		return 0xf;
	}

	u8 mask = (index == 10) ? 3 : 0xf; // port A is 2-bit
	u8 inp = m_read_r[index](index, mask);

	if (m_read_r[index].isunset())
	{
		inp = m_r_mask[index];
		logerror("read from unmapped port R%X @ $%04X\n", index, m_prev_pc);
	}

	u8 out = m_r[index];

	// R32/R33 are multiplexed with ext interrupts
	if (index == 3)
	{
		u8 pmr_mask = m_pmr & 0xc;
		u8 ext_int = m_int_line[1] << 3 | m_int_line[0] << 2;
		inp = (inp & ~pmr_mask) | (ext_int & pmr_mask);
		out = (out & ~pmr_mask) | (m_r_mask[index] & pmr_mask);
	}

	if (m_r_mask[index])
		return (inp & out) & mask;
	else
		return (inp | out) & mask;
}

void hmcs400_cpu_device::write_r(u8 index, u8 data)
{
	data &= 0xf;

	// ignore writes to read-only or non-existent ports
	if (index > 8)
		return;

	if (m_write_r[index].isunset())
		logerror("write $%X to unmapped port R%d @ $%04X\n", data, index, m_prev_pc);

	m_r[index] = data;
	u8 out = data;

	// R32/R33 are multiplexed with ext interrupts
	if (index == 3)
	{
		u8 pmr_mask = m_pmr & 0xc;
		out = (out & ~pmr_mask) | (m_r_mask[index] & pmr_mask);
	}

	m_write_r[index](index, out, 0xf);
}

int hmcs400_cpu_device::read_d(u8 index)
{
	index &= 0xf;
	u16 mask = 1 << index;
	u16 inp = m_read_d(0, mask);

	if (m_read_d.isunset())
	{
		inp = m_d_mask;
		logerror("read from unmapped port D%d @ $%04X\n", index, m_prev_pc);
	}

	if (m_d_mask & mask)
		return BIT(inp & m_d, index);
	else
		return BIT(inp | m_d, index);
}

void hmcs400_cpu_device::write_d(u8 index, int state)
{
	index &= 0xf;
	u16 mask = 1 << index;

	if (m_write_d.isunset())
		logerror("write %d to unmapped port D%d @ $%04X\n", state, index, m_prev_pc);

	m_d = (m_d & ~mask) | (state ? mask : 0);
	m_write_d(0, m_d, mask);
}


//-------------------------------------------------
//  interrupts
//-------------------------------------------------

bool hmcs400_cpu_device::access_mode(u8 mem_mask, bool bit_mode)
{
	mem_mask &= 0xf;
	bool err = true;

	if (bit_mode)
	{
		if (population_count_32(mem_mask) == 1)
			return true;
		err = mem_mask == 0xf;
	}
	else
	{
		if (mem_mask == 0xf)
			return true;
	}

	if (err)
		logerror("invalid access to I/O register @ $%04X\n", m_prev_pc);

	return false;
}

u8 hmcs400_cpu_device::irq_control_r(offs_t offset, u8 mem_mask)
{
	// mask out unused bits (RSP is write-only)
	const u16 unused = 0xcc02;
	u16 data = m_irq_flags | unused;

	if (!machine().side_effects_disabled())
	{
		// can only read one bit at a time
		if (!access_mode(mem_mask, true))
			return 0xf;

		if (mem_mask << (offset * 4) & unused)
			logerror("read from unused IRQ control bit @ $%04X\n", m_prev_pc);
	}

	return data >> (offset * 4) & 0xf;
}

void hmcs400_cpu_device::irq_control_w(offs_t offset, u8 data, u8 mem_mask)
{
	// can only write one bit at a time
	if (!access_mode(mem_mask, true))
		return;

	data &= mem_mask;
	u16 mask = mem_mask << (offset * 4);

	// ignore writes to unused bits
	if (mask & 0xcc00)
		return;

	// ignore writing 1 to flags that can only be cleared
	if (mask & 0x5556 && data)
		return;

	// bit 1: RSP (reset SP)
	if (mask & 0x0002)
		m_sp = 0x3ff;

	m_irq_flags = (m_irq_flags & ~mask) | (data ? mask : 0);
}

void hmcs400_cpu_device::pmr_w(offs_t offset, u8 data, u8 mem_mask)
{
	if (!access_mode(mem_mask))
		return;

	u8 prev = m_pmr;
	m_pmr = data;

	// trigger irq on rising edge if ext int line was low
	for (int i = 0; i < 2; i++)
		if (BIT(data & ~prev, i + 2))
			ext_int_edge(i);

	// refresh R32/R33
	if ((data ^ prev) & 0xc)
		write_r(3, m_r[3]);
}

void hmcs400_cpu_device::take_interrupt(int irq)
{
	cycle();
	cycle();
	push_stack();
	m_irq_flags &= ~1;

	standard_irq_callback(irq, m_pc);

	u8 vector = irq * 2 + 2;
	m_prev_pc = m_pc = vector;
}

void hmcs400_cpu_device::check_interrupts()
{
	// irq priority is in the same order as the irq control flags
	u16 irq = m_irq_flags >> 2;

	for (int i = 0; i < 7; i++)
	{
		// pending irq when IF=1 and IM=0
		if ((irq & 3) == 1)
		{
			if (m_irq_flags & 1)
				take_interrupt(i);

			m_standby = false;
			return;
		}

		irq >>= 2;
	}
}

void hmcs400_cpu_device::ext_int_edge(int line)
{
	// ext interrupts are masked with PMR2/3
	if (!m_int_line[line] && BIT(m_pmr, line + 2))
	{
		m_irq_flags |= 1 << (line * 2 + 2);

		// timer B event counter on INT1
		if (line == 1 && !m_timer_div[1])
			clock_timer(1);
	}
}

void hmcs400_cpu_device::execute_set_input(int line, int state)
{
	state = state ? 1 : 0;

	if (line != 0 && line != 1)
		return;

	// active-low, irq on falling edge
	state ^= 1;
	bool irq = (m_int_line[line] && !state);
	m_int_line[line] = state;

	if (irq && !m_stop)
		ext_int_edge(line);
}


//-------------------------------------------------
//  timers
//-------------------------------------------------

void hmcs400_cpu_device::tm_w(offs_t offset, u8 data, u8 mem_mask)
{
	if (!access_mode(mem_mask))
		return;

	// TMA/TMB prescaler divide ratio masks
	static const int div[2][8] =
	{
		{ 0x400, 0x200, 0x100, 0x40, 0x10, 4, 2, 1 },
		{ 0x400, 0x100, 0x40, 0x10, 4, 2, 1, 0 }
	};

	m_timer_mode[offset] = data & 0xf;
	m_timer_div[offset] = div[offset][data & 7];
}

void hmcs400_cpu_device::tlrl_w(offs_t offset, u8 data, u8 mem_mask)
{
	if (!access_mode(mem_mask))
		return;

	// TLRL: timer load register lower
	m_timer_load = (m_timer_load & 0xf0) | (data & 0xf);
}

void hmcs400_cpu_device::tlru_w(offs_t offset, u8 data, u8 mem_mask)
{
	if (!access_mode(mem_mask))
		return;

	// TLRU: timer load register upper
	m_timer_load = (m_timer_load & 0x0f) | data << 4;
	m_timer_count[1] = m_timer_load;
}

u8 hmcs400_cpu_device::tcbl_r(offs_t offset, u8 mem_mask)
{
	if (!access_mode(mem_mask))
		return 0xf;

	// TCBL: timer counter B lower
	return m_timer_b_low;
}

u8 hmcs400_cpu_device::tcbu_r(offs_t offset, u8 mem_mask)
{
	if (!access_mode(mem_mask))
		return 0xf;

	// TCBU: timer counter B upper (latches TCBL)
	if (!machine().side_effects_disabled())
		m_timer_b_low = m_timer_count[1] & 0xf;

	return m_timer_count[1] >> 4;
}

void hmcs400_cpu_device::clock_timer(int timer)
{
	if (++m_timer_count[timer] == 0)
	{
		// set timer overflow irq flag
		m_irq_flags |= 1 << (timer * 2 + 6);

		// timer B reload function
		if (timer == 1 && m_timer_mode[1] & 8)
			m_timer_count[1] = m_timer_load;
	}
}

void hmcs400_cpu_device::clock_prescaler()
{
	u16 prev = m_prescaler;
	m_prescaler = (m_prescaler + 1) & 0x7ff;

	// increment timers based on prescaler divide ratio
	for (int i = 0; i < 2; i++)
		if (m_prescaler & ~prev & m_timer_div[i])
			clock_timer(i);
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void hmcs400_cpu_device::cycle()
{
	m_icount--;
	clock_prescaler();
}

u16 hmcs400_cpu_device::fetch()
{
	u16 data = m_program->read_word(m_pc);
	m_pc = (m_pc + 1) & 0x3fff;
	cycle();

	return data & 0x3ff;
}

void hmcs400_cpu_device::execute_run()
{
	// in stop mode, the internal clock is not running
	if (m_stop)
	{
		m_icount = 0;
		return;
	}

	while (m_icount > 0)
	{
		m_prev_pc = m_pc;
		check_interrupts();

		// in standby mode, opcode execution is halted
		if (m_standby)
		{
			cycle();
			continue;
		}

		// fetch next opcode
		debugger_instruction_hook(m_pc);
		m_op = fetch();
		m_i = m_op & 0xf;

		// 2-byte opcodes / RAM address
		if ((m_op >= 0x100 && m_op < 0x140) || (m_op >= 0x150 && m_op < 0x1b0))
			m_param = fetch();
		else
			m_param = (m_w << 8 | m_x << 4 | m_y) & 0x3ff;

		// handle opcode
		switch (m_op & 0x3f0)
		{
			case 0x1c0: case 0x1d0: case 0x1e0: case 0x1f0: op_cal(); break;

			case 0x020: case 0x120: op_inem(); break;
			case 0x030: case 0x130: op_ilem(); break;
			case 0x070: op_ynei(); break;
			case 0x0b0: op_tbr(); break;
			case 0x150: op_jmpl(); break;
			case 0x160: op_call(); break;
			case 0x170: op_brl(); break;
			case 0x1a0: op_lmi(); break;
			case 0x1b0: op_p(); break;

			case 0x200: op_lbi(); break;
			case 0x210: op_lyi(); break;
			case 0x220: op_lxi(); break;
			case 0x230: op_lai(); break;
			case 0x240: op_lbr(); break;
			case 0x250: op_lar(); break;
			case 0x260: op_redd(); break;
			case 0x270: op_lamr(); break;
			case 0x280: op_ai(); break;
			case 0x290: op_lmiiy(); break;
			case 0x2a0: op_tdd(); break;
			case 0x2b0: op_alei(); break;
			case 0x2c0: op_lrb(); break;
			case 0x2d0: op_lra(); break;
			case 0x2e0: op_sedd(); break;
			case 0x2f0: op_xmra(); break;

			default:
				if ((m_op & 0x300) == 0x300)
				{
					op_br(); break;
				}

				switch (m_op & 0x3fc)
				{
			case 0x084: case 0x184: op_sem(); break;
			case 0x088: case 0x188: op_rem(); break;
			case 0x08c: case 0x18c: op_tm(); break;

			case 0x000: op_xsp(); break;
			case 0x040: op_lbm(); break;
			case 0x080: op_xma(); break;
			case 0x090: op_lam(); break;
			case 0x094: op_lma(); break;
			case 0x0c0: op_xmb(); break;
			case 0x0f0: op_lwi(); break;

			default:
				switch (m_op)
				{
			case 0x004: case 0x104: op_anem(); break;
			case 0x008: case 0x108: op_am(); break;
			case 0x00c: case 0x10c: op_orm(); break;
			case 0x014: case 0x114: op_alem(); break;
			case 0x018: case 0x118: op_amc(); break;
			case 0x01c: case 0x11c: op_eorm(); break;
			case 0x098: case 0x198: op_smc(); break;
			case 0x09c: case 0x19c: op_anm(); break;

			case 0x010: op_rtn(); break;
			case 0x011: op_rtni(); break;
			case 0x044: op_bnem(); break;
			case 0x048: op_lab(); break;
			case 0x04c: op_ib(); break;
			case 0x050: case 0x051: op_lmaiy(); break;
			case 0x054: op_ayy(); break;
			case 0x058: op_laspy(); break;
			case 0x05c: op_iy(); break;
			case 0x060: op_nega(); break;
			case 0x064: op_red(); break;
			case 0x068: op_laspx(); break;
			case 0x06f: op_tc(); break;

			case 0x0a0: op_rotr(); break;
			case 0x0a1: op_rotl(); break;
			case 0x0a6: op_daa(); break;
			case 0x0aa: op_das(); break;
			case 0x0af: op_lay(); break;
			case 0x0c4: op_blem(); break;
			case 0x0c8: op_lba(); break;
			case 0x0cf: op_db(); break;
			case 0x0d0: case 0x0d1: op_lmady(); break;
			case 0x0d4: op_syy(); break;
			case 0x0d8: op_lya(); break;
			case 0x0df: op_dy(); break;
			case 0x0e0: op_td(); break;
			case 0x0e4: op_sed(); break;
			case 0x0e8: op_lxa(); break;
			case 0x0ec: op_rec(); break;
			case 0x0ef: op_sec(); break;

			case 0x100: op_law(); break;
			case 0x110: op_lwa(); break;
			case 0x140: op_comb(); break;
			case 0x144: op_or(); break;
			case 0x148: op_sts(); break;
			case 0x14c: op_sby(); break;
			case 0x14d: op_stop(); break;
			case 0x180: op_xma(); break;
			case 0x190: op_lam(); break;
			case 0x194: op_lma(); break;

			default: op_illegal(); break;
				}
				break; // 0x3ff

				}
				break; // 0x3fc

		} // 0x3f0
	}
}
