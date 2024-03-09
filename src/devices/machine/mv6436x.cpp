// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Marvell MV64360/1/2

    System controller for PowerPC processors

***************************************************************************/

#include "emu.h"
#include "mv6436x.h"

#define LOG_PCI_CONFIG (1U << 1)
#define LOG_PCI_MEM    (1U << 2)
#define LOG_PCI_IO     (1U << 3)
#define LOG_REG_READ   (1U << 4)
#define LOG_REG_WRITE  (1U << 5)

#define VERBOSE (LOG_GENERAL | LOG_REG_WRITE | LOG_PCI_CONFIG | LOG_PCI_MEM)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MV64361, mv64361_device, "mv64361", "Marvell MV64361 System Controller")
DEFINE_DEVICE_TYPE(MV64361_PCI_HOST, mv64361_pci_host_device, "mv64361_pci_host", "Marvell MV64361 PCI Host")


//**************************************************************************
//  MV64361 SYSTEM CONTROLLER
//**************************************************************************

mv64361_device::mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MV64361, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_pcihost(*this, finder_base::DUMMY_TAG, 0)
{
}

void mv64361_device::device_start()
{
	m_cpu_space = &m_cpu->space(AS_PROGRAM);

	// 2 MB internal SRAM
	m_sram = std::make_unique<uint8_t[]>(0x200000);

	// TODO savestates
}

void mv64361_device::device_reset()
{
	// initialize internal registers
	m_regs[REG_CPU_CONFIG] = 0x028000ff;
	m_regs[REG_INTERNAL_BASE] = 0x01001400;
	m_regs[REG_BOOTCS_BASE] = 0x0000ff80;
	m_regs[REG_BOOTCS_SIZE] = 0x0000007f;
	m_regs[REG_PCI0_IO_BASE] = 0x01001000;
	m_regs[REG_PCI0_IO_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_IO_REMAP] = 0x00001000;
	m_regs[REG_PCI0_MEM0_BASE] = 0x01001200;
	m_regs[REG_PCI0_MEM0_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM0_REMAP_LOW] = 0x00001200;
	m_regs[REG_PCI0_MEM0_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_MEM1_BASE] = 0x0100f200;
	m_regs[REG_PCI0_MEM1_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM1_REMAP_LOW] = 0x0000f200;
	m_regs[REG_PCI0_MEM1_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_MEM2_BASE] = 0x0100f400;
	m_regs[REG_PCI0_MEM2_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM2_REMAP_LOW] = 0x0000f400;
	m_regs[REG_PCI0_MEM2_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_MEM3_BASE] = 0x0100f600;
	m_regs[REG_PCI0_MEM3_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM3_REMAP_LOW] = 0x0000f600;
	m_regs[REG_PCI0_MEM3_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_IO_BASE] = 0x01002000;
	m_regs[REG_PCI1_IO_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_IO_REMAP] = 0x00002000;
	m_regs[REG_PCI1_MEM0_BASE] = 0x00002200;
	m_regs[REG_PCI1_MEM0_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM0_REMAP_LOW] = 0x00002200;
	m_regs[REG_PCI1_MEM0_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_MEM1_BASE] = 0x01002400;
	m_regs[REG_PCI1_MEM1_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM1_REMAP_LOW] = 0x00002400;
	m_regs[REG_PCI1_MEM1_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_MEM2_BASE] = 0x01002600;
	m_regs[REG_PCI1_MEM2_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM2_REMAP_LOW] = 0x00002600;
	m_regs[REG_PCI1_MEM2_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_MEM3_BASE] = 0x01002800;
	m_regs[REG_PCI1_MEM3_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM3_REMAP_LOW] = 0x00002800;
	m_regs[REG_PCI1_MEM3_REMAP_HIGH] = 0x00000000;
	m_regs[REG_BASE_ADDRESS_ENABLE] = 0x00000000;

	// pegasos2 specific override (TODO: make generic)
	m_regs[REG_INTERNAL_BASE] = 0x0100f100;
	m_regs[REG_PCI1_IO_BASE] = 0x0100fe00;
	m_regs[REG_PCI1_IO_SIZE] = 0x000000ff;
	m_regs[REG_PCI1_IO_REMAP] = 0x00000000;
	m_regs[REG_BASE_ADDRESS_ENABLE] = 0x00efbfff;

	for (int i = 0; i < 21; i++)
		m_ranges[i].enabled = false;

	map_windows();
}

