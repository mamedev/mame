// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Memory Access Controller emulation

**********************************************************************/

#include "emu.h"
#include "abc1600mac.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0
#define LOG_MAC 0
#define LOG_DMA 0

#define A8          BIT(offset, 8)

#define FC2         BIT(fc, 2)

#define PAGE_WP     BIT(page_data, 14)
#define PAGE_NONX   BIT(page_data, 15)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC1600_MAC, abc1600_mac_device, "abc1600mac", "ABC 1600 MAC")


void abc1600_mac_device::program_map(address_map &map)
{
	// populated in drivers/abc1600.cpp
}

void abc1600_mac_device::mac_map(address_map &map)
{
	map(0x80000, 0x80000).mirror(0x006f8).select(0x7f900).rw(FUNC(abc1600_mac_device::page_hi_r), FUNC(abc1600_mac_device::page_hi_w));
	map(0x80001, 0x80001).mirror(0x006f8).select(0x7f900).rw(FUNC(abc1600_mac_device::page_lo_r), FUNC(abc1600_mac_device::page_lo_w));
	map(0x80002, 0x80002).mirror(0x7fff8).noprw();
	map(0x80003, 0x80003).mirror(0x07ef8).select(0x78100).rw(FUNC(abc1600_mac_device::segment_r), FUNC(abc1600_mac_device::segment_w));
	map(0x80004, 0x80004).mirror(0x7fff8).noprw();
	map(0x80005, 0x80005).mirror(0x7fff8).nopr().w(FUNC(abc1600_mac_device::task_w));
	map(0x80006, 0x80006).mirror(0x7fff8).noprw();
	map(0x80007, 0x80007).mirror(0x7fff8).r(FUNC(abc1600_mac_device::cause_r)).nopw();
}


void abc1600_mac_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1600)); // XTAL(64'000'000)/8/10/20000/8/8
}


//-------------------------------------------------
//  ROM( abc1600_mac )
//-------------------------------------------------

ROM_START( abc1600_mac )
	ROM_REGION( 0x4000, "boot", 0 )
	ROM_LOAD( "boot 6490356-04.1f", 0x0000, 0x4000, CRC(9372f6f2) SHA1(86f0681f7ef8dd190b49eda5e781881582e0c2a4) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "1022 6490351-01.17e", 0x000, 0x104, CRC(5dd00d43) SHA1(a3871f0d796bea9df8f25d41b3169dd4b8ef65ab) ) // PAL16L8 MAC register address decoder
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc1600_mac_device::device_rom_region() const
{
	return ROM_NAME( abc1600_mac );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc1600_mac_device - constructor
//-------------------------------------------------

abc1600_mac_device::abc1600_mac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC1600_MAC, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_BIG, 8, 21, 0, address_map_constructor(FUNC(abc1600_mac_device::program_map), this)),
	m_mac_config("mac", ENDIANNESS_BIG, 8, 20, 0, address_map_constructor(FUNC(abc1600_mac_device::mac_map), this)),
	m_rom(*this, "boot"),
	m_segment_ram(*this, "segment_ram", 0x400, ENDIANNESS_LITTLE),
	m_page_ram(*this, "page_ram", 0x800, ENDIANNESS_LITTLE),
	m_watchdog(*this, "watchdog"),
	m_read_fc(*this),
	m_write_buserr(*this),
	m_read_tren(*this),
	m_write_tren(*this),
	m_boote(0),
	m_magic(0),
	m_task(0),
	m_cause(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600_mac_device::device_start()
{
	// resolve callbacks
	m_read_fc.resolve_safe(0);
	m_write_buserr.resolve_safe();
	m_read_tren.resolve_all_safe(0xff);
	m_write_tren.resolve_all_safe();

	// state saving
	save_item(NAME(m_boote));
	save_item(NAME(m_magic));
	save_item(NAME(m_task));
	save_item(NAME(m_dmamap));
	save_item(NAME(m_cause));

	// HACK fill segment RAM or abcenix won't boot
	memset(m_segment_ram, 0xff, 0x200);
	memset(m_page_ram, 0xff, 0x800);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc1600_mac_device::device_reset()
{
	// clear task register
	m_boote = 0;
	m_magic = 0;
	m_task = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector abc1600_mac_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_MAC, &m_mac_config)
	};
}


