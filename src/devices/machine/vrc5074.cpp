// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "vrc5074.h"

#define LOG_NILE            (0)
#define LOG_NILE_IRQS       (0)
#define LOG_PCI             (0)
#define LOG_TIMERS          (0)
#define LOG_MAP             (0)
#define LOG_NILE_MASTER     (0)
#define LOG_NILE_TARGET     (0)
#define PRINTF_SERIAL       (0)

/* NILE 4 registers 0x000-0x0ff */
#define NREG_SDRAM0         (0x000/4)
#define NREG_SDRAM1         (0x008/4)
#define NREG_DCS2           (0x010/4)   /* SIO misc */
#define NREG_DCS3           (0x018/4)   /* ADC */
#define NREG_DCS4           (0x020/4)   /* CMOS */
#define NREG_DCS5           (0x028/4)   /* SIO */
#define NREG_DCS6           (0x030/4)   /* IOASIC */
#define NREG_DCS7           (0x038/4)   /* ethernet */
#define NREG_DCS8           (0x040/4)
#define NREG_PCIW0          (0x060/4)
#define NREG_PCIW1          (0x068/4)
#define NREG_INTCS          (0x070/4)
#define NREG_BOOTCS         (0x078/4)
#define NREG_CPUSTAT        (0x080/4)
#define NREG_INTCTRL        (0x088/4)
#define NREG_INTSTAT0       (0x090/4)
#define NREG_INTSTAT1       (0x098/4)
#define NREG_INTCLR         (0x0A0/4)
#define NREG_INTPPES        (0x0A8/4)
#define NREG_PCIERR         (0x0B8/4)
#define NREG_MEMCTRL        (0x0C0/4)
#define NREG_ACSTIME        (0x0C8/4)
#define NREG_CHKERR         (0x0D0/4)
#define NREG_PCICTRL        (0x0E0/4)
#define NREG_PCIARB         (0x0E8/4)
#define NREG_PCIINIT0       (0x0F0/4)
#define NREG_PCIINIT1       (0x0F8/4)

/* NILE 4 registers 0x100-0x1ff */
#define NREG_LCNFG          (0x100/4)
#define NREG_LCST2          (0x110/4)
#define NREG_LCST3          (0x118/4)
#define NREG_LCST4          (0x120/4)
#define NREG_LCST5          (0x128/4)
#define NREG_LCST6          (0x130/4)
#define NREG_LCST7          (0x138/4)
#define NREG_LCST8          (0x140/4)
#define NREG_DCSFN          (0x150/4)
#define NREG_DCSIO          (0x158/4)
#define NREG_BCST           (0x178/4)
#define NREG_DMACTRL0       (0x180/4)
#define NREG_DMASRCA0       (0x188/4)
#define NREG_DMADESA0       (0x190/4)
#define NREG_DMACTRL1       (0x198/4)
#define NREG_DMASRCA1       (0x1A0/4)
#define NREG_DMADESA1       (0x1A8/4)
#define NREG_T0CTRL         (0x1C0/4)
#define NREG_T0CNTR         (0x1C8/4)
#define NREG_T1CTRL         (0x1D0/4)
#define NREG_T1CNTR         (0x1D8/4)
#define NREG_T2CTRL         (0x1E0/4)
#define NREG_T2CNTR         (0x1E8/4)
#define NREG_T3CTRL         (0x1F0/4)
#define NREG_T3CNTR         (0x1F8/4)

/* NILE 4 registers 0x300-0x3ff */
#define NREG_UARTRBR        (0x00/4)
#define NREG_UARTTHR        (0x00/4)
#define NREG_UARTIER        (0x08/4)
#define NREG_UARTDLL        (0x00/4)
#define NREG_UARTDLM        (0x08/4)
#define NREG_UARTIIR        (0x10/4)
#define NREG_UARTFCR        (0x10/4)
#define NREG_UARTLCR        (0x18/4)
#define NREG_UARTMCR        (0x20/4)
#define NREG_UARTLSR        (0x28/4)
#define NREG_UARTMSR        (0x30/4)
#define NREG_UARTSCR        (0x38/4)

/* NILE 4 interrupts */
#define NINT_CPCE           (0)
#define NINT_CNTD           (1)
#define NINT_MCE            (2)
#define NINT_DMA            (3)
#define NINT_UART           (4)
#define NINT_WDOG           (5)
#define NINT_GPT            (6)
#define NINT_LBRTD          (7)
#define NINT_INTA           (8)
#define NINT_INTB           (9)
#define NINT_INTC           (10)
#define NINT_INTD           (11)
#define NINT_INTE           (12)
#define NINT_RESV           (13)
#define NINT_PCIS           (14)
#define NINT_PCIE           (15)

#define TIMER_PERIOD        attotime::from_hz(clock())

#define PCI_BUS_CLOCK        33000000
// Number of dma words to transfer at a time, real hardware bursts 8
#define DMA_BURST_SIZE       128
#define DMA_TIMER_PERIOD     attotime::from_hz(PCI_BUS_CLOCK / 32)

#define DMA_BUSY                0x80000000
#define DMA_INTEN               0x40000000
#define DMA_INTVLD              0x20000000
#define DMA_GO                  0x10000000
#define DMA_SUS                 0x08000000
#define DMA_DSTINC              0x04000000
#define DMA_SRCINC              0x02000000
#define DMA_RST                 0x01000000
#define DMA_BLK_SIZE            0x000fffff


DEFINE_DEVICE_TYPE(VRC5074, vrc5074_device, "vrc5074", "NEC VRC5074 System Controller")

