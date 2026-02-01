// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************
 Hyperstone cpu emulator
 written by Pierpaolo Prazzoli

 Hyperstone models:

 Model       Core    Bus     IRAM         Maximum core frequency         Process  Package
 E1-16T      E1      16-bit   4 KiB DRAM  66 MHz @ 5.0 V                          100-pin TQFP
 E1-32T      E1      32-bit   4 KiB DRAM  66 MHz @ 5.0 V                          144-pin TQFP
 E1-32N      E1      32-bit   4 KiB DRAM  66 MHz @ 5.0 V                          160-pin PQFP
 E1-16XT     E1-X    16-bit   8 KiB DRAM  80 MHz @ 5.0 V, 53 MHz @ 3.3V  0.5  µm  100-pin TQFP
 E1-32XT     E1-X    32-bit   8 KiB DRAM  80 MHz @ 5.0 V, 53 MHz @ 3.3V  0.5  µm  144-pin TQFP
 E1-32XN     E1-X    32-bit   8 KiB DRAM  80 MHz @ 5.0 V, 53 MHz @ 3.3V  0.5  µm  160-pin PQFP
 E1-16XS     E1-XS   16-bit  16 KiB SRAM  115 MHz                        0.25 µm  100-pin LQFP
 E1-16XSB    E1-XS   16-bit  16 KiB SRAM  115 MHz                        0.25 µm  100-pin TFBGA
 E1-32XS     E1-XS   32-bit  16 KiB SRAM  115 MHz                        0.25 µm  144-pin LQFP
 E1-16XSR    E1-XSR  16-bit  16 KiB SRAM  128 MHz                        0.25 µm  100-pin LQFP
 E1-32XSR    E1-XSR  32-bit  16 KiB SRAM  128 MHz                        0.25 µm  144-pin LQFP

 Hynix models:

 Model       Core    Bus     IRAM         Maximum core frequency          Process  Package
 GMS30C2116  E1      16-bit   4 KiB DRAM  66 MHz @ 5.0 V, 40 MHz @ 3.3 V  0.6  µm  100-pin TQFP
 GMS30C2132  E1      32-bit   4 KiB DRAM  66 MHz @ 5.0 V, 40 MHz @ 3.3 V  0.6  µm  144-pin TQFP, 160-pin MQFP
 GMS30C2216  E1-X    16-bit   8 KiB DRAM  108 MHz                         0.35 µm  100-pin TQFP
 GMS30C2232  E1-X    32-bit   8 KiB DRAM  108 MHz                         0.35 µm  144-pin TQFP, 160-pin MQFP

 E1-X changes:
 * Adds PLL with up to 4* multiplication
 * Adds CLKOUT signal configuration
 * Increases IRAM to 8 KiB
 * Changes DRAM refresh interval configuration to prescaler units
 * Adds MEM0 EDO DRAM support
 * Adds MEM0/MEM1/MEM2 byte write strobe/byte enable selection
 * Adds MEM2 and I/O wait support
 * Changes memory timing options
 * Changes to bus hold break always enabled for DRAM
 * Moves power down from MCR to an I/O address

 E-1XS changes:
 * Changes to 3.3 V I/O voltage and 2.5 V core voltage
 * Increases PLL options to up to 8* multiplication
 * Increases IRAM to 16 KiB
 * Changes IRAM to SRAM
 * Adds MEM0 SDRAM support
 * Removes bus output voltage and input threshold selection

 E-1XS changes:
 * Changes SDRAM timing options
 * Adds more DRAM clock configuration options
 * Removes MEM0/MEM1/MEM2 byte enable support

 The Hynix models are generally similar to the Hyperstone models
 based on the same core with minor differences:
 * Hynix models are fabricated with smaller feature sizes
 * The GMS30C2216 and GMS30C2232 support higher core frequencies
 * The GMS30C2216 and GMS30C2232 only support a 3.3 V power supply
 * The GMS30C2216 and GMS30C2232 lack bus output voltage and input
   threshold selection (inputs are 5 V tolerant)
 * Hynix offered a 160-pin MQFP package rather than LQFP

 Backwards compatibility is fairly good across models.
 Incompatibilities include:
 * Power supply and bus voltages changed
 * Additional memory types and features are supported on later models
 * Only the E1-X and E1-XS support memory byte enable signals
 * The E1-XSR changes the available DRAM timing options
 * PLL control bits added to the TPR register
 * The BCR, MCR and SDCR register formats change in incompatible ways

 TODO:
 - All instructions should clear the H flag (not just MOV/MOVI)
 - Fix behaviour of branches in delay slots for recompiler
 - Many wrong cycle counts
 - Prevent reading write-only BCR, TPR, FCR and MCR
 - IRAM selection should happen before EA calculation
 - No emulation of memory access latency and pipleline
 - Should a zero bit shift clear C or leave it unchanged?
 - What actually happens on trying to load to PC, SR, G14 or G15?
 - Verify register wrapping with sregf/dregf on hardware
 - Tracing doesn't work properly for the recompiler
   DRC does not generate trace exceptions on branch or return
 - INT/IO polarity
 - IO3 timing and timer interrupt modes
 - Watchdog
 - Sleep mode

*********************************************************************/

#include "emu.h"
#include "e132xs.h"
#include "e132xsfe.h"

#include "32xsdefs.h"

#include "emuopts.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


// size of the execution code cache
constexpr size_t CACHE_SIZE = 32 * 1024 * 1024;

//**************************************************************************
//  INTERNAL ADDRESS MAP
//**************************************************************************

// 4KiB IRAM (On-Chip Memory)

void hyperstone_device::iram_4k_map(address_map &map)
{
	map(0xc0000000, 0xc0000fff).ram().mirror(0x1ffff000);
}


// 8KiB IRAM (On-Chip Memory)

void hyperstone_x_device::iram_8k_map(address_map &map)
{
	map(0xc0000000, 0xc0001fff).ram().mirror(0x1fffe000);
}


// 16KiB IRAM (On-Chip Memory)

void hyperstone_xs_device::iram_16k_map(address_map &map)
{
	map(0xc0000000, 0xc0003fff).ram().mirror(0x1fffc000);
}


// Internal I/O

void hyperstone_x_device::internal_io_map(address_map &map)
{
	map(0x1c00, 0x1dff).w(FUNC(hyperstone_x_device::power_down_w));
	map(0x1e00, 0x1fff).w(FUNC(hyperstone_x_device::sleep_w));
}


//-------------------------------------------------
//  hyperstone_device - constructor
//-------------------------------------------------

hyperstone_device::hyperstone_device(
		const machine_config &mconfig,
		const device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint32_t prg_data_width,
		uint32_t io_data_width,
		uint32_t io_addr_bits,
		address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, prg_data_width, 32, 0, internal_map)
	, m_io_config("io", ENDIANNESS_BIG, io_data_width, io_addr_bits, (io_data_width == 16) ? -1 : -2)
	, m_cache(CACHE_SIZE + sizeof(internal_hyperstone_state))
	, m_drcuml(nullptr)
	, m_drcfe(nullptr)
	, m_drcoptions(0)
	, m_single_instruction_mode(false)
	, m_cache_dirty(0)
	, m_entry(nullptr)
	, m_nocode(nullptr)
	, m_interrupt_checks(nullptr)
	, m_out_of_cycles(nullptr)
	, m_eat_all_cycles(nullptr)
	, m_mem_read8(nullptr)
	, m_mem_write8(nullptr)
	, m_mem_read16(nullptr)
	, m_mem_write16(nullptr)
	, m_mem_read32(nullptr)
	, m_mem_write32(nullptr)
	, m_io_read32(nullptr)
	, m_io_write32(nullptr)
	, m_exception(nullptr)
	, m_enable_drc(false)
{
	std::fill(std::begin(m_delay_taken), std::end(m_delay_taken), nullptr);
}

hyperstone_device::~hyperstone_device()
{
}


//-------------------------------------------------
//  hyperstone_x_device - constructor
//-------------------------------------------------

hyperstone_x_device::hyperstone_x_device(
		const machine_config &mconfig,
		const device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint32_t prg_data_width,
		uint32_t io_data_width,
		uint32_t io_addr_bits,
		address_map_constructor internal_map)
	: hyperstone_device(mconfig, type, tag, owner, clock, prg_data_width, io_data_width, io_addr_bits, internal_map)
	, m_internal_config("internal", ENDIANNESS_BIG, 32, 10 + 3, -2, address_map_constructor(FUNC(hyperstone_x_device::internal_io_map), this))
{
}


//-------------------------------------------------
//  e116_device - constructor
//-------------------------------------------------

e116_device::e116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(
			mconfig, E116, tag, owner, clock,
			16, 16, 6 + 3, address_map_constructor(FUNC(e116_device::iram_4k_map), this))
{
}


//-------------------------------------------------
//  e116x_device - constructor
//-------------------------------------------------

e116x_device::e116x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_x_device(
			mconfig, E116X, tag, owner, clock,
			16, 16, 6 + 3, address_map_constructor(FUNC(e116x_device::iram_8k_map), this))
{
}


//-------------------------------------------------
//  e116xs_device - constructor
//-------------------------------------------------

e116xs_device::e116xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_xs_device(
			mconfig, E116XS, tag, owner, clock,
			16, 16, 6 + 3, address_map_constructor(FUNC(e116xs_device::iram_16k_map), this))
{
}


//-------------------------------------------------
//  e116xsr_device - constructor
//-------------------------------------------------

e116xsr_device::e116xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_xsr_device(
			mconfig, E116XSR, tag, owner, clock,
			16, 16, 6 + 3, address_map_constructor(FUNC(e116xsr_device::iram_16k_map), this))
{
}


//-------------------------------------------------
//  e132_device - constructor
//-------------------------------------------------

e132_device::e132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(
			mconfig, E132, tag, owner, clock,
			32, 32, 10 + 3, address_map_constructor(FUNC(e132_device::iram_4k_map), this))
{
}


//-------------------------------------------------
//  e132x_device - constructor
//-------------------------------------------------

e132x_device::e132x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_x_device(
			mconfig, E132X, tag, owner, clock,
			32, 32, 10 + 3, address_map_constructor(FUNC(e132x_device::iram_8k_map), this))
{
}


//-------------------------------------------------
//  e132xs_device - constructor
//-------------------------------------------------

e132xs_device::e132xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_xs_device(
			mconfig, E132XS, tag, owner, clock,
			32, 32, 10 + 3, address_map_constructor(FUNC(e132xs_device::iram_16k_map), this))
{
}


//-------------------------------------------------
//  e132xsr_device - constructor
//-------------------------------------------------

e132xsr_device::e132xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_xsr_device(
			mconfig, E132XSR, tag, owner, clock,
			32, 32, 10 + 3, address_map_constructor(FUNC(e132xsr_device::iram_16k_map), this))
{
}


//-------------------------------------------------
//  gms30c2116_device - constructor
//-------------------------------------------------

gms30c2116_device::gms30c2116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(
			mconfig, GMS30C2116, tag, owner, clock,
			16, 16, 6 + 3, address_map_constructor(FUNC(gms30c2116_device::iram_4k_map), this))
{
}


//-------------------------------------------------
//  gms30c2132_device - constructor
//-------------------------------------------------

gms30c2132_device::gms30c2132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(
			mconfig, GMS30C2132, tag, owner, clock,
			32, 32, 10 + 3, address_map_constructor(FUNC(gms30c2132_device::iram_4k_map), this))
{
}


//-------------------------------------------------
//  gms30c2216_device - constructor
//-------------------------------------------------

gms30c2216_device::gms30c2216_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_x_device(
			mconfig, GMS30C2216, tag, owner, clock,
			16, 16, 6 + 3, address_map_constructor(FUNC(gms30c2216_device::iram_8k_map), this))
{
}


//-------------------------------------------------
//  gms30c2232_device - constructor
//-------------------------------------------------

gms30c2232_device::gms30c2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_x_device(
			mconfig, GMS30C2232, tag, owner, clock,
			32, 32, 10 + 3, address_map_constructor(FUNC(gms30c2232_device::iram_8k_map), this))
{
}

/* Return the entry point for a determinated trap */
uint32_t hyperstone_device::get_trap_addr(uint8_t trapno)
{
	uint32_t addr;
	if (m_core->trap_entry == 0xffffff00) /* @ MEM3 */
	{
		addr = trapno * 4;
	}
	else
	{
		addr = (63 - trapno) * 4;
	}
	addr |= m_core->trap_entry;

	return addr;
}

