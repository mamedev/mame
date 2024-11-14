// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ALA13 - Acorn Plus 3

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_PLUS3_H
#define MAME_BUS_ELECTRON_PLUS3_H

#include "exp.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_plus3_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_plus3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	void wd1770_status_w(uint8_t data);
	static void floppy_formats(format_registration &fr);

	required_device<electron_expansion_slot_device> m_exp;
	required_memory_region m_exp_rom;
	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	uint8_t m_romsel;
	int m_drive_control;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_PLUS3, electron_plus3_device)


#endif // MAME_BUS_ELECTRON_PLUS3_H