void vrc5074_device::config_map(address_map &map)
{
	pci_bridge_device::config_map(map);
	map(0x00000018, 0x00000027).rw(FUNC(vrc5074_device::sdram_addr_r), FUNC(vrc5074_device::sdram_addr_w));
}

// cpu i/f map
void vrc5074_device::cpu_map(address_map &map)
{
	map(0x00000000, 0x000001ff).rw(FUNC(vrc5074_device::cpu_reg_r), FUNC(vrc5074_device::cpu_reg_w));
}

void vrc5074_device::serial_map(address_map &map)
{
	map(0x00000000, 0x0000003f).rw(FUNC(vrc5074_device::serial_r), FUNC(vrc5074_device::serial_w));
}

// Target Window 1 map
void vrc5074_device::target1_map(address_map &map)
{
	map(0x00000000, 0xFFFFFFFF).rw(FUNC(vrc5074_device::target1_r), FUNC(vrc5074_device::target1_w));
}

void vrc5074_device::device_add_mconfig(machine_config &config)
{
	NS16550(config, m_uart, DERIVED_CLOCK(1, 12));
	m_uart->out_int_callback().set(FUNC(vrc5074_device::uart_irq_callback));
	m_uart->out_tx_callback().set("ttys00", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("ttys00", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("ttys00", FUNC(rs232_port_device::write_rts));

	rs232_port_device &ttys00(RS232_PORT(config, "ttys00", default_rs232_devices, nullptr));
	ttys00.rxd_handler().set(m_uart, FUNC(ns16550_device::rx_w));
	ttys00.dcd_handler().set(m_uart, FUNC(ns16550_device::dcd_w));
	ttys00.cts_handler().set(m_uart, FUNC(ns16550_device::cts_w));
}

vrc5074_device::vrc5074_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_host_device(mconfig, VRC5074, tag, owner, clock),
	m_uart(*this, "uart"),
	m_cpu_space(nullptr), m_cpu(*this, finder_base::DUMMY_TAG),
	m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32),
	m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32),
	m_romRegion(*this, "rom"),
	m_updateRegion(*this, "update")
{
	set_ids_host(0x1033005a, 0x04, 0x00000000);

	for (int i = 0; i < 2; i++)
		m_sdram_size[i] = 0x0;

	for (int csIndex = 2; csIndex < 9; csIndex++) {
		m_cs_devices[csIndex - 2] = nullptr;
	}
}

void vrc5074_device::set_map(int id, const address_map_constructor &map, device_t *device)
{
	if (id < 2)
		fatalerror("set_map: chip select must be greater or equal to 2.\n");
	m_cs_maps[id - 2] = map;
	m_cs_devices[id - 2] = device;
}

device_memory_interface::space_config_vector vrc5074_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void vrc5074_device::device_start()
{
	pci_host_device::device_start();
	m_cpu_space = &m_cpu->space(AS_PROGRAM);
	memory_space = &space(AS_DATA);
	io_space = &space(AS_IO);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffffffff;
	io_offset       = 0x00000000;
	status = 0x0280;
	// Size SDRAM
	m_sdram[0].resize(m_sdram_size[0]);
	m_sdram[1].resize(m_sdram_size[1]);
	// ROM
	uint32_t romSize = m_romRegion->bytes();
	m_cpu_space->install_rom(0x1fc00000, 0x1fc00000 + romSize - 1, m_romRegion->base());
	// Update region address is based on vegas driver
	if (m_updateRegion) {
		romSize = m_updateRegion->bytes();
		m_cpu_space->install_rom(0x1fd00000, 0x1fd00000 + romSize - 1, m_updateRegion->base());
		if (LOG_NILE)
			logerror("%s: vrc5074_device::device_start UPDATE Mapped size: 0x%08X start: 0x1fd00000 end: %08X\n", tag(), romSize, 0x1fd00000 + romSize - 1);
	}
	// Nile cpu register mapppings
	m_cpu_space->install_device(0x1fa00000, 0x1fa001ff, *static_cast<vrc5074_device *>(this), &vrc5074_device::cpu_map);
	// PCI Configuration also mapped at 0x1fa00200
	m_cpu_space->install_device(0x1fa00200, 0x1fa002ff, *static_cast<vrc5074_device *>(this), &vrc5074_device::config_map);
	// Nile serial register mapppings
	m_cpu_space->install_device(0x1fa00300, 0x1fa0033f, *static_cast<vrc5074_device *>(this), &vrc5074_device::serial_map);

	// MIPS drc
	m_cpu->add_fastram(0x1fc00000, 0x1fcfffff, true, m_romRegion->base());

	// DMA timer
	m_dma_timer = timer_alloc(FUNC(vrc5074_device::dma_transfer), this);
	// Leave the timer disabled.
	m_dma_timer->adjust(attotime::never, 0, DMA_TIMER_PERIOD);
	/* allocate timers for the NILE */
	m_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate());
	m_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate());
	m_timer[2] = timer_alloc(FUNC(vrc5074_device::nile_timer_callback), this);
	m_timer[3] = timer_alloc(FUNC(vrc5074_device::nile_timer_callback), this);

	// Save states
	// m_sdram
	save_item(NAME(m_sdram[0]));
	if (m_sdram_size[1]) save_item(NAME(m_sdram[1]));
	save_item(NAME(m_cpu_regs));
	save_item(NAME(m_nile_irq_state));
	save_item(NAME(m_sdram_addr));
	save_item(NAME(m_uart_irq));
	save_item(NAME(m_irq_pins));
	save_item(NAME(m_timer_period));
}

