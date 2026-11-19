// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    VIA VT8231 South Bridge - IDE controller

***************************************************************************/

#include "emu.h"
#include "vt8231_ide.h"

#define LOG_MAPPING (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_MAPPING)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT8231_IDE, vt8231_ide_device, "vt8231_ide", "VT8231 South Bridge - IDE")

vt8231_ide_device::vt8231_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, VT8231_IDE, tag, owner, clock),
	m_bus_master_space(*this, finder_base::DUMMY_TAG, -1, 32),
	m_ide(*this, "ide%u", 0U),
	m_isa(*this, finder_base::DUMMY_TAG),
	m_initialized(false)
{
	set_ids(0x11060571, 0x06, 0x01018a, 0x00000000);
}

void vt8231_ide_device::device_start()
{
	pci_device::device_start();

	// PCI BAR entries
	add_map(8, M_IO | M_DISABLED, FUNC(vt8231_ide_device::pri_command_map));
	add_map(4, M_IO | M_DISABLED, FUNC(vt8231_ide_device::pri_control_map));
	add_map(8, M_IO | M_DISABLED, FUNC(vt8231_ide_device::sec_command_map));
	add_map(4, M_IO | M_DISABLED, FUNC(vt8231_ide_device::sec_control_map));
	add_map(16, M_IO, FUNC(vt8231_ide_device::bus_master_ctrl_regs_map));

	// register for save states
	save_item(NAME(pclass)); // programmable
	save_item(NAME(m_bar));
	save_item(NAME(m_chip_enable));
	save_item(NAME(m_ide_configuration_1));
	save_item(NAME(m_ide_configuration_2));
	save_item(NAME(m_fifo_configuration));
	save_item(NAME(m_misc_control_1));
	save_item(NAME(m_misc_control_2));
	save_item(NAME(m_misc_control_3));
	save_item(NAME(m_drive_timing_control));
	save_item(NAME(m_address_setup_time));
	save_item(NAME(m_ultradma_extended_timing_control));
	save_item(NAME(m_ultradma_fifo_control));
	save_item(NAME(m_sector_size));
	save_item(NAME(m_ide_status));
	save_item(NAME(m_interrupt_control));
	save_item(NAME(m_sg_descriptor_address));
	save_item(NAME(m_initialized));
}

void vt8231_ide_device::device_reset()
{
	pci_device::device_reset();

	status = 0x0290;
	command = 0x0080;
	command_mask = 0x03c7;
	pclass = 0x01018a;
	intr_line = 0x0e; // IRQ 14/15 (for native mode)
	intr_pin = 0x01;

	m_bar[0] = 0x000001f0;
	m_bar[1] = 0x000003f4;
	m_bar[2] = 0x00000170;
	m_bar[3] = 0x00000374;
	m_bar[4] = 0x0000cc00;

	m_chip_enable = 0x08;
	m_ide_configuration_1 = 0x06;
	m_ide_configuration_2 = 0xc0;
	m_fifo_configuration = 0x0a;
	m_misc_control_1 = 0x68;
	m_misc_control_2 = 0x00;
	m_misc_control_3 = 0xc0;
	m_drive_timing_control[0] = 0xa8;
	m_drive_timing_control[1] = 0xa8;
	m_drive_timing_control[2] = 0xa8;
	m_drive_timing_control[3] = 0xa8;
	m_address_setup_time = 0xff;
	m_ultradma_extended_timing_control[0] = 0x07;
	m_ultradma_extended_timing_control[1] = 0x07;
	m_ultradma_extended_timing_control[2] = 0x07;
	m_ultradma_extended_timing_control[3] = 0x07;
	m_ultradma_fifo_control = 0x04;
	m_sector_size[0] = 0x0200;
	m_sector_size[1] = 0x0200;
	m_ide_status[0] = 0x00;
	m_ide_status[1] = 0x00;
	m_interrupt_control[0] = 0x00;
	m_interrupt_control[1] = 0x00;
	m_sg_descriptor_address[0] = 0x00;
	m_sg_descriptor_address[1] = 0x00;

	m_initialized = true;

	// disable BARs (maps 1 and 3 are always disabled)
	set_map_flags(0, M_IO | M_DISABLED);
	set_map_flags(2, M_IO | M_DISABLED);

	update_bar_address();

	remap_cb();
}

