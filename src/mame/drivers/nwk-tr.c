/*  Konami NWK-TR System

    Driver by Ville Linde



    Hardware overview:

    GN676 CPU Board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Ricoh RF5c400 sound chip
        National Semiconductor ADC12138

    GN676 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami K001604 (2D tilemaps + 2x ROZ)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0003-03 (Voodoo) FBI with 2MB RAM
        2x 3DFX 500-0004-02 (Voodoo) TMU with 2MB RAM

    GN676 LAN Board:
    ----------------
        Xilinx XC5210 FPGA
        Xilinx XC5204 FPGA

*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "machine/konamiic.h"
#include "machine/adc1213x.h"
#include "sound/rf5c400.h"
#include "video/voodoo.h"
#include "machine/timekpr.h"

static UINT8 led_reg0 = 0x7f, led_reg1 = 0x7f;

static UINT32 *work_ram;
static UINT8 backup_ram[0x2000];


static WRITE32_HANDLER( paletteram32_w )
{
	COMBINE_DATA(&paletteram32[offset]);
	data = paletteram32[offset];
	palette_set_color_rgb(Machine, offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}



/* K001604 Tilemap chip (move to konamiic.c ?) */

#define MAX_K001604_CHIPS	2

static UINT32 *K001604_tile_ram[MAX_K001604_CHIPS];
static UINT32 *K001604_char_ram[MAX_K001604_CHIPS];
static UINT8 *K001604_dirty_map[MAX_K001604_CHIPS][2];
static int K001604_gfx_index[MAX_K001604_CHIPS][2], K001604_char_dirty[MAX_K001604_CHIPS][2];
static tilemap *K001604_layer_8x8[MAX_K001604_CHIPS][2];
static int K001604_tilemap_offset;
static tilemap *K001604_layer_roz[MAX_K001604_CHIPS][2];
static int K001604_roz_size[MAX_K001604_CHIPS];

static UINT32 K001604_reg[MAX_K001604_CHIPS][256];

static int K001604_layer_size;

#define K001604_NUM_TILES_LAYER0		16384
#define K001604_NUM_TILES_LAYER1		4096

static const gfx_layout K001604_char_layout_layer_8x8 =
{
	8, 8,
	K001604_NUM_TILES_LAYER0,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
};

static const gfx_layout K001604_char_layout_layer_16x16 =
{
	16, 16,
	K001604_NUM_TILES_LAYER1,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16, 9*16, 8*16, 11*16, 10*16, 13*16, 12*16, 15*16, 14*16 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256 },
	16*256
};


static TILEMAP_MAPPER( K001604_scan_layer_8x8_0 )
{
	/* logical (col,row) -> memory offset */
	int width = K001604_layer_size ? 256 : 128;
	return K001604_tilemap_offset + (row * width) + col;
}

static TILEMAP_MAPPER( K001604_scan_layer_8x8_1 )
{
	/* logical (col,row) -> memory offset */
	int width = K001604_layer_size ? 256 : 128;
	return K001604_tilemap_offset + (row * width) + col + 64;
}

static TILEMAP_MAPPER( K001604_scan_layer_roz_0 )
{
	/* logical (col,row) -> memory offset */
	int width = K001604_layer_size ? 256 : 128;
	return (row * width) + col + (K001604_layer_size ? 128 : 0);
}

static TILEMAP_MAPPER( K001604_scan_layer_roz_1 )
{
	/* logical (col,row) -> memory offset */
	int width = K001604_layer_size ? 256 : 128;
	return (row * width) + col + (K001604_layer_size ? 192 : 64);
}

static TILE_GET_INFO( K001604_0_tile_info_layer_8x8 )
{
	UINT32 val = K001604_tile_ram[0][tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7fff);
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(K001604_gfx_index[0][0], tile, color, flags);
}

static TILE_GET_INFO( K001604_0_tile_info_layer_roz )
{
	UINT32 val = K001604_tile_ram[0][tile_index];
	int flags = 0;
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x7ff;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tile += K001604_roz_size[0] ? 0x800 : 0x2000;

	SET_TILE_INFO(K001604_gfx_index[0][K001604_roz_size[0]], tile, color, flags);
}

static TILE_GET_INFO( K001604_1_tile_info_layer_8x8 )
{
	UINT32 val = K001604_tile_ram[1][tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7fff);
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(K001604_gfx_index[1][0], tile, color, flags);
}

static TILE_GET_INFO( K001604_1_tile_info_layer_roz )
{
	UINT32 val = K001604_tile_ram[1][tile_index];
	int flags = 0;
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7ff);

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tile += K001604_roz_size[1] ? 0x800 : 0x2000;

	SET_TILE_INFO(K001604_gfx_index[1][K001604_roz_size[1]], tile, color, flags);
}

