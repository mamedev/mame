// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Crime Fighters

*************************************************************************/
#include "cpu/m6809/konami.h"
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"

class crimfght_state : public driver_device
{
public:
	crimfght_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	std::vector<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(crimfght_coin_w);
	DECLARE_WRITE8_MEMBER(crimfght_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	DECLARE_WRITE8_MEMBER(crimfght_snd_bankswitch_w);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_crimfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(crimfght_interrupt);
	DECLARE_WRITE8_MEMBER(volume_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);
};