/* Return the entry point for a determinated emulated code (the one for "extend" opcode is reserved) */
uint32_t hyperstone_device::get_emu_code_addr(uint8_t num) /* num is OP */
{
	uint32_t addr;
	if (m_core->trap_entry == 0xffffff00) /* @ MEM3 */
	{
		addr = (m_core->trap_entry - 0x100) | ((num & 0xf) << 4);
	}
	else
	{
		addr = m_core->trap_entry | (0x10c | ((0xcf - num) << 4));
	}
	return addr;
}

/*static*/ const uint32_t hyperstone_device::s_trap_entries[8] = {
	0x00000000, // MEM0
	0x40000000, // MEM1
	0x80000000, // MEM2
	0xc0000000, // IRAM
	0,
	0,
	0,
	0xffffff00, // MEM3
};

#if E132XS_LOG_INTERPRETER_REGS
void hyperstone_device::dump_registers()
{
	uint8_t packed[4];
	packed[0] = (uint8_t)m_core->intblock;
	packed[1] = (uint8_t)(m_core->icount >> 16);
	packed[2] = (uint8_t)(m_core->icount >>  8);
	packed[3] = (uint8_t)(m_core->icount >>  0);
	fwrite(packed, 1, 4, m_trace_log);
	fwrite(m_core->global_regs, 4, 32, m_trace_log);
	fwrite(m_core->local_regs, 4, 64, m_trace_log);
}
#endif

void hyperstone_device::compute_tr()
{
	uint64_t cycles_since_base = total_cycles() - m_core->tr_base_cycles;
	uint64_t clocks_since_base = cycles_since_base >> m_core->clck_scale;
	m_core->tr_result = m_core->tr_base_value + (clocks_since_base / m_core->tr_clocks_per_tick);
}

void hyperstone_device::update_timer_prescale()
{
	TPR &= ~0x80000000;
	m_core->clck_scale = (TPR >> 26) & m_core->clock_scale_mask;
	m_core->clock_cycles_1 = 1 << m_core->clck_scale;
	m_core->clock_cycles_2 = 2 << m_core->clck_scale;
	m_core->clock_cycles_3 = 3 << m_core->clck_scale;
	m_core->clock_cycles_4 = 4 << m_core->clck_scale;
	m_core->clock_cycles_6 = 6 << m_core->clck_scale;
	m_core->clock_cycles_36 = 36 << m_core->clck_scale;
	m_core->tr_clocks_per_tick = ((TPR >> 16) & 0xff) + 2;
	m_core->tr_base_value = m_core->tr_result;
	m_core->tr_base_cycles = total_cycles();
}

void hyperstone_device::adjust_timer_interrupt()
{
	uint64_t cycles_since_base = total_cycles() - m_core->tr_base_cycles;
	uint64_t clocks_since_base = cycles_since_base >> m_core->clck_scale;
	uint64_t cycles_until_next_clock = cycles_since_base - (clocks_since_base << m_core->clck_scale);

	if (cycles_until_next_clock == 0)
		cycles_until_next_clock = (uint64_t)(1 << m_core->clck_scale);

	if (TPR & 0x80000000)
	{
		// special case: if we have a change pending, set a timer to fire then
		uint64_t clocks_until_int = m_core->tr_clocks_per_tick - (clocks_since_base % m_core->tr_clocks_per_tick);
		uint64_t cycles_until_int = (clocks_until_int << m_core->clck_scale) + cycles_until_next_clock;
		m_timer->adjust(cycles_to_attotime(cycles_until_int + 1), 1);
	}
	else if (!(FCR & 0x00800000))
	{
		// else if the timer interrupt is enabled, configure it to fire at the appropriate time
		uint32_t curtr = m_core->tr_base_value + (clocks_since_base / m_core->tr_clocks_per_tick);
		uint32_t delta = TCR - curtr;
		if (delta > 0x80000000)
		{
			if (!m_core->timer_int_pending)
				m_timer->adjust(attotime::zero);
		}
		else
		{
			uint64_t clocks_until_int = mulu_32x32(delta, m_core->tr_clocks_per_tick);
			uint64_t cycles_until_int = (clocks_until_int << m_core->clck_scale) + cycles_until_next_clock;
			m_timer->adjust(cycles_to_attotime(cycles_until_int));
		}
	}
	else
	{
		// otherwise, disable the timer
		m_timer->adjust(attotime::never);
	}
}

void hyperstone_device::update_bus_control()
{
	const uint32_t val = m_core->global_regs[BCR_REGISTER];

	const unsigned mem2hold     = BIT(val,  0, 2);
	const unsigned mem3hold     = bitswap<3>(val, 23, 3, 2);
	// 4..6 page size code
	// 7 reserved, must be 1
	const unsigned rastocas     = BIT(val,  8, 2) + 1; // for MEM0 DRAM
	const unsigned rasprecharge = BIT(val, 10, 2) + 1; // for MEM0 DRAM
	const unsigned mem0hold     = BIT(val, 10, 2);     // for MEM0 non-DRAM
	// 12..13 refresh select
	const unsigned mem2setup    = BIT(val, 14);
	const unsigned mem1hold     = BIT(val, 15);
	const unsigned mem0access   = BIT(val, 16, 2) + 1;
	const unsigned mem1access   = BIT(val, 18, 2) + 1;
	const unsigned mem2access   = BIT(val, 20, 3) + 1;
	// 23 MEM3 hold (2)
	const unsigned mem3access   = BIT(val, 24, 4) + 1;

	LOG("%s: Set BCR = 0x%08x\n", machine().describe_context(), val);
	if (BIT(m_core->global_regs[MCR_REGISTER], 21))
	{
		LOG("MEM0 parity %s, access time %d cycle(s), hold time %d cycle(s)\n",
				BIT(val, 28) ? "disabled" : "enabled", mem0access, mem0hold);
	}
	else
	{
		char const *const refresh[4] = {
				"every 512 cycles",
				"every 256 cycles",
				"every 128 cycles",
				"disabled" };
		char const *const page[8] = { "64K", "32K", "16K", "8K", "4K", "2K", "1K", "512" };
		LOG("MEM0 parity %s, RAS precharge time %d cycle(s), RAS to CAS delay time %d cycle(s), CAS access time %d cycle(s), %s byte rows, refresh %s\n",
				BIT(val, 28) ? "disabled" : "enabled",
				rasprecharge, rastocas, mem0access, page[BIT(val, 4, 3)], refresh[BIT(val, 12, 2)]);
	}
	LOG("MEM1 parity %s, access time %d cycle(s), hold time %d cycle(s)\n",
			BIT(val, 29) ? "disabled" : "enabled", mem1access, mem1hold);
	LOG("MEM2 parity %s, access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
			BIT(val, 30) ? "disabled" : "enabled", mem2access, mem2hold, mem2setup);
	LOG("MEM3 parity %s, access time %d cycle(s), hold time %d cycle(s)\n",
			BIT(val, 31) ? "disabled" : "enabled",
			mem3access, mem3hold);
}

void hyperstone_x_device::update_bus_control()
{
	const uint32_t val = m_core->global_regs[BCR_REGISTER];

	const unsigned mem2hold     = BIT(val,  0, 3);
	const unsigned mem2setup    = BIT(val,  3);
	// 4..6 page size code
	const unsigned mem3setup    = BIT(val,  7);
	const unsigned mem3hold     = BIT(val,  8, 3);
	// 11..13 refresh select
	const unsigned rastocas     = BIT(val, 14, 2) + 1; // for MEM0 DRAM
	const unsigned casaccess    = BIT(val, 16, 2) + 1 + (BIT(m_core->global_regs[MCR_REGISTER], 8) * 2);
	const unsigned rasprecharge = BIT(val, 18, 2) + 1 + (BIT(m_core->global_regs[MCR_REGISTER], 8) * 2);
	const unsigned mem0hold     = BIT(val, 11, 3);     // for MEM0 non-DRAM
	const unsigned mem0setup    = BIT(val, 14, 2);     // for MEM0 non-DRAM
	const unsigned mem0access   = BIT(val, 16, 4) + 1; // for MEM0 non-DRAM
	const unsigned mem1access   = BIT(val, 20, 3) + 1;
	const unsigned mem1hold     = BIT(val, 23) + BIT(val, 22);
	const unsigned mem2access   = BIT(val, 24, 4) + 1;
	const unsigned mem3access   = BIT(val, 28, 4) + 1;

	LOG("%s: Set BCR = 0x%08x\n", machine().describe_context(), val);
	if (BIT(m_core->global_regs[MCR_REGISTER], 21))
	{
		LOG("MEM0 access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
				mem0access, mem0hold, mem0setup);
	}
	else
	{
		char const *const refresh[8] = {
				"every 256 prescaler time units",
				"every 128 prescaler time units",
				"every 64 prescaler time units",
				"every 32 prescaler time units",
				"every 16 prescaler time units",
				"every 8 prescaler time units",
				"every 4 prescaler time units",
				"disabled" };
		char const *const page[8] = { "64K", "32K", "16K", "8K", "4K", "2K", "1K", "512" };
		LOG("MEM0 RAS precharge time %d cycle(s), RAS to CAS delay time %d cycle(s), CAS access time %d cycle(s), %s byte rows, refresh %s\n",
				rasprecharge, rastocas, casaccess, page[BIT(val, 4, 3)], refresh[BIT(val, 11, 3)]);
	}
	LOG("MEM1 access time %d cycle(s), hold time %d cycle(s)\n",
			mem1access, mem1hold);
	LOG("MEM2 access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
			mem2access, mem2hold, mem2setup);
	LOG("MEM3 access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
			mem3access, mem3hold, mem3setup);
}

void hyperstone_xsr_device::update_bus_control()
{
	const uint32_t val = m_core->global_regs[BCR_REGISTER];

	const unsigned mem2hold     = BIT(val,  0, 3);
	const unsigned mem2setup    = BIT(val,  3);
	// 4..6 page size code
	const unsigned mem3setup    = BIT(val,  7);
	const unsigned mem3hold     = BIT(val,  8, 3);
	const unsigned mem0hold     = BIT(val, 11, 3);     // for MEM0 non-DRAM
	const unsigned mem0setup    = BIT(val, 14, 2);     // for MEM0 non-DRAM
	const unsigned mem0access   = BIT(val, 16, 4) + 1; // for MEM0 non-DRAM
	const unsigned mem1access   = BIT(val, 20, 3) + 1;
	const unsigned mem1hold     = BIT(val, 23) + BIT(val, 22);
	const unsigned mem2access   = BIT(val, 24, 4) + 1;
	const unsigned mem3access   = BIT(val, 28, 4) + 1;

	LOG("%s: Set BCR = 0x%08x\n", machine().describe_context(), val);
	if (BIT(m_core->global_regs[MCR_REGISTER], 21))
	{
		LOG("MEM0 access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
				mem0access, mem0hold, mem0setup);
	}
	else
	{
		char const *const refresh[8] = {
				"every 256 prescaler time units",
				"every 128 prescaler time units",
				"every 64 prescaler time units",
				"every 32 prescaler time units",
				"every 16 prescaler time units",
				"every 8 prescaler time units",
				"every 4 prescaler time units",
				"disabled" };
		char const *const page[8] = { "64K", "32K", "16K", "8K", "4K", "2K", "1K", "512" };
		unsigned rastocas, casaccess, rasprecharge;
		if (BIT(m_core->global_regs[MCR_REGISTER], 22))
		{
			rastocas     = BIT(val, 14, 2) + 1;
			casaccess    = BIT(val, 16, 2) + 1 + (BIT(m_core->global_regs[MCR_REGISTER], 8) * 2);
			rasprecharge = BIT(val, 18, 2) + 1 + (BIT(m_core->global_regs[MCR_REGISTER], 8) * 2);
		}
		else
		{
			rastocas     = (BIT(val, 14, 2) + 1) << BIT(m_core->global_regs[MCR_REGISTER], 8);
			casaccess    = (BIT(val, 16, 2) + 1) << BIT(m_core->global_regs[MCR_REGISTER], 8);
			rasprecharge = (BIT(val, 18, 2) + 1) << BIT(m_core->global_regs[MCR_REGISTER], 8);
		}
		LOG("MEM0 RAS precharge time %d cycle(s), RAS to CAS delay time %d cycle(s), CAS access time %d cycle(s), %s byte rows, refresh %s\n",
				rasprecharge, rastocas, casaccess, page[BIT(val, 4, 3)], refresh[BIT(val, 11, 3)]);
	}
	LOG("MEM1 access time %d cycle(s), hold time %d cycle(s)\n",
			mem1access, mem1hold);
	LOG("MEM2 access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
			mem2access, mem2hold, mem2setup);
	LOG("MEM3 access time %d cycle(s), hold time %d cycle(s), setup time %d cycle(s)\n",
			mem3access, mem3hold, mem3setup);
}

