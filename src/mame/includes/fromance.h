// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi. Bryan McPhail, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/
#include "sound/msm5205.h"
#include "video/vsystem_spr2.h"

class fromance_state : public driver_device
{
public:
	fromance_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_subcpu(*this, "sub"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers (used by pipedrm) */
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;

	optional_device<vsystem_spr2_device> m_spr_old; // only used by pipe dream, split this state up and clean things...

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	std::unique_ptr<uint8_t[]>   m_local_videoram[2];
	std::unique_ptr<uint8_t[]>  m_local_paletteram;
	uint8_t    m_selected_videoram;
	uint8_t    m_selected_paletteram;
	uint32_t   m_scrollx[2];
	uint32_t   m_scrolly[2];
	uint8_t    m_gfxreg;
	uint8_t    m_flipscreen;
	uint8_t    m_flipscreen_old;
	uint32_t   m_scrolly_ofs;
	uint32_t   m_scrollx_ofs;

	uint8_t    m_crtc_register;
	uint8_t    m_crtc_data[0x10];
	emu_timer *m_crtc_timer;

	/* misc */
	uint8_t    m_directionflag;
	uint8_t    m_commanddata;
	uint8_t    m_portselect;
	uint8_t    m_adpcm_reset;
	uint8_t    m_adpcm_data;
	uint8_t    m_vclk_left;
	uint8_t    m_pending_command;
	uint8_t    m_sound_command;

	/* devices */
	required_device<cpu_device> m_subcpu;
	uint8_t fromance_commanddata_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromance_commanddata_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fromance_busycheck_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t fromance_busycheck_sub_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromance_busycheck_sub_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_portselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fromance_keymatrix_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromance_coinctr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_gfxreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fromance_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromance_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fromance_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromance_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_crtc_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_crtc_register_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromance_adpcm_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_fromance_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fromance_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_nekkyoku_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_nekkyoku_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_fromance();
	void machine_reset_fromance();
	void video_start_nekkyoku();
	void video_start_fromance();
	void video_start_pipedrm();
	void video_start_hatris();
	uint32_t screen_update_fromance(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pipedrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deferred_commanddata_w(void *ptr, int32_t param);
	void crtc_interrupt_gen(void *ptr, int32_t param);
	inline void get_fromance_tile_info( tile_data &tileinfo, int tile_index, int layer );
	inline void get_nekkyoku_tile_info( tile_data &tileinfo, int tile_index, int layer );
	void init_common(  );
	void fromance_adpcm_int(int state);
	required_device<cpu_device> m_maincpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
