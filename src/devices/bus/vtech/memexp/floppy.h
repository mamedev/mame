// license:GPL-2.0+
// copyright-holders:Dirk Best, Olivier Galibert
/***************************************************************************

    VTech Laser/VZ Floppy Controller Cartridge

    Laser DD 20
    Dick Smith Electronics X-7304

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_FLOPPY_H
#define MAME_BUS_VTECH_MEMEXP_FLOPPY_H

#pragma once

#include "memexp.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_floppy_controller_device

class vtech_floppy_controller_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_floppy_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_device<vtech_memexp_slot_device> m_memexp;
	required_device_array<floppy_connector, 2> m_floppy;
	floppy_image_device *m_selected_floppy;

	static void floppy_formats(format_registration &fr);

	void latch_w(uint8_t data);
	uint8_t shifter_r();
	uint8_t rd_r();
	uint8_t wpt_r();

	void index_callback(floppy_image_device *floppy, int state);
	void update_latching_inverter();
	void flush_writes(bool keep_margin = false);

	uint8_t m_latch, m_shifter;
	bool m_latching_inverter;
	int m_current_cyl;
	attotime m_last_latching_inverter_update_time;
	attotime m_write_start_time, m_write_buffer[32];
	int m_write_position;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_FLOPPY_CONTROLLER, vtech_floppy_controller_device)

#endif // MAME_BUS_VTECH_MEMEXP_FLOPPY_H
