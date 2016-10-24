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

	void calomega_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void calomega_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t s903_mux_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void s903_mux_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t s905_mux_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void s905_mux_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pia0_ain_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pia0_bin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia0_aout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia0_bout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia0_ca2_w(int state);
	uint8_t pia1_ain_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pia1_bin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia1_aout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia1_bout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps_903a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps_903b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps_905_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_acia_tx(int state);
	void write_acia_clock(int state);
	void update_aciabaud_scale(int state);
	void init_sys903();
	void init_comg080();
	void init_s903mod();
	void init_sys905();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t screen_update_calomega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init_calomega(palette_device &palette);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<acia6850_device> m_acia6850_0;
	optional_device<clock_device> m_aciabaud;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	optional_ioport m_in0;
	optional_ioport m_in0_0;
	optional_ioport m_in0_1;
	optional_ioport m_in0_2;
	optional_ioport m_in0_3;
	optional_ioport m_frq;
	optional_ioport m_sw2;

	uint8_t m_tx_line;
	int m_s903_mux_data;
	int m_s905_mux_data;
	tilemap_t *m_bg_tilemap;
};