void vrc5074_device::device_post_load()
{
	map_cpu_space();
	setup_pci_space();
	//remap_cb();
}

void vrc5074_device::device_reset()
{
	pci_device::device_reset();
	memset(m_cpu_regs, 0, sizeof(m_cpu_regs));
	m_nile_irq_state = 0;
	regenerate_config_mapping();
	m_dma_timer->adjust(attotime::never);
	m_sdram_addr[0] = 0;
	m_sdram_addr[1] = 0;
	m_uart_irq = 0;
	m_irq_pins = 0;
}

void vrc5074_device::map_cpu_space()
{
	uint32_t winStart, winSize;
	uint32_t regConfig;

	// VRC5074 is at 0x1fa00000 to 0x1fa003ff
	// ROM region starts at 0x1fc00000
	m_cpu_space->unmap_readwrite(0x00000000, 0x1f9fffff);
	m_cpu_space->unmap_readwrite(0x1fa00400, 0x1fbfffff);

	// Clear fastram regions in cpu after rom
	m_cpu->clear_fastram(1);

	// Map SDRAM
	for (int index = 0; index < 2; index++) {
		regConfig = m_cpu_regs[NREG_SDRAM0 + index * 0x8 / 4];
		int mask = regConfig & 0xf;
		if (mask > 0) {
			if (mask < 5)
				fatalerror("map_cpu_space: Trying to map greater than 32 bit size. index: %d regValue: %08X\n", index, regConfig);
			winSize = (1 << (36 - mask));
			// Cap size at physical size
			if (winSize > m_sdram[index].size())
				winSize = m_sdram[index].size();
			winStart = regConfig & 0xffe00000;
			if (winSize > 0) {
				m_cpu_space->install_ram(winStart, winStart + winSize - 1, m_sdram[index].data());
				m_cpu->add_fastram(winStart, winStart + winSize - 1, false, m_sdram[index].data());
			}
			if (LOG_NILE | LOG_MAP)
				logerror("map_cpu_space ram_size=%08X ram_base=%08X\n", winSize, winStart);
		}
	}

	// Map CS
	for (int index = 2; index < 9; index++) {
		regConfig = m_cpu_regs[NREG_SDRAM0 + index * 0x8 / 4];
		int mask = regConfig & 0xf;
		if (mask > 0) {
			if (mask < 5)
				fatalerror("map_cpu_space: Trying to map greater than 32 bit size. index: %d regValue: %08X\n", index, regConfig);
			winSize = (1 << (36 - mask));
			winStart = regConfig & 0xffe00000;
			if (winSize > 0 && m_cs_devices[index - 2] != nullptr) {
				m_cpu_space->install_device_delegate(winStart, winStart + winSize - 1, *m_cs_devices[index - 2], m_cs_maps[index - 2]);
			}
			if (LOG_NILE | LOG_MAP)
				logerror("map_cpu_space cs%d_size=%08X cs%d_base=%08X\n", index, winSize, index, winStart);
		}
	}

	// PCI Windows
	for (int index = 0; index < 2; index++) {
		regConfig = m_cpu_regs[NREG_PCIW0 + index * 0x8 / 4];
		int mask = regConfig & 0xf;
		if (mask > 0) {
			if (mask < 5)
				fatalerror("map_cpu_space: Trying to map greater than 32 bit size. index: %d regValue: %08X\n", index, regConfig);
			winSize = (1 << (36 - mask));
			winStart = regConfig & 0xffe00000;
			if (winSize > 0) {
				if (index == 0) {
					m_cpu_space->install_read_handler(winStart, winStart + winSize - 1, read32s_delegate(*this, FUNC(vrc5074_device::pci0_r)));
					m_cpu_space->install_write_handler(winStart, winStart + winSize - 1, write32s_delegate(*this, FUNC(vrc5074_device::pci0_w)));
				}
				else {
					m_cpu_space->install_read_handler(winStart, winStart + winSize - 1, read32s_delegate(*this, FUNC(vrc5074_device::pci1_r)));
					m_cpu_space->install_write_handler(winStart, winStart + winSize - 1, write32s_delegate(*this, FUNC(vrc5074_device::pci1_w)));
				}
			}
			if (LOG_NILE | LOG_MAP)
				logerror("map_cpu_space pci%d_size=%08X pci%d_base=%08X\n", index, winSize, index, winStart);
		}
	}
}

void vrc5074_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	uint32_t winStart, winEnd, winSize;

	// PCI Target Window 1
	int mask = m_sdram_addr[0] & 0xf;
	//uint32_t addr_mask = (1 << (36 - mask)) - 1;
	winSize = 1 << (36 - mask);
	if (winSize > m_sdram[0].size() * 4)
		winSize = m_sdram[0].size() * 4;
	if (m_sdram[0].size() && mask > 0) {
		winStart = 0x0;

		winEnd = winStart + winSize -1;
		memory_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(vrc5074_device::target1_r)));
		memory_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(vrc5074_device::target1_w)));
		if (LOG_NILE | LOG_MAP)
			logerror("%s: map_extra Target Window 1 start=%08X end=%08X size=%08X\n", tag(), winStart, winEnd, winSize);
	}
	//// PCI Target Window 2
	//if (m_cpu_regs[NREG_PCITW2]&0x1000) {
	//  winStart = m_cpu_regs[NREG_PCITW2]&0xffe00000;
	//  winEnd = winStart | (~(0xf0000000 | (((m_cpu_regs[NREG_PCITW2]>>13)&0x7f)<<21)));
	//  winSize = winEnd - winStart + 1;
	//  memory_space->install_read_handler(winStart, winEnd, read32s_delegate(*this, FUNC(vrc5074_device::target2_r)));
	//  memory_space->install_write_handler(winStart, winEnd, write32s_delegate(*this, FUNC(vrc5074_device::target2_w)));
	//  if (LOG_NILE)
	//      logerror("%s: map_extra Target Window 2 start=%08X end=%08X size=%08X laddr=%08X\n", tag(), winStart, winEnd, winSize,  m_target2_laddr);
	//}
}

