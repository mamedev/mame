/*************************************************************************

    Rollergames

*************************************************************************/
#include "sound/k053260.h"
#include "machine/k053252.h"
#include "video/k051316.h"
#include "video/konami_helper.h"
#include "video/k053244_k053245.h"

class rollerg_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	rollerg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053260(*this, "k053260"),
		m_k053244(*this, "k053244"),
		m_k051316(*this, "k051316"),
		m_k053252(*this, "k053252")
		{ }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_sprite_colorbase;
	int        m_zoom_colorbase;

	/* misc */
	int        m_readzoomroms;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053260_device> m_k053260;
	required_device<k05324x_device> m_k053244;
	required_device<k051316_device> m_k051316;
	required_device<k053252_device> m_k053252;
	DECLARE_WRITE8_MEMBER(rollerg_0010_w);
	DECLARE_READ8_MEMBER(rollerg_k051316_r);
	DECLARE_WRITE8_MEMBER(soundirq_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_READ8_MEMBER(pip_r);
	DECLARE_READ8_MEMBER(rollerg_sound_r);
	DECLARE_WRITE_LINE_MEMBER(rollerg_irq_ack_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_rollerg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

/*----------- defined in video/rollerg.c -----------*/
extern void rollerg_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void rollerg_zoom_callback(running_machine &machine, int *code,int *color,int *flags);
