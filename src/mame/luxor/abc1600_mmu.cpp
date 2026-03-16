// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Memory Access Controller emulation

**********************************************************************/

#include "emu.h"
#include "abc1600_mmu.h"

#define LOG_MAC    (1U << 1)
#define LOG_DMA    (1U << 2)
#define LOG_TASKS  (1U << 3)
#define LOG_ERRORS (1U << 4)

#define VERBOSE (LOG_MAC|LOG_TASKS)
#include "logmacro.h"

#define PAGE_WP     BIT(page_data, 14)
#define PAGE_NONX   BIT(page_data, 15)

DEFINE_DEVICE_TYPE(LUXOR_ABC1600_MMU, abc1600_mmu_device, "abc1600mac", "Luxor ABC 1600 MAC")

void abc1600_mmu_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1600)); // XTAL(64'000'000)/8/10/20000/8/8
}

ROM_START( abc1600_mac )
	ROM_REGION( 0x4000, "boot", 0 )
	ROM_LOAD( "boot 6490356-04.1f", 0x0000, 0x4000, CRC(9372f6f2) SHA1(86f0681f7ef8dd190b49eda5e781881582e0c2a4) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "1022 6490351-01.17e", 0x000, 0x104, CRC(5dd00d43) SHA1(a3871f0d796bea9df8f25d41b3169dd4b8ef65ab) ) // X36 SEGMENT-PAGE MAP R/W STROBE HANDLER
ROM_END

const tiny_rom_entry *abc1600_mmu_device::device_rom_region() const
{
	return ROM_NAME( abc1600_mac );
}

void abc1600_mmu_device::boot_map(address_map &map)
{
	map(0x00000, 0x03fff).mirror(0x1c000).rom().region("boot", 0);
}

void abc1600_mmu_device::mac_map(address_map &map)
{
	map(0x80000, 0x80000).mirror(0x006f8).select(0x7f900).rw(FUNC(abc1600_mmu_device::page_hi_r), FUNC(abc1600_mmu_device::page_hi_w));
	map(0x80001, 0x80001).mirror(0x006f8).select(0x7f900).rw(FUNC(abc1600_mmu_device::page_lo_r), FUNC(abc1600_mmu_device::page_lo_w));
	map(0x80002, 0x80002).mirror(0x7fff8).noprw();
	map(0x80003, 0x80003).mirror(0x07ef8).select(0x78100).rw(FUNC(abc1600_mmu_device::segment_r), FUNC(abc1600_mmu_device::segment_w));
	map(0x80004, 0x80004).mirror(0x7fff8).noprw();
	map(0x80005, 0x80005).mirror(0x7fff8).nopr().w(FUNC(abc1600_mmu_device::task_w));
	map(0x80006, 0x80006).mirror(0x7fff8).noprw();
	map(0x80007, 0x80007).mirror(0x7fff8).r(FUNC(abc1600_mmu_device::cause_r)).nopw();
}

