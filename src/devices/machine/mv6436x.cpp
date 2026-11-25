// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Marvell MV64360/1/2

    System controller for PowerPC processors

***************************************************************************/

#include "emu.h"
#include "mv6436x.h"
#include "endianness.h"

#define LOG_PCI_CONFIG  (1U << 1)
#define LOG_PCI_MEM     (1U << 2)
#define LOG_PCI_IO      (1U << 3)
#define LOG_REGS        (1U << 5)
#define LOG_MAPPING     (1U << 6)

#define VERBOSE (LOG_GENERAL | LOG_REGS | LOG_PCI_CONFIG | LOG_MAPPING)
#include "logmacro.h"


namespace
{

enum : uint8_t
{
	BAR_CS0 = 0,
	BAR_CS1,
	BAR_CS2,
	BAR_CS3,
	BAR_DEVCS0,
	BAR_DEVCS1,
	BAR_DEVCS2,
	BAR_DEVCS3,
	BAR_BOOTCS,
	BAR_P2P_MEM0,
	BAR_P2P_MEM1,
	BAR_P2P_IO,
	BAR_CPU,
	BAR_SRAM,
	BAR_EXPANSION_ROM
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(MV64361, mv64361_device, "mv64361", "Marvell MV64361 System Controller")
DEFINE_DEVICE_TYPE(MV64361_PCI_HOST, mv64361_pci_host_device, "mv64361_pci_host", "Marvell MV64361 PCI Host")
DEFINE_DEVICE_TYPE(MV64361_PCI_FUNCTION1, mv64361_pci_function1_device, "mv64361_pci_function1", "Marvell MV64361 PCI Function 1")


//**************************************************************************
//  MV64361 SYSTEM CONTROLLER
//**************************************************************************

mv64361_device::mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MV64361, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_pci_bus(*this, finder_base::DUMMY_TAG, 0U),
	m_pci_functions{ { *this, finder_base::DUMMY_TAG, 0U }, { *this, finder_base::DUMMY_TAG, 0U } },
	m_cpuint0_cb(*this), m_cpuint1_cb(*this),
	m_pci_iack_cb(*this, 0xff)
{
}

void mv64361_device::device_start()
{
	m_cpu_space = &m_cpu->space(AS_PROGRAM);

	// 2 MBit internal SRAM
	m_sram = std::make_unique<uint8_t[]>(0x40000);

	// assign CS# (SDRAM) memory shares from tags
	for (int i = 0; i < 4; i++)
		m_cs_share[i] = !m_cs_tag[i].empty() ? memshare(m_cs_tag[i]) : nullptr;

	// forward a reference to us to the PCI function devices
	for (unsigned bus = 0; bus < 2; bus++)
	{
		for (unsigned func = 0; func < 2; func++)
			m_pci_functions[bus][func]->set_syscon(*this, bus, func);
	}

	// register for save states
	save_item(NAME(m_regs));
	save_pointer(NAME(m_sram), 0x40000);

	// update the windows after loading state
	machine().save().register_postload(save_prepost_delegate(FUNC(mv64361_device::update_windows), this));
}

void mv64361_device::device_reset()
{
	// initialize internal registers
	m_regs[REG_CS0_BASE] = 0x00000000;
	m_regs[REG_CS0_SIZE] = 0x0000007f;
	m_regs[REG_CS1_BASE] = 0x00000080;
	m_regs[REG_CS1_SIZE] = 0x0000007f;
	m_regs[REG_CS2_BASE] = 0x00000100;
	m_regs[REG_CS2_SIZE] = 0x0000007f;
	m_regs[REG_CS3_BASE] = 0x00000180;
	m_regs[REG_CS3_SIZE] = 0x0000007f;
	m_regs[REG_BOOTCS_BASE] = 0x0000ff80;
	m_regs[REG_BOOTCS_SIZE] = 0x0000007f;
	m_regs[REG_PCI0_IO_BASE] = 0x01001000;
	m_regs[REG_PCI0_IO_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM0_BASE] = 0x01001200;
	m_regs[REG_PCI0_MEM0_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM1_BASE] = 0x0100f200;
	m_regs[REG_PCI0_MEM1_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM2_BASE] = 0x0100f400;
	m_regs[REG_PCI0_MEM2_SIZE] = 0x000001ff;
	m_regs[REG_PCI0_MEM3_BASE] = 0x0100f600;
	m_regs[REG_PCI0_MEM3_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_IO_BASE] = 0x01002000;
	m_regs[REG_PCI1_IO_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM0_BASE] = 0x00002200;
	m_regs[REG_PCI1_MEM0_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM1_BASE] = 0x01002400;
	m_regs[REG_PCI1_MEM1_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM2_BASE] = 0x01002600;
	m_regs[REG_PCI1_MEM2_SIZE] = 0x000001ff;
	m_regs[REG_PCI1_MEM3_BASE] = 0x01002800;
	m_regs[REG_PCI1_MEM3_SIZE] = 0x000001ff;
	m_regs[REG_SRAM_BASE] = 0x42000000;
	m_regs[REG_INTERNAL_BASE] = 0x01001400;
	m_regs[REG_BASE_ADDRESS_ENABLE] = 0x00000000;
	m_regs[REG_PCI0_IO_REMAP] = 0x00001000;
	m_regs[REG_PCI0_MEM0_REMAP_LOW] = 0x00001200;
	m_regs[REG_PCI0_MEM0_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_MEM1_REMAP_LOW] = 0x0000f200;
	m_regs[REG_PCI0_MEM1_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_MEM2_REMAP_LOW] = 0x0000f400;
	m_regs[REG_PCI0_MEM2_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_MEM3_REMAP_LOW] = 0x0000f600;
	m_regs[REG_PCI0_MEM3_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_IO_REMAP] = 0x00002000;
	m_regs[REG_PCI1_MEM0_REMAP_LOW] = 0x00002200;
	m_regs[REG_PCI1_MEM0_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_MEM1_REMAP_LOW] = 0x00002400;
	m_regs[REG_PCI1_MEM1_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_MEM2_REMAP_LOW] = 0x00002600;
	m_regs[REG_PCI1_MEM2_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_MEM3_REMAP_LOW] = 0x00002800;
	m_regs[REG_PCI1_MEM3_REMAP_HIGH] = 0x00000000;
	m_regs[REG_CPU_CONFIG] = 0x028000ff;
	m_regs[REG_PCI0_CS0_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI1_CS0_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI0_CS1_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI1_CS1_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI0_CS2_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI1_CS2_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI0_CS3_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI1_CS3_BAR_SIZE] = 0x007ff000;
	m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE] = 0xffff7c00;
	m_regs[REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE] = 0xffff7c00;
	m_regs[REG_PCI0_CS0_BASE_ADDRESS_REMAP] = 0x00000000;
	m_regs[REG_PCI1_CS0_BASE_ADDRESS_REMAP] = 0x00000000;
	m_regs[REG_PCI0_CS1_BASE_ADDRESS_REMAP] = 0x00800000;
	m_regs[REG_PCI1_CS1_BASE_ADDRESS_REMAP] = 0x00800000;
	m_regs[REG_PCI0_CS2_BASE_ADDRESS_REMAP] = 0x01000000;
	m_regs[REG_PCI1_CS2_BASE_ADDRESS_REMAP] = 0x01000000;
	m_regs[REG_PCI0_CS3_BASE_ADDRESS_REMAP] = 0x01800000;
	m_regs[REG_PCI1_CS3_BASE_ADDRESS_REMAP] = 0x01800000;
	m_regs[REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_LOW] = 0x40000000;
	m_regs[REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_LOW] = 0x40000000;
	m_regs[REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_HIGH] = 0x00000000;
	m_regs[REG_PCI0_PCI_ADDRESS_DECODE_CONTROL] = 0x00000008;
	m_regs[REG_PCI1_PCI_ADDRESS_DECODE_CONTROL] = 0x00000008;
	m_regs[REG_PCI0_COMMAND] = 0x0007e371;
	m_regs[REG_PCI1_COMMAND] = 0x0007e371;
	m_regs[REG_CUNIT_ARBITER_CONTROL] = 0x11ff0000;
	m_regs[REG_GPP_IO_CONTROL] = 0x00000000;
	m_regs[REG_GPP_LEVEL_CONTROL] = 0x00000000;
	m_regs[REG_GPP_VALUE] = 0x00000000;
	m_regs[REG_GPP_INTERRUPT_CAUSE] = 0x00000000;
	m_regs[REG_GPP_INTERRUPT_MASK0] = 0x00000000;
	m_regs[REG_GPP_INTERRUPT_MASK1] = 0x00000000;

	// pegasos2 specific override (TODO: make generic)
	m_regs[REG_INTERNAL_BASE] = 0x0100f100; // DevAD[5] pulled high

	// presumely these values are pulled from the configuration eeprom
	m_regs[REG_PCI1_IO_BASE] = 0x0100fe00;
	m_regs[REG_PCI1_IO_SIZE] = 0x000000ff;
	m_regs[REG_PCI1_IO_REMAP] = 0x00000000;
	m_regs[REG_PCI1_MEM1_BASE] = 0x0100fd00;
	m_regs[REG_PCI1_MEM1_SIZE] = 0x000000ff;
	m_regs[REG_PCI1_MEM1_REMAP_LOW] = 0x00000000;
	m_regs[REG_PCI1_MEM1_REMAP_HIGH] = 0x00000000;
	m_regs[REG_BASE_ADDRESS_ENABLE] = 0x00efbfff;

	for (auto &range : m_ranges)
		range.enabled = false;

