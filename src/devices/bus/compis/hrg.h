// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis (Ultra) High Resolution Graphics adapter emulation

**********************************************************************/

#ifndef MAME_BUS_COMPIS_HRG_H
#define MAME_BUS_COMPIS_HRG_H

#pragma once

#include "graphics.h"
#include "video/upd7220.h"
#include "emupal.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_hrg_device

class compis_hrg_device : public device_t,
					 public device_compis_graphics_card_interface
{
public:
	// construction/destruction
	compis_hrg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	compis_hrg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_compis_graphics_card_interface overrides
	virtual uint8_t pcs6_6_r(offs_t offset) override;
	virtual void pcs6_6_w(offs_t offset, uint8_t data) override;

	required_device<upd7220_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_video_ram;

	uint8_t m_unk_video;

private:
	UPD7220_DISPLAY_PIXELS_MEMBER( display_pixels );

	void hrg_map(address_map &map) ATTR_COLD;
};


// ======================> compis_uhrg_device

class compis_uhrg_device : public compis_hrg_device
{
public:
	// construction/destruction
	compis_uhrg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	UPD7220_DISPLAY_PIXELS_MEMBER( display_pixels );

	void uhrg_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_HRG,  compis_hrg_device)
DECLARE_DEVICE_TYPE(COMPIS_UHRG, compis_uhrg_device)


#endif // MAME_BUS_COMPIS_HRG_H
