// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_ROUTE16_H
#define MAME_INCLUDES_ROUTE16_H

#pragma once

#include "sound/sn76477.h"
#include "emupal.h"
#include "screen.h"

class route16_state : public driver_device
{
public:
	route16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_sn(*this, "snsnd")
		, m_sharedram(*this, "sharedram")
		, m_videoram1(*this, "videoram1")
		, m_videoram2(*this, "videoram2")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_protection_data(0)
	{}

	void routex(machine_config &config);
	void jongpute(machine_config &config);
	void spacecho(machine_config &config);
	void speakres(machine_config &config);
	void stratvox(machine_config &config);
	void route16(machine_config &config);

	void init_route16();
	void init_route16a();
	void init_route16c();

private:
	DECLARE_WRITE8_MEMBER(out0_w);
	DECLARE_WRITE8_MEMBER(out1_w);
	template<bool cpu1> DECLARE_WRITE8_MEMBER(route16_sharedram_w);
	DECLARE_READ8_MEMBER(route16_prot_read);
	DECLARE_READ8_MEMBER(routex_prot_read);
	DECLARE_WRITE8_MEMBER(jongpute_input_port_matrix_w);
	DECLARE_READ8_MEMBER(jongpute_p1_matrix_r);
	DECLARE_READ8_MEMBER(jongpute_p2_matrix_r);
	DECLARE_READ8_MEMBER(speakres_in3_r);
	DECLARE_WRITE8_MEMBER(speakres_out2_w);
	DECLARE_WRITE8_MEMBER(stratvox_sn76477_w);
	DECLARE_MACHINE_START(speakres);
	DECLARE_MACHINE_START(jongpute);

	uint32_t screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jongpute(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cpu1_io_map(address_map &map);
	void route16_cpu1_map(address_map &map);
	void route16_cpu2_map(address_map &map);
	void routex_cpu1_map(address_map &map);
	void speakres_cpu1_map(address_map &map);
	void stratvox_cpu1_map(address_map &map);
	void stratvox_cpu2_map(address_map &map);
	void jongpute_cpu1_map(address_map &map);

	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	optional_device<sn76477_device> m_sn;

	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_videoram2;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	uint8_t m_protection_data;

	uint8_t m_jongpute_port_select;
	int m_speakres_vrx;
	uint8_t m_flipscreen;
	uint8_t m_palette_1;
	uint8_t m_palette_2;

	virtual void video_start() override;
};

#endif // MAME_INCLUDES_ROUTE16_H
