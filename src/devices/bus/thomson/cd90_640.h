// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CD 90-640 - Floppy drive controller built from a wd1770
//
// Handles up to two 5.25 dual-sided drives (DD 90-320)

#ifndef MAME_BUS_THOMSON_CD90_640_H
#define MAME_BUS_THOMSON_CD90_640_H

#include "extension.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

class cd90_640_device : public device_t, public thomson_extension_interface
{
public:
	cd90_640_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cd90_640_device() = default;

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<wd1770_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_rom;

	u8 m_control;

	void control_w(u8 data);
	u8 control_r();

	static void floppy_formats(format_registration &fr);
	static void floppy_drives(device_slot_interface &device);
};

DECLARE_DEVICE_TYPE(CD90_640, cd90_640_device)

#endif
