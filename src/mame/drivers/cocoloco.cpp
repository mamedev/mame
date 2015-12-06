// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Angelo Salese
/**************************************************************************************

  Coco Loco
  Petaco S.A. (Spain)

  Driver by Roberto Fresca & Angelo Salese.

***************************************************************************************

  Hardware Notes:
  --------------

  Single Board.
  Etched in copper: "112-020"

  - CPU:        1x R6502AP.
  - Sound:      1x AY-3-8910.

  - RAM:        11x 4116 (16K x 1) Dynamic RAM. Changed to equivalent MM5290N-3.

  - ROMs:       6X 2716.
  - PROMs:      1X 74S471 bipolar PROM (256 x 8) at location 10C, filled with 0xff.
                Colors look fine and without this device there is no picture at all.

  - Crystal:    1x 20 MHz.

  - 2x 8 DIP switches banks.
  - 1x 2x22 pins edge connector.

***************************************************************************************

  PCB Layout...

  .------------------------------------------------------------------------------------.
  |     A             B             C              D              E          F         |
  |                  .-----. .-----. .-----. .-----. .-----. .-----.                   |
  |                  |2716 | |2716 | |2716 | |2716 | |2716 | |2716 |                   |
  |                  |     | |     | |     | |     | |     | |     |                   |
  |                  |     | |     | |     | |     | |     | |     |                  1|
  |                  |     | |     | |     | |     | |     | |     |                   |
  |                  |     | |     | |     | |     | |     | |     |                   |
  |                  |     | |     | |     | |     | |     | |     |                   |
  |                  '-----' '-----' '-----' '-----' '-----' '-----'                   |
  |                                                                                    |
  |           .--. .--. .--. .--. .--. .--. .--. .--. .--. .--. .--.                   |
  |           |41| |41| |41| |41| |41| |41| |41| |41| |41| |41| |41|  .--. .--. .--.   |
  |           |16| |16| |16| |16| |16| |16| |16| |16| |16| |16| |16|  |74| |74| |74|   |
  |           |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | |  |  |LS| |LS| |LS|  2|
  |           |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | |  |  |74| |74| |74|   |
  |          |'--' '--' '--' '--' '--' '--' '--' '--' '--' '--' '--'| '--' '--' '--'   |
  |          '----------- 11x 4116 (16K x 1) Dynamic RAM -----------'                  |
  |                                                                                    |
  |                                                                                    |
  | .--------.   .--------.     .--------.      .--------.    .--------.   .-    -.    |
  | |74LS244N|   |74LS157N|     |74LS157N|      |74LS42PC|    |74LS175N|   | XTAL |   3|
  | '--------'   '--------'     '--------'      '--------'    '--------'   '-    -'    |
  |                                                                                    |
  |                             .---------------------.                                |
  | .--------.   .--------.     |                     |       .--------.  .-------.    |
  | |74LS244N|   | DIP SW |     |      AY-3-8910      |       |74LS166N|  |SN7404N|   4|
  | '--------'   '--------'     |                     |       '--------'  '-------'    |
  |                             '---------------------'                                |
  |                                                                                    |
  | .-------.                   .--------.      .--------.    .--------.  .-------.    |
  | |74LS14N|                   | DIP SW |      |74LS245N|    |74LS74N |  |74LS00N|   5|
  | '-------'                   '--------'      '--------'    '--------'  '-------'    |
  |                                                                                    |
  |                                                                                    |
  | .-------.    .--------.     .---------.        .------.   .--------.  .--------.   |
  | |74LS04 |    |74LS42PC|     |DM81LS95N|        |R6502 |   |74LS02N |  |74LS161N|  6|
  | '-------'    '--------'     '---------'        |   -13|   '--------'  '--------'   |
  |                                                |      |                            |
  |                                                |      |                            |
  | .--------.   .--------.     .---------.        |      |   .--------.  .--------.   |
  | |74LS174N|   |74LS138N|     |DM81LS95N|        |      |   |74LS161N|  |74LS20PC|  7|
  | '--------'   '--------'     '---------'        |      |   '--------'  '--------'   |
  |                                                |      |                            |
  |                                                |      |                            |
  | .-------.    .--------.     .---------.        |      |   .--------.  .--------.   |
  | |DM7408N|    |74LS138N|     |DM81LS95N|        |      |   |74LS161N|  |74LS74N |  8|
  | '-------'    '--------'     '---------'        |      |   '--------'  '--------'   |
  |                                                '------'                            |
  |                                                                                    |
  | .--------.   .--------.     .---------.     .--------.    .--------.  .--------.   |
  | |74HC174N|   |74LS00N |     |DM81LS95N|     | 74LS04 |    | 74LS04 |  |74LS161N|  9|
  | '--------'   '--------'     '---------'     '--------'    '--------'  '--------'   |
  |                                                                                    |
  |   RESNET     .--------.     .---------.     .--------.    .--------.  .--------.   |
  |              |F-7417PC|     |SN74S471N|     | 74LS04 |    |74LS112N|  |74LS112N| 10|
  |   RESNET     '--------'     '---------'     '--------'    '--------'  '--------'   |
  |                                PROM                                                    |
  |                                                                AUDIO               |
  |                                                                   DISCRETE         |
  |                                2x22 EDGE CONNECTOR                     CIRCUITRY   |
  |               .---.                                           .---.    .-----.     |
  |               |   | | | | | | | | | | | | | | | | | | | | | | |   |    | POT |     |
  | 112-020       |   | | | | | | | | | | | | | | | | | | | | | | |   |    '-----'     |
  '---------------'   '-------------------------------------------'   '----------------'


***************************************************************************************

  AY-3-8910 channels discrete circuitry
  -------------------------------------

                                                         +12v    + --. EC
                                                          |   .----| |----.
  .------.    R 4K7                                       |   |    --'    |
  |    03|---/\/\/\/--.                                   +---+ 470uf 25v +----.
  |      |    R 4K7   |                                   |   |           |    |
  |    04|---/\/\/\/--+----.                              |   '----| |----'   -+-
  |      |    R 4K7   |    |            EC                |       0.1 uf      GND
  |    38|---/\/\/\/--'    Z         10uf 16v           |\|5             EC
  |      |                 Z 5K        .--             1| \           100uf 25v
  | AY-3 |                 Z <---------| |--------------|  \LM383T     + --.
  | 8910 |                 Z POT       '-- +            |   >-----+------| |-----------> SPKR (edge pin 22)
  |      |                 Z                      .-----|  / 4    |      --'
  |      |                 |               + --.  |    2| /       Z  R            .----> SPKR (edge pin 21)
  |      |                -+-           .----| |--'     |/|3      Z 220           |
  |      |                GND           |    --'          |       Z              -+-
  |      |                              |  470uf 25v     -+-      |              GND
  |      |                              |    EC          GND      |   .--| |---.
  '------'                              |                         |   | 0.1 uf |
                                        +-------------------------+---+        +----.
                                        |                             | 0.1 uf |    |
                                        |   R 2.2                     '--| |---'   -+-
                                        '--/\/\/\/--.                              GND
                                                    |
                                                   -+-
                                                   GND

***************************************************************************************

  ***  Memory Map  ***

  $0000 - $00FF   RAM     ; Zero Page (pointers and registers)
  $0100 - $01FF   RAM     ; 6502 Stack Pointer.
  $0200 - $07FF   RAM     ; R/W. (settings)

  $2000 - $3FFF   VRAM    ; Video RAM.

  $6001 - $6001   PSG     ; AY-3-8910 data read.
  $6002 - $6002   PSG     ; AY-3-8910 data write.
  $6003 - $6003   PSG     ; AY-3-8910 address write.

  $8003 - $8003   BNK     ; Video bank register.
  $8005 - $8005           ; Coin counter.

  $A000 - $A000   INP     ; Input port.

  $A005 - $A005   ???     ; Unknown reads.
                          ; VRAM clear?

  $D000 - $FFFF   ROM     ; ROM space.

***************************************************************************************

  DRIVER UPDATES:

  2014-05-11
  - Initial Release.


  TODO:

  - Find the 25 Pesetas missing coin input.
  - Verify video timings.
  - PROM verification and real colors.
  - Find the meaning of A005h reads (always discarded).

**************************************************************************************/


