/*************************************************************************

    Lethal Enforcers

*************************************************************************/
#include "sound/k054539.h"
#include "video/konicdev.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k05324x.h"
#include "video/k054000.h"

class lethal_state : public driver_device
{
public:
	lethal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_k054539(*this, "k054539"),
		m_k056832(*this, "k056832"),
		m_k053244(*this, "k053244"),
		m_k054000(*this, "k054000") { }

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;

	/* misc */
	UINT8      m_cur_control2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<k054539_device> m_k054539;
	required_device<k056832_device> m_k056832;
	required_device<k05324x_device> m_k053244;
	required_device<k054000_device> m_k054000;
	DECLARE_WRITE8_MEMBER(control2_w);
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(sound_irq_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(le_bankswitch_w);
	DECLARE_READ8_MEMBER(le_4800_r);
	DECLARE_WRITE8_MEMBER(le_4800_w);
	DECLARE_WRITE8_MEMBER(le_bgcolor_w);
	DECLARE_READ8_MEMBER(guns_r);
	DECLARE_READ8_MEMBER(gunsaux_r);
	DECLARE_WRITE8_MEMBER(lethalen_palette_control);
	DECLARE_DRIVER_INIT(lethalen);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_lethalen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(lethalen_interrupt);
};

/*----------- defined in video/lethal.c -----------*/
extern void lethalen_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
extern void lethalen_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
