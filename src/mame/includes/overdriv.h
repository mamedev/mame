// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Over Drive

*************************************************************************/
#include "machine/k053252.h"
#include "video/k051316.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class overdriv_state : public driver_device
{
public:
	overdriv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_k051316_1(*this, "k051316_1"),
		m_k051316_2(*this, "k051316_2"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_sprram(*this, "sprram"),
		m_screen(*this, "screen")
	{ }

	/* video-related */
	int       m_zoom_colorbase[2];
	int       m_road_colorbase[2];
	int       m_sprite_colorbase;

	/* misc */
	UINT16     m_cpuB_ctrl;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	required_shared_ptr<UINT16> m_sprram;
	required_device<screen_device> m_screen;
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(cpuA_ctrl_w);
	DECLARE_READ16_MEMBER(cpuB_ctrl_r);
	DECLARE_WRITE16_MEMBER(cpuB_ctrl_w);
	DECLARE_WRITE16_MEMBER(overdriv_soundirq_w);
	DECLARE_WRITE16_MEMBER(overdriv_cpuB_irq_x_w);
	DECLARE_WRITE16_MEMBER(overdriv_cpuB_irq_y_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_overdriv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpuB_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(overdriv_cpuA_scanline);

	DECLARE_WRITE16_MEMBER( overdriv_k053246_word_w );
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
	K053246_CB_MEMBER(sprite_callback);
};
