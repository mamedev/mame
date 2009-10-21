/***************************************************************************

 Arcade games (hacks of console games) running on SNES harware.

 Driver (based on nss.c ) by Tomasz Slanina  analog[at]op.pl

    Supported games:
    - Killer Instinct
    - Final Fight 2
    - Sonic Blast Man 2

    Not dumped:
    - Final Fight 3

TODO:

 - ffight2b -  dipswitches ($7eadd7 ?)
 - sblast2b -  dipswicthes
 - kiinstb -   fix gfx glitches, missing texts



  Final Fight 2 PCB layout:
  ------------------------

 |----------------------------------------------------------------------------|
 | |-----------|                                                              |
 | |           |        21.47727 MHz      24.576 MHz                          |
 | | Lattice   |                                                              |
 | | pLSI      |               |--------|   |--------|          HM65256       |
 | | 1024-60LJ |  |--------|   |        |   |        |                        |
 | |           |  |        |   | 86A621 |   | 86A537 |                        |
 | |-----------|  | 86A623 |   |  JDCF  |   |  JDCF  |                        |
 |                |  JDCF  |   |        |   |        |          D42832C       |
 |    ff2_1.u8    |        |   |--------|   |--------|                        |
 |                |--------|                                                  |
 |                             |--------|   |--------|                        |
 |    ff2_2.u7    |--------|   |        |   |        |          KM62256       |
 |                |        |   | 86A617 |   | 86A618 |                        |
 |                | 86A540 |   |  JDCF  |   |  JDKF  |                        |
 |    ff2_3.u6    |  JDKF  |   |        |   |        |                        |
 |                |        |   |--------|   |--------|          KM62256       |
 |                |--------|                                                  |
 |     GL324                  D41464C     D41464C                             |
 |                                                                            |
 |                            D41464C     D41464C         DSW2      DSW1      |
 |                                                                            |
 |                              7414        74245        74245     74245      |
 |                                                                            |
 |    uPC1242H       VR1       GD4021B     GD4021B      GD4021B   GD4021B     |
 |                                                                            |
 |                                                                            |
 |               |---|              JAMMA                 |---|               |
 |---------------|   |------------------------------------|   |---------------|

  Killer Instinct PCB Info:
  --------------------------

    PQFP 100(?)pin chip marked "SP-BE0"
    PQFP 100(?)pin chip marked "SP-BH0"
    PQFP 100(?)pin chip marked "SP-AF0"
    Lattice pLSI 1024-60LJ B604S03
    6116 SRAM    x2
    AS7C256 SRAM x8
    jumper pack (12)
    dsw8         x2
    Xtal 24.576 MHz
    Xtal 21.47727 MHz
    volume pot
    27c801       x4
    two empty eprom sockets

    It's SNES version of KI with few mods (removed copyright messages,
    extra code for coin input, etc).

    256 bytes of RAM ( mapped to reserved area) are shared with some
    device (probably Lattice PLD) used for handle coin inputs and dips

    Data lines of eproms are bitswapped.

***************************************************************************/
#include "driver.h"
#include "cpu/spc700/spc700.h"
#include "cpu/g65816/g65816.h"
#include "includes/snes.h"

static INT8 *shared_ram;
static UINT8 ffight2b_coins;

static READ8_HANDLER(sharedram_r)
{
	static INT32 oldinput=0;
	INT32 coincnt;
	INT32 input = input_port_read(space->machine, "COIN");

	if(input&3)
	{
		if( ((input&1)==1)&&((oldinput&1)==0))	{shared_ram[0]++;}

		coincnt=shared_ram[0];

		if(coincnt>99){coincnt=99;}

		shared_ram[0xb]=(coincnt/10)+'0';
		shared_ram[0xa]=(coincnt%10)+'0';
	}
	oldinput=input;
	return shared_ram[offset];
}

static WRITE8_HANDLER(sharedram_w)
{
	shared_ram[offset]=data;
}

static READ8_HANDLER(ffight2b_coin_r)
{
	static INT32 oldinput=0;
	INT32 input = input_port_read(space->machine, "COIN");

	if( ((input&1)==1)&&((oldinput&1)==0))
	{
		INT32 coin_cnt=(ffight2b_coins&0xf)+10*(ffight2b_coins>>4);
		if(++coin_cnt>99) coin_cnt=99;
		ffight2b_coins=(coin_cnt%10)|((coin_cnt/10)<<4);
	}
	oldinput=input;
	return ffight2b_coins;
}

