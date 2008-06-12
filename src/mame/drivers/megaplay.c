/* Sega MegaPlay

  changelog:

  22 Sept 2007 - Started updating this to use the new Megadrive code,
                 fixing issues with Mazin Wars + Grand Slam.
                 However I'm still not convinced that the handling of
                 the Megaplay side of things is correct at all, and
                 we're still hanging off the old SMS vdp code and
                 IO code.

*/

/*

About MegaPlay:

Megaplay games are specially designed Genesis games, produced for arcade use.

The code of these games has significant modifications when compared to the Genesis
releases and in many cases the games are cut-down versions of the games that were
released for the home system.  For example, Sonic has less zones, and no special
stages, thus making it impossible to get all the chaos emeralds.  Zones also have a
strict timer.

Coins buy you credits on Megaplay games, meaning if you lose all your lives the game is
over, like a regular Arcade game.

Like the Megatech the Megaplay boards have an additional Z80 and SMS VDP chip when compared
to the standard Genesis hardware.  In this case the additional hardware creates a layer
which is displayed as an overlay to the game screen.  This layer contains basic text
such as Insert Coin, and the Megaplay Logo / Instructions during the attract loop.

Communication between the various CPUs seems to be fairly complex and it is not fully
understood what is shared, where, and how.  One of the BIOS sets doesn't work, maybe for
this reason.

Only a handful of games were released for this system.

*/

#include "driver.h"
#include "deprecat.h"
#include "genesis.h"
#include "megadriv.h"

#define MASTER_CLOCK		53693100

/* Megaplay BIOS specific */
#define MP_ROM  0x10
#define MP_GAME 0

static UINT32 bios_bank; // ROM bank selection
static UINT16 game_banksel;  // Game bank selection
static UINT32 bios_mode = MP_ROM;  // determines whether ROM banks or Game data
                                  // is to read from 0x8000-0xffff
static UINT32 bios_width;  // determines the way the game info ROM is read
UINT8 bios_ctrl[6];
static UINT8 bios_6600;
UINT8 bios_6204;
static UINT8 bios_6403;
static UINT8 bios_6404;
static UINT8* ic3_ram;

static UINT8* ic37_ram;
UINT16 *ic36_ram;

//static UINT8 ic36_ram[0x4000];


static UINT8 hintcount;			/* line interrupt counter, decreased each scanline */
extern UINT8 segae_vintpending;
extern UINT8 segae_hintpending;
extern UINT8 *segae_vdp_regs[];		/* pointer to vdp's registers */

// Interrupt handler - from drivers/segasyse.c
static INTERRUPT_GEN (megaplay_bios_irq)
{
	int sline;
	sline = 261 - cpu_getiloops();

	if (sline ==0) {
		hintcount = segae_vdp_regs[0][10];
	}

	if (sline <= 192) {

//      if (sline != 192) segae_drawscanline(sline,1,1);

		if (sline == 192)
			segae_vintpending = 1;

		if (hintcount == 0) {
			hintcount = segae_vdp_regs[0][10];
			segae_hintpending = 1;

			if  ((segae_vdp_regs[0][0] & 0x10)) {
				cpunum_set_input_line(machine, 2, 0, HOLD_LINE);
				return;
			}

		} else {
			hintcount--;
		}
	}

	if (sline > 192) {
		hintcount = segae_vdp_regs[0][10];

		if ( (sline<0xe0) && (segae_vintpending) ) {
			cpunum_set_input_line(machine, 2, 0, HOLD_LINE);
		}
	}

}


static UINT32 readpos = 1;  // serial bank selection position (9-bit)

#define MEGAPLAY_TEST \
	PORT_START \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Select") PORT_CODE(KEYCODE_0) \
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 1") PORT_CODE(KEYCODE_W) \
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 2") PORT_CODE(KEYCODE_E) \
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 3") PORT_CODE(KEYCODE_R) \
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 4") PORT_CODE(KEYCODE_T) \
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 5") PORT_CODE(KEYCODE_Y) \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 6") PORT_CODE(KEYCODE_U) \
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

#define MEGAPLAY_COIN \
	PORT_START \
 	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) \
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) \
    PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) \
    PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) \
    PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) \
    PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
    PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
    PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* Caused 01081:
 *  PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 ) \
 *  PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_START2 )
 */

#define MEGAPLAY_DSWA \
	PORT_START \
	PORT_DIPNAME( 0x0f, 0x0f, "Coin slot 1" ) PORT_DIPLOCATION("SW1:1,2,3,4") \
    PORT_DIPSETTING( 0x07, DEF_STR( 4C_1C ) ) \
    PORT_DIPSETTING( 0x08, DEF_STR( 3C_1C ) ) \
    PORT_DIPSETTING( 0x09, DEF_STR( 2C_1C ) ) \
    PORT_DIPSETTING( 0x05, "2 coins/1 credit - 5 coins/3 credits - 6 coins/4 credits" ) \
    PORT_DIPSETTING( 0x04, "2 coins/1 credit - 4 coins/3 credits" ) \
    PORT_DIPSETTING( 0x0f, DEF_STR( 1C_1C ) ) \
    PORT_DIPSETTING( 0x06, DEF_STR( 2C_3C ) ) \
    PORT_DIPSETTING( 0x0e, DEF_STR( 1C_2C ) ) \
    PORT_DIPSETTING( 0x0d, DEF_STR( 1C_3C ) ) \
    PORT_DIPSETTING( 0x0c, DEF_STR( 1C_4C ) ) \
    PORT_DIPSETTING( 0x0b, DEF_STR( 1C_5C ) ) \
    PORT_DIPSETTING( 0x0a, DEF_STR( 1C_6C ) ) \
    PORT_DIPSETTING( 0x03, "1 coin/1 credit - 5 coins/6 credits" ) \
    PORT_DIPSETTING( 0x02, "1 coin/1 credit - 4 coins/5 credits" ) \
    PORT_DIPSETTING( 0x01, "1 coin/1 credit - 2 coins/3 credits" ) \
    PORT_DIPSETTING( 0x00, DEF_STR( Free_Play ) )

