// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The LISA FDC subsystem

#ifndef MAME_APPLE_LISAFDC_H
#define MAME_APPLE_LISAFDC_H

#pragma once

#include "cpu/m6502/m6504.h"
#include "machine/iwm.h"
#include "machine/nvram.h"

class lisa_base_fdc_device : public device_t {
public:
	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);

	auto write_diag() { return m_diag_cb.bind(); }
	auto write_fdir() { return m_fdir_cb.bind(); }

protected:
	required_device<m6504_device> m_cpu;
	required_device<nvram_device> m_nvram;
	required_shared_ptr<u8> m_ram;
	required_device_array<floppy_connector, 2> m_floppy;
	devcb_write_line m_diag_cb;
	devcb_write_line m_fdir_cb;
	floppy_image_device *m_cur_floppy;
	u8 m_hds, m_sel, m_phases;

	lisa_base_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void mt0_0();
	void mt0_1();
	void mt1_0();
	void mt1_1();
	void dis_0();
	void dis_1();
	void hds_0();
	void hds_1();
	void diag_0();
	void diag_1();
	void fdir_0();
	void fdir_1();

	virtual void update_sel();
	void update_phases();
};

class lisa_original_fdc_device : public lisa_base_fdc_device {
protected:
	lisa_original_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

	void ph0_0();
	void ph0_1();
	void ph1_0();
	void ph1_1();
	void ph2_0();
	void ph2_1();
	void ph3_0();
	void ph3_1();
	void dr1_0();
	void dr1_1();
	void dr2_0();
	void dr2_1();
};

class lisa_fdc_device : public lisa_original_fdc_device {
public:
	lisa_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class lisa2_fdc_device : public lisa_original_fdc_device {
public:
	lisa2_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class macxl_fdc_device : public lisa_base_fdc_device {
public:
	macxl_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void pwm_w(u8 data);

protected:
	required_device<iwm_device> m_iwm;
	u8 m_pwm;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

	void stop_0();
	void stop_1();

	void devsel_w(u8 devsel);
	void phases_w(uint8_t phases);

	virtual void update_sel() override;
	void update_pwm();
};

DECLARE_DEVICE_TYPE(LISAFDC,  lisa_fdc_device)
DECLARE_DEVICE_TYPE(LISA2FDC, lisa2_fdc_device)
DECLARE_DEVICE_TYPE(MACXLFDC, macxl_fdc_device)

#endif