/* Sonic blast man 2 turbo */

static READ8_HANDLER(sb2b_75bd37_r)
{
	/* protection check */
	static UINT8 cnt=0;
 	return ++cnt;
}

static READ8_HANDLER(sb2b_6a6xxx_r)
{
	/* protection checks */
	switch(offset)
	{
		case 0x26f: return 0xb1;
		case 0x3e0:	return 0x9e;
		case 0x5c8: return 0xf4;
		case 0x94b: return 0x3a;
		case 0xd1a:	return 0xc5;
		case 0xfb7:	return 0x47;
	}

	logerror("Unknown protection read read %x @ %x\n",offset, cpu_get_pc(space->cpu));

	return 0;
}

static READ8_HANDLER(sb2b_770071_r)
{
 	return input_port_read(space->machine, "DSW");
}

static READ8_HANDLER(sb2b_770079_r)
{
 	return input_port_read(space->machine, "COIN");
}

static READ8_HANDLER(sb2b_7xxx_r)
{
	return snes_ram[0xc07000 + offset];
}

static ADDRESS_MAP_START( snesb_map, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x000000, 0x2fffff) AM_READWRITE(snes_r_bank1, snes_w_bank1)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x300000, 0x3fffff) AM_READWRITE(snes_r_bank2, snes_w_bank2)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x400000, 0x5fffff) AM_READWRITE(snes_r_bank3, SMH_ROM)	/* ROM (and reserved in Mode 20) */
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE(snes_r_bank4, snes_w_bank4)	/* used by Mode 20 DSP-1 */
	AM_RANGE(0x700000, 0x7dffff) AM_READWRITE(snes_r_bank5, snes_w_bank5)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM					/* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE(snes_r_bank6, snes_w_bank6)	/* Mirror and ROM */
	AM_RANGE(0xc00000, 0xffffff) AM_READWRITE(snes_r_bank7, snes_w_bank7)	/* Mirror and ROM */
ADDRESS_MAP_END


static READ8_HANDLER( spc_ram_100_r )
{
	return spc_ram_r(space, offset + 0x100);
}

static WRITE8_HANDLER( spc_ram_100_w )
{
	spc_ram_w(space, offset + 0x100, data);
}

static ADDRESS_MAP_START( spc_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ef) AM_READWRITE(spc_ram_r, spc_ram_w) AM_BASE(&spc_ram)   	/* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_READWRITE(spc_io_r, spc_io_w)   	/* spc io */
	AM_RANGE(0x0100, 0xffff) AM_WRITE(spc_ram_100_w)
	AM_RANGE(0x0100, 0xffbf) AM_READ(spc_ram_100_r)
	AM_RANGE(0xffc0, 0xffff) AM_READ(spc_ipl_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( kinstb )
	PORT_START("PAD1L")		/* IN 0 : Joypad 1 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_START("PAD1H")		/* IN 1 : Joypad 1 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("PAD2L")		/* IN 2 : Joypad 2 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_START("PAD2H")		/* IN 3 : Joypad 2 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("PAD3L")		/* IN 4 : Joypad 3 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P3 Button A") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P3 Button X") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P3 Button L") PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P3 Button R") PORT_PLAYER(3)
	PORT_START("PAD3H")		/* IN 5 : Joypad 3 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P3 Button B") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P3 Button Y") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("P3 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("P3 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)

	PORT_START("PAD4L")		/* IN 6 : Joypad 4 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P4 Button A") PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P4 Button X") PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P4 Button L") PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P4 Button R") PORT_PLAYER(4)
	PORT_START("PAD4H")		/* IN 7 : Joypad 4 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P4 Button B") PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P4 Button Y") PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_NAME("P4 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("P4 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)

	PORT_START("INTERNAL")		/* IN 8 : Internal switches */
	PORT_DIPNAME( 0x1, 0x1, "Enforce 32 sprites/line" )
	PORT_DIPSETTING(   0x0, DEF_STR( No )  )
	PORT_DIPSETTING(   0x1, DEF_STR( Yes ) )

#ifdef MAME_DEBUG
	PORT_START("DEBUG1")	/* IN 9 : debug switches */
	PORT_DIPNAME( 0x3, 0x0, "Browse tiles" )
	PORT_DIPSETTING(   0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x1, "2bpl"  )
	PORT_DIPSETTING(   0x2, "4bpl"  )
	PORT_DIPSETTING(   0x3, "8bpl"  )
	PORT_DIPNAME( 0xc, 0x0, "Browse maps" )
	PORT_DIPSETTING(   0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x4, "2bpl"  )
	PORT_DIPSETTING(   0x8, "4bpl"  )
	PORT_DIPSETTING(   0xc, "8bpl"  )

	PORT_START("DEBUG2")	/* IN 10 : debug switches */
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle BG 1") PORT_PLAYER(2)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Toggle BG 2") PORT_PLAYER(2)
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Toggle BG 3") PORT_PLAYER(2)
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Toggle BG 4") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle Objects") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Toggle Main/Sub") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Toggle Back col") PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Toggle Windows") PORT_PLAYER(3)

	PORT_START("DEBUG3")	/* IN 11 : debug input */
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Pal prev")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Pal next")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle Transparency") PORT_PLAYER(4)
#endif

	PORT_START("DSW")	/* IN 12 : dip-switches */

	PORT_START("COIN")	/* IN 13 : coins */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )

INPUT_PORTS_END


static INPUT_PORTS_START( ffight2b )
	PORT_START("PAD1L")		/* IN 0 : Joypad 1 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_START("PAD1H")		/* IN 1 : Joypad 1 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("PAD2L")		/* IN 2 : Joypad 2 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_START("PAD2H")		/* IN 3 : Joypad 2 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("PAD3L")		/* IN 4 : Joypad 3 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P3 Button A") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P3 Button X") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P3 Button L") PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P3 Button R") PORT_PLAYER(3)
	PORT_START("PAD3H")		/* IN 5 : Joypad 3 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P3 Button B") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P3 Button Y") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("P3 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("P3 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)

	PORT_START("PAD4L")		/* IN 6 : Joypad 4 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P4 Button A") PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P4 Button X") PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P4 Button L") PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P4 Button R") PORT_PLAYER(4)
	PORT_START("PAD4H")		/* IN 7 : Joypad 4 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P4 Button B") PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P4 Button Y") PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_NAME("P4 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("P4 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)

	PORT_START("INTERNAL")		/* IN 8 : Internal switches */
	PORT_DIPNAME( 0x1, 0x1, "Enforce 32 sprites/line" )
	PORT_DIPSETTING(   0x0, DEF_STR( No )  )
	PORT_DIPSETTING(   0x1, DEF_STR( Yes ) )

#ifdef MAME_DEBUG
	PORT_START("DEBUG1")	/* IN 9 : debug switches */
	PORT_DIPNAME( 0x3, 0x0, "Browse tiles" )
	PORT_DIPSETTING(   0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x1, "2bpl"  )
	PORT_DIPSETTING(   0x2, "4bpl"  )
	PORT_DIPSETTING(   0x3, "8bpl"  )
	PORT_DIPNAME( 0xc, 0x0, "Browse maps" )
	PORT_DIPSETTING(   0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x4, "2bpl"  )
	PORT_DIPSETTING(   0x8, "4bpl"  )
	PORT_DIPSETTING(   0xc, "8bpl"  )

	PORT_START("DEBUG2")	/* IN 10 : debug switches */
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle BG 1") PORT_PLAYER(2)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Toggle BG 2") PORT_PLAYER(2)
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Toggle BG 3") PORT_PLAYER(2)
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Toggle BG 4") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle Objects") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Toggle Main/Sub") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Toggle Back col") PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Toggle Windows") PORT_PLAYER(3)

	PORT_START("DEBUG3")	/* IN 11 : debug input */
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Pal prev")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Pal next")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle Transparency") PORT_PLAYER(4)
#endif

	PORT_START("DSW")	/* IN 12 : dip-switches */
	PORT_DIPNAME( 0x01, 0x01, "Player Bonus" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x04, "Game Level" ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
/*  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) ) */
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

	PORT_START("COIN")	/* IN 13 : coins */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )

INPUT_PORTS_END


