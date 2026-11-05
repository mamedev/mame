// license:BSD-3-Clause
// copyright-holders:
#ifndef MAME_BUS_DIGIBLAST_NAND_H
#define MAME_BUS_DIGIBLAST_NAND_H

#pragma once

#include "slot.h"

#include "machine/i2cmem.h"
#include "machine/nandflash.h"

class digiblast_nand_device : public device_t, public device_digiblast_cart_interface
{
public:
	virtual u8 data_r() override;
	virtual void data_w(u8 data) override;
	virtual void command_w(u8 data) override;
	virtual void address_w(u8 data) override;
	virtual int i2c_sda_r() override;
	virtual void i2c_sda_w(int state) override;
	virtual void i2c_scl_w(int state) override;
	virtual u32 rom_size() const override;
	virtual void set_rom(const u8 *data, u32 size) override;

protected:
	digiblast_nand_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void configure_cart(machine_config &config) ATTR_COLD;

	required_device<nand_device> m_nand;

private:
	required_device<i2cmem_device> m_i2cmem;
	required_device<digiblast_cart_slot_device> m_slot;
};

class digiblast_nand_16m_device : public digiblast_nand_device
{
public:
	digiblast_nand_16m_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class digiblast_nand_32m_device : public digiblast_nand_device
{
public:
	digiblast_nand_32m_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class digiblast_nand_64m_device : public digiblast_nand_device
{
public:
	digiblast_nand_64m_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	digiblast_nand_64m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class digiblast_nand_64m_r_device : public digiblast_nand_64m_device
{
public:
	digiblast_nand_64m_r_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(DIGIBLAST_NAND_16M,   digiblast_nand_16m_device)
DECLARE_DEVICE_TYPE(DIGIBLAST_NAND_32M,   digiblast_nand_32m_device)
DECLARE_DEVICE_TYPE(DIGIBLAST_NAND_64M,   digiblast_nand_64m_device)
DECLARE_DEVICE_TYPE(DIGIBLAST_NAND_64M_R, digiblast_nand_64m_r_device)

#endif // MAME_BUS_DIGIBLAST_NAND_H
