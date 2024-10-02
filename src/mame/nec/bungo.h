// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/************************************************
 *
 * NEC 文豪 / "Bungo" Word Processors (laptop)
 *
 ***********************************************/

#ifndef MAME_NEC_BUNGO_H
#define MAME_NEC_BUNGO_H

#pragma once

#include "pc9801.h"

class bungo_mini5sx_state : public pc98_base_state
{
public:
	bungo_mini5sx_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc98_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "upd765")
		, m_gvram(*this, "gvram")
	{
	}

	void mini5sx_config(machine_config &config);

protected:
	void mini5sx_map(address_map &map) ATTR_COLD;
	void mini5sx_io(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
private:
	required_device<v33_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<uint16_t> m_gvram;

	u16 fake_dict_r(offs_t offset);

	void bungo_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

#endif // MAME_NEC_BUNGO_H
