// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana Floppy Disk System

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_FDsystem.html

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_CUMANA_H
#define MAME_BUS_ELECTRON_CART_CUMANA_H

#include "slot.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/mc146818.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_cumana_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_cumana_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	void control_w(uint8_t data);
	static void floppy_formats(format_registration &fr);

	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<mc146818_device> m_rtc;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_CUMANA, electron_cumana_device)


#endif // MAME_BUS_ELECTRON_CART_CUMANA_H