void hyperstone_device::update_memory_control()
{
	const uint32_t val = m_core->global_regs[MCR_REGISTER];

	static char const *const entrymap[8] = { "MEM0", "MEM1", "MEM2", "IRAM", "reserved", "reserved", "reserved", "MEM3" };
	LOG("%s: Set MCR = 0x%08x, entry map in %s, %s output voltage, input threshold for VDD=%sV\n",
			machine().describe_context(),
			val,
			entrymap[BIT(val, 12, 3)],
			BIT(val, 25) ? "rail-to-rail" : "reduced",
			BIT(val, 24) ? "5.0" : "3.3");

	static char const *const size[4] = { "32 bit", "reserved", "16 bit", "8 bit" };
	char const *const refresh[8] = {
			"every 128 cycles",
			"every 64 cycles",
			"every 32 cycles",
			"every 16 cycles",
			"every 8 cycles",
			"every 4 cycles",
			"every 2 cycles",
			"disabled" };
	LOG("IRAM %s mode, refresh %s\n",
			BIT(val, 20) ? "normal" : "test",         // IRAM refresh test
			refresh[BIT(val, 16, 2)]);                // IRAM refresh rate
	LOG("MEM0 %s %sDRAM, bus hold break %s\n",
			size[BIT(val, 0, 2)],                     // MEM0 bus size
			BIT(val, 21) ? "non-"     : "fast page ", // MEM0 memory type
			BIT(val,  8) ? "disabled" : "enabled");   // MEM0 bus hold break
	LOG("MEM1 %s, bus hold break %s\n",
			size[BIT(val, 2, 2)],                     // MEM1 bus size
			BIT(val,  9) ? "disabled" : "enabled");   // MEM1 bus hold break
	LOG("MEM2 %s, bus hold break %s\n",
			size[BIT(val, 4, 2)],                     // MEM2 bus size
			BIT(val, 10) ? "disabled" : "enabled");   // MEM2 bus hold break
	LOG("MEM3 %s, bus hold break %s\n",
			size[BIT(val, 6, 2)],                     // MEM3 bus size
			BIT(val, 11) ? "disabled" : "enabled");   // MEM3 bus hold break

	// bits 14..12 EntryTableMap
	const int which = (val & 0x7000) >> 12;
	assert(which < 4 || which == 7);
	m_core->trap_entry = s_trap_entries[which];

	const uint8_t power_down_req = BIT(val, 22);
	if (!power_down_req && m_power_down_req)
	{
		LOG("entering power down\n");
		m_core->powerdown = 1;
	}
	m_power_down_req = power_down_req;
}

void hyperstone_x_device::update_memory_control()
{
	const uint32_t val = m_core->global_regs[MCR_REGISTER];

	// GMS30C2216 and GMS30C2232 drop bus output voltage/input
	// threshold selection as they only support 3.3V power supply
	// and have 5V tolerant inputs.

	static char const *const entrymap[8] = { "MEM0", "MEM1", "MEM2", "IRAM", "reserved", "reserved", "reserved", "MEM3" };
	LOG("%s: Set MCR = 0x%08x, entry map in %s, %s output voltage, input threshold for VDD=%sV\n",
			machine().describe_context(),
			val,
			entrymap[BIT(val, 12, 3)],
			BIT(val, 25) ? "rail-to-rail" : "reduced",
			BIT(val, 24) ? "5.0" : "3.3");

	static char const *const size[4] = { "32 bit", "reserved", "16 bit", "8 bit" };
	char const *const refresh[8] = {
			"every 128 prescaler time units",
			"every 64 prescaler time units",
			"every 32 prescaler time units",
			"every 16 prescaler time units",
			"every 8 prescaler time units",
			"every 4 prescaler time units",
			"every 2 prescaler time units",
			"disabled" };
	LOG("IRAM %s mode, refresh %s\n",
			BIT(val, 20) ? "normal" : "test",        // IRAM refresh test
			refresh[BIT(val, 16, 2)]);               // IRAM refresh rate
	if (BIT(val, 21))
	{
		LOG("MEM0 %s, bus hold break %s, parity %s, byte %s\n",
				size[BIT(val, 0, 2)],                   // MEM0 bus size
				BIT(val,  8) ? "disabled" : "enabled",  // MEM0 bus hold break
				BIT(val, 28) ? "disabled" : "enabled",  // MEM0 parity
				BIT(val, 15) ? "strobe"   : "enable");  // MEM0 byte mode
	}
	else
	{
		LOG("MEM0 %s %s DRAM, hold time %s, parity %s\n",
				size[BIT(val, 0, 2)],                    // MEM0 bus size
				BIT(val, 15) ? "fast page" : "EDO",      // MEM0 DRAM type
				BIT(val,  8) ? "1 cycle"   : "0 cycles", // MEM0 bus hold
				BIT(val, 28) ? "disabled"  : "enabled"); // MEM0 parity
	}
	LOG("MEM1 %s, bus hold break %s, parity %s, byte %s\n",
			size[BIT(val, 2, 2)],                   // MEM1 bus size
			BIT(val,  9) ? "disabled" : "enabled",  // MEM1 bus hold break
			BIT(val, 29) ? "disabled" : "enabled",  // MEM1 parity
			BIT(val, 19) ? "strobe"   : "enable");  // MEM1 byte mode
	LOG("MEM2 %s, bus hold break %s, parity %s, byte %s, wait %s\n",
			size[BIT(val, 4, 2)],                   // MEM2 bus size
			BIT(val, 10) ? "disabled" : "enabled",  // MEM2 bus hold break
			BIT(val, 30) ? "disabled" : "enabled",  // MEM2 parity
			BIT(val, 23) ? "strobe"   : "enable",   // MEM2 byte mode
			BIT(val, 26) ? "disabled" : "enabled"); // MEM2 wait
	LOG("MEM3 %s, bus hold break %s, parity %s\n",
			size[BIT(val, 6, 2)],                   // MEM3 bus size
			BIT(val, 11) ? "disabled" : "enabled",  // MEM3 bus hold break
			BIT(val, 31) ? "disabled" : "enabled"); // MEM3 parity

	// bits 14..12 EntryTableMap
	const int which = (val & 0x7000) >> 12;
	assert(which < 4 || which == 7);
	m_core->trap_entry = s_trap_entries[which];

	// this was moved to an I/O address for the E1-X core
	// apparently this method still works as the Limenko games use it
	const uint8_t power_down_req = BIT(val, 22);
	if (!power_down_req && m_power_down_req)
	{
		LOG("entering power down\n");
		m_core->powerdown = 1;
	}
	m_power_down_req = power_down_req;
}

void hyperstone_xs_device::update_memory_control()
{
	const uint32_t val = m_core->global_regs[MCR_REGISTER];

	static char const *const entrymap[8] = { "MEM0", "MEM1", "MEM2", "IRAM", "reserved", "reserved", "reserved", "MEM3" };
	LOG("%s: Set MCR = 0x%08x, entry map in %s\n",
			machine().describe_context(),
			val,
			entrymap[BIT(val, 12, 3)]);

	static char const *const size[4] = { "32 bit", "reserved", "16 bit", "8 bit" };
	if (BIT(val, 21))
	{
		LOG("MEM0 %s, bus hold break %s, parity %s, byte %s\n",
				size[BIT(val, 0, 2)],                   // MEM0 bus size
				BIT(val,  8) ? "disabled" : "enabled",  // MEM0 bus hold break
				BIT(val, 28) ? "disabled" : "enabled",  // MEM0 parity
				BIT(val, 15) ? "strobe"   : "enable");  // MEM0 byte mode
	}
	else
	{
		static char const *const dramtype[4] = { "S", "S", "EDO ", "fast page " };
		LOG("MEM0 %s %sDRAM, hold time %s, parity %s\n",
				size[BIT(val, 0, 2)],                   // MEM0 bus size
				dramtype[bitswap<2>(val, 22, 15)],      // MEM0 DRAM type
				BIT(val,  8) ? "1 cycle"  : "0 cycles", // MEM0 bus hold
				BIT(val, 28) ? "disabled" : "enabled"); // MEM0 parity
	}
	LOG("MEM1 %s, bus hold break %s, parity %s, byte %s\n",
			size[BIT(val, 2, 2)],                   // MEM1 bus size
			BIT(val,  9) ? "disabled" : "enabled",  // MEM1 bus hold break
			BIT(val, 29) ? "disabled" : "enabled",  // MEM1 parity
			BIT(val, 19) ? "strobe"   : "enable");  // MEM1 byte mode
	LOG("MEM2 %s, bus hold break %s, parity %s, byte %s, wait %s\n",
			size[BIT(val, 4, 2)],                   // MEM2 bus size
			BIT(val, 10) ? "disabled" : "enabled",  // MEM2 bus hold break
			BIT(val, 30) ? "disabled" : "enabled",  // MEM2 parity
			BIT(val, 23) ? "strobe"   : "enable",   // MEM2 byte mode
			BIT(val, 26) ? "disabled" : "enabled"); // MEM2 wait
	LOG("MEM3 %s, bus hold break %s, parity %s\n",
			size[BIT(val, 6, 2)],                   // MEM3 bus size
			BIT(val, 11) ? "disabled" : "enabled",  // MEM3 bus hold break
			BIT(val, 31) ? "disabled" : "enabled"); // MEM3 parity

	// install SDRAM mode and control handlers if appropriate
	if (!BIT(val, 21) && !BIT(val, 22))
		install_sdram_mode_control();

	// bits 14..12 EntryTableMap
	const int which = (val & 0x7000) >> 12;
	assert(which < 4 || which == 7);
	m_core->trap_entry = s_trap_entries[which];
}

void hyperstone_xsr_device::update_memory_control()
{
	const uint32_t val = m_core->global_regs[MCR_REGISTER];

	static char const *const entrymap[8] = { "MEM0", "MEM1", "MEM2", "IRAM", "reserved", "reserved", "reserved", "MEM3" };
	LOG("%s: Set MCR = 0x%08x, entry map in %s\n",
			machine().describe_context(),
			val,
			entrymap[BIT(val, 12, 3)]);

	static char const *const size[4] = { "32 bit", "reserved", "16 bit", "8 bit" };
	if (BIT(val, 21))
	{
		LOG("MEM0 %s, bus hold break %s, parity %s\n",
				size[BIT(val, 0, 2)],                   // MEM0 bus size
				BIT(val,  8) ? "disabled" : "enabled",  // MEM0 bus hold break
				BIT(val, 28) ? "disabled" : "enabled"); // MEM0 parity
	}
	else
	{
		static char const *const dramtype[4] = { "S", "S", "EDO ", "fast page " };
		LOG("MEM0 %s %sDRAM, hold time %s, parity %s\n",
				size[BIT(val, 0, 2)],                   // MEM0 bus size
				dramtype[bitswap<2>(val, 22, 15)],      // MEM0 DRAM type
				BIT(val,  8) ? "1 cycle"  : "0 cycles", // MEM0 bus hold
				BIT(val, 28) ? "disabled" : "enabled"); // MEM0 parity
	}
	LOG("MEM1 %s, bus hold break %s, parity %s\n",
			size[BIT(val, 2, 2)],                   // MEM1 bus size
			BIT(val,  9) ? "disabled" : "enabled",  // MEM1 bus hold break
			BIT(val, 29) ? "disabled" : "enabled"); // MEM1 parity
	LOG("MEM2 %s, bus hold break %s, parity %s, wait %s\n",
			size[BIT(val, 4, 2)],                   // MEM2 bus size
			BIT(val, 10) ? "disabled" : "enabled",  // MEM2 bus hold break
			BIT(val, 30) ? "disabled" : "enabled",  // MEM2 parity
			BIT(val, 26) ? "disabled" : "enabled"); // MEM2 wait
	LOG("MEM3 %s, bus hold break %s, parity %s\n",
			size[BIT(val, 6, 2)],                   // MEM3 bus size
			BIT(val, 11) ? "disabled" : "enabled",  // MEM3 bus hold break
			BIT(val, 31) ? "disabled" : "enabled"); // MEM3 parity

	// install SDRAM mode and control handlers if appropriate
	if (!BIT(val, 21) && !BIT(val, 22))
		install_sdram_mode_control();

	// bits 14..12 EntryTableMap
	const int which = (val & 0x7000) >> 12;
	assert(which < 4 || which == 7);
	m_core->trap_entry = s_trap_entries[which];
}


void hyperstone_x_device::power_down_w(uint32_t data)
{
	// actually has latency of a few clock cycles
	LOG("%s: entering power down\n", machine().describe_context());
	m_core->powerdown = 1;
}

