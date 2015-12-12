// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles
/***************************************************************************

    Amiga hardware

    Driver by: Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "includes/amiga.h"



/*************************************
 *
 *  Macros
 *
 *************************************/

#define COPPER_CYCLES_TO_PIXELS(x)      (4 * (x))



/*************************************
 *
 *  Tables
 *
 *************************************/

/* expand an 8-bit bit pattern into 16 bits, every other bit */
const UINT16 amiga_expand_byte[256] =
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

const UINT16 delay[256] =
{
	1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,    /* 0x000 - 0x03e */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x040 - 0x05e */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x060 - 0x07e */
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,                        /* 0x080 - 0x09e */
	1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 0x0a0 - 0x0de */
	/* BPLxPTH/BPLxPTL */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x0e0 - 0x0fe */
	/* BPLCON0-3,BPLMOD1-2 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x100 - 0x11e */
	/* SPRxPTH/SPRxPTL */
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,                        /* 0x120 - 0x13e */
	/* SPRxPOS/SPRxCTL/SPRxDATA/SPRxDATB */
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,    /* 0x140 - 0x17e */
	/* COLORxx */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* 0x180 - 0x1be */
	/* RESERVED */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 /* 0x1c0 - 0x1fe */
};



/*************************************
 *
 *  4-4-4 palette init
 *
 *************************************/

