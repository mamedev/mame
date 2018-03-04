// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_ORICEXT_JASMIN_H
#define MAME_BUS_ORICEXT_JASMIN_H

#pragma once

#include "oricext.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

DECLARE_DEVICE_TYPE(JASMIN, jasmin_device)

class jasmin_device : public oricext_device
{
public:
	jasmin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~jasmin_device();

	DECLARE_INPUT_CHANGED_MEMBER(boot_pressed);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	void remap();

	DECLARE_WRITE8_MEMBER(side_sel_w);
	DECLARE_WRITE8_MEMBER(fdc_reset_w);
	DECLARE_WRITE8_MEMBER(ram_access_w);
	DECLARE_WRITE8_MEMBER(rom_access_w);
	DECLARE_WRITE8_MEMBER(select_w);

	void map(address_map &map);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<wd1770_device> fdc;

	bool side_sel, fdc_reset, ram_access, rom_access, select[4];
	uint8_t *jasmin_rom;
	floppy_image_device *cur_floppy, *floppies[4];
};

#endif // MAME_BUS_ORICEXT_JASMIN_H
