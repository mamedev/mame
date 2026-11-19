// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    VIA VT8231 South Bridge - IDE controller

***************************************************************************/

#ifndef MAME_MACHINE_VT8231_IDE_H
#define MAME_MACHINE_VT8231_IDE_H

#pragma once

#include "pci.h"
#include "vt8231_isa.h"
#include "machine/idectrl.h"


class vt8231_ide_device : public pci_device
{
public:
	template <typename T>
	vt8231_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&bm_tag, int bm_space = AS_PROGRAM) :
		vt8231_ide_device(mconfig, tag, owner, clock)
	{
		set_bus_master_space(std::forward<T>(bm_tag), bm_space);
	}

	vt8231_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T>
	vt8231_ide_device &set_bus_master_space(T &&... args)
	{
		m_bus_master_space.set_tag(std::forward<T>(args)...);
		return *this;
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual uint8_t capptr_r() override;

private:
	void prog_if_w(uint8_t data);
	uint32_t bar_r(offs_t offset, uint32_t mem_mask);
	void bar_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t chip_enable_r();
	void chip_enable_w(uint8_t data);
	uint8_t ide_configuration_1_r();
	void ide_configuration_1_w(uint8_t data);
	uint8_t ide_configuration_2_r();
	void ide_configuration_2_w(uint8_t data);
	uint8_t fifo_configuration_r();
	void fifo_configuration_w(uint8_t data);
	uint8_t misc_control_1_r();
	void misc_control_1_w(uint8_t data);
	uint8_t misc_control_2_r();
	void misc_control_2_w(uint8_t data);
	uint8_t misc_control_3_r();
	void misc_control_3_w(uint8_t data);
	uint8_t drive_timing_control_r(offs_t offset);
	void drive_timing_control_w(offs_t offset, uint8_t data);
	uint8_t address_setup_time_r();
	void address_setup_time_w(uint8_t data);
	uint8_t ultradma_extended_timing_control_r(offs_t offset);
	void ultradma_extended_timing_control_w(offs_t offset, uint8_t data);
	uint8_t ultradma_fifo_control_r();
	void ultradma_fifo_control_w(uint8_t data);
	template <int N> uint16_t sector_size_r(offs_t offset, uint16_t mem_mask);
	template <int N> void sector_size_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	template <int N> uint8_t ide_status_r();
	template <int N> void ide_status_w(uint8_t data);
	template <int N> uint8_t interrupt_control_r();
	template <int N> void interrupt_control_w(uint8_t data);
	uint16_t secondary_sector_size_r(uint16_t mem_mask);
	void secondary_sector_size_w(uint16_t data, uint16_t mem_mask);
	template <int N> uint32_t sg_descriptor_address_r(uint32_t mem_mask);
	template <int N> void sg_descriptor_address_w(uint32_t data, uint32_t mem_mask);
	uint32_t pci_pm_block1_r(offs_t offset, uint32_t mem_mask);
	uint32_t pci_pm_block2_r(offs_t offset, uint32_t mem_mask);

	bool pri_channel_is_native();
	bool sec_channel_is_native();
	void update_bar_address();

	void pri_command_map(address_map &map) ATTR_COLD;
	void pri_control_map(address_map &map) ATTR_COLD;
	void sec_command_map(address_map &map) ATTR_COLD;
	void sec_control_map(address_map &map) ATTR_COLD;
	void bus_master_ctrl_regs_map(address_map &map) ATTR_COLD;

	uint8_t pri_cs1_r();
	void pri_cs1_w(uint8_t data);
	uint8_t sec_cs1_r();
	void sec_cs1_w(uint8_t data);

	template <int Channel> void ide_irq_w(int state);

	uint32_t pri_busmaster_dma_r(offs_t offset, uint32_t mem_mask);
	void pri_busmaster_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sec_busmaster_dma_r(offs_t offset, uint32_t mem_mask);
	void sec_busmaster_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	required_address_space m_bus_master_space;
	required_device_array<bus_master_ide_controller_device, 2> m_ide;

	required_device<vt8231_isa_device> m_isa;
	std::string m_isa_tag;

	uint32_t m_bar[5];
	uint8_t m_chip_enable;
	uint8_t m_ide_configuration_1;
	uint8_t m_ide_configuration_2;
	uint8_t m_fifo_configuration;
	uint8_t m_misc_control_1;
	uint8_t m_misc_control_2;
	uint8_t m_misc_control_3;
	uint8_t m_drive_timing_control[4];
	uint8_t m_address_setup_time;
	uint8_t m_ultradma_extended_timing_control[4];
	uint8_t m_ultradma_fifo_control;
	uint16_t m_sector_size[2];
	uint8_t m_ide_status[2];
	uint8_t m_interrupt_control[2];
	uint32_t m_sg_descriptor_address[2];

	bool m_initialized;
};


// device type definition
DECLARE_DEVICE_TYPE(VT8231_IDE, vt8231_ide_device)

#endif // MAME_MACHINE_VT8231_IDE_H