#define MEGAPLAY_DSWB \
	PORT_DIPNAME( 0xf0, 0xf0, "Coin slot 2" ) PORT_DIPLOCATION("SW2:1,2,3,4")\
    PORT_DIPSETTING( 0x70, DEF_STR( 4C_1C ) ) \
    PORT_DIPSETTING( 0x80, DEF_STR( 3C_1C ) ) \
    PORT_DIPSETTING( 0x90, DEF_STR( 2C_1C ) ) \
    PORT_DIPSETTING( 0x50, "2 coins/1 credit - 5 coins/3 credits - 6 coins/4 credits" ) \
    PORT_DIPSETTING( 0x40, "2 coins/1 credit - 4 coins/3 credits" ) \
    PORT_DIPSETTING( 0xf0, DEF_STR( 1C_1C ) ) \
    PORT_DIPSETTING( 0x60, DEF_STR( 2C_3C ) ) \
    PORT_DIPSETTING( 0xe0, DEF_STR( 1C_2C ) ) \
    PORT_DIPSETTING( 0xd0, DEF_STR( 1C_3C ) ) \
    PORT_DIPSETTING( 0xc0, DEF_STR( 1C_4C ) ) \
    PORT_DIPSETTING( 0xb0, DEF_STR( 1C_5C ) ) \
    PORT_DIPSETTING( 0xa0, DEF_STR( 1C_6C ) ) \
    PORT_DIPSETTING( 0x30, "1 coin/1 credit - 5 coins/6 credits" ) \
    PORT_DIPSETTING( 0x20, "1 coin/1 credit - 4 coins/5 credits" ) \
    PORT_DIPSETTING( 0x10, "1 coin/1 credit - 2 coins/3 credits" ) \
    PORT_DIPSETTING( 0x00, " 1 coin/1 credit" )

static INPUT_PORTS_START ( megaplay )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_sonic )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, "Initial Players" ) PORT_DIPLOCATION("SW3:1,2")
    PORT_DIPSETTING( 0x00, "4" )
    PORT_DIPSETTING( 0x01, "3" )
    PORT_DIPSETTING( 0x02, "2" )
    PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
    PORT_DIPSETTING( 0x00, DEF_STR( Hardest ) )
    PORT_DIPSETTING( 0x04, DEF_STR( Hard ) )
    PORT_DIPSETTING( 0x08, DEF_STR( Easy ) )
    PORT_DIPSETTING( 0x0c, DEF_STR( Normal ) )
    // Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gaxe2 )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:1")
    PORT_DIPSETTING( 0x01, DEF_STR( Normal ) )
    PORT_DIPSETTING( 0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, "Life" ) PORT_DIPLOCATION("SW3:2")
    PORT_DIPSETTING( 0x02, "1" )
    PORT_DIPSETTING( 0x00, "2" )
	PORT_DIPNAME( 0x04, 0x04, "Initial Players" ) PORT_DIPLOCATION("SW3:3")
    PORT_DIPSETTING( 0x00, "1" )
    PORT_DIPSETTING( 0x04, "2" )
	PORT_DIPNAME( 0x08, 0x00, "Timer" ) PORT_DIPLOCATION("SW3:4")
    PORT_DIPSETTING( 0x08, DEF_STR( Off )  )
    PORT_DIPSETTING( 0x00, DEF_STR( On ) )
    // Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

#ifdef UNUSED_DEFINITION
static INPUT_PORTS_START ( mp_col3 )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
    PORT_DIPSETTING( 0x01, DEF_STR( English ) )
    PORT_DIPSETTING( 0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x02, 0x02, "2P Mode Games" ) PORT_DIPLOCATION("SW3:2")
    PORT_DIPSETTING( 0x02, "1" )
    PORT_DIPSETTING( 0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x0c, "Speed / Difficulty" ) PORT_DIPLOCATION("SW3:3,4")
    PORT_DIPSETTING( 0x08, "Slow"  )
    PORT_DIPSETTING( 0x0c, "Middle"  )
    PORT_DIPSETTING( 0x04, "Fast"  )
    PORT_DIPSETTING( 0x00, "Max"  )
    // Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END
#endif

