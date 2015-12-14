// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************

    Bishi Bashi Champ Mini Game Senshuken

*************************************************************************/


#include "video/k054156_k054157_k056832.h"
#include "video/k055555.h"
#include "video/k054338.h"
#include "video/konami_helper.h"
#include "screen.h"

#define CPU_CLOCK       (XTAL_24MHz / 2)        /* 68000 clock */
#define SOUND_CLOCK     XTAL_16_9344MHz     /* YMZ280 clock */

class bishi_state : public driver_device
{
public:
	bishi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tilemap(*this, "tilemap"),
		m_blender(*this, "blender"),
		m_mixer(*this, "mixer"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	uint8_t *    m_ram;

	/* video-related */
	int        m_layer_colorbase[4];

	/* misc */
	uint16_t     m_cur_control;
	uint16_t     m_cur_control2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<k058143_056832_device> m_tilemap;
	required_device<k054338_device> m_blender;
	required_device<k055555_device> m_mixer;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	DECLARE_READ16_MEMBER(control_r);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_READ16_MEMBER(bishi_mirror_r);
	DECLARE_READ16_MEMBER(bishi_K056832_rom_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_bishi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bishi_scanline);

	void blender_update(screen_device &screen, bitmap_ind16 **bitmaps, const rectangle &cliprect);
	void mixer_init(screen_device &screen, bitmap_ind16 **bitmaps);
	void mixer_update(screen_device &screen, bitmap_ind16 **bitmaps, const rectangle &cliprect);
};
