/*****************************************************************************
 *
 * video/epnick.c
 *
 * Nick Graphics Chip - found in Enterprise
 *
 * this is a display list graphics chip, with bitmap,
 * character and attribute graphics modes. Each entry in the
 * display list defines a char line, with variable number of
 * scanlines. Colour modes are 2,4, 16 and 256 colour.
 * Nick has 256 colours, 3 bits for R and G, with 2 bits for Blue.
 * It's a nice and flexible graphics processor..........
 *
 ****************************************************************************/

#include "emu.h"
#include "machine/ram.h"
#include "includes/enterp.h"

/* given a colour index in range 0..255 gives the Red component */
#define NICK_GET_RED8(x) \
	((      \
		( ( (x & (1<<0)) >>0) <<2) | \
		( ( (x & (1<<3)) >>3) <<1) | \
		( ( (x & (1<<6)) >>6) <<0) \
	)<<5)

/* given a colour index in range 0..255 gives the Red component */
#define NICK_GET_GREEN8(x) \
	((  \
		( ( (x & (1<<1)) >>1) <<2) | \
		( ( (x & (1<<4)) >>4) <<1) | \
		( ( (x & (1<<7)) >>7) <<0) \
	)<<5)

/* given a colour index in range 0..255 gives the Red component */
#define NICK_GET_BLUE8(x) \
	(( \
		( ( (x & (1<<2)) >>2) <<1) | \
		( ( (x & (1<<5)) >>5) <<0) \
	)<<6)


/* Nick executes a Display list, in the form of a list of Line Parameter
Tables, this is the form of the data */
struct LPT_ENTRY
{
	unsigned char SC;       /* scanlines in this modeline (two's complement) */
	unsigned char MB;       /* the MODEBYTE (defines video display mode) */
	unsigned char LM;       /* left margin etc */
	unsigned char RM;       /* right margin etc */
	unsigned char LD1L; /* (a7..a0) of line data pointer LD1 */
	unsigned char LD1H; /* (a8..a15) of line data pointer LD1 */
	unsigned char LD2L; /* (a7..a0) of line data pointer LD2 */
	unsigned char LD2H; /* (a8..a15) of line data pointer LD2 */
	unsigned char COL[8];   /* COL0..COL7 */
};

struct NICK_STATE
{
	/* horizontal position */
	unsigned char HorizontalClockCount;
	/* current scanline within LPT */
	unsigned char ScanLineCount;

	unsigned char FIXBIAS;
	unsigned char BORDER;
	unsigned char LPL;
	unsigned char LPH;

	unsigned long LD1;
	unsigned long LD2;

	LPT_ENTRY   LPT;

	UINT16 *dest;
	int dest_pos;
	int dest_max_pos;

	unsigned char Reg[16];

	/* first clock visible on left hand side */
	unsigned char FirstVisibleClock;
	/* first clock visible on right hand side */
	unsigned char LastVisibleClock;

	/* given a bit pattern, this will get the pen index */
	unsigned int PenIndexLookup_4Colour[256];
	/* given a bit pattern, this will get the pen index */
	unsigned int PenIndexLookup_16Colour[256];

	UINT8 *videoram;
};

/* colour mode types */
#define NICK_2_COLOUR_MODE  0
#define NICK_4_COLOUR_MODE  1
#define NICK_16_COLOUR_MODE 2
#define NICK_256_COLOUR_MODE    3

/* Display mode types */
#define NICK_VSYNC_MODE 0
#define NICK_PIXEL_MODE 1
#define NICK_ATTR_MODE  2
#define NICK_CH256_MODE 3
#define NICK_CH128_MODE 4
#define NICK_CH64_MODE  5
#define NICK_UNUSED_MODE    6
#define NICK_LPIXEL_MODE    7

/* MODEBYTE defines */
#define NICK_MB_VIRQ            (1<<7)
#define NICK_MB_VRES            (1<<4)
#define NICK_MB_LPT_RELOAD      (1<<0)

/* Left margin defines */
#define NICK_LM_MSBALT          (1<<7)
#define NICK_LM_LSBALT          (1<<6)

/* Right margin defines */
#define NICK_RM_ALTIND1         (1<<7)
#define NICK_RM_ALTIND0         (1<<6)