static INPUT_PORTS_START ( mp_twc )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x01, 0x01, "Time" ) PORT_DIPLOCATION("SW3:1")
    PORT_DIPSETTING( 0x01, DEF_STR( Normal ) )
    PORT_DIPSETTING( 0x00, "Short" )
	PORT_DIPNAME( 0x0e, 0x08, "Level" ) PORT_DIPLOCATION("SW3:2,3,4")
    PORT_DIPSETTING( 0x00, "0" )
    PORT_DIPSETTING( 0x02, "0" )
    PORT_DIPSETTING( 0x04, "5" )
    PORT_DIPSETTING( 0x06, "4" )
    PORT_DIPSETTING( 0x08, "3" )
    PORT_DIPSETTING( 0x0a, "2" )
    PORT_DIPSETTING( 0x0c, "1" )
    PORT_DIPSETTING( 0x0e, "0" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_sor2 )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
    PORT_DIPSETTING( 0x00, "4" )
    PORT_DIPSETTING( 0x01, "3" )
    PORT_DIPSETTING( 0x02, "2" )
    PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
    PORT_DIPSETTING( 0x00, DEF_STR ( Hardest ) )
    PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
    PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
    PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_bio )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
    PORT_DIPSETTING( 0x00, "5" )
    PORT_DIPSETTING( 0x01, "4" )
    PORT_DIPSETTING( 0x02, "2" )
    PORT_DIPSETTING( 0x03, "3" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
    PORT_DIPSETTING( 0x00, DEF_STR ( Hardest ) )
    PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
    PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
    PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gslam )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x07, 0x04, DEF_STR ( Game_Time ) ) PORT_DIPLOCATION("SW3:1,2,3")
    PORT_DIPSETTING( 0x00, "5:00" )
    PORT_DIPSETTING( 0x01, "4:30" )
    PORT_DIPSETTING( 0x02, "4:00" )
    PORT_DIPSETTING( 0x03, "3:30" )
    PORT_DIPSETTING( 0x04, "3:00" )
    PORT_DIPSETTING( 0x05, "2:30" )
    PORT_DIPSETTING( 0x06, "2:00" )
    PORT_DIPSETTING( 0x07, "1:30" )
	PORT_DIPNAME( 0x08, 0x08, "2P-Play Continue" ) PORT_DIPLOCATION("SW3:4")
    PORT_DIPSETTING( 0x00, "1 Credit" )
    PORT_DIPSETTING( 0x08, "2 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_mazin )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x01, 0x01, "Initial Player" ) PORT_DIPLOCATION("SW3:1")
    PORT_DIPSETTING( 0x01, "2" )
    PORT_DIPSETTING( 0x00, "1" )
	PORT_DIPNAME( 0x02, 0x02, "Initial Player" ) PORT_DIPLOCATION("SW3:2")
    PORT_DIPSETTING( 0x02, "1" )
    PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3")
    PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
    PORT_DIPSETTING( 0x00, DEF_STR ( Normal ) )
	PORT_DIPNAME( 0x08, 0x08, "Title" ) PORT_DIPLOCATION("SW3:4")
    PORT_DIPSETTING( 0x08, "EUROPE" )
    PORT_DIPSETTING( 0x00, "U.S.A" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_soni2 )
	GENESIS_PORTS
	MEGAPLAY_TEST
	MEGAPLAY_COIN
	MEGAPLAY_DSWA
	MEGAPLAY_DSWB

	PORT_START
	// DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, "Initial Players (Normal mode)" ) PORT_DIPLOCATION("SW3:1,2")
    PORT_DIPSETTING( 0x00, "4" )
    PORT_DIPSETTING( 0x01, "3" )
    PORT_DIPSETTING( 0x02, "2" )
    PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0x0c, 0x0c, "Initial Players (Dual mode)" ) PORT_DIPLOCATION("SW3:3,4")
    PORT_DIPSETTING( 0x00, "4" )
    PORT_DIPSETTING( 0x04, "2" )
    PORT_DIPSETTING( 0x08, "1" )
    PORT_DIPSETTING( 0x0c, "3" )
INPUT_PORTS_END

/*MEGAPLAY specific*/

static READ8_HANDLER( megaplay_bios_banksel_r )
{
	return bios_bank;
}

static WRITE8_HANDLER( megaplay_bios_banksel_w )
{
/*  Multi-slot note:
    Bits 0 and 1 appear to determine the selected game slot.
    It should be possible to multiplex different game ROMs at
    0x000000-0x3fffff based on these bits.
*/
	bios_bank = data;
	bios_mode = MP_ROM;
//  logerror("BIOS: ROM bank %i selected [0x%02x]\n",bios_bank >> 6, data);
}

static READ8_HANDLER( megaplay_bios_gamesel_r )
{
	return bios_6403;
}

static WRITE8_HANDLER( megaplay_bios_gamesel_w )
{
	bios_6403 = data;

//  logerror("BIOS: 0x6403 write: 0x%02x\n",data);
	bios_mode = data & 0x10;
}


