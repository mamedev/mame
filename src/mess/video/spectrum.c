/***************************************************************************

  spectrum.c

  Functions to emulate the video hardware of the ZX Spectrum.

  Changes:

  DJR 08/02/00 - Added support for FLASH 1.
  DJR 16/05/00 - Support for TS2068/TC2048 hires and 64 column modes.
  DJR 19/05/00 - Speeded up Spectrum 128 screen refresh.
  DJR 23/05/00 - Preliminary support for border colour emulation.

***************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/
VIDEO_START_MEMBER(spectrum_state,spectrum)
{
	m_LastDisplayedBorderColor = -1;
	m_frame_invert_count = 25;
	m_frame_number = 0;
	m_flash_invert = 0;

	spectrum_EventList_Initialise(machine(), 30000);

	m_retrace_cycles = SPEC_RETRACE_CYCLES;

	m_screen_location = m_video_ram;
}

VIDEO_START_MEMBER(spectrum_state,spectrum_128)
{
	m_LastDisplayedBorderColor = -1;
	m_frame_invert_count = 25;
	m_frame_number = 0;
	m_flash_invert = 0;

	spectrum_EventList_Initialise(machine(), 30000);

	m_retrace_cycles = SPEC128_RETRACE_CYCLES;
}


/* return the color to be used inverting FLASHing colors if necessary */
INLINE unsigned char get_display_color (unsigned char color, int invert)
{
	if (invert && (color & 0x80))
		return (color & 0xc0) + ((color & 0x38) >> 3) + ((color & 0x07) << 3);
	else
		return color;
}

/* Code to change the FLASH status every 25 frames. Note this must be
   independent of frame skip etc. */
SCREEN_VBLANK( spectrum )
{
	// rising edge
	if (vblank_on)
	{
		spectrum_state *state = screen.machine().driver_data<spectrum_state>();
		EVENT_LIST_ITEM *pItem;
		int NumItems;

		state->m_frame_number++;
		if (state->m_frame_number >= state->m_frame_invert_count)
		{
			state->m_frame_number = 0;
			state->m_flash_invert = !state->m_flash_invert;
		}

		/* Empty event buffer for undisplayed frames noting the last border
           colour (in case colours are not changed in the next frame). */
		NumItems = spectrum_EventList_NumEvents(screen.machine());
		if (NumItems)
		{
			pItem = spectrum_EventList_GetFirstItem(screen.machine());
			spectrum_border_set_last_color ( screen.machine(), pItem[NumItems-1].Event_Data );
			spectrum_EventList_Reset(screen.machine());
			spectrum_EventList_SetOffsetStartTime ( screen.machine(), screen.machine().firstcpu->attotime_to_cycles(screen.scan_period() * screen.vpos()) );
			logerror ("Event log reset in callback fn.\n");
		}
	}
}



/***************************************************************************
  Update the spectrum screen display.

  The screen consists of 312 scanlines as follows:
  64  border lines (the last 48 are actual border lines; the others may be
                    border lines or vertical retrace)
  192 screen lines
  56  border lines

  Each screen line has 48 left border pixels, 256 screen pixels and 48 right
  border pixels.

  Each scanline takes 224 T-states divided as follows:
  128 Screen (reads a screen and ATTR byte [8 pixels] every 4 T states)
  24  Right border
  48  Horizontal retrace
  24  Left border

  The 128K Spectrums have only 63 scanlines before the TV picture (311 total)
  and take 228 T-states per scanline.

***************************************************************************/

INLINE void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

