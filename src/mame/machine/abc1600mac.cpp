// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Memory Access Controller emulation

**********************************************************************/

/*

    TODO:

    - segment/page RAM addresses are not correctly decoded, "sas/format/format" can't find the SASI interface because of this
        forcetask0 1 t0 0 t1 0 t2 0 t3 0
        sega19 0 task 0
        sega 000 segd 00 pga 008 pgd 4058 virtual 02c730 (should be 004730)
*/

#include "abc1600mac.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


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

const device_type ABC1600_MAC = &device_creator<abc1600_mac_device>;


DEVICE_ADDRESS_MAP_START( map, 8, abc1600_mac_device )
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, abc1600_mac_device )
ADDRESS_MAP_END


//-------------------------------------------------
//  ROM( abc1600_mac )
//-------------------------------------------------

ROM_START( abc1600_mac )
	ROM_REGION( 0x4000, "boot", 0 )
	ROM_LOAD( "boot 6490356-04.1f", 0x0000, 0x4000, CRC(9372f6f2) SHA1(86f0681f7ef8dd190b49eda5e781881582e0c2a4) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "1022 6490351-01.17e", 0x000, 0x104, CRC(5dd00d43) SHA1(a3871f0d796bea9df8f25d41b3169dd4b8ef65ab) ) // MAC register address decoder
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc1600_mac_device::device_rom_region() const
{
	return ROM_NAME( abc1600_mac );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc1600_mac_device - constructor
//-------------------------------------------------

abc1600_mac_device::abc1600_mac_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ABC1600_MAC, "ABC 1600 MAC", tag, owner, clock, "abc1600mac", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("program", ENDIANNESS_LITTLE, 8, 22, 0, *ADDRESS_MAP_NAME(program_map)),
		m_rom(*this, "boot"),
		m_segment_ram(*this, "segment_ram"),
		m_page_ram(*this, "page_ram"),
		m_task(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600_mac_device::device_start()
{
	// get the CPU device
	m_cpu = machine().device<m68000_base_device>(m_cpu_tag);
	assert(m_cpu != nullptr);

	// allocate memory
	m_segment_ram.allocate(0x400);
	m_page_ram.allocate(0x400);

	// HACK fill segment RAM or abcenix won't boot
	memset(m_segment_ram, 0xcd, 0x400);

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

const address_space_config *abc1600_mac_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_space_config : nullptr;
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

offs_t abc1600_mac_device::get_page_address(offs_t offset, UINT8 segd)
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
	UINT8 segd = m_segment_ram[sega];

	// page
	offs_t pga = get_page_address(offset, segd);
	UINT16 page_data = m_page_ram[pga];

	offs_t virtual_offset = ((page_data & 0x3ff) << 11) | (offset & 0x7ff);

	if (PAGE_NONX)
	{
		//logerror("Bus error %06x : %06x\n", offset, virtual_offset);
		//m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		//m_cpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}

	*nonx = PAGE_NONX;
	*wp = PAGE_WP;

	return virtual_offset;
}


//-------------------------------------------------
//  read_user_memory -
//-------------------------------------------------

UINT8 abc1600_mac_device::read_user_memory(offs_t offset)
{
	int nonx = 0, wp = 0;
	offs_t virtual_offset = translate_address(offset, &nonx, &wp);

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  write_user_memory -
//-------------------------------------------------

void abc1600_mac_device::write_user_memory(offs_t offset, UINT8 data)
{
	int nonx = 0, wp = 0;
	offs_t virtual_offset = translate_address(offset, &nonx, &wp);

	//if (nonx || !wp) return;

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  read_supervisor_memory -
//-------------------------------------------------

UINT8 abc1600_mac_device::read_supervisor_memory(address_space &space, offs_t offset)
{
	UINT8 data = 0;

	if (!A2 && !A1)
	{
		// _EP
		data = page_r(space, offset);
	}
	else if (!A2 && A1 && A0)
	{
		// _ES
		data = segment_r(space, offset);
	}
	else if (A2 && A1 && A0)
	{
		// _CAUSE
		data = cause_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  write_supervisor_memory -
//-------------------------------------------------

void abc1600_mac_device::write_supervisor_memory(address_space &space, offs_t offset, UINT8 data)
{
	if (!A2 && !A1)
	{
		// _WEP
		page_w(space, offset, data);
	}
	else if (!A2 && A1 && A0)
	{
		// _WES
		segment_w(space, offset, data);
	}
	else if (A2 && !A1 && A0)
	{
		// W(C)
		task_w(space, offset, data);
	}
}


//-------------------------------------------------
//  get_fc -
//-------------------------------------------------

int abc1600_mac_device::get_fc()
{
	UINT16 fc = m_cpu->get_fc();

	m_ifc2 = !(!(MAGIC || FC0) || FC2);

	return fc;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( abc1600_mac_device::read )
{
	int fc = get_fc();

	UINT8 data = 0;

	if (!BOOTE && !A19 && !A18 && !A17)
	{
		// _BOOTCE
		data = m_rom->base()[offset & 0x3fff];
	}
	else if (A19 && !m_ifc2 && !FC1)
	{
		data = read_supervisor_memory(space, offset);
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

WRITE8_MEMBER( abc1600_mac_device::write )
{
	int fc = get_fc();

	if (A19 && !m_ifc2 && !FC1)
	{
		write_supervisor_memory(space, offset, data);
	}
	else
	{
		write_user_memory(offset, data);
	}
}


//-------------------------------------------------
//  cause_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_mac_device::cause_r )
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

	UINT8 data = 0x02;

	// DMA status
	data |= m_cause;

	machine().watchdog_reset();

	return data;
}


//-------------------------------------------------
//  task_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mac_device::task_w )
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

	if (LOG) logerror("%s: %06x Task %u BOOTE %u MAGIC %u\n", machine().describe_context(), offset, get_current_task(offset), BOOTE, MAGIC);
}


//-------------------------------------------------
//  segment_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_mac_device::segment_r )
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
	UINT8 segd = m_segment_ram[sega];

	return (READ_MAGIC << 7) | (segd & 0x7f);
}


//-------------------------------------------------
//  segment_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mac_device::segment_w )
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

	if (LOG) logerror("%s: %06x Task %u Segment %03x : %02x\n", machine().describe_context(), offset, get_current_task(offset), sega, data);
}


//-------------------------------------------------
//  page_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_mac_device::page_r )
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
	UINT8 segd = m_segment_ram[sega];

	// page
	offs_t pga = get_page_address(offset, segd);
	UINT16 pgd = m_page_ram[pga];

	UINT8 data = 0;

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

WRITE8_MEMBER( abc1600_mac_device::page_w )
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
	UINT8 segd = m_segment_ram[sega];

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

	if (LOG) logerror("%s: %06x Task %u Segment %03x Page %03x : %02x -> %04x\n", machine().describe_context(), offset, get_current_task(offset), sega, pga, data, m_page_ram[pga]);
}


//-------------------------------------------------
//  get_dma_address -
//-------------------------------------------------

offs_t abc1600_mac_device::get_dma_address(int index, UINT16 offset)
{
	// A0 = DMA15, A1 = BA1, A2 = BA2
	UINT8 dmamap_addr = index | BIT(offset, 15);
	UINT8 dmamap = m_dmamap[dmamap_addr];

	m_cause = (dmamap & 0x1f) << 3;

	return ((dmamap & 0x1f) << 16) | offset;
}


//-------------------------------------------------
//  dma_mreq_r - DMA memory read
//-------------------------------------------------

UINT8 abc1600_mac_device::dma_mreq_r(int index, UINT16 offset)
{
	offs_t virtual_offset = get_dma_address(index, offset);

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  dma_mreq_w - DMA memory write
//-------------------------------------------------

void abc1600_mac_device::dma_mreq_w(int index, UINT16 offset, UINT8 data)
{
	offs_t virtual_offset = get_dma_address(index, offset);

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  dma_iorq_r - DMA I/O read
//-------------------------------------------------

UINT8 abc1600_mac_device::dma_iorq_r(int index, UINT16 offset)
{
	offs_t virtual_offset = 0x1fe000 | get_dma_address(index, offset);

	return space().read_byte(virtual_offset);
}


//-------------------------------------------------
//  dma_iorq_w - DMA I/O write
//-------------------------------------------------

void abc1600_mac_device::dma_iorq_w(int index, UINT16 offset, UINT8 data)
{
	offs_t virtual_offset = 0x1fe000 | get_dma_address(index, offset);

	space().write_byte(virtual_offset, data);
}


//-------------------------------------------------
//  dmamap_w - DMA map write
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mac_device::dmamap_w )
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

	if (LOG) logerror("DMAMAP %u %02x\n", offset & 7, data);

	m_dmamap[offset & 7] = data;
}
