/*************************************************************************

    ldpr8210.c

    Pioneer PR-8210 laserdisc emulation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************

    Still to do:

        * implement SLOW TRG
        * figure out Simutrek without jump hack
        * figure out serial protocol issues (current hack works nicely)
        * determine actual slow/fast speeds

*************************************************************************/

#include "emu.h"
#include "ldcore.h"
#include "cpu/mcs48/mcs48.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_VBLANK_VBI				0
#define LOG_SERIAL					0
#define LOG_SIMUTREK				0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Overlay constants, related to 720-pixel wide capture */
#define OVERLAY_GROUP0_X				(82.0f / 720.0f)
#define OVERLAY_GROUP1_X				(162.0f / 720.0f)
#define OVERLAY_GROUP2_X				(322.0f / 720.0f)
#define OVERLAY_GROUP3_X				(483.0f / 720.0f)
#define OVERLAY_Y						(104/2)
#define OVERLAY_PIXEL_WIDTH				(4.5f / 720.0f)
#define OVERLAY_PIXEL_HEIGHT			2
#define OVERLAY_X_PIXELS				5
#define OVERLAY_Y_PIXELS				7

/* scanning speeds */
#define SCAN_SPEED						(2000 / 30)			/* 2000 frames/second */
#define SEEK_FAST_SPEED					(4000 / 30)			/* 4000 frames/second */

/* serial timing, mostly from the service manual, derived from the XTAL */
#define SERIAL_CLOCK					XTAL_455kHz
#define SERIAL_0_BIT_TIME				ATTOTIME_IN_HZ((double)SERIAL_CLOCK / 512)
#define SERIAL_1_BIT_TIME				ATTOTIME_IN_HZ((double)SERIAL_CLOCK / 1024)
#define SERIAL_MIDPOINT_TIME			ATTOTIME_IN_HZ((double)SERIAL_CLOCK / 600)
#define SERIAL_MAX_BIT_TIME				ATTOTIME_IN_HZ((double)SERIAL_CLOCK / 4096)
#define SERIAL_MAX_WORD_TIME			ATTOTIME_IN_HZ((double)SERIAL_CLOCK / 11520)
#define SERIAL_REJECT_DUPLICATE_TIME	ATTOTIME_IN_HZ((double)SERIAL_CLOCK / 11520 / 4)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* PIA data */
typedef struct _pioneer_pia pioneer_pia;
struct _pioneer_pia
{
	UINT8				frame[7];				/* (20-26) 7 characters for the chapter/frame */
	UINT8				text[17];				/* (20-30) 17 characters for the display */
	UINT8				control;				/* (40) control lines */
	UINT8				latchdisplay;			/*   flag: set if the display was latched */
	UINT8				portb;					/* (60) port B value (LEDs) */
	UINT8				display;				/* (80) display enable */
	UINT8				porta;					/* (A0) port A value (from serial decoder) */
	UINT8				vbi1;					/* (C0) VBI decoding state 1 */
	UINT8				vbi2;					/* (E0) VBI decoding state 2 */
};


/* Simutrek-specific data */
typedef struct _simutrek_data simutrek_data;
struct _simutrek_data
{
	running_device *cpu;					/* 8748 CPU device */
	UINT8				audio_squelch;			/* audio squelch value */
	UINT8				data;					/* parallel data for simutrek */
	UINT8				data_ready;				/* ready flag for simutrek data */
	UINT8				port2;					/* 8748 port 2 state */
	UINT8				controlnext;			/* latch to control next pair of fields */
	UINT8				controlthis;			/* latched value for our control over the current pair of fields */
};


/* player-specific data */
struct _ldplayer_data
{
	/* serial/command interface processing */
	UINT8				lastcommand;			/* last command seen */
	UINT16				accumulator;			/* bit accumulator */
	attotime			lastcommandtime;		/* time of the last command */
	attotime			lastbittime;			/* time of last bit received */
	attotime			firstbittime;			/* time of first bit in command */

	/* low-level emulation data */
	running_device *cpu;					/* 8049 CPU device */
	attotime			slowtrg;				/* time of the last SLOW TRG */
	pioneer_pia			pia;					/* PIA state */
	UINT8				vsync;					/* live VSYNC state */
	UINT8				port1;					/* 8049 port 1 state */
	UINT8				port2;					/* 8049 port 2 state */

	/* Simutrek-specific data */
	simutrek_data		simutrek;				/* Simutrek-specific data */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void pr8210_init(laserdisc_state *ld);
static void pr8210_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static INT32 pr8210_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static void pr8210_overlay(laserdisc_state *ld, bitmap_t *bitmap);
static void pr8210_control_w(laserdisc_state *ld, UINT8 prev, UINT8 data);

static TIMER_CALLBACK( vsync_off );
static TIMER_CALLBACK( vbi_data_fetch );

static READ8_HANDLER( pr8210_pia_r );
static WRITE8_HANDLER( pr8210_pia_w );
static READ8_HANDLER( pr8210_bus_r );
static WRITE8_HANDLER( pr8210_port1_w );
static WRITE8_HANDLER( pr8210_port2_w );
static READ8_HANDLER( pr8210_t0_r );
static READ8_HANDLER( pr8210_t1_r );

static void overlay_draw_group(bitmap_t *bitmap, const UINT8 *text, int count, float xstart);
static void overlay_erase(bitmap_t *bitmap, float xstart, float xend);
static void overlay_draw_char(bitmap_t *bitmap, UINT8 ch, float xstart);

static void simutrek_init(laserdisc_state *ld);
static void simutrek_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static INT32 simutrek_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static UINT8 simutrek_ready_r(laserdisc_state *ld);
static UINT8 simutrek_status_r(laserdisc_state *ld);
static void simutrek_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data);

