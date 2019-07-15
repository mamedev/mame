// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "cpu/h6280/h6280.h"
#include "audio/decobsmt.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "video/deco16ic.h"
#include "video/deco_ace.h"
#include "machine/deco_irq.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/lc7535.h"
#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "machine/deco146.h"
#include "machine/deco104.h"
#include "video/deco_zoomspr.h"
#include "emupal.h"
#include "screen.h"

class deco32_state : public driver_device
{
public:
	deco32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_sprgen(*this, "spritegen%u", 1)
		, m_deco_tilegen(*this, "tilegen%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_deco_irq(*this, "irq")
		, m_decobsmt(*this, "decobsmt")
		, m_eeprom(*this, "eeprom")
		, m_ioprot(*this, "ioprot")
		, m_ym2151(*this, "ymsnd")
		, m_oki(*this, "oki%u", 1)
		, m_soundlatch(*this, "soundlatch")
		, m_maincpu(*this, "maincpu")
		, m_pf_rowscroll32(*this, "pf%u_rowscroll32", 1)
		, m_paletteram(*this, "paletteram")
	{ }

	void sound_bankswitch_w(u8 data);

protected:
	void h6280_sound_custom_latch_map(address_map &map);
	void h6280_sound_map(address_map &map);
	void z80_sound_io(address_map &map);
	void z80_sound_map(address_map &map);

	// common
	u16 ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void volume_w(u8 data);
	void vblank_ack_w(u32 data);

	template<int Chip> void pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// captaven, fghthist, nslasher and tattass
	template<int Chip> u32 spriteram_r(offs_t offset);
	template<int Chip> void spriteram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Chip> void buffer_spriteram_w(u32 data);
	void pri_w(u32 data);

	// all but captaven
	void buffered_palette_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palette_dma_w(u32 data);

	optional_device<cpu_device> m_audiocpu;
	optional_device_array<decospr_device, 2> m_sprgen;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<deco_irq_device> m_deco_irq;
	optional_device<decobsmt_device> m_decobsmt;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<deco_146_base_device> m_ioprot;
	optional_device<ym2151_device> m_ym2151;
	optional_device_array<okim6295_device, 3> m_oki;
	optional_device<generic_latch_8_device> m_soundlatch;
	required_device<cpu_device> m_maincpu;

	void allocate_spriteram(int chip);
	void allocate_buffered_palette();
	void allocate_rowscroll(int size1, int size2, int size3, int size4);

	virtual void video_start() override;

	std::unique_ptr<u8[]> m_dirty_palette; // all but captaven
	int m_pri; // all but dragngun
	std::unique_ptr<u16[]> m_spriteram16[2]; // all but dragngun
	std::unique_ptr<u16[]> m_spriteram16_buffered[2]; // all but dragngun
	std::unique_ptr<u16[]> m_pf_rowscroll[4]; // common

private:
	// we use the pointers below to store a 32-bit copy..
	required_shared_ptr_array<u32, 4> m_pf_rowscroll32;
	optional_shared_ptr<u32> m_paletteram;
};

class captaven_state : public deco32_state
{
public:
	captaven_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag)
		, m_io_dsw(*this, "DSW%u", 1U)
	{ }

	void captaven(machine_config &config);

	void init_captaven();

private:
	required_ioport_array<3> m_io_dsw;
	u32 _71_r();
	u8 captaven_soundcpu_status_r();

	virtual void video_start() override;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(captaven_pri_callback);

	void captaven_map(address_map &map);
};

class fghthist_state : public deco32_state
{
public:
	fghthist_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag)
		, m_io_in(*this, "IN%u", 0U)
	{ }

	void fghthist(machine_config &config);
	void fghthistu(machine_config &config);
	void fghthsta(machine_config &config);

	void init_fghthist();

private:
	required_ioport_array<2> m_io_in;
//	DECLARE_WRITE32_MEMBER(sound_w);
	u32 unk_status_r();

	virtual void video_start() override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_callback);

	void fghthist_map(address_map &map);
	void fghthsta_memmap(address_map &map);