void hyperstone_x_device::sleep_w(uint32_t data)
{
	logerror("%s: unimplemented sleep mode\n", machine().describe_context());
}


void hyperstone_xs_device::sdram_mode_w(offs_t offset, uint32_t data)
{
	// writes to mode register of the connected SDRAM
	LOG("%s: set SDRAM mode = 0x%07x\n", machine().describe_context(), offset);
}


void hyperstone_xs_device::sdram_control_w(offs_t offset, uint32_t data)
{
	const uint32_t val = offset << 2;
	LOG("%s: set SDCR = 0x%08x\n", machine().describe_context(), val);
	LOG("MEM0 SDRAM bank bits 0x%08x, second SDRAM chip select CS#1 %s, A%u selects CS#0/CS#1, CAS latency %s, SDCLK CPU clock%s\n",
			BIT(val, 12, 9) << 20,
			BIT(val, 11) ? "disabled" : "enabled",
			BIT(val, 8, 3) + 21,
			BIT(val, 6) ? "2 clock cycles" : "1 clock cycle",
			BIT(val, 3) ? " / 2" : "");
}

void hyperstone_xsr_device::sdram_control_w(offs_t offset, uint32_t data)
{
	const uint32_t val = offset << 2;
	static char const *const sdclk[4] = { "CPU clock", "reserved", "CPU clock / 2", "CPU clock / 4" };
	LOG("%s: set SDCR = 0x%08x\n", machine().describe_context(), val);
	LOG("MEM0 SDRAM bank bits 0x%08x, second SDRAM chip select CS#1 %s, A%u selects CS#0/CS#1, CAS latency %s, SDCLK based on %s %s, %sdelayed synchronisation\n",
			BIT(val, 12, 9) << 20,
			BIT(val, 11) ? "disabled" : "enabled",
			BIT(val, 8, 3) + 21,
			BIT(val, 6) ? "2 clock cycles" : "1 clock cycle",
			BIT(val, 5) ? "rising" : "falling",
			sdclk[BIT(val, 2, 2)],
			BIT(val, 4) ? "non-" : "");
}

void hyperstone_xs_device::install_sdram_mode_control()
{
	if (!m_sdram_installed)
	{
		m_program->unmap_read(0x20000000, 0x3fffffff);
		m_program->install_write_handler(0x20000000, 0x2fffffff, emu::rw_delegate(*this, FUNC(hyperstone_xs_device::sdram_mode_w)));
		m_program->install_write_handler(0x30000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(hyperstone_xs_device::sdram_control_w)));

		m_sdram_installed = true;
	}
}

TIMER_CALLBACK_MEMBER( hyperstone_device::timer_callback )
{
	int update = param;

	// update the values if necessary
	if (update)
		update_timer_prescale();

	// see if the timer is right for firing
	compute_tr();
	if (!((m_core->tr_result - TCR) & 0x80000000))
	{
		m_core->timer_int_pending = 1;
		if (!BIT(FCR, 23))
		{
			if (m_core->powerdown)
				LOG("exiting power down for timer\n");
			m_core->powerdown = 0;
		}
	}
	else
	{
		// adjust ourselves for the next time
		adjust_timer_interrupt();
	}
}




uint32_t hyperstone_device::get_global_register(uint8_t code)
{
/*
    if( code >= 16 )
    {
        switch( code )
        {
        case 16:
        case 17:
        case 28:
        case 29:
        case 30:
        case 31:
            LOG("read _Reserved_ Global Register %d @ %08X\n",code,PC);
            break;

        case BCR_REGISTER:
            LOG("read write-only BCR register @ %08X\n",PC);
            return 0;

        case TPR_REGISTER:
            LOG("read write-only TPR register @ %08X\n",PC);
            return 0;

        case FCR_REGISTER:
            LOG("read write-only FCR register @ %08X\n",PC);
            return 0;

        case MCR_REGISTER:
            LOG("read write-only MCR register @ %08X\n",PC);
            return 0;
        }
    }
*/
	if (code == TR_REGISTER)
	{
		// it is common to poll this in a loop
		if (m_core->icount > m_core->tr_clocks_per_tick / 2)
			m_core->icount -= m_core->tr_clocks_per_tick / 2;
		compute_tr();
		return m_core->tr_result;
	}
	return m_core->global_regs[code & 0x1f];
}

void hyperstone_device::set_local_register(uint8_t code, uint32_t val)
{
	m_core->local_regs[(code + GET_FP) & 0x3f] = val;
}

void hyperstone_device::set_global_register(uint8_t code, uint32_t val)
{
	//TODO: add correct FER set instruction
	code &= 0x1f;
	switch (code)
	{
		case PC_REGISTER:
			SET_PC(val);
			return;
		case SR_REGISTER:
			{
				const bool privilege_error = !GET_S && !GET_L && (val & L_MASK);
				SET_LOW_SR(val); // only a RET instruction can change the full content of SR
				SR &= ~0x40; // reserved bit 6 always zero
				if (privilege_error)
					execute_exception(TRAPNO_PRIVILEGE_ERROR);
			}
			return;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		// are the below ones set only when privilege bit is set?
		case 17:
			m_core->global_regs[code] = val;
			return;
		case SP_REGISTER:
		case UB_REGISTER:
			m_core->global_regs[code] = val & ~3;
			return;
		case BCR_REGISTER:
			m_core->global_regs[code] = val;
			update_bus_control();
			return;
		case TPR_REGISTER:
			m_core->global_regs[code] = val;
			if (!(val & 0x80000000)) /* change immediately */
			{
				compute_tr();
				update_timer_prescale();
			}
			adjust_timer_interrupt();
			return;
		case TCR_REGISTER:
			if (m_core->global_regs[code] != val)
			{
				m_core->global_regs[code] = val;
				adjust_timer_interrupt();
			}
			return;
		case TR_REGISTER:
			m_core->global_regs[code] = val;
			m_core->tr_base_value = val;
			m_core->tr_base_cycles = total_cycles();
			adjust_timer_interrupt();
			return;
		case WCR_REGISTER:
			m_core->global_regs[code] = val;
			return;
		case ISR_REGISTER:
			return;
		case FCR_REGISTER:
			if ((m_core->global_regs[code] ^ val) & 0x00800000)
				adjust_timer_interrupt();
			m_core->global_regs[code] = val;
			return;
		case MCR_REGISTER:
			m_core->global_regs[code] = val;
			update_memory_control();
			return;
		case 28:
		case 29:
		case 30:
		case 31:
			m_core->global_regs[code] = val;
			return;
	}
}

/*static*/ const int32_t hyperstone_device::s_immediate_values[16] =
{
	16, 0, 0, 0, 32, 64, 128, int32_t(0x80000000),
	-8, -7, -6, -5, -4, -3, -2, -1
};

constexpr uint32_t WRITE_ONLY_REGMASK = (1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER);

inline ATTR_FORCE_INLINE void hyperstone_device::check_delay_pc()
{
	// if PC is used in a delay instruction, the delayed PC should be used
	if (!m_core->delay_slot)
	{
		m_core->delay_slot_taken = 0;
	}
	else
	{
		using std::swap;
		swap(PC, m_core->delay_pc);
		m_core->delay_slot = 0;
		m_core->delay_slot_taken = 1;
	}
}