SCREEN_UPDATE_IND16( spectrum )
{
	/* for now do a full-refresh */
	spectrum_state *state = screen.machine().driver_data<spectrum_state>();
	int x, y, b, scrx, scry;
	unsigned short ink, pap;
	unsigned char *attr, *scr;
		int full_refresh = 1;

	scr=state->m_screen_location;

	for (y=0; y<192; y++)
	{
		scrx=SPEC_LEFT_BORDER;
		scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);
		attr=state->m_screen_location + ((scry>>3)*32) + 0x1800;

		for (x=0;x<32;x++)
		{
			/* Get ink and paper colour with bright */
			if (state->m_flash_invert && (*attr & 0x80))
			{
				ink=((*attr)>>3) & 0x0f;
				pap=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
			}
			else
			{
				ink=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
				pap=((*attr)>>3) & 0x0f;
			}

			for (b=0x80;b!=0;b>>=1)
			{
				if (*scr&b)
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,ink);
				else
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,pap);
			}

			scr++;
			attr++;
		}
	}

	spectrum_border_draw(screen.machine(), bitmap, full_refresh,
		SPEC_TOP_BORDER, SPEC_DISPLAY_YSIZE, SPEC_BOTTOM_BORDER,
		SPEC_LEFT_BORDER, SPEC_DISPLAY_XSIZE, SPEC_RIGHT_BORDER,
		SPEC_LEFT_BORDER_CYCLES, SPEC_DISPLAY_XSIZE_CYCLES,
		SPEC_RIGHT_BORDER_CYCLES, state->m_retrace_cycles, 200, 0xfe);
	return 0;
}


static const rgb_t spectrum_palette[16] = {
	MAKE_RGB(0x00, 0x00, 0x00),
	MAKE_RGB(0x00, 0x00, 0xbf),
	MAKE_RGB(0xbf, 0x00, 0x00),
	MAKE_RGB(0xbf, 0x00, 0xbf),
	MAKE_RGB(0x00, 0xbf, 0x00),
	MAKE_RGB(0x00, 0xbf, 0xbf),
	MAKE_RGB(0xbf, 0xbf, 0x00),
	MAKE_RGB(0xbf, 0xbf, 0xbf),
	MAKE_RGB(0x00, 0x00, 0x00),
	MAKE_RGB(0x00, 0x00, 0xff),
	MAKE_RGB(0xff, 0x00, 0x00),
	MAKE_RGB(0xff, 0x00, 0xff),
	MAKE_RGB(0x00, 0xff, 0x00),
	MAKE_RGB(0x00, 0xff, 0xff),
	MAKE_RGB(0xff, 0xff, 0x00),
	MAKE_RGB(0xff, 0xff, 0xff)
};
/* Initialise the palette */
PALETTE_INIT_MEMBER(spectrum_state,spectrum)
{
	palette_set_colors(machine(), 0, spectrum_palette, ARRAY_LENGTH(spectrum_palette));
}

/***************************************************************************
        Border engine:

        Functions for drawing multi-coloured screen borders using the
        Event List processing.

Changes:

28/05/2000 DJR - Initial implementation.
08/06/2000 DJR - Now only uses events with the correct ID value.
28/06/2000 DJR - draw_border now uses full_refresh flag.

***************************************************************************/

/* Force the border to be redrawn on the next frame */
void spectrum_border_force_redraw (running_machine &machine)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	state->m_LastDisplayedBorderColor = -1;
}

/* Set the last border colour to have been displayed. Used when loading snap
   shots and to record the last colour change in a frame that was skipped. */
void spectrum_border_set_last_color(running_machine &machine, int NewColor)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	state->m_CurrBorderColor = NewColor;
}

