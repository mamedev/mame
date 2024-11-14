// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sound Expansion v3 cartridge (Complex Software Systems)

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_SNDEXP3_H
#define MAME_BUS_ELECTRON_CART_SNDEXP3_H

#include "slot.h"
#include "sound/sn76496.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_sndexp3_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_sndexp3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_device<sn76489_device> m_sn;

	uint8_t m_sound_latch;
	uint8_t m_sound_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_SNDEXP3, electron_sndexp3_device)


#endif // MAME_BUS_ELECTRON_CART_SNDEXP3_H
