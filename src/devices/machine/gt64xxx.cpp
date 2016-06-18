// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Ted Green
#include "gt64xxx.h"

/*************************************
 *
 *  Debugging constants
 *
 *************************************/
#define LOG_GALILEO         (0)
#define LOG_TIMERS          (0)
#define LOG_DMA             (0)
#define LOG_PCI             (0)

const device_type GT64XXX      = &device_creator<gt64xxx_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, gt64xxx_device)
	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END

// cpu i/f map
DEVICE_ADDRESS_MAP_START(cpu_map, 32, gt64xxx_device)
	AM_RANGE(0x00000000, 0x00000cff) AM_READWRITE(    cpu_if_r,          cpu_if_w)
ADDRESS_MAP_END

gt64xxx_device::gt64xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_host_device(mconfig, GT64XXX, "Galileo GT-64XXX System Controller", tag, owner, clock, "gt64xxx", __FILE__),
		m_be(0), m_autoconfig(0), m_irq_num(-1),
		m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32),
		m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32),
		m_romRegion(*this, "rom"),
		m_updateRegion(*this, "update")
{
}

void gt64xxx_device::set_cs_map(int id, address_map_constructor map, const char *name, device_t *device)
{
	m_cs_map[id].enable = true;
	m_cs_map[id].name = name;
	m_cs_map[id].device = device;
	m_cs_map[id].map = map;
}

const address_space_config *gt64xxx_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? pci_bridge_device::memory_space_config(spacenum) : (spacenum == AS_DATA) ? &m_mem_config : (spacenum == AS_IO) ? &m_io_config : nullptr;
}

void gt64xxx_device::device_start()
{
	pci_host_device::device_start();
	m_cpu = machine().device<mips3_device>(cpu_tag);
	m_cpu_space = &m_cpu->space(AS_PROGRAM);
	memory_space = &space(AS_DATA);
	io_space = &space(AS_IO);

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffffffff;
	io_offset       = 0x00000000;
	status = 0x0;

	// DMA timer
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt64xxx_device::perform_dma), this));
	// Leave the timer disabled.
	m_dma_timer->adjust(attotime::never, 0, DMA_TIMER_PERIOD);

	// ROM
	UINT32 romSize = m_romRegion->bytes();
	m_cpu_space->install_rom   (0x1fc00000, 0x1fc00000 + romSize - 1, m_romRegion->base());
	// ROM MIPS DRC
	m_cpu->add_fastram(0x1fc00000, 0x1fc00000 + romSize - 1, TRUE, m_romRegion->base());
	if (LOG_GALILEO)
		logerror("%s: gt64xxx_device::device_start ROM Mapped size: 0x%08X start: 0x1fc00000 end: %08X\n", tag(), romSize, 0x1fc00000 + romSize - 1);

	// Update region address is based on seattle driver
	if (m_updateRegion) {
		romSize = m_updateRegion->bytes();
		m_cpu_space->install_rom(0x1fd00000, 0x1fd00000 + romSize - 1, m_updateRegion->base());
		if (LOG_GALILEO)
			logerror("%s: gt64xxx_device::device_start UPDATE Mapped size: 0x%08X start: 0x1fd00000 end: %08X\n", tag(), romSize, 0x1fd00000 + romSize - 1);
	}

	/* allocate timers for the galileo */
	m_timer[0].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt64xxx_device::timer_callback), this));
	m_timer[1].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt64xxx_device::timer_callback), this));
	m_timer[2].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt64xxx_device::timer_callback), this));
	m_timer[3].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt64xxx_device::timer_callback), this));
}

void gt64xxx_device::device_reset()
{
	pci_device::device_reset();

	// Configuration register defaults
	m_reg[GREG_CPU_CONFIG] = m_be ? 0 : (1<<12);
	m_reg[GREG_RAS_1_0_LO] = 0x0;
	m_reg[GREG_RAS_1_0_HI] = 0x7;
	m_reg[GREG_RAS_3_2_LO] = 0x8;
	m_reg[GREG_RAS_3_2_HI] = 0xf;
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

	m_pci_stall_state = 0;
	m_retry_count = 0;
	m_pci_cpu_stalled = 0;
	m_cpu_stalled_offset = 0;
	m_cpu_stalled_data = 0;
	m_cpu_stalled_mem_mask = 0;

	m_dma_active = 0;
	m_dma_timer->adjust(attotime::never);
	m_last_dma = 0;

	m_prev_addr = 0;
}