private:
};

// nslasher, tattass
class nslasher_state : public deco32_state
{
public:
	nslasher_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag)
		, m_deco_ace(*this, "deco_ace")
	{ }

	void nslasheru(machine_config &config);
	void tattass(machine_config &config);
	void nslasher(machine_config &config);

	void init_tattass();
	void init_nslasher();

private:
	required_device<deco_ace_device> m_deco_ace;

	void tattass_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(tattass_sound_irq_w);
	u16 nslasher_debug_r();

	virtual void video_start() override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 port_b_tattass();
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	void nslasher_map(address_map &map);
	void tattass_map(address_map &map);

	void mixDualAlphaSprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx0, gfx_element *gfx1, int mixAlphaTilemap);

	std::unique_ptr<bitmap_ind16> m_tilemap_alpha_bitmap;

	int m_tattass_eprom_bit;
	int m_last_clock;
	u32 m_buffer;
	int m_buf_ptr;
	int m_pending_command;
	int m_read_bit_count;
	int m_byte_addr;
};

class dragngun_state : public deco32_state
{
public:
	dragngun_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag)
		, m_sprgenzoom(*this, "spritegen_zoom")
		, m_spriteram(*this, "spriteram")
		, m_sprite_layout_ram(*this, "lay%u", 0)
		, m_sprite_lookup_ram(*this, "look%u", 0)
		, m_vol_main(*this, "vol_main")
		, m_vol_gun(*this, "vol_gun")
		, m_io_inputs(*this, "INPUTS")
		, m_io_light_x(*this, "LIGHT%u_X", 0U)
		, m_io_light_y(*this, "LIGHT%u_Y", 0U)
		, m_gun_speaker_disabled(true)
	{ }

	void dragngun(machine_config &config);
	void lockload(machine_config &config);
	void lockloadu(machine_config &config);

	void init_dragngun();
	void init_dragngunj();
	void init_lockload();

	DECLARE_INPUT_CHANGED_MEMBER(lockload_gun_trigger);

private:
	required_device<deco_zoomspr_device> m_sprgenzoom;
	required_device<buffered_spriteram32_device> m_spriteram;

	required_shared_ptr_array<u32, 2> m_sprite_layout_ram;
	required_shared_ptr_array<u32, 2> m_sprite_lookup_ram;
	required_device<lc7535_device> m_vol_main;
	optional_device<lc7535_device> m_vol_gun;

	optional_ioport m_io_inputs;
	optional_ioport_array<2> m_io_light_x;
	optional_ioport_array<2> m_io_light_y;

	u32 m_sprite_ctrl;
	int m_lightgun_port;
	int m_oki2_bank; // lockload
	bitmap_rgb32 m_temp_render_bitmap;

	u32 lightgun_r();
	void lightgun_w(offs_t offset, u32 data = 0);
	void sprite_control_w(u32 data);
	void spriteram_dma_w(u32 data);
	void gun_irq_ack_w(u32 data);
	u32 unk_video_r();
	void eeprom_w(u8 data);
	u32 lockload_gun_mirror_r(offs_t offset);

	void volume_w(u32 data);
	void speaker_switch_w(u32 data);
	LC7535_VOLUME_CHANGED(volume_main_changed);
	LC7535_VOLUME_CHANGED(volume_gun_changed);

	void lockload_okibank_lo_w(u8 data);
	void lockload_okibank_hi_w(u8 data); // lockload

	virtual void video_start() override;
	void dragngun_init_common();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_1_callback);
	DECO16IC_BANK_CB_MEMBER(bank_2_callback);

	void dragngun_map(address_map &map);
	void lockload_map(address_map &map);
	void lockloadu_map(address_map &map);
	void lockload_sound_map(address_map &map);
	void lockloadu_sound_map(address_map &map);
	bool m_gun_speaker_disabled;
};
