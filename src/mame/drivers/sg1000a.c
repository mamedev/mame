// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*********************************************************
Sega hardware based on their SG-1000 console
Driver by Tomasz Slanina  analog [at] op.pl


Supported games :
- Champion Boxing
- Champion Pro Wrestling
- Doki Doki Penguin Land

Memory map :
0x0000 - 0xBFFF ROM
0xC000 - 0xC3FF RAM

CPU:
Z80 A                    3.57954 MHz (Champion Boxing)
315-5114 (encrypted Z80) 3.57954 MHz (Champion Pro Wrestling)

8255 for I/O port work
3 Eproms for program and graphics
TMM2064 for program RAM
TMS9928 for graphics ( 3.57954 MHz? )
8 8118 dynamic RAMs for the graphics
74LS139 and 74LS32 for logic gating
ULN2003 for coin counter output
76489 for music
7808 voltage regulator to a transistorized circuit for TV output
secondary crystal, numbers unknown for the TMS9928

--

Doki Doki Penguin Land
Sega, 1985

PCB Layout
----------

834-5492
|---------------------------------------------|
|    CN3          CN2          CN4            |
|                                  DSW(4)     |
|3.579545MHz                          TD62003 |
|   74HC04     8255             SN76489       |
|                                         CN5 |
|                                     VR5     |
| 6116         Z80A                           |
|                                  LA4460     |
|            74LS32 74LS139                   |
|EPR-7358.IC3                                 |
|              TMS9928                    CN1 |
|                                             |
|                              10.7386MHz     |
|           MB8118  MB8118                    |
|EPR-7357.IC2                                 |
|           MB8118  MB8118                    |
|                                        7808 |
|           MB8118  MB8118     VR4            |
|EPR7356.IC1                                  |
|    7805   MB8118  MB8118       VR3  VR2  VR1|
|---------------------------------------------|
Notes:
      All IC's shown
      CN1/2/3/4/5   - Connectors for power/video/sound/controls
      VR1/2/3       - Potentiometers for RGB adjustment
      VR4           - Potentiometer for Horizontal Sync adjustment
      VR5           - Potentiometer for volume
      TMS9928 clock - 2.68465 [10.7386/4]
      Z80 clock     - 3.579545MHz
      VSync         - 60Hz
      HSync         - 15.58kHz

Doki Doki Penguinland Dip Switches (DIP4) and Pinout

Option               1     2     3     4
------------------------------------------
1coin 1credit        off   off
1c 2cr               on    off
1c 3cr               off   on
2c 1cr               on    on
attract sound  yes               on
               no                off
not used                               off

Hold Service + 1P start = test mode


CN1               CN2                       CN3
1 Red             1 Player 1 UP             1 Player 2 UP
2 Green           2 Player 1 DOWN           2 Player 2 DOWN
3 Blue            3 Player 1 LEFT           3 Player 2 LEFT
4 Gnd             4 Player 1 RIGHT          4 Player 2 RIGHT
5 Sync            5 Player 1 Button B       5 Player 2 Button B
6 Key             6 Player 1 Button A       6 Player 2 Button A
7                 7 Key                     7 Service
8 Speaker +       8 Player 1 START          8 Coin
9 Speaker -       9 Player 2 START          9 Key
10                10 Gnd                    10 Gnd


CN4               CN5
1                 1 +5V
2                 2 +5V
3                 3 +5V
4                 4 Gnd
5 Key             5 Gnd
6 Coin Meter      6 Gnd
7                 7 +12V
8                 8 Key
9 +5V             9 +12V
10 Gnd            10 Gnd

******************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "machine/i8255.h"
#include "machine/segacrpt.h"


class sg1000a_state : public driver_device
{
public:
	sg1000a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	DECLARE_WRITE_LINE_MEMBER(vdp_interrupt);
	DECLARE_WRITE8_MEMBER(sg1000a_coin_counter_w);
	DECLARE_DRIVER_INIT(sg1000a);
	DECLARE_DRIVER_INIT(chwrestl);
	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;
};


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, sg1000a_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_MIRROR(0x400)
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, sg1000a_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0xbfff) AM_ROM AM_REGION("maincpu", 0x8000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, sg1000a_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_DEVWRITE("snsnd", sn76489_device, write)
	AM_RANGE(0xbe, 0xbe) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xbf, 0xbf) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xdc, 0xdf) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
ADDRESS_MAP_END