void gt64xxx_device::map_cpu_space()
{
	UINT32 winStart, winEnd;

	// ROM region starts at 0x1fc00000
	m_cpu_space->unmap_readwrite(0x00000000, 0x1fbfffff);
	m_cpu_space->unmap_readwrite(0x20000000, 0xffffffff);

	// Clear fastram regions in cpu after rom
	m_cpu->clear_fastram(1);

	// CPU Regs
	winStart = m_reg[GREG_INTERNAL_SPACE]<<21;
	winEnd = winStart + sizeof(m_reg) - 1;
	m_cpu_space->install_device(winStart, winEnd, *static_cast<gt64xxx_device *>(this), &gt64xxx_device::cpu_map);
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space cpu_reg start: %08X end: %08X\n", tag(), winStart, winEnd);

	// RAS[0:3]
	for (int ramIndex = 0; ramIndex < 4; ++ramIndex)
	{
		winStart = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex/2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
		winEnd = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
		m_ram[ramIndex].resize((winEnd + 1 - winStart) / 4);
		m_cpu_space->install_ram(winStart, winEnd, m_ram[ramIndex].data());
		//m_cpu->add_fastram(winStart, m_ram[ramIndex].size() * sizeof(m_ram[ramIndex][0]), FALSE, &m_ram[ramIndex][0]);
		//m_cpu->add_fastram(winStart, m_ram[ramIndex].size() * sizeof(UINT32), FALSE, m_ram[ramIndex].data());
		if (LOG_GALILEO)
			logerror("%s: map_cpu_space ras[%i] start: %08X end: %08X\n", tag(), ramIndex, winStart, winEnd);
	}

	// CS[0:3]
	//m_cpu_space->install_device_delegate(0x16000000, 0x17ffffff, machine().root_device(), m_cs_map[3].map);
	typedef void (gt64xxx_device::*tramp_t)(::address_map &, device_t &);
	static const tramp_t trampolines[4] = {
		&gt64xxx_device::map_trampoline<0>,
		&gt64xxx_device::map_trampoline<1>,
		&gt64xxx_device::map_trampoline<2>,
		&gt64xxx_device::map_trampoline<3>
	};
	for (int ramIndex = 0; ramIndex < 4; ++ramIndex)
	{
		if (m_cs_map[ramIndex].enable)
		{
			winStart = (m_reg[GREG_CS_2_0_LO + 0x10 / 4 * (ramIndex / 3)] << 21) | (m_reg[GREG_CS0_LO + 0x8 / 4 * ramIndex] << 20);
			winEnd = (m_reg[GREG_CS_2_0_LO + 0x10 / 4 * (ramIndex / 3)] << 21) | (m_reg[GREG_CS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
			install_cs_map(winStart, winEnd, trampolines[ramIndex], m_cs_map[ramIndex].name);
			if (LOG_GALILEO)
				logerror("%s: map_cpu_space cs[%i] start: %08X end: %08X\n", tag(), ramIndex, winStart, winEnd);
		}
	}


	// PCI IO Window
	winStart = m_reg[GREG_PCI_IO_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_IO_LO]<<21) | (m_reg[GREG_PCI_IO_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::master_io_r), this));
	m_cpu_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::master_io_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space pci_io start: %08X end: %08X\n", tag(), winStart, winEnd);

	// PCI MEM0 Window
	winStart = m_reg[GREG_PCI_MEM0_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_MEM0_LO]<<21) | (m_reg[GREG_PCI_MEM0_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::master_mem0_r), this));
	m_cpu_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::master_mem0_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space pci_mem0 start: %08X end: %08X\n", tag(), winStart, winEnd);

	// PCI MEM1 Window
	winStart = m_reg[GREG_PCI_MEM1_LO]<<21;
	winEnd =   (m_reg[GREG_PCI_MEM1_LO]<<21) | (m_reg[GREG_PCI_MEM1_HI]<<21) | 0x1fffff;
	m_cpu_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::master_mem1_r), this));
	m_cpu_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::master_mem1_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_cpu_space pci_mem1 start: %08X end: %08X\n", tag(), winStart, winEnd);

	// Setup the address mapping table for DMA lookups
	for (size_t index = 0; index < proc_addr_bank::ADDR_NUM; ++index)
	{
		if (index < proc_addr_bank::ADDR_PCI_MEM1) {
			dma_addr_map[index].low_addr = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * index] << 21);
			dma_addr_map[index].high_addr = (dma_addr_map[index].low_addr & 0xf0000000) | (m_reg[GREG_RAS_1_0_HI + 0x10 / 4 * index] << 21) | 0x1fffff;
		}
		else {
			dma_addr_map[index].low_addr = (m_reg[GREG_PCI_MEM1_LO] << 21);
			dma_addr_map[index].high_addr = (dma_addr_map[index].low_addr & 0xf0000000) | (m_reg[GREG_PCI_MEM1_HI] << 21) | 0x1fffff;
		}

	switch (index) {
		case proc_addr_bank::ADDR_PCI_IO:
			dma_addr_map[index].space = &this->space(AS_IO);
			break;
		case proc_addr_bank::ADDR_PCI_MEM0:
		case proc_addr_bank::ADDR_PCI_MEM1:
			dma_addr_map[index].space = &this->space(AS_DATA);
			break;
		default:
			dma_addr_map[index].space = m_cpu_space;
			break;
		}
	}
}

