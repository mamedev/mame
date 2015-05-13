// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*
    video/dgn_beta.c

The Dragon Beta uses a 68B45 for it's display generation, this is used in the
conventional wat with a character generator ROM in the two text modes, which are
standard 40x25 and 80x25. In adition to the 6845 there is some TTL logic which
provides colour and attributes. In text modes the video ram is organised as pairs
of character and attribute, in alternate bytes.

The attributes decode as follows :-

    7-6-5-4-3-2-1-0
    f-u-F-F-F-B-B-B

    f=flash
    u=underline

    FFF = foreground colour
    BBB = bakcground colour

        000 black
        001 red
        010 green
        011 yellow
        100 blue
        101 magenta
        110 cyan
        111 white

    If flash is true, foreground and background colours will be exchanged.

It is interesting to note that the 6845 uses 16 bit wide access to the ram, in contrast
to the 8 bit accesses from the CPUs, this allows each increment of the MA lines to move
2 bytes at a time, and therefore feed both the character rom and the attribute decode
circuit simultaniously.

The RAM addresses are made up of two parts, the MA0..13 from the 6845, plus two output
lines from the 6821 PIA, I28, the lines are BP6 and PB7, with PB7 being the most
significant. This effectivly allows the 6845 access to the first 128K of memory, as there
are 16 address lines, accessing a 16 bit wide memory.

The relationship between how the cpu sees the RAM, and the way the 6845 sees it is simply
CPU addr=2x6845 addr. So for the default video address of $1F000, the CPU sees this as
being at $1F000 (after DAT trasnlation). The 6845 is programmed to start it's MA lines
counting at $3800, plus A14 and A15 being supplied by PB6 and PB7 from I28, gives an address
of $F800, which is the same as $1F000 / 2.

I am currently at this time not sure of how any of the graphics modes, work, this will need
further investigation.

However the modes supported are :-

Text Modes
        width   height  colours
        40  25  8
        80  25  8
Graphics modes
        320 256 4
        320 256 16
        640 512 4
        640 256 4**
        640 512 2

Looking at the parts of the circuit sheet that I have seen it looks like the graphics modes
are driven using a combination of the 6845 MA and RA lines to address more than 64K or memory
which is needed for some of the modes above (specifically, 640x512x4, which needs 80K).

2006-11-30, Text mode is almost completely implemented, in both 40 and 80 column modes.
I have made a start on implementing the graphics modes of the beta, drawing from technical
documents, from the project, this is still a work in progress. I have however managed to get
it to display a distorted graphical image, so I know I am going in the correct direction !

** 2006-12-05, this mode is not documented in any of the printed documentation, however it
is supported and displayed by the graphics test rom, it is basically the 640x512x4 mode with
half the number of vertical lines, and in non-interlaced mode.

It seems that the 640x512 modes operate the 6845 in interlaced mode, just how this affects
the access to the video memory is unclear to me at the moment.

*/

#include "emu.h"
#include "includes/dgn_beta.h"

/* GCtrl bitmasks, infered from bits of Beta schematic */
#define GCtrlWI     0x01
#define GCtrlSWChar 0x02    /* Character set select */
#define GCtrlHiLo   0x04    /* Hi/Lo res graphics, Hi=1, Lo=0 */
#define GCtrlChrGfx 0x08    /* Character=1 / Graphics=0 */
#define GCtrlControl    0x10    /* Control bit, sets direct drive mode */
#define GCtrlFS     0x20    /* labeled F/S, not yet sure of function Fast or Slow scan ? */
#define GCtrlAddrLines  0xC0    /* Top two address lines for text mode */

#define IsTextMode  (m_GCtrl & GCtrlChrGfx) ? 1 : 0                  // Is this text mode ?
#define IsGfx16     ((~m_GCtrl & GCtrlChrGfx) && (~m_GCtrl & GCtrlControl)) ? 1 : 0   // is this 320x256x16bpp mode
#define IsGfx2      ((m_GCtrl & GCtrlHiLo) && (~m_GCtrl & GCtrlFS)) ? 1 : 0       // Is this a 2 colour mode
#define SWChar      (m_GCtrl & GCtrlSWChar)>>1                   // Swchar bit