void mv64361_device::register_map(address_map &map)
{
	map(0x0000, 0x0003).rw(FUNC(mv64361_device::cpu_config_r), FUNC(mv64361_device::cpu_config_w));
	map(0x0048, 0x004b).rw(FUNC(mv64361_device::pci0_io_base_r), FUNC(mv64361_device::pci0_io_base_w));
	map(0x0050, 0x0053).rw(FUNC(mv64361_device::pci0_io_size_r), FUNC(mv64361_device::pci0_io_size_w));
	map(0x0058, 0x005b).rw(FUNC(mv64361_device::pci0_mem0_base_r), FUNC(mv64361_device::pci0_mem0_base_w));
	map(0x0060, 0x0063).rw(FUNC(mv64361_device::pci0_mem0_size_r), FUNC(mv64361_device::pci0_mem0_size_w));
	map(0x0068, 0x006b).rw(FUNC(mv64361_device::internal_base_r), FUNC(mv64361_device::internal_base_w));
	map(0x0080, 0x0083).rw(FUNC(mv64361_device::pci0_mem1_base_r), FUNC(mv64361_device::pci0_mem1_base_w));
	map(0x0088, 0x008b).rw(FUNC(mv64361_device::pci0_mem1_size_r), FUNC(mv64361_device::pci0_mem1_size_w));
	map(0x0090, 0x0093).rw(FUNC(mv64361_device::pci1_io_base_r), FUNC(mv64361_device::pci1_io_base_w));
	map(0x0098, 0x009b).rw(FUNC(mv64361_device::pci1_io_size_r), FUNC(mv64361_device::pci1_io_size_w));
	map(0x00a0, 0x00a3).rw(FUNC(mv64361_device::pci1_mem0_base_r), FUNC(mv64361_device::pci1_mem0_base_w));
	map(0x00a8, 0x00ab).rw(FUNC(mv64361_device::pci1_mem0_size_r), FUNC(mv64361_device::pci1_mem0_size_w));
	map(0x00b0, 0x00b3).rw(FUNC(mv64361_device::pci1_mem1_base_r), FUNC(mv64361_device::pci1_mem1_base_w));
	map(0x00b8, 0x00bb).rw(FUNC(mv64361_device::pci1_mem1_size_r), FUNC(mv64361_device::pci1_mem1_size_w));
	map(0x00f0, 0x00f3).rw(FUNC(mv64361_device::pci0_io_remap_r), FUNC(mv64361_device::pci0_io_remap_w));
	map(0x00f8, 0x00fb).rw(FUNC(mv64361_device::pci0_mem0_remap_low_r), FUNC(mv64361_device::pci0_mem0_remap_low_w));
	map(0x0100, 0x0103).rw(FUNC(mv64361_device::pci0_mem1_remap_low_r), FUNC(mv64361_device::pci0_mem1_remap_low_w));
	map(0x0108, 0x010b).rw(FUNC(mv64361_device::pci1_io_remap_r), FUNC(mv64361_device::pci1_io_remap_w));
	map(0x0110, 0x0113).rw(FUNC(mv64361_device::pci1_mem0_remap_low_r), FUNC(mv64361_device::pci1_mem0_remap_low_w));
	map(0x0118, 0x011b).rw(FUNC(mv64361_device::pci1_mem1_remap_low_r), FUNC(mv64361_device::pci1_mem1_remap_low_w));
	map(0x0238, 0x023b).rw(FUNC(mv64361_device::bootcs_base_r), FUNC(mv64361_device::bootcs_base_w));
	map(0x0240, 0x0243).rw(FUNC(mv64361_device::bootcs_size_r), FUNC(mv64361_device::bootcs_size_w));
	map(0x0258, 0x025b).rw(FUNC(mv64361_device::pci0_mem2_base_r), FUNC(mv64361_device::pci0_mem2_base_w));
	map(0x0260, 0x0263).rw(FUNC(mv64361_device::pci0_mem2_size_r), FUNC(mv64361_device::pci0_mem2_size_w));
	map(0x0268, 0x026b).rw(FUNC(mv64361_device::sram_base_r), FUNC(mv64361_device::sram_base_w));
	map(0x0278, 0x027b).rw(FUNC(mv64361_device::base_address_enable_r), FUNC(mv64361_device::base_address_enable_w));
	map(0x0280, 0x0283).rw(FUNC(mv64361_device::pci0_mem3_base_r), FUNC(mv64361_device::pci0_mem3_base_w));
	map(0x0288, 0x028b).rw(FUNC(mv64361_device::pci0_mem3_size_r), FUNC(mv64361_device::pci0_mem3_size_w));
	map(0x02a0, 0x02a3).rw(FUNC(mv64361_device::pci1_mem2_base_r), FUNC(mv64361_device::pci1_mem2_base_w));
	map(0x02a8, 0x02ab).rw(FUNC(mv64361_device::pci1_mem2_size_r), FUNC(mv64361_device::pci1_mem2_size_w));
	map(0x02b0, 0x02b3).rw(FUNC(mv64361_device::pci1_mem3_base_r), FUNC(mv64361_device::pci1_mem3_base_w));
	map(0x02b8, 0x02bb).rw(FUNC(mv64361_device::pci1_mem3_size_r), FUNC(mv64361_device::pci1_mem3_size_w));
	map(0x02f8, 0x02fb).rw(FUNC(mv64361_device::pci0_mem2_remap_low_r), FUNC(mv64361_device::pci0_mem2_remap_low_w));
	map(0x0300, 0x0303).rw(FUNC(mv64361_device::pci0_mem3_remap_low_r), FUNC(mv64361_device::pci0_mem3_remap_low_w));
	map(0x0310, 0x0313).rw(FUNC(mv64361_device::pci1_mem2_remap_low_r), FUNC(mv64361_device::pci1_mem2_remap_low_w));
	map(0x0318, 0x031b).rw(FUNC(mv64361_device::pci1_mem3_remap_low_r), FUNC(mv64361_device::pci1_mem3_remap_low_w));
	map(0x0320, 0x0323).rw(FUNC(mv64361_device::pci0_mem0_remap_high_r), FUNC(mv64361_device::pci0_mem0_remap_high_w));
	map(0x0328, 0x032b).rw(FUNC(mv64361_device::pci0_mem1_remap_high_r), FUNC(mv64361_device::pci0_mem1_remap_high_w));
	map(0x0330, 0x0333).rw(FUNC(mv64361_device::pci0_mem2_remap_high_r), FUNC(mv64361_device::pci0_mem2_remap_high_w));
	map(0x0338, 0x033b).rw(FUNC(mv64361_device::pci0_mem3_remap_high_r), FUNC(mv64361_device::pci0_mem3_remap_high_w));
	map(0x0340, 0x0343).rw(FUNC(mv64361_device::pci1_mem0_remap_high_r), FUNC(mv64361_device::pci1_mem0_remap_high_w));
	map(0x0348, 0x034b).rw(FUNC(mv64361_device::pci1_mem1_remap_high_r), FUNC(mv64361_device::pci1_mem1_remap_high_w));
	map(0x0350, 0x0353).rw(FUNC(mv64361_device::pci1_mem2_remap_high_r), FUNC(mv64361_device::pci1_mem2_remap_high_w));
	map(0x0358, 0x035b).rw(FUNC(mv64361_device::pci1_mem3_remap_high_r), FUNC(mv64361_device::pci1_mem3_remap_high_w));

	map(0x0c78, 0x0c7b).rw(m_pcihost[0], FUNC(mv64361_pci_host_device::config_address_r), FUNC(mv64361_pci_host_device::config_address_w));
	map(0x0c7c, 0x0c7f).rw(m_pcihost[0], FUNC(mv64361_pci_host_device::config_data_r), FUNC(mv64361_pci_host_device::config_data_w));
	map(0x0cf8, 0x0cfb).rw(m_pcihost[1], FUNC(mv64361_pci_host_device::config_address_r), FUNC(mv64361_pci_host_device::config_address_w));
	map(0x0cfc, 0x0cff).rw(m_pcihost[1], FUNC(mv64361_pci_host_device::config_data_r), FUNC(mv64361_pci_host_device::config_data_w));
}

