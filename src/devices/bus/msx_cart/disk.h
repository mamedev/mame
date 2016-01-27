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
	msx_cart_disk(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);

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
	msx_cart_disk_wd(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);

protected:
	required_device<wd_fdc_analog_t> m_fdc;
};


class msx_cart_disk_type1 : public msx_cart_disk_wd
{
public:
	msx_cart_disk_type1(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

	void post_load();

protected:
	UINT8 m_side_control;
	UINT8 m_control;

	void set_side_control(UINT8 data);
	void set_control(UINT8 data);
};


class msx_cart_disk_type2 : public msx_cart_disk_wd
{
public:
	msx_cart_disk_type2(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

	void post_load();

protected:
	UINT8 m_control;

	void set_control(UINT8 data);
};


class msx_cart_vy0010 : public msx_cart_disk_type1
{
public:
	msx_cart_vy0010(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};


class msx_cart_fsfd1 : public msx_cart_disk_type1
{
public:
	msx_cart_fsfd1(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};


class msx_cart_fscf351 : public msx_cart_disk_type2
{
public:
	msx_cart_fscf351(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};


class msx_cart_disk_tc8566 : public msx_cart_disk
{
public:
	msx_cart_disk_tc8566(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);

protected:
	required_device<tc8566af_device> m_fdc;
};


class msx_cart_fsfd1a : public msx_cart_disk_tc8566
{
public:
	msx_cart_fsfd1a(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;
};


#endif