int K001604_vh_start(running_machine *machine, int chip)
{
	const char *gamename = machine->gamedrv->name;

	/* HACK !!! To be removed */
	if (mame_stricmp(gamename, "racingj") == 0 || mame_stricmp(gamename, "racingj2") == 0
		|| mame_stricmp(gamename, "hangplt") == 0 || mame_stricmp(gamename, "slrasslt") == 0
		|| mame_stricmp(gamename, "jetwave") == 0)
	{
		K001604_layer_size = 0;		// width = 128 tiles
	}
	else
	{
		K001604_layer_size = 1;		// width = 256 tiles
	}

	/* HACK !!! To be removed */
	if (mame_stricmp(gamename, "slrasslt") == 0)
	{
		K001604_tilemap_offset = 16384;

		K001604_roz_size[chip] = 0;		// 8x8
	}
	else
	{
		K001604_tilemap_offset = 0;

		K001604_roz_size[chip] = 1;		// 16x16
	}

	for (K001604_gfx_index[chip][0] = 0; K001604_gfx_index[chip][0] < MAX_GFX_ELEMENTS; K001604_gfx_index[chip][0]++)
		if (machine->gfx[K001604_gfx_index[chip][0]] == 0)
			break;
	if (K001604_gfx_index[chip][0] == MAX_GFX_ELEMENTS)
	{
		return 1;
	}

	for (K001604_gfx_index[chip][1] = K001604_gfx_index[chip][0] + 1; K001604_gfx_index[chip][1] < MAX_GFX_ELEMENTS; K001604_gfx_index[chip][1]++)
		if (machine->gfx[K001604_gfx_index[chip][1]] == 0)
			break;
	if (K001604_gfx_index[chip][1] == MAX_GFX_ELEMENTS)
	{
		return 1;
	}

	K001604_char_ram[chip] = auto_malloc(0x200000);

	K001604_tile_ram[chip] = auto_malloc(0x20000);

	K001604_dirty_map[chip][0] = auto_malloc(K001604_NUM_TILES_LAYER0);
	K001604_dirty_map[chip][1] = auto_malloc(K001604_NUM_TILES_LAYER1);


	if (chip == 0)
	{
		int roz_tile_size = K001604_roz_size[chip] ? 16 : 8;
		int roz_width = K001604_layer_size ? 64 : 128;
		K001604_layer_8x8[chip][0] = tilemap_create(K001604_0_tile_info_layer_8x8, K001604_scan_layer_8x8_0, 8, 8, 64, 64);
		K001604_layer_8x8[chip][1] = tilemap_create(K001604_0_tile_info_layer_8x8, K001604_scan_layer_8x8_1, 8, 8, 64, 64);

		K001604_layer_roz[chip][0] = tilemap_create(K001604_0_tile_info_layer_roz, K001604_scan_layer_roz_0, roz_tile_size, roz_tile_size, roz_width, 64);
		K001604_layer_roz[chip][1] = tilemap_create(K001604_0_tile_info_layer_roz, K001604_scan_layer_roz_1, roz_tile_size, roz_tile_size, 64, 64);
	}
	else
	{
		int roz_tile_size = K001604_roz_size[chip] ? 16 : 8;
		int roz_width = K001604_layer_size ? 64 : 128;
		K001604_layer_8x8[chip][0] = tilemap_create(K001604_1_tile_info_layer_8x8, K001604_scan_layer_8x8_0, 8, 8, 64, 64);
		K001604_layer_8x8[chip][1] = tilemap_create(K001604_1_tile_info_layer_8x8, K001604_scan_layer_8x8_1, 8, 8, 64, 64);

		K001604_layer_roz[chip][0] = tilemap_create(K001604_1_tile_info_layer_roz, K001604_scan_layer_roz_0, roz_tile_size, roz_tile_size, roz_width, 64);
		K001604_layer_roz[chip][1] = tilemap_create(K001604_1_tile_info_layer_roz, K001604_scan_layer_roz_1, roz_tile_size, roz_tile_size, 64, 64);
	}

	tilemap_set_transparent_pen(K001604_layer_8x8[chip][0], 0);
	tilemap_set_transparent_pen(K001604_layer_8x8[chip][1], 0);

	memset(K001604_char_ram[chip], 0, 0x200000);
	memset(K001604_tile_ram[chip], 0, 0x10000);
	memset(K001604_dirty_map[chip][0], 0, K001604_NUM_TILES_LAYER0);
	memset(K001604_dirty_map[chip][1], 0, K001604_NUM_TILES_LAYER1);


	machine->gfx[K001604_gfx_index[chip][0]] = allocgfx(&K001604_char_layout_layer_8x8);
	decodegfx(machine->gfx[K001604_gfx_index[chip][0]], (UINT8*)&K001604_char_ram[chip][0], 0, machine->gfx[K001604_gfx_index[chip][0]]->total_elements);
	machine->gfx[K001604_gfx_index[chip][1]] = allocgfx(&K001604_char_layout_layer_16x16);
	decodegfx(machine->gfx[K001604_gfx_index[chip][1]], (UINT8*)&K001604_char_ram[chip][0], 0, machine->gfx[K001604_gfx_index[chip][1]]->total_elements);

	machine->gfx[K001604_gfx_index[chip][0]]->total_colors = machine->config->total_colors / 16;
	machine->gfx[K001604_gfx_index[chip][1]]->total_colors = machine->config->total_colors / 16;

	return 0;
}

