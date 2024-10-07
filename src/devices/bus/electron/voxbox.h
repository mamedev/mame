// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Millsgrade Voxbox Speech Synthesiser

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_VOXBOX_H
#define MAME_BUS_ELECTRON_VOXBOX_H

#pragma once


#include "exp.h"
#include "sound/sp0256.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_voxbox_device

class electron_voxbox_device : public device_t, public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_voxbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void expbus_w(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	required_device<sp0256_device> m_nsp;

	void lrq_cb(int state);

	int m_nmi;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_VOXBOX, electron_voxbox_device)


#endif // MAME_BUS_ELECTRON_VOXBOX_H
