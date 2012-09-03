/***************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

  This chip is used in:
  - shanghai.c
  - adp.c
  - sigmab52.c
  - wildpkr.c

  ACRTC memory map:

  00000-3ffff = RAM
  40000-7ffff = ROM     handled with a hack in the drivers
  80000-bffff = unused
  c0000-fffff = unused

***************************************************************************/

#include "emu.h"
#include "video/hd63484.h"

#define LOG_COMMANDS 0

/* the on-chip FIFO is 16 bytes long, but we use a larger one to simplify */
/* decoding of long commands. Commands can be up to 64KB long... but Shanghai */
/* doesn't reach that length. */

#define FIFO_LENGTH 256

typedef struct _hd63484_state hd63484_state;
struct _hd63484_state
{
	UINT16 *   ram;
	UINT16 reg[256/2];

	int          fifo_counter;
	UINT16       fifo[FIFO_LENGTH];
	UINT16       readfifo;

	UINT16       pattern[16];
	int          org, org_dpd, rwp;
	UINT16       cl0, cl1, ccmp, edg, mask, ppy, pzcy, ppx, pzcx, psy, psx, pey, pzy, pex, pzx, xmin, ymin, xmax, ymax, rwp_dn;
	INT16        cpx, cpy;

	int          regno;

	int          skattva_hack;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE hd63484_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == HD63484);

	return (hd63484_state *)downcast<hd63484_device *>(device)->token();
}