void gt64xxx_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	int ramIndex;
	UINT32 winStart, winEnd, winSize;

	// Not sure if GREG_RAS_1_0_LO should be added on PCI address map side.
	// RAS0
	ramIndex = 0;
	winStart = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::ras_0_r), this));
	memory_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::ras_0_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_extra RAS0 start=%08X end=%08X size=%08X\n", tag(), winStart, winEnd, winSize);

	// RAS1
	ramIndex = 1;
	winStart = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::ras_1_r), this));
	memory_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::ras_1_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_extra RAS1 start=%08X end=%08X size=%08X\n", tag(), winStart, winEnd, winSize);

	// RAS2
	ramIndex = 2;
	winStart = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::ras_2_r), this));
	memory_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::ras_2_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_extra RAS2 start=%08X end=%08X size=%08X\n", tag(), winStart, winEnd, winSize);

	// RAS3
	ramIndex = 3;
	winStart = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_LO + 0x8 / 4 * ramIndex] << 20);
	winEnd = (m_reg[GREG_RAS_1_0_LO + 0x10 / 4 * (ramIndex / 2)] << 21) | (m_reg[GREG_RAS0_HI + 0x8 / 4 * ramIndex] << 20) | 0xfffff;
	winSize = winEnd - winStart + 1;
	memory_space->install_read_handler(winStart, winEnd, read32_delegate(FUNC(gt64xxx_device::ras_3_r), this));
	memory_space->install_write_handler(winStart, winEnd, write32_delegate(FUNC(gt64xxx_device::ras_3_w), this));
	if (LOG_GALILEO)
		logerror("%s: map_extra RAS3 start=%08X end=%08X size=%08X\n", tag(), winStart, winEnd, winSize);
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
			// master_mem0_w -- Should actually be checking for master_mem1_w as well
			this->space(AS_DATA).write_dword((m_reg[GREG_PCI_MEM0_LO] << 21) | (m_cpu_stalled_offset * 4), m_cpu_stalled_data, m_cpu_stalled_mem_mask);
			/* resume CPU execution */
			machine().scheduler().trigger(45678);
			if (LOG_GALILEO)
				logerror("Resuming CPU on PCI Stall offset=0x%08X data=0x%08X\n", m_cpu_stalled_offset * 4, m_cpu_stalled_data);
		}
	}

	/* set the new state */
	m_pci_stall_state = state;
}