void vt8231_ide_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
	m_ide[0]->set_bus_master_space(m_bus_master_space);
	m_ide[0]->irq_handler().set(FUNC(vt8231_ide_device::ide_irq_w<0>));

	BUS_MASTER_IDE_CONTROLLER(config, m_ide[1]).options(ata_devices, "cdrom", nullptr, false);
	m_ide[1]->set_bus_master_space(m_bus_master_space);
	m_ide[1]->irq_handler().set(FUNC(vt8231_ide_device::ide_irq_w<1>));
}

void vt8231_ide_device::device_config_complete()
{
	pci_device::device_config_complete();

	m_isa_tag = basetag();
	m_isa_tag.back() = '0';
	m_isa.set_tag(*owner(), m_isa_tag.c_str());
}

void vt8231_ide_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
								  uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (!BIT(command, 0))
		return;

	// map only byte 2: normal mapping would hide the floppy registers at 3f4/3f5/3f7
	for (int channel = 0; channel < 2; channel++)
	{
		// channel disabled?
		if (!BIT(m_chip_enable, 1 - channel))
			continue;

		const auto &bank = bank_infos[1 + channel * 2];
		if (bank.adr == -1)
			continue;

		const uint64_t address = (bank.adr & ~(uint64_t(bank.size) - 1)) + io_offset + 2;
		if (address < io_window_start || address > io_window_end)
			continue;

		if (channel == 0)
			io_space->install_readwrite_handler(address, address, read8smo_delegate(*this, FUNC(vt8231_ide_device::pri_cs1_r)), write8smo_delegate(*this, FUNC(vt8231_ide_device::pri_cs1_w)));
		else
			io_space->install_readwrite_handler(address, address, read8smo_delegate(*this, FUNC(vt8231_ide_device::sec_cs1_r)), write8smo_delegate(*this, FUNC(vt8231_ide_device::sec_cs1_w)));
	}
}

void vt8231_ide_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x09, 0x09).w(FUNC(vt8231_ide_device::prog_if_w));
	map(0x10, 0x23).rw(FUNC(vt8231_ide_device::bar_r), FUNC(vt8231_ide_device::bar_w));
	map(0x24, 0x33).noprw();
	map(0x3c, 0x3c).rw(FUNC(vt8231_ide_device::interrupt_line_r), FUNC(vt8231_ide_device::interrupt_line_w));
	map(0x3d, 0x3d).lw8(NAME([] (uint8_t data) { })); // interrupt pin is read-only
	map(0x40, 0x40).rw(FUNC(vt8231_ide_device::chip_enable_r), FUNC(vt8231_ide_device::chip_enable_w));
	map(0x41, 0x41).rw(FUNC(vt8231_ide_device::ide_configuration_1_r), FUNC(vt8231_ide_device::ide_configuration_1_w));
	map(0x42, 0x42).rw(FUNC(vt8231_ide_device::ide_configuration_2_r), FUNC(vt8231_ide_device::ide_configuration_2_w));
	map(0x43, 0x43).rw(FUNC(vt8231_ide_device::fifo_configuration_r), FUNC(vt8231_ide_device::fifo_configuration_w));
	map(0x44, 0x44).rw(FUNC(vt8231_ide_device::misc_control_1_r), FUNC(vt8231_ide_device::misc_control_1_w));
	map(0x45, 0x45).rw(FUNC(vt8231_ide_device::misc_control_2_r), FUNC(vt8231_ide_device::misc_control_2_w));
	map(0x46, 0x46).rw(FUNC(vt8231_ide_device::misc_control_3_r), FUNC(vt8231_ide_device::misc_control_3_w));
	map(0x48, 0x4b).rw(FUNC(vt8231_ide_device::drive_timing_control_r), FUNC(vt8231_ide_device::drive_timing_control_w));
	map(0x4c, 0x4c).rw(FUNC(vt8231_ide_device::address_setup_time_r), FUNC(vt8231_ide_device::address_setup_time_w));
	map(0x50, 0x53).rw(FUNC(vt8231_ide_device::ultradma_extended_timing_control_r), FUNC(vt8231_ide_device::ultradma_extended_timing_control_w)).umask32(0xffffffff);
	map(0x54, 0x54).rw(FUNC(vt8231_ide_device::ultradma_fifo_control_r), FUNC(vt8231_ide_device::ultradma_fifo_control_w));
	map(0x60, 0x61).rw(FUNC(vt8231_ide_device::sector_size_r<0>), FUNC(vt8231_ide_device::sector_size_w<0>));
	map(0x68, 0x69).rw(FUNC(vt8231_ide_device::sector_size_r<1>), FUNC(vt8231_ide_device::sector_size_w<1>));
	map(0x70, 0x70).rw(FUNC(vt8231_ide_device::ide_status_r<0>), FUNC(vt8231_ide_device::ide_status_w<0>));
	map(0x71, 0x71).rw(FUNC(vt8231_ide_device::interrupt_control_r<0>), FUNC(vt8231_ide_device::interrupt_control_w<0>));
	map(0x78, 0x78).rw(FUNC(vt8231_ide_device::ide_status_r<1>), FUNC(vt8231_ide_device::ide_status_w<1>));
	map(0x79, 0x79).rw(FUNC(vt8231_ide_device::interrupt_control_r<1>), FUNC(vt8231_ide_device::interrupt_control_w<1>));
	map(0x80, 0x83).rw(FUNC(vt8231_ide_device::sg_descriptor_address_r<0>), FUNC(vt8231_ide_device::sg_descriptor_address_w<0>));
	map(0x88, 0x8b).rw(FUNC(vt8231_ide_device::sg_descriptor_address_r<1>), FUNC(vt8231_ide_device::sg_descriptor_address_w<1>));
	map(0xc0, 0xc3).r(FUNC(vt8231_ide_device::pci_pm_block1_r));
	map(0xc4, 0xc7).r(FUNC(vt8231_ide_device::pci_pm_block2_r));
}

