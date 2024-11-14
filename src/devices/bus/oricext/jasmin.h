// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_ORICEXT_JASMIN_H
#define MAME_BUS_ORICEXT_JASMIN_H

#pragma once

#include "oricext.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/wd_fdc.h"

DECLARE_DEVICE_TYPE(ORIC_JASMIN, oric_jasmin_device)

class oric_jasmin_device : public device_t, public device_oricext_interface
{
public:
	oric_jasmin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~oric_jasmin_device();

	DECLARE_INPUT_CHANGED_MEMBER(boot_pressed);

protected:
	virtual void device_start() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map_io(address_space_installer &space) override;
	virtual void map_rom() override;

private:
	void remap();

	void side_sel_w(int state);
	void ram_access_w(int state);
	void rom_access_w(int state);
	void select_w(int state);

	static void floppy_formats(format_registration &fr);

	required_device<wd1770_device> m_fdc;
	required_device<ls259_device> m_fdlatch;
	required_device_array<floppy_connector, 4> m_floppies;
	required_region_ptr<uint8_t> m_jasmin_rom;
	floppy_image_device *m_cur_floppy;
};

#endif // MAME_BUS_ORICEXT_JASMIN_H
