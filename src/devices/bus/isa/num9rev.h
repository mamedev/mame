// license:BSD-3-Clause
// copyright-holders:Carl
#pragma once

#ifndef __NUM9REV_H__
#define __NUM9REV_H__

#include "isa.h"
#include "video/upd7220.h"
#include "machine/bankdev.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa8_number_9_rev_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_number_9_rev_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;

		UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_display_pixels);
		DECLARE_READ8_MEMBER(pal8_r);
		DECLARE_WRITE8_MEMBER(pal8_w);
		DECLARE_READ8_MEMBER(pal12_r);
		DECLARE_WRITE8_MEMBER(pal12_w);
		DECLARE_READ8_MEMBER(overlay_r);
		DECLARE_WRITE8_MEMBER(overlay_w);
		DECLARE_READ8_MEMBER(bank_r);
		DECLARE_WRITE8_MEMBER(bank_w);
		DECLARE_READ8_MEMBER(ctrl_r);
		DECLARE_WRITE8_MEMBER(ctrl_w);
		DECLARE_READ8_MEMBER(read8);
		DECLARE_WRITE8_MEMBER(write8);

		uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		required_device<upd7220_device> m_upd7220;
		required_device<palette_device> m_palette;
		std::vector<uint8_t> m_ram;
		std::vector<uint8_t> m_overlay;

		uint8_t m_bank;
		uint8_t m_mode;
		bool m_1024;
};

// device type definition
extern const device_type ISA8_NUM_9_REV;

#endif  /* __NUM9REV_H__ */
