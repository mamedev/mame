// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.h

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#ifndef MAME_INCLUDES_COCO3_H
#define MAME_INCLUDES_COCO3_H

#pragma once

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

	virtual void ff20_write(offs_t offset, uint8_t data) override;
	virtual uint8_t ff40_read(offs_t offset) override;
	virtual void ff40_write(offs_t offset, uint8_t data) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void coco3p(machine_config &config);
	void coco3h(machine_config &config);
	void coco3dw1(machine_config &config);
	void coco3(machine_config &config);
	void coco3_mem(address_map &map);
protected:
	virtual void update_cart_base(uint8_t *cart_base) override;

	// miscellaneous
	virtual void update_keyboard_input(uint8_t value) override;
	virtual void cart_w(bool line) override;

private:
	required_device<gime_device> m_gime;
};

#endif // MAME_INCLUDES_COCO3_H
