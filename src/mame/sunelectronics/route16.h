// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_SUNELECTRONICS_ROUTE16_H
#define MAME_SUNELECTRONICS_ROUTE16_H

#pragma once

#include "emupal.h"
#include "screen.h"

class route16_state : public driver_device
{
public:
	route16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_sharedram(*this, "sharedram")
		, m_videoram1(*this, "videoram1")
		, m_videoram2(*this, "videoram2")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_key(*this, "KEY%u", 0U)
		, m_protection_data(0)
	{}

	void routex(machine_config &config);
	void jongpute(machine_config &config);
	void route16(machine_config &config);
	void vscompmj(machine_config &config);

	void init_route16();
	void init_route16a();
	void init_route16c();
	void init_route16d();
	void init_vscompmj();

protected:
	virtual void video_start() override ATTR_COLD;

	void out0_w(uint8_t data);
	void out1_w(uint8_t data);

private:
	template<bool cpu1> void route16_sharedram_w(offs_t offset, uint8_t data);
	uint8_t route16_prot_read();
	uint8_t routex_prot_read();
	void jongpute_input_port_matrix_w(uint8_t data);
	uint8_t jongpute_p1_matrix_r();
	uint8_t jongpute_p2_matrix_r();
	DECLARE_MACHINE_START(jongpute);

	uint32_t screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	uint32_t screen_update_jongpute(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	void cpu1_io_map(address_map &map) ATTR_COLD;
	void route16_cpu1_map(address_map &map) ATTR_COLD;
	void route16_cpu2_map(address_map &map) ATTR_COLD;
	void routex_cpu1_map(address_map &map) ATTR_COLD;
	void jongpute_cpu1_map(address_map &map) ATTR_COLD;
	void vscompmj_cpu1_map(address_map &map) ATTR_COLD;
	void vscompmj_decrypted_opcodes(address_map &map) ATTR_COLD;

protected:
	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;

	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_videoram2;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	optional_ioport_array<8> m_key;
	uint8_t m_protection_data;

	uint8_t m_jongpute_port_select = 0;
	uint8_t m_flipscreen = 0;
	uint8_t m_palette_1 = 0;
	uint8_t m_palette_2 = 0;
};

#endif // MAME_SUNELECTRONICS_ROUTE16_H
