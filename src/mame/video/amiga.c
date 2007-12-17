/***************************************************************************

    Amiga hardware

    Driver by: Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles

***************************************************************************/

#include "driver.h"
#include "includes/amiga.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_COPPER			0
#define GUESS_COPPER_OFFSET	0
#define LOG_SPRITE_DMA		0



/*************************************
 *
 *  Macros
 *
 *************************************/

#define COPPER_CYCLES_TO_PIXELS(x)		(4 * (x))



/*************************************
 *
 *  Statics
 *
 *************************************/

/* sprite states */
static UINT8 sprite_comparitor_enable_mask;
static UINT8 sprite_dma_reload_mask;
static UINT8 sprite_dma_live_mask;
static UINT32 sprite_shiftreg[8];
static UINT8 sprite_remain[8];

/* playfield states */
static int last_scanline;
static UINT16 ham_color;

/* copper states */
static UINT32 copper_pc;
static UINT8 copper_waiting;
static UINT8 copper_waitblit;
static UINT16 copper_waitval;
static UINT16 copper_waitmask;
static UINT16 copper_pending_offset;
static UINT16 copper_pending_data;

/* misc states */
static UINT16 genlock_color;

#if GUESS_COPPER_OFFSET
static int wait_offset = 3;
#endif



/*************************************
 *
 *  Tables
 *
 *************************************/

/* expand an 8-bit bit pattern into 16 bits, every other bit */
static const UINT16 expand_byte[256] =
{
	0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
	0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
	0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
	0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
	0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
	0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
	0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
	0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
	0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
	0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
	0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
	0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
	0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
	0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
	0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
	0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,

	0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
	0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
	0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
	0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
	0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
	0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
	0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
	0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
	0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
	0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
	0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
	0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
	0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
	0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
	0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
	0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};

/* separate 6 in-order bitplanes into 2 x 3-bit bitplanes in two nibbles */
static UINT8 separate_bitplanes[2][64];



/*************************************
 *
 *  4-4-4 palette init
 *
 *************************************/