//-------------------------------------------------
//  get_physical_offset -
//-------------------------------------------------

inline offs_t abc1600_mac_device::get_physical_offset(offs_t offset, int task, bool &nonx, bool &wp)
{
	// segment
	offs_t sega = (task << 5) | ((offset >> 15) & 0x1f);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0x0f);
	uint16_t page_data = m_page_ram[pga];

	offs_t virtual_offset = ((page_data & 0x3ff) << 11) | (offset & 0x7ff);

	nonx = PAGE_NONX;
	wp = PAGE_WP;

	if (LOG && (offset != virtual_offset)) logerror("%s MAC %05x:%06x (SEGA %03x SEGD %02x PGA %03x PGD %04x NONX %u WP %u TASK %u FC %u MAGIC %u)\n",
		machine().describe_context(), offset, virtual_offset, sega, segd, pga, page_data, nonx, wp, task, m_read_fc(), m_magic);

	return virtual_offset;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t abc1600_mac_device::read(offs_t offset)
{
	if (!m_boote && (offset < 0x20000))
	{
		return m_rom->base()[offset & 0x3fff];
	}

	uint8_t fc = m_read_fc();
	int task = m_task;

	if (FC2)
	{
		if (offset & 0x80000)
		{
			if (LOG_MAC && (offset != 0x80007)) logerror("%s MAC R %05x\n",
					machine().describe_context(), offset);

			return space(AS_MAC).read_byte(offset);
		}
		else
		{
			task = 0;
		}
	}

	if (!m_magic && (fc == M68K_FC_USER_PROGRAM))
	{
		task = 0;
	}

	bool nonx, wp;
	offs_t virtual_offset = get_physical_offset(offset, task, nonx, wp);

	if (!machine().side_effects_disabled())
	{
		if (nonx)
		{
			logerror("%s BUS ERROR R %05x:%06x (NONX %u WP %u TASK %u FC %u MAGIC %u)\n", 
				machine().describe_context(), offset, virtual_offset, nonx, wp, task, fc, m_magic);
			dump();
			m_write_buserr(offset, 1);
		}
	}

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void abc1600_mac_device::write(offs_t offset, uint8_t data)
{
	uint8_t fc = m_read_fc();
	int task = m_task;

	if (FC2)
	{
		if (offset & 0x80000)
		{
			if (LOG_MAC) logerror("%s MAC W %05x:%02x\n",
				machine().describe_context(), offset, data);

			space(AS_MAC).write_byte(offset, data);
			return;
		}
		else
		{
			task = 0;
		}
	}

	bool nonx, wp;
	offs_t virtual_offset = get_physical_offset(offset, task, nonx, wp);

	if (!machine().side_effects_disabled())
	{
		if (nonx)
		{
			logerror("%s BUS ERROR W %05x:%06x (NONX %u WP %u TASK %u FC %u MAGIC %u)\n", 
				machine().describe_context(), offset, virtual_offset, nonx, wp, task, fc, m_magic);
			dump();
			m_write_buserr(offset, 0);
		}
		if (!wp)
		{
			logerror("%s BUS ERROR W %05x:%06x (NONX %u WP %u TASK %u FC %u MAGIC %u)\n", 
				machine().describe_context(), offset, virtual_offset, nonx, wp, task, fc, m_magic);
			dump();
			m_write_buserr(offset, 0);
		}
	}

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  cause_r -
//-------------------------------------------------

uint8_t abc1600_mac_device::cause_r()
{
	/*

	    bit     description

	    0       RSTBUT
	    1       1
	    2       DMAOK
	    3       X16
	    4       X17
	    5       X18
	    6       X19
	    7       X20

	*/

	uint8_t data = 0x02;

	// DMA status
	data |= m_cause;

	m_watchdog->watchdog_reset();

	return data;
}


//-------------------------------------------------
//  task_w -
//-------------------------------------------------

void abc1600_mac_device::task_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       TASKD0* (inverted SEGA5)
	    1       TASKD1* (inverted SEGA6)
	    2       TASKD2* (inverted SEGA7)
	    3       TASKD3* (inverted SEGA8)
	    4
	    5
	    6       BOOTE*
	    7       MAGIC*

	*/

	m_task = data & 0x0f;
	m_boote = !BIT(data, 6);
	m_magic = !BIT(data, 7);

	if (LOG_MAC) logerror("%s TASK W %02x (TASK %u BOOTE %u MAGIC %u)\n",
		machine().describe_context(), data, m_task, m_boote, m_magic);
}


//-------------------------------------------------
//  segment_r -
//-------------------------------------------------

uint8_t abc1600_mac_device::segment_r(offs_t offset)
{
	/*

	    bit     description

	    0       SEGD0
	    1       SEGD1
	    2       SEGD2
	    3       SEGD3
	    4       SEGD4
	    5       SEGD5
	    6       SEGD6
	    7       READ_MAGIC

	*/

	offs_t sega = (m_task << 5) | A8 << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	return (!m_magic << 7) | (segd & 0x7f);
}


//-------------------------------------------------
//  segment_w -
//-------------------------------------------------

void abc1600_mac_device::segment_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       SEGD0
	    1       SEGD1
	    2       SEGD2
	    3       SEGD3
	    4       SEGD4
	    5       SEGD5
	    6       SEGD6
	    7       0

	*/

	offs_t sega = (m_task << 5) | A8 << 4 | ((offset >> 15) & 0xf);
	m_segment_ram[sega] = data & 0x7f;

	if (LOG_MAC) logerror("%s SEG W %05x:%02x SEGA %03x SEGD %02x TASK %u\n",
		machine().describe_context(), offset, data,
		sega, m_segment_ram[sega], m_task);
}


