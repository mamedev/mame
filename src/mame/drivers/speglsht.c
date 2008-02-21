/*
Super Eagle Shot
(c)1994 Seta (distributed by Visco)
-----------------------------------
driver by Tomasz Slanina


GOLF
E30-001A

CPU           : Integrated Device IDT79R3051-25J 9407C (R3000A)
Sound+Objects : ST-0016
OSC           : 50.0000MHz (X1) 42.9545MHz (X3)

ROMs:
SX004-01.PR0 - R3051 Main programs (MX27C4000)
SX004-02.PR1 |
SX004-03.PR2 |
SX004-04.PR3 /

SX004-05.RD0 - Data and Graphics (D23C8000SCZ)
SX004-06.RD1 /

SX004-07.ZPR - ST-0016 Program and Data (16M mask, read as 27c160)

GALs (not dumped):
SX004-08.27 (16V8B)
SX004-09.46 (16V8B)
SX004-10.59 (16V8B)
SX004-11.61 (22V10B)
SX004-12.62 (22V10B)
SX004-13.63 (22V10B)

Custom chips:
SETA ST-0015 60EN502F12 JAPAN 9415YAI (U18, 208pin PQFP, system controller)
SETA ST-0016 TC6187AF JAPAN 9348YAA (U68, 208pin PQFP, sound & object)

 R3051    ST-0015              SX004-01   49.9545MHz    ST-0016       5588-25
                               SX004-02      52B256-70  514256 514256
 50MHz                         SX004-03      52B256-70  SX004-07
 528257-70 514256-70 514256-70 SX004-04
 528257-70 514256-70 514256-70 SX004-05
 528257-70 514256-70 514256-70 SX004-06
           514256-70 514256-70
                                                NEC D6376


*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/mips/r3000.h"
#include "sound/st0016.h"
#include "st0016.h"

READ8_HANDLER(st0016_dma_r);
static UINT8 *shared;

UINT32 *speglsht_framebuffer;
UINT32  speglsht_videoreg;

extern UINT8 *st0016_charram;

static ADDRESS_MAP_START( st0016_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xcfff) AM_READ(st0016_sprite_ram_r) AM_WRITE(st0016_sprite_ram_w)
	AM_RANGE(0xd000, 0xdfff) AM_READ(st0016_sprite2_ram_r) AM_WRITE(st0016_sprite2_ram_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xe87f) AM_RAM
	AM_RANGE(0xe900, 0xe9ff) AM_RAM AM_WRITE(st0016_snd_w) AM_BASE(&st0016_sound_regs)
	AM_RANGE(0xea00, 0xebff) AM_READ(st0016_palette_ram_r) AM_WRITE(st0016_palette_ram_w)
	AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_BASE(&shared)
ADDRESS_MAP_END

static ADDRESS_MAP_START( st0016_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w)
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xe2, 0xe2) AM_WRITE(st0016_sprite_bank_w)
	AM_RANGE(0xe3, 0xe4) AM_WRITE(st0016_character_bank_w)
	AM_RANGE(0xe5, 0xe5) AM_WRITE(st0016_palette_bank_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITENOP
	AM_RANGE(0xe7, 0xe7) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END

static READ32_HANDLER(shared_r)
{
	return shared[offset];
}

static WRITE32_HANDLER(shared_w)
{
	shared[offset]=data&0xff;
}

static WRITE32_HANDLER(videoreg_w)
{
	COMBINE_DATA(&speglsht_videoreg);
}

static UINT32 *cop_ram;

static WRITE32_HANDLER(cop_w)
{
	COMBINE_DATA(&cop_ram[offset]);

	if(cop_ram[offset]&0x8000) //fix (sign)
	{
		cop_ram[offset]|=0xffff0000;
	}
}

//matrix * vector
static READ32_HANDLER(cop_r)
{
	INT32 *cop=(INT32*)&cop_ram[0];

	union
	{
		INT32  a;
		UINT32 b;
	}temp;

	switch (offset)
	{
		case 0x40/4:
		{
			temp.a=((cop[0x3]*cop[0x0]+cop[0x4]*cop[0x1]+cop[0x5]*cop[0x2])>>14)+cop[0xc];
			return temp.b;
		}

		case 0x44/4:
		{
			temp.a=((cop[0x6]*cop[0x0]+cop[0x7]*cop[0x1]+cop[0x8]*cop[0x2])>>14)+cop[0xd];
			return temp.b;
		}

		case 0x48/4:
		{
			temp.a=((cop[0x9]*cop[0x0]+cop[0xa]*cop[0x1]+cop[0xb]*cop[0x2])>>14)+cop[0xe];
			return temp.b;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( speglsht_mem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAM
	AM_RANGE(0x01000000, 0x01007fff) AM_RAM //tested - STATIC RAM
	AM_RANGE(0x01600000, 0x0160004f) AM_READWRITE(cop_r, cop_w) AM_BASE(&cop_ram)
	AM_RANGE(0x01800200, 0x01800203) AM_WRITE(videoreg_w)
	AM_RANGE(0x01800300, 0x01800303) AM_READ(input_port_0_dword_r)
	AM_RANGE(0x01800400, 0x01800403) AM_READ(input_port_1_dword_r)
	AM_RANGE(0x01a00000, 0x01afffff) AM_RAM AM_BASE(&speglsht_framebuffer)
	AM_RANGE(0x01b00000, 0x01b07fff) AM_RAM //cleared ...  video related ?
	AM_RANGE(0x01c00000, 0x01dfffff) AM_READ(MRA32_ROM) AM_WRITE(MWA32_ROM) AM_REGION(REGION_USER2, 0)
	AM_RANGE(0x0a000000, 0x0a003fff) AM_READWRITE(shared_r, shared_w)
	AM_RANGE(0x1eff0000, 0x1eff001f) AM_RAM
	AM_RANGE(0x1eff003c, 0x1eff003f) AM_READNOP //interrupt related
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION(REGION_USER1, 0)
	AM_RANGE(0x2fc00000, 0x2fdfffff) AM_ROM AM_REGION(REGION_USER1, 0) // mirror for interrupts
ADDRESS_MAP_END

static INPUT_PORTS_START( speglsht )

PORT_START
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
  PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

PORT_START
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0003, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0001, "1C/1C or 2C/3C" ) /* 1 coin/1 credit or 2 coins/3 credits */
	PORT_DIPSETTING(    0x0002, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0000, "2C Start/1C Continue" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0018, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0008, "1C/1C or 2C/3C" ) /* 1 coin/1 credit or 2 coins/3 credits */
	PORT_DIPSETTING(    0x0010, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0000, "2C Start/1C Continue" )
	PORT_DIPNAME( 0x0040, 0x0040, "Unkown / Unused" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Bonus for PAR Play" )
	PORT_DIPSETTING(    0x0080, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0000, "Extra Hole" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Number of Players" )
	PORT_DIPSETTING(    0x0c00, "3" )
	PORT_DIPSETTING(    0x0800, "4" )
	PORT_DIPSETTING(    0x0400, "2" )
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Control Panel" )
	PORT_DIPSETTING(    0x2000, "Double" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Single ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Country" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( USA ) )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00010000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80A00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
  PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
INPUT_PORTS_END

static GFXDECODE_START( speglsht )
GFXDECODE_END

static const struct ST0016interface st0016_interface =
{
	&st0016_charram
};

 static INTERRUPT_GEN( irq4_gen )
{
	cpunum_set_input_line(machine, 1, R3000_IRQ4, ASSERT_LINE);
}

static const struct r3000_config config =
{
	0,
	4096,	/* code cache size */
	2048	/* data cache size */
};

static MACHINE_RESET(speglsht)
{
	memset(shared,0,0x1000);
}

static MACHINE_DRIVER_START( speglsht )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main",Z80, 8000000) /* 8 MHz ? */
	MDRV_CPU_PROGRAM_MAP(st0016_mem,0)
	MDRV_CPU_IO_MAP(st0016_io,0)

	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(R3000LE, 25000000)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(speglsht_mem,0)
	MDRV_CPU_VBLANK_INT(irq4_gen,1)

	MDRV_INTERLEAVE(100)
	MDRV_MACHINE_RESET(speglsht)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 8, 239-8)

	MDRV_GFXDECODE(speglsht)
	MDRV_PALETTE_LENGTH(16*16*4+1)

	MDRV_VIDEO_START(st0016)
	MDRV_VIDEO_UPDATE(st0016)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(ST0016, 0)
	MDRV_SOUND_CONFIG(st0016_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

ROM_START( speglsht )
	ROM_REGION( 0x210000, REGION_CPU1, 0 )
	ROM_LOAD( "sx004-07.u70", 0x010000, 0x200000, CRC(2d759cc4) SHA1(9fedd829190b2aab850b2f1088caaec91e8715dd) ) /* Noted as "ZPRO0" IE: Z80 (ST0016) Program 0 */
	/* U71 unpopulated, Noted as ZPRO1 */
	ROM_COPY( REGION_CPU1,  0x10000, 0x00000, 0x08000 )

	ROM_REGION32_BE( 0x200000, REGION_USER1, 0 )
	ROM_LOAD32_BYTE( "sx004-04.u33", 0x00000, 0x80000, CRC(e46d2e57) SHA1(b1fb836ab2ce547dc2e8d1046d7ef835b87bb04e) ) /* Noted as "RPRO3" IE: R3000 Program 3 */
	ROM_LOAD32_BYTE( "sx004-03.u32", 0x00001, 0x80000, CRC(c6ffb00e) SHA1(f57ef45bb5c690c3e63101a36835d2687abfcdbd) ) /* Noted as "RPRO2" */
	ROM_LOAD32_BYTE( "sx004-02.u31", 0x00002, 0x80000, CRC(21eb46e4) SHA1(0ab21ed012c9a76e01c83b60c6f4670836dfa718) ) /* Noted as "RPRO1" */
	ROM_LOAD32_BYTE( "sx004-01.u30", 0x00003, 0x80000, CRC(65646949) SHA1(74931c230f4e4b1008fbc5fba169292e216aa23b) ) /* Noted as "RPRO0" */

	ROM_REGION( 0x200000, REGION_USER2,0)
	ROM_LOAD32_WORD( "sx004-05.u34", 0x000000, 0x100000, CRC(f3c69468) SHA1(81daef6d0596cb67bb6f87b39874aae1b1ffe6a6) ) /* Noted as "RD0" IE: R3000 Data 0 */
	ROM_LOAD32_WORD( "sx004-06.u35", 0x000002, 0x100000, CRC(5af78e44) SHA1(0131d50348fef80c2b100d74b7c967c6a710d548) ) /* Noted as "RD1" */

ROM_END



static DRIVER_INIT(speglsht)
{
	st0016_game=3;
}


GAME( 1994, speglsht, 0, speglsht, speglsht, speglsht, ROT0, "Seta",  "Super Eagle Shot", GAME_IMPERFECT_GRAPHICS )
