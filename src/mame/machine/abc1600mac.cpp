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
#define LOG_IO 0


#define A0          BIT(offset, 0)
#define A1          BIT(offset, 1)
#define A2          BIT(offset, 2)
#define A4          BIT(offset, 4)
#define A7          BIT(offset, 7)
#define A8          BIT(offset, 8)
#define X11         BIT(offset, 11)
#define X12         BIT(offset, 12)
#define A17         BIT(offset, 17)
#define A18         BIT(offset, 18)
#define A19         BIT(offset, 19)

#define A10_A9_A8   ((offset >> 8) & 0x07)
#define A2_A1_A0    (offset & 0x07)
#define A1_A2       ((A1 << 1) | A2)
#define A2_A1       ((offset >> 1) & 0x03)

#define FC0         BIT(fc, 0)
#define FC1         BIT(fc, 1)
#define FC2         BIT(fc, 2)

#define PAGE_WP     BIT(page_data, 14)
#define PAGE_NONX   BIT(page_data, 15)

#define BOOTE       BIT(m_task, 6)
#define MAGIC       BIT(m_task, 7)
#define READ_MAGIC  !MAGIC



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC1600_MAC, abc1600_mac_device, "abc1600mac", "ABC 1600 MAC")


void abc1600_mac_device::map(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(abc1600_mac_device::read), FUNC(abc1600_mac_device::write));
}


