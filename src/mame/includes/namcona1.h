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

#define NAMCONA1_NUM_TILEMAPS 4


class namcona1_state : public driver_device
{
public:
	namcona1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_c140(*this, "c140"),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram"),
		m_workram(*this,"workram"),
		m_vreg(*this,"vreg"),
		m_scroll(*this,"scroll")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<eeprom_parallel_28xx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<c140_device> m_c140;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_workram;
	required_shared_ptr<UINT16> m_vreg;
	required_shared_ptr<UINT16> m_scroll;

	UINT16 *m_mpBank0;
	UINT16 *m_mpBank1;
	int m_mEnableInterrupts;
	int m_gametype;
	UINT16 m_count;
	UINT32 m_keyval;
	UINT16 m_mcu_mailbox[8];
	UINT8 m_mcu_port4;
	UINT8 m_mcu_port5;
	UINT8 m_mcu_port6;
	UINT8 m_mcu_port8;
	UINT16 *m_shaperam;
	UINT16 *m_cgram;
	tilemap_t *m_roz_tilemap;
	int m_roz_palette;
	tilemap_t *m_bg_tilemap[NAMCONA1_NUM_TILEMAPS];
	int m_tilemap_palette_bank[NAMCONA1_NUM_TILEMAPS];
	int m_palette_is_dirty;
	UINT8 m_mask_data[8];
	UINT8 m_conv_data[9];


	DECLARE_READ16_MEMBER(custom_key_r);
	DECLARE_WRITE16_MEMBER(custom_key_w);
	DECLARE_READ16_MEMBER(namcona1_vreg_r);
	DECLARE_WRITE16_MEMBER(namcona1_vreg_w);
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
	void namcona1_blit();
	void init_namcona1(int gametype);
	void UpdatePalette(int offset);
	DECLARE_WRITE16_MEMBER(namcona1_videoram_w);
	DECLARE_READ16_MEMBER(namcona1_videoram_r);
	DECLARE_READ16_MEMBER(namcona1_paletteram_r);
	DECLARE_WRITE16_MEMBER(namcona1_paletteram_w);
	DECLARE_READ16_MEMBER(namcona1_gfxram_r);
	DECLARE_WRITE16_MEMBER(namcona1_gfxram_w);
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
	TILE_GET_INFO_MEMBER(tilemap_get_info0);
	TILE_GET_INFO_MEMBER(tilemap_get_info1);
	TILE_GET_INFO_MEMBER(tilemap_get_info2);
	TILE_GET_INFO_MEMBER(tilemap_get_info3);
	TILE_GET_INFO_MEMBER(roz_get_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_namcona1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(namcona1_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_interrupt);

private:
	void tilemap_get_info(tile_data &tileinfo, int tile_index, const UINT16 *tilemap_videoram, int tilemap_color, bool use_4bpp_gfx);
};
