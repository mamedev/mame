// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*

    Skeleton driver for Yamaha PortaSound PS-400 keyboard

    IC1: Yamaha YM1034 (KAP2) "key assigner"
    IC2: Yamaha YM1019 (GE4) sound chip
    IC16: Toshiba TMP80C39P-6
    IC18: Toshiba TMM2332P mask ROM

    Service manual/schematic: https://elektrotanya.com/yamaha_ps_400.pdf/download.html

*/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "speaker.h"

namespace {

class ps400_state : public driver_device
{
public:
	ps400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ps400(machine_config &config);

private:
	void ps400_map(address_map &map) ATTR_COLD;

	required_device<i8039_device> m_maincpu;
};


void ps400_state::ps400_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}


void ps400_state::ps400(machine_config &config)
{
	I8039(config, m_maincpu, 6'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps400_state::ps400_map);

	SPEAKER(config, "speaker").front_center();
}


INPUT_PORTS_START(ps400)
INPUT_PORTS_END


ROM_START( ps400 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "yamaha_igo83803a_3997-7802.ic18", 0x0000, 0x1000, CRC(251e6572) SHA1(cd1a0bf1cf10eb2cb3b6fbaf14faa62ccb682eb2))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT         COMPANY   FULLNAME  FLAGS
SYST( 1982, ps400,   0,      0,      ps400,   ps400,  ps400_state, empty_init,  "Yamaha", "PS-400", MACHINE_IS_SKELETON )