PALETTE_INIT_MEMBER(amiga_state,amiga)
{
	int i;

	for (i = 0; i < 0x1000; i++)
		palette.set_pen_color(i, pal4bit(i >> 8), pal4bit(i >> 4), pal4bit(i));
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER( amiga_state, amiga )
{
	int j;

	/* generate tables that produce the correct playfield color for dual playfield mode */
	for (j = 0; j < 64; j++)
	{
		int pf1pix = ((j >> 0) & 1) | ((j >> 1) & 2) | ((j >> 2) & 4);
		int pf2pix = ((j >> 1) & 1) | ((j >> 2) & 2) | ((j >> 3) & 4);

		m_separate_bitplanes[0][j] = (pf1pix || !pf2pix) ? pf1pix : (pf2pix + 8);
		m_separate_bitplanes[1][j] = pf2pix ? (pf2pix + 8) : pf1pix;
	}

#if GUESS_COPPER_OFFSET
	m_wait_offset = 3;
#endif

	/* reset the genlock color */
	m_genlock_color = 0xffff;

	m_sprite_ctl_written = 0;

	m_screen->register_screen_bitmap(m_flickerfixer);
}



/*************************************
 *
 *  Beam position
 *
 *************************************/

UINT32 amiga_state::amiga_gethvpos()
{
	amiga_state *state = this;
	UINT32 hvpos = (m_last_scanline << 8) | (m_screen->hpos() >> 2);
	UINT32 latchedpos = m_hvpos ? m_hvpos->read() : 0;

	/* if there's no latched position, or if we are in the active display area */
	/* but before the latching point, return the live HV position */
	if ((CUSTOM_REG(REG_BPLCON0) & 0x0008) == 0 || latchedpos == 0 || (m_last_scanline >= 20 && hvpos < latchedpos))
		return hvpos;

	/* otherwise, return the latched position */
	return latchedpos;
}



/*************************************
 *
 *  Genlock interaction
 *
 *************************************/

void amiga_set_genlock_color(running_machine &machine, UINT16 color)
{
	amiga_state *state = machine.driver_data<amiga_state>();

	state->m_genlock_color = color;
}



/*************************************
 *
 *  Copper emulation
 *
 *************************************/

void amiga_copper_setpc(running_machine &machine, UINT32 pc)
{
	amiga_state *state = machine.driver_data<amiga_state>();

	if (LOG_COPPER)
		state->logerror("copper_setpc(%06x)\n", pc);

	state->m_copper_pc = pc;
	state->m_copper_waiting = FALSE;
}


int amiga_copper_execute_next(running_machine &machine, int xpos)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	UINT8 ypos = state->m_last_scanline & 0xff;
	int word0, word1;

	/* bail if not enabled */
	if ((CUSTOM_REG(REG_DMACON) & (DMACON_COPEN | DMACON_DMAEN)) != (DMACON_COPEN | DMACON_DMAEN))
		return 511;

	/* flush any pending writes */
	if (state->m_copper_pending_offset)
	{
		if (LOG_COPPER)
			state->logerror("%02X.%02X: Write to %s = %04x\n", state->m_last_scanline, xpos / 2, amiga_custom_names[state->m_copper_pending_offset & 0xff], state->m_copper_pending_data);
		state->custom_chip_w(state->m_copper_pending_offset, state->m_copper_pending_data);
		state->m_copper_pending_offset = 0;
	}

	/* if we're waiting, check for a breakthrough */
	if (state->m_copper_waiting)
	{
		int curpos = (ypos << 8) | (xpos >> 1);

		/* if we're past the wait time, stop it and hold up 2 cycles */
		if ((curpos & state->m_copper_waitmask) >= (state->m_copper_waitval & state->m_copper_waitmask) &&
			(!state->m_copper_waitblit || !(CUSTOM_REG(REG_DMACON) & DMACON_BBUSY)))
		{
			state->m_copper_waiting = FALSE;
#if GUESS_COPPER_OFFSET
			return xpos + COPPER_CYCLES_TO_PIXELS(1 + state->m_wait_offset);
#else
			return xpos + COPPER_CYCLES_TO_PIXELS(1 + 3);
#endif
		}

		/* otherwise, see if this line is even a possibility; if not, punt */
		if (((curpos | 0xff) & state->m_copper_waitmask) < (state->m_copper_waitval & state->m_copper_waitmask))
			return 511;

		/* else just advance another pixel */
		xpos += COPPER_CYCLES_TO_PIXELS(1);
		return xpos;
	}

	/* fetch the first data word */
	word0 = state->chip_ram_r(state->m_copper_pc);
	state->m_copper_pc += 2;
	xpos += COPPER_CYCLES_TO_PIXELS(1);

	/* fetch the second data word */
	word1 = state->chip_ram_r(state->m_copper_pc);
	state->m_copper_pc += 2;
	xpos += COPPER_CYCLES_TO_PIXELS(1);

	if (LOG_COPPER)
		state->logerror("%02X.%02X: Copper inst @ %06x = %04x %04x\n", state->m_last_scanline, xpos / 2, state->m_copper_pc, word0, word1);

	/* handle a move */
	if ((word0 & 1) == 0)
	{
		int min = (CUSTOM_REG(REG_COPCON) & 2) ? 0x20 : 0x40;

		/* do the write if we're allowed */
		word0 = (word0 >> 1) & 0xff;
		if (word0 >= min)
		{
			if (delay[word0] == 0)
			{
				if (LOG_COPPER)
					state->logerror("%02X.%02X: Write to %s = %04x\n", state->m_last_scanline, xpos / 2, amiga_custom_names[word0 & 0xff], word1);
				state->custom_chip_w(word0, word1);
			}
			else    // additional 2 cycles needed for non-Agnus registers
			{
				state->m_copper_pending_offset = word0;
				state->m_copper_pending_data = word1;
			}
		}

		/* illegal writes suspend until next frame */
		else
		{
			if (LOG_COPPER)
				state->logerror("%02X.%02X: Aborting copper on illegal write\n", state->m_last_scanline, xpos / 2);

			state->m_copper_waitval = 0xffff;
			state->m_copper_waitmask = 0xffff;
			state->m_copper_waitblit = FALSE;
			state->m_copper_waiting = TRUE;

			return 511;
		}
	}
	else
	{
		/* extract common wait/skip values */
		state->m_copper_waitval = word0 & 0xfffe;

#if 0
		if (state->m_copper_waitval != 0xfffe)
			state->m_copper_waitval = (word0 & 0x00fe) | ((((word0 >> 8) & 0xff) + 1) << 8);
#endif

		state->m_copper_waitmask = word1 | 0x8001;
		state->m_copper_waitblit = (~word1 >> 15) & 1;

		/* handle a wait */
		if ((word1 & 1) == 0)
		{
			if (LOG_COPPER)
				state->logerror("  Waiting for %04x & %04x (currently %04x)\n", state->m_copper_waitval, state->m_copper_waitmask, (state->m_last_scanline << 8) | (xpos >> 1));

			state->m_copper_waiting = TRUE;
		}

		/* handle a skip */
		else
		{
			int curpos = (ypos << 8) | (xpos >> 1);

			if (LOG_COPPER)
				state->logerror("  Skipping if %04x & %04x (currently %04x)\n", state->m_copper_waitval, state->m_copper_waitmask, (state->m_last_scanline << 8) | (xpos >> 1));

			/* if we're past the wait time, stop it and hold up 2 cycles */
			if ((curpos & state->m_copper_waitmask) >= (state->m_copper_waitval & state->m_copper_waitmask) &&
				(!state->m_copper_waitblit || !(CUSTOM_REG(REG_DMACON) & DMACON_BBUSY)))
			{
				if (LOG_COPPER)
					state->logerror("  Skipped\n");

				/* count the cycles it out have taken to fetch the next instruction */
				state->m_copper_pc += 4;
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

void amiga_sprite_dma_reset(running_machine &machine, int which)
{
	amiga_state *state = machine.driver_data<amiga_state>();

	if (LOG_SPRITE_DMA) state->logerror("sprite %d dma reset\n", which );
	state->m_sprite_dma_reload_mask |= 1 << which;
	state->m_sprite_dma_live_mask |= 1 << which;
}


void amiga_sprite_enable_comparitor(running_machine &machine, int which, int enable)
{
	amiga_state *state = machine.driver_data<amiga_state>();

	if (LOG_SPRITE_DMA) state->logerror("sprite %d comparitor %sable\n", which, enable ? "en" : "dis" );
	if (enable)
	{
		state->m_sprite_comparitor_enable_mask |= 1 << which;
		state->m_sprite_dma_live_mask &= ~(1 << which);
	}
	else
	{
		state->m_sprite_comparitor_enable_mask &= ~(1 << which);
		state->m_sprite_ctl_written |= (1 << which);
	}
}



/*************************************
 *
 *  Per-scanline sprite fetcher
 *
 *************************************/

static inline void fetch_sprite_data(amiga_state *state, int scanline, int sprite)
{
	CUSTOM_REG(REG_SPR0DATA + 4 * sprite) = state->chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 0);
	CUSTOM_REG(REG_SPR0DATB + 4 * sprite) = state->chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) + 2);
	CUSTOM_REG_LONG(REG_SPR0PTH + 2 * sprite) += 4;
	if (LOG_SPRITE_DMA) state->logerror("%3d:sprite %d fetch: data=%04X-%04X\n", scanline, sprite, CUSTOM_REG(REG_SPR0DATA + 4 * sprite), CUSTOM_REG(REG_SPR0DATB + 4 * sprite));
}

static void update_sprite_dma(amiga_state *state, int scanline)
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
		if (dmaenable && (state->m_sprite_dma_live_mask & bitmask) && (state->m_sprite_dma_reload_mask & bitmask))
		{
			/* disable the sprite */
			state->m_sprite_comparitor_enable_mask &= ~bitmask;
			state->m_sprite_dma_reload_mask &= ~bitmask;

			/* fetch data into the control words */
			CUSTOM_REG(REG_SPR0POS + 4 * num) = state->chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 0);
			CUSTOM_REG(REG_SPR0CTL + 4 * num) = state->chip_ram_r(CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) + 2);
			CUSTOM_REG_LONG(REG_SPR0PTH + 2 * num) += 4;
			if (LOG_SPRITE_DMA) state->logerror("%3d:sprite %d fetch: pos=%04X ctl=%04X\n", scanline, num, CUSTOM_REG(REG_SPR0POS + 4 * num), CUSTOM_REG(REG_SPR0CTL + 4 * num));
		}

		/* compute vstart/vstop */
		vstart = (CUSTOM_REG(REG_SPR0POS + 4 * num) >> 8) | ((CUSTOM_REG(REG_SPR0CTL + 4 * num) << 6) & 0x100);
		vstop = (CUSTOM_REG(REG_SPR0CTL + 4 * num) >> 8) | ((CUSTOM_REG(REG_SPR0CTL + 4 * num) << 7) & 0x100);

		/* if we hit vstart, enable the comparitor */
		if (scanline == vstart)
		{
			state->m_sprite_comparitor_enable_mask |= 1 << num;
			if (LOG_SPRITE_DMA) state->logerror("%3d:sprite %d comparitor enable\n", scanline, num);
		}

		/* if we hit vstop, disable the comparitor and trigger a reload for the next scanline */
		if (scanline == vstop)
		{
			state->m_sprite_ctl_written &= ~bitmask;
			state->m_sprite_comparitor_enable_mask &= ~bitmask;
			state->m_sprite_dma_reload_mask |= 1 << num;
			CUSTOM_REG(REG_SPR0DATA + 4 * num) = 0;     /* just a guess */
			CUSTOM_REG(REG_SPR0DATB + 4 * num) = 0;
			if (LOG_SPRITE_DMA) state->logerror("%3d:sprite %d comparitor disable, prepare for reload\n", scanline, num);
		}

		/* fetch data if this sprite is enabled */
		if (dmaenable && (state->m_sprite_dma_live_mask & bitmask) && (state->m_sprite_comparitor_enable_mask & bitmask))
		{
			fetch_sprite_data(state, scanline, num);
		}
	}
}



