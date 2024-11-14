// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*
    VTech Phusion (2000)

    This handheld organizer/PDA runs on an unknown CR16B SoC.
    It's probably similar (but not identical) to the one described in glcx.cpp.

    Main board:
        U1:  Analog Integrations AIC1652CS
        U2:  8Mbit flash (Toshiba TC58FVT800FT-85)
        U3:  Hyundai GM76FV18ALLFW70
        U10: unknown CR16B-based SoC (epoxy blob)
        U11: National Semiconductor DS14C232CM
        U12: National Semiconductor LM4882
        U14: 16Mbit mask ROM (Sharp LH53V16500)

*/

#include "emu.h"

#include "cpu/cr16b/cr16b.h"
#include "machine/intelfsh.h"
#include "screen.h"
#include "speaker.h"

namespace {

class phusion_state : public driver_device
{
public:
	phusion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void phusion(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void phusion_map(address_map &map) ATTR_COLD;

	required_device<cr16b_device> m_maincpu;
};

uint32_t phusion_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void phusion_state::phusion_map(address_map &map)
{
	/*
	TODO: this is a placeholder - flash, RAM, etc should also be in here somewhere
	the instruction at $000050 branches to $080054, which is supposed to mirror the next instruction(s) at $000054
	*/
	map(0x000000, 0x1fffff).rom();
}

INPUT_PORTS_START( phusion )
INPUT_PORTS_END

void phusion_state::phusion(machine_config &config)
{
	CR16B(config, m_maincpu, 10000000); // FIXME: determine exact type and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &phusion_state::phusion_map);

	TC58FVT800(config, "flash");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(160, 160);
	screen.set_visarea(0, 160-1, 0, 160-1);
	screen.set_screen_update(FUNC(phusion_state::screen_update));

	SPEAKER(config, "speaker", 0).front_center();
}

ROM_START( phusion )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "27-06559-0-0.u14", 0x000000, 0x200000, CRC(afaee544) SHA1(b9cd5e708e122a9ada6b5aa92ef516cf1f026f72)) // LHMV7FNU

	ROM_REGION( 0x100000, "flash", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace

SYST( 2000, phusion, 0, 0, phusion, phusion, phusion_state, empty_init, "VTech", "Phusion", MACHINE_IS_SKELETON )