static READ8_HANDLER( bank_r )
{
	UINT8* bank = memory_region(REGION_CPU3);
	UINT8* game = memory_region(REGION_CPU1);

	if(game_banksel == 0x142) // Genesis I/O
		return megaplay_genesis_io_r(machine, (offset & 0x1f) / 2, 0xffff);

	if(bios_mode & MP_ROM)
	{
		int sel = (bios_bank >> 6) & 0x03;

//      popmessage("Reading from Bank %i",sel);

		if(sel == 0)
			return 0xff;
		else
			return bank[0x10000 + (sel-1)*0x8000 + offset];
	}
	else
	{
		if(game_banksel == 0x60 || game_banksel == 0x61)  /* read game info ROM */
			if(bios_width & 0x08)
			{
				if(offset >= 0x2000)
					return ic36_ram[offset - 0x2000];
				else
					return ic37_ram[(0x2000 * (bios_bank & 0x03)) + offset];
			}
			else
				return game[((game_banksel)*0x8000 + offset)];
		else
			return game[(game_banksel*0x8000 + (offset ^ 0x01))];
	}
}

static WRITE8_HANDLER ( bank_w )
{
	if(game_banksel == 0x142) // Genesis I/O
		genesis_io_w(machine, (offset & 0x1f) / 2, data, 0xffff);

	if(offset <= 0x1fff && (bios_width & 0x08))
		ic37_ram[(0x2000 * (bios_bank & 0x03)) + offset] = data;

	if(offset >= 0x2000 && (bios_width & 0x08))
//      ic36_ram[offset] = data;
		ic36_ram[offset - 0x2000] = data;
}


static READ8_HANDLER( megaplay_bios_6402_r )
{
	return genesis_io_ram[3];// & 0xfe;
//  return bios_6402;// & 0xfe;
}

static WRITE8_HANDLER( megaplay_bios_6402_w )
{
	genesis_io_ram[3] = (genesis_io_ram[3] & 0x07) | ((data & 0x70) >> 1);
//  bios_6402 = (data >> 4) & 0x07;
//  logerror("BIOS: 0x6402 write: 0x%02x\n",data);
}

static READ8_HANDLER( megaplay_bios_6404_r )
{
//  logerror("BIOS: 0x6404 read: returned 0x%02x\n",bios_6404 | (bios_6403 & 0x10) >> 4);
	return (bios_6404 & 0xfe) | ((bios_6403 & 0x10) >> 4);
//  return bios_6404 | (bios_6403 & 0x10) >> 4;
}

static WRITE8_HANDLER( megaplay_bios_6404_w )
{
	if(((bios_6404 & 0x0c) == 0x00) && ((data & 0x0c) == 0x0c))
		cpunum_set_input_line(machine, 0, INPUT_LINE_RESET, PULSE_LINE);
	bios_6404 = data;

//  logerror("BIOS: 0x6404 write: 0x%02x\n",data);
}

static READ8_HANDLER( megaplay_bios_6204_r )
{
	return (genesis_io_ram[3]);
//  return (bios_width & 0xf8) + (bios_6204 & 0x07);
}

static WRITE8_HANDLER( megaplay_bios_width_w )
{
	bios_width = data;
	genesis_io_ram[3] = (genesis_io_ram[3] & 0x07) | ((data & 0xf8));

//  logerror("BIOS: 0x6204 - Width write: %02x\n",data);
}

static READ8_HANDLER( megaplay_bios_6600_r )
{
/*  Multi-slot note:
    0x6600 appears to be used to check for extra slots being used.
    Enter the following line in place of the return statement in this
    function to make the BIOS check all 4 slots (3 and 4 will be "not used")
        return (bios_6600 & 0xfe) | (bios_bank & 0x01);
*/
	return bios_6600;// & 0xfe;
}

static WRITE8_HANDLER( megaplay_bios_6600_w )
{
	bios_6600 = data;
//  logerror("BIOS: 0x6600 write: 0x%02x\n",data);
}

static WRITE8_HANDLER( megaplay_game_w )
{
	if(readpos == 1)
		game_banksel = 0;
	game_banksel |= (1 << (readpos-1)) * (data & 0x01);

	readpos++;
	if(readpos > 9)
	{
		bios_mode = MP_GAME;
		readpos = 1;
//      popmessage("Game bank selected: 0x%03x",game_banksel);
		logerror("BIOS [0x%04x]: 68K address space bank selected: 0x%03x\n",activecpu_get_previouspc(),game_banksel);
	}
}