abc1600_mmu_device::abc1600_mmu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
    device_t(mconfig, LUXOR_ABC1600_MMU, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_state_interface(mconfig, *this),
	m_read_tren(*this, 0xff),
	m_write_tren(*this),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_boot_config("boot", ENDIANNESS_BIG, 8, 17, 0, address_map_constructor(FUNC(abc1600_mmu_device::boot_map), this)),
	m_program_config("program", ENDIANNESS_BIG, 8, 21),
	m_cpu_space_config("cpu_space", ENDIANNESS_BIG, 8, 20),
	m_s_program_config("mac", ENDIANNESS_BIG, 8, 20, 0, address_map_constructor(FUNC(abc1600_mmu_device::mac_map), this)),
	m_watchdog(*this, "watchdog"),
	m_segment_ram(*this, "segment_ram", 0x400, ENDIANNESS_LITTLE),
	m_page_ram(*this, "page_ram", 0x400*2, ENDIANNESS_LITTLE)
{
}

abc1600_mmu_device::mmu::mmu(m68008_device *maincpu, address_space &boot, address_space &program, address_space &cpu_space, address_space &mac_space, u8 *segment_ram, u16 *page_ram) : 
    m_maincpu(maincpu), 
	m_segment_ram(segment_ram),
	m_page_ram(page_ram),
	m_a_boot(boot),
    m_a_program(program),
    m_a_cpu_space(cpu_space),
	m_a_mac_space(mac_space),
	m_super(false),
	m_boote(0),
	m_magic(0),
	m_task(0)
{
	boot.specific(m_boot);
	program.specific(m_program);
	cpu_space.specific(m_cpu_space);
	mac_space.specific(m_s_program);

	// HACK fill segment RAM or abcenix won't boot
	memset(m_segment_ram, 0xff, 0x400);
}

void abc1600_mmu_device::device_start()
{
	m_mmu = std::make_unique<mmu>(m_maincpu.target(), space(AS_BOOT), space(AS_PROGRAM), space(m68000_base_device::AS_CPU_SPACE), space(AS_MAC), m_segment_ram.target(), m_page_ram.target());
	m_maincpu->set_current_mmu8(m_mmu.get());

	state_add(0, "TASK", m_mmu->m_task).formatstr("%2u");
	state_add(1, "BOOTE", m_mmu->m_boote).formatstr("%1u");
	state_add(2, "MAGIC", m_mmu->m_magic).formatstr("%1u");

	// state saving
	save_item(NAME(m_rstbut));
	save_item(NAME(m_partst));
	save_item(NAME(m_dmamap));
	save_item(NAME(m_mmu->m_super));
	save_item(NAME(m_mmu->m_boote));
	save_item(NAME(m_mmu->m_magic));
	save_item(NAME(m_mmu->m_task));
	save_item(NAME(m_mmu->m_cause));
}

void abc1600_mmu_device::device_reset()
{
	m_mmu->reset();
}

device_memory_interface::space_config_vector abc1600_mmu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_BOOT, &m_boot_config),
        std::make_pair(AS_PROGRAM, &m_program_config),
        std::make_pair(m68000_base_device::AS_CPU_SPACE, &m_cpu_space_config),
		std::make_pair(AS_MAC, &m_s_program_config)
	};
}

void abc1600_mmu_device::mmu::reset()
{
	m_boote = 0;
	m_magic = 0;
	m_task = 0;
}

offs_t abc1600_mmu_device::mmu::get_physical_address(offs_t logical, int task, bool &nonx, bool &wp)
{
	// segment
	offs_t sega = (task << 5) | ((logical >> 15) & 0x1f);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((logical >> 11) & 0x0f);
	uint16_t page_data = m_page_ram[pga];

	offs_t physical = ((page_data & 0x3ff) << 11) | (logical & 0x7ff);

	nonx = PAGE_NONX;
	wp = PAGE_WP;

	m_cause = ((logical >> 13) & 0x1f) | DMAOK;

	return physical;
}

bool abc1600_mmu_device::mmu::translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
 	if (spacenum == m68000_base_device::AS_CPU_SPACE) {
		target_space = &m_a_cpu_space;
		return true;
	}

    if (spacenum != AS_PROGRAM)
		return false;

	if (!m_boote && (intention != TR_WRITE) && (address < 0x20000)) {
	    target_space = &m_a_boot;
		return true;
	}

	if (m_super && (address & 0x80000)) {
		target_space = &m_a_mac_space;
		return true;
	}

	int task = m_task;
	if (m_super || (!m_magic && (intention == TR_FETCH)))
		task = 0;

	bool nonx, wp;
	address = get_physical_address(address, task, nonx, wp);

	if (nonx || ((intention == TR_WRITE) && !wp))
		return false;

	target_space = &m_a_program;
	return true;
}

