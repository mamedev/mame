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
		, m_videoram(*this, "videoram%u", 1U)
		, m_proms(*this, "proms")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{ }

	void routex(machine_config &config);
	void route16(machine_config &config);

	void init_route16();
	void init_route16a();
	void init_route16c();
	void init_route16d();

protected:
	virtual void machine_start() override ATTR_COLD;

	void out0_w(uint8_t data);
	void out1_w(uint8_t data);

	uint32_t screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_stratvox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;

	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_region_ptr<uint8_t> m_proms;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	uint8_t m_protection_data = 0;
	uint8_t m_flipscreen = 0;
	uint8_t m_palreg[2] = { };

private:
	uint8_t route16_prot_r();
	uint8_t routex_prot_r();

	void cpu1_io_map(address_map &map) ATTR_COLD;
	void route16_cpu1_map(address_map &map) ATTR_COLD;
	void route16_cpu2_map(address_map &map) ATTR_COLD;
	void routex_cpu1_map(address_map &map) ATTR_COLD;
};

class jongpute_state : public route16_state
{
public:
	jongpute_state(const machine_config &mconfig, device_type type, const char *tag)
		: route16_state(mconfig, type, tag)
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_key(*this, "KEY%u", 0U)
	{ }

	void jongpute(machine_config &config);
	void vscompmj(machine_config &config);

	void init_vscompmj();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void input_w(uint8_t data);
	template <int N> uint8_t input_r();

	void jongpute_cpu1_map(address_map &map) ATTR_COLD;
	void vscompmj_cpu1_map(address_map &map) ATTR_COLD;
	void vscompmj_decrypted_opcodes(address_map &map) ATTR_COLD;

	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_ioport_array<8> m_key;

	uint8_t m_port_select = 0;
};

#endif // MAME_SUNELECTRONICS_ROUTE16_H
