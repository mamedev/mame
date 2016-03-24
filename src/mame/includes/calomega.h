// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
#include "machine/6850acia.h"
#include "machine/clock.h"

class calomega_state : public driver_device
{
public:
	calomega_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia6850_0(*this, "acia6850_0"),
		m_aciabaud(*this, "aciabaud"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_in0(*this, "IN0"),
		m_in0_0(*this, "IN0-0"),
		m_in0_1(*this, "IN0-1"),
		m_in0_2(*this, "IN0-2"),
		m_in0_3(*this, "IN0-3"),
		m_frq(*this, "FRQ"),
		m_sw2(*this, "SW2")
	{
	}

	DECLARE_WRITE8_MEMBER(calomega_videoram_w);
	DECLARE_WRITE8_MEMBER(calomega_colorram_w);
	DECLARE_READ8_MEMBER(s903_mux_port_r);
	DECLARE_WRITE8_MEMBER(s903_mux_w);
	DECLARE_READ8_MEMBER(s905_mux_port_r);
	DECLARE_WRITE8_MEMBER(s905_mux_w);
	DECLARE_READ8_MEMBER(pia0_ain_r);
	DECLARE_READ8_MEMBER(pia0_bin_r);
	DECLARE_WRITE8_MEMBER(pia0_aout_w);
	DECLARE_WRITE8_MEMBER(pia0_bout_w);
	DECLARE_WRITE_LINE_MEMBER(pia0_ca2_w);
	DECLARE_READ8_MEMBER(pia1_ain_r);
	DECLARE_READ8_MEMBER(pia1_bin_r);
	DECLARE_WRITE8_MEMBER(pia1_aout_w);
	DECLARE_WRITE8_MEMBER(pia1_bout_w);
	DECLARE_WRITE8_MEMBER(lamps_903a_w);
	DECLARE_WRITE8_MEMBER(lamps_903b_w);
	DECLARE_WRITE8_MEMBER(lamps_905_w);
	DECLARE_WRITE_LINE_MEMBER(write_acia_tx);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(update_aciabaud_scale);
	DECLARE_DRIVER_INIT(sys903);
	DECLARE_DRIVER_INIT(comg080);
	DECLARE_DRIVER_INIT(s903mod);
	DECLARE_DRIVER_INIT(sys905);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	UINT32 screen_update_calomega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(calomega);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<acia6850_device> m_acia6850_0;
	optional_device<clock_device> m_aciabaud;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	optional_ioport m_in0;
	optional_ioport m_in0_0;
	optional_ioport m_in0_1;
	optional_ioport m_in0_2;
	optional_ioport m_in0_3;
	optional_ioport m_frq;
	optional_ioport m_sw2;

	UINT8 m_tx_line;
	int m_s903_mux_data;
	int m_s905_mux_data;
	tilemap_t *m_bg_tilemap;
};
