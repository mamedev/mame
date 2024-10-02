// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*

    Skeleton driver for Yamaha PortaSound PCS-30 keyboard with Playcard support

    CPU: Toshiba TMPZ84C00P (Z80 clone)
    Sound: Yamaha YM2142 (GE8)
    Other: Yamaha YM3514 (64 pins)

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "speaker.h"

namespace {

class pcs30_state : public driver_device
{
public:
	pcs30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void pcs30(machine_config &config);

private:
	void pcs30_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
};


void pcs30_state::pcs30_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}


void pcs30_state::pcs30(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000); // TODO: verify clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pcs30_state::pcs30_map);

	SPEAKER(config, "speaker").front_center();
}


INPUT_PORTS_START(pcs30)
INPUT_PORTS_END


ROM_START( pcs30 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pcs30.bin", 0x0000, 0x8000, CRC(d63bc611) SHA1(7ad9c200ec4ea586efe1e9e5b5ad9bc8ca03407d)) // YM2211-22712
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT         COMPANY   FULLNAME  FLAGS
SYST( 1984, pcs30,   0,      0,      pcs30,   pcs30,  pcs30_state, empty_init,  "Yamaha", "PCS-30", MACHINE_IS_SKELETON )