/*************************************
 *
 *  Per-pixel sprite computations
 *
 *************************************/

static inline UINT32 interleave_sprite_data(UINT16 lobits, UINT16 hibits)
{
	return (amiga_expand_byte[lobits & 0xff] << 0) | (amiga_expand_byte[lobits >> 8] << 16) |
			(amiga_expand_byte[hibits & 0xff] << 1) | (amiga_expand_byte[hibits >> 8] << 17);
}


static int get_sprite_pixel(amiga_state *state, int x)
{
	int pixels = 0;
	int num, pair;

	/* loop over sprite channels */
	for (num = 0; num < 8; num++)
		if (state->m_sprite_comparitor_enable_mask & (1 << num))
		{
			/* if we're not currently clocking, check against hstart */
			if (state->m_sprite_remain[num] == 0)
			{
				int hstart = ((CUSTOM_REG(REG_SPR0POS + 4 * num) & 0xff) << 1) | (CUSTOM_REG(REG_SPR0CTL + 4 * num) & 1);
				if (hstart == x)
				{
					state->m_sprite_remain[num] = 16;
					state->m_sprite_shiftreg[num] = interleave_sprite_data(CUSTOM_REG(REG_SPR0DATA + 4 * num), CUSTOM_REG(REG_SPR0DATB + 4 * num));
				}
			}

			/* clock the next pixel if we're doing it */
			if (state->m_sprite_remain[num] != 0)
			{
				state->m_sprite_remain[num]--;
				pixels |= (state->m_sprite_shiftreg[num] & 0xc0000000) >> (16 + 2 * (7 - num));
				state->m_sprite_shiftreg[num] <<= 2;
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
		const int esprm = 0x10, osprm = 0x10;

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
				UINT32 result = (collide << 6) | (pair << 10);

				/* attached case */
				if (CUSTOM_REG(REG_SPR1CTL + 8 * pair) & 0x0080)
					return (pixels & 0xf) | osprm | result;

				/* lower-numbered sprite of pair */
				else if (pixels & 3)
					return (pixels & 3) | esprm | (pair << 2) | result;

				/* higher-numbered sprite of pair */
				else
					return ((pixels >> 2) & 3) | osprm | (pair << 2) | result;
			}
	}

	return 0;
}



