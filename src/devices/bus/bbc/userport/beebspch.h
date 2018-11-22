// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Beeb Speech Synthesiser - Watford Electronics

**********************************************************************/

#ifndef MAME_BUS_BBC_USERPORT_BEEBSPCH_H
#define MAME_BUS_BBC_USERPORT_BEEBSPCH_H

#include "userport.h"
#include "sound/sp0256.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_beebspch_device

class bbc_beebspch_device :
	public device_t,
	public device_bbc_userport_interface
{
public:
	// construction/destruction
	bbc_beebspch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(cb1_w);
	DECLARE_WRITE_LINE_MEMBER(cb2_w);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual DECLARE_WRITE8_MEMBER(pb_w) override;

private:
	required_device<sp0256_device> m_nsp;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_BEEBSPCH, bbc_beebspch_device)


#endif // MAME_BUS_BBC_USERPORT_BEEBSPCH_H