void K001604_tile_update(running_machine *machine, int chip)
{
	if(K001604_char_dirty[chip][0])
	{
		int i;
		for (i=0; i < K001604_NUM_TILES_LAYER0; i++)
		{
			if(K001604_dirty_map[chip][0][i])
			{
				K001604_dirty_map[chip][0][i] = 0;
				decodechar(machine->gfx[K001604_gfx_index[chip][0]], i, (UINT8*)&K001604_char_ram[chip][0]);
			}
		}
		tilemap_mark_all_tiles_dirty(K001604_layer_8x8[chip][0]);
		tilemap_mark_all_tiles_dirty(K001604_layer_8x8[chip][1]);

		if (K001604_roz_size[chip] == 0)
		{
			tilemap_mark_all_tiles_dirty(K001604_layer_roz[chip][0]);
			tilemap_mark_all_tiles_dirty(K001604_layer_roz[chip][1]);
		}
		K001604_char_dirty[chip][0] = 0;
	}
	if(K001604_char_dirty[chip][1])
	{
		int i;
		for (i=0; i < K001604_NUM_TILES_LAYER1; i++)
		{
			if(K001604_dirty_map[chip][1][i])
			{
				K001604_dirty_map[chip][1][i] = 0;
				decodechar(machine->gfx[K001604_gfx_index[chip][1]], i, (UINT8*)&K001604_char_ram[chip][0]);
			}
		}

		if (K001604_roz_size[chip] == 1)
		{
			tilemap_mark_all_tiles_dirty(K001604_layer_roz[chip][0]);
			tilemap_mark_all_tiles_dirty(K001604_layer_roz[chip][1]);
		}
		K001604_char_dirty[chip][1] = 0;
	}
}

void K001604_draw_back_layer(int chip, bitmap_t *bitmap, const rectangle *cliprect)
{
	int layer;
	int num_layers;
	fillbitmap(bitmap, 0, cliprect);

	num_layers = K001604_layer_size ? 2 : 1;

	for (layer=0; layer < num_layers; layer++)
	{
		int reg = 0x08;

		INT32 x  = (INT16)((K001604_reg[chip][reg+0] >> 16) & 0xffff);
		INT32 y  = (INT16)((K001604_reg[chip][reg+0] >>  0) & 0xffff);
		INT32 xx = (INT16)((K001604_reg[chip][reg+1] >>  0) & 0xffff);
		INT32 xy = (INT16)((K001604_reg[chip][reg+1] >> 16) & 0xffff);
		INT32 yx = (INT16)((K001604_reg[chip][reg+2] >>  0) & 0xffff);
		INT32 yy = (INT16)((K001604_reg[chip][reg+2] >> 16) & 0xffff);

		x  = (x + 320) * 256;
		y  = (y + 208) * 256;
		xx = (xx);
		xy = (-xy);
		yx = (-yx);
		yy = (yy);

		if ((K001604_reg[chip][0x6c/4] & (0x08 >> layer)) != 0)
		{
			tilemap_draw_roz(bitmap, cliprect, K001604_layer_roz[chip][layer],
							 x << 5, y << 5, xx << 5, xy << 5, yx << 5, yy << 5, 1, 0, 0);
		}
	}
}

void K001604_draw_front_layer(int chip, bitmap_t *bitmap, const rectangle *cliprect)
{
	//tilemap_draw(bitmap, cliprect, K001604_layer_8x8[chip][1], 0,0);
	tilemap_draw(bitmap, cliprect, K001604_layer_8x8[chip][0], 0,0);
}