/*************************************
 *
 *  Bitplane assembly
 *
 *************************************/

static inline UINT8 assemble_odd_bitplanes(amiga_state *state, int planes, int obitoffs)
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


static inline UINT8 assemble_even_bitplanes(amiga_state *state, int planes, int ebitoffs)
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

static inline void fetch_bitplane_data(amiga_state *state, int plane)
{
	CUSTOM_REG(REG_BPL1DAT + plane) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2));
	CUSTOM_REG_LONG(REG_BPL1PTH + plane * 2) += 2;
}


/*************************************
 *
 *  Hold and modify pixel computations
 *
 *************************************/

static inline int update_ham(amiga_state *state, int newpix)
{
	switch (newpix >> 4)
	{
		case 0:
			state->m_ham_color = CUSTOM_REG(REG_COLOR00 + (newpix & 0xf));
			break;

		case 1:
			state->m_ham_color = (state->m_ham_color & 0xff0) | ((newpix & 0xf) << 0);
			break;

		case 2:
			state->m_ham_color = (state->m_ham_color & 0x0ff) | ((newpix & 0xf) << 8);
			break;

		case 3:
			state->m_ham_color = (state->m_ham_color & 0xf0f) | ((newpix & 0xf) << 4);
			break;
	}
	return state->m_ham_color;
}