	update_windows();
}

void mv64361_device::device_config_complete()
{
	for (unsigned bus = 0; bus < 2; bus++)
	{
		pci_root_device *const pci_bus = m_pci_bus[bus].lookup();
		if (!pci_bus)
			continue;

		for (device_t &dev : pci_bus->subdevices())
		{
			device_mv64361_pci_function_interface *func = nullptr;
			if (!dev.interface(func))
				continue;

			std::string_view basetag(dev.basetag());
			if (basetag.size() != 4 || basetag[2] != '.' || (basetag[3] != '0' && basetag[3] != '1'))
				fatalerror("%s: Invalid PCI function tag '%s'\n", this->tag(), dev.tag());

			unsigned const func_index = basetag[3] - '0';
			if (!m_pci_function_tag[bus][func_index].empty())
				fatalerror("%s: Duplicate PCI function %u ('%s')\n", this->tag(), func_index, dev.tag());

			if (func_index == 0 && !dynamic_cast<mv64361_pci_host_device *>(func))
				fatalerror("%s: PCI function 0 ('%s') must be MV64361_PCI_HOST\n", this->tag(), dev.tag());

			m_pci_function_tag[bus][func_index] = basetag;
			m_pci_functions[bus][func_index].set_tag(*pci_bus, m_pci_function_tag[bus][func_index].c_str());
		}
	}
}

// CPU access to registers
void mv64361_device::cpu_register_map(address_map &map)
{
	// CPU interface
	map(0x0000, 0x0003).rw(FUNC(mv64361_device::cpu_config_r), FUNC(mv64361_device::cpu_config_w));
	map(0x0008, 0x000b).rw(FUNC(mv64361_device::cs0_base_r), FUNC(mv64361_device::cs0_base_w));
	map(0x0010, 0x0013).rw(FUNC(mv64361_device::cs0_size_r), FUNC(mv64361_device::cs0_size_w));
	map(0x0018, 0x001b).rw(FUNC(mv64361_device::cs2_base_r), FUNC(mv64361_device::cs2_base_w));
	map(0x0020, 0x0023).rw(FUNC(mv64361_device::cs2_size_r), FUNC(mv64361_device::cs2_size_w));
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
	map(0x0208, 0x020b).rw(FUNC(mv64361_device::cs1_base_r), FUNC(mv64361_device::cs1_base_w));
	map(0x0210, 0x0213).rw(FUNC(mv64361_device::cs1_size_r), FUNC(mv64361_device::cs1_size_w));
	map(0x0218, 0x021b).rw(FUNC(mv64361_device::cs3_base_r), FUNC(mv64361_device::cs3_base_w));
	map(0x0220, 0x0223).rw(FUNC(mv64361_device::cs3_size_r), FUNC(mv64361_device::cs3_size_w));
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
	// PCI interface
	map(0x0c00, 0x0c03).rw(FUNC(mv64361_device::pci0_command_r), FUNC(mv64361_device::pci0_command_w));
	map(0x0c08, 0x0c0b).rw(FUNC(mv64361_device::pci0_cs0_bar_size_r), FUNC(mv64361_device::pci0_cs0_bar_size_w));
	map(0x0c0c, 0x0c0f).rw(FUNC(mv64361_device::pci0_cs2_bar_size_r), FUNC(mv64361_device::pci0_cs2_bar_size_w));
	map(0x0c34, 0x0c37).r(FUNC(mv64361_device::pci_interrupt_acknowledge_r<0>));
	map(0x0c3c, 0x0c3f).rw(FUNC(mv64361_device::pci0_base_address_registers_enable_r), FUNC(mv64361_device::pci0_base_address_registers_enable_w));
	map(0x0c48, 0x0c4b).rw(FUNC(mv64361_device::pci0_cs0_base_address_remap_r), FUNC(mv64361_device::pci0_cs0_base_address_remap_w));
	map(0x0c4c, 0x0c4f).rw(FUNC(mv64361_device::pci0_cs2_base_address_remap_r), FUNC(mv64361_device::pci0_cs2_base_address_remap_w));
	map(0x0c78, 0x0c7b).rw(FUNC(mv64361_device::config_address_r<1>), FUNC(mv64361_device::config_address_w<1>));
	map(0x0c7c, 0x0c7f).rw(FUNC(mv64361_device::config_data_r<1>), FUNC(mv64361_device::config_data_w<1>));
	map(0x0c80, 0x0c83).rw(FUNC(mv64361_device::pci1_command_r), FUNC(mv64361_device::pci1_command_w));
	map(0x0c88, 0x0c8b).rw(FUNC(mv64361_device::pci1_cs0_bar_size_r), FUNC(mv64361_device::pci1_cs0_bar_size_w));
	map(0x0c8c, 0x0c8f).rw(FUNC(mv64361_device::pci1_cs2_bar_size_r), FUNC(mv64361_device::pci1_cs2_bar_size_w));
	map(0x0cb4, 0x0cb7).r(FUNC(mv64361_device::pci_interrupt_acknowledge_r<1>));
	map(0x0cbc, 0x0cbf).rw(FUNC(mv64361_device::pci1_base_address_registers_enable_r), FUNC(mv64361_device::pci1_base_address_registers_enable_w));
	map(0x0cc8, 0x0ccb).rw(FUNC(mv64361_device::pci1_cs0_base_address_remap_r), FUNC(mv64361_device::pci1_cs0_base_address_remap_w));
	map(0x0ccc, 0x0ccf).rw(FUNC(mv64361_device::pci1_cs2_base_address_remap_r), FUNC(mv64361_device::pci1_cs2_base_address_remap_w));
	map(0x0cf8, 0x0cfb).rw(FUNC(mv64361_device::config_address_r<0>), FUNC(mv64361_device::config_address_w<0>));
	map(0x0cfc, 0x0cff).rw(FUNC(mv64361_device::config_data_r<0>), FUNC(mv64361_device::config_data_w<0>));
	map(0x0d08, 0x0d0b).rw(FUNC(mv64361_device::pci0_cs1_bar_size_r), FUNC(mv64361_device::pci0_cs1_bar_size_w));
	map(0x0d0c, 0x0d0f).rw(FUNC(mv64361_device::pci0_cs3_bar_size_r), FUNC(mv64361_device::pci0_cs3_bar_size_w));
	map(0x0d3c, 0x0d3f).rw(FUNC(mv64361_device::pci0_pci_address_decode_control_r), FUNC(mv64361_device::pci0_pci_address_decode_control_w));
	map(0x0d48, 0x0d4b).rw(FUNC(mv64361_device::pci0_cs1_base_address_remap_r), FUNC(mv64361_device::pci0_cs1_base_address_remap_w));
	map(0x0d4c, 0x0d4f).rw(FUNC(mv64361_device::pci0_cs3_base_address_remap_r), FUNC(mv64361_device::pci0_cs3_base_address_remap_w));
	map(0x0d70, 0x0d73).rw(FUNC(mv64361_device::pci0_pci_cpu_base_address_remap_low_r), FUNC(mv64361_device::pci0_pci_cpu_base_address_remap_low_w));
	map(0x0d74, 0x0d77).rw(FUNC(mv64361_device::pci0_pci_cpu_base_address_remap_high_r), FUNC(mv64361_device::pci0_pci_cpu_base_address_remap_high_w));
	map(0x0d88, 0x0d8b).rw(FUNC(mv64361_device::pci1_cs1_bar_size_r), FUNC(mv64361_device::pci1_cs1_bar_size_w));
	map(0x0d8c, 0x0d8f).rw(FUNC(mv64361_device::pci1_cs3_bar_size_r), FUNC(mv64361_device::pci1_cs3_bar_size_w));
	map(0x0dbc, 0x0dbf).rw(FUNC(mv64361_device::pci1_pci_address_decode_control_r), FUNC(mv64361_device::pci1_pci_address_decode_control_w));
	map(0x0dc8, 0x0dcb).rw(FUNC(mv64361_device::pci1_cs1_base_address_remap_r), FUNC(mv64361_device::pci1_cs1_base_address_remap_w));
	map(0x0dcc, 0x0dcf).rw(FUNC(mv64361_device::pci1_cs3_base_address_remap_r), FUNC(mv64361_device::pci1_cs3_base_address_remap_w));
	map(0x0df0, 0x0df3).rw(FUNC(mv64361_device::pci1_pci_cpu_base_address_remap_low_r), FUNC(mv64361_device::pci1_pci_cpu_base_address_remap_low_w));
	map(0x0df4, 0x0df7).rw(FUNC(mv64361_device::pci1_pci_cpu_base_address_remap_high_r), FUNC(mv64361_device::pci1_pci_cpu_base_address_remap_high_w));
	// General Purpose Port interface
	map(0xf100, 0xf103).rw(FUNC(mv64361_device::gpp_io_control_r), FUNC(mv64361_device::gpp_io_control_w));
	map(0xf104, 0xf107).rw(FUNC(mv64361_device::gpp_value_r), FUNC(mv64361_device::gpp_value_w));
	map(0xf108, 0xf10b).rw(FUNC(mv64361_device::gpp_interrupt_cause_r), FUNC(mv64361_device::gpp_interrupt_cause_w));
	map(0xf10c, 0xf10f).rw(FUNC(mv64361_device::gpp_interrupt_mask0_r), FUNC(mv64361_device::gpp_interrupt_mask0_w));
	map(0xf110, 0xf113).rw(FUNC(mv64361_device::gpp_level_control_r), FUNC(mv64361_device::gpp_level_control_w));
	map(0xf114, 0xf117).rw(FUNC(mv64361_device::gpp_interrupt_mask1_r), FUNC(mv64361_device::gpp_interrupt_mask1_w));
	map(0xf118, 0xf11b).rw(FUNC(mv64361_device::gpp_value_set_r), FUNC(mv64361_device::gpp_value_set_w));
	map(0xf11c, 0xf11f).rw(FUNC(mv64361_device::gpp_value_clear_r), FUNC(mv64361_device::gpp_value_clear_w));
	// Communication Unit interface
	map(0xf300, 0xf303).rw(FUNC(mv64361_device::cunit_arbiter_control_r), FUNC(mv64361_device::cunit_arbiter_control_w));
}

// PCI device access to internal registers
void mv64361_device::pci_register_map(address_map &map)
{
	// TODO
}


//**************************************************************************
//  CPU INTERFACE
//**************************************************************************

uint32_t mv64361_device::cpu_config_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CPU_CONFIG]);
}

void mv64361_device::cpu_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cpu_config_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CPU_CONFIG]);
	m_regs[REG_CPU_CONFIG] = (m_regs[REG_CPU_CONFIG] & 0x0e4e3bff) | (1 << 23); // handle reserved bits
}

