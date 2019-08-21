// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Jaleco Moero Pro Yakyuu Homerun hardware

*************************************************************************/

#include "sound/upd7759.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class homerun_state : public driver_device
{
public:
	homerun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_d7756(*this, "d7756"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_mainbank(*this, "mainbank")
	{ }

	void ganjaja(machine_config &config);
	void dynashot(machine_config &config);
	void homerun(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(sprite0_r);
	DECLARE_CUSTOM_INPUT_MEMBER(homerun_d7756_busy_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ganjaja_d7756_busy_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ganjaja_hopper_status_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_spriteram;
	optional_device<upd7756_device> m_d7756;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_memory_bank m_mainbank;

	u8 m_control;
	u8 m_sample;

	tilemap_t *m_tilemap;
	int m_gfx_ctrl;
	int m_scrollx;
	int m_scrolly;

	void control_w(u8 data);
	void d7756_sample_w(u8 data);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(scrollhi_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);
	DECLARE_WRITE8_MEMBER(scrollx_w);

	static rgb_t homerun_RGB332(u32 raw);
	TILE_GET_INFO_MEMBER(get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void banking_w(u8 data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map);
	void mem_map(address_map &map);
};