READ32_HANDLER(K001604_tile_r)
{
	int chip = get_cgboard_id();

	return K001604_tile_ram[chip][offset];
}

READ32_HANDLER(K001604_char_r)
{
	int chip = get_cgboard_id();

	int set, bank;
	UINT32 addr;

	set = (K001604_reg[chip][0x60/4] & 0x1000000) ? 0x100000 : 0;

	if (set)
	{
		bank = (K001604_reg[chip][0x60/4] >> 8) & 0x3;
	}
	else
	{
		bank = (K001604_reg[chip][0x60/4] & 0x3);
	}

	addr = offset + ((set + (bank * 0x40000)) / 4);

	return K001604_char_ram[chip][addr];
}

WRITE32_HANDLER(K001604_tile_w)
{
	int chip = get_cgboard_id();

	int x, y;
	COMBINE_DATA(K001604_tile_ram[chip] + offset);

	if (K001604_layer_size)
	{
		x = offset & 0xff;
		y = offset / 256;
	}
	else
	{
		x = offset & 0x7f;
		y = offset / 128;
	}

	if (K001604_layer_size)
	{
		if (x < 64)
		{
			tilemap_mark_tile_dirty(K001604_layer_8x8[chip][0], offset);
		}
		else if (x < 128)
		{
			tilemap_mark_tile_dirty(K001604_layer_8x8[chip][1], offset);
		}
		else if (x < 192)
		{
			tilemap_mark_tile_dirty(K001604_layer_roz[chip][0], offset);
		}
		else
		{
			tilemap_mark_tile_dirty(K001604_layer_roz[chip][1], offset);
		}
	}
	else
	{
		if (x < 64)
		{
			tilemap_mark_tile_dirty(K001604_layer_8x8[chip][0], offset);

			tilemap_mark_tile_dirty(K001604_layer_roz[chip][0], offset);
		}
		else
		{
			tilemap_mark_tile_dirty(K001604_layer_8x8[chip][1], offset);

			tilemap_mark_tile_dirty(K001604_layer_roz[chip][1], offset);
		}
	}
}

WRITE32_HANDLER(K001604_char_w)
{
	int chip = get_cgboard_id();

	int set, bank;
	UINT32 addr;

	set = (K001604_reg[chip][0x60/4] & 0x1000000) ? 0x100000 : 0;

	if (set)
	{
		bank = (K001604_reg[chip][0x60/4] >> 8) & 0x3;
	}
	else
	{
		bank = (K001604_reg[chip][0x60/4] & 0x3);
	}

	addr = offset + ((set + (bank * 0x40000)) / 4);

	COMBINE_DATA(K001604_char_ram[chip] + addr);

	K001604_dirty_map[chip][0][addr / 32] = 1;
	K001604_char_dirty[chip][0] = 1;

	K001604_dirty_map[chip][1][addr / 128] = 1;
	K001604_char_dirty[chip][1] = 1;
}



WRITE32_HANDLER(K001604_reg_w)
{
	int chip = get_cgboard_id();

	COMBINE_DATA(K001604_reg[chip] + offset);

	switch (offset)
	{
		case 0x8:
		case 0x9:
		case 0xa:
			//printf("K001604_reg_w %02X, %08X, %08X\n", offset, data, mem_mask);
			break;
	}

	if (offset != 0x08 && offset != 0x09 && offset != 0x0a /*&& offset != 0x17 && offset != 0x18*/)
	{
		//printf("K001604_reg_w (%d), %02X, %08X, %08X at %08X\n", chip, offset, data, mem_mask, activecpu_get_pc());
	}
}

READ32_HANDLER(K001604_reg_r)
{
	int chip = get_cgboard_id();

	switch (offset)
	{
		case 0x54/4:	return mame_rand(Machine) << 16; break;
		case 0x5c/4:	return mame_rand(Machine) << 16 | mame_rand(Machine); break;
	}

	return K001604_reg[chip][offset];
}




static void voodoo_vblank_0(int param)
{
	cpunum_set_input_line(Machine, 0, INPUT_LINE_IRQ0, ASSERT_LINE);
}

static VIDEO_START( nwktr )
{
	voodoo_start(0, machine->primary_screen, VOODOO_1, 2, 2, 2);
	voodoo_set_vblank_callback(0, voodoo_vblank_0);

	K001604_vh_start(machine, 0);
}


static VIDEO_UPDATE( nwktr )
{
	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	voodoo_update(0, bitmap, cliprect);

	K001604_tile_update(screen->machine, 0);
	K001604_draw_front_layer(0, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);
	return 0;
}

/*****************************************************************************/

