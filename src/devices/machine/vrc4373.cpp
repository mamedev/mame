// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "vrc4373.h"

#define LOG_GENERAL         (1U << 0)
#define LOG_NILE            (1U << 1)
#define LOG_NILE_MASTER     (1U << 2)
#define LOG_NILE_TARGET     (1U << 3)

//#define VERBOSE (LOG_NILE | LOG_NILE_MASTER | LOG_NILE_TARGET)
#include "logmacro.h"

#define LOGNILE(...)        LOGMASKED(LOG_NILE, __VA_ARGS__)
#define LOGNILEMASTER(...)  LOGMASKED(LOG_NILE_MASTER, __VA_ARGS__)
#define LOGNILETARGET(...)  LOGMASKED(LOG_NILE_TARGET, __VA_ARGS__)


#define VRC4373_PAGESHIFT 12

/* NILE 3 registers 0x000-0x0ff */
#define NREG_BMCR           (0x000/4)
#define NREG_SIMM1          (0x004/4)
#define NREG_SIMM2          (0x008/4)
#define NREG_SIMM3          (0x00C/4)
#define NREG_SIMM4          (0x010/4)
#define NREG_PCIMW1         (0x014/4)
#define NREG_PCIMW2         (0x018/4)
#define NREG_PCITW1         (0x01C/4)
#define NREG_PCITW2         (0x020/4)
#define NREG_PCIMIOW        (0x024/4)
#define NREG_PCICDR         (0x028/4)
#define NREG_PCICAR         (0x02C/4)
#define NREG_PCIMB1         (0x030/4)
#define NREG_PCIMB2         (0x034/4)
#define NREG_DMACR1         (0x038/4)
#define NREG_DMAMAR1        (0x03C/4)
#define NREG_DMAPCI1        (0x040/4)
#define NREG_DMACR2         (0x044/4)
#define NREG_DMAMAR2        (0x048/4)
#define NREG_DMAPCI2        (0x04C/4)

#define NREG_BESR           (0x050/4)
#define NREG_ICSR           (0x054/4)
#define NREG_DRAMRCR        (0x058/4)
#define NREG_BOOTWP         (0x05C/4)
#define NREG_PCIEAR         (0x060/4)
#define NREG_DMA_REM        (0x064/4)
#define NREG_DMA_CMAR       (0x068/4)
#define NREG_DMA_CPAR       (0x06C/4)
#define NREG_PCIRC          (0x070/4)
#define NREG_PCIEN          (0x074/4)
#define NREG_PMIR           (0x078/4)

#define PCI_BUS_CLOCK       33000000
// Number of dma words to transfer at a time, real hardware bursts 8
#define DMA_BURST_SIZE      128
#define DMA_TIMER_PERIOD    attotime::from_hz(PCI_BUS_CLOCK / 32)

#define DMA_BUSY            0x80000000
#define DMA_INT_EN          0x40000000
#define DMA_RW              0x20000000
#define DMA_GO              0x10000000
#define DMA_SUS             0x08000000
#define DMA_INC             0x04000000
#define DMA_MIO             0x02000000
#define DMA_RST             0x01000000
#define DMA_BLK_SIZE        0x000fffff


DEFINE_DEVICE_TYPE(VRC4373, vrc4373_device, "vrc4373", "NEC VRC4373 System Controller")

void vrc4373_device::config_map(address_map &map)
{
	pci_bridge_device::config_map(map);
	map(0x40, 0x43).rw(FUNC(vrc4373_device::pcictrl_r), FUNC(vrc4373_device::pcictrl_w));
}

// cpu i/f map
void vrc4373_device::cpu_map(address_map &map)
{
	map(0x00000000, 0x0000007b).rw(FUNC(vrc4373_device::cpu_if_r), FUNC(vrc4373_device::cpu_if_w));
}

// Target Window 1 map
void vrc4373_device::target1_map(address_map &map)
{
	map(0x00000000, 0xFFFFFFFF).rw(FUNC(vrc4373_device::target1_r), FUNC(vrc4373_device::target1_w));
}