uint32_t mv64361_device::cs0_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS0_BASE]);
}

void mv64361_device::cs0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs0_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS0_BASE]);
	m_regs[REG_CS0_BASE] &= 0x000fffff;
	update_window(0);
}

uint32_t mv64361_device::cs0_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS0_SIZE]);
}

void mv64361_device::cs0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs0_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS0_SIZE]);
	m_regs[REG_CS0_SIZE] &= 0x0000ffff;
	update_window(0);
}

uint32_t mv64361_device::cs1_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS1_BASE]);
}

void mv64361_device::cs1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs1_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS1_BASE]);
	m_regs[REG_CS1_BASE] &= 0x000fffff;
	update_window(1);
}

uint32_t mv64361_device::cs1_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS1_SIZE]);
}

void mv64361_device::cs1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs1_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS1_SIZE]);
	m_regs[REG_CS1_SIZE] &= 0x0000ffff;
	update_window(1);
}

uint32_t mv64361_device::cs2_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS2_BASE]);
}

void mv64361_device::cs2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs2_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS2_BASE]);
	m_regs[REG_CS2_BASE] &= 0x000fffff;
	update_window(2);
}

uint32_t mv64361_device::cs2_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS2_SIZE]);
}

void mv64361_device::cs2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs2_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS2_SIZE]);
	m_regs[REG_CS2_SIZE] &= 0x0000ffff;
	update_window(2);
}

uint32_t mv64361_device::cs3_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS3_BASE]);
}

void mv64361_device::cs3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs3_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS3_BASE]);
	m_regs[REG_CS3_BASE] &= 0x000fffff;
	update_window(3);
}

uint32_t mv64361_device::cs3_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CS3_SIZE]);
}

void mv64361_device::cs3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cs3_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CS3_SIZE]);
	m_regs[REG_CS3_SIZE] &= 0x0000ffff;
	update_window(3);
}

uint32_t mv64361_device::pci0_io_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_IO_BASE]);
}

void mv64361_device::pci0_io_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_io_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_IO_BASE]);
	m_regs[REG_PCI0_IO_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_IO_REMAP] = m_regs[REG_PCI0_IO_BASE] & 0x0000fffff;

	update_window(9);
}

uint32_t mv64361_device::pci0_io_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_IO_SIZE]);
}

void mv64361_device::pci0_io_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_io_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_IO_SIZE]);
	m_regs[REG_PCI0_IO_SIZE] &= 0x0000ffff;
	update_window(9);
}

uint32_t mv64361_device::pci0_mem0_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM0_BASE]);
}

void mv64361_device::pci0_mem0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem0_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM0_BASE]);
	m_regs[REG_PCI0_MEM0_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM0_REMAP_LOW] = m_regs[REG_PCI0_MEM0_BASE] & 0x0000fffff;

	update_window(10);
}

uint32_t mv64361_device::pci0_mem0_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM0_SIZE]);
}

void mv64361_device::pci0_mem0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem0_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM0_SIZE]);
	m_regs[REG_PCI0_MEM0_SIZE] &= 0x0000ffff;
	update_window(10);
}

uint32_t mv64361_device::internal_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_INTERNAL_BASE]);
}

void mv64361_device::internal_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "internal_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_INTERNAL_BASE]);
	m_regs[REG_INTERNAL_BASE] &= 0x000fffff;
	update_window(20);
}

uint32_t mv64361_device::pci0_mem1_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM1_BASE]);
}

void mv64361_device::pci0_mem1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem1_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM1_BASE]);
	m_regs[REG_PCI0_MEM1_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM1_REMAP_LOW] = m_regs[REG_PCI0_MEM1_BASE] & 0x0000fffff;

	update_window(11);
}

uint32_t mv64361_device::pci0_mem1_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM1_SIZE]);
}

void mv64361_device::pci0_mem1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem1_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM1_SIZE]);
	m_regs[REG_PCI0_MEM1_SIZE] &= 0x0000ffff;
	update_window(11);
}

uint32_t mv64361_device::pci1_io_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_IO_BASE]);
}

void mv64361_device::pci1_io_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_io_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_IO_BASE]);
	m_regs[REG_PCI1_IO_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_IO_REMAP] = m_regs[REG_PCI1_IO_BASE] & 0x0000fffff;

	update_window(14);
}

uint32_t mv64361_device::pci1_io_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_IO_SIZE]);
}

void mv64361_device::pci1_io_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_io_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_IO_SIZE]);
	m_regs[REG_PCI1_IO_SIZE] &= 0x0000ffff;
	update_window(14);
}

uint32_t mv64361_device::pci1_mem0_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM0_BASE]);
}

void mv64361_device::pci1_mem0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem0_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM0_BASE]);
	m_regs[REG_PCI1_MEM0_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM0_REMAP_LOW] = m_regs[REG_PCI1_MEM0_BASE] & 0x0000fffff;

	update_window(15);
}

uint32_t mv64361_device::pci1_mem0_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM0_SIZE]);
}

void mv64361_device::pci1_mem0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem0_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM0_SIZE]);
	m_regs[REG_PCI1_MEM0_SIZE] &= 0x0000ffff;
	update_window(15);
}

uint32_t mv64361_device::pci1_mem1_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM1_BASE]);
}

void mv64361_device::pci1_mem1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem1_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM1_BASE]);
	m_regs[REG_PCI1_MEM1_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM1_REMAP_LOW] = m_regs[REG_PCI1_MEM1_BASE] & 0x0000fffff;

	update_window(16);
}

uint32_t mv64361_device::pci1_mem1_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM1_SIZE]);
}

void mv64361_device::pci1_mem1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem1_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM1_SIZE]);
	m_regs[REG_PCI1_MEM1_SIZE] &= 0x0000ffff;
	update_window(16);
}

uint32_t mv64361_device::pci0_io_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_IO_REMAP]);
}

void mv64361_device::pci0_io_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_io_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_IO_REMAP]);
	m_regs[REG_PCI0_IO_REMAP] &= 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem0_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM0_REMAP_LOW]);
}

void mv64361_device::pci0_mem0_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem0_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM0_REMAP_LOW]);
	m_regs[REG_PCI0_MEM0_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem1_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM1_REMAP_LOW]);
}

void mv64361_device::pci0_mem1_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem1_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM1_REMAP_LOW]);
	m_regs[REG_PCI0_MEM1_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci1_io_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_IO_REMAP]);
}

void mv64361_device::pci1_io_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_io_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_IO_REMAP]);
	m_regs[REG_PCI1_IO_REMAP] &= 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem0_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM0_REMAP_LOW]);
}

void mv64361_device::pci1_mem0_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem0_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM0_REMAP_LOW]);
	m_regs[REG_PCI1_MEM0_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem1_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM1_REMAP_LOW]);
}

void mv64361_device::pci1_mem1_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem1_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM1_REMAP_LOW]);
	m_regs[REG_PCI1_MEM1_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::bootcs_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_BOOTCS_BASE]);
}

void mv64361_device::bootcs_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "bootcs_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_BOOTCS_BASE]);
	m_regs[REG_BOOTCS_BASE] &= 0x000fffff;
	update_window(8);
}

uint32_t mv64361_device::bootcs_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_BOOTCS_SIZE]);
}

void mv64361_device::bootcs_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "bootcs_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_BOOTCS_SIZE]);
	m_regs[REG_BOOTCS_SIZE] &= 0x0000ffff;
	update_window(8);
}

uint32_t mv64361_device::pci0_mem2_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM2_BASE]);
}

void mv64361_device::pci0_mem2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem2_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM2_BASE]);
	m_regs[REG_PCI0_MEM2_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM2_REMAP_LOW] = m_regs[REG_PCI0_MEM2_BASE] & 0x0000fffff;

	update_window(12);
}

uint32_t mv64361_device::pci0_mem2_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM2_SIZE]);
}

void mv64361_device::pci0_mem2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem2_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM2_SIZE]);
	m_regs[REG_PCI0_MEM2_SIZE] &= 0x0000ffff;
	update_window(12);
}

uint32_t mv64361_device::sram_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_SRAM_BASE]);
}

void mv64361_device::sram_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "sram_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_SRAM_BASE]);
	m_regs[REG_SRAM_BASE] &= 0x000fffff;
	update_window(19);
}

uint32_t mv64361_device::base_address_enable_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_BASE_ADDRESS_ENABLE]);
}

void mv64361_device::base_address_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "base_address_enable_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_BASE_ADDRESS_ENABLE]);
	m_regs[REG_BASE_ADDRESS_ENABLE] &= 0x001fffff;
	update_windows();
}

uint32_t mv64361_device::pci0_mem3_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM3_BASE]);
}

void mv64361_device::pci0_mem3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem3_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM3_BASE]);
	m_regs[REG_PCI0_MEM3_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI0_MEM3_REMAP_LOW] = m_regs[REG_PCI0_MEM3_BASE] & 0x0000fffff;

	update_window(13);
}

uint32_t mv64361_device::pci0_mem3_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM3_SIZE]);
}

void mv64361_device::pci0_mem3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem3_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM3_SIZE]);
	m_regs[REG_PCI0_MEM3_SIZE] &= 0x0000ffff;
	update_window(13);
}

uint32_t mv64361_device::pci1_mem2_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM2_BASE]);
}

void mv64361_device::pci1_mem2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem2_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM2_BASE]);
	m_regs[REG_PCI1_MEM2_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM2_REMAP_LOW] = m_regs[REG_PCI1_MEM2_BASE] & 0x0000fffff;

	update_window(17);
}

uint32_t mv64361_device::pci1_mem2_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM2_SIZE]);
}

void mv64361_device::pci1_mem2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem2_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM2_SIZE]);
	m_regs[REG_PCI1_MEM2_SIZE] &= 0x0000ffff;
	update_window(17);
}

uint32_t mv64361_device::pci1_mem3_base_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM3_BASE]);
}

void mv64361_device::pci1_mem3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem3_base_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM3_BASE]);
	m_regs[REG_PCI1_MEM3_BASE] &= 0x030fffff;

	if (BIT(m_regs[REG_CPU_CONFIG], 27) == 0)
		m_regs[REG_PCI1_MEM3_REMAP_LOW] = m_regs[REG_PCI1_MEM3_BASE] & 0x0000fffff;

	update_window(18);
}

