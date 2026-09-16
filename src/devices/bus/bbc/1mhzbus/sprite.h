// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Logotron Sprite Board

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_SPRITE_H
#define MAME_BUS_BBC_1MHZBUS_SPRITE_H

#include "1mhzbus.h"
#include "video/tms9928a.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_sprite_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;

private:
	required_device<tms9129_device> m_vdp;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_SPRITE, bbc_sprite_device);


#endif /* MAME_BUS_BBC_1MHZBUS_SPRITE_H */