#define MASTER_CLOCK    XTAL_20MHz           /* confirmed */
#define CPU_CLOCK       MASTER_CLOCK / 16    /* confirmed */
#define SND_CLOCK       MASTER_CLOCK / 8     /* confirmed */

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"


class cocoloco_state : public driver_device
{
public:
	cocoloco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	UINT8 *m_videoram;
	UINT8 m_videobank;

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(vbank_w);
	DECLARE_WRITE8_MEMBER(vram_clear_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	virtual void video_start() override;
	DECLARE_PALETTE_INIT(cocoloco);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/***********************************
*          Sound Hardware          *
***********************************/

static NETLIST_START(nl_cocoloco)

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-5)
	ANALOG_INPUT(V5, 5)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000);
	RES(R_AY1_2, 1000);
	RES(R_AY1_3, 1000);

	RES(R1, 4700)
	RES(R2, 4700)
	RES(R3, 4700)
	RES(RAMP, 150000)
	//RES(RAMP, 150)
	POT(P1, 5000)
	PARAM(P1.DIAL, 0.5) // 50%

	CAP(C1, 10e-6)

	NET_C(V5, R_AY1_1.1, R_AY1_2.1, R_AY1_3.1)

	NET_C(R_AY1_1.2, R1.1)
	NET_C(R_AY1_2.2, R2.1)
	NET_C(R_AY1_3.2, R3.1)

	NET_C(R1.2, R2.2, R3.2, P1.1)

	NET_C(P1.3, RAMP.2, GND)
	NET_C(P1.2, C1.1)
	NET_C(C1.2, RAMP.1)
