/*
   Taito Type-Zero hardware

   Skeleton driver.

Landing High Japan PCB info
===========================

Taito Landing High

Top board
    silkscreened        TYPE-ZERO MOTHER PCB

    stickered       298100308
                K11X0886A
                JC101

.5  29LV400BC-90        stamped E68-05-1
.6  29LV400BC-90        stamped E68-04-1
.24 PALV 18V8-10JC      stamped E68-06


IC30    Taito TCG020AGP
IC26    IDT7024 S35J V9928P
IC10    IBM EMPPC603eBG-100
IC53    ADV7120KP30 9926 F101764.1
IC16,17         M54256V32A-10
IC15,25,31,39,45,49,44  48D4811650GF-A10-9BT
IC27,36         D4564163G5-A10n-9JF
IC 41,46,42,47      D4516161AG5-A10B-9F
IC43            QSV991-7JRI
66.6667 Oscillator near IC43

Bottom board
    silkscreened        JC101 DAUGHTER PCB

    stickered       K91J0775A
                LANDING H.JAPAN

                299100308

                M43J0741A
                LANDING H.JAPAN

.14 27c1001     stickered   E82
                    03*

.15 27c1001     stickered   E82
                    04*

.44 PALCE16V8H  stamped     E82-01

.45 PALCE22V10H stamped     E82-02

IC40    Toshiba TMP95C063F
IC55    Panasonic MN89306
EPSON 9X5C pscillator near IC55
IC56    HY57V161610D TC-10
IC22    ID7133 SA70J
25.000 oscillator near IC22
IC11    Xilinx XC9572
IC5 HY5118164C JC-60
IC10    ZOOM ZSG-2
IC20    ZOOM ZFX 2 HD 96NE2VJ
IC26    TM TECH  UA4464V T224162B-28J
IC7 Panasonic MN1020819DA E68-01
20.000 oscillator near IC7

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/tlcs900/tlcs900.h"

// Interesting mem areas

// 0x40080400...0x400807fc: PPC Interrupt handler table for TLCS900 interrupts (called from 0x4002ea80)
//      0x40080440 (0x10): 0x4003b274       (0xfc06f7 on TLCS sets 0x1000)
//      0x400804c0 (0x30): 0x4003f1e4       (INT1 handler on TLCS sets 0x3010)  (comms?)
//      0x40080500 (0x40): 0x4002ed94       (debug/trace?)
//      0x40080504 (0x41): 0x4002ede4       (debug/trace?)
//      0x40080540 (0x50): 0x4003c21c
//      0x40080740 (0xd0): 0x4002ecc0       (INT3 handler on TLCS sets 0xd000)
//      0x400807c0 (0xf0): 0x4002eae4       (0xfc0a44 on TLCS sets 0xf001)


class taitotz_state : public driver_device
{
public:
	taitotz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 video_char_ram[0x4000];
	UINT32 video_tile_ram[0x20000];
	UINT32 video_screen_ram[0x40000];

	UINT32 video_reg;
	UINT32 *video_ram;
	UINT32 video_ram_ptr;

	bitmap_rgb32 framebuffer;
};



static VIDEO_START( taitotz )
{
	taitotz_state *state = machine.driver_data<taitotz_state>();

	machine.primary_screen->register_screen_bitmap(state->framebuffer);
}

static SCREEN_UPDATE_RGB32( taitotz )
{
	/*
    {
        UINT16 *s = (UINT16*)video_char_ram;
        int x,y,t,u;
        for (u=0; u < 16; u++)
        {
            for (t=0; t < 16; t++)
            {
                UINT32 tile = (u*16)+t;

                tile &= 0xff;

                for (y=0; y < FONT_HEIGHT; y++)
                {
                    UINT32 *fb = &bitmap.pix32(y+(u*FONT_WIDTH));
                    for (x=0; x < FONT_WIDTH; x++)
                    {
                        UINT32 p = s[((tile*(FONT_WIDTH*FONT_HEIGHT*2)) + ((y*FONT_WIDTH)+x)) ^ 1];
                        if (p) p = 0xffffffff;
                        fb[x+(t*FONT_HEIGHT)] = p;
                    }
                }
            }
        }
    }
    */

	/*
    {
        UINT16 *s = (UINT16*)video_char_ram;
        int t,u;
        for (u=0; u < 256; u++)
        {
            UINT32 *fb = &bitmap.pix32(u);
            for (t=0; t < 512; t++)
            {
                UINT32 p = s[((u*512)+t) ^ 1];
                if (p) p = 0xffffffff;
                fb[t] = p;
            }
        }
    }
    */

	taitotz_state *state = screen.machine().driver_data<taitotz_state>();

	bitmap.fill(0, cliprect);
	copybitmap_trans(bitmap, state->framebuffer, 0, 0, 0, 0, cliprect, 0);

	return 0;
}

