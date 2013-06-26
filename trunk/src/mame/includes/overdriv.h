/*************************************************************************

    Over Drive

*************************************************************************/
#include "sound/k053260.h"
#include "machine/k053252.h"

class overdriv_state : public driver_device
{
public:
	overdriv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_k053260_1(*this, "k053260_1"),
		m_k053260_2(*this, "k053260_2"),
		m_k051316_1(*this, "k051316_1"),
		m_k051316_2(*this, "k051316_2"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"){ }

	/* memory pointers */
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

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
	required_device<k053260_device> m_k053260_1;
	required_device<k053260_device> m_k053260_2;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(cpuA_ctrl_w);
	DECLARE_READ16_MEMBER(cpuB_ctrl_r);
	DECLARE_WRITE16_MEMBER(cpuB_ctrl_w);
	DECLARE_WRITE16_MEMBER(overdriv_soundirq_w);
	DECLARE_WRITE16_MEMBER(overdriv_cpuB_irq5_w);
	DECLARE_WRITE16_MEMBER(overdriv_cpuB_irq6_w);
	DECLARE_READ8_MEMBER(overdriv_1_sound_r);
	DECLARE_READ8_MEMBER(overdriv_2_sound_r);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_overdriv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpuB_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(overdriv_cpuA_scanline);
};

/*----------- defined in video/overdriv.c -----------*/
extern void overdriv_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void overdriv_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
extern void overdriv_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);