/* useful macros */
#define NICK_GET_LEFT_MARGIN(x)     (x & 0x03f)
#define NICK_GET_RIGHT_MARGIN(x)    (x & 0x03f)
#define NICK_GET_DISPLAY_MODE(x)    ((x>>1) & 0x07)
#define NICK_GET_COLOUR_MODE(x)     ((x>>5) & 0x03)

#define NICK_RELOAD_LPT(x)          (x & 0x080)
#define NICK_CLOCK_LPT(x)           (x & 0x040)

/* Macros to generate memory address is CHx modes */
/* x = LD2, y = buf1 */
#define ADDR_CH256(x,y)     (((x & 0x0ff)<<8) | (y & 0x0ff))
#define ADDR_CH128(x,y)     (((x & 0x01ff)<<7) | (y & 0x07f))
#define ADDR_CH64(x,y)      (((x & 0x03ff)<<6) | (y & 0x03f))


/*************************************************************/
/* MESS stuff */

// MESS specific
/* fetch a byte from "video ram" at Addr specified */
char ep_state::Nick_FetchByte(unsigned long Addr)
{
	return nick->videoram[Addr & 0x0ffff];
}

// MESS specific
/* 8-bit pixel write! */
void ep_state::nick_write_pixel(int ci)
{
	if (nick->dest_pos < nick->dest_max_pos)
	{
		nick->dest[nick->dest_pos++] = ci;
	}
}

/*****************************************************/


/* Enterprise has 256 colours, all may be on the screen at once!
the NICK_GET_RED8, NICK_GET_GREEN8, NICK_GET_BLUE8 macros
return a 8-bit colour value for the index specified.  */

/* initial the palette */
void ep_state::palette_init()
{
	int i;

	for (i=0; i<256; i++)
	{
		palette_set_color_rgb( machine(), i, NICK_GET_RED8(i), NICK_GET_GREEN8(i), NICK_GET_BLUE8(i) );
	}
}

/* No of highest resolution pixels per "clock" */
#define NICK_PIXELS_PER_CLOCK   16

/* "clocks" per line */
#define NICK_TOTAL_CLOCKS_PER_LINE  64

/* we align based on the clocks */
void ep_state::Nick_CalcVisibleClocks(int Width)
{
	/* number of clocks we can see */
	int NoOfVisibleClocks = Width/NICK_PIXELS_PER_CLOCK;

	nick->FirstVisibleClock =
		(NICK_TOTAL_CLOCKS_PER_LINE - NoOfVisibleClocks)>>1;

	nick->LastVisibleClock = nick->FirstVisibleClock + NoOfVisibleClocks;
}


void ep_state::Nick_Init()
{
	int i;

	for (i=0; i<256; i++)
	{
		int PenIndex;

		PenIndex = (
			(((i & 0x080)>>7)<<0) |
			(((i & 0x08)>>3)<<1)
		);

		nick->PenIndexLookup_4Colour[i] = PenIndex;

		PenIndex = (
			((((i & 0x080)>>7))<<0) |
			((((i & 0x08)>>3))<<1)  |
			((((i & 0x020)>>5))<<2) |
			((((i & 0x02)>>1))<<3)
		);

		nick->PenIndexLookup_16Colour[i] = PenIndex;
	}

	Nick_CalcVisibleClocks(ENTERPRISE_SCREEN_WIDTH);

	//nick->BORDER = 0;
	//nick->FIXBIAS = 0;
}

/* write border colour */
void ep_state::Nick_WriteBorder(int Clocks)
{
	int i;
	int ColIndex = nick->BORDER;

	for (i=0; i<(Clocks<<4); i++)
	{
		nick_write_pixel(ColIndex);
	}
}


void ep_state::Nick_DoLeftMargin()
{
	unsigned char LeftMargin;

	LeftMargin = NICK_GET_LEFT_MARGIN(nick->LPT.LM);

	if (LeftMargin>nick->FirstVisibleClock)
	{
		unsigned char LeftMarginVisible;

		/* some of the left margin is visible */
		LeftMarginVisible = LeftMargin-nick->FirstVisibleClock;

		/* render the border */
		Nick_WriteBorder(LeftMarginVisible);
	}
}