uint32_t mv64361_device::cpu_config_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "cpu_config_r\n");

	return swapendian_int32(m_regs[REG_CPU_CONFIG]);
}

void mv64361_device::cpu_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "cpu_config_w: %08x\n", data);
	m_regs[REG_CPU_CONFIG] = (data & 0xe4e3bff) | (1 << 23); // handle reserved bits
}

uint32_t mv64361_device::pci0_io_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_io_base_r\n");

	return swapendian_int32(m_regs[REG_PCI0_IO_BASE]);
}

void mv64361_device::pci0_io_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_io_base_w: %08x\n", data);
	m_regs[REG_PCI0_IO_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_IO_REMAP] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci0_io_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_io_size_r\n");

	return swapendian_int32(m_regs[REG_PCI0_IO_SIZE]);
}

void mv64361_device::pci0_io_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_io_size_w: %08x\n", data);
	m_regs[REG_PCI0_IO_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem0_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem0_base_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM0_BASE]);
}

void mv64361_device::pci0_mem0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem0_base_w: %08x\n", data);
	m_regs[REG_PCI0_MEM0_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM0_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci0_mem0_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem0_size_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM0_SIZE]);
}

void mv64361_device::pci0_mem0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem0_size_w: %08x\n", data);
	m_regs[REG_PCI0_MEM0_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::internal_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "internal_base_r\n");

	return swapendian_int32(m_regs[REG_INTERNAL_BASE]);
}

void mv64361_device::internal_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "internal_base_w: %08x\n", data);
	m_regs[REG_INTERNAL_BASE] = data & 0x000fffff;
}

uint32_t mv64361_device::pci0_mem1_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem1_base_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM1_BASE]);
}

void mv64361_device::pci0_mem1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem1_base_w: %08x\n", data);
	m_regs[REG_PCI0_MEM1_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM1_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci0_mem1_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem1_size_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM1_SIZE]);
}

