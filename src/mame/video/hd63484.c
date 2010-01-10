/***************************************************************************

hd63484

this chip is used in:
- shanghai.c
- adp.c

***************************************************************************/

#include "emu.h"
#include "video/hd63484.h"

#define LOG_COMMANDS 0

static int get_pixel(int x,int y);

/* the on-chip FIFO is 16 bytes long, but we use a larger one to simplify */
/* decoding of long commands. Commands can be up to 64KB long... but Shanghai */
/* doesn't reach that length. */

#define FIFO_LENGTH 256

static int fifo_counter;
static UINT16 fifo[FIFO_LENGTH];
static UINT16 readfifo;
UINT16 *HD63484_ram;
UINT16 HD63484_reg[256/2];
static UINT16 pattern[16];
static int org,org_dpd,rwp;
static UINT16 cl0,cl1,ccmp,edg,mask,ppy,pzcy,ppx,pzcx,psy,psx,pey,pzy,pex,pzx,xmin,ymin,xmax,ymax,rwp_dn;
static INT16 cpx,cpy;

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

void HD63484_start(running_machine *machine)
{
	fifo_counter = 0;
	HD63484_ram = auto_alloc_array_clear(machine, UINT16, HD63484_RAM_SIZE);
}

static void doclr16(int opcode,UINT16 fill,int *dst,INT16 _ax,INT16 _ay)
{
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
					HD63484_ram[*dst]  = fill;
					break;
				case 1:
					HD63484_ram[*dst] |= fill;
					break;
				case 2:
					HD63484_ram[*dst] &= fill;
					break;
				case 3:
					HD63484_ram[*dst] ^= fill;
					break;
			}
			if (ax == 0) break;
			else if (ax > 0)
			{
				*dst = (*dst + 1) & (HD63484_RAM_SIZE-1);
				ax--;
			}
			else
			{
				*dst = (*dst - 1) & (HD63484_RAM_SIZE-1);
				ax++;
			}
		}

		ax = _ax;
		if (_ay < 0)
		{
			*dst = (*dst + (HD63484_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE-1);
			if (ay == 0) break;
			ay++;
		}
		else
		{
			*dst = (*dst - (HD63484_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE-1);
			if (ay == 0) break;
			ay--;
		}
	}
}

