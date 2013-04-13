/*************************************************************************

    Parodius

*************************************************************************/
#include "sound/k053260.h"

class parodius_state : public driver_device
{
public:
	parodius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053260(*this, "k053260"),
		m_k052109(*this, "k052109"),
		m_k053245(*this, "k053245"),
		m_k053251(*this, "k053251") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	int        m_videobank;
	//int        m_nmi_enabled;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053260_device> m_k053260;
	required_device<k052109_device> m_k052109;
	required_device<k05324x_device> m_k053245;
	required_device<k053251_device> m_k053251;
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_READ8_MEMBER(parodius_052109_053245_r);
	DECLARE_WRITE8_MEMBER(parodius_052109_053245_w);
	DECLARE_WRITE8_MEMBER(parodius_videobank_w);
	DECLARE_WRITE8_MEMBER(parodius_3fc0_w);
	DECLARE_WRITE8_MEMBER(parodius_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_READ8_MEMBER(parodius_sound_r);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_parodius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(parodius_interrupt);
	TIMER_CALLBACK_MEMBER(nmi_callback);
};

/*----------- defined in video/parodius.c -----------*/

extern void parodius_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void parodius_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
