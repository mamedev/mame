// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_DISK_H
#define __MSX_CART_DISK_H

#include "bus/msx_cart/cartridge.h"
#include "machine/wd_fdc.h"
#include "machine/upd765.h"
#include "imagedev/flopdrv.h"
#include "imagedev/floppy.h"


extern const device_type MSX_CART_VY0010;
extern const device_type MSX_CART_FSFD1;
extern const device_type MSX_CART_FSFD1A;
extern const device_type MSX_CART_FSCF351;


class msx_cart_disk : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_disk(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname);

	virtual void initialize_cartridge() override;

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
};


class msx_cart_disk_wd : public msx_cart_disk
{
public:
	msx_cart_disk_wd(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname);

protected:
	required_device<wd_fdc_analog_t> m_fdc;
};


class msx_cart_disk_type1 : public msx_cart_disk_wd
{
public:
	msx_cart_disk_type1(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void post_load();

protected:
	uint8_t m_side_control;
	uint8_t m_control;

	void set_side_control(uint8_t data);
	void set_control(uint8_t data);
};


class msx_cart_disk_type2 : public msx_cart_disk_wd
{
public:
	msx_cart_disk_type2(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void post_load();

protected:
	uint8_t m_control;

	void set_control(uint8_t data);
};


class msx_cart_vy0010 : public msx_cart_disk_type1
{
public:
	msx_cart_vy0010(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};


class msx_cart_fsfd1 : public msx_cart_disk_type1
{
public:
	msx_cart_fsfd1(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};


class msx_cart_fscf351 : public msx_cart_disk_type2
{
public:
	msx_cart_fscf351(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};


class msx_cart_disk_tc8566 : public msx_cart_disk
{
public:
	msx_cart_disk_tc8566(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname);

protected:
	required_device<tc8566af_device> m_fdc;
};


class msx_cart_fsfd1a : public msx_cart_disk_tc8566
{
public:
	msx_cart_fsfd1a(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


#endif
