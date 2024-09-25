// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC Text/Image/Graphics controller emulation

**********************************************************************/

#ifndef MAME_BUS_WANGPC_TIG_H
#define MAME_BUS_WANGPC_TIG_H

#pragma once

#include "wangpc.h"
#include "video/upd7220.h"
#include "emupal.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_tig_device

class wangpc_tig_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_tig_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint8_t wangpcbus_dack_r(int line) override;
	virtual void wangpcbus_dack_w(int line, uint8_t data) override;
	virtual bool wangpcbus_have_dack(int line) override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );

	void upd7220_0_map(address_map &map) ATTR_COLD;
	void upd7220_1_map(address_map &map) ATTR_COLD;

	// internal state
	required_device<upd7220_device> m_hgdc0;
	required_device<upd7220_device> m_hgdc1;

	uint8_t m_option;
	uint8_t m_attr[16];
	uint8_t m_underline;
	required_device<palette_device> m_palette;
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_TIG, wangpc_tig_device)

#endif // MAME_BUS_WANGPC_TIG_H