static INPUT_PORTS_START( sblast2b )
	PORT_START("PAD1L")		/* IN 0 : Joypad 1 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_START("PAD1H")		/* IN 1 : Joypad 1 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("PAD2L")		/* IN 2 : Joypad 2 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_START("PAD2H")		/* IN 3 : Joypad 2 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("PAD3L")		/* IN 4 : Joypad 3 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P3 Button A") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P3 Button X") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P3 Button L") PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P3 Button R") PORT_PLAYER(3)
	PORT_START("PAD3H")		/* IN 5 : Joypad 3 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P3 Button B") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P3 Button Y") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("P3 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("P3 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)

	PORT_START("PAD4L")		/* IN 6 : Joypad 4 - L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P4 Button A") PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P4 Button X") PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P4 Button L") PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P4 Button R") PORT_PLAYER(4)
	PORT_START("PAD4H")		/* IN 7 : Joypad 4 - H */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P4 Button B") PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P4 Button Y") PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_NAME("P4 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("P4 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)

	PORT_START("INTERNAL")		/* IN 8 : Internal switches */
	PORT_DIPNAME( 0x1, 0x1, "Enforce 32 sprites/line" )
	PORT_DIPSETTING(   0x0, DEF_STR( No )  )
	PORT_DIPSETTING(   0x1, DEF_STR( Yes ) )

#ifdef MAME_DEBUG
	PORT_START("DEBUG1")	/* IN 9 : debug switches */
	PORT_DIPNAME( 0x3, 0x0, "Browse tiles" )
	PORT_DIPSETTING(   0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x1, "2bpl"  )
	PORT_DIPSETTING(   0x2, "4bpl"  )
	PORT_DIPSETTING(   0x3, "8bpl"  )
	PORT_DIPNAME( 0xc, 0x0, "Browse maps" )
	PORT_DIPSETTING(   0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x4, "2bpl"  )
	PORT_DIPSETTING(   0x8, "4bpl"  )
	PORT_DIPSETTING(   0xc, "8bpl"  )

	PORT_START("DEBUG2")	/* IN 10 : debug switches */
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle BG 1") PORT_PLAYER(2)
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Toggle BG 2") PORT_PLAYER(2)
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Toggle BG 3") PORT_PLAYER(2)
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Toggle BG 4") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle Objects") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Toggle Main/Sub") PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Toggle Back col") PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Toggle Windows") PORT_PLAYER(3)

	PORT_START("DEBUG3")	/* IN 11 : debug input */
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Pal prev")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Pal next")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Toggle Transparency") PORT_PLAYER(4)
#endif

	PORT_START("DSW")	/* IN 12 : dip-switches */
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("COIN")	/* IN 13 : coins */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )

INPUT_PORTS_END

static MACHINE_DRIVER_START( kinstb )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", G65816, 3580000)	/* 2.68Mhz, also 3.58Mhz */
	MDRV_CPU_PROGRAM_MAP(snesb_map)

	/* audio CPU */
	MDRV_CPU_ADD("soundcpu", SPC700, 2048000/2)	/* 2.048 Mhz, but internal divider */
	MDRV_CPU_PROGRAM_MAP(spc_mem)

	MDRV_QUANTUM_PERFECT_CPU("maincpu")

	MDRV_MACHINE_START( snes )
	MDRV_MACHINE_RESET( snes )

	/* video hardware */
	MDRV_VIDEO_START( snes )
	MDRV_VIDEO_UPDATE( snes )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(DOTCLK_NTSC, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("snes", SNES, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.00)
MACHINE_DRIVER_END

static DRIVER_INIT(kinstb)
{
	INT32 i;
	UINT8 *rom = memory_region(machine, "user3");

	for(i=0;i<0x400000;i++)
	{
		rom[i]=BITSWAP8(rom[i],5,0,6,1,7,4,3,2);
	}

	shared_ram=auto_alloc_array(machine, INT8, 0x100);
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x781000, 0x7810ff, 0, 0, sharedram_r, sharedram_w);

	DRIVER_INIT_CALL(snes_hirom);
}

static DRIVER_INIT( ffight2b )
{
	INT32 i;
	UINT8 *rom = memory_region(machine, "user3");

	for(i=0;i<0x200000;i++)
	{
		rom[i]=rom[i]^0xff;

		if (i<0x10000) /* 0x00000 - 0x0ffff */
		{
			rom[i]=BITSWAP8(rom[i],3,1,6,4,7,0,2,5);
		}
		else if (i<0x20000) /* 0x10000 - 0x1ffff */
		{
			rom[i]=BITSWAP8(rom[i],3,7,0,5,1,6,2,4);
		}

		else if (i<0x30000) /* 0x20000 - 0x2ffff */
		{
			rom[i]=BITSWAP8(rom[i],1,7,6,4,5,2,3,0);
		}
		else if (i<0x40000) /* 0x30000 - 0x3ffff */
		{
			rom[i]=BITSWAP8(rom[i],0,3,2,5,4,6,7,1);
		}
		else if (i<0x150000)
		{
			rom[i]=BITSWAP8(rom[i],6,4,0,5,1,3,2,7);
		}
	}

	/*  boot vector */
	rom[0x7ffd]=0x89;
	rom[0x7ffc]=0x54;

	ffight2b_coins=0;
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x7eadce, 0x7eadce, 0, 0, ffight2b_coin_r);


	DRIVER_INIT_CALL(snes);
}


