// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/*******************************************************************************

  Potten's Poker stealth with Breakout front game.
  Running in a PCB silkscreened "5981-A".
  Unknown manufacturer & year.

  Driver by Roberto Fresca & Grull Osgo.

********************************************************************************

  Specs...

  1x Z80A.
  1x Z80ACTC.
  1x AY-3-8910.
  1x TI TMS9129NL (Video Display Processor).
  1x Altera EP310-DC (EPLD).
  1x TBA820 (2 Watt Audio Amplifier).

  2x 27128 EPROMS.

  1x HM6116 2Kx8 SRAM.
  2x TMS4416-15 16Kx4 DRAM (for VDP).

  1x 8 DIP switches bank (Connected to AY8910 portA).
  1x 10 DIP switches bank (8 of them connected to EP310 EPLD).

  1x 10.738 Xtal.

  1x Empty socket between EPROMs and HM6116 RAM.  />_<\

  The following components made me suspect that there is some sort of
  TX-RX circuitry to connect with an external device for unknown purposes.
  Maybe a kind of device to trigger remote credits...

  1x HEF4528BP   ; Dual Monostable Multivibrator 3/15V.
  1x MM53200N    ; Encoder/Decoder. Digital Code Transmitter-Receiver System.
  1x L200C       ; 2.8 -> 36V. Voltage Regulator.
  1x SN94501N    ; Unknown... No Data.
  2x BC237B      ; NPN transistor.


  PCB layer:

  .----------------------------------------------------------------------------------------------------------.
  |                                                                                                          |
  |   .----------.                                                                                           |
  |   |DM74LS139N|                                                                                           |
  |   '----------'     .-------------.      .---------. .---------. .---------. .----------. .----------.    |
  |                    | Z8430AB1    |      |SN74LS32N| |SN74LS00N| | 74LS02  | |SN74LS74AN| |  74LS08  |    |
  |                    |     Z80ACTC |      '---------' '---------' '---------' '----------' '----------'    |
  |   .----------.     '-------------'                                                                       |
  |   |SN74LS74AN|                                                                                           |
  |   '----------'                                                     .----.                                |
  |                                                                    |DIP |                           .----'
  |   .---------.  .-------------------.    .---------------------.    |SWIT|                           |
  |   | 74LS175 |  | Z8400AB1          |    | AY-3-8910           |    |CHES|                           |
  |   '---------'  |           Z80ACPU |    |                 PSG |    | #1 |                           '----.
  |                '-------------------'    '---------------------'    '----'        .------------.      ====|
  |   .---------.                                                                    | T74LS244B1 |   2  ====|
  |   | 74LS273 |                                                                    '------------'   x  ====|
  |   '---------'                                                                                     2  ====|
  |                                                                                                   2  ====|
  |   .---------.                                                                                        ====|
  |   | SN7407N |     .-------------.       .------------.       .-----------.                        E  ====|
  |   '---------'     |TMS27128JL-20|       |TMS4416-15NL|       | EP-310 DC |       .------------.   D  ====|
  |                   | "BRK 01"    |       '------------'       '-++++++++--'       | T74LS244B1 |   G  ====|
  |   .---------.     '-------------'                            .-++++++++-----.    '------------'   E  ====|
  |   | SN7407N |                                                | DIP SWITCHES |                        ====|
  |   '---------'     .-------------.       .------------.       |     #2       |                     C  ====|
  |   .---------.     |TMS27128JL-20|       |TMS4416-15NL|       '--------------'                     O  ====|
  |   |MM53200N |     | "F"         |       '------------'                                            N  ====|
  |   '---------'     '-------------'                                                                 N  ====|
  |                                                                                                   E  ====|
  |   .----------.    .-------------.       .---------------------.                  .------------.   C  ====|
  |   |DM74LS139N|    | EMPTY       |       | TMS9129NL           |                  | SN94501N   |   T  ====|
  |   '----------'    |      SOCKET |       |                 VDP |                  '------------'   O  ====|
  |   .---------.     '-------------'       '---------------------'                                   R  ====|
  |   |SN74LS32N|                                                                                        ====|
  |   '---------'      .------------.       .------.                                                    .----'
  |   .---------.      | HM6116LP-4 |       | XTAL |                                                    |
  |   | SN7416N |      |            |       |10.738|                                                    |
  |   '---------'      '------------'       '------'                                                    '----.
  |   .----------.                                                                                           |
  |   |SN74LS74AN|                                                                                           |
  |   '----------'                                                                                           |
  |                                                                                                          |
  |                                                                                                          |
  |                                .---------.                                        .--|||--.              |
  |   .---------.                  | TBA820  |                                        | L200C |              |
  |   |HEF4528BP| .------..------. '---------'                                     .--+-------+--.           |
  |   '---------' |BC237B||BC237B|                                                 |  HEATSINK   |           |
  |               '-|||--''-|||--'                                                 '-------------'           |
  | 5981-A                                                                                                   |
  '----------------------------------------------------------------------------------------------------------'

