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
	msx_cart_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	static void floppy_formats(format_registration &fr);
};


class msx_cart_disk_wd_device : public msx_cart_disk_device
{
protected:
	msx_cart_disk_wd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<wd_fdc_analog_device_base> m_fdc;
};


class msx_cart_disk_type1_device : public msx_cart_disk_wd_device
{
public:
	virtual void initialize_cartridge() override;

protected:
	msx_cart_disk_type1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	u8 side_control_r();
	u8 control_r();
	u8 status_r();
	void set_side_control(u8 data);
	void set_control(u8 data);

	output_finder<> m_led;
	u8 m_side_control;
	u8 m_control;

};


class msx_cart_disk_type2_device : public msx_cart_disk_wd_device
{
public:
	virtual void initialize_cartridge() override;

protected:
	msx_cart_disk_type2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	void set_control(u8 data);
	u8 status_r();

	output_finder<> m_led;
	u8 m_control;
};


class msx_cart_vy0010_device : public msx_cart_disk_type1_device
{
public:
	msx_cart_vy0010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class msx_cart_fsfd1_device : public msx_cart_disk_type1_device
{
public:
	msx_cart_fsfd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class msx_cart_fscf351_device : public msx_cart_disk_type2_device
{
public:
	msx_cart_fscf351_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


class msx_cart_disk_tc8566_device : public msx_cart_disk_device
{
protected:
	msx_cart_disk_tc8566_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<tc8566af_device> m_fdc;
};


class msx_cart_fsfd1a_device : public msx_cart_disk_tc8566_device
{
public:
	msx_cart_fsfd1a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void initialize_cartridge() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
};


#endif // MAME_BUS_MSX_CART_DISK_H
