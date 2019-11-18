// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/*************************************************************************

    Megazone

*************************************************************************/
#ifndef MAME_INCLUDES_MEGAZONE_H
#define MAME_INCLUDES_MEGAZONE_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/flt_rc.h"
#include "emupal.h"

class megazone_state : public driver_device
{
public:
	megazone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scrolly(*this, "scrolly"),
		m_scrollx(*this, "scrollx"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_daccpu(*this, "daccpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_filter(*this, "filter.0.%u", 0U)
	{ }

	void megazone(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	std::unique_ptr<bitmap_ind16>   m_tmpbitmap;
	int           m_flipscreen;

	/* misc */
	int           m_i8039_status;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_daccpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<filter_rc_device, 3> m_filter;

	uint8_t         m_irq_mask;
	DECLARE_WRITE8_MEMBER(megazone_i8039_irq_w);
	DECLARE_WRITE8_MEMBER(i8039_irqen_and_status_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_READ8_MEMBER(megazone_port_a_r);
	DECLARE_WRITE8_MEMBER(megazone_port_b_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void megazone_palette(palette_device &palette) const;
	uint32_t screen_update_megazone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void megazone_i8039_io_map(address_map &map);
	void megazone_i8039_map(address_map &map);
	void megazone_map(address_map &map);
	void megazone_sound_io_map(address_map &map);
	void megazone_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MEGAZONE_H
