// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for redemption games by GameMasters, Inc.

    Not much is known about this Atlanta-based company and its games, aside
    from a number of flyers.

****************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
//#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class gamemasters_state : public driver_device
{
public:
	gamemasters_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void gmsshoot(machine_config &config);
private:
	void output_1100(uint8_t data);
	void output_1300(uint8_t data);
	void output_1520(uint8_t data);
	void output_1540(offs_t offset, uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void gamemasters_state::output_1100(uint8_t data)
{
	// IRQ ack?
	logerror("%s: Writing %02X to 1100\n", machine().describe_context(), data);
}

void gamemasters_state::output_1300(uint8_t data)
{
	logerror("%s: Writing %02X to 1300\n", machine().describe_context(), data);
}

void gamemasters_state::output_1520(uint8_t data)
{
	logerror("%s: Writing %02X to 1520\n", machine().describe_context(), data);
}

void gamemasters_state::output_1540(offs_t offset, uint8_t data)
{
	logerror("%s: Writing %02X to 1540 + %X\n", machine().describe_context(), data, offset);
}

void gamemasters_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x1100, 0x1100).w(FUNC(gamemasters_state::output_1100));
	map(0x1300, 0x1300).w(FUNC(gamemasters_state::output_1300));
	map(0x1400, 0x1400).portr("IN0");
	map(0x1500, 0x1500).portr("IN1");
	map(0x1520, 0x1520).w(FUNC(gamemasters_state::output_1520));
	map(0x1540, 0x1540).select(0x20).w(FUNC(gamemasters_state::output_1540));
	map(0x1600, 0x1601).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x1601, 0x1601).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


void gamemasters_state::gmsshoot(machine_config &config)
{
	R65C02(config, m_maincpu, 4_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &gamemasters_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02B-25

	// LCD video?

	SPEAKER(config, "mono").front_center();
	ay8912_device &aysnd(AY8912(config, "aysnd", 4_MHz_XTAL / 4));
	aysnd.port_a_read_callback().set_ioport("DSW");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.30);
}


static INPUT_PORTS_START(gmsshoot)
	// Flyer: "Sharpshooter can be set to have quarters or tokens in, and/or quarters or tokens out."
	// "Ticket Dispenser" and "Optional Dollar Bill Acceptor" are also mentioned as features.
	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	// SW1 also exists, but appears to be just a DS1232 reset button.

	PORT_START("IN0")
	PORT_BIT(0x01, 0x01, IPT_UNKNOWN)
	PORT_BIT(0x02, 0x02, IPT_UNKNOWN)
	PORT_BIT(0x04, 0x04, IPT_UNKNOWN)
	PORT_BIT(0x08, 0x08, IPT_UNKNOWN)
	PORT_BIT(0x10, 0x10, IPT_UNKNOWN)
	PORT_BIT(0x20, 0x20, IPT_UNKNOWN)
	PORT_BIT(0x40, 0x40, IPT_UNKNOWN)
	PORT_BIT(0x80, 0x80, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, 0x01, IPT_UNUSED)
	PORT_BIT(0x02, 0x02, IPT_UNKNOWN)
	PORT_BIT(0x04, 0x04, IPT_UNKNOWN)
	PORT_BIT(0x08, 0x08, IPT_UNKNOWN)
	PORT_BIT(0x10, 0x10, IPT_UNKNOWN)
	PORT_BIT(0x20, 0x20, IPT_UNKNOWN)
	PORT_BIT(0x40, 0x40, IPT_UNKNOWN)
	PORT_BIT(0x80, 0x80, IPT_UNKNOWN)
INPUT_PORTS_END


ROM_START(gmsshoot)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("sharp_shooter__u20_2764.bin", 0x0000, 0x2000, CRC(902bd63b) SHA1(606e9d1083677d7eb90ad3626a6340238a260253)) // M2764AFI
ROM_END

} // anonymous namespace


GAME(1989, gmsshoot, 0, gmsshoot, gmsshoot, gamemasters_state, empty_init, ROT0, "GameMasters", "Sharpshooter (coin pusher)", MACHINE_IS_SKELETON_MECHANICAL) // flyer and PCB dated 1988, but program strings claim 1989 copyright