/*************************************
 *
 *  Generic Port definitions
 *
 *************************************/

static INPUT_PORTS_START( sg1000 )
	PORT_START("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW")
	PORT_BIT ( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
INPUT_PORTS_END

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( chwrestl )
	PORT_INCLUDE( sg1000 )

	PORT_MODIFY("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("P2")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( chboxing )
	PORT_INCLUDE( sg1000 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dokidoki )
	PORT_INCLUDE( sg1000 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

WRITE_LINE_MEMBER(sg1000a_state::vdp_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

WRITE8_MEMBER(sg1000a_state::sg1000a_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 0x01);
}

/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( sg1000a, sg1000a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sg1000a_state, sg1000a_coin_counter_w))

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(sg1000a_state, vdp_interrupt))

	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76489, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sg1000ax, sg1000a )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( chwrestl )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "5732",   0x0000, 0x4000, CRC(a4e44370) SHA1(a9dbf60e77327dd2bec6816f3142b42ad9ca4d09) ) /* encrypted */
	ROM_LOAD( "5733",   0x4000, 0x4000, CRC(4f493538) SHA1(467862fe9337497e3cdebb29bf28f6cfe3066ccd) ) /* encrypted */
	ROM_LOAD( "5734",   0x8000, 0x4000, CRC(d99b6301) SHA1(5e762ed45cde08d5223828c6b1d3569b2240462c) )
ROM_END

ROM_START( chboxing )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb6105.bin", 0x0000, 0x4000, CRC(43516f2e) SHA1(e3a9bbe914b5bfdcd1f85ca5fae922c4cae3c106) )
	ROM_LOAD( "cb6106.bin", 0x4000, 0x4000, CRC(65e2c750) SHA1(843466b8d6baebb4d5e434fbdafe3ae8fed03475) )
	ROM_LOAD( "cb6107.bin", 0x8000, 0x2000, CRC(c2f8e522) SHA1(932276e7ad33aa9efbb4cd10bc3071d88cb082cb) )
ROM_END

ROM_START( dokidoki )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7356.ic1",   0x0000, 0x4000, CRC(95658c31) SHA1(f7b5638ab1b8b244b189317d954eb37b51923791) )
	ROM_LOAD( "epr-7357.ic2",   0x4000, 0x4000, CRC(e8dbad85) SHA1(9f13dafacee370d6e4720d8e27cf889053e79eb3) )
	ROM_LOAD( "epr-7358.ic3",   0x8000, 0x4000, CRC(c6f26b0b) SHA1(3753e05b6e77159832dbe88562ba7a818120d1a3) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(sg1000a_state,sg1000a)
{
}

DRIVER_INIT_MEMBER(sg1000a_state,chwrestl)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x28,0x08,0xa8,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...0...0...0 */
		{ 0x28,0x08,0xa8,0x88 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...0...0...1 */
		{ 0x88,0x80,0x08,0x00 }, { 0x88,0x08,0x80,0x00 },   /* ...0...0...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...0...1...1 */
		{ 0x28,0x08,0xa8,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...0...0 */
		{ 0x88,0x80,0x08,0x00 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...0...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0x88,0x08,0x80,0x00 },   /* ...0...1...1...0 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...1...1...1 */
		{ 0x80,0xa0,0x00,0x20 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...0...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...0...0...1 */
		{ 0x80,0xa0,0x00,0x20 }, { 0x80,0xa0,0x00,0x20 },   /* ...1...0...1...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x80,0xa0,0x00,0x20 },   /* ...1...0...1...1 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...1...0...0 */
		{ 0x80,0xa0,0x00,0x20 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...1...0...1 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0x80,0xa0,0x00,0x20 },   /* ...1...1...1...0 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0xa0,0x80,0xa8,0x88 }    /* ...1...1...1...1 */
	};

	DRIVER_INIT_CALL(sg1000a);
	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);
}

/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, chboxing, 0, sg1000a,  chboxing, sg1000a_state, sg1000a,  ROT0, "Sega", "Champion Boxing", 0 )
GAME( 1985, chwrestl, 0, sg1000ax, chwrestl, sg1000a_state, chwrestl, ROT0, "Sega", "Champion Pro Wrestling", 0 )
GAME( 1985, dokidoki, 0, sg1000a,  dokidoki, sg1000a_state, sg1000a,  ROT0, "Sega", "Doki Doki Penguin Land", 0 )
