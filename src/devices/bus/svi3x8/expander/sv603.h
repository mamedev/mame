// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-603 Coleco Game Adapter for SVI-318/328

***************************************************************************/

#pragma once

#ifndef __SVI3X8_EXPANDER_SV603_H__
#define __SVI3X8_EXPANDER_SV603_H__

#include "emu.h"
#include "expander.h"
#include "sound/sn76496.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv603_device

class sv603_device : public device_t, public device_svi_expander_interface
{
public:
	// construction/destruction
	sv603_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// from host
	virtual DECLARE_READ8_MEMBER( mreq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( mreq_w ) override;
	virtual DECLARE_READ8_MEMBER( iorq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( iorq_w ) override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);

protected:
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_bios;
	required_device<sn76489a_device> m_snd;
	required_device<generic_slot_device> m_cart_rom;
};

// device type definition
extern const device_type SV603;

#endif // __SVI3X8_EXPANDER_SV603_H__