// Target Window 2 map
void vrc4373_device::target2_map(address_map &map)
{
	map(0x00000000, 0xFFFFFFFF).rw(FUNC(vrc4373_device::target2_r), FUNC(vrc4373_device::target2_w));
}

vrc4373_device::vrc4373_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, VRC4373, tag, owner, clock)
	, m_cpu_space(nullptr), m_irq_cb(*this), m_cpu(*this, finder_base::DUMMY_TAG), m_ram_size(0x0), m_simm0_size(0x0)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32), m_pci1_laddr(0), m_pci2_laddr(0), m_pci_io_laddr(0), m_target1_laddr(0), m_target2_laddr(0)
	, m_romRegion(*this, "rom")
{
	set_ids_host(0x1033005B, 0x00, 0x00000000);
}

device_memory_interface::space_config_vector vrc4373_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void vrc4373_device::device_start()
{
	pci_host_device::device_start();

	m_cpu_space = &m_cpu->space(AS_PCI_CONFIG);
	memory_space = &space(AS_PCI_MEM);
	io_space = &space(AS_PCI_IO);
	is_multifunction_device = false;

	std::fill(std::begin(m_cpu_regs), std::end(m_cpu_regs), 0);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffffffff;
	io_offset       = 0x00000000;
	status = 0x0280;

	m_irq_cb.resolve();

	// Reserve 8M for ram
	m_ram.reserve(0x00800000 / 4);
	m_ram.resize(m_ram_size);
	// Reserve 32M for simm[0]
	m_simm[0].reserve(0x02000000 / 4);
	m_simm[0].resize(m_simm0_size / 4);
	// ROM
	uint32_t romSize = m_romRegion->bytes();
	m_cpu_space->install_rom(0x1fc00000, 0x1fc00000 + romSize - 1, m_romRegion->base());
	// Nile register mapppings
	m_cpu_space->install_device(0x0f000000, 0x0f0000ff, *static_cast<vrc4373_device *>(this), &vrc4373_device::cpu_map);
	// PCI Configuration also mapped at 0x0f000100
	m_cpu_space->install_device(0x0f000100, 0x0f0001ff, *static_cast<vrc4373_device *>(this), &vrc4373_device::config_map);

	// MIPS drc
	m_cpu->add_fastram(0x1fc00000, 0x1fcfffff, true, m_romRegion->base());

	// DMA timer
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vrc4373_device::dma_transfer), this));
	// Leave the timer disabled.
	m_dma_timer->adjust(attotime::never, 0, DMA_TIMER_PERIOD);

	// Save states
	// m_ram
	save_item(NAME(m_ram));
	// m_simm
	save_item(NAME(m_simm[0]));
	save_item(NAME(m_cpu_regs));
	save_item(NAME(m_pci1_laddr));
	save_item(NAME(m_pci2_laddr));
	save_item(NAME(m_pci_io_laddr));
	save_item(NAME(m_target1_laddr));
	save_item(NAME(m_target2_laddr));
}

void vrc4373_device::device_post_load()
{
	map_cpu_space();
	//remap_cb();
}

void vrc4373_device::device_reset()
{
	pci_device::device_reset();
	memset(m_cpu_regs, 0, sizeof(m_cpu_regs));
	regenerate_config_mapping();
	m_dma_timer->adjust(attotime::never);
}

