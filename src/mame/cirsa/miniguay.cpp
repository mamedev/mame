// license:BSD-3-Clause
// copyright-holders:

/*
    Mini Guay (c) 1986 Cirsa

    Slot machine.

    Main components:
    1 x 8085
    1 x AY-3-8910
    1 x 8256A (MUART, unemulated)
    2 x 8155
    2 x 8-dip banks (between AY and MUART)
    1 x 6.144MHz Osc
    battery backed RAM

   ________________________________________________
  |C |  ________   ________                       |
  |O |  ULN2064B  |_7417N_|                       |
  |N |  ___________________           TDA2002     |
  |N | | 8155P            |  ________             |
  |__| |__________________| |74HC04P|             |
  |     ___________________  _________            |
  |    | AY-3-8910        | |74LS240N|            |
  |    |__________________|  ________             |__
  |     ________   ________ |74LS08N|           ___ -|
  |    |_DIPSx8|  |_DIPSx8|  ________          |C | -|
  |     ___________________  74HC32B1          |O | -|
  |    | SAB8256A         |  ________          |N | -|
  |    |__________________|  74LS139B1         |N | -|
  |                          _________         |__| -|
  |                         |_PAT_031|              -|
  |                          _________              -|
  |     ___________________ |l4LS373_|              -|
  |    | D8085AHC         |                        __|
  |    |__________________|                       |
  |                                               |
  |     XTAL                 ________________     |
  |   6.144 MHz             | 27256         |     |
  |                ________ |_______________|     |
  |               |74HC00B1   _______________     |
  |                ________  | D446C-2      |     |
  |     ________  |74LS04P_| |______________|     |
  |    |74LS393N                                  |
  |     ________         _____     BATT           |
  |    HEF4060BP         LM393N    3.6V           |
  |     ___________________          _____________|
  |    | 8155P            |         |
  |    |__________________|         |
  |     ________   ________         |
  | __ |ULN2003A   74LS145N ________|
  ||C|                      74LS247N|
  ||O|                              |
  ||N|                              |
  ||N|    PCB 860607-2A             |
  |_________________________________|

    Two different PCBs were found with same components, albeit some from different producers.
    The coin acceptor is driven by a MCU (unknown type).

    The dumped version (Mini Guay VD) uses plastic displays with light bulbs, but there's a different
    one (undumped) called "Mini Guay VR" (VR stands for "Version Rodillos") or just "Mini Guay" with
    reels instead, with an additional PCB for reels control (8031 + 2764 EPROM).
*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
//#include "machine/i8256.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


namespace {

class miniguay_state : public driver_device
{
public:
	miniguay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_psg(*this, "psg")
		, m_psg_control(0)
		, m_psg_data(0)
	{ }

	void miniguay(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void psg_control_w(u8 data);
	u8 psg_db_r();
	void psg_db_w(u8 data);

	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_psg;

	void main_map(address_map &map) ATTR_COLD;

	u8 m_psg_control;
	u8 m_psg_data;
};

void miniguay_state::machine_start()
{
	save_item(NAME(m_psg_control));
	save_item(NAME(m_psg_data));
}

void miniguay_state::psg_control_w(u8 data)
{
	if (BIT(m_psg_control, 6) && !BIT(data, 6))
		m_psg->data_address_w(BIT(m_psg_control, 7), m_psg_data);
	m_psg_control = data;
}

u8 miniguay_state::psg_db_r()
{
	if ((m_psg_control & 0xc0) == 0x80)
		return m_psg->data_r();
	else
		return m_psg_data;
}

void miniguay_state::psg_db_w(u8 data)
{
	m_psg_data = data;
}

void miniguay_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();
	//map(0x9400, 0x940f).rw("muart1", FUNC(i8256_device::read), FUNC(i8256_device::write));
	map(0x9900, 0x9907).rw("i8155_1", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x9d00, 0x9d07).rw("i8155_2", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	//map(0xb400, 0xb40f).rw("muart2", FUNC(i8256_device::read), FUNC(i8256_device::write));
	map(0xb409, 0xb409).rw(FUNC(miniguay_state::psg_db_r), FUNC(miniguay_state::psg_db_w));
	map(0xb900, 0xb907).rw("i8155_3", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xbd00, 0xbd07).rw("i8155_4", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc000, 0xc000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xd000, 0xd000).nopw(); // resets something?
}

static INPUT_PORTS_START( miniguay )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void miniguay_state::miniguay(machine_config &config)
{
	// basic machine hardware
	I8085A(config, m_maincpu, 6.144_MHz_XTAL); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &miniguay_state::main_map);

	WATCHDOG_TIMER(config, "watchdog");

	//I8256(config, "muart1", 6.144_MHz_XTAL / 2);
	//I8256(config, "muart2", 6.144_MHz_XTAL / 2);

	I8155(config, "i8155_1", 6.144_MHz_XTAL / 2); // divider not verified

	I8155(config, "i8155_2", 6.144_MHz_XTAL / 2); // divider not verified

	I8155(config, "i8155_3", 6.144_MHz_XTAL / 2); // divider not verified

	i8155_device &i8155_4(I8155(config, "i8155_4", 6.144_MHz_XTAL / 2)); // divider not verified
	i8155_4.out_pb_callback().set(FUNC(miniguay_state::psg_control_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_psg, 6.144_MHz_XTAL / 4); // divider not verified
	//m_psg->port_a_read_callback().set_ioport("DSW1");
	//m_psg->port_b_read_callback().set_ioport("DSW2");
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( miniguay )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a21t_b-82.bin", 0x0000, 0x8000, CRC(04865da9) SHA1(78cf41d8428eb67ae40e764494ac03d45762500a) ) // Dumped from two different PCBs

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "pat_031_pal16r4.bin", 0x000, 0x104, NO_DUMP )
ROM_END

} // Anonymous namespace


GAME( 1986, miniguay, 0, miniguay, miniguay, miniguay_state, empty_init, ROT0, "Cirsa", "Mini Guay VD", MACHINE_IS_SKELETON_MECHANICAL ) // VD stands for "Version Displays".
