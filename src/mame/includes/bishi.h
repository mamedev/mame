// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************

    Bishi Bashi Champ Mini Game Senshuken

*************************************************************************/


#include "video/k054156_k054157_k056832.h"
#include "video/k055555.h"
#include "video/k054338.h"
#include "video/konami_helper.h"

#define CPU_CLOCK       (XTAL_24MHz / 2)        /* 68000 clock */
#define SOUND_CLOCK     XTAL_16_9344MHz     /* YMZ280 clock */

class bishi_state : public driver_device
{
public:
	bishi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056832(*this, "k056832"),
		m_k054338(*this, "k054338"),
		m_k055555(*this, "k055555"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	UINT8 *    m_ram;

	/* video-related */
	int        m_layer_colorbase[4];

	/* misc */
	UINT16     m_cur_control;
	UINT16     m_cur_control2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<k056832_device> m_k056832;
	required_device<k054338_device> m_k054338;
	required_device<k055555_device> m_k055555;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	DECLARE_READ16_MEMBER(control_r);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_READ16_MEMBER(bishi_mirror_r);
	DECLARE_READ16_MEMBER(bishi_K056832_rom_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_bishi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bishi_scanline);
	K056832_CB_MEMBER(tile_callback);
};
