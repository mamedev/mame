// license:BSD-3-Clause
// copyright-holders:David Haywood, MetalliC
/* Multifish */


#include "emu.h"
#include "sound/ay8910.h"
#include "cpu/z80/z80.h"
#include "machine/timekpr.h"

#define igrosoft_gamble_ROM_SIZE 0x80000
#define igrosoft_gamble_VIDRAM_SIZE (0x2000*0x04)

class igrosoft_gamble_state : public driver_device
{
public:
	igrosoft_gamble_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_m48t35(*this, "m48t35" ),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{
	}

	/* Video related */

	int m_disp_enable;
	int m_xor_paltype;
	int m_xor_palette;

	tilemap_t *m_tilemap;
	tilemap_t *m_reel_tilemap;

	/* Misc related */

	UINT8 m_rambk;

	UINT8 m_hopper_motor;
	UINT8 m_hopper;

	UINT8 m_vid[igrosoft_gamble_VIDRAM_SIZE];
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_vid_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_bank_w);
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_rambank_w);
	DECLARE_READ8_MEMBER(ray_r);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_hopper_w);
	DECLARE_WRITE8_MEMBER(rollfr_hopper_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_lamps1_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_lamps2_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_lamps3_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_counters_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_f3_w);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_dispenable_w);
	DECLARE_CUSTOM_INPUT_MEMBER(igrosoft_gamble_hopper_r);
	DECLARE_READ8_MEMBER(igrosoft_gamble_timekeeper_r);
	DECLARE_WRITE8_MEMBER(igrosoft_gamble_timekeeper_w);
	DECLARE_DRIVER_INIT(customl);
	DECLARE_DRIVER_INIT(island2l);
	DECLARE_DRIVER_INIT(keksl);
	DECLARE_DRIVER_INIT(pirate2l);
	DECLARE_DRIVER_INIT(fcockt2l);
	DECLARE_DRIVER_INIT(sweetl2l);
	DECLARE_DRIVER_INIT(gnomel);
	DECLARE_DRIVER_INIT(crzmonent);
	DECLARE_DRIVER_INIT(fcocktent);
	DECLARE_DRIVER_INIT(garageent);
	DECLARE_DRIVER_INIT(rclimbent);
	DECLARE_DRIVER_INIT(sweetl2ent);
	DECLARE_DRIVER_INIT(resdntent);
	DECLARE_DRIVER_INIT(island2ent);
	DECLARE_DRIVER_INIT(pirate2ent);
	DECLARE_DRIVER_INIT(keksent);
	DECLARE_DRIVER_INIT(gnomeent);
	DECLARE_DRIVER_INIT(lhauntent);
	DECLARE_DRIVER_INIT(fcockt2ent);
	DECLARE_DRIVER_INIT(crzmon2);
	DECLARE_DRIVER_INIT(crzmon2lot);
	DECLARE_DRIVER_INIT(crzmon2ent);
	TILE_GET_INFO_MEMBER(get_igrosoft_gamble_tile_info);
	TILE_GET_INFO_MEMBER(get_igrosoft_gamble_reel_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_igrosoft_gamble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<timekeeper_device> m_m48t35;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

#define mfish_parent mfish_13
#define czmon_parent czmon_13
#define fcockt_parent fcockt_8
#define lhaunt_parent lhaunt_6
#define rollfr_parent rollfr_4
#define garage_parent garage_5
#define rclimb_parent rclimb_3
#define sweetl_parent sweetl
#define resdnt_parent resdnt_6
#define island_parent island
#define pirate_parent pirate_3
#define island2_parent island2
#define pirate2_parent pirate2
#define keks_parent keks_2
#define gnome_parent gnome_9
#define sweetl2_parent sweetl2
#define fcockt2_parent fcockt2
#define crzmon2_parent crzmon2

MACHINE_CONFIG_EXTERN( igrosoft_gamble );
MACHINE_CONFIG_EXTERN( rollfr );
INPUT_PORTS_EXTERN( igrosoft_gamble );
