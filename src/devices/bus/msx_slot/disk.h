// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_DISK_H
#define __MSX_SLOT_DISK_H

#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "machine/wd_fdc.h"
#include "machine/upd765.h"
#include "imagedev/flopdrv.h"
#include "imagedev/floppy.h"


/* WD FDC accessed through 7ffx */
extern const device_type MSX_SLOT_DISK1;
/* WD FDC accessed through 7fbx */
extern const device_type MSX_SLOT_DISK2;
/* TC8566 accessed through 7ff8-7fff */
extern const device_type MSX_SLOT_DISK3;
/* TC8566 accessed through 7ff0-7ff7 (used in Turob-R, untested) */
extern const device_type MSX_SLOT_DISK4;
/* WD FDC accessed through i/o ports 0xd0-0xd4 */
extern const device_type MSX_SLOT_DISK5;
/* WD FDC accessed through 7ff0-7ff? (used in Toshiba HX34) */
extern const device_type MSX_SLOT_DISK6;


#define MCFG_MSX_SLOT_DISK1_ADD(_tag, _startpage, _numpages, _region, _offset, _fdc_tag, _floppy0_tag, _floppy1_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_DISK1, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_disk_device::set_fdc_tag(*device, _fdc_tag); \
	msx_slot_disk_device::set_floppy0_tag(*device, _floppy0_tag); \
	msx_slot_disk_device::set_floppy1_tag(*device, _floppy1_tag);

#define MCFG_MSX_SLOT_DISK2_ADD(_tag, _startpage, _numpages, _region, _offset, _fdc_tag, _floppy0_tag, _floppy1_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_DISK2, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_disk_device::set_fdc_tag(*device, _fdc_tag); \
	msx_slot_disk_device::set_floppy0_tag(*device, _floppy0_tag); \
	msx_slot_disk_device::set_floppy1_tag(*device, _floppy1_tag);

#define MCFG_MSX_SLOT_DISK3_ADD(_tag, _startpage, _numpages, _region, _offset, _fdc_tag, _floppy0_tag, _floppy1_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_DISK3, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_disk_device::set_fdc_tag(*device, _fdc_tag); \
	msx_slot_disk_device::set_floppy0_tag(*device, _floppy0_tag); \
	msx_slot_disk_device::set_floppy1_tag(*device, _floppy1_tag);

#define MCFG_MSX_SLOT_DISK4_ADD(_tag, _startpage, _numpages, _region, _offset, _fdc_tag, _floppy0_tag, _floppy1_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_DISK4, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_disk_device::set_fdc_tag(*device, _fdc_tag); \
	msx_slot_disk_device::set_floppy0_tag(*device, _floppy0_tag); \
	msx_slot_disk_device::set_floppy1_tag(*device, _floppy1_tag);

#define MCFG_MSX_SLOT_DISK5_ADD(_tag, _startpage, _numpages, _region, _offset, _fdc_tag, _floppy0_tag, _floppy1_tag, _floppy2_tag, _floppy3_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_DISK5, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_disk_device::set_fdc_tag(*device, _fdc_tag); \
	msx_slot_disk_device::set_floppy0_tag(*device, _floppy0_tag); \
	msx_slot_disk_device::set_floppy1_tag(*device, _floppy1_tag); \
	msx_slot_disk_device::set_floppy2_tag(*device, _floppy2_tag); \
	msx_slot_disk_device::set_floppy3_tag(*device, _floppy3_tag);

#define MCFG_MSX_SLOT_DISK6_ADD(_tag, _startpage, _numpages, _region, _offset, _fdc_tag, _floppy0_tag, _floppy1_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_DISK6, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_disk_device::set_fdc_tag(*device, _fdc_tag); \
	msx_slot_disk_device::set_floppy0_tag(*device, _floppy0_tag); \
	msx_slot_disk_device::set_floppy1_tag(*device, _floppy1_tag);


class msx_slot_disk_device : public msx_slot_rom_device
{
public:
	msx_slot_disk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void device_start() override;

	// static configuration helpers
	static void set_fdc_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_fdc_tag = tag; }
	static void set_floppy0_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_floppy0_tag = tag; }
	static void set_floppy1_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_floppy1_tag = tag; }
	static void set_floppy2_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_floppy2_tag = tag; }
	static void set_floppy3_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_floppy3_tag = tag; }

protected:
	floppy_connector *m_floppy0;
	floppy_connector *m_floppy1;
	floppy_connector *m_floppy2;
	floppy_connector *m_floppy3;
	floppy_image_device *m_floppy;

	const char *m_fdc_tag;
	const char *m_floppy0_tag;
	const char *m_floppy1_tag;
	const char *m_floppy2_tag;
	const char *m_floppy3_tag;
};


class msx_slot_wd_disk_device : public msx_slot_disk_device
{
public:
	msx_slot_wd_disk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void device_start() override;

protected:
	wd_fdc_analog_t *m_fdc;
};


class msx_slot_tc8566_disk_device : public msx_slot_disk_device
{
public:
	msx_slot_tc8566_disk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void device_start() override;

protected:
	tc8566af_device *m_fdc;
};


class msx_slot_disk1_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	void post_load();

private:
	UINT8 m_side_control;
	UINT8 m_control;

	void set_control(UINT8 data);
	void set_side_control(UINT8 data);
};


class msx_slot_disk2_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	void post_load();

private:
	UINT8 m_control;

	void set_control(UINT8 data);
};


class msx_slot_disk3_device : public msx_slot_tc8566_disk_device
{
public:
	msx_slot_disk3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
};


class msx_slot_disk4_device : public msx_slot_tc8566_disk_device
{
public:
	msx_slot_disk4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
};


class msx_slot_disk5_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	DECLARE_READ8_MEMBER(io_read);
	DECLARE_WRITE8_MEMBER(io_write);

	void post_load();

private:
	UINT8 m_control;

	void set_control(UINT8 control);
};


class msx_slot_disk6_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk6_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	void post_load();

private:
	UINT8 m_side_motor;
	UINT8 m_drive_select0;
	UINT8 m_drive_select1;

	void set_side_motor();
	void select_drive();
};


#endif
