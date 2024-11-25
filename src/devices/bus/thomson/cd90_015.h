// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CD 90-015 - Floppy drive controller built from a hd 46503s aka hd6843 aka mc6843
//
// Handles up to four 5.25 single-sided drives (DD 90-015)

#ifndef MAME_BUS_THOMSON_CD90_015_H
#define MAME_BUS_THOMSON_CD90_015_H

#include "extension.h"
#include "imagedev/floppy.h"
#include "machine/mc6843.h"

class cd90_015_device : public device_t, public thomson_extension_interface
{
public:
	cd90_015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cd90_015_device() = default;

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(motor_tick);

private:
	required_device<mc6843_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_memory_region m_rom;

	emu_timer *m_motor_timer[4];
	u8 m_select;

	void select_w(u8 data);
	u8 motor_r();

	static void floppy_formats(format_registration &fr);
	static void floppy_drives(device_slot_interface &device);
};

DECLARE_DEVICE_TYPE(CD90_015, cd90_015_device)

#endif
