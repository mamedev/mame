// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#include "emu.h"
#include "ds5002fp.h"
#include "mcs51dasm.h"

/*****************************************************************************
 *
 * DS5002FP emulator by Manuel Abadia
 *
 * The main features of the DS5002FP are:
 * - 100% code-compatible with 8051
 * - Directly addresses 64kB program/64kB data memory
 * - Nonvolatile memory control circuitry
 * - 10-year data retention in the absence of power
 * - In-system reprogramming via serial port
 * - Dedicated memory bus, preserving four 8-bit ports for general purpose I/O
 * - Power-fail reset
 * - Early warning power-fail interrupt
 * - Watchdog timer
 * - Accesses up to 128kB on the bytewide bus
 * - Decodes memory for 32kB x 8 or 128kB x 8 SRAMs
 * - Four additional decoded peripheral-chip enables
 * - CRC hardware for checking memory validity
 * - Optionally emulates an 8042-style slave interface
 * - Memory encryption using an 80-bit encryption key
 * - Automatic random generation of encryption keys
 * - Self-destruct input for tamper protection
 * - Optional top-coating prevents microprobe
 *
 * TODO:
 * - Peripherals and Reprogrammable Peripheral Controller
 * - CRC-16
 * - Watchdog timer
 *
 *****************************************************************************/

// ds5002fp: FEATURE:DS5002FP,CMOS  ds5002fp

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DS5002FP, ds5002fp_device, "ds5002fp", "Dallas DS5002FP")

/* program width field is set to 0 because technically the SRAM isn't internal */
ds5002fp_device::ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, DS5002FP, tag, owner, clock, 0, 7)
	, device_nvram_interface(mconfig, *this)
	, m_region(*this, "internal")
{
	m_has_pd = true;
	m_data_config.m_addr_width = 18;
}

void ds5002fp_device::device_start()
{
	mcs51_cpu_device::device_start();

	save_item(NAME(m_previous_ta));
	save_item(NAME(m_ta_window));
	save_item(NAME(m_range));
	save_item(NAME(m_rnr_delay));
}

void ds5002fp_device::device_reset()
{
	mcs51_cpu_device::device_reset();

	// set initial values (some of them are set using the bootstrap loader)
	m_pcon = 0;
	m_rps = 0;
	m_rnr = 0;
	m_crc = 0;
	m_ta = 0;

	// set internal CPU state
	m_previous_ta = 0;
	m_ta_window = 0;
	m_range = (BIT(m_mcon, MCON_RG1) << 1) | BIT(m_rpctl, RPCTL_RG0);
	m_rnr_delay = 160;
}


/*
    The DS5002FP has 2 16 bits data address buses (the byte-wide bus and the expanded bus). The exact memory position accessed depends on the
    partition mode, the memory range and the expanded bus select. The partition mode and the expanded bus select can be changed at any time.

    In order to simplify memory mapping to the data address bus, the following address map is assumed for partitioned mode:

    PES = 0:
    0x00000-0x0ffff -> data memory on the expanded bus
    0x10000-0x1ffff -> data memory on the byte-wide bus
    PES = 1:
    0x20000-0x2ffff -> memory-mapped peripherals on the byte-wide bus

    For non-partitioned mode the following memory map is assumed:

    PES = 0:
    0x00000-0x0ffff -> data memory (the bus used to access it does not matter)
    PES = 1:
    0x20000-0x2ffff -> memory-mapped peripherals on the byte-wide bus
*/

offs_t ds5002fp_device::external_ram_iaddr(offs_t offset, offs_t mem_mask)
{
	/* Memory Range (RG1 and RG0 @ m_mcon and m_rpctl registers) */
	static const u16 ds5002fp_ranges[4] = { 0x1fff, 0x3fff, 0x7fff, 0xffff };
	/* Memory Partition Table (RG1 & RG0 @ m_mcon & m_rpctl registers) */
	static const u32 ds5002fp_partitions[16] =
	{
		0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000,  0x7000,
		0x8000, 0x9000, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000, 0x10000
	};

	/* adjust offset based on the bus */
	if (BIT(m_mcon, MCON_PES))
		offset += 0x20000;
	else if (!BIT(m_mcon, MCON_PM))
	{
		if (!BIT(m_rpctl, RPCTL_EXBS))
		{
			if ((offset >= ds5002fp_partitions[BIT(m_mcon, MCON_PA)]) && (offset <= ds5002fp_ranges[m_range]))
				offset += 0x10000;
		}
	}
	return offset;
}