static void draw_tile(running_machine &machine, UINT32 pos, UINT32 tile)
{
	taitotz_state *state = machine.driver_data<taitotz_state>();

	int tileu = (tile & 0x1f) * 16;
	int tilev = ((tile >> 5) & 0x1f) * 16;

	int tilex = (pos & 0x1f) * 16;
	int tiley = ((pos >> 5) & 0x1f) * 16;

	UINT16 *src_tile = (UINT16*)state->video_char_ram;

	int v = tilev;

	for (int j=tiley; j < (tiley+16); j++)
	{
		int u = tileu;

		UINT32 *fb = &state->framebuffer.pix32(j);

		for (int i=tilex; i < (tilex+16); i++)
		{
			UINT32 p = src_tile[((v*512) + u) ^ 1];
			fb[i] = p ? 0xffffffff : 0;
			u++;
		}
		v++;
	}
}

static void video_reg_w(running_machine &machine, UINT32 reg, UINT32 data)
{
	taitotz_state *state = machine.driver_data<taitotz_state>();

	switch ((reg >> 28) & 0xf)
	{
	case 0x1:		// Register write?
		{
			logerror("video_reg_w: r: %08X d: %08X\n", reg, data);
			break;
		}
	case 0x3:		// Tile write?
		{
			UINT32 pos = (data >> 12) & 0xfff;
			UINT32 tile = data & 0xfff;
			draw_tile(machine, pos, tile);
			break;
		}
	case 0xb:		// RAM write?
		{
			state->video_ram[state->video_ram_ptr++] = data;
			break;
		}
	default:
		{
			logerror("video_reg_w: r: %08X d: %08X\n", reg, data);
			break;
		}
	}
}

// Video chip RAM commands?

// 0xB0800000, size 0x200000  : screen RAM
// 0xB0980000, size 0x4000?   : char RAM?
// 0xB09C0000, size 0x20000   : tile RAM?
// 0xB1800000, size 0x80000   : frame RAM
// 0xB1000000, size 0x800000  : texture RAM


static WRITE64_HANDLER(video_chip_w)
{
	taitotz_state *state = space->machine().driver_data<taitotz_state>();

	//printf("unk_w: %08X, %08X%08X, %08X%08X at %08X\n", offset, (UINT32)(data >> 32), (UINT32)(data), (UINT32)(mem_mask >> 32), (UINT32)(mem_mask), cpu_get_pc(space->cpu));

	UINT32 reg = offset * 8;
	UINT32 regdata;

	if (ACCESSING_BITS_0_31)
	{
		reg += 4;
		regdata = (UINT32)(data);
	}
	else
	{
		regdata = (UINT32)(data >> 32);
	}

	switch (reg)
	{
		case 0:
			{
				video_reg_w(space->machine(), state->video_reg, regdata);
				break;
			}
		case 0x8:
			{
				state->video_reg = regdata;

				if ((state->video_reg & 0xf0000000) == 0xb0000000)
				{
					state->video_ram_ptr = 0;

					switch (state->video_reg & 0x0fffffff)
					{
						case 0x0980000: state->video_ram = state->video_char_ram; break;
						case 0x09c0000: state->video_ram = state->video_tile_ram; break;
						case 0x1800000: state->video_ram = state->video_screen_ram; break;
						default:
							{
								fatalerror("video_chip_ram sel %08X\n", state->video_reg & 0x0fffffff);
								break;
							}
					}
				}
				break;
			}
		case 0xc:
			{
				logerror("video_chip_w: port 0xc: %08X\n", regdata);
				break;
			}
	}
}

static READ64_HANDLER(ppc_common_r)
{
	if (offset == 0x7ff)
	{
		return U64(0x0000d00000000000);
	}
	if (offset == 1)
	{
		return U64(0x0000455200005200);
	}
	return 0;
}

