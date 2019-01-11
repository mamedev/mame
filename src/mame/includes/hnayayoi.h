// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Hana Yayoi & other Dynax games (using 1st version of their blitter)

*************************************************************************/
#ifndef MAME_INCLUDES_HNAYAYOI_H
#define MAME_INCLUDES_HNAYAYOI_H

#pragma once

#include "machine/74259.h"
#include "sound/msm5205.h"
#include "video/mc6845.h"
#include "emupal.h"

class hnayayoi_state : public driver_device
{
public:
	hnayayoi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette")
	{ }

	void untoucha(machine_config &config);
	void hnayayoi(machine_config &config);
	void hnfubuki(machine_config &config);

	void init_hnfubuki();

private:
	/* video-related */
	std::unique_ptr<uint8_t[]> m_pixmap[8];
	int        m_palbank;
	uint8_t      m_blit_layer;
	uint16_t     m_blit_dest;
	uint32_t     m_blit_src;

	/* misc */
	int        m_keyb;
	bool m_nmi_enable;

	DECLARE_READ8_MEMBER(keyboard_0_r);
	DECLARE_READ8_MEMBER(keyboard_1_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_param_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_start_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_clear_w);
	DECLARE_WRITE8_MEMBER(hnayayoi_palbank_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_clock_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(untoucha);
	MC6845_UPDATE_ROW(hnayayoi_update_row);
	MC6845_UPDATE_ROW(untoucha_update_row);
	void common_vh_start( int num_pixmaps );
	void copy_pixel( int x, int y, int pen );
	void draw_layer_interleaved(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t row, uint16_t y, uint8_t x_count, int left_pixmap, int right_pixmap, int palbase, bool transp);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	required_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;
	void hnayayoi_io_map(address_map &map);
	void hnayayoi_map(address_map &map);
	void hnfubuki_map(address_map &map);
	void untoucha_io_map(address_map &map);
	void untoucha_map(address_map &map);
};

#endif // MAME_INCLUDES_HNAYAYOI_H