void vrc4373_device::map_cpu_space()
{
	uint32_t winStart, winEnd, winSize;
	uint32_t regConfig;

	// VRC4373 is at 0x0f000000 to 0x0f0001ff
	// ROM region starts at 0x1f000000
	m_cpu_space->unmap_readwrite(0x00000000, 0x0effffff);
	m_cpu_space->unmap_readwrite(0x0f000200, 0x1effffff);

	// Clear fastram regions in cpu after rom
	m_cpu->clear_fastram(1);

	regConfig = m_cpu_regs[NREG_BMCR];
	if (regConfig & 0x8) {
		winSize = 1 << 22;  // 4MB
		for (int i = 14; i <= 15; i++) {
			if (!((regConfig >> i) & 0x1)) winSize <<= 1;
			else break;
		}
		winStart = (regConfig & 0x0fc00000);
		winEnd = winStart + winSize - 1;

		m_ram.resize(winSize / 4);
		m_cpu_space->install_ram(winStart, winEnd, m_ram.data());
		m_cpu->add_fastram(winStart, winEnd, false, m_ram.data());
		LOGNILE("map_cpu_space ram_size=%08X ram_base=%08X\n", winSize, winStart);
	}

	// Map SIMMs
	for (int simIndex = 0; simIndex < 4; simIndex++) {
		regConfig = m_cpu_regs[NREG_SIMM1 + simIndex];
		if (regConfig & 0x8) {
			winSize = 1 << 21;  // 2MB
			for (int i = 13; i <= 17; i++) {
				if (!((regConfig >> i) & 0x1)) winSize <<= 1;
				else break;
			}
			winStart = (regConfig & 0x0fe00000);
			winEnd = winStart + winSize - 1;

			m_simm[simIndex].resize(winSize / 4);
			m_cpu_space->install_ram(winStart, winEnd, m_simm[simIndex].data());
			m_cpu->add_fastram(winStart, winEnd, false, m_simm[simIndex].data());
			LOGNILE("map_cpu_space simm_size[%i]=%08X simm_base=%08X\n", simIndex, winSize, winStart);
		}
	}

	// PCI Master Window 1
	if (m_cpu_regs[NREG_PCIMW1]&0x1000) {
		winStart = m_cpu_regs[NREG_PCIMW1]&0xff000000;
		winEnd = winStart | (~(0x80000000 | (((m_cpu_regs[NREG_PCIMW1]>>13)&0x7f)<<24)));
		winSize = winEnd - winStart + 1;
		m_cpu_space->install_read_handler(winStart, winEnd, read32_delegate(*this, FUNC(vrc4373_device::master1_r)));
		m_cpu_space->install_write_handler(winStart, winEnd, write32_delegate(*this, FUNC(vrc4373_device::master1_w)));
		LOGNILE("map_cpu_space Master Window 1 start=%08X end=%08X size=%08X laddr=%08X\n", winStart, winEnd, winSize,  m_pci1_laddr);
	}
	// PCI Master Window 2
	if (m_cpu_regs[NREG_PCIMW2]&0x1000) {
		winStart = m_cpu_regs[NREG_PCIMW2]&0xff000000;
		winEnd = winStart | (~(0x80000000 | (((m_cpu_regs[NREG_PCIMW2]>>13)&0x7f)<<24)));
		winSize = winEnd - winStart + 1;
		m_cpu_space->install_read_handler(winStart, winEnd, read32_delegate(*this, FUNC(vrc4373_device::master2_r)));
		m_cpu_space->install_write_handler(winStart, winEnd, write32_delegate(*this, FUNC(vrc4373_device::master2_w)));
		LOGNILE("map_cpu_space Master Window 2 start=%08X end=%08X size=%08X laddr=%08X\n", winStart, winEnd, winSize,  m_pci2_laddr);
	}
	// PCI IO Window
	if (m_cpu_regs[NREG_PCIMIOW]&0x1000) {
		winStart = m_cpu_regs[NREG_PCIMIOW]&0xff000000;
		winEnd = winStart | (~(0x80000000 | (((m_cpu_regs[NREG_PCIMIOW]>>13)&0x7f)<<24)));
		winSize = winEnd - winStart + 1;
		m_cpu_space->install_read_handler(winStart, winEnd, read32_delegate(*this, FUNC(vrc4373_device::master_io_r)));
		m_cpu_space->install_write_handler(winStart, winEnd, write32_delegate(*this, FUNC(vrc4373_device::master_io_w)));
		LOGNILE("map_cpu_space IO Window start=%08X end=%08X size=%08X laddr=%08X\n", tag(), winStart, winEnd, winSize,  m_pci_io_laddr);
	}
}