void mv64361_device::pci0_mem1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem1_size_w: %08x\n", data);
	m_regs[REG_PCI0_MEM1_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_io_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_io_base_r\n");

	return swapendian_int32(m_regs[REG_PCI1_IO_BASE]);
}

void mv64361_device::pci1_io_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_io_base_w: %08x\n", data);
	m_regs[REG_PCI1_IO_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_IO_REMAP] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci1_io_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_io_size_r\n");

	return swapendian_int32(m_regs[REG_PCI1_IO_SIZE]);
}

void mv64361_device::pci1_io_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_io_size_w: %08x\n", data);
	m_regs[REG_PCI1_IO_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem0_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem0_base_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM0_BASE]);
}

void mv64361_device::pci1_mem0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem0_base_w: %08x\n", data);
	m_regs[REG_PCI1_MEM0_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM0_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci1_mem0_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem0_size_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM0_SIZE]);
}

void mv64361_device::pci1_mem0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem0_size_w: %08x\n", data);
	m_regs[REG_PCI1_MEM0_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem1_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem1_base_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM1_BASE]);
}

void mv64361_device::pci1_mem1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem1_base_w: %08x\n", data);
	m_regs[REG_PCI1_MEM1_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM1_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci1_mem1_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem1_size_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM1_SIZE]);
}

void mv64361_device::pci1_mem1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem1_size_w: %08x\n", data);
	m_regs[REG_PCI0_MEM1_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_io_remap_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_io_remap_r\n");

	return swapendian_int32(m_regs[REG_PCI0_IO_REMAP]);
}

void mv64361_device::pci0_io_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_io_remap_w: %08x\n", data);
	m_regs[REG_PCI0_IO_REMAP] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem0_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem0_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM0_REMAP_LOW]);
}

void mv64361_device::pci0_mem0_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem0_remap_low_w: %08x\n", data);
	m_regs[REG_PCI0_MEM0_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem1_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem1_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM1_REMAP_LOW]);
}

void mv64361_device::pci0_mem1_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem1_remap_low_w: %08x\n", data);
	m_regs[REG_PCI0_MEM1_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_io_remap_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_io_remap_r\n");

	return swapendian_int32(m_regs[REG_PCI1_IO_REMAP]);
}

void mv64361_device::pci1_io_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_io_remap_w: %08x\n", data);
	m_regs[REG_PCI1_IO_REMAP] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem0_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem0_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM0_REMAP_LOW]);
}

void mv64361_device::pci1_mem0_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem0_remap_low_w: %08x\n", data);
	m_regs[REG_PCI1_MEM0_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem1_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem1_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM1_REMAP_LOW]);
}

void mv64361_device::pci1_mem1_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem1_remap_low_w: %08x\n", data);
	m_regs[REG_PCI1_MEM1_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::bootcs_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "bootcs_base_r\n");

	return swapendian_int32(m_regs[REG_BOOTCS_BASE]);
}

void mv64361_device::bootcs_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "bootcs_base_w: %08x\n", data);
	m_regs[REG_BOOTCS_BASE] = data & 0x000fffff;
}

uint32_t mv64361_device::bootcs_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "bootcs_size_r\n");

	return swapendian_int32(m_regs[REG_BOOTCS_SIZE]);
}

void mv64361_device::bootcs_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "bootcs_size_w: %08x\n", data);
	m_regs[REG_BOOTCS_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem2_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem2_base_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM2_BASE]);
}

void mv64361_device::pci0_mem2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem2_base_w: %08x\n", data);
	m_regs[REG_PCI0_MEM2_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM2_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci0_mem2_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem2_size_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM2_SIZE]);
}

void mv64361_device::pci0_mem2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem2_size_w: %08x\n", data);
	m_regs[REG_PCI0_MEM2_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::sram_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "sram_base_r\n");

	return swapendian_int32(m_regs[REG_SRAM_BASE]);
}

void mv64361_device::sram_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "sram_base_w: %08x\n", data);
	m_regs[REG_SRAM_BASE] = data & 0x000fffff;
}

uint32_t mv64361_device::base_address_enable_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "base_address_enable_r\n");

	return swapendian_int32(m_regs[REG_BASE_ADDRESS_ENABLE]);
}

void mv64361_device::base_address_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "base_address_enable_w: %08x\n", data);
	m_regs[REG_BASE_ADDRESS_ENABLE] = data & 0x001fffff;

	map_windows();
}

uint32_t mv64361_device::pci0_mem3_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem3_base_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM3_BASE]);
}

void mv64361_device::pci0_mem3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem3_base_w: %08x\n", data);
	m_regs[REG_PCI0_MEM3_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM3_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci0_mem3_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem3_size_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM3_SIZE]);
}