uint32_t mv64361_device::pci1_mem3_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM3_SIZE]);
}

void mv64361_device::pci1_mem3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem3_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM3_SIZE]);
	m_regs[REG_PCI1_MEM3_SIZE] &= 0x0000ffff;
	update_window(18);
}

uint32_t mv64361_device::pci0_mem2_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM2_REMAP_LOW]);
}

void mv64361_device::pci0_mem2_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem2_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM2_REMAP_LOW]);
	m_regs[REG_PCI0_MEM2_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem3_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM3_REMAP_LOW]);
}

void mv64361_device::pci0_mem3_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem3_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM3_REMAP_LOW]);
	m_regs[REG_PCI0_MEM3_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem2_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM2_REMAP_LOW]);
}

void mv64361_device::pci1_mem2_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem2_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM2_REMAP_LOW]);
	m_regs[REG_PCI1_MEM2_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci1_mem3_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM3_REMAP_LOW]);
}

void mv64361_device::pci1_mem3_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem3_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM3_REMAP_LOW]);
	m_regs[REG_PCI1_MEM3_REMAP_LOW] &= 0x0000ffff;
}

uint32_t mv64361_device::pci0_mem0_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM0_REMAP_HIGH]);
}

void mv64361_device::pci0_mem0_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem0_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM0_REMAP_HIGH]);
}

uint32_t mv64361_device::pci0_mem1_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM1_REMAP_HIGH]);
}

void mv64361_device::pci0_mem1_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem1_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM1_REMAP_HIGH]);
}

uint32_t mv64361_device::pci0_mem2_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM2_REMAP_HIGH]);
}

void mv64361_device::pci0_mem2_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem2_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM2_REMAP_HIGH]);
}

uint32_t mv64361_device::pci0_mem3_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_MEM3_REMAP_HIGH]);
}

void mv64361_device::pci0_mem3_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_mem3_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_MEM3_REMAP_HIGH]);
}

uint32_t mv64361_device::pci1_mem0_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM0_REMAP_HIGH]);
}

void mv64361_device::pci1_mem0_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem0_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM0_REMAP_HIGH]);
}

uint32_t mv64361_device::pci1_mem1_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM1_REMAP_HIGH]);
}

void mv64361_device::pci1_mem1_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem1_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM1_REMAP_HIGH]);
}

uint32_t mv64361_device::pci1_mem2_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM2_REMAP_HIGH]);
}

void mv64361_device::pci1_mem2_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem2_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM2_REMAP_HIGH]);
}

uint32_t mv64361_device::pci1_mem3_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_MEM3_REMAP_HIGH]);
}

void mv64361_device::pci1_mem3_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_mem3_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_MEM3_REMAP_HIGH]);
}


//**************************************************************************
//  PCI INTERFACE
//**************************************************************************

uint32_t mv64361_device::pci0_command_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_COMMAND]);
}

void mv64361_device::pci0_command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const old_command = m_regs[REG_PCI0_COMMAND];
	data = swapendian_int32(data) & ~0xec40200e;
	data |= 0x00002000; // always set
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_command_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_COMMAND]);

	if (BIT(old_command ^ m_regs[REG_PCI0_COMMAND], 23))
		downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->remap_cb();
}

uint32_t mv64361_device::pci0_cs0_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS0_BAR_SIZE]);
}

void mv64361_device::pci0_cs0_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs0_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS0_BAR_SIZE]);

	m_pci_functions[0][0]->set_bar_size(BAR_CS0, m_regs[REG_PCI0_CS0_BAR_SIZE]);
}

uint32_t mv64361_device::pci0_cs2_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS2_BAR_SIZE]);
}

void mv64361_device::pci0_cs2_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs2_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS2_BAR_SIZE]);

	m_pci_functions[0][1]->set_bar_size(BAR_CS2, m_regs[REG_PCI0_CS2_BAR_SIZE]);
}

uint32_t mv64361_device::pci0_base_address_registers_enable_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE]);
}

void mv64361_device::pci0_base_address_registers_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xffff0000;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_base_address_registers_enable_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE]);

	// internal registers cannot be completely disabled
	if (BIT(m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE], 9) && BIT(m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE], 10))
		m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE] &= ~(1 << 9);

	m_pci_functions[0][0]->set_bar_enable(m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE]);
	m_pci_functions[0][1]->set_bar_enable(m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE]);
}

uint32_t mv64361_device::pci0_cs0_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS0_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci0_cs0_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs0_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS0_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci0_cs2_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS2_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci0_cs2_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs2_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS2_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci1_command_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_COMMAND]);
}

void mv64361_device::pci1_command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const old_command = m_regs[REG_PCI1_COMMAND];
	data = swapendian_int32(data) & ~0xec40200e;
	data |= 0x00002000; // always set
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_command_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_COMMAND]);

	if (BIT(old_command ^ m_regs[REG_PCI1_COMMAND], 23))
		downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->remap_cb();
}

uint32_t mv64361_device::pci1_cs0_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS0_BAR_SIZE]);
}

void mv64361_device::pci1_cs0_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs0_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS0_BAR_SIZE]);

	m_pci_functions[1][0]->set_bar_size(BAR_CS0, m_regs[REG_PCI1_CS0_BAR_SIZE]);
}

uint32_t mv64361_device::pci1_cs2_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS2_BAR_SIZE]);
}

void mv64361_device::pci1_cs2_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs2_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS2_BAR_SIZE]);

	m_pci_functions[1][1]->set_bar_size(BAR_CS2, m_regs[REG_PCI1_CS2_BAR_SIZE]);
}

uint32_t mv64361_device::pci1_base_address_registers_enable_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE]);
}

void mv64361_device::pci1_base_address_registers_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xffff0000;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_base_address_registers_enable_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE]);

	m_pci_functions[1][0]->set_bar_enable(m_regs[REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE]);
	m_pci_functions[1][1]->set_bar_enable(m_regs[REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE]);
}

uint32_t mv64361_device::pci1_cs0_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS0_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci1_cs0_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs0_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS0_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci1_cs2_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS2_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci1_cs2_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs2_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS2_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci0_cs1_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS1_BAR_SIZE]);
}

void mv64361_device::pci0_cs1_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs1_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS1_BAR_SIZE]);

	m_pci_functions[0][0]->set_bar_size(BAR_CS1, m_regs[REG_PCI0_CS1_BAR_SIZE]);
}

uint32_t mv64361_device::pci0_cs3_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS3_BAR_SIZE]);
}

void mv64361_device::pci0_cs3_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs3_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS3_BAR_SIZE]);

	m_pci_functions[0][1]->set_bar_size(BAR_CS3, m_regs[REG_PCI0_CS3_BAR_SIZE]);
}

uint32_t mv64361_device::pci0_pci_address_decode_control_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_PCI_ADDRESS_DECODE_CONTROL]);
}

void mv64361_device::pci0_pci_address_decode_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xfe0000f6;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_pci_address_decode_control_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_PCI_ADDRESS_DECODE_CONTROL]);
}

uint32_t mv64361_device::pci0_cs1_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS1_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci0_cs1_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs1_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS1_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci0_cs3_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_CS3_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci0_cs3_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_cs3_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_CS3_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci0_pci_cpu_base_address_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_LOW]);
}

void mv64361_device::pci0_pci_cpu_base_address_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_pci_cpu_base_address_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_LOW]);
}

uint32_t mv64361_device::pci0_pci_cpu_base_address_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_HIGH]);
}

void mv64361_device::pci0_pci_cpu_base_address_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xfffffff0;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci0_pci_cpu_base_address_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_HIGH]);
}

uint32_t mv64361_device::pci1_cs1_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS1_BAR_SIZE]);
}

void mv64361_device::pci1_cs1_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs1_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS1_BAR_SIZE]);

	m_pci_functions[1][0]->set_bar_size(BAR_CS1, m_regs[REG_PCI1_CS1_BAR_SIZE]);
}

uint32_t mv64361_device::pci1_cs3_bar_size_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS3_BAR_SIZE]);
}

void mv64361_device::pci1_cs3_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs3_bar_size_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS3_BAR_SIZE]);

	m_pci_functions[1][1]->set_bar_size(BAR_CS3, m_regs[REG_PCI1_CS3_BAR_SIZE]);
}

uint32_t mv64361_device::pci1_pci_address_decode_control_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_PCI_ADDRESS_DECODE_CONTROL]);
}

void mv64361_device::pci1_pci_address_decode_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xfe0000f6;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_pci_address_decode_control_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_PCI_ADDRESS_DECODE_CONTROL]);
}

uint32_t mv64361_device::pci1_cs1_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS1_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci1_cs1_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs1_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS1_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci1_cs3_base_address_remap_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_CS3_BASE_ADDRESS_REMAP]);
}

void mv64361_device::pci1_cs3_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_cs3_base_address_remap_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_CS3_BASE_ADDRESS_REMAP]);
}

uint32_t mv64361_device::pci1_pci_cpu_base_address_remap_low_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_LOW]);
}

void mv64361_device::pci1_pci_cpu_base_address_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0x00000fff;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_pci_cpu_base_address_remap_low_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_LOW]);
}

uint32_t mv64361_device::pci1_pci_cpu_base_address_remap_high_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_HIGH]);
}

void mv64361_device::pci1_pci_cpu_base_address_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xfffffff0;
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "pci1_pci_cpu_base_address_remap_high_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_HIGH]);
}


//**************************************************************************
//  GENERAL PURPOSE PORT INTERFACE / INTERRUPTS
//**************************************************************************

template <int Bus>
uint32_t mv64361_device::pci_interrupt_acknowledge_r()
{
	return swapendian_int32(uint32_t(m_pci_iack_cb[Bus]()));
}

uint32_t mv64361_device::gpp_io_control_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_GPP_IO_CONTROL]);
}

void mv64361_device::gpp_io_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_io_control_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_GPP_IO_CONTROL]);
}