//**************************************************************************
//  DISPLAY WINDOW
//**************************************************************************

void amiga_state::update_display_window()
{
	amiga_state *state = this;

	int vstart = CUSTOM_REG(REG_DIWSTRT) >> 8;
	int vstop = CUSTOM_REG(REG_DIWSTOP) >> 8;
	int hstart = CUSTOM_REG(REG_DIWSTRT) & 0xff;
	int hstop = CUSTOM_REG(REG_DIWSTOP) & 0xff;

	if (m_diwhigh_valid)
	{
		vstart |= (CUSTOM_REG(REG_DIWHIGH) & 7) << 8;
		vstop  |= ((CUSTOM_REG(REG_DIWHIGH) >> 8) & 7) << 8;
		hstart |= ((CUSTOM_REG(REG_DIWHIGH) >> 5) & 1) << 8;
		hstop  |= ((CUSTOM_REG(REG_DIWHIGH) >> 13) & 1) << 8;
	}
	else
	{
		vstop |= ((~CUSTOM_REG(REG_DIWSTOP) >> 7) & 0x100);
		hstop |= 0x100;
	}

	if (hstop < hstart)
	{
		hstart = 0x00;
		hstop = 0x1ff;
	}

	m_diw.set(hstart, hstop, vstart, vstop);
}


/*************************************
 *
 *  Single scanline rasterizer
 *
 *************************************/