void spectrum_border_draw(running_machine &machine, bitmap_ind16 &bitmap,
	int full_refresh,               /* Full refresh flag */
	int TopBorderLines,             /* Border lines before actual screen */
	int ScreenLines,                /* Screen height in pixels */
	int BottomBorderLines,          /* Border lines below screen */
	int LeftBorderPixels,           /* Border pixels to the left of each screen line */
	int ScreenPixels,               /* Width of actual screen in pixels */
	int RightBorderPixels,          /* Border pixels to the right of each screen line */
	int LeftBorderCycles,           /* Cycles taken to draw left border of each scan line */
	int ScreenCycles,               /* Cycles taken to draw screen data part of each scan line */
	int RightBorderCycles,          /* Cycles taken to draw right border of each scan line */
	int HorizontalRetraceCycles,    /* Cycles taken to return to LHS of CRT after each scan line */
	int VRetraceTime,               /* Cycles taken before start of first border line */
	int EventID)                    /* Event ID of border messages */
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	EVENT_LIST_ITEM *pItem;
	int TotalScreenHeight = TopBorderLines+ScreenLines+BottomBorderLines;
	int TotalScreenWidth = LeftBorderPixels+ScreenPixels+RightBorderPixels;
	int DisplayCyclesPerLine = LeftBorderCycles+ScreenCycles+RightBorderCycles;
	int CyclesPerLine = DisplayCyclesPerLine+HorizontalRetraceCycles;
	int CyclesSoFar = 0;
	int NumItems, CurrItem = 0, NextItem;
	int Count, ScrX, NextScrX, ScrY;
	rectangle r;

	pItem = spectrum_EventList_GetFirstItem(machine);
	NumItems = spectrum_EventList_NumEvents(machine);

	for (Count = 0; Count < NumItems; Count++)
	{
//      logerror ("Event no %05d, ID = %04x, data = %04x, time = %ld\n", Count, pItem[Count].Event_ID, pItem[Count].Event_Data, (long) pItem[Count].Event_Time);
	}

	/* Find the first and second events with the correct ID */
	while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID))
		CurrItem++;
	NextItem = CurrItem + 1;
	while ((NextItem < NumItems) && (pItem[NextItem].Event_ID != EventID))
		NextItem++;

	/* Single border colour */
	if ((CurrItem < NumItems) && (NextItem >= NumItems))
		state->m_CurrBorderColor = pItem[CurrItem].Event_Data;

	if ((NextItem >= NumItems) && (state->m_CurrBorderColor==state->m_LastDisplayedBorderColor) && !full_refresh)
	{
		/* Do nothing if border colour has not changed */
	}
	else if (NextItem >= NumItems)
	{
			/* Single border colour - this is not strictly correct as the
                colour change may have occurred midway through the frame
                or after the last visible border line however the whole
                border would be redrawn in the correct colour during the
                next frame anyway! */
		r.set(0, TotalScreenWidth-1, 0, TopBorderLines-1);
		bitmap.fill(machine.pens[state->m_CurrBorderColor], r);

		r.set(0, LeftBorderPixels-1, TopBorderLines, TopBorderLines+ScreenLines-1);
		bitmap.fill(machine.pens[state->m_CurrBorderColor], r);

		r.setx(LeftBorderPixels+ScreenPixels, TotalScreenWidth-1);
		bitmap.fill(machine.pens[state->m_CurrBorderColor], r);

		r.set(0, TotalScreenWidth-1, TopBorderLines+ScreenLines, TotalScreenHeight-1);
		bitmap.fill(machine.pens[state->m_CurrBorderColor], r);

//          logerror ("Setting border colour to %d (Last = %d, Full Refresh = %d)\n", state->m_CurrBorderColor, state->m_LastDisplayedBorderColor, full_refresh);
		state->m_LastDisplayedBorderColor = state->m_CurrBorderColor;
	}
	else
	{
		/* Multiple border colours */

		/* Process entries before first displayed line */
		while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time <= VRetraceTime))
		{
			state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
			do {
				CurrItem++;
			} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
		}

		/* Draw top border */
		CyclesSoFar = VRetraceTime;
		for (ScrY = 0; ScrY < TopBorderLines; ScrY++)
		{
			r.min_x = 0;
			r.min_y = r.max_y = ScrY;
			if ((CurrItem >= NumItems) || (pItem[CurrItem].Event_Time >= (CyclesSoFar+DisplayCyclesPerLine)))
			{
				/* Single colour on line */
				r.max_x = TotalScreenWidth-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}
			else
			{
				/* Multiple colours on a line */
				ScrX = (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)TotalScreenWidth / (float)DisplayCyclesPerLine;
				r.max_x = ScrX-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));

				while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time < (CyclesSoFar+DisplayCyclesPerLine)))
				{
					NextScrX = (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)TotalScreenWidth / (float)DisplayCyclesPerLine;
					r.setx(ScrX, NextScrX-1);
					bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
					ScrX = NextScrX;
					state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
					do {
						CurrItem++;
					} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
				}
				r.setx(ScrX, TotalScreenWidth-1);
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}

			/* Process colour changes during horizontal retrace */
			CyclesSoFar+= CyclesPerLine;
			while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time <= CyclesSoFar))
			{
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
			}
		}

		/* Draw left and right borders next to screen lines */
		for (ScrY = TopBorderLines; ScrY < (TopBorderLines+ScreenLines); ScrY++)
		{
			/* Draw left hand border */
			r.min_x = 0;
			r.min_y = r.max_y = ScrY;

			if ((CurrItem >= NumItems) || (pItem[CurrItem].Event_Time >= (CyclesSoFar+LeftBorderCycles)))
			{
				/* Single colour */
				r.max_x = LeftBorderPixels-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}
			else
			{
				/* Multiple colours */
				ScrX = (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)LeftBorderPixels / (float)LeftBorderCycles;
				r.max_x = ScrX-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));

				while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time < (CyclesSoFar+LeftBorderCycles)))
				{
					NextScrX = (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)LeftBorderPixels / (float)LeftBorderCycles;
					r.setx(ScrX, NextScrX-1);
					bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
					ScrX = NextScrX;
					state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
					do {
						CurrItem++;
					} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
				}
				r.setx(ScrX, LeftBorderPixels-1);
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}

			/* Process colour changes during screen draw */
			while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time <= (CyclesSoFar+LeftBorderCycles+ScreenCycles)))
			{
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
			}

			/* Draw right hand border */
			r.min_x = LeftBorderPixels+ScreenPixels;
			if ((CurrItem >= NumItems) || (pItem[CurrItem].Event_Time >= (CyclesSoFar+DisplayCyclesPerLine)))
			{
				/* Single colour */
				r.max_x = TotalScreenWidth-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}
			else
			{
				/* Multiple colours */
				ScrX = LeftBorderPixels + ScreenPixels + (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)RightBorderPixels / (float)RightBorderCycles;
				r.max_x = ScrX-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));

				while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time < (CyclesSoFar+DisplayCyclesPerLine)))
				{
					NextScrX = LeftBorderPixels + ScreenPixels + (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)RightBorderPixels / (float)RightBorderCycles;
					r.setx(ScrX, NextScrX-1);
					bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
					ScrX = NextScrX;
					state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
					do {
						CurrItem++;
					} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
				}
				r.setx(ScrX, TotalScreenWidth-1);
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}

			/* Process colour changes during horizontal retrace */
			CyclesSoFar+= CyclesPerLine;
			while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time <= CyclesSoFar))
			{
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
			}
		}

		/* Draw bottom border */
		for (ScrY = TopBorderLines+ScreenLines; ScrY < TotalScreenHeight; ScrY++)
		{
			r.min_x = 0;
			r.min_y = r.max_y = ScrY;
			if ((CurrItem >= NumItems) || (pItem[CurrItem].Event_Time >= (CyclesSoFar+DisplayCyclesPerLine)))
			{
				/* Single colour on line */
				r.max_x = TotalScreenWidth-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}
			else
			{
				/* Multiple colours on a line */
				ScrX = (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)TotalScreenWidth / (float)DisplayCyclesPerLine;
				r.max_x = ScrX-1;
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));

				while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time < (CyclesSoFar+DisplayCyclesPerLine)))
				{
					NextScrX = (int)(pItem[CurrItem].Event_Time - CyclesSoFar) * (float)TotalScreenWidth / (float)DisplayCyclesPerLine;
					r.setx(ScrX, NextScrX-1);
					bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
					ScrX = NextScrX;
					state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
					do {
						CurrItem++;
					} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
				}
				r.setx(ScrX, TotalScreenWidth-1);
				bitmap.fill(machine.pens[state->m_CurrBorderColor], r);
			}

			/* Process colour changes during horizontal retrace */
			CyclesSoFar+= CyclesPerLine;
			while ((CurrItem < NumItems) && (pItem[CurrItem].Event_Time <= CyclesSoFar))
			{
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
				do {
					CurrItem++;
				} while ((CurrItem < NumItems) && (pItem[CurrItem].Event_ID != EventID));
			}
		}

		/* Process colour changes after last displayed line */
		while (CurrItem < NumItems)
		{
			if (pItem[CurrItem].Event_ID == EventID)
				state->m_CurrBorderColor = pItem[CurrItem].Event_Data;
			CurrItem++;
		}

		/* Set value to ensure redraw on next frame */
		state->m_LastDisplayedBorderColor = -1;

