/*  Konami Hornet System

    Driver by Ville Linde



    Hardware overview:

    GN715 CPU Board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Ricoh RF5c400 sound chip

    GN715 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami 0000037122 (2D Tilemap)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0003-03 (Voodoo) FBI with 2MB RAM
        3DFX 500-0004-02 (Voodoo) TMU with 2MB RAM

    GQ871 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami 0000037122 (2D Tilemap)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0009-01 (Voodoo 2) FBI with 2MB RAM
        3DFX 500-0010-01 (Voodoo 2) TMU with 4MB RAM


    Hardware configurations:

    Game             | ID        | CPU PCB      | CG Board(s)   | LAN PCB
    ---------------------------------------------------------------------------
    Gradius 4        | GX837     | GN715(A)     | GN715(B)      |
    NBA Play by Play | GX778     | GN715(A)     | GN715(B)      |
    Silent Scope     | GQ830     | GN715(A)     | 2x GN715(B)   |
    Silent Scope 2   | GQ931     | GN715(A)     | 2x GQ871(B)   | GQ931(H)




    Top Board GN715 PWB(A)A
    |--------------------------------------------------------------|
    |                                                              |
    |                                                        PAL   |
    |               PAL               68EC000          837A08.7S   |
    |NE5532         PAL                                            |
    |                         DRM1M4SJ8                            |
    |NE5532                                                        |
    |     SM5877              RF5C400                              |
    |                                                              |
    |     SM5877     16.9344MHz                                    |
    |                SRAM256K               837A10.14P  837A05.14T |
    |                                                              |
    |                SRAM256K               837A09.16P  837A04.16T |
    |  ADC12138                                                    |
    |             056800                                           |
    |                                                              |
    |                      MACH111                                 |
    |                                                              |
    |                      DRAM16X16                      PPC403GA |
    |                                   837C01.27P                 |
    |                                                              |
    |                      DRAM16X16                               |
    | 4AK16                                                        |
    |                                                              |
    |                                                              |
    | 0038323  PAL                                        7.3728MHz|
    | E9825                                                        |
    |                                                     50.000MHz|
    |                                                              |
    |M48T58Y-70PC1                                        64.000MHz|
    |--------------------------------------------------------------|

    Notes:
        DRM1M4SJ8 = Fujitsu 81C4256 DRAM (SOJ24)
        SRAM256K = Cypress CY7C199 SRAM (SOJ28)
        DRAM16X16 = Fujitsu 8118160A-60 DRAM (SOJ42)
        0038323 E9825 = SOIC8, I've seen a similar chip in the security cart of System573
        M48T58Y-70PC1 = ST Timekeeper RAM


    Bottom Board GN715 PWB(B)A
    |--------------------------------------------------------------|
    |                                                              |
    |JP1                                          4M EDO   4M EDO  |
    |                                                              |
    |  4M EDO 4M EDO      TEXELFX                                  |
    |                                                       4M EDO |
    |  4M EDO 4M EDO                  PIXELFX               4M EDO |
    |                                                              |
    |  4M EDO 4M EDO                                KONAMI         |
    |                                   50MHz       0000033906     |
    |  4M EDO 4M EDO                                               |
    |                       256KSRAM 256KSRAM                      |
    |                                                              |
    |         AV9170                     1MSRAM 1MSRAM             |
    | MC44200                                                      |
    |                       256KSRAM 256KSRAM                      |
    |                                    1MSRAM 1MSRAM             |
    |                                               837A13.24U     |
    |  KONAMI    MACH111                                 837A15.24V|
    |  0000037122                        1MSRAM 1MSRAM             |
    |                       ADSP-21062                             |
    |                       SHARC    36.00MHz                      |
    |1MSRAM                              1MSRAM 1MSRAM             |
    |                                                              |
    |1MSRAM                  PAL  PAL                              |
    |           256KSRAM                            837A14.32U     |
    |1MSRAM     256KSRAM                                 837A16.32V|
    |           256KSRAM                                           |
    |1MSRAM                                                        |
    |           JP2                                                |
    |--------------------------------------------------------------|

    Notes:
        4M EDO = SM81C256K16CJ-35 RAM 66MHz
        1MSRAM = CY7C109-25VC
        256KSRAM = W24257AJ-15
        TEXELFX = 3DFX 500-0004-02 BD0665.1 TMU (QFP208)
        PIXELFX = 3DFX 500-0003-03 F001701.1 FBI (QFP240)
        JP1 = Jumper set to SCR. Alt. setting is TWN
        JP2 = Jumper set for MASTER, Alt. setting SLAVE



    LAN PCB: GQ931 PWB(H)      (C) 1999 Konami
    ------------------------------------------

    2 x LAN ports, LANC(1) & LANC(2)
    1 x 32.0000MHz Oscillator

         HYC2485S  SMC ARCNET Media Transceiver, RS485 5Mbps-2.5Mbps
    8E   931A19    Konami 32meg masked ROM, ROM0 (compressed GFX data?)
    6E   931A20    Konami 32meg masked ROM, ROM1 (compressed GFX data?)
    12F  XC9536    Xilinx  CPLD,  44 pin PLCC, Konami no. Q931H1
    12C  XCS10XL   Xilinx  FPGA, 100 pin PQFP, Konami no. 4C
    12B  CY7C199   Cypress 32kx8 SRAM
    8B   AT93C46   Atmel 1K serial EEPROM, 8 pin SOP
    16G  DS2401    Dallas Silicon Serial Number IC, 6 pin SOP



    GFX PCB:  GQ871 PWB(B)A    (C) 1999 Konami
    ------------------------------------------

    There are no ROMs on the two GFX PCBs, all sockets are empty.
    Prior to the game starting there is a message saying downloading data.


    Jumpers set on GFX PCB to main monitor:
    4A   set to TWN (twin GFX PCBs)
    37D  set to Master


    Jumpers set on GFX PCB to scope monitor:
    4A   set to TWN (twin GFX PCBs)
    37D  set to Slave



    1 x 64.0000MHz
    1 x 36.0000MHz  (to 27L, ADSP)

    21E  AV9170           ICS, Clock synchroniser and multiplier

    27L  ADSP-21062       Analog Devices SHARC ADSP, 512k flash, Konami no. 022M16C
    15T  0000033906       Konami Custom, 160 pin PQFP
    19R  W241024AI-20     Winbond, 1Meg SRAM
    22R  W241024AI-20     Winbond, 1Meg SRAM
    25R  W241024AI-20     Winbond, 1Meg SRAM
    29R  W241024AI-20     Winbond, 1Meg SRAM
    19P  W241024AI-20     Winbond, 1Meg SRAM
    22P  W241024AI-20     Winbond, 1Meg SRAM
    25P  W241024AI-20     Winbond, 1Meg SRAM
    29P  W241024AI-20     Winbond, 1Meg SRAM
    18N  W24257AJ-15      Winbond, 256K SRAM
    14N  W24257AJ-15      Winbond, 256K SRAM
    18M  W24257AJ-15      Winbond, 256K SRAM
    14M  W24257AJ-15      Winbond, 256K SRAM

    28D  000037122        Konami Custom, 208 pin PQFP
    33E  W24257AJ-15      Winbond, 256K SRAM
    33D  W24257AJ-15      Winbond, 256K SRAM
    33C  W24257AJ-15      Winbond, 256K SRAM
    27A  W241024AI-20     Winbond, 1Meg SRAM
    30A  W241024AI-20     Winbond, 1Meg SRAM
    32A  W241024AI-20     Winbond, 1Meg SRAM
    35A  W241024AI-20     Winbond, 1Meg SRAM

    7K   500-0010-01      3DFX, Texture processor
    16F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    13F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    9F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    5F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    16D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    13D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    9D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    5D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg

    9P   500-0009-01      3DFX, Pixel processor
    10U  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    7U   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    3S   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    3R   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg

    27G  XC9536           Xilinx, CPLD, Konami no. Q830B1
    21C  MC44200FT        Motorola, 3 Channel video D/A converter
*/