INLINE const hd63484_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == HD63484);
	return (const hd63484_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static int get_pixel(device_t *device, int x, int y);

static const int instruction_length[64] =
{
	 0, 3, 2, 1,	/* 0x */
	 0, 0,-1, 2,	/* 1x */
	 0, 3, 3, 3,	/* 2x */
	 0, 0, 0, 0,	/* 3x */
	 0, 1, 2, 2,	/* 4x */
	 0, 0, 4, 4,	/* 5x */
	 5, 5, 5, 5,	/* 6x */
	 5, 5, 5, 5,	/* 7x */
	 3, 3, 3, 3,	/* 8x */
	 3, 3,-2,-2,	/* 9x */
	-2,-2, 2, 4,	/* Ax */
	 5, 5, 7, 7,	/* Bx */
	 3, 3, 1, 1,	/* Cx */
	 2, 2, 2, 2,	/* Dx */
	 5, 5, 5, 5,	/* Ex */
	 5, 5, 5, 5 	/* Fx */
};

static const char *const instruction_name[64] =
{
	"undef","ORG  ","WPR  ","RPR  ",	/* 0x */
	"undef","undef","WPTN ","RPTN ",	/* 1x */
	"undef","DRD  ","DWT  ","DMOD ",	/* 2x */
	"undef","undef","undef","undef",	/* 3x */
	"undef","RD   ","WT   ","MOD  ",	/* 4x */
	"undef","undef","CLR  ","SCLR ",	/* 5x */
	"CPY  ","CPY  ","CPY  ","CPY  ",	/* 6x */
	"SCPY ","SCPY ","SCPY ","SCPY ",	/* 7x */
	"AMOVE","RMOVE","ALINE","RLINE",	/* 8x */
	"ARCT ","RRCT ","APLL ","RPLL ",	/* 9x */
	"APLG ","RPLG ","CRCL ","ELPS ",	/* Ax */
	"AARC ","RARC ","AEARC","REARC",	/* Bx */
	"AFRCT","RFRCT","PAINT","DOT  ",	/* Cx */
	"PTN  ","PTN  ","PTN  ","PTN  ",	/* Dx */
	"AGCPY","AGCPY","AGCPY","AGCPY",	/* Ex */
	"RGCPY","RGCPY","RGCPY","RGCPY" 	/* Fx */
};

static void doclr16( device_t *device, int opcode, UINT16 fill, int *dst, INT16 _ax, INT16 _ay )
{
	hd63484_state *hd63484 = get_safe_token(device);
	INT16 ax,ay;

	ax = _ax;
	ay = _ay;

	for (;;)
	{
		for (;;)
		{
			switch (opcode & 0x0003)
			{
				case 0:
					hd63484->ram[*dst]  = fill;
					break;
				case 1:
					hd63484->ram[*dst] |= fill;
					break;
				case 2:
					hd63484->ram[*dst] &= fill;
					break;
				case 3:
					hd63484->ram[*dst] ^= fill;
					break;
			}
			if (ax == 0)
				break;
			else if (ax > 0)
			{
				*dst = (*dst + 1) & (HD63484_RAM_SIZE - 1);
				ax--;
			}
			else
			{
				*dst = (*dst - 1) & (HD63484_RAM_SIZE - 1);
				ax++;
			}
		}

		ax = _ax;
		if (_ay < 0)
		{
			*dst = (*dst + (hd63484->reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
			if (ay == 0)
				break;
			ay++;
		}
		else
		{
			*dst = (*dst - (hd63484->reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
			if (ay == 0)
				break;
			ay--;
		}
	}
}

static void docpy16( device_t *device, int opcode, int src, int *dst, INT16 _ax, INT16 _ay )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int dstep1,dstep2;
	int ax = _ax;
	int ay = _ay;

	switch (opcode & 0x0700)
	{
		default:
		case 0x0000: dstep1 =  1; dstep2 = -1 * (hd63484->reg[0xca/2] & 0x0fff) - ax * dstep1; break;
		case 0x0100: dstep1 =  1; dstep2 =      (hd63484->reg[0xca/2] & 0x0fff) - ax * dstep1; break;
		case 0x0200: dstep1 = -1; dstep2 = -1 * (hd63484->reg[0xca/2] & 0x0fff) + ax * dstep1; break;
		case 0x0300: dstep1 = -1; dstep2 =      (hd63484->reg[0xca/2] & 0x0fff) + ax * dstep1; break;
		case 0x0400: dstep1 = -1 * (hd63484->reg[0xca/2] & 0x0fff); dstep2 =  1 - ay * dstep1; break;
		case 0x0500: dstep1 =      (hd63484->reg[0xca/2] & 0x0fff); dstep2 =  1 - ay * dstep1; break;
		case 0x0600: dstep1 = -1 * (hd63484->reg[0xca/2] & 0x0fff); dstep2 = -1 + ay * dstep1; break;
		case 0x0700: dstep1 =      (hd63484->reg[0xca/2] & 0x0fff); dstep2 = -1 + ay * dstep1; break;
	}

	for (;;)
	{
		for (;;)
		{
			switch (opcode & 0x0007)
			{
				case 0:
					hd63484->ram[*dst]  = hd63484->ram[src];
					break;
				case 1:
					hd63484->ram[*dst] |= hd63484->ram[src];
					break;
				case 2:
					hd63484->ram[*dst] &= hd63484->ram[src];
					break;
				case 3:
					hd63484->ram[*dst] ^= hd63484->ram[src];
					break;
				case 4:
					if (hd63484->ram[*dst] == (hd63484->ccmp & 0xff))
						hd63484->ram[*dst] = hd63484->ram[src];
					break;
				case 5:
					if (hd63484->ram[*dst] != (hd63484->ccmp & 0xff))
						hd63484->ram[*dst] = hd63484->ram[src];
					break;
				case 6:
					if (hd63484->ram[*dst] < hd63484->ram[src])
						hd63484->ram[*dst] = hd63484->ram[src];
					break;
				case 7:
					if (hd63484->ram[*dst] > hd63484->ram[src])
						hd63484->ram[*dst] = hd63484->ram[src];
					break;
			}

			if (opcode & 0x0800)
			{
				if (ay == 0) break;
				if (_ay > 0)
				{
					src = (src - (hd63484->reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE - 1);
					ay--;
				}
				else
				{
					src = (src + (hd63484->reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE - 1);
					ay++;
				}
			}
			else
			{
				if (ax == 0) break;
				else if (ax > 0)
				{
					src = (src + 1) & (HD63484_RAM_SIZE - 1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE - 1);
					ax--;
				}
				else
				{
					src = (src - 1) & (HD63484_RAM_SIZE - 1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE - 1);
					ax++;
				}
			}
		}

		if (opcode & 0x0800)
		{
			ay = _ay;
			if (_ax < 0)
			{
				src = (src - 1 + ay * (hd63484->reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ax == 0) break;
				ax++;
			}
			else
			{
				src = (src + 1 - ay * (hd63484->reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ax == 0) break;
				ax--;
			}
		}
		else
		{
			ax = _ax;
			if (_ay < 0)
			{
				src = (src + (hd63484->reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ay == 0) break;
				ay++;
			}
			else
			{
				src = (src - (hd63484->reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ay == 0) break;
				ay--;
			}
		}
	}
}

static int org_first_pixel( device_t *device, int _org_dpd )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int gbm = (hd63484->reg[0x02/2] & 0x700) >> 8;

	switch (gbm)
	{
		case 0:
			return (_org_dpd & 0x0f);
		case 1:
			return (_org_dpd & 0x0e) >> 1;
		case 2:
			return (_org_dpd & 0x0c) >> 2;
		case 3:
			return (_org_dpd & 0x08) >> 3;
		case 4:
			return 0;

		default:
			logerror ("Graphic bit mode not supported\n");
			return 0;
	}
}

static void dot( device_t *device, int x, int y, int opm, UINT16 color )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int dst, x_int, x_mod, bpp;
	UINT16 color_shifted, bitmask, bitmask_shifted;

	x += org_first_pixel(device, hd63484->org_dpd);

	switch ((hd63484->reg[0x02/2] & 0x700) >> 8)
	{
		case 0:
			bpp = 1;
			bitmask = 0x0001;
			break;
		case 1:
			bpp = 2;
			bitmask = 0x0003;
			break;
		case 2:
			bpp = 4;
			bitmask = 0x000f;
			break;
		case 3:
			bpp = 8;
			bitmask = 0x00ff;
			break;
		case 4:
			bpp = 16;
			bitmask = 0xffff;
			break;

		default:
			bpp = 0;
			bitmask = 0x0000;
			logerror ("Graphic bit mode not supported\n");
	}

	// bpp = 4;          // for skattva
	// bitmask = 0x000f; // for skattva

	if (x >= 0)
	{
		x_int = x / (16 / bpp);
		x_mod = x % (16 / bpp);
	}
	else
	{
		x_int = x / (16 / bpp);
		x_mod = -1 * (x % (16 / bpp));
		if (x_mod) {
			x_int--;
			x_mod = (16 / bpp) - x_mod;
		}
	}

	color &= bitmask;

	bitmask_shifted = bitmask << (x_mod * bpp);
	color_shifted = color << (x_mod * bpp);

	dst = (hd63484->org + x_int - y * (hd63484->reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);

	switch (opm)
	{
		case 0:
			hd63484->ram[dst] = (hd63484->ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 1:
			hd63484->ram[dst] = hd63484->ram[dst] | color_shifted;
			break;
		case 2:
			hd63484->ram[dst] = hd63484->ram[dst] & ((hd63484->ram[dst] & ~bitmask_shifted) | color_shifted);
			break;
		case 3:
			hd63484->ram[dst] = hd63484->ram[dst] ^ color_shifted;
			break;
		case 4:
			if (get_pixel(device, x, y) == (hd63484->ccmp & bitmask))
				hd63484->ram[dst] = (hd63484->ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 5:
			if (get_pixel(device, x, y) != (hd63484->ccmp & bitmask))
				hd63484->ram[dst] = (hd63484->ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 6:
			if (get_pixel(device, x, y) < (hd63484->cl0 & bitmask))
				hd63484->ram[dst] = (hd63484->ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 7:
			if (get_pixel(device, x, y) > (hd63484->cl0 & bitmask))
				hd63484->ram[dst] = (hd63484->ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
	}
}

static int get_pixel( device_t *device, int x, int y )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int dst, x_int, x_mod, bpp;
	UINT16 bitmask, bitmask_shifted;

	switch ((hd63484->reg[0x02/2] & 0x700) >> 8)
	{
		case 0:
			bpp = 1;
			bitmask = 0x0001;
			break;
		case 1:
			bpp = 2;
			bitmask = 0x0003;
			break;
		case 2:
			bpp = 4;
			bitmask = 0x000f;
			break;
		case 3:
			bpp = 8;
			bitmask = 0x00ff;
			break;
		case 4:
			bpp = 16;
			bitmask = 0xffff;
			break;

		default:
			bpp = 0;
			bitmask = 0x0000;
			logerror ("Graphic bit mode not supported\n");
	}
	if (x >= 0)
	{
		x_int = x / (16 / bpp);
		x_mod = x % (16 / bpp);
	}
	else
	{
		x_int = x / (16 / bpp);
		x_mod = -1 * (x % (16 / bpp));
		if (x_mod) {
			x_int--;
			x_mod = (16 / bpp) - x_mod;
		}
	}

	bitmask_shifted = bitmask << (x_mod * bpp);

	dst = (hd63484->org + x_int - y * (hd63484->reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);

	return ((hd63484->ram[dst] & bitmask_shifted) >> (x_mod * bpp));
}

static int get_pixel_ptn( device_t *device, int x, int y )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int dst, x_int, x_mod, bpp;
	UINT16 bitmask, bitmask_shifted;

	bpp = 1;
	bitmask = 0x0001;

	if (x >= 0)
	{
		x_int = x / (16 / bpp);
		x_mod = x % (16 / bpp);
	}
	else
	{
		x_int = x / (16 / bpp);
		x_mod = -1 * (x % (16 / bpp));
		if (x_mod) {
			x_int--;
			x_mod = (16 / bpp) - x_mod;
		}
	}

	bitmask_shifted = bitmask << (x_mod * bpp);

	dst = (x_int + y * 1);

	if ((hd63484->pattern[dst] & bitmask_shifted) >> (x_mod * bpp))
		return 1;
	else
		return 0;
}

static void agcpy( device_t *device, int opcode, int src_x, int src_y, int dst_x, int dst_y, INT16 _ax, INT16 _ay )
{
	int dst_step1_x,dst_step1_y,dst_step2_x,dst_step2_y;
	int src_step1_x,src_step1_y,src_step2_x,src_step2_y;
	int ax_neg,ay_neg;
	int ax = _ax;
	int ay = _ay;
	int xxs = src_x;
	int yys = src_y;
	int xxd = dst_x;
	int yyd = dst_y;

	if (ax < 0)
		ax_neg = -1;
	else
		ax_neg = 1;
	if (ay < 0)
		ay_neg = -1;
	else
		ay_neg = 1;

	if (opcode & 0x0800)
		switch (opcode & 0x0700)
		{
			default:
			case 0x0000: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ay_neg*ay; dst_step2_y =   1; break;
			case 0x0100: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ay_neg*ay; dst_step2_y =  -1; break;
			case 0x0200: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ay_neg*ay; dst_step2_y =   1; break;
			case 0x0300: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ay_neg*ay; dst_step2_y =  -1; break;
			case 0x0400: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x =  1; dst_step2_y = -ay_neg*ay; break;
			case 0x0500: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x =  1; dst_step2_y =  ay_neg*ay; break;
			case 0x0600: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x = -1; dst_step2_y = -ay_neg*ay; break;
			case 0x0700: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x = -1; dst_step2_y =  ay_neg*ay; break;
		}
	else
		switch (opcode & 0x0700)
		{
			default:
			case 0x0000: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ax_neg*ax; dst_step2_y =   1; break;
			case 0x0100: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ax_neg*ax; dst_step2_y =  -1; break;
			case 0x0200: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ax_neg*ax; dst_step2_y =   1; break;
			case 0x0300: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ax_neg*ax; dst_step2_y =  -1; break;
			case 0x0400: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x =  1; dst_step2_y =  ax_neg*ax; break;
			case 0x0500: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x =  1; dst_step2_y = -ax_neg*ax; break;
			case 0x0600: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x = -1; dst_step2_y =  ax_neg*ax; break;
			case 0x0700: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x = -1; dst_step2_y = -ax_neg*ax; break;
		}

	if ((_ax >= 0) && (_ay >= 0) && ((opcode & 0x0800) == 0x0000))
		{ src_step1_x =  1; src_step1_y =  0; src_step2_x = -ax; src_step2_y =   1; }
	else if ((_ax >= 0) && (_ay < 0) && ((opcode & 0x0800) == 0x0000))
		{ src_step1_x =  1; src_step1_y =  0; src_step2_x = -ax; src_step2_y =  -1; }
	else if ((_ax < 0) && (_ay >= 0) && ((opcode & 0x0800) == 0x0000))
		{ src_step1_x = -1; src_step1_y =  0; src_step2_x = -ax; src_step2_y =   1; }
	else if ((_ax < 0) && (_ay < 0) && ((opcode & 0x0800) == 0x0000))
		{ src_step1_x = -1; src_step1_y =  0; src_step2_x = -ax; src_step2_y =  -1; }
	else if ((_ax >= 0) && (_ay >= 0) && ((opcode & 0x0800) == 0x0800))
		{ src_step1_x =  0; src_step1_y =  1; src_step2_x =   1; src_step2_y = -ay; }
	else if ((_ax >= 0) && (_ay < 0) && ((opcode & 0x0800) == 0x0800))
		{ src_step1_x =  0; src_step1_y = -1; src_step2_x =   1; src_step2_y = -ay; }
	else if ((_ax < 0) && (_ay >= 0) && ((opcode & 0x0800) == 0x0800))
		{ src_step1_x =  0; src_step1_y =  1; src_step2_x =  -1; src_step2_y = -ay; }
	else // ((_ax < 0) && (_ay < 0) && ((opcode & 0x0800) == 0x0800))
		{ src_step1_x =  0; src_step1_y = -1; src_step2_x =  -1; src_step2_y = -ay; }

	for (;;)
	{
		for (;;)
		{
			dot(device, xxd, yyd, opcode & 0x0007, get_pixel(device, xxs, yys));

			if (opcode & 0x0800)
			{
				if (ay == 0) break;
				if (_ay > 0)
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay--;
				}
				else
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay++;
				}
			}
			else
			{
				if (ax == 0) break;
				else if (ax > 0)
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax--;
				}
				else
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax++;
				}
			}
		}

		if (opcode & 0x0800)
		{
			ay = _ay;
			if (_ax < 0)
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ax == 0) break;
				ax++;
			}
			else
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ax == 0) break;
				ax--;
			}
		}
		else
		{
			ax = _ax;
			if (_ay < 0)
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ay == 0) break;
				ay++;
			}
			else
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ay == 0) break;
				ay--;
			}
		}
	}
}

static void ptn( device_t *device, int opcode, int src_x, int src_y, INT16 _ax, INT16 _ay )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int dst_step1_x = 0,dst_step1_y = 0,dst_step2_x = 0,dst_step2_y = 0;
	int src_step1_x,src_step1_y,src_step2_x,src_step2_y;
	int ax = _ax;
	int ay = _ay;
	int ax_neg; //,ay_neg;
	int xxs = src_x;
	int yys = src_y;
	int xxd = hd63484->cpx;
	int yyd = hd63484->cpy;
	int getpixel;

	if (ax < 0)
		ax_neg = -1;
	else
		ax_neg = 1;
/*  if (ay < 0)
        ay_neg = -1;
    else
        ay_neg = 1;*/

	if (opcode & 0x0800)
		switch (opcode & 0x0700)
		{
			default:
			case 0x0000: logerror("PTN: not supported"); break;
			case 0x0100: logerror("PTN: not supported"); break;
			case 0x0200: logerror("PTN: not supported"); break;
			case 0x0300: logerror("PTN: not supported"); break;
			case 0x0400: logerror("PTN: not supported"); break;
			case 0x0500: logerror("PTN: not supported"); break;
			case 0x0600: logerror("PTN: not supported"); break;
			case 0x0700: logerror("PTN: not supported"); break;
		}
	else
		switch (opcode & 0x0700)
		{
			default:
			case 0x0000: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ax_neg*ax; dst_step2_y =  1; break;
			case 0x0100: logerror("PTN: not supported"); break;
			case 0x0200: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x = -1; dst_step2_y = -ax_neg*ax; break;
			case 0x0300: logerror("PTN: not supported"); break;
			case 0x0400: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ax_neg*ax; dst_step2_y = -1; break;
			case 0x0500: logerror("PTN: not supported"); break;
			case 0x0600: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x =  1; dst_step2_y =  ax_neg*ax; break;
			case 0x0700: logerror("PTN: not supported"); break;
		}

	src_step1_x =  1; src_step1_y =  0; src_step2_x = -ax; src_step2_y =  1;

	for (;;)
	{
		for (;;)
		{
			getpixel = get_pixel_ptn(device, xxs, yys);
			switch ((opcode & 0x0018) >> 3)
			{
				case 0x0000:
					if (getpixel)
						dot(device, xxd, yyd, opcode & 0x0007, hd63484->cl1);
					else
						dot(device, xxd, yyd, opcode & 0x0007, hd63484->cl0);
					break;
				case 0x0001:
					if (getpixel)
						dot(device, xxd, yyd, opcode & 0x0007, hd63484->cl1);
					break;
				case 0x0002:
					if (getpixel == 0)
						dot(device, xxd, yyd, opcode & 0x0007, hd63484->cl0);
					break;
				case 0x0003:
					logerror("PTN: not supported");
					break;
			}

			if (opcode & 0x0800)
			{
				if (ay == 0) break;
				if (_ay > 0)
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay--;
				}
				else
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay++;
				}
			}
			else
			{
				if (ax == 0) break;
				else if (ax > 0)
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax--;
				}
				else
				{
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax++;
				}
			}
		}

		if (opcode & 0x0800)
		{
			ay = _ay;
			if (_ax < 0)
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ax == 0) break;
				ax++;
			}
			else
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ax == 0) break;
				ax--;
			}
		}
		else
		{
			ax = _ax;
			if (_ay < 0)
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ay == 0) break;
				ay++;
			}
			else
			{
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if (ay == 0) break;
				ay--;
			}
		}
	}
}

