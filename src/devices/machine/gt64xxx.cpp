// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Ted Green
#include "emu.h"
#include "gt64xxx.h"

#define LOG_GENERAL         (1U << 0)
#define LOG_GALILEO         (1U << 1)
#define LOG_TIMERS          (1U << 2)
#define LOG_DMA             (1U << 3)
#define LOG_PCI             (1U << 4)
#define LOG_IRQ             (1U << 5)

//#define VERBOSE (LOG_GENERAL | LOG_GALILEO | LOG_TIMERS | LOG_DMA | LOG_PCI)
#include "logmacro.h"

#define LOGGALILEO(...)     LOGMASKED(LOG_GALILEO, __VA_ARGS__)
#define LOGTIMERS(...)      LOGMASKED(LOG_TIMERS, __VA_ARGS__)
#define LOGDMA(...)         LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGPCI(...)         LOGMASKED(LOG_PCI, __VA_ARGS__)
#define LOGIRQ(...)         LOGMASKED(LOG_IRQ, __VA_ARGS__)


//************************************
//
//  Galileo constants
//
//************************************

#define TIMER_PERIOD        attotime::from_hz(clock())
#define PCI_BUS_CLOCK       33000000
// Number of dma words (32 bits) to transfer at a time, real hardware configurable between 8-32
#define DMA_BURST_SIZE      32
// DMA will transfer up to DMA_BURST_SIZE*4 bytes every DMA_TIMER_PERIOD seconds
#define DMA_TIMER_PERIOD    attotime::from_hz(PCI_BUS_CLOCK / 64)

// Galileo registers - 0x000-0x3ff
#define GREG_CPU_CONFIG     (0x000/4)
#define GREG_R1_0_LO        (0x008/4)
#define GREG_R1_0_HI        (0x010/4)
#define GREG_R3_2_LO        (0x018/4)
#define GREG_R3_2_HI        (0x020/4)
#define GREG_CS_2_0_LO      (0x028/4)
#define GREG_CS_2_0_HI      (0x030/4)
#define GREG_CS_3_BOOT_LO   (0x038/4)
#define GREG_CS_3_BOOT_HI   (0x040/4)
#define GREG_PCI_IO_LO      (0x048/4)
#define GREG_PCI_IO_HI      (0x050/4)
#define GREG_PCI_MEM0_LO    (0x058/4)
#define GREG_PCI_MEM0_HI    (0x060/4)
#define GREG_INTERNAL_SPACE (0x068/4)
#define GREG_BUSERR_LO      (0x070/4)
#define GREG_BUSERR_HI      (0x078/4)
// GT-64111 only
#define GREG_PCI_MEM1_LO    (0x080/4)
#define GREG_PCI_MEM1_HI    (0x088/4)

// Galileo registers - 0x400-0x7ff
#define GREG_RAS0_LO        (0x400/4)
#define GREG_RAS0_HI        (0x404/4)
#define GREG_RAS1_LO        (0x408/4)
#define GREG_RAS1_HI        (0x40c/4)
#define GREG_RAS2_LO        (0x410/4)
#define GREG_RAS2_HI        (0x414/4)
#define GREG_RAS3_LO        (0x418/4)
#define GREG_RAS3_HI        (0x41c/4)
#define GREG_CS0_LO         (0x420/4)
#define GREG_CS0_HI         (0x424/4)
#define GREG_CS1_LO         (0x428/4)
#define GREG_CS1_HI         (0x42c/4)
#define GREG_CS2_LO         (0x430/4)
#define GREG_CS2_HI         (0x434/4)
#define GREG_CS3_LO         (0x438/4)
#define GREG_CS3_HI         (0x43c/4)
#define GREG_CSBOOT_LO      (0x440/4)
#define GREG_CSBOOT_HI      (0x444/4)
#define GREG_DRAM_CONFIG    (0x448/4)
#define GREG_DRAM_BANK0     (0x44c/4)
#define GREG_DRAM_BANK1     (0x450/4)
#define GREG_DRAM_BANK2     (0x454/4)
#define GREG_DRAM_BANK3     (0x458/4)
#define GREG_DEVICE_BANK0   (0x45c/4)
#define GREG_DEVICE_BANK1   (0x460/4)
#define GREG_DEVICE_BANK2   (0x464/4)
#define GREG_DEVICE_BANK3   (0x468/4)
#define GREG_DEVICE_BOOT    (0x46c/4)
#define GREG_ADDRESS_ERROR  (0x470/4)

// Galileo registers - 0x800-0xbff
#define GREG_DMA0_COUNT     (0x800/4)
#define GREG_DMA1_COUNT     (0x804/4)
#define GREG_DMA2_COUNT     (0x808/4)
#define GREG_DMA3_COUNT     (0x80c/4)
#define GREG_DMA0_SOURCE    (0x810/4)
#define GREG_DMA1_SOURCE    (0x814/4)
#define GREG_DMA2_SOURCE    (0x818/4)
#define GREG_DMA3_SOURCE    (0x81c/4)
#define GREG_DMA0_DEST      (0x820/4)
#define GREG_DMA1_DEST      (0x824/4)
#define GREG_DMA2_DEST      (0x828/4)
#define GREG_DMA3_DEST      (0x82c/4)
#define GREG_DMA0_NEXT      (0x830/4)
#define GREG_DMA1_NEXT      (0x834/4)
#define GREG_DMA2_NEXT      (0x838/4)
#define GREG_DMA3_NEXT      (0x83c/4)
#define GREG_DMA0_CONTROL   (0x840/4)
#define GREG_DMA1_CONTROL   (0x844/4)
#define GREG_DMA2_CONTROL   (0x848/4)
#define GREG_DMA3_CONTROL   (0x84c/4)
#define GREG_TIMER0_COUNT   (0x850/4)
#define GREG_TIMER1_COUNT   (0x854/4)
#define GREG_TIMER2_COUNT   (0x858/4)
#define GREG_TIMER3_COUNT   (0x85c/4)
#define GREG_DMA_ARBITER    (0x860/4)
#define GREG_TIMER_CONTROL  (0x864/4)