PALETTE_INIT( amiga )
{
	int i;

	for (i = 0; i < 0x1000; i++)
		palette_set_color_rgb(machine, i, pal4bit(i >> 8), pal4bit(i >> 4), pal4bit(i));
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( amiga )
{
	int j;

	/* generate tables that produce the correct playfield color for dual playfield mode */
	for (j = 0; j < 64; j++)
	{
		int pf1pix = ((j >> 0) & 1) | ((j >> 1) & 2) | ((j >> 2) & 4);
		int pf2pix = ((j >> 1) & 1) | ((j >> 2) & 2) | ((j >> 3) & 4);

		separate_bitplanes[0][j] = (pf1pix || !pf2pix) ? pf1pix : (pf2pix + 8);
		separate_bitplanes[1][j] = pf2pix ? (pf2pix + 8) : pf1pix;
	}

	/* reset the genlock color */
	genlock_color = 0xffff;

	video_start_generic_bitmapped(machine);
}



/*************************************
 *
 *  Beam position
 *
 *************************************/

UINT32 amiga_gethvpos(void)
{
	UINT32 hvpos = (last_scanline << 8) | (video_screen_get_hpos(0) >> 2);
	UINT32 latchedpos = readinputportbytag_safe("HVPOS", 0);

	/* if there's no latched position, or if we are in the active display area */
	/* but before the latching point, return the live HV position */
	if ((CUSTOM_REG(REG_BPLCON0) & 0x0008) == 0 || latchedpos == 0 || (last_scanline >= 20 && hvpos < latchedpos))
		return hvpos;

	/* otherwise, return the latched position */
	return latchedpos;
}



/*************************************
 *
 *  Genlock interaction
 *
 *************************************/

void amiga_set_genlock_color(UINT16 color)
{
	genlock_color = color;
}



/*************************************
 *
 *  Copper emulation
 *
 *************************************/

void copper_setpc(UINT32 pc)
{
	if (LOG_COPPER)
		logerror("copper_setpc(%06x)\n", pc);

	copper_pc = pc;
	copper_waiting = FALSE;
}


static int copper_execute_next(int xpos)
{
	int word0, word1;

	/* bail if not enabled */
	if ((CUSTOM_REG(REG_DMACON) & (DMACON_COPEN | DMACON_DMAEN)) != (DMACON_COPEN | DMACON_DMAEN))
		return 511;

	/* flush any pending writes */
	if (copper_pending_offset)
	{
		if (LOG_COPPER)
			logerror("%02X.%02X: Write to %s = %04x\n", last_scanline, xpos / 2, amiga_custom_names[copper_pending_offset & 0xff], copper_pending_data);

		amiga_custom_w(copper_pending_offset, copper_pending_data, 0);
		copper_pending_offset = 0;
	}

	/* if we're waiting, check for a breakthrough */
	if (copper_waiting)
	{
		int curpos = (last_scanline << 8) | (xpos >> 1);

		/* if we're past the wait time, stop it and hold up 2 cycles */
		if ((curpos & copper_waitmask) >= (copper_waitval & copper_waitmask) &&
			(!copper_waitblit || !(CUSTOM_REG(REG_DMACON) & DMACON_BBUSY)))
		{
			copper_waiting = FALSE;
#if GUESS_COPPER_OFFSET
			return xpos + COPPER_CYCLES_TO_PIXELS(1 + wait_offset);
#else
			return xpos + COPPER_CYCLES_TO_PIXELS(1 + 3);
#endif
		}

		/* otherwise, see if this line is even a possibility; if not, punt */
		if (((curpos | 0xff) & copper_waitmask) < (copper_waitval & copper_waitmask))
			return 511;

		/* else just advance another pixel */
		xpos += COPPER_CYCLES_TO_PIXELS(1);
		return xpos;
	}

	/* fetch the first data word */
	word0 = amiga_chip_ram_r(copper_pc);
	copper_pc += 2;
	xpos += COPPER_CYCLES_TO_PIXELS(1);

	/* fetch the second data word */
	word1 = amiga_chip_ram_r(copper_pc);
	copper_pc += 2;
	xpos += COPPER_CYCLES_TO_PIXELS(1);

	if (LOG_COPPER)
		logerror("%02X.%02X: Copper inst @ %06x = %04x %04x\n", last_scanline, xpos / 2, copper_pc, word0, word1);

	/* handle a move */
	if ((word0 & 1) == 0)
	{
		int min = (CUSTOM_REG(REG_COPCON) & 2) ? 0x20 : 0x40;

		/* do the write if we're allowed */
		word0 = (word0 >> 1) & 0xff;
		if (word0 >= min)
		{
			/* write it at the *end* of this instruction's cycles */
			/* needed for Arcadia's Fast Break */
			copper_pending_offset = word0;
			copper_pending_data = word1;
		}

		/* illegal writes suspend until next frame */
		else
		{
			if (LOG_COPPER)
				logerror("%02X.%02X: Aborting copper on illegal write\n", last_scanline, xpos / 2);

			copper_waitval = 0xffff;
			copper_waitmask = 0xffff;
			copper_waitblit = FALSE;
			copper_waiting = TRUE;
			return 511;
		}
	}
	else
	{
		/* extract common wait/skip values */
		copper_waitval = word0 & 0xfffe;
		copper_waitmask = word1 | 0x8001;
		copper_waitblit = (~word1 >> 15) & 1;

		/* handle a wait */
		if ((word1 & 1) == 0)
		{
			if (LOG_COPPER)
				logerror("  Waiting for %04x & %04x (currently %04x)\n", copper_waitval, copper_waitmask, (last_scanline << 8) | (xpos >> 1));

			copper_waiting = TRUE;
		}

		/* handle a skip */
		else
		{
			int curpos = (last_scanline << 8) | (xpos >> 1);

			if (LOG_COPPER)
				logerror("  Skipping if %04x & %04x (currently %04x)\n", copper_waitval, copper_waitmask, (last_scanline << 8) | (xpos >> 1));

			/* if we're past the wait time, stop it and hold up 2 cycles */
			if ((curpos & copper_waitmask) >= (copper_waitval & copper_waitmask) &&
				(!copper_waitblit || !(CUSTOM_REG(REG_DMACON) & DMACON_BBUSY)))
			{
				if (LOG_COPPER)
					logerror("  Skipped\n");

				/* count the cycles it out have taken to fetch the next instruction */
				copper_pc += 4;
				xpos += COPPER_CYCLES_TO_PIXELS(2);
			}
		}
	}

	/* advance and consume 8 cycles */
	return xpos;
}



/*************************************
 *
 *  External sprite controls
 *
 *************************************/

void amiga_sprite_dma_reset(int which)
{
	sprite_dma_reload_mask |= 1 << which;
	sprite_dma_live_mask |= 1 << which;
}


void amiga_sprite_enable_comparitor(int which, int enable)
{
	if (enable)
	{
		sprite_comparitor_enable_mask |= 1 << which;
		sprite_dma_live_mask &= ~(1 << which);
	}
	else
		sprite_comparitor_enable_mask &= ~(1 << which);
}



/*************************************
 *
 *  Per-scanline sprite fetcher
 *
 *************************************/

static void update_sprite_dma(int scanline)
{
	int dmaenable = (CUSTOM_REG(REG_DMACON) & (DMACON_SPREN | DMACON_DMAEN)) == (DMACON_SPREN | DMACON_DMAEN);
	int num, maxdma;

	/* channels are limited by DDFSTART */
	maxdma = (CUSTOM_REG(REG_DDFSTRT) - 0x14) / 4;
	if (maxdma > 8)
		maxdma = 8;

	/* loop over sprite channels */
	for (num = 0; num < maxdma; num++)
	{
		int bitmask = 1 << num;
		int vstart, vstop;

		/* if we are == VSTOP, fetch new control words */
		if (dmaenable && (sprite_dma_live_mask & bitmask) && (sprite_dma_reload_mask & bitmask))
		{
			/* disable the sprite */
			sprite_comparitor_enable_mask &= ~bitmask;
			sprite_dma_reload_mask &= ~bitmask;

			/* fetch data into the control words */
			CUSTOM_REG(REG_SPR0POS + 4 * num) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 0);
			CUSTOM_REG(REG_SPR0CTL + 4 * num) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) += 4;
			if (LOG_SPRITE_DMA) logerror("%3d:sprite %d fetch: pos=%04X ctl=%04X\n", scanline, num, CUSTOM_REG(REG_SPR0POS + 4 * num), CUSTOM_REG(REG_SPR0CTL + 4 * num));
		}

		/* compute vstart/vstop */
		vstart = (CUSTOM_REG(REG_SPR0POS + 4 * num) >> 8) | ((CUSTOM_REG(REG_SPR0CTL + 4 * num) << 6) & 0x100);
		vstop = (CUSTOM_REG(REG_SPR0CTL + 4 * num) >> 8) | ((CUSTOM_REG(REG_SPR0CTL + 4 * num) << 7) & 0x100);

		/* if we hit vstart, enable the comparitor */
		if (scanline == vstart)
		{
			sprite_comparitor_enable_mask |= 1 << num;
			if (LOG_SPRITE_DMA) logerror("%3d:sprite %d comparitor enable\n", scanline, num);
		}

		/* if we hit vstop, disable the comparitor and trigger a reload for the next scanline */
		if (scanline == vstop)
		{
			sprite_comparitor_enable_mask &= ~bitmask;
			sprite_dma_reload_mask |= 1 << num;
			CUSTOM_REG(REG_SPR0DATA + 4 * num) = 0;		/* just a guess */
			CUSTOM_REG(REG_SPR0DATB + 4 * num) = 0;
			if (LOG_SPRITE_DMA) logerror("%3d:sprite %d comparitor disable, prepare for reload\n", scanline, num);
		}

		/* fetch data if this sprite is enabled */
		if (dmaenable && (sprite_dma_live_mask & bitmask) && (sprite_comparitor_enable_mask & bitmask))
		{
			CUSTOM_REG(REG_SPR0DATA + 4 * num) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 0);
			CUSTOM_REG(REG_SPR0DATB + 4 * num) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) += 4;
			if (LOG_SPRITE_DMA) logerror("%3d:sprite %d fetch: data=%04X-%04X\n", scanline, num, CUSTOM_REG(REG_SPR0DATA + 4 * num), CUSTOM_REG(REG_SPR0DATB + 4 * num));
		}
	}
}