static TIMER_CALLBACK( simutrek_latched_data_w );

static READ8_HANDLER( simutrek_port2_r );
static WRITE8_HANDLER( simutrek_port2_w );
static READ8_HANDLER( simutrek_data_r );
static READ8_HANDLER( simutrek_t0_r );



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* bitmaps for the characters */
static const UINT8 text_bitmap[0x40][7] =
{
	{ 0 },									/* @ */
	{ 0x20,0x50,0x88,0x88,0xf8,0x88,0x88 },	/* A */
	{ 0 },									/* B */
	{ 0x70,0x88,0x80,0x80,0x80,0x88,0x70 },	/* C */
	{ 0 },									/* D */
	{ 0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8 },	/* E */
	{ 0xf8,0x80,0x80,0xf0,0x80,0x80,0x80 },	/* F */
	{ 0 },									/* G */
	{ 0x88,0x88,0x88,0xf8,0x88,0x88,0x88 },	/* H */
	{ 0 },									/* I */
	{ 0 },									/* J */
	{ 0 },									/* K */
	{ 0 },									/* L */
	{ 0x88,0xd8,0xa8,0xa8,0xa8,0x88,0x88 },	/* M */
	{ 0 },									/* N */
	{ 0 },									/* O */
	{ 0xf0,0x88,0x88,0xf0,0x80,0x80,0x80 },	/* P */
	{ 0 },									/* Q */
	{ 0xf0,0x88,0x88,0xf0,0xa0,0x90,0x88 },	/* R */
	{ 0x70,0x88,0x80,0x70,0x08,0x88,0x70 },	/* S */
	{ 0 },									/* T */
	{ 0 },									/* U */
	{ 0 },									/* V */
	{ 0 },									/* W */
	{ 0 },									/* X */
	{ 0 },									/* Y */
	{ 0 },									/* Z */
	{ 0 },									/* [ */
	{ 0 },									/* \ */
	{ 0 },									/* ] */
	{ 0 },									/* ^ */
	{ 0 },									/* _ */

	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00 },	/* <space> */
	{ 0 },									/* ! */
	{ 0 },									/* " */
	{ 0 },									/* # */
	{ 0 },									/* $ */
	{ 0 },									/* % */
	{ 0 },									/* & */
	{ 0 },									/* ' */
	{ 0 },									/* ( */
	{ 0 },									/* ) */
	{ 0 },									/* * */
	{ 0 },									/* + */
	{ 0 },									/* , */
	{ 0 },									/* - */
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x40 },	/* . */
	{ 0 },									/* / */
	{ 0x70,0x88,0x88,0x88,0x88,0x88,0x70 },	/* 0 */
	{ 0x20,0x60,0x20,0x20,0x20,0x20,0x70 },	/* 1 */
	{ 0x70,0x88,0x08,0x70,0x80,0x80,0xf8 },	/* 2 */
	{ 0xf8,0x08,0x10,0x30,0x08,0x88,0x70 },	/* 3 */
	{ 0x10,0x30,0x50,0x90,0xf8,0x10,0x10 },	/* 4 */
	{ 0xf8,0x80,0xf0,0x08,0x08,0x88,0x70 },	/* 5 */
	{ 0x78,0x80,0x80,0xf0,0x88,0x88,0x70 },	/* 6 */
	{ 0xf8,0x08,0x08,0x10,0x20,0x40,0x80 },	/* 7 */
	{ 0x70,0x88,0x88,0x70,0x88,0x88,0x70 },	/* 8 */
	{ 0x70,0x88,0x88,0x78,0x08,0x08,0xf0 },	/* 9 */
	{ 0 },									/* : */
	{ 0 },									/* ; */
	{ 0 },									/* < */
	{ 0 },									/* = */
	{ 0 },									/* > */
	{ 0 }									/* ? */
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    update_video_squelch - update the state of
    the video squelch
-------------------------------------------------*/

INLINE void update_video_squelch(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	ldcore_set_video_squelch(ld, (player->port1 & 0x20) != 0);
}


/*-------------------------------------------------
    update_audio_squelch - update the state of
    the audio squelch
-------------------------------------------------*/

INLINE void update_audio_squelch(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	if (player->simutrek.cpu == NULL)
		ldcore_set_audio_squelch(ld, (player->port1 & 0x40) || !(player->pia.portb & 0x01), (player->port1 & 0x40) || !(player->pia.portb & 0x02));
	else
		ldcore_set_audio_squelch(ld, player->simutrek.audio_squelch, player->simutrek.audio_squelch);
}



/***************************************************************************
    PR-8210 ROM AND MACHINE INTERFACES
***************************************************************************/