static WRITE64_HANDLER(ppc_common_w)
{
	if (offset == 0x7ff)
	{
		cputag_set_input_line(space->machine(), "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

// BAT Config:
// IBAT0 U: 0xf0001fff   L: 0xf0000023   (0xf0000000...0xffffffff)
// IBAT1 U: 0xe0001fff   L: 0xe0000023   (0xe0000000...0xefffffff)
// IBAT2 U: 0x40001fff   L: 0x40000003   (0x40000000...0x4fffffff)
// IBAT3 U: 0xa0001fff   L: 0xa0000023   (0xa0000000...0xafffffff)
// DBAT0 U: 0x00001fff   L: 0x00000022   (0x00000000...0x0fffffff)
// DBAT1 U: 0x10001fff   L: 0x10000002   (0x10000000...0x1fffffff)
// DBAT2 U: 0x40001fff   L: 0x40000002   (0x40000000...0x4fffffff)
// DBAT3 U: 0xa0001fff   L: 0xa0000022   (0xa0000000...0xafffffff)

// 0x10000000...0x1000001f: texture FIFO?
// 0x40000000...0x400fffff: BIOS Work RAM
// 0x40100000...0x40ffffff: User Work RAM

static ADDRESS_MAP_START( ppc603e_mem, AS_PROGRAM, 64, taitotz_state )
	//AM_RANGE(0x00000000, 0x00000007) AM_RAM   // Register/RAM access port? - Written 128k+256k times on boot
	//AM_RANGE(0x00000008, 0x0000000f) AM_RAM   // Register/RAM address port?
	AM_RANGE(0x00000000, 0x0000000f) AM_WRITE(video_chip_w)
	AM_RANGE(0x40000000, 0x400fffff) AM_RAM   // Work RAM
	AM_RANGE(0xa8000000, 0xa8003fff) AM_READWRITE(ppc_common_r, ppc_common_w)   // Common RAM (with TLCS-900)
	//AM_RANGE(0xa8003ff8, 0xa8003fff) AM_RAM   // TLCS-900 related?
	AM_RANGE(0xac000000, 0xac0fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

// TLCS900 interrupt vectors
// 0xfc0150:    reset
// 0xfc0120:    SWI1-7, NMI, INTWD, INTT0, INTT2-7, INTTR8-A, INTAD, INTTC0-3
// 0xfc0122:    INT0
// 0xfc0137:    INT1
// 0xfc013c:    INT2
// 0xfc0141:    INT3
// 0xfc0146:    INT4
// 0xfc0147:    INT5
// 0xfc0148:    INT6
// 0xfc0149:    INT7
// 0xfc014a:    INT8
// 0xfc0467:    INTT1
// 0xfc0d1d:    INTRX0
// 0xfc0ca5:    INTTX0
// 0xfc0d55:    INTRX1
// 0xfc0ce1:    INTTX1

static ADDRESS_MAP_START( tlcs900h_mem, AS_PROGRAM, 8, taitotz_state )
	AM_RANGE(0x010000, 0x02ffff) AM_RAM		// Work RAM
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("io_cpu", 0)
ADDRESS_MAP_END



static WRITE8_DEVICE_HANDLER( taitotz_tlcs900_to1 )
{
}

static WRITE8_DEVICE_HANDLER( taitotz_tlcs900_to3 )
{
}



static INPUT_PORTS_START( taitotz )
INPUT_PORTS_END


static MACHINE_RESET( taitotz )
{
	//cputag_set_input_line(machine, "iocpu", INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_START( taitotz )
{
	/*
    UINT32 *rom = (UINT32*)machine->region("user1")->base();

    rom[(0x228b0^4)/4] = 0x60000000;
    rom[(0x22900^4)/4] = 0x60000000;
    rom[(0x2293c^4)/4] = 0x60000000;
    */
}

static INTERRUPT_GEN( taitotz_vbi )
{

}


static const powerpc_config ppc603e_config =
{
	XTAL_66_6667MHz		/* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
};

static const tlcs900_interface taitotz_tlcs900_interface =
{
	DEVCB_HANDLER( taitotz_tlcs900_to1 ),
	DEVCB_HANDLER( taitotz_tlcs900_to3 )
};


static MACHINE_CONFIG_START( taitotz, taitotz_state )
	MCFG_CPU_ADD("maincpu", PPC603E, 10000000)
	MCFG_CPU_CONFIG(ppc603e_config)
	MCFG_CPU_PROGRAM_MAP(ppc603e_mem)
	MCFG_CPU_VBLANK_INT("screen", taitotz_vbi)

	/* TMP95C063F I/O CPU */
	MCFG_CPU_ADD("iocpu", TLCS900H, 25000000)
	MCFG_CPU_CONFIG(taitotz_tlcs900_interface)
	MCFG_CPU_PROGRAM_MAP(tlcs900h_mem)

	/* MN1020819DA sound CPU */

	MCFG_MACHINE_START( taitotz )
	MCFG_MACHINE_RESET( taitotz )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 511)
	MCFG_SCREEN_UPDATE_STATIC(taitotz)

	MCFG_VIDEO_START(taitotz)
MACHINE_CONFIG_END

#define TAITOTZ_BIOS	\
	ROM_LOAD32_WORD_SWAP( "e68-05-1.ic6", 0x000000, 0x080000, CRC(6ad9b006) SHA1(f05a0ae26b6abaeda9c7944aee96c72b08fff7a5) )	\
	ROM_LOAD32_WORD_SWAP( "e68-04-1.ic5", 0x000002, 0x080000, CRC(c7c2dc6b) SHA1(bf88c818166c285130c5c73d6982f009da26e143) )


ROM_START( taitotz )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "sound_cpu", ROMREGION_ERASE00 ) /* Internal ROM :( */
	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	DISK_REGION( "ide" )
ROM_END

ROM_START( landhigh )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e82-03.ic14", 0x000000, 0x020000, CRC(0de65b4d) SHA1(932316f7435259b723a29843d58b2e3dca92e7b7) )
	ROM_LOAD16_BYTE( "e82-04.ic15", 0x000001, 0x020000, CRC(b3cb0f3d) SHA1(80414f50a1593c6b849d9f37e94a32168699a5c1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x500, "plds", 0 )
	ROM_LOAD( "e82-01.ic44", 0x000, 0x117, CRC(49eea30f) SHA1(ef97c792358f05b9214a2f58ee1e97e8208806c4) )
	ROM_LOAD( "e82-02.ic45", 0x117, 0x2dd, CRC(f581cff5) SHA1(468e0e6a3828f2dcda35c6d523154510f9c99db7) )
	ROM_LOAD( "e68-06.ic24", 0x3f4, 0x100, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "landhigh", 0, SHA1(7cea4ea5c3899e6ac774a4eb12821f44541d9c9c) )
ROM_END

ROM_START( batlgear )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e68-07.ic14",  0x000000, 0x020000, CRC(554c6fd7) SHA1(9f203dead81c7ccf73d7fd462cab147cd17f890f) )
	ROM_LOAD16_BYTE( "e68-08.ic15",  0x000001, 0x020000, CRC(f1932380) SHA1(64d12e858af15a9ba8254917da13863ac7c9c050) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "batlgear", 0, SHA1(eab283839ad3e0a3e6be11f6482570db334eacca) )
ROM_END

ROM_START( batlgr2 )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
	ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "bg2_204j", 0, SHA1(7ac100fba39ae0b93980c0af2f0212a731106912) )
ROM_END

ROM_START( batlgr2a )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
	ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "bg2_201j", 0, SHA1(542d12682bd0f95143368578461c6a4fcc492fcc) )
ROM_END

ROM_START( pwrshovl )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
    ROM_LOAD16_BYTE( "e74-04.ic14",   0x000000, 0x020000, CRC(ef21a261) SHA1(7398826dbf48014b9c7e9454f978f3e419ebc64b) )
    ROM_LOAD16_BYTE( "e74-05.ic15",   0x000001, 0x020000, CRC(2466217d) SHA1(dc814da3a1679cff001f179d3c1641af985a6490) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "pwrshovl", 0, SHA1(360f63b39f645851c513b4644fb40601b9ba1412) )
ROM_END

GAME( 1999, taitotz,  0, taitotz, taitotz, 0, ROT0, "Taito", "Type Zero BIOS", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT)
GAME( 1999, landhigh, taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Landing High Japan", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1999, batlgear, taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Battle Gear", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1999, pwrshovl, taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Power Shovel ni Norou!! - Power Shovel Simulator", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, batlgr2,  taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Battle Gear 2 (v2.04J)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, batlgr2a, batlgr2, taitotz, taitotz, 0, ROT0, "Taito", "Battle Gear 2 (v2.01J)", GAME_NOT_WORKING | GAME_NO_SOUND )

