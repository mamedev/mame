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
		m_soundlatch(*this, "soundlatch"),
		m_soundbank(*this, "soundbank"),
		m_okibank(*this, "okibank") { }

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

	optional_memory_bank m_soundbank;
	optional_memory_bank m_okibank;

	/* video-related */
	tilemap_t   *m_bg1_tilemap;
	tilemap_t   *m_bg2_tilemap;
	uint8_t     m_gfxbank[8];
	uint16_t    m_bank[4];
	uint16_t    m_bg1scrollx;
	uint16_t    m_bg1scrolly;
	uint16_t    m_bg2scrollx;
	uint16_t    m_bg2scrolly;
	bool        m_flip_screen;
	uint16_t    m_wbbc97_bitmap_enable;
	int       m_charpalettebank;
	int       m_spritepalettebank;
	int       m_sprite_gfx;
	int       m_spikes91_lookup;
	uint32_t aerofgt_tile_callback( uint32_t code );

	uint32_t aerofgt_old_tile_callback( uint32_t code );
	uint32_t aerofgt_ol2_tile_callback( uint32_t code );

	/* handlers */
	DECLARE_WRITE8_MEMBER(karatblzbl_soundlatch_w);
	DECLARE_READ8_MEMBER(pending_command_r);
	DECLARE_WRITE8_MEMBER(aerofgt_sh_bankswitch_w);
	DECLARE_WRITE8_MEMBER(spinlbrk_sh_bankswitch_w);
	DECLARE_WRITE8_MEMBER(aerfboot_okim6295_banking_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1videoram_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(pspikes_gfxbank_w);
	DECLARE_WRITE16_MEMBER(pspikesb_gfxbank_w);
	DECLARE_WRITE16_MEMBER(spikes91_lookup_w);
	DECLARE_WRITE8_MEMBER(karatblz_gfxbank_w);
	DECLARE_WRITE8_MEMBER(spinlbrk_gfxbank_w);
	DECLARE_WRITE8_MEMBER(kickball_gfxbank_w);
	DECLARE_WRITE16_MEMBER(turbofrc_gfxbank_w);
	DECLARE_WRITE16_MEMBER(aerofgt_gfxbank_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1scrollx_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1scrolly_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2scrollx_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2scrolly_w);
	DECLARE_WRITE8_MEMBER(pspikes_palette_bank_w);
	DECLARE_WRITE8_MEMBER(spinlbrk_flip_screen_w);
	DECLARE_WRITE8_MEMBER(turbofrc_flip_screen_w);
	DECLARE_WRITE16_MEMBER(wbbc97_bitmap_enable_w);
	DECLARE_WRITE16_MEMBER(pspikesb_oki_banking_w);
	DECLARE_WRITE16_MEMBER(aerfboo2_okim6295_banking_w);
	DECLARE_WRITE8_MEMBER(karatblzbl_d7759_write_port_0_w);
	DECLARE_WRITE8_MEMBER(karatblzbl_d7759_reset_w);
	TILE_GET_INFO_MEMBER(get_pspikes_tile_info);
	TILE_GET_INFO_MEMBER(karatblz_bg1_tile_info);
	TILE_GET_INFO_MEMBER(karatblz_bg2_tile_info);
	TILE_GET_INFO_MEMBER(spinlbrk_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	DECLARE_MACHINE_START(aerofgt);
	DECLARE_MACHINE_START(spinlbrk);
	DECLARE_MACHINE_RESET(aerofgt);
	DECLARE_VIDEO_START(pspikes);
	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_RESET(common);
	DECLARE_VIDEO_START(karatblz);
	DECLARE_VIDEO_START(spinlbrk);
	DECLARE_VIDEO_START(turbofrc);
	DECLARE_VIDEO_START(wbbc97);
	DECLARE_DRIVER_INIT(banked_oki);
	DECLARE_DRIVER_INIT(kickball);
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
	void spinlbrk(machine_config &config);
	void aerofgt(machine_config &config);
	void karatblz(machine_config &config);
	void pspikesb(machine_config &config);
	void aerfboo2(machine_config &config);
	void pspikes(machine_config &config);
	void wbbc97(machine_config &config);
	void aerfboot(machine_config &config);
	void pspikesc(machine_config &config);
	void karatblzbl(machine_config &config);
	void spikes91(machine_config &config);
	void aerofgtb(machine_config &config);
	void turbofrc(machine_config &config);
	void kickball(machine_config &config);
	void aerfboo2_map(address_map &map);
	void aerfboot_map(address_map &map);
	void aerfboot_sound_map(address_map &map);
	void aerofgt_map(address_map &map);
	void aerofgt_sound_portmap(address_map &map);
	void aerofgtb_map(address_map &map);
	void karatblz_map(address_map &map);
	void karatblzbl_map(address_map &map);
	void karatblzbl_sound_map(address_map &map);
	void karatblzbl_sound_portmap(address_map &map);
	void kickball_map(address_map &map);
	void kickball_sound_map(address_map &map);
	void kickball_sound_portmap(address_map &map);
	void oki_map(address_map &map);
	void pspikes_map(address_map &map);
	void pspikesb_map(address_map &map);
	void pspikesc_map(address_map &map);
	void sound_map(address_map &map);
	void spikes91_map(address_map &map);
	void spinlbrk_map(address_map &map);
	void spinlbrk_sound_portmap(address_map &map);
	void turbofrc_map(address_map &map);
	void turbofrc_sound_portmap(address_map &map);
	void wbbc97_map(address_map &map);
	void wbbc97_sound_map(address_map &map);
};