void hyperstone_device::ignore_immediate_s()
{
	static const uint32_t lengths[16] = {
			1 << ILC_SHIFT, 3 << ILC_SHIFT, 2 << ILC_SHIFT, 2 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT,
			1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT, 1 << ILC_SHIFT };
	static const uint32_t offsets[16] = { 0, 4, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const uint8_t nybble = m_op & 0x0f;
	m_instruction_length = lengths[nybble];
	PC += offsets[nybble];
}

uint32_t hyperstone_device::decode_immediate_s()
{
	const uint8_t nybble = m_op & 0x0f;
	switch (nybble)
	{
		case 0:
			return 16;
		case 1:
		{
			m_instruction_length = 3 << ILC_SHIFT;
			uint32_t extra_u = (m_pr16(PC) << 16) | m_pr16(PC + 2);
			PC += 4;
			return extra_u;
		}
		case 2:
		{
			m_instruction_length = 2 << ILC_SHIFT;
			uint32_t extra_u = m_pr16(PC);
			PC += 2;
			return extra_u;
		}
		case 3:
		{
			m_instruction_length = 2 << ILC_SHIFT;
			uint32_t extra_u = 0xffff0000 | m_pr16(PC);
			PC += 2;
			return extra_u;
		}
		default:
			return s_immediate_values[nybble];
	}
}

uint32_t hyperstone_device::decode_const()
{
	const uint16_t imm_1 = m_pr16(PC);

	PC += 2;

	if (imm_1 & 0x8000)
	{
		const uint16_t imm_2 = m_pr16(PC);

		PC += 2;
		m_instruction_length = 3 << ILC_SHIFT;

		uint32_t imm = imm_2;
		imm |= ((imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			imm |= 0xc0000000;
		return imm;
	}
	else
	{
		m_instruction_length = 2 << ILC_SHIFT;

		uint32_t imm = imm_1 & 0x3fff;

		if (imm_1 & 0x4000)
			imm |= 0xffffc000;
		return imm;
	}
}

int32_t hyperstone_device::decode_pcrel()
{
	if (OP & 0x80)
	{
		uint16_t next = m_pr16(PC);

		PC += 2;
		m_instruction_length = 2 << ILC_SHIFT;

		int32_t offset = (OP & 0x7f) << 16;
		offset |= (next & 0xfffe);

		if (next & 1)
			offset |= 0xff800000;

		return offset;
	}
	else
	{
		int32_t offset = OP & 0x7e;

		if (OP & 1)
			offset |= 0xffffff80;

		return offset;
	}
}

inline void hyperstone_device::ignore_pcrel()
{
	if (m_op & 0x80)
	{
		PC += 2;
		m_instruction_length = 2 << ILC_SHIFT;
	}
}

void hyperstone_device::execute_trap(uint8_t trapno)
{
	debugger_exception_hook(int(unsigned(trapno)));

	const uint32_t addr = get_trap_addr(trapno);
	const uint8_t reg = GET_FP + GET_FL;
	SET_ILC(m_instruction_length);
	const uint32_t oldSR = SR;

	SET_FL(6);
	SET_FP(reg);

	m_core->local_regs[(0 + reg) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(1 + reg) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= (L_MASK | S_MASK);

	PC = addr;

	m_core->icount -= m_core->clock_cycles_2;
}


void hyperstone_device::execute_int(uint32_t addr)
{
	const uint8_t reg = GET_FP + GET_FL;
	const uint32_t oldSR = SR;

	SET_FL(2);
	SET_FP(reg);

	m_core->local_regs[(0 + reg) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(1 + reg) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= (L_MASK | S_MASK | I_MASK);

	PC = addr;

	m_core->icount -= m_core->clock_cycles_2;
}

/* TODO: mask Parity Error and Extended Overflow exceptions */
void hyperstone_device::execute_exception(uint8_t trapno)
{
	debugger_exception_hook(int(unsigned(trapno)));

	const uint32_t addr = get_trap_addr(trapno);
	const uint8_t reg = GET_FP + GET_FL;

	if (!m_core->delay_slot_taken)
		SET_ILC(m_instruction_length);
	else
		PC = m_core->delay_pc - (m_instruction_length >> ILC_SHIFT);

	// RET does not automatically set P
	if (((m_op & 0xfef0) != 0x0400) || !(m_op & 0x010e))
		SET_P(1);

	const uint32_t oldSR = SR;

	SET_FL(2);
	SET_FP(reg);

	m_core->local_regs[(0 + reg) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(1 + reg) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= (L_MASK | S_MASK);

	PC = addr;

	m_core->icount -= m_core->clock_cycles_2;
}

void hyperstone_device::execute_software()
{
	check_delay_pc();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_core->local_regs[(src_code + fp) & 0x3f];
	const uint32_t sregf = m_core->local_regs[(src_code + 1 + fp) & 0x3f];

	SET_ILC(1 << ILC_SHIFT);

	const uint32_t addr = get_emu_code_addr((m_op & 0xff00) >> 8);
	const uint8_t reg = fp + GET_FL;

	//since it's sure the register is in the register part of the stack,
	//set the stack address to a value above the highest address
	//that can be set by a following frame instruction
	const uint32_t stack_of_dst = (SP & ~0xff) + 0x100 + (((fp + DST_CODE) & 0x3f) << 2); //converted to 32bits offset

	m_core->local_regs[(reg + 0) & 0x3f] = stack_of_dst;
	m_core->local_regs[(reg + 1) & 0x3f] = sreg;
	m_core->local_regs[(reg + 2) & 0x3f] = sregf;
	m_core->local_regs[(reg + 3) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(reg + 4) & 0x3f] = SR;

	SET_FL(6);
	SET_FP(reg);

	SR &= ~(M_MASK | T_MASK);
	SR |= L_MASK;

	PC = addr;

	m_core->icount -= m_core->clock_cycles_6;
}


/*
    IRQ lines :
        0 - IO2     (trap 48)
        1 - IO1     (trap 49)
        2 - INT4    (trap 50)
        3 - INT3    (trap 51)
        4 - INT2    (trap 52)
        5 - INT1    (trap 53)
        6 - IO3     (trap 54)
        7 - TIMER   (trap 55)
*/

#define INT1_LINE_STATE     (ISR & 0x01)
#define INT2_LINE_STATE     (ISR & 0x02)
#define INT3_LINE_STATE     (ISR & 0x04)
#define INT4_LINE_STATE     (ISR & 0x08)
#define IO1_LINE_STATE      (ISR & 0x10)
#define IO2_LINE_STATE      (ISR & 0x20)
#define IO3_LINE_STATE      (ISR & 0x40)

template <hyperstone_device::is_timer Timer>
void hyperstone_device::check_interrupts()
{
	// Interrupt-Lock flag isn't set
	if (GET_L)
		return;

	// quick exit if nothing
	if (Timer == NO_TIMER && (ISR & 0x7f) == 0)
		return;

	// IO3 is priority 5; state is in bit 6 of ISR; FCR bit 10 enables input and FCR bit 8 inhibits interrupt
	if (IO3_LINE_STATE && (FCR & 0x00000500) == 0x00000400)
	{
		standard_irq_callback(IRQ_IO3, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_IO3));
		return;
	}

	// timer int might be priority 6 if FCR bits 20-21 == 3; FCR bit 23 inhibits interrupt
	if (Timer && (FCR & 0x00b00000) == 0x00300000)
	{
		m_core->timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	// INT1 is priority 7; state is in bit 0 of ISR; FCR bit 28 inhibits interrupt
	if (INT1_LINE_STATE && (FCR & 0x10000000) == 0x00000000)
	{
		standard_irq_callback(IRQ_INT1, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_INT1));
		return;
	}

	// timer int might be priority 8 if FCR bits 20-21 == 2; FCR bit 23 inhibits interrupt
	if (Timer && (FCR & 0x00b00000) == 0x00200000)
	{
		m_core->timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	// INT2 is priority 9; state is in bit 1 of ISR; FCR bit 29 inhibits interrupt
	if (INT2_LINE_STATE && (FCR & 0x20000000) == 0x00000000)
	{
		standard_irq_callback(IRQ_INT2, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_INT2));
		return;
	}

	// timer int might be priority 10 if FCR bits 20-21 == 1; FCR bit 23 inhibits interrupt
	if (Timer && (FCR & 0x00b00000) == 0x00100000)
	{
		m_core->timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	// INT3 is priority 11; state is in bit 2 of ISR; FCR bit 30 inhibits interrupt
	if (INT3_LINE_STATE && (FCR & 0x40000000) == 0x00000000)
	{
		standard_irq_callback(IRQ_INT3, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_INT3));
		return;
	}

	// timer int might be priority 12 if FCR bits 20-21 == 0; FCR bit 23 inhibits interrupt
	if (Timer && (FCR & 0x00b00000) == 0x00000000)
	{
		m_core->timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	// INT4 is priority 13; state is in bit 3 of ISR; FCR bit 31 inhibits interrupt
	if (INT4_LINE_STATE && (FCR & 0x80000000) == 0x00000000)
	{
		standard_irq_callback(IRQ_INT4, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_INT4));
		return;
	}

	// IO1 is priority 14; state is in bit 4 of ISR; FCR bit 2 enables input and FCR bit 0 inhibits interrupt
	if (IO1_LINE_STATE && (FCR & 0x00000005) == 0x00000004)
	{
		standard_irq_callback(IRQ_IO1, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_IO1));
		return;
	}

	// IO2 is priority 15; state is in bit 5 of ISR; FCR bit 6 enables input and FCR bit 4 inhibits interrupt
	if (IO2_LINE_STATE && (FCR & 0x00000050) == 0x00000040)
	{
		standard_irq_callback(IRQ_IO2, m_core->global_regs[0]);
		execute_int(get_trap_addr(TRAPNO_IO2));
		return;
	}
}


void hyperstone_device::device_start()
{
	m_enable_drc = allow_drc();
	if (m_enable_drc)
	{
		m_cache.allocate_cache(mconfig().options().drc_rwx());
		m_core = m_cache.alloc_near<internal_hyperstone_state>();
	}
	else
	{
		m_core = &m_local_core;
	}
	memset(m_core, 0, sizeof(internal_hyperstone_state));

#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	if (m_enable_drc)
		m_trace_log = fopen("e1_drc.log", "wb");
	else
		m_trace_log = fopen("e1_interpreter.log", "wb");
#endif

	memset(m_op_counts, 0, sizeof(uint32_t) * 256);
	std::fill(std::begin(m_core->global_regs), std::end(m_core->global_regs), 0);
	std::fill(std::begin(m_core->local_regs), std::end(m_core->local_regs), 0);
	m_core->intblock = 0;
	m_core->powerdown = 0;

	m_power_down_req = 1;

	m_op = 0;
	m_instruction_length = 0;
	m_instruction_length_valid = false;

	m_program = &space(AS_PROGRAM);
	if (m_program->data_width() == 16)
	{
		m_program->cache(m_cache16);
		m_program->specific(m_specific16);
		m_read_byte      = b_r_delegate( [this] (offs_t address) { return m_specific16.read_byte(address); });
		m_read_halfword  = hw_r_delegate([this] (offs_t address) { return m_specific16.read_word(address & ~offs_t(1)); });
		m_read_word      = w_r_delegate( [this] (offs_t address) { return m_specific16.read_dword(address & ~offs_t(3)); });
		m_write_byte     = b_w_delegate( [this] (offs_t address, uint8_t  data) { m_specific16.write_byte(address, data); });
		m_write_halfword = hw_w_delegate([this] (offs_t address, uint16_t data) { m_specific16.write_word(address & ~offs_t(1), data); });
		m_write_word     = w_w_delegate( [this] (offs_t address, uint32_t data) { m_specific16.write_dword(address & ~offs_t(3), data); });

		m_pr16 = [this] (offs_t address) -> u16 { return m_cache16.read_word(address); };
		m_prptr = [this] (offs_t address) -> const void * { return m_cache16.read_ptr(address); };
	}
	else
	{
		m_program->cache(m_cache32);
		m_program->specific(m_specific32);
		m_read_byte      = b_r_delegate( [this] (offs_t address) { return m_specific32.read_byte(address); });
		m_read_halfword  = hw_r_delegate([this] (offs_t address) { return m_specific32.read_word(address & ~offs_t(1)); });
		m_read_word      = w_r_delegate( [this] (offs_t address) { return m_specific32.read_dword(address & ~offs_t(3)); });
		m_write_byte     = b_w_delegate( [this] (offs_t address, uint8_t  data) { m_specific32.write_byte(address, data); });
		m_write_halfword = hw_w_delegate([this] (offs_t address, uint16_t data) { m_specific32.write_word(address & ~offs_t(1), data); });
		m_write_word     = w_w_delegate( [this] (offs_t address, uint32_t data) { m_specific32.write_dword(address & ~offs_t(3), data); });

		m_pr16 = [this](offs_t address) -> u16 { return m_cache32.read_word(address); };
		if (ENDIANNESS_NATIVE != ENDIANNESS_BIG)
			m_prptr = [this] (offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache32.read_ptr(address & ~3));
				if(!(address & 2))
					ptr++;
				return ptr;
			};
		else
			m_prptr = [this] (offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache32.read_ptr(address & ~3));
				if(address & 2)
					ptr++;
				return ptr;
			};
	}

	address_space &iospace = space(AS_IO);
	if (iospace.data_width() == 16)
	{
		iospace.specific(m_io16);
		m_read_io  = w_r_delegate([this] (offs_t address) -> uint32_t { return m_io16.read_word(address >> 13); });
		m_write_io = w_w_delegate([this] (offs_t address, uint32_t data) { m_io16.write_word(address >> 13, uint16_t(data)); });
	}
	else
	{
		iospace.specific(m_io32);
		m_read_io  = w_r_delegate([this] (offs_t address) -> uint32_t { return m_io32.read_dword(address >> 13); });
		m_write_io = w_w_delegate([this] (offs_t address, uint32_t data) { m_io32.write_dword(address >> 13, data); });
	}

	m_timer = timer_alloc(FUNC(hyperstone_device::timer_callback), this);
	m_core->clock_scale_mask = 0;

	for (uint8_t i = 0; i < 16; i++)
	{
		m_core->fl_lut[i] = (i ? i : 16);
	}

	if (m_enable_drc)
	{
		const uint32_t umlflags = 0;
		m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, umlflags, 4, 32, 1);

		// add UML symbols
		m_drcuml->symbol_add(&m_core->global_regs[PC_REGISTER],  sizeof(m_core->global_regs[PC_REGISTER]),  "pc");
		m_drcuml->symbol_add(&m_core->global_regs[SR_REGISTER],  sizeof(m_core->global_regs[SR_REGISTER]),  "sr");
		m_drcuml->symbol_add(&m_core->global_regs[FER_REGISTER], sizeof(m_core->global_regs[FER_REGISTER]), "fer");
		m_drcuml->symbol_add(&m_core->global_regs[SP_REGISTER],  sizeof(m_core->global_regs[SP_REGISTER]),  "sp");
		m_drcuml->symbol_add(&m_core->global_regs[UB_REGISTER],  sizeof(m_core->global_regs[UB_REGISTER]),  "ub");
		m_drcuml->symbol_add(&m_core->trap_entry,                sizeof(m_core->trap_entry),                "trap_entry");
		m_drcuml->symbol_add(&m_core->delay_pc,                  sizeof(m_core->delay_pc),                  "delay_pc");
		m_drcuml->symbol_add(&m_core->delay_slot,                sizeof(m_core->delay_slot),                "delay_slot");
		m_drcuml->symbol_add(&m_core->delay_slot_taken,          sizeof(m_core->delay_slot_taken),          "delay_slot_taken");
		m_drcuml->symbol_add(&m_core->intblock,                  sizeof(m_core->intblock),                  "intblock");
		m_drcuml->symbol_add(&m_core->powerdown,                 sizeof(m_core->powerdown),                 "powerdown");
		m_drcuml->symbol_add(&m_core->arg0,                      sizeof(m_core->arg0),                      "arg0");
		m_drcuml->symbol_add(&m_core->arg1,                      sizeof(m_core->arg1),                      "arg1");
		m_drcuml->symbol_add(&m_core->icount,                    sizeof(m_core->icount),                    "icount");

		char buf[4];
		buf[3] = '\0';
		buf[0] = 'g';
		for (int i = 0; i < 32; i++)
		{
			if (9 < i)
			{
				buf[1] = '0' + (i / 10);
				buf[2] = '0' + (i % 10);
			}
			else
			{
				buf[1] = '0' + i;
				buf[2] = '\0';
			}
			m_drcuml->symbol_add(&m_core->global_regs[i], sizeof(uint32_t), buf);
		}
		buf[0] = 'l';
		for (int i = 0; i < 64; i++)
		{
			if (9 < i)
			{
				buf[1] = '0' + (i / 10);
				buf[2] = '0' + (i % 10);
			}
			else
			{
				buf[1] = '0' + i;
				buf[2] = '\0';
			}
			m_drcuml->symbol_add(&m_core->local_regs[i], sizeof(uint32_t), buf);
		}

		m_drcuml->symbol_add(&m_core->arg0, sizeof(uint32_t), "arg0");
		m_drcuml->symbol_add(&m_core->arg1, sizeof(uint32_t), "arg1");

		// initialize the front-end helper
		m_drcfe = std::make_unique<e132xs_frontend>(*this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, m_single_instruction_mode ? 1 : COMPILE_MAX_SEQUENCE);

		// generate invariant code
		generate_invariant();

		// mark the cache dirty so it is updated on next execute
		m_cache_dirty = true;
	}

	// register our state for the debugger
	state_add(STATE_GENPC,    "GENPC",     m_core->global_regs[0]).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_core->global_regs[0]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_core->global_regs[1]).callimport().callexport().formatstr("%40s").noshow();
	state_add(E132XS_PC,      "PC", m_core->global_regs[0]).mask(0xffffffff);
	state_add(E132XS_SR,      "SR", m_core->global_regs[1]).mask(0xffffffff);
	state_add(E132XS_FER,     "FER", m_core->global_regs[2]).mask(0xffffffff);
	state_add(E132XS_G3,      "G3", m_core->global_regs[3]).mask(0xffffffff);
	state_add(E132XS_G4,      "G4", m_core->global_regs[4]).mask(0xffffffff);
	state_add(E132XS_G5,      "G5", m_core->global_regs[5]).mask(0xffffffff);
	state_add(E132XS_G6,      "G6", m_core->global_regs[6]).mask(0xffffffff);
	state_add(E132XS_G7,      "G7", m_core->global_regs[7]).mask(0xffffffff);
	state_add(E132XS_G8,      "G8", m_core->global_regs[8]).mask(0xffffffff);
	state_add(E132XS_G9,      "G9", m_core->global_regs[9]).mask(0xffffffff);
	state_add(E132XS_G10,     "G10", m_core->global_regs[10]).mask(0xffffffff);
	state_add(E132XS_G11,     "G11", m_core->global_regs[11]).mask(0xffffffff);
	state_add(E132XS_G12,     "G12", m_core->global_regs[12]).mask(0xffffffff);
	state_add(E132XS_G13,     "G13", m_core->global_regs[13]).mask(0xffffffff);
	state_add(E132XS_G14,     "G14", m_core->global_regs[14]).mask(0xffffffff);
	state_add(E132XS_G15,     "G15", m_core->global_regs[15]).mask(0xffffffff);
	state_add(E132XS_G16,     "G16", m_core->global_regs[16]).mask(0xffffffff);
	state_add(E132XS_G17,     "G17", m_core->global_regs[17]).mask(0xffffffff);
	state_add(E132XS_SP,      "SP", m_core->global_regs[18]).mask(0xffffffff);
	state_add(E132XS_UB,      "UB", m_core->global_regs[19]).mask(0xffffffff);
	state_add(E132XS_BCR,     "BCR", m_core->global_regs[20]).mask(0xffffffff);
	state_add(E132XS_TPR,     "TPR", m_core->global_regs[21]).mask(0xffffffff);
	state_add(E132XS_TCR,     "TCR", m_core->global_regs[22]).mask(0xffffffff);
	state_add(E132XS_TR,      "TR", m_core->global_regs[23]).mask(0xffffffff);
	state_add(E132XS_WCR,     "WCR", m_core->global_regs[24]).mask(0xffffffff);
	state_add(E132XS_ISR,     "ISR", m_core->global_regs[25]).mask(0xffffffff);
	state_add(E132XS_FCR,     "FCR", m_core->global_regs[26]).mask(0xffffffff);
	state_add(E132XS_MCR,     "MCR", m_core->global_regs[27]).mask(0xffffffff);
	state_add(E132XS_G28,     "G28", m_core->global_regs[28]).mask(0xffffffff);
	state_add(E132XS_G29,     "G29", m_core->global_regs[29]).mask(0xffffffff);
	state_add(E132XS_G30,     "G30", m_core->global_regs[30]).mask(0xffffffff);
	state_add(E132XS_G31,     "G31", m_core->global_regs[31]).mask(0xffffffff);
	for (int i = 0; i < 16; i++)
		state_add(E132XS_CL0 + i, util::string_format("L%d", i).c_str(), m_debug_local_regs[i]).mask(0xffffffff).callimport().callexport();
	for (int i = 0; i < 64; i++)
		state_add(E132XS_L0 + i, util::string_format("S%d", i).c_str(), m_core->local_regs[i]).mask(0xffffffff);

	save_item(NAME(m_core->global_regs));
	save_item(NAME(m_core->local_regs));
	save_item(NAME(m_core->trap_entry));
	save_item(NAME(m_core->intblock));
	save_item(NAME(m_core->powerdown));
	save_item(NAME(m_core->delay_pc));
	save_item(NAME(m_core->delay_slot));
	save_item(NAME(m_core->delay_slot_taken));
	save_item(NAME(m_core->tr_clocks_per_tick));
	save_item(NAME(m_core->tr_base_value));
	save_item(NAME(m_core->tr_base_cycles));
	save_item(NAME(m_core->timer_int_pending));
	save_item(NAME(m_core->clck_scale));
	save_item(NAME(m_core->clock_cycles_1));
	save_item(NAME(m_core->clock_cycles_2));
	save_item(NAME(m_core->clock_cycles_3));
	save_item(NAME(m_core->clock_cycles_4));
	save_item(NAME(m_core->clock_cycles_6));
	save_item(NAME(m_core->clock_cycles_36));
	save_item(NAME(m_power_down_req));
	save_item(NAME(m_instruction_length));

	// set our instruction counter
	set_icountptr(m_core->icount);
}

