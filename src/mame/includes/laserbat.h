// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/

#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "machine/s2636.h"
#include "sound/sn76477.h"

class laserbat_state : public driver_device
{
public:
	laserbat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_s2636_1(*this, "s2636_1"),
		m_s2636_2(*this, "s2636_2"),
		m_s2636_3(*this, "s2636_3"),
		m_sn(*this, "snsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<ay8910_device> m_ay1;
	optional_device<ay8910_device> m_ay2;
	required_device<s2636_device> m_s2636_1;
	required_device<s2636_device> m_s2636_2;
	required_device<s2636_device> m_s2636_3;
	optional_device<sn76477_device> m_sn;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_video_page;

	/* misc */
	int        m_input_mux;
	int        m_active_8910;
	int        m_port0a;
	int        m_last_port0b;
	int        m_cb1_toggle;

	/* information for the single 32x32 sprite displayed */
	int        m_sprite_x;
	int        m_sprite_y;
	int        m_sprite_code;
	int        m_sprite_color;
	int        m_sprite_enable;

	/* sound-related */
	int        m_csound1;
	int        m_ksound1;
	int        m_ksound2;
	int        m_ksound3;
	int        m_degr;
	int        m_filt;
	int        m_a;
	int        m_us;
	int        m_bit14;

	/* device */
	pia6821_device *m_pia;
	tms3615_device *m_tms1;
	tms3615_device *m_tms2;

	// memory
	UINT8      m_videoram[0x400];
	UINT8      m_colorram[0x400];
	DECLARE_WRITE8_MEMBER(laserbat_videoram_w);
	DECLARE_WRITE8_MEMBER(video_extra_w);
	DECLARE_WRITE8_MEMBER(sprite_x_y_w);
	DECLARE_WRITE8_MEMBER(laserbat_input_mux_w);
	DECLARE_READ8_MEMBER(laserbat_input_r);
	DECLARE_WRITE8_MEMBER(laserbat_cnteff_w);
	DECLARE_WRITE8_MEMBER(laserbat_cntmov_w);
	DECLARE_WRITE8_MEMBER(laserbat_csound1_w);
	DECLARE_WRITE8_MEMBER(laserbat_csound2_w);
	DECLARE_WRITE_LINE_MEMBER(zaccaria_irq0a);
	DECLARE_WRITE_LINE_MEMBER(zaccaria_irq0b);
	DECLARE_READ8_MEMBER(zaccaria_port0a_r);
	DECLARE_WRITE8_MEMBER(zaccaria_port0a_w);
	DECLARE_WRITE8_MEMBER(zaccaria_port0b_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_laserbat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(laserbat_interrupt);
	INTERRUPT_GEN_MEMBER(zaccaria_cb1_toggle);
};
