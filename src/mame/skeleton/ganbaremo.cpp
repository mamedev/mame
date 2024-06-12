// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************************

    Skeleton driver for:
    "Ganbare Momotarou Oni Taiji" (がんばれ ももたろう おにたいじ). Electromechanical arcade by Shoken.
    https://www.youtube.com/watch?v=jxri_Wmoyfs

    DISPLAY PCB
     ______________________________________________________________________________________________
    |__               __________________    _________________    _____________    _____________   |
    ||:|             |·················|   |················|   |············|   |············|   |
    ||:|  ___________   ___________   ___________   ___________   ___________         ___________ |
    ||:| |TD62783AP_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|        |JW25N-DC12V |
    ||:|                                                                             |___________||
    ||:|  ___________   ___________   ___________   ___________   ___________     ___________     |
    ||_| |TD62783AP_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|    |JW25N-DC12V   __|
    |                                                                            |___________| |:||
    |     ___________      ___________              ___________                   ___________     |
    |    |SN74LS273N|     |LC3517BSL_|             |SN74LS138N|                  |JW25N-DC12V     |
    |      ________     ______________                                           |___________|  __|
    |     | Xtal  |    | EPROM       |                                            ___________  |:||
    |     | 20.000 MHz |_____________|                                           |JW25N-DC12V  |:||
    |     ___________  _________________           ___________   ___________     |___________|    |
    |    |SN74LS74AN|  | Z08040004PSC  |          |_74LS245N_|  |_74LS240N_|      ___________     |
    |                  |_______________|            ___________  ___________     |JW25N-DC12V     |
    |     ___________   ___________   ___________  |_TPL521-4_| |_TPL521-4_|     |___________|  __|
    |    |74HC4020AP|  |_SN74LS04N|  |_SN74LS02N|                                 ___________  |:||
    |                                                                            |JW25N-DC12V  |·||
    |                             SHOKEN M005 DISPLAY                            |___________|    |
    |                                              _____________   ________   _______________     |
    |                                             |············|  |·······|  |··············|     |
    |_____________________________________________________________________________________________|

    SOUND PCB
     ___________________________________________________________________________________________
    |         ___________   ________   ________      ______      ____________                  |
    |__      |··········|  |·······|  |·······|     |·····|     |···········|                : |
    ||:|                                                                                     : |
    ||:|        _________________     (o)<- Reset switch     ____                            : |
    |          | D8255AC-2      |                           8212CPA            o o o <- LEDs : |
    |          |________________|   _________  _________    _________________                __|
    |__         _________________  SN74LS138N  SN74LS00N   | D8255AC-2      |               |:||
    ||:|       | Z08040004PSC   |   _________  _________   |________________|               |:||
    ||:|       |________________| SN74LS139AN SN74LS138N    ________  _______   _________   |:||
    |       _________   _________   _________  _________   |DIPSx6_| |DIPSx4|  |ULN2803A|      |
    |__    SN74LS245N  SN74LS244N  |SN74LS10N SN74LS74AN    _________________   _________    __|
    ||:|    ___________  _________  _________  _________   | D8255AC-2      |  |ULN2803A|   |:||
    ||:|   | EPROM    |  SN74LS04N  SN74LS04N SN74LS74AN   |________________|               |:||
    |      |__________|  _________  _________  _________    _________________   _________   |:||
    | ____    _________  SN74LS02N  SN74LS32N SN74LS688N   | AY38910A/P     |  |ULN2803A|      |
    | BATT    LC3517BSL             _________  _________   |________________|   _________    __|
    |       _________   _________  SN74LS161AN SN74LS161AN                     | RELAY  |   |:||
    |      |74HC00AP|  SN74LS74AN           ____________            Resonator               |:||
    |  _________   _________   _________   | EPROM     |  _________  _________              |:||
    | SN74LS161AN |TC4020BP|   SN74LS09N   |___________| |TC4020BP| |OKI M5205  _________      |
    |                                                                          |uPC2500H|    __|
    |   Xtal 12.000 MHz        _________   _________      _________                         |:||
    |                         SN74LS161AN SN74LS161AN    |_LM324N_|    SHOKEN M904-A        |:||
    |__________________________________________________________________________________________|


TODO:
- figure out interrupts
- figure out inputs
- hook up MSM5205
- simulate mechanical pieces via layouts
- everything for the display PCB
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

#include "speaker.h"


// configurable logging
#define LOG_PORTS     (1U << 1)

// #define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)


namespace {

class ganbaremo_state : public driver_device
{
public:
	ganbaremo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_displaycpu(*this, "displaycpu")
	{
	}

	void ganbaremo(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_displaycpu;

	void main_program_map(address_map &map);
	void display_program_map(address_map &map);
	void main_io_map(address_map &map);
	void display_io_map(address_map &map);
};


void ganbaremo_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void ganbaremo_state::display_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void ganbaremo_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x60, 0x60).w("ay8910", FUNC(ay8910_device::address_w));
	map(0x61, 0x61).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	//map(0x80, 0x80).w
}

void ganbaremo_state::display_io_map(address_map &map)
{
	map.global_mask(0xff);

	// map(0x00, 0x00).w
	// map(0x20, 0x20).w
	// map(0x40, 0x40).w
	// map(0x80, 0x80).w
	// map(0xc0, 0xc0).r
}


static INPUT_PORTS_START( ganbaremo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2)

	PORT_START("IN2") // playing with these makes some sound effects / BGM play
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(3)

	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW0:6")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW1:4")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


