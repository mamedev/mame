// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/***************************************************************************

  NOTE CHANCE
  Banpresto, 1995

  Driver by Roberto Fresca.

  System type:       Screenless prize machine.
  Manufacturer:      BANDAI NAMCO Entertainment (Banpresto Label).
  Release:           1995.
  Number of players: 1 (single player).

  Reference video: https://www.youtube.com/watch?v=TSIWO75udL8

****************************************************************************

  Hardware Notes...

  Board etched PT-877.

  CPU:
  1x TOSHIBA TMPZ84C00AP-6 (Z80).

  Sound Device:
  1x OKI6295 or similar...

  ROM:
  1x 27C040 (SND ROM).
  1x 27C256 (PRG ROM).

  RAM:
  1x BR6265A-10LL (8K x 8 CMOS SRAM).

  Backup System:
  1x Fujitsu MB3780A (Battery Backup IC)
  1x Unknown battery.

  Clock:
  1x Xtal: 8.44800 MHz.

  Other:
  1x 8-DIP switches bank.
  1x volume pot.
  1x 60-pins female connector. (CN2: 3 rows of 21, 18, 21)
  1x 10-pins male connector (CN1: for power supply)


  CN1 connector:

  01- DC +12V.
  02- GND
  03- DC +12V.
  04- GND
  05- DC +5V.
  06- GND
  07- 
  08- ACFAIL
  09- DC 12V
  10- GND

  
****************************************************************************

  Specs...

  Cab Size: 470 mm x 450 mm x 1350 mm.
  Weight:   45 Kg.
  Voltage:  AC 90/110 V., 50/60 Hz.
  Power:    30 W.

  
****************************************************************************

  Samples:

  The waveform is ADPCM 4-bit mono, 8000 Hz.
  There are two banks. The sampleset has sounds, music and voices
  at the following rom offsets:

  Bank #01 (00000-3FFFF)

  $000400-$0013B2:    Sample #01    FX sound: 'cling' (credits).
  $0013B3-$0090B1:    Sample #02    music #1.
  $0090B2-$00DD8D:    Sample #03    voice: unknown.
  $00DD8E-$00EF2F:    Sample #04    voice: unknown.
  $00EF30-$0101D0:    Sample #05    voice: unknown.
  $0101D1-$011713:    Sample #06    voice: unknown.
  $011714-$0129FF:    Sample #07    voice: unknown.
  $012A00-$014136:    Sample #08    voice: unknown.
  $014137-$015B57:    Sample #09    voice: unknown.
  $015B58-$018E0E:    Sample #10    voice: unknown.
  $018E0F-$01BB61:    Sample #11    voice: unknown.
  $01BB62-$01F25C:    Sample #12    voice: unknown.
  $01F25D-$01FA35:    Sample #13    voice: unknown.
  $01FA36-$020372:    Sample #14    voice: unknown.
  $020373-$0227E2:    Sample #15    voice: unknown.
  $0227E3-$023E8D:    Sample #16    voice: unknown.
  $023E8E-$026FF7:    Sample #17    music #2
  $026FF8-$02A649:    Sample #18    music #3
  $02A64A-$02D8E9:    Sample #19    music #4
  $02D8EA-$02E635:    Sample #20    FX sound: 'boing' (start).
  $02E636-$02FFB6:    Sample #21    voice: unknown.
  $02FFB7-$03171E:    Sample #22    voice: unknown.
  $03171F-$031EC9:    Sample #23    voice: unknown.
  $031ECA-$032A0D:    Sample #24    voice: unknown.
  $032A0E-$0336E2:    Sample #25    voice: unknown.
  $0336E3-$034748:    Sample #26    voice: unknown.
  $034749-$03523C:    Sample #27    voice: unknown.
  $03523D-$035B00:    Sample #28    voice: unknown.
  $035B01-$03BBE9:    Sample #29    music #5
  $03BBEA-$03E9E1:    Sample #30    voice: unknown.
  $03E9E2-$03F872:    Sample #31    voice: unknown.

  Bank #02 (40000-7FFFF)

  $000400-$0013B2:    Sample #32    FX sound: 'cling' (credits).
  $0013B3-$001B81:    Sample #33    FX sound.
  $001B82-$0035CA:    Sample #34    voice: unknown.
  $0035CB-$003D99:    Sample #35    FX sound.
  $003D9A-$004C09:    Sample #36    voice: unknown.
  $004C0A-$004ED7:    Sample #37    FX sound.
  $004ED8-$0050CD:    Sample #38    FX sound.
  $0050CE-$005DC9:    Sample #39    voice: unknown.
  $005DCA-$00729A:    Sample #40    voice: unknown.
  $00729B-$008A02:    Sample #41    voice: unknown.
  $008A03-$00974E:    Sample #42    FX sound: 'boing' (start).
  $00974F-$009E5E:    Sample #43    voice: unknown.
  $009E5F-$00D476:    Sample #44    music #6
  $00D477-$00F156:    Sample #45    music #7
  $00F157-$00F90A:    Sample #46    voice: unknown.
  $00F90B-$011032:    Sample #47    voice: unknown.
  $011033-$01194B:    Sample #48    voice: unknown.
  $01194C-$012E94:    Sample #49    voice: unknown.
  $012E95-$01429D:    Sample #50    voice: unknown.
  $01429E-$015D2E:    Sample #51    FX sound.
  $015D2F-$0194AE:    Sample #52    FX sound.
  $0194AF-$01B0FE:    Sample #53    voice: unknown.
  $01B0FF-$01D362:    Sample #54    voice: unknown.
  $01D363-$020087:    Sample #55    voice: unknown.
  $020088-$02305F:    Sample #56    voice: unknown.
  $023060-$025F98:    Sample #57    voice: unknown.
  $025F99-$027EE0:    Sample #58    FX sound.
  $027EE1-$029B08:    Sample #59    music #8
  $029B09-$02C6CF:    Sample #60    music #9
  $02C6D0-$02E370:    Sample #61    voice: unknown.
  $02E371-$030EDE:    Sample #62    voice: unknown.
  $030EDF-$0333F8:    Sample #63    voice: unknown.

 