#if 0
	CAP(C2, 0.1e-6)
	NET_C(C2.2, GND)
	NET_C(C2.1, RAMP.1)
#endif
NETLIST_END()


/***********************************
*          Video Hardware          *
***********************************/

PALETTE_INIT_MEMBER(cocoloco_state, cocoloco)
{
	for(int i = 0; i < 0x10; i += 2)
	{
		int r,g,b;

		r = pal2bit(i & 3);
		g = pal2bit((i >> 1) & 3);
		b = pal2bit((i >> 2) & 3);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	palette.set_pen_color(0x01, rgb_t(0xff, 0xff, 0x00));
	palette.set_pen_color(0x02, rgb_t(0x00, 0xff, 0x00));
	palette.set_pen_color(0x04, rgb_t(0x00, 0x7f, 0xff));

	palette.set_pen_color(0x08, rgb_t(0xff, 0x7f, 0x00));
	palette.set_pen_color(0x0a, rgb_t(0x00, 0xff, 0xff));
	palette.set_pen_color(0x0c, rgb_t(0xff, 0x00, 0x00));
	palette.set_pen_color(0x0e, rgb_t(0xff, 0xff, 0xff));

}


void cocoloco_state::video_start()
{
	m_videoram = auto_alloc_array(machine(), UINT8, 0x2000 * 8);

	save_pointer(NAME(m_videoram), 0x2000 * 8);
	save_item(NAME(m_videobank));
}

UINT32 cocoloco_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, count, xi;

	bitmap.fill(m_palette->black_pen(), cliprect);

	count = 0;

	for(x = 0; x < 256; x += 8)
	{
		for(y = 0; y < 256; y++)
		{
			for (xi = 0; xi < 8; xi++)
			{
				int color;
				color  =  (m_videoram[count|0x0000] >> (xi)) & 1;
				color |= ((m_videoram[count|0x2000] >> (xi)) & 1) << 1;
				color |= ((m_videoram[count|0x4000] >> (xi)) & 1) << 2;
				color |= ((m_videoram[count|0x6000] >> (xi)) & 1) << 3;

				if(cliprect.contains(x + xi, 256 - y))
					bitmap.pix16(256 - y, x + xi) = m_palette->pen(color & 0x0f);
			}

			count++;
		}
	}

	return 0;
}


