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
#include "screen.h"

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
		m_sprites(*this, "sprites"),
		m_mixer(*this, "mixer"),
		m_video_timings(*this, "video_timings"),
		m_screen(*this, "screen")
	{ }

	/* video-related */
	int       m_zoom_colorbase[2];
	int       m_road_colorbase[2];
	int       m_sprite_colorbase;

	/* misc */
	uint16_t     m_cpuB_ctrl;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;
	required_device<k053246_053247_device> m_sprites;
	required_device<k053251_device> m_mixer;
	required_device<k053252_device> m_video_timings;
	required_device<screen_device> m_screen;
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(cpuA_ctrl_w);
	DECLARE_READ16_MEMBER(cpuB_ctrl_r);
	DECLARE_WRITE16_MEMBER(cpuB_ctrl_w);
	DECLARE_WRITE16_MEMBER(overdriv_soundirq_w);
	DECLARE_WRITE8_MEMBER(sound_ack_w);
	DECLARE_WRITE16_MEMBER(slave_irq4_assert_w);
	DECLARE_WRITE16_MEMBER(slave_irq5_assert_w);
	DECLARE_WRITE16_MEMBER(objdma_w);
	TIMER_CALLBACK_MEMBER(objdma_end_cb);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	//INTERRUPT_GEN_MEMBER(cpuB_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(overdriv_cpuA_scanline);
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
};
