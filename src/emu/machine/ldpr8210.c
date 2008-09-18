/*************************************************************************

    ldpr8210.c

    Pioneer PR-8210 laserdisc emulation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************

    Still to do:

        * add overlay properly (need capture)
        * implement SLOW TRG
        * figure out Simutrek without jump hack
        * figure out serial protocol issues (current hack works nicely)
        * determine actual slow/fast speeds

*************************************************************************/

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

/* Pioneer PR-8210 specific information */
#define PR8210_SCAN_SPEED				(2000 / 30)			/* 2000 frames/second */
#define PR8210_SEEK_FAST_SPEED			(4000 / 30)			/* 4000 frames/second */

/* serial timing, mostly from the service manual, derived from the XTAL */
#define PR8210_SERIAL_CLOCK				XTAL_455kHz
#define PR8210_0_BIT_TIME				ATTOTIME_IN_HZ(PR8210_SERIAL_CLOCK / 512)
#define PR8210_1_BIT_TIME				ATTOTIME_IN_HZ(PR8210_SERIAL_CLOCK / 1024)
#define PR8210_MIDPOINT_TIME			ATTOTIME_IN_HZ(PR8210_SERIAL_CLOCK / 600)
#define PR8210_MAX_WORD_TIME			ATTOTIME_IN_HZ(PR8210_SERIAL_CLOCK / 11520)
#define PR8210_REJECT_DUPLICATE_TIME	ATTOTIME_IN_HZ(PR8210_SERIAL_CLOCK / 11520 / 4)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* PIA data */
typedef struct _pioneer_pia pioneer_pia;
struct _pioneer_pia
{
	UINT8				frame[7];				/* (20-26) 7 characters for the chapter/frame */
	UINT8				text[15];				/* (22-30) 15 characters for the display */
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
	int					cpunum;					/* CPU index of the 8748 */
	UINT8				audio_squelch;			/* audio squelch value */
	UINT8				data;					/* parallel data for simutrek */
	UINT8				data_ready;				/* ready flag for simutrek data */
	UINT8				port2;					/* 8748 port 2 state */
	UINT8				jumphack;
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
	int					cpunum;					/* CPU index of the 8049 */
	attotime			slowtrg;				/* time of the last SLOW TRG */
	pioneer_pia			pia;					/* PIA state */
	UINT8				vsync;					/* live VSYNC state */
	UINT8 				port1;					/* 8049 port 1 state */
	UINT8 				port2;					/* 8049 port 2 state */

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
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    find_pr8210 - find our device; assumes there
    is only one
-------------------------------------------------*/

INLINE laserdisc_state *find_pr8210(running_machine *machine)
{
	return ldcore_get_safe_token(device_list_first(machine->config->devicelist, LASERDISC));
}


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
	if (player->simutrek.cpunum == -1)
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
	MDRV_CPU_IO_MAP(pr8210_portmap,0)
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
	rom_pr8210,									/* pointer to ROM region information */
	machine_config_pr8210,						/* pointer to machine configuration */
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
    pr8210_init - Pioneer PR-8210-specific
    initialization
-------------------------------------------------*/

static void pr8210_init(laserdisc_state *ld)
{
	astring *tempstring = astring_alloc();
	attotime curtime = timer_get_time();
	ldplayer_data *player = ld->player;

	/* find our CPU */
	astring_printf(tempstring, "%s:%s", ld->device->tag, "pr8210");
	player->cpunum = mame_find_cpu_index(ld->device->machine, astring_c(tempstring));
	astring_free(tempstring);

	/* do a soft reset */
	player->lastcommand = 0;
	player->accumulator = 0;
	player->lastcommandtime = curtime;
	player->firstbittime = curtime;
	player->lastbittime = curtime;
	player->slowtrg = curtime;

	/* we don't have the Simutrek player overrides */
	player->simutrek.cpunum = -1;
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
			printf("%3d:VSYNC(%d,%05d)\n", video_screen_get_vpos(ld->screen), fieldnum, VBI_CAV_PICTURE(vbi->line1718));
		else
			printf("%3d:VSYNC(%d)\n", video_screen_get_vpos(ld->screen), fieldnum);
	}