/*************************************
 *
 *  Per-pixel sprite computations
 *
 *************************************/

INLINE UINT32 interleave_sprite_data(UINT16 lobits, UINT16 hibits)
{
	return (expand_byte[lobits & 0xff] << 0) | (expand_byte[lobits >> 8] << 16) |
		   (expand_byte[hibits & 0xff] << 1) | (expand_byte[hibits >> 8] << 17);
}


static int get_sprite_pixel(int x)
{
	int pixels = 0;
	int num, pair;

	/* loop over sprite channels */
	for (num = 0; num < 8; num++)
		if (sprite_comparitor_enable_mask & (1 << num))
		{
			/* if we're not currently clocking, check against hstart */
			if (sprite_remain[num] == 0)
			{
				int hstart = ((CUSTOM_REG(REG_SPR0POS + 4 * num) & 0xff) << 1) | (CUSTOM_REG(REG_SPR0CTL + 4 * num) & 1);
				if (hstart == x)
				{
					sprite_remain[num] = 16;
					sprite_shiftreg[num] = interleave_sprite_data(CUSTOM_REG(REG_SPR0DATA + 4 * num), CUSTOM_REG(REG_SPR0DATB + 4 * num));
				}
			}

			/* clock the next pixel if we're doing it */
			if (sprite_remain[num] != 0)
			{
				sprite_remain[num]--;
				pixels |= (sprite_shiftreg[num] & 0xc0000000) >> (16 + 2 * (7 - num));
				sprite_shiftreg[num] <<= 2;
			}
		}

	/* if we have pixels, determine the actual color and get out */
	if (pixels)
	{
		static const UINT16 ormask[16] =
		{
			0x0000, 0x000c, 0x00c0, 0x00cc, 0x0c00, 0x0c0c, 0x0cc0, 0x0ccc,
			0xc000, 0xc00c, 0xc0c0, 0xc0cc, 0xcc00, 0xcc0c, 0xccc0, 0xcccc
		};
		static const UINT16 spritecollide[16] =
		{
			0x0000, 0x0000, 0x0000, 0x0200, 0x0000, 0x0400, 0x1000, 0x1600,
			0x0000, 0x0800, 0x2000, 0x2a00, 0x4000, 0x4c00, 0x7000, 0x7e00
		};
		int collide;

		/* OR the two sprite bits together so we only have 1 bit per sprite */
		collide = pixels | (pixels >> 1);

		/* based on the CLXCON, merge even/odd sprite results */
		collide |= (collide & ormask[CUSTOM_REG(REG_CLXCON) >> 12]) >> 2;

		/* collapse down to a 4-bit final "sprite present" mask */
		collide = (collide & 1) | ((collide >> 3) & 2) | ((collide >> 6) & 4) | ((collide >> 9) & 8);

		/* compute sprite-sprite collisions */
		CUSTOM_REG(REG_CLXDAT) |= spritecollide[collide];

		/* now determine the actual color */
		for (pair = 0; pixels; pair++, pixels >>= 4)
			if (pixels & 0x0f)
			{
				/* final result is:
                    topmost sprite color in bits 0-5
                    sprite present bitmask in bits 6-9
                    topmost sprite pair index in bits 10-11
                */

				/* attached case */
				if (CUSTOM_REG(REG_SPR1CTL + 8 * pair) & 0x0080)
					return (pixels & 0xf) | 0x10 | (collide << 6) | (pair << 10);

				/* lower-numbered sprite of pair */
				else if (pixels & 3)
					return (pixels & 3) | 0x10 | (pair << 2) | (collide << 6) | (pair << 10);

				/* higher-numbered sprite of pair */
				else
					return ((pixels >> 2) & 3) | 0x10 | (pair << 2) | (collide << 6) | (pair << 10);
			}
	}

	return 0;
}