void vt8231_ide_device::prog_if_w(uint8_t data)
{
	data &= ~0x70; // bits fixed at 0
	data |= 0x8a; // bits fixed at 1
	LOG("prog_if_w: %02x (primary: %s mode, secondary: %s mode)\n",	data,
		BIT(data, 0) ? "native" : "compatibility",
		BIT(data, 2) ? "native" : "compatibility");
	pclass = (pclass & ~0xff) | data;
	update_bar_address();
}

uint32_t vt8231_ide_device::bar_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0: return pri_channel_is_native() ? ((m_bar[0] & ~0x07) | 1) : 0x000001f0;
		case 1: return pri_channel_is_native() ? ((m_bar[1] & ~0x03) | 1) : 0x000003f4;
		case 2: return sec_channel_is_native() ? ((m_bar[2] & ~0x07) | 1) : 0x00000170;
		case 3: return sec_channel_is_native() ? ((m_bar[3] & ~0x03) | 1) : 0x00000374;
		case 4: return (m_bar[4] & ~0x0000000f) | 1;
	}

	return 0;
}

void vt8231_ide_device::bar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0: data &= ~0xffff0007; break; // primary channel data/command
		case 1: data &= ~0xffff0003; break; // primary channel control/status
		case 2: data &= ~0xffff0007; break; // secondary channel data/command
		case 3: data &= ~0xffff0003; break; // secondary channel control/status
		case 4:	data &= ~0xffff000f; break; // bus master control
	}

	COMBINE_DATA(&m_bar[offset]);
	LOGMASKED(LOG_MAPPING, "Mapping BAR%d at %08x\n", offset, m_bar[offset]);

	update_bar_address();
}

uint8_t vt8231_ide_device::capptr_r()
{
	return 0xc0;
}

uint8_t vt8231_ide_device::chip_enable_r()
{
	return m_chip_enable;
}

void vt8231_ide_device::chip_enable_w(uint8_t data)
{
	data &= 0x0f;
	LOG("chip_enable_w: %02x (primary: %s, secondary: %s)\n", data,
		BIT(data, 1) ? "enabled" : "disabled",
		BIT(data, 0) ? "enabled" : "disabled");
	m_chip_enable = data;

	// enable/disable channels
	set_map_flags(0, M_IO | (BIT(data, 1) == 0 ? M_DISABLED : 0));
	set_map_flags(2, M_IO | (BIT(data, 0) == 0 ? M_DISABLED : 0));
}

uint8_t vt8231_ide_device::ide_configuration_1_r()
{
	return m_ide_configuration_1;
}

void vt8231_ide_device::ide_configuration_1_w(uint8_t data)
{
	LOG("ide_configuration_1_w: %02x\n", data);
	m_ide_configuration_1 = data;
}

uint8_t vt8231_ide_device::ide_configuration_2_r()
{
	return m_ide_configuration_2;
}

