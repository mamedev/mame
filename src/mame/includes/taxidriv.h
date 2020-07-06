// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_TAXIDRIV_H
#define MAME_INCLUDES_TAXIDRIV_H

#pragma once

#include "emupal.h"

class taxidriv_state : public driver_device
{
public:
	taxidriv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram0(*this, "vram0"),
		m_vram1(*this, "vram1"),
		m_vram2(*this, "vram2"),
		m_vram3(*this, "vram3"),
		m_vram4(*this, "vram4"),
		m_vram5(*this, "vram5"),
		m_vram6(*this, "vram6"),
		m_vram7(*this, "vram7"),
		m_scroll(*this, "scroll")
	{ }

	void taxidriv(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_vram0;
	required_shared_ptr<uint8_t> m_vram1;
	required_shared_ptr<uint8_t> m_vram2;
	required_shared_ptr<uint8_t> m_vram3;
	required_shared_ptr<uint8_t> m_vram4;
	required_shared_ptr<uint8_t> m_vram5;
	required_shared_ptr<uint8_t> m_vram6;
	required_shared_ptr<uint8_t> m_vram7;
	required_shared_ptr<uint8_t> m_scroll;

	int m_s1;
	int m_s2;
	int m_s3;
	int m_s4;
	int m_latchA;
	int m_latchB;
	int m_bghide;
	int m_spritectrl[9];

	void p2a_w(uint8_t data);
	void p2b_w(uint8_t data);
	void p2c_w(uint8_t data);
	void p3a_w(uint8_t data);
	void p3b_w(uint8_t data);
	void p3c_w(uint8_t data);
	void p4a_w(uint8_t data);
	void p4b_w(uint8_t data);
	void p4c_w(uint8_t data);
	uint8_t p0a_r();
	uint8_t p0c_r();
	void p0b_w(uint8_t data);
	void p0c_w(uint8_t data);
	uint8_t p1b_r();
	uint8_t p1c_r();
	void p1a_w(uint8_t data);
	void p1c_w(uint8_t data);
	uint8_t p8910_0a_r();
	uint8_t p8910_1a_r();
	void p8910_0b_w(uint8_t data);
	void spritectrl_w(offs_t offset, uint8_t data);

	void taxidriv_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpu2_map(address_map &map);
	void cpu3_map(address_map &map);
	void cpu3_port_map(address_map &map);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_TAXIDRIV_H