static void line( device_t *device, INT16 sx, INT16 sy, INT16 ex, INT16 ey, INT16 col )
{
	hd63484_state *hd63484 = get_safe_token(device);
	INT16 ax,ay;

	int cpx_t = sx;
	int cpy_t = sy;

	ax = ex - sx;
	ay = ey - sy;

	if (abs(ax) >= abs(ay))
	{
		while (ax)
		{
			dot(device, cpx_t, cpy_t, col & 7, hd63484->cl0);

			if (ax > 0)
			{
				cpx_t++;
				ax--;
			}
			else
			{
				cpx_t--;
				ax++;
			}
			cpy_t = sy + ay * (cpx_t - sx) / (ex - sx);
		}
	}
	else
	{
		while (ay)
		{
			dot(device, cpx_t, cpy_t, col & 7, hd63484->cl0);

			if (ay > 0)
			{
				cpy_t++;
				ay--;
			}
			else
			{
				cpy_t--;
				ay++;
			}
			cpx_t = sx + ax * (cpy_t - sy) / (ey - sy);
		}
	}

}

static void circle( device_t *device, INT16 sx, INT16 sy, UINT16 r, INT16 col )
{
	const float DEG2RAD = 3.14159f/180;
	hd63484_state *hd63484 = get_safe_token(device);
	int i;

	for (i = 0; i < 360 * (r / 10); i++)
	{
		float degInRad = i * DEG2RAD / (r / 10);
		dot(device, sx + cos(degInRad) * r,sy + sin(degInRad) * r, col & 7, hd63484->cl0);
	}
}