void mv64361_device::pci0_mem3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem3_size_w: %08x\n", data);
	m_regs[REG_PCI0_MEM3_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem2_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem2_base_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM2_BASE]);
}

void mv64361_device::pci1_mem2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem2_base_w: %08x\n", data);
	m_regs[REG_PCI1_MEM2_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM2_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci1_mem2_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem2_size_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM2_SIZE]);
}

void mv64361_device::pci1_mem2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem2_size_w: %08x\n", data);
	m_regs[REG_PCI1_MEM2_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem3_base_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem3_base_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM3_BASE]);
}

void mv64361_device::pci1_mem3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem3_base_w: %08x\n", data);
	m_regs[REG_PCI1_MEM3_BASE] = data & 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM3_REMAP_LOW] = data & 0x0000fffff;
}

uint32_t mv64361_device::pci1_mem3_size_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem3_size_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM3_SIZE]);
}

void mv64361_device::pci1_mem3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem3_size_w: %08x\n", data);
	m_regs[REG_PCI1_MEM3_SIZE] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem2_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem2_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM2_REMAP_LOW]);
}

void mv64361_device::pci0_mem2_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem2_remap_low_w: %08x\n", data);
	m_regs[REG_PCI0_MEM2_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem3_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem3_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM3_REMAP_LOW]);
}

void mv64361_device::pci0_mem3_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem3_remap_low_w: %08x\n", data);
	m_regs[REG_PCI0_MEM3_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem2_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem2_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM2_REMAP_LOW]);
}

void mv64361_device::pci1_mem2_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem2_remap_low_w: %08x\n", data);
	m_regs[REG_PCI1_MEM2_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem3_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem3_remap_low_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM3_REMAP_LOW]);
}

void mv64361_device::pci1_mem3_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem3_remap_low_w: %08x\n", data);
	m_regs[REG_PCI1_MEM3_REMAP_LOW] = data & 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem0_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem0_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM0_REMAP_HIGH]);
}

void mv64361_device::pci0_mem0_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem0_remap_high_w: %08x\n", data);
	m_regs[REG_PCI0_MEM0_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci0_mem1_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem1_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM1_REMAP_HIGH]);
}

void mv64361_device::pci0_mem1_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem1_remap_high_w: %08x\n", data);
	m_regs[REG_PCI0_MEM1_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci0_mem2_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem2_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM2_REMAP_HIGH]);
}

void mv64361_device::pci0_mem2_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem2_remap_high_w: %08x\n", data);
	m_regs[REG_PCI0_MEM2_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci0_mem3_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci0_mem3_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI0_MEM3_REMAP_HIGH]);
}

void mv64361_device::pci0_mem3_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci0_mem3_remap_high_w: %08x\n", data);
	m_regs[REG_PCI0_MEM3_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci1_mem0_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem0_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM0_REMAP_HIGH]);
}

void mv64361_device::pci1_mem0_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem0_remap_high_w: %08x\n", data);
	m_regs[REG_PCI1_MEM0_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci1_mem1_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem1_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM1_REMAP_HIGH]);
}

void mv64361_device::pci1_mem1_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem1_remap_high_w: %08x\n", data);
	m_regs[REG_PCI1_MEM1_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci1_mem2_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem2_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM2_REMAP_HIGH]);
}

void mv64361_device::pci1_mem2_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem2_remap_high_w: %08x\n", data);
	m_regs[REG_PCI1_MEM2_REMAP_HIGH] = data;
}

uint32_t mv64361_device::pci1_mem3_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REG_READ, "pci1_mem3_remap_high_r\n");

	return swapendian_int32(m_regs[REG_PCI1_MEM3_REMAP_HIGH]);
}

void mv64361_device::pci1_mem3_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	LOGMASKED(LOG_REG_WRITE, "pci1_mem3_remap_high_w: %08x\n", data);
	m_regs[REG_PCI1_MEM3_REMAP_HIGH] = data;
}

