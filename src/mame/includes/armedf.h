// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Carlos A. Lozano

#include "includes/nb1414m4.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"

class armedf_state : public driver_device
{
public:
	armedf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_extra(*this, "extra"),
		m_nb1414m4(*this, "nb1414m4"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_soundlatch(*this, "soundlatch"),
		m_spr_pal_clut(*this, "spr_pal_clut"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_extra;
	optional_device<nb1414m4_device> m_nb1414m4;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	std::unique_ptr<uint8_t[]> m_text_videoram;
	required_shared_ptr<uint16_t> m_spr_pal_clut;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	uint16_t m_legion_cmd[4]; // legionjb only!

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_tx_tilemap;
	uint16_t   m_scroll_msb;
	uint16_t   m_vreg;
	uint16_t   m_fg_scrollx;
	uint16_t   m_fg_scrolly;
	uint16_t   m_bg_scrollx;
	uint16_t   m_bg_scrolly;
	int      m_scroll_type;
	int      m_sprite_offy;
	int      m_old_mcu_mode;
	int      m_waiting_msb;

	DECLARE_WRITE16_MEMBER(terraf_io_w);
	DECLARE_WRITE16_MEMBER(terrafjb_io_w);
	DECLARE_WRITE16_MEMBER(bootleg_io_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(soundlatch_clear_r);
	DECLARE_WRITE16_MEMBER(irq_lv1_ack_w);
	DECLARE_WRITE16_MEMBER(irq_lv2_ack_w);
	DECLARE_WRITE8_MEMBER(legionjb_fg_scroll_w);
	DECLARE_READ8_MEMBER(blitter_txram_r);
	DECLARE_WRITE8_MEMBER(blitter_txram_w);
	DECLARE_WRITE8_MEMBER(fg_scrollx_w);
	DECLARE_WRITE8_MEMBER(fg_scrolly_w);
	DECLARE_WRITE8_MEMBER(fg_scroll_msb_w);
	DECLARE_READ8_MEMBER(nb1414m4_text_videoram_r);
	DECLARE_WRITE8_MEMBER(nb1414m4_text_videoram_w);
	DECLARE_READ8_MEMBER(armedf_text_videoram_r);
	DECLARE_WRITE8_MEMBER(armedf_text_videoram_w);
	DECLARE_WRITE16_MEMBER(armedf_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(armedf_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(terraf_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(terraf_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(terraf_fg_scroll_msb_arm_w);
	DECLARE_WRITE16_MEMBER(armedf_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(armedf_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(armedf_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(armedf_bg_scrolly_w);
	DECLARE_DRIVER_INIT(cclimbr2);
	DECLARE_DRIVER_INIT(armedf);
	DECLARE_DRIVER_INIT(legion);
	DECLARE_DRIVER_INIT(terrafu);
	DECLARE_DRIVER_INIT(legionjb);
	DECLARE_DRIVER_INIT(kozure);
	DECLARE_DRIVER_INIT(terraf);
	DECLARE_DRIVER_INIT(terrafjb);
	TILEMAP_MAPPER_MEMBER(armedf_scan_type1);
	TILEMAP_MAPPER_MEMBER(armedf_scan_type2);
	TILEMAP_MAPPER_MEMBER(armedf_scan_type3);
	TILE_GET_INFO_MEMBER(get_nb1414m4_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_armedf_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_START(armedf);
	DECLARE_MACHINE_RESET(armedf);
	DECLARE_VIDEO_START(terraf);
	DECLARE_VIDEO_START(armedf);
	uint32_t screen_update_armedf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void armedf_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
						uint32_t code,uint32_t color, uint32_t clut,int flipx,int flipy,int offsx,int offsy,
						int transparent_color);
	void terraf_sound(machine_config &config);
	void terrafb(machine_config &config);
	void legion(machine_config &config);
	void terraf(machine_config &config);
	void legionjb(machine_config &config);
	void cclimbr2(machine_config &config);
	void terrafjb(machine_config &config);
	void armedf(machine_config &config);
	void kozure(machine_config &config);
	void armedf_map(address_map &map);
	void cclimbr2_map(address_map &map);
	void cclimbr2_soundmap(address_map &map);
	void kozure_map(address_map &map);
	void legion_map(address_map &map);
	void legionjb_map(address_map &map);
	void sound_3526_portmap(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
	void terraf_map(address_map &map);
	void terrafjb_extraz80_map(address_map &map);
	void terrafjb_extraz80_portmap(address_map &map);
};

class bigfghtr_state : public armedf_state
{
public:
	bigfghtr_state(const machine_config &mconfig, device_type type, const char *tag)
		: armedf_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_sharedram(*this, "sharedram")
		{ }

	required_device<cpu_device> m_mcu;
	required_shared_ptr<uint8_t> m_sharedram;

	DECLARE_READ16_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(main_sharedram_w);
	DECLARE_READ8_MEMBER(main_sharedram_r);
	DECLARE_WRITE8_MEMBER(mcu_spritelist_w);
	void bigfghtr(machine_config &config);
	void bigfghtr_map(address_map &map);
	void bigfghtr_mcu_io_map(address_map &map);
	void bigfghtr_mcu_map(address_map &map);
};
