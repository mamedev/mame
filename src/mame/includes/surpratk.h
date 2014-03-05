/*************************************************************************

    Surprise Attack

*************************************************************************/

#include "video/k053244_k053245.h"
#include "video/k052109.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class surpratk_state : public driver_device
{
public:
	surpratk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_k052109(*this, "k052109"),
		m_k053244(*this, "k053244"),
		m_k053251(*this, "k053251"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	dynamic_array<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	int        m_videobank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<k052109_device> m_k052109;
	required_device<k05324x_device> m_k053244;
	required_device<k053251_device> m_k053251;
	required_device<palette_device> m_palette;
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(surpratk_videobank_w);
	DECLARE_WRITE8_MEMBER(surpratk_5fc0_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_surpratk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(surpratk_interrupt);
};

/*----------- defined in video/surpratk.c -----------*/

extern void surpratk_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void surpratk_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
