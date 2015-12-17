// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/k054539.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053251.h"
#include "video/k052109.h"
#include "video/konami_helper.h"

class xmen_state : public driver_device
{
public:
	xmen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_xmen6p_spriteramleft(*this, "spriteramleft"),
		m_xmen6p_spriteramright(*this, "spriteramright"),
		m_xmen6p_tilemapleft(*this, "tilemapleft"),
		m_xmen6p_tilemapright(*this, "tilemapright"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_screen(*this, "screen"),
		m_z80bank(*this, "z80bank") { }

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* for xmen6p */
	std::unique_ptr<bitmap_ind16> m_screen_right;
	std::unique_ptr<bitmap_ind16> m_screen_left;
	optional_shared_ptr<UINT16> m_xmen6p_spriteramleft;
	optional_shared_ptr<UINT16> m_xmen6p_spriteramright;
	optional_shared_ptr<UINT16> m_xmen6p_tilemapleft;
	optional_shared_ptr<UINT16> m_xmen6p_tilemapright;
	UINT16 *   m_k053247_ram;

	/* misc */
	UINT8       m_vblank_irq_mask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<screen_device> m_screen;

	required_memory_bank m_z80bank;
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_WRITE16_MEMBER(xmen_18fa00_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_CUSTOM_INPUT_MEMBER(xmen_frame_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(xmen6p);
	UINT32 screen_update_xmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_xmen6p_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_xmen6p_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_xmen6p(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(xmen_scanline);
	K052109_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
};