void vrc5074_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();
}

uint32_t vrc5074_device::sdram_addr_r()
{
	return 0;
}

void vrc5074_device::sdram_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
		m_sdram_addr[0] = data;
	else if (offset == 2)
		m_sdram_addr[1] = data;
	logerror("sdram_addr_w: offset: %08X data: %08X mem_mask: %08X\n", offset*4, data, mem_mask);
}

void vrc5074_device::setup_pci_space()
{
	for (int index = 0; index < 2; index++) {
		int mask = m_cpu_regs[NREG_PCIW0 + index * 2] & 0xf;
		m_pci_mask[index] = (1 << (36 - mask)) - 1;
		m_pci_laddr[index] = m_cpu_regs[NREG_PCIINIT0 + index * 2] & (~m_pci_mask[index]);
		m_pci_type[index] = m_cpu_regs[NREG_PCIINIT0 + index * 2] & 0xe;
		if (1 && LOG_NILE)
			logerror("setup_pci_space: mask_sel=%x pci_type=%x pci_mask[%d]=%08X pci_laddr[%d]=%08X\n",
				mask, m_pci_type[index], index, m_pci_mask[index], index, m_pci_laddr[index]);
	}
}
// PCI Master Window 0
uint32_t vrc5074_device::pci0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	int index = 0;
	uint32_t pci_addr = m_pci_laddr[index] | ((offset << 2) & m_pci_mask[index]);
	switch (m_pci_type[index]) {
	case 0x6:
		// Mem Space
		result = this->space(AS_DATA).read_dword(pci_addr, mem_mask);
		break;
	case 0x2:
		// I/O Space
		result = this->space(AS_IO).read_dword(pci_addr, mem_mask);
		break;
	case 0xa:
		// Config Space
		{
			uint32_t new_data;
			for (int dev = 0; dev < 31 - 21; dev++)
			{
				if ((pci_addr >> (21 + dev)) & 0x1) {
					new_data = (dev << 11) | (0x80000000) | (pci_addr & 0xff);
					//printf("writing pci_addr: %08x dev: %x new_data: %08x\n", pci_addr, dev, new_data);
					pci_host_device::config_address_w(offset, new_data);
					break;
				}
			}
			result = pci_host_device::config_data_r(offset);
		}
		break;
	default:
		logerror("Unknown PCI type\n");
		break;
	}
	if (LOG_NILE_MASTER)
		logerror("%s nile pci0_r offset %08X = %08X & %08X\n", machine().describe_context(), pci_addr, result, mem_mask);
	return result;
}
void vrc5074_device::pci0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int index = 0;
	uint32_t pci_addr = m_pci_laddr[index] | ((offset << 2) & m_pci_mask[index]);
	switch (m_pci_type[index]) {
	case 0x6:
		// Mem Space
		this->space(AS_DATA).write_dword(pci_addr, data, mem_mask);
		break;
	case 0x2:
		// I/O Space
		this->space(AS_IO).write_dword(pci_addr, data, mem_mask);
		break;
	case 0xa:
		// Config Space
		{
			// Config Space
			uint32_t new_data;
			for (int dev = 0; dev < 31 - 21; dev++)
			{
				if ((pci_addr >> (21 + dev)) & 0x1) {
					new_data = (dev << 11) | (0x80000000) | (pci_addr & 0xff);
					//printf("writing pci_addr: %08x dev: %x new_data: %08x\n", pci_addr, dev, new_data);
					pci_host_device::config_address_w(offset, new_data);
					break;
				}
			}
			pci_host_device::config_data_w(offset, data);
		}
		break;
	default:
		logerror("Unknown PCI type\n");
		break;
	}
	//this->space(AS_DATA).write_dword(m_pci0_laddr | (offset*4), data, mem_mask);
	if (LOG_NILE_MASTER)
		logerror("%s nile pci0_w offset %08X = %08X & %08X\n", machine().describe_context(), pci_addr, data, mem_mask);
}