MC6845_UPDATE_ROW( dgn_beta_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *videoram = m_videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;
	if(IsTextMode)
	{
		UINT8 *chr_gen = memregion("gfx1")->base();
		for ( i = 0; i < x_count; i++ )
		{
			UINT32 offset = ( ( ma + i ) | ((m_GCtrl & GCtrlAddrLines)<<8)) << 1;
			UINT8 chr = videoram[ offset ];
			UINT8 attr = videoram[ offset +1 ];

			/* Extract non-colour attributes, in character set 1, undeline is used */
			/* We will extract the colours below, when we have decoded inverse */
			/* to indicate a double height character */
			int UnderLine=(attr & 0x40) >> 6; // Underline active
			int FlashChar=(attr & 0x80) >> 7; // Flashing char

			// underline is active for character set 0, on character row 9
			int ULActive=(UnderLine && (ra==9) && ~SWChar);

			/* Invert forground and background if flashing char and flash acive */
			int Invert=(FlashChar & m_FlashBit);

			/* Underline inverts flash */
			if (ULActive)
				Invert=~Invert;

			/* Cursor on also inverts */
			if (i == cursor_x)
				Invert=~Invert;

			UINT16 fg = 0;
			UINT16 bg = 0;

			/* Invert colours if invert is true */
			if(!Invert)
			{
				fg  = (attr & 0x38) >> 3;
				bg  = (attr & 0x07);
			}
			else
			{
				bg  = (attr & 0x38) >> 3;
				fg  = (attr & 0x07);
			}



			UINT8 data = chr_gen[ chr * 16 + ra ];

			*p = palette[( data & 0x80 ) ? fg : bg]; p++;
			*p = palette[( data & 0x80 ) ? fg : bg]; p++;
			*p = palette[( data & 0x40 ) ? fg : bg]; p++;
			*p = palette[( data & 0x40 ) ? fg : bg]; p++;
			*p = palette[( data & 0x20 ) ? fg : bg]; p++;
			*p = palette[( data & 0x20 ) ? fg : bg]; p++;
			*p = palette[( data & 0x10 ) ? fg : bg]; p++;
			*p = palette[( data & 0x10 ) ? fg : bg]; p++;
			*p = palette[( data & 0x08 ) ? fg : bg]; p++;
			*p = palette[( data & 0x08 ) ? fg : bg]; p++;
			*p = palette[( data & 0x04 ) ? fg : bg]; p++;
			*p = palette[( data & 0x04 ) ? fg : bg]; p++;
			*p = palette[( data & 0x02 ) ? fg : bg]; p++;
			*p = palette[( data & 0x02 ) ? fg : bg]; p++;
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		}

	}
	else
	{
		for ( i = 0; i < x_count; i++ )
		{
			UINT32 offset = ((((ma + i ) & 0x1FFF) << 3) | (ra & 0x07)) << 1;

			UINT8 Lo = videoram[ offset ];
			UINT8 Hi = videoram[ offset +1 ];
			UINT16 Word = (Hi<<8) | Lo;
			int Red;
			int Green;
			int Blue;
			int Intense;
			int Colour;
			int Dot;

			/* If contol is low then we are plotting 4 bit per pixel, 16 colour mode */
			/* This directly drives the colour output lines, from the pixel value */
			/* If Control is high, then we lookup the colour from the LS670 4x4 bit */
			/* palate register */
			if (IsGfx16)
			{
				Intense =(Lo & 0x0F);
				Red =(Lo & 0xF0)>>4;
				Green   =(Hi & 0x0F);
				Blue    =(Hi & 0xF0)>>4;
				Colour=((Intense&0x08) | (Red&0x08)>>1) | ((Green&0x08)>>2) | ((Blue&0x08)>>3);

				for (Dot=0;Dot<4;Dot++)
				{
					*p = palette[Colour]; p++;
					*p = palette[Colour]; p++;
					*p = palette[Colour]; p++;
					*p = palette[Colour]; p++;

					Intense =Intense<<1;
					Red =Red<<1;
					Green   =Green<<1;
					Blue    =Blue<<1;
				}
			}
			else if (IsGfx2)
			{
				for (Dot=0;Dot<16;Dot=Dot+1)
				{
					Colour=m_ColourRAM[((Word&0x8000)>>15)];

					*p = palette[Colour]; p++;

					Hi=(Word&0x8000) >> 15;
					Word=((Word<<1)&0xFFFE) | Hi;
				}
			}
			else
			{
				for (Dot=0;Dot<8;Dot++)
				{
					Colour=m_ColourRAM[((Word&0x8000)>>14) | ((Word&0x80)>>7)];
					*p = palette[Colour]; p++;
					*p = palette[Colour]; p++;

					Hi=(Word&0x8000) >> 15;
					Word=((Word<<1)&0xFFFE) | Hi;
				}
			}
		}
	}

}

WRITE_LINE_MEMBER(dgn_beta_state::dgnbeta_vsync_changed)
{
	m_beta_VSync=state;
	if (!m_beta_VSync)
	{
		m_FlashCount++;
		if(m_FlashCount==10)
		{
			m_FlashCount=0;         // Reset counter
			m_FlashBit=(!m_FlashBit) & 0x01;    // Invert flash bit.
		}
	}

	dgn_beta_frame_interrupt(state);
}


/* Set video control register from I28 port B, the control register is laid out as */
/* follows :-                                                                      */
/*  bit function                               */
/*  0   WI, unknown                            */
/*  1   Character set select, drives A12 of character rom in text mode     */
/*  2   High (1) or Low(0) resolution if in graphics mode.         */
/*  3   Text (1) or Graphics(0) mode                       */
/*  4   Control bit, Selects between colour palate and drirect drive       */
/*  5   F/S bit, 1=80 bytes/line, 0=40bytes/line               */
/*  6   Effective A14, to ram, in text mode                    */
/*  7   Effective A15, to ram, in text mode                */
/* the top two address lines for the video ram, are supplied by the BB6 and PB7 on */
/* 6821-I28, this allows the 6845 to access the full 64K address range, however    */
/* since the ram data is addressed as a 16bit wide unit, this allows the 6845      */
/* access to the first 128K or ram.                                                */
void dgn_beta_state::dgnbeta_vid_set_gctrl(int data)
{
	m_GCtrl=data;
}


/* Write handler for colour, pallate ram */
WRITE8_MEMBER(dgn_beta_state::dgnbeta_colour_ram_w)
{
	m_ColourRAM[offset]=data&0x0f;          /* Colour ram 4 bit and write only to CPU */
}