static ADDRESS_MAP_START( megaplay_bios_readmem, ADDRESS_SPACE_PROGRAM, 8 )
 	AM_RANGE(0x0000, 0x3fff) AM_READ(SMH_ROM)
	AM_RANGE(0x4000, 0x4fff) AM_READ(SMH_RAM)
	AM_RANGE(0x5000, 0x5fff) AM_READ(SMH_RAM)
	AM_RANGE(0x6200, 0x6200) AM_READ(input_port_7_r)
	AM_RANGE(0x6201, 0x6201) AM_READ(input_port_8_r)
	AM_RANGE(0x6400, 0x6400) AM_READ(input_port_5_r)
	AM_RANGE(0x6401, 0x6401) AM_READ(input_port_6_r)
	AM_RANGE(0x6204, 0x6204) AM_READ(megaplay_bios_6204_r)
	AM_RANGE(0x6203, 0x6203) AM_READ(megaplay_bios_banksel_r)
	AM_RANGE(0x6402, 0x6402) AM_READ(megaplay_bios_6402_r)
	AM_RANGE(0x6403, 0x6403) AM_READ(megaplay_bios_gamesel_r)
	AM_RANGE(0x6404, 0x6404) AM_READ(megaplay_bios_6404_r)
	AM_RANGE(0x6600, 0x6600) AM_READ(megaplay_bios_6600_r)
	AM_RANGE(0x6800, 0x77ff) AM_READ(SMH_RAM)
	AM_RANGE(0x8000, 0xffff) AM_READ(bank_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( megaplay_bios_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x5000, 0x5fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(megaplay_game_w)
	AM_RANGE(0x6203, 0x6203) AM_WRITE(megaplay_bios_banksel_w)
	AM_RANGE(0x6204, 0x6204) AM_WRITE(megaplay_bios_width_w)
	AM_RANGE(0x6402, 0x6402) AM_WRITE(megaplay_bios_6402_w)
	AM_RANGE(0x6403, 0x6403) AM_WRITE(megaplay_bios_gamesel_w)
	AM_RANGE(0x6404, 0x6404) AM_WRITE(megaplay_bios_6404_w)
	AM_RANGE(0x6600, 0x6600) AM_WRITE(megaplay_bios_6600_w)
	AM_RANGE(0x6001, 0x67ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x6800, 0x77ff) AM_WRITE(SMH_RAM) AM_BASE(&ic3_ram)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(bank_w)
ADDRESS_MAP_END

/* basically from src/drivers/segasyse.c */
UINT8 segae_vdp_ctrl_r ( UINT8 chip );
UINT8 segae_vdp_data_r ( UINT8 chip );
void segae_vdp_ctrl_w ( UINT8 chip, UINT8 data );
void segae_vdp_data_w ( running_machine *machine, UINT8 chip, UINT8 data );

static READ8_HANDLER (megaplay_bios_port_be_bf_r)
{
	UINT8 temp = 0;

	switch (offset)
	{
		case 0: /* port 0xbe, VDP 1 DATA Read */
			temp = segae_vdp_data_r(0); break ;
		case 1: /* port 0xbf, VDP 1 CTRL Read */
			temp = segae_vdp_ctrl_r(0); break ;
	}
	return temp;
}

static WRITE8_HANDLER (megaplay_bios_port_be_bf_w)
{
	switch (offset)
	{
		case 0: /* port 0xbe, VDP 1 DATA Write */
			segae_vdp_data_w(machine, 0, data); break;
		case 1: /* port 0xbf, VDP 1 CTRL Write */
			segae_vdp_ctrl_w(0, data); break;
	}
}

static ADDRESS_MAP_START( megaplay_bios_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0xdc, 0xdc) AM_READ(megatech_bios_port_dc_r)  // player inputs
//  AM_RANGE(0xdd, 0xdd) AM_READ(megatech_bios_port_dd_r)  // other player 2 inputs
	AM_RANGE(0xbe, 0xbf) AM_READ(megaplay_bios_port_be_bf_r)			/* VDP */
ADDRESS_MAP_END

static ADDRESS_MAP_START( megaplay_bios_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x3f, 0x3f) AM_WRITE(megatech_bios_port_ctrl_w)
	AM_RANGE(0x7f, 0x7f) AM_WRITE(SN76496_1_w)	/* SN76489 */
	AM_RANGE(0xbe, 0xbf) AM_WRITE(megaplay_bios_port_be_bf_w)			/* VDP */
ADDRESS_MAP_END


/* in video/segasyse.c */
VIDEO_START( megaplay_normal );
VIDEO_UPDATE( megaplay_normal );

static VIDEO_START(megplay)
{
	//printf("megplay vs\n");
	VIDEO_START_CALL(megadriv);
	VIDEO_START_CALL(megaplay_normal);
}

static VIDEO_UPDATE(megplay)
{
	//printf("megplay vu\n");
	VIDEO_UPDATE_CALL(megadriv);
	VIDEO_UPDATE_CALL(megaplay_normal);
	return 0;
}


//extern VIDEO_EOF(megadriv);

static MACHINE_DRIVER_START( mpnew )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(megadriv)

	/* The Megaplay has an extra BIOS cpu which drives an SMS VDP
       which includes an SN76496 for sound */
	MDRV_CPU_ADD_TAG("megaplay_bios", Z80, MASTER_CLOCK / 15) /* ?? */
	MDRV_CPU_PROGRAM_MAP(megaplay_bios_readmem, megaplay_bios_writemem)
	MDRV_CPU_IO_MAP(megaplay_bios_readport,megaplay_bios_writeport)
	MDRV_CPU_VBLANK_INT_HACK(megaplay_bios_irq, 262)

	MDRV_INTERLEAVE(100)

	MDRV_SOUND_ADD(SN76496, MASTER_CLOCK/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.25) /* 3.58 MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right",0.25) /* 3.58 MHz */

	/* New update functions to handle the extra layer */
	MDRV_VIDEO_START(megplay)
	MDRV_VIDEO_UPDATE(megplay)
MACHINE_DRIVER_END


/* MegaPlay Games - Modified Genesis games */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define MEGAPLAY_BIOS \
	ROM_SYSTEM_BIOS( 0, "ver1",       "Megaplay Bios (Ver. 1)" ) \
	ROM_LOAD_BIOS( 0, "ep15294.ic2",   0x000000, 0x20000, CRC(aa8dc2d8) SHA1(96771ad7b79dc9c83a1594243250d65052d23176) ) \
	ROM_SYSTEM_BIOS( 1, "ver2",       "Megaplay Bios (Ver. 2)" ) /* this one doesn't boot .. dump was verified with another working pcb */ \
	ROM_LOAD_BIOS( 1, "epr-a15294.ic2",0x000000, 0x20000, CRC(f97c68aa) SHA1(bcabc879950bca1ced11c550a484e697ec5706b2) ) \

ROM_START( megaplay )
	ROM_REGION( 0x400000, REGION_CPU1, ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, REGION_USER1, ROMREGION_ERASEFF )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_sonic ) /* Sonic */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "ep15177.ic2", 0x000000, 0x040000, CRC(a389b03b) SHA1(8e9e1cf3dd65ddf08757f5a1ce472130c902ea2c) )
	ROM_LOAD16_BYTE( "ep15176.ic1", 0x000001, 0x040000, CRC(d180cc21) SHA1(62805cfaaa80c1da6146dd89fc2b49d819fd4f22) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "ep15175-01.ic3", 0x000000, 0x08000, CRC(99246889) SHA1(184aa3b7fdedcf578c5e34edb7ed44f57f832258) )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