#include "driver.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "machine/konamiic.h"
#include "video/voodoo.h"
#include "machine/timekpr.h"
#include "sound/rf5c400.h"
#include "rendlay.h"

static UINT8 led_reg0 = 0x7f, led_reg1 = 0x7f;

/* K037122 Tilemap chip (move to konamiic.c ?) */

#define MAX_K037122_CHIPS	2

static UINT32 *K037122_tile_ram[MAX_K037122_CHIPS];
static UINT32 *K037122_char_ram[MAX_K037122_CHIPS];
static UINT8 *K037122_dirty_map[MAX_K037122_CHIPS];
static int K037122_gfx_index[MAX_K037122_CHIPS], K037122_char_dirty[MAX_K037122_CHIPS];
static tilemap *K037122_layer[MAX_K037122_CHIPS][2];
static UINT32 K037122_reg[MAX_K037122_CHIPS][256];

#define K037122_NUM_TILES		16384

static const gfx_layout K037122_char_layout =
{
	8, 8,
	K037122_NUM_TILES,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
};

static TILE_GET_INFO( K037122_0_tile_info_layer0 )
{
	UINT32 val = K037122_tile_ram[0][tile_index + (0x8000/4)];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(K037122_gfx_index[0], tile, color, flags);
}

static TILE_GET_INFO( K037122_0_tile_info_layer1 )
{
	UINT32 val = K037122_tile_ram[0][tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(K037122_gfx_index[0], tile, color, flags);
}

static TILE_GET_INFO( K037122_1_tile_info_layer0 )
{
	UINT32 val = K037122_tile_ram[1][tile_index + (0x8000/4)];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(K037122_gfx_index[1], tile, color, flags);
}

static TILE_GET_INFO( K037122_1_tile_info_layer1 )
{
	UINT32 val = K037122_tile_ram[1][tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(K037122_gfx_index[1], tile, color, flags);
}

static int K037122_vh_start(running_machine *machine, int chip)
{
	for(K037122_gfx_index[chip] = 0; K037122_gfx_index[chip] < MAX_GFX_ELEMENTS; K037122_gfx_index[chip]++)
		if (machine->gfx[K037122_gfx_index[chip]] == 0)
			break;
	if(K037122_gfx_index[chip] == MAX_GFX_ELEMENTS)
		return 1;

	K037122_char_ram[chip] = auto_malloc(0x200000);

	K037122_tile_ram[chip] = auto_malloc(0x20000);

	K037122_dirty_map[chip] = auto_malloc(K037122_NUM_TILES);

	if (chip == 0)
	{
		K037122_layer[chip][0] = tilemap_create(K037122_0_tile_info_layer0, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 256, 64);
		K037122_layer[chip][1] = tilemap_create(K037122_0_tile_info_layer1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 128, 64);
	}
	else
	{
		K037122_layer[chip][0] = tilemap_create(K037122_1_tile_info_layer0, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 256, 64);
		K037122_layer[chip][1] = tilemap_create(K037122_1_tile_info_layer1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 128, 64);
	}

	tilemap_set_transparent_pen(K037122_layer[chip][0], 0);
	tilemap_set_transparent_pen(K037122_layer[chip][1], 0);

	memset(K037122_char_ram[chip], 0, 0x200000);
	memset(K037122_tile_ram[chip], 0, 0x20000);
	memset(K037122_dirty_map[chip], 0, K037122_NUM_TILES);

	machine->gfx[K037122_gfx_index[chip]] = allocgfx(&K037122_char_layout);
	decodegfx(machine->gfx[K037122_gfx_index[chip]], (UINT8*)K037122_char_ram[chip], 0, machine->gfx[K037122_gfx_index[chip]]->total_elements);

	if (machine->drv->color_table_len)
		machine->gfx[K037122_gfx_index[chip]]->total_colors = machine->drv->color_table_len / 16;
	else
		machine->gfx[K037122_gfx_index[chip]]->total_colors = machine->drv->total_colors / 16;

	return 0;
}

static void K037122_tile_update(running_machine *machine, int chip)
{
	if (K037122_char_dirty[chip])
	{
		int i;
		for (i=0; i < K037122_NUM_TILES; i++)
		{
			if (K037122_dirty_map[chip][i])
			{
				K037122_dirty_map[chip][i] = 0;
				decodechar(machine->gfx[K037122_gfx_index[chip]], i, (UINT8 *)K037122_char_ram[chip]);
			}
		}
		tilemap_mark_all_tiles_dirty(K037122_layer[chip][0]);
		tilemap_mark_all_tiles_dirty(K037122_layer[chip][1]);
		K037122_char_dirty[chip] = 0;
	}
}

static void K037122_tile_draw(int chip, mame_bitmap *bitmap, const rectangle *cliprect)
{
	if (K037122_reg[chip][0xc] & 0x10000)
	{
		tilemap_set_scrolldx(K037122_layer[chip][1], Machine->screen[0].visarea.min_x, Machine->screen[0].visarea.min_x);
		tilemap_set_scrolldy(K037122_layer[chip][1], Machine->screen[0].visarea.min_y, Machine->screen[0].visarea.min_y);
		tilemap_draw(bitmap, cliprect, K037122_layer[chip][1], 0,0);
	}
	else
	{
		tilemap_set_scrolldx(K037122_layer[chip][0], Machine->screen[0].visarea.min_x, Machine->screen[0].visarea.min_x);
		tilemap_set_scrolldy(K037122_layer[chip][0], Machine->screen[0].visarea.min_y, Machine->screen[0].visarea.min_y);
		tilemap_draw(bitmap, cliprect, K037122_layer[chip][0], 0,0);
	}
}

static void update_palette_color(running_machine *machine, int chip, UINT32 palette_base, int color)
{
	UINT32 data = K037122_tile_ram[chip][(palette_base/4) + color];
	palette_set_color_rgb(machine, color, pal5bit(data >> 6), pal6bit(data >> 0), pal5bit(data >> 11));
}

static READ32_HANDLER(K037122_sram_r)
{
	int chip = get_cgboard_id();

	return K037122_tile_ram[chip][offset];
}

static WRITE32_HANDLER(K037122_sram_w)
{
	int chip = get_cgboard_id();

	COMBINE_DATA(K037122_tile_ram[chip] + offset);

	if (K037122_reg[chip][0xc] & 0x10000)
	{
		if (offset < 0x8000/4)
		{
			tilemap_mark_tile_dirty(K037122_layer[chip][1], offset);
		}
		else if (offset >= 0x8000/4 && offset < 0x18000/4)
		{
			tilemap_mark_tile_dirty(K037122_layer[chip][0], offset - (0x8000/4));
		}
		else if (offset >= 0x18000/4)
		{
			update_palette_color(Machine, chip, 0x18000, offset - (0x18000/4));
		}
	}
	else
	{
		if (offset < 0x8000/4)
		{
			update_palette_color(Machine, chip, 0, offset);
		}
		else if (offset >= 0x8000/4 && offset < 0x18000/4)
		{
			tilemap_mark_tile_dirty(K037122_layer[chip][0], offset - (0x8000/4));
		}
		else if (offset >= 0x18000/4)
		{
			tilemap_mark_tile_dirty(K037122_layer[chip][1], offset - (0x18000/4));
		}
	}
}


static READ32_HANDLER(K037122_char_r)
{
	int chip = get_cgboard_id();

	UINT32 addr;
	int bank = K037122_reg[chip][0x30/4] & 0x7;

	addr = offset + (bank * (0x40000/4));

	return K037122_char_ram[chip][addr];
}

static WRITE32_HANDLER(K037122_char_w)
{
	int chip = get_cgboard_id();

	UINT32 addr;
	int bank = K037122_reg[chip][0x30/4] & 0x7;

	addr = offset + (bank * (0x40000/4));

	COMBINE_DATA(K037122_char_ram[chip] + addr);
	K037122_dirty_map[chip][addr / 32] = 1;
	K037122_char_dirty[chip] = 1;
}

static READ32_HANDLER(K037122_reg_r)
{
	int chip = get_cgboard_id();

	switch (offset)
	{
		case 0x14/4:
		{
			return 0x000003fa;
		}
	}
	return K037122_reg[chip][offset];
}

static WRITE32_HANDLER(K037122_reg_w)
{
	int chip = get_cgboard_id();

	COMBINE_DATA( K037122_reg[chip] + offset );
}

static int voodoo_version = 0;

static void voodoo_vblank_0(int param)
{
	cpunum_set_input_line(0, INPUT_LINE_IRQ0, ASSERT_LINE);
}

static void voodoo_vblank_1(int param)
{
	cpunum_set_input_line(0, INPUT_LINE_IRQ1, ASSERT_LINE);
}

static void hornet_exit(running_machine *machine)
{
	voodoo_exit(0);
}

static void hornet_2board_exit(running_machine *machine)
{
	voodoo_exit(0);
	voodoo_exit(1);
}

static VIDEO_START( hornet )
{
	add_exit_callback(machine, hornet_exit);

	if (voodoo_version == 0)
		voodoo_start(0, 0, VOODOO_1, 2, 4, 0);
	else
		voodoo_start(0, 0, VOODOO_2, 2, 4, 0);

	voodoo_set_vblank_callback(0, voodoo_vblank_0);

	K037122_vh_start(machine, 0);
}

static VIDEO_START( hornet_2board )
{
	add_exit_callback(machine, hornet_2board_exit);

	if (voodoo_version == 0)
	{
		voodoo_start(0, 0, VOODOO_1, 2, 4, 0);
		voodoo_start(1, 1, VOODOO_1, 2, 4, 0);
	}
	else
	{
		voodoo_start(0, 0, VOODOO_2, 2, 4, 0);
		voodoo_start(1, 1, VOODOO_2, 2, 4, 0);
	}

	voodoo_set_vblank_callback(0, voodoo_vblank_0);
	voodoo_set_vblank_callback(1, voodoo_vblank_1);

	K037122_vh_start(machine, 0);
	K037122_vh_start(machine, 1);
}


static VIDEO_UPDATE( hornet )
{
	voodoo_update(0, bitmap, cliprect);

	K037122_tile_update(machine, 0);
	K037122_tile_draw(0, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);
	return 0;
}

static VIDEO_UPDATE( hornet_2board )
{
	voodoo_update(screen, bitmap, cliprect);

	/* TODO: tilemaps per screen */
	K037122_tile_update(machine, screen);
	K037122_tile_draw(screen, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);
	return 0;
}

/*****************************************************************************/

static READ32_HANDLER( sysreg_r )
{
	UINT32 r = 0;
	if (offset == 0)
	{
		if (!(mem_mask & 0xff000000))
		{
			//printf("read sysreg 0\n");
			r |= readinputport(0) << 24;
		}
		if (!(mem_mask & 0x00ff0000))
		{
			r |= readinputport(1) << 16;
		}
		if (!(mem_mask & 0x0000ff00))
		{
			r |= readinputport(2) << 8;
		}
		if (!(mem_mask & 0x000000ff))
		{
			r |= 0xf7;
		}
	}
	else if (offset == 1)
	{
		if (!(mem_mask & 0xff000000))
		{
			r |= readinputport(3) << 24;
		}
	}
	return r;
}

static WRITE32_HANDLER( sysreg_w )
{
	if( offset == 0 ) {
		if (!(mem_mask & 0xff000000))
		{
			led_reg0 = (data >> 24) & 0xff;
		}
		if (!(mem_mask & 0x00ff0000))
		{
			led_reg1 = (data >> 16) & 0xff;
		}
		return;
	}
	if( offset == 1 )
	{
		if (!(mem_mask & 0xff000000))
		{
		}
		if (!(mem_mask & 0x000000ff))
		{
			if (data & 0x80)	/* CG Board 1 IRQ Ack */
			{
				cpunum_set_input_line(0, INPUT_LINE_IRQ1, CLEAR_LINE);
			}
			if (data & 0x40)	/* CG Board 0 IRQ Ack */
			{
				cpunum_set_input_line(0, INPUT_LINE_IRQ0, CLEAR_LINE);
			}
			set_cgboard_id((data >> 4) & 0x3);
		}
		return;
	}
}

static int comm_rombank = 0;

static WRITE32_HANDLER( comm1_w )
{
	printf("comm1_w: %08X, %08X, %08X\n", offset, data, mem_mask);
}

static WRITE32_HANDLER( comm_rombank_w )
{
	int bank = data >> 24;
	if (memory_region(REGION_USER3))
		if( bank != comm_rombank ) {
			printf("rombank %02X\n", bank);
			comm_rombank = bank & 0x7f;
			memory_set_bankptr(1, memory_region(REGION_USER3) + (comm_rombank * 0x10000));
		}
}

static READ32_HANDLER( comm0_unk_r )
{
//  printf("comm0_unk_r: %08X, %08X\n", offset, mem_mask);
	return 0xffffffff;
}

/*****************************************************************************/

static ADDRESS_MAP_START( hornet_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0x80000000) AM_RAM		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_MIRROR(0x80000000) AM_READWRITE(K037122_reg_r, K037122_reg_w)
	AM_RANGE(0x74020000, 0x7403ffff) AM_MIRROR(0x80000000) AM_READWRITE(K037122_sram_r, K037122_sram_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_MIRROR(0x80000000) AM_READWRITE(K037122_char_r, K037122_char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x780c0000, 0x780c0003) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7d000000, 0x7d00ffff) AM_MIRROR(0x80000000) AM_READ(sysreg_r)
	AM_RANGE(0x7d010000, 0x7d01ffff) AM_MIRROR(0x80000000) AM_WRITE(sysreg_w)
	AM_RANGE(0x7d020000, 0x7d021fff) AM_MIRROR(0x80000000) AM_READWRITE(timekeeper_0_32be_r, timekeeper_0_32be_w)	/* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7d030000, 0x7d030007) AM_MIRROR(0x80000000) AM_READWRITE(K056800_host_r, K056800_host_w)
	AM_RANGE(0x7d042000, 0x7d043fff) AM_MIRROR(0x80000000) AM_RAM				/* COMM BOARD 0 */
	AM_RANGE(0x7d044000, 0x7d044007) AM_MIRROR(0x80000000) AM_READ(comm0_unk_r)
	AM_RANGE(0x7d048000, 0x7d048003) AM_MIRROR(0x80000000) AM_WRITE(comm1_w)
	AM_RANGE(0x7d04a000, 0x7d04a003) AM_MIRROR(0x80000000) AM_WRITE(comm_rombank_w)
	AM_RANGE(0x7d050000, 0x7d05ffff) AM_MIRROR(0x80000000) AM_ROMBANK(1)		/* COMM BOARD 1 */
	AM_RANGE(0x7e000000, 0x7e7fffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER2, 0)		/* Data ROM */
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_MIRROR(0x80000000) AM_ROM AM_SHARE(2)
	AM_RANGE(0x7fc00000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER1, 0) AM_SHARE(2)	/* Program ROM */
ADDRESS_MAP_END

/*****************************************************************************/

static ADDRESS_MAP_START( sound_memmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM		/* Work RAM */
	AM_RANGE(0x200000, 0x200fff) AM_READWRITE(RF5C400_0_r, RF5C400_0_w)		/* Ricoh RF5C400 */
	AM_RANGE(0x300000, 0x30000f) AM_READWRITE(K056800_sound_r, K056800_sound_w)
	AM_RANGE(0x600000, 0x600001) AM_NOP
ADDRESS_MAP_END

/*****************************************************************************/

static UINT32 *sharc_dataram[2];

static READ32_HANDLER( dsp_dataram0_r )
{
	return sharc_dataram[0][offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram0_w )
{
	sharc_dataram[0][offset] = data;
}

static READ32_HANDLER( dsp_dataram1_r )
{
	return sharc_dataram[1][offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram1_w )
{
	sharc_dataram[1][offset] = data;
}

static ADDRESS_MAP_START( sharc0_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_READWRITE(voodoo_0_r, voodoo_0_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_READWRITE(K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK(5)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sharc1_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_1_shared_sharc_r, cgboard_1_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram1_r, dsp_dataram1_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_READWRITE(voodoo_1_r, voodoo_1_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_1_comm_sharc_r, cgboard_1_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_READWRITE(K033906_1_r, K033906_1_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK(6)
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( hornet )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip (H)" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Flip (V)" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Harness" )
	PORT_DIPSETTING( 0x04, "JVS" )
	PORT_DIPSETTING( 0x00, "JAMMA" )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" )
	PORT_DIPSETTING( 0x01, "24KHz" )
	PORT_DIPSETTING( 0x00, "15KHz" )

INPUT_PORTS_END

static INPUT_PORTS_START( sscope )
	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// Gun trigger
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip (H)" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Flip (V)" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Harness" )
	PORT_DIPSETTING( 0x04, "JVS" )
	PORT_DIPSETTING( 0x00, "JAMMA" )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" )
	PORT_DIPSETTING( 0x01, "24KHz" )
	PORT_DIPSETTING( 0x00, "15KHz" )

INPUT_PORTS_END

static const struct RF5C400interface rf5c400_interface =
{
	REGION_SOUND1
};

static const ppc_config hornet_ppc_cfg =
{
	PPC_MODEL_403GA
};

static sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};

/* PowerPC interrupts

    IRQ0:   Vblank CG Board 0
    IRQ1:   Vblank CG Board 1
    IRQ2:   LANC
    DMA0
    NMI:    SCI

*/

static MACHINE_RESET( hornet )
{
	if (memory_region(REGION_USER3))
		memory_set_bankptr(1, memory_region(REGION_USER3));
	cpunum_set_input_line(2, INPUT_LINE_RESET, ASSERT_LINE);

	if (memory_region(REGION_USER5))
		memory_set_bankptr(5, memory_region(REGION_USER5));
}

static MACHINE_DRIVER_START( hornet )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", PPC403, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_CONFIG(hornet_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(hornet_map, 0)

	MDRV_CPU_ADD(M68000, 64000000/4)	/* 16MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc0_map, 0)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_INTERLEAVE(100)

	MDRV_MACHINE_RESET( hornet )

	MDRV_NVRAM_HANDLER( timekeeper_0 )

 	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 48*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)
	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(hornet)
	MDRV_VIDEO_UPDATE(hornet)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(RF5C400, 64000000/4)
	MDRV_SOUND_CONFIG(rf5c400_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

MACHINE_DRIVER_END

static MACHINE_RESET( hornet_2board )
{
	if (memory_region(REGION_USER3))
		memory_set_bankptr(1, memory_region(REGION_USER3));
	cpunum_set_input_line(2, INPUT_LINE_RESET, ASSERT_LINE);
	cpunum_set_input_line(3, INPUT_LINE_RESET, ASSERT_LINE);

	if (memory_region(REGION_USER5))
		memory_set_bankptr(5, memory_region(REGION_USER5));
}

static MACHINE_DRIVER_START( hornet_2board )

	MDRV_IMPORT_FROM(hornet)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc1_map, 0)

	MDRV_MACHINE_RESET(hornet_2board)

	MDRV_VIDEO_START(hornet_2board)
	MDRV_VIDEO_UPDATE(hornet_2board)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_PALETTE_LENGTH(65536)

	MDRV_SCREEN_ADD("left", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_SCREEN_ADD("right", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
MACHINE_DRIVER_END


/*****************************************************************************/

static UINT8 jvs_rdata[1024];
static UINT8 jvs_sdata[1024];

static int jvs_rdata_ptr = 0;
static int jvs_sdata_ptr = 0;

static UINT8 jamma_jvs_r(void)
{
	UINT8 r;
	r = jvs_rdata[jvs_rdata_ptr];
	jvs_rdata_ptr++;

	return r;
}

static void jamma_jvs_w(UINT8 data)
{
	jvs_sdata[jvs_sdata_ptr] = data;
	jvs_sdata_ptr++;
}

static int jvs_encode_data(UINT8 *in, UINT8 *out, int length)
{
	int outptr = 0;
	int inptr = 0;

	while (inptr < length)
	{
		UINT8 b = in[inptr++];
		if (b == 0xe0)
		{
			out[outptr++] = 0xd0;
			out[outptr++] = 0xdf;
		}
		else if (b == 0xd0)
		{
			out[outptr++] = 0xd0;
			out[outptr++] = 0xcf;
		}
		else
		{
			out[outptr++] = b;
		}
	};

	return outptr;
}

static int jvs_decode_data(UINT8 *in, UINT8 *out, int length)
{
	int outptr = 0;
	int inptr = 0;

	while (inptr < length)
	{
		UINT8 b = in[inptr++];
		if (b == 0xd0)
		{
			UINT8 b2 = in[inptr++];
			out[outptr++] = b2 + 1;
		}
		else
		{
			out[outptr++] = b;
		}
	};

	return outptr;
}

static void jamma_jvs_cmd_exec(void)
{
	UINT8 sync, node, byte_num;
	UINT8 data[1024], rdata[1024];
	int i, length;
	int rdata_ptr;
	int sum;

	sync = jvs_sdata[0];
	node = jvs_sdata[1];
	byte_num = jvs_sdata[2];

	if (sync != 0xe0)
	{
		printf("jamma_jvs_cmd_exec: SYNC byte not found! (%02X)\n", sync);
		return;
	}

	length = jvs_decode_data(&jvs_sdata[3], data, byte_num-1);

	/*
    printf("jvs input data:\n");
    for (i=0; i < byte_num; i++)
    {
        printf("%02X ", jvs_sdata[3+i]);
    }
    printf("\n");

    printf("jvs data decoded to:\n");
    for (i=0; i < length; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n\n");
    */

	// clear return data
	memset(rdata, 0, sizeof(rdata));
	rdata_ptr = 0;

	// status
	rdata[rdata_ptr++] = 0x01;		// normal

	// handle the command
	switch (data[0])
	{
		case 0xf0:		// Reset
		{
			break;
		}
		case 0xf1:		// Address setting
		{
			rdata[rdata_ptr++] = 0x01;		// report data (normal)
			break;
		}
		case 0xfa:
		{
			break;
		}
		default:
		{
			fatalerror("jamma_jvs_cmd_exec: unknown command %02X\n", data[0]);
		}
	}

	// write jvs return data
	jvs_rdata[0] = 0xe0;			// sync
	jvs_rdata[1] = 0x00;			// node
	jvs_rdata[2] = rdata_ptr+1;		// num of bytes

	length = jvs_encode_data(rdata, &jvs_rdata[3], rdata_ptr);

	// calculate sum
	sum = 0;
	for (i=0; i < length+2; i++)
	{
		sum += jvs_rdata[1+i];
	}

	// write sum
	jvs_rdata[3+length] = (UINT8)(sum-1);

	jvs_rdata_ptr = 0;
	jvs_sdata_ptr = 0;
}

/*****************************************************************************/


static UINT8 jamma_rdata[1024];
static void jamma_r(int length)
{
	int i;
//  printf("jamma_r %d\n", length);
	for (i=0; i < length; i++)
	{
		jamma_rdata[i] = jamma_jvs_r();
	}
}

static UINT8 jamma_wdata[1024];
static void jamma_w(int length)
{
	int i;
//  printf("jamma_w %d\n", length);
	for (i=0; i < length; i++)
	{
		jamma_jvs_w(jamma_wdata[i]);
	}

	jamma_jvs_cmd_exec();
}

static void sound_irq_callback(int irq)
{
	if (irq == 0)
	{
		cpunum_set_input_line(1, INPUT_LINE_IRQ1, PULSE_LINE);
	}
	else
	{
		cpunum_set_input_line(1, INPUT_LINE_IRQ2, PULSE_LINE);
	}
}

static UINT8 backup_ram[0x2000];
static void init_hornet(running_machine *machine)
{
	init_konami_cgboard(1, CGBOARD_TYPE_HORNET);
	set_cgboard_texture_bank(0, 5, memory_region(REGION_USER5));

	sharc_dataram[0] = auto_malloc(0x100000);

	K056800_init(sound_irq_callback);
	K033906_init();

	timekeeper_init(0, TIMEKEEPER_M48T58, backup_ram);

	ppc403_install_spu_tx_dma_handler(jamma_w, jamma_wdata);
	ppc403_install_spu_rx_dma_handler(jamma_r, jamma_rdata);
}

static void init_hornet_2board(running_machine *machine)
{
	init_konami_cgboard(2, CGBOARD_TYPE_HORNET);
	set_cgboard_texture_bank(0, 5, memory_region(REGION_USER5));
	set_cgboard_texture_bank(1, 6, memory_region(REGION_USER5));

	sharc_dataram[0] = auto_malloc(0x100000);
	sharc_dataram[1] = auto_malloc(0x100000);

	K056800_init(sound_irq_callback);
	K033906_init();

	timekeeper_init(0, TIMEKEEPER_M48T58, backup_ram);

	ppc403_install_spu_tx_dma_handler(jamma_w, jamma_wdata);
	ppc403_install_spu_rx_dma_handler(jamma_r, jamma_rdata);
}

static DRIVER_INIT(gradius4)
{
	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x58;	// 'X'
	backup_ram[0x02] = 0x38;	// '8'
	backup_ram[0x03] = 0x33;	// '3'
	backup_ram[0x04] = 0x37;	// '7'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x11;	//
	backup_ram[0x07] = 0x06;	// 06 / 11
	backup_ram[0x08] = 0x19;	//
	backup_ram[0x09] = 0x98;	// 1998
	backup_ram[0x0a] = 0x4a;	// 'J'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x43;	// 'C'
	backup_ram[0x0d] = 0x00;	//
	backup_ram[0x0e] = 0x02;	// checksum
	backup_ram[0x0f] = 0xd7;	// checksum

	voodoo_version = 0;
	init_hornet(machine);
}

static DRIVER_INIT(nbapbp)
{
	int i;
	UINT16 checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x58;	// 'X'
	backup_ram[0x02] = 0x37;	// '7'
	backup_ram[0x03] = 0x37;	// '7'
	backup_ram[0x04] = 0x38;	// '8'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	//
	backup_ram[0x07] = 0x00;	//
	backup_ram[0x08] = 0x19;	//
	backup_ram[0x09] = 0x98;	// 1998
	backup_ram[0x0a] = 0x4a;	// 'J'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x41;	// 'A'
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
	for (i=0; i < 14; i++)
	{
		checksum += backup_ram[i];
		checksum &= 0xffff;
	}
	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum

	voodoo_version = 0;
	init_hornet(machine);
}

static DRIVER_INIT(terabrst)
{
	int i;
	UINT16 checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x4e;	// 'N'
	backup_ram[0x02] = 0x37;	// '7'
	backup_ram[0x03] = 0x31;	// '1'
	backup_ram[0x04] = 0x35;	// '5'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	//
	backup_ram[0x07] = 0x00;	//
	backup_ram[0x08] = 0x19;	//
	backup_ram[0x09] = 0x98;	// 1998
	backup_ram[0x0a] = 0x41;	// 'J'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x45;	// 'E'
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
	for (i=0; i < 14; i+=2)
	{
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
	}
	checksum = ~checksum - 0;
	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum

	voodoo_version = 0;
	init_hornet_2board(machine);
}

static DRIVER_INIT(sscope)
{
	int i;
	UINT16 checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x51;	// 'Q'
	backup_ram[0x02] = 0x38;	// '8'
	backup_ram[0x03] = 0x33;	// '3'
	backup_ram[0x04] = 0x30;	// '0'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	//
	backup_ram[0x07] = 0x00;	//
	backup_ram[0x08] = 0x20;	//
	backup_ram[0x09] = 0x00;	// 2000
	backup_ram[0x0a] = 0x55;	// 'U'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x41;	// 'A'
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
	for (i=0; i < 14; i+=2)
	{
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
	}
	checksum = ~checksum - 1;
	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum

	voodoo_version = 0;
	init_hornet_2board(machine);
}

static DRIVER_INIT(sscope2)
{
	int i;
	int checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x4b;	// 'K'
	backup_ram[0x02] = 0x39;	// '9'
	backup_ram[0x03] = 0x33;	// '3'
	backup_ram[0x04] = 0x31;	// '1'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	//
	backup_ram[0x07] = 0x00;	//
	backup_ram[0x08] = 0x20;	//
	backup_ram[0x09] = 0x00;	// 2000
	backup_ram[0x0a] = 0x55;	// 'U'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x41;	// 'A'
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
	for (i=0; i < 14; i+=2)
	{
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
		checksum &= 0xffff;
	}
	checksum = (-1 - checksum) - 1;
	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum


	/* Silent Scope data */
	backup_ram[0x1f40] = 0x47;	// 'G'
	backup_ram[0x1f41] = 0x4b;	// 'Q'
	backup_ram[0x1f42] = 0x38;	// '8'
	backup_ram[0x1f43] = 0x33;	// '3'
	backup_ram[0x1f44] = 0x30;	// '0'
	backup_ram[0x1f45] = 0x00;	//
	backup_ram[0x1f46] = 0x00;	//
	backup_ram[0x1f47] = 0x00;	//
	backup_ram[0x1f48] = 0x20;	//
	backup_ram[0x1f49] = 0x00;	// 2000
	backup_ram[0x1f4a] = 0x55;	// 'U'
	backup_ram[0x1f4b] = 0x41;	// 'A'
	backup_ram[0x1f4c] = 0x41;	// 'A'
	backup_ram[0x1f4d] = 0x00;	//

	checksum = 0;
	for (i=0x1f40; i < 0x1f4e; i+=2)
	{
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
		checksum &= 0xffff;
	}
	checksum = (-1 - checksum) - 1;
	backup_ram[0x1f4e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x1f4f] = (checksum >> 0) & 0xff;	// checksum

	voodoo_version = 1;
	init_hornet_2board(machine);
}

/*****************************************************************************/

ROM_START(sscope)
	ROM_REGION32_BE(0x400000, REGION_USER1, 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("ss1-1.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14))
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, REGION_USER2, ROMREGION_ERASE00)	/* Data roms */

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("ss1-1.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1))

	ROM_REGION(0x1000000, REGION_USER5, 0)		/* CG Board texture roms */
    	ROM_LOAD32_WORD( "ss1-3.u32",    0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
    	ROM_LOAD32_WORD( "ss1-3.u24",    0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0)		/* PCM sample roms */
        ROM_LOAD( "830a09.16p",    0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
        ROM_LOAD( "830a10.14p",    0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )
ROM_END

ROM_START(sscopea)
	ROM_REGION32_BE(0x400000, REGION_USER1, 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830_a01.bin", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37))
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, REGION_USER2, ROMREGION_ERASE00)	/* Data roms */

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("ss1-1.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1))

	ROM_REGION(0x1000000, REGION_USER5, 0)		/* CG Board texture roms */
    	ROM_LOAD32_WORD( "ss1-3.u32",    0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
    	ROM_LOAD32_WORD( "ss1-3.u24",    0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0)		/* PCM sample roms */
        ROM_LOAD( "830a09.16p",    0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
        ROM_LOAD( "830a10.14p",    0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )
ROM_END

ROM_START(sscope2)
	ROM_REGION32_BE(0x400000, REGION_USER1, 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("931d01.bin", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2))
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)	/* Data roms */
		ROM_LOAD32_WORD_SWAP("931a04.bin", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814))

	ROM_REGION32_BE(0x800000, REGION_USER3, 0)	/* Comm board roms */
	ROM_LOAD("931a19.bin", 0x000000, 0x400000, BAD_DUMP CRC(8e8bb6af) SHA1(1bb399f7897fbcbe6852fda3215052b2810437d8))
	ROM_LOAD("931a20.bin", 0x400000, 0x400000, BAD_DUMP CRC(a14a7887) SHA1(daf0cbaf83e59680a0d3c4d66fcc48d02c9723d1))

	ROM_REGION(0x800000, REGION_USER5, ROMREGION_ERASE00)	/* CG Board texture roms */

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("931a08.bin", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c))

	ROM_REGION(0xc00000, REGION_SOUND1, 0)		/* PCM sample roms */
        ROM_LOAD( "931a09.bin",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
        ROM_LOAD( "931a10.bin",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
        ROM_LOAD( "931a11.bin",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )
ROM_END

ROM_START(gradius4)
	ROM_REGION32_BE(0x400000, REGION_USER1, 0)	/* PowerPC program */
        ROM_LOAD16_WORD_SWAP( "837c01.27p",   0x200000, 0x200000, CRC(ce003123) SHA1(15e33997be2c1b3f71998627c540db378680a7a1) )
        ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)	/* Data roms */
        ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
        ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, REGION_USER5, 0)	/* CG Board texture roms */
        ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
        ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
        ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
        ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68K Program */
        ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION(0x800000, REGION_SOUND1, 0)		/* PCM sample roms */
        ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
        ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )
ROM_END

ROM_START(nbapbp)
	ROM_REGION32_BE(0x400000, REGION_USER1, 0)	/* PowerPC program */
        ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
        ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)	/* Data roms */
        ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
        ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, REGION_USER5, 0)	/* CG Board texture roms */
        ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
        ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
        ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
        ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68K Program */
        ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0)		/* PCM sample roms */
        ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
        ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
        ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
        ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )
ROM_END

ROM_START(terabrst)
	ROM_REGION32_BE(0x400000, REGION_USER1, 0)	/* PowerPC program */
        ROM_LOAD32_WORD_SWAP( "715a02.25p",   0x000000, 0x200000, CRC(070c48b3) SHA1(066cefbd34d8f6476083417471114f782bef97fb) )
        ROM_LOAD32_WORD_SWAP( "715a03.22p",   0x000002, 0x200000, CRC(f77d242f) SHA1(7680e4abcccd549b3f6d1d245f64631fab57e80d) )

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)	/* Data roms */
        ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
        ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, REGION_USER5, 0)	/* CG Board texture roms */
        ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
        ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68K Program */
        ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0)		/* PCM sample roms */
        ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
        ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, REGION_CPU3, 0)		/* 68K Program */
        ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )
ROM_END

/*************************************************************************/

GAME( 1998, gradius4,	0,		hornet,			hornet,	gradius4,	ROT0,	"Konami",	"Gradius 4: Fukkatsu", GAME_IMPERFECT_SOUND )
GAME( 1998, nbapbp,		0,		hornet,			hornet,	nbapbp,		ROT0,	"Konami",	"NBA Play By Play", GAME_IMPERFECT_SOUND )
GAMEL( 1998, terabrst,   0,     hornet_2board,  hornet, terabrst,   ROT0,   "Konami",   "Teraburst", GAME_IMPERFECT_SOUND, layout_dualhsxs )
GAMEL( 2000, sscope,	0,		hornet_2board,	sscope,	sscope,		ROT0,	"Konami",	"Silent Scope (ver UAB)", GAME_IMPERFECT_SOUND|GAME_NOT_WORKING, layout_dualhsxs )
GAMEL( 2000, sscopea,	sscope, hornet_2board,	sscope,	sscope,		ROT0,	"Konami",	"Silent Scope (ver UAA)", GAME_IMPERFECT_SOUND|GAME_NOT_WORKING, layout_dualhsxs )
GAMEL( 2000, sscope2,	0,		hornet_2board,	sscope,	sscope2,	ROT0,	"Konami",	"Silent Scope 2", GAME_IMPERFECT_SOUND|GAME_NOT_WORKING, layout_dualhsxs )
