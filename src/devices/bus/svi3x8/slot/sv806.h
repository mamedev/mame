// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-806 80 column card for SVI-318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SV806_H
#define MAME_BUS_SVI3X8_SLOT_SV806_H

#pragma once

#include "slot.h"
#include "video/mc6845.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv806_device

class sv806_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv806_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_gfx;

	std::unique_ptr<uint8_t[]> m_ram;
	int m_ram_enabled;
};

// device type definition
DECLARE_DEVICE_TYPE(SV806, sv806_device)

#endif // MAME_BUS_SVI3X8_SLOT_SV806_H