********************************************************************************

  CTC connection...

    .-----v-----.
    |  CLK/TRG0 |--------.
    |    ZC/TO0 |--(NC)  |
    |           |        |
    |  CLK/TRG1 |--------+---(74LS74,CLK/2)---> CLK AY8910.
    |    ZC/TO1 |--(NC)  |
    |           |        |
    |  CLK/TRG2 |--------'
    |    ZC/TO2 |--(NC)      .------> 74LS244 (to be traced)
    |           |            |
    |  CLK/TRG3 |------------+------> Z80 /NMI.
    |           |
    |       IEI |--------(1K RES)---> Z80 /BUSRQ.
    |       IEO |--(NC)
    |      /INT |-------------------> Z80 /INT.
    '-----------'

********************************************************************************

  Game notes...

  This is a STEALTH game. It's a poker game hidden in another front game.
  The goal is to have full operative gambling games in clandestine places
  like bars, social clubs, and other locations where they are forbidden.

  Obviously the front game is operative and pretends to be the main game.
  But some inputs combinations trigger the real game hidden inside.

  How to start:

  At begining, this machine needs to be turned on keeping the joystick UP,
  to initialize the dual game system, otherwise only could access the front game.
  To do it in MAME, you need to RESET (F3) the game keeping UP pressed.

  Then...

  The game will boot in breakout mode. You can coin with key 5 as usual, start
  the game with START 1 (key 1), use the arrows to drive the pad in all directions,
  and play a 3-lives blockout game.

  To enter the hidden poker game, you must coin through the hidden coin/notes
  slot (key Q). Once you have credits, bet with START 2 (key 2) and then draw
  the cards with START 1 (key 1).

  Use the LEFT/RIGHT keys to move the cursor horizontally, and hold the desired
  card using DOWN (you can cancel the held card using UP). Once finished, just
  press START 1 again for deal/draw.

  If you get a winning hand, the game will ask you if you want to double or take.
  Use UP for double-up, and DOWN for take score.

  The double-up feature will allow you to choose one between BIG/SMALL, on the
  next card to draw. Use LEFT for BIG or RIGHT for SMALL, then press START 1 to
  draw the card.

  DIP switches could set the coinage and maximum bet.

  As soon as you get out of credits or payout, the game will go to breakout mode.
  You can payout only amounts of credits multiples of 10.

  If you *still* have credits, resetting the machine (F3) alone will switch the
  machine to blockout mode, and resetting with UP will switch to poker game mode.

  Whilst you're in poker game mode, you have two service test buttons:

  TEST 1 (key 9) will show you a sort of percentage screen, and using RIGHT, you
  can see the bookkeeping and statistics. Press payout (key W) to exit.
  Using LEFT you're entering in the TEST MODE (also TEST 2 key does the same).

  TEST 2 (key 0) will enter the TEST MODE, where you can test the game buttons
  and DIP switches. There are some inputs used by the code for unknown purposes
  that are not present in this mode. Press START 2 (key 2) + UP to exit the mode.

********************************************************************************

  TODO:

  - Trace/analysis of inputs out of test mode, but used by the code
    (Port 90h D0-D3)
  - Identify the writes to ports 80h, 90h & A0h.

*******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK        XTAL(10'738'000)
#define VDP_CLOCK           MASTER_CLOCK
#define CPU_CLOCK           MASTER_CLOCK / 3
#define PSG_CLOCK           MASTER_CLOCK / 3 / 2

#define VDP_MEM             0x4000    // 16k RAM, provided by 2x TMS4416.


class pokerout_state : public driver_device
{
public:
	pokerout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void pokerout(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


/*************************************************
*               Memory Map / I/O                 *
*************************************************/

void pokerout_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0xc000, 0xc7ff).ram().share("nvram");        // battery backed RAM
}