void hyperstone_x_device::device_start()
{
	hyperstone_device::device_start();
	m_core->clock_scale_mask = 3;

	address_space &internalspace = space(AS_INTERNAL);
	internalspace.specific(m_internal_specific);
	if (space(AS_IO).data_width() == 16)
	{
		m_read_io = w_r_delegate(
				[this] (offs_t address) -> uint32_t
				{
					if (!BIT(address, 27))
						return m_io16.read_word(address >> 13);
					else
						return m_internal_specific.read_dword(address >> 13);
				});
		m_write_io = w_w_delegate(
				[this] (offs_t address, uint32_t data)
				{
					if (!BIT(address, 27))
						m_io16.write_word(address >> 13, uint16_t(data));
					else
						m_internal_specific.write_dword(address >> 13, data);
				});
	}
	else
	{
		m_read_io = w_r_delegate(
				[this] (offs_t address) -> uint32_t
				{
					if (!BIT(address, 27))
						return m_io32.read_dword(address >> 13);
					else
						return m_internal_specific.read_dword(address >> 13);
				});
		m_write_io = w_w_delegate(
				[this] (offs_t address, uint32_t data)
				{
					if (!BIT(address, 27))
						m_io32.write_dword(address >> 13, data);
					else
						m_internal_specific.write_dword(address >> 13, data);
				});
	}
}

void hyperstone_xs_device::device_start()
{
	hyperstone_x_device::device_start();

	m_core->clock_scale_mask = 7;
	m_sdram_installed = false;
}

void hyperstone_xs_device::device_post_load()
{
	hyperstone_x_device::device_post_load();

	const uint32_t mcr = m_core->global_regs[MCR_REGISTER];
	if (!BIT(mcr, 21) && !BIT(mcr, 22))
		install_sdram_mode_control();
}

void e116_device::device_start()
{
	hyperstone_device::device_start();
	m_core->clock_scale_mask = 0;
}

void gms30c2116_device::device_start()
{
	hyperstone_device::device_start();
	m_core->clock_scale_mask = 0;
}

void e132_device::device_start()
{
	hyperstone_device::device_start();
	m_core->clock_scale_mask = 0;
}

void gms30c2132_device::device_start()
{
	hyperstone_device::device_start();
	m_core->clock_scale_mask = 0;
}

void hyperstone_device::device_reset()
{
	// TODO: Add different reset initializations for BCR, MCR, FCR, TPR

	m_core->tr_clocks_per_tick = 2;

	m_core->trap_entry = s_trap_entries[E132XS_ENTRY_MEM3]; // default entry point @ MEM3

	m_core->global_regs[BCR_REGISTER] = ~uint32_t(0);
	m_core->global_regs[MCR_REGISTER] = ~uint32_t(0);
	update_bus_control();
	update_memory_control();
	set_global_register(FCR_REGISTER, ~uint32_t(0));
	set_global_register(TPR_REGISTER, 0xc000000);

	PC = get_trap_addr(TRAPNO_RESET);

	SET_FP(0);
	SET_FL(2);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);
	SET_ILC(1 << ILC_SHIFT);

	set_local_register(0, (PC & 0xfffffffe) | GET_S);
	set_local_register(1, SR);

	m_core->icount -= m_core->clock_cycles_2;
}

void hyperstone_device::device_stop()
{
	if (m_drcfe != nullptr)
	{
		m_drcfe = nullptr;
	}
	if (m_drcuml != nullptr)
	{
		m_drcuml = nullptr;
	}
#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	fclose(m_trace_log);
#endif
#if E132XS_COUNT_INSTRUCTIONS
	uint32_t indices[256];
	for (uint32_t i = 0; i < 256; i++)
		indices[i] = i;
	for (uint32_t i = 0; i < 256; i++)
	{
		for (uint32_t j = 0; j < 256; j++)
		{
			if (m_op_counts[j] < m_op_counts[i])
			{
				uint32_t temp = m_op_counts[i];
				m_op_counts[i] = m_op_counts[j];
				m_op_counts[j] = temp;

				temp = indices[i];
				indices[i] = indices[j];
				indices[j] = temp;
			}
		}
	}
	for (uint32_t i = 0; i < 256; i++)
	{
		if (m_op_counts[i] != 0)
		{
			printf("%02x: %d\n", (uint8_t)indices[i], m_op_counts[i]);
		}
	}
#endif
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector hyperstone_device::memory_space_config() const
{
	return space_config_vector{
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config) };
}


device_memory_interface::space_config_vector hyperstone_x_device::memory_space_config() const
{
	return space_config_vector{
			std::make_pair(AS_PROGRAM,  &m_program_config),
			std::make_pair(AS_IO,       &m_io_config),
			std::make_pair(AS_INTERNAL, &m_internal_config), };
}


//-------------------------------------------------
//  state_import - import state for the debugger
//-------------------------------------------------

void hyperstone_device::state_import(const device_state_entry &entry)
{
	if ((entry.index() >= E132XS_CL0) && (entry.index() <= E132XS_CL15))
	{
		const auto index = entry.index() - E132XS_CL0;
		m_core->local_regs[(index + GET_FP) & 0x3f] = m_debug_local_regs[index];
	}
}


//-------------------------------------------------
//  state_export - export state for the debugger
//-------------------------------------------------