//          logerror ("Multi coloured border drawn (last colour = %d)\n", CurrBorderColor);
	}

	/* Assume all other routines have processed their data from the list */
	spectrum_EventList_Reset(machine);
	spectrum_EventList_SetOffsetStartTime ( machine, machine.firstcpu->attotime_to_cycles(machine.primary_screen->scan_period() * machine.primary_screen->vpos()));
}


/* initialise */

/* if the CPU is the controlling factor, the size of the buffer
can be setup as:

Number_of_CPU_Cycles_In_A_Frame/Minimum_Number_Of_Cycles_Per_Instruction */
void spectrum_EventList_Initialise(running_machine &machine, int NumEntries)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	state->m_pEventListBuffer = auto_alloc_array(machine, char, NumEntries);
	state->m_TotalEvents = NumEntries;
	state->m_CyclesPerFrame = 0;
	spectrum_EventList_Reset(machine);
}

/* reset the change list */
void spectrum_EventList_Reset(running_machine &machine)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	state->m_NumEvents = 0;
	state->m_pCurrentItem = (EVENT_LIST_ITEM *)state->m_pEventListBuffer;
}


#ifdef UNUSED_FUNCTION
/* add an event to the buffer */
void EventList_AddItem(running_machine &machine, int ID, int Data, int Time)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	if (state->m_NumEvents < state->m_TotalEvents)
	{
		/* setup item only if there is space in the buffer */
		state->m_pCurrentItem->Event_ID = ID;
		state->m_pCurrentItem->Event_Data = Data;
		state->m_pCurrentItem->Event_Time = Time;

		state->m_pCurrentItem++;
		state->m_NumEvents++;
	}
}
#endif

