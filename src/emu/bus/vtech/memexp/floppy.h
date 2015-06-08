// license:GPL-2.0+
// copyright-holders:Dirk Best, Olivier Galibert
/***************************************************************************

    VTech Laser/VZ Floppy Controller Cartridge

    Laser DD 20
    Dick Smith Electronics X-7304

***************************************************************************/

#pragma once

#ifndef __VTECH_MEMEXP_FLOPPY_H__
#define __VTECH_MEMEXP_FLOPPY_H__

#include "emu.h"
#include "memexp.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> floppy_controller_device

class floppy_controller_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	floppy_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_READ8_MEMBER(shifter_r);
	DECLARE_READ8_MEMBER(rd_r);
	DECLARE_READ8_MEMBER(wpt_r);

protected:
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

	required_device<memexp_slot_device> m_memexp;
	required_device<floppy_connector> m_floppy0, m_floppy1;
	floppy_image_device *m_floppy;

	UINT8 m_latch, m_shifter;
	bool m_latching_inverter;
	int m_current_cyl;
	attotime m_last_latching_inverter_update_time;
	attotime m_write_start_time, m_write_buffer[32];
	int m_write_position;

	void index_callback(floppy_image_device *floppy, int state);
	void update_latching_inverter();
	void flush_writes(bool keep_margin = false);
};

// device type definition
extern const device_type FLOPPY_CONTROLLER;

#endif // __VTECH_MEMEXP_FLOPPY_H__
