// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __JASMIN_H__
#define __JASMIN_H__

#include "oricext.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

extern const device_type JASMIN;

class jasmin_device : public oricext_device
{
public:
	jasmin_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~jasmin_device();

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_ADDRESS_MAP(map, 8);
	DECLARE_INPUT_CHANGED_MEMBER(boot_pressed);
	DECLARE_WRITE8_MEMBER(side_sel_w);
	DECLARE_WRITE8_MEMBER(fdc_reset_w);
	DECLARE_WRITE8_MEMBER(ram_access_w);
	DECLARE_WRITE8_MEMBER(rom_access_w);
	DECLARE_WRITE8_MEMBER(select_w);

protected:
	required_device<wd1770_t> fdc;

	bool side_sel, fdc_reset, ram_access, rom_access, select[4];
	UINT8 *jasmin_rom;
	floppy_image_device *cur_floppy, *floppies[4];

	virtual void device_start() override;
	virtual void device_reset() override;
	const rom_entry *device_rom_region() const override;
	machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void remap();
};

#endif
