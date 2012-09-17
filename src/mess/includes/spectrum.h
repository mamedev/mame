/*****************************************************************************
 *
 * includes/spectrum.h
 *
 ****************************************************************************/

#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include "imagedev/snapquik.h"
#include "imagedev/cartslot.h"

/* Spectrum crystals */

#define X1 XTAL_14MHz		// Main clock
#define X2 XTAL_4_433619MHz // PAL color subcarrier

/* Spectrum screen size in pixels */
#define SPEC_UNSEEN_LINES  16   /* Non-visible scanlines before first border
                                   line. Some of these may be vertical retrace. */
#define SPEC_TOP_BORDER    48   /* Number of border lines before actual screen */
#define SPEC_DISPLAY_YSIZE 192  /* Vertical screen resolution */
#define SPEC_BOTTOM_BORDER 56   /* Number of border lines at bottom of screen */
#define SPEC_SCREEN_HEIGHT (SPEC_TOP_BORDER + SPEC_DISPLAY_YSIZE + SPEC_BOTTOM_BORDER)

#define SPEC_LEFT_BORDER   48   /* Number of left hand border pixels */
#define SPEC_DISPLAY_XSIZE 256  /* Horizontal screen resolution */
#define SPEC_RIGHT_BORDER  48   /* Number of right hand border pixels */
#define SPEC_SCREEN_WIDTH (SPEC_LEFT_BORDER + SPEC_DISPLAY_XSIZE + SPEC_RIGHT_BORDER)

#define SPEC_LEFT_BORDER_CYCLES   24   /* Cycles to display left hand border */
#define SPEC_DISPLAY_XSIZE_CYCLES 128  /* Horizontal screen resolution */
#define SPEC_RIGHT_BORDER_CYCLES  24   /* Cycles to display right hand border */
#define SPEC_RETRACE_CYCLES       48   /* Cycles taken for horizonal retrace */
#define SPEC_CYCLES_PER_LINE      224  /* Number of cycles to display a single line */

/* 128K machines take an extra 4 cycles per scan line - add this to retrace */
#define SPEC128_UNSEEN_LINES    15
#define SPEC128_RETRACE_CYCLES  52
#define SPEC128_CYCLES_PER_LINE 228

/* Border sizes for TS2068. These are guesses based on the number of cycles
   available per frame. */
#define TS2068_TOP_BORDER    32
#define TS2068_BOTTOM_BORDER 32
#define TS2068_SCREEN_HEIGHT (TS2068_TOP_BORDER + SPEC_DISPLAY_YSIZE + TS2068_BOTTOM_BORDER)

/* Double the border sizes to maintain ratio of screen to border */
#define TS2068_LEFT_BORDER   96   /* Number of left hand border pixels */
#define TS2068_DISPLAY_XSIZE 512  /* Horizontal screen resolution */
#define TS2068_RIGHT_BORDER  96   /* Number of right hand border pixels */
#define TS2068_SCREEN_WIDTH (TS2068_LEFT_BORDER + TS2068_DISPLAY_XSIZE + TS2068_RIGHT_BORDER)

struct EVENT_LIST_ITEM
{
	/* driver defined ID for this write */
	int	Event_ID;
	/* driver defined data for this write */
	int	Event_Data;
	/* time at which this write occurred */
	int Event_Time;
};


class spectrum_state : public driver_device
{
public:
	spectrum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"){ }

	int m_port_fe_data;
	int m_port_7ffd_data;
	int m_port_1ffd_data;	/* scorpion and plus3 */
	int m_port_ff_data; /* Display enhancement control */
	int m_port_f4_data; /* Horizontal Select Register */

	int m_floppy;

	/* video support */
	int m_frame_invert_count;
	int m_frame_number;    /* Used for handling FLASH 1 */
	int m_flash_invert;
	UINT8 m_retrace_cycles;
	optional_shared_ptr<UINT8> m_video_ram;
	UINT8 *m_screen_location;

	int m_ROMSelection;

	/* Last border colour output in the previous frame */
	int m_CurrBorderColor;
	int m_LastDisplayedBorderColor; /* Negative value indicates redraw */

	EVENT_LIST_ITEM *m_pCurrentItem;
	int m_NumEvents;
	int m_TotalEvents;
	char *m_pEventListBuffer;
	int m_LastFrameStartTime;
	int m_CyclesPerFrame;

	UINT8 *m_ram_0000;
	UINT8 m_ram_disabled_by_beta;
	DECLARE_WRITE8_MEMBER(spectrum_port_fe_w);
	DECLARE_READ8_MEMBER(spectrum_port_fe_r);
	DECLARE_READ8_MEMBER(spectrum_port_1f_r);
	DECLARE_READ8_MEMBER(spectrum_port_7f_r);
	DECLARE_READ8_MEMBER(spectrum_port_df_r);
	DECLARE_READ8_MEMBER(spectrum_port_ula_r);
	DECLARE_DIRECT_UPDATE_MEMBER(spectrum_direct);
	DECLARE_DRIVER_INIT(spectrum);
	DECLARE_DRIVER_INIT(plus2);
	DECLARE_DRIVER_INIT(plus3);
	DECLARE_MACHINE_RESET(spectrum);
	DECLARE_VIDEO_START(spectrum);
	DECLARE_PALETTE_INIT(spectrum);
	DECLARE_MACHINE_RESET(tc2048);
	DECLARE_VIDEO_START(spectrum_128);
	DECLARE_MACHINE_RESET(spectrum_128);
	DECLARE_MACHINE_RESET(spectrum_plus3);
	DECLARE_MACHINE_RESET(ts2068);
	DECLARE_VIDEO_START(ts2068);
	UINT32 screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tc2048(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ts2068(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_spectrum(screen_device &screen, bool state);
};


/*----------- defined in drivers/spectrum.c -----------*/

INPUT_PORTS_EXTERN( spectrum );
INPUT_PORTS_EXTERN( spec_plus );

MACHINE_CONFIG_EXTERN( spectrum );



/*----------- defined in drivers/spec128.c -----------*/

MACHINE_CONFIG_EXTERN( spectrum_128 );

void spectrum_128_update_memory(running_machine &machine);

/*----------- defined in drivers/specpls3.c -----------*/

void spectrum_plus3_update_memory(running_machine &machine);

/*----------- defined in drivers/timex.c -----------*/

void ts2068_update_memory(running_machine &machine);

/*----------- defined in video/spectrum.c -----------*/









void spectrum_border_force_redraw (running_machine &machine);
void spectrum_border_set_last_color (running_machine &machine, int NewColor);
void spectrum_border_draw(running_machine &machine, bitmap_ind16 &bitmap, int full_refresh,
                int TopBorderLines, int ScreenLines, int BottomBorderLines,
                int LeftBorderPixels, int ScreenPixels, int RightBorderPixels,
                int LeftBorderCycles, int ScreenCycles, int RightBorderCycles,
                int HorizontalRetraceCycles, int VRetraceTime, int EventID);

void spectrum_EventList_Initialise(running_machine &machine, int NumEntries);
void spectrum_EventList_Reset(running_machine &machine);
void spectrum_EventList_SetOffsetStartTime(running_machine &machine, int StartTime);
void spectrum_EventList_AddItemOffset(running_machine &machine, int ID, int Data,int Time);
int spectrum_EventList_NumEvents(running_machine &machine);
EVENT_LIST_ITEM *spectrum_EventList_GetFirstItem(running_machine &machine);

/*----------- defined in video/timex.c -----------*/






#endif /* __SPECTRUM_H__ */
