/*
RDI Video Systems Thayer's Quest laserdisc hardware
Driver by Andrew Gardner from schematics with help from Daphne Source

Notes:
    How odd.  The schematics say there's a COP421L hooked up, but they also make mention of
      a COP404L - A ROMless COP420 with 1k of RAM.  I wonder which one is correct?

Todo:
    Add SIO to the cop420 core.
    Fix up my poor interpretation of the hard-to-read schematics.
    Write a real SC-01 (SSI-263) emulator for MAME.
    Convert to tilemaps.
*/

#include "driver.h"
#include "render.h"
#include "machine/laserdsc.h"
#include "cpu/cop400/cop400.h"

#define SCHEMATIC_CLOCK (4000000)

/*
 * Z80 IRQ Status bit
 * ------------------
 *
 * Bit      | Use
 * 76543210-+----------------
 * -------x | Unused
 * ------x- | Unused
 * -----x-- | SSI-263 Request Data.
 * ----x--- | always high. (? but it's checked at 0x1bb ?)
 * ---x---- | /TIMER INT.
 * --x----- | /DATA RDY INT.
 * -x------ | /CART PRES.
 * x------- | Unused
 */
static UINT8 z80_irq_status;

#define Z80_IRQ_STATUS_SET()   (z80_irq_status = 0x00)
#define Z80_IRQ_STATUS_CLEAR() (z80_irq_status = 0xff)

#define Z80_SSI_REQUEST()  ((z80_irq_status & 0x04) >> 2)
#define Z80_TIMER_INT()    ((z80_irq_status & 0x10) >> 4)
#define Z80_DATA_RDY_INT() ((z80_irq_status & 0x20) >> 5)
#define Z80_CART_PRES()    ((z80_irq_status & 0x40) >> 6)

#define SET_Z80_SSI_REQUEST()  (z80_irq_status &= ~0x04)
#define SET_Z80_TIMER_INT()    (z80_irq_status &= ~0x10)
#define SET_Z80_DATA_RDY_INT() (z80_irq_status &= ~0x20)
#define SET_Z80_CART_PRES()    (z80_irq_status &= ~0x40)

#define CLEAR_Z80_SSI_REQUEST()  (z80_irq_status |= 0x04)
#define CLEAR_Z80_TIMER_INT()    (z80_irq_status |= 0x10)
#define CLEAR_Z80_DATA_RDY_INT() (z80_irq_status |= 0x20)
#define CLEAR_Z80_CART_PRES()    (z80_irq_status |= 0x40)


/* Misc variables */
static UINT8* ram_ic;
static laserdisc_info *discinfo;

static UINT8 m_ssi_control;
static UINT8 cop_g_latch;
static UINT8 cop_l_latch;



/* VIDEO GOODS */
static VIDEO_UPDATE( thayers )
{
	/* display disc information */
	if (discinfo != NULL)
		popmessage("%s", laserdisc_describe_state(discinfo));

	return 0;
}



/* MEMORY HANDLERS */
/* Z80 R/W */
static READ8_HANDLER(z80_irq_status_r)
{
	return z80_irq_status;
}

static READ8_HANDLER(ram_ic_read)
{
	/* Daphne patches.
    if (offset == 0xbe07-0x8000)
        return 0x02;

    if (offset == 0xbe17-0x8000)
        return 0x01;
    */

	return ram_ic[offset];
}

static CUSTOM_INPUT( laserdisc_status_r )
{
	if (discinfo == NULL)
		return 0;

	/* Someday handle multiple LD players */
	return (laserdisc_line_r(discinfo, LASERDISC_LINE_STATUS) == ASSERT_LINE) ? 0 : 1;
}

static CUSTOM_INPUT( laserdisc_command_r )
{
	if (discinfo == NULL)
		return 0;

	/* Someday handle multiple LD players */
	return (laserdisc_line_r(discinfo, LASERDISC_LINE_COMMAND) == ASSERT_LINE) ? 0 : 1;
}

static WRITE8_HANDLER(ram_ic_write)
{
	ram_ic[offset] = data;
}

static WRITE8_HANDLER(data_ready_int_w)
{
	CLEAR_Z80_DATA_RDY_INT();
	/* Hackaroo? */
	/* cop_l_latch = 0x00; */
}

static WRITE8_HANDLER(timer_int_w)
{
	CLEAR_Z80_TIMER_INT();
}

