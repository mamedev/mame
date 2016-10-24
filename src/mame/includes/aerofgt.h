// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "machine/gen_latch.h"
#include "video/vsystem_spr.h"
#include "video/vsystem_spr2.h"
#include "sound/okim6295.h"
#include "sound/upd7759.h"

class aerofgt_state : public driver_device
{
public:
	aerofgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_rasterram(*this, "rasterram"),
		m_bitmapram(*this, "bitmapram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram3(*this, "spriteram3"),
		m_tx_tilemap_ram(*this, "tx_tilemap_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spr(*this, "vsystem_spr"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_spr_old2(*this, "vsystem_spr_ol2"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg1videoram;
	optional_shared_ptr<uint16_t> m_bg2videoram;
	optional_shared_ptr<uint16_t> m_rasterram;
	optional_shared_ptr<uint16_t> m_bitmapram;
	optional_shared_ptr<uint16_t> m_spriteram1;
	optional_shared_ptr<uint16_t> m_spriteram2;
	required_shared_ptr<uint16_t> m_spriteram3;
	optional_shared_ptr<uint16_t> m_tx_tilemap_ram;

	/* devices referenced above */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<upd7759_device> m_upd7759; // karatblzbl
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<vsystem_spr_device> m_spr; // only the aerofgt parent uses this chip
	optional_device<vsystem_spr2_device> m_spr_old; // every other (non-bootleg) uses this
	optional_device<vsystem_spr2_device> m_spr_old2; //  or a pair of them..
	optional_device<generic_latch_8_device> m_soundlatch;



	/* video-related */
	tilemap_t   *m_bg1_tilemap;
	tilemap_t   *m_bg2_tilemap;
	uint8_t     m_gfxbank[8];
	uint16_t    m_bank[4];
	uint16_t    m_bg1scrollx;
	uint16_t    m_bg1scrolly;
	uint16_t    m_bg2scrollx;
	uint16_t    m_bg2scrolly;
	uint16_t    m_wbbc97_bitmap_enable;
	int       m_charpalettebank;
	int       m_spritepalettebank;
	int       m_sprite_gfx;
	int       m_spikes91_lookup;
	uint32_t aerofgt_tile_callback( uint32_t code );

	uint32_t aerofgt_old_tile_callback( uint32_t code );
	uint32_t aerofgt_ol2_tile_callback( uint32_t code );

	/* misc */
	int       m_pending_command;

	/* handlers */
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void turbofrc_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerfboot_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pending_command_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pending_command_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aerofgt_sh_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aerfboot_okim6295_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aerofgt_bg1videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerofgt_bg2videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pspikes_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pspikesb_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spikes91_lookup_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void karatblz_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spinlbrk_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void turbofrc_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerofgt_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerofgt_bg1scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerofgt_bg1scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerofgt_bg2scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerofgt_bg2scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pspikes_palette_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wbbc97_bitmap_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pspikesb_oki_banking_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aerfboo2_okim6295_banking_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void karatblzbl_d7759_write_port_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void karatblzbl_d7759_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_pspikes_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void karatblz_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void karatblz_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void spinlbrk_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_aerofgt();
	void machine_reset_aerofgt();
	void video_start_pspikes();
	void machine_start_common();
	void machine_reset_common();
	void video_start_karatblz();
	void video_start_spinlbrk();
	void video_start_turbofrc();
	void video_start_wbbc97();
	void init_banked_oki();
	uint32_t screen_update_pspikes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spikes91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pspikesb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_karatblz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spinlbrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_turbofrc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aerofgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aerfboot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aerfboo2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wbbc97(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void aerofgt_register_state_globals(  );
	void setbank( tilemap_t *tmap, int num, int bank );
	void aerfboo2_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri );
	void pspikesb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void spikes91_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void aerfboot_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void wbbc97_draw_bitmap( bitmap_rgb32 &bitmap );
};
