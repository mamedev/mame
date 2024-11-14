// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    wozfdc.h

    Apple Disk II floppy disk controller

*********************************************************************/

#ifndef MAME_MACHINE_WOZFDC_H
#define MAME_MACHINE_WOZFDC_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/74259.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class wozfdc_device:
	public device_t
{
public:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// construction/destruction
	wozfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(generic_tick);
	TIMER_CALLBACK_MEMBER(delayed_tick);

	void control(int offset);
	void set_phase(uint8_t data);
	uint64_t time_to_cycles(const attotime &tm);
	attotime cycles_to_time(uint64_t cycles);
	void a3_update_drive_sel();

	void lss_start();
	void lss_sync();

	enum {
		MODE_IDLE, MODE_ACTIVE, MODE_DELAY
	};

	floppy_connector *floppy0, *floppy1, *floppy2, *floppy3;
	floppy_image_device *floppy;

	required_device<addressable_latch_device> m_phaselatch;

	uint64_t cycles;
	uint8_t data_reg, address;
	attotime write_start_time;
	attotime write_buffer[32];
	int write_position;
	bool write_line_active;

	const uint8_t *m_rom_p6;
	uint8_t last_6502_write;
	bool mode_write, mode_load;
	int active;
	emu_timer *timer, *delay_timer;
	bool external_drive_select;
	bool external_io_select;

	int drvsel;
	int enable1;
};

class diskii_fdc_device : public wozfdc_device
{
public:
	diskii_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_floppies(floppy_connector *f0, floppy_connector *f1);

protected:
	virtual void device_reset() override ATTR_COLD;
};

class appleiii_fdc_device : public wozfdc_device
{
public:
	appleiii_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_floppies_4(floppy_connector *f0, floppy_connector *f1, floppy_connector *f2, floppy_connector *f3);

	uint8_t read_c0dx(uint8_t offset);
	void write_c0dx(uint8_t offset, uint8_t data);

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	void control_dx(int offset);
};

// device type definition
DECLARE_DEVICE_TYPE(DISKII_FDC,   diskii_fdc_device)
DECLARE_DEVICE_TYPE(APPLEIII_FDC, appleiii_fdc_device)

#endif // MAME_MACHINE_WOZFDC_H
