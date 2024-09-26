// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Voicebox for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_VOICEBOX_H
#define MAME_BUS_SAMCOUPE_EXPANSION_VOICEBOX_H

#pragma once

#include "expansion.h"
#include "sound/sp0256.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_voicebox_device

class sam_voicebox_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_voicebox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<sp0256_device> m_sp0256;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_VOICEBOX, sam_voicebox_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_VOICEBOX_H
