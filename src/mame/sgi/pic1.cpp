// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Processor Interface Controller (PIC1).
 *
 * TODO:
 *  - parity
 *  - graphics dma
 *  - vme bus
 *
 */
#include "emu.h"
#include "pic1.h"

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_PIC1, sgi_pic1_device, "sgi_pic1", "SGI PIC1")

enum config_mask : u32
{
	CONFIG_RFE      = 0x0001, // refresh enable
	CONFIG_BIGEND   = 0x0002, // endianness
	CONFIG_DBE      = 0x0004, // enable data block refill
	CONFIG_IBE      = 0x0008, // enable instr block refill
	CONFIG_GDE      = 0x0010, // graphics dma complete int enable
	CONFIG_GSE      = 0x0020, // graphics dma sync enable
	CONFIG_FREF     = 0x0040, // fast refresh (giobus >= 33 mhz)
	CONFIG_DVMEBE   = 0x0080, // disable vme bus error
	CONFIG_OLDFREF  = 0x0080, // rfr for pre pic1.5 (rev <= b)
	CONFIG_GR2      = 0x0100, // gr2 giobus mode
	CONFIG_INIT     = 0x0200, // drive vme sysreset (will reset cpu)
	CONFIG_ENPAR    = 0x0400, // parity check on memory reads
	CONFIG_SLAVE    = 0x0800, // allow slave accesses to board
	CONFIG_ARB      = 0x1000, // enable vme arbiter function
	CONFIG_BADPAR   = 0x2000, // write bad parity
	CONFIG_DOG      = 0x4000, // enable watchdog timeout
	CONFIG_GRRESET  = 0x8000, // reset graphics subsystem
};

enum sid_mask : u32
{
	SID_FPPRES  = 0x0001, // fpu present (1=absent)
	SID_DMAERR  = 0x0004, // dma error
	SID_DMAIDLE = 0x0008, // dma ended
	SID_VMERMW  = 0x0020, // vme read-modify-write cycle active
	SID_REV     = 0x01c0, // chip revision

	SID_REVA    = 0x0000,
	SID_REVB    = 0x0040,
	SID_REVC    = 0x0080,
};

