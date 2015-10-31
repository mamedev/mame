// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria
/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/

#include "sound/dac.h"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_dac(*this, "dac"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2")
	{ }

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	optional_device<dac_device> m_dac;
	optional_device<dac_device> m_dac1;
	optional_device<dac_device> m_dac2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram_2;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	UINT8 m_gfx_bank;
	UINT8 m_palette_bank;

	/* misc */
	UINT8 m_irq_mask;

	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(champbas_mcu_switch_w);
	DECLARE_WRITE8_MEMBER(champbas_mcu_halt_w);
	DECLARE_READ8_MEMBER(champbja_alt_protection_r);
	DECLARE_WRITE8_MEMBER(champbas_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(champbas_gfxbank_w);
	DECLARE_WRITE8_MEMBER(champbas_palette_bank_w);
	DECLARE_WRITE8_MEMBER(champbas_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(champbas_watchdog_bit2);
	DECLARE_WRITE8_MEMBER(champbas_dac_w);
	DECLARE_WRITE8_MEMBER(champbas_dac1_w);
	DECLARE_WRITE8_MEMBER(champbas_dac2_w);
	DECLARE_DRIVER_INIT(exctsccr);
	DECLARE_DRIVER_INIT(champbas);
	TILE_GET_INFO_MEMBER(champbas_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(exctsccr_get_bg_tile_info);
	DECLARE_MACHINE_START(champbas);
	DECLARE_MACHINE_RESET(champbas);
	DECLARE_VIDEO_START(champbas);
	DECLARE_PALETTE_INIT(champbas);
	DECLARE_MACHINE_START(exctsccr);
	DECLARE_VIDEO_START(exctsccr);
	DECLARE_PALETTE_INIT(exctsccr);
	UINT32 screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(exctsccr_fm_callback);
	void champbas_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void exctsccr_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