void vt8231_ide_device::ide_configuration_2_w(uint8_t data)
{
	LOG("ide_configuration_2_w: %02x\n", data);
	m_ide_configuration_2 = data;
	update_bar_address();
}

uint8_t vt8231_ide_device::fifo_configuration_r()
{
	return m_fifo_configuration;
}

void vt8231_ide_device::fifo_configuration_w(uint8_t data)
{
	data &= ~0xf0; // reserved bits
	LOG("fifo_configuration_w: %02x\n", data);
	m_fifo_configuration = data;
}

uint8_t vt8231_ide_device::misc_control_1_r()
{
	return m_misc_control_1;
}

void vt8231_ide_device::misc_control_1_w(uint8_t data)
{
	data &= ~0x82; // reserved bits
	LOG("misc_control_1_w: %02x\n", data);
	m_misc_control_1 = data;
}

uint8_t vt8231_ide_device::misc_control_2_r()
{
	return m_misc_control_2;
}

void vt8231_ide_device::misc_control_2_w(uint8_t data)
{
	data &= ~0xb3; // reserved bits
	LOG("misc_control_2_w: %02x\n", data);
	m_misc_control_2 = data;
}

uint8_t vt8231_ide_device::misc_control_3_r()
{
	return m_misc_control_3;
}

void vt8231_ide_device::misc_control_3_w(uint8_t data)
{
	data &= ~0x3f; // reserved bits
	LOG("misc_control_3_w: %02x\n", data);
	m_misc_control_3 = data;
}

uint8_t vt8231_ide_device::drive_timing_control_r(offs_t offset)
{
	return m_drive_timing_control[offset];
}

void vt8231_ide_device::drive_timing_control_w(offs_t offset, uint8_t data)
{
	LOG("drive_timing_control_w[%d]: %02x\n", offset, data);
	m_drive_timing_control[offset] = data;
}

uint8_t vt8231_ide_device::address_setup_time_r()
{
	return m_address_setup_time;
}

void vt8231_ide_device::address_setup_time_w(uint8_t data)
{
	LOG("address_setup_time_w: %02x\n", data);
	m_address_setup_time = data;
}

uint8_t vt8231_ide_device::ultradma_extended_timing_control_r(offs_t offset)
{
	return m_ultradma_extended_timing_control[offset];
}

void vt8231_ide_device::ultradma_extended_timing_control_w(offs_t offset, uint8_t data)
{
	data &= ~0x08; // reserved bits
	LOG("ultradma_extended_timing_control_w[%d]: %02x\n", offset, data);
	m_ultradma_extended_timing_control[offset] = data;
}

uint8_t vt8231_ide_device::ultradma_fifo_control_r()
{
	return m_ultradma_fifo_control;
}

void vt8231_ide_device::ultradma_fifo_control_w(uint8_t data)
{
	data &= ~0xea;
	LOG("ultradma_fifo_control_w: %02x\n", data);
	m_ultradma_fifo_control = data;
}

template <int N>
uint16_t vt8231_ide_device::sector_size_r(offs_t offset, uint16_t mem_mask)
{
	return m_sector_size[N];
}

template <int N>
void vt8231_ide_device::sector_size_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data &= ~0xf000;
	LOG("sector_size_w[%d]: %04x & %04x\n", N, data, mem_mask);
	COMBINE_DATA(&m_sector_size[N]);
}

template <int N>
uint8_t vt8231_ide_device::ide_status_r()
{
	return m_ide_status[N];
}

template <int N>
void vt8231_ide_device::ide_status_w(uint8_t data)
{
	data &= ~0x03;
	LOG("ide_status_w[%d]: %02x\n", N, data);
	m_ide_status[N] = data;
}

template <int N>
uint8_t vt8231_ide_device::interrupt_control_r()
{
	return m_interrupt_control[N];
}

template <int N>
void vt8231_ide_device::interrupt_control_w(uint8_t data)
{
	data &= ~0xfe;
	LOG("interrupt_control_w[%d]: %02x\n", N, data);
	m_interrupt_control[N] = data;
}

template <int N>
uint32_t vt8231_ide_device::sg_descriptor_address_r(uint32_t mem_mask)
{
	return m_sg_descriptor_address[N];
}

template <int N>
void vt8231_ide_device::sg_descriptor_address_w(uint32_t data, uint32_t mem_mask)
{
	LOG("sg_descriptor_address_r[%d]: %08x & %08x\n", N, data, mem_mask);
	COMBINE_DATA(&m_sg_descriptor_address[N]);
}