static void paint( device_t *device, int sx, int sy, int col )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int getpixel;
	dot(device, sx, sy, 0, col);

	getpixel = get_pixel(device, sx+1,sy);
	switch ((hd63484->reg[0x02/2] & 0x700) >> 8)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
			break;
		case 3:
			getpixel = (getpixel << 8) | (getpixel << 0);
			break;
		case 4:
			break;

		default:
			logerror ("Graphic bit mode not supported\n");
	}
	if ((getpixel != col) && (getpixel != hd63484->edg))
		{
			sx++;
			paint(device, sx, sy, col);
			sx--;
		}

	getpixel = get_pixel(device, sx - 1, sy);
	switch ((hd63484->reg[0x02/2] & 0x700) >> 8)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
			break;
		case 3:
			getpixel = (getpixel << 8) | (getpixel << 0);
			break;
		case 4:
			break;

		default:
			logerror ("Graphic bit mode not supported\n");
	}
	if ((getpixel != col) && (getpixel != hd63484->edg))
		{
			sx--;
			paint(device, sx, sy, col);
			sx++;
		}

	getpixel = get_pixel(device, sx, sy + 1);
	switch ((hd63484->reg[0x02/2] & 0x700) >> 8)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
			break;
		case 3:
			getpixel = (getpixel << 8) | (getpixel << 0);
			break;
		case 4:
			break;

		default:
			logerror ("Graphic bit mode not supported\n");
	}
	if ((getpixel != col) && (getpixel != hd63484->edg))
		{
			sy++;
			paint(device, sx, sy, col);
			sy--;
		}

	getpixel = get_pixel(device, sx, sy - 1);
	switch ((hd63484->reg[0x02/2] & 0x700) >> 8)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
			break;
		case 3:
			getpixel = (getpixel << 8) | (getpixel << 0);
			break;
		case 4:
			break;

		default:
			logerror ("Graphic bit mode not supported\n");
	}
	if ((getpixel != col) && (getpixel != hd63484->edg))
		{
			sy--;
			paint(device, sx, sy, col);
			sy++;
		}
}