void vrc4373_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	uint32_t winStart, winEnd, winSize;

	// PCI Target Window 1
	if (m_cpu_regs[NREG_PCITW1]&0x1000) {
		winStart = m_cpu_regs[NREG_PCITW1]&0xffe00000;
		winEnd = winStart | (~(0xf0000000 | (((m_cpu_regs[NREG_PCITW1]>>13)&0x7f)<<21)));
		winSize = winEnd - winStart + 1;
		memory_space->install_read_handler(winStart, winEnd, read32_delegate(*this, FUNC(vrc4373_device::target1_r)));
		memory_space->install_write_handler(winStart, winEnd, write32_delegate(*this, FUNC(vrc4373_device::target1_w)));
		LOGNILE("map_extra Target Window 1 start=%08X end=%08X size=%08X laddr=%08X\n", winStart, winEnd, winSize,  m_target1_laddr);
	}
	// PCI Target Window 2
	if (m_cpu_regs[NREG_PCITW2]&0x1000) {
		winStart = m_cpu_regs[NREG_PCITW2]&0xffe00000;
		winEnd = winStart | (~(0xf0000000 | (((m_cpu_regs[NREG_PCITW2]>>13)&0x7f)<<21)));
		winSize = winEnd - winStart + 1;
		memory_space->install_read_handler(winStart, winEnd, read32_delegate(*this, FUNC(vrc4373_device::target2_r)));
		memory_space->install_write_handler(winStart, winEnd, write32_delegate(*this, FUNC(vrc4373_device::target2_w)));
		LOGNILE("map_extra Target Window 2 start=%08X end=%08X size=%08X laddr=%08X\n", winStart, winEnd, winSize,  m_target2_laddr);
	}
}

void vrc4373_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();
}

