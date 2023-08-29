// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics CPU Control Gate Array (CTL1).
 *
 * This gate array theoretically supports up to 16x4 slots of 30-pin 1M or 4M
 * SIMMS with parity. SIMMs must be installed in banks of 4, and all installed
 * SIMMs must be the same capacity. The Personal IRIS 4D/20 and 4D/25 use this
 * device with 4 banks of SIMM slots.
 *
 * Sources:
 *  - VME-Eclipse CPU (VIP10) Specification, Silicon Graphics, Inc.
 *
 * TODO:
 *  - graphics dma
 *  - vme bus
 *  - irq5, fpu present
 */

#include "emu.h"
#include "ctl1.h"

#define LOG_PARITY (1U << 1)

//#define VERBOSE (LOG_PARITY)

#include "logmacro.h"

enum cpucfg_mask : u16
{
	CPUCFG_SERDATA = 0x0100, // serial memory data out
	CPUCFG_SIN     = 0x0200, // system init (reset)
	CPUCFG_RPAR    = 0x0400, // enable parity checking
	CPUCFG_SLA     = 0x0800, // enable slave accesses
	CPUCFG_ARB     = 0x1000, // enable vme arbiter
	CPUCFG_BAD     = 0x2000, // write bad parity
	CPUCFG_DOG     = 0x4000, // enable watchdog timer
	CPUCFG_FPER    = 0x8000, // fast peripheral cycle
};

enum memcfg_mask : u8
{
	MEMCFG_MEMSIZE  = 0x0f, // (n+1)/16 memory populated
	MEMCFG_4MRAM    = 0x10, // 4M DRAMs
	MEMCFG_TIMERDIS = 0x20, // disable timer (active low)
	MEMCFG_FMEM     = 0x40, // reduce cas pulse on reads
	MEMCFG_REFDIS   = 0x80, // disable memory refresh (active low)
};

enum parerr_mask : u8
{
	PARERR_GDMA  = 0x01,
	PARERR_DMA   = 0x02,
	PARERR_CPU   = 0x04,
	PARERR_VME   = 0x08,
	PARERR_B3    = 0x10,
	PARERR_B2    = 0x20,
	PARERR_B1    = 0x40,
	PARERR_B0    = 0x80,
	PARERR_BYTE  = 0xf0,
};

DEFINE_DEVICE_TYPE(SGI_CTL1, sgi_ctl1_device, "sgi_ctl1", "SGI CTL1")