uint32_t mv64361_device::gpp_level_control_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_GPP_LEVEL_CONTROL]);
}

void mv64361_device::gpp_level_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_level_control_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_GPP_LEVEL_CONTROL]);
}

uint32_t mv64361_device::gpp_value_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_GPP_VALUE]);
}

void mv64361_device::gpp_value_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_value_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_GPP_VALUE]);

	// TODO: output to MPP pins
}

uint32_t mv64361_device::gpp_interrupt_cause_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_GPP_INTERRUPT_CAUSE]);
}

void mv64361_device::gpp_interrupt_cause_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_interrupt_cause_w: %08x & %08x\n", data, mem_mask);
	m_regs[REG_GPP_INTERRUPT_CAUSE] &= (data | ~mem_mask);

	update_interrupts();
}

uint32_t mv64361_device::gpp_interrupt_mask0_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_GPP_INTERRUPT_MASK0]);
}

void mv64361_device::gpp_interrupt_mask0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_interrupt_mask0_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_GPP_INTERRUPT_MASK0]);

	update_interrupts();
}

uint32_t mv64361_device::gpp_interrupt_mask1_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_GPP_INTERRUPT_MASK1]);
}

void mv64361_device::gpp_interrupt_mask1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_interrupt_mask1_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_GPP_INTERRUPT_MASK1]);

	update_interrupts();
}

uint32_t mv64361_device::gpp_value_set_r(offs_t offset, uint32_t mem_mask)
{
	return 0;
}

void mv64361_device::gpp_value_set_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_value_set_w: %08x & %08x\n", data, mem_mask);
	m_regs[REG_GPP_VALUE] |= (data & mem_mask);
}

uint32_t mv64361_device::gpp_value_clear_r(offs_t offset, uint32_t mem_mask)
{
	return 0;
}

void mv64361_device::gpp_value_clear_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "gpp_value_clear_w: %08x & %08x\n", data, mem_mask);
	m_regs[REG_GPP_VALUE] &= ~(data & mem_mask);
}

void mv64361_device::mpp31_w(int state)
{
	state ^= BIT(m_regs[REG_GPP_LEVEL_CONTROL], 31);

	if (BIT(m_regs[REG_GPP_IO_CONTROL], 31) == 0)
	{
		m_regs[REG_GPP_VALUE] &= ~(1 << 31);
		m_regs[REG_GPP_VALUE] |= (state << 31);

		if (state)
			m_regs[REG_GPP_INTERRUPT_CAUSE] |= (state << 31);
	}

	update_interrupts();
}

void mv64361_device::update_interrupts()
{
	if (BIT(m_regs[REG_CUNIT_ARBITER_CONTROL], 10))
	{
		// level sensitive
		m_cpuint0_cb(m_regs[REG_GPP_VALUE] & m_regs[REG_GPP_INTERRUPT_MASK0] ? ASSERT_LINE : CLEAR_LINE);
		m_cpuint1_cb(m_regs[REG_GPP_VALUE] & m_regs[REG_GPP_INTERRUPT_MASK1] ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		// edge triggered
		m_cpuint0_cb(m_regs[REG_GPP_INTERRUPT_CAUSE] & m_regs[REG_GPP_INTERRUPT_MASK0] ? ASSERT_LINE : CLEAR_LINE);
		m_cpuint1_cb(m_regs[REG_GPP_INTERRUPT_CAUSE] & m_regs[REG_GPP_INTERRUPT_MASK1] ? ASSERT_LINE : CLEAR_LINE);
	}
}


//**************************************************************************
//  COMMUNICATIONS UNIT INTERFACE
//**************************************************************************

uint32_t mv64361_device::cunit_arbiter_control_r(offs_t offset, uint32_t mem_mask)
{
	return swapendian_int32(m_regs[REG_CUNIT_ARBITER_CONTROL]);
}

void mv64361_device::cunit_arbiter_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = swapendian_int32(data) & ~0xee00fbff;
	data |= 0x11ff0000; // always set
	mem_mask = swapendian_int32(mem_mask);
	LOGMASKED(LOG_REGS, "cunit_arbiter_control_w: %08x & %08x\n", data, mem_mask);
	COMBINE_DATA(&m_regs[REG_CUNIT_ARBITER_CONTROL]);

	update_interrupts();
}

