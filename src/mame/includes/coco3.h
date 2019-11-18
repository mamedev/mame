// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.h

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_COCO3_H
#define MAME_INCLUDES_COCO3_H


#include "includes/coco12.h"
#include "video/gime.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define GIME_TAG                "gime"
#define VDG_TAG                 "vdg"
#define COMPOSITE_SCREEN_TAG    "composite"
#define RGB_SCREEN_TAG          "rgb"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class coco3_state : public coco_state
{
public:
	coco3_state(const machine_config &mconfig, device_type type, const char *tag)
		: coco_state(mconfig, type, tag)
		, m_gime(*this, GIME_TAG) { }

	virtual DECLARE_WRITE8_MEMBER( ff20_write ) override;
	virtual DECLARE_READ8_MEMBER( ff40_read ) override;
	virtual DECLARE_WRITE8_MEMBER( ff40_write ) override;

	DECLARE_WRITE_LINE_MEMBER(gime_firq_w) { recalculate_firq(); }
	DECLARE_WRITE_LINE_MEMBER(gime_irq_w) { recalculate_irq(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void coco3p(machine_config &config);
	void coco3h(machine_config &config);
	void coco3dw1(machine_config &config);
	void coco3(machine_config &config);
	void coco3_mem(address_map &map);
protected:
	virtual void update_cart_base(uint8_t *cart_base) override;

	// interrupts
	virtual bool firq_get_line(void) override;
	virtual bool irq_get_line(void) override;

	// miscellaneous
	virtual void update_keyboard_input(uint8_t value) override;
	virtual void cart_w(bool line) override;

private:
	required_device<gime_device> m_gime;
};

#endif // MAME_INCLUDES_COCO3_H