static WRITE8_HANDLER(write_cop)
{
	logerror("Writing 0x%x to COP port G\n", data);

	/* Bits 5-7 write to COP421 ports G0-G2 */
	cop_g_latch = (data >> 5) & 0x07;

	/* Bit 2 goes to BANKSELT */
	/* bankselect = (data & 0x04) >> 2; */

	/* Bit 1 goes to CS128x? (illegible schems) */
}

#ifdef UNUSED_FUNCTION
static WRITE8_HANDLER(data_write_cop)
{
	cop_l_latch = data;
}
#endif

static WRITE8_HANDLER(intrq_w)
{
	cpunum_set_input_line(0, 0, ASSERT_LINE);
}

static WRITE8_HANDLER(sc01_register_w)
{
	/* (Thank you Daphne) */
	switch (offset)
	{
		case 0x00:
			if (m_ssi_control)
			{
				logerror("SC-01 control mode : 0x%x\n", data);

				switch(data)
				{
					/* Stop requesting phonemes */
					case 0x00:
						CLEAR_Z80_SSI_REQUEST();
						break;

					/* Start adding phonemes */
					case 0xc0:
						SET_Z80_SSI_REQUEST();
				        if (!(Z80_SSI_REQUEST()))
							logerror("SETTING IRQ LINE sc01\n");
				            cpunum_set_input_line(0, 0, ASSERT_LINE);
						break;

					default:
						logerror("SC-01 control mode invalid code 0x%x\n", data);
						break;
				}
			}
			else
			{
				logerror("SC-01 receiving phonemes 0x%x\n", data & 0x3f);
			}
			break;

		case 0x01:
			logerror("SC-01 inflection changed to 0x%x\n", data);
			break;

		case 0x02:
			logerror("SC-01 speech rate changed to 0x%x\n", data);
			break;

		case 0x03:
			if (data & 0x80)
			{
				m_ssi_control = 1;
				logerror("SC-01 now in control mode.\n");
			}
			else if (data & 0x70)
			{
				m_ssi_control = 0;
				logerror("SC-01 control mode cleared.\n");
			}
			else
			{
				logerror("SC-01 amplitude manipulated 0x%x.\n", data);
			}
			break;

		case 0x04:
			logerror("SC-01 filter frequency changed to 0x%x.\n", data);
			break;
	}
}


/* COP R/W */
/* All 8 l pins are hooked up here */
static READ8_HANDLER(cop_l_read)
{
	logerror("COP - Reading from L @ (0x%x)\n", activecpu_get_pc());
	return cop_l_latch;
}

/* Pins g0, g1, g2, and g3 are all hooked up */
static READ8_HANDLER(cop_g_read)
{
	logerror("COP - Reading from G @ (0x%x)\n", activecpu_get_pc());
	return cop_g_latch;
}

static WRITE8_HANDLER(cop_l_write)
{
	logerror("COP - WRITING TO L 0x%x @ (0x%x)\n", data, activecpu_get_pc());
	cop_l_latch = data;
}

static WRITE8_HANDLER(cop_g_write)
{
	logerror("COP - WRITING TO G 0x%x @ (0x%x)\n", data, activecpu_get_pc());

	/* Bit 3 goes to an enable pin on the data latch? */
	/* Bits 0-2 seem to go into the wrong side of a logic IC? */
}

/* Schematics say pins d0 and d1 are the only two that are hooked up */
static WRITE8_HANDLER(cop_d_write)
{
	int irq_flag = 0;

	logerror("COP - WRITING TO D 0x%x @ (0x%x)\n", data, activecpu_get_pc());

	/* Pin d0 - TIMER INT */
	if (!(data & 0x01))
	{
		SET_Z80_TIMER_INT();
		irq_flag = 1;
		logerror("THE COP IS TRIPPING THE TIMER INTERRUPT\n");
	}
	else
	{
		CLEAR_Z80_TIMER_INT();
	}

	/* Pin d1 - DATA READY INT */
	if (!(data & 0x02))
	{
		SET_Z80_DATA_RDY_INT();
		irq_flag = 1;
		logerror("THE COP IS TRIPPING THE DATA READY INTERRUPT\n");
	}
	else
	{
		CLEAR_Z80_DATA_RDY_INT();
	}

	/* Assert the Z80's interrupt line if necessary */
	if (irq_flag)
	{
		cpunum_set_input_line(0, 0, ASSERT_LINE);
	}
}

static WRITE8_HANDLER(cop_sk_write)
{
	/* I think this data falls off into a black hole - the pins aren't hooked up.
       The COP400 XAS instruction writes to both SK and SIO, that's why the CPU writes data here at all. */
}