static double adc12138_input_callback(int input)
{
	int value = 0;
	switch (input)
	{
		case 0:		value = readinputport(4) - 0x800; break;
		case 1:		value = readinputport(5); break;
		case 2:		value = readinputport(6); break;
		case 3:		value = readinputport(7); break;
		case 4:		value = readinputport(8); break;
	}

	return (double)(value) / 2047.0;
}

static READ32_HANDLER( sysreg_r )
{
	UINT32 r = 0;
	if (offset == 0)
	{
		if (!(mem_mask & 0xff000000))
		{
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
			r |= (adc1213x_do_r(0)) | (adc1213x_eoc_r(0) << 2);
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
	if( offset == 0 )
	{
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
			int cs = (data >> 27) & 0x1;
			int conv = (data >> 26) & 0x1;
			int di = (data >> 25) & 0x1;
			int sclk = (data >> 24) & 0x1;

			adc1213x_cs_w(0, cs);
			adc1213x_conv_w(0, conv);
			adc1213x_di_w(0, di);
			adc1213x_sclk_w(0, sclk);
		}
		if (!(mem_mask & 0x000000ff))
		{
			if (data & 0x80)	// CG Board 1 IRQ Ack
			{
				//cpunum_set_input_line(Machine, 0, INPUT_LINE_IRQ1, CLEAR_LINE);
			}
			if (data & 0x40)	// CG Board 0 IRQ Ack
			{
				//cpunum_set_input_line(Machine, 0, INPUT_LINE_IRQ0, CLEAR_LINE);
			}
		}
		return;
	}
}

static int fpga_uploaded = 0;
static int lanc2_ram_r = 0;
static int lanc2_ram_w = 0;
static UINT8 lanc2_ram[0x8000];

static READ32_HANDLER( lanc1_r )
{
	switch (offset)
	{
		case 0x40/4:
		{
			UINT32 r = 0;

			r |= (fpga_uploaded) ? (1 << 6) : 0;
			r |= 1 << 5;

			return (r) << 24;
		}

		default:
		{
			//printf("lanc1_r: %08X, %08X at %08X\n", offset, mem_mask, activecpu_get_pc());
			return 0xffffffff;
		}
	}
}

static WRITE32_HANDLER( lanc1_w )
{
	//printf("lanc1_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, activecpu_get_pc());
}

static READ32_HANDLER( lanc2_r )
{
	UINT32 r = 0;

	if (offset == 0)
	{
		if (!(mem_mask & 0x000000ff))
		{
			r |= lanc2_ram[lanc2_ram_r & 0x7fff];
			lanc2_ram_r++;
		}
		else
		{
			r |= 0xffffff00;
		}
	}

	if (offset == 4)
	{
		if (!(mem_mask & 0xff000000))
		{
			r |= 0x00000000;
		}
	}

	//printf("lanc2_r: %08X, %08X at %08X\n", offset, mem_mask, activecpu_get_pc());

	return r;
}

static WRITE32_HANDLER( lanc2_w )
{
	if (offset == 0)
	{
		if (!(mem_mask & 0xff000000))
		{
			UINT8 value = data >> 24;

			value = ((value >> 7) & 0x01) |
					((value >> 5) & 0x02) |
					((value >> 3) & 0x04) |
					((value >> 1) & 0x08) |
					((value << 1) & 0x10) |
					((value << 3) & 0x20) |
					((value << 5) & 0x40) |
					((value << 7) & 0x80);

			fpga_uploaded = 1;

			//printf("lanc2_fpga_w: %02X at %08X\n", value, activecpu_get_pc());
		}
		else if (!(mem_mask & 0x000000ff))
		{
			lanc2_ram[lanc2_ram_w & 0x7fff] = data & 0xff;
			lanc2_ram_w++;
		}
		else
		{
			//printf("lanc2_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, activecpu_get_pc());
		}
	}
	if (offset == 4)
	{
		if (mame_stricmp(Machine->gamedrv->name, "thrilld") == 0)
		{
			work_ram[(0x3ffed0/4) + 0] = 0x472a3731;
			work_ram[(0x3ffed0/4) + 1] = 0x33202020;
			work_ram[(0x3ffed0/4) + 2] = 0x2d2d2a2a;
			work_ram[(0x3ffed0/4) + 3] = 0x2a207878;

			work_ram[(0x3fff40/4) + 0] = 0x47433731;
			work_ram[(0x3fff40/4) + 1] = 0x33000000;
			work_ram[(0x3fff40/4) + 2] = 0x19994a41;
			work_ram[(0x3fff40/4) + 3] = 0x4100a9b1;
		}
	}

	//printf("lanc2_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, activecpu_get_pc());
}

/*****************************************************************************/

static ADDRESS_MAP_START( nwktr_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0x80000000) AM_RAM AM_BASE(&work_ram)		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_reg_r, K001604_reg_w)
	AM_RANGE(0x74010000, 0x74017fff) AM_MIRROR(0x80000000) AM_READWRITE(SMH_RAM, paletteram32_w) AM_BASE(&paletteram32)
	AM_RANGE(0x74020000, 0x7403ffff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_tile_r, K001604_tile_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_MIRROR(0x80000000) AM_READWRITE(K001604_char_r, K001604_char_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x780c0000, 0x780c0003) AM_MIRROR(0x80000000) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7d000000, 0x7d00ffff) AM_MIRROR(0x80000000) AM_READ(sysreg_r)
	AM_RANGE(0x7d010000, 0x7d01ffff) AM_MIRROR(0x80000000) AM_WRITE(sysreg_w)
	AM_RANGE(0x7d020000, 0x7d021fff) AM_MIRROR(0x80000000) AM_READWRITE(timekeeper_0_32be_r, timekeeper_0_32be_w)	/* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7d030000, 0x7d030007) AM_MIRROR(0x80000000) AM_READ(K056800_host_r)
	AM_RANGE(0x7d030000, 0x7d030007) AM_MIRROR(0x80000000) AM_WRITE(K056800_host_w)
	AM_RANGE(0x7d030008, 0x7d03000f) AM_MIRROR(0x80000000) AM_WRITE(K056800_host_w)
	AM_RANGE(0x7d040000, 0x7d04ffff) AM_MIRROR(0x80000000) AM_READWRITE(lanc1_r, lanc1_w)
	AM_RANGE(0x7d050000, 0x7d05ffff) AM_MIRROR(0x80000000) AM_READWRITE(lanc2_r, lanc2_w)
	AM_RANGE(0x7e000000, 0x7e7fffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER2, 0)	/* Data ROM */
	AM_RANGE(0x7f000000, 0x7f1fffff) AM_MIRROR(0x80000000) AM_ROM AM_SHARE(2)
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION(REGION_USER1, 0) AM_SHARE(2)	/* Program ROM */
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

