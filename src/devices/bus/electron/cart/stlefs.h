// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk EFS

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_EFS.html

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_STLEFS_H
#define MAME_BUS_ELECTRON_CART_STLEFS_H

#include "slot.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "formats/acorn_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_stlefs_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_stlefs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	void wd1770_control_w(uint8_t data);
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_STLEFS, electron_stlefs_device)


#endif // MAME_BUS_ELECTRON_CART_STLEFS_H