// Galileo registers - 0xc00-0xfff
#define GREG_PCI_COMMAND    (0xc00/4)
#define GREG_PCI_TIMEOUT    (0xc04/4)
#define GREG_PCI_R1_0       (0xc08/4)
#define GREG_PCI_R3_2       (0xc0c/4)
#define GREG_PCI_CS_2_0     (0xc10/4)
#define GREG_PCI_CS_3_BOOT  (0xc14/4)
#define GREG_INTR_CAUSE     (0xc18/4)
#define GREG_CPU_MASK       (0xc1c/4)
#define GREG_PCI_MASK       (0xc24/4)
#define GREG_CONFIG_ADDRESS (0xcf8/4)
#define GREG_CONFIG_DATA    (0xcfc/4)

// Galileo interrupts
#define GINT_SUMMARY_SHIFT  (0)
#define GINT_MEMOUT_SHIFT   (1)
#define GINT_DMAOUT_SHIFT   (2)
#define GINT_CPUOUT_SHIFT   (3)
#define GINT_DMA0COMP_SHIFT (4)
#define GINT_DMA1COMP_SHIFT (5)
#define GINT_DMA2COMP_SHIFT (6)
#define GINT_DMA3COMP_SHIFT (7)
#define GINT_T0EXP_SHIFT    (8)
#define GINT_T1EXP_SHIFT    (9)
#define GINT_T2EXP_SHIFT    (10)
#define GINT_T3EXP_SHIFT    (11)
#define GINT_MASRDERR_SHIFT (12)
#define GINT_SLVWRERR_SHIFT (13)
#define GINT_MASWRERR_SHIFT (14)
#define GINT_SLVRDERR_SHIFT (15)
#define GINT_ADDRERR_SHIFT  (16)
#define GINT_MEMERR_SHIFT   (17)
#define GINT_MASABORT_SHIFT (18)
#define GINT_TARABORT_SHIFT (19)
#define GINT_RETRYCTR_SHIFT (20)


DEFINE_DEVICE_TYPE(GT64010, gt64010_device, "gt64010", "Galileo GT-64010 System Controller")
DEFINE_DEVICE_TYPE(GT64111, gt64111_device, "gt64111", "Galileo GT-64111 System Controller")

void gt64xxx_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

// cpu i/f map
void gt64xxx_device::cpu_map(address_map &map)
{
	map(0x00000000, 0x00000cff).rw(FUNC(gt64xxx_device::cpu_if_r), FUNC(gt64xxx_device::cpu_if_w));
}

gt64xxx_device::gt64xxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG), m_be(0), m_autoconfig(0), m_irq_num(-1)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_romRegion(*this, "rom")
	, m_updateRegion(*this, "update")
{
	for (int csIndex = 0; csIndex < 4; csIndex++) {
		m_cs_devices[csIndex] = nullptr;
		m_simm_size[csIndex] = 0;
	}
}

void gt64xxx_device::set_map(int id, const address_map_constructor &map, device_t *device)
{
	m_cs_devices[id] = device;
	m_cs_maps[id] = map;
}

device_memory_interface::space_config_vector gt64xxx_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void gt64xxx_device::device_start()
{
	pci_host_device::device_start();
	m_cpu_space = &m_cpu->space(AS_PCI_CONFIG);
	memory_space = &space(AS_PCI_MEM);
	io_space = &space(AS_PCI_IO);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffffffff;
	io_offset       = 0x00000000;
	status = 0x0;

	// DMA timer
	m_dma_timer = timer_alloc(FUNC(gt64xxx_device::perform_dma), this);
	// Leave the timer disabled.
	m_dma_timer->adjust(attotime::never, 0, DMA_TIMER_PERIOD);

	// Reserve RAM
	for (int i=0; i<4; i++)
		m_ram[i].resize(m_simm_size[i] / 4);

	// ROM
	uint32_t romSize = m_romRegion->bytes();
	m_cpu_space->install_rom   (0x1fc00000, 0x1fc00000 + romSize - 1, m_romRegion->base());
	// ROM MIPS DRC
	m_cpu->add_fastram(0x1fc00000, 0x1fc00000 + romSize - 1, true, m_romRegion->base());
	LOGGALILEO("gt64xxx_device::device_start ROM Mapped size: 0x%08X start: 0x1fc00000 end: %08X\n", romSize, 0x1fc00000 + romSize - 1);

	// Update region address is based on seattle driver
	if (m_updateRegion) {
		romSize = m_updateRegion->bytes();
		m_cpu_space->install_rom(0x1fd00000, 0x1fd00000 + romSize - 1, m_updateRegion->base());
		LOGGALILEO("gt64xxx_device::device_start UPDATE Mapped size: 0x%08X start: 0x1fd00000 end: %08X\n", romSize, 0x1fd00000 + romSize - 1);
	}

	// allocate timers for the galileo
	m_timer[0].timer = timer_alloc(FUNC(gt64xxx_device::timer_callback), this);
	m_timer[1].timer = timer_alloc(FUNC(gt64xxx_device::timer_callback), this);
	m_timer[2].timer = timer_alloc(FUNC(gt64xxx_device::timer_callback), this);
	m_timer[3].timer = timer_alloc(FUNC(gt64xxx_device::timer_callback), this);

	// Save states
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_pci_stall_state));
	save_item(NAME(m_retry_count));
	save_item(NAME(m_pci_cpu_stalled));
	save_item(NAME(m_stall_windex));
	save_item(NAME(m_cpu_stalled_offset));
	save_item(NAME(m_cpu_stalled_data));
	save_item(NAME(m_cpu_stalled_mem_mask));
	save_item(NAME(m_prev_addr));
	save_item(NAME(m_reg));
	save_item(STRUCT_MEMBER(m_timer, active));
	save_item(STRUCT_MEMBER(m_timer, count));
	save_item(NAME(m_dma_active));
	// m_ram[4]
	save_pointer(NAME(m_ram[0].data()), m_simm_size[0] / 4);
	save_pointer(NAME(m_ram[1].data()), m_simm_size[1] / 4);
	save_pointer(NAME(m_ram[2].data()), m_simm_size[2] / 4);
	save_pointer(NAME(m_ram[3].data()), m_simm_size[3] / 4);
	save_item(NAME(m_last_dma));
}