/*************************************
 *
 *  Bitplane assembly
 *
 *************************************/

INLINE UINT8 assemble_odd_bitplanes(int planes, int obitoffs)
{
	UINT8 pix = (CUSTOM_REG(REG_BPL1DAT) >> obitoffs) & 1;
	if (planes >= 3)
	{
		pix |= ((CUSTOM_REG(REG_BPL3DAT) >> obitoffs) & 1) << 2;
		if (planes >= 5)
			pix |= ((CUSTOM_REG(REG_BPL5DAT) >> obitoffs) & 1) << 4;
	}
	return pix;
}


INLINE UINT8 assemble_even_bitplanes(int planes, int ebitoffs)
{
	UINT8 pix = 0;
	if (planes >= 2)
	{
		pix |= ((CUSTOM_REG(REG_BPL2DAT) >> ebitoffs) & 1) << 1;
		if (planes >= 4)
		{
			pix |= ((CUSTOM_REG(REG_BPL4DAT) >> ebitoffs) & 1) << 3;
			if (planes >= 6)
				pix |= ((CUSTOM_REG(REG_BPL6DAT) >> ebitoffs) & 1) << 5;
		}
	}
	return pix;
}



/*************************************
 *
 *  Hold and modify pixel computations
 *
 *************************************/

INLINE int update_ham(int newpix)
{
	switch (newpix >> 4)
	{
		case 0:
			ham_color = CUSTOM_REG(REG_COLOR00 + (newpix & 0xf));
			break;

		case 1:
			ham_color = (ham_color & 0xff0) | ((newpix & 0xf) << 0);
			break;

		case 2:
			ham_color = (ham_color & 0x0ff) | ((newpix & 0xf) << 8);
			break;

		case 3:
			ham_color = (ham_color & 0xf0f) | ((newpix & 0xf) << 4);
			break;
	}
	return ham_color;
}