void ganbaremo_state::ganbaremo(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // Guess
	m_maincpu->set_addrmap(AS_PROGRAM, &ganbaremo_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &ganbaremo_state::main_io_map);
	m_maincpu->set_periodic_int(FUNC(ganbaremo_state::irq0_line_hold), attotime::from_hz(60*4)); // TODO: proper IRQ ack

	Z80(config, m_displaycpu, 12_MHz_XTAL / 4); // Guess
	m_displaycpu->set_addrmap(AS_PROGRAM, &ganbaremo_state::display_program_map);
	m_displaycpu->set_addrmap(AS_IO, &ganbaremo_state::display_io_map);

	i8255_device &ppi1(I8255A(config, "ppi1")); // NEC D8255AC-2, inputs
	ppi1.in_pa_callback().set([this] () { LOGPORTS("%s: PPI1 port A in\n", machine().describe_context()); return ioport("IN0")->read(); });
	ppi1.in_pb_callback().set([this] () { LOGPORTS("%s: PPI1 port B in\n", machine().describe_context()); return ioport("IN1")->read(); });
	ppi1.in_pc_callback().set([this] () { LOGPORTS("%s: PPI1 port C in\n", machine().describe_context()); return ioport("IN2")->read(); });
	ppi1.out_pa_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI1 port A out %02x\n", machine().describe_context(), data); }); // doesn't seem to be written
	ppi1.out_pb_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI1 port B out %02x\n", machine().describe_context(), data); }); // doesn't seem to be written
	ppi1.out_pc_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI1 port C out %02x\n", machine().describe_context(), data); }); // doesn't seem to be written

	i8255_device &ppi2(I8255A(config, "ppi2")); // NEC D8255AC-2, comms to display CPU and/or MSM5205 commands?
	ppi2.in_pa_callback().set([this] () { LOGPORTS("%s: PPI2 port A in\n", machine().describe_context()); return uint8_t(0); }); // doesn't seem to be read
	ppi2.in_pb_callback().set([this] () { LOGPORTS("%s: PPI2 port B in\n", machine().describe_context()); return uint8_t(0); }); // doesn't seem to be read
	ppi2.in_pc_callback().set([this] () { LOGPORTS("%s: PPI2 port C in\n", machine().describe_context()); return uint8_t(0); }); // doesn't seem to be read
	ppi2.out_pa_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI2 port A out %02x\n", machine().describe_context(), data); }); // written
	ppi2.out_pb_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI2 port B out %02x\n", machine().describe_context(), data); }); // written
	ppi2.out_pc_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI2 port C out %02x\n", machine().describe_context(), data); }); // written

	i8255_device &ppi3(I8255A(config, "ppi3")); // NEC D8255AC-2, DSW
	ppi3.in_pa_callback().set([this] () { LOGPORTS("%s: PPI3 port A in\n", machine().describe_context()); return ioport("DSW0")->read(); });
	ppi3.in_pb_callback().set([this] () { LOGPORTS("%s: PPI3 port B in\n", machine().describe_context()); return ioport("DSW1")->read(); });
	ppi3.in_pc_callback().set([this] () { LOGPORTS("%s: PPI3 port C in\n", machine().describe_context()); return uint8_t(0); }); // doesn't seem to be read
	ppi3.out_pa_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI3 port A out %02x\n", machine().describe_context(), data); }); // doesn't seem to be written
	ppi3.out_pb_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI3 port B out %02x\n", machine().describe_context(), data); }); // doesn't seem to be written
	ppi3.out_pc_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI3 port C out %02x\n", machine().describe_context(), data); }); // doesn't seem to be written

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay(AY8910(config, "ay8910", 12_MHz_XTAL / 8)); // Guess
	ay.port_a_read_callback().set([this] () { LOGPORTS("%s: AY port A in\n", machine().describe_context()); return uint8_t(0); }); // doesn't seem to be read
	ay.port_b_read_callback().set([this] () { LOGPORTS("%s: AY port B in\n", machine().describe_context()); return uint8_t(0); }); // doesn't seem to be read
	ay.port_a_write_callback().set([this] (uint8_t data) { LOGPORTS("%s: AY port A out %02x\n", machine().describe_context(), data); }); // rarely written
	ay.port_b_write_callback().set([this] (uint8_t data) { LOGPORTS("%s: AY port B out %02x\n", machine().describe_context(), data); }); // rarely written
	ay.add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, "msm5205", XTAL(384'000));
}


ROM_START( ganbaremo )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "m005_p5.bin", 0x00000, 0x08000, CRC(dddb97a6) SHA1(47465bb2cd26ddd0f80c729ef3bb3b187b684d97) )

	ROM_REGION( 0x08000, "displaycpu", 0 )
	ROM_LOAD( "m005_xx.bin", 0x00000, 0x08000, CRC(42a7aa23) SHA1(6140f4a7769ab35cb32e3079adbee6468f3ce880) )

	ROM_REGION( 0x10000, "samples", 0)
	ROM_LOAD( "m005_v.bin",  0x00000, 0x10000, CRC(e3cb69a8) SHA1(a49878adae08d56d78d168367659ac322d7fb5eb) )
ROM_END

} // anonymous namespace


GAME( 1992?, ganbaremo, 0, ganbaremo, ganbaremo, ganbaremo_state, empty_init, ROT0, "Shoken", "Ganbare Momotarou Oni Taiji", MACHINE_IS_SKELETON_MECHANICAL ) // was advertised in 1992