static const UINT8 data_substitution0[] = {0x88,0x02,0x2a,0x08,0x28,0xaa,0x8a,0x0a,0xa2,0x00,0x80,0xa0,0x22,0xa8,0x82,0x20,};
static const UINT8 data_substitution1[] = {0x44,0x01,0x51,0x40,0x50,0x55,0x45,0x41,0x15,0x00,0x04,0x14,0x11,0x54,0x05,0x10,};
static const UINT8 address_substitution_low[] =
{
	0x32,0x35,0x3a,0x28,0x2a,0x0c,0x36,0x38,0x29,0x39,0x04,0x2c,0x21,0x23,0x3d,0x2d,
	0x3c,0x02,0x17,0x31,0x00,0x2e,0x0a,0x2f,0x25,0x26,0x27,0x30,0x33,0x01,0x18,0x19,
	0x10,0x11,0x24,0x16,0x1b,0x0d,0x0e,0x12,0x13,0x05,0x22,0x34,0x1c,0x06,0x07,0x37,
	0x08,0x3b,0x09,0x14,0x15,0x1d,0x0b,0x0f,0x1e,0x1f,0x2b,0x1a,0x03,0x20,0x3f,0x3e,
};

static const UINT8  address_substitution_high[] =
{
	0x1b,0x15,0x08,0x1f,0x06,0x02,0x13,0x0a,0x1d,0x04,0x0e,0x00,0x17,0x0c,0x11,0x19,
	0x16,0x0d,0x1c,0x07,0x10,0x03,0x1a,0x0b,0x12,0x05,0x0f,0x18,0x1e,0x01,0x14,0x09,
	0x2b,0x25,0x28,0x2f,0x26,0x22,0x23,0x2a,0x2d,0x24,0x2e,0x20,0x27,0x2c,0x21,0x29
};

static DRIVER_INIT( sblast2b )
{
	int i, cipherText, plainText, newAddress;
	UINT8 *src = memory_region(machine, "user7");
	UINT8 *dst = memory_region(machine, "user3");
	for (i=0; i<0x80000*3; i++)
    {
       	cipherText = src[i];
       	plainText = data_substitution0[cipherText&0xf]|data_substitution1[cipherText>>4];
   		newAddress = (address_substitution_high[i>>15]<<15)|(i&0x7fc0)|(address_substitution_low[i&0x3f]);

		if(newAddress<0x10000)
	 	{
	 		plainText = BITSWAP8(plainText, 6,3,5,4,2,0,7,1) ^ 0xff;
	 	}
		else
	 	if(newAddress<0x20000)
	 	{
	 		plainText =	BITSWAP8(plainText, 4,0,7,6,3,1,2,5) ^ 0xff;
	 	}
	 	else
	 	if(newAddress<0x30000)
	 	{
	 		plainText =	BITSWAP8(plainText, 5,7,6,1,4,3,0,2);
	 	}
	 	else
	 	if(newAddress<0x40000)
	 	{
	 		plainText = BITSWAP8(plainText, 3,1,2,0,5,6,4,7) ^ 0xff;
	 	}
        dst[newAddress] = plainText;
    }

 	/*  boot vector */
    dst[0xfffc]=0xc0;
    dst[0xfffd]=0x7a;

   	/*  protection checks */
 	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x75bd37, 0x75bd37, 0, 0, sb2b_75bd37_r);
  	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6a6000, 0x6a6fff, 0, 0, sb2b_6a6xxx_r);

  	/* extra inputs */
   	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x770071, 0x770071, 0, 0, sb2b_770071_r);
    memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x770079, 0x770079, 0, 0, sb2b_770079_r);

    /* handler to read boot code */
    memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x007000, 0x007fff, 0, 0, sb2b_7xxx_r);

	DRIVER_INIT_CALL(snes_hirom);
}

