// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_DISK_H
#define MAME_BUS_MSX_SLOT_DISK_H

#pragma once

#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "machine/wd_fdc.h"
#include "machine/upd765.h"
#include "imagedev/floppy.h"


/* WD FDC accessed through 7ffx */
DECLARE_DEVICE_TYPE(MSX_SLOT_DISK1, msx_slot_disk1_device)
/* WD FDC accessed through 7fbx */
DECLARE_DEVICE_TYPE(MSX_SLOT_DISK2, msx_slot_disk2_device)
/* TC8566 accessed through 7ff8-7fff */
DECLARE_DEVICE_TYPE(MSX_SLOT_DISK3, msx_slot_disk3_device)
/* TC8566 accessed through 7ff0-7ff7 (used in Turob-R, untested) */
DECLARE_DEVICE_TYPE(MSX_SLOT_DISK4, msx_slot_disk4_device)
/* WD FDC accessed through i/o ports 0xd0-0xd4 */
DECLARE_DEVICE_TYPE(MSX_SLOT_DISK5, msx_slot_disk5_device)
/* WD FDC accessed through 7ff0-7ff? (used in Toshiba HX34) */
DECLARE_DEVICE_TYPE(MSX_SLOT_DISK6, msx_slot_disk6_device)


class msx_slot_disk_device : public msx_slot_rom_device
{
public:
	// configuration helpers
	void set_fdc_tag(const char *tag) { m_fdc_tag = tag; }
	void set_floppy0_tag(const char *tag) { m_floppy0_tag = tag; }
	void set_floppy1_tag(const char *tag) { m_floppy1_tag = tag; }
	void set_floppy2_tag(const char *tag) { m_floppy2_tag = tag; }
	void set_floppy3_tag(const char *tag) { m_floppy3_tag = tag; }

	void set_tags(const char *fdc_tag, const char *floppy0_tag, const char *floppy1_tag)
	{
		m_fdc_tag = fdc_tag;
		m_floppy0_tag = floppy0_tag;
		m_floppy1_tag = floppy1_tag;
	}
	void set_tags(const char *fdc_tag, const char *floppy0_tag, const char *floppy1_tag, const char *floppy2_tag, const char *floppy3_tag)
	{
		m_fdc_tag = fdc_tag;
		m_floppy0_tag = floppy0_tag;
		m_floppy1_tag = floppy1_tag;
		m_floppy2_tag = floppy2_tag;
		m_floppy3_tag = floppy3_tag;
	}

protected:
	msx_slot_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

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
protected:
	msx_slot_wd_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

	wd_fdc_analog_device_base *m_fdc;
	output_finder<> m_led;
};


class msx_slot_tc8566_disk_device : public msx_slot_disk_device
{
protected:
	msx_slot_tc8566_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

	tc8566af_device *m_fdc;
};


class msx_slot_disk1_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t m_side_control;
	uint8_t m_control;

	void set_control(uint8_t data);
	void set_side_control(uint8_t data);
};


class msx_slot_disk2_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t m_control;

	void set_control(uint8_t data);
};


class msx_slot_disk3_device : public msx_slot_tc8566_disk_device
{
public:
	msx_slot_disk3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


class msx_slot_disk4_device : public msx_slot_tc8566_disk_device
{
public:
	msx_slot_disk4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


class msx_slot_disk5_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t io_read(offs_t offset);
	void io_write(offs_t offset, uint8_t data);

	uint8_t m_control;

	void set_control(uint8_t control);
};


class msx_slot_disk6_device : public msx_slot_wd_disk_device
{
public:
	msx_slot_disk6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	uint8_t m_side_motor;
	uint8_t m_drive_select0;
	uint8_t m_drive_select1;

	void set_side_motor();
	void select_drive();
};


#endif // MAME_BUS_MSX_SLOT_DISK_H
