// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Exidy 440 hardware

*************************************************************************/

#include "audio/exidy440.h"

#define EXIDY440_MASTER_CLOCK       (XTAL_12_9792MHz)


class exidy440_state : public driver_device
{
public:
	exidy440_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_imageram(*this, "imageram"),
		m_spriteram(*this, "spriteram"),
		m_scanline(*this, "scanline"),
		m_maincpu(*this, "maincpu"),
		m_custom(*this, "custom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_imageram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scanline;

	required_device<cpu_device> m_maincpu;
	required_device<exidy440_sound_device> m_custom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT8 m_bank;
	const UINT8 *m_showdown_bank_data[2];
	INT8 m_showdown_bank_select;
	UINT8 m_showdown_bank_offset;
	UINT8 m_firq_vblank;
	UINT8 m_firq_beam;
	UINT8 *m_topsecex_yscroll;
	UINT8 m_latched_x;
	UINT8 *m_local_videoram;
	UINT8 *m_local_paletteram;
	UINT8 m_firq_enable;
	UINT8 m_firq_select;
	UINT8 m_palettebank_io;
	UINT8 m_palettebank_vis;
	DECLARE_WRITE8_MEMBER(bankram_w);
	DECLARE_READ8_MEMBER(exidy440_input_port_3_r);
	DECLARE_READ8_MEMBER(sound_command_ack_r);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(exidy440_input_port_3_w);
	DECLARE_WRITE8_MEMBER(exidy440_coin_counter_w);
	DECLARE_READ8_MEMBER(showdown_bank0_r);
	DECLARE_READ8_MEMBER(claypign_protection_r);
	DECLARE_READ8_MEMBER(topsecex_input_port_5_r);
	DECLARE_WRITE8_MEMBER(topsecex_yscroll_w);
	DECLARE_READ8_MEMBER(exidy440_videoram_r);
	DECLARE_WRITE8_MEMBER(exidy440_videoram_w);
	DECLARE_READ8_MEMBER(exidy440_paletteram_r);
	DECLARE_WRITE8_MEMBER(exidy440_paletteram_w);
	DECLARE_READ8_MEMBER(exidy440_horizontal_pos_r);
	DECLARE_READ8_MEMBER(exidy440_vertical_pos_r);
	DECLARE_WRITE8_MEMBER(exidy440_spriteram_w);
	DECLARE_WRITE8_MEMBER(exidy440_control_w);
	DECLARE_WRITE8_MEMBER(exidy440_interrupt_clear_w);
	DECLARE_CUSTOM_INPUT_MEMBER(firq_beam_r);
	DECLARE_CUSTOM_INPUT_MEMBER(firq_vblank_r);
	DECLARE_CUSTOM_INPUT_MEMBER(hitnmiss_button1_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(showdown);
	DECLARE_DRIVER_INIT(topsecex);
	DECLARE_DRIVER_INIT(yukon);
	DECLARE_DRIVER_INIT(exidy440);
	DECLARE_DRIVER_INIT(claypign);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(exidy440);
	DECLARE_VIDEO_START(topsecex);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int scroll_offset, int check_collision);
	void update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,  int scroll_offset, int check_collision);
	UINT32 screen_update_exidy440(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_topsecex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(exidy440_vblank_interrupt);
	TIMER_CALLBACK_MEMBER(delayed_sound_command_w);
	TIMER_CALLBACK_MEMBER(beam_firq_callback);
	TIMER_CALLBACK_MEMBER(collide_firq_callback);
	void exidy440_update_firq();
	void exidy440_bank_select(UINT8 bank);
};

/*----------- defined in video/exidy440.c -----------*/

MACHINE_CONFIG_EXTERN( exidy440_video );
MACHINE_CONFIG_EXTERN( topsecex_video );
