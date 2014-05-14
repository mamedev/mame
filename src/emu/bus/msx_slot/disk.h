#ifndef __MSX_SLOT_DISK_H
#define __MSX_SLOT_DISK_H

#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "machine/wd_fdc.h"
#include "imagedev/flopdrv.h"
#include "imagedev/floppy.h"


extern const device_type MSX_SLOT_DISK1;
extern const device_type MSX_SLOT_DISK2;


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


class msx_slot_disk_device : public msx_slot_rom_device
{
public:
	msx_slot_disk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void device_start();

	// static configuration helpers
	static void set_fdc_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_fdc_tag = tag; }
	static void set_floppy0_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_floppy0_tag = tag; }
	static void set_floppy1_tag(device_t &device, const char *tag) { dynamic_cast<msx_slot_disk_device &>(device).m_floppy1_tag = tag; }

protected:
	wd_fdc_analog_t *m_fdc;
	floppy_connector *m_floppy0;
	floppy_connector *m_floppy1;
	floppy_image_device *m_floppy;

private:
	const char *m_fdc_tag;
	const char *m_floppy0_tag;
	const char *m_floppy1_tag;
};


class msx_slot_disk1_device : public msx_slot_disk_device
{
public:
	msx_slot_disk1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start();
	virtual void device_reset();

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

	void post_load();

private:
	UINT8 m_side_control;
	UINT8 m_control;

	void set_control(UINT8 data);
	void set_side_control(UINT8 data);
};


class msx_slot_disk2_device : public msx_slot_disk_device
{
public:
	msx_slot_disk2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start();
	virtual void device_reset();

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

	void post_load();

private:
	UINT8 m_control;

	void set_control(UINT8 data);
};

#endif