void amiga_state::render_scanline(bitmap_ind16 &bitmap, int scanline)
{
	amiga_state *state = this;
	UINT16 save_color0 = CUSTOM_REG(REG_COLOR00);
	int ddf_start_pixel = 0, ddf_stop_pixel = 0;
	int hires = 0, dualpf = 0, ham = 0;
	int pf1pri = 0, pf2pri = 0;
	int planes = 0;

	UINT16 *dst = nullptr;
	int ebitoffs = 0, obitoffs = 0;
	int ecolmask = 0, ocolmask = 0;
	int edelay = 0, odelay = 0;
	int next_copper_x;
	int pl;
	const int defbitoffs = 15;

	int save_scanline = scanline;

	// we need to do a bit more work on the first scanline
	if (scanline == 0)
	{
		m_previous_lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		// toggle lof if enabled
		if (CUSTOM_REG(REG_BPLCON0) & BPLCON0_LACE)
			CUSTOM_REG(REG_VPOSR) ^= VPOSR_LOF;

		// reset copper and ham color
		amiga_copper_setpc(machine(), CUSTOM_REG_LONG(REG_COP1LCH));
		m_ham_color = CUSTOM_REG(REG_COLOR00);
	}

	// in visible area?
	if (bitmap.valid())
	{
		bool lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		if ((scanline & 1) ^ lof)
		{
			// lof matches? then render this scanline
			dst = &bitmap.pix16(scanline);
		}
		else
		{
			// lof doesn't match, we don't render this scanline
			// if we didn't switch lof we have a full non-interlace screen,
			// so we fill the black gaps with the contents of the previous scanline
			// otherwise just render the contents of the previous frame's scanline
			int shift = (m_previous_lof == lof) ? 1 : 0;

			memcpy(&bitmap.pix16(scanline), &m_flickerfixer.pix16(scanline - shift), amiga_state::SCREEN_WIDTH * 2);
			return;
		}
	}

	scanline /= 2;

	m_last_scanline = scanline;

	/* update sprite data fetching */
	update_sprite_dma(state, scanline);

	/* all sprites off at the start of the line */
	memset(m_sprite_remain, 0, sizeof(m_sprite_remain));

	/* temporary set color 0 to the genlock color */
	if (m_genlock_color != 0xffff)
		CUSTOM_REG(REG_COLOR00) = m_genlock_color;

	/* loop over the line */
	next_copper_x = 0;
	for (int x = 0; x < amiga_state::SCREEN_WIDTH / 2; x++)
	{
		int sprpix;

		/* time to execute the copper? */
		if (x == next_copper_x)
		{
			/* execute the next batch, restoring and re-saving color 0 around it */
			CUSTOM_REG(REG_COLOR00) = save_color0;
			next_copper_x = amiga_copper_execute_next(machine(), x);
			save_color0 = CUSTOM_REG(REG_COLOR00);
			if (m_genlock_color != 0xffff)
				CUSTOM_REG(REG_COLOR00) = m_genlock_color;

			/* compute update-related register values */
			planes = (CUSTOM_REG(REG_BPLCON0) & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) >> 12;
			hires = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HIRES;
			ham = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HOMOD;
			dualpf = CUSTOM_REG(REG_BPLCON0) & BPLCON0_DBLPF;

			/* compute the pixel fetch parameters */
			ddf_start_pixel = (CUSTOM_REG(REG_DDFSTRT) & (hires ? 0xfc : 0xf8)) * 2;
			ddf_start_pixel += hires ? 9 : 17;
			ddf_stop_pixel = (CUSTOM_REG(REG_DDFSTOP) & (hires ? 0xfc : 0xf8)) * 2;
			ddf_stop_pixel += hires ? (9 + defbitoffs) : (17 + defbitoffs);

			if ( ( CUSTOM_REG(REG_DDFSTRT) ^ CUSTOM_REG(REG_DDFSTOP) ) & 0x04 )
				ddf_stop_pixel += 8;

			// display window
			update_display_window();

			/* extract playfield priorities */
			pf1pri = CUSTOM_REG(REG_BPLCON2) & 7;
			pf2pri = (CUSTOM_REG(REG_BPLCON2) >> 3) & 7;

			/* extract collision masks */
			ocolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x15;
			ecolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x2a;
		}

		/* clear the target pixels to the background color as a starting point */
		if (dst != nullptr)
			dst[x*2+0] =
			dst[x*2+1] = CUSTOM_REG(REG_COLOR00);

		/* if we hit the first fetch pixel, reset the counters and latch the delays */
		if (x == ddf_start_pixel)
		{
			odelay = CUSTOM_REG(REG_BPLCON1) & 0xf;
			edelay = ( CUSTOM_REG(REG_BPLCON1) >> 4 ) & 0x0f;

			if ( hires )
			{
				obitoffs = defbitoffs + ( odelay << 1 );
				ebitoffs = defbitoffs + ( edelay << 1 );
			}
			else
			{
				if ( CUSTOM_REG(REG_DDFSTRT) & 0x04 )
				{
					odelay = ( odelay + 8 ) & 0x0f;
					edelay = ( edelay + 8 ) & 0x0f;
				}

				obitoffs = defbitoffs + odelay;
				ebitoffs = defbitoffs + edelay;
			}

			for (pl = 0; pl < 6; pl++)
				CUSTOM_REG(REG_BPL1DAT + pl) = 0;
		}

		/* need to run the sprite engine every pixel to ensure display */
		sprpix = get_sprite_pixel(state, x);

		/* to render, we must have bitplane DMA enabled, at least 1 plane, and be within the */
		/* vertical display window */
		if ((CUSTOM_REG(REG_DMACON) & (DMACON_BPLEN | DMACON_DMAEN)) == (DMACON_BPLEN | DMACON_DMAEN) &&
			planes > 0 && scanline >= m_diw.min_y && scanline < m_diw.max_y)
		{
			int pfpix0 = 0, pfpix1 = 0, collide;

			/* fetch the odd bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + odelay)
			{
				/* if we need to fetch more data, do it now */
				if (obitoffs == defbitoffs)
				{
					for (pl = 0; pl < planes; pl += 2)
					{
						fetch_bitplane_data(state, pl);
					}
				}

				/* now assemble the bits */
				pfpix0 |= assemble_odd_bitplanes(state, planes, obitoffs);
				obitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (obitoffs < 0)
					{
						obitoffs = defbitoffs;

						for (pl = 0; pl < planes; pl += 2)
						{
							fetch_bitplane_data(state, pl);
						}
					}

					pfpix1 |= assemble_odd_bitplanes(state, planes, obitoffs);
					obitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x15;

				/* reset bit offsets if needed */
				if (obitoffs < 0)
					obitoffs = defbitoffs;
			}

			/* fetch the even bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + edelay)
			{
				/* if we need to fetch more data, do it now */
				if (ebitoffs == defbitoffs)
				{
					for (pl = 1; pl < planes; pl += 2)
					{
						fetch_bitplane_data(state, pl);
					}
				}

				/* now assemble the bits */
				pfpix0 |= assemble_even_bitplanes(state, planes, ebitoffs);
				ebitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (ebitoffs < 0)
					{
						ebitoffs = defbitoffs;

						for (pl = 1; pl < planes; pl += 2)
						{
							fetch_bitplane_data(state, pl);
						}
					}

					pfpix1 |= assemble_even_bitplanes(state, planes, ebitoffs);
					ebitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x2a;

				/* reset bit offsets if needed */
				if (ebitoffs < 0)
					ebitoffs = defbitoffs;
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
			if (dst != nullptr && x >= m_diw.min_x && x < m_diw.max_x)
			{
				int pix, pri;

				/* hold-and-modify mode -- assume low-res (hi-res not supported by the hardware) */
				if (ham)
				{
					/* update the HAM color */
					pfpix0 = update_ham(state, pfpix0);

					pix = sprpix & 0x1f;
					pri = (sprpix >> 10);

					/* sprite has priority */
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + pix);
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
					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					pri = (sprpix >> 10);
					if (pix)
					{
						if ((pfpix0 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix0 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the left pixel */
					if (pix)
						dst[x*2+0] = CUSTOM_REG(REG_COLOR00 + pix);
					else
						dst[x*2+0] = CUSTOM_REG(REG_COLOR00 + m_separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix0]);

					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					if (pix)
					{
						if ((pfpix1 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix1 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the right pixel */
					if (pix)
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + pix);
					else
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + m_separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix1]);
				}

				/* single playfield mode */
				else
				{
					pix = sprpix & 0x1f;
					pri = (sprpix >> 10);

					/* sprite has priority */
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = CUSTOM_REG(REG_COLOR00 + pix);
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

	// end of the line: time to add the modulos
	if (scanline >= m_diw.min_y && scanline < m_diw.max_y)
	{
		// update odd planes
		for (pl = 0; pl < planes; pl += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += CUSTOM_REG_SIGNED(REG_BPL1MOD);

		// update even planes
		for (pl = 1; pl < planes; pl += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += CUSTOM_REG_SIGNED(REG_BPL2MOD);
	}

	// restore color00
	CUSTOM_REG(REG_COLOR00) = save_color0;

	// save
	if (dst != nullptr)
		memcpy(&m_flickerfixer.pix16(save_scanline), dst, amiga_state::SCREEN_WIDTH * 2);

#if GUESS_COPPER_OFFSET
	if (m_screen->frame_number() % 64 == 0 && scanline == 0)
	{
		if (machine().input().code_pressed(KEYCODE_Q))
			popmessage("%d", m_wait_offset -= 1);
		if (machine().input().code_pressed(KEYCODE_W))
			popmessage("%d", m_wait_offset += 1);
	}
#endif
}



/*************************************
 *
 *  Update
 *
 *************************************/

/* TODO: alg.c requires that this uses RGB32 */
UINT32 amiga_state::screen_update_amiga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// sometimes the core tells us to render a bunch of lines to keep up (resolution change, for example)
	// this causes trouble for us since it can happen at any time
	if (cliprect.min_y != cliprect.max_y)
		return 0;

	// render each scanline in the visible region
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		render_scanline(bitmap, y);

	return 0;
}

void amiga_state::update_screenmode()
{
	amiga_state *state = this;
	bool pal;

	// first let's see if we're PAL or NTSC
	if (m_agnus_id >= AGNUS_HR_PAL)
		// we support dynamic switching between PAL and NTSC, determine mode from register
		pal = CUSTOM_REG(REG_BEAMCON0) & 0x20;
	else
		// old agnus, agnus id determines PAL or NTSC
		pal = !(m_agnus_id & 0x10);

	// basic height & vblank length
	int height = pal ? SCREEN_HEIGHT_PAL : SCREEN_HEIGHT_NTSC;
	int vblank = pal ? VBLANK_PAL : VBLANK_NTSC;

	// frame period
	attoseconds_t period = HZ_TO_ATTOSECONDS(m_screen->clock()) * SCREEN_WIDTH * height;

	// adjust visible area
	rectangle visarea = m_screen->visible_area();
	visarea.sety(vblank, height - 1);

	// finally set our new mode
	m_screen->configure(SCREEN_WIDTH, height, visarea, period);
}


//**************************************************************************
//  MACHINE DRIVER FRAGMENTS
//**************************************************************************

MACHINE_CONFIG_FRAGMENT( pal_video )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS
	(
		(amiga_state::CLK_28M_PAL / 4) * 2 * 2,
		amiga_state::SCREEN_WIDTH, amiga_state::HBLANK, amiga_state::SCREEN_WIDTH,
		amiga_state::SCREEN_HEIGHT_PAL, amiga_state::VBLANK_PAL, amiga_state::SCREEN_HEIGHT_PAL
	)
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( ntsc_video )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS
	(
		(amiga_state::CLK_28M_NTSC / 4) * 2 * 2,
		amiga_state::SCREEN_WIDTH, amiga_state::HBLANK, amiga_state::SCREEN_WIDTH,
		amiga_state::SCREEN_HEIGHT_NTSC, amiga_state::VBLANK_NTSC, amiga_state::SCREEN_HEIGHT_NTSC
	)
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END
