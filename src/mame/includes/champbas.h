// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria, hap
/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/

#include "machine/alpha8201.h"
#include "sound/dac.h"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_alpha_8201(*this, "alpha_8201"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{ }

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<alpha_8201_device> m_alpha_8201;
	optional_device<dac_device> m_dac1;
	optional_device<dac_device> m_dac2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_mainram;
	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_spriteram2;

	// internal state
	UINT8 m_irq_mask;
	tilemap_t *m_bg_tilemap;
	UINT8 m_gfx_bank;
	UINT8 m_palette_bank;

	// handlers
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_switch_w);
	DECLARE_WRITE8_MEMBER(mcu_start_w);
	DECLARE_READ8_MEMBER(champbja_protection_r);

	DECLARE_CUSTOM_INPUT_MEMBER(watchdog_bit2);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(exctsccr_sound_irq);

	DECLARE_WRITE8_MEMBER(dac1_w);
	DECLARE_WRITE8_MEMBER(dac2_w);

	DECLARE_WRITE8_MEMBER(tilemap_w);
	DECLARE_WRITE8_MEMBER(gfxbank_w);
	DECLARE_WRITE8_MEMBER(palette_bank_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	DECLARE_DRIVER_INIT(exctsccr);
	DECLARE_DRIVER_INIT(champbas);

	DECLARE_PALETTE_INIT(champbas);
	DECLARE_PALETTE_INIT(exctsccr);
	DECLARE_VIDEO_START(champbas);
	DECLARE_VIDEO_START(exctsccr);
	TILE_GET_INFO_MEMBER(champbas_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(exctsccr_get_bg_tile_info);

	UINT32 screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void champbas_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void exctsccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};
