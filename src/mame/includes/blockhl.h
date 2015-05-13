// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Block Hole

*************************************************************************/

#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"

class blockhl_state : public driver_device
{
public:
	blockhl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	std::vector<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* misc */
	int        m_palette_selected;
	int        m_rombank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(blockhl_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_blockhl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(blockhl_interrupt);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);
};
