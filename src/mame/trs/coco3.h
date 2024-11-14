// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.h

    TRS-80 Radio Shack Color Computer 3 Family

***************************************************************************/

#ifndef MAME_TRS_COCO3_H
#define MAME_TRS_COCO3_H

#pragma once

#include "coco12.h"
#include "gime.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class coco3_state : public coco_state
{
public:
	coco3_state(const machine_config &mconfig, device_type type, const char *tag)
		: coco_state(mconfig, type, tag)
		, m_gime(*this, "gime")
		, m_screen_config(*this, "screen_config")
	{
	}

	virtual void ff20_write(offs_t offset, uint8_t data) override;
	virtual uint8_t ff40_read(offs_t offset) override;
	virtual void ff40_write(offs_t offset, uint8_t data) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void coco3p(machine_config &config);
	void coco3h(machine_config &config);
	void coco3dw1(machine_config &config);
	void coco3(machine_config &config);
	void coco3_mem(address_map &map) ATTR_COLD;

protected:
	virtual void update_cart_base(uint8_t *cart_base) override;

	// miscellaneous
	virtual void update_keyboard_input(uint8_t value) override;
	virtual void cart_w(bool line) override;

	uint8_t m_pia1b_control_register = 0U;

private:
	required_device<gime_device> m_gime;
	required_ioport m_screen_config;
};

#endif // MAME_TRS_COCO3_H
