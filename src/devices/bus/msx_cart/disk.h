// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_DISK_H
#define MAME_BUS_MSX_CART_DISK_H

#pragma once

#include "bus/msx_cart/cartridge.h"
#include "machine/wd_fdc.h"
#include "machine/upd765.h"
#include "imagedev/floppy.h"


DECLARE_DEVICE_TYPE(MSX_CART_VY0010,  msx_cart_vy0010_device)
DECLARE_DEVICE_TYPE(MSX_CART_FSFD1,   msx_cart_fsfd1_device)
DECLARE_DEVICE_TYPE(MSX_CART_FSFD1A,  msx_cart_fsfd1a_device)
DECLARE_DEVICE_TYPE(MSX_CART_FSCF351, msx_cart_fscf351_device)


class msx_cart_disk_device : public device_t, public msx_cart_interface
{
public:
	virtual void initialize_cartridge() override;

protected:
	msx_cart_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	DECLARE_FLOPPY_FORMATS(floppy_formats);
};


class msx_cart_disk_wd_device : public msx_cart_disk_device
{
protected:
	msx_cart_disk_wd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<wd_fdc_analog_device_base> m_fdc;
};


class msx_cart_disk_type1_device : public msx_cart_disk_wd_device
{
public:
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	msx_cart_disk_type1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	output_finder<> m_led;
	uint8_t m_side_control;
	uint8_t m_control;

	void set_side_control(uint8_t data);
	void set_control(uint8_t data);
};


class msx_cart_disk_type2_device : public msx_cart_disk_wd_device
{
public:
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	msx_cart_disk_type2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	output_finder<> m_led;
	uint8_t m_control;

	void set_control(uint8_t data);
};


class msx_cart_vy0010_device : public msx_cart_disk_type1_device
{
public:
	msx_cart_vy0010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class msx_cart_fsfd1_device : public msx_cart_disk_type1_device
{
public:
	msx_cart_fsfd1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class msx_cart_fscf351_device : public msx_cart_disk_type2_device
{
public:
	msx_cart_fscf351_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class msx_cart_disk_tc8566_device : public msx_cart_disk_device
{
protected:
	msx_cart_disk_tc8566_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<tc8566af_device> m_fdc;
};


class msx_cart_fsfd1a_device : public msx_cart_disk_tc8566_device
{
public:
	msx_cart_fsfd1a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
};


#endif // MAME_BUS_MSX_CART_DISK_H