static ADDRESS_MAP_START( pr8210_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(pr8210_pia_r, pr8210_pia_w)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(pr8210_bus_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(pr8210_port1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(pr8210_port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(pr8210_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(pr8210_t1_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( pr8210 )
	MDRV_CPU_ADD("pr8210", I8049, XTAL_4_41MHz)
	MDRV_CPU_IO_MAP(pr8210_portmap)
MACHINE_DRIVER_END


ROM_START( pr8210 )
	ROM_REGION( 0x800, "pr8210", ROMREGION_LOADBYNAME )
	ROM_LOAD( "pr-8210_mcu_ud6005a.bin", 0x000, 0x800, CRC(120fa83b) SHA1(b514326ca1f52d6d89056868f9d17eabd4e3f31d) )
ROM_END



/***************************************************************************
    PR-8210 PLAYER INTERFACE
***************************************************************************/

const ldplayer_interface pr8210_interface =
{
	LASERDISC_TYPE_PIONEER_PR8210,				/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Pioneer PR-8210",							/* name of the player */
	ROM_NAME(pr8210),							/* pointer to ROM region information */
	MACHINE_DRIVER_NAME(pr8210),				/* pointer to machine configuration */
	pr8210_init,								/* initialization callback */
	pr8210_vsync,								/* vsync callback */
	pr8210_update,								/* update callback */
	pr8210_overlay,								/* overlay callback */
	NULL,										/* parallel data write */
	{											/* single line write: */
		NULL,									/*    LASERDISC_LINE_ENTER */
		pr8210_control_w						/*    LASERDISC_LINE_CONTROL */
	},
	NULL,										/* parallel data read */
	{											/* single line read: */
		NULL,									/*    LASERDISC_LINE_READY */
		NULL,									/*    LASERDISC_LINE_STATUS */
		NULL,									/*    LASERDISC_LINE_COMMAND */
		NULL,									/*    LASERDISC_LINE_DATA_AVAIL */
	}
};



/***************************************************************************
    PIONEER PR-8210 IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    pr8210_init - player-specific initialization
-------------------------------------------------*/

static void pr8210_init(laserdisc_state *ld)
{
	astring tempstring;
	attotime curtime = timer_get_time(ld->device->machine);
	ldplayer_data *player = ld->player;

	/* reset our state */
	memset(player, 0, sizeof(*player));
	player->lastcommandtime = curtime;
	player->firstbittime = curtime;
	player->lastbittime = curtime;
	player->slowtrg = curtime;

	/* find our CPU */
	player->cpu = ld->device->subdevice("pr8210");
	assert(player->cpu != NULL);

	/* we don't have the Simutrek player overrides */
	player->simutrek.cpu = NULL;
	player->simutrek.audio_squelch = FALSE;
}


/*-------------------------------------------------
    pr8210_vsync - VSYNC callback, called at the
    start of the blanking period
-------------------------------------------------*/

static void pr8210_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_data *player = ld->player;

	/* logging */
	if (LOG_VBLANK_VBI)
	{
		if ((vbi->line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
			printf("%3d:VSYNC(%d,%05d)\n", ld->screen->vpos(), fieldnum, VBI_CAV_PICTURE(vbi->line1718));
		else
			printf("%3d:VSYNC(%d)\n", ld->screen->vpos(), fieldnum);
	}

	/* signal VSYNC and set a timer to turn it off */
	player->vsync = TRUE;
	timer_set(ld->device->machine, attotime_mul(ld->screen->scan_period(), 4), ld, 0, vsync_off);

	/* also set a timer to fetch the VBI data when it is ready */
	timer_set(ld->device->machine, ld->screen->time_until_pos(19*2), ld, 0, vbi_data_fetch);
}


/*-------------------------------------------------
    pr8210_update - update callback, called on
    the first visible line of the frame
-------------------------------------------------*/

static INT32 pr8210_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_data *player = ld->player;
	UINT8 spdl_on = !(player->port1 & 0x10);

	/* logging */
	if (LOG_VBLANK_VBI)
		printf("%3d:Update(%d)\n", ld->screen->vpos(), fieldnum);

	/* if the spindle is on, we advance by 1 track after completing field #1 */
	return (spdl_on) ? fieldnum : 0;
}


/*-------------------------------------------------
    pr8210_overlay - overlay callback, called
    during frame processing in update to overlay
    player data
-------------------------------------------------*/

static void pr8210_overlay(laserdisc_state *ld, bitmap_t *bitmap)
{
	ldplayer_data *player = ld->player;

	/* custom display */
	if (player->pia.display)
	{
		overlay_draw_group(bitmap, &player->pia.text[2], 5, OVERLAY_GROUP1_X);
		overlay_draw_group(bitmap, &player->pia.text[7], 5, OVERLAY_GROUP2_X);
		overlay_draw_group(bitmap, &player->pia.text[12], 5, OVERLAY_GROUP3_X);
	}

	/* chapter/frame display */
	else
	{
		/* frame display */
		if (player->pia.latchdisplay & 2)
			overlay_draw_group(bitmap, &player->pia.text[2], 5, OVERLAY_GROUP1_X);

		/* chapter overlay */
		if (player->pia.latchdisplay & 1)
			overlay_draw_group(bitmap, &player->pia.text[0], 2, OVERLAY_GROUP0_X);
	}
	player->pia.latchdisplay = 0;
}


/*-------------------------------------------------
    pr8210_control_w - write callback when the
    CONTROL line is toggled
-------------------------------------------------*/

static void pr8210_control_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	ldplayer_data *player = ld->player;

	/* handle rising edge */
	if (prev != ASSERT_LINE && data == ASSERT_LINE)
	{
		attotime curtime = timer_get_time(ld->device->machine);
		attotime delta, overalldelta;
		int longpulse;

		/* get the time difference from the last assert */
		/* and update our internal command time */
		delta = attotime_sub(curtime, player->lastbittime);
		player->lastbittime = curtime;

		/* if we timed out since the first bit, reset the accumulator */
		overalldelta = attotime_sub(curtime, player->firstbittime);
		if (attotime_compare(overalldelta, SERIAL_MAX_WORD_TIME) > 0 || attotime_compare(delta, SERIAL_MAX_BIT_TIME) > 0)
		{
			player->firstbittime = curtime;
			player->accumulator = 0x5555;
			if (LOG_SERIAL)
				printf("Reset accumulator\n");
		}

		/* 0 bit delta is 1.05 msec, 1 bit delta is 2.11 msec */
		longpulse = (attotime_compare(delta, SERIAL_MIDPOINT_TIME) < 0) ? 0 : 1;
		player->accumulator = (player->accumulator << 1) | longpulse;

		/* log the deltas for debugging */
		if (LOG_SERIAL)
		{
			int usecdiff = (int)(delta.attoseconds / ATTOSECONDS_IN_USEC(1));
			printf("bitdelta = %5d (%d) - accum = %04X\n", usecdiff, longpulse, player->accumulator);
		}

		/* if we have a complete command, signal it */
		/* a complete command is 0,0,1 followed by 5 bits, followed by 0,0 */
		if ((player->accumulator & 0x383) == 0x80)
		{
			UINT8 newcommand = (player->accumulator >> 2) & 0x1f;
			attotime rejectuntil;

			/* data is stored to the PIA in bit-reverse order */
			player->pia.porta = BITSWAP8(newcommand, 0,1,2,3,4,5,6,7);

			/* the MCU logic requires a 0 to execute many commands; however, nobody
               consistently sends a 0, whereas they do tend to send duplicate commands...
               if we assume that each duplicate causes a 0, we get the correct results */
			rejectuntil = attotime_add(player->lastcommandtime, SERIAL_REJECT_DUPLICATE_TIME);
			player->lastcommandtime = curtime;
			if (player->pia.porta == player->lastcommand && attotime_compare(curtime, rejectuntil) < 0)
				player->pia.porta = 0x00;
			else
				player->lastcommand = player->pia.porta;

			/* log the command and wait for a keypress */
			if (LOG_SERIAL)
				printf("--- Command = %02X\n", player->pia.porta >> 3);

			/* reset the first bit time so that the accumulator clears on the next write */
			player->firstbittime = attotime_sub(curtime, SERIAL_MAX_WORD_TIME);
		}
	}
}


/*-------------------------------------------------
    vsync_off - timer callback to clear the VSYNC
    flag
-------------------------------------------------*/

static TIMER_CALLBACK( vsync_off )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ld->player->vsync = FALSE;
}


/*-------------------------------------------------
    vbi_data_fetch - timer callback to update the
    VBI data in the PIA as soon as it is ready;
    this must happy early in the frame because
    the player logic relies on fetching it here
-------------------------------------------------*/

static TIMER_CALLBACK( vbi_data_fetch )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;
	UINT8 focus_on = !(player->port1 & 0x08);
	UINT8 laser_on = !(player->port2 & 0x01);
	UINT32 line16 = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE16, FALSE);
	UINT32 line1718 = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE1718, FALSE);

	/* logging */
	if (LOG_VBLANK_VBI)
	{
		if ((line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
			printf("%3d:VBI(%05d)\n", ld->screen->vpos(), VBI_CAV_PICTURE(line1718));
		else
			printf("%3d:VBI()\n", ld->screen->vpos());
	}

	/* update PIA registers based on vbi code */
	player->pia.vbi1 = 0xff;
	player->pia.vbi2 = 0xff;
	if (focus_on && laser_on)
	{
		if (line1718 == VBI_CODE_LEADIN)
			player->pia.vbi1 &= ~0x01;
		if (line1718 == VBI_CODE_LEADOUT)
			player->pia.vbi1 &= ~0x02;
		if (line16 == VBI_CODE_STOP)
			player->pia.vbi1 &= ~0x04;
		/* unsure what this bit means: player->pia.vbi1 &= ~0x08; */
		if ((line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
		{
			player->pia.vbi1 &= ~0x10;
			player->pia.frame[2] = 0xf0 | ((line1718 >> 16) & 0x07);
			player->pia.frame[3] = 0xf0 | ((line1718 >> 12) & 0x0f);
			player->pia.frame[4] = 0xf0 | ((line1718 >>  8) & 0x0f);
			player->pia.frame[5] = 0xf0 | ((line1718 >>  4) & 0x0f);
			player->pia.frame[6] = 0xf0 | ((line1718 >>  0) & 0x0f);
		}
		if ((line1718 & VBI_MASK_CHAPTER) == VBI_CODE_CHAPTER)
		{
			player->pia.vbi2 &= ~0x01;
			player->pia.frame[0] = 0xf0 | ((line1718 >> 16) & 0x07);
			player->pia.frame[1] = 0xf0 | ((line1718 >> 12) & 0x0f);
		}
	}
}


/*-------------------------------------------------
    pr8210_pia_r - handle reads from the mystery
    Pioneer PIA
-------------------------------------------------*/

static READ8_HANDLER( pr8210_pia_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;
	UINT8 result = 0xff;

	switch (offset)
	{
		/* (20-26) 7 characters for the chapter/frame */
		case 0x20:	case 0x21:
		case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
			result = player->pia.frame[offset - 0x20];
			break;

		/* (1D-1F,27) invalid read but normal */
		case 0x1d:	case 0x1e:	case 0x1f:
		case 0x27:
			break;

		/* (A0) port A value (from serial decoder) */
		case 0xa0:
			result = player->pia.porta;
			break;

		/* (C0) VBI decoding state 1 */
		case 0xc0:
			if (LOG_VBLANK_VBI)
				printf("%3d:PIA(C0)\n", ld->screen->vpos());
			result = player->pia.vbi1;
			break;

		/* (E0) VBI decoding state 2 */
		case 0xe0:
			if (LOG_VBLANK_VBI)
				printf("%3d:PIA(E0)\n", ld->screen->vpos());
			result = player->pia.vbi2;
			break;

		default:
			mame_printf_debug("%03X:Unknown PR-8210 PIA read from offset %02X\n", cpu_get_pc(space->cpu), offset);
			break;
	}
	return result;
}


/*-------------------------------------------------
    pr8210_pia_w - handle writes to the mystery
    Pioneer PIA
-------------------------------------------------*/

static WRITE8_HANDLER( pr8210_pia_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;
	UINT8 value;

	switch (offset)
	{
		/* (20-30) 17 characters for the display */
		case 0x20:	case 0x21:
		case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
		case 0x27:	case 0x28:	case 0x29:	case 0x2a:	case 0x2b:
		case 0x2c:	case 0x2d:	case 0x2e:	case 0x2f:	case 0x30:
			player->pia.text[offset - 0x20] = data;
			break;

		/* (40) control lines */
		case 0x40:

			/* toggle bit 0 to latch chapter number into display area */
			if (!(data & 0x01) && (player->pia.control & 0x01))
			{
				memcpy(&player->pia.text[0], &player->pia.frame[0], 2);
				player->pia.latchdisplay |= 1;
			}

			/* toggle bit 1 to latch frame number into display area */
			if (!(data & 0x02) && (player->pia.control & 0x02))
			{
				memcpy(&player->pia.text[2], &player->pia.frame[2], 5);
				player->pia.latchdisplay |= 2;
			}
			player->pia.control = data;
			break;

		/* (60) port B value (LEDs) */
		case 0x60:

			/* these 4 are direct-connect */
			output_set_value("pr8210_audio1", (data & 0x01) != 0);
			output_set_value("pr8210_audio2", (data & 0x02) != 0);
			output_set_value("pr8210_clv", (data & 0x04) != 0);
			output_set_value("pr8210_cav", (data & 0x08) != 0);

			/* remaining 3 bits select one of 5 LEDs via a mux */
			value = ((data & 0x40) >> 6) | ((data & 0x20) >> 4) | ((data & 0x10) >> 2);
			output_set_value("pr8210_srev", (value == 0));
			output_set_value("pr8210_sfwd", (value == 1));
			output_set_value("pr8210_play", (value == 2));
			output_set_value("pr8210_step", (value == 3));
			output_set_value("pr8210_pause", (value == 4));

			player->pia.portb = data;
			update_audio_squelch(ld);
			break;

		/* (80) display enable */
		case 0x80:
			player->pia.display = data & 0x01;
			break;

		/* no other writes known */
		default:
			mame_printf_debug("%03X:Unknown PR-8210 PIA write to offset %02X = %02X\n", cpu_get_pc(space->cpu), offset, data);
			break;
	}
}


/*-------------------------------------------------
    pr8210_bus_r - handle reads from the 8049 BUS
    input, which is enabled via the PIA above
-------------------------------------------------*/

static READ8_HANDLER( pr8210_bus_r )
{
	/*
       $80 = n/c
       $40 = (in) slider pot interrupt source (slider position limit detector, inside and outside)
       $20 = n/c
       $10 = (in) /FOCUS LOCK
       $08 = (in) /SPDL LOCK
       $04 = (in) SIZE 8/12
       $02 = (in) FG via op-amp (spindle motor stop detector)
       $01 = (in) SLOW TIMER OUT
    */
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;
	slider_position sliderpos = ldcore_get_slider_position(ld);
	UINT8 focus_on = !(player->port1 & 0x08);
	UINT8 spdl_on = !(player->port1 & 0x10);
	UINT8 result = 0x00;

	/* bus bit 6: slider position limit detector, inside and outside */
	if (sliderpos != SLIDER_MINIMUM && sliderpos != SLIDER_MAXIMUM)
		result |= 0x40;

	/* bus bit 4: /FOCUS LOCK */
	if (!focus_on)
		result |= 0x10;

	/* bus bit 3: /SPDL LOCK */
	if (!spdl_on)
		result |= 0x08;

	/* bus bit 1: spindle motor stop detector */
	if (!spdl_on)
		result |= 0x02;

	/* bus bit 0: SLOW TIMER OUT */
//  if (attotime_compare(attotime_sub(timer_get_time(ld->device->machine), player->slowtrg),

	/* loop at beginning waits for $40=0, $02=1 */
	return result;
}


/*-------------------------------------------------
    pr8210_port1_w - handle writes to the 8049
    port #1
-------------------------------------------------*/

static WRITE8_HANDLER( pr8210_port1_w )
{
	/*
       $80 = (out) SCAN C (F/R)
       $40 = (out) AUDIO SQ
       $20 = (out) VIDEO SQ
       $10 = (out) /SPDL ON
       $08 = (out) /FOCUS ON
       $04 = (out) SCAN B (L/H)
       $02 = (out) SCAN A (/SCAN)
       $01 = (out) JUMP TRG (jump back trigger, clock on high->low)
    */
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;
	UINT8 prev = player->port1;
	int direction;

	/* set the new value */
	player->port1 = data;

	/* bit 7 selects the direction of slider movement for JUMP TRG and scanning */
	direction = (data & 0x80) ? 1 : -1;

	/* on the falling edge of bit 0, jump one track in either direction */
	if (!(data & 0x01) && (prev & 0x01))
	{
		/* special override for the Simutrek, which takes over control of this is some situations */
		if (player->simutrek.cpu == NULL || !player->simutrek.controlthis)
		{
			if (LOG_SIMUTREK)
				printf("%3d:JUMP TRG\n", ld->screen->vpos());
			ldcore_advance_slider(ld, direction);
		}
		else if (LOG_SIMUTREK)
			printf("%3d:Skipped JUMP TRG\n", ld->screen->vpos());
	}

	/* bit 1 low enables scanning */
	if (!(data & 0x02))
	{
		/* bit 2 selects the speed */
		int delta = (data & 0x04) ? SCAN_SPEED : SEEK_FAST_SPEED;
		ldcore_set_slider_speed(ld, delta * direction);
	}

	/* bit 1 high stops scanning */
	else
		ldcore_set_slider_speed(ld, 0);

	/* video squelch is controlled by bit 5 */
	update_video_squelch(ld);

	/* audio squelch is controlled by bit 6 */
	update_audio_squelch(ld);
}


/*-------------------------------------------------
    pr8210_port2_w - handle writes to the 8049
    port #2
-------------------------------------------------*/

static WRITE8_HANDLER( pr8210_port2_w )
{
	/*
       $80 = (out) /CS on PIA
       $40 = (out) 0 to self-generate IRQ
       $20 = (out) SLOW TRG
       $10 = (out) STANDBY LED
       $08 = (out) TP2
       $04 = (out) TP1
       $02 = (out) ???
       $01 = (out) LASER ON
    */
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;
	UINT8 prev = player->port2;

	/* set the new value */
	player->port2 = data;

	/* on the falling edge of bit 5, start the slow timer */
	if (!(data & 0x20) && (prev & 0x20))
		player->slowtrg = timer_get_time(space->machine);

	/* bit 6 when low triggers an IRQ on the MCU */
	if (player->cpu != NULL)
		cpu_set_input_line(player->cpu, MCS48_INPUT_IRQ, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

	/* standby LED is set accordingl to bit 4 */
	output_set_value("pr8210_standby", (data & 0x10) != 0);
}


/*-------------------------------------------------
    pr8210_t0_r - return the state of the 8049
    T0 input (connected to VSYNC)
-------------------------------------------------*/

static READ8_HANDLER( pr8210_t0_r )
{
	/* returns VSYNC state */
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	return !ld->player->vsync;
}


/*-------------------------------------------------
    pr8210_t1_r - return the state of the 8049
    T1 input (pulled high)
-------------------------------------------------*/

static READ8_HANDLER( pr8210_t1_r )
{
	return 1;
}


/*-------------------------------------------------
    overlay_draw_group - draw a single group of
    characters
-------------------------------------------------*/

static void overlay_draw_group(bitmap_t *bitmap, const UINT8 *text, int count, float xstart)
{
	int skip = TRUE;
	int x;

	/* rease the background */
	overlay_erase(bitmap, xstart, xstart + ((OVERLAY_X_PIXELS + 1) * count + 1) * OVERLAY_PIXEL_WIDTH);

	/* draw each character, suppressing leading 0's */
	for (x = 0; x < count; x++)
		if (!skip || x == count - 1 || (text[x] & 0x3f) != 0x30)
		{
			skip = FALSE;
			overlay_draw_char(bitmap, text[x], xstart + ((OVERLAY_X_PIXELS + 1) * x + 1) * OVERLAY_PIXEL_WIDTH);
		}
}


/*-------------------------------------------------
    overlay_erase - erase the background area
    where the text overlay will be displayed
-------------------------------------------------*/

static void overlay_erase(bitmap_t *bitmap, float xstart, float xend)
{
	UINT32 xmin = (UINT32)(xstart * 256.0f * (float)bitmap->width);
	UINT32 xmax = (UINT32)(xend * 256.0f * (float)bitmap->width);
	UINT32 x, y;

	for (y = OVERLAY_Y; y < (OVERLAY_Y + (OVERLAY_Y_PIXELS + 2) * OVERLAY_PIXEL_HEIGHT); y++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, xmin >> 8);
		UINT16 ymin, ymax, yres;

		ymax = *dest >> 8;
		ymin = ymax * 3 / 8;
		yres = ymin + ((ymax - ymin) * (xmin & 0xff)) / 256;
		*dest = (yres << 8) | (*dest & 0xff);
		dest++;

		for (x = (xmin | 0xff) + 1; x < xmax; x += 0x100)
		{
			yres = (*dest >> 8) * 3 / 8;
			*dest = (yres << 8) | (*dest & 0xff);
			dest++;
		}

		ymax = *dest >> 8;
		ymin = ymax * 3 / 8;
		yres = ymin + ((ymax - ymin) * (~xmax & 0xff)) / 256;
		*dest = (yres << 8) | (*dest & 0xff);
		dest++;
	}
}


/*-------------------------------------------------
    overlay_draw_char - draw a single character
    of the text overlay
-------------------------------------------------*/

static void overlay_draw_char(bitmap_t *bitmap, UINT8 ch, float xstart)
{
	UINT32 xminbase = (UINT32)(xstart * 256.0f * (float)bitmap->width);
	UINT32 xsize = (UINT32)(OVERLAY_PIXEL_WIDTH * 256.0f * (float)bitmap->width);
	const UINT8 *chdataptr = &text_bitmap[ch & 0x3f][0];
	UINT32 x, y, xx, yy;

	/* iterate over pixels */
	for (y = 0; y < OVERLAY_Y_PIXELS; y++)
	{
		UINT8 chdata = *chdataptr++;

		for (x = 0; x < OVERLAY_X_PIXELS; x++, chdata <<= 1)
			if (chdata & 0x80)
			{
				UINT32 xmin = xminbase + x * xsize;
				UINT32 xmax = xmin + xsize;
				for (yy = 0; yy < OVERLAY_PIXEL_HEIGHT; yy++)
				{
					UINT16 *dest = BITMAP_ADDR16(bitmap, OVERLAY_Y + (y + 1) * OVERLAY_PIXEL_HEIGHT + yy, xmin >> 8);
					UINT16 ymin, ymax, yres;

					ymax = 0xff;
					ymin = *dest >> 8;
					yres = ymin + ((ymax - ymin) * (~xmin & 0xff)) / 256;
					*dest = (yres << 8) | (*dest & 0xff);
					dest++;

					for (xx = (xmin | 0xff) + 1; xx < xmax; xx += 0x100)
						*dest++ = 0xf080;

					ymax = 0xff;
					ymin = *dest >> 8;
					yres = ymin + ((ymax - ymin) * (xmax & 0xff)) / 256;
					*dest = (yres << 8) | (*dest & 0xff);
					dest++;
				}
			}
	}
}



/***************************************************************************
    SIMUTREK ROM AND MACHINE INTERFACES
***************************************************************************/

static ADDRESS_MAP_START( simutrek_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(simutrek_data_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(simutrek_port2_r, simutrek_port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(simutrek_t0_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( simutrek )
	MDRV_CPU_ADD("simutrek", I8748, XTAL_6MHz)
	MDRV_CPU_IO_MAP(simutrek_portmap)

	MDRV_IMPORT_FROM(pr8210)
MACHINE_DRIVER_END


ROM_START( simutrek )
	ROM_REGION( 0x800, "pr8210", ROMREGION_LOADBYNAME )
	ROM_LOAD( "pr-8210_mcu_ud6005a.bin", 0x000, 0x800, CRC(120fa83b) SHA1(b514326ca1f52d6d89056868f9d17eabd4e3f31d) )

	ROM_REGION( 0x400, "simutrek", ROMREGION_LOADBYNAME)
	ROM_LOAD( "laser_player_interface_d8748_a308.bin", 0x0000, 0x0400, CRC(eed3e728) SHA1(1eb3467f1c41553375b2c21952cd593b167f5416) )
ROM_END



/***************************************************************************
    SIMUTREK PLAYER INTERFACE
***************************************************************************/

const ldplayer_interface simutrek_interface =
{
	LASERDISC_TYPE_SIMUTREK_SPECIAL,			/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Simutrek Modified PR-8210",				/* name of the player */
	ROM_NAME(simutrek),							/* pointer to ROM region information */
	MACHINE_DRIVER_NAME(simutrek),				/* pointer to machine configuration */
	simutrek_init,								/* initialization callback */
	simutrek_vsync,								/* vsync callback */
	simutrek_update,							/* update callback */
	pr8210_overlay,								/* overlay callback */
	simutrek_data_w,							/* parallel data write */
	{											/* single line write: */
		NULL,									/*    LASERDISC_LINE_ENTER */
		NULL									/*    LASERDISC_LINE_CONTROL */
	},
	NULL,										/* parallel data read */
	{											/* single line read: */
		simutrek_ready_r,						/*    LASERDISC_LINE_READY */
		simutrek_status_r,						/*    LASERDISC_LINE_STATUS */
		NULL,									/*    LASERDISC_LINE_COMMAND */
		NULL,									/*    LASERDISC_LINE_DATA_AVAIL */
	}
};



/***************************************************************************
    SIMUTREK IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    simutrek_set_audio_squelch - Simutrek-specific
    command to enable/disable audio squelch
-------------------------------------------------*/

void simutrek_set_audio_squelch(running_device *device, int state)
{
	laserdisc_state *ld = ldcore_get_safe_token(device);
	ldplayer_data *player = ld->player;
	if (LOG_SIMUTREK && player->simutrek.audio_squelch != (state == 0))
		printf("--> audio squelch = %d\n", state == 0);
	player->simutrek.audio_squelch = (state == 0);
	update_audio_squelch(ld);
}


/*-------------------------------------------------
    simutrek_init - Simutrek-specific
    initialization
-------------------------------------------------*/

static void simutrek_init(laserdisc_state *ld)
{
	astring tempstring;
	ldplayer_data *player = ld->player;

	/* standard PR-8210 initialization */
	pr8210_init(ld);

	/* initialize the Simutrek state */
	/* for proper synchronization of initial attract mode, this needs to be set */
	player->simutrek.data_ready = 1;

	/* find the Simutrek CPU */
	player->simutrek.cpu = ld->device->subdevice("simutrek");
}


/*-------------------------------------------------
    simutrek_vsync - VSYNC callback, called at the
    start of the blanking period
-------------------------------------------------*/

static TIMER_CALLBACK( irq_off )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;
	cpu_set_input_line(player->simutrek.cpu, MCS48_INPUT_IRQ, CLEAR_LINE);
	if (LOG_SIMUTREK)
		printf("%3d:**** Simutrek IRQ clear\n", ld->screen->vpos());
}

static void simutrek_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_data *player = ld->player;

	if (fieldnum == 1)
	{
		player->simutrek.controlthis = player->simutrek.controlnext;
		player->simutrek.controlnext = 0;
	}

	if (LOG_SIMUTREK)
		printf("%3d:VSYNC(%d)\n", ld->screen->vpos(), fieldnum);
	pr8210_vsync(ld, vbi, fieldnum, curtime);

	if (player->simutrek.data_ready)
	{
		if (LOG_SIMUTREK)
			printf("%3d:VSYNC IRQ\n", ld->screen->vpos());
		cpu_set_input_line(player->simutrek.cpu, MCS48_INPUT_IRQ, ASSERT_LINE);
		timer_set(ld->device->machine, ld->screen->scan_period(), ld, 0, irq_off);
	}
}


/*-------------------------------------------------
    simutrek_update - update callback, called on
    the first visible line of the frame
-------------------------------------------------*/

static INT32 simutrek_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	return pr8210_update(ld, vbi, fieldnum, curtime);
}


/*-------------------------------------------------
    simutrek_ready_r - read callback when the
    READY line is read
-------------------------------------------------*/

static UINT8 simutrek_ready_r(laserdisc_state *ld)
{
	return !ld->player->simutrek.data_ready;
}


/*-------------------------------------------------
    simutrek_status_r - read callback when the
    STATUS line is read
-------------------------------------------------*/

static UINT8 simutrek_status_r(laserdisc_state *ld)
{
	return ((ld->player->simutrek.port2 & 0x03) == 0x03) ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    simutrek_data_w - write callback when the
    parallel data port is written to
-------------------------------------------------*/

static void simutrek_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	timer_call_after_resynch(ld->device->machine, ld, data, simutrek_latched_data_w);
	if (LOG_SIMUTREK)
		printf("%03d:**** Simutrek Command = %02X\n", ld->screen->vpos(), data);
}


/*-------------------------------------------------
    simutrek_latched_data_w - deferred write
    callback for when data is written
-------------------------------------------------*/

static TIMER_CALLBACK( simutrek_latched_data_w )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;

	/* store the data and set the ready flag */
	player->simutrek.data = param;
	player->simutrek.data_ready = TRUE;
}


/*-------------------------------------------------
    simutrek_port2_r - handle reads from the 8748
    port #2
-------------------------------------------------*/

static READ8_HANDLER( simutrek_port2_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;

	/* bit $80 is the pr8210 video squelch */
	return (player->port1 & 0x20) ? 0x00 : 0x80;
}


/*-------------------------------------------------
    simutrek_port2_w - handle writes to the 8748
    port #2
-------------------------------------------------*/

static WRITE8_HANDLER( simutrek_port2_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;
	UINT8 prev = player->simutrek.port2;

	/* update stat */
	player->simutrek.port2 = data;

	/* bit $20 goes to the serial line */
	if ((data ^ prev) & 0x20)
		pr8210_control_w(ld, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	/* bit $10 goes to JUMP TRG */
	/* bit $08 controls direction */
	if (!(data & 0x10) && (prev & 0x10))
	{
		int direction = (data & 0x08) ? 1 : -1;
		if (LOG_SIMUTREK)
			printf("%3d:JUMP TRG (Simutrek PC=%03X)\n", ld->screen->vpos(), cpu_get_pc(space->cpu));
		ldcore_advance_slider(ld, direction);
	}

	/* bit $04 controls who owns the JUMP TRG command */
	if (LOG_SIMUTREK && ((data ^ prev) & 0x04))
		printf("%3d:Simutrek ownership line = %d (Simutrek PC=%03X)\n", ld->screen->vpos(), (data >> 2) & 1, cpu_get_pc(space->cpu));
	player->simutrek.controlnext = (~data >> 2) & 1;

	/* bits $03 control something (status?) */
	if (LOG_SIMUTREK && ((data ^ prev) & 0x03))
		printf("Simutrek Status = %d\n", data & 0x03);
}


/*-------------------------------------------------
    simutrek_data_r - handle external 8748 data
    reads
-------------------------------------------------*/

static READ8_HANDLER( simutrek_data_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	ldplayer_data *player = ld->player;

	/* acknowledge the read and clear the data ready flag */
	player->simutrek.data_ready = FALSE;
	return player->simutrek.data;
}


/*-------------------------------------------------
    simutrek_t0_r - return the status of the
    8748 T0 input
-------------------------------------------------*/

static READ8_HANDLER( simutrek_t0_r )
{
	/* return 1 if data is waiting from main CPU */
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner());
	return ld->player->simutrek.data_ready;
}
