/*
    Gals Panic 3
    (c) Kaneko 1995

    Skeleton driver by Haze
    WIP Driver by Sebastien Volpe

Check done by main code, as part of EEPROM data:
'Gals Panic 3 v0.96 95/08/29(Tue)'


TODO:
- find a working board to dump MCU provided code & data, as code is
  involved just after initial checks; the game currently goes nowhere.
- finish emulation, mainly backgounds

What's been done lately:
- palette, inputs, sound, backgounds 'decoded' (RLE)
*/

/*

Gals Panic 3 (JPN Ver.)
(c)1995 Kaneko

CPU:    68000-16
Sound:  YMZ280B-F
OSC:    28.6363MHz
        33.3333MHz
EEPROM: 93C46
Chips.: GRAP2 x3                <- R/G/B Chips?
        APRIO-GL
        BABY004
        GCNT2
        TBSOP01                 <- ToyBox NEC uPD78324 series MCU with 32K internal rom
        CG24173 6186            <- Sprites, see suprnova.c
        CG24143 4181            <- ^


G3P0J1.71     prg.
G3P1J1.102

GP340000.123  chr.
GP340100.122
GP340200.121
GP340300.120
G3G0J0.101
G3G1J0.100

G3D0X0.134

GP320000.1    OBJ chr.

GP310000.41   sound data
GP310100.40


--- Team Japump!!! ---
http://www.rainemu.com/japump/
http://japump.i.am/
Dumped by Uki
10/22/2000

*/

#include "driver.h"
#include "deprecat.h"
#include "sound/ymz280b.h"
#include "includes/suprnova.h"
#include "kaneko16.h"

/***************************************************************************

 video

***************************************************************************/

static UINT16 *galpani3_sprregs, *galpani3_spriteram;
static UINT32* galpani3_spriteram32, *galpani3_spc_regs;
static bitmap_t *sprite_bitmap_1;

static INTERRUPT_GEN( galpani3_vblank ) // 2, 3, 5 ?
{
	switch ( cpu_getiloops(device) )
	{
		case 2:  cpu_set_input_line(device, 2, HOLD_LINE); break;
		case 1:  cpu_set_input_line(device, 3, HOLD_LINE); break;
		case 0:  cpu_set_input_line(device, 5, HOLD_LINE); break;
	}
}


extern int suprnova_alt_enable_sprites;

static VIDEO_START(galpani3)
{
	/* so we can use suprnova.c */
	galpani3_spriteram32 = auto_malloc ( 0x4000 );
	spriteram_size = 0x4000;
	galpani3_spc_regs = auto_malloc (0x40);
	suprnova_alt_enable_sprites = 1;


	sprite_bitmap_1 = auto_bitmap_alloc(1024,1024,BITMAP_FORMAT_INDEXED16);

}

static VIDEO_UPDATE(galpani3)
{
	int x,y;
	UINT16* src1;
	UINT16* dst;
	UINT16 pixdata1;

	fillbitmap(bitmap, 0x0000, cliprect);

	//skns_draw_sprites(screen->machine,bitmap,cliprect);
	fillbitmap(sprite_bitmap_1, 0x0000, cliprect);

	skns_draw_sprites(screen->machine, sprite_bitmap_1, cliprect, galpani3_spriteram32, spriteram_size, memory_region(screen->machine,"gfx1"), memory_region_length (screen->machine, "gfx1"), galpani3_spc_regs );

	// ignoring priority bits for now..
	for (y=0;y<240;y++)
	{
		src1 = BITMAP_ADDR16(sprite_bitmap_1, y, 0);
		dst =  BITMAP_ADDR16(bitmap, y, 0);

		for (x=0;x<320;x++)
		{
			pixdata1 = src1[x];

			if (pixdata1 & 0x3fff)
			{
				dst[x] = (pixdata1 & 0x3fff);
			}
		}
	}

	return 0;
}


static INPUT_PORTS_START( galpani3 )
	PORT_START("P1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1  ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2  ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED	)

	PORT_START("DSW")	/* provided by the MCU - $200386.b <- $400200 */
	PORT_DIPNAME( 0x0100, 0x0100, "Test Mode" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) // ?
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )	// unused ?
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static WRITE16_HANDLER( galpani3_suprnova_sprite32_w )
{
	COMBINE_DATA(&galpani3_spriteram[offset]);
	offset>>=1;
	galpani3_spriteram32[offset]=(galpani3_spriteram[offset*2+1]<<16) | (galpani3_spriteram[offset*2]);
}

