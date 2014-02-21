/*************************************************************************

    Rock'n Rage

*************************************************************************/

#include "sound/vlm5030.h"
#include "video/k007342.h"
#include "video/k007420.h"

class rockrage_state : public driver_device
{
public:
	rockrage_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[2];
	int        m_vreg;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	
	DECLARE_WRITE8_MEMBER(rockrage_bankswitch_w);
	DECLARE_WRITE8_MEMBER(rockrage_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(rockrage_vreg_w);
	DECLARE_READ8_MEMBER(rockrage_VLM5030_busy_r);
	DECLARE_WRITE8_MEMBER(rockrage_speech_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_rockrage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(rockrage_interrupt);
};

/*----------- defined in video/rockrage.c -----------*/
void rockrage_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags);
void rockrage_sprite_callback(running_machine &machine, int *code, int *color);