void hyperstone_device::state_export(const device_state_entry &entry)
{
	if ((entry.index() >= E132XS_CL0) && (entry.index() <= E132XS_CL15))
	{
		const auto index = entry.index() - E132XS_CL0;
		m_debug_local_regs[index] = m_core->local_regs[(index + GET_FP) & 0x3f];
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void hyperstone_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c FTE:%X FRM:%X ILC:%d FL:%d FP:%d",
				GET_S ? 'S':'.',
				GET_P ? 'P':'.',
				GET_T ? 'T':'.',
				GET_L ? 'L':'.',
				GET_I ? 'I':'.',
				m_core->global_regs[1] & 0x00040 ? '?':'.',
				GET_H ? 'H':'.',
				GET_M ? 'M':'.',
				GET_V ? 'V':'.',
				GET_N ? 'N':'.',
				GET_Z ? 'Z':'.',
				GET_C ? 'C':'.',
				GET_FTE,
				GET_FRM,
				GET_ILC,
				GET_FL,
				GET_FP);
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> hyperstone_device::create_disassembler()
{
	return std::make_unique<hyperstone_disassembler>(this);
}

bool hyperstone_device::get_h() const
{
	return GET_H;
}

/* Opcodes */

void hyperstone_device::hyperstone_trap()
{
	m_core->icount -= m_core->clock_cycles_1;

	static const uint32_t conditions[16] = {
		0, 0, 0, 0, N_MASK | Z_MASK, N_MASK | Z_MASK, N_MASK, N_MASK, C_MASK | Z_MASK, C_MASK | Z_MASK, C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, 0
	};
	static const bool trap_if_set[16] = {
		false, false, false, false, true, false, true, false, true, false, true, false, true, false, true, false
	};

	check_delay_pc();

	const uint8_t trapno = (m_op & 0xfc) >> 2;
	const uint8_t code = ((m_op & 0x300) >> 6) | (m_op & 0x03);

	if (trap_if_set[code])
	{
		if (SR & conditions[code])
			execute_trap(trapno);
	}
	else
	{
		if (!(SR & conditions[code]))
			execute_trap(trapno);
	}
}


#include "e132xsop.hxx"

//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t hyperstone_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t hyperstone_device::execute_max_cycles() const noexcept
{
	return 36;
}


void hyperstone_device::execute_set_input(int inputnum, int state)
{
	if (inputnum < 7)
	{
		if (state)
		{
			if (!BIT(ISR, inputnum))
			{
				ISR |= 1 << inputnum;

				if ((inputnum < 4) && !BIT(FCR, 28 + inputnum))
				{
					if (m_core->powerdown)
						LOG("exiting power down for INT%d\n", inputnum + 1);
					m_core->powerdown = 0;
				}

				if ((inputnum == INPUT_IO3) && ((FCR & 0x00000500) == 0x00000400))
				{
					if (m_core->powerdown)
						LOG("exiting power down for IO3\n", inputnum + 1);
					m_core->powerdown = 0;
				}
			}
		}
		else
		{
			ISR &= ~(1 << inputnum);
		}
	}
}

void hyperstone_device::hyperstone_reserved()
{
	LOG("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP);
}

void hyperstone_device::hyperstone_do()
{
	fatalerror("Executed hyperstone_do instruction. PC = %08X\n", PC-4);
}

uint32_t hyperstone_device::imm_length(uint16_t op)
{
	switch (op & 0x0f)
	{
		case 0:
		default:
			return 1;
		case 1:
			return 3;
		case 2:
		case 3:
			return 2;
	}
}

int32_t hyperstone_device::get_instruction_length(uint16_t op)
{
	switch (op >> 8)
	{
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xee: case 0xef:
		m_instruction_length = ((m_pr16(PC+2) & 0x8000) ? 3 : 2) << ILC_SHIFT;
		break;
	case 0x61: case 0x63: case 0x65: case 0x67: case 0x69: case 0x6b: case 0x6d: case 0x6f:
	case 0x71: case 0x73: case 0x75: case 0x77: case 0x79: case 0x7b: case 0x7d: case 0x7f:
		m_instruction_length = imm_length(op) << ILC_SHIFT;
		break;
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc:
		m_instruction_length = ((op & 0x80) ? 2 : 1) << ILC_SHIFT;
		break;
	case 0xce:
		m_instruction_length = 2 << ILC_SHIFT;
		break;
	default:
		m_instruction_length = 1 << ILC_SHIFT;
		break;
	}
	m_instruction_length_valid = true;
	return m_instruction_length;
}

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void hyperstone_device::execute_run()
{
	if (m_enable_drc)
	{
		execute_run_drc();
		return;
	}

	if (!m_instruction_length_valid)
		SET_ILC(get_instruction_length(m_pr16(PC)));

	while (m_core->icount > 0)
	{
		if (m_core->powerdown)
		{
			m_core->icount = 0;
			break;
		}

		if (--m_core->intblock <= 0)
		{
			m_core->intblock = 0;
			if (m_core->timer_int_pending)
				check_interrupts<IS_TIMER>();
			else
				check_interrupts<NO_TIMER>();
		}

#if E132XS_LOG_INTERPRETER_REGS
		dump_registers();
#endif

		debugger_instruction_hook(PC);

		OP = m_pr16(PC);
		PC += 2;

		m_instruction_length = 1 << ILC_SHIFT;

#if E132XS_COUNT_INSTRUCTIONS
		m_op_counts[m_op >> 8]++;
#endif
		switch (m_op >> 8)
		{
			case 0x00: hyperstone_chk<GLOBAL, GLOBAL>(); break;
			case 0x01: hyperstone_chk<GLOBAL, LOCAL>(); break;
			case 0x02: hyperstone_chk<LOCAL, GLOBAL>(); break;
			case 0x03: hyperstone_chk<LOCAL, LOCAL>(); break;
			case 0x04: hyperstone_movd<GLOBAL, GLOBAL>(); break;
			case 0x05: hyperstone_movd<GLOBAL, LOCAL>(); break;
			case 0x06: hyperstone_movd<LOCAL, GLOBAL>(); break;
			case 0x07: hyperstone_movd<LOCAL, LOCAL>(); break;
			case 0x08: hyperstone_divsu<GLOBAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0x09: hyperstone_divsu<GLOBAL, LOCAL, IS_UNSIGNED>(); break;
			case 0x0a: hyperstone_divsu<LOCAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0x0b: hyperstone_divsu<LOCAL, LOCAL, IS_UNSIGNED>(); break;
			case 0x0c: hyperstone_divsu<GLOBAL, GLOBAL, IS_SIGNED>(); break;
			case 0x0d: hyperstone_divsu<GLOBAL, LOCAL, IS_SIGNED>(); break;
			case 0x0e: hyperstone_divsu<LOCAL, GLOBAL, IS_SIGNED>(); break;
			case 0x0f: hyperstone_divsu<LOCAL, LOCAL, IS_SIGNED>(); break;
			case 0x10: hyperstone_xm<GLOBAL, GLOBAL>(); break;
			case 0x11: hyperstone_xm<GLOBAL, LOCAL>(); break;
			case 0x12: hyperstone_xm<LOCAL, GLOBAL>(); break;
			case 0x13: hyperstone_xm<LOCAL, LOCAL>(); break;
			case 0x14: hyperstone_mask<GLOBAL, GLOBAL>(); break;
			case 0x15: hyperstone_mask<GLOBAL, LOCAL>(); break;
			case 0x16: hyperstone_mask<LOCAL, GLOBAL>(); break;
			case 0x17: hyperstone_mask<LOCAL, LOCAL>(); break;
			case 0x18: hyperstone_sum<GLOBAL, GLOBAL>(); break;
			case 0x19: hyperstone_sum<GLOBAL, LOCAL>(); break;
			case 0x1a: hyperstone_sum<LOCAL, GLOBAL>(); break;
			case 0x1b: hyperstone_sum<LOCAL, LOCAL>(); break;
			case 0x1c: hyperstone_sums<GLOBAL, GLOBAL>(); break;
			case 0x1d: hyperstone_sums<GLOBAL, LOCAL>(); break;
			case 0x1e: hyperstone_sums<LOCAL, GLOBAL>(); break;
			case 0x1f: hyperstone_sums<LOCAL, LOCAL>(); break;
			case 0x20: hyperstone_cmp<GLOBAL, GLOBAL>(); break;
			case 0x21: hyperstone_cmp<GLOBAL, LOCAL>(); break;
			case 0x22: hyperstone_cmp<LOCAL, GLOBAL>(); break;
			case 0x23: hyperstone_cmp<LOCAL, LOCAL>(); break;
			case 0x24: hyperstone_mov<GLOBAL, GLOBAL>(); break;
			case 0x25: hyperstone_mov<GLOBAL, LOCAL>(); break;
			case 0x26: hyperstone_mov<LOCAL, GLOBAL>(); break;
			case 0x27: hyperstone_mov<LOCAL, LOCAL>(); break;
			case 0x28: hyperstone_add<GLOBAL, GLOBAL>(); break;
			case 0x29: hyperstone_add<GLOBAL, LOCAL>(); break;
			case 0x2a: hyperstone_add<LOCAL, GLOBAL>(); break;
			case 0x2b: hyperstone_add<LOCAL, LOCAL>(); break;
			case 0x2c: hyperstone_adds<GLOBAL, GLOBAL>(); break;
			case 0x2d: hyperstone_adds<GLOBAL, LOCAL>(); break;
			case 0x2e: hyperstone_adds<LOCAL, GLOBAL>(); break;
			case 0x2f: hyperstone_adds<LOCAL, LOCAL>(); break;
			case 0x30: hyperstone_cmpb<GLOBAL, GLOBAL>(); break;
			case 0x31: hyperstone_cmpb<GLOBAL, LOCAL>(); break;
			case 0x32: hyperstone_cmpb<LOCAL, GLOBAL>(); break;
			case 0x33: hyperstone_cmpb<LOCAL, LOCAL>(); break;
			case 0x34: hyperstone_andn<GLOBAL, GLOBAL>(); break;
			case 0x35: hyperstone_andn<GLOBAL, LOCAL>(); break;
			case 0x36: hyperstone_andn<LOCAL, GLOBAL>(); break;
			case 0x37: hyperstone_andn<LOCAL, LOCAL>(); break;
			case 0x38: hyperstone_or<GLOBAL, GLOBAL>(); break;
			case 0x39: hyperstone_or<GLOBAL, LOCAL>(); break;
			case 0x3a: hyperstone_or<LOCAL, GLOBAL>(); break;
			case 0x3b: hyperstone_or<LOCAL, LOCAL>(); break;
			case 0x3c: hyperstone_xor<GLOBAL, GLOBAL>(); break;
			case 0x3d: hyperstone_xor<GLOBAL, LOCAL>(); break;
			case 0x3e: hyperstone_xor<LOCAL, GLOBAL>(); break;
			case 0x3f: hyperstone_xor<LOCAL, LOCAL>(); break;
			case 0x40: hyperstone_subc<GLOBAL, GLOBAL>(); break;
			case 0x41: hyperstone_subc<GLOBAL, LOCAL>(); break;
			case 0x42: hyperstone_subc<LOCAL, GLOBAL>(); break;
			case 0x43: hyperstone_subc<LOCAL, LOCAL>(); break;
			case 0x44: hyperstone_not<GLOBAL, GLOBAL>(); break;
			case 0x45: hyperstone_not<GLOBAL, LOCAL>(); break;
			case 0x46: hyperstone_not<LOCAL, GLOBAL>(); break;
			case 0x47: hyperstone_not<LOCAL, LOCAL>(); break;
			case 0x48: hyperstone_sub<GLOBAL, GLOBAL>(); break;
			case 0x49: hyperstone_sub<GLOBAL, LOCAL>(); break;
			case 0x4a: hyperstone_sub<LOCAL, GLOBAL>(); break;
			case 0x4b: hyperstone_sub<LOCAL, LOCAL>(); break;
			case 0x4c: hyperstone_subs<GLOBAL, GLOBAL>(); break;
			case 0x4d: hyperstone_subs<GLOBAL, LOCAL>(); break;
			case 0x4e: hyperstone_subs<LOCAL, GLOBAL>(); break;
			case 0x4f: hyperstone_subs<LOCAL, LOCAL>(); break;
			case 0x50: hyperstone_addc<GLOBAL, GLOBAL>(); break;
			case 0x51: hyperstone_addc<GLOBAL, LOCAL>(); break;
			case 0x52: hyperstone_addc<LOCAL, GLOBAL>(); break;
			case 0x53: hyperstone_addc<LOCAL, LOCAL>(); break;
			case 0x54: hyperstone_and<GLOBAL, GLOBAL>(); break;
			case 0x55: hyperstone_and<GLOBAL, LOCAL>(); break;
			case 0x56: hyperstone_and<LOCAL, GLOBAL>(); break;
			case 0x57: hyperstone_and<LOCAL, LOCAL>(); break;
			case 0x58: hyperstone_neg<GLOBAL, GLOBAL>(); break;
			case 0x59: hyperstone_neg<GLOBAL, LOCAL>(); break;
			case 0x5a: hyperstone_neg<LOCAL, GLOBAL>(); break;
			case 0x5b: hyperstone_neg<LOCAL, LOCAL>(); break;
			case 0x5c: hyperstone_negs<GLOBAL, GLOBAL>(); break;
			case 0x5d: hyperstone_negs<GLOBAL, LOCAL>(); break;
			case 0x5e: hyperstone_negs<LOCAL, GLOBAL>(); break;
			case 0x5f: hyperstone_negs<LOCAL, LOCAL>(); break;
			case 0x60: hyperstone_cmpi<GLOBAL, SIMM>(); break;
			case 0x61: hyperstone_cmpi<GLOBAL, LIMM>(); break;
			case 0x62: hyperstone_cmpi<LOCAL, SIMM>(); break;
			case 0x63: hyperstone_cmpi<LOCAL, LIMM>(); break;
			case 0x64: hyperstone_movi<GLOBAL, SIMM>(); break;
			case 0x65: hyperstone_movi<GLOBAL, LIMM>(); break;
			case 0x66: hyperstone_movi<LOCAL, SIMM>(); break;
			case 0x67: hyperstone_movi<LOCAL, LIMM>(); break;
			case 0x68: hyperstone_addi<GLOBAL, SIMM>(); break;
			case 0x69: hyperstone_addi<GLOBAL, LIMM>(); break;
			case 0x6a: hyperstone_addi<LOCAL, SIMM>(); break;
			case 0x6b: hyperstone_addi<LOCAL, LIMM>(); break;
			case 0x6c: hyperstone_addsi<GLOBAL, SIMM>(); break;
			case 0x6d: hyperstone_addsi<GLOBAL, LIMM>(); break;
			case 0x6e: hyperstone_addsi<LOCAL, SIMM>(); break;
			case 0x6f: hyperstone_addsi<LOCAL, LIMM>(); break;
			case 0x70: hyperstone_cmpbi<GLOBAL, SIMM>(); break;
			case 0x71: hyperstone_cmpbi<GLOBAL, LIMM>(); break;
			case 0x72: hyperstone_cmpbi<LOCAL, SIMM>(); break;
			case 0x73: hyperstone_cmpbi<LOCAL, LIMM>(); break;
			case 0x74: hyperstone_andni<GLOBAL, SIMM>(); break;
			case 0x75: hyperstone_andni<GLOBAL, LIMM>(); break;
			case 0x76: hyperstone_andni<LOCAL, SIMM>(); break;
			case 0x77: hyperstone_andni<LOCAL, LIMM>(); break;
			case 0x78: hyperstone_ori<GLOBAL, SIMM>(); break;
			case 0x79: hyperstone_ori<GLOBAL, LIMM>(); break;
			case 0x7a: hyperstone_ori<LOCAL, SIMM>(); break;
			case 0x7b: hyperstone_ori<LOCAL, LIMM>(); break;
			case 0x7c: hyperstone_xori<GLOBAL, SIMM>(); break;
			case 0x7d: hyperstone_xori<GLOBAL, LIMM>(); break;
			case 0x7e: hyperstone_xori<LOCAL, SIMM>(); break;
			case 0x7f: hyperstone_xori<LOCAL, LIMM>(); break;
			case 0x80: hyperstone_shrdi<N_LO>(); break;
			case 0x81: hyperstone_shrdi<N_HI>(); break;
			case 0x82: hyperstone_shrd(); break;
			case 0x83: hyperstone_shr(); break;
			case 0x84: hyperstone_sardi<N_LO>(); break;
			case 0x85: hyperstone_sardi<N_HI>(); break;
			case 0x86: hyperstone_sard(); break;
			case 0x87: hyperstone_sar(); break;
			case 0x88: hyperstone_shldi<N_LO>(); break;
			case 0x89: hyperstone_shldi<N_HI>(); break;
			case 0x8a: hyperstone_shld(); break;
			case 0x8b: hyperstone_shl(); break;
			case 0x8c: hyperstone_reserved(); break;
			case 0x8d: hyperstone_reserved(); break;
			case 0x8e: hyperstone_testlz(); break;
			case 0x8f: hyperstone_rol(); break;
			case 0x90: hyperstone_ldxx1<GLOBAL, GLOBAL>(); break;
			case 0x91: hyperstone_ldxx1<GLOBAL, LOCAL>(); break;
			case 0x92: hyperstone_ldxx1<LOCAL, GLOBAL>(); break;
			case 0x93: hyperstone_ldxx1<LOCAL, LOCAL>(); break;
			case 0x94: hyperstone_ldxx2<GLOBAL, GLOBAL>(); break;
			case 0x95: hyperstone_ldxx2<GLOBAL, LOCAL>(); break;
			case 0x96: hyperstone_ldxx2<LOCAL, GLOBAL>(); break;
			case 0x97: hyperstone_ldxx2<LOCAL, LOCAL>(); break;
			case 0x98: hyperstone_stxx1<GLOBAL, GLOBAL>(); break;
			case 0x99: hyperstone_stxx1<GLOBAL, LOCAL>(); break;
			case 0x9a: hyperstone_stxx1<LOCAL, GLOBAL>(); break;
			case 0x9b: hyperstone_stxx1<LOCAL, LOCAL>(); break;
			case 0x9c: hyperstone_stxx2<GLOBAL, GLOBAL>(); break;
			case 0x9d: hyperstone_stxx2<GLOBAL, LOCAL>(); break;
			case 0x9e: hyperstone_stxx2<LOCAL, GLOBAL>(); break;
			case 0x9f: hyperstone_stxx2<LOCAL, LOCAL>(); break;
			case 0xa0: hyperstone_shri<N_LO, GLOBAL>(); break;
			case 0xa1: hyperstone_shri<N_HI, GLOBAL>(); break;
			case 0xa2: hyperstone_shri<N_LO, LOCAL>(); break;
			case 0xa3: hyperstone_shri<N_HI, LOCAL>(); break;
			case 0xa4: hyperstone_sari<N_LO, GLOBAL>(); break;
			case 0xa5: hyperstone_sari<N_HI, GLOBAL>(); break;
			case 0xa6: hyperstone_sari<N_LO, LOCAL>(); break;
			case 0xa7: hyperstone_sari<N_HI, LOCAL>(); break;
			case 0xa8: hyperstone_shli<N_LO, GLOBAL>(); break;
			case 0xa9: hyperstone_shli<N_HI, GLOBAL>(); break;
			case 0xaa: hyperstone_shli<N_LO, LOCAL>(); break;
			case 0xab: hyperstone_shli<N_HI, LOCAL>(); break;
			case 0xac: hyperstone_reserved(); break;
			case 0xad: hyperstone_reserved(); break;
			case 0xae: hyperstone_reserved(); break;
			case 0xaf: hyperstone_reserved(); break;
			case 0xb0: hyperstone_mulsu<GLOBAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0xb1: hyperstone_mulsu<GLOBAL, LOCAL, IS_UNSIGNED>(); break;
			case 0xb2: hyperstone_mulsu<LOCAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0xb3: hyperstone_mulsu<LOCAL, LOCAL, IS_UNSIGNED>(); break;
			case 0xb4: hyperstone_mulsu<GLOBAL, GLOBAL, IS_SIGNED>(); break;
			case 0xb5: hyperstone_mulsu<GLOBAL, LOCAL, IS_SIGNED>(); break;
			case 0xb6: hyperstone_mulsu<LOCAL, GLOBAL, IS_SIGNED>(); break;
			case 0xb7: hyperstone_mulsu<LOCAL, LOCAL, IS_SIGNED>(); break;
			case 0xb8: hyperstone_set<N_LO, GLOBAL>(); break;
			case 0xb9: hyperstone_set<N_HI, GLOBAL>(); break;
			case 0xba: hyperstone_set<N_LO, LOCAL>(); break;
			case 0xbb: hyperstone_set<N_HI, LOCAL>(); break;
			case 0xbc: hyperstone_mul<GLOBAL, GLOBAL>(); break;
			case 0xbd: hyperstone_mul<GLOBAL, LOCAL>(); break;
			case 0xbe: hyperstone_mul<LOCAL, GLOBAL>(); break;
			case 0xbf: hyperstone_mul<LOCAL, LOCAL>(); break;
			case 0xc0: execute_software(); break; // fadd
			case 0xc1: execute_software(); break; // faddd
			case 0xc2: execute_software(); break; // fsub
			case 0xc3: execute_software(); break; // fsubd
			case 0xc4: execute_software(); break; // fmul
			case 0xc5: execute_software(); break; // fmuld
			case 0xc6: execute_software(); break; // fdiv
			case 0xc7: execute_software(); break; // fdivd
			case 0xc8: execute_software(); break; // fcmp
			case 0xc9: execute_software(); break; // fcmpd
			case 0xca: execute_software(); break; // fcmpu
			case 0xcb: execute_software(); break; // fcmpud
			case 0xcc: execute_software(); break; // fcvt
			case 0xcd: execute_software(); break; // fcvtd
			case 0xce: hyperstone_extend(); break;
			case 0xcf: hyperstone_do(); break;
			case 0xd0: hyperstone_ldwr<GLOBAL>(); break;
			case 0xd1: hyperstone_ldwr<LOCAL>(); break;
			case 0xd2: hyperstone_lddr<GLOBAL>(); break;
			case 0xd3: hyperstone_lddr<LOCAL>(); break;
			case 0xd4: hyperstone_ldwp<GLOBAL>(); break;
			case 0xd5: hyperstone_ldwp<LOCAL>(); break;
			case 0xd6: hyperstone_lddp<GLOBAL>(); break;
			case 0xd7: hyperstone_lddp<LOCAL>(); break;
			case 0xd8: hyperstone_stwr<GLOBAL>(); break;
			case 0xd9: hyperstone_stwr<LOCAL>(); break;
			case 0xda: hyperstone_stdr<GLOBAL>(); break;
			case 0xdb: hyperstone_stdr<LOCAL>(); break;
			case 0xdc: hyperstone_stwp<GLOBAL>(); break;
			case 0xdd: hyperstone_stwp<LOCAL>(); break;
			case 0xde: hyperstone_stdp<GLOBAL>(); break;
			case 0xdf: hyperstone_stdp<LOCAL>(); break;
			case 0xe0: hyperstone_db<COND_V,  IS_SET>(); break;
			case 0xe1: hyperstone_db<COND_V,  IS_CLEAR>(); break;
			case 0xe2: hyperstone_db<COND_Z,  IS_SET>(); break;
			case 0xe3: hyperstone_db<COND_Z,  IS_CLEAR>(); break;
			case 0xe4: hyperstone_db<COND_C,  IS_SET>(); break;
			case 0xe5: hyperstone_db<COND_C,  IS_CLEAR>(); break;
			case 0xe6: hyperstone_db<COND_CZ, IS_SET>(); break;
			case 0xe7: hyperstone_db<COND_CZ, IS_CLEAR>(); break;
			case 0xe8: hyperstone_db<COND_N,  IS_SET>(); break;
			case 0xe9: hyperstone_db<COND_N,  IS_CLEAR>(); break;
			case 0xea: hyperstone_db<COND_NZ, IS_SET>(); break;
			case 0xeb: hyperstone_db<COND_NZ, IS_CLEAR>(); break;
			case 0xec: hyperstone_dbr(); break;
			case 0xed: hyperstone_frame(); break;
			case 0xee: hyperstone_call<GLOBAL>(); break;
			case 0xef: hyperstone_call<LOCAL>(); break;
			case 0xf0: hyperstone_b<COND_V,  IS_SET>(); break;
			case 0xf1: hyperstone_b<COND_V,  IS_CLEAR>(); break;
			case 0xf2: hyperstone_b<COND_Z,  IS_SET>(); break;
			case 0xf3: hyperstone_b<COND_Z,  IS_CLEAR>(); break;
			case 0xf4: hyperstone_b<COND_C,  IS_SET>(); break;
			case 0xf5: hyperstone_b<COND_C,  IS_CLEAR>(); break;
			case 0xf6: hyperstone_b<COND_CZ, IS_SET>(); break;
			case 0xf7: hyperstone_b<COND_CZ, IS_CLEAR>(); break;
			case 0xf8: hyperstone_b<COND_N,  IS_SET>(); break;
			case 0xf9: hyperstone_b<COND_N,  IS_CLEAR>(); break;
			case 0xfa: hyperstone_b<COND_NZ, IS_SET>(); break;
			case 0xfb: hyperstone_b<COND_NZ, IS_CLEAR>(); break;
			case 0xfc: hyperstone_br(); break;
			case 0xfd: hyperstone_trap(); break;
			case 0xfe: hyperstone_trap(); break;
			case 0xff: hyperstone_trap(); break;
		}

		if (((m_op & 0xfef0) != 0x0400) || !(m_op & 0x010e))
		{
			// anything other than RET updates ILC and sets P
			SET_ILC(m_instruction_length);
			SET_P(1);
		}

		if (GET_T && GET_P && !m_core->delay_slot) /* Not in a Delayed Branch instructions */
		{
			m_core->delay_slot_taken = 0;
			execute_exception(TRAPNO_TRACE_EXCEPTION);
		}
	}
}

DEFINE_DEVICE_TYPE(E116,       e116_device,       "e116",       "hyperstone E1-16")
DEFINE_DEVICE_TYPE(E116X,      e116x_device,      "e116x",      "hyperstone E1-16X")
DEFINE_DEVICE_TYPE(E116XS,     e116xs_device,     "e116xs",     "hyperstone E1-16XS")
DEFINE_DEVICE_TYPE(E116XSR,    e116xsr_device,    "e116xsr",    "hyperstone E1-16XSR")
DEFINE_DEVICE_TYPE(E132,       e132_device,       "e132",       "hyperstone E1-32")
DEFINE_DEVICE_TYPE(E132X,      e132x_device,      "e132x",      "hyperstone E1-32X")
DEFINE_DEVICE_TYPE(E132XS,     e132xs_device,     "e132xs",     "hyperstone E1-32XS")
DEFINE_DEVICE_TYPE(E132XSR,    e132xsr_device,    "e132xsr",    "hyperstone E1-32XSR")
DEFINE_DEVICE_TYPE(GMS30C2116, gms30c2116_device, "gms30c2116", "Hynix GMS30C2116")
DEFINE_DEVICE_TYPE(GMS30C2132, gms30c2132_device, "gms30c2132", "Hynix GMS30C2132")
DEFINE_DEVICE_TYPE(GMS30C2216, gms30c2216_device, "gms30c2216", "Hynix GMS30C2216")
DEFINE_DEVICE_TYPE(GMS30C2232, gms30c2232_device, "gms30c2232", "Hynix GMS30C2232")