void mv64361_device::map_windows()
{
	for (int i = 0; i < 21; i++)
	{
		// unmap currently mapped ranges
		if (m_ranges[i].enabled)
		{
			m_cpu_space->unmap_readwrite(m_ranges[i].start, m_ranges[i].end);
			m_ranges[i].enabled = false;
		}

		if (BIT(m_regs[REG_BASE_ADDRESS_ENABLE], i) == 0)
		{
			m_ranges[i].enabled = true;

			switch (i)
			{
			case 0: // CS0
				m_ranges[0].enabled = false;
				logerror("Mapping CS0\n");
				break;

			case 1: // CS1
				m_ranges[1].enabled = false;
				logerror("Mapping CS1\n");
				break;

			case 2: // CS2
				m_ranges[2].enabled = false;
				logerror("Mapping CS2\n");
				break;

			case 3: // CS3
				m_ranges[3].enabled = false;
				logerror("Mapping CS3\n");
				break;

			case 4: // DEVCS0
				m_ranges[4].enabled = false;
				logerror("Mapping DEVCS0\n");
				break;

			case 5: // DEVCS1
				m_ranges[5].enabled = false;
				logerror("Mapping DEVCS1\n");
				break;

			case 6: // DEVCS2
				m_ranges[6].enabled = false;
				logerror("Mapping DEVCS2\n");
				break;

			case 7: // DEVCS3
				m_ranges[7].enabled = false;
				logerror("Mapping DEVCS3\n");
				break;

			case 8: // BOOTCS
				m_ranges[8].start = m_regs[REG_BOOTCS_BASE] << 16;
				m_ranges[8].end = (m_regs[REG_BOOTCS_BASE] << 16) + ((m_regs[REG_BOOTCS_SIZE] << 16) | 0xffff);
				m_ranges[8].enabled = false;
				logerror("MAP %08x-%08x BOOTCS (not supported)\n", m_ranges[8].start , m_ranges[8].end);
				break;

			case 9: // PCI0 I/O
				m_ranges[9].start = m_regs[REG_PCI0_IO_BASE] << 16;
				m_ranges[9].end = (m_regs[REG_PCI0_IO_BASE] << 16) + ((m_regs[REG_PCI0_IO_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI0 I/O (remap: %04x)\n", m_ranges[9].start, m_ranges[9].end, m_regs[REG_PCI0_IO_REMAP]);
				m_cpu_space->install_read_handler (m_ranges[9].start, m_ranges[9].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_io_r)));
				m_cpu_space->install_write_handler(m_ranges[9].start, m_ranges[9].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_io_w)));
				break;

			case 10: // PCI0 MEM0
				m_ranges[10].start = m_regs[REG_PCI0_MEM0_BASE] << 16;
				m_ranges[10].end = (m_regs[REG_PCI0_MEM0_BASE] << 16) + ((m_regs[REG_PCI0_MEM0_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI0 MEM0 (remap: %08x %04x)\n", m_ranges[10].start, m_ranges[10].end, m_regs[REG_PCI0_MEM0_REMAP_HIGH], m_regs[REG_PCI0_MEM0_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[10].start, m_ranges[10].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem0_r)));
				m_cpu_space->install_write_handler(m_ranges[10].start, m_ranges[10].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem0_w)));
				break;

			case 11: // PCI0 MEM1
				m_ranges[11].start = m_regs[REG_PCI0_MEM1_BASE] << 16;
				m_ranges[11].end = (m_regs[REG_PCI0_MEM1_BASE] << 16) + ((m_regs[REG_PCI0_MEM1_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI0 MEM1 (remap: %08x %04x)\n", m_ranges[11].start, m_ranges[11].end, m_regs[REG_PCI0_MEM1_REMAP_HIGH], m_regs[REG_PCI0_MEM1_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[11].start, m_ranges[11].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem1_r)));
				m_cpu_space->install_write_handler(m_ranges[11].start, m_ranges[11].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem1_w)));
				break;

			case 12: // PCI0 MEM2
				m_ranges[12].start = m_regs[REG_PCI0_MEM2_BASE] << 16;
				m_ranges[12].end = (m_regs[REG_PCI0_MEM2_BASE] << 16) + ((m_regs[REG_PCI0_MEM2_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI0 MEM2 (remap: %08x %04x)\n", m_ranges[12].start, m_ranges[12].end, m_regs[REG_PCI0_MEM2_REMAP_HIGH], m_regs[REG_PCI0_MEM2_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[12].start, m_ranges[12].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem2_r)));
				m_cpu_space->install_write_handler(m_ranges[12].start, m_ranges[12].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem2_w)));
				break;

			case 13: // PCI0 MEM3
				m_ranges[13].start = m_regs[REG_PCI0_MEM3_BASE] << 16;
				m_ranges[13].end = (m_regs[REG_PCI0_MEM3_BASE] << 16) + ((m_regs[REG_PCI0_MEM3_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI0 MEM3 (remap: %08x %04x)\n", m_ranges[13].start, m_ranges[13].end, m_regs[REG_PCI0_MEM3_REMAP_HIGH], m_regs[REG_PCI0_MEM3_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[13].start, m_ranges[13].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem3_r)));
				m_cpu_space->install_write_handler(m_ranges[13].start, m_ranges[13].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem3_w)));
				break;

			case 14: // PCI1 I/O
				m_ranges[14].start = m_regs[REG_PCI1_IO_BASE] << 16;
				m_ranges[14].end = (m_regs[REG_PCI1_IO_BASE] << 16) + ((m_regs[REG_PCI1_IO_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI1 I/O (remap: %04x)\n", m_ranges[14].start, m_ranges[14].end, m_regs[REG_PCI1_IO_REMAP]);
				m_cpu_space->install_read_handler (m_ranges[14].start, m_ranges[14].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_io_r)));
				m_cpu_space->install_write_handler(m_ranges[14].start, m_ranges[14].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_io_w)));
				break;

			case 15: // PCI1 MEM0
				m_ranges[15].start = m_regs[REG_PCI1_MEM0_BASE] << 16;
				m_ranges[15].end = (m_regs[REG_PCI1_MEM0_BASE] << 16) + ((m_regs[REG_PCI1_MEM0_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI1 MEM0 (remap: %08x %04x)\n", m_ranges[15].start, m_ranges[15].end, m_regs[REG_PCI1_MEM0_REMAP_HIGH], m_regs[REG_PCI1_MEM0_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[15].start, m_ranges[15].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem0_r)));
				m_cpu_space->install_write_handler(m_ranges[15].start, m_ranges[15].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem0_w)));
				break;

			case 16: // PCI1 MEM1
				m_ranges[16].start = m_regs[REG_PCI1_MEM1_BASE] << 16;
				m_ranges[16].end = (m_regs[REG_PCI1_MEM1_BASE] << 16) + ((m_regs[REG_PCI1_MEM1_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI1 MEM1 (remap: %08x %04x)\n", m_ranges[16].start, m_ranges[16].end, m_regs[REG_PCI1_MEM1_REMAP_HIGH], m_regs[REG_PCI1_MEM1_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[16].start, m_ranges[16].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem1_r)));
				m_cpu_space->install_write_handler(m_ranges[16].start, m_ranges[16].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem1_w)));

				break;

			case 17: // PCI1 MEM2
				m_ranges[17].start = m_regs[REG_PCI1_MEM2_BASE] << 16;
				m_ranges[17].end = (m_regs[REG_PCI1_MEM2_BASE] << 16) + ((m_regs[REG_PCI1_MEM2_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI1 MEM2 (remap: %08x %04x)\n", m_ranges[17].start, m_ranges[17].end, m_regs[REG_PCI1_MEM2_REMAP_HIGH], m_regs[REG_PCI1_MEM2_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[17].start, m_ranges[17].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem2_r)));
				m_cpu_space->install_write_handler(m_ranges[17].start, m_ranges[17].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem2_w)));
				break;

			case 18: // PCI1 MEM3
				m_ranges[18].start = m_regs[REG_PCI1_MEM3_BASE] << 16;
				m_ranges[18].end = (m_regs[REG_PCI1_MEM3_BASE] << 16) + ((m_regs[REG_PCI1_MEM3_SIZE] << 16) | 0xffff);
				logerror("MAP %08x-%08x PCI1 MEM3 (remap: %08x %04x)\n", m_ranges[18].start, m_ranges[18].end, m_regs[REG_PCI1_MEM3_REMAP_HIGH], m_regs[REG_PCI1_MEM3_REMAP_LOW]);
				m_cpu_space->install_read_handler (m_ranges[18].start, m_ranges[18].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem3_r)));
				m_cpu_space->install_write_handler(m_ranges[18].start, m_ranges[18].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem3_w)));
				break;

			case 19: // SRAM
				m_ranges[19].start = m_regs[REG_SRAM_BASE] << 16;
				m_ranges[19].end = (m_regs[REG_SRAM_BASE] << 16) + 0x1fffff;
				logerror("MAP %08x-%08x SRAM\n", m_ranges[19].start, m_ranges[19].end);
				m_cpu_space->install_ram(m_ranges[19].start, m_ranges[19].end, m_sram.get());
				break;

			case 20: // INTERNAL
				m_ranges[20].start = m_regs[REG_INTERNAL_BASE] << 16;
				m_ranges[20].end = (m_regs[REG_INTERNAL_BASE] << 16) + 0xffff;
				logerror("MAP %08x-%08x INTERNAL\n", m_ranges[20].start, m_ranges[20].end);
				m_cpu_space->install_device(m_ranges[20].start, m_ranges[20].end, *static_cast<mv64361_device *>(this), &mv64361_device::register_map);
				break;
			}
		}
	}
}

uint32_t mv64361_device::remap_offset(offs_t offset, uint16_t size, uint16_t remap)
{
	uint32_t size_mask = (size << 16) | 0xffff;
	return (offset & size_mask) | ((remap << 16) & ~size_mask);
}

uint32_t mv64361_device::pci0_io_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[0]->io_r(remap_offset(offset, m_regs[REG_PCI0_IO_SIZE], m_regs[REG_PCI0_IO_REMAP]), mem_mask);
}