void mv64361_device::map_window(int window)
{
	if (BIT(m_regs[REG_BASE_ADDRESS_ENABLE], window))
		return;

	switch (window)
	{
		case 0: // CS0
			{
				uint64_t const length = (uint64_t(m_regs[REG_CS0_SIZE]) + 1) << 16;
				m_ranges[0].start = uint64_t(m_regs[REG_CS0_BASE]) << 16;
				m_ranges[0].end = m_ranges[0].start + length - 1;
				if (m_cs_share[0] && length <= m_cs_share[0]->bytes())
				{
					m_cpu_space->install_ram(m_ranges[0].start, m_ranges[0].end, m_cs_share[0]->ptr());
					m_ranges[0].enabled = true;
				}
				LOGMASKED(LOG_MAPPING, "MAP %08x-%08x CS0\n", m_ranges[0].start, m_ranges[0].end);
			}
			break;

		case 1: // CS1
			{
				uint64_t const length = (uint64_t(m_regs[REG_CS1_SIZE]) + 1) << 16;
				m_ranges[1].start = uint64_t(m_regs[REG_CS1_BASE]) << 16;
				m_ranges[1].end = m_ranges[1].start + length - 1;
				if (m_cs_share[1] && length <= m_cs_share[1]->bytes())
				{
					m_cpu_space->install_ram(m_ranges[1].start, m_ranges[1].end, m_cs_share[1]->ptr());
					m_ranges[1].enabled = true;
				}
				LOGMASKED(LOG_MAPPING, "MAP %08x-%08x CS1\n", m_ranges[1].start, m_ranges[1].end);
			}
			break;

		case 2: // CS2
			{
				uint64_t const length = (uint64_t(m_regs[REG_CS2_SIZE]) + 1) << 16;
				m_ranges[2].start = uint64_t(m_regs[REG_CS2_BASE]) << 16;
				m_ranges[2].end = m_ranges[2].start + length - 1;
				if (m_cs_share[2] && length <= m_cs_share[2]->bytes())
				{
					m_cpu_space->install_ram(m_ranges[2].start, m_ranges[2].end, m_cs_share[2]->ptr());
					m_ranges[2].enabled = true;
				}
				LOGMASKED(LOG_MAPPING, "MAP %08x-%08x CS2\n", m_ranges[2].start, m_ranges[2].end);
			}
			break;

		case 3: // CS3
			{
				uint64_t const length = (uint64_t(m_regs[REG_CS3_SIZE]) + 1) << 16;
				m_ranges[3].start = uint64_t(m_regs[REG_CS3_BASE]) << 16;
				m_ranges[3].end = m_ranges[3].start + length - 1;
				if (m_cs_share[3] && length <= m_cs_share[3]->bytes())
				{
					m_cpu_space->install_ram(m_ranges[3].start, m_ranges[3].end, m_cs_share[3]->ptr());
					m_ranges[3].enabled = true;
				}
				LOGMASKED(LOG_MAPPING, "MAP %08x-%08x CS3\n", m_ranges[3].start, m_ranges[3].end);
			}
			break;

		case 4: // DEVCS0
			LOGMASKED(LOG_MAPPING, "Mapping DEVCS0 (not supported)\n");
			break;

		case 5: // DEVCS1
			LOGMASKED(LOG_MAPPING, "Mapping DEVCS1 (not supported)\n");
			break;

		case 6: // DEVCS2
			LOGMASKED(LOG_MAPPING, "Mapping DEVCS2 (not supported)\n");
			break;

		case 7: // DEVCS3
			LOGMASKED(LOG_MAPPING, "Mapping DEVCS3 (not supported)\n");
			break;

		case 8: // BOOTCS
			m_ranges[8].start = m_regs[REG_BOOTCS_BASE] << 16;
			m_ranges[8].end = (m_regs[REG_BOOTCS_BASE] << 16) + ((m_regs[REG_BOOTCS_SIZE] << 16) | 0xffff);
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x BOOTCS (not supported)\n", m_ranges[8].start, m_ranges[8].end);
			break;

		case 9: // PCI0 I/O
			m_ranges[9].start = m_regs[REG_PCI0_IO_BASE] << 16;
			m_ranges[9].end = (m_regs[REG_PCI0_IO_BASE] << 16) + ((m_regs[REG_PCI0_IO_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[9].start, m_ranges[9].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_io_r)));
			m_cpu_space->install_write_handler(m_ranges[9].start, m_ranges[9].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_io_w)));
			m_ranges[9].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI0 I/O (remap: %04x)\n", m_ranges[9].start, m_ranges[9].end, m_regs[REG_PCI0_IO_REMAP]);
			break;

		case 10: // PCI0 MEM0
			m_ranges[10].start = m_regs[REG_PCI0_MEM0_BASE] << 16;
			m_ranges[10].end = (m_regs[REG_PCI0_MEM0_BASE] << 16) + ((m_regs[REG_PCI0_MEM0_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[10].start, m_ranges[10].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem0_r)));
			m_cpu_space->install_write_handler(m_ranges[10].start, m_ranges[10].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem0_w)));
			m_ranges[10].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI0 MEM0 (remap: %08x %04x)\n", m_ranges[10].start, m_ranges[10].end, m_regs[REG_PCI0_MEM0_REMAP_HIGH], m_regs[REG_PCI0_MEM0_REMAP_LOW]);
			break;

		case 11: // PCI0 MEM1
			m_ranges[11].start = m_regs[REG_PCI0_MEM1_BASE] << 16;
			m_ranges[11].end = (m_regs[REG_PCI0_MEM1_BASE] << 16) + ((m_regs[REG_PCI0_MEM1_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[11].start, m_ranges[11].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem1_r)));
			m_cpu_space->install_write_handler(m_ranges[11].start, m_ranges[11].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem1_w)));
			m_ranges[11].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI0 MEM1 (remap: %08x %04x)\n", m_ranges[11].start, m_ranges[11].end, m_regs[REG_PCI0_MEM1_REMAP_HIGH], m_regs[REG_PCI0_MEM1_REMAP_LOW]);
			break;

		case 12: // PCI0 MEM2
			m_ranges[12].start = m_regs[REG_PCI0_MEM2_BASE] << 16;
			m_ranges[12].end = (m_regs[REG_PCI0_MEM2_BASE] << 16) + ((m_regs[REG_PCI0_MEM2_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[12].start, m_ranges[12].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem2_r)));
			m_cpu_space->install_write_handler(m_ranges[12].start, m_ranges[12].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem2_w)));
			m_ranges[12].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI0 MEM2 (remap: %08x %04x)\n", m_ranges[12].start, m_ranges[12].end, m_regs[REG_PCI0_MEM2_REMAP_HIGH], m_regs[REG_PCI0_MEM2_REMAP_LOW]);
			break;

		case 13: // PCI0 MEM3
			m_ranges[13].start = m_regs[REG_PCI0_MEM3_BASE] << 16;
			m_ranges[13].end = (m_regs[REG_PCI0_MEM3_BASE] << 16) + ((m_regs[REG_PCI0_MEM3_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[13].start, m_ranges[13].end, read32s_delegate(*this, FUNC(mv64361_device::pci0_mem3_r)));
			m_cpu_space->install_write_handler(m_ranges[13].start, m_ranges[13].end, write32s_delegate(*this, FUNC(mv64361_device::pci0_mem3_w)));
			m_ranges[13].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI0 MEM3 (remap: %08x %04x)\n", m_ranges[13].start, m_ranges[13].end, m_regs[REG_PCI0_MEM3_REMAP_HIGH], m_regs[REG_PCI0_MEM3_REMAP_LOW]);
			break;

		case 14: // PCI1 I/O
			m_ranges[14].start = m_regs[REG_PCI1_IO_BASE] << 16;
			m_ranges[14].end = (m_regs[REG_PCI1_IO_BASE] << 16) + ((m_regs[REG_PCI1_IO_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[14].start, m_ranges[14].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_io_r)));
			m_cpu_space->install_write_handler(m_ranges[14].start, m_ranges[14].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_io_w)));
			m_ranges[14].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI1 I/O (remap: %04x)\n", m_ranges[14].start, m_ranges[14].end, m_regs[REG_PCI1_IO_REMAP]);
			break;

		case 15: // PCI1 MEM0
			m_ranges[15].start = m_regs[REG_PCI1_MEM0_BASE] << 16;
			m_ranges[15].end = (m_regs[REG_PCI1_MEM0_BASE] << 16) + ((m_regs[REG_PCI1_MEM0_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[15].start, m_ranges[15].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem0_r)));
			m_cpu_space->install_write_handler(m_ranges[15].start, m_ranges[15].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem0_w)));
			m_ranges[15].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI1 MEM0 (remap: %08x %04x)\n", m_ranges[15].start, m_ranges[15].end, m_regs[REG_PCI1_MEM0_REMAP_HIGH], m_regs[REG_PCI1_MEM0_REMAP_LOW]);
			break;

		case 16: // PCI1 MEM1
			m_ranges[16].start = m_regs[REG_PCI1_MEM1_BASE] << 16;
			m_ranges[16].end = (m_regs[REG_PCI1_MEM1_BASE] << 16) + ((m_regs[REG_PCI1_MEM1_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[16].start, m_ranges[16].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem1_r)));
			m_cpu_space->install_write_handler(m_ranges[16].start, m_ranges[16].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem1_w)));
			m_ranges[16].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI1 MEM1 (remap: %08x %04x)\n", m_ranges[16].start, m_ranges[16].end, m_regs[REG_PCI1_MEM1_REMAP_HIGH], m_regs[REG_PCI1_MEM1_REMAP_LOW]);
			break;

		case 17: // PCI1 MEM2
			m_ranges[17].start = m_regs[REG_PCI1_MEM2_BASE] << 16;
			m_ranges[17].end = (m_regs[REG_PCI1_MEM2_BASE] << 16) + ((m_regs[REG_PCI1_MEM2_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[17].start, m_ranges[17].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem2_r)));
			m_cpu_space->install_write_handler(m_ranges[17].start, m_ranges[17].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem2_w)));
			m_ranges[17].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI1 MEM2 (remap: %08x %04x)\n", m_ranges[17].start, m_ranges[17].end, m_regs[REG_PCI1_MEM2_REMAP_HIGH], m_regs[REG_PCI1_MEM2_REMAP_LOW]);
			break;

		case 18: // PCI1 MEM3
			m_ranges[18].start = m_regs[REG_PCI1_MEM3_BASE] << 16;
			m_ranges[18].end = (m_regs[REG_PCI1_MEM3_BASE] << 16) + ((m_regs[REG_PCI1_MEM3_SIZE] << 16) | 0xffff);
			m_cpu_space->install_read_handler (m_ranges[18].start, m_ranges[18].end, read32s_delegate(*this, FUNC(mv64361_device::pci1_mem3_r)));
			m_cpu_space->install_write_handler(m_ranges[18].start, m_ranges[18].end, write32s_delegate(*this, FUNC(mv64361_device::pci1_mem3_w)));
			m_ranges[18].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x PCI1 MEM3 (remap: %08x %04x)\n", m_ranges[18].start, m_ranges[18].end, m_regs[REG_PCI1_MEM3_REMAP_HIGH], m_regs[REG_PCI1_MEM3_REMAP_LOW]);
			break;

		case 19: // SRAM
			m_ranges[19].start = m_regs[REG_SRAM_BASE] << 16;
			m_ranges[19].end = (m_regs[REG_SRAM_BASE] << 16) + 0x3ffff;
			m_cpu_space->install_ram(m_ranges[19].start, m_ranges[19].end, m_sram.get());
			m_ranges[19].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x SRAM\n", m_ranges[19].start, m_ranges[19].end);
			break;

		case 20: // INTERNAL
			m_ranges[20].start = m_regs[REG_INTERNAL_BASE] << 16;
			m_ranges[20].end = (m_regs[REG_INTERNAL_BASE] << 16) + 0xffff;
			m_cpu_space->install_device(m_ranges[20].start, m_ranges[20].end, *static_cast<mv64361_device *>(this), &mv64361_device::cpu_register_map);
			m_ranges[20].enabled = true;
			LOGMASKED(LOG_MAPPING, "MAP %08x-%08x INTERNAL\n", m_ranges[20].start, m_ranges[20].end);
			break;
	}
}

void mv64361_device::unmap_window(int window)
{
	if (m_ranges[window].enabled)
	{
		m_cpu_space->unmap_readwrite(m_ranges[window].start, m_ranges[window].end);
		m_ranges[window].enabled = false;
	}
}

void mv64361_device::update_window(int window)
{
	unmap_window(window);
	map_window(window);
}

void mv64361_device::update_windows()
{
	// undo current mappings
	for (int i = 0; i < std::size(m_ranges); i++)
		unmap_window(i);

	// apply new mappings
	for (int i = 0; i < std::size(m_ranges); i++)
		map_window(i);

	// refresh PCI BARs
	m_pci_functions[0][0]->set_bar_size(BAR_CS0, m_regs[REG_PCI0_CS0_BAR_SIZE]);
	m_pci_functions[0][0]->set_bar_size(BAR_CS1, m_regs[REG_PCI0_CS1_BAR_SIZE]);
	m_pci_functions[0][1]->set_bar_size(BAR_CS2, m_regs[REG_PCI0_CS2_BAR_SIZE]);
	m_pci_functions[0][1]->set_bar_size(BAR_CS3, m_regs[REG_PCI0_CS3_BAR_SIZE]);
	m_pci_functions[1][0]->set_bar_size(BAR_CS0, m_regs[REG_PCI1_CS0_BAR_SIZE]);
	m_pci_functions[1][0]->set_bar_size(BAR_CS1, m_regs[REG_PCI1_CS1_BAR_SIZE]);
	m_pci_functions[1][1]->set_bar_size(BAR_CS2, m_regs[REG_PCI1_CS2_BAR_SIZE]);
	m_pci_functions[1][1]->set_bar_size(BAR_CS3, m_regs[REG_PCI1_CS3_BAR_SIZE]);

	for (unsigned func = 0; func < 2; func++)
	{
		m_pci_functions[0][func]->set_bar_enable(m_regs[REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE]);
		m_pci_functions[1][func]->set_bar_enable(m_regs[REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE]);
	}
}

offs_t mv64361_device::remap_offset(offs_t offset, uint32_t size, uint32_t remap, unsigned shift)
{
	uint32_t const size_mask = (uint32_t(size) << shift) | util::make_bitmask<uint16_t>(shift);
	return (offset & size_mask) | ((uint32_t(remap) << shift) & ~size_mask);
}

offs_t mv64361_device::cpu_remap_offset(offs_t offset, uint32_t size, uint32_t remap)
{
	// CPU windows are 64 kb / 4 = 1 << 14
	return remap_offset(offset, size, remap, 14);
}

offs_t mv64361_device::pci_remap_offset(offs_t offset, uint32_t size, uint32_t remap)
{
	// PCI windows are 4 kb / 4 = 1 << 10
	return remap_offset(offset, size, remap, 10);
}

void mv64361_device::pci_io_delay(uint8_t pci_bus, uint32_t pci_cycles)
{
	uint32_t const pci_clock = m_pci_bus[pci_bus]->clock();
	if (pci_clock && !machine().side_effects_disabled())
	{
		// basic PCI latency emulation: address, turnaround (for reads) and data
		// the pegasos2 serial console breaks when there are no delays (and the CPU frequency is high enough)
		uint64_t const cpu_cycles = (uint64_t(m_cpu->clock()) * pci_cycles + pci_clock - 1) / pci_clock;
		m_cpu->adjust_icount(-int(cpu_cycles));
	}
}

offs_t mv64361_device::pci_cs_offset(uint8_t pci_bus, uint8_t cs, offs_t offset)
{
	unsigned const reg = cs * 2 + pci_bus;
	uint32_t const size = m_regs[REG_PCI0_CS0_BAR_SIZE + reg] >> 12;
	uint32_t const remap = m_regs[REG_PCI0_CS0_BASE_ADDRESS_REMAP + reg] >> 12;
	offset = pci_remap_offset(offset, size, remap);
	offset = DWORD_XOR_BE(offset << 2) >> 2;
	offset &= (m_cs_share[cs]->bytes() >> 2) - 1;

	return offset;
}