//-------------------------------------------------
//  page_lo_r -
//-------------------------------------------------

uint8_t abc1600_mac_device::page_lo_r(offs_t offset)
{
	/*

	    bit     description

	    0       X11
	    1       X12
	    2       X13
	    3       X14
	    4       X15
	    5       X16
	    6       X17
	    7       X18

	*/

	// segment
	offs_t sega = (m_task << 5) | A8 << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	uint16_t pgd = m_page_ram[pga];

	return pgd & 0xff;
}


//-------------------------------------------------
//  page_hi_r -
//-------------------------------------------------

uint8_t abc1600_mac_device::page_hi_r(offs_t offset)
{
	/*

	    bit     description

	    0       X19
	    1       X20
	    2       X20
	    3       X20
	    4       X20
	    5       X20
	    6       _WP
	    7       NONX

	*/

	// segment
	offs_t sega = (m_task << 5) | A8 << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	uint16_t pgd = m_page_ram[pga];

	int x20 = BIT(pgd, 9);

	return (pgd >> 8) | (x20 << 2) | (x20 << 3) | (x20 << 4) | (x20 << 5);
}


//-------------------------------------------------
//  page_lo_w -
//-------------------------------------------------

void abc1600_mac_device::page_lo_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       X11
	    1       X12
	    2       X13
	    3       X14
	    4       X15
	    5       X16
	    6       X17
	    7       X18

	*/

	// segment
	offs_t sega = (m_task << 5) | A8 << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	m_page_ram[pga] = (m_page_ram[pga] & 0xff00) | data;

	if (LOG_MAC) logerror("%s PAGE W %05x:%02x (SEGA %03x SEGD %02x PGA %03x PGD %04x TASK %u)\n",
		machine().describe_context(), offset, data,
		sega, segd, pga, m_page_ram[pga], m_task);
}


//-------------------------------------------------
//  page_hi_w -
//-------------------------------------------------

void abc1600_mac_device::page_hi_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       X19
	    1       X20
	    2
	    3
	    4
	    5
	    6       _WP
	    7       NONX

	*/

	// segment
	offs_t sega = (m_task << 5) | A8 << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	m_page_ram[pga] = ((data & 0xc3) << 8) | (m_page_ram[pga] & 0xff);
}


//-------------------------------------------------
//  get_dma_address -
//-------------------------------------------------