void pokerout_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("vdp", FUNC(tms9129_device::vram_read), FUNC(tms9129_device::vram_write));
	map(0x01, 0x01).rw("vdp", FUNC(tms9129_device::register_read), FUNC(tms9129_device::register_write));
	map(0x40, 0x41).w("psg", FUNC(ay8910_device::address_data_w));
	map(0x42, 0x42).r("psg", FUNC(ay8910_device::data_r));
	map(0x80, 0x80).portr("IN0");
	map(0x90, 0x90).portr("IN1");
	map(0xa0, 0xa0).portr("IN2");
	map(0xc0, 0xc3).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

}
/*
[:psg] warning: unmapped write 02 to AY-3-8910A PSG Port B (disconnected) ----> Hopper or counter pulse x10
[:maincpu] ':maincpu' (18EA): unmapped io memory write to 00B0 = 01 & FF  ----> Hopper or counter pulse x10
[:maincpu] ':maincpu' (18F0): unmapped io memory write to 00B0 = 00 & FF
[:psg] warning: unmapped write 00 to AY-3-8910A PSG Port B (disconnected)

other writes found...

  80  W  ---> 07 0F 0B
  90  W  ---> 66 7F
  A0  W  ---> 00 01

*/


/*************************************************
*            Input Ports Definitions             *
*************************************************/

static INPUT_PORTS_START(pokerout)

	PORT_START("IN0")  // 80h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_NAME("Right / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_NAME("Down / Hold / Take")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Spare C / Poker Credits")  PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Reserved") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Spare P / Poker Payout")  PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test 2")   PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )  PORT_NAME("Start 1 / Draw")

	PORT_START("IN1")  // 90h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN1-01") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN1-02") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN1-04") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN1-08") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY    PORT_NAME("Up / Cancel / Double-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY  PORT_NAME("Left / Big")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )  PORT_NAME("Start 2 / Bet")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test 1") PORT_CODE(KEYCODE_9)

	PORT_START("IN2")  // A0h
//  Program does complex operations with this pull-up/down mask to boot the game.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")  // PSG port A
	PORT_DIPNAME(0x03, 0x03, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(0x00, "10 Credits" )
	PORT_DIPSETTING(0x01, "20 Credits" )  // code @ $2107 is ok, but this option isn't working...
	PORT_DIPSETTING(0x02, "50 Credits" )
	PORT_DIPSETTING(0x03, "100 Credits" )
	PORT_DIPNAME(0x0c, 0x0c, "Maximum Bet" )       PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(0x0c, "10" )
	PORT_DIPSETTING(0x08, "20" )
	PORT_DIPSETTING(0x04, "50" )
	PORT_DIPSETTING(0x00, "100" )
	PORT_DIPNAME(0x10, 0x10, "Arcade Mode")        PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))     PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown))     PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))     PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END


/*************************************************
*        Machine Start & Reset Routines          *
*************************************************/

void pokerout_state::machine_start()
{
}

void pokerout_state::machine_reset()
{
}


/*********************************************
*      Daisy Chain Interrupts Interface      *
*********************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


/*************************************************
*                Machine Config                  *
*************************************************/

void pokerout_state::pokerout(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", CPU_CLOCK));
	maincpu.set_daisy_config(daisy_chain);
	maincpu.set_addrmap(AS_PROGRAM, &pokerout_state::mem_map);
	maincpu.set_addrmap(AS_IO, &pokerout_state::io_map);

	z80ctc_device& ctc(Z80CTC(config, "ctc", CPU_CLOCK));
	ctc.intr_callback().set_inputline(maincpu, INPUT_LINE_IRQ0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	tms9129_device &vdp(TMS9129(config, "vdp", VDP_CLOCK));
	vdp.set_screen("screen");
	vdp.set_vram_size(VDP_MEM);
//  int line no connected, so no callback.
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", PSG_CLOCK));
	psg.port_a_read_callback().set_ioport("DSW");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************************
*                 ROMs Loading                   *
*************************************************/

ROM_START(pokerout)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("brk01_tms128a.6f", 0x0000, 0x4000, CRC(ccf2e9d7) SHA1(b48630d1e6c223aacf4f785b2a70c5c3ed781a51))
	ROM_LOAD("f_tms128a.8f",     0x4000, 0x4000, CRC(57a7bff2) SHA1(a466c881fcd2a339960936e21da7e2079e7d75ca))
ROM_END

} // anonymous namespace


/***************************************************************************
*                              Game Drivers                                *
***************************************************************************/

//    YEAR  NAME      PARENT   MACHINE   INPUT     STATE           INIT        ROT    COMPANY      FULLNAME                                          FLAGS
GAME( 19??, pokerout, 0,       pokerout, pokerout, pokerout_state, empty_init, ROT0, "<unknown>", "Potten's Poker stealth with Breakout front game", 0 )