/* set the start time for use with EventList_AddItemOffset usually this will
   be cpu_getcurrentcycles() at the time that the screen is being refreshed */
void spectrum_EventList_SetOffsetStartTime(running_machine &machine, int StartTime)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	state->m_LastFrameStartTime = StartTime;
}

/* add an event to the buffer with a time index offset from a specified time */
void spectrum_EventList_AddItemOffset(running_machine &machine, int ID, int Data, int Time)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();

	if (!state->m_CyclesPerFrame)
		state->m_CyclesPerFrame = (int)(machine.firstcpu->unscaled_clock() / machine.primary_screen->frame_period().attoseconds);	//totalcycles();    //_(int)(cpunum_get_clock(0) / machine.config()->frames_per_second);

	if (state->m_NumEvents < state->m_TotalEvents)
	{
		/* setup item only if there is space in the buffer */
		state->m_pCurrentItem->Event_ID = ID;
		state->m_pCurrentItem->Event_Data = Data;

		Time -= state->m_LastFrameStartTime;
		if ((Time < 0) || ((Time == 0) && state->m_NumEvents))
			Time += state->m_CyclesPerFrame;
		state->m_pCurrentItem->Event_Time = Time;

		state->m_pCurrentItem++;
		state->m_NumEvents++;
	}
}

/* get number of events */
int spectrum_EventList_NumEvents(running_machine &machine)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	return state->m_NumEvents;
}

/* get first item in buffer */
EVENT_LIST_ITEM *spectrum_EventList_GetFirstItem(running_machine &machine)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	return (EVENT_LIST_ITEM *)state->m_pEventListBuffer;
}
