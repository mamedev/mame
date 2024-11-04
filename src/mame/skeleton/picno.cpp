// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************************

Konami Picno and Picno2

Skeleton driver started on 2017-11-30, can be claimed by anyone interested.

Information provided by Team Europe.

Chips: HD6435328F10 (H8/532 CPU with inbuilt ROM), HN62334BP (27c040 ROM), Konami custom chip 054715 (rectangular 100 pins),
       HM538121JP-10, M514256B-70J, OKI M6585.
Crystals: D200L2 (Y1) and D214A3 (Y2), frequencies unknown.

The hardware of the Picno 1 and Picno 2 is completely the same. The Picno 1 has an Audio-Line-Out, which the Picno 2 does not have.

Maskrom of PICNO 1: RX001-Z8-V3J
Maskrom of PICNO 2: RX001-Z8-V4J

The size of the address space and other things is controlled by the 3 mode pins. It's assumed we are in Mode 4.

Can't do anything until the internal ROM is dumped.

******************************************************************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8532.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"

//#include "sound/multipcm.h"
//#include "screen.h"
//#include "speaker.h"


namespace {

class picno_state : public driver_device
{
public:
	picno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void picno(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8532_device> m_maincpu;
};

void picno_state::mem_map(address_map &map)
{
	//map(0x00000, 0x07fff).rom().region("roms", 0); // 32kb internal rom
	//map(0x0fb80, 0x0ff7f).ram(); // internal ram
	//map(0x0ff80, 0x0ffff); // internal controls
	map(0x10000, 0x8ffff).rom().region("roms", 0); // guess
}

static INPUT_PORTS_START( picno )
INPUT_PORTS_END

void picno_state::picno(machine_config &config)
{
	/* basic machine hardware */
	HD6435328(config, m_maincpu, 20'000'000); // TODO: clock is a guess, divided by 2 in the cpu
	m_maincpu->set_addrmap(AS_PROGRAM, &picno_state::mem_map);

	//SPEAKER(config, "lspeaker").front_left(); // no speaker in the unit, but there's a couple of sockets on the back
	//SPEAKER(config, "rspeaker").front_right();
	//sound.add_route(0, "lspeaker", 1.0);
	//sound.add_route(1, "rspeaker", 1.0);

	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "picno_cart");

	SOFTWARE_LIST(config, "cart_list").set_original("picno");
}

ROM_START( picno )
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "hd6435328f10.u5", 0x00000, 0x08000, NO_DUMP ) // internal rom

	ROM_REGION(0x80000, "roms", 0)
	ROM_LOAD( "rx001-z8-v3j.u2", 0x00000, 0x80000, CRC(e3c8929d) SHA1(1716f09b0a594b3782d257330282d77b6ca6fa0d) ) //HN62334BP
ROM_END

ROM_START( picno2 )
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "hd6435328f10.u5", 0x00000, 0x08000, NO_DUMP ) // internal rom

	ROM_REGION(0x80000, "roms", 0)
	ROM_LOAD( "rx001-z8-v4j.u2", 0x00000, 0x80000, CRC(ae89a9a5) SHA1(51ed458ffd151e19019beb23517263efce4be272) ) //HN62334BP
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME   FLAGS
CONS( 1993, picno,  0,      0,      picno,   picno, picno_state, empty_init, "Konami", "Picno",   MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
CONS( 1993, picno2, 0,      0,      picno,   picno, picno_state, empty_init, "Konami", "Picno 2", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