// PCI bus control
READ32_MEMBER (gt64xxx_device::pci_config_r)
{
	UINT32 result = 0;
	if (LOG_GALILEO)
		logerror("%06X:galileo pci_config_r from offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::pci_config_w)
{
	if (LOG_GALILEO)
		logerror("%06X:galileo pci_config_w to offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
}
// PCI Master Window 0
READ32_MEMBER (gt64xxx_device::master_mem0_r)
{
	UINT32 result = this->space(AS_DATA).read_dword((m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), mem_mask);
	if (LOG_PCI)
		logerror("%06X:galileo pci mem0 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::master_mem0_w)
{
	if (m_pci_stall_state) {
		// Save the write data and stall the cpu
		m_pci_cpu_stalled = 1;
		m_cpu_stalled_offset = offset;
		m_cpu_stalled_data = data;
		m_cpu_stalled_mem_mask = mem_mask;
		// Stall cpu until trigger
		m_cpu_space->device().execute().spin_until_trigger(45678);
		if (LOG_GALILEO || LOG_PCI)
			logerror("%08X:Stalling CPU on PCI Stall\n", m_cpu_space->device().safe_pc());
		return;
	}
	this->space(AS_DATA).write_dword((m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), data, mem_mask);
	if (LOG_PCI)
		logerror("%06X:galileo pci mem0 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM0_LO]<<21) | (offset*4), data, mem_mask);
}

// PCI Master Window 1
READ32_MEMBER (gt64xxx_device::master_mem1_r)
{
	UINT32 result = this->space(AS_DATA).read_dword((m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), mem_mask);
	if (LOG_PCI)
		logerror("%06X:galileo pci mem1 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), result, mem_mask);
	return result;
}
WRITE32_MEMBER (gt64xxx_device::master_mem1_w)
{
	this->space(AS_DATA).write_dword((m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), data, mem_mask);
	if (LOG_PCI)
		logerror("%06X:galileo pci mem1 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_MEM1_LO]<<21) | (offset*4), data, mem_mask);
}

// PCI Master IO
READ32_MEMBER (gt64xxx_device::master_io_r)
{
	UINT32 result = this->space(AS_IO).read_dword((m_reg[GREG_PCI_IO_LO]<<21) | (offset*4), mem_mask);
	if (LOG_PCI && m_prev_addr != offset) {
		m_prev_addr = offset;
		logerror("%06X:galileo pci io read from offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_IO_LO] << 21) | (offset * 4), result, mem_mask);
	}
	return result;
}
WRITE32_MEMBER (gt64xxx_device::master_io_w)
{
	this->space(AS_IO).write_dword((m_reg[GREG_PCI_IO_LO]<<21) | (offset*4), data, mem_mask);
	if (LOG_PCI && m_prev_addr != offset) {
		m_prev_addr = offset;
		logerror("%06X:galileo pciio write to offset %08X = %08X & %08X\n", space.device().safe_pc(), (m_reg[GREG_PCI_IO_LO] << 21) | (offset * 4), data, mem_mask);
	}
}

READ32_MEMBER(gt64xxx_device::ras_0_r)
{
	UINT32 result = m_ram[0][offset];
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(gt64xxx_device::ras_0_w)
{
	COMBINE_DATA(&m_ram[0][offset]);
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, data, mem_mask);
}

READ32_MEMBER(gt64xxx_device::ras_1_r)
{
	UINT32 result = m_ram[1][offset];
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(gt64xxx_device::ras_1_w)
{
	COMBINE_DATA(&m_ram[1][offset]);
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, data, mem_mask);
}

READ32_MEMBER(gt64xxx_device::ras_2_r)
{
	UINT32 result = m_ram[2][offset];
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(gt64xxx_device::ras_2_w)
{
	COMBINE_DATA(&m_ram[2][offset]);
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, data, mem_mask);
}

READ32_MEMBER(gt64xxx_device::ras_3_r)
{
	UINT32 result = m_ram[3][offset];
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 read from offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(gt64xxx_device::ras_3_w)
{
	COMBINE_DATA(&m_ram[3][offset]);
	if (LOG_PCI)
		logerror("%06X:galileo ras_0 write to offset %08X = %08X & %08X\n", space.device().safe_pc(), offset * 4, data, mem_mask);
}


// CPU I/F
READ32_MEMBER (gt64xxx_device::cpu_if_r)
{
	UINT32 result = m_reg[offset];

	/* switch off the offset for special cases */
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
				UINT32 elapsed = (timer->timer->elapsed() * m_clock).as_double();
				result = (result > elapsed) ? (result - elapsed) : 0;
			}

			/* eat some time for those which poll this register */
			//space.device().execute().eat_cycles(100);

			if (LOG_TIMERS)
				logerror("%08X:hires_timer_r = %08X\n", space.device().safe_pc(), result);
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
			result = config_data_r(space, offset);
			if (LOG_GALILEO)
				logerror("%08X:Galileo GREG_CONFIG_DATA read from offset %03X = %08X\n", space.device().safe_pc(), offset*4, result);
			break;

		case GREG_CONFIG_ADDRESS:
			result = config_address_r(space, offset);
			break;

		case GREG_INT_STATE:
		case GREG_INT_MASK:
		case GREG_TIMER_CONTROL:
//          if (LOG_GALILEO)
//              logerror("%08X:Galileo read from offset %03X = %08X\n", space.device().safe_pc(), offset*4, result);
			break;

		default:
			if (LOG_GALILEO)
				logerror("%08X:Galileo read from offset %03X = %08X\n", space.device().safe_pc(), offset*4, result);
			break;
	}

	if (m_be) result =  FLIPENDIAN_INT32(result);

	return result;
}

WRITE32_MEMBER(gt64xxx_device::cpu_if_w)
{
	if (m_be) {
		data = FLIPENDIAN_INT32(data);
		mem_mask = FLIPENDIAN_INT32(mem_mask);
	}

	UINT32 oldata = m_reg[offset];
	COMBINE_DATA(&m_reg[offset]);

	/* switch off the offset for special cases */
	switch (offset)
	{
		case GREG_RAS_1_0_LO:
		case GREG_RAS_1_0_HI:
		case GREG_RAS_3_2_LO:
		case GREG_RAS_3_2_HI:
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
			if (LOG_GALILEO)
				logerror("%08X:Galileo Memory Map data write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		case GREG_DMA0_CONTROL:
		case GREG_DMA1_CONTROL:
		case GREG_DMA2_CONTROL:
		case GREG_DMA3_CONTROL:
		{
			int which = offset % 4;

			/* keep the read only activity bit */
			m_reg[offset] &= ~0x4000;
			m_reg[offset] |= (oldata & 0x4000);

			/* fetch next record */
			if (data & 0x2000)
				dma_fetch_next(space, which);
			m_reg[offset] &= ~0x2000;

			/* if enabling, start the DMA */
			if (!(oldata & 0x1000) && (data & 0x1000) && !(m_dma_active & (1<<which)))
			{
				// Trigger the timer if there are no dma's active
				if (m_dma_active==0)
					m_dma_timer->adjust(attotime::zero, 0, DMA_TIMER_PERIOD);
				m_dma_active |= (1<< which);
				//perform_dma(space, which);
				if (LOG_DMA)
					logerror("%08X:Galileo starting DMA Chan %i\n", space.device().safe_pc(), which);
			}
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset * 4, data, mem_mask);
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
			if (LOG_TIMERS)
				logerror("%08X:timer/counter %d count = %08X [start=%08X]\n", space.device().safe_pc(), offset % 4, data, timer->count);
			break;
		}

		case GREG_TIMER_CONTROL:
		{
			int which, mask;

			if (LOG_TIMERS)
				logerror("%08X:timer/counter control = %08X\n", space.device().safe_pc(), data);
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
					if (LOG_TIMERS)
						logerror("Adjusted timer to fire in %f secs\n", (TIMER_PERIOD * timer->count).as_double());
				}
				else if (timer->active && !(data & mask))
				{
					UINT32 elapsed = (timer->timer->elapsed() * m_clock).as_double();
					timer->active = 0;
					timer->count = (timer->count > elapsed) ? (timer->count - elapsed) : 0;
					timer->timer->adjust(attotime::never, which);
					if (LOG_TIMERS)
						logerror("Disabled timer\n");
				}
			}
			break;
		}

		case GREG_INT_STATE:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to IRQ clear = %08X & %08X\n", offset*4, data, mem_mask);
			m_reg[offset] = oldata & data;
			update_irqs();
			break;

		case GREG_CONFIG_DATA:
			pci_host_device::config_data_w(space, offset, data);
			if (LOG_GALILEO)
				logerror("%08X:Galileo PCI config data write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		case GREG_CONFIG_ADDRESS:
			// Type 0 config transactions signalled by Bus Num = 0 and Device Num != 0
			// Bits 15:11 get mapped into device number for configuration
			UINT32 modData;
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
			pci_host_device::config_address_w(space, offset, modData);
			if (LOG_GALILEO)
				logerror("%08X:Galileo PCI config address write to offset %03X = %08X & %08X origData = %08X\n", space.device().safe_pc(), offset*4, modData, mem_mask, data);
			break;

		case GREG_DMA0_COUNT:   case GREG_DMA1_COUNT:   case GREG_DMA2_COUNT:   case GREG_DMA3_COUNT:
		case GREG_DMA0_SOURCE:  case GREG_DMA1_SOURCE:  case GREG_DMA2_SOURCE:  case GREG_DMA3_SOURCE:
		case GREG_DMA0_DEST:    case GREG_DMA1_DEST:    case GREG_DMA2_DEST:    case GREG_DMA3_DEST:
		case GREG_DMA0_NEXT:    case GREG_DMA1_NEXT:    case GREG_DMA2_NEXT:    case GREG_DMA3_NEXT:
		case GREG_INT_MASK:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;

		default:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;
	}
}

/*************************************
 *
 *  Galileo timers & interrupts
 *
 *************************************/

void gt64xxx_device::update_irqs()
{
	int state = CLEAR_LINE;

	/* if any unmasked interrupts are live, we generate */
	if (m_reg[GREG_INT_STATE] & m_reg[GREG_INT_MASK])
		state = ASSERT_LINE;
	if (m_irq_num != -1)
		m_cpu->set_input_line(m_irq_num, state);

	if (1 && LOG_GALILEO)
		logerror("Galileo IRQ %s irqNum: %i state = %08X mask = %08X\n", (state == ASSERT_LINE) ? "asserted" : "cleared", m_irq_num, m_reg[GREG_INT_STATE], m_reg[GREG_INT_MASK]);
}


TIMER_CALLBACK_MEMBER(gt64xxx_device::timer_callback)
{
	int which = param;
	galileo_timer *timer = &m_timer[which];

	if (LOG_TIMERS)
		logerror("timer %d fired\n", which);

	/* copy the start value from the registers */
	timer->count = m_reg[GREG_TIMER0_COUNT + which];
	if (which != 0)
		timer->count &= 0xffffff;

	/* if we're a timer, adjust the timer to fire again */
	if (m_reg[GREG_TIMER_CONTROL] & (2 << (2 * which)))
		timer->timer->adjust(TIMER_PERIOD * timer->count, which);
	else
		timer->active = timer->count = 0;

	/* trigger the interrupt */
	m_reg[GREG_INT_STATE] |= 1 << (GINT_T0EXP_SHIFT + which);
	update_irqs();
}

/*************************************
 *
 *  Galileo DMA handler
 *
 *************************************/
address_space* gt64xxx_device::dma_decode_address(UINT32 &addr)
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
	UINT32 data;

	/* no-op for unchained mode */
	if (!(m_reg[GREG_DMA0_CONTROL + which] & 0x200))
		address = m_reg[GREG_DMA0_NEXT + which];

	/* if we hit the end address, signal an interrupt */
	if (address == 0)
	{
		if (m_reg[GREG_DMA0_CONTROL + which] & 0x400)
		{
			m_reg[GREG_INT_STATE] |= 1 << (GINT_DMA0COMP_SHIFT + which);
			update_irqs();
		}
		m_reg[GREG_DMA0_CONTROL + which] &= ~0x5000;
		return 0;
	}

	/* fetch the byte count */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_COUNT + which] = data;

	/* fetch the source address */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_SOURCE + which] = data;

	/* fetch the dest address */
	data = space.read_dword(address); address += 4;
	m_reg[GREG_DMA0_DEST + which] = data;

	/* fetch the next record address */
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
	} else
	{
		offs_t srcaddr = m_reg[GREG_DMA0_SOURCE + which];
		offs_t dstaddr = m_reg[GREG_DMA0_DEST + which];
		UINT32 bytesleft = m_reg[GREG_DMA0_COUNT + which] & 0xffff;
		address_space* srcSpace = dma_decode_address(srcaddr);
		address_space* dstSpace = dma_decode_address(dstaddr);

		int srcinc, dstinc;

		m_reg[GREG_DMA0_CONTROL + which] |= 0x5000;

		/* determine src/dst inc */
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

		if (LOG_DMA)
			logerror("Performing DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc);

		int burstCount = 0;
		/* standard transfer */
		while (bytesleft > 0 && burstCount < DMA_BURST_SIZE)
		{
			if (m_pci_stall_state) {
				if (LOG_DMA && m_retry_count<4)
					logerror("%08X:Stalling DMA on voodoo retry_count: %i\n", m_cpu_space->device().safe_pc(), m_retry_count);
				// Save info
				m_reg[GREG_DMA0_SOURCE + which] = srcaddr;
				m_reg[GREG_DMA0_DEST + which] = dstaddr;
				m_reg[GREG_DMA0_COUNT + which] = (m_reg[GREG_DMA0_COUNT + which] & ~0xffff) | bytesleft;

				m_retry_count++;
				UINT32 configRetryCount = (m_reg[GREG_PCI_TIMEOUT] >> 16) & 0xff;
				if (m_retry_count >= configRetryCount && configRetryCount > 0) {
					logerror("gt64xxx_device::perform_dma Error! Too many PCI retries. DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc);
					// Signal error and abort DMA
					m_dma_active &= ~(1 << which);
					m_retry_count = 0;
					return;
				}
				else {
					// Come back later
					return;
				}
			}
			if (bytesleft < 4)
			{
				dstSpace->write_byte(dstaddr, srcSpace->read_byte(srcaddr));
				srcaddr += srcinc;
				dstaddr += dstinc;
				bytesleft--;
			}
			else {
				//space.write_byte(dstaddr, space.read_byte(srcaddr));
				dstSpace->write_dword(dstaddr, srcSpace->read_dword(srcaddr));
				srcaddr += srcinc * 4;
				dstaddr += dstinc * 4;
				bytesleft -= 4;
			}
			burstCount++;
		}
		/* not verified, but seems logical these should be updated byte the end */
		m_reg[GREG_DMA0_SOURCE + which] = srcaddr;
		m_reg[GREG_DMA0_DEST + which] = dstaddr;
		m_reg[GREG_DMA0_COUNT + which] = (m_reg[GREG_DMA0_COUNT + which] & ~0xffff) | bytesleft;

		/* if we did not hit zero, punt and return later */
		if (bytesleft != 0)
		{
			return;
		}
		/* interrupt? */
		if (!(m_reg[GREG_DMA0_CONTROL + which] & 0x400))
		{
			m_reg[GREG_INT_STATE] |= 1 << (GINT_DMA0COMP_SHIFT + which);
			update_irqs();
		}

		// Fetch the next dma for this channel (to be performed next scheduled burst)
		if (dma_fetch_next(*m_cpu_space, which) == 0)
		{
			m_dma_active &= ~(1 << which);
			// Turn off the timer
			m_dma_timer->adjust(attotime::never);
		}
	}
}
