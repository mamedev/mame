// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Pegasus 400 disk interface

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_PEG400_H
#define MAME_BUS_ELECTRON_CART_PEG400_H

#include "slot.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_peg400_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_peg400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	void wd1770_control_w(uint8_t data);
	static void floppy_formats(format_registration &fr);
	void fdc_drq_w(int state);

	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	int m_fdc_ie;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_PEG400, electron_peg400_device)


#endif // MAME_BUS_ELECTRON_CART_PEG400_H
