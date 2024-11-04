// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, Tomasz Slanina
/***************************************************************************

  BIG 10
  Success, 1985.

  Driver by Angelo Salese, Roberto Fresca & Tomasz Slanina.

****************************************************************************

  Dumper Notes:

  Z80A
  XTAL is 21.?727
  YM2149
  8-position DSW x1
  RAM 6264 x1
  RAM 41464 x4
  unknown SDIP64 chip with welded heatsink! Might be a video chip or MCU?

****************************************************************************

  Dev Notes...

  - Guessed and hooked the Yamaha VDP (SDIP64 IC). Same VDP used on MSX systems.
  - Added v9938 stuff, interrupts, video start, machine reset, input ports,
    DIP switch, ym2149 interface, pre-defined main Xtal and derivatives for
    z80 and ym2149.
  - Added NVRAM, defined half of DIP switches bank (coinage & main game rate).
    Added inputs for coins A, B & C, payout, reset, and service mode.
  - Reorganized the driver.

****************************************************************************

  How to Play:

  - This is actually a Keno game (slightly modified Raffle/Bingo/Tombola game).
  - First off, select the bet amount with the BET button.
  - Then choose between "SELECT 10" button (pseudo-random) or user-defined
    numbers,by pressing the desired number with the numpad then "select"
    (enters the decimals first then the units, if three or more buttons
    are pressed the older pressed buttons are discarded, i.e. press 1234
    then SELECT, 1 and 2 are discarded).
  - Press "CANCEL ALL" to redo the numbering scheme.
  - Once that you are happy with it, press START to begin the extraction of
    winning numbers.
  - If you get at least 2-4 numbers out of 20 extracted numbers, you win a
    prize and you are entitled to do a big/small (double up) sub-game.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "machine/nvram.h"
#include "machine/ticket.h"

#include "screen.h"
#include "speaker.h"


namespace {

class big10_state : public driver_device
{
public:
	big10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_hopper(*this, "hopper")
		, m_in(*this, "IN%u", 1)
		, m_lamp(*this, "lamp")
	{ }

	void big10(machine_config &config);

protected:
	virtual void machine_start() override { save_item(NAME(m_mux_data)); m_lamp.resolve(); }

private:
	uint8_t m_mux_data = 0;
	required_device<cpu_device> m_maincpu;
	required_device<ticket_dispenser_device> m_hopper;
	required_ioport_array<6> m_in;
	output_finder<> m_lamp;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	uint8_t mux_r();
	void mux_w(uint8_t data);
};


/****************************************
*  Input Ports Demux & Common Routines  *
****************************************/

void big10_state::mux_w(uint8_t data)
{
	m_mux_data = ~data;
	m_hopper->motor_w(BIT(~data, 6));
	m_lamp = BIT(~data, 7); // maybe a coin counter?
}

uint8_t big10_state::mux_r()
{
	uint8_t result = 0xff;
	for (int b = 0; b < 6; b++)
		if (BIT(m_mux_data, b))
			result &= m_in[b]->read();

	return result;
}


/**************************************
*             Memory Map              *
**************************************/

void big10_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram().share("nvram");
	map(0xf000, 0xffff).ram();
}

void big10_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(big10_state::mux_r)); // present in test mode
	map(0x02, 0x02).portr("SYSTEM"); // coins and service
	map(0x98, 0x9b).rw("v9938", FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0xa0, 0xa1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xa2, 0xa2).r("aysnd", FUNC(ay8910_device::data_r)); // Dip-Switches routes here.
}


/**************************************
*            Input Ports              *
**************************************/

static INPUT_PORTS_START( big10 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Analyze Mode") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // in test mode, go to the game whilst keep pressed.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Number 0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Number 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Number 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Number 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Number 4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Number 5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Number 6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Number 7")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_8_PAD)     PORT_NAME("Number 8")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD )   PORT_CODE(KEYCODE_9_PAD)     PORT_NAME("Number 9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_F)         PORT_NAME("Flip Flop")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_Z)         PORT_NAME("Select 10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Cancel All")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )    PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BET )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // in test mode triggers a sound and screen turns black, hanging the game.

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Credit Limit?" )             PORT_DIPLOCATION("DSW1:8,7")
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPSETTING(    0x01, "1000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "9999" )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown" )                   PORT_DIPLOCATION("DSW1:6,5") // $C17E
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x30, 0x30, "Main Game Rate" )            PORT_DIPLOCATION("DSW1:4,3")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPSETTING(    0x20, "80%" )
	PORT_DIPSETTING(    0x30, "90%" )
	PORT_DIPNAME( 0xC0, 0xc0, "Coinage (A=1; B=5; C=10)" )  PORT_DIPLOCATION("DSW1:2,1")
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x40, "x2" )
	PORT_DIPSETTING(    0x80, "x5" )
	PORT_DIPSETTING(    0xC0, "x10" )

	// Unconnected, probably missing from the board
	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/**************************************
*           Machine Driver            *
**************************************/

void big10_state::big10(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 21.477272_MHz_XTAL; // Dumper notes poorly refers to a 21.?727 Xtal.

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK/6); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &big10_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &big10_state::main_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(40));

	// video hardware
	v9938_device &v9938(V9938(config, "v9938", MASTER_CLOCK));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(0x40000);
	v9938.int_cb().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ym2149_device &aysnd(YM2149(config, "aysnd", MASTER_CLOCK/12)); // guess
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.port_a_write_callback().set(FUNC(big10_state::mux_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.30);
}


/**************************************
*              ROM Load               *
**************************************/

ROM_START( big10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1", 0x0000, 0x4000, CRC(03e50455) SHA1(36834d35d037303e8b9e4ce950d22f11a52e9388) )
	ROM_LOAD( "2", 0x4000, 0x4000, CRC(b4626a5f) SHA1(a9b3b9575c657748a7f0b60ec2c7411dad0c83c1) )
	ROM_LOAD( "3", 0x8000, 0x4000, CRC(8d15da74) SHA1(0e114de6fcf79beac800575bfb739e6a6bf35660) )
ROM_END

} // anonymous namespace


/**************************************
*           Game Driver(s)            *
**************************************/

//    YEAR  NAME   PARENT    MACHINE   INPUT     STATE        INIT        ROT     COMPANY     FULLNAME   FLAGS
GAME( 1985, big10, 0,        big10,    big10,    big10_state, empty_init, ROT0,   "Success",  "Big 10",  MACHINE_SUPPORTS_SAVE )