void gt64xxx_device::device_post_load()
{
	map_cpu_space();
	remap_cb();
}

void gt64xxx_device::device_reset()
{
	pci_device::device_reset();

	// Configuration register defaults
	m_reg[GREG_CPU_CONFIG] = m_be ? 0 : (1<<12);
	m_reg[GREG_R1_0_LO] = 0x0;
	m_reg[GREG_R1_0_HI] = 0x7;
	m_reg[GREG_R3_2_LO] = 0x8;
	m_reg[GREG_R3_2_HI] = 0xf;
	m_reg[GREG_CS_2_0_LO] = 0xe0;
	m_reg[GREG_CS_2_0_HI] = 0x70;
	m_reg[GREG_CS_3_BOOT_LO] = 0xf8;
	m_reg[GREG_CS_3_BOOT_HI] = 0x7f;
	m_reg[GREG_PCI_IO_LO] = 0x80;
	m_reg[GREG_PCI_IO_HI] = 0xf;
	m_reg[GREG_PCI_MEM0_LO] = 0x90;
	m_reg[GREG_PCI_MEM0_HI] = 0x1f;
	m_reg[GREG_INTERNAL_SPACE] = 0xa0;
	m_reg[GREG_PCI_MEM1_LO] = 0x790;
	m_reg[GREG_PCI_MEM1_HI] = 0x1f;

	m_reg[GREG_RAS0_LO] = 0x0;
	m_reg[GREG_RAS0_HI] = 0x7;
	m_reg[GREG_RAS1_LO] = 0x8;
	m_reg[GREG_RAS1_HI] = 0xf;
	m_reg[GREG_RAS2_LO] = 0x10;
	m_reg[GREG_RAS2_HI] = 0x17;
	m_reg[GREG_RAS3_LO] = 0x18;
	m_reg[GREG_RAS3_HI] = 0x1f;
	m_reg[GREG_CS0_LO] = 0xc0;
	m_reg[GREG_CS0_HI] = 0xc7;
	m_reg[GREG_CS1_LO] = 0xc8;
	m_reg[GREG_CS1_HI] = 0xcf;
	m_reg[GREG_CS2_LO] = 0xd0;
	m_reg[GREG_CS2_HI] = 0xdf;
	m_reg[GREG_CS3_LO] = 0xf0;
	m_reg[GREG_CS3_HI] = 0xfb;
	m_reg[GREG_CSBOOT_LO] = 0xfc;
	m_reg[GREG_CSBOOT_HI] = 0xff;

	m_reg[GREG_PCI_COMMAND] = m_be ? 0 : 1;

	map_cpu_space();
	regenerate_config_mapping();

	m_irq_pending = 0;
	m_irq_state = CLEAR_LINE;
	m_pci_stall_state = 0;
	m_retry_count = 0;
	m_pci_cpu_stalled = 0;
	m_stall_windex = 0;

	m_dma_active = 0;
	m_dma_timer->adjust(attotime::never);
	m_last_dma = 0;

	m_prev_addr = 0;
}