u8 abc1600_mmu_device::mmu::mmu_read(offs_t logical, int intention)
{
	if (!m_boote && (logical < 0x20000))
	    return m_boot.read_interruptible(logical);

	if (m_super && (logical & 0x80000))
		return m_s_program.read_interruptible(logical);
	
	int task = m_task;
	if (m_super || (!m_magic && (intention == TR_FETCH)))
		task = 0;

	bool nonx, wp;
	offs_t physical = get_physical_address(logical, task, nonx, wp);

	if (nonx) {
		m_maincpu->trigger_bus_error();
		return 0xff;
	}

	return m_program.read_interruptible(physical);
}

void abc1600_mmu_device::mmu::mmu_write(offs_t logical, u8 data)
{
	if (m_super && (logical & 0x80000)) {
		m_s_program.write_interruptible(logical, data);
		return;
	}

	int task = m_task;
	if (m_super)
		task = 0;

	bool nonx, wp;
	offs_t physical = get_physical_address(logical, task, nonx, wp);

	if (nonx || !wp) {
		m_maincpu->trigger_bus_error();
		return;
	}

	m_program.write_interruptible(physical, data);
}

u8 abc1600_mmu_device::mmu::read_program(offs_t logical)
{
	return mmu_read(logical, TR_FETCH);
}

void abc1600_mmu_device::mmu::write_program(offs_t logical, u8 data)
{
	mmu_write(logical, data);
}

u8 abc1600_mmu_device::mmu::read_data(offs_t logical)
{
	return mmu_read(logical, TR_READ);
}

void abc1600_mmu_device::mmu::write_data(offs_t logical, u8 data)
{
	mmu_write(logical, data);
}

u8 abc1600_mmu_device::mmu::read_cpu(offs_t logical)
{
	return m_cpu_space.read_byte(logical);
}

void abc1600_mmu_device::mmu::set_super(bool super)
{
	m_super = super;
}

uint8_t abc1600_mmu_device::cause_r()
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

	uint8_t data = m_rstbut;

	if (!m_partst)
	{
		data = 0x02 | m_mmu->m_cause;
	}

	if (!machine().side_effects_disabled())
	{
		m_watchdog->watchdog_reset();
	}

	return data;
}

void abc1600_mmu_device::task_w(offs_t offset, uint8_t data)
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

	m_mmu->m_task = data & 0x0f;
	m_mmu->m_boote = !BIT(data, 6);
	m_mmu->m_magic = !BIT(data, 7);

	LOGMASKED(LOG_TASKS, "%s TASK W %02x (TASK %u BOOTE %u MAGIC %u)\n", machine().describe_context(), data, m_mmu->m_task, m_mmu->m_boote, m_mmu->m_magic);
}

uint8_t abc1600_mmu_device::segment_r(offs_t offset)
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

	offs_t sega = (m_mmu->m_task << 5) | BIT(offset, 8) << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	return (!m_mmu->m_magic << 7) | (segd & 0x7f);
}

void abc1600_mmu_device::segment_w(offs_t offset, uint8_t data)
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

	offs_t sega = (m_mmu->m_task << 5) | BIT(offset, 8) << 4 | ((offset >> 15) & 0xf);
	m_segment_ram[sega] = data & 0x7f;

	LOGMASKED(LOG_MAC, "%s SEG W %05x:%02x SEGA %03x SEGD %02x TASK %u\n", machine().describe_context(), offset, data, sega, m_segment_ram[sega], m_mmu->m_task);
}

uint8_t abc1600_mmu_device::page_lo_r(offs_t offset)
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
	offs_t sega = (m_mmu->m_task << 5) | BIT(offset, 8) << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	uint16_t pgd = m_page_ram[pga];

	return pgd & 0xff;
}

uint8_t abc1600_mmu_device::page_hi_r(offs_t offset)
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
	offs_t sega = (m_mmu->m_task << 5) | BIT(offset, 8) << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	uint16_t pgd = m_page_ram[pga];

	int x20 = BIT(pgd, 9);

	return (pgd >> 8) | (x20 << 2) | (x20 << 3) | (x20 << 4) | (x20 << 5);
}

