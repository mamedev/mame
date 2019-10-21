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


#define MYSTSTON_MASTER_CLOCK   (XTAL(12'000'000))


class mystston_state : public driver_device
{
public:
	mystston_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ay8910(*this, "ay%u", 1U),
		m_ay8910_data(*this, "ay8910_data"),
		m_ay8910_select(*this, "ay8910_select"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_scroll(*this, "scroll"),
		m_video_control(*this, "video_control") ,
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

	/* machine state */
	required_device_array<ay8910_device, 2> m_ay8910;
	required_shared_ptr<uint8_t> m_ay8910_data;
	required_shared_ptr<uint8_t> m_ay8910_select;

	/* video state */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	emu_timer *m_interrupt_timer;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_video_control;
	DECLARE_WRITE8_MEMBER(irq_clear_w);
	DECLARE_WRITE8_MEMBER(mystston_ay8910_select_w);
	DECLARE_WRITE8_MEMBER(mystston_video_control_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update_mystston(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void set_palette();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip);
	void mystston_on_scanline_interrupt();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	void mystston_video(machine_config &config);
	void main_map(address_map &map);
};
