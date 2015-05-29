// license:BSD-3-Clause
// copyright-holders:Roberto Zandona'
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

const device_type HD63484 = &device_creator<hd63484_device>;

hd63484_device::hd63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HD63484, "HD63484 CRTC", tag, owner, clock, "hd63484", __FILE__),
	m_ram(NULL),
	m_fifo_counter(0),
	m_readfifo(0),
	m_org(0),
	m_org_dpd(0),
	m_rwp(0),
	m_cl0(0),
	m_cl1(0),
	m_ccmp(0),
	m_edg(0),
	m_mask(0),
	m_ppy(0),
	m_pzcy(0),
	m_ppx(0),
	m_pzcx(0),
	m_psy(0),
	m_psx(0),
	m_pey(0),
	m_pzy(0),
	m_pex(0),
	m_pzx(0),
	m_xmin(0),
	m_ymin(0),
	m_xmax(0),
	m_ymax(0),
	m_rwp_dn(0),
	m_cpx(0),
	m_cpy(0),
	m_regno(0),
	m_skattva_hack(0)
{
	memset(m_reg, 0x00, sizeof(m_reg));
	//m_pattern[16],
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd63484_device::device_start()
{
	m_ram = auto_alloc_array_clear(machine(), UINT16, HD63484_RAM_SIZE);

	save_pointer(NAME(m_ram), HD63484_RAM_SIZE);
	save_item(NAME(m_reg));
	save_item(NAME(m_fifo_counter));
	save_item(NAME(m_fifo));
	save_item(NAME(m_readfifo));
	save_item(NAME(m_pattern));
	save_item(NAME(m_org));
	save_item(NAME(m_org_dpd));
	save_item(NAME(m_rwp));
	save_item(NAME(m_cl0));
	save_item(NAME(m_cl1));
	save_item(NAME(m_ccmp));
	save_item(NAME(m_edg));
	save_item(NAME(m_mask));
	save_item(NAME(m_ppy));
	save_item(NAME(m_pzcy));
	save_item(NAME(m_ppx));
	save_item(NAME(m_pzcx));
	save_item(NAME(m_psy));
	save_item(NAME(m_psx));
	save_item(NAME(m_pey));
	save_item(NAME(m_pzy));
	save_item(NAME(m_pex));
	save_item(NAME(m_pzx));
	save_item(NAME(m_xmin));
	save_item(NAME(m_ymin));
	save_item(NAME(m_xmax));
	save_item(NAME(m_ymax));
	save_item(NAME(m_rwp_dn));
	save_item(NAME(m_cpx));
	save_item(NAME(m_cpy));
	save_item(NAME(m_regno));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd63484_device::device_reset()
{
	m_fifo_counter = 0;
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static const int instruction_length[64] =
{
		0, 3, 2, 1, /* 0x */
		0, 0,-1, 2, /* 1x */
		0, 3, 3, 3, /* 2x */
		0, 0, 0, 0, /* 3x */
		0, 1, 2, 2, /* 4x */
		0, 0, 4, 4, /* 5x */
		5, 5, 5, 5, /* 6x */
		5, 5, 5, 5, /* 7x */
		3, 3, 3, 3, /* 8x */
		3, 3,-2,-2, /* 9x */
	-2,-2, 2, 4,    /* Ax */
		5, 5, 7, 7, /* Bx */
		3, 3, 1, 1, /* Cx */
		2, 2, 2, 2, /* Dx */
		5, 5, 5, 5, /* Ex */
		5, 5, 5, 5  /* Fx */
};

static const char *const instruction_name[64] =
{
	"undef","ORG  ","WPR  ","RPR  ",    /* 0x */
	"undef","undef","WPTN ","RPTN ",    /* 1x */
	"undef","DRD  ","DWT  ","DMOD ",    /* 2x */
	"undef","undef","undef","undef",    /* 3x */
	"undef","RD   ","WT   ","MOD  ",    /* 4x */
	"undef","undef","CLR  ","SCLR ",    /* 5x */
	"CPY  ","CPY  ","CPY  ","CPY  ",    /* 6x */
	"SCPY ","SCPY ","SCPY ","SCPY ",    /* 7x */
	"AMOVE","RMOVE","ALINE","RLINE",    /* 8x */
	"ARCT ","RRCT ","APLL ","RPLL ",    /* 9x */
	"APLG ","RPLG ","CRCL ","ELPS ",    /* Ax */
	"AARC ","RARC ","AEARC","REARC",    /* Bx */
	"AFRCT","RFRCT","PAINT","DOT  ",    /* Cx */
	"PTN  ","PTN  ","PTN  ","PTN  ",    /* Dx */
	"AGCPY","AGCPY","AGCPY","AGCPY",    /* Ex */
	"RGCPY","RGCPY","RGCPY","RGCPY"     /* Fx */
};

void hd63484_device::doclr16( int opcode, UINT16 fill, int *dst, INT16 _ax, INT16 _ay )
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
					m_ram[*dst]  = fill;
					break;
				case 1:
					m_ram[*dst] |= fill;
					break;
				case 2:
					m_ram[*dst] &= fill;
					break;
				case 3:
					m_ram[*dst] ^= fill;
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
			*dst = (*dst + (m_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
			if (ay == 0)
				break;
			ay++;
		}
		else
		{
			*dst = (*dst - (m_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
			if (ay == 0)
				break;
			ay--;
		}
	}
}

void hd63484_device::docpy16( int opcode, int src, int *dst, INT16 _ax, INT16 _ay )
{
	int dstep1,dstep2;
	int ax = _ax;
	int ay = _ay;

	switch (opcode & 0x0700)
	{
		default:
		case 0x0000: dstep1 =  1; dstep2 = -1 * (m_reg[0xca/2] & 0x0fff) - ax * dstep1; break;
		case 0x0100: dstep1 =  1; dstep2 =      (m_reg[0xca/2] & 0x0fff) - ax * dstep1; break;
		case 0x0200: dstep1 = -1; dstep2 = -1 * (m_reg[0xca/2] & 0x0fff) + ax * dstep1; break;
		case 0x0300: dstep1 = -1; dstep2 =      (m_reg[0xca/2] & 0x0fff) + ax * dstep1; break;
		case 0x0400: dstep1 = -1 * (m_reg[0xca/2] & 0x0fff); dstep2 =  1 - ay * dstep1; break;
		case 0x0500: dstep1 =      (m_reg[0xca/2] & 0x0fff); dstep2 =  1 - ay * dstep1; break;
		case 0x0600: dstep1 = -1 * (m_reg[0xca/2] & 0x0fff); dstep2 = -1 + ay * dstep1; break;
		case 0x0700: dstep1 =      (m_reg[0xca/2] & 0x0fff); dstep2 = -1 + ay * dstep1; break;
	}

	for (;;)
	{
		for (;;)
		{
			switch (opcode & 0x0007)
			{
				case 0:
					m_ram[*dst]  = m_ram[src];
					break;
				case 1:
					m_ram[*dst] |= m_ram[src];
					break;
				case 2:
					m_ram[*dst] &= m_ram[src];
					break;
				case 3:
					m_ram[*dst] ^= m_ram[src];
					break;
				case 4:
					if (m_ram[*dst] == (m_ccmp & 0xff))
						m_ram[*dst] = m_ram[src];
					break;
				case 5:
					if (m_ram[*dst] != (m_ccmp & 0xff))
						m_ram[*dst] = m_ram[src];
					break;
				case 6:
					if (m_ram[*dst] < m_ram[src])
						m_ram[*dst] = m_ram[src];
					break;
				case 7:
					if (m_ram[*dst] > m_ram[src])
						m_ram[*dst] = m_ram[src];
					break;
			}

			if (opcode & 0x0800)
			{
				if (ay == 0) break;
				if (_ay > 0)
				{
					src = (src - (m_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE - 1);
					ay--;
				}
				else
				{
					src = (src + (m_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
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
				src = (src - 1 + ay * (m_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ax == 0) break;
				ax++;
			}
			else
			{
				src = (src + 1 - ay * (m_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);
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
				src = (src + (m_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ay == 0) break;
				ay++;
			}
			else
			{
				src = (src - (m_reg[0xca/2] & 0x0fff) - ax) & (HD63484_RAM_SIZE - 1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE - 1);
				if (ay == 0) break;
				ay--;
			}
		}
	}
}

int hd63484_device::org_first_pixel( int _org_dpd )
{
	int gbm = (m_reg[0x02/2] & 0x700) >> 8;

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

void hd63484_device::dot( int x, int y, int opm, UINT16 color )
{
	int dst, x_int, x_mod, bpp;
	UINT16 color_shifted, bitmask, bitmask_shifted;

	x += org_first_pixel(m_org_dpd);

	switch ((m_reg[0x02/2] & 0x700) >> 8)
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

	dst = (m_org + x_int - y * (m_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);

	switch (opm)
	{
		case 0:
			m_ram[dst] = (m_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 1:
			m_ram[dst] = m_ram[dst] | color_shifted;
			break;
		case 2:
			m_ram[dst] = m_ram[dst] & ((m_ram[dst] & ~bitmask_shifted) | color_shifted);
			break;
		case 3:
			m_ram[dst] = m_ram[dst] ^ color_shifted;
			break;
		case 4:
			if (get_pixel(x, y) == (m_ccmp & bitmask))
				m_ram[dst] = (m_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 5:
			if (get_pixel(x, y) != (m_ccmp & bitmask))
				m_ram[dst] = (m_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 6:
			if (get_pixel(x, y) < (m_cl0 & bitmask))
				m_ram[dst] = (m_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
		case 7:
			if (get_pixel(x, y) > (m_cl0 & bitmask))
				m_ram[dst] = (m_ram[dst] & ~bitmask_shifted) | color_shifted;
			break;
	}
}

int hd63484_device::get_pixel( int x, int y )
{
	int dst, x_int, x_mod, bpp;
	UINT16 bitmask, bitmask_shifted;

	switch ((m_reg[0x02/2] & 0x700) >> 8)
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

	dst = (m_org + x_int - y * (m_reg[0xca/2] & 0x0fff)) & (HD63484_RAM_SIZE - 1);

	return ((m_ram[dst] & bitmask_shifted) >> (x_mod * bpp));
}

int hd63484_device::get_pixel_ptn( int x, int y )
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

	if ((m_pattern[dst] & bitmask_shifted) >> (x_mod * bpp))
		return 1;
	else
		return 0;
}

void hd63484_device::agcpy( int opcode, int src_x, int src_y, int dst_x, int dst_y, INT16 _ax, INT16 _ay )
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
			dot(xxd, yyd, opcode & 0x0007, get_pixel(xxs, yys));

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

void hd63484_device::ptn( int opcode, int src_x, int src_y, INT16 _ax, INT16 _ay )
{
	int dst_step1_x = 0,dst_step1_y = 0,dst_step2_x = 0,dst_step2_y = 0;
	int src_step1_x,src_step1_y,src_step2_x,src_step2_y;
	int ax = _ax;
	int ay = _ay;
	int ax_neg; //,ay_neg;
	int xxs = src_x;
	int yys = src_y;
	int xxd = m_cpx;
	int yyd = m_cpy;
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
			getpixel = get_pixel_ptn(xxs, yys);
			switch ((opcode & 0x0018) >> 3)
			{
				case 0x0000:
					if (getpixel)
						dot(xxd, yyd, opcode & 0x0007, m_cl1);
					else
						dot(xxd, yyd, opcode & 0x0007, m_cl0);
					break;
				case 0x0001:
					if (getpixel)
						dot(xxd, yyd, opcode & 0x0007, m_cl1);
					break;
				case 0x0002:
					if (getpixel == 0)
						dot(xxd, yyd, opcode & 0x0007, m_cl0);
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

void hd63484_device::line( INT16 sx, INT16 sy, INT16 ex, INT16 ey, INT16 col )
{
	INT16 ax,ay;

	int cpx_t = sx;
	int cpy_t = sy;

	ax = ex - sx;
	ay = ey - sy;

	if (abs(ax) >= abs(ay))
	{
		while (ax)
		{
			dot(cpx_t, cpy_t, col & 7, m_cl0);

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
			dot(cpx_t, cpy_t, col & 7, m_cl0);

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

void hd63484_device::circle( INT16 sx, INT16 sy, UINT16 r, INT16 col )
{
	const float DEG2RAD = 3.14159f/180;
	int i;

	for (i = 0; i < 360 * (r / 10); i++)
	{
		float degInRad = i * DEG2RAD / (r / 10);
		dot(sx + cos(degInRad) * r,sy + sin(degInRad) * r, col & 7, m_cl0);
	}
}

void hd63484_device::paint( int sx, int sy, int col )
{
	int getpixel;
	dot(sx, sy, 0, col);

	getpixel = get_pixel(sx+1,sy);
	switch ((m_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != m_edg))
		{
			sx++;
			paint(sx, sy, col);
			sx--;
		}

	getpixel = get_pixel(sx - 1, sy);
	switch ((m_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != m_edg))
		{
			sx--;
			paint(sx, sy, col);
			sx++;
		}

	getpixel = get_pixel(sx, sy + 1);
	switch ((m_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != m_edg))
		{
			sy++;
			paint(sx, sy, col);
			sy--;
		}

	getpixel = get_pixel(sx, sy - 1);
	switch ((m_reg[0x02/2] & 0x700) >> 8)
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
	if ((getpixel != col) && (getpixel != m_edg))
		{
			sy--;
			paint(sx, sy, col);
			sy++;
		}
}

void hd63484_device::command_w(UINT16 cmd)
{
	int len;

	m_fifo[m_fifo_counter++] = cmd;

	len = instruction_length[m_fifo[0] >> 10];
	if (len == -1)
	{
		if (m_fifo_counter < 2) return;
		else len = m_fifo[1] + 2;
	}
	else if (len == -2)
	{
		if (m_fifo_counter < 2) return;
		else len = 2 * m_fifo[1] + 2;
	}

	if (m_fifo_counter >= len)
	{
#if LOG_COMMANDS
		int i;

		logerror("%s: HD63484 command %s (%04x) ", machine().describe_context(), instruction_name[m_fifo[0] >> 10], m_fifo[0]);
		for (i = 1; i < m_fifo_counter; i++)
			logerror("%04x ", m_fifo[i]);
		logerror("\n");
#endif

		if (m_fifo[0] == 0x0400) { /* ORG */
			m_org = ((m_fifo[1] & 0x00ff) << 12) | ((m_fifo[2] & 0xfff0) >> 4);
			m_org_dpd = m_fifo[2] & 0x000f;
		}
		else if ((m_fifo[0] & 0xffe0) == 0x0800) /* WPR */
		{
			if (m_fifo[0] == 0x0800)
				m_cl0 = m_fifo[1];
			else if (m_fifo[0] == 0x0801)
				m_cl1 = m_fifo[1];
			else if (m_fifo[0] == 0x0802)
				m_ccmp = m_fifo[1];
			else if (m_fifo[0] == 0x0803)
				m_edg = m_fifo[1];
			else if (m_fifo[0] == 0x0804)
				m_mask = m_fifo[1];
			else if (m_fifo[0] == 0x0805)
				{
					m_ppy  = (m_fifo[1] & 0xf000) >> 12;
					m_pzcy = (m_fifo[1] & 0x0f00) >> 8;
					m_ppx  = (m_fifo[1] & 0x00f0) >> 4;
					m_pzcx = (m_fifo[1] & 0x000f) >> 0;
				}
			else if (m_fifo[0] == 0x0806)
				{
					m_psy  = (m_fifo[1] & 0xf000) >> 12;
					m_psx  = (m_fifo[1] & 0x00f0) >> 4;
				}
			else if (m_fifo[0] == 0x0807)
				{
					m_pey  = (m_fifo[1] & 0xf000) >> 12;
					m_pzy  = (m_fifo[1] & 0x0f00) >> 8;
					m_pex  = (m_fifo[1] & 0x00f0) >> 4;
					m_pzx  = (m_fifo[1] & 0x000f) >> 0;
				}
			else if (m_fifo[0] == 0x0808)
				m_xmin = m_fifo[1];
			else if (m_fifo[0] == 0x0809)
				m_ymin = m_fifo[1];
			else if (m_fifo[0] == 0x080a)
				m_xmax = m_fifo[1];
			else if (m_fifo[0] == 0x080b)
				m_ymax = m_fifo[1];
			else if (m_fifo[0] == 0x080c)
				{
					m_rwp = (m_rwp & 0x00fff) | ((m_fifo[1] & 0x00ff) << 12);
					m_rwp_dn = (m_fifo[1] & 0xc000) >> 14;
				}
			else if (m_fifo[0] == 0x080d)
				{
					m_rwp = (m_rwp & 0xff000) | ((m_fifo[1] & 0xfff0) >> 4);
				}
			else
				logerror("unsupported register\n");
		}
		else if ((m_fifo[0] & 0xffe0) == 0x0c00) /* RPR */
		{
			if (m_fifo[0] == 0x0c00)
				m_fifo[1] = m_cl0;
			else if (m_fifo[0] == 0x0c01)
				m_fifo[1] = m_cl1;
			else if (m_fifo[0] == 0x0c02)
				m_fifo[1] = m_ccmp;
			else if (m_fifo[0] == 0x0c03)
				m_fifo[1] = m_edg;
			else if (m_fifo[0] == 0x0c04)
				m_fifo[1] = m_mask;
			else if (m_fifo[0] == 0x0c05)
				{
					m_fifo[1] = (m_ppy << 12) | (m_pzcy << 8) | (m_ppx << 4) | m_pzcx;
				}
			else if (m_fifo[0] == 0x0c06)
				{
					m_fifo[1] = (m_psx << 12) | (m_psx << 4);
				}
			else if (m_fifo[0] == 0x0c07)
				{
					m_fifo[1] = (m_pey << 12) | (m_pzy << 8) | (m_pex << 4) | m_pzx;
				}
			else if (m_fifo[0] == 0x0c08)
				m_fifo[1] = m_xmin;
			else if (m_fifo[0] == 0x0c09)
				m_fifo[1] = m_ymin;
			else if (m_fifo[0] == 0x0c0a)
				m_fifo[1] = m_xmax;
			else if (m_fifo[0] == 0x0c0b)
				m_fifo[1] = m_ymax;
			else if (m_fifo[0] == 0x0c0c)
				{
					m_fifo[1] = (m_rwp_dn << 14) | ((m_rwp >> 12) & 0xff);
				}
			else if (m_fifo[0] == 0x0c0d)
				{
					m_fifo[1] = (m_rwp & 0x0fff) << 4;
				}
			else if (m_fifo[0] == 0x0c10)
				{
					// TODO
				}
			else if (m_fifo[0] == 0x0c11)
				{
					// TODO
				}
			else if (m_fifo[0] == 0x0c12)
				{
					m_fifo[1] = m_cpx;
				}
			else if (m_fifo[0] == 0x0c13)
				{
					m_fifo[1] = m_cpy;
				}
			else
				logerror("unsupported register\n");
		}
		else if ((m_fifo[0] & 0xfff0) == 0x1800) /* WPTN */
		{
			int i;
			int start = m_fifo[0] & 0x000f;
			int n = m_fifo[1];
			for (i = 0; i < n; i++)
				m_pattern[start + i] = m_fifo[2 + i];
		}
		else if (m_fifo[0] == 0x4400)    /* RD */
		{
			m_readfifo = m_ram[m_rwp];
			m_rwp = (m_rwp + 1) & (HD63484_RAM_SIZE - 1);
		}
		else if (m_fifo[0] == 0x4800)    /* WT */
		{
			m_ram[m_rwp] = m_fifo[1];
			m_rwp = (m_rwp + 1) & (HD63484_RAM_SIZE - 1);
		}
		else if (m_fifo[0] == 0x5800)    /* CLR */
		{
			doclr16(m_fifo[0], m_fifo[1], &m_rwp, m_fifo[2], m_fifo[3]);

			{
			int fifo2 = (int)m_fifo[2], fifo3 = (int)m_fifo[3];
			if (fifo2 < 0) fifo2 *= -1;
			if (fifo3 < 0) fifo3 *= -1;
			m_rwp += ((fifo2 + 1) * (fifo3 + 1));
			}

		}
		else if ((m_fifo[0] & 0xfffc) == 0x5c00) /* SCLR */
		{
			doclr16(m_fifo[0], m_fifo[1], &m_rwp, m_fifo[2], m_fifo[3]);

			{
				int fifo2 = (int)m_fifo[2], fifo3 = (int)m_fifo[3];
				if (fifo2 < 0) fifo2 *= -1;
				if (fifo3 < 0) fifo3 *= -1;
				m_rwp += ((fifo2 + 1) * (fifo3 + 1));
			}

		}
		else if ((m_fifo[0] & 0xf0ff) == 0x6000) /* CPY */
		{
			docpy16(m_fifo[0], ((m_fifo[1] & 0x00ff) << 12) | ((m_fifo[2] & 0xfff0) >> 4), &m_rwp, m_fifo[3], m_fifo[4]);

			{
				int fifo2 = (int)m_fifo[2], fifo3 = (int)m_fifo[3];
				if (fifo2 < 0) fifo2 *= -1;
				if (fifo3 < 0) fifo3 *= -1;
				m_rwp += ((fifo2 + 1) * (fifo3 + 1));
			}

		}
		else if ((m_fifo[0] & 0xf0fc) == 0x7000) /* SCPY */
		{
			docpy16(m_fifo[0], ((m_fifo[1] & 0x00ff) << 12) | ((m_fifo[2] & 0xfff0) >> 4), &m_rwp, m_fifo[3], m_fifo[4]);

			{
				int fifo2 = (int)m_fifo[2], fifo3 = (int)m_fifo[3];
				if (fifo2 < 0) fifo2 *= -1;
				if (fifo3 < 0) fifo3 *= -1;
				m_rwp += ((fifo2 + 1) * (fifo3 + 1));
			}

		}
		else if (m_fifo[0] == 0x8000)    /* AMOVE */
		{
			m_cpx = m_fifo[1];
			m_cpy = m_fifo[2];
		}
		else if (m_fifo[0] == 0x8400)    /* RMOVE */
		{
			m_cpx += (INT16)m_fifo[1];
			m_cpy += (INT16)m_fifo[2];
		}
		else if ((m_fifo[0] & 0xff00) == 0x8800) /* ALINE */
		{
			line(m_cpx, m_cpy, m_fifo[1], m_fifo[2], m_fifo[0] & 0xff);
			m_cpx = (INT16)m_fifo[1];
			m_cpy = (INT16)m_fifo[2];
		}
		else if ((m_fifo[0] & 0xff00) == 0x8c00) /* RLINE */
		{
			line(m_cpx, m_cpy, m_cpx + (INT16)m_fifo[1], m_cpy + (INT16)m_fifo[2], m_fifo[0] & 0xff);
			m_cpx += (INT16)m_fifo[1];
			m_cpy += (INT16)m_fifo[2];
		}
		else if ((m_fifo[0] & 0xfff8) == 0x9000) /* ARCT */
		{
			line(m_cpx, m_cpy, (INT16)m_fifo[1], m_cpy, m_fifo[0] & 0xff);
			line((INT16)m_fifo[1], m_cpy, (INT16)m_fifo[1], (INT16)m_fifo[2], m_fifo[0] & 0xff);
			line((INT16)m_fifo[1], (INT16)m_fifo[2], m_cpx, (INT16)m_fifo[2], m_fifo[0] & 0xff);
			line(m_cpx, (INT16)m_fifo[2], m_cpx, m_cpy, m_fifo[0] & 0xff);
			m_cpx = (INT16)m_fifo[1];
			m_cpy = (INT16)m_fifo[2];
		}
		else if ((m_fifo[0] & 0xfff8) == 0x9400) /* RRCT  added*/
		{
			line(m_cpx, m_cpy, m_cpx + (INT16)m_fifo[1], m_cpy, m_fifo[0] & 0xff);
			line(m_cpx + (INT16)m_fifo[1], m_cpy, m_cpx + (INT16)m_fifo[1], m_cpy + (INT16)m_fifo[2], m_fifo[0] & 0xff);
			line(m_cpx + (INT16)m_fifo[1], m_cpy + (INT16)m_fifo[2], m_cpx, m_cpy + (INT16)m_fifo[2], m_fifo[0] & 0xff);
			line(m_cpx, m_cpy + (INT16)m_fifo[2], m_cpx, m_cpy, m_fifo[0] & 0xff);

			m_cpx += (INT16)m_fifo[1];
			m_cpy += (INT16)m_fifo[2];
		}
		else if ((m_fifo[0] & 0xfff8) == 0xa400) /* RPLG  added*/
		{
			int nseg, sx, sy, ex, ey;
			sx = m_cpx;
			sy = m_cpy;
			for (nseg = 0; nseg < m_fifo[1]; nseg++)
			{
				ex = sx + (INT16)m_fifo[2 + nseg * 2];
				ey = sy + (INT16)m_fifo[2 + nseg * 2 + 1];
				line(sx, sy, ex, ey, m_fifo[0] & 7);
				sx = ex;
				sy = ey;
			}
			line(sx, sy, m_cpx, m_cpy, m_fifo[0] & 7);
		}
		else if ((m_fifo[0] & 0xfe00) == 0xa800) /* CRCL  added*/
		{
			circle(m_cpx, m_cpy, m_fifo[1] & 0x1fff, m_fifo[0] & 7); // only 13 bit are used for the radius
		}
		else if ((m_fifo[0] & 0xfff8) == 0xc000) /* AFRCT */
		{
			INT16 pcx, pcy;
			INT16 ax, ay, xx, yy;


			pcx = m_fifo[1];
			pcy = m_fifo[2];
			ax = pcx - m_cpx;
			ay = pcy - m_cpy;
			xx = m_cpx;
			yy = m_cpy;

			for (;;)
			{
				for (;;)
				{
					dot(xx, yy, m_fifo[0] & 0x07, m_cl0);

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

				ax = pcx - m_cpx;
				if (pcy < m_cpy)
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
		else if ((m_fifo[0] & 0xfff8) == 0xc400) /* RFRCT  added TODO*/
		{
			line(m_cpx, m_cpy, m_cpx + (INT16)m_fifo[1], m_cpy, m_fifo[0] & 0xff);
			line(m_cpx + m_fifo[1], m_cpy, m_cpx + m_fifo[1], m_cpy + m_fifo[2], m_fifo[0] & 0xff);
			line(m_cpx + m_fifo[1], m_cpy + m_fifo[2], m_cpx, m_cpy + m_fifo[2], m_fifo[0] & 0xff);
			line(m_cpx, m_cpy + m_fifo[2], m_cpx, m_cpy, m_fifo[0] & 0xff);

			m_cpx = m_cpx + (INT16)m_fifo[1];
			m_cpy = m_cpy + (INT16)m_fifo[2];
		}
		else if (m_fifo[0] == 0xc800)    /* PAINT */
		{
			paint(m_cpx, m_cpy, m_cl0);
		}
		else if ((m_fifo[0] & 0xfff8) == 0xcc00) /* DOT */
		{
			dot(m_cpx, m_cpy, m_fifo[0] & 0xff, m_cl0);
		}
		else if ((m_fifo[0] & 0xf000) == 0xd000) /* PTN (to do) */
		{
			ptn(m_fifo[0], m_psx, m_psy, m_pex - m_psx, m_pey - m_psy);

			if ((m_fifo[0] & 0x0800) == 0x0000)
				switch (m_fifo[0] & 0x0700)
				{
					case 0x0000:
						if ((m_pey - m_psy) > 0)
							m_cpy += (m_pey - m_psy);
						else
							m_cpy -= (m_pey - m_psy);
						break;
					case 0x0100:
						// missing
						break;
					case 0x0200:
						if ((m_pey - m_psy) > 0)
							m_cpx += (m_pey - m_psy);
						else
							m_cpx -= (m_pey - m_psy);
						break;
					case 0x0300:
						// missing
						break;
					case 0x0400:
						if ((m_pey - m_psy) > 0)
							m_cpy -= (m_pey - m_psy);
						else
							m_cpy += (m_pey - m_psy);
						break;
					case 0x0500:
						// missing
						break;
					case 0x0600:
						if ((m_pey - m_psy) > 0)
							m_cpx -= (m_pey - m_psy);
						else
							m_cpx += (m_pey - m_psy);
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
		else if ((m_fifo[0] & 0xf018) == 0xe000) /* agcpy */
		{
			agcpy(m_fifo[0], (INT16)m_fifo[1], (INT16)m_fifo[2], m_cpx, m_cpy, m_fifo[3], m_fifo[4]);

			switch (m_fifo[0] & 0x0700)
			{
				case 0x0000:
					if (m_fifo[4] > 0)
						m_cpy += m_fifo[4];
					else
						m_cpy -= m_fifo[4];
					break;
				case 0x0100:
					if (m_fifo[4] > 0)
						m_cpy -= m_fifo[4];
					else
						m_cpy += m_fifo[4];
					break;
				case 0x0200:
					if (m_fifo[4] > 0)
						m_cpy += m_fifo[4];
					else
						m_cpy -= m_fifo[4];
					break;
				case 0x0300:
					if (m_fifo[4] > 0)
						m_cpy -= m_fifo[4];
					else
						m_cpy += m_fifo[4];
					break;
				case 0x0400:
					if (m_fifo[3] > 0)
						m_cpx += m_fifo[3];
					else
						m_cpx -= m_fifo[3];
					break;
				case 0x0500:
					if (m_fifo[3] > 0)
						m_cpx += m_fifo[3];
					else
						m_cpx -= m_fifo[3];
					break;
				case 0x0600:
					if (m_fifo[3] > 0)
						m_cpx -= m_fifo[3];
					else
						m_cpx += m_fifo[3];
					break;
				case 0x0700:
					if (m_fifo[3] > 0)
						m_cpx -= m_fifo[3];
					else
						m_cpx += m_fifo[3];
					break;
			}
		}
		else
		{
			logerror("unsupported command\n");
			popmessage("unsupported command %s (%04x)", instruction_name[m_fifo[0] >> 10], m_fifo[0]);
		}

		m_fifo_counter = 0;
	}
}

READ16_MEMBER( hd63484_device::status_r )
{
//  if (space.device().safe_pc() != 0xfced6 && space.device().safe_pc() != 0xfe1d6)
//      logerror("%05x: HD63484 status read\n",space.device().safe_pc());

	return 0xff22 | (machine().rand() & 0x0004);    /* write FIFO ready + command end    +  (read FIFO ready or read FIFO not ready) */
}

WRITE16_MEMBER( hd63484_device::address_w )
{
	/* only low 8 bits are used */
	if (ACCESSING_BITS_0_7)
		m_regno = data;
}

WRITE16_MEMBER( hd63484_device::data_w )
{
	COMBINE_DATA(&m_reg[m_regno/2]);

	if (m_skattva_hack)
		m_reg[2/2] = (m_reg[2/2] & 0xf8ff) | 0x0200; // hack to set proper color depth in skattva

	if (m_regno & 0x80)
		m_regno += 2;    /* autoincrement */

#if LOG_COMMANDS
//  logerror("PC %05x: HD63484 register %02x write %04x\n", space.device().safe_pc(), m_regno, m_reg[m_regno/2]);
#endif

	if (m_regno == 0)    /* FIFO */
		command_w(m_reg[0]);
}

READ16_MEMBER( hd63484_device::data_r )
{
	int res;

	if (m_regno == 0x80)
		res = machine().first_screen()->vpos();
	else if (m_regno == 0)
	{
#if LOG_COMMANDS
//      logerror("%05x: HD63484 read FIFO\n", space.device().safe_pc());
#endif
		res = m_readfifo;
	}
	else
	{
#if LOG_COMMANDS
//      logerror("%05x: HD63484 read register %02x\n", space.device().safe_pc(), m_regno);
#endif
		res = 0;
	}

	return res;
}

READ16_MEMBER( hd63484_device::ram_r )
{
	return m_ram[offset];
}

READ16_MEMBER( hd63484_device::regs_r )
{
	return m_reg[offset];
}

WRITE16_MEMBER( hd63484_device::ram_w )
{
	COMBINE_DATA(&m_ram[offset]);
}

WRITE16_MEMBER( hd63484_device::regs_w )
{
	COMBINE_DATA(&m_reg[offset]);
}