	/* signal VSYNC and set a timer to turn it off */
	player->vsync = TRUE;
	timer_set(attotime_mul(video_screen_get_scan_period(ld->screen), 4), ld, 0, vsync_off);

	/* also set a timer to fetch the VBI data when it is ready */
	timer_set(video_screen_get_time_until_pos(ld->screen, 19*2, 0), ld, 0, vbi_data_fetch);
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
		printf("%3d:Update(%d)\n", video_screen_get_vpos(ld->screen), fieldnum);

	/* update overlay */
	if (player->pia.display || player->pia.latchdisplay)
	{
		char buffer[16] = { 0 };
		int i;
		for (i = 0; i < 15; i++)
		{
			UINT8 c = player->pia.text[i];
			if (c >= 0xf0 && c <= 0xf9)
				c = '0' + (c - 0xf0);
			else if (c < 0x20 || c > 0x7f)
				c = '?';
			buffer[i] = c;
		}
		popmessage("%s", buffer);
	}
	else
		popmessage(NULL);
	player->pia.latchdisplay = 0;

	/* if the spindle is on, we advance by 1 track after completing field #1 */
	return (spdl_on) ? fieldnum : 0;
}


/*-------------------------------------------------
    pr8210_overlay - overlay callback, called
    during frame processing in update to overlay
    player data
-------------------------------------------------*/

void pr8210_overlay(laserdisc_state *ld, bitmap_t *bitmap)
{
//  ldplayer_data *player = ld->player;
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
		attotime curtime = timer_get_time();
		attotime delta;
		int longpulse;

		/* if we timed out since the first bit, reset the accumulator */
		delta = attotime_sub(curtime, player->firstbittime);
		if (attotime_compare(delta, PR8210_MAX_WORD_TIME) > 0)
		{
			player->firstbittime = curtime;
			player->accumulator = 0x5555;
			if (LOG_SERIAL)
				printf("Reset accumulator\n");
		}

		/* get the time difference from the last assert */
		/* and update our internal command time */
		delta = attotime_sub(curtime, player->lastbittime);
		player->lastbittime = curtime;

		/* 0 bit delta is 1.05 msec, 1 bit delta is 2.11 msec */
		longpulse = (attotime_compare(delta, PR8210_MIDPOINT_TIME) < 0) ? 0 : 1;
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
			rejectuntil = attotime_add(player->lastcommandtime, PR8210_REJECT_DUPLICATE_TIME);
			player->lastcommandtime = curtime;
			if (player->pia.porta == player->lastcommand && attotime_compare(curtime, rejectuntil) < 0)
				player->pia.porta = 0x00;
			else
				player->lastcommand = player->pia.porta;

			/* log the command and wait for a keypress */
			if (LOG_SERIAL)
			{
				printf("--- Command = %02X\n", player->pia.porta >> 3);
				while (input_code_pressed(KEYCODE_ENTER)) ;
				while (!input_code_pressed(KEYCODE_ENTER)) ;
			}

			/* reset the first bit time so that the accumulator clears on the next write */
			player->firstbittime = attotime_sub(curtime, PR8210_MAX_WORD_TIME);
		}
	}
}


/*-------------------------------------------------
    vsync_off - timer callback to clear the VSYNC
    flag
-------------------------------------------------*/