void gt64xxx_device::map_cpu_space()
{
	uint32_t winStart, winEnd;

	// ROM region starts at 0x1fc00000
	m_cpu_space->unmap_readwrite(0x00000000, 0x1fbfffff);
	m_cpu_space->unmap_readwrite(0x20000000, 0xffffffff);

	// Clear fastram regions in cpu after rom
	m_cpu->clear_fastram(1);

	// CPU Regs
	winStart = m_reg[GREG_INTERNAL_SPACE]<<21;
	winEnd = winStart + sizeof(m_reg) - 1;
	m_cpu_space->install_device(winStart, winEnd, *static_cast<gt64xxx_device *>(this), &gt64xxx_device::cpu_map);
	logerror("map_cpu_space cpu_reg start: %08X end: %08X\n", winStart, winEnd);

	// RAS[0:3]
	for (int ramIndex = 0; ramIndex < 4; ++ramIndex)
	{
		winStart = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex/2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
		winEnd = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
		// Cap window end at physical memory bounds
		uint32_t winSize = winEnd - winStart + 1;
		if (winSize > m_ram[ramIndex].size() * 4)
			winEnd = winStart + m_ram[ramIndex].size() * 4 - 1;
		//m_ram[ramIndex].resize((winEnd + 1 - winStart) / 4);
		if (m_ram[ramIndex].size()>0)
			m_cpu_space->install_ram(winStart, winEnd, m_ram[ramIndex].data());
		//m_cpu->add_fastram(winStart, m_ram[ramIndex].size() * sizeof(m_ram[ramIndex][0]), false, &m_ram[ramIndex][0]);
		//m_cpu->add_fastram(winStart, m_ram[ramIndex].size() * sizeof(uint32_t), false, m_ram[ramIndex].data());
		logerror("map_cpu_space ras[%i] start: %08X end: %08X\n", ramIndex, winStart, winEnd);
		//printf("%s: map_cpu_space ras[%i] start: %08X end: %08X size: %08X\n", tag(), ramIndex, winStart, winEnd, winEnd-winStart+1);
	}

	// CS[0:3]
	for (int csIndex = 0; csIndex < 4; ++csIndex)
	{
		winStart = (m_reg[GREG_CS_2_0_LO + 0x10 / 4 * (csIndex / 3)] << 21) | (m_reg[GREG_CS0_LO + 0x8 / 4 * csIndex] << 20);
		winEnd = (m_reg[GREG_CS_2_0_LO + 0x10 / 4 * (csIndex / 3)] << 21) | (m_reg[GREG_CS0_HI + 0x8 / 4 * csIndex] << 20) | 0xfffff;
		m_cpu_space->install_device_delegate(winStart, winEnd, *m_cs_devices[csIndex], m_cs_maps[csIndex]);
		logerror("map_cpu_space cs[%i] start: %08X end: %08X\n", csIndex, winStart, winEnd);
	}

	// PCI IO Window
	winStart = m_reg[GREG_PCI_IO_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_IO_LO]<<21) | (m_reg[GREG_PCI_IO_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::master_io_r)));
	m_cpu_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::master_io_w)));
	logerror("map_cpu_space pci_io start: %08X end: %08X\n", winStart, winEnd);

	// PCI MEM0 Window
	winStart = m_reg[GREG_PCI_MEM0_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_MEM0_LO]<<21) | (m_reg[GREG_PCI_MEM0_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::master_mem0_r)));
	m_cpu_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::master_mem0_w)));
	logerror("map_cpu_space pci_mem0 start: %08X end: %08X\n", winStart, winEnd);

	// PCI MEM1 Window
	winStart = m_reg[GREG_PCI_MEM1_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_MEM1_LO]<<21) | (m_reg[GREG_PCI_MEM1_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::master_mem1_r)));
	m_cpu_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::master_mem1_w)));
	logerror("map_cpu_space pci_mem1 start: %08X end: %08X\n", winStart, winEnd);

	// Setup the address mapping table for DMA lookups
	for (size_t index = 0; index < proc_addr_bank::ADDR_NUM; ++index)
	{
		if (index < proc_addr_bank::ADDR_PCI_MEM1) {
			dma_addr_map[index].low_addr = (m_reg[GREG_R1_0_LO + 0x10 / 4 * index] << 21);
			dma_addr_map[index].high_addr = (dma_addr_map[index].low_addr & 0xf0000000) | (m_reg[GREG_R1_0_HI + 0x10 / 4 * index] << 21) | 0x1fffff;
		}
		else {
			dma_addr_map[index].low_addr = (m_reg[GREG_PCI_MEM1_LO] << 21);
			dma_addr_map[index].high_addr = (dma_addr_map[index].low_addr & 0xf0000000) | (m_reg[GREG_PCI_MEM1_HI] << 21) | 0x1fffff;
		}

	switch (index) {
		case proc_addr_bank::ADDR_PCI_IO:
			dma_addr_map[index].space = &this->space(AS_PCI_IO);
			break;
		case proc_addr_bank::ADDR_PCI_MEM0:
		case proc_addr_bank::ADDR_PCI_MEM1:
			dma_addr_map[index].space = &this->space(AS_PCI_MEM);
			break;
		default:
			dma_addr_map[index].space = m_cpu_space;
			break;
		}
	}
}

void gt64xxx_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	int ramIndex;
	uint32_t winStart, winEnd, winSize;

	// Not sure if GREG_R1_0_LO should be added on PCI address map side.
	// RAS0
	ramIndex = 0;
	winStart = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::ras_0_r)));
	memory_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::ras_0_w)));
	LOGGALILEO("map_extra RAS0 start=%08X end=%08X size=%08X\n", winStart, winEnd, winSize);

	// RAS1
	ramIndex = 1;
	winStart = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::ras_1_r)));
	memory_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::ras_1_w)));
	LOGGALILEO("map_extra RAS1 start=%08X end=%08X size=%08X\n", winStart, winEnd, winSize);

	// RAS2
	ramIndex = 2;
	winStart = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::ras_2_r)));
	memory_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::ras_2_w)));
	LOGGALILEO("map_extra RAS2 start=%08X end=%08X size=%08X\n", winStart, winEnd, winSize);

	// RAS3
	ramIndex = 3;
	winStart = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_R1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(gt64xxx_device::ras_3_r)));
	memory_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(gt64xxx_device::ras_3_w)));
	LOGGALILEO("map_extra RAS3 start=%08X end=%08X size=%08X\n", winStart, winEnd, winSize);
}

void gt64xxx_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();
}

// PCI Stalling
WRITE_LINE_MEMBER(gt64xxx_device::pci_stall)
{
	// Reset the retry count once unstalled
	if (state==0 && m_pci_stall_state==1) {
		m_retry_count = 0;
		// Check if it is a stalled cpu access and re-issue
		if (m_pci_cpu_stalled) {
			m_pci_cpu_stalled = 0;
			int index = 0;
			// Should actually check for a stall after each write...
			while (m_stall_windex > 0) {
				// master_mem0_w -- Should actually be checking for master_mem1_w as well
				this->space(AS_PCI_MEM).write_dword((m_reg[GREG_PCI_MEM0_LO] << 21) | (m_cpu_stalled_offset[index] * 4),
					m_cpu_stalled_data[index], m_cpu_stalled_mem_mask[index]);
				LOGGALILEO("pci_stall: Writing index: %d offset: %08x data: %08x mask: %08x\n",
					index, m_cpu_stalled_offset[index] * 4, m_cpu_stalled_data[index], m_cpu_stalled_mem_mask[index]);
				m_stall_windex--;
				index++;
			}
			// resume CPU execution
			machine().scheduler().trigger(45678);
			LOGMASKED(LOG_GALILEO | LOG_PCI | LOG_DMA, "Resuming CPU on PCI Stall\n");
		}
	}

	// set the new state
	m_pci_stall_state = state;
}