void mv64361_device::pci0_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[0]->io_w(remap_offset(offset, m_regs[REG_PCI0_IO_SIZE], m_regs[REG_PCI0_IO_REMAP]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[0]->mem_r(remap_offset(offset, m_regs[REG_PCI0_MEM0_SIZE], m_regs[REG_PCI0_MEM0_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[0]->mem_w(remap_offset(offset, m_regs[REG_PCI0_MEM0_SIZE], m_regs[REG_PCI0_MEM0_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[0]->mem_r(remap_offset(offset, m_regs[REG_PCI0_MEM1_SIZE], m_regs[REG_PCI0_MEM1_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[0]->mem_w(remap_offset(offset, m_regs[REG_PCI0_MEM0_SIZE], m_regs[REG_PCI0_MEM1_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[0]->mem_r(remap_offset(offset, m_regs[REG_PCI0_MEM2_SIZE], m_regs[REG_PCI0_MEM2_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[0]->mem_w(remap_offset(offset, m_regs[REG_PCI0_MEM2_SIZE], m_regs[REG_PCI0_MEM2_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[0]->mem_r(remap_offset(offset, m_regs[REG_PCI0_MEM3_SIZE], m_regs[REG_PCI0_MEM3_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[0]->mem_w(remap_offset(offset, m_regs[REG_PCI0_MEM3_SIZE], m_regs[REG_PCI0_MEM3_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_io_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[1]->io_r(remap_offset(offset, m_regs[REG_PCI1_IO_SIZE], m_regs[REG_PCI1_IO_REMAP]), mem_mask);
}

void mv64361_device::pci1_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[1]->io_w(remap_offset(offset, m_regs[REG_PCI1_IO_SIZE], m_regs[REG_PCI1_IO_REMAP]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[1]->mem_r(remap_offset(offset, m_regs[REG_PCI1_MEM0_SIZE], m_regs[REG_PCI1_MEM0_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[1]->mem_w(remap_offset(offset, m_regs[REG_PCI1_MEM0_SIZE], m_regs[REG_PCI1_MEM0_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[1]->mem_r(remap_offset(offset, m_regs[REG_PCI1_MEM1_SIZE], m_regs[REG_PCI1_MEM1_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[1]->mem_w(remap_offset(offset, m_regs[REG_PCI1_MEM0_SIZE], m_regs[REG_PCI1_MEM1_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[1]->mem_r(remap_offset(offset, m_regs[REG_PCI1_MEM2_SIZE], m_regs[REG_PCI1_MEM2_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[1]->mem_w(remap_offset(offset, m_regs[REG_PCI1_MEM2_SIZE], m_regs[REG_PCI1_MEM2_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcihost[1]->mem_r(remap_offset(offset, m_regs[REG_PCI1_MEM3_SIZE], m_regs[REG_PCI1_MEM3_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_pcihost[1]->mem_w(remap_offset(offset, m_regs[REG_PCI1_MEM3_SIZE], m_regs[REG_PCI1_MEM3_REMAP_LOW]), data, mem_mask);
}


//**************************************************************************
//  MV64361 PCI HOST
//**************************************************************************

mv64361_pci_host_device::mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_host_device(mconfig, MV64361_PCI_HOST, tag, owner, clock),
	m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32),
	m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
{
	set_ids_host(0x11ab6460, 0x01, 0x00000000);
}

void mv64361_pci_host_device::device_start()
{
	pci_host_device::device_start();

	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;
}

void mv64361_pci_host_device::device_reset()
{
	pci_host_device::device_reset();
}

void mv64361_pci_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}

device_memory_interface::space_config_vector mv64361_pci_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

uint32_t mv64361_pci_host_device::config_address_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = pci_host_device::config_address_r();

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PCI_CONFIG, "config_address_r: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));

	return swapendian_int32(data);
}

void mv64361_pci_host_device::config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_CONFIG, "config_address_w: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));
	pci_host_device::config_address_w(0, swapendian_int32(data), swapendian_int32(mem_mask));
}

uint32_t mv64361_pci_host_device::config_data_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = pci_host_device::config_data_r(0, swapendian_int32(mem_mask));

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PCI_CONFIG, "config_data_r: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));

	return swapendian_int32(data);
}

void mv64361_pci_host_device::config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_CONFIG, "config_data_w: %08x & %08x\n", swapendian_int32(data), swapendian_int32(mem_mask));
	pci_host_device::config_data_w(0, swapendian_int32(data), swapendian_int32(mem_mask));
}

uint32_t mv64361_pci_host_device::io_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = space(AS_PCI_IO).read_dword(offset * 4, swapendian_int32(mem_mask));

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PCI_IO, "io_r[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));

	return swapendian_int32(data);
}

void mv64361_pci_host_device::io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_IO, "io_w[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
	space(AS_PCI_IO).write_dword(offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
}

uint32_t mv64361_pci_host_device::mem_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = space(AS_PCI_MEM).read_dword(offset * 4, swapendian_int32(mem_mask));

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PCI_MEM, "mem_r[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));

	return swapendian_int32(data);
}

void mv64361_pci_host_device::mem_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_MEM, "mem_w[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
	space(AS_PCI_MEM).write_dword(offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
}