/*************************************
 *
 *  Single scanline rasterizer
 *
 *************************************/

void amiga_render_scanline(int scanline)
{
	UINT16 save_color0 = CUSTOM_REG(REG_COLOR00);
	int ddf_start_pixel = 0, ddf_stop_pixel = 0;
	int hires = 0, dualpf = 0, lace = 0, ham = 0;
	int hstart = 0, hstop = 0;
	int vstart = 0, vstop = 0;
	int pf1pri = 0, pf2pri = 0;
	int planes = 0;

	int x;
	UINT16 *dst;
	int ebitoffs = 0, obitoffs = 0;
	int ecolmask = 0, ocolmask = 0;
	int edelay = 0, odelay = 0;
	int next_copper_x;
	int p;

	last_scanline = scanline;

	/* on the first scanline, reset the COPPER and HAM color */
	if (scanline == 0)
	{
		copper_setpc(CUSTOM_REG_LONG(REG_COP1LCH));
		ham_color = CUSTOM_REG(REG_COLOR00);
	}

	/* update sprite data fetching */
	update_sprite_dma(scanline);

	/* start of a new line, signal we're not done with it and fill up vars */
	dst = BITMAP_ADDR16(tmpbitmap, scanline, 0);

	/* all sprites off at the start of the line */
	memset(sprite_remain, 0, sizeof(sprite_remain));

	/* temporary set color 0 to the genlock color */
	if (genlock_color != 0xffff)
		CUSTOM_REG(REG_COLOR00) = genlock_color;

	/* loop over the line */
	next_copper_x = 2;	/* copper runs on odd timeslots */
	for (x = 0; x < 0xe4*2; x++)
	{
		int sprpix;

		/* time to execute the copper? */
		if (x == next_copper_x)
		{
			/* execute the next batch, restoring and re-saving color 0 around it */
			CUSTOM_REG(REG_COLOR00) = save_color0;
			next_copper_x = copper_execute_next(x);
			save_color0 = CUSTOM_REG(REG_COLOR00);
			if (genlock_color != 0xffff)
				CUSTOM_REG(REG_COLOR00) = genlock_color;

			/* compute update-related register values */
			planes = (CUSTOM_REG(REG_BPLCON0) & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) >> 12;
			hires = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HIRES;
			ham = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HOMOD;
			dualpf = CUSTOM_REG(REG_BPLCON0) & BPLCON0_DBLPF;
			lace = CUSTOM_REG(REG_BPLCON0) & BPLCON0_LACE;

			/* compute the pixel fetch parameters */
			ddf_start_pixel = ( CUSTOM_REG(REG_DDFSTRT) & 0xfc ) * 2 + (hires ? 9 : 17);
			ddf_stop_pixel = ( CUSTOM_REG(REG_DDFSTOP) & 0xfc ) * 2 + (hires ? (9 + 15) : (17 + 15));

			if ( ( CUSTOM_REG(REG_DDFSTRT) ^ CUSTOM_REG(REG_DDFSTOP) ) & 0x04 )
				ddf_stop_pixel += 8;

			/* compute the horizontal start/stop */
			hstart = CUSTOM_REG(REG_DIWSTRT) & 0xff;
			hstop = 0x100 + (CUSTOM_REG(REG_DIWSTOP) & 0xff);

			/* compute the vertical start/stop */
			vstart = CUSTOM_REG(REG_DIWSTRT) >> 8;
			vstop = (CUSTOM_REG(REG_DIWSTOP) >> 8) | ((~CUSTOM_REG(REG_DIWSTOP) >> 7) & 0x100);

			/* extract playfield priorities */
			pf1pri = CUSTOM_REG(REG_BPLCON2) & 7;
			pf2pri = (CUSTOM_REG(REG_BPLCON2) >> 3) & 7;

			/* extract collision masks */
			ocolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x15;
			ecolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x2a;
		}

		/* clear the target pixels to the background color as a starting point */
		dst[x*2+0] =
		dst[x*2+1] = CUSTOM_REG(REG_COLOR00);

		/* if we hit the first fetch pixel, reset the counters and latch the delays */
		if (x == ddf_start_pixel)
		{
			odelay = CUSTOM_REG(REG_BPLCON1) & 0xf;
			edelay = ( CUSTOM_REG(REG_BPLCON1) >> 4 ) & 0x0f;

			if ( hires )
			{
				obitoffs = 15 + ( odelay << 1 );
				ebitoffs = 15 + ( edelay << 1 );
			}
			else
			{
				if ( CUSTOM_REG(REG_DDFSTRT) & 0x04 )
				{
					odelay = ( odelay + 8 ) & 0x0f;
					edelay = ( edelay + 8 ) & 0x0f;
				}

				obitoffs = 15 + odelay;
				ebitoffs = 15 + edelay;
			}

			for (p = 0; p < 6; p++)
				CUSTOM_REG(REG_BPL1DAT + p) = 0;
		}

		/* need to run the sprite engine every pixel to ensure display */
		sprpix = get_sprite_pixel(x);

		/* to render, we must have bitplane DMA enabled, at least 1 plane, and be within the */
		/* vertical display window */
		if ((CUSTOM_REG(REG_DMACON) & (DMACON_BPLEN | DMACON_DMAEN)) == (DMACON_BPLEN | DMACON_DMAEN) &&
			planes > 0 && scanline >= vstart && scanline < vstop)
		{
			int pfpix0 = 0, pfpix1 = 0, collide;

			/* fetch the odd bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + odelay)
			{
				/* if we need to fetch more data, do it now */
				if (obitoffs == 15)
					for (p = 0; p < planes; p += 2)
					{
						CUSTOM_REG(REG_BPL1DAT + p) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BPL1PTH + p * 2));
						CUSTOM_REG_LONG(REG_BPL1PTH + p * 2) += 2;
					}

				/* now assemble the bits */
				pfpix0 |= assemble_odd_bitplanes(planes, obitoffs);
				obitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (obitoffs < 0)
					{
						obitoffs = 15;

						for (p = 0; p < planes; p += 2)
						{
							CUSTOM_REG(REG_BPL1DAT + p) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BPL1PTH + p * 2));
							CUSTOM_REG_LONG(REG_BPL1PTH + p * 2) += 2;
						}
					}

					pfpix1 |= assemble_odd_bitplanes(planes, obitoffs);
					obitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x15;

				/* reset bit offsets if needed */
				if (obitoffs < 0)
					obitoffs = 15;
			}

			/* fetch the even bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + edelay)
			{
				/* if we need to fetch more data, do it now */
				if (ebitoffs == 15)
					for (p = 1; p < planes; p += 2)
					{
						CUSTOM_REG(REG_BPL1DAT + p) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BPL1PTH + p * 2));
						CUSTOM_REG_LONG(REG_BPL1PTH + p * 2) += 2;
					}

				/* now assemble the bits */
				pfpix0 |= assemble_even_bitplanes(planes, ebitoffs);
				ebitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (ebitoffs < 0)
					{
						ebitoffs = 15;

						for (p = 1; p < planes; p += 2)
						{
							CUSTOM_REG(REG_BPL1DAT + p) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BPL1PTH + p * 2));
							CUSTOM_REG_LONG(REG_BPL1PTH + p * 2) += 2;
						}
					}

					pfpix1 |= assemble_even_bitplanes(planes, ebitoffs);
					ebitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x2a;

				/* reset bit offsets if needed */
				if (ebitoffs < 0)
					ebitoffs = 15;
			}

			/* compute playfield/sprite collisions for first pixel */
			collide = pfpix0 ^ CUSTOM_REG(REG_CLXCON);
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 5) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 1) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* compute playfield/sprite collisions for second pixel */
			collide = pfpix1 ^ CUSTOM_REG(REG_CLXCON);
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 5) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 1) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* if we are within the display region, render */
			if (x >= hstart && x < hstop)
			{
				/* hold-and-modify mode -- assume low-res (hi-res not supported by the hardware) */
				if (ham)
				{
					/* update the HAM color */
					pfpix0 = update_ham(pfpix0);

					/* sprite has priority */
					if (sprpix && pf1pri > (sprpix >> 10))
					{
						dst[x*2+0] =
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + (sprpix & 0x1f));
					}

					/* playfield has priority */
					else
					{
						dst[x*2+0] =
						dst[x*2+1] = pfpix0;
					}
				}

				/* dual playfield mode */
				else if (dualpf)
				{
					int pix;

					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					if (pix)
					{
						if ((pfpix0 & 0x15) && pf1pri <= (sprpix >> 10))
							pix = 0;
						if ((pfpix0 & 0x2a) && pf2pri <= (sprpix >> 10))
							pix = 0;
					}

					/* write out the left pixel */
					if (pix)
						dst[x*2+0] = CUSTOM_REG(REG_COLOR00 + pix);
					else
						dst[x*2+0] = CUSTOM_REG(REG_COLOR00 + separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix0]);

					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					if (pix)
					{
						if ((pfpix1 & 0x15) && pf1pri <= (sprpix >> 10))
							pix = 0;
						if ((pfpix1 & 0x2a) && pf2pri <= (sprpix >> 10))
							pix = 0;
					}

					/* write out the right pixel */
					if (pix)
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + pix);
					else
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix1]);
				}

				/* single playfield mode */
				else
				{
					/* sprite has priority */
					if (sprpix && pf1pri > (sprpix >> 10))
					{
						dst[x*2+0] =
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + (sprpix & 0x1f));
					}

					/* playfield has priority */
					else
					{
						dst[x*2+0] = CUSTOM_REG(REG_COLOR00 + pfpix0);
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + pfpix1);
					}
				}
			}
		}
	}