READ8_MEMBER( cocoloco_state::vram_r )
{
	return m_videoram[offset|0x0000] | m_videoram[offset|0x2000] | m_videoram[offset|0x4000] | m_videoram[offset|0x6000];
}

WRITE8_MEMBER( cocoloco_state::vram_w )
{
	m_videoram[offset|0x0000] = (m_videobank == 0) ? data : 0;
	m_videoram[offset|0x2000] = (m_videobank & 2) ? data : 0;
	m_videoram[offset|0x4000] = (m_videobank & 4) ? data : 0;
	m_videoram[offset|0x6000] = (m_videobank & 8) ? data : 0;
}

WRITE8_MEMBER( cocoloco_state::vbank_w )
{
	m_videobank = data;
}

WRITE8_MEMBER( cocoloco_state::vram_clear_w )
{
	/* ??? */
//  for(int i=0;i<0x8000;i++)
//      m_videoram[i] = 0;

//  popmessage("A005 writes: %02X", data);
}


WRITE8_MEMBER( cocoloco_state::coincounter_w )
{
/*  - bits -
    7654 3210
    ---- x---   Coin counter (see $DF90).
    xxxx -xxx   Unknown.

    The coin counter gives 2 pulses for each coin inserted.
    They explain in a sheet that the coin in for 50 pesetas
    behaves like 2x 25 pesetas (1 duro) coins, so has sense.
*/
	coin_counter_w(machine(), 0, data & 0x08);
}


/***********************************
*      Memory Map Information      *
***********************************/

static ADDRESS_MAP_START( cocoloco_map, AS_PROGRAM, 8, cocoloco_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(vram_r, vram_w)     // 256 x 256 x 1
	AM_RANGE(0x6001, 0x6001) AM_DEVREAD("ay8910", ay8910_device, data_r)
	AM_RANGE(0x6002, 0x6002) AM_DEVWRITE("ay8910", ay8910_device, data_w)
	AM_RANGE(0x6003, 0x6003) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(vbank_w)
	AM_RANGE(0x8005, 0x8005) AM_WRITE(coincounter_w)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa005, 0xa005) AM_WRITE(vram_clear_w)
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*
  1800-3fff: RW  --> code inits reading and writing the whole range.

  All 3 instances of A005 reads (d07e, d355 and dca8),
  discard the read in a non-sense way....

  There is another register (8005h), that is written by the code
  (bit3 on/off) after coin-in, and checking the inputs too...
  Seems coin counter, but the input check is suspicious.

  To see this. Put a BP at DF90h, run the game and coin in...

*/

/***********************************
*           Input Ports            *
***********************************/

INPUT_CHANGED_MEMBER(cocoloco_state::coin_inserted)
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static INPUT_PORTS_START( cocoloco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, cocoloco_state, coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Select / Speed-Up Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1") // DSW1 @4B
	PORT_DIPNAME( 0x01, 0x00, "Char Speed" )                    PORT_DIPLOCATION("DSW1:!1")
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPNAME( 0x02, 0x00, "Monsters Speed" )                PORT_DIPLOCATION("DSW1:!2")
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPSETTING(    0x02, "Slow" )
	PORT_DIPNAME( 0x0c, 0x00, "Monsters: Time before go out" )  PORT_DIPLOCATION("DSW1:!3,!4")
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPSETTING(    0x04, "Medium-Long" )
	PORT_DIPSETTING(    0x08, "Medium-Short" )
	PORT_DIPSETTING(    0x0c, "Short" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )             // switches 5-6-7-8 marked as unused.

	PORT_START("DSW2") // DSW2 @5C
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW2:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:!3,!4")
	PORT_DIPSETTING(    0x00, "10000 Points" )
	PORT_DIPSETTING(    0x04, "15000 Points" )
	PORT_DIPSETTING(    0x08, "20000 Points" )
	PORT_DIPSETTING(    0x0c, "30000 Points" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW2:!5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x20, 0x00, "Monsters" )                  PORT_DIPLOCATION("DSW2:!6")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, "Vitamine Time" )             PORT_DIPLOCATION("DSW2:!7,!8")
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPSETTING(    0x40, "Medium-Long" )
	PORT_DIPSETTING(    0x80, "Medium-Short" )
	PORT_DIPSETTING(    0xc0, "Short" )
