// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "audio/decobsmt.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "video/deco16ic.h"
#include "machine/deco_irq.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/lc7535.h"
#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "machine/deco146.h"
#include "machine/deco104.h"
#include "video/deco_zoomspr.h"
#include "screen.h"

class deco32_state : public driver_device
{
public:
	deco32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco_irq(*this, "irq"),
		m_deco146(*this, "ioprot"),
		m_deco104(*this, "ioprot104"),
		m_decobsmt(*this, "decobsmt"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_sprgenzoom(*this, "spritegen_zoom"),
		m_eeprom(*this, "eeprom"),
		m_ym2151(*this, "ymsnd"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram"),
		m_pf1_rowscroll32(*this, "pf1_rowscroll32"),
		m_pf2_rowscroll32(*this, "pf2_rowscroll32"),
		m_pf3_rowscroll32(*this, "pf3_rowscroll32"),
		m_pf4_rowscroll32(*this, "pf4_rowscroll32"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_ace_ram(*this, "ace_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<deco_irq_device> m_deco_irq;
	optional_device<deco146_device> m_deco146;
	optional_device<deco104_device> m_deco104;
	optional_device<decobsmt_device> m_decobsmt;
	optional_device<buffered_spriteram32_device> m_spriteram;
	optional_device<decospr_device> m_sprgen;
	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;
	optional_device<deco_zoomspr_device> m_sprgenzoom;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<ym2151_device> m_ym2151;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint32_t> m_ram;
	// we use the pointers below to store a 32-bit copy..
	required_shared_ptr<uint32_t> m_pf1_rowscroll32;
	required_shared_ptr<uint32_t> m_pf2_rowscroll32;
	required_shared_ptr<uint32_t> m_pf3_rowscroll32;
	required_shared_ptr<uint32_t> m_pf4_rowscroll32;
	optional_shared_ptr<uint32_t> m_generic_paletteram_32;
	optional_shared_ptr<uint32_t> m_ace_ram;

	uint8_t m_nslasher_sound_irq; // nslasher and lockload
	int m_tattass_eprom_bit; // tattass
	int m_lastClock; // tattass
	char m_buffer[32]; // tattass
	int m_bufPtr; // tattass
	int m_pendingCommand; // tattass
	int m_readBitCount; // tattass
	int m_byteAddr; // tattass
	int m_ace_ram_dirty; // nslasher and tattass
	int m_has_ace_ram; // all - config
	std::unique_ptr<uint8_t[]> m_dirty_palette; // all but captaven
	int m_pri; // captaven, fghthist, nslasher and tattass
	std::unique_ptr<bitmap_ind16> m_tilemap_alpha_bitmap; // nslasher
	uint16_t m_spriteram16[0x1000]; // captaven, fghthist, nslasher and tattass
	uint16_t m_spriteram16_buffered[0x1000]; // captaven, fghthist, nslasher and tattass
	uint16_t m_spriteram16_2[0x1000]; //nslasher and tattass
	uint16_t m_spriteram16_2_buffered[0x1000]; //nslasher and tattass
	uint16_t    m_pf1_rowscroll[0x1000]; // common
	uint16_t    m_pf2_rowscroll[0x1000]; // common
	uint16_t    m_pf3_rowscroll[0x1000]; // common
	uint16_t    m_pf4_rowscroll[0x1000]; // common

	// common
	DECLARE_READ16_MEMBER(deco_104_r);
	DECLARE_WRITE16_MEMBER(deco_104_w);
	DECLARE_READ16_MEMBER(deco_146_r);
	DECLARE_WRITE16_MEMBER(deco_146_w);
	DECLARE_READ8_MEMBER(eeprom_r);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_WRITE8_MEMBER(volume_w);
	DECLARE_WRITE32_MEMBER(vblank_ack_w);

	DECLARE_WRITE32_MEMBER(pf1_rowscroll_w);
	DECLARE_WRITE32_MEMBER(pf2_rowscroll_w);
	DECLARE_WRITE32_MEMBER(pf3_rowscroll_w);
	DECLARE_WRITE32_MEMBER(pf4_rowscroll_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);

	// captaven
	DECLARE_READ32_MEMBER(_71_r);
	DECLARE_READ8_MEMBER(captaven_dsw1_r);
	DECLARE_READ8_MEMBER(captaven_dsw2_r);
	DECLARE_READ8_MEMBER(captaven_dsw3_r);
	DECLARE_READ8_MEMBER(captaven_soundcpu_status_r);

	// fghthist
	DECLARE_WRITE32_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(fghthist_in0_r);
	DECLARE_READ16_MEMBER(fghthist_in1_r);

	// tattass
	DECLARE_WRITE32_MEMBER(tattass_control_w);

	// nslasher and lockload
	DECLARE_WRITE_LINE_MEMBER(sound_irq_nslasher);
	DECLARE_READ8_MEMBER(latch_r);

	// nslasher and tattass
	DECLARE_READ16_MEMBER(nslasher_debug_r);
	DECLARE_READ32_MEMBER(spriteram2_r);
	DECLARE_WRITE32_MEMBER(spriteram2_w);
	DECLARE_WRITE32_MEMBER(buffer_spriteram2_w);
	DECLARE_WRITE32_MEMBER(ace_ram_w);

	// captaven, fghthist, nslasher and tattass
	DECLARE_READ32_MEMBER(spriteram_r);
	DECLARE_WRITE32_MEMBER(spriteram_w);
	DECLARE_WRITE32_MEMBER(buffer_spriteram_w);
	DECLARE_WRITE32_MEMBER(pri_w);

	// all but captaven
	DECLARE_WRITE32_MEMBER(buffered_palette_w);
	DECLARE_WRITE32_MEMBER(palette_dma_w);

	DECLARE_DRIVER_INIT(tattass);
	DECLARE_DRIVER_INIT(nslasher);
	DECLARE_DRIVER_INIT(captaven);
	DECLARE_DRIVER_INIT(fghthist);
	DECLARE_VIDEO_START(captaven);
	DECLARE_VIDEO_START(fghthist);
	DECLARE_VIDEO_START(nslasher);

	uint32_t screen_update_captaven(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fghthist(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nslasher(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void updateAceRam();
	void mixDualAlphaSprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap);

	void deco32_sound_cb( address_space &space, uint16_t data, uint16_t mem_mask );

	void nslasher_sound_cb( address_space &space, uint16_t data, uint16_t mem_mask );
	DECLARE_READ16_MEMBER(port_b_tattass);
	void tattass_sound_cb( address_space &space, uint16_t data, uint16_t mem_mask );

	DECO16IC_BANK_CB_MEMBER(fghthist_bank_callback);
	DECO16IC_BANK_CB_MEMBER(captaven_bank_callback);
	DECO16IC_BANK_CB_MEMBER(tattass_bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(captaven_pri_callback);

protected:
	virtual void video_start() override;

};

class dragngun_state : public deco32_state
{
public:
	dragngun_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag),
		m_sprite_layout_0_ram(*this, "lay0"),
		m_sprite_layout_1_ram(*this, "lay1"),
		m_sprite_lookup_0_ram(*this, "look0"),
		m_sprite_lookup_1_ram(*this, "look1"),
		m_oki3(*this, "oki3"),
		m_vol_main(*this, "vol_main"),
		m_vol_gun(*this, "vol_gun"),
		m_gun_speaker_disabled(true)
	{ }

	required_shared_ptr<uint32_t> m_sprite_layout_0_ram;
	required_shared_ptr<uint32_t> m_sprite_layout_1_ram;
	required_shared_ptr<uint32_t> m_sprite_lookup_0_ram;
	required_shared_ptr<uint32_t> m_sprite_lookup_1_ram;
	optional_device<okim6295_device> m_oki3;
	required_device<lc7535_device> m_vol_main;
	required_device<lc7535_device> m_vol_gun;

	uint32_t m_sprite_ctrl;
	int m_lightgun_port;
	bitmap_rgb32 m_temp_render_bitmap;

	DECLARE_READ32_MEMBER(lightgun_r);
	DECLARE_WRITE32_MEMBER(lightgun_w);
	DECLARE_WRITE32_MEMBER(sprite_control_w);
	DECLARE_WRITE32_MEMBER(spriteram_dma_w);
	DECLARE_WRITE32_MEMBER(gun_irq_ack_w);
	DECLARE_READ32_MEMBER(unk_video_r);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ32_MEMBER(lockload_gun_mirror_r);

	DECLARE_WRITE32_MEMBER(volume_w);
	DECLARE_WRITE32_MEMBER(speaker_switch_w);
	LC7535_VOLUME_CHANGED(volume_main_changed);
	LC7535_VOLUME_CHANGED(volume_gun_changed);

	virtual void video_start() override;
	DECLARE_DRIVER_INIT(dragngun);
	DECLARE_DRIVER_INIT(dragngunj);
	DECLARE_DRIVER_INIT(lockload);
	DECLARE_VIDEO_START(dragngun);
	DECLARE_VIDEO_START(lockload);
	void dragngun_init_common();
	DECLARE_INPUT_CHANGED_MEMBER(lockload_gun_trigger);

	uint32_t screen_update_dragngun(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_1_callback);
	DECO16IC_BANK_CB_MEMBER(bank_2_callback);

private:
	bool m_gun_speaker_disabled;
};
