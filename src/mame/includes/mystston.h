// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

***************************************************************************/

#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class mystston_state : public driver_device
{
public:
	mystston_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ay8910(*this, "ay%u", 1U),
		m_ay8910_data(*this, "ay8910_data"),
		m_ay8910_select(*this, "ay8910_select"),
		m_dsw1(*this, "DSW1"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_scroll(*this, "scroll"),
		m_video_control(*this, "video_control"),
		m_color_prom(*this, "proms"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	void mystston(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void video_start() override;
	virtual void video_reset() override;

private:
	static constexpr XTAL MASTER_CLOCK = XTAL(12'000'000);

	// machine state
	required_device_array<ay8910_device, 2> m_ay8910;
	required_shared_ptr<uint8_t> m_ay8910_data;
	required_shared_ptr<uint8_t> m_ay8910_select;
	required_ioport m_dsw1;

	// video state
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	emu_timer *m_interrupt_timer = nullptr;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_video_control;
	required_region_ptr<uint8_t> m_color_prom;

	void irq_clear_w(uint8_t data);
	void ay8910_select_w(uint8_t data);
	void video_control_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void set_palette();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip);
	void on_scanline_interrupt();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	void video(machine_config &config);
	void main_map(address_map &map);
};