void ep_state::Nick_DoRightMargin()
{
	unsigned char RightMargin;

	RightMargin = NICK_GET_RIGHT_MARGIN(nick->LPT.RM);

	if (RightMargin<nick->LastVisibleClock)
	{
		unsigned char RightMarginVisible;

		/* some of the right margin is visible */
		RightMarginVisible = nick->LastVisibleClock - RightMargin;

		/* render the border */
		Nick_WriteBorder(RightMarginVisible);
	}
}

int ep_state::Nick_GetColourIndex(int PenIndex)
{
	if (PenIndex & 0x08)
	{
		return ((nick->FIXBIAS & 0x01f)<<3) | (PenIndex & 0x07);
	}
	else
	{
		return nick->LPT.COL[PenIndex];
	}
}

void ep_state::Nick_WritePixels2Colour(unsigned char Pen0, unsigned char Pen1, unsigned char DataByte)
{
	int i;
	int ColIndex[2];
	int PenIndex;
	unsigned char Data;

	Data = DataByte;

	ColIndex[0] = Nick_GetColourIndex(Pen0);
	ColIndex[1] = Nick_GetColourIndex(Pen1);

	for (i=0; i<8; i++)
	{
		PenIndex = ColIndex[(Data>>7) & 0x01];

		nick_write_pixel(PenIndex);

		Data = Data<<1;
	}
}

void ep_state::Nick_WritePixels2ColourLPIXEL(unsigned char Pen0, unsigned char Pen1, unsigned char DataByte)
{
	int i;
	int ColIndex[2];
	int PenIndex;
	unsigned char Data;

	Data = DataByte;

	ColIndex[0] = Nick_GetColourIndex(Pen0);
	ColIndex[1] = Nick_GetColourIndex(Pen1);

	for (i=0; i<8; i++)
	{
		PenIndex = ColIndex[(Data>>7) & 0x01];

		nick_write_pixel(PenIndex);
		nick_write_pixel(PenIndex);

		Data = Data<<1;
	}
}


void ep_state::Nick_WritePixels(unsigned char DataByte, unsigned char CharIndex)
{
	int i;

	/* pen index colour 2-C (0,1), 4-C (0..3) 16-C (0..16) */
	int PenIndex;
	/* Col index = EP colour value */
	int PalIndex;
	unsigned char ColourMode = NICK_GET_COLOUR_MODE(nick->LPT.MB);
	unsigned char Data = DataByte;

	switch (ColourMode)
	{
		case NICK_2_COLOUR_MODE:
		{
			int PenOffset = 0;

			/* do before displaying byte */

			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				if (Data & 0x080)
				{
					PenOffset |= 2;
				}

				Data &=~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				if (Data & 0x001)
				{
					PenOffset |= 4;
				}

				Data &=~0x01;
			}

			if (nick->LPT.RM & NICK_RM_ALTIND1)
			{
				if (CharIndex & 0x080)
				{
					PenOffset|=0x02;
				}
			}

#if 0
			if (nick->LPT.RM & NICK_RM_ALTIND0)
			{
				if (Data & 0x040)
				{
					PenOffset|=0x04;
				}
			}
#endif


			Nick_WritePixels2Colour(PenOffset,
				(PenOffset|0x01), Data);
		}
		break;

		case NICK_4_COLOUR_MODE:
		{
			//mame_printf_info("4 colour\r\n");

			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				Data &= ~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				Data &= ~0x01;
			}


			for (i=0; i<4; i++)
			{
				PenIndex = nick->PenIndexLookup_4Colour[Data];
				PalIndex = nick->LPT.COL[PenIndex & 0x03];

				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);

				Data = Data<<1;
			}
		}
		break;

		case NICK_16_COLOUR_MODE:
		{
			//mame_printf_info("16 colour\r\n");

			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				Data &= ~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				Data &= ~0x01;
			}


			for (i=0; i<2; i++)
			{
				PenIndex = nick->PenIndexLookup_16Colour[Data];

				PalIndex = Nick_GetColourIndex(PenIndex);

				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);

				Data = Data<<1;
			}
		}
		break;

		case NICK_256_COLOUR_MODE:
		{
			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				Data &= ~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				Data &= ~0x01;
			}


			PalIndex = Data;

			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);


		}
		break;
	}
}

