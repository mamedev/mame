// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    wozfdc.h

    Apple Disk II floppy disk controller

*********************************************************************/

#pragma once

#ifndef __WOZFDC_H__
#define __WOZFDC_H__

#include "emu.h"
#include "imagedev/floppy.h"
#include "formats/flopimg.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class wozfdc_device:
	public device_t
{
	friend class diskii_fdc;
	friend class appleiii_fdc;

public:
	// construction/destruction
	wozfdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	floppy_connector *floppy0, *floppy1, *floppy2, *floppy3;
	floppy_image_device *floppy;

private:
	enum {
		MODE_IDLE, MODE_ACTIVE, MODE_DELAY
	};

	struct lss {
		attotime tm;
		UINT64 cycles;
		UINT8 data_reg, address;
		attotime write_start_time;
		attotime write_buffer[32];
		int write_position;
		bool write_line_active;
	};

	const UINT8 *m_rom_p6;
	UINT8 last_6502_write;
	bool mode_write, mode_load;
	int active;
	UINT8 phases;
	emu_timer *timer, *delay_timer;
	bool external_drive_select;
	bool external_io_select;

	lss cur_lss, predicted_lss;

	int drvsel;
	int enable1;

	void control(int offset);
	void phase(int ph, bool on);
	UINT64 time_to_cycles(const attotime &tm);
	attotime cycles_to_time(UINT64 cycles);
	void a3_update_drive_sel();

	void lss_start();
	void lss_delay(UINT64 cycles, const attotime &tm, UINT8 data_reg, UINT8 address, bool write_line_active);
	void lss_delay(UINT64 cycles, UINT8 data_reg, UINT8 address, bool write_line_active);
	void lss_sync();
	void lss_predict(attotime limit = attotime::never);
	void commit_predicted();
};

class diskii_fdc : public wozfdc_device
{
public:
	diskii_fdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_reset();

	void set_floppies(floppy_connector *f0, floppy_connector *f1);
};

class appleiii_fdc : public wozfdc_device
{
public:
	appleiii_fdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_reset();

	void set_floppies_4(floppy_connector *f0, floppy_connector *f1, floppy_connector *f2, floppy_connector *f3);

	DECLARE_READ8_MEMBER(read_c0dx);
	DECLARE_WRITE8_MEMBER(write_c0dx);

private:
	void control_dx(int offset);
};

// device type definition
extern const device_type DISKII_FDC;
extern const device_type APPLEIII_FDC;

#endif  /* __WOZFDC_H__ */
