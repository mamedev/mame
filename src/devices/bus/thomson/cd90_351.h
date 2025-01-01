// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CD 90-351 - Custom floppy drive controller (THMFC1)
//
// Handles up to two 3.5 dual-sided drives (DD 90-352)
// or up to two 2.8 dual-sided QDD drivers (QD 90-280)

#ifndef MAME_BUS_THOMSON_CD90_351_H
#define MAME_BUS_THOMSON_CD90_351_H

#include "extension.h"
#include "imagedev/floppy.h"

class cd90_351_device : public device_t, public thomson_extension_interface
{
public:
	cd90_351_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cd90_351_device() = default;

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_rom;
	memory_bank_creator m_rom_bank;

	static void floppy_formats(format_registration &fr);
	static void floppy_drives(device_slot_interface &device);

	void bank_w(u8 data);
};

DECLARE_DEVICE_TYPE(CD90_351, cd90_351_device)

#endif
