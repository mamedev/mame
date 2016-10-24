// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "audio/decobsmt.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "video/deco16ic.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "machine/deco146.h"
#include "machine/deco104.h"
#include "video/deco_zoomspr.h"

class deco32_state : public driver_device
{
public:
	deco32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco146(*this, "ioprot"),
		m_deco104(*this, "ioprot104"),
		m_decobsmt(*this, "decobsmt"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_sprgenzoom(*this, "spritegen_zoom"),
		m_eeprom(*this, "eeprom"),
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
	optional_device<deco146_device> m_deco146;
	optional_device<deco104_device> m_deco104;
	optional_device<decobsmt_device> m_decobsmt;
	optional_device<buffered_spriteram32_device> m_spriteram;
	optional_device<decospr_device> m_sprgen;
	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;
	optional_device<deco_zoomspr_device> m_sprgenzoom;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
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
	required_shared_ptr<uint32_t> m_generic_paletteram_32;
	optional_shared_ptr<uint32_t> m_ace_ram;

	int m_raster_enable; // captaven, dragongun and lockload
	timer_device *m_raster_irq_timer; // captaven, dragongun and lockload
	uint8_t m_nslasher_sound_irq; // nslasher and lockload
	uint8_t m_irq_source; // captaven, dragongun and lockload
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
	void pf1_rowscroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void pf2_rowscroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void pf3_rowscroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void pf4_rowscroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// captaven
	uint32_t _71_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t captaven_soundcpu_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void nonbuffered_palette_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// fghthist
	void sound_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t fghthist_control_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void fghthist_eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t fghthist_protection_region_0_146_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void fghthist_protection_region_0_146_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// nslasher
	void nslasher_eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// tattass
	void tattass_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// nslasher and lockload
	void sound_irq_nslasher(int state);
	uint8_t latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// captaven, dragongun and lockload
	uint32_t irq_controller_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void irq_controller_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// nslasher and tattass
	uint16_t nslasher_protection_region_0_104_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void nslasher_protection_region_0_104_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t nslasher_debug_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t spriteram2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void spriteram2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void buffer_spriteram2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ace_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// captaven, fghthist, nslasher and tattass
	uint32_t spriteram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void spriteram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void buffer_spriteram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void pri_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// all but captaven
	void buffered_palette_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void palette_dma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// captaven, dragongun and lockload
	uint16_t dg_protection_region_0_146_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dg_protection_region_0_146_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	virtual void video_start() override;
	void init_tattass();
	void init_nslasher();
	void init_captaven();
	void init_fghthist();
	void machine_reset_deco32();
	void video_start_captaven();
	void video_start_fghthist();
	void video_start_nslasher();

	void deco32_vbl_interrupt(device_t &device);
	void interrupt_gen(timer_device &timer, void *ptr, int32_t param);

	uint32_t screen_update_captaven(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fghthist(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nslasher(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void updateAceRam();
	void mixDualAlphaSprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap);

	uint16_t port_a_fghthist(int unused);
	uint16_t port_b_fghthist(int unused);
	uint16_t port_c_fghthist(int unused);
	void deco32_sound_cb( address_space &space, uint16_t data, uint16_t mem_mask );

	uint16_t port_b_nslasher(int unused);
	void nslasher_sound_cb( address_space &space, uint16_t data, uint16_t mem_mask );
	uint16_t port_b_tattass(int unused);
	void tattass_sound_cb( address_space &space, uint16_t data, uint16_t mem_mask );

	DECO16IC_BANK_CB_MEMBER(fghthist_bank_callback);
	DECO16IC_BANK_CB_MEMBER(captaven_bank_callback);
	DECO16IC_BANK_CB_MEMBER(tattass_bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(captaven_pri_callback);
};

class dragngun_state : public deco32_state
{
public:
	dragngun_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag),
		m_sprite_layout_0_ram(*this, "lay0"),
		m_sprite_layout_1_ram(*this, "lay1"),
		m_sprite_lookup_0_ram(*this, "look0"),
		m_sprite_lookup_1_ram(*this, "look1")
	{ }

	required_shared_ptr<uint32_t> m_sprite_layout_0_ram;
	required_shared_ptr<uint32_t> m_sprite_layout_1_ram;
	required_shared_ptr<uint32_t> m_sprite_lookup_0_ram;
	required_shared_ptr<uint32_t> m_sprite_lookup_1_ram;

	uint32_t m_sprite_ctrl;
	int m_lightgun_port;
	bitmap_rgb32 m_temp_render_bitmap;

	uint32_t lightgun_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void lightgun_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sprite_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void spriteram_dma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t unk_video_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t service_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t eeprom_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t lockload_gun_mirror_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	virtual void video_start() override;
	void init_dragngun();
	void init_dragngunj();
	void init_lockload();
	void video_start_dragngun();
	void video_start_lockload();
	void dragngun_init_common();

	void lockload_vbl_irq(timer_device &timer, void *ptr, int32_t param);

	uint32_t screen_update_dragngun(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_1_callback);
	DECO16IC_BANK_CB_MEMBER(bank_2_callback);
};