static TIMER_CALLBACK( vsync_off )
{
	laserdisc_state *ld = ptr;
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
	laserdisc_state *ld = ptr;
	ldplayer_data *player = ld->player;
	UINT8 focus_on = !(player->port1 & 0x08);
	UINT8 laser_on = !(player->port2 & 0x01);
	UINT32 line16 = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE16);
	UINT32 line1718 = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE1718);

	/* logging */
	if (LOG_VBLANK_VBI)
	{
		if ((line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
			printf("%3d:VBI(%05d)\n", video_screen_get_vpos(ld->screen), VBI_CAV_PICTURE(line1718));
		else
			printf("%3d:VBI()\n", video_screen_get_vpos(ld->screen));
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
	laserdisc_state *ld = find_pr8210(machine);
	ldplayer_data *player = ld->player;
	UINT8 result = 0xff;

	switch (offset)
	{
		/* (20-26) 7 characters for the chapter/frame */
		case 0x20:	case 0x21:
		case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
			result = player->pia.frame[offset - 0x20];
			break;

		/* (A0) port A value (from serial decoder) */
		case 0xa0:
			result = player->pia.porta;
			break;

		/* (C0) VBI decoding state 1 */
		case 0xc0:
			if (LOG_VBLANK_VBI)
				printf("%3d:PIA(C0)\n", video_screen_get_vpos(ld->screen));
			result = player->pia.vbi1;
			break;

		/* (E0) VBI decoding state 2 */
		case 0xe0:
			if (LOG_VBLANK_VBI)
				printf("%3d:PIA(E0)\n", video_screen_get_vpos(ld->screen));
			result = player->pia.vbi2;
			break;

		default:
			mame_printf_debug("%03X:Unknown PR-8210 PIA read from offset %02X\n", activecpu_get_pc(), offset);
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
	laserdisc_state *ld = find_pr8210(machine);
	ldplayer_data *player = ld->player;
	UINT8 value;

	switch (offset)
	{
		/* (22-30) 15 characters for the display */
		case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
		case 0x27:	case 0x28:	case 0x29:	case 0x2a:	case 0x2b:
		case 0x2c:	case 0x2d:	case 0x2e:	case 0x2f:	case 0x30:
			player->pia.text[offset - 0x22] = data;
			break;

		/* (40) control lines */
		case 0x40:

			/* toggle bit 0 to latch chapter number into display area */
			if (!(data & 0x01) && (player->pia.control & 0x01))
			{
				memcpy(&player->pia.text[0], &player->pia.frame[0], 2);
				memset(&player->pia.text[3], 0, 10);
				player->pia.latchdisplay = 1;
			}

			/* toggle bit 1 to latch frame number into display area */
			if (!(data & 0x02) && (player->pia.control & 0x02))
			{
				memcpy(&player->pia.text[0], &player->pia.frame[2], 5);
				memset(&player->pia.text[5], 0, 10);
				player->pia.latchdisplay = 1;
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
			mame_printf_debug("%03X:Unknown PR-8210 PIA write to offset %02X = %02X\n", activecpu_get_pc(), offset, data);
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
	laserdisc_state *ld = find_pr8210(machine);
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
//  if (attotime_compare(attotime_sub(timer_get_time(), player->slowtrg),

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
	laserdisc_state *ld = find_pr8210(machine);
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
		if (player->simutrek.cpunum == -1 || (player->simutrek.port2 & 0x04) != 0 || player->simutrek.jumphack)
		{
			if (LOG_SIMUTREK)
				printf("%3d:JUMP TRG\n", video_screen_get_vpos(ld->screen));
			ldcore_advance_slider(ld, direction);
			player->simutrek.jumphack = 0;
		}
		else if (LOG_SIMUTREK)
			printf("%3d:Skipped JUMP TRG\n", video_screen_get_vpos(ld->screen));
	}

	/* bit 1 low enables scanning */
	if (!(data & 0x02))
	{
		/* bit 2 selects the speed */
		int delta = (data & 0x04) ? PR8210_SCAN_SPEED : PR8210_SEEK_FAST_SPEED;
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
	laserdisc_state *ld = find_pr8210(machine);
	ldplayer_data *player = ld->player;
	UINT8 prev = player->port2;

	/* set the new value */
	player->port2 = data;

	/* on the falling edge of bit 5, start the slow timer */
	if (!(data & 0x20) && (prev & 0x20))
		player->slowtrg = timer_get_time();

	/* bit 6 when low triggers an IRQ on the MCU */
	cpunum_set_input_line(machine, player->cpunum, MCS48_INPUT_IRQ, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

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
	laserdisc_state *ld = find_pr8210(machine);
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



/***************************************************************************
    SIMUTREK ROM AND MACHINE INTERFACES
***************************************************************************/

static ADDRESS_MAP_START( simutrek_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(simutrek_data_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(simutrek_port2_r, simutrek_port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(simutrek_t0_r)
ADDRESS_MAP_END


MACHINE_DRIVER_START( simutrek )
	MDRV_CPU_ADD("simutrek", I8748, XTAL_6MHz)
	MDRV_CPU_IO_MAP(simutrek_portmap,0)

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
	rom_simutrek,								/* pointer to ROM region information */
	machine_config_simutrek,					/* pointer to machine configuration */
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

void simutrek_set_audio_squelch(const device_config *device, int state)
{
	laserdisc_state *ld = ldcore_get_safe_token(device);
	ldplayer_data *player = ld->player;
	player->simutrek.audio_squelch = (state == 0);
	update_audio_squelch(ld);
}


/*-------------------------------------------------
    simutrek_init - Simutrek-specific
    initialization
-------------------------------------------------*/

static void simutrek_init(laserdisc_state *ld)
{
	astring *tempstring = astring_alloc();

	/* standard PR-8210 initialization */
	pr8210_init(ld);

	/* find the Simutrek CPU */
	astring_printf(tempstring, "%s:%s", ld->device->tag, "simutrek");
	ld->player->simutrek.cpunum = mame_find_cpu_index(ld->device->machine, astring_c(tempstring));
	astring_free(tempstring);
}


/*-------------------------------------------------
    simutrek_vsync - VSYNC callback, called at the
    start of the blanking period
-------------------------------------------------*/

static TIMER_CALLBACK( irq_off )
{
	laserdisc_state *ld = ptr;
	ldplayer_data *player = ld->player;
	cpunum_set_input_line(ld->device->machine, player->simutrek.cpunum, MCS48_INPUT_IRQ, CLEAR_LINE);
	if (LOG_SIMUTREK)
		printf("%3d:**** Simutrek IRQ clear\n", video_screen_get_vpos(ld->screen));
}

static void simutrek_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_data *player = ld->player;

	if (LOG_SIMUTREK)
		printf("%3d:VSYNC(%d)\n", video_screen_get_vpos(ld->screen), fieldnum);
	pr8210_vsync(ld, vbi, fieldnum, curtime);

	if (player->simutrek.data_ready)
	{
		if (LOG_SIMUTREK)
			printf("%3d:VSYNC IRQ\n", video_screen_get_vpos(ld->screen));
		cpunum_set_input_line(ld->device->machine, player->simutrek.cpunum, MCS48_INPUT_IRQ, ASSERT_LINE);
		timer_set(video_screen_get_scan_period(ld->screen), ld, 0, irq_off);
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
	timer_call_after_resynch(ld, data, simutrek_latched_data_w);
	if (LOG_SIMUTREK)
		printf("%03d:**** Simutrek Command = %02X\n", video_screen_get_vpos(ld->screen), data);
}


/*-------------------------------------------------
    simutrek_latched_data_w - deferred write
    callback for when data is written
-------------------------------------------------*/

static TIMER_CALLBACK( simutrek_latched_data_w )
{
	laserdisc_state *ld = ptr;
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
	laserdisc_state *ld = find_pr8210(machine);
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
	laserdisc_state *ld = find_pr8210(machine);
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
			printf("%3d:JUMP TRG (Simutrek PC=%03X)\n", video_screen_get_vpos(ld->screen), activecpu_get_pc());
		ldcore_advance_slider(ld, direction);
	}

	/* bit $04 controls who owns the JUMP TRG command */
	if (!(data & 0x04) && (prev & 0x04))
		player->simutrek.jumphack = 1;

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
	laserdisc_state *ld = find_pr8210(machine);
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
	laserdisc_state *ld = find_pr8210(machine);
	return ld->player->simutrek.data_ready;
}
