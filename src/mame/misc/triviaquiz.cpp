// license:BSD-3-Clause
// copyright-holders:Dirk Best, Ivan Vangelista
/*
    Trivia Quiz by Intermatic Manufacturing Ltd.
    1985

    main components:
    TMS9995NL
    TMS9129NL
    1 8-dips bank (another is not populated)
    2 x HM6116LP-3
    3 x ROMs (all sockets populated)

    Question daughter-cards contain just ROMs and some logic. They are plugged into the main board using a ZIF socket.

    4 question daughter-cards have been dumped, which contained different combinations of the following question sets (sic):
    Norway blanding 2
    Norway fotball 1
    Norway geografi 2
    Norway geografi 3
    Norway historie 1
    Norway kjendiser 2
    Norway pop-musikk 2
    Norway sport 2

    TODO:
    * sound (discrete?)
    * https://www.tvspels-nostalgi.com/pcb_unk.htm shows red background. Faulty harness or bad emulation?
    * several unmapped read and writes both in program and in IO map
    * proper fix for the 'faulty link problem, call attendant' message
*/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "video/tms9928a.h"
#include "screen.h"


namespace {

class triviaquiz_state : public driver_device
{
public:
	triviaquiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bankdev(*this, "bankdev")
	{ }

	void triviaquiz(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<tms9995_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;

	void prg_map(address_map &map) ATTR_COLD;
	void romboard_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void triviaquiz_state::prg_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x7fff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x6000, 0x6000).lw8(NAME([this] (uint8_t data) { m_bankdev->set_bank(data); }));
	map(0x8000, 0x8000).rw("vdp", FUNC(tms9129_device::vram_read), FUNC(tms9129_device::vram_write));
	map(0x8002, 0x8002).rw("vdp", FUNC(tms9129_device::register_read), FUNC(tms9129_device::register_write));
	map(0x8100, 0x8100).portr("IN0");
	map(0x8200, 0x8200).portr("IN1");
	map(0x8300, 0x8300).portr("IN2");
	map(0x8600, 0x8600).portr("DSW1");
	map(0x8700, 0x8700).nopr(); // DSW2, not populated
	map(0xa000, 0xa7ff).ram().share("nvram");
	map(0xe000, 0xe7ff).ram();
}

void triviaquiz_state::romboard_map(address_map &map)
{
	map(0x00000, 0x00000).lr8(NAME([]() -> uint8_t { return 0x3e; })); // 00
	map(0x20000, 0x27fff).rom().region("questions", 0x00000); // 10-13
	map(0x40000, 0x47fff).rom().region("questions", 0x08000); // 20-23
	map(0x60000, 0x67fff).rom().region("questions", 0x10000); // 30-33
	map(0x80000, 0x87fff).rom().region("questions", 0x18000); // 40-43
	map(0xa0000, 0xa7fff).rom().region("questions", 0x20000); // 50-53
}

void triviaquiz_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( triviaquiz )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // hack to get in game, if this and the two below aren't low it gets stuck 'faulty link problem, call attendant'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN4 ) // gives 10 credits
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) // gives 2 credits
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) // gives 5 credits
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) // gives 1 credit

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // these work also as start buttons
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // hack to get in game

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Insert / clear advertising") // gives the possibility to insert customized advertising
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // hack to get in game
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives 'bad switch'

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW1:7")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	// DSW2 not populated
INPUT_PORTS_END