void ep_state::Nick_WritePixelsLPIXEL(unsigned char DataByte, unsigned char CharIndex)
{
	int i;

	/* pen index colour 2-C (0,1), 4-C (0..3) 16-C (0..16) */
	int PenIndex;
	/* Col index = EP colour value */
	int PalIndex;
	unsigned char ColourMode = NICK_GET_COLOUR_MODE(nick->LPT.MB);
	unsigned char Data = DataByte;

	switch (ColourMode)
	{
		case NICK_2_COLOUR_MODE:
		{
			int PenOffset = 0;

			/* do before displaying byte */

			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				if (Data & 0x080)
				{
					PenOffset |= 2;
				}

				Data &=~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				if (Data & 0x001)
				{
					PenOffset |= 4;
				}

				Data &=~0x01;
			}

			if (nick->LPT.RM & NICK_RM_ALTIND1)
			{
				if (CharIndex & 0x080)
				{
					PenOffset|=0x02;
				}
			}

#if 0
			if (nick->LPT.RM & NICK_RM_ALTIND0)
			{
				if (Data & 0x040)
				{
					PenOffset|=0x04;
				}
			}
#endif


			Nick_WritePixels2ColourLPIXEL(PenOffset,(PenOffset|0x01), Data);
		}
		break;

		case NICK_4_COLOUR_MODE:
		{
			//mame_printf_info("4 colour\r\n");

			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				Data &= ~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				Data &= ~0x01;
			}


			for (i=0; i<4; i++)
			{
				PenIndex = nick->PenIndexLookup_4Colour[Data];
				PalIndex = nick->LPT.COL[PenIndex & 0x03];

				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);

				Data = Data<<1;
			}
		}
		break;

		case NICK_16_COLOUR_MODE:
		{
			//mame_printf_info("16 colour\r\n");

			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				Data &= ~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				Data &= ~0x01;
			}


			for (i=0; i<2; i++)
			{
				PenIndex = nick->PenIndexLookup_16Colour[Data];

				PalIndex = Nick_GetColourIndex(PenIndex);

				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);
				nick_write_pixel(PalIndex);

				Data = Data<<1;
			}
		}
		break;

		case NICK_256_COLOUR_MODE:
		{
			/* left margin attributes */
			if (nick->LPT.LM & NICK_LM_MSBALT)
			{
				Data &= ~0x080;
			}

			if (nick->LPT.LM & NICK_LM_LSBALT)
			{
				Data &= ~0x01;
			}


			PalIndex = Data;

			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);

			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);
			nick_write_pixel(PalIndex);


		}
		break;
	}
}


void ep_state::Nick_DoPixel(int ClocksVisible)
{
	int i;
	unsigned char Buf1, Buf2;

	for (i=0; i<ClocksVisible; i++)
	{
		Buf1 = Nick_FetchByte(nick->LD1);
		nick->LD1++;

		Buf2 = Nick_FetchByte(nick->LD1);
		nick->LD1++;

		Nick_WritePixels(Buf1, Buf1);

		Nick_WritePixels(Buf2, Buf1);
	}
}


void ep_state::Nick_DoLPixel(int ClocksVisible)
{
	int i;
	unsigned char Buf1;

	for (i=0; i<ClocksVisible; i++)
	{
		Buf1 = Nick_FetchByte(nick->LD1);
		nick->LD1++;

		Nick_WritePixelsLPIXEL(Buf1, Buf1);
	}
}

void ep_state::Nick_DoAttr(int ClocksVisible)
{
	int i;
	unsigned char Buf1, Buf2;

	for (i=0; i<ClocksVisible; i++)
	{
		Buf1 = Nick_FetchByte(nick->LD1);
		nick->LD1++;

		Buf2 = Nick_FetchByte(nick->LD2);
		nick->LD2++;

		{
			unsigned char BackgroundColour = ((Buf1>>4) & 0x0f);
			unsigned char ForegroundColour = (Buf1 & 0x0f);

			Nick_WritePixels2ColourLPIXEL(BackgroundColour, ForegroundColour, Buf2);
		}
	}
}

void ep_state::Nick_DoCh256(int ClocksVisible)
{
	int i;
	unsigned char Buf1, Buf2;

	for (i=0; i<ClocksVisible; i++)
	{
		Buf1 = Nick_FetchByte(nick->LD1);
		nick->LD1++;
		Buf2 = Nick_FetchByte(ADDR_CH256(nick->LD2, Buf1));

		Nick_WritePixelsLPIXEL(Buf2, Buf1);
	}
}