void ds5002fp_device::irqs_complete_and_mask(u8 &ints, u8 int_mask)
{
	ints |= BIT(m_pcon, PCON_PFW) << 5;
	m_irq_prio[6] = 3; /* force highest priority */
	/* mask out interrupts not enabled */
	ints &= ((int_mask & 0x1f) | ((BIT(m_pcon, PCON_EPFW)) << 5));
}

void ds5002fp_device::ds_protected(u8 &val, u8 data, u8 ta_mask, u8 mask)
{
	u8 is_timed_access;

	is_timed_access = (m_ta_window > 0) && (m_ta == 0x55);
	if (is_timed_access)
	{
		ta_mask = 0xff;
	}
	data = (val & (~ta_mask)) | (data & ta_mask);
	val = (val & (~mask)) | (data & mask);
}

void ds5002fp_device::sfr_map(address_map &map)
{
	mcs51_cpu_device::sfr_map(map);
	map(0x87, 0x87).rw(FUNC(ds5002fp_device::pcon_ds_r), FUNC(ds5002fp_device::pcon_ds_w));
	map(0xb8, 0xb8).rw(FUNC(ds5002fp_device::ip_r     ), FUNC(ds5002fp_device::ip_ds_w  ));
	map(0xc1, 0xc1).rw(FUNC(ds5002fp_device::crcr_r   ), FUNC(ds5002fp_device::crcr_w   ));
	map(0xc2, 0xc3).rw(FUNC(ds5002fp_device::crc_r    ), FUNC(ds5002fp_device::crc_w    ));
	map(0xc6, 0xc6).rw(FUNC(ds5002fp_device::mcon_r   ), FUNC(ds5002fp_device::mcon_w   ));
	map(0xc7, 0xc7).rw(FUNC(ds5002fp_device::ta_r     ), FUNC(ds5002fp_device::ta_w     ));
	map(0xcf, 0xcf).rw(FUNC(ds5002fp_device::rnr_r    ), FUNC(ds5002fp_device::rnr_w    ));
	map(0xd8, 0xd8).rw(FUNC(ds5002fp_device::rpctl_r  ), FUNC(ds5002fp_device::rpctl_w  ));
	map(0xda, 0xda).rw(FUNC(ds5002fp_device::rps_r    ), FUNC(ds5002fp_device::rps_w    ));
}

void ds5002fp_device::handle_ta_window()
{
	// decrement the timed access window
	m_ta_window = (m_ta_window ? (m_ta_window - 1) : 0x00);

	if (m_rnr_delay > 0)
		m_rnr_delay -= m_inst_cycles;
}

bool ds5002fp_device::manage_idle_on_interrupt(u8 ints)
{
	/* any interrupt terminates idle mode */
	set_idl(0);
	return BIT(m_pcon, PCON_PD);
}


void ds5002fp_device::handle_irq(int irqline, int state, u32 new_state, u32 tr_state)
{
	switch (irqline)
	{
		/* Power Fail Interrupt */
		case DS5002FP_PFI_LINE:
			/* Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo! */
			if (BIT(tr_state, MCS51_INT1_LINE))
				set_pfw(1);
			break;
	}
}

u8 ds5002fp_device::pcon_ds_r()
{
	set_pfw(0);
	return m_pcon;
}

void ds5002fp_device::pcon_ds_w(u8 data)
{
	ds_protected(m_pcon, data, 0xb9, 0xff);
}

void ds5002fp_device::ip_ds_w(u8 data)
{
	ds_protected(m_ip, data, 0x7f, 0xff);
}

u8 ds5002fp_device::crcr_r()
{
	logerror("crcr read (%s)\n", machine().describe_context());
	return m_crcr;
}

void ds5002fp_device::crcr_w(u8 data)
{
	ds_protected(m_crcr, data, 0xff, 0x0f);
	logerror("crcr write %02x -> %02x (%s)\n", data, m_crcr, machine().describe_context());
}


u8 ds5002fp_device::crc_r(offs_t offset)
{
	logerror("crc read (%s)\n", machine().describe_context());
	return m_crc >> (offset * 8);
}

void ds5002fp_device::crc_w(offs_t offset, u8 data)
{
	m_crc = (m_crc & ~(0xff << (offset*8))) | (data << (offset*8));
}

u8 ds5002fp_device::mcon_r()
{
	logerror("mcon read (%s)\n", machine().describe_context());
	return m_mcon;
}