sgi_pic1_device::sgi_pic1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_PIC1, tag, owner, clock)
	, m_bus(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_simms(*this, "SIMMS")
	, m_memcfg{ 0x003f003f, 0x003f003f }
{
}

void sgi_pic1_device::device_start()
{
}

void sgi_pic1_device::device_reset()
{
	u8 const simms = m_simms->read();

	// allocate installed memory
	unsigned bank16m = 0;
	for (unsigned bank = 0; bank < 4; bank++)
	{
		unsigned const size = BIT(simms, bank * 2, 2);

		switch (size)
		{
		case 0:
			LOG("ram bank %c empty\n", bank + 'A');
			m_ram[bank].reset();
			break;
		case 2:
			bank16m++;
			[[fallthrough]];
		default:
			LOG("ram bank %c size %dM\n", bank + 'A', 1U << (size + 2));
			m_ram[bank] = std::make_unique<u8[]>(1U << (size + 22));
			break;
		}
	}

	if (bank16m > 1)
		logerror("invalid memory configuration, at most one bank may contain 4M SIMMs\n");

	m_cpucfg = CONFIG_BIGEND;

	// assume memory configuration is invalidated by reset
	memcfg_w(0, 0x003f003f);
	memcfg_w(1, 0x003f003f);

	m_sid = SID_DMAIDLE;

	m_dabr = 0;
}

void sgi_pic1_device::map(address_map &map)
{
	// system registers
	map(0x0'0000, 0x0'0003).rw(FUNC(sgi_pic1_device::cpucfg_r), FUNC(sgi_pic1_device::cpucfg_w));
	//map(0x0'0004, 0x0'0007); // RSTCONFIG system mode register
	map(0x0'0008, 0x0'000b).r(FUNC(sgi_pic1_device::sid_r));

	// memory configuration registers
	map(0x1'0000, 0x1'0007).rw(FUNC(sgi_pic1_device::memcfg_r), FUNC(sgi_pic1_device::memcfg_w));
	//map(0x1'0100, 0x1'0103); // REFTIM0   refresh timer (wo)

	// memory parity error registers
	map(0x1'0200, 0x1'0203).nopr(); // PARERR    parity error register (ro)
	map(0x1'0204, 0x1'0207).nopr(); // CPUPARADR cpu parity error address (ro)
	map(0x1'0208, 0x1'020b).nopr(); // DMAPARADR dma slave parity error address (ro)
	map(0x1'0210, 0x1'0213).w(FUNC(sgi_pic1_device::parcl_ws));

	// gio bus config
	map(0x2'0000, 0x2'0003).nopw(); // GIO_SLOT0 slot 0 config register
	map(0x2'0004, 0x2'0007).nopw(); // GIO_SLOT1 slot 1 config register
	map(0x2'0008, 0x2'000b).nopw(); // GIO_BURST arbiter burst register
	map(0x2'000c, 0x2'000f).nopw(); // GIO_DELAY arbiter delay register

	// three-way transfer
	//map(0x7'0000, 0x7'0003); // TW_TRIG   3-way transfer trigger data
	//map(0x8'0000, 0x8'0003); // TW_STATUS 3-way status (ro)
	//map(0x8'0004, 0x8'0007); // TW_START  3-way transfer start address (ro)
	//map(0x8'0008, 0x8'000b); // TW_MASK   3-way transfer address mask
	//map(0x8'000c, 0x8'000f); // TW_SUBS   3-way transfer address substitute

	// graphics channel registers
	map(0xa'0000, 0xa'0003).rw(FUNC(sgi_pic1_device::dabr_r), FUNC(sgi_pic1_device::dabr_w));
	//map(0xa'0004, 0xa'0007); // buffer address (ro)
	//map(0xa'0008, 0xa'000b); // buffer length (ro)
	//map(0xa'000c, 0xa'000f); // graphics operand address (ro)
	//map(0xa'0010, 0xa'0013); // stride/line count (ro)
	//map(0xa'0014, 0xa'0017); // next descriptor pointer (ro)
	//map(0xa'0100, 0xa'0103); // start graphics dma
}

void sgi_pic1_device::memcfg_w(offs_t offset, u32 data)
{
	LOG("memcfg%d 0x%08x\n", offset, data);

	// remove existing mapping
	for (unsigned bank = 0; bank < 2; bank++)
	{
		u32 const base = BIT(m_memcfg[offset], 16 - bank * 16, 6) << 22;

		if (base != 0x3f)
		{
			unsigned const size = ram_size(offset * 2 + bank);
			if (size)
			{
				LOG("unmap bank %c 0x%08x to 0x%08x\n", offset * 2 + bank + 'A', base, base + size - 1);
				m_bus->unmap_readwrite(base, base + size - 1);
			}
		}
	}

	m_memcfg[offset] = data;

	// install configured mapping
	for (unsigned bank = 0; bank < 2; bank++)
	{
		u32 const base = BIT(m_memcfg[offset], 16 - bank * 16, 6) << 22;

		if (base != 0x3f)
		{
			unsigned const size = ram_size(offset * 2 + bank);
			if (size)
			{
				LOG("remap bank %c 0x%08x to 0x%08x\n", offset * 2 + bank + 'A', base, base + size - 1);
				m_bus->install_ram(base, base + size - 1, m_ram[offset * 2 + bank].get());
			}
		}
	}
}

void sgi_pic1_device::parcl_ws(u32 data)
{
	LOG("clear parity error\n");
}

// return bytes available in a bank (minimum of installed and configured ram)
unsigned sgi_pic1_device::ram_size(unsigned bank) const
{
	unsigned const inst_size = BIT(m_simms->read(), bank * 2, 2);
	unsigned const conf_size = BIT(m_memcfg[bank >> 1], 24 - (bank & 1) * 16, 4);

	if (inst_size && conf_size)
		return std::min((conf_size + 1) << 22, 1U << (inst_size + 22));
	else
		return 0;
}

static INPUT_PORTS_START(pic1)
	PORT_START("VALID")
	PORT_CONFNAME(0x0f, 0x00, "Valid Banks")

	PORT_START("SIMMS")
	PORT_CONFNAME(0x03, 0x02, "RAM bank A") PORT_CONDITION("VALID", 0x01, EQUALS, 0x01)
	PORT_CONFSETTING(0x00, "Empty")
	PORT_CONFSETTING(0x01, "4x2M")
	PORT_CONFSETTING(0x02, "4x4M")
	PORT_CONFSETTING(0x03, "4x8M")

	PORT_CONFNAME(0x0c, 0x00, "RAM bank B") PORT_CONDITION("VALID", 0x02, EQUALS, 0x02)
	PORT_CONFSETTING(0x00, "Empty")
	PORT_CONFSETTING(0x04, "4x2M")
	PORT_CONFSETTING(0x08, "4x4M")
	PORT_CONFSETTING(0x0c, "4x8M")

	PORT_CONFNAME(0x30, 0x00, "RAM bank C") PORT_CONDITION("VALID", 0x04, EQUALS, 0x04)
	PORT_CONFSETTING(0x00, "Empty")
	PORT_CONFSETTING(0x10, "4x2M")
	PORT_CONFSETTING(0x20, "4x4M")
	PORT_CONFSETTING(0x30, "4x8M")

	PORT_CONFNAME(0xc0, 0x00, "RAM bank D") PORT_CONDITION("VALID", 0x08, EQUALS, 0x08)
	PORT_CONFSETTING(0x00, "Empty")
	PORT_CONFSETTING(0x40, "4x2M")
	PORT_CONFSETTING(0x80, "4x4M")
	PORT_CONFSETTING(0xc0, "4x8M")
INPUT_PORTS_END

ioport_constructor sgi_pic1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pic1);
}
