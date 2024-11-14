// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Tekram DC-320/DC-820 SCSI Controllers

***************************************************************************/

#ifndef MAME_BUS_ISA_TEKRAM_DC820_H
#define MAME_BUS_ISA_TEKRAM_DC820_H

#pragma once

#include "isa.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/upd765.h"

class tekram_eisa_scsi_device : public device_t, public device_isa16_card_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	tekram_eisa_scsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	u8 latch_status_r();
	void int0_ack_w(u8 data);
	u8 status_r();
	void misc_w(u8 data);
	void aux_w(u8 data);
	void mask_w(u8 data);
	void eeprom_w(u8 data);

	void common_map(address_map &map) ATTR_COLD;
	void scsic_config(device_t *device);
	void scsi_add(machine_config &config);

	required_device<cpu_device> m_mpu;
	required_device<generic_latch_8_device> m_cmdlatch;
	required_device<generic_latch_8_device> m_hostlatch;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

class tekram_dc320b_device : public tekram_eisa_scsi_device
{
public:
	tekram_dc320b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void eeprom_w(u8 data);
	u8 latch_status_r();
	u8 status_r();

	void mpu_map(address_map &map) ATTR_COLD;
};

class tekram_dc320e_device : public tekram_eisa_scsi_device
{
public:
	tekram_dc320e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mpu_map(address_map &map) ATTR_COLD;
};

class tekram_dc820_device : public tekram_eisa_scsi_device
{
public:
	tekram_dc820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void eeprom_w(u8 data);

	void mpu_map(address_map &map) ATTR_COLD;
};

class tekram_dc820b_device : public tekram_eisa_scsi_device
{
public:
	tekram_dc820b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mpu_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(TEKRAM_DC320B, tekram_dc320b_device)
DECLARE_DEVICE_TYPE(TEKRAM_DC320E, tekram_dc320e_device)
DECLARE_DEVICE_TYPE(TEKRAM_DC820, tekram_dc820_device)
DECLARE_DEVICE_TYPE(TEKRAM_DC820B, tekram_dc820b_device)

#endif // MAME_BUS_ISA_TEKRAM_DC820_H