static UINT32 *sharc_dataram;

static READ32_HANDLER( dsp_dataram_r )
{
	return sharc_dataram[offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram_w )
{
	sharc_dataram[offset] = data;
}

static ADDRESS_MAP_START( sharc_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram_r, dsp_dataram_w)
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_READWRITE(nwk_voodoo_0_r, nwk_voodoo_0_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_READWRITE(K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK(5)
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( nwktr )
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
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Disable Machine Init" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP3" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP6" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "DIP8" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START_TAG("ANALOG1")		// Steering
	PORT_BIT( 0xfff, 0x800, IPT_PADDLE ) PORT_MINMAX(0x000, 0xfff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG2")		// Acceleration pedal
	PORT_BIT( 0x7ff, 0x000, IPT_PEDAL ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG3")		// Foot brake pedal
	PORT_BIT( 0x7ff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG4")		// Hand brake lever
	PORT_BIT( 0x7ff, 0x000, IPT_AD_STICK_Y ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START_TAG("ANALOG5")		// Clutch pedal
	PORT_BIT( 0x7ff, 0x000, IPT_PEDAL3 ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

INPUT_PORTS_END

static const struct RF5C400interface rf5c400_interface =
{
	REGION_SOUND1
};

static const ppc_config nwktr_ppc_cfg =
{
	PPC_MODEL_403GA
};

static const sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};

static MACHINE_RESET( nwktr )
{
	cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_DRIVER_START( nwktr )

	/* basic machine hardware */
	MDRV_CPU_ADD(PPC403, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_CONFIG(nwktr_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(nwktr_map, 0)

	MDRV_CPU_ADD(M68000, 64000000/4)	/* 16MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap, 0)

	MDRV_CPU_ADD(ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc_map, 0)

	MDRV_INTERLEAVE(100)

	MDRV_MACHINE_RESET(nwktr)
	MDRV_NVRAM_HANDLER( timekeeper_0 )

 	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(nwktr)
	MDRV_VIDEO_UPDATE(nwktr)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(RF5C400, 64000000/4)
	MDRV_SOUND_CONFIG(rf5c400_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

MACHINE_DRIVER_END

/*****************************************************************************/

static UINT8 jamma_rdata[1024];
static void jamma_r(int length)
{
	int i;
	//printf("jamma_r %d\n", length);
	for (i=0; i < length; i++)
	{
		jamma_rdata[i] = 0;
	}
}

static UINT8 jamma_wdata[1024];
static void jamma_w(int length)
{
	int i;
	//printf("jamma_w %d\n", length);
	for (i=0; i < length; i++)
	{
		//jamma_jvs_w(jamma_wdata[i]);
	}

	//jamma_jvs_cmd_exec();
}

static void sound_irq_callback(int irq)
{
	if (irq == 0)
		cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ1, PULSE_LINE);
	else
		cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ2, PULSE_LINE);
}

static DRIVER_INIT( nwktr )
{
	init_konami_cgboard(1, CGBOARD_TYPE_NWKTR);
	set_cgboard_texture_bank(0, 5, memory_region(REGION_USER5));

	sharc_dataram = auto_malloc(0x100000);
	timekeeper_init(0, TIMEKEEPER_M48T58, backup_ram);

	K056800_init(sound_irq_callback);
	K033906_init();

	ppc403_install_spu_tx_dma_handler(jamma_w, jamma_wdata);
	ppc403_install_spu_rx_dma_handler(jamma_r, jamma_rdata);

	adc1213x_init(0, adc12138_input_callback);
}

static DRIVER_INIT(thrilld)
{
	int i;
	UINT16 checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x43;	// 'C'
	backup_ram[0x02] = 0x36;	// '6'
	backup_ram[0x03] = 0x37;	// '7'
	backup_ram[0x04] = 0x36;	// '6'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	// ?
	backup_ram[0x07] = 0x00;	// ?
	backup_ram[0x08] = 0x19;	//
	backup_ram[0x09] = 0x99;	// 1999
	backup_ram[0x0a] = 0x4a;	// 'J'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x41;	//
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
	for (i=0; i < 14; i+=2)
	{
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
		checksum &= 0xffff;
	}
	checksum = -1 - checksum;

	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum

	DRIVER_INIT_CALL(nwktr);
}

static DRIVER_INIT(racingj)
{
	int i;
	UINT32 checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x6e;	// 'n'
	backup_ram[0x02] = 0x36;	// '6'
	backup_ram[0x03] = 0x37;	// '7'
	backup_ram[0x04] = 0x36;	// '6'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	//
	backup_ram[0x07] = 0x00;	//
	backup_ram[0x08] = 0x00;	//
	backup_ram[0x09] = 0x00;	//
	backup_ram[0x0a] = 0x4a;	// 'J'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x45;	// 'E'
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
    for (i=0; i < 14; i+=2)
    {
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
        checksum &= 0xffff;
    }
	checksum = -1 - checksum;
	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum

	DRIVER_INIT_CALL(nwktr);
}

static DRIVER_INIT(racingj2)
{
	int i;
	UINT32 checksum;

	/* RTC data */
	backup_ram[0x00] = 0x47;	// 'G'
	backup_ram[0x01] = 0x6e;	// 'n'
	backup_ram[0x02] = 0x38;	// '8'
	backup_ram[0x03] = 0x38;	// '8'
	backup_ram[0x04] = 0x38;	// '8'
	backup_ram[0x05] = 0x00;	//
	backup_ram[0x06] = 0x00;	//
	backup_ram[0x07] = 0x00;	//
	backup_ram[0x08] = 0x00;	//
	backup_ram[0x09] = 0x00;	//
	backup_ram[0x0a] = 0x4a;	// 'J'
	backup_ram[0x0b] = 0x41;	// 'A'
	backup_ram[0x0c] = 0x45;	// 'E'
	backup_ram[0x0d] = 0x00;	//

	checksum = 0;
	for (i=0; i < 14; i+=2)
	{
		checksum += (backup_ram[i] << 8) | (backup_ram[i+1]);
		checksum &= 0xffff;
	}
	checksum = -1 - checksum;
	backup_ram[0x0e] = (checksum >> 8) & 0xff;	// checksum
	backup_ram[0x0f] = (checksum >> 0) & 0xff;	// checksum

	DRIVER_INIT_CALL(nwktr);
}


/*****************************************************************************/

ROM_START(racingj)
	ROM_REGION32_BE(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("676nc01.bin", 0x000000, 0x200000, CRC(690346b5) SHA1(157ab6788382ef4f5a8772f08819f54d0856fcc8))

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)		/* Data roms */
		ROM_LOAD32_WORD_SWAP("676a04.bin", 0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f))
		ROM_LOAD32_WORD_SWAP("676a05.bin", 0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5))

	ROM_REGION32_BE(0x800000, REGION_USER5, 0)	/* CG Board texture roms */
	    ROM_LOAD32_WORD_SWAP( "676a13.8x",    0x000000, 0x400000, CRC(29077763) SHA1(ee087ca0d41966ca0fd10727055bb1dcd05a0873) )
        ROM_LOAD32_WORD_SWAP( "676a14.16x",   0x000002, 0x400000, CRC(50a7e3c0) SHA1(7468a66111a3ddf7c043cd400fa175cae5f65632) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program roms */
        ROM_LOAD16_WORD_SWAP( "676gna08.7s", 0x000000, 0x080000, CRC(8973f6f2) SHA1(f5648a7e0205f7e979ccacbb52936809ce14a184) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0) 		/* other roms (textures?) */
        ROM_LOAD( "676a09.16p",   0x000000, 0x400000, CRC(f85c8dc6) SHA1(8b302c80be309b5cc68b75945fcd7b87a56a4c9b) )
        ROM_LOAD( "676a10.14p",   0x400000, 0x400000, CRC(7b5b7828) SHA1(aec224d62e4b1e8fdb929d7947ce70d84ba676cf) )
ROM_END

ROM_START(racingj2)
	ROM_REGION32_BE(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("888a01.27p", 0x000000, 0x200000, CRC(d077890a) SHA1(08b252324cf46fbcdb95e8f9312287920cd87c5d))

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)		/* Data roms */
		ROM_LOAD32_WORD_SWAP( "676a04.bin",	0x000000, 0x200000, CRC(d7808cb6) SHA1(0668fae5bb94cc120fe196d4b18200f7b512317f))
		ROM_LOAD32_WORD_SWAP( "676a05.bin",	0x000002, 0x200000, CRC(fb4de1ad) SHA1(f6aa4eb1b5d22901a2aaf899ed3237a9dfdc55b5))
	ROM_LOAD32_WORD_SWAP( "888a06.12t",	0x400000, 0x200000, CRC(00cbec4d) SHA1(1ce7807d86e90edbf4eecba462a27c725f5ad862))

	ROM_REGION32_BE(0x800000, REGION_USER5, 0)	/* CG Board Texture roms */
	    ROM_LOAD32_WORD_SWAP( "888a13.8x",    0x000000, 0x400000, CRC(2292f530) SHA1(0f4d1332708fd5366a065e0a928cc9610558b42d) )
        ROM_LOAD32_WORD_SWAP( "888a14.16x",   0x000002, 0x400000, CRC(6a834a26) SHA1(d1fbd7ae6afd05f0edac4efde12a5a45aa2bc7df) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program roms */
        ROM_LOAD16_WORD_SWAP( "888a08.7s",    0x000000, 0x080000, CRC(55fbea65) SHA1(ad953f758181731efccadcabc4326e6634c359e8) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0) 		/* PCM sample roms */
        ROM_LOAD( "888a09.16p",   0x000000, 0x400000, CRC(11e2fed2) SHA1(24b8a367b59fedb62c56f066342f2fa87b135fc5) )
        ROM_LOAD( "888a10.14p",   0x400000, 0x400000, CRC(328ce610) SHA1(dbbc779a1890c53298c0db129d496df048929496) )
ROM_END

ROM_START(thrilld)
	ROM_REGION32_BE(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
	ROM_LOAD16_WORD_SWAP("713be01.27p", 0x000000, 0x200000, CRC(d84a7723) SHA1(f4e9e08591b7e5e8419266dbe744d56a185384ed))

	ROM_REGION32_BE(0x800000, REGION_USER2, 0)		/* Data roms */
		ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a))
		ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69))

	ROM_REGION32_BE(0x800000, REGION_USER5, 0)	/* CG Board Texture roms */
        ROM_LOAD32_WORD_SWAP( "713a13.8x",    0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
        ROM_LOAD32_WORD_SWAP( "713a14.16x",   0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, REGION_CPU2, 0)		/* 68k program roms */
        ROM_LOAD16_WORD_SWAP( "713a08.7s",    0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION(0x1000000, REGION_SOUND1, 0) 		/* PCM sample roms */
        ROM_LOAD( "713a09.16p",   0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
        ROM_LOAD( "713a10.14p",   0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )
ROM_END

/*****************************************************************************/

GAME( 1998, racingj,	0,		nwktr,	nwktr,	racingj,	ROT0,	"Konami",	"Racing Jam", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1999, racingj2,	racingj,nwktr,	nwktr,	racingj2,	ROT0,	"Konami",	"Racing Jam: Chapter 2", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, thrilld,	0,		nwktr,	nwktr,	thrilld,	ROT0,	"Konami",	"Thrill Drive", GAME_NOT_WORKING|GAME_IMPERFECT_SOUND )