void abc1600_mac_device::program_map(address_map &map)
{
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
	m_space_config("program", ENDIANNESS_LITTLE, 8, 22, 0, address_map_constructor(FUNC(abc1600_mac_device::program_map), this)),
	m_rom(*this, "boot"),
	m_segment_ram(*this, "segment_ram", 0x400, ENDIANNESS_LITTLE),
	m_page_ram(*this, "page_ram", 0x800, ENDIANNESS_LITTLE),
	m_watchdog(*this, "watchdog"),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_task(0),
	m_cause(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600_mac_device::device_start()
{
	// HACK fill segment RAM or abcenix won't boot
	memset(m_segment_ram, 0xcd, 0x400);
	//memset(m_page_ram, 0xcd, 0x400);

	// state saving
	save_item(NAME(m_ifc2));
	save_item(NAME(m_task));
	save_item(NAME(m_dmamap));
	save_item(NAME(m_cause));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc1600_mac_device::device_reset()
{
	// clear task register
	m_task = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector abc1600_mac_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config)
	};
}


//-------------------------------------------------
//  get_current_task -
//-------------------------------------------------

int abc1600_mac_device::get_current_task(offs_t offset)
{
	int force_task0 = !(m_ifc2 || A19);
	int t0 = !(BIT(m_task, 0) || force_task0);
	int t1 = !(BIT(m_task, 1) || force_task0);
	int t2 = !(BIT(m_task, 2) || force_task0);
	int t3 = !(BIT(m_task, 3) || force_task0);

	return (t3 << 3) | (t2 << 2) | (t1 << 1) | t0;
}


//-------------------------------------------------
//  get_segment_address -
//-------------------------------------------------

offs_t abc1600_mac_device::get_segment_address(offs_t offset)
{
	int sega19 = !(!(A8 || m_ifc2) || !A19);
	int task = get_current_task(offset);

	return (task << 5) | (sega19 << 4) | ((offset >> 15) & 0x0f);
}


//-------------------------------------------------
//  get_page_address -
//-------------------------------------------------

offs_t abc1600_mac_device::get_page_address(offs_t offset, uint8_t segd)
{
	return ((segd & 0x3f) << 4) | ((offset >> 11) & 0x0f);
}


//-------------------------------------------------
//  translate_address -
//-------------------------------------------------

offs_t abc1600_mac_device::translate_address(offs_t offset, int *nonx, int *wp)
{
	// segment
	offs_t sega = get_segment_address(offset);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = get_page_address(offset, segd);
	uint16_t page_data = m_page_ram[pga];

	offs_t virtual_offset = ((page_data & 0x3ff) << 11) | (offset & 0x7ff);

	if (PAGE_NONX)
	{
		//logerror("Bus error %06x : %06x\n", offset, virtual_offset);
		//m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		//m_cpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}

	*nonx = PAGE_NONX;
	*wp = PAGE_WP;

	if (LOG_MAC && offset != virtual_offset) logerror("%s MAC %05x:%06x (SEGA %03x SEGD %02x PGA %03x PGD %04x NONX %u WP %u TASK %u FC %u)\n", machine().describe_context(), offset, virtual_offset, sega, segd, pga, m_page_ram[pga], *nonx, *wp, get_current_task(offset), get_fc());

	return virtual_offset;
}


//-------------------------------------------------
//  read_user_memory -
//-------------------------------------------------

uint8_t abc1600_mac_device::read_user_memory(offs_t offset)
{
	int nonx = 0, wp = 0;
	offs_t virtual_offset = translate_address(offset, &nonx, &wp);
	uint8_t data = space().read_byte(virtual_offset);

	if (LOG_IO && virtual_offset >= 0x1fe000) logerror("%s user read %06x:%02x\n", machine().describe_context(), virtual_offset, data);

	return data;
}


//-------------------------------------------------
//  write_user_memory -
//-------------------------------------------------

void abc1600_mac_device::write_user_memory(offs_t offset, uint8_t data)
{
	int nonx = 0, wp = 0;
	offs_t virtual_offset = translate_address(offset, &nonx, &wp);

	//if (nonx || !wp) return;

	if (LOG_IO && virtual_offset >= 0x1fe000) logerror("%s user write %06x:%02x\n", machine().describe_context(), virtual_offset, data);

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  read_supervisor_memory -
//-------------------------------------------------

uint8_t abc1600_mac_device::read_supervisor_memory(offs_t offset)
{
	uint8_t data = 0;

	if (!A2 && !A1)
	{
		// _EP
		data = page_r(offset);
	}
	else if (!A2 && A1 && A0)
	{
		// _ES
		data = segment_r(offset);
	}
	else if (A2 && A1 && A0)
	{
		// _CAUSE
		data = cause_r();
	}

	return data;
}


//-------------------------------------------------
//  write_supervisor_memory -
//-------------------------------------------------

void abc1600_mac_device::write_supervisor_memory(offs_t offset, uint8_t data)
{
	if (!A2 && !A1)
	{
		// _WEP
		page_w(offset, data);
	}
	else if (!A2 && A1 && A0)
	{
		// _WES
		segment_w(offset, data);
	}
	else if (A2 && !A1 && A0)
	{
		// W(C)
		task_w(offset, data);
	}
}


//-------------------------------------------------
//  get_fc -
//-------------------------------------------------

int abc1600_mac_device::get_fc()
{
	uint16_t fc = m_cpu->get_fc();

	m_ifc2 = !(!(MAGIC || FC0) || FC2);

	return fc;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t abc1600_mac_device::read(offs_t offset)
{
	int fc = get_fc();

	uint8_t data = 0;

	if (!BOOTE && !A19 && !A18 && !A17)
	{
		// _BOOTCE
		data = m_rom->base()[offset & 0x3fff];
	}
	else if (A19 && !m_ifc2 && !FC1)
	{
		data = read_supervisor_memory(offset);
	}
	else
	{
		data = read_user_memory(offset);
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void abc1600_mac_device::write(offs_t offset, uint8_t data)
{
	int fc = get_fc();

	if (A19 && !m_ifc2 && !FC1)
	{
		write_supervisor_memory(offset, data);
	}
	else
	{
		write_user_memory(offset, data);
	}
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

	m_task = data ^ 0xff;

	if (LOG) logerror("%s TASK %05x:%02x (TASK %u BOOTE %u MAGIC %u)\n", machine().describe_context(), offset, data,
		get_current_task(offset), BOOTE, MAGIC);
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

	offs_t sega = get_segment_address(offset);
	uint8_t segd = m_segment_ram[sega];

	return (READ_MAGIC << 7) | (segd & 0x7f);
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

	offs_t sega = get_segment_address(offset);

	m_segment_ram[sega] = data & 0x7f;

	if (LOG) logerror("%s %05x:%02x TASK %u SEGMENT %u MEM %05x-%05x\n", machine().describe_context(), offset, data,
		get_current_task(offset), sega & 0x1f, (sega & 0x1f) * 0x8000, ((sega & 0x1f) * 0x8000) + 0x7fff);
}


//-------------------------------------------------
//  page_r -
//-------------------------------------------------

uint8_t abc1600_mac_device::page_r(offs_t offset)
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

	    8       X19
	    9       X20
	    10      X20
	    11      X20
	    12      X20
	    13      X20
	    14      _WP
	    15      NONX

	*/

	// segment
	offs_t sega = get_segment_address(offset);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = get_page_address(offset, segd);
	uint16_t pgd = m_page_ram[pga];

	uint8_t data = 0;

	if (A0)
	{
		data = pgd & 0xff;
	}
	else
	{
		int x20 = BIT(pgd, 9);

		data = (pgd >> 8) | (x20 << 2) | (x20 << 3) | (x20 << 4) | (x20 << 5);
	}

	return data;
}


//-------------------------------------------------
//  page_w -
//-------------------------------------------------

void abc1600_mac_device::page_w(offs_t offset, uint8_t data)
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

	    8       X19
	    9       X20
	    10
	    11
	    12
	    13
	    14      _WP
	    15      NONX

	*/

	// segment
	offs_t sega = get_segment_address(offset);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = get_page_address(offset, segd);

	if (A0)
	{
		m_page_ram[pga] = (m_page_ram[pga] & 0xff00) | data;
	}
	else
	{
		m_page_ram[pga] = ((data & 0xc3) << 8) | (m_page_ram[pga] & 0xff);
	}

	if (LOG) logerror("%s %05x:%02x TASK %u SEGMENT %u PAGE %u MEM %05x-%05x %06x\n", machine().describe_context(), offset, data,
		get_current_task(offset), sega & 0x1f, ((offset >> 11) & 0x0f), ((sega & 0x1f) * 0x8000) + ((offset >> 11) & 0x0f) * 0x800,
		((sega & 0x1f) * 0x8000) + (((offset >> 11) & 0x0f) * 0x800) + 0x7ff, (m_page_ram[pga] & 0x3ff) << 11);
}


//-------------------------------------------------
//  get_dma_address -
//-------------------------------------------------

offs_t abc1600_mac_device::get_dma_address(int index, uint16_t offset)
{
	// A0 = DMA15, A1 = BA1, A2 = BA2
	uint8_t dmamap_addr = index | BIT(offset, 15);
	uint8_t dmamap = m_dmamap[dmamap_addr];

	m_cause = (dmamap & 0x1f) << 3;

	return ((dmamap & 0x1f) << 16) | offset;
}


//-------------------------------------------------
//  dma_mreq_r - DMA memory read
//-------------------------------------------------

uint8_t abc1600_mac_device::dma_mreq_r(int index, uint16_t offset)
{
	offs_t virtual_offset = get_dma_address(index, offset);

	if (LOG_DMA)logerror("%s DMA R %04x:%06x\n", machine().describe_context(), offset, virtual_offset);

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  dma_mreq_w - DMA memory write
//-------------------------------------------------

void abc1600_mac_device::dma_mreq_w(int index, uint16_t offset, uint8_t data)
{
	offs_t virtual_offset = get_dma_address(index, offset);

	if (LOG_DMA)logerror("%s DMA W %04x:%06x\n", machine().describe_context(), offset, virtual_offset);

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  dma_iorq_r - DMA I/O read
//-------------------------------------------------

uint8_t abc1600_mac_device::dma_iorq_r(int index, uint16_t offset)
{
	offs_t virtual_offset = 0x1fe000 | get_dma_address(index, offset);

	if (LOG_DMA)logerror("%s DMA R %04x:%06x\n", machine().describe_context(), offset, virtual_offset);

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  dma_iorq_w - DMA I/O write
//-------------------------------------------------

void abc1600_mac_device::dma_iorq_w(int index, uint16_t offset, uint8_t data)
{
	offs_t virtual_offset = 0x1fe000 | get_dma_address(index, offset);

	if (LOG_DMA)logerror("%s DMA W %04x:%06x\n", machine().describe_context(), offset, virtual_offset);

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
