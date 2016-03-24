// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "video/bufsprite.h"
#include "sound/upd7759.h"
#include "sound/k007232.h"

class twin16_state : public driver_device
{
public:
	twin16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_gfxrombank(*this, "gfxrombank"),
		m_fixram(*this, "fixram"),
		m_videoram(*this, "videoram"),
		m_zipram(*this, "zipram"),
		m_sprite_gfx_ram(*this, "sprite_gfx_ram"),
		m_gfxrom(*this, "gfxrom") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_memory_bank m_gfxrombank;
	required_shared_ptr<UINT16> m_fixram;
	required_shared_ptr_array<UINT16, 2> m_videoram;
	optional_shared_ptr<UINT16> m_zipram;
	optional_shared_ptr<UINT16> m_sprite_gfx_ram;
	required_region_ptr<UINT16> m_gfxrom;

	UINT16 m_CPUA_register;
	UINT16 m_CPUB_register;
	bool m_is_fround;
	UINT16 m_sprite_buffer[0x800];
	emu_timer *m_sprite_timer;
	int m_sprite_busy;
	int m_need_process_spriteram;
	UINT16 m_scrollx[3];
	UINT16 m_scrolly[3];
	UINT16 m_video_register;
	tilemap_t *m_fixed_tmap;
	tilemap_t *m_scroll_tmap[2];

	DECLARE_WRITE16_MEMBER(CPUA_register_w);
	DECLARE_WRITE16_MEMBER(CPUB_register_w);

	DECLARE_READ16_MEMBER(sprite_status_r);
	DECLARE_WRITE16_MEMBER(video_register_w);
	DECLARE_WRITE16_MEMBER(fixram_w);
	DECLARE_WRITE16_MEMBER(videoram0_w);
	DECLARE_WRITE16_MEMBER(videoram1_w);
	DECLARE_WRITE16_MEMBER(zipram_w);

	DECLARE_READ8_MEMBER(upd_busy_r);
	DECLARE_WRITE8_MEMBER(upd_reset_w);
	DECLARE_WRITE8_MEMBER(upd_start_w);

	DECLARE_DRIVER_INIT(twin16);

	TILE_GET_INFO_MEMBER(fix_tile_info);
	TILE_GET_INFO_MEMBER(layer0_tile_info);
	TILE_GET_INFO_MEMBER(layer1_tile_info);

	UINT32 screen_update_twin16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_twin16(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(CPUA_interrupt);
	INTERRUPT_GEN_MEMBER(CPUB_interrupt);
	TIMER_CALLBACK_MEMBER(sprite_tick);
	DECLARE_WRITE8_MEMBER(volume_callback);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void tile_get_info(tile_data &tileinfo, UINT16 data, int color_base);
private:
	int set_sprite_timer();
	void spriteram_process();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap );
	int spriteram_process_enable();
	void twin16_postload();
};

class fround_state : public twin16_state
{
public:
	fround_state(const machine_config &mconfig, device_type type, const char *tag)
		: twin16_state(mconfig, type, tag)
	{}

	DECLARE_WRITE16_MEMBER(fround_CPU_register_w);
	DECLARE_WRITE16_MEMBER(gfx_bank_w);
	DECLARE_DRIVER_INIT(fround);

protected:
	virtual void video_start() override;
	virtual void tile_get_info(tile_data &tileinfo, UINT16 data, int color_base) override;

private:
	UINT8 m_gfx_bank[4];
};

class cuebrickj_state : public twin16_state
{
public:
	cuebrickj_state(const machine_config &mconfig, device_type type, const char *tag)
		: twin16_state(mconfig, type, tag)
	{}

	DECLARE_WRITE8_MEMBER(nvram_bank_w);
	DECLARE_DRIVER_INIT(cuebrickj);

private:
	UINT16 m_nvram[0x400 * 0x20 / 2];
};
