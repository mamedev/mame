// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_POWERINS_H
#define MAME_INCLUDES_POWERINS_H

#include "nmk16.h"
#include "nmk16spr.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class powerins_state : public nmk16_state
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag) :
		nmk16_state(mconfig, type, tag)
	{ }

	void powerins(machine_config &config);
	void powerinsa(machine_config &config);
	void powerinsb(machine_config &config);
	void powerinsc(machine_config &config);

private:
	void powerinsa_okibank_w(u8 data);
	u8 powerinsb_fake_ym2203_r();

	DECLARE_MACHINE_START(powerinsa);

	TILE_GET_INFO_MEMBER(powerins_get_bg_tile_info);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_powerinsa);

	virtual void video_start() override;

	void get_colour_6bit(u32 &colour, u32 &pri_mask);
	void get_flip_extcode(u16 attr, int &flipx, int &flipy, int &code);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void powerins_map(address_map &map);
	void powerins_sound_map(address_map &map);
	void powerinsa_map(address_map &map);
	void powerinsa_oki_map(address_map &map);
	void powerinsb_sound_io_map(address_map &map);
};

#endif // MAME_INCLUDES_POWERINS_H
