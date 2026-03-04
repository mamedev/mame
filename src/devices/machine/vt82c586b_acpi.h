// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_VT82C586B_ACPI_H
#define MAME_MACHINE_VT82C586B_ACPI_H

#pragma once

#include "lpc.h"
#include "pci.h"

class acpi_pipc_device;

class vt82c586b_acpi_device : public pci_device
{
public:
	vt82c586b_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto sci_pin_cb() { return m_sci_pin_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	void acpi_map(address_map &map) ATTR_COLD;
private:
	required_device<acpi_pipc_device> m_acpi;
	devcb_write8 m_sci_pin_cb;

	u8 m_pin_config;
	u8 m_general_config;
	u8 m_sci_irq_config;
	u16 m_iobase;
	u32 m_gp_timer_control;
	u16 m_irq_channel[2];
};

class acpi_pipc_device : public lpc_device
                       , public device_memory_interface
{
public:
	acpi_pipc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = XTAL(3'579'545).value());

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;


	void map(address_map &map);

	auto smi() { return m_write_smi.bind(); }
	auto sci() { return m_write_sci.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_validity_check(validity_checker &valid) const override;

	virtual space_config_vector memory_space_config() const override;

private:
	devcb_write_line m_write_smi;
	devcb_write_line m_write_sci;

	address_space_config m_space_config;

	void io_map(address_map &map);

	u16 m_pmsts;
	u16 m_pmen;
	u16 m_pmcntrl;
	u16 m_gpen;
	u32 m_pcntrl;

	u16 m_gpsts;
	u16 m_gp_sci_enable;
	u16 m_gp_smi_enable;
	u16 m_power_supply_control;
	u16 m_global_status;
	u16 m_global_enable;
	u16 m_gbl_ctl;
	u8 m_smi_cmd;
	u32 m_primary_activity_status;
	u32 m_primary_activity_enable;
	u32 m_gp_timer_reload_enable;
	u8 m_gpio_dir;
	u8 m_gpio_val;
	u16 m_gpo_val;

	u8 gpsts_r(offs_t offset);
	void gpsts_w(offs_t offset, u8 data);
	u8 gp_sci_enable_r(offs_t offset);
	void gp_sci_enable_w(offs_t offset, u8 data);

	void check_smi();
//	void check_sci();
};

DECLARE_DEVICE_TYPE(VT82C586B_ACPI, vt82c586b_acpi_device)
DECLARE_DEVICE_TYPE(ACPI_PIPC, acpi_pipc_device)

#endif // MAME_MACHINE_VT82C586B_ACPI_H
