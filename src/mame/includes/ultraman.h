/*************************************************************************

    Ultraman

*************************************************************************/

#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"

class ultraman_state : public driver_device
{
public:
	ultraman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k051316_1(*this, "k051316_1"),
		m_k051316_2(*this, "k051316_2"),
		m_k051316_3(*this, "k051316_3"),
		m_k051960(*this, "k051960") { }

	/* memory pointers */
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_sprite_colorbase;
	int        m_zoom_colorbase[3];
	int        m_bank0;
	int        m_bank1;
	int        m_bank2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;
	required_device<k051316_device> m_k051316_3;
	required_device<k051960_device> m_k051960;
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_WRITE16_MEMBER(sound_irq_trigger_w);
	DECLARE_WRITE16_MEMBER(ultraman_gfxctrl_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_ultraman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in video/ultraman.c -----------*/
extern void ultraman_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void ultraman_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
extern void ultraman_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);
extern void ultraman_zoom_callback_2(running_machine &machine, int *code,int *color,int *flags);