/* this cart looks to be a conversion from something else.. sega rom numbers were missing
   but the code looks like it's probably real */
/* pcb  171-5834 */
ROM_START( mp_col3 ) /* Columns 3 */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "3.ic2", 0x000000, 0x040000, CRC(a1602235) SHA1(38751b585849c8966acc3f508714937fe29dcf5c) )
	ROM_LOAD16_BYTE( "2.ic1", 0x000001, 0x040000, CRC(999b2fe6) SHA1(ad967a28e4eebd7b01273e4e04c35a0198ef834a) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "1.ic3", 0x000000, 0x08000,  CRC(dac9bf91) SHA1(0117972a7181f8aaf942a259cc8764b821031253) )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_gaxe2 ) /* Golden Axe 2 */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "ep15179b.ic2", 0x000000, 0x040000, CRC(00d97b84) SHA1(914bbf566ddf940aab67b92af237d251650ddadf) )
	ROM_LOAD16_BYTE( "ep15178b.ic1", 0x000001, 0x040000, CRC(2ea576db) SHA1(6d96b948243533de1f488b1f80e0d5431a4f1f53) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "ep15175-02b.ic3", 0x000000, 0x08000, CRC(3039b653) SHA1(b19874c74d0fc0cca1169f62e5e74f0e8ca83679) ) // 15175-02b.ic3

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_gslam ) /* Grand Slam */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "epr-15181.ic2", 0x000000, 0x040000, CRC(642437c1) SHA1(cbf88e196c04b6d886bf9642b69bf165045510fe) )
	ROM_LOAD16_BYTE( "epr-15180.ic1", 0x000001, 0x040000, CRC(73bb48f1) SHA1(981b64f834d5618599352f5fad683bf232390ba3) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-03.ic3", 0x000000, 0x08000, CRC(70ea1aec) SHA1(0d9d82a1f8aa51d02707f7b343e7cfb6591efccd) ) // 15175-02b.ic3

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END


ROM_START( mp_twc ) /* Tecmo World Cup */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "ep15183.ic2", 0x000000, 0x040000, CRC(8b79b861) SHA1(c72af72840513b82f2562409eccdf13b031bf3c0) )
	ROM_LOAD16_BYTE( "ep15182.ic1", 0x000001, 0x040000, CRC(eb8325c3) SHA1(bb21ac926c353e14184dd476222bc6a8714606e5) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

 	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "ep15175-04.ic3", 0x000000, 0x08000, CRC(faf7c030) SHA1(16ef405335b4d3ecb0b7d97b088dafc4278d4726) )

 	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
 	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_sor2 ) /* Streets of Rage 2 */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-15425.ic1", 0x000000, 0x200000, CRC(cd6376af) SHA1(57ec210975e40505649f152b60ef54f99da31f0e) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-05.ic2", 0x000000, 0x08000, CRC(1df5347c) SHA1(faced2e875e1914392f61577b5256d006eebeef9) )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_bio ) /* Bio Hazard Battle */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-15699-f.ic1", 0x000000, 0x100000, CRC(4b193229) SHA1(f8629171ae9b4792f142f6957547d886e5cc6817) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-06.ic2", 0x000000, 0x08000, CRC(1ef64e41) SHA1(13984b714b014ea41963b70de74a5358ed223bc5) )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_soni2 ) /* Sonic The Hedgehog 2 */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16011.ic1", 0x000000, 0x100000, CRC(3d7bf98a) SHA1(dce0e4e8f2573e0ffe851edaa235e4ed9e61ee2d) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-07.ic1", 0x000000, 0x08000, CRC(bb5f67f0) SHA1(33b7a5d14015a5fcf41976a8f648f8f48ce9bb03) )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_mazin ) /* Mazin Wars */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16460.ic1", 0x000000, 0x100000, CRC(e9635a83) SHA1(ab3afa11656f0ae3a50c957dce012fb15d3992e0) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, REGION_USER1, 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-11.ic2", 0x000000, 0x08000, CRC(bb651120) SHA1(81cb736f2732373e260dde162249c1d29a3489c3) )

	ROM_REGION( 0x28000, REGION_CPU3, 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END


static void megplay_stat(void)
{
	UINT8 *src = memory_region(REGION_CPU3);
	UINT8 *instruction_rom = memory_region(REGION_USER1);
	UINT8 *game_rom = memory_region(REGION_CPU1);
	int offs;


	memmove(src+0x10000,src+0x8000,0x18000); // move bios..

	/* copy game instruction rom to main map.. maybe this should just be accessed
      through a handler instead?.. */
	for (offs=0;offs<0x8000;offs++)
	{
		UINT8 dat;

		dat=instruction_rom[offs];

		game_rom[0x300000+offs*2] = dat;
		game_rom[0x300001+offs*2] = dat;

	}
}

/* Old code, todo, update the new code instead */
static READ16_HANDLER ( OLD_megaplay_genesis_io_r )
{
	/* 8-bit only, data is mirrored in both halves */

	UINT8 return_value = 0;

	switch (offset)
	{
		case 0:
		/* Charles MacDonald ( http://cgfm2.emuviews.com/ )
            D7 : Console is 1= Export (USA, Europe, etc.) 0= Domestic (Japan)
            D6 : Video type is 1= PAL, 0= NTSC
            D5 : Sega CD unit is 1= not present, 0= connected.
            D4 : Unused (always returns zero)
            D3 : Bit 3 of version number
            D2 : Bit 2 of version number
            D1 : Bit 1 of version number
            D0 : Bit 0 of version number
        */
			return_value = 0x80; /* ? megatech is usa? */
			break;

		case 1: /* port A data (joypad 1) */

			if (genesis_io_ram[offset] & 0x40)
				return_value = input_port_read_indexed(machine, 1) & (genesis_io_ram[4]^0xff);
			else
			{
				return_value = input_port_read_indexed(machine, 2) & (genesis_io_ram[4]^0xff);
				return_value |= input_port_read_indexed(machine, 1) & 0x03;
			}
			return_value = (genesis_io_ram[offset] & 0x80) | return_value;
//          logerror ("reading joypad 1 , type %02x %02x\n",genesis_io_ram[offset] & 0xb0, return_value &0x7f);
			break;

		case 2: /* port B data (joypad 2) */

			if (genesis_io_ram[offset] & 0x40)
				return_value = input_port_read_indexed(machine, 3) & (genesis_io_ram[5]^0xff);
			else
			{
				return_value = input_port_read_indexed(machine, 4) & (genesis_io_ram[5]^0xff);
				return_value |= input_port_read_indexed(machine, 3) & 0x03;
			}
			return_value = (genesis_io_ram[offset] & 0x80) | return_value;
//          logerror ("reading joypad 2 , type %02x %02x\n",genesis_io_ram[offset] & 0xb0, return_value &0x7f);
			break;

//      case 3: /* port C data */
//          return_value = bios_6402 << 3;
//          break;

	default:
			return_value = genesis_io_ram[offset];

	}
	return return_value | return_value << 8;
}

static WRITE16_HANDLER ( OLD_megaplay_genesis_io_w )
{
//  logerror ("write io offset :%02x data %04x PC: 0x%06x\n",offset,data,activecpu_get_previouspc());

	switch (offset)
	{
		case 0x00:
		/*??*/
		break;

		case 0x01:/* port A data */
		genesis_io_ram[offset] = (data & (genesis_io_ram[0x04])) | (genesis_io_ram[offset] & ~(genesis_io_ram[0x04]));
		break;

		case 0x02: /* port B data */
		genesis_io_ram[offset] = (data & (genesis_io_ram[0x05])) | (genesis_io_ram[offset] & ~(genesis_io_ram[0x05]));
		break;

		case 0x03: /* port C data */
		genesis_io_ram[offset] = (data & (genesis_io_ram[0x06])) | (genesis_io_ram[offset] & ~(genesis_io_ram[0x06]));
		bios_6204 = data & 0x07;
		break;

		case 0x04: /* port A control */
		genesis_io_ram[offset] = data;
		break;

		case 0x05: /* port B control */
		genesis_io_ram[offset] = data;
		break;

		case 0x06: /* port C control */
		genesis_io_ram[offset] = data;
		break;

		case 0x07: /* port A TxData */
		genesis_io_ram[offset] = data;
		break;

		default:
		genesis_io_ram[offset] = data;
	}
}


static READ16_HANDLER( megadriv_68k_read_z80_extra_ram )
{
	return ic36_ram[(offset<<1)^1] | (ic36_ram[(offset<<1)]<<8);
}

static WRITE16_HANDLER( megadriv_68k_write_z80_extra_ram )
{
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		ic36_ram[(offset<<1)] = (data & 0xff00) >> 8;
	}
	else if (!ACCESSING_BITS_8_15)
	{
		ic36_ram[(offset<<1)^1] = (data & 0x00ff);
	}
	else // for WORD access only the MSB is used, LSB is ignored
	{
		ic36_ram[(offset<<1)] = (data & 0xff00) >> 8;
	}
}


