/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"

#define CPK_VIDEO_SIZE	0x800

UINT8 * cpk_colorram;
UINT8 * cpk_videoram;
UINT8 * cpk_expram;
static UINT8 * cpk_palette;
static UINT8 * cpk_palette2;

static int hopperOK = 0;
static int intstate1 = 0;	/* unknown */
static int intstate2 = 0;	/* unknown */
static int oldstate = 0;
static int machinestate = 0;
static int abilityflag = 0;
static int sleepcountdown = 0;



/* Palette handling */

WRITE8_HANDLER( cpk_palette_w )
{
	cpk_palette[offset] = data;
}

WRITE8_HANDLER( cpk_palette2_w )
{
	int newword;

	cpk_palette2[offset] = data;

	newword = cpk_palette[offset] + 256 * cpk_palette2[offset];
	palette_set_color_rgb(machine,offset,pal5bit(newword >> 0),pal5bit(newword >> 5),pal5bit(newword >> 10));
}

/* Video handling */

READ8_HANDLER( cpk_expansion_r )
{
	UINT8 * RAM = memory_region(machine, REGION_GFX3);
	return RAM[offset];
}


/* Machine init/deinit  */

VIDEO_START( cska )
{
	int i;

	cpk_palette  = auto_malloc(0x800 * sizeof(UINT8));
	cpk_palette2 = auto_malloc(0x800 * sizeof(UINT8));

	/* leave everything at the default, but map all foreground 0 pens as transparent */
	for (i = 0;i < 16;i++)
        {
//               palette_used_colors[64 * i] = PALETTE_COLOR_TRANSPARENT;
        }
}



static void init_csk227_internals(void)
{
	hopperOK = 0xf9d0;
	machinestate = 0xf6b0;
	intstate1 = 0xf08c;
	intstate2 = 0xf08d;
	sleepcountdown = 0xf994;
}

static void init_csk234_internals(void)
{
	hopperOK = 0xf9ca;
	machinestate = 0xf6ad;
	intstate1 = 0xf088;
	intstate2 = 0xf089;
	sleepcountdown = 0xf98e;
}

MACHINE_RESET (cpk)
{
	static int initonce = 0;

	memset(cpk_videoram, 0, CPK_VIDEO_SIZE);
	memset(cpk_colorram, 0, CPK_VIDEO_SIZE);
	memset(cpk_expram, 0, CPK_VIDEO_SIZE);
	memset(cpk_palette, 0, 0x800);
	memset(cpk_palette2, 0, 0x800);

	if (!initonce)
	{
		initonce = 1;

		if (strcmp(machine->gamedrv->name,"csk227it") == 0)
			init_csk227_internals();
		else if (strcmp(machine->gamedrv->name,"csk234it") == 0)
			init_csk234_internals();
	}
}

INTERRUPT_GEN( cpoker_interrupt )
{
	if (cpu_getiloops() % 2)
		cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
	else
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN( cska_interrupt )
{
	UINT8 * RAM = memory_region(machine, REGION_CPU1);

	RAM[ hopperOK ] = 1;	/* simulate hopper working! */

	if (oldstate == 5)
	{
		if (RAM[sleepcountdown] == 0 && RAM[sleepcountdown+1] == 0)
			oldstate = RAM[ machinestate ];
	}
	else oldstate = RAM[ machinestate ];

	abilityflag = (oldstate == 5 || (RAM[ intstate1 ] == 1 && RAM[ intstate2 ] == 5)) ? 1 : 0;

	if (cpu_getiloops() % 2)
		cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
	else
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);

}



/* Screen refresh section */

VIDEO_UPDATE( cska )
{
	int offs;

//     palette_init_used_colors();

	for (offs = CPK_VIDEO_SIZE-1; offs >= 0; offs--)
	{
//             int color = ((cpk_colorram[offs] & 0xe0) >> 4) + 1;
//             memset(&palette_used_colors[64 * color + 1],PALETTE_COLOR_USED,63);
	}

#if 0
	if (keyboard_pressed(KEYCODE_9))
	{
		FILE * f = fopen("dump.bin", "wb");
		if (f)
		{
			fwrite(&cpk_videoram[0], 1, CPK_VIDEO_SIZE, f);
			fwrite(&cpk_colorram[0], 1, CPK_VIDEO_SIZE, f);
			fclose(f);
		}
	}
#endif

	if (abilityflag)
	{
		for (offs = (CPK_VIDEO_SIZE)-1; offs >= 0; offs--)
		{
			int sx,sy;


			sx = offs % 64;
			sy = offs / 64;

			drawgfx(bitmap,screen->machine->gfx[1+(offs % 4)],
					cpk_expram[offs],
					0,
					0,0,
					8*sx,32*sy,
					cliprect,TRANSPARENCY_NONE,0);
		}
	}

	for (offs = CPK_VIDEO_SIZE-1; offs >= 0; offs--)
	{
		int tile = cpk_videoram[offs] + ((cpk_colorram[offs] & 0x1f) << 8);
		int color = (tile != 0x1fff) ? (((cpk_colorram[offs] & 0xe0) >> 4) + 1) : 0;

		int sx,sy;


		sx = offs % 64;
		sy = offs / 64;

		drawgfx(bitmap,screen->machine->gfx[0],
				tile,
				color,
				0,0,
				8*sx,8*sy,
				cliprect,
				(abilityflag) ? TRANSPARENCY_PEN : TRANSPARENCY_NONE, 0);
	}

	return 0;
}