// PCI Master Window 1
uint32_t vrc5074_device::pci1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	int index = 1;
	uint32_t pci_addr = m_pci_laddr[index] | ((offset << 2) & m_pci_mask[index]);
	switch (m_pci_type[index]) {
	case 0x6:
		// Mem Space
		result = this->space(AS_DATA).read_dword(pci_addr, mem_mask);
		break;
	case 0x2:
		// I/O Space
		result = this->space(AS_IO).read_dword(pci_addr, mem_mask);
		break;
	case 0xa:
		// Config Space
		{
			uint32_t new_data;
			for (int dev = 0; dev < 31 - 21; dev++)
			{
				if ((pci_addr >> (21 + dev)) & 0x1) {
					new_data = (dev << 11) | (0x80000000) | (pci_addr & 0xff);
					//printf("writing pci_addr: %08x dev: %x new_data: %08x\n", pci_addr, dev, new_data);
					pci_host_device::config_address_w(offset, new_data);
					break;
				}
			}
			result = pci_host_device::config_data_r(offset);
		}
		break;
	default:
		logerror("Unknown PCI type\n");
		break;
	}
	if (LOG_NILE_MASTER)
		logerror("%s nile pci1_r offset %08X = %08X & %08X\n", machine().describe_context(), pci_addr, result, mem_mask);
	return result;
}
void vrc5074_device::pci1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int index = 1;
	uint32_t pci_addr = m_pci_laddr[index] | ((offset << 2) & m_pci_mask[index]);
	switch (m_pci_type[index]) {
	case 0x6:
		// Mem Space
		this->space(AS_DATA).write_dword(pci_addr, data, mem_mask);
		break;
	case 0x2:
		// I/O Space
		this->space(AS_IO).write_dword(pci_addr, data, mem_mask);
		break;
	case 0xa:
		// Config Space
		{
			uint32_t new_data;
			for (int dev = 0; dev < 31 - 21; dev++)
			{
				if ((pci_addr >> (21 + dev)) & 0x1) {
					new_data = (dev << 11) | (0x80000000) | (pci_addr & 0xff);
					//printf("writing pci_addr: %08x dev: %x new_data: %08x\n", pci_addr, dev, new_data);
					pci_host_device::config_address_w(offset, new_data);
					break;
				}
			}
			pci_host_device::config_data_w(offset, data);
		}
		break;
	default:
		logerror("Unknown PCI type\n");
		break;
	}
	//this->space(AS_DATA).write_dword(m_pci0_laddr | (offset*4), data, mem_mask);
	if (LOG_NILE_MASTER)
		logerror("%s nile pci1_w offset %08X = %08X & %08X\n", machine().describe_context(), pci_addr, data, mem_mask);
}

