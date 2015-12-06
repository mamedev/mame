// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.h

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#pragma once

#ifndef __COCO3__
#define __COCO3__


#include "includes/coco.h"
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
	: coco_state(mconfig, type, tag),
		m_gime(*this, GIME_TAG) { }

	required_device<gime_base_device> m_gime;

	virtual DECLARE_WRITE8_MEMBER( ff20_write ) override;
	virtual DECLARE_READ8_MEMBER( ff40_read ) override;
	virtual DECLARE_WRITE8_MEMBER( ff40_write ) override;

	DECLARE_WRITE_LINE_MEMBER(gime_firq_w) { recalculate_firq(); }
	DECLARE_WRITE_LINE_MEMBER(gime_irq_w) { recalculate_irq(); }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void update_cart_base(UINT8 *cart_base) override;

	/* interrupts */
	virtual bool firq_get_line(void) override;
	virtual bool irq_get_line(void) override;

	/* miscellaneous */
	virtual void update_keyboard_input(UINT8 value, UINT8 z) override;
	virtual void cart_w(bool line) override;
};

#endif // __COCO3__