ROM_START( kinstb )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD( "1.u14", 0x000000, 0x100000, CRC(70889919) SHA1(1451714cbdacb7f6ced2bc7afa478ad7264cf3b7) )
	ROM_LOAD( "2.u15", 0x100000, 0x100000, CRC(e4a5d1da) SHA1(6ae566bd2f740a251d7a81b8ebb92a651cfaac8d) )
	ROM_LOAD( "3.u16", 0x200000, 0x100000, CRC(7a40f7dd) SHA1(cebe632e8d2d68d0619077cc1e931af73c9a723b) )
	ROM_LOAD( "4.u17", 0x300000, 0x100000, CRC(3d7564c1) SHA1(392b513991897668d5dd469ac84a34f785895774) )

	ROM_REGION(0x100,           "user5", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)

ROM_END

ROM_START( ffight2b )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD( "ff2_3.u6", 0x000000, 0x008000, CRC(343bf582) SHA1(cc6b7219bb2fe61f0b377b606ad28b0e5a78be0b) )
	ROM_CONTINUE(          0x088000, 0x008000 )
	ROM_CONTINUE(          0x010000, 0x008000 )
	ROM_CONTINUE(          0x098000, 0x008000 )
	ROM_CONTINUE(          0x020000, 0x008000 )
	ROM_CONTINUE(          0x0a8000, 0x008000 )
	ROM_CONTINUE(          0x030000, 0x008000 )
	ROM_CONTINUE(          0x0b8000, 0x008000 )
	ROM_CONTINUE(          0x040000, 0x008000 )
	ROM_CONTINUE(          0x0c8000, 0x008000 )
	ROM_CONTINUE(          0x050000, 0x008000 )
	ROM_CONTINUE(          0x0d8000, 0x008000 )
	ROM_CONTINUE(          0x060000, 0x008000 )
	ROM_CONTINUE(          0x0e8000, 0x008000 )
	ROM_CONTINUE(          0x070000, 0x008000 )
	ROM_CONTINUE(          0x0f8000, 0x008000 )
	ROM_LOAD( "ff2_2.u7", 0x080000, 0x008000, CRC(b2078ae5) SHA1(e7bc3ad26ed672707d0dcfcaff238aad74986532) )
	ROM_CONTINUE(          0x008000, 0x008000 )
	ROM_CONTINUE(          0x090000, 0x008000 )
	ROM_CONTINUE(          0x018000, 0x008000 )
	ROM_CONTINUE(          0x0a0000, 0x008000 )
	ROM_CONTINUE(          0x028000, 0x008000 )
	ROM_CONTINUE(          0x0b0000, 0x008000 )
	ROM_CONTINUE(          0x038000, 0x008000 )
	ROM_CONTINUE(          0x0c0000, 0x008000 )
	ROM_CONTINUE(          0x048000, 0x008000 )
	ROM_CONTINUE(          0x0d0000, 0x008000 )
	ROM_CONTINUE(          0x058000, 0x008000 )
	ROM_CONTINUE(          0x0e0000, 0x008000 )
	ROM_CONTINUE(          0x068000, 0x008000 )
	ROM_CONTINUE(          0x0f0000, 0x008000 )
	ROM_CONTINUE(          0x078000, 0x008000 )
	ROM_LOAD( "ff2_1.u8", 0x100000, 0x040000, CRC(ea315ac1) SHA1(a85de091882d35bc77dc99677511828ff7c20350) )

	ROM_REGION(0x100,           "user5", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)

ROM_END

ROM_START( sblast2b )
	ROM_REGION( 0x180000, "user3", ROMREGION_ERASEFF )

	ROM_REGION(0x100,           "user5", 0)
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )

	ROM_REGION(0x800,           "user6", ROMREGION_ERASEFF)

	ROM_REGION( 0x180000, "user7", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x0080000, CRC(bea10c40) SHA1(d9cc65267b9b57145d714f2c17b436c1fb21513f) )
	ROM_LOAD( "2.bin", 0x080000, 0x0080000, CRC(57d2b6e9) SHA1(1a7b347101f67b254e2f86294d501b0669431644) )
	ROM_LOAD( "3.bin", 0x100000, 0x0080000, CRC(9e63a5ce) SHA1(1d18606fbb28b55a921fc37e1af1aff4caae9003) )

ROM_END

GAME( 199?, kinstb,       0,     kinstb,	     kinstb,    kinstb,		ROT0, "bootleg",	"Killer Instinct (SNES bootleg)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, ffight2b,     0,     kinstb,	     ffight2b,  ffight2b,	ROT0, "bootleg",	"Final Fight 2 (SNES bootleg)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, sblast2b,     0,     kinstb,	     sblast2b,  sblast2b,	ROT0, "bootleg",	"Sonic Blast Man 2 Special Turbo (SNES bootleg)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS)