// PCI Target Window 1
uint32_t vrc5074_device::target1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_sdram[0][offset];
	if (LOG_NILE_TARGET)
		logerror("%s nile target1 read from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
void vrc5074_device::target1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//m_cpu->space(AS_PROGRAM).write_dword(m_target1_laddr | (offset*4), data, mem_mask);
	COMBINE_DATA(&m_sdram[0][offset]);
	//m_sdram[0][offset] = data;
	if (LOG_NILE_TARGET)
		logerror("%s nile target1 write to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}

// DMA Transfer
TIMER_CALLBACK_MEMBER (vrc5074_device::dma_transfer)
{
	//int which = param;

	//// Check for dma suspension
	//if (m_cpu_regs[NREG_DMACR1 + which * 0xc] & DMA_SUS) {
	//  if (LOG_NILE)
	//      logerror("%s nile DMA Suspended PCI: %08X MEM: %08X Words: %X\n", machine().describe_context(), m_cpu_regs[NREG_DMA_CPAR], m_cpu_regs[NREG_DMA_CMAR], m_cpu_regs[NREG_DMA_REM]);
	//  return;
	//}

	//int pciSel = (m_cpu_regs[NREG_DMACR1+which*0xC] & DMA_MIO) ? AS_DATA : AS_IO;
	//address_space *src, *dst;
	//uint32_t srcAddr, dstAddr;

	//if (m_cpu_regs[NREG_DMACR1+which*0xC]&DMA_RW) {
	//  // Read data from PCI and write to cpu
	//  src = &this->space(pciSel);
	//  dst = &m_cpu->space(AS_PROGRAM);
	//  srcAddr = m_cpu_regs[NREG_DMA_CPAR];
	//  dstAddr = m_cpu_regs[NREG_DMA_CMAR];
	//} else {
	//  // Read data from cpu and write to PCI
	//  src = &m_cpu->space(AS_PROGRAM);
	//  dst = &this->space(pciSel);
	//  srcAddr = m_cpu_regs[NREG_DMA_CMAR];
	//  dstAddr = m_cpu_regs[NREG_DMA_CPAR];
	//}
	//int dataCount = m_cpu_regs[NREG_DMA_REM];
	//int burstCount = DMA_BURST_SIZE;
	//while (dataCount>0 && burstCount>0) {
	//  dst->write_dword(dstAddr, src->read_dword(srcAddr));
	//  dstAddr += 0x4;
	//  srcAddr += 0x4;
	//  --dataCount;
	//  --burstCount;
	//}
	//if (m_cpu_regs[NREG_DMACR1+which*0xC]&DMA_RW) {
	//  m_cpu_regs[NREG_DMA_CPAR] = srcAddr;
	//  m_cpu_regs[NREG_DMA_CMAR] = dstAddr;
	//} else {
	//  m_cpu_regs[NREG_DMA_CMAR] = srcAddr;
	//  m_cpu_regs[NREG_DMA_CPAR] = dstAddr;
	//}
	//m_cpu_regs[NREG_DMA_REM] = dataCount;
	//// Check for end of DMA
	//if (dataCount == 0) {
	//  // Clear the busy and go flags
	//  m_cpu_regs[NREG_DMACR1 + which * 0xc] &= ~DMA_BUSY;
	//  m_cpu_regs[NREG_DMACR1 + which * 0xc] &= ~DMA_GO;
	//  // Set the interrupt
	//  if (m_cpu_regs[NREG_DMACR1 + which * 0xc] & DMA_INT_EN) {
	//      if (m_irq_num != -1) {
	//          m_cpu->set_input_line(m_irq_num, ASSERT_LINE);
	//      } else {
	//          logerror("vrc5074_device::dma_transfer Error: DMA configured to trigger interrupt but no interrupt line configured\n");
	//      }
	//  }
	//  // Turn off the timer
	//  m_dma_timer->adjust(attotime::never);
	//}
}

/*************************************
*
*  nile timers & interrupts
*
*************************************/
WRITE_LINE_MEMBER(vrc5074_device::pci_intr_a) {
	update_pci_irq(0, state);
}
WRITE_LINE_MEMBER(vrc5074_device::pci_intr_b) {
	update_pci_irq(1, state);
}
WRITE_LINE_MEMBER(vrc5074_device::pci_intr_c) {
	update_pci_irq(2, state);
}
WRITE_LINE_MEMBER(vrc5074_device::pci_intr_d) {
	update_pci_irq(3, state);
}
WRITE_LINE_MEMBER(vrc5074_device::pci_intr_e) {
	update_pci_irq(4, state);
}

void vrc5074_device::update_pci_irq(const int index, const int state)
{
	m_nile_irq_state &= ~(1 << (index + 8));
	m_nile_irq_state |= state << (index + 8);
	if (LOG_NILE_IRQS)
		logerror("update_pci_irq: m_nile_irq_state: %04x index=%d state=%d\n", m_nile_irq_state, index, state);
	update_nile_irqs();
}

void vrc5074_device::update_nile_irqs()
{
	uint32_t intctll = m_cpu_regs[NREG_INTCTRL + 0];
	uint32_t intctlh = m_cpu_regs[NREG_INTCTRL + 1];
	uint8_t irq = 0;
	int i;

	/* check for UART transmit IRQ enable and synthsize one */
	if (m_uart_irq)
		m_nile_irq_state |= 0x0010;
	else
		m_nile_irq_state &= ~0x0010;

	m_cpu_regs[NREG_INTSTAT0 + 0] = 0;
	m_cpu_regs[NREG_INTSTAT0 + 1] = 0;
	m_cpu_regs[NREG_INTSTAT1 + 0] = 0;
	m_cpu_regs[NREG_INTSTAT1 + 1] = 0;

	/* handle the lower interrupts */
	for (i = 0; i < 8; i++)
		if (m_nile_irq_state & (1 << i))
			if ((intctll >> (4 * i + 3)) & 1)
			{
				int vector = (intctll >> (4 * i)) & 7;
				if (vector < 6)
				{
					irq |= 1 << vector;
					m_cpu_regs[NREG_INTSTAT0 + vector / 2] |= 1 << (i + 16 * (vector & 1));
				}
			}

	/* handle the upper interrupts */
	for (i = 0; i < 8; i++)
		if (m_nile_irq_state & (1 << (i + 8)))
			if ((intctlh >> (4 * i + 3)) & 1)
			{
				int vector = (intctlh >> (4 * i)) & 7;
				if (vector < 6)
				{
					irq |= 1 << vector;
					m_cpu_regs[NREG_INTSTAT0 + vector / 2] |= 1 << (i + 8 + 16 * (vector & 1));
				}
			}

	/* push out the state */
	uint8_t change = m_irq_pins ^ irq;
	if (LOG_NILE_IRQS) logerror("NILE IRQs:");
	for (i = 0; i < 6; i++)
	{
		if (LOG_NILE_IRQS) logerror(" %d", (irq & (1 << i)) ? 1 : 0);
		if (change & (1 << i)) {
			if (irq & (1 << i))
			{
				m_cpu->set_input_line(MIPS3_IRQ0 + i, ASSERT_LINE);
			}
			else
			{
				m_cpu->set_input_line(MIPS3_IRQ0 + i, CLEAR_LINE);
			}
		}
	}
	m_irq_pins = irq;
	if (LOG_NILE_IRQS) logerror("\n");
}


TIMER_CALLBACK_MEMBER(vrc5074_device::nile_timer_callback)
{
	int which = param;

	if (LOG_TIMERS | LOG_NILE_IRQS) logerror("timer %d fired period: %e\n", which, m_timer_period[which]);

	/* adjust the timer to fire again */
	{
		m_timer[which]->adjust(attotime::from_double(m_timer_period[which]), which);
	}

	/* trigger the interrupt */
	if (which == 2)
		m_nile_irq_state |= 1 << 6;
	if (which == 3)
		m_nile_irq_state |= 1 << 5;

	update_nile_irqs();
}



/*************************************
*
*  Nile system controller
*
*************************************/

uint32_t vrc5074_device::cpu_reg_r(offs_t offset)
{
	uint32_t result = m_cpu_regs[offset];
	bool logit = true;
	int which;

	switch (offset)
	{
	case NREG_CPUSTAT + 0:    /* CPU status */
	case NREG_CPUSTAT + 1:    /* CPU status */
		if (LOG_NILE) logerror("%s NILE READ: CPU status(%03X) = %08X\n", machine().describe_context(), offset * 4, result);
		logit = 0;
		break;

	case NREG_INTCTRL + 0:    /* Interrupt control */
	case NREG_INTCTRL + 1:    /* Interrupt control */
		if (LOG_NILE) logerror("%s NILE READ: interrupt control(%03X) = %08X\n", machine().describe_context(), offset * 4, result);
		//update_nile_irqs();
		logit = 0;
		break;

	case NREG_INTSTAT0 + 0:   /* Interrupt status 0 */
	case NREG_INTSTAT0 + 1:   /* Interrupt status 0 */
		if (LOG_NILE | LOG_NILE_IRQS) logerror("%s NILE READ: interrupt status 0(%03X) = %08X\n", machine().describe_context(), offset * 4, result);
		logit = 0;
		break;

	case NREG_INTSTAT1 + 0:   /* Interrupt status 1 */
	case NREG_INTSTAT1 + 1:   /* Interrupt status 1 */
		if (LOG_NILE | LOG_NILE_IRQS) logerror("%s NILE READ: interrupt status 1/enable(%03X) = %08X\n", machine().describe_context(), offset * 4, result);
		logit = 0;
		break;

	case NREG_INTCLR + 0:     /* Interrupt clear */
	case NREG_INTCLR + 1:     /* Interrupt clear */
		if (LOG_NILE) logerror("%s NILE READ: interrupt clear(%03X) = %08X\n", machine().describe_context(), offset * 4, result);
		logit = 0;
		break;

	case NREG_INTPPES + 0:    /* PCI Interrupt control */
	case NREG_INTPPES + 1:    /* PCI Interrupt control */
		if (LOG_NILE) logerror("%s NILE READ: PCI interrupt ppes(%03X) = %08X\n", machine().describe_context(), offset * 4, result);
		logit = 0;
		break;

	case NREG_PCIERR + 0:     /* PCI error */
	case NREG_PCIERR + 1:     /* PCI error */
	case NREG_PCICTRL + 0:    /* PCI control */
	case NREG_PCICTRL + 1:    /* PCI arbiter */
	case NREG_PCIINIT0 + 0:   /* PCI master */
	case NREG_PCIINIT0 + 1:   /* PCI master */
	case NREG_PCIINIT1 + 0:   /* PCI master */
	case NREG_PCIINIT1 + 1:   /* PCI master */
		logit = 0;
		break;

	case NREG_T0CNTR:       /* SDRAM timer control (counter) */
	case NREG_T1CNTR:       /* bus timeout timer control (counter) */
	case NREG_T2CNTR:       /* general purpose timer control (counter) */
	case NREG_T3CNTR:       /* watchdog timer control (counter) */
		which = (offset - NREG_T0CNTR) / 4;
		if (m_cpu_regs[offset - 1] & 1)
		{
			// Should check for cascaded timer
			result = m_cpu_regs[offset] = m_timer[which]->remaining().as_double() * clock();
		}

		if (LOG_TIMERS) logerror("%s NILE READ: timer %d counter(%03X) = %08X\n", machine().describe_context(), which, offset * 4, result);
		logit = 0;
		break;
	}
	if (LOG_NILE && logit)
		logerror("%s cpu_reg_r offset %03X = %08X\n", machine().describe_context(), offset * 4, result);
	return result;
}

void vrc5074_device::cpu_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t olddata = m_cpu_regs[offset];
	bool logit = true;
	int which;

	COMBINE_DATA(&m_cpu_regs[offset]);

	switch (offset)
	{
	//case NREG_SDRAM0 + 0:
	//case NREG_SDRAM1 + 0:
	//case NREG_DCS2 + 0:
	//case NREG_DCS3 + 0:
	//case NREG_DCS4 + 0:
	//case NREG_DCS5 + 0:
	//case NREG_DCS6 + 0:
	//case NREG_DCS7 + 0:
	//case NREG_DCS8 + 0:
	case NREG_SDRAM0 + 1:
	case NREG_SDRAM1 + 1:
	case NREG_DCS2 + 1:
	case NREG_DCS3 + 1:
	case NREG_DCS4 + 1:
	case NREG_DCS5 + 1:
	case NREG_DCS6 + 1:
	case NREG_DCS7 + 1:
	case NREG_DCS8 + 1:
		map_cpu_space();
		break;
	case NREG_PCIW0:
	case NREG_PCIW1:
		map_cpu_space();
		break;
	case NREG_CPUSTAT + 0:    /* CPU status */
		if (data & 0x1) logerror("cpu_reg_w: System Cold Reset\n");
		if (data & 0x2) logerror("cpu_reg_w: CPU Warm Reset\n");
		[[fallthrough]];
	case NREG_CPUSTAT + 1:    /* CPU status */
		if (LOG_NILE) logerror("%s NILE WRITE: CPU status(%03X) = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
		logit = 0;
		break;

	case NREG_INTCTRL + 0:    /* Interrupt control */
	case NREG_INTCTRL + 1:    /* Interrupt control */
		if (LOG_NILE | LOG_NILE_IRQS) logerror("%s NILE WRITE: interrupt control(%03X) = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
		logit = 0;
		update_nile_irqs();
		break;

	case NREG_INTSTAT0 + 0:   /* Interrupt status 0 */
	case NREG_INTSTAT0 + 1:   /* Interrupt status 0 */
		if (LOG_NILE | LOG_NILE_IRQS) logerror("%s NILE WRITE: interrupt status 0/1(%03X) = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
		logit = 0;
		//update_nile_irqs();
		break;

	case NREG_INTSTAT1 + 0:   /* Interrupt status 1 */
	case NREG_INTSTAT1 + 1:   /* Interrupt status 1 */
		if (LOG_NILE | LOG_NILE_IRQS) logerror("%s NILE WRITE: interrupt status 0/1 enable(%03X) = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
		logit = 0;
		//update_nile_irqs();
		break;

	case NREG_INTCLR + 0:     /* Interrupt clear */
	//case NREG_INTCLR + 1:     /* Interrupt clear */
		if (LOG_NILE | LOG_NILE_IRQS) logerror("%s NILE WRITE: interrupt clear(%03X) = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
		logit = 0;
		// Only edge triggered interrupts are cleared
		// TODO: Check which are edge triggered for midway vegas it is only the lower (non-pci) interrupts
		m_nile_irq_state &= ~(m_cpu_regs[offset] & ~0xf00);
		update_nile_irqs();
		break;

	case NREG_INTPPES + 0:    /* PCI Interrupt control */
	case NREG_INTPPES + 1:    /* PCI Interrupt control */
		if (LOG_NILE) logerror("%s NILE WRITE: PCI interrupt ppes(%03X) = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
		logit = 0;
		break;

	case NREG_PCIERR + 0:     /* PCI error */
	case NREG_PCIERR + 1:     /* PCI error */
	case NREG_PCICTRL + 0:    /* PCI control */
	case NREG_PCICTRL + 1:    /* PCI arbiter */
	case NREG_PCIINIT0 + 1:   /* PCI master */
	case NREG_PCIINIT1 + 1:   /* PCI master */
		logit = 0;
		break;

	case NREG_PCIINIT0 + 0:   /* PCI master */
	case NREG_PCIINIT1 + 0:   /* PCI master */
	//if (((olddata & 0xe) == 0xa) != ((m_cpu_regs[offset] & 0xe) == 0xa))
		//  remap_dynamic_addresses();
		//remap_cb();
		setup_pci_space();
		logit = 0;
		break;
	case NREG_DMACTRL0:
	case NREG_DMACTRL1:
		which = (offset - NREG_DMACTRL0) / 6;
		logerror("%s NILE WRITE: DMACTRL %d = %08X\n", machine().describe_context(), which, data);
		logit = 0;
		break;
	case NREG_T0CTRL + 1:     /* SDRAM timer control (control bits) */
	case NREG_T1CTRL + 1:     /* bus timeout timer control (control bits) */
	case NREG_T2CTRL + 1:     /* general purpose timer control (control bits) */
	case NREG_T3CTRL + 1:     /* watchdog timer control (control bits) */
		which = (offset - NREG_T0CTRL) / 4;
		if (LOG_NILE | LOG_TIMERS) logerror("%s NILE WRITE: timer %d control(%03X) = %08X & %08X\n", machine().describe_context(), which, offset * 4, data, mem_mask);
		logit = 0;
		m_timer_period[which] = (uint64_t(m_cpu_regs[NREG_T0CTRL + which * 4]) + 1) * attotime::from_hz(clock()).as_double();
		if (m_cpu_regs[offset] & 2) {
			// Cascade timer
			uint32_t scaleSrc = (m_cpu_regs[offset] >> 2) & 0x3;
			m_timer_period[which] += (uint64_t(m_cpu_regs[NREG_T0CTRL + scaleSrc * 4]) + 1) * attotime::from_hz(clock()).as_double();
			logerror("Timer scale: timer %d is scaled by %08X\n", which, m_cpu_regs[NREG_T0CTRL + which * 4]);
		}
		/* timer just enabled? */
		if (!(olddata & 1) && (m_cpu_regs[offset] & 1))
		{
			m_timer[which]->adjust(attotime::from_hz(clock()) * m_cpu_regs[NREG_T0CNTR + which * 4], which);
			if (LOG_TIMERS) logerror("Starting timer %d at a rate of %f Hz\n", which, attotime::from_double(m_timer_period[which]).as_hz());
		}

		/* timer disabled? */
		else if ((olddata & 1) && !(m_cpu_regs[offset] & 1))
		{
			m_cpu_regs[offset + 1] = m_timer[which]->remaining().as_double() * clock();
			m_timer[which]->adjust(attotime::never, which);
		}
		break;

	case NREG_T0CNTR:       /* SDRAM timer control (counter) */
	case NREG_T1CNTR:       /* bus timeout timer control (counter) */
	case NREG_T2CNTR:       /* general purpose timer control (counter) */
	case NREG_T3CNTR:       /* watchdog timer control (counter) */
		which = (offset - NREG_T0CNTR) / 4;
		if (LOG_TIMERS) logerror("%s NILE WRITE: timer %d counter(%03X) = %08X & %08X\n", machine().describe_context(), which, offset * 4, data, mem_mask);
		logit = 0;

		if (m_cpu_regs[offset - 1] & 1)
		{
			m_timer[which]->adjust(attotime::from_hz(clock()) * m_cpu_regs[offset], which);
		}
		break;
	}

	if (LOG_NILE && logit)
		logerror("%s cpu_reg_w offset %03X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}

WRITE_LINE_MEMBER(vrc5074_device::uart_irq_callback)
{
	if (state ^ m_uart_irq) {
		m_uart_irq = state;
		update_nile_irqs();
		if (LOG_NILE | LOG_NILE_IRQS)
			logerror("uart_irq_callback: state = %d\n", state);
	}
}

uint32_t vrc5074_device::serial_r(offs_t offset)
{
	uint32_t result = m_uart->ins8250_r(offset>>1);

	if (0 && LOG_NILE)
		logerror("%s serial_r offset %03X = %08X (%08x)\n", machine().describe_context(), offset>>1, result, offset*4);
	return result;
}

void vrc5074_device::serial_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_uart->ins8250_w(offset>>1, data);
	if (PRINTF_SERIAL && offset == NREG_UARTTHR) {
		static std::string debugStr;
		printf("%c", data);
		if (data == 0xd || debugStr.length()>=80) {
			logerror("%s", debugStr.c_str());
			debugStr.clear();
		}
		else {
			debugStr += char(data);
		}
	}
	if (0 && LOG_NILE)
		logerror("%s serial_w offset %03X = %08X & %08X (%08x)\n", machine().describe_context(), offset>>1, data, mem_mask, offset*4);
}
