// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.h

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#pragma once

#ifndef __COCO3__
#define __COCO3__


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
	: coco_state(mconfig, type, tag),
		m_gime(*this, GIME_TAG) { }

	required_device<gime_base_device> m_gime;

	virtual void ff20_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t ff40_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void ff40_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void gime_firq_w(int state) { recalculate_firq(); }
	void gime_irq_w(int state) { recalculate_irq(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void update_cart_base(uint8_t *cart_base) override;

	/* interrupts */
	virtual bool firq_get_line(void) override;
	virtual bool irq_get_line(void) override;

	/* miscellaneous */
	virtual void update_keyboard_input(uint8_t value, uint8_t z) override;
	virtual void cart_w(bool line) override;
};

#endif // __COCO3__
