/***************************************************************************

hd63484

this chip is used in:
- shanghai.c
- adp.c

***************************************************************************/

#include "driver.h"
#include "video/hd63484.h"

static int get_pixel(int x,int y);

/* the on-chip FIFO is 16 bytes long, but we use a larger one to simplify */
/* decoding of long commands. Commands can be up to 64KB long... but Shanghai */
/* doesn't reach that length. */

#define FIFO_LENGTH 50

static int fifo_counter;
static UINT16 fifo[FIFO_LENGTH];
static UINT16 readfifo;
UINT16 *HD63484_ram;
UINT16 HD63484_reg[256/2];
static int org,org_dpd,rwp;
static UINT16 cl0,cl1,ccmp,edg,mask,ppy,pzcy,ppx,pzcs,psy,psx,pey,pzy,pex,pzx,xmin,ymin,xmax,ymax,rwp_dn;
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
	 3, 3, 3, 3, 	/* 8x */
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
	"AMOVE","RMOVE","ALINE","RLINE", 	/* 8x */
	"ARCT ","RRCT ","APLL ","RPLL ",	/* 9x */
	"APLG ","RPLG ","CRCL ","ELPS ",	/* Ax */
	"AARC ","RARC ","AEARC","REARC",	/* Bx */
	"AFRCT","RFRCT","PAINT","DOT  ",	/* Cx */
	"PTN  ","PTN  ","PTN  ","PTN  ",	/* Dx */
	"AGCPY","AGCPY","AGCPY","AGCPY",	/* Ex */
	"RGCPY","RGCPY","RGCPY","RGCPY" 	/* Fx */
};

void HD63484_start(void)
{
	fifo_counter = 0;
	HD63484_ram = auto_malloc(HD63484_RAM_SIZE);
	memset(HD63484_ram,0,HD63484_RAM_SIZE);
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

static void agcpy(int opcode,int src_x,int src_y,int dst_x,int dst_y,INT16 _ax,INT16 _ay)
{
	int step1_x,step1_y,step2_x,step2_y;
	int ax = _ax;
	int ay = _ay;
	int xxs = src_x;
	int yys = src_y;
	int xxd = dst_x;
	int yyd = dst_y;

	switch (opcode & 0x0700)
	{
		default:
		case 0x0000: step1_x =  1; step1_y =  0; step2_x = -ax; step2_y =   1; break;
		case 0x0100: step1_x =  1; step1_y =  0; step2_x = -ax; step2_y =  -1; break;
		case 0x0200: step1_x = -1; step1_y =  0; step2_x =  ax; step2_y =   1; break;
		case 0x0300: step1_x = -1; step1_y =  0; step2_x =  ax; step2_y =  -1; break;
		case 0x0400: step1_x =  0; step1_y =  1; step2_x =   1; step2_y =  ay; break;
		case 0x0500: step1_x =  0; step1_y = -1; step2_x =   1; step2_y = -ay; break;
		case 0x0600: step1_x =  0; step1_y =  1; step2_x =  -1; step2_y =  ay; break;
		case 0x0700: step1_x =  0; step1_y = -1; step2_x =  -1; step2_y = -ay; break;
	}

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
					yys++;
					xxd += step1_x;
					yyd += step1_y;
					ay--;
				}
				else
				{
					yys--;
					xxd += step1_x;
					yyd += step1_y;
					ay++;
				}
			}
			else
			{
				if (ax == 0) break;
				else if (ax > 0)
				{
					xxs++;
					xxd += step1_x;
					yyd += step1_y;
					ax--;
				}
				else
				{
					xxs--;
					xxd += step1_x;
					yyd += step1_y;
					ax++;
				}
			}
		}

		if (opcode & 0x0800)
		{
			ay = _ay;
			if (_ax < 0)
			{
				xxs--;
				yys -= ay;
				xxd += step2_x;
				yyd += step2_y;
				if (ax == 0) break;
				ax++;
			}
			else
			{
				xxs++;
				yys += ay;
				xxd += step2_x;
				yyd += step2_y;
				if (ax == 0) break;
				ax--;
			}
		}
		else
		{
			ax = _ax;
			if (_ay < 0)
			{
				xxs -= ax;
				yys--;
				xxd += step2_x;
				yyd += step2_y;
				if (ay == 0) break;
				ay++;
			}
			else
			{
				xxs -= ax;
				yys++;
				xxd += step2_x;
				yyd += step2_y;
				if (ay == 0) break;
				ay--;
			}
		}
	}
}

static void HD63484_command_w(UINT16 cmd)
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
		int i;

		logerror("PC %05x: HD63484 command %s (%04x) ",activecpu_get_pc(),instruction_name[fifo[0]>>10],fifo[0]);
		for (i = 1;i < fifo_counter;i++)
			logerror("%04x ",fifo[i]);
		logerror("\n");

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
					pzcs = (fifo[1] & 0x000f) >> 0;
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
				rwp = (rwp & 0xff000) | ((fifo[1] & 0xfff0) >> 4);
			else