static void hd63484_command_w(device_t *device, UINT16 cmd)
{
	hd63484_state *hd63484 = get_safe_token(device);
	int len;

	hd63484->fifo[hd63484->fifo_counter++] = cmd;

	len = instruction_length[hd63484->fifo[0] >> 10];
	if (len == -1)
	{
		if (hd63484->fifo_counter < 2) return;
		else len = hd63484->fifo[1] + 2;
	}
	else if (len == -2)
	{
		if (hd63484->fifo_counter < 2) return;
		else len = 2 * hd63484->fifo[1] + 2;
	}

	if (hd63484->fifo_counter >= len)
	{
#if LOG_COMMANDS
		int i;

		logerror("%s: HD63484 command %s (%04x) ", device->machine().describe_context(), instruction_name[hd63484->fifo[0] >> 10], hd63484->fifo[0]);
		for (i = 1; i < hd63484->fifo_counter; i++)
			logerror("%04x ", hd63484->fifo[i]);
		logerror("\n");
#endif

		if (hd63484->fifo[0] == 0x0400) { /* ORG */
			hd63484->org = ((hd63484->fifo[1] & 0x00ff) << 12) | ((hd63484->fifo[2] & 0xfff0) >> 4);
			hd63484->org_dpd = hd63484->fifo[2] & 0x000f;
		}
		else if ((hd63484->fifo[0] & 0xffe0) == 0x0800)	/* WPR */
		{
			if (hd63484->fifo[0] == 0x0800)
				hd63484->cl0 = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x0801)
				hd63484->cl1 = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x0802)
				hd63484->ccmp = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x0803)
				hd63484->edg = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x0804)
				hd63484->mask = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x0805)
				{
					hd63484->ppy  = (hd63484->fifo[1] & 0xf000) >> 12;
					hd63484->pzcy = (hd63484->fifo[1] & 0x0f00) >> 8;
					hd63484->ppx  = (hd63484->fifo[1] & 0x00f0) >> 4;
					hd63484->pzcx = (hd63484->fifo[1] & 0x000f) >> 0;
				}
			else if (hd63484->fifo[0] == 0x0806)
				{
					hd63484->psy  = (hd63484->fifo[1] & 0xf000) >> 12;
					hd63484->psx  = (hd63484->fifo[1] & 0x00f0) >> 4;
				}
			else if (hd63484->fifo[0] == 0x0807)
				{
					hd63484->pey  = (hd63484->fifo[1] & 0xf000) >> 12;
					hd63484->pzy  = (hd63484->fifo[1] & 0x0f00) >> 8;
					hd63484->pex  = (hd63484->fifo[1] & 0x00f0) >> 4;
					hd63484->pzx  = (hd63484->fifo[1] & 0x000f) >> 0;
				}
			else if (hd63484->fifo[0] == 0x0808)
				hd63484->xmin = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x0809)
				hd63484->ymin = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x080a)
				hd63484->xmax = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x080b)
				hd63484->ymax = hd63484->fifo[1];
			else if (hd63484->fifo[0] == 0x080c)
				{
					hd63484->rwp = (hd63484->rwp & 0x00fff) | ((hd63484->fifo[1] & 0x00ff) << 12);
					hd63484->rwp_dn = (hd63484->fifo[1] & 0xc000) >> 14;
				}
			else if (hd63484->fifo[0] == 0x080d)
				{
					hd63484->rwp = (hd63484->rwp & 0xff000) | ((hd63484->fifo[1] & 0xfff0) >> 4);
				}
			else
				logerror("unsupported register\n");
		}
		else if ((hd63484->fifo[0] & 0xffe0) == 0x0c00)	/* RPR */
		{
			if (hd63484->fifo[0] == 0x0c00)
				hd63484->fifo[1] = hd63484->cl0;
			else if (hd63484->fifo[0] == 0x0c01)
				hd63484->fifo[1] = hd63484->cl1;
			else if (hd63484->fifo[0] == 0x0c02)
				hd63484->fifo[1] = hd63484->ccmp;
			else if (hd63484->fifo[0] == 0x0c03)
				hd63484->fifo[1] = hd63484->edg;
			else if (hd63484->fifo[0] == 0x0c04)
				hd63484->fifo[1] = hd63484->mask;
			else if (hd63484->fifo[0] == 0x0c05)
				{
					hd63484->fifo[1] = (hd63484->ppy << 12) | (hd63484->pzcy << 8) | (hd63484->ppx << 4) | hd63484->pzcx;
				}
			else if (hd63484->fifo[0] == 0x0c06)
				{
					hd63484->fifo[1] = (hd63484->psx << 12) | (hd63484->psx << 4);
				}
			else if (hd63484->fifo[0] == 0x0c07)
				{
					hd63484->fifo[1] = (hd63484->pey << 12) | (hd63484->pzy << 8) | (hd63484->pex << 4) | hd63484->pzx;
				}
			else if (hd63484->fifo[0] == 0x0c08)
				hd63484->fifo[1] = hd63484->xmin;
			else if (hd63484->fifo[0] == 0x0c09)
				hd63484->fifo[1] = hd63484->ymin;
			else if (hd63484->fifo[0] == 0x0c0a)
				hd63484->fifo[1] = hd63484->xmax;
			else if (hd63484->fifo[0] == 0x0c0b)
				hd63484->fifo[1] = hd63484->ymax;
			else if (hd63484->fifo[0] == 0x0c0c)
				{
					hd63484->fifo[1] = (hd63484->rwp_dn << 14) | ((hd63484->rwp >> 12) & 0xff);
				}
			else if (hd63484->fifo[0] == 0x0c0d)
				{
					hd63484->fifo[1] = (hd63484->rwp & 0x0fff) << 4;
				}
			else if (hd63484->fifo[0] == 0x0c10)
				{
					// TODO
				}
			else if (hd63484->fifo[0] == 0x0c11)
				{
					// TODO
				}
			else if (hd63484->fifo[0] == 0x0c12)
				{
					hd63484->fifo[1] = hd63484->cpx;
				}
			else if (hd63484->fifo[0] == 0x0c13)
				{
					hd63484->fifo[1] = hd63484->cpy;
				}
			else
				logerror("unsupported register\n");
		}
		else if ((hd63484->fifo[0] & 0xfff0) == 0x1800)	/* WPTN */
		{
			int i;
			int start = hd63484->fifo[0] & 0x000f;
			int n = hd63484->fifo[1];
			for (i = 0; i < n; i++)
				hd63484->pattern[start + i] = hd63484->fifo[2 + i];
		}
		else if (hd63484->fifo[0] == 0x4400)	/* RD */
		{
			hd63484->readfifo = hd63484->ram[hd63484->rwp];
			hd63484->rwp = (hd63484->rwp + 1) & (HD63484_RAM_SIZE - 1);
		}
		else if (hd63484->fifo[0] == 0x4800)	/* WT */
		{
			hd63484->ram[hd63484->rwp] = hd63484->fifo[1];
			hd63484->rwp = (hd63484->rwp + 1) & (HD63484_RAM_SIZE - 1);
		}
		else if (hd63484->fifo[0] == 0x5800)	/* CLR */
		{
			doclr16(device, hd63484->fifo[0], hd63484->fifo[1], &hd63484->rwp, hd63484->fifo[2], hd63484->fifo[3]);

            {
			int fifo2 = (int)hd63484->fifo[2], fifo3 = (int)hd63484->fifo[3];
			if (fifo2 < 0) fifo2 *= -1;
			if (fifo3 < 0) fifo3 *= -1;
			hd63484->rwp += ((fifo2 + 1) * (fifo3 + 1));
            }

		}
		else if ((hd63484->fifo[0] & 0xfffc) == 0x5c00)	/* SCLR */
		{
			doclr16(device, hd63484->fifo[0], hd63484->fifo[1], &hd63484->rwp, hd63484->fifo[2], hd63484->fifo[3]);

            {
                int fifo2 = (int)hd63484->fifo[2], fifo3 = (int)hd63484->fifo[3];
                if (fifo2 < 0) fifo2 *= -1;
                if (fifo3 < 0) fifo3 *= -1;
                hd63484->rwp += ((fifo2 + 1) * (fifo3 + 1));
            }

		}
		else if ((hd63484->fifo[0] & 0xf0ff) == 0x6000)	/* CPY */
		{
			docpy16(device, hd63484->fifo[0], ((hd63484->fifo[1] & 0x00ff) << 12) | ((hd63484->fifo[2] & 0xfff0) >> 4), &hd63484->rwp, hd63484->fifo[3], hd63484->fifo[4]);

            {
                int fifo2 = (int)hd63484->fifo[2], fifo3 = (int)hd63484->fifo[3];
                if (fifo2 < 0) fifo2 *= -1;
                if (fifo3 < 0) fifo3 *= -1;
                hd63484->rwp += ((fifo2 + 1) * (fifo3 + 1));
            }

		}
		else if ((hd63484->fifo[0] & 0xf0fc) == 0x7000)	/* SCPY */
		{
			docpy16(device, hd63484->fifo[0], ((hd63484->fifo[1] & 0x00ff) << 12) | ((hd63484->fifo[2] & 0xfff0) >> 4), &hd63484->rwp, hd63484->fifo[3], hd63484->fifo[4]);

            {
                int fifo2 = (int)hd63484->fifo[2], fifo3 = (int)hd63484->fifo[3];
                if (fifo2 < 0) fifo2 *= -1;
                if (fifo3 < 0) fifo3 *= -1;
                hd63484->rwp += ((fifo2 + 1) * (fifo3 + 1));
            }

		}
		else if (hd63484->fifo[0] == 0x8000)	/* AMOVE */
		{
			hd63484->cpx = hd63484->fifo[1];
			hd63484->cpy = hd63484->fifo[2];
		}
		else if (hd63484->fifo[0] == 0x8400)	/* RMOVE */
		{
			hd63484->cpx += (INT16)hd63484->fifo[1];
			hd63484->cpy += (INT16)hd63484->fifo[2];
		}
		else if ((hd63484->fifo[0] & 0xff00) == 0x8800)	/* ALINE */
		{
			line(device, hd63484->cpx, hd63484->cpy, hd63484->fifo[1], hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			hd63484->cpx = (INT16)hd63484->fifo[1];
			hd63484->cpy = (INT16)hd63484->fifo[2];
		}
		else if ((hd63484->fifo[0] & 0xff00) == 0x8c00)	/* RLINE */
		{
			line(device, hd63484->cpx, hd63484->cpy, hd63484->cpx + (INT16)hd63484->fifo[1], hd63484->cpy + (INT16)hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			hd63484->cpx += (INT16)hd63484->fifo[1];
			hd63484->cpy += (INT16)hd63484->fifo[2];
		}
		else if ((hd63484->fifo[0] & 0xfff8) == 0x9000)	/* ARCT */
		{
			line(device, hd63484->cpx, hd63484->cpy, (INT16)hd63484->fifo[1], hd63484->cpy, hd63484->fifo[0] & 0xff);
			line(device, (INT16)hd63484->fifo[1], hd63484->cpy, (INT16)hd63484->fifo[1], (INT16)hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			line(device, (INT16)hd63484->fifo[1], (INT16)hd63484->fifo[2], hd63484->cpx, (INT16)hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx, (INT16)hd63484->fifo[2], hd63484->cpx, hd63484->cpy, hd63484->fifo[0] & 0xff);
			hd63484->cpx = (INT16)hd63484->fifo[1];
			hd63484->cpy = (INT16)hd63484->fifo[2];
		}
		else if ((hd63484->fifo[0] & 0xfff8) == 0x9400)	/* RRCT  added*/
		{
			line(device, hd63484->cpx, hd63484->cpy, hd63484->cpx + (INT16)hd63484->fifo[1], hd63484->cpy, hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx + (INT16)hd63484->fifo[1], hd63484->cpy, hd63484->cpx + (INT16)hd63484->fifo[1], hd63484->cpy + (INT16)hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx + (INT16)hd63484->fifo[1], hd63484->cpy + (INT16)hd63484->fifo[2], hd63484->cpx, hd63484->cpy + (INT16)hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx, hd63484->cpy + (INT16)hd63484->fifo[2], hd63484->cpx, hd63484->cpy, hd63484->fifo[0] & 0xff);

			hd63484->cpx += (INT16)hd63484->fifo[1];
			hd63484->cpy += (INT16)hd63484->fifo[2];
		}
		else if ((hd63484->fifo[0] & 0xfff8) == 0xa400)	/* RPLG  added*/
		{
			int nseg, sx, sy, ex, ey;
			sx = hd63484->cpx;
			sy = hd63484->cpy;
			for (nseg = 0; nseg < hd63484->fifo[1]; nseg++)
			{
				ex = sx + (INT16)hd63484->fifo[2 + nseg * 2];
				ey = sy + (INT16)hd63484->fifo[2 + nseg * 2 + 1];
				line(device, sx, sy, ex, ey, hd63484->fifo[0] & 7);
				sx = ex;
				sy = ey;
			}
			line(device, sx, sy, hd63484->cpx, hd63484->cpy, hd63484->fifo[0] & 7);
		}
		else if ((hd63484->fifo[0] & 0xfe00) == 0xa800)	/* CRCL  added*/
		{
			circle(device, hd63484->cpx, hd63484->cpy, hd63484->fifo[1] & 0x1fff, hd63484->fifo[0] & 7); // only 13 bit are used for the radius
		}
		else if ((hd63484->fifo[0] & 0xfff8) == 0xc000)	/* AFRCT */
		{
			INT16 pcx, pcy;
			INT16 ax, ay, xx, yy;


			pcx = hd63484->fifo[1];
			pcy = hd63484->fifo[2];
			ax = pcx - hd63484->cpx;
			ay = pcy - hd63484->cpy;
			xx = hd63484->cpx;
			yy = hd63484->cpy;

			for (;;)
			{
				for (;;)
				{
					dot(device, xx, yy, hd63484->fifo[0] & 0x07, hd63484->cl0);

					if (ax == 0) break;
					else if (ax > 0)
					{
						xx++;
						ax--;
					}
					else
					{
						xx--;
						ax++;
					}
				}

				ax = pcx - hd63484->cpx;
				if (pcy < hd63484->cpy)
				{
					yy--;
					xx -= ax;
					if (ay == 0) break;
					ay++;
				}
				else
				{
					yy++;
					xx -= ax;
					if (ay == 0) break;
					ay--;
				}
			}
		}
		else if ((hd63484->fifo[0] & 0xfff8) == 0xc400)	/* RFRCT  added TODO*/
		{
			line(device, hd63484->cpx, hd63484->cpy, hd63484->cpx + (INT16)hd63484->fifo[1], hd63484->cpy, hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx + hd63484->fifo[1], hd63484->cpy, hd63484->cpx + hd63484->fifo[1], hd63484->cpy + hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx + hd63484->fifo[1], hd63484->cpy + hd63484->fifo[2], hd63484->cpx, hd63484->cpy + hd63484->fifo[2], hd63484->fifo[0] & 0xff);
			line(device, hd63484->cpx, hd63484->cpy + hd63484->fifo[2], hd63484->cpx, hd63484->cpy, hd63484->fifo[0] & 0xff);

			hd63484->cpx = hd63484->cpx + (INT16)hd63484->fifo[1];
			hd63484->cpy = hd63484->cpy + (INT16)hd63484->fifo[2];
		}
		else if (hd63484->fifo[0] == 0xc800)	/* PAINT */
		{
			paint(device, hd63484->cpx, hd63484->cpy, hd63484->cl0);
		}
		else if ((hd63484->fifo[0] & 0xfff8) == 0xcc00)	/* DOT */
		{
			dot(device, hd63484->cpx, hd63484->cpy, hd63484->fifo[0] & 0xff, hd63484->cl0);
		}
		else if ((hd63484->fifo[0] & 0xf000) == 0xd000)	/* PTN (to do) */
		{
			ptn(device, hd63484->fifo[0], hd63484->psx, hd63484->psy, hd63484->pex - hd63484->psx, hd63484->pey - hd63484->psy);

			if ((hd63484->fifo[0] & 0x0800) == 0x0000)
				switch (hd63484->fifo[0] & 0x0700)
				{
					case 0x0000:
						if ((hd63484->pey - hd63484->psy) > 0)
							hd63484->cpy += (hd63484->pey - hd63484->psy);
						else
							hd63484->cpy -= (hd63484->pey - hd63484->psy);
						break;
					case 0x0100:
						// missing
						break;
					case 0x0200:
						if ((hd63484->pey - hd63484->psy) > 0)
							hd63484->cpx += (hd63484->pey - hd63484->psy);
						else
							hd63484->cpx -= (hd63484->pey - hd63484->psy);
						break;
					case 0x0300:
						// missing
						break;
					case 0x0400:
						if ((hd63484->pey - hd63484->psy) > 0)
							hd63484->cpy -= (hd63484->pey - hd63484->psy);
						else
							hd63484->cpy += (hd63484->pey - hd63484->psy);
						break;
					case 0x0500:
						// missing
						break;
					case 0x0600:
						if ((hd63484->pey - hd63484->psy) > 0)
							hd63484->cpx -= (hd63484->pey - hd63484->psy);
						else
							hd63484->cpx += (hd63484->pey - hd63484->psy);
						break;
					case 0x0700:
						// missing
						break;
				}
			else
				{
					// missing
				}
		}
		else if ((hd63484->fifo[0] & 0xf018) == 0xe000)	/* agcpy */
		{
			agcpy(device, hd63484->fifo[0], (INT16)hd63484->fifo[1], (INT16)hd63484->fifo[2], hd63484->cpx, hd63484->cpy, hd63484->fifo[3], hd63484->fifo[4]);

			switch (hd63484->fifo[0] & 0x0700)
			{
				case 0x0000:
					if (hd63484->fifo[4] > 0)
						hd63484->cpy += hd63484->fifo[4];
					else
						hd63484->cpy -= hd63484->fifo[4];
					break;
				case 0x0100:
					if (hd63484->fifo[4] > 0)
						hd63484->cpy -= hd63484->fifo[4];
					else
						hd63484->cpy += hd63484->fifo[4];
					break;
				case 0x0200:
					if (hd63484->fifo[4] > 0)
						hd63484->cpy += hd63484->fifo[4];
					else
						hd63484->cpy -= hd63484->fifo[4];
					break;
				case 0x0300:
					if (hd63484->fifo[4] > 0)
						hd63484->cpy -= hd63484->fifo[4];
					else
						hd63484->cpy += hd63484->fifo[4];
					break;
				case 0x0400:
					if (hd63484->fifo[3] > 0)
						hd63484->cpx += hd63484->fifo[3];
					else
						hd63484->cpx -= hd63484->fifo[3];
					break;
				case 0x0500:
					if (hd63484->fifo[3] > 0)
						hd63484->cpx += hd63484->fifo[3];
					else
						hd63484->cpx -= hd63484->fifo[3];
					break;
				case 0x0600:
					if (hd63484->fifo[3] > 0)
						hd63484->cpx -= hd63484->fifo[3];
					else
						hd63484->cpx += hd63484->fifo[3];
					break;
				case 0x0700:
					if (hd63484->fifo[3] > 0)
						hd63484->cpx -= hd63484->fifo[3];
					else
						hd63484->cpx += hd63484->fifo[3];
					break;
			}
		}
		else
		{
			logerror("unsupported command\n");
			popmessage("unsupported command %s (%04x)", instruction_name[hd63484->fifo[0] >> 10], hd63484->fifo[0]);
		}

		hd63484->fifo_counter = 0;
	}
}

READ16_DEVICE_HANDLER( hd63484_status_r )
{
//  if (cpu_get_pc(&space->device()) != 0xfced6 && cpu_get_pc(&space->device()) != 0xfe1d6)
//      logerror("%05x: HD63484 status read\n",cpu_get_pc(&space->device()));

	return 0xff22 | (device->machine().rand() & 0x0004);	/* write FIFO ready + command end    +  (read FIFO ready or read FIFO not ready) */
}

WRITE16_DEVICE_HANDLER( hd63484_address_w )
{
	hd63484_state *hd63484 = get_safe_token(device);

	/* only low 8 bits are used */
	if (ACCESSING_BITS_0_7)
		hd63484->regno = data;
}

WRITE16_DEVICE_HANDLER( hd63484_data_w )
{
	hd63484_state *hd63484 = get_safe_token(device);

	COMBINE_DATA(&hd63484->reg[hd63484->regno/2]);

	if (hd63484->skattva_hack)
		hd63484->reg[2/2] = (hd63484->reg[2/2] & 0xf8ff) | 0x0200; // hack to set proper color depth in skattva

	if (hd63484->regno & 0x80)
		hd63484->regno += 2;	/* autoincrement */

#if LOG_COMMANDS
//  logerror("PC %05x: HD63484 register %02x write %04x\n", cpu_get_pc(&space->device()), hd63484->regno, hd63484->reg[hd63484->regno/2]);
#endif

	if (hd63484->regno == 0)	/* FIFO */
		hd63484_command_w(device, hd63484->reg[0]);
}

READ16_DEVICE_HANDLER( hd63484_data_r )
{
	hd63484_state *hd63484 = get_safe_token(device);
	int res;

	if (hd63484->regno == 0x80)
		res = device->machine().primary_screen->vpos();
	else if (hd63484->regno == 0)
	{
#if LOG_COMMANDS
//      logerror("%05x: HD63484 read FIFO\n", cpu_get_pc(&space->device()));
#endif
		res = hd63484->readfifo;
	}
	else
	{
#if LOG_COMMANDS
//      logerror("%05x: HD63484 read register %02x\n", cpu_get_pc(&space->device()), hd63484->regno);
#endif
		res = 0;
	}

	return res;
}

READ16_DEVICE_HANDLER( hd63484_ram_r )
{
	hd63484_state *hd63484 = get_safe_token(device);

	return hd63484->ram[offset];
}

READ16_DEVICE_HANDLER( hd63484_regs_r )
{
	hd63484_state *hd63484 = get_safe_token(device);

	return hd63484->reg[offset];
}

WRITE16_DEVICE_HANDLER( hd63484_ram_w )
{
	hd63484_state *hd63484 = get_safe_token(device);

	COMBINE_DATA(&hd63484->ram[offset]);
}

WRITE16_DEVICE_HANDLER( hd63484_regs_w )
{
	hd63484_state *hd63484 = get_safe_token(device);

	COMBINE_DATA(&hd63484->reg[offset]);
}


static DEVICE_START( hd63484 )
{
	hd63484_state *hd63484 = get_safe_token(device);
	const hd63484_interface *intf = get_interface(device);

	hd63484->skattva_hack = intf->skattva_hack;
	hd63484->ram = auto_alloc_array_clear(device->machine(), UINT16, HD63484_RAM_SIZE);

//  device->save_item(NAME(hd63484->clear_bitmap));
//  device->save_pointer(NAME(hd63484->spriteram), 0x1000);
//  device->save_item(NAME(*hd63484->sprites_bitmap));
}

static DEVICE_RESET( hd63484 )
{
	hd63484_state *hd63484 = get_safe_token(device);

	hd63484->fifo_counter = 0;
}

const device_type HD63484 = &device_creator<hd63484_device>;

hd63484_device::hd63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HD63484, "HD63484", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(hd63484_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void hd63484_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd63484_device::device_start()
{
	DEVICE_START_NAME( hd63484 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd63484_device::device_reset()
{
	DEVICE_RESET_NAME( hd63484 )(this);
}