void ep_state::Nick_DoCh128(int ClocksVisible)
{
	int i;
	unsigned char Buf1, Buf2;

	for (i=0; i<ClocksVisible; i++)
	{
		Buf1 = Nick_FetchByte(nick->LD1);
		nick->LD1++;
		Buf2 = Nick_FetchByte(ADDR_CH128(nick->LD2, Buf1));

		Nick_WritePixelsLPIXEL(Buf2, Buf1);
	}
}

void ep_state::Nick_DoCh64(int ClocksVisible)
{
	int i;
	unsigned char Buf1, Buf2;

	for (i=0; i<ClocksVisible; i++)
	{
		Buf1 = Nick_FetchByte(nick->LD1);
		nick->LD1++;
		Buf2 = Nick_FetchByte(ADDR_CH64(nick->LD2, Buf1));

		Nick_WritePixelsLPIXEL(Buf2, Buf1);
	}
}


void ep_state::Nick_DoDisplay()
{
	LPT_ENTRY *pLPT = &nick->LPT;
	unsigned char ClocksVisible;
	unsigned char RightMargin, LeftMargin;

	LeftMargin = NICK_GET_LEFT_MARGIN(pLPT->LM);
	RightMargin = NICK_GET_RIGHT_MARGIN(pLPT->RM);

	ClocksVisible = RightMargin - LeftMargin;

	if (ClocksVisible!=0)
	{
		unsigned char DisplayMode;

		/* get display mode */
		DisplayMode = NICK_GET_DISPLAY_MODE(pLPT->MB);

		if (nick->ScanLineCount == 0)   // ||
			//((pLPT->MB & NICK_MB_VRES)==0))
		{
			/* doing first line */
			/* reload LD1, and LD2 (if necessary) regardless of display mode */
			nick->LD1 = (pLPT->LD1L & 0x0ff) |
					((pLPT->LD1H & 0x0ff)<<8);

			if ((DisplayMode != NICK_LPIXEL_MODE) && (DisplayMode != NICK_PIXEL_MODE))
			{
				/* lpixel and pixel modes don't use LD2 */
				nick->LD2 = (pLPT->LD2L & 0x0ff) |
					((pLPT->LD2H & 0x0ff)<<8);
			}
		}
		else
		{
			/* not first line */

			switch (DisplayMode)
			{
				case NICK_ATTR_MODE:
				{
					/* reload LD1 */
					nick->LD1 = (pLPT->LD1L & 0x0ff) |
					((pLPT->LD1H & 0x0ff)<<8);
				}
				break;

				case NICK_CH256_MODE:
				case NICK_CH128_MODE:
				case NICK_CH64_MODE:
				{
					/* reload LD1 */
					nick->LD1 = (pLPT->LD1L & 0x0ff) |
						((pLPT->LD1H & 0x0ff)<<8);
					nick->LD2++;
				}
				break;

				default:
					break;
			}
		}

		switch (DisplayMode)
		{
			case NICK_PIXEL_MODE:
			{
				Nick_DoPixel(ClocksVisible);
			}
			break;

			case NICK_ATTR_MODE:
			{
				//mame_printf_info("attr mode\r\n");
				Nick_DoAttr(ClocksVisible);
			}
			break;

			case NICK_CH256_MODE:
			{
				//mame_printf_info("ch256 mode\r\n");
				Nick_DoCh256(ClocksVisible);
			}
			break;

			case NICK_CH128_MODE:
			{
				Nick_DoCh128(ClocksVisible);
			}
			break;

			case NICK_CH64_MODE:
			{
				//mame_printf_info("ch64 mode\r\n");
				Nick_DoCh64(ClocksVisible);
			}
			break;

			case NICK_LPIXEL_MODE:
			{
				Nick_DoLPixel(ClocksVisible);
			}
			break;

			default:
				break;
		}
	}
}

void ep_state::Nick_UpdateLPT()
{
	unsigned long CurLPT;

	CurLPT = (nick->LPL & 0x0ff) | ((nick->LPH & 0x0f)<<8);
	CurLPT++;
	nick->LPL = CurLPT & 0x0ff;
	nick->LPH = (nick->LPH & 0x0f0) | ((CurLPT>>8) & 0x0f);
}