void abc1600_mmu_device::page_lo_w(offs_t offset, uint8_t data)
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
	offs_t sega = (m_mmu->m_task << 5) | BIT(offset, 8) << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	m_page_ram[pga] = (m_page_ram[pga] & 0xff00) | data;

	LOGMASKED(LOG_MAC, "%s PAGE W %05x:%02x (SEGA %03x SEGD %02x PGA %03x PGD %04x TASK %u)\n", machine().describe_context(), offset, data, sega, segd, pga, m_page_ram[pga], m_mmu->m_task);
}

void abc1600_mmu_device::page_hi_w(offs_t offset, uint8_t data)
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
	offs_t sega = (m_mmu->m_task << 5) | BIT(offset, 8) << 4 | ((offset >> 15) & 0xf);
	uint8_t segd = m_segment_ram[sega];

	// page
	offs_t pga = ((segd & 0x3f) << 4) | ((offset >> 11) & 0xf);
	m_page_ram[pga] = ((data & 0xc3) << 8) | (m_page_ram[pga] & 0xff);
}

offs_t abc1600_mmu_device::get_dma_address(int index, offs_t logical, bool &rw)
{
	/*
	            BA2 BA1 A15
	    DMA0     1   1   x
	    DMA1     1   0   x
	    DMA2     0   0   x
	*/

	uint8_t dmamap_addr = index | BIT(logical, 15);
	uint8_t dmamap = m_dmamap[dmamap_addr];

	m_mmu->m_cause = (dmamap & 0x1f) << 3;

	rw = BIT(dmamap, 7);

	return ((dmamap & 0x1f) << 16) | logical;
}

uint8_t abc1600_mmu_device::dma_mreq_r(int index, int dmamap, offs_t logical)
{
	bool rw;
	offs_t physical = get_dma_address(dmamap, logical, rw);

	uint8_t data = 0xff;

	if (rw)
	{
		data = space(AS_PROGRAM).read_byte(physical);

		m_write_tren[index](data);
	}
	else
	{
		data = m_read_tren[index](data);

		space(AS_PROGRAM).write_byte(physical, data);
	}

	LOGMASKED(LOG_DMA, "%s DMRQ R:%c %04x:%06x=%02x\n", machine().describe_context(), rw ? 'R' : 'W', logical, physical, data);

	return data;
}

void abc1600_mmu_device::dma_mreq_w(int index, int dmamap, offs_t logical, uint8_t data)
{
	bool rw;
	offs_t physical = get_dma_address(dmamap, logical, rw);

	LOGMASKED(LOG_DMA, "%s DMRQ W:%c %04x:%06x=%02x\n", machine().describe_context(), rw ? 'R' : 'W', logical, physical, data);

	if (!rw)
	{
		space().write_byte(physical, data);
	}
}

uint8_t abc1600_mmu_device::dma_iorq_r(int dmamap, offs_t logical)
{
	uint8_t data = 0xff;
	bool rw;
	offs_t physical = 0x1fe000 | (get_dma_address(dmamap, logical, rw) & 0x1fff);

	LOGMASKED(LOG_DMA, "%s DIORQ R %04x:%06x\n", machine().describe_context(), logical, physical);

	if ((logical & 0x1800) == 0x1000)
	{
		data = space(AS_PROGRAM).read_byte(physical);
	}

	return data;
}

void abc1600_mmu_device::dma_iorq_w(int dmamap, offs_t logical, uint8_t data)
{
	bool rw;
	offs_t physical = 0x1fe000 | (get_dma_address(dmamap, logical, rw) & 0x1fff);

	LOGMASKED(LOG_DMA, "%s DIORQ W %04x:%06x=%02x\n", machine().describe_context(), logical, physical, data);

	if ((logical & 0x1800) == 0x1000)
	{
		space(AS_PROGRAM).write_byte(physical, data);
	}
}

void abc1600_mmu_device::dmamap_w(offs_t offset, uint8_t data)
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

	LOGMASKED(LOG_DMA, "%s DMAMAP %u:%02x\n", machine().describe_context(), offset & 7, data);

	m_dmamap[offset & 7] = data;
}