#if 0
	if ( cpu_getcurrentframe() % 64 == 0 && scanline == 100 )
	{
		const char *m_lores = "LORES";
		const char *m_hires = "HIRES";
		const char *m_ham = "HAM";
		const char *m_dualpf = "DUALPF";
		const char *m_lace = "LACE";
		const char *m_hilace = "HI-LACE";
		const char *p = m_lores;

		if ( hires ) p = m_hires;
		if ( ham ) p = m_ham;
		if ( dualpf ) p = m_dualpf;
		if ( lace ) p = m_lace;

		if ( hires && lace ) p = m_hilace;

		popmessage("%s(%d pl od=%04x ed=%04x start=%04x stop=%04x)", p, planes, odelay, edelay, CUSTOM_REG(REG_DDFSTRT), CUSTOM_REG(REG_DDFSTOP) );
	}
#endif

	/* end of the line: time to add the modulos */
	if (scanline >= vstart && scanline < vstop)
	{
		int p;

		/* update odd planes */
		for (p = 0; p < planes; p += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + p * 2) += CUSTOM_REG_SIGNED(REG_BPL1MOD);

		/* update even planes */
		for (p = 1; p < planes; p += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + p * 2) += CUSTOM_REG_SIGNED(REG_BPL2MOD);
	}

	/* restore color00 */
	CUSTOM_REG(REG_COLOR00) = save_color0;

#if GUESS_COPPER_OFFSET
	if (cpu_getcurrentframe() % 64 == 0 && scanline == 0)
	{
		if (input_code_pressed(KEYCODE_Q))
			popmessage("%d", wait_offset -= 1);
		if (input_code_pressed(KEYCODE_W))
			popmessage("%d", wait_offset += 1);
	}
#endif
}