uint32_t vt8231_ide_device::pci_pm_block1_r(offs_t offset, uint32_t mem_mask)
{
	return 0x00020001;
}

uint32_t vt8231_ide_device::pci_pm_block2_r(offs_t offset, uint32_t mem_mask)
{
	// bit 1-0: power state (00 = on)
	return 0x00000000;
}

bool vt8231_ide_device::pri_channel_is_native()
{
	return BIT(pclass, 0) || BIT(m_ide_configuration_2, 7);
}

bool vt8231_ide_device::sec_channel_is_native()
{
	return BIT(pclass, 2) || BIT(m_ide_configuration_2, 6);
}

void vt8231_ide_device::update_bar_address()
{
	pci_device::address_base_w(0, pri_channel_is_native() ? m_bar[0] : 0x000001f0);
	pci_device::address_base_w(1, pri_channel_is_native() ? m_bar[1] : 0x000003f4);
	pci_device::address_base_w(2, sec_channel_is_native() ? m_bar[2] : 0x00000170);
	pci_device::address_base_w(3, sec_channel_is_native() ? m_bar[3] : 0x00000374);
	pci_device::address_base_w(4, m_bar[4]);
}

void vt8231_ide_device::pri_command_map(address_map &map)
{
	// 0x01f0
	map(0x0000, 0x0007).rw(m_ide[0], FUNC(bus_master_ide_controller_device::read_cs0), FUNC(bus_master_ide_controller_device::write_cs0));
}

void vt8231_ide_device::pri_control_map(address_map &map)
{
	// 0x03f4 - handled in map_extra()
}

void vt8231_ide_device::sec_command_map(address_map &map)
{
	// 0x0170
	map(0x0000, 0x0007).rw(m_ide[1], FUNC(bus_master_ide_controller_device::read_cs0), FUNC(bus_master_ide_controller_device::write_cs0));
}

void vt8231_ide_device::sec_control_map(address_map &map)
{
	// 0x0374 - handled in map_extra()
}

void vt8231_ide_device::bus_master_ctrl_regs_map(address_map &map)
{
	// 0xcc00
	map(0x0, 0x7).rw(FUNC(vt8231_ide_device::pri_busmaster_dma_r), FUNC(vt8231_ide_device::pri_busmaster_dma_w));
	map(0x8, 0xf).rw(FUNC(vt8231_ide_device::sec_busmaster_dma_r), FUNC(vt8231_ide_device::sec_busmaster_dma_w));
}

uint8_t vt8231_ide_device::pri_cs1_r()
{
	return m_ide[0]->read_cs1(1, 0x00ff0000) >> 16;
}

void vt8231_ide_device::pri_cs1_w(uint8_t data)
{
	m_ide[0]->write_cs1(1, (uint32_t)data << 16, 0x00ff0000);
}

uint8_t vt8231_ide_device::sec_cs1_r()
{
	return m_ide[1]->read_cs1(1, 0x00ff0000) >> 16;
}

void vt8231_ide_device::sec_cs1_w(uint8_t data)
{
	m_ide[1]->write_cs1(1, (uint32_t)data << 16, 0x00ff0000);
}

template <int Channel>
void vt8231_ide_device::ide_irq_w(int state)
{
	m_ide_status[Channel] &= ~0x80;
	m_ide_status[Channel] |= state ? 0x80 : 0x00;

	m_isa->ide_irq_w(Channel, state);
}

uint32_t vt8231_ide_device::pri_busmaster_dma_r(offs_t offset, uint32_t mem_mask)
{
	if (BIT(command, 2) && BIT(m_chip_enable, 1))
		return m_ide[0]->bmdma_r(offset, mem_mask);

	return 0xffffffff;
}

void vt8231_ide_device::pri_busmaster_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (BIT(command, 2) && BIT(m_chip_enable, 1))
		m_ide[0]->bmdma_w(offset, data, mem_mask);
}

uint32_t vt8231_ide_device::sec_busmaster_dma_r(offs_t offset, uint32_t mem_mask)
{
	if (BIT(command, 2) && BIT(m_chip_enable, 0))
		return m_ide[1]->bmdma_r(offset, mem_mask);

	return 0xffffffff;
}

void vt8231_ide_device::sec_busmaster_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (BIT(command, 2) && BIT(m_chip_enable, 0))
		m_ide[1]->bmdma_w(offset, data, mem_mask);
}