void ds5002fp_device::mcon_w(u8 data)
{
	ds_protected(m_mcon, data, 0x0f, 0xf7);
	logerror("mcon write %02x -> %02x (%s)\n", data, m_mcon, machine().describe_context());
}

u8 ds5002fp_device::ta_r()
{
	logerror("ta read (%s)\n", machine().describe_context());
	return m_ta;
}

void ds5002fp_device::ta_w(u8 data)
{
	m_previous_ta = m_ta;
	/*  init the time window after having wrote 0xaa */
	if ((data == 0xaa) && (m_ta_window == 0))
	{
		m_ta_window = 6; /* 4*12 + 2*12 */
		LOG("ta window initiated at 0x%04x\n", m_pc);
	}
	m_ta = data;
}

u8 ds5002fp_device::rnr_r()
{
	logerror("rnr read (%s)\n", machine().describe_context());
	return handle_rnr();
}

void ds5002fp_device::rnr_w(u8 data)
{
	m_rnr = data;
	logerror("rnr write %02x (%s)\n", m_rnr, machine().describe_context());
}

u8 ds5002fp_device::rpctl_r()
{
	logerror("rpctl read (%s)\n", machine().describe_context());
	return m_rnr_delay <= 0 ? 0x80 : 0x00;
}

void ds5002fp_device::rpctl_w(u8 data)
{
	ds_protected(m_rpctl, data, 0xef, 0xfe);
	logerror("rpctl write %02x -> %02x (%s)\n", data, m_rpctl, machine().describe_context());
}


u8 ds5002fp_device::rps_r()
{
	logerror("rps read (%s)\n", machine().describe_context());
	return m_rps;
}

void ds5002fp_device::rps_w(u8 data)
{
	m_rps = data;
	logerror("rps write %02x (%s)\n", m_rps, machine().describe_context());
}

u8 ds5002fp_device::handle_rnr()
{
	if (m_rnr_delay <= 0)
	{
		m_rnr_delay = 160; // delay before another random number can be read
		return machine().rand();
	}
	else
		return 0x00;
}


/*
Documentation states that having the battery connected "maintains the internal internal_ram RAM" and "certain SFRs"
(although it isn't clear exactly which SFRs except for those explicitly mentioned)
*/

void ds5002fp_device::nvram_default()
{
	memset(m_internal_ram, 0, 0x80);

	m_mcon = 0;
	m_rpctl = 0;
	m_crcr = 0;

	int expected_bytes = 0x80 + 0x80;

	if (!m_region.found())
	{
		logerror("ds5002fp_device region not found\n");
	}
	else if (m_region->bytes() != expected_bytes)
	{
		logerror("ds5002fp_device region length 0x%x expected 0x%x\n", m_region->bytes(), expected_bytes);
	}
	else
	{
		const u8 *region = m_region->base();

		memcpy(m_internal_ram, region, 0x80);;
		m_mcon = region[0xc6];
		m_rpctl = region[0xd8];
		m_crcr = region[0xc1];
		/* does anything else need storing? any registers that aren't in sfr ram?
		   It isn't clear if the various initial MCON registers etc. are just stored in sfr ram
		   or if the DS5002FP stores them elsewhere and the bootstrap copies them */
	}
}

bool ds5002fp_device::nvram_read(util::read_stream &file)
{
	std::error_condition err;
	size_t actual;
	std::tie(err, actual) = read(file, m_internal_ram, 0x80);
	if (err || (actual != 0x80))
		return false;
	std::tie(err, actual) = read(file, &m_mcon, 1);
	if (err || (actual != 1))
		return false;
	std::tie(err, actual) = read(file, &m_rpctl, 1);
	if (err || (actual != 1))
		return false;
	std::tie(err, actual) = read(file, &m_crcr, 1);
	if (err || (actual != 1))
		return false;
	return true;
}

bool ds5002fp_device::nvram_write(util::write_stream &file)
{
	std::error_condition err;
	size_t actual;
	std::tie(err, actual) = write(file, m_internal_ram, 0x80);
	if (err || (actual != 0x80))
		return false;
	std::tie(err, actual) = write(file, &m_mcon, 1);
	if (err || (actual != 1))
		return false;
	std::tie(err, actual) = write(file, &m_rpctl, 1);
	if (err || (actual != 1))
		return false;
	std::tie(err, actual) = write(file, &m_crcr, 1);
	if (err || (actual != 1))
		return false;
	return true;
}

std::unique_ptr<util::disasm_interface> ds5002fp_device::create_disassembler()
{
	return std::make_unique<ds5002fp_disassembler>();
}