static WRITE16_HANDLER( galpani3_suprnova_sprite32regs_w )
{
	COMBINE_DATA(&galpani3_sprregs[offset]);
	offset>>=1;
	galpani3_spc_regs[offset]=(galpani3_sprregs[offset*2+1]<<16) | (galpani3_sprregs[offset*2]);
}



/***************************************************************************

                            MCU Code Simulation
                (follows the implementation of kaneko16.c)

***************************************************************************/
static UINT16 *mcu_ram, galpani3_mcu_com[4];

static void galpani3_mcu_run(running_machine *machine)
{
	UINT16 mcu_command = mcu_ram[0x0010/2];		/* command nb */
	UINT16 mcu_offset  = mcu_ram[0x0012/2] / 2;	/* offset in shared RAM where MCU will write */
	UINT16 mcu_subcmd  = mcu_ram[0x0014/2];		/* sub-command parameter, happens only for command #4 */

	logerror("(PC=%06X): MCU executed command : %04X %04X\n",cpu_get_pc(machine->activecpu),mcu_command,mcu_offset*2);

	/* the only MCU commands found in program code are:
         0x04: protection: provide code/data, that's exactly where we are stuck !!!
         0x03: read DSW
         0x02: load NVRAM settings \ ATMEL AT93C46 chip,
         0x42: save NVRAM settings / 128 bytes serial EEPROM
    */
	switch (mcu_command >> 8)
	{
		case 0x03:	// DSW
		{
			mcu_ram[mcu_offset] = input_port_read(machine, "DSW");
			logerror("PC=%06X : MCU executed command: %04X %04X (read DSW)\n", cpu_get_pc(machine->activecpu), mcu_command, mcu_offset*2);
		}
		break;

		case 0x02: // $38950 - load NVRAM settings
		{
			/* NOTE: code @ $38B46 & $38ab8 does exactly what is checked after MCU command
                     so that's what we'll mimic here... probably the initial NVRAM settings */
			int i;

			/* MCU writes 128 bytes to shared ram: last byte is the byte-sum */
			/* first 32 bytes (header): 0x8BE08E71.L, then the string "95/06/30 Gals Panic3Ver 0.95"; */
			mcu_ram[mcu_offset +  0] = 0x8BE0; mcu_ram[mcu_offset +  1] = 0x8E71;
			mcu_ram[mcu_offset +  2] = 0x3935; mcu_ram[mcu_offset +  3] = 0x2F30;
			mcu_ram[mcu_offset +  4] = 0x362F; mcu_ram[mcu_offset +  5] = 0x3330;
			mcu_ram[mcu_offset +  6] = 0x2047; mcu_ram[mcu_offset +  7] = 0x616C;
			mcu_ram[mcu_offset +  8] = 0x7320; mcu_ram[mcu_offset +  9] = 0x5061;
			mcu_ram[mcu_offset + 10] = 0x6E69; mcu_ram[mcu_offset + 11] = 0x6333;
			mcu_ram[mcu_offset + 12] = 0x5665; mcu_ram[mcu_offset + 13] = 0x7220;
			mcu_ram[mcu_offset + 14] = 0x302E; mcu_ram[mcu_offset + 15] = 0x3935;
			/* next 11 bytes - initial NVRAM settings */
			mcu_ram[mcu_offset + 16] = 0x0001; mcu_ram[mcu_offset + 17] = 0x0101;
			mcu_ram[mcu_offset + 18] = 0x0100; mcu_ram[mcu_offset + 19] = 0x0208;
			mcu_ram[mcu_offset + 20] = 0x02FF; mcu_ram[mcu_offset + 21] = 0x0000;
			/* rest is zeroes */
			for (i=22;i<63;i++)
				mcu_ram[mcu_offset + i] = 0;
			/* and sum is $0c.b */
			mcu_ram[mcu_offset + 63] = 0x000c;
		}
		break;

		case 0x04: // $38842 - provides code/data
		{
			toxboy_handle_04_subcommand(machine, mcu_subcmd, mcu_ram);
		}
		break;

		case 0x42: // $389ee - save NVRAM settings
		{
			// found, TODO: trace call in code !!!
		}
		break;

		default:
			logerror("UNKNOWN COMMAND\n");
	}
}

