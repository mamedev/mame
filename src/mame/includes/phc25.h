// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
#pragma once

#ifndef MAME_INCLUDES_PHC25_H
#define MAME_INCLUDES_PHC25_H


#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "bus/centronics/ctronics.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"
#include "formats/phc25_cas.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define AY8910_TAG      "ay8910"
#define MC6847_TAG      "mc6847"
#define CENTRONICS_TAG  "centronics"

#define PHC25_VIDEORAM_SIZE     0x1800

class phc25_state : public driver_device
{
public:
	phc25_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "videoram")
		, m_maincpu(*this, Z80_TAG)
		, m_vdg(*this, MC6847_TAG)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_cassette(*this, "cassette")
	{ }

	void phc25(machine_config &config);
	void pal(machine_config &config);
	void ntsc(machine_config &config);

private:
	void io_map(address_map &map);
	void mem_map(address_map &map);
	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	uint8_t port40_r();
	void port40_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	uint8_t video_ram_r(offs_t offset);
	MC6847_GET_CHARROM_MEMBER(ntsc_char_rom_r);
	MC6847_GET_CHARROM_MEMBER(pal_char_rom_r);
	void machine_start() override;
	void machine_reset() override;
	uint8_t *m_char_rom = nullptr;
	uint8_t m_port40 = 0U;
	int m_centronics_busy = 0;
	required_shared_ptr<uint8_t> m_vram;
	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
};

#endif
