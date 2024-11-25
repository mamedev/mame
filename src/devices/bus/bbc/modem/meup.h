// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Master Extra User Port

**********************************************************************/


#ifndef MAME_BUS_BBC_MODEM_MEUP_H
#define MAME_BUS_BBC_MODEM_MEUP_H

#include "modem.h"
#include "machine/6522via.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_meup_device: public device_t, public device_bbc_modem_interface
{
public:
	// construction/destruction
	bbc_meup_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<via6522_device> m_via;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_MEUP, bbc_meup_device);


#endif /* MAME_BUS_BBC_MODEM_MEUP_H */