void triviaquiz_state::machine_reset()
{
	//TODO: check these
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void triviaquiz_state::triviaquiz(machine_config &config)
{
	TMS9995(config, m_maincpu, 10.738635_MHz_XTAL); // TMS9995NL, 10.73864 XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &triviaquiz_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &triviaquiz_state::io_map);

	ADDRESS_MAP_BANK(config, m_bankdev, 0);
	m_bankdev->set_addrmap(AS_PROGRAM, &triviaquiz_state::romboard_map);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(20);
	m_bankdev->set_stride(0x2000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	tms9129_device &vdp(TMS9129(config, "vdp", 10.738635_MHz_XTAL)); //TMS9129NL, 10.73864 XTAL on one PCB, 10.70000 on another
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000); // verified

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

/***************************************************************************

  Game drivers

***************************************************************************/


ROM_START( triviaqz )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD( "ic6_qn_e1c.bin", 0x0000, 0x2000, CRC(b0e8c85a) SHA1(da61b6b0bdad6cea8e46f25bcdd9c1ecf21a6a4f) )
	ROM_LOAD( "ic5_qn_e2b.bin", 0x2000, 0x2000, CRC(662a8fb4) SHA1(7c021da698b57d2cd34e15780737be831c937f00) )
	ROM_LOAD( "ic4_qn_e3.bin",  0x4000, 0x2000, CRC(5fb41252) SHA1(2701e12b6d3da5e4bba1d224851a6bd42093e1d7) )

	ROM_REGION( 0x28000, "questions", 0 )
	ROM_LOAD( "norway_blanding_2.bin",   0x00000, 0x8000, CRC(d02b9b5a) SHA1(4f1bb69de0b5565cbafe8ed333e5d5f86b1f7dc2) )
	ROM_LOAD( "norway_geografi_2.bin",   0x08000, 0x8000, CRC(27011e41) SHA1(5dd87c9e4ce6c162210c4ab328ac75a2fd598b3b) )
	ROM_LOAD( "norway_kjendiser_2.bin",  0x10000, 0x8000, CRC(97dd1361) SHA1(00d196a2b3c7eaae55b16e4c68c56a95db46fd21) )
	ROM_LOAD( "norway_pop-musikk_2.bin", 0x18000, 0x8000, CRC(253b626c) SHA1(8bddf3a710ef03b06dde2ff432fa723cdd9f0103) )
	ROM_LOAD( "norway_sport_2.bin",      0x20000, 0x8000, CRC(be24021a) SHA1(98782f2916a13656867219f7132729747ae30d52) )
ROM_END

ROM_START( triviaqz2 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD( "ic6_qn_e1a.bin", 0x0000, 0x2000, CRC(529f5252) SHA1(1676d372b32efeb34f68d804fd630d6c8de15bc0) )
	ROM_LOAD( "ic5_qn_e2.bin",  0x2000, 0x2000, CRC(8e26e49e) SHA1(f99001b4a44754f7fa4e4535a1895c09922d2099) )
	ROM_LOAD( "ic4_qn_e3.bin",  0x4000, 0x2000, CRC(5fb41252) SHA1(2701e12b6d3da5e4bba1d224851a6bd42093e1d7) )

	ROM_REGION( 0x28000, "questions", 0 )
	ROM_LOAD( "norway_blanding_2.bin",   0x00000, 0x8000, CRC(d02b9b5a) SHA1(4f1bb69de0b5565cbafe8ed333e5d5f86b1f7dc2) )
	ROM_LOAD( "norway_fotball_1.bin",    0x08000, 0x8000, CRC(fb73f811) SHA1(0b993723a7bc1bb982e55c729538edc178e508ea) )
	ROM_LOAD( "norway_geografi_3.bin",   0x10000, 0x8000, CRC(8fbe0d46) SHA1(7c9a6b4896daeeea020e4e85b2348c46aab9451f) )
	ROM_LOAD( "norway_pop-musikk_2.bin", 0x18000, 0x8000, CRC(253b626c) SHA1(8bddf3a710ef03b06dde2ff432fa723cdd9f0103) )
	ROM_LOAD( "norway_historie_1.bin",   0x20000, 0x8000, CRC(b2fbce41) SHA1(9e94fc4efd03aff180a5bd94727f5be59647af52) )
ROM_END

} // anonymous namespace


GAME( 1985, triviaqz,         0, triviaquiz, triviaquiz, triviaquiz_state, empty_init, ROT0, "Intermatic Manufacturing", "Professor Trivia (set 1)",  MACHINE_NO_SOUND ) // or Trivia Video Quiz? Professor Trivia appears on more screens
GAME( 1985, triviaqz2, triviaqz, triviaquiz, triviaquiz, triviaquiz_state, empty_init, ROT0, "Intermatic Manufacturing", "Professor Trivia (set 2)",  MACHINE_NO_SOUND )