// PCI bus control
READ32_MEMBER (vrc4373_device::pcictrl_r)
{
	uint32_t result = 0;
	LOGNILE("%s nile pcictrl_r from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (vrc4373_device::pcictrl_w)
{
	LOGNILE("%s nile pcictrl_w to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}
// PCI Master Window 1
READ32_MEMBER (vrc4373_device::master1_r)
{
	uint32_t result = this->space(AS_PCI_MEM).read_dword(m_pci1_laddr | (offset*4), mem_mask);
	LOGNILEMASTER("%s nile master1 read from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (vrc4373_device::master1_w)
{
	this->space(AS_PCI_MEM).write_dword(m_pci1_laddr | (offset*4), data, mem_mask);
	LOGNILEMASTER("%s nile master1 write to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}

// PCI Master Window 2
READ32_MEMBER (vrc4373_device::master2_r)
{
	uint32_t result = this->space(AS_PCI_MEM).read_dword(m_pci2_laddr | (offset*4), mem_mask);
	LOGNILEMASTER("%s nile master2 read from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (vrc4373_device::master2_w)
{
	this->space(AS_PCI_MEM).write_dword(m_pci2_laddr | (offset*4), data, mem_mask);
	LOGNILEMASTER("%s nile master2 write to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}

// PCI Master IO Window
READ32_MEMBER (vrc4373_device::master_io_r)
{
	uint32_t result = this->space(AS_PCI_IO).read_dword(m_pci_io_laddr | (offset*4), mem_mask);
	LOGNILEMASTER("%s nile master io read from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (vrc4373_device::master_io_w)
{
	this->space(AS_PCI_IO).write_dword(m_pci_io_laddr | (offset*4), data, mem_mask);
	LOGNILEMASTER("%s nile master io write to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}

// PCI Target Window 1
READ32_MEMBER (vrc4373_device::target1_r)
{
	uint32_t result = m_cpu->space(AS_PCI_CONFIG).read_dword(m_target1_laddr | (offset*4), mem_mask);
	LOGNILETARGET("%08X:nile target1 read from offset %02X = %08X & %08X\n", m_cpu->pc(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (vrc4373_device::target1_w)
{
	m_cpu->space(AS_PCI_CONFIG).write_dword(m_target1_laddr | (offset*4), data, mem_mask);
	LOGNILETARGET("%08X:nile target1 write to offset %02X = %08X & %08X\n", m_cpu->pc(), offset*4, data, mem_mask);
}

// PCI Target Window 2
READ32_MEMBER (vrc4373_device::target2_r)
{
	uint32_t result = m_cpu->space(AS_PCI_CONFIG).read_dword(m_target2_laddr | (offset*4), mem_mask);
	LOGNILETARGET("%08X:nile target2 read from offset %02X = %08X & %08X\n", m_cpu->pc(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (vrc4373_device::target2_w)
{
	m_cpu->space(AS_PCI_CONFIG).write_dword(m_target2_laddr | (offset*4), data, mem_mask);
	LOGNILETARGET("%08X:nile target2 write to offset %02X = %08X & %08X\n", m_cpu->pc(), offset*4, data, mem_mask);
}

// DMA Transfer
TIMER_CALLBACK_MEMBER (vrc4373_device::dma_transfer)
{
	int which = param;

	// Check for dma suspension
	if (m_cpu_regs[NREG_DMACR1 + which * 0xc] & DMA_SUS) {
		LOGNILE("%08X:nile DMA Suspended PCI: %08X MEM: %08X Words: %X\n", m_cpu->pc(), m_cpu_regs[NREG_DMA_CPAR], m_cpu_regs[NREG_DMA_CMAR], m_cpu_regs[NREG_DMA_REM]);
		return;
	}

	int pciSel = (m_cpu_regs[NREG_DMACR1+which*0xC] & DMA_MIO) ? AS_PCI_MEM : AS_PCI_IO;
	address_space *src, *dst;
	uint32_t srcAddr, dstAddr;

	if (m_cpu_regs[NREG_DMACR1+which*0xC]&DMA_RW) {
		// Read data from PCI and write to cpu
		src = &this->space(pciSel);
		dst = &m_cpu->space(AS_PCI_CONFIG);
		srcAddr = m_cpu_regs[NREG_DMA_CPAR];
		dstAddr = m_cpu_regs[NREG_DMA_CMAR];
	} else {
		// Read data from cpu and write to PCI
		src = &m_cpu->space(AS_PCI_CONFIG);
		dst = &this->space(pciSel);
		srcAddr = m_cpu_regs[NREG_DMA_CMAR];
		dstAddr = m_cpu_regs[NREG_DMA_CPAR];
	}
	int dataCount = m_cpu_regs[NREG_DMA_REM];
	int burstCount = DMA_BURST_SIZE;
	while (dataCount>0 && burstCount>0) {
		dst->write_dword(dstAddr, src->read_dword(srcAddr));
		dstAddr += 0x4;
		srcAddr += 0x4;
		--dataCount;
		--burstCount;
	}
	if (m_cpu_regs[NREG_DMACR1+which*0xC]&DMA_RW) {
		m_cpu_regs[NREG_DMA_CPAR] = srcAddr;
		m_cpu_regs[NREG_DMA_CMAR] = dstAddr;
	} else {
		m_cpu_regs[NREG_DMA_CMAR] = srcAddr;
		m_cpu_regs[NREG_DMA_CPAR] = dstAddr;
	}
	m_cpu_regs[NREG_DMA_REM] = dataCount;
	// Check for end of DMA
	if (dataCount == 0) {
		// Clear the busy and go flags
		m_cpu_regs[NREG_DMACR1 + which * 0xc] &= ~DMA_BUSY;
		m_cpu_regs[NREG_DMACR1 + which * 0xc] &= ~DMA_GO;
		// Set the interrupt
		if (m_cpu_regs[NREG_DMACR1 + which * 0xc] & DMA_INT_EN) {
			if (!m_irq_cb.isnull()) {
				m_irq_cb(ASSERT_LINE);
			} else {
				logerror("vrc4373_device::dma_transfer Error: DMA configured to trigger interrupt but no interrupt line configured\n");
			}
		}
		// Turn off the timer
		m_dma_timer->adjust(attotime::never);
	}
}

// CPU I/F
READ32_MEMBER (vrc4373_device::cpu_if_r)
{
	uint32_t result = m_cpu_regs[offset];
	switch (offset) {
		case NREG_PCICAR:
			result = config_address_r(space, offset);
			break;
		case NREG_PCICDR:
			result = config_data_r(space, offset);
			break;
		case NREG_ICSR:
			// Top 16 bits always read as zero
			result &= 0xffff;
			break;
		default:
			break;
	}
	LOGNILE("%s nile read from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(vrc4373_device::cpu_if_w)
{
	LOGNILE("%s nile write to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);

	uint32_t modData, oldData;
	oldData = m_cpu_regs[offset];
	COMBINE_DATA(&m_cpu_regs[offset]);
	switch (offset) {
		case NREG_PCIMW1:
				m_pci1_laddr = (data&0xff)<<24;
				map_cpu_space();
			break;
		case NREG_PCIMW2:
				m_pci2_laddr = (data&0xff)<<24;
				map_cpu_space();
			break;
		case NREG_PCIMIOW:
				m_pci_io_laddr = (data&0xff)<<24;
				map_cpu_space();
			break;
		case NREG_PCITW1:
				m_target1_laddr = 0x00000000 | ((data&0x7FF)<<21);
				remap_cb();
			break;
		case NREG_PCITW2:
				m_target2_laddr = 0x00000000 | ((data&0x7FF)<<21);
				remap_cb();
			break;
		case NREG_PCICAR:
			// Bits in reserved area are used for device selection of type 0 config transactions
			// Assuming 23:11 get mapped into device number for configuration
			if ((data&0x3) == 0x0) {
				// Type 0 transaction
				modData = 0;
				// Select the device based on one hot bit
				for (int i=11; i<24; i++) {
					if ((data>>i)&0x1) {
						// One hot encoding, bit 11 will mean device 1
						modData = i-10;
						break;
					}
				}
				// Re-organize into Type 1 transaction for bus 0 (local bus)
				modData = (modData<<11) | (data&0x7ff) | (0x80000000);
			} else {
				// Type 1 transaction, no modification needed
				modData = data;
			}
			pci_host_device::config_address_w(space, offset, modData);
			break;
		case NREG_PCICDR:
			pci_host_device::config_data_w(space, offset, data);
			break;
		case NREG_DMACR1:
		case NREG_DMACR2:
			// Start when DMA_GO bit is set
			if (!(oldData & DMA_GO) && (data & DMA_GO)) {
				int which = (offset - NREG_DMACR1) >> 3;
				// Set counts and address
				m_cpu_regs[NREG_DMA_CPAR] = m_cpu_regs[NREG_DMAPCI1 + which * 0xC];
				m_cpu_regs[NREG_DMA_CMAR] = m_cpu_regs[NREG_DMAMAR1 + which * 0xC];
				// Set number of words remaining
				m_cpu_regs[NREG_DMA_REM] = (data & DMA_BLK_SIZE) >> 2;
				// Set busy flag
				m_cpu_regs[NREG_DMACR1 + which * 0xc] |= DMA_BUSY;
				// Start the transfer
				m_dma_timer->set_param(which);
				m_dma_timer->adjust(attotime::zero, 0, DMA_TIMER_PERIOD);
				LOGNILE("%08X:nile Start DMA Lane %i PCI: %08X MEM: %08X Words: %X\n", m_cpu->pc(), which, m_cpu_regs[NREG_DMA_CPAR], m_cpu_regs[NREG_DMA_CMAR], m_cpu_regs[NREG_DMA_REM]);
			}
			break;
		case NREG_ICSR:
			// TODO: Check and clear individual interrupts
			if (data & 0xff000000) {
				if (!m_irq_cb.isnull())
					m_irq_cb(CLEAR_LINE);
			}
			break;
		case NREG_BMCR:
		case NREG_SIMM1:
		case NREG_SIMM2:
		case NREG_SIMM3:
		case NREG_SIMM4:
			map_cpu_space();
			break;
		default:
			break;
	}

}