sgi_ctl1_device::sgi_ctl1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_CTL1, tag, owner, clock)
	, m_bus(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_simms(*this, "SIMMS")
	, m_serdin(*this, 0)
	, m_serdout(*this)
	, m_cpuberr(*this)
{
}

void sgi_ctl1_device::device_start()
{
	m_parity_bad = 0;
}

void sgi_ctl1_device::device_reset()
{
	u16 const simms = m_simms->read();

	for (unsigned bank = 0; bank < 16; bank++)
	{
		unsigned const simm = BIT(simms, bank * 2, 2);

		if (simm)
			m_ram[bank] = std::make_unique<u8[]>(1U << (simm + 21));
		else
			m_ram[bank].reset();
	}

	m_cpucfg = 0;

	// unmap ram
	if (m_memcfg.has_value())
	{
		m_bus->unmap_readwrite(0x0000'0000U, 0x0fff'ffffU);
		m_memcfg.reset();
	}

	m_erradr = 0;
	m_parerr = 0;
	m_refadr = 0;

	m_refresh_timer = machine().time();
}

u8 sgi_ctl1_device::clrerr_r(offs_t offset)
{
	m_parerr &= ~(PARERR_BYTE | (1U << offset));

	return 0;
}

void sgi_ctl1_device::clrerr_w(offs_t offset, u8 data)
{
	m_parerr &= ~(PARERR_BYTE | (1U << offset));
}

void sgi_ctl1_device::cpucfg_w(u16 data)
{
	LOG("cpucfg_w 0x%04x\n", data);

	m_serdout(BIT(data, 8));

	// reset system
	if (BIT(data, 9))
		machine().schedule_soft_reset();

	if ((m_cpucfg ^ data) & CPUCFG_RPAR)
		LOGMASKED(LOG_PARITY, "parity checking %d\n", BIT(data, 10));

	//BIT(data, 11); // enable slave accesses
	//BIT(data, 12); // enable vme arbiter

	if ((m_cpucfg ^ data) & CPUCFG_BAD)
	{
		LOGMASKED(LOG_PARITY, "write bad parity %d\n", BIT(data, 13));

		if ((data & CPUCFG_BAD) && !m_parity && m_memcfg.has_value())
		{
			unsigned const ram_size = ((m_memcfg.value() & MEMCFG_MEMSIZE) + 1) * ((m_memcfg.value() & MEMCFG_4MRAM) ? 16 : 4);

			LOGMASKED(LOG_PARITY, "bad parity activated %dM\n", ram_size);

			m_parity = std::make_unique<u8[]>(ram_size << (20 - 3));
			m_parity_mph = m_bus->install_readwrite_tap(0, (ram_size << 20) - 1, "parity",
				std::bind(&sgi_ctl1_device::parity_r, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
				std::bind(&sgi_ctl1_device::parity_w, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}
	}

	//BIT(data, 14); // enable watchdog timer
	//BIT(data, 15); // fast peripheral cycle

	m_cpucfg = data;
}

void sgi_ctl1_device::memcfg_w(u8 data)
{
	LOG("memcfg_w 0x%02x\n", data);

	if (!m_memcfg.has_value() || ((m_memcfg.value() ^ data) & (MEMCFG_4MRAM | MEMCFG_MEMSIZE)))
	{
		// remove existing mapping
		if (m_memcfg.has_value())
			m_bus->unmap_readwrite(0x0000'0000U, 0x0fff'ffffU);

		unsigned conf_size = (data & MEMCFG_4MRAM) ? 0x100'0000 : 0x40'0000;
		u16 const simms = m_simms->read();

		for (unsigned bank = 0; bank <= (data & MEMCFG_MEMSIZE); bank++)
		{
			if (m_ram[bank])
			{
				unsigned inst_size = 1U << (BIT(simms, bank * 2, 2) + 21);
				unsigned const size = std::min(inst_size, conf_size);

				m_bus->install_ram(bank * conf_size, bank * conf_size + size - 1, (conf_size - 1) ^ (size - 1), m_ram[bank].get());
			}
		}
	}

	m_memcfg = data;
}

u32 sgi_ctl1_device::refadr_r()
{
	if (m_memcfg.value() & MEMCFG_TIMERDIS)
	{
		// refresh cycle is generated every 64μs
		u64 const refreshes = (machine().time() - m_refresh_timer).as_ticks(15.625_kHz_XTAL);

		// each refresh cycle generates 4 sequential accesses
		// TODO: should the other factor be 1024 for 1M DRAM?
		return u32(m_refadr + refreshes * 4096 * 4);
	}
	else
		return m_refadr;
}

void sgi_ctl1_device::refadr_w(u32 data)
{
	m_refadr = data;
	m_refresh_timer = machine().time();
}

void sgi_ctl1_device::parity_r(offs_t offset, u32 &data, u32 mem_mask)
{
	if (m_cpucfg & CPUCFG_RPAR)
	{
		bool error = false;
		for (unsigned byte = 0; byte < 4; byte++)
		{
			if (BIT(mem_mask, 24 - byte * 8, 8) && BIT(m_parity[offset >> 3], BIT(offset, 2) * 4 + byte))
			{
				m_parerr |= (PARERR_B0 >> byte) | PARERR_CPU;
				error = true;

				LOGMASKED(LOG_PARITY, "bad parity err 0x%08x byte %d count %d\n", offset, byte, m_parity_bad);
			}
		}

		if (error)
		{
			m_erradr = offset;
			m_cpuberr(1);
		}
	}
}

void sgi_ctl1_device::parity_w(offs_t offset, u32 &data, u32 mem_mask)
{
	if (m_cpucfg & CPUCFG_BAD)
	{
		for (unsigned byte = 0; byte < 4; byte++)
		{
			if (BIT(mem_mask, 24 - byte * 8, 8) && !BIT(m_parity[offset >> 3], BIT(offset, 2) * 4 + byte))
			{
				m_parity[offset >> 3] |= 1U << (BIT(offset, 2) * 4 + byte);
				m_parity_bad++;

				LOGMASKED(LOG_PARITY, "bad parity set 0x%08x byte %d count %d\n", offset, byte, m_parity_bad);
			}
		}
	}
	else
	{
		for (unsigned byte = 0; byte < 4; byte++)
		{
			if (BIT(mem_mask, 24 - byte * 8, 8) && BIT(m_parity[offset >> 3], BIT(offset, 2) * 4 + byte))
			{
				m_parity[offset >> 3] &= ~(1U << (BIT(offset, 2) * 4 + byte));
				m_parity_bad--;

				LOGMASKED(LOG_PARITY, "bad parity clr 0x%08x byte %d count %d\n", offset, byte, m_parity_bad);
			}
		}

		if (m_parity_bad == 0)
		{
			LOGMASKED(LOG_PARITY, "bad parity deactivated\n");

			m_parity_mph.remove();
			m_parity.reset();
		}
	}
}

static INPUT_PORTS_START(ctl1)
	PORT_START("VALID")
	PORT_CONFNAME(0x000f, 0x000f, "Valid Banks")

	PORT_START("SIMMS")
	PORT_CONFNAME(0x00000003, 0x00000001, "RAM bank A") PORT_CONDITION("VALID", 0x0001, EQUALS, 0x0001)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00000001, "4x1M")
	PORT_CONFSETTING(0x00000003, "4x4M")

	PORT_CONFNAME(0x0000000c, 0x00000004, "RAM bank B") PORT_CONDITION("VALID", 0x0002, EQUALS, 0x0002)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00000004, "4x1M")
	PORT_CONFSETTING(0x0000000c, "4x4M")

	PORT_CONFNAME(0x00000030, 0x00000010, "RAM bank C") PORT_CONDITION("VALID", 0x0004, EQUALS, 0x0004)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00000010, "4x1M")
	PORT_CONFSETTING(0x00000030, "4x4M")

	PORT_CONFNAME(0x000000c0, 0x00000040, "RAM bank D") PORT_CONDITION("VALID", 0x0008, EQUALS, 0x0008)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00000040, "4x1M")
	PORT_CONFSETTING(0x000000c0, "4x4M")


	PORT_CONFNAME(0x00000300, 0x00000000, "RAM bank E") PORT_CONDITION("VALID", 0x0010, EQUALS, 0x0010)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00000100, "4x1M")
	PORT_CONFSETTING(0x00000300, "4x4M")

	PORT_CONFNAME(0x00000c00, 0x00000000, "RAM bank F") PORT_CONDITION("VALID", 0x0020, EQUALS, 0x0020)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00000400, "4x1M")
	PORT_CONFSETTING(0x00000c00, "4x4M")

	PORT_CONFNAME(0x00003000, 0x00000000, "RAM bank G") PORT_CONDITION("VALID", 0x0040, EQUALS, 0x0040)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00001000, "4x1M")
	PORT_CONFSETTING(0x00003000, "4x4M")

	PORT_CONFNAME(0x0000c000, 0x00000000, "RAM bank H") PORT_CONDITION("VALID", 0x0080, EQUALS, 0x0080)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00004000, "4x1M")
	PORT_CONFSETTING(0x0000c000, "4x4M")


	PORT_CONFNAME(0x00030000, 0x00000000, "RAM bank I") PORT_CONDITION("VALID", 0x0100, EQUALS, 0x0100)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00010000, "4x1M")
	PORT_CONFSETTING(0x00030000, "4x4M")

	PORT_CONFNAME(0x000c0000, 0x00000000, "RAM bank J") PORT_CONDITION("VALID", 0x0200, EQUALS, 0x0200)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00040000, "4x1M")
	PORT_CONFSETTING(0x000c0000, "4x4M")

	PORT_CONFNAME(0x00300000, 0x00000000, "RAM bank K") PORT_CONDITION("VALID", 0x0400, EQUALS, 0x0400)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00100000, "4x1M")
	PORT_CONFSETTING(0x00300000, "4x4M")

	PORT_CONFNAME(0x00c00000, 0x00000000, "RAM bank L") PORT_CONDITION("VALID", 0x0800, EQUALS, 0x0800)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x00400000, "4x1M")
	PORT_CONFSETTING(0x00c00000, "4x4M")


	PORT_CONFNAME(0x03000000, 0x00000000, "RAM bank M") PORT_CONDITION("VALID", 0x1000, EQUALS, 0x1000)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x01000000, "4x1M")
	PORT_CONFSETTING(0x03000000, "4x4M")

	PORT_CONFNAME(0x0c000000, 0x00000000, "RAM bank N") PORT_CONDITION("VALID", 0x2000, EQUALS, 0x2000)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x04000000, "4x1M")
	PORT_CONFSETTING(0x0c000000, "4x4M")

	PORT_CONFNAME(0x30000000, 0x00000000, "RAM bank O") PORT_CONDITION("VALID", 0x4000, EQUALS, 0x4000)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x10000000, "4x1M")
	PORT_CONFSETTING(0x30000000, "4x4M")

	PORT_CONFNAME(0xc0000000, 0x00000000, "RAM bank P") PORT_CONDITION("VALID", 0x8000, EQUALS, 0x8000)
	PORT_CONFSETTING(0x00000000, "Empty")
	PORT_CONFSETTING(0x40000000, "4x1M")
	PORT_CONFSETTING(0xc0000000, "4x4M")

INPUT_PORTS_END

ioport_constructor sgi_ctl1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ctl1);
}