static DRIVER_INIT (megaplay)
{
	/* to support the old code.. */
	ic36_ram = auto_malloc(0x10000);
	ic37_ram = auto_malloc(0x10000);
	genesis_io_ram = auto_malloc(0x20);

	DRIVER_INIT_CALL(megadrij);
	megplay_stat();

	/* for now ... */
	memory_install_readwrite16_handler(machine, 0,  ADDRESS_SPACE_PROGRAM, 0xa10000, 0xa1001f, 0, 0, OLD_megaplay_genesis_io_r, OLD_megaplay_genesis_io_w);

	/* megaplay has ram shared with the bios cpu here */
	memory_install_readwrite8_handler(machine, 1,  ADDRESS_SPACE_PROGRAM, 0x2000, 0x3fff, 0, 0, SMH_BANK7, SMH_BANK7);
	memory_set_bankptr(7, &ic36_ram[0]);

	/* instead of a RAM mirror the 68k sees the extra ram of the 2nd z80 too */
	memory_install_readwrite16_handler(machine, 0,  ADDRESS_SPACE_PROGRAM, 0xa02000, 0xa03fff, 0, 0, megadriv_68k_read_z80_extra_ram, megadriv_68k_write_z80_extra_ram);
}

/*
Sega Mega Play Cartridges
-------------------------

These are cart-based games for use with Sega Mega Play hardware. There are 2 known types of carts. Both carts
are very simple, almost exactly the same as Mega Tech carts. They contain just 2 or 3 ROMs.
PCB 171-6215A has locations for 2 ROMs and is dated 1991.
PCB 171-5834 has locations for 3 ROMs and is dated 1989.

                                                                       |------------------------------- ROMs ------------------------------|
                                                                       |                                                                   |
Game                 PCB #       Sticker on PCB    Sticker on cart      IC1                     IC2                    IC3
-------------------------------------------------------------------------------------------------------------------------------------------
Sonic The Hedgehog                                    -    -01
Golden Axe 2                                          -    -02
Grand Slam           171-5834    837-9165-03       610-0297-03          EPR-15180   (27C020)    EPR-15181    (27C020)  EPR-15175-03 (27256)
Tecmo World Cup                                       -    -04
Columns 3            171-5834                      610-0297-04*         2           (27C020)    3            (27C020)  1            (27256)
Streets Of Rage II   171-6215A   837-9165-05       610-0297-05          MPR-15425   (8316200A)  EPR-15175-05 (27256)   n/a
Bio-Hazard Battle    171-6215A   837-9165-06       610-0298-06          MPR-15699-F (838200)    EPR-15175-06 (27256)   n/a
Sonic The Hedgehog 2 171-6215A   837-9165-07       610-0297-07          MPR-16011   (838200)    EPR-15175-07 (27256)   n/a
Mazin Wars           171-6215A   837-9165-11       610-0297-11          MPR-16460   (838200)    EPR-15175-11 (27256)   n/a

* This is the code for Tecmo World Cup, as the ROMs in the Columns 3 cart
didn't have original Sega part numbers it's probably a converted TWC cart

*/