/*
  MCU doesn't execute exactly as it is coded right know (ala jchan):
   * com0=com1=0xFFFF -> command to execute
   * com2=com3=0xFFFF -> status reading only
*/
#define GALPANI3_MCU_COM_W(_n_) \
static WRITE16_HANDLER( galpani3_mcu_com##_n_##_w ) \
{ \
	COMBINE_DATA(&galpani3_mcu_com[_n_]); \
	if (galpani3_mcu_com[0] != 0xFFFF)	return; \
	if (galpani3_mcu_com[1] != 0xFFFF)	return; \
	if (galpani3_mcu_com[2] != 0xFFFF)	return; \
	if (galpani3_mcu_com[3] != 0xFFFF)	return; \
\
	memset(galpani3_mcu_com, 0, 4 * sizeof( UINT16 ) ); \
	galpani3_mcu_run(machine); \
}

GALPANI3_MCU_COM_W(0)
GALPANI3_MCU_COM_W(1)
GALPANI3_MCU_COM_W(2)
GALPANI3_MCU_COM_W(3)

static READ16_HANDLER( galpani3_mcu_status_r )
{
	logerror("cpu #%d (PC=%06X): read mcu status\n", cpunum_get_active(), cpu_get_previouspc(machine->activecpu));
	return 0;
}

// might be blitter regs? - there are 3, probably GRAP2 chips
READ16_HANDLER( galpani3_regs1_r )
{

	logerror("cpu #%d (PC=%06X): galpani3_regs1_r %02x %04x\n", cpunum_get_active(), cpu_get_previouspc(machine->activecpu), offset, mem_mask);
	switch (offset)
	{
		case 0xb:
		{
			static int i = 0;
			i^=1;
			if (i) return 0xfffe;
			else return 0xffff;
		}

	}

	return 0x0000;

}

READ16_HANDLER( galpani3_regs2_r )
{
	logerror("cpu #%d (PC=%06X): galpani3_regs2_r %02x %04x\n", cpunum_get_active(), cpu_get_previouspc(machine->activecpu), offset, mem_mask);
	switch (offset)
	{
		case 0xb:
		{
			static int i = 0;
			i^=1;
			if (i) return 0xfffe;
			else return 0xffff;
		}

	}

	return 0x0000;}

READ16_HANDLER( galpani3_regs3_r )
{
	logerror("cpu #%d (PC=%06X): galpani3_regs3_r %02x %04x\n", cpunum_get_active(), cpu_get_previouspc(machine->activecpu), offset, mem_mask);
	switch (offset)
	{
		case 0xb:
		{
			static int i = 0;
			i^=1;
			if (i) return 0xfffe;
			else return 0xffff;
		}

	}

	return 0x0000;
}



static ADDRESS_MAP_START( galpani3_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM

	AM_RANGE(0x200000, 0x20ffff) AM_RAM // area [B] - Work RAM
	AM_RANGE(0x280000, 0x287fff) AM_RAM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE(&paletteram16) // area [A] - palette for sprites

	AM_RANGE(0x300000, 0x303fff) AM_RAM_WRITE(galpani3_suprnova_sprite32_w) AM_BASE(&galpani3_spriteram)
	AM_RANGE(0x380000, 0x38003f) AM_RAM_WRITE(galpani3_suprnova_sprite32regs_w) AM_BASE(&galpani3_sprregs)

	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_BASE(&mcu_ram) // area [C]

	AM_RANGE(0x580000, 0x580001) AM_WRITE(galpani3_mcu_com0_w)	// ] see $387e8: these 2 locations are written (w.#$ffff)
	AM_RANGE(0x600000, 0x600001) AM_WRITE(galpani3_mcu_com1_w)	// ] then bit #0 of $780000.l is tested: 0 = OK!
	AM_RANGE(0x680000, 0x680001) AM_WRITE(galpani3_mcu_com2_w)	// ] see $387e8: these 2 locations are written (w.#$ffff)
	AM_RANGE(0x700000, 0x700001) AM_WRITE(galpani3_mcu_com3_w)	// ] then bit #0 of $780000.l is tested: 0 = OK!
	AM_RANGE(0x780000, 0x780001) AM_READ(galpani3_mcu_status_r)


	// GRAP2 1?
	AM_RANGE(0x800000, 0x8003ff) AM_RAM // ??? see subroutine $39f42 (R?)
	AM_RANGE(0x800800, 0x800bff) AM_RAM // ??? see subroutine $39f42 (R?)
	AM_RANGE(0x800c00, 0x800c1f) AM_READ(galpani3_regs1_r)// ? R layer regs ? see subroutine $3a03e
	AM_RANGE(0x880000, 0x8801ff) AM_RAM // area [G] - R area ? linescroll ?
	AM_RANGE(0x900000, 0x97ffff) AM_RAM // area [D] - R area ? odd bytes only, initialized 00..ff,00..ff,...

	// GRAP2 2?
	AM_RANGE(0xa00000, 0xa003ff) AM_RAM // ??? see subroutine $39f42 (G?)
	AM_RANGE(0xa00800, 0xa00bff) AM_RAM // ??? see subroutine $39f42 (G?)
	AM_RANGE(0xa00c00, 0xa00c1f) AM_READ(galpani3_regs2_r) // ? G layer regs ? see subroutine $3a03e
	AM_RANGE(0xa80000, 0xa801ff) AM_RAM // area [H] - G area ? linescroll ?
	AM_RANGE(0xb00000, 0xb7ffff) AM_RAM // area [E] - G area ? odd bytes only, initialized 00..ff,00..ff,...

	// GRAP2 3?
	AM_RANGE(0xc00000, 0xc003ff) AM_RAM // ??? see subroutine $39f42 (B?)
	AM_RANGE(0xc00800, 0xc00bff) AM_RAM // ??? see subroutine $39f42 (B?)
	AM_RANGE(0xc00c00, 0xc00c1f) AM_READ(galpani3_regs3_r) // ? B layer regs ? see subroutine $3a03e
	AM_RANGE(0xc80000, 0xc801ff) AM_RAM // area [I] - B area ? linescroll ?
	AM_RANGE(0xd00000, 0xd7ffff) AM_RAM // area [F] - B area ? odd bytes only, initialized 00..ff,00..ff,...

	// ??
	AM_RANGE(0xe00000, 0xe7ffff) AM_RAM // area [J] - A area ? odd bytes only, initialized 00..ff,00..ff,..., then cleared

	AM_RANGE(0xf00000, 0xf00001) AM_NOP // ? written once (2nd opcode, $1.b)
	AM_RANGE(0xf00010, 0xf00011) AM_READ_PORT("P1")
	AM_RANGE(0xf00012, 0xf00013) AM_READ_PORT("P2")
	AM_RANGE(0xf00014, 0xf00015) AM_READ_PORT("COIN")
	AM_RANGE(0xf00016, 0xf00017) AM_NOP // ? read, but overwritten
	AM_RANGE(0xf00020, 0xf00021) AM_WRITE(ymz280b_register_0_lsb_w)	// sound
	AM_RANGE(0xf00022, 0xf00023) AM_WRITE(ymz280b_data_0_lsb_w)		//
	AM_RANGE(0xf00040, 0xf00041) AM_READWRITE(watchdog_reset16_r, watchdog_reset16_w)	// watchdog
	AM_RANGE(0xf00050, 0xf00051) AM_NOP // ? written once (3rd opcode, $30.b)
ADDRESS_MAP_END


static const ymz280b_interface ymz280b_intf =
{
	0	// irq ?
};

static MACHINE_DRIVER_START( galpani3 )
	MDRV_CPU_ADD("main", M68000, 16000000)	 // ? (from which clock?)
	MDRV_CPU_PROGRAM_MAP(galpani3_map,0)
	MDRV_CPU_VBLANK_INT_HACK(galpani3_vblank, 3)


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(galpani3)
	MDRV_VIDEO_UPDATE(galpani3)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymz", YMZ280B, 28636400 / 2)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)
	MDRV_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( galpani3 )
	ROM_REGION( 0x100000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0j1.71",  0x000000, 0x080000, CRC(52893326) SHA1(78fdbf3436a4ba754d7608fedbbede5c719a4505) )
	ROM_LOAD16_BYTE( "g3p1j1.102", 0x000001, 0x080000, CRC(05f935b4) SHA1(81e78875585bcdadad1c302614b2708e60563662) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )		// 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )		// 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )		// 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )		// 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0j0.101", 0x800000, 0x040000, CRC(fbb1e0dc) SHA1(14f6377afd93054aa5dc38af235ae12b932e847f) )	// 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1j0.100", 0x800001, 0x040000, CRC(18edb5f0) SHA1(5e2ed0105b3e6037f6116494d3b186a368824171) )	//

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END


static DRIVER_INIT( galpani3 )
{
	DRIVER_INIT_CALL( decrypt_toybox_rom );

	memset(galpani3_mcu_com, 0, 4 * sizeof( UINT16) );
}

GAME( 1995, galpani3, 0, galpani3, galpani3, galpani3, ROT90, "Kaneko", "Gals Panic 3", GAME_NOT_WORKING )