logerror("unsupported register\n");
		}
		else if ((fifo[0] & 0xfff0) == 0x1800)	/* WPTN */
		{
			/* pattern RAM not supported */
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

		/*
            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }
            */
		}
		else if ((fifo[0] & 0xfffc) == 0x5c00)	/* SCLR */
		{
			doclr16(fifo[0],fifo[1],&rwp,fifo[2],fifo[3]);

		/*
            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }
            */
		}
		else if ((fifo[0] & 0xf0ff) == 0x6000)	/* CPY */
		{
			docpy16(fifo[0],((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4),&rwp,fifo[3],fifo[4]);

		/*
            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }
            */
		}
		else if ((fifo[0] & 0xf0fc) == 0x7000)	/* SCPY */
		{
			docpy16(fifo[0],((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4),&rwp,fifo[3],fifo[4]);

		/*
            {
                int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
                if (fifo2<0) fifo2 *= -1;
                if (fifo3<0) fifo3 *= -1;
                rwp += ((fifo2+1)*(fifo3+1));
            }
            */
		}
		else if (fifo[0] == 0x8000)	/* AMOVE */
		{
			cpx = fifo[1];
			cpy = fifo[2];
		}
		else if ((fifo[0] & 0xfff8) == 0x8800)	/* ALINE */
		{
			INT16 ex,ey,sx,sy;
			INT16 ax,ay;

			sx = cpx;
			sy = cpy;
			ex = fifo[1];
			ey = fifo[2];

			ax = ex - sx;
			ay = ey - sy;

			if (abs(ax) >= abs(ay))
			{
				while (ax)
				{
					dot(cpx,cpy,fifo[0] & 0x0007,cl0);

					if (ax > 0)
					{
						cpx++;
						ax--;
					}
					else
					{
						cpx--;
						ax++;
					}
					cpy = sy + ay * (cpx - sx) / (ex - sx);
				}
			}
			else
			{
				while (ay)
				{
					dot(cpx,cpy,fifo[0] & 0x0007,cl0);

					if (ay > 0)
					{
						cpy++;
						ay--;
					}
					else
					{
						cpy--;
						ay++;
					}
					cpx = sx + ax * (cpy - sy) / (ey - sy);
				}
			}
		}
		else if ((fifo[0] & 0xfff8) == 0x9000)	/* ARCT */
		{
			INT16 pcx,pcy;
			INT16 ax,ay,xx,yy;

			pcx = fifo[1];
			pcy = fifo[2];

			xx = cpx;
			yy = cpy;

			ax = pcx - cpx;
			for (;;)
			{
				dot(xx,yy,fifo[0] & 0x0007,cl0);

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

			ay = pcy - cpy;
			for (;;)
			{
				dot(xx,yy,fifo[0] & 0x0007,cl0);

				if (ay == 0) break;
				else if (ay > 0)
				{
					yy++;
					ay--;
				}
				else
				{
					yy--;
					ay++;
				}
			}

			ax = cpx - pcx;
			for (;;)
			{
				dot(xx,yy,fifo[0] & 0x0007,cl0);

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

			ay = cpy - pcy;
			for (;;)
			{
				dot(xx,yy,fifo[0] & 0x0007,cl0);

				if (ay == 0) break;
				else if (ay > 0)
				{
					yy++;
					ay--;
				}
				else
				{
					yy--;
					ay++;
				}
			}
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
					dot(xx,yy,fifo[0] & 0x0007,cl0);

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
		else if ((fifo[0] & 0xfff8) == 0xcc00)	/* DOT */
		{
			dot(cpx,cpy,fifo[0] & 0x0007,cl0);
		}
		else if ((fifo[0] & 0xf0f8) == 0xe000)	/* AGCPY */
		{
			agcpy(fifo[0],fifo[1],fifo[2],cpx,cpy,fifo[3],fifo[4]);

			cpx += fifo[3];
			cpy += fifo[4];
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
	if (activecpu_get_pc() != 0xfced6 && activecpu_get_pc() != 0xfe1d6) logerror("%05x: HD63484 status read\n",activecpu_get_pc());
	return 0xff22|4;	/* write FIFO ready + command end    + read FIFO ready */
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
	if (regno & 0x80) regno += 2;	/* autoincrement */
logerror("PC %05x: HD63484 register %02x write %04x\n",activecpu_get_pc(),regno,HD63484_reg[regno/2]);
	if (regno == 0)	/* FIFO */
		HD63484_command_w(HD63484_reg[0]);
}

READ16_HANDLER( HD63484_data_r )
{
	int res;

	if (regno == 0x80)
		res = video_screen_get_vpos(machine->primary_screen);
	else if (regno == 0)
	{
logerror("%05x: HD63484 read FIFO\n",activecpu_get_pc());
		res = readfifo;
	}
	else
	{
logerror("%05x: HD63484 read register %02x\n",activecpu_get_pc(),regno);
		res = 0;
	}

	return res;
}