/* -- */ GAME( 1993, megaplay, 0, mpnew, megaplay, megaplay, ROT0, "Sega",                  "Mega Play BIOS", GAME_IS_BIOS_ROOT )
/* 01 */ GAME( 1993, mp_sonic, megaplay, mpnew, mp_sonic, megaplay, ROT0, "Sega",                  "Sonic The Hedgehog (Mega Play)" , 0 )
/* 02 */ GAME( 1993, mp_gaxe2, megaplay, mpnew, mp_gaxe2, megaplay, ROT0, "Sega",                  "Golden Axe II (Mega Play)" , 0 )
/* 03 */ GAME( 1993, mp_gslam, megaplay, mpnew, mp_gslam, megaplay, ROT0, "Sega",                  "Grand Slam (Mega Play)",0  )
/* 04 */ GAME( 1993, mp_twc,   megaplay, mpnew, mp_twc,	 megaplay, ROT0, "Sega",                  "Tecmo World Cup (Mega Play)" , 0 )
/* 05 */ GAME( 1993, mp_sor2,  megaplay, mpnew, mp_sor2,	 megaplay, ROT0, "Sega",                  "Streets of Rage II (Mega Play)" , 0 )
/* 06 */ GAME( 1993, mp_bio,   megaplay, mpnew, mp_bio,   megaplay, ROT0, "Sega",                  "Bio-hazard Battle (Mega Play)" , 0 )
/* 07 */ GAME( 1993, mp_soni2, megaplay, mpnew, mp_soni2, megaplay, ROT0, "Sega",                  "Sonic The Hedgehog 2 (Mega Play)" , 0 )
/* 08 */
/* 09 */
/* 10 */
/* 11 */ GAME( 1993, mp_mazin, megaplay, mpnew, mp_mazin, megaplay, ROT0, "Sega",                  "Mazin Wars / Mazin Saga (Mega Play)",0  )

/* ?? */ GAME( 1993, mp_col3,  megaplay, mpnew, megaplay, megaplay, ROT0, "Sega",                  "Columns III (Mega Play)" , 0 )


/* Also known to exist:
Gunstar Heroes
+ at least 2 others?

*/