****************************************************************************

  About lamps...

  (nothing yet)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "speaker.h"

#include "notechan.lh"


#define MASTER_CLOCK     XTAL_8_448MHz
#define CPU_CLOCK        MASTER_CLOCK / 2    // guess... not verified
#define SND_CLOCK        MASTER_CLOCK / 8    // guess... not verified


class notechan_state : public driver_device
{
public:
	notechan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;

	DECLARE_WRITE8_MEMBER(out_f8_w);
	DECLARE_WRITE8_MEMBER(out_f9_w);
	DECLARE_WRITE8_MEMBER(out_fa_w);
	DECLARE_WRITE8_MEMBER(out_ff_w);
};


/*********************************************
*           Memory Map Definition            *
*********************************************/

static ADDRESS_MAP_START( notechan_map, AS_PROGRAM, 8, notechan_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( notechan_port_map, AS_IO, 8, notechan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("IN0") AM_WRITE(out_f8_w)
	AM_RANGE(0xf9, 0xf9) AM_READ_PORT("IN1") AM_WRITE(out_f9_w)
	AM_RANGE(0xfa, 0xfa) AM_READ_PORT("IN2") AM_WRITE(out_fa_w)
	AM_RANGE(0xfb, 0xfb) AM_READ_PORT("IN3")
	AM_RANGE(0xff, 0xff) AM_WRITE(out_ff_w)  // watchdog reset? (written immediately upon reset, INT and NMI)
ADDRESS_MAP_END


/*********************************************
*           Output Ports / Lamps             *
*********************************************/

WRITE8_MEMBER(notechan_state::out_f8_w)
{
	output().set_lamp_value(0, data & 1 );
	output().set_lamp_value(1, data >> 1 & 1);
	output().set_lamp_value(2, data >> 2 & 1);
	output().set_lamp_value(3, data >> 3 & 1);
	output().set_lamp_value(4, data >> 4 & 1);
	output().set_lamp_value(5, data >> 5 & 1);
	output().set_lamp_value(6, data >> 6 & 1);
	output().set_lamp_value(7, data >> 7 & 1);

	logerror("Output %02X to $F8\n", data);
}

WRITE8_MEMBER(notechan_state::out_f9_w)
{
	output().set_lamp_value(8, data & 1 );
	output().set_lamp_value(9, data >> 1 & 1);
	output().set_lamp_value(10, data >> 2 & 1);
	output().set_lamp_value(11, data >> 3 & 1);
	output().set_lamp_value(12, data >> 4 & 1);
	output().set_lamp_value(13, data >> 5 & 1);
	output().set_lamp_value(14, data >> 6 & 1);
	output().set_lamp_value(15, data >> 7 & 1);

	logerror("Output %02X to $F9\n", data);
}

WRITE8_MEMBER(notechan_state::out_fa_w)
{
	output().set_lamp_value(16, data & 1 );
	output().set_lamp_value(17, data >> 1 & 1);
	output().set_lamp_value(18, data >> 2 & 1);
	output().set_lamp_value(19, data >> 3 & 1);
	output().set_lamp_value(20, data >> 4 & 1);
	output().set_lamp_value(21, data >> 5 & 1);
	output().set_lamp_value(22, data >> 6 & 1);
	output().set_lamp_value(23, data >> 7 & 1);

	logerror("Output %02X to $FA\n", data);
}

WRITE8_MEMBER(notechan_state::out_ff_w)
{
	output().set_lamp_value(24, data & 1 );
	output().set_lamp_value(25, data >> 1 & 1);
	output().set_lamp_value(26, data >> 2 & 1);
	output().set_lamp_value(27, data >> 3 & 1);
	output().set_lamp_value(28, data >> 4 & 1);
	output().set_lamp_value(29, data >> 5 & 1);
	output().set_lamp_value(30, data >> 6 & 1);
	output().set_lamp_value(31, data >> 7 & 1);

	logerror("Output %02X to $FF\n", data);
}


/*********************************************
*          Input Ports Definitions           *
*********************************************/

static INPUT_PORTS_START( notechan )
	PORT_START("IN0")  // Port F8h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")  // Port F9h
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")  // Port FAh
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)  // Note (1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)  // Note (2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)  // Note (3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")  // Port FBh
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

/*
  Inputs notes...

  Port FAh:
  
  (1) Pulsed under reset, activates lamp 21 and triggers sample #20 (boing). Maybe it's
      the 'start' button.

  (2) Pulsing and keep pressed under reset, triggers the sample #01 (cling) and starts
      a sequence of lamps (24-25-26). Then triggers sample #04 (voice). After a little
	  while also triggers sample #05 (voice).

  (3) Pulsing this input lites lamp 17 and triggers sample #01 (cling). Maybe it's
      the 'coin-in' button. 

*/  
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*********************************************
*               Machine Config               *
*********************************************/

static MACHINE_CONFIG_START( notechan )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)  // unknown...
	MCFG_CPU_PROGRAM_MAP(notechan_map)
	MCFG_CPU_IO_MAP(notechan_port_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(driver_device, irq0_line_hold, 60)

	/* NO VIDEO */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")

	MCFG_OKIM6295_ADD("oki", SND_CLOCK, PIN7_HIGH)  // match the real sounds
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.0)
MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( notechan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "p-650_p1_ver1.10.ic15",  0x0000, 0x8000, CRC(f4878009) SHA1(e8b7f4d84a8995f60d59fe9f4b25e2d1babcf923) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "p-650_s1_ver1.00.ic21",  0x0000, 0x80000, CRC(1b8c835b) SHA1(73749c0077605f9ad56e9dd73b60ee04fe54eb73) )
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT  ROT    COMPANY      FULLNAME      FLAGS                 LAYOUT
GAMEL( 1995, notechan, 0,      notechan, notechan, notechan_state, 0,    ROT0, "Banpresto", "Note Chance", MACHINE_NOT_WORKING,  layout_notechan )