static void docpy16(int opcode,int src,int *dst,INT16 _ax,INT16 _ay)
{
	int dstep1,dstep2;
	int ax = _ax;
	int ay = _ay;

	switch (opcode & 0x0700)
	{
		default:
		case 0x0000: dstep1 =  1; dstep2 = -1 * (HD63484_reg[0xca/2] & 0x0fff) - ax * dstep1; break;
		case 0x0100: dstep1 =  1; dstep2 =      (HD63484_reg[0xca/2] & 0x0fff) - ax * dstep1; break;
		case 0x0200: dstep1 = -1; dstep2 = -1 * (HD63484_reg[0xca/2] & 0x0fff) + ax * dstep1; break;
		case 0x0300: dstep1 = -1; dstep2 =      (HD63484_reg[0xca/2] & 0x0fff) + ax * dstep1; break;
		case 0x0400: dstep1 = -1 * (HD63484_reg[0xca/2] & 0x0fff); dstep2 =  1 - ay * dstep1; break;
		case 0x0500: dstep1 =      (HD63484_reg[0xca/2] & 0x0fff); dstep2 =  1 - ay * dstep1; break;
		case 0x0600: dstep1 = -1 * (HD63484_reg[0xca/2] & 0x0fff); dstep2 = -1 + ay * dstep1; break;
		case 0x0700: dstep1 =      (HD63484_reg[0xca/2] & 0x0fff); dstep2 = -1 + ay * dstep1; break;
	}

	for (;;)
	{
		for (;;)
		{
			switch (opcode & 0x0007)
			{
				case 0:
					HD63484_ram[*dst]  = HD63484_ram[src];
					break;
				case 1:
					HD63484_ram[*dst] |= HD63484_ram[src];
					break;
				case 2:
					HD63484_ram[*dst] &= HD63484_ram[src];
					break;
				case 3:
					HD63484_ram[*dst] ^= HD63484_ram[src];
					break;
				case 4:
					if (HD63484_ram[*dst] == (ccmp & 0xff))
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
				case 5:
					if (HD63484_ram[*dst] != (ccmp & 0xff))
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
				case 6:
					if (HD63484_ram[*dst] < HD63484_ram[src])
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
				case 7:
					if (HD63484_ram[*dst] > HD63484_ram[src])
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
			}

			if (opcode & 0x0800)
			{
				if (ay == 0) break;
				if (_ay > 0)
				{
					src = (src - (HD63484_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ay--;
				}
				else
				{
					src = (src + (HD63484_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ay++;
				}
			}
			else
			{
				if (ax == 0) break;
				else if (ax > 0)
				{
					src = (src + 1) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ax--;
				}
				else
				{
					src = (src - 1) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ax++;
				}
			}
		}

		if (opcode & 0x0800)
		{
			ay = _ay;
			if (_ax < 0)
			{
				src = (src - 1 + ay * (HD63484_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ax == 0) break;
				ax++;
			}
			else
			{
				src = (src + 1 - ay * (HD63484_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ax == 0) break;
				ax--;
			}
		}
		else
		{
			ax = _ax;
			if (_ay < 0)
			{
				src = (src + (HD63484_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ay == 0) break;
				ay++;
			}
			else
			{
				src = (src - (HD63484_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ay == 0) break;
				ay--;
			}
		}
	}
}

static int org_first_pixel(int _org_dpd)
{
	int gbm = (HD63484_reg[0x02/2] & 0x700) >> 8;

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

static void dot(int x, int y, int opm, UINT16 color)
{
	int dst, x_int, x_mod, bpp;
	UINT16 color_shifted, bitmask, bitmask_shifted;

	x += org_first_pixel(org_dpd);

	switch ((HD63484_reg[0x02/2] & 0x700) >> 8)
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

	dst = (org + x_int - y * (HD63484_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE-1);

	switch (opm)
	{
		case 0:
			HD63484_ram[dst] = (HD63484_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 1:
			HD63484_ram[dst] = HD63484_ram[dst] | color_shifted;
			break;
		case 2:
			HD63484_ram[dst] = HD63484_ram[dst] & ((HD63484_ram[dst] & ~bitmask_shifted) | color_shifted);
			break;
		case 3:
			HD63484_ram[dst] = HD63484_ram[dst] ^ color_shifted;
			break;
		case 4:
			if (get_pixel(x,y) == (ccmp & bitmask))
				HD63484_ram[dst] = (HD63484_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 5:
			if (get_pixel(x,y) != (ccmp & bitmask))
				HD63484_ram[dst] = (HD63484_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 6:
			if (get_pixel(x,y) < (cl0 & bitmask))
				HD63484_ram[dst] = (HD63484_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 7:
			if (get_pixel(x,y) > (cl0 & bitmask))
				HD63484_ram[dst] = (HD63484_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
	}
}

static int get_pixel(int x,int y)
{
	int dst, x_int, x_mod, bpp;
	UINT16 bitmask, bitmask_shifted;

	switch ((HD63484_reg[0x02/2] & 0x700) >> 8)
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

	dst = (org + x_int - y * (HD63484_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE-1);

	return ((HD63484_ram[dst] & bitmask_shifted) >> (x_mod * bpp));
}

static int get_pixel_ptn(int x,int y)
{
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

	if ((pattern[dst] & bitmask_shifted) >> (x_mod * bpp))
		return 1;
	else
		return 0;
}

static void agcpy(int opcode,int src_x,int src_y,int dst_x,int dst_y,INT16 _ax,INT16 _ay)
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
			dot(xxd,yyd,opcode & 0x0007,get_pixel(xxs,yys));

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

static void ptn(int opcode,int src_x,int src_y,INT16 _ax,INT16 _ay)
{
	int dst_step1_x = 0,dst_step1_y = 0,dst_step2_x = 0,dst_step2_y = 0;
	int src_step1_x,src_step1_y,src_step2_x,src_step2_y;
	int ax = _ax;
	int ay = _ay;
	int ax_neg,ay_neg;
	int xxs = src_x;
	int yys = src_y;
	int xxd = cpx;
	int yyd = cpy;
	int getpixel;

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
			getpixel = get_pixel_ptn(xxs,yys);
			switch ((opcode & 0x0018) >> 3)
			{
				case 0x0000:
					if (getpixel)
						dot(xxd,yyd,opcode & 0x0007,cl1);
					else
						dot(xxd,yyd,opcode & 0x0007,cl0);
					break;
				case 0x0001:
					if (getpixel)
						dot(xxd,yyd,opcode & 0x0007,cl1);
					break;
				case 0x0002:
					if (getpixel == 0)
						dot(xxd,yyd,opcode & 0x0007,cl0);
					break;
				case 0x0003:
					logerror("PTN: not supported"); break;
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

static void line(INT16 sx, INT16 sy, INT16 ex, INT16 ey, INT16 col)
{
			INT16 ax,ay;

			int cpx_t=sx;
			int cpy_t=sy;

			ax = ex - sx;
			ay = ey - sy;

			if (abs(ax) >= abs(ay))
			{
				while (ax)
				{
					dot(cpx_t,cpy_t,col & 7,cl0);

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
					dot(cpx_t,cpy_t,col & 7,cl0);

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

static void circle(INT16 sx, INT16 sy, UINT16 r, INT16 col)
{
	const float DEG2RAD = 3.14159f/180;
	int i;
	for (i = 0; i < 360 * (r / 10); i++)
	{
		float degInRad = i * DEG2RAD / (r / 10);
		dot(sx + cos(degInRad) * r,sy + sin(degInRad) * r,col & 7,cl0);
	}
}

static void paint(int sx, int sy, int col)
{
	int getpixel;
	dot(sx,sy,0,col);

	getpixel = get_pixel(sx+1,sy);
	switch ((HD63484_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != edg))
		{
			sx++;
			paint(sx,sy,col);
			sx--;
		}

	getpixel = get_pixel(sx-1,sy);
	switch ((HD63484_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != edg))
		{
			sx--;
			paint(sx,sy,col);
			sx++;
		}

	getpixel = get_pixel(sx,sy+1);
	switch ((HD63484_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != edg))
		{
			sy++;
			paint(sx,sy,col);
			sy--;
		}

	getpixel = get_pixel(sx,sy-1);
	switch ((HD63484_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != edg))
		{
			sy--;
			paint(sx,sy,col);
			sy++;
		}
}

static void HD63484_command_w(running_machine *machine, UINT16 cmd)
{
	int len;

	fifo[fifo_counter++] = cmd;

	len = instruction_length[fifo[0]>>10];
	if (len == -1)
	{
		if (fifo_counter < 2) return;
		else len = fifo[1]+2;
	}
	else if (len == -2)
	{
		if (fifo_counter < 2) return;
		else len = 2*fifo[1]+2;
	}

	if (fifo_counter >= len)
	{
#if LOG_COMMANDS
		int i;

		logerror("%s: HD63484 command %s (%04x) ",cpuexec_describe_context(machine),instruction_name[fifo[0]>>10],fifo[0]);
		for (i = 1;i < fifo_counter;i++)
			logerror("%04x ",fifo[i]);
		logerror("\n");
#endif

		if (fifo[0] == 0x0400) { /* ORG */
			org = ((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4);
			org_dpd = fifo[2] & 0x000f;
		}
		else if ((fifo[0] & 0xffe0) == 0x0800)	/* WPR */
		{
			if (fifo[0] == 0x0800)
				cl0 = fifo[1];
			else if (fifo[0] == 0x0801)
				cl1 = fifo[1];
			else if (fifo[0] == 0x0802)
				ccmp = fifo[1];
			else if (fifo[0] == 0x0803)
				edg = fifo[1];
			else if (fifo[0] == 0x0804)
				mask = fifo[1];
			else if (fifo[0] == 0x0805)
				{
					ppy  = (fifo[1] & 0xf000) >> 12;
					pzcy = (fifo[1] & 0x0f00) >> 8;
					ppx  = (fifo[1] & 0x00f0) >> 4;
					pzcx = (fifo[1] & 0x000f) >> 0;
				}
			else if (fifo[0] == 0x0806)
				{
					psy  = (fifo[1] & 0xf000) >> 12;
					psx  = (fifo[1] & 0x00f0) >> 4;
				}
			else if (fifo[0] == 0x0807)
				{
					pey  = (fifo[1] & 0xf000) >> 12;
					pzy  = (fifo[1] & 0x0f00) >> 8;
					pex  = (fifo[1] & 0x00f0) >> 4;
					pzx  = (fifo[1] & 0x000f) >> 0;
				}
			else if (fifo[0] == 0x0808)
				xmin = fifo[1];
			else if (fifo[0] == 0x0809)
				ymin = fifo[1];
			else if (fifo[0] == 0x080a)
				xmax = fifo[1];
			else if (fifo[0] == 0x080b)
				ymax = fifo[1];
			else if (fifo[0] == 0x080c)
				{
					rwp = (rwp & 0x00fff) | ((fifo[1] & 0x00ff) << 12);
					rwp_dn = (fifo[1] & 0xc000) >> 14;
				}
			else if (fifo[0] == 0x080d)
				{
					rwp = (rwp & 0xff000) | ((fifo[1] & 0xfff0) >> 4);
				}
			else
				logerror("unsupported register\n");
		}
		else if ((fifo[0] & 0xffe0) == 0x0c00)	/* RPR */
		{
			if (fifo[0] == 0x0c00)
				fifo[1] = cl0;
			else if (fifo[0] == 0x0c01)
				fifo[1] = cl1;
			else if (fifo[0] == 0x0c02)
				fifo[1] = ccmp;
			else if (fifo[0] == 0x0c03)
				fifo[1] = edg;
			else if (fifo[0] == 0x0c04)
				fifo[1] = mask;
			else if (fifo[0] == 0x0c05)
				{
					fifo[1] = (ppy << 12) | (pzcy << 8) | (ppx << 4) | pzcx;
				}
			else if (fifo[0] == 0x0c06)
				{
					fifo[1] = (psx << 12) | (psx << 4);
				}
			else if (fifo[0] == 0x0c07)
				{
					fifo[1] = (pey << 12) | (pzy << 8) | (pex << 4) | pzx;
				}
			else if (fifo[0] == 0x0c08)
				fifo[1] = xmin;
			else if (fifo[0] == 0x0c09)
				fifo[1] = ymin;
			else if (fifo[0] == 0x0c0a)
				fifo[1] = xmax;
			else if (fifo[0] == 0x0c0b)
				fifo[1] = ymax;
			else if (fifo[0] == 0x0c0c)
				{
					fifo[1] = (rwp_dn << 14) | ((rwp >> 12) & 0xff);
				}
			else if (fifo[0] == 0x0c0d)
				{
					fifo[1] = (rwp & 0x0fff) << 4;
				}
			else if (fifo[0] == 0x0c10)
				{
					// TODO
				}
			else if (fifo[0] == 0x0c11)
				{
					// TODO
				}
			else if (fifo[0] == 0x0c12)
				{
					fifo[1] = cpx;
				}
			else if (fifo[0] == 0x0c13)
				{
					fifo[1] = cpy;
				}
			else
				logerror("unsupported register\n");
		}
		else if ((fifo[0] & 0xfff0) == 0x1800)	/* WPTN */
		{
			int i;
			int start = fifo[0] & 0x000f;
			int n = fifo[1];
			for (i = 0; i < n; i++)
				pattern[start + i] = fifo[2 + i];
		}
		else if (fifo[0] == 0x4400)	/* RD */
		{
			readfifo = HD63484_ram[rwp];
			rwp = (rwp + 1) & (HD63484_RAM_SIZE-1);
		}
		else if (fifo[0] == 0x4800)	/* WT */
		{
			HD63484_ram[rwp] = fifo[1];
			rwp = (rwp + 1) & (HD63484_RAM_SIZE-1);
		}
		else if (fifo[0] == 0x5800)	/* CLR */
		{
			doclr16(fifo[0],fifo[1],&rwp,fifo[2],fifo[3]);

            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }

		}
		else if ((fifo[0] & 0xfffc) == 0x5c00)	/* SCLR */
		{
			doclr16(fifo[0],fifo[1],&rwp,fifo[2],fifo[3]);

            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }

		}
		else if ((fifo[0] & 0xf0ff) == 0x6000)	/* CPY */
		{
			docpy16(fifo[0],((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4),&rwp,fifo[3],fifo[4]);

            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }

		}
		else if ((fifo[0] & 0xf0fc) == 0x7000)	/* SCPY */
		{
			docpy16(fifo[0],((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4),&rwp,fifo[3],fifo[4]);

            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }

		}
		else if (fifo[0] == 0x8000)	/* AMOVE */
		{
			cpx = fifo[1];
			cpy = fifo[2];
		}
		else if (fifo[0] == 0x8400)	/* RMOVE */
		{
			cpx += (INT16)fifo[1];
			cpy += (INT16)fifo[2];
		}
		else if ((fifo[0] & 0xff00) == 0x8800)	/* ALINE */
		{
			line(cpx,cpy,fifo[1],fifo[2],fifo[0] & 0xff);
			cpx = (INT16)fifo[1];
			cpy = (INT16)fifo[2];
		}
		else if ((fifo[0] & 0xff00) == 0x8c00)	/* RLINE */
		{
			line(cpx,cpy,cpx+(INT16)fifo[1],cpy+(INT16)fifo[2],fifo[0] & 0xff);
			cpx += (INT16)fifo[1];
			cpy += (INT16)fifo[2];
		}
		else if ((fifo[0] & 0xfff8) == 0x9000)	/* ARCT */
		{
			line(cpx,cpy,(INT16)fifo[1],cpy,fifo[0] & 0xff);
			line((INT16)fifo[1],cpy,(INT16)fifo[1],(INT16)fifo[2],fifo[0] & 0xff);
			line((INT16)fifo[1],(INT16)fifo[2],cpx,(INT16)fifo[2],fifo[0] & 0xff);
			line(cpx,(INT16)fifo[2],cpx,cpy,fifo[0] & 0xff);
			cpx = (INT16)fifo[1];
			cpy = (INT16)fifo[2];
		}
		else if ((fifo[0] & 0xfff8) == 0x9400)	/* RRCT  added*/
		{
			line(cpx,cpy,cpx+(INT16)fifo[1],cpy,fifo[0] & 0xff);
			line(cpx+(INT16)fifo[1],cpy,cpx+(INT16)fifo[1],cpy+(INT16)fifo[2],fifo[0] & 0xff);
			line(cpx+(INT16)fifo[1],cpy+(INT16)fifo[2],cpx,cpy+(INT16)fifo[2],fifo[0] & 0xff);
			line(cpx,cpy+(INT16)fifo[2],cpx,cpy,fifo[0] & 0xff);

			cpx += (INT16)fifo[1];
			cpy += (INT16)fifo[2];
		}
		else if ((fifo[0] & 0xfff8) == 0xa400)	/* RPLG  added*/
		{
			int nseg,sx,sy,ex,ey;
			sx = cpx;
			sy = cpy;
			for(nseg=0;nseg<fifo[1];nseg++)
			{
				ex = sx + (INT16)fifo[2+nseg*2];
				ey = sy + (INT16)fifo[2+nseg*2+1];
				line(sx,sy,ex,ey,fifo[0]&7);
				sx = ex;
				sy = ey;
			}
			line(sx,sy,cpx,cpy,fifo[0]&7);
		}
		else if ((fifo[0] & 0xfe00) == 0xa800)	/* CRCL  added*/
		{
			circle(cpx,cpy,fifo[1] & 0x1fff,fifo[0]&7); // only 13 bit are used for the radius
		}
		else if ((fifo[0] & 0xfff8) == 0xc000)	/* AFRCT */
		{
			INT16 pcx,pcy;
			INT16 ax,ay,xx,yy;


			pcx = fifo[1];
			pcy = fifo[2];
			ax = pcx - cpx;
			ay = pcy - cpy;
			xx = cpx;
			yy = cpy;

			for (;;)
			{
				for (;;)
				{
					dot(xx,yy,fifo[0] & 0x07,cl0);

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

				ax = pcx - cpx;
				if (pcy < cpy)
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
		else if ((fifo[0] & 0xfff8) == 0xc400)	/* RFRCT  added TODO*/
		{
			line(cpx,cpy,cpx+(INT16)fifo[1],cpy,fifo[0] & 0xff);
			line(cpx+fifo[1],cpy,cpx+fifo[1],cpy+fifo[2],fifo[0] & 0xff);
			line(cpx+fifo[1],cpy+fifo[2],cpx,cpy+fifo[2],fifo[0] & 0xff);
			line(cpx,cpy+fifo[2],cpx,cpy,fifo[0] & 0xff);

			cpx=cpx+(INT16)fifo[1];
			cpy=cpy+(INT16)fifo[2];
		}
		else if (fifo[0] == 0xc800)	/* PAINT */
		{
			paint(cpx,cpy,cl0);
		}
		else if ((fifo[0] & 0xfff8) == 0xcc00)	/* DOT */
		{
			dot(cpx,cpy,fifo[0] & 0xff,cl0);
		}
		else if ((fifo[0] & 0xf000) == 0xd000)	/* PTN (to do) */
		{
			ptn(fifo[0],psx,psy,pex - psx,pey - psy);

			if ((fifo[0] & 0x0800) == 0x0000)
				switch (fifo[0] & 0x0700)
				{
					case 0x0000:
						if ((pey - psy) > 0)
							cpy += (pey - psy);
						else
							cpy -= (pey - psy);
						break;
					case 0x0100:
						// missing
						break;
					case 0x0200:
						if ((pey - psy) > 0)
							cpx += (pey - psy);
						else
							cpx -= (pey - psy);
						break;
					case 0x0300:
						// missing
						break;
					case 0x0400:
						if ((pey - psy) > 0)
							cpy -= (pey - psy);
						else
							cpy += (pey - psy);
						break;
					case 0x0500:
						// missing
						break;
					case 0x0600:
						if ((pey - psy) > 0)
							cpx -= (pey - psy);
						else
							cpx += (pey - psy);
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
		else if ((fifo[0] & 0xf018) == 0xe000)	/* AGCPY */
		{
			agcpy(fifo[0],(INT16)fifo[1],(INT16)fifo[2],cpx,cpy,fifo[3],fifo[4]);

			switch (fifo[0] & 0x0700)
			{
				case 0x0000:
					if (fifo[4] > 0)
						cpy += fifo[4];
					else
						cpy -= fifo[4];
					break;
				case 0x0100:
					if (fifo[4] > 0)
						cpy -= fifo[4];
					else
						cpy += fifo[4];
					break;
				case 0x0200:
					if (fifo[4] > 0)
						cpy += fifo[4];
					else
						cpy -= fifo[4];
					break;
				case 0x0300:
					if (fifo[4] > 0)
						cpy -= fifo[4];
					else
						cpy += fifo[4];
					break;
				case 0x0400:
					if (fifo[3] > 0)
						cpx += fifo[3];
					else
						cpx -= fifo[3];
					break;
				case 0x0500:
					if (fifo[3] > 0)
						cpx += fifo[3];
					else
						cpx -= fifo[3];
					break;
				case 0x0600:
					if (fifo[3] > 0)
						cpx -= fifo[3];
					else
						cpx += fifo[3];
					break;
				case 0x0700:
					if (fifo[3] > 0)
						cpx -= fifo[3];
					else
						cpx += fifo[3];
					break;
			}
		}
		else
			{
				logerror("unsupported command\n");
				popmessage("unsupported command %s (%04x)",instruction_name[fifo[0]>>10],fifo[0]);
			}

		fifo_counter = 0;
	}
}

static int regno;

READ16_HANDLER( HD63484_status_r )
{
//  if (cpu_get_pc(space->cpu) != 0xfced6 && cpu_get_pc(space->cpu) != 0xfe1d6)
//      logerror("%05x: HD63484 status read\n",cpu_get_pc(space->cpu));

	return 0xff22|(mame_rand(space->machine) & 0x0004);	/* write FIFO ready + command end    +  (read FIFO ready or read FIFO not ready) */
}

WRITE16_HANDLER( HD63484_address_w )
{
	/* only low 8 bits are used */
	if (ACCESSING_BITS_0_7)
		regno = data;
}

WRITE16_HANDLER( HD63484_data_w )
{
	COMBINE_DATA(&HD63484_reg[regno/2]);

	if ( !strcmp(space->machine->gamedrv->name, "skattva")) HD63484_reg[2/2] = (HD63484_reg[2/2] & 0xf8ff) | 0x0200; // hack to set proper color depth in skattva

	if (regno & 0x80) regno += 2;	/* autoincrement */

#if LOG_COMMANDS
	logerror("PC %05x: HD63484 register %02x write %04x\n",cpu_get_pc(space->cpu),regno,HD63484_reg[regno/2]);
#endif

	if (regno == 0)	/* FIFO */
		HD63484_command_w(space->machine, HD63484_reg[0]);
}

READ16_HANDLER( HD63484_data_r )
{
	int res;

	if (regno == 0x80)
		res = video_screen_get_vpos(space->machine->primary_screen);
	else if (regno == 0)
	{
#if LOG_COMMANDS
		logerror("%05x: HD63484 read FIFO\n",cpu_get_pc(space->cpu));
#endif
		res = readfifo;
	}
	else
	{
#if LOG_COMMANDS
		logerror("%05x: HD63484 read register %02x\n",cpu_get_pc(space->cpu),regno);
#endif
		res = 0;
	}

	return res;
}