// PCI bus control
uint32_t gt64xxx_device::pci_config_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	LOGGALILEO("%s galileo pci_config_r from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
void gt64xxx_device::pci_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGGALILEO("%s galileo pci_config_w to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}
// PCI Master Window 0
uint32_t gt64xxx_device::master_mem0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = this->space(AS_PCI_MEM).read_dword((m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), mem_mask);
	LOGPCI("%s galileo pci mem0 read from offset %08X = %08X & %08X\n", machine().describe_context(), (m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
void gt64xxx_device::master_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_pci_stall_state) {
		if (m_stall_windex < 2) {
			// Save the write data and stall the cpu
			// For some reason sometimes two writes get through before the cpu is stalled (calspeed) so need to store multiple accesses.
			m_pci_cpu_stalled = 1;
			m_cpu_stalled_offset[m_stall_windex] = offset;
			m_cpu_stalled_data[m_stall_windex] = data;
			m_cpu_stalled_mem_mask[m_stall_windex] = mem_mask;
			m_stall_windex++;
			// Stall cpu until trigger
			m_cpu_space->device().execute().spin_until_trigger(45678);
			LOGMASKED(LOG_GALILEO | LOG_PCI | LOG_DMA, "%s Stalling CPU on PCI Stall\n", machine().describe_context());
		}
		else {
			fatalerror("master_mem0_w: m_stall_windex full\n");
		}
		return;
	}
	this->space(AS_PCI_MEM).write_dword((m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), data, mem_mask);
	LOGPCI("%s galileo pci mem0 write to offset %08X = %08X & %08X\n", machine().describe_context(), (m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), data, mem_mask);
}

// PCI Master Window 1
uint32_t gt64xxx_device::master_mem1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = this->space(AS_PCI_MEM).read_dword((m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), mem_mask);
	LOGPCI("%s galileo pci mem1 read from offset %08X = %08X & %08X\n", machine().describe_context(), (m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
void gt64xxx_device::master_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	this->space(AS_PCI_MEM).write_dword((m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), data, mem_mask);
	LOGPCI("%s galileo pci mem1 write to offset %08X = %08X & %08X\n", machine().describe_context(), (m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), data, mem_mask);
}

// PCI Master IO
uint32_t gt64xxx_device::master_io_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = this->space(AS_PCI_IO).read_dword((m_reg[GREG_PCI_IO_LO] << 21) | (offset * 4), mem_mask);
	if (m_prev_addr != offset) {
		m_prev_addr = offset;
		LOGPCI("%s galileo pci io read from offset %08X = %08X & %08X\n", machine().describe_context(), (m_reg[GREG_PCI_IO_LO] << 21) | (offset * 4), result, mem_mask);
	}
	return result;
}
void gt64xxx_device::master_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	this->space(AS_PCI_IO).write_dword((m_reg[GREG_PCI_IO_LO] << 21) | (offset * 4), data, mem_mask);
	if (m_prev_addr != offset) {
		m_prev_addr = offset;
		LOGPCI("%s galileo pci io write to offset %08X = %08X & %08X\n", machine().describe_context(), (m_reg[GREG_PCI_IO_LO] << 21) | (offset * 4), data, mem_mask);
	}
}

uint32_t gt64xxx_device::ras_0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_ram[0][offset];
	LOGPCI("%s galileo ras_0 read from offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, result, mem_mask);
	return result;
}

void gt64xxx_device::ras_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ram[0][offset]);
	LOGPCI("%s galileo ras_0 write to offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}

uint32_t gt64xxx_device::ras_1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_ram[1][offset];
	LOGPCI("%s galileo ras_0 read from offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, result, mem_mask);
	return result;
}

void gt64xxx_device::ras_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ram[1][offset]);
	LOGPCI("%s galileo ras_0 write to offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}

uint32_t gt64xxx_device::ras_2_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_ram[2][offset];
	LOGPCI("%s galileo ras_0 read from offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, result, mem_mask);
	return result;
}

void gt64xxx_device::ras_2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ram[2][offset]);
	LOGPCI("%s galileo ras_0 write to offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}

uint32_t gt64xxx_device::ras_3_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_ram[3][offset];
	LOGPCI("%s galileo ras_0 read from offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, result, mem_mask);
	return result;
}

void gt64xxx_device::ras_3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ram[3][offset]);
	LOGPCI("%s galileo ras_0 write to offset %08X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}


// CPU I/F
uint32_t gt64xxx_device::cpu_if_r(offs_t offset)
{
	uint32_t result = m_reg[offset];

	// switch off the offset for special cases
	switch (offset)
	{
		case GREG_TIMER0_COUNT:
		case GREG_TIMER1_COUNT:
		case GREG_TIMER2_COUNT:
		case GREG_TIMER3_COUNT:
		{
			int which = offset % 4;
			galileo_timer *timer = &m_timer[which];

			result = timer->count;
			if (timer->active)
			{
				uint32_t elapsed = (timer->timer->elapsed() * clock()).as_double();
				result = (result > elapsed) ? (result - elapsed) : 0;
			}

			LOGTIMERS("%s hires_timer_r = %08X\n", machine().describe_context(), result);
			break;
		}

		case GREG_PCI_COMMAND:
			// code at 40188 loops until this returns non-zero in bit 0
			//result = 0x0001;
			// bit 0 => byte swap
			// bit 2:1 => SyncMode, 00 = PCLK=[0,33], 01 = PCLK>=TClk/2, 10 = PCLK = TCLK/2
			result = (result & ~0x1) | (m_be ^ 0x1);
			break;

		case GREG_CONFIG_DATA:
			result = config_data_r(offset);
			LOGGALILEO("%s Galileo GREG_CONFIG_DATA read from offset %03X = %08X\n", machine().describe_context(), offset*4, result);
			break;

		case GREG_CONFIG_ADDRESS:
			result = config_address_r();
			break;

		case GREG_INTR_CAUSE:
			LOGIRQ("%s Galileo GREG_INTR_CAUSE read from offset %03X = %08X\n", machine().describe_context(), offset * 4, result);
			break;

		case GREG_CPU_MASK:
			LOGGALILEO("%s Galileo GREG_CPU_MASK read from offset %03X = %08X\n", machine().describe_context(), offset*4, result);
			break;

		case GREG_TIMER_CONTROL:
			LOGTIMERS("%s Galileo read from offset %03X = %08X\n", machine().describe_context(), offset*4, result);
			break;

		default:
			LOGGALILEO("%s Galileo read from offset %03X = %08X\n", machine().describe_context(), offset*4, result);
			break;
	}

	if (m_be) result =  swapendian_int32(result);

	return result;
}

void gt64xxx_device::cpu_if_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_be) {
		data = swapendian_int32(data);
		mem_mask = swapendian_int32(mem_mask);
	}

	uint32_t oldata = m_reg[offset];
	COMBINE_DATA(&m_reg[offset]);

	// switch off the offset for special cases
	switch (offset)
	{
		case GREG_R1_0_LO:
		case GREG_R1_0_HI:
		case GREG_R3_2_LO:
		case GREG_R3_2_HI:
		case GREG_CS_2_0_LO:
		case GREG_CS_2_0_HI:
		case GREG_CS_3_BOOT_LO:
		case GREG_CS_3_BOOT_HI:
		case GREG_PCI_IO_LO:
		case GREG_PCI_IO_HI:
		case GREG_PCI_MEM0_LO:
		case GREG_PCI_MEM0_HI:
		case GREG_INTERNAL_SPACE:
		case GREG_PCI_MEM1_LO:
		case GREG_PCI_MEM1_HI:
		case GREG_CS3_HI:
			map_cpu_space();
			remap_cb();
			LOGGALILEO("%s Galileo Memory Map data write to offset %03X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;

		case GREG_DMA0_CONTROL:
		case GREG_DMA1_CONTROL:
		case GREG_DMA2_CONTROL:
		case GREG_DMA3_CONTROL:
		{
			int which = offset % 4;

			// keep the read only activity bit
			m_reg[offset] &= ~0x4000;
			m_reg[offset] |= (oldata & 0x4000);

			// fetch next record
			if (data & 0x2000)
				dma_fetch_next(space, which);
			m_reg[offset] &= ~0x2000;

			// if enabling, start the DMA
			if (!(oldata & 0x1000) && (data & 0x1000) && !(m_dma_active & (1<<which)))
			{
				// Trigger the timer if there are no dma's active
				if (m_dma_active==0)
					m_dma_timer->adjust(DMA_TIMER_PERIOD, 0, DMA_TIMER_PERIOD);
				m_dma_active |= (1<< which);
				//perform_dma(space, which);
				LOGDMA("%s Galileo starting DMA Chan %i\n", machine().describe_context(), which);
			}
			if ((oldata & 0x1000) && !(data & 0x1000) && (m_dma_active & (1 << which)))
			{
				m_dma_active &= ~(1 << which);
				// Turn off the timer
				m_dma_timer->adjust(attotime::never);
				LOGDMA("%s Galileo stopping DMA Chan %i\n", machine().describe_context(), which);
			}

			LOGGALILEO("%s Galileo write to offset %03X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
			break;
		}

		case GREG_TIMER0_COUNT:
		case GREG_TIMER1_COUNT:
		case GREG_TIMER2_COUNT:
		case GREG_TIMER3_COUNT:
		{
			int which = offset % 4;
			galileo_timer *timer = &m_timer[which];

			if (which != 0)
				data &= 0xffffff;
			if (!timer->active)
				timer->count = data;
			LOGTIMERS("%s timer/counter %d count = %08X [start=%08X]\n", machine().describe_context(), offset % 4, data, timer->count);
			break;
		}

		case GREG_TIMER_CONTROL:
		{
			int which, mask;

			LOGTIMERS("%s timer/counter control = %08X\n", machine().describe_context(), data);
			for (which = 0, mask = 0x01; which < 4; which++, mask <<= 2)
			{
				galileo_timer *timer = &m_timer[which];
				if (!timer->active && (data & mask))
				{
					timer->active = 1;
					if (timer->count == 0)
					{
						timer->count = m_reg[GREG_TIMER0_COUNT + which];
						if (which != 0)
							timer->count &= 0xffffff;
					}
					timer->timer->adjust(TIMER_PERIOD * timer->count, which);
					LOGTIMERS("Adjusted timer%d to fire in %f secs\n", which, (TIMER_PERIOD * timer->count).as_double());
				}
				else if (timer->active && !(data & mask))
				{
					uint32_t elapsed = (timer->timer->elapsed() * clock()).as_double();
					timer->active = 0;
					timer->count = (timer->count > elapsed) ? (timer->count - elapsed) : 0;
					timer->timer->adjust(attotime::never, which);
					LOGTIMERS("Disabled timer%d\n", which);
				}
			}
			break;
		}

		case GREG_INTR_CAUSE:
			LOGIRQ("%s Galileo GREG_INTR_CAUSE write to offset %03X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
			m_reg[offset] = oldata & data;
			update_irqs();
			break;

		case GREG_CPU_MASK:
			LOGGALILEO("%s Galileo GREG_CPU_MASK write to offset %03X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
			// Bits 0, 25:21, 31:30 are read only '0'
			m_reg[offset] &= 0x3c1ffffe;
			update_irqs();
			break;

		case GREG_CONFIG_DATA:
			pci_host_device::config_data_w(offset, data);
			LOGGALILEO("%s Galileo PCI config data write to offset %03X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;

		case GREG_CONFIG_ADDRESS:
			// Type 0 config transactions signalled by Bus Num = 0 and Device Num != 0
			// Bits 15:11 get mapped into device number for configuration
			uint32_t modData;
			if (0 && (data & 0xff0000) == 0x0 && (data & 0xf800)) {
				// Type 0 transaction
				modData = 0;
				// Select the device based on one hot bit
				for (int i = 11; i<16; i++) {
					if ((data >> i) & 0x1) {
						// One hot encoding, bit 11 will mean device 1
						modData = i - 10;
						break;
					}
				}
				// Re-organize into Type 1 transaction for bus 0 (local bus)
				modData = (modData << 11) | (data & 0x7ff) | (0x80000000);
			}
			else {
				// Type 1 transaction, no modification needed
				modData = data;
			}
			pci_host_device::config_address_w(offset, modData);
			LOGGALILEO("%s Galileo PCI config address write to offset %03X = %08X & %08X origData = %08X\n", machine().describe_context(), offset*4, modData, mem_mask, data);
			break;

		case GREG_DMA0_COUNT:   case GREG_DMA1_COUNT:   case GREG_DMA2_COUNT:   case GREG_DMA3_COUNT:
		case GREG_DMA0_SOURCE:  case GREG_DMA1_SOURCE:  case GREG_DMA2_SOURCE:  case GREG_DMA3_SOURCE:
		case GREG_DMA0_DEST:    case GREG_DMA1_DEST:    case GREG_DMA2_DEST:    case GREG_DMA3_DEST:
		case GREG_DMA0_NEXT:    case GREG_DMA1_NEXT:    case GREG_DMA2_NEXT:    case GREG_DMA3_NEXT:
			LOGGALILEO("%s Galileo write to offset %03X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;

		default:
			LOGGALILEO("%s Galileo write to offset %03X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}

//************************************
//
//  Galileo timers & interrupts
//
//************************************

void gt64xxx_device::update_irqs()
{
	// Set cause from pending only if current irq state is clear
	// seattle hyprdriv freezes (MT07568) if a DMA0 interrupt is sent while the processor is already in the exception handler
	if (!m_irq_state)
	{
		m_reg[GREG_INTR_CAUSE] = m_irq_pending;
		m_irq_pending = 0;
		// Set interrupt summary bit
		if (m_reg[GREG_INTR_CAUSE] & 0xfffffffe)
			m_reg[GREG_INTR_CAUSE] |= (1 << 0);
		else
			m_reg[GREG_INTR_CAUSE] &= ~(1 << 0);

		// set CPU interrupt summary of bits 29:26, 20:1
		if (m_reg[GREG_INTR_CAUSE] & m_reg[GREG_CPU_MASK] & 0x3c1ffffe)
			m_reg[GREG_INTR_CAUSE] |= (1 << 30);
		else
			m_reg[GREG_INTR_CAUSE] &= ~(1 << 30);
	}
	// if any unmasked interrupts are active, we generate
	int state = CLEAR_LINE;
	if (m_reg[GREG_INTR_CAUSE] & m_reg[GREG_CPU_MASK])
		state = ASSERT_LINE;

	if (m_irq_num != -1 && state != m_irq_state)
	{
		m_cpu->set_input_line(m_irq_num, state);
		m_irq_state = state;
		LOGIRQ("gt64xxx_device IRQ %s irqNum: %i cause = %08X mask = %08X time: %s\n", (state == ASSERT_LINE) ? "asserted" : "cleared", m_irq_num, m_reg[GREG_INTR_CAUSE], m_reg[GREG_CPU_MASK], machine().time().as_string());
	}

	// Run again if we cleared and there are new interrupts pending
	if (!state && m_irq_pending)
	{
		LOGIRQ("gt64xxx_device new irq pending %08x time: %s\n", m_irq_pending, machine().time().as_string());
		update_irqs();
	}
}

TIMER_CALLBACK_MEMBER(gt64xxx_device::timer_callback)
{
	int which = param;
	galileo_timer *timer = &m_timer[which];

	LOGTIMERS("timer%d fired at time %s\n", which, machine().time().as_string());

	// copy the start value from the registers
	timer->count = m_reg[GREG_TIMER0_COUNT + which];
	if (which != 0)
		timer->count &= 0xffffff;

	// if we're a timer, adjust the timer to fire again
	if (m_reg[GREG_TIMER_CONTROL] & (2 << (2 * which)))
	{
		// unsure what a 0-length timer should do, but it produces an infinite loop so guard against it
		u32 effcount = timer->count;
		if (effcount == 0)
			effcount = (which != 0) ? 0xffffff : 0xffffffff;
		timer->timer->adjust(TIMER_PERIOD * effcount, which);
	}
	else
		timer->active = timer->count = 0;

	// trigger the interrupt
	//m_reg[GREG_INTR_CAUSE] |= 1 << (GINT_T0EXP_SHIFT + which);
	m_irq_pending |= 1 << (GINT_T0EXP_SHIFT + which);
	update_irqs();
}

/*************************************
 *
 *  Galileo DMA handler
 *
 *************************************/
address_space* gt64xxx_device::dma_decode_address(uint32_t &addr)
{
	for (size_t index = 0; index < proc_addr_bank::ADDR_NUM; ++index)
	{
		if (addr >= dma_addr_map[index].low_addr && addr <= dma_addr_map[index].high_addr)
			return dma_addr_map[index].space;
	}
	return nullptr;
}

int gt64xxx_device::dma_fetch_next(address_space &space, int which)
{
	offs_t address = 0;
	uint32_t data;

	// no-op for unchained mode
	if (!(m_reg[GREG_DMA0_CONTROL + which] & 0x200))
		address = m_reg[GREG_DMA0_NEXT + which];

	// exit if we hit the end address
	if (address == 0)
	{
		m_reg[GREG_DMA0_CONTROL + which] &= ~0x5000;
		return 0;
	}

	// fetch the byte count
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_COUNT + which] = data;

	// fetch the source address
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_SOURCE + which] = data;

	// fetch the dest address
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_DEST + which] = data;

	// fetch the next record address
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_NEXT + which] = data;
	return 1;
}


TIMER_CALLBACK_MEMBER (gt64xxx_device::perform_dma)
{
	// Cycle through the channels
	int which = -1;
	for (int i = 1; i <= 4; i++)
	{
		which = (m_last_dma + i) % 4;
		if ((m_dma_active & (1 << which)) && (m_reg[GREG_DMA0_CONTROL + which] & 0x1000))
			break;

	}
	// Save which dma is processed for arbitration next time
	m_last_dma = which;

	if (which==-1)
	{
		logerror("gt64xxx_device::perform_dma Warning! DMA Timer called with no pending DMA. m_dma_active = %08X\n", m_dma_active);
	}
	else
	{
		offs_t srcaddr = m_reg[GREG_DMA0_SOURCE + which];
		offs_t dstaddr = m_reg[GREG_DMA0_DEST + which];
		uint32_t bytesleft = m_reg[GREG_DMA0_COUNT + which] & 0xffff;
		address_space* srcSpace = dma_decode_address(srcaddr);
		address_space* dstSpace = dma_decode_address(dstaddr);

		int srcinc, dstinc;

		m_reg[GREG_DMA0_CONTROL + which] |= 0x5000;

		// determine src/dst inc
		switch ((m_reg[GREG_DMA0_CONTROL + which] >> 2) & 3)
		{
			default:
			case 0:     srcinc = 1;     break;
			case 1:     srcinc = -1;    break;
			case 2:     srcinc = 0;     break;
		}
		switch ((m_reg[GREG_DMA0_CONTROL + which] >> 4) & 3)
		{
			default:
			case 0:     dstinc = 1;     break;
			case 1:     dstinc = -1;    break;
			case 2:     dstinc = 0;     break;
		}

		// check for pci stall
		if (m_pci_stall_state)
		{
			uint32_t configRetryCount = (m_reg[GREG_PCI_TIMEOUT] >> 16) & 0xff;
			m_retry_count++;
			if (m_retry_count < 4)
				LOGDMA("%s Stalling DMA on voodoo retry_count: %i max: %i time: %s\n", machine().describe_context(), m_retry_count, configRetryCount, machine().time().as_string());
			if (configRetryCount == 0)
			{
				// Almost infinite retries, but avoid hanging the machine
				if (configRetryCount == ~0x0)
					fatalerror("gt64xxx_device::perform_dma Error! PCI is hung. DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc);
			}
			else if (m_retry_count >= configRetryCount)
			{
				logerror("gt64xxx_device::perform_dma Error! Too many PCI retries. DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc);
				// Signal error and abort DMA
				m_dma_active &= ~(1 << which);
				m_retry_count = 0;
				// Turn off the timer
				m_dma_timer->adjust(attotime::never);
				// Set the RetryCtr interrupt
				m_irq_pending |= 1 << (GINT_DMA0COMP_SHIFT + which);
				update_irqs();
			}
			return;
		}

		// do the transfer
		LOGDMA("gt64xxx_device: Starting DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d time=%s\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc, machine().time().as_string());
		int burstCount = 0;
		while (bytesleft > 0 && burstCount < DMA_BURST_SIZE && !m_pci_stall_state)
		{
			if (bytesleft < 4)
			{
				dstSpace->write_byte(dstaddr, srcSpace->read_byte(srcaddr));
				srcaddr += srcinc;
				dstaddr += dstinc;
				bytesleft--;
			}
			else
			{
				dstSpace->write_dword(dstaddr, srcSpace->read_dword(srcaddr));
				srcaddr += srcinc * 4;
				dstaddr += dstinc * 4;
				bytesleft -= 4;
			}
			burstCount++;
		}
		// not verified, but seems logical these should be updated at the end
		m_reg[GREG_DMA0_SOURCE + which] = srcaddr;
		m_reg[GREG_DMA0_DEST + which] = dstaddr;
		m_reg[GREG_DMA0_COUNT + which] = (m_reg[GREG_DMA0_COUNT + which] & ~0xffff) | bytesleft;

		// Check if we are done this descriptor
		if (bytesleft == 0)
		{
			// byte count zero interrupt
			if (!(m_reg[GREG_DMA0_CONTROL + which] & (1 << 10)))
			{
				m_irq_pending |= 1 << (GINT_DMA0COMP_SHIFT + which);
				update_irqs();
			}

			// Fetch the next dma for this channel (to be performed next scheduled burst)
			if ((m_reg[GREG_DMA0_CONTROL + which] & (1 << 9)) || dma_fetch_next(*m_cpu_space, which) == 0)
			{
				LOGDMA("gt64xxx_device: Done DMA descriptors time: %s\n", machine().time().as_string());
				m_dma_active &= ~(1 << which);

				// Turn off the timer
				m_dma_timer->adjust(attotime::never);

				// no more descriptors interrupt (bit 10) in chained mode (not bit 9)
				if ((m_reg[GREG_DMA0_CONTROL + which] & (1 << 10)) && !(m_reg[GREG_DMA0_CONTROL + which] & (1 << 9)))
				{
					m_irq_pending |= 1 << (GINT_DMA0COMP_SHIFT + which);
					update_irqs();
				}
			}
		}
	}
}