void ep_state::Nick_ReloadLPT()
{
	unsigned long LPT_Addr;

		/* get addr of LPT */
		LPT_Addr = ((nick->LPL & 0x0ff)<<4) | ((nick->LPH & 0x0f)<<(8+4));

		/* update internal LPT state */
		nick->LPT.SC = Nick_FetchByte(LPT_Addr);
		nick->LPT.MB = Nick_FetchByte(LPT_Addr+1);
		nick->LPT.LM = Nick_FetchByte(LPT_Addr+2);
		nick->LPT.RM = Nick_FetchByte(LPT_Addr+3);
		nick->LPT.LD1L = Nick_FetchByte(LPT_Addr+4);
		nick->LPT.LD1H = Nick_FetchByte(LPT_Addr+5);
		nick->LPT.LD2L = Nick_FetchByte(LPT_Addr+6);
		nick->LPT.LD2H = Nick_FetchByte(LPT_Addr+7);
		nick->LPT.COL[0] = Nick_FetchByte(LPT_Addr+8);
		nick->LPT.COL[1] = Nick_FetchByte(LPT_Addr+9);
		nick->LPT.COL[2] = Nick_FetchByte(LPT_Addr+10);
		nick->LPT.COL[3] = Nick_FetchByte(LPT_Addr+11);
		nick->LPT.COL[4] = Nick_FetchByte(LPT_Addr+12);
		nick->LPT.COL[5] = Nick_FetchByte(LPT_Addr+13);
		nick->LPT.COL[6] = Nick_FetchByte(LPT_Addr+14);
		nick->LPT.COL[7] = Nick_FetchByte(LPT_Addr+15);
}

/* call here to render a line of graphics */
void ep_state::Nick_DoLine()
{
	unsigned char ScanLineCount;

	if ((nick->LPT.MB & NICK_MB_LPT_RELOAD)!=0)
	{
		/* reload LPT */

		nick->LPL = nick->Reg[2];
		nick->LPH = nick->Reg[3];

		Nick_ReloadLPT();
	}

	/* left border */
	Nick_DoLeftMargin();

	/* do visible part */
	Nick_DoDisplay();

	/* right border */
	Nick_DoRightMargin();

	// 0x0f7 is first!
	/* scan line count for this LPT */
	ScanLineCount = ((~nick->LPT.SC)+1) & 0x0ff;

	//printf("ScanLineCount %02x\r\n",ScanLineCount);

	/* update count of scanlines done so far */
	nick->ScanLineCount++;

	if (((unsigned char)nick->ScanLineCount) ==
		((unsigned char)ScanLineCount))
	{
		/* done all scanlines of this Line Parameter Table, get next */


		nick->ScanLineCount = 0;

		Nick_UpdateLPT();
		Nick_ReloadLPT();


	}
}

WRITE8_MEMBER( ep_state::epnick_reg_w )
{
	//mame_printf_info("Nick write %02x %02x\r\n",offset, data);

	/* write to a nick register */
	nick->Reg[offset & 0x0f] = data;

	if ((offset == 0x03) || (offset == 0x02))
	{
		/* write LPH */

		/* reload LPT base? */
		//if (NICK_RELOAD_LPT(data))
		{
			/* reload LPT base pointer */
			nick->LPL = nick->Reg[2];
			nick->LPH = nick->Reg[3];

			Nick_ReloadLPT();
		}
	}

	if (offset == 0x01)
	{
		nick->BORDER = data;
	}

	if (offset == 0x00)
	{
		nick->FIXBIAS = data;
	}
}

void ep_state::Nick_DoScreen(bitmap_ind16 &bm)
{
	int line = 0;

	do
	{
		/* set write address for line */
		nick->dest = &bm.pix16(line);
		nick->dest_pos = 0;
		nick->dest_max_pos = bm.width();

		/* write line */
		Nick_DoLine();

		/* next line */
		line++;
	}
	while (((nick->LPT.MB & 0x080)==0) && (line<ENTERPRISE_SCREEN_HEIGHT));

}


void ep_state::video_start()
{
	nick = auto_alloc_clear(machine(), NICK_STATE);

	nick->videoram = machine().device<ram_device>(RAM_TAG)->pointer();
	Nick_Init();
	machine().primary_screen->register_screen_bitmap(m_bitmap);
}


UINT32 ep_state::screen_update_epnick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	Nick_DoScreen(m_bitmap);
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
