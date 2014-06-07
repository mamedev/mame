/***************************************************************************

	VTech Laser/VZ Floppy Controller Cartridge

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

	Laser DD 20
    Dick Smith Electronics X-7304

***************************************************************************/

#pragma once

#ifndef __VTECH_MEMEXP_FLOPPY_H__
#define __VTECH_MEMEXP_FLOPPY_H__

#include "emu.h"
#include "memexp.h"
#include "imagedev/flopdrv.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> floppy_controller_device

class floppy_controller_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	floppy_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( floppy_r );
	DECLARE_WRITE8_MEMBER( floppy_w );

	// accessed from loadproc
	int get_floppy_id(device_image_interface *image);
	UINT8 m_fdc_wrprot[2];

protected:
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

private:
	static const int TRKSIZE_VZ = 0x9a0;	// arbitrary (actually from analyzing format)
	static const int TRKSIZE_FM = 3172;		// size of a standard FM mode track

	device_image_interface *get_floppy_device(int drive);
	void put_track();
	void get_track();

	required_device<memexp_slot_device> m_memexp;
	required_device<legacy_floppy_image_device> m_floppy0;
	required_device<legacy_floppy_image_device> m_floppy1;

	int m_drive;
	UINT8 m_fdc_track_x2[2];
	UINT8 m_fdc_status;
	UINT8 m_fdc_data[TRKSIZE_FM];
	int m_data;
	int m_fdc_edge;
	int m_fdc_bits;
	int m_fdc_start;
	int m_fdc_write;
	int m_fdc_offs;
	int m_fdc_latch;
};

// device type definition
extern const device_type FLOPPY_CONTROLLER;

#endif // __VTECH_MEMEXP_FLOPPY_H__