static WRITE8_HANDLER(cop_sio_write)
{
	/* This data is sent to the CLK pin of the keyboard handling logic.  We don't need to emulate it here. */
}



/* PROGRAM MAPS */
static ADDRESS_MAP_START( mainmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(ram_ic_read,ram_ic_write) AM_BASE(&ram_ic)	/* Not quite sure how big the RAM IC is. */
	AM_RANGE(0xc000, 0xdfff) AM_ROM														/* This is what Daphne says - we'll soon see. */
ADDRESS_MAP_END

static ADDRESS_MAP_START( copmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x3ff) AM_ROM						/* The schematics seem to claim this is a 2716 (0x800 bytes) - is the dump too small? */
ADDRESS_MAP_END


/* IO MAPS */
static ADDRESS_MAP_START( mainio, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00,0x04) AM_WRITE(sc01_register_w)
	AM_RANGE(0x20,0x20) AM_WRITE(write_cop)
	AM_RANGE(0x40,0x40) AM_READ(z80_irq_status_r)
/*  AM_RANGE(0x80,0x80) AM_WRITE(data_write_cop) */
	AM_RANGE(0xa0,0xa0) AM_WRITE(timer_int_w)
	AM_RANGE(0xc0,0xc0) AM_WRITE(data_ready_int_w)

/*  AM_RANGE(0xf0,0xf0) */						/* Laserdisc data read */
	AM_RANGE(0xf1,0xf1)	AM_READ_PORT("DSWB")	/* 4 dips, 2 coins, Laserdisc 'ready' and 'enter' pins */
	AM_RANGE(0xf2,0xf2)	AM_READ_PORT("DSWA")	/* 8 dips */
	AM_RANGE(0xf3,0xf3)	AM_WRITE(intrq_w)		/* goes to /INTRQ */
/*  AM_RANGE(0xf4,0xf4) */						/* Laserdisc data write */
/*  AM_RANGE(0xf5,0xf5) */						/* Coin counter, 0xf4 write enable, DIP-controlled (SWB-6) LDV1000 'enter' line, and LDV1000 INT/!EXT line */
/*  AM_RANGE(0xf6,0xf6) */						/* Scoreboard DEN1 */
/*  AM_RANGE(0xf7,0xf7) */						/* Scoreboard DEN2 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( copio, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(COP400_PORT_L,  COP400_PORT_L)   AM_READWRITE(cop_l_read,cop_l_write)
	AM_RANGE(COP400_PORT_G,  COP400_PORT_G)   AM_READWRITE(cop_g_read,cop_g_write)
	AM_RANGE(COP400_PORT_D,  COP400_PORT_D)   AM_WRITE(cop_d_write)
	AM_RANGE(COP400_PORT_SK, COP400_PORT_SK)  AM_WRITE(cop_sk_write)
	AM_RANGE(COP400_PORT_SIO,COP400_PORT_SIO) AM_READ_PORT("THAYERS_LETTERS") AM_WRITE(cop_sio_write)	/* Unemulated in COP40x core, so nothing happens ATM */
ADDRESS_MAP_END																										/* This is also very wrong.  The keyboard interface needs to
                                                                                                                       be understood much better than what I have here */

/* PORTS */
static INPUT_PORTS_START( thayers )
	PORT_START_TAG("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Time Per Coin" ) PORT_DIPLOCATION( "A:3,2,1" )
	PORT_DIPSETTING(    0x07, "110 Seconds" )
	PORT_DIPSETTING(    0x06, "95 Seconds" )
	PORT_DIPSETTING(    0x05, "80 Seconds" )
	PORT_DIPSETTING(    0x04, "70 Seconds" )
	PORT_DIPSETTING(    0x03, "60 Seconds" )
	PORT_DIPSETTING(    0x02, "45 Seconds" )
	PORT_DIPSETTING(    0x01, "30 Seconds" )
	PORT_DIPSETTING(    0x00, DEF_STR ( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) ) PORT_DIPLOCATION( "A:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION( "A:5" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "A:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Attract Mode Audio" ) PORT_DIPLOCATION( "A:7" )
	PORT_DIPSETTING(    0x40, "Always Playing" )
	PORT_DIPSETTING(    0x00, "One Out of 8 Times" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "A:8" )

	PORT_START_TAG("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Gameplay Mode" ) PORT_DIPLOCATION( "B:1" )
	PORT_DIPSETTING(    0x01, "Normal Play" )
	PORT_DIPSETTING(    0x00, "Self-Diagnostics Mode" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "B:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "B:3" )
	PORT_DIPNAME( 0x08, 0x08, "LD Player" ) PORT_DIPLOCATION( "B:4" )
	PORT_DIPSETTING(    0x08, "LDV-1000" )
	PORT_DIPSETTING(    0x00, "PR-7820" )
	/* DIP SWITCH 5 isn't read here, but it also controls LD player selection - I wonder how to add it properly? */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_command_r, 0 )	/* Enter pin on LD player */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_status_r, 0 )	/* Ready pin on LD player */

	/* The following 3 port definitions can be combined into one (thus making this scheme work) if the port bit limit is raised from 32 to 64 */
	/* But maybe there's a better way to do it than what I have here? */
	/* Or maybe this doesn't even work at all :) */
	/* "Scan codes" heisted from Daphne - need to be tested! */
	PORT_START_TAG("THAYERS_ACTIONS")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( DEF_STR( No ) ) PORT_CODE( KEYCODE_DEL_PAD )
	PORT_BIT( 0x31, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( DEF_STR( Yes ) ) PORT_CODE( KEYCODE_0_PAD )
	PORT_BIT( 0x32, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Items" ) PORT_CODE( KEYCODE_1_PAD )
	PORT_BIT( 0x33, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Drop Item" ) PORT_CODE( KEYCODE_2_PAD )
	PORT_BIT( 0x34, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Give Score" ) PORT_CODE( KEYCODE_3_PAD )
	PORT_BIT( 0x35, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Replay" ) PORT_CODE( KEYCODE_4_PAD )
	PORT_BIT( 0x36, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Combine Action" ) PORT_CODE( KEYCODE_6_PAD )
	PORT_BIT( 0x37, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Save Game" ) PORT_CODE( KEYCODE_7_PAD )
	PORT_BIT( 0x38, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Update" ) PORT_CODE( KEYCODE_8_PAD )
	PORT_BIT( 0x39, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Hint" ) PORT_CODE( KEYCODE_9_PAD )

	PORT_START_TAG("THAYERS_LETTERS")
	PORT_BIT( 0x41, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "A" ) PORT_CODE( KEYCODE_A )
	PORT_BIT( 0x42, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "B - Silver Wheat" ) PORT_CODE( KEYCODE_B )
	PORT_BIT( 0x43, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "C - Spell of Seeing" ) PORT_CODE( KEYCODE_C )
	PORT_BIT( 0x44, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "D - Great Circlet" ) PORT_CODE( KEYCODE_D )
	PORT_BIT( 0x45, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "E - Black Mace" ) PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x46, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "F - Hunting Horn" ) PORT_CODE( KEYCODE_F )
	PORT_BIT( 0x47, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "G - Long Bow" ) PORT_CODE( KEYCODE_G )
	PORT_BIT( 0x48, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "H - Medallion" ) PORT_CODE( KEYCODE_H )
	PORT_BIT( 0x49, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "I - Crown" ) PORT_CODE( KEYCODE_I )
	PORT_BIT( 0x4a, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "J - Onyx Seal" ) PORT_CODE( KEYCODE_J )
	PORT_BIT( 0x4b, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "K - Orb of Quoid" ) PORT_CODE( KEYCODE_K )
	PORT_BIT( 0x4c, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "L" ) PORT_CODE( KEYCODE_L )
	PORT_BIT( 0x4d, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "M - Spell of Understanding" ) PORT_CODE( KEYCODE_M )
	PORT_BIT( 0x4e, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "N - Staff" ) PORT_CODE( KEYCODE_N )
	PORT_BIT( 0x4f, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "O - Crystal" ) PORT_CODE( KEYCODE_O )
	PORT_BIT( 0x50, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "P" ) PORT_CODE( KEYCODE_P )
	PORT_BIT( 0x51, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Q" ) PORT_CODE( KEYCODE_Q )
	PORT_BIT( 0x52, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "R - Blood Sword" ) PORT_CODE( KEYCODE_R )
	PORT_BIT( 0x53, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "S - Dagger" ) PORT_CODE( KEYCODE_S )
	PORT_BIT( 0x54, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "T - Chalice" ) PORT_CODE( KEYCODE_T )
	PORT_BIT( 0x55, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "U - Cold Fire" ) PORT_CODE( KEYCODE_U )
	PORT_BIT( 0x56, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "V - Shield" ) PORT_CODE( KEYCODE_V )
	PORT_BIT( 0x57, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "W - Amulet" ) PORT_CODE( KEYCODE_W )
	PORT_BIT( 0x58, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "X - Scepter" ) PORT_CODE( KEYCODE_X )
	PORT_BIT( 0x59, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Y - Coins" ) PORT_CODE( KEYCODE_Y )
	PORT_BIT( 0x5a, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Z - Spell of Release" ) PORT_CODE( KEYCODE_Z )

	PORT_START_TAG("THAYERS_NUMBERS")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "1 - Clear" ) PORT_CODE( KEYCODE_LCONTROL )
	PORT_BIT( 0x81, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "2" ) PORT_CODE( KEYCODE_RCONTROL )
	PORT_BIT( 0x82, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "3 - Enter" ) PORT_CODE( KEYCODE_ENTER )
	PORT_BIT( 0x83, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "4 - Space" ) PORT_CODE( KEYCODE_SPACE )
INPUT_PORTS_END


static MACHINE_START( thayers )
{
	discinfo = laserdisc_init(LASERDISC_TYPE_LDV1000, get_disk_handle(0), 0);
	return;
}

static INTERRUPT_GEN( vblank_callback_thayers )
{
	laserdisc_vsync(discinfo);
}


/* DRIVER */
static MACHINE_DRIVER_START( thayers )
/*  main cpu */
	MDRV_CPU_ADD(Z80, SCHEMATIC_CLOCK)
	MDRV_CPU_PROGRAM_MAP(mainmem,0)
	MDRV_CPU_IO_MAP(mainio,0)
	MDRV_CPU_VBLANK_INT(vblank_callback_thayers, 1)

	MDRV_MACHINE_START(thayers)

/*  io device cpu */
	MDRV_CPU_ADD(COP420, SCHEMATIC_CLOCK/2/8)		/* Can't read the schematics, but this is what daphne says */
	MDRV_CPU_PROGRAM_MAP(copmem,0)
	MDRV_CPU_IO_MAP(copio,0)

/*  video */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)

	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(32*10, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*10, 32*10-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(256)
	MDRV_VIDEO_UPDATE(thayers)

/*  sound */
MACHINE_DRIVER_END


ROM_START( thayers )
	ROM_REGION( 0xe000, REGION_CPU1, 0 )
	ROM_LOAD( "tq_u33.bin", 0x0000, 0x8000, CRC(82df5d89) SHA1(58dfd62bf8c5a55d1eba397d2c284e99a4685a3f) )
	ROM_LOAD( "tq_u1.bin",  0xc000, 0x2000, CRC(e8e7f566) SHA1(df7b83ef465c65446c8418bc6007447693b75021) )

	ROM_REGION( 0x400, REGION_CPU2, 0 )
	ROM_LOAD( "tq_cop.bin", 0x000, 0x400, CRC(6748e6b3) SHA1(5d7d1ecb57c1501ef6a2d9691eecc9970586606b) )
ROM_END

ROM_START( thayersa )
	ROM_REGION( 0xe000, REGION_CPU1, 0 )
	ROM_LOAD( "tq_u33.bin", 0x0000, 0x8000, CRC(82df5d89) SHA1(58dfd62bf8c5a55d1eba397d2c284e99a4685a3f) )
	ROM_LOAD( "tq_u1.bin",  0xc000, 0x2000, CRC(33817e25) SHA1(f9750da863dd57fe2f5b6e8fce9c6695dc5c9adc) )

	ROM_REGION( 0x400, REGION_CPU2, 0 )
	ROM_LOAD( "tq_cop.bin", 0x000, 0x400, CRC(6748e6b3) SHA1(5d7d1ecb57c1501ef6a2d9691eecc9970586606b) )
ROM_END

static DRIVER_INIT( thayers )
{
	Z80_IRQ_STATUS_CLEAR();
	m_ssi_control = 0;

	cop_g_latch = 0xff;
}

/*    YEAR  NAME      PARENT   MACHINE  INPUT    INIT     MONITOR  COMPANY               FULLNAME                           FLAGS) */
GAME( 1983, thayers,  0,       thayers, thayers, thayers, ROT0,    "RDI Video Systems",  "Thayer's Quest",                  GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1983, thayersa, thayers, thayers, thayers, thayers, ROT0,    "RDI Video Systems",  "Thayer's Quest (Alternate Set)",  GAME_NOT_WORKING|GAME_NO_SOUND)