offs_t abc1600_mac_device::get_dma_address(int index, offs_t offset, bool &rw)
{
	// A0 = DMA15, A1 = BA1, A2 = BA2
	uint8_t dmamap_addr = index | BIT(offset, 15);
	uint8_t dmamap = m_dmamap[dmamap_addr];

	m_cause = (dmamap & 0x1f) << 3;

	rw = BIT(dmamap, 7);

	return ((dmamap & 0x1f) << 16) | offset;
}


//-------------------------------------------------
//  dma_mreq_r - DMA memory read
//-------------------------------------------------

uint8_t abc1600_mac_device::dma_mreq_r(int index, int dmamap, offs_t offset)
{
	bool rw;
	offs_t virtual_offset = get_dma_address(dmamap, offset, rw);

	if (LOG_DMA) logerror("%s DMRQ R %04x:%06x %c\n", machine().describe_context(), offset, virtual_offset, rw ? 'R' : 'W');

	uint8_t data = 0xff;

	if (rw)
	{
		data = space().read_byte(virtual_offset);

		m_write_tren[index](data);
	}
	else
	{
		data = m_read_tren[index](data);

		space().write_byte(virtual_offset, data);
	}

	return data;
}


//-------------------------------------------------
//  dma_mreq_w - DMA memory write
//-------------------------------------------------

void abc1600_mac_device::dma_mreq_w(int index, int dmamap, offs_t offset, uint8_t data)
{
	bool rw;
	offs_t virtual_offset = get_dma_address(dmamap, offset, rw);

	if (LOG_DMA) logerror("%s DMRQ W %04x:%06x %c\n", machine().describe_context(), offset, virtual_offset, rw ? 'R' : 'W');

	if (!rw)
	{
		space().write_byte(virtual_offset, data);
	}
}


//-------------------------------------------------
//  dma_iorq_r - DMA I/O read
//-------------------------------------------------

uint8_t abc1600_mac_device::dma_iorq_r(int dmamap, offs_t offset)
{
	bool rw;
	offs_t virtual_offset = 0x1fe000 | get_dma_address(dmamap, offset, rw);

	if (LOG_DMA) logerror("%s DIORQ R %04x:%06x\n", machine().describe_context(), offset, virtual_offset);

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  dma_iorq_w - DMA I/O write
//-------------------------------------------------

void abc1600_mac_device::dma_iorq_w(int dmamap, offs_t offset, uint8_t data)
{
	bool rw;
	offs_t virtual_offset = 0x1fe000 | get_dma_address(dmamap, offset, rw);

	if (LOG_DMA) logerror("%s DIORQ W %04x:%06x\n", machine().describe_context(), offset, virtual_offset);

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  dmamap_w - DMA map write
//-------------------------------------------------

void abc1600_mac_device::dmamap_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       X16
	    1       X17
	    2       X18
	    3       X19
	    4       X20
	    5
	    6
	    7       _R/W

	*/

	if (LOG_DMA) logerror("%s DMAMAP %u:%02x\n", machine().describe_context(), offset & 7, data);

	m_dmamap[offset & 7] = data;
}


//-------------------------------------------------
//  dump - dump MAC mappings
//-------------------------------------------------

void abc1600_mac_device::dump()
{
	for (int task = 0; task < 16; task++) {
		for (int seg = 0; seg < 32; seg++) {
			for (int page = 0; page < 16; page++) {
				offs_t logical = (seg * 0x8000) | (page * 0x800);

				offs_t sega = (task << 5) | seg;
				uint8_t segd = m_segment_ram[sega];

				offs_t pga = ((segd & 0x3f) << 4) | page;
				uint16_t page_data = m_page_ram[pga];

				offs_t physical = (page_data & 0x3ff) << 11;
				bool nonx = PAGE_NONX;
				bool wp = PAGE_WP;

				logerror("TASK %.2u SEGMENT %.2u PAGE %.2u MEM %05x-%05x %06x-%06x %c %c\n",
					task, seg, page, logical, logical + 0x7ff, physical, physical + 0x7ff, nonx ? 'X' : ' ', wp ? ' ' : 'W');
			}
		}
	}
}