INPUT_PORTS_END


/***********************************
*         Machine Drivers          *
***********************************/

static MACHINE_CONFIG_START( cocoloco, cocoloco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, CPU_CLOCK)   /* confirmed */
	MCFG_CPU_PROGRAM_MAP(cocoloco_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(CPU_CLOCK * 4, 384, 0, 256, 262, 0, 256) /* TODO: not accurate, ~50 Hz */
	MCFG_SCREEN_UPDATE_DRIVER(cocoloco_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x10)
	MCFG_PALETTE_INIT_OWNER(cocoloco_state, cocoloco)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, SND_CLOCK) /* confirmed */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_OUTPUT_TYPE(AY8910_RESISTOR_OUTPUT)
	MCFG_SOUND_ROUTE_EX(0, "snd_nl", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "snd_nl", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "snd_nl", 1.0, 2)

	/* NETLIST configuration using internal AY8910 resistor values */

	MCFG_SOUND_ADD("snd_nl", NETLIST_SOUND, 48000)
	MCFG_NETLIST_SETUP(nl_cocoloco)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NETLIST_STREAM_INPUT("snd_nl", 0, "R_AY1_1.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 1, "R_AY1_2.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 2, "R_AY1_3.R")

	MCFG_NETLIST_STREAM_OUTPUT("snd_nl", 0, "RAMP.1")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(30000.0 * 1.5, 0)

MACHINE_CONFIG_END


/***********************************
*             Rom Load             *
***********************************/

ROM_START( cocoloco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "coco1-c.b1", 0xd000, 0x0800, CRC(f2a699a9) SHA1(7032af9b416665df2f17bb308a297d65c29a09fb) )
	ROM_LOAD( "coco1-d.b1", 0xd800, 0x0800, CRC(08e00d09) SHA1(82b3982df81fb374bd385935c3ca61330f1a673f) )
	ROM_LOAD( "coco1-e.c1", 0xe000, 0x0800, CRC(2adbf6ff) SHA1(f2d3f33b7d8812974765303f3ea5230e3b7bed37) )
	ROM_LOAD( "coco1-f.c1", 0xe800, 0x0800, CRC(2ada04a0) SHA1(8d0402d7f77e382afebecf5e8f7b2aac4567b9e7) )
	ROM_LOAD( "coco1-g.e1", 0xf000, 0x0800, CRC(454fbc8e) SHA1(edf22d1939a3a14f30f14c021e47aa9404fa4c75) )
	ROM_LOAD( "coco1-h.e1", 0xf800, 0x0800, CRC(b6d0ebea) SHA1(a8f09558f71dfe0d300a6bb946dcb3bf6393c02b) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "prom.c10",   0x0000, 0x0100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )
ROM_END


/***********************************
*           Game Drivers           *
***********************************/

/*    YEAR  NAME       PARENT   MACHINE   INPUT     STATE          INIT   ROT     COMPANY         FULLNAME      FLAGS  */
GAME( 198?, cocoloco,  0,       cocoloco, cocoloco, driver_device, 0,     ROT90, "Petaco S.A.",  "Coco Loco", MACHINE_SUPPORTS_SAVE )