uint32_t mv64361_device::pci_cs_r(uint8_t pci_bus, uint8_t cs, offs_t offset, uint32_t mem_mask)
{
	if (!m_cs_share[cs])
		return 0xffffffff;

	uint32_t const *ram = static_cast<uint32_t *>(m_cs_share[cs]->ptr());

	return swapendian_int32(ram[pci_cs_offset(pci_bus, cs, offset)]);
}

void mv64361_device::pci_cs_w(uint8_t pci_bus, uint8_t cs, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!m_cs_share[cs])
		return;

	uint32_t *ram = static_cast<uint32_t *>(m_cs_share[cs]->ptr());

	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);
	COMBINE_DATA(&ram[pci_cs_offset(pci_bus, cs, offset)]);
}

void mv64361_device::function_command_w(uint8_t pci_bus, uint8_t pci_function, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (unsigned i = 0; i < 2; i++)
	{
		if (pci_function != i)
			m_pci_functions[pci_bus][i]->function_command_w(offset, data, mem_mask);
	}
}

bool mv64361_device::remap_write_disabled(uint8_t pci_bus) const
{
	return BIT(m_regs[REG_PCI0_PCI_ADDRESS_DECODE_CONTROL + pci_bus], 0);
}

bool mv64361_device::pci_loopback_enabled(uint8_t pci_bus) const
{
	return BIT(m_regs[pci_bus ? REG_PCI1_COMMAND : REG_PCI0_COMMAND], 23);
}

uint32_t mv64361_device::pci0_io_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t const data = downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->io_r(
		cpu_remap_offset(offset, m_regs[REG_PCI0_IO_SIZE], m_regs[REG_PCI0_IO_REMAP]), mem_mask);

	pci_io_delay(0, 3);

	return data;
}

void mv64361_device::pci0_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->io_w(
		cpu_remap_offset(offset, m_regs[REG_PCI0_IO_SIZE], m_regs[REG_PCI0_IO_REMAP]), data, mem_mask);

	pci_io_delay(0, 2);
}

uint32_t mv64361_device::pci0_mem0_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM0_SIZE], m_regs[REG_PCI0_MEM0_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM0_SIZE], m_regs[REG_PCI0_MEM0_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem1_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM1_SIZE], m_regs[REG_PCI0_MEM1_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM1_SIZE], m_regs[REG_PCI0_MEM1_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem2_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM2_SIZE], m_regs[REG_PCI0_MEM2_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM2_SIZE], m_regs[REG_PCI0_MEM2_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci0_mem3_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM3_SIZE], m_regs[REG_PCI0_MEM3_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci0_mem3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[0][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI0_MEM3_SIZE], m_regs[REG_PCI0_MEM3_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_io_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t const data = downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->io_r(
		cpu_remap_offset(offset, m_regs[REG_PCI1_IO_SIZE], m_regs[REG_PCI1_IO_REMAP]), mem_mask);

	pci_io_delay(1, 3);

	return data;
}

void mv64361_device::pci1_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->io_w(
		cpu_remap_offset(offset, m_regs[REG_PCI1_IO_SIZE], m_regs[REG_PCI1_IO_REMAP]), data, mem_mask);

	pci_io_delay(1, 2);
}

uint32_t mv64361_device::pci1_mem0_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM0_SIZE], m_regs[REG_PCI1_MEM0_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM0_SIZE], m_regs[REG_PCI1_MEM0_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem1_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM1_SIZE], m_regs[REG_PCI1_MEM1_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM1_SIZE], m_regs[REG_PCI1_MEM1_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem2_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM2_SIZE], m_regs[REG_PCI1_MEM2_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM2_SIZE], m_regs[REG_PCI1_MEM2_REMAP_LOW]), data, mem_mask);
}

uint32_t mv64361_device::pci1_mem3_r(offs_t offset, uint32_t mem_mask)
{
	return downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_r(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM3_SIZE], m_regs[REG_PCI1_MEM3_REMAP_LOW]), mem_mask);
}

void mv64361_device::pci1_mem3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	downcast<mv64361_pci_host_device *>(m_pci_functions[1][0].target())->mem_w(
		cpu_remap_offset(offset, m_regs[REG_PCI1_MEM3_SIZE], m_regs[REG_PCI1_MEM3_REMAP_LOW]), data, mem_mask);
}

template <int Bus>
uint32_t mv64361_device::config_address_r(offs_t offset, uint32_t mem_mask)
{
	if (auto *host = dynamic_cast<mv64361_pci_host_device *>(m_pci_functions[Bus][0].target()))
		return host->config_address_r(offset, mem_mask);

	return 0xffffffff;
}

template <int Bus>
void mv64361_device::config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (auto *host = dynamic_cast<mv64361_pci_host_device *>(m_pci_functions[Bus][0].target()))
		host->config_address_w(offset, data, mem_mask);
}

template <int Bus>
uint32_t mv64361_device::config_data_r(offs_t offset, uint32_t mem_mask)
{
	if (auto *host = dynamic_cast<mv64361_pci_host_device *>(m_pci_functions[Bus][0].target()))
		return host->config_data_r(offset, mem_mask);

	return 0xffffffff;
}

template <int Bus>
void mv64361_device::config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (auto *host = dynamic_cast<mv64361_pci_host_device *>(m_pci_functions[Bus][0].target()))
		host->config_data_w(offset, data, mem_mask);
}


//**************************************************************************
//  MV64361 PCI FUNCTION INTERFACE
//**************************************************************************

device_mv64361_pci_function_interface::device_mv64361_pci_function_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "mv64361_pci_function")
{
}

device_mv64361_pci_function_interface::~device_mv64361_pci_function_interface()
{
}

void device_mv64361_pci_function_interface::set_syscon(mv64361_device &syscon, uint8_t pci_bus, uint8_t pci_function)
{
	m_syscon = &syscon;
	m_pci_bus = pci_bus;
	m_pci_function = pci_function;
}


//**************************************************************************
//  MV64361 PCI HOST
//**************************************************************************

mv64361_pci_host_device::mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_host_device(mconfig, MV64361_PCI_HOST, tag, owner, clock),
	device_mv64361_pci_function_interface(mconfig, *this),
	m_mem_config("pci_cpu_space", ENDIANNESS_LITTLE, 32, 32),
	m_io_config("pci_io_space", ENDIANNESS_LITTLE, 32, 32),
	m_busmaster_config("pci_busmaster_space", ENDIANNESS_LITTLE, 32, 32)
{
	set_ids_host(0x11ab6460, 0x03, 0x000000);
	set_multifunction_device(true);
}

void mv64361_pci_host_device::device_start()
{
	if (!m_syscon)
		throw device_missing_dependencies();

	pci_host_device::device_start();

	set_spaces(&space(AS_PCI_CPU_MEM), &space(AS_PCI_IO), &space(AS_PCI_BUSMASTER_MEM));

	// PCI BAR entries
	add_map(8 * 1024 * 1024, M_MEM | M_64A, FUNC(mv64361_pci_host_device::cs0_map));
	add_map(8 * 1024 * 1024, M_MEM | M_64A, FUNC(mv64361_pci_host_device::cs1_map));
	add_map(64 * 1024, M_MEM | M_64A, *m_syscon, FUNC(mv64361_device::pci_register_map));

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;
	status = 0x02a0;

	save_item(NAME(m_cache_line_size));
	save_item(NAME(m_latency_timer));
	save_item(NAME(m_header_type));
	save_item(NAME(m_bist));
}

void mv64361_pci_host_device::device_reset()
{
	pci_host_device::device_reset();

	m_cache_line_size = 0x00;
	m_latency_timer = 0x00;
	m_header_type = 0x80; // multifunction device
	m_bist = 0x80;

	// reset BAR addresses
	set_map_address(0, 0x00000000);
	set_map_address(1, 0x00800000);
	set_map_address(2, 0xf1000000); // DevAD[5] pulled high (else: 14000000)
}

void mv64361_pci_host_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
										 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// base only clears AS_PCI_CPU_MEM
	address_space &pci_busmaster = space(AS_PCI_BUSMASTER_MEM);
	pci_busmaster.unmap_readwrite(memory_window_start, memory_window_end);

	pci_bridge_device::map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
		io_window_start, io_window_end, io_offset, io_space);

	pci_bridge_device::map_device(memory_window_start, memory_window_end, memory_offset, &pci_busmaster,
		io_window_start, io_window_end, io_offset, io_space);
}

void mv64361_pci_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
										uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// only map BARs into the PCI CPU space if loopback is enabled
	if (memory_space->spacenum() == AS_PCI_CPU_MEM && !m_syscon->pci_loopback_enabled(m_pci_bus))
		return;

	map_bars(memory_window_start, memory_window_end, memory_offset, memory_space,
		io_window_start, io_window_end, io_offset, io_space);
}

void mv64361_pci_host_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x04, 0x05).w(FUNC(mv64361_pci_host_device::command_w));
	map(0x0c, 0x0c).rw(FUNC(mv64361_pci_host_device::cache_line_size_r), FUNC(mv64361_pci_host_device::cache_line_size_w));
	map(0x0d, 0x0d).rw(FUNC(mv64361_pci_host_device::latency_timer_r), FUNC(mv64361_pci_host_device::latency_timer_w));
	map(0x0e, 0x0e).rw(FUNC(mv64361_pci_host_device::header_type_r), FUNC(mv64361_pci_host_device::header_type_w));
	map(0x0f, 0x0f).rw(FUNC(mv64361_pci_host_device::bist_r), FUNC(mv64361_pci_host_device::bist_w));
	map(0x10, 0x27).rw(FUNC(mv64361_pci_host_device::address_base_r), FUNC(mv64361_pci_host_device::address_base_w));
}

