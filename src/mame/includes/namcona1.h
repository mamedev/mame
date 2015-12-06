// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco NA-1 System hardware

***************************************************************************/

#include "machine/eeprompar.h"
#include "sound/c140.h"

enum
{
	NAMCO_CGANGPZL,
	NAMCO_EMERALDA,
	NAMCO_KNCKHEAD,
	NAMCO_BKRTMAQ,
	NAMCO_EXBANIA,
	NAMCO_QUIZTOU,
	NAMCO_SWCOURT,
	NAMCO_TINKLPIT,
	NAMCO_NUMANATH,
	NAMCO_FA,
	NAMCO_XDAY2
};


class namcona1_state : public driver_device
{
public:
	namcona1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_c140(*this, "c140"),
		m_muxed_inputs(*this, muxed_inputs),
		m_io_p3(*this, "P3"),
		m_workram(*this,"workram"),
		m_vreg(*this,"vreg"),
		m_paletteram(*this, "paletteram"),
		m_cgram(*this, "cgram"),
		m_videoram(*this,"videoram"),
		m_scroll(*this,"scroll"),
		m_spriteram(*this,"spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<c140_device> m_c140;

	DECLARE_IOPORT_ARRAY(muxed_inputs);
	required_ioport_array<4> m_muxed_inputs;
	required_ioport          m_io_p3;

	required_shared_ptr<UINT16> m_workram;
	required_shared_ptr<UINT16> m_vreg;
	required_shared_ptr<UINT16> m_paletteram;
	required_shared_ptr<UINT16> m_cgram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_scroll;
	required_shared_ptr<UINT16> m_spriteram;

	// this has to be UINT8 to be in the right byte order for the tilemap system
	std::vector<UINT8> m_shaperam;

	UINT16 *m_prgrom;
	UINT16 *m_maskrom;
	int m_mEnableInterrupts;
	int m_gametype;
	UINT16 m_count;
	UINT32 m_keyval;
	UINT16 m_mcu_mailbox[8];
	UINT8 m_mcu_port4;
	UINT8 m_mcu_port5;
	UINT8 m_mcu_port6;
	UINT8 m_mcu_port8;
	tilemap_t *m_bg_tilemap[4+1];
	int m_palette_is_dirty;

	DECLARE_READ16_MEMBER(custom_key_r);
	DECLARE_WRITE16_MEMBER(custom_key_w);
	DECLARE_WRITE16_MEMBER(vreg_w);
	DECLARE_READ16_MEMBER(mcu_mailbox_r);
	DECLARE_WRITE16_MEMBER(mcu_mailbox_w_68k);
	DECLARE_WRITE16_MEMBER(mcu_mailbox_w_mcu);
	DECLARE_READ16_MEMBER(na1mcu_shared_r);
	DECLARE_WRITE16_MEMBER(na1mcu_shared_w);
	DECLARE_READ8_MEMBER(port4_r);
	DECLARE_WRITE8_MEMBER(port4_w);
	DECLARE_READ8_MEMBER(port5_r);
	DECLARE_WRITE8_MEMBER(port5_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(port7_r);
	DECLARE_WRITE8_MEMBER(port7_w);
	DECLARE_READ8_MEMBER(port8_r);
	DECLARE_WRITE8_MEMBER(port8_w);
	DECLARE_READ8_MEMBER(portana_r);
	void simulate_mcu();
	void write_version_info();
	int transfer_dword(UINT32 dest, UINT32 source);
	void blit();
	void UpdatePalette(int offset);
	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(paletteram_w);
	DECLARE_READ16_MEMBER(gfxram_r);
	DECLARE_WRITE16_MEMBER(gfxram_w);
	void pdraw_tile( screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, UINT32 code, int color,
		int sx, int sy, int flipx, int flipy, int priority, int bShadow, int bOpaque, int gfx_region );
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask );
	DECLARE_READ16_MEMBER(snd_r);
	DECLARE_WRITE16_MEMBER(snd_w);

	DECLARE_DRIVER_INIT(bkrtmaq);
	DECLARE_DRIVER_INIT(quiztou);
	DECLARE_DRIVER_INIT(emeralda);
	DECLARE_DRIVER_INIT(numanath);
	DECLARE_DRIVER_INIT(fa);
	DECLARE_DRIVER_INIT(cgangpzl);
	DECLARE_DRIVER_INIT(tinklpit);
	DECLARE_DRIVER_INIT(swcourt);
	DECLARE_DRIVER_INIT(knckhead);
	DECLARE_DRIVER_INIT(xday2);
	DECLARE_DRIVER_INIT(exbania);
	DECLARE_DRIVER_INIT(emeraldj);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(tilemap_get_info0);
	TILE_GET_INFO_MEMBER(tilemap_get_info1);
	TILE_GET_INFO_MEMBER(tilemap_get_info2);
	TILE_GET_INFO_MEMBER(tilemap_get_info3);
	TILE_GET_INFO_MEMBER(roz_get_info);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void postload();

private:
	void tilemap_get_info(tile_data &tileinfo, int tile_index, const UINT16 *tilemap_videoram, bool use_4bpp_gfx);
};