device_memory_interface::space_config_vector mv64361_pci_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_CPU_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	r.emplace_back(std::make_pair(AS_PCI_BUSMASTER_MEM, &m_busmaster_config));
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
	uint32_t data = space(AS_PCI_CPU_MEM).read_dword(offset * 4, swapendian_int32(mem_mask));

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PCI_MEM, "mem_r[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));

	return swapendian_int32(data);
}

void mv64361_pci_host_device::mem_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_PCI_MEM, "mem_w[%08x]: %08x & %08x\n", offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
	space(AS_PCI_CPU_MEM).write_dword(offset * 4, swapendian_int32(data), swapendian_int32(mem_mask));
}

void mv64361_pci_host_device::function_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	pci_device::command_w(offset, data, mem_mask);
}

void mv64361_pci_host_device::set_bar_size(int bar, uint64_t size)
{
	LOG("BAR %d size %x\n", bar, size);

	switch (bar)
	{
		case BAR_CS0:
		case BAR_CS1:
			const int map_id = (bar == BAR_CS0) ? 0 : 1;
			set_map_size(map_id, (size | 0x00000fff) + 1);
			break;
	}
}

void mv64361_pci_host_device::set_bar_enable(uint16_t enable)
{
	LOG("BAR enable %04x\n", enable);

	set_map_flags(0, M_MEM | M_64A | (BIT(enable, 0) ? M_DISABLED : 0));
	set_map_flags(1, M_MEM | M_64A | (BIT(enable, 1) ? M_DISABLED : 0));
	set_map_flags(2, M_MEM | M_64A | (BIT(enable, 9) ? M_DISABLED : 0));
}

void mv64361_pci_host_device::cs0_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mv64361_pci_host_device::cs_r<0>), FUNC(mv64361_pci_host_device::cs_w<0>));
}

void mv64361_pci_host_device::cs1_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mv64361_pci_host_device::cs_r<1>), FUNC(mv64361_pci_host_device::cs_w<1>));
}

template <int CS>
uint32_t mv64361_pci_host_device::cs_r(offs_t offset, uint32_t mem_mask)
{
	return m_syscon->pci_cs_r(m_pci_bus, CS, offset, mem_mask);
}

template <int CS>
void mv64361_pci_host_device::cs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_syscon->pci_cs_w(m_pci_bus, CS, offset, data, mem_mask);
}

void mv64361_pci_host_device::command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_syscon->function_command_w(m_pci_bus, m_pci_function, offset, data, mem_mask);
	pci_device::command_w(offset, data, mem_mask);
}

uint8_t mv64361_pci_host_device::cache_line_size_r()
{
	return m_cache_line_size;
}

void mv64361_pci_host_device::cache_line_size_w(uint8_t data)
{
	LOGMASKED(LOG_REGS, "cache_line_size_w: %02x\n", data);
	m_cache_line_size = data;
}

uint8_t mv64361_pci_host_device::latency_timer_r()
{
	return m_latency_timer;
}

void mv64361_pci_host_device::latency_timer_w(uint8_t data)
{
	LOGMASKED(LOG_REGS, "latency_timer_w: %02x\n", data);
	m_latency_timer = data;
}

uint8_t mv64361_pci_host_device::header_type_r()
{
	return m_header_type;
}

void mv64361_pci_host_device::header_type_w(uint8_t data)
{
	LOGMASKED(LOG_REGS, "header_type_w: %02x\n", data);
	m_header_type = data;
}

uint8_t mv64361_pci_host_device::bist_r()
{
	return m_bist;
}

void mv64361_pci_host_device::bist_w(uint8_t data)
{
	data &= ~0x30; // reserved bits
	LOGMASKED(LOG_REGS, "bist_w: %02x\n", data);
	m_bist = data;
}

uint32_t mv64361_pci_host_device::address_base_r(offs_t offset, uint32_t mem_mask)
{
	int bid = bank_reg_infos[offset].bank;
	if (bid == -1 || bank_infos[bid].flags & M_DISABLED)
		return 0;

	return pci_device::address_base_r(offset);
}

void mv64361_pci_host_device::address_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// ignore writes for disabled BARs
	int bid = bank_reg_infos[offset].bank;
	if (bid == -1 || bank_infos[bid].flags & M_DISABLED)
		return;

	pci_device::address_base_w(offset, data, mem_mask);

	if (m_syscon->remap_write_disabled(m_pci_bus))
		return;

	// update remap register when the base address is written
	switch (offset)
	{
		case 0:
			if (m_pci_bus == 0)
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI0_CS0_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI0_CS0_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			else
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI1_CS0_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI1_CS0_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			break;
		case 1:
			break;
		case 2:
			if (m_pci_bus == 0)
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI0_CS1_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI0_CS1_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			else
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI1_CS1_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI1_CS1_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			break;
		case 3:
			break;
		case 4:
			if (m_pci_bus == 0)
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_LOW]);
				m_syscon->m_regs[mv64361_device::REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_LOW] &= ~0x0000ffff;
			}
			else
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_LOW]);
				m_syscon->m_regs[mv64361_device::REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_LOW] &= ~0x0000ffff;
			}
			break;
		case 5:
			if (m_pci_bus == 0)
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_HIGH]);
			else
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_HIGH]);
			break;
	}
}


//**************************************************************************
//  MV64361 PCI FUNCTION 1
//**************************************************************************

mv64361_pci_function1_device::mv64361_pci_function1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, MV64361_PCI_FUNCTION1, tag, owner, clock),
	device_mv64361_pci_function_interface(mconfig, *this)
{
	set_ids_host(0x11ab6460, 0x03, 0x000000);
}

void mv64361_pci_function1_device::device_start()
{
	if (!m_syscon)
		throw device_missing_dependencies();

	pci_device::device_start();

	// PCI BAR entries
	add_map(8 * 1024 * 1024, M_MEM | M_64A, FUNC(mv64361_pci_function1_device::cs2_map));
	add_map(8 * 1024 * 1024, M_MEM | M_64A, FUNC(mv64361_pci_function1_device::cs3_map));
	// TODO: SRAM map
}

void mv64361_pci_function1_device::device_reset()
{
	pci_device::device_reset();

	// reset BAR addresses
	set_map_address(0, 0x01000000);
	set_map_address(1, 0x01800000);
	// TODO: SRAM address
}

void mv64361_pci_function1_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
											  uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// only map BARs into the PCI CPU space if loopback is enabled
	if (memory_space->spacenum() == mv64361_pci_host_device::AS_PCI_CPU_MEM && !m_syscon->pci_loopback_enabled(m_pci_bus))
		return;

	map_bars(memory_window_start, memory_window_end, memory_offset, memory_space,
		io_window_start, io_window_end, io_offset, io_space);
}

void mv64361_pci_function1_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x04, 0x05).w(FUNC(mv64361_pci_function1_device::command_w));
	map(0x10, 0x27).rw(FUNC(mv64361_pci_function1_device::address_base_r), FUNC(mv64361_pci_function1_device::address_base_w));
}

void mv64361_pci_function1_device::function_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	pci_device::command_w(offset, data, mem_mask);
}

void mv64361_pci_function1_device::set_bar_size(int bar, uint64_t size)
{
	LOG("BAR %d size %x\n", bar, size);

	switch (bar)
	{
		case BAR_CS2:
		case BAR_CS3:
			const int map_id = (bar == BAR_CS2) ? 0 : 1;
			set_map_size(map_id, (size | 0x00000fff) + 1);
			break;
		// SRAM BAR not handled
	}
}

void mv64361_pci_function1_device::set_bar_enable(uint16_t enable)
{
	LOG("BAR enable %04x\n", enable);

	set_map_flags(0, M_MEM | M_64A | (BIT(enable, 2) ? M_DISABLED : 0));
	set_map_flags(1, M_MEM | M_64A | (BIT(enable, 3) ? M_DISABLED : 0));

	// SRAM map not handled
}

void mv64361_pci_function1_device::cs2_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mv64361_pci_function1_device::cs_r<2>), FUNC(mv64361_pci_function1_device::cs_w<2>));
}

void mv64361_pci_function1_device::cs3_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mv64361_pci_function1_device::cs_r<3>), FUNC(mv64361_pci_function1_device::cs_w<3>));
}

template <int CS>
uint32_t mv64361_pci_function1_device::cs_r(offs_t offset, uint32_t mem_mask)
{
	return m_syscon->pci_cs_r(m_pci_bus, CS, offset, mem_mask);
}

template <int CS>
void mv64361_pci_function1_device::cs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_syscon->pci_cs_w(m_pci_bus, CS, offset, data, mem_mask);
}

void mv64361_pci_function1_device::command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_syscon->function_command_w(m_pci_bus, m_pci_function, offset, data, mem_mask);
	pci_device::command_w(offset, data, mem_mask);
}

uint32_t mv64361_pci_function1_device::address_base_r(offs_t offset, uint32_t mem_mask)
{
	// disabled BARs return 0
	int bid = bank_reg_infos[offset].bank;
	if (bid == -1 || bank_infos[bid].flags & M_DISABLED)
		return 0;

	return pci_device::address_base_r(offset);
}

void mv64361_pci_function1_device::address_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// ignore writes for disabled BARs
	int bid = bank_reg_infos[offset].bank;
	if (bid == -1 || bank_infos[bid].flags & M_DISABLED)
		return;

	pci_device::address_base_w(offset, data, mem_mask);

	if (m_syscon->remap_write_disabled(m_pci_bus))
		return;

	// update remap register when the base address is written
	switch (offset)
	{
		case 0:
			if (m_pci_bus == 0)
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI0_CS2_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI0_CS2_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			else
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI1_CS2_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI1_CS2_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			break;
		case 1:
			break;
		case 2:
			if (m_pci_bus == 0)
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI0_CS3_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI0_CS3_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			else
			{
				COMBINE_DATA(&m_syscon->m_regs[mv64361_device::REG_PCI1_CS3_BASE_ADDRESS_REMAP]);
				m_syscon->m_regs[mv64361_device::REG_PCI1_CS3_BASE_ADDRESS_REMAP] &= ~0x00000fff;
			}
			break;
		case 3:
			break;
		case 4:
		case 5:
			// TODO: SRAM
			break;
	}
}
