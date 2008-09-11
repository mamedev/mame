/*************************************************************************

    ldpr8210.c

    Pioneer PR-8210 laserdisc emulation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#include "ldcore.h"
#include "cpu/mcs48/mcs48.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define EMULATE_PR8210_ROM			0

#define PRINTF_COMMANDS				1
#define CMDPRINTF(x)				do { if (PRINTF_COMMANDS) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Player-specific states */
enum
{
	LDSTATE_STEPPING_BY_PARAMETER = LDSTATE_OTHER,
	LDSTATE_STEPPING_BY_PARAMETER_PAUSED
};

/* Pioneer PR-8210 specific information */
#define PR8210_SCAN_SPEED			(2000 / 30)			/* 2000 frames/second */
#define PR8210_SCAN_DURATION		(10)				/* scan for 10 vsyncs each time */
#define PR8210_SLOW_SPEED			(5)					/* 1/5x normal speed */
#define PR8210_FAST_SPEED			(3)					/* 3x normal speed */
#define PR8210_JUMP_REV_SPEED		(15)				/* 15x normal speed */
#define PR8210_SEEK_FAST_SPEED		(1000)				/* 1000x normal speed */

/* Us vs. Them requires at least this much "non-video" time in order to work */
#define PR8210_MINIMUM_SEEK_TIME	ATTOTIME_IN_MSEC(150)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* player-specific data */
struct _ldplayer_data
{
	/* general modes and states */
	UINT8				audio1disable;			/* explicit disable for audio 1 */
	UINT8				audio2disable;			/* explicit disable for audio 2 */
	UINT8				framedisplay;			/* frame display */
	INT32				parameter;				/* parameter for numbers */

	/* serial/command interface processing */
	UINT8				lastcommand;			/* last command byte received */
	UINT8				seekstate;				/* state of the seek command */
	UINT16				accumulator;			/* bit accumulator */
	attotime			lastbittime;			/* time of last bit received */
	attotime			firstbittime;			/* time of first bit in command */

	/* Simutrek-specific data */
	UINT8				cmdcnt;					/* counter for multi-byte command */
	UINT8				cmdbytes[3];			/* storage for multi-byte command */
	void				(*cmd_ack_callback)(void); /* callback to clear game command write flag */
	
	/* low-level emulation data */
	int					cpunum;
	UINT8 				pia[0x100];
	UINT8				vsync;
	UINT8 				porta;
	UINT8 				portb;
	UINT8 				pia_porta;
	UINT8 				pia_portb;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void pr8210_init(laserdisc_state *ld);
static void pr8210_soft_reset(laserdisc_state *ld);
static void pr8210_update_squelch(laserdisc_state *ld);
static int pr8210_switch_state(laserdisc_state *ld, UINT8 newstate, INT32 stateparam);
static INT32 pr8210_hle_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
#if (EMULATE_PR8210_ROM)
static void pr8210_vsync(laserdisc_state *ld);
static INT32 pr8210_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
#else
static void pr8210_hle_command(laserdisc_state *ld);
#endif
static void pr8210_control_w(laserdisc_state *ld, UINT8 prev, UINT8 data);

static void simutrek_init(laserdisc_state *ld);
static UINT8 simutrek_status_r(laserdisc_state *ld);
static void simutrek_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data);



/***************************************************************************
    ROM AND MACHINE INTERFACES
***************************************************************************/

MACHINE_DRIVER_EXTERN( pr8210 );

#if EMULATE_PR8210_ROM
ROM_START( pr8210 )
	ROM_REGION( 0x800, "pr8210", ROMREGION_LOADBYNAME )
	ROM_LOAD( "pr-8210_mcu_ud6005a.bin", 0x000, 0x800, CRC(120fa83b) SHA1(b514326ca1f52d6d89056868f9d17eabd4e3f31d) )
ROM_END
#endif



/***************************************************************************
    INTERFACES
***************************************************************************/

const ldplayer_interface pr8210_interface =
{
	LASERDISC_TYPE_PIONEER_PR8210,				/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Pioneer PR-8210",							/* name of the player */
#if EMULATE_PR8210_ROM
	rom_pr8210,									/* pointer to ROM region information */
	machine_config_pr8210,						/* pointer to machine configuration */
	pr8210_init,								/* initialization callback */
	pr8210_vsync,								/* vsync callback */
	pr8210_update,								/* update callback */
#else
	NULL,										/* pointer to ROM region information */
	NULL,										/* pointer to machine configuration */
	pr8210_init,								/* initialization callback */
	NULL,										/* vsync callback */
	pr8210_hle_update,							/* update callback */
#endif
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


const ldplayer_interface simutrek_interface =
{
	LASERDISC_TYPE_SIMUTREK_SPECIAL,			/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Simutrek Modified PR-8210",				/* name of the player */
	NULL,										/* pointer to ROM region information */
	NULL,										/* pointer to machine configuration */
	simutrek_init,								/* initialization callback */
	NULL,										/* vsync callback */
	pr8210_hle_update,							/* update callback */
	simutrek_data_w,							/* parallel data write */
	{											/* single line write: */
		NULL,									/*    LASERDISC_LINE_ENTER */
		NULL									/*    LASERDISC_LINE_CONTROL */
	},
	NULL,										/* parallel data read */
	{											/* single line read: */
		NULL,									/*    LASERDISC_LINE_READY */
		simutrek_status_r,						/*    LASERDISC_LINE_STATUS */
		NULL,									/*    LASERDISC_LINE_COMMAND */
		NULL,									/*    LASERDISC_LINE_DATA_AVAIL */
	}
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    requires_state_save - returns TRUE if the
    given state will return to the previous
    state when done
-------------------------------------------------*/

INLINE int requires_state_save(UINT8 state)
{
	return (state == LDSTATE_SCANNING);
}


/*-------------------------------------------------
    requires_state_save - returns TRUE if the
    given state will return to the previous
    state when done
-------------------------------------------------*/

INLINE void update_audio_squelch(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	ldcore_set_audio_squelch(ld, (player->porta & 0x40) || !(player->pia_portb & 0x01), (player->porta & 0x40) || !(player->pia_portb & 0x02));
}




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
	
	/* find our CPU */
	astring_printf(tempstring, "%s:%s", ld->device->tag, "pr8210");
	ld->player->cpunum = mame_find_cpu_index(ld->device->machine, astring_c(tempstring));
	astring_free(tempstring);
	
	/* do a soft reset */
	pr8210_soft_reset(ld);
}


/*-------------------------------------------------
    pr8210_soft_reset - Pioneer PR-8210-specific
    soft reset
-------------------------------------------------*/

static void pr8210_soft_reset(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	attotime curtime = timer_get_time();

	ld->state.state = LDSTATE_NONE;
	ld->state.substate = 0;
	ld->state.param = 0;
	ld->state.endtime = curtime;

	player->audio1disable = 0;
	player->audio2disable = 0;
	player->parameter = 0;

	player->lastcommand = 0;
	player->seekstate = 0;
	player->accumulator = 0;
	player->firstbittime = curtime;
	player->lastbittime = curtime;

	pr8210_switch_state(ld, LDSTATE_PARKED, 0);
}


/*-------------------------------------------------
    pr8210_update_squelch - update the squelch
    settings based on the current state
-------------------------------------------------*/

static void pr8210_update_squelch(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;

	switch (ld->state.state)
	{
		/* video on, audio on */
		case LDSTATE_PLAYING:
			ldcore_set_audio_squelch(ld, player->audio1disable, player->audio2disable);
			ldcore_set_video_squelch(ld, FALSE);
			break;

		/* video on, audio off */
		case LDSTATE_PAUSING:
		case LDSTATE_PAUSED:
		case LDSTATE_PLAYING_SLOW_REVERSE:
		case LDSTATE_PLAYING_SLOW_FORWARD:
		case LDSTATE_PLAYING_FAST_REVERSE:
		case LDSTATE_PLAYING_FAST_FORWARD:
		case LDSTATE_STEPPING_REVERSE:
		case LDSTATE_STEPPING_FORWARD:
		case LDSTATE_SCANNING:
			ldcore_set_audio_squelch(ld, TRUE, TRUE);
			ldcore_set_video_squelch(ld, FALSE);
			break;

		/* video off, audio off */
		case LDSTATE_NONE:
		case LDSTATE_EJECTED:
		case LDSTATE_EJECTING:
		case LDSTATE_PARKED:
		case LDSTATE_LOADING:
		case LDSTATE_SPINUP:
		case LDSTATE_SEEKING:
			ldcore_set_audio_squelch(ld, TRUE, TRUE);
			ldcore_set_video_squelch(ld, TRUE);
			break;

		/* Simutrek: stepping by parameter with explicit audio squelch controls */
		case LDSTATE_STEPPING_BY_PARAMETER:
		case LDSTATE_STEPPING_BY_PARAMETER_PAUSED:
			ldcore_set_video_squelch(ld, FALSE);
			break;
	}
}


/*-------------------------------------------------
    pr8210_switch_state - attempt to switch states
    if appropriate, and ensure the squelch values
    are correct
-------------------------------------------------*/

static int pr8210_switch_state(laserdisc_state *ld, UINT8 newstate, INT32 parameter)
{
	attotime curtime = timer_get_time();

	/* if this is the same state, ignore */
	if (ld->state.state == newstate)
		return TRUE;

	/* if we're in a timed state, we ignore changes until the time elapses */
	if (attotime_compare(curtime, ld->state.endtime) < 0)
		return FALSE;

	/* if moving to a state that requires it, save the old state */
	if (requires_state_save(newstate) && !requires_state_save(ld->state.state))
		ld->savestate = ld->state;

	/* set up appropriate squelch and timing */
	ld->state.state = newstate;
	ld->state.substate = 0;
	ld->state.param = parameter;
	ld->state.endtime = curtime;
	pr8210_update_squelch(ld);

	/* if we're switching to a timed state, set the appropriate timer */
	switch (ld->state.state)
	{
		case LDSTATE_EJECTING:
			ld->state.endtime = attotime_add(curtime, GENERIC_EJECT_TIME);
			break;

		case LDSTATE_LOADING:
			ld->state.endtime = attotime_add(curtime, GENERIC_LOAD_TIME);
			break;

		case LDSTATE_SPINUP:
			ld->state.endtime = attotime_add(curtime, GENERIC_SPINUP_TIME);
			break;

		case LDSTATE_SEEKING:
			ld->state.endtime = attotime_add(curtime, PR8210_MINIMUM_SEEK_TIME);
			break;
	}
	return TRUE;
}


/*-------------------------------------------------
    pr8210_vsync - Start of VSYNC callback
-------------------------------------------------*/

#if (EMULATE_PR8210_ROM)
static TIMER_CALLBACK( vsync_off )
{
	ldplayer_data *player = ptr;
	player->vsync = FALSE;
}

static void pr8210_vsync(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	player->vsync = TRUE;
	timer_set(attotime_mul(video_screen_get_scan_period(ld->screen), 4), player, 0, vsync_off);
}
#endif


/*-------------------------------------------------
    pr8210_update - Pioneer PR-8210-specific
    update callback when using a ROM
-------------------------------------------------*/

#if (EMULATE_PR8210_ROM)

static INT32 pr8210_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_data *player = ld->player;
	UINT8 focus_on = !(player->porta & 0x08);
	UINT8 laser_on = !(player->portb & 0x01);
	UINT8 spdl_on = !(player->porta & 0x10);
	INT32 advanceby = 0;
	int frame, chapter;

	/* update PIA registers based on vbi code */
	frame = frame_from_metadata(vbi);
	chapter = chapter_from_metadata(vbi);
	if (focus_on && laser_on && !(player->pia[0x80] & 1))
	{
		if (frame == FRAME_LEAD_IN)
			player->pia[0xc0] = 0x10; /* or 0x12 */
		else if (frame == FRAME_LEAD_OUT)
			player->pia[0xc0] = 0x11;
		else if (frame != FRAME_NOT_PRESENT)
		{
			player->pia[0xc0] = 0x02;	/* bit 0x02 must be set for forward scanning to work */
			player->pia[0x22] = 0xf0 | ((frame / 10000) % 10);
			player->pia[0x23] = 0xf0 | ((frame / 1000) % 10);
			player->pia[0x24] = 0xf0 | ((frame / 100) % 10);
			player->pia[0x25] = 0xf0 | ((frame / 10) % 10);
			player->pia[0x26] = 0xf0 | ((frame / 1) % 10);
printf("Frame:%05d\n", frame);
		}
		else if (chapter != CHAPTER_NOT_PRESENT)
		{
			player->pia[0xc0] = 0x13;
			player->pia[0x20] = 0xf0 | ((chapter / 10) % 10);
			player->pia[0x21] = 0xf0 | ((chapter / 1) % 10);
		}
//		else
//			player->pia[0xc0] = 0x00;
	}
	player->pia[0xc0] |= 12;//4;//fieldnum << 2;

	if (spdl_on)
		advanceby = fieldnum;

	/* update overlay */
	if ((player->pia[0x80] & 1) || player->framedisplay)
	{
		char buffer[16] = { 0 };
		int i;
		for (i = 0; i < 15; i++)
		{
			UINT8 c = player->pia[0x22 + i];
			if (c >= 0xf0 && c <= 0xf9)
				c = '0' + (c - 0xf0);
			else if (c < 0x20 || c > 0x7f)
				c = '?';
			buffer[i] = c;
		}
		popmessage("%s", buffer);
	}
	player->framedisplay = 0;

if (advanceby != 0)
	printf("Advancing by %d\n", advanceby);
	return advanceby;
}
#endif


/*-------------------------------------------------
    pr8210_hle_update - Pioneer PR-8210-specific
    update callback when using HLE
-------------------------------------------------*/

static INT32 pr8210_hle_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_state newstate;
	INT32 advanceby = 0;
	int frame;

	/* handle things based on the state */
	switch (ld->state.state)
	{
		case LDSTATE_EJECTING:
		case LDSTATE_EJECTED:
		case LDSTATE_PARKED:
		case LDSTATE_LOADING:
		case LDSTATE_SPINUP:
		case LDSTATE_PAUSING:
		case LDSTATE_PAUSED:
		case LDSTATE_PLAYING:
		case LDSTATE_PLAYING_SLOW_REVERSE:
		case LDSTATE_PLAYING_SLOW_FORWARD:
		case LDSTATE_PLAYING_FAST_REVERSE:
		case LDSTATE_PLAYING_FAST_FORWARD:
		case LDSTATE_STEPPING_REVERSE:
		case LDSTATE_STEPPING_FORWARD:
		case LDSTATE_SCANNING:
			/* generic behaviors are appropriate for all of these commands */
			advanceby = ldcore_generic_update(ld, vbi, fieldnum, curtime, &newstate);
			pr8210_switch_state(ld, newstate.state, newstate.param);
			break;

		case LDSTATE_SEEKING:
			/* if we're in the final state, look for a matching frame and pause there */
			frame = frame_from_metadata(vbi);
			if (ld->state.substate == 3 && is_start_of_frame(vbi) && frame == ld->state.param)
				pr8210_switch_state(ld, LDSTATE_PAUSED, fieldnum);

			/* otherwise, if we got frame data from the VBI, update our seeking logic */
			else if (ld->state.substate < 3 && frame != FRAME_NOT_PRESENT)
			{
				static const INT32 seekspeed[] = { -PR8210_SEEK_FAST_SPEED, PR8210_SEEK_FAST_SPEED, -PR8210_JUMP_REV_SPEED };
				INT32 curseekspeed = seekspeed[ld->state.substate];
				INT32 delta = (ld->state.param - 1) - frame;

				if ((curseekspeed < 0 && delta <= 0) || (curseekspeed > 0 && delta >= 0))
					advanceby = curseekspeed;
				else
					ld->state.substate++;
			}

			/* otherwise, keep advancing until we know what's up */
			else if (fieldnum == 1)
				advanceby = 1;
			break;

		case LDSTATE_STEPPING_BY_PARAMETER:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
			{
				/* note that we switch directly to PAUSED as we are not looking for frames */
				advanceby = ld->state.param;
				pr8210_switch_state(ld, LDSTATE_STEPPING_BY_PARAMETER_PAUSED, 0);
			}
			break;
		
		case LDSTATE_STEPPING_BY_PARAMETER_PAUSED:
			/* generic pause behavior */
			ld->state.state = LDSTATE_PAUSED;
			advanceby = ldcore_generic_update(ld, vbi, fieldnum, curtime, &newstate);
			if (newstate.state != LDSTATE_PAUSED)
				pr8210_switch_state(ld, newstate.state, newstate.param);
			else
				ld->state.state = LDSTATE_STEPPING_BY_PARAMETER_PAUSED;
			break;
	}

	return advanceby;
}



/*-------------------------------------------------
    pr8210_hle_command - Pioneer PR-8210-specific
    command processing
-------------------------------------------------*/

#if (!EMULATE_PR8210_ROM)
static void pr8210_hle_command(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	UINT8 cmd = player->lastcommand;

	switch (cmd)
	{
		case 0x00:	CMDPRINTF(("pr8210: EOC\n"));
			/* EOC marker - can be safely ignored */
			break;

		case 0x01:	CMDPRINTF(("pr8210: 0\n"));
			player->parameter = (player->parameter == -1) ? 0 : (player->parameter * 10 + 0);
			break;

		case 0x02:	CMDPRINTF(("pr8210: Slow reverse\n"));
			pr8210_switch_state(ld, LDSTATE_PLAYING_SLOW_REVERSE, PR8210_SLOW_SPEED);
			break;

		case 0x03:	CMDPRINTF(("pr8210: 8\n"));
			player->parameter = (player->parameter == -1) ? 8 : (player->parameter * 10 + 8);
			break;

		case 0x04:	CMDPRINTF(("pr8210: Step forward\n"));
			pr8210_switch_state(ld, LDSTATE_STEPPING_FORWARD, 0);
			break;

		case 0x05:	CMDPRINTF(("pr8210: 4\n"));
			player->parameter = (player->parameter == -1) ? 4 : (player->parameter * 10 + 4);
			break;

		case 0x06 :	CMDPRINTF(("pr8210: Chapter\n"));
			/* chapter -- not implemented */
			break;

		case 0x08:	CMDPRINTF(("pr8210: Scan forward\n"));
			if (ld->state.state != LDSTATE_SCANNING)
				pr8210_switch_state(ld, LDSTATE_SCANNING, SCANNING_PARAM(PR8210_SCAN_SPEED, PR8210_SCAN_DURATION));
			else
				ld->state.substate = 0;
			break;

		case 0x09:	CMDPRINTF(("pr8210: 2\n"));
			player->parameter = (player->parameter == -1) ? 2 : (player->parameter * 10 + 2);
			break;

		case 0x0a:	CMDPRINTF(("pr8210: Pause\n"));
			pr8210_switch_state(ld, LDSTATE_PAUSING, 0);
			break;

		case 0x0b :	CMDPRINTF(("pr8210: Frame\n"));
			/* frame -- not implemented */
			break;

		case 0x0c:	CMDPRINTF(("pr8210: Fast reverse\n"));
			pr8210_switch_state(ld, LDSTATE_PLAYING_FAST_REVERSE, PR8210_FAST_SPEED);
			break;

		case 0x0d:	CMDPRINTF(("pr8210: 6\n"));
			player->parameter = (player->parameter == -1) ? 6 : (player->parameter * 10 + 6);
			break;

		case 0x0e:	CMDPRINTF(("pr8210: Ch1 toggle\n"));
			player->audio1disable = !player->audio1disable;
			pr8210_update_squelch(ld);
			break;

		case 0x10:	CMDPRINTF(("pr8210: Fast forward\n"));
			pr8210_switch_state(ld, LDSTATE_PLAYING_FAST_FORWARD, PR8210_FAST_SPEED);
			break;

		case 0x11:	CMDPRINTF(("pr8210: 1\n"));
			player->parameter = (player->parameter == -1) ? 1 : (player->parameter * 10 + 1);
			break;

		case 0x12:	CMDPRINTF(("pr8210: Step reverse\n"));
			pr8210_switch_state(ld, LDSTATE_STEPPING_REVERSE, 0);
			break;

		case 0x13:	CMDPRINTF(("pr8210: 9\n"));
			player->parameter = (player->parameter == -1) ? 9 : (player->parameter * 10 + 9);
			break;

		case 0x14:  CMDPRINTF(("pr8210: Play\n"));
			if (ld->state.state == LDSTATE_EJECTED)
				pr8210_switch_state(ld, LDSTATE_LOADING, 0);
			else if (ld->state.state == LDSTATE_PARKED)
				pr8210_switch_state(ld, LDSTATE_SPINUP, 0);
			else
				pr8210_switch_state(ld, LDSTATE_PLAYING, 0);
			break;

		case 0x15:	CMDPRINTF(("pr8210: 5\n"));
			player->parameter = (player->parameter == -1) ? 5 : (player->parameter * 10 + 5);
			break;

		case 0x16:	CMDPRINTF(("pr8210: Ch2 toggle\n"));
			player->audio2disable = !player->audio2disable;
			pr8210_update_squelch(ld);
			break;

		case 0x18:	CMDPRINTF(("pr8210: Slow forward\n"));
			pr8210_switch_state(ld, LDSTATE_PLAYING_SLOW_FORWARD, PR8210_SLOW_SPEED);
			break;

		case 0x19:	CMDPRINTF(("pr8210: 3\n"));
			player->parameter = (player->parameter == -1) ? 3 : (player->parameter * 10 + 3);
			break;

		case 0x1a:	CMDPRINTF(("pr8210: Seek\n"));
			if (player->seekstate)
			{
				/* we're ready to seek */
				CMDPRINTF(("pr8210: Seeking to frame %d\n", player->parameter));
				pr8210_switch_state(ld, LDSTATE_SEEKING, player->parameter);
			}
			else
			{
				/* waiting for digits indicating position */
				player->parameter = 0;
			}
			player->seekstate ^= 1;
			break;

		case 0x1c:	CMDPRINTF(("pr8210: Scan reverse\n"));
			if (ld->state.state != LDSTATE_SCANNING)
				pr8210_switch_state(ld, LDSTATE_SCANNING, SCANNING_PARAM(-PR8210_SCAN_SPEED, PR8210_SCAN_DURATION));
			else
				ld->state.substate = 0;
			break;

		case 0x1d:	CMDPRINTF(("pr8210: 7\n"));
			player->parameter = (player->parameter == -1) ? 7 : (player->parameter * 10 + 7);
			break;

		case 0x1e:	CMDPRINTF(("pr8210: Reject\n"));
			pr8210_switch_state(ld, LDSTATE_EJECTING, 0);
			break;

		default:	CMDPRINTF(("pr8210: Unknown command %02X\n", cmd));
			break;
	}
}
#endif


/*-------------------------------------------------
    pr8210_control_w - write callback when the
    CONTROL line is toggled
-------------------------------------------------*/

static void pr8210_control_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	ldplayer_data *player = ld->player;

	if (data == ASSERT_LINE)
	{
		attotime curtime = timer_get_time();
		attotime delta;
		int longpulse;

		/* if we timed out, reset the accumulator */
		delta = attotime_sub(curtime, player->firstbittime);
		if (delta.attoseconds > ATTOTIME_IN_USEC(25320).attoseconds)
		{
			player->firstbittime = curtime;
			player->accumulator = 0x5555;
//          printf("Reset accumulator\n");
		}

		/* get the time difference from the last assert */
		/* and update our internal command time */
		delta = attotime_sub(curtime, player->lastbittime);
		player->lastbittime = curtime;

		/* 0 bit delta is 1.05 msec, 1 bit delta is 2.11 msec */
		longpulse = (delta.attoseconds < ATTOTIME_IN_USEC(1500).attoseconds) ? 0 : 1;
		player->accumulator = (player->accumulator << 1) | longpulse;

#if 0
		{
			int usecdiff = (int)(delta.attoseconds / ATTOSECONDS_IN_USEC(1));
			printf("bitdelta = %5d (%d) - accum = %04X\n", usecdiff, longpulse, player->accumulator);
		}
#endif

		/* if we have a complete command, signal it */
		/* a complete command is 0,0,1 followed by 5 bits, followed by 0,0 */
		if ((player->accumulator & 0x383) == 0x80)
		{
			UINT8 newcommand = (player->accumulator >> 2) & 0x1f;

//printf("New command = %02X (last=%02X)\n", newcommand, player->lastcommand);

#if EMULATE_PR8210_ROM
			printf("Command = %02X\n", newcommand);
//			player->pia_porta = BITSWAP8(newcommand, 4,3,2,1,0,5,6,7);
			player->pia_porta = BITSWAP8(newcommand, 0,1,2,3,4,5,6,7);
#else
			/* if we got a double command, act on it */
			if (newcommand == player->lastcommand)
			{
				pr8210_hle_command(ld);
				player->lastcommand = 0;
			}
			else
				player->lastcommand = newcommand;
#endif
		}
	}
}



/***************************************************************************
    SIMUTREK MODIFIED PR-8210 PLAYER IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------

    Command Set:

    FX XX XX  : Seek to frame XXXXX
    01-19     : Skip forward 1-19 frames
    99-81     : Skip back 1-19 frames
    5a        : Toggle frame display

-------------------------------------------------*/

/*-------------------------------------------------
    simutrek_init - Simutrek-specific
    initialization
-------------------------------------------------*/

static void simutrek_init(laserdisc_state *ld)
{
	/* do a soft reset */
	pr8210_soft_reset(ld);
}


/*-------------------------------------------------
    simutrek_status_r - Simutrek-specific
    command processing
-------------------------------------------------*/

static UINT8 simutrek_status_r(laserdisc_state *ld)
{
	return (ld->state.state != LDSTATE_SEEKING) ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    simutrek_data_w - Simutrek-specific
    data processing
-------------------------------------------------*/

static void simutrek_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	ldplayer_data *player = ld->player;

	/* Acknowledge every command byte */
	if (player->cmd_ack_callback != NULL)
		(*player->cmd_ack_callback)();

	/* Is this byte part of a multi-byte seek command? */
	if (player->cmdcnt > 0)
	{
		CMDPRINTF(("Simutrek: Seek to frame byte %d of 3\n", player->cmdcnt + 1));
		player->cmdbytes[player->cmdcnt++] = data;

		if (player->cmdcnt == 3)
		{
			player->parameter = ((player->cmdbytes[0] & 0xf) * 10000) +
								((player->cmdbytes[1] >> 4)  * 1000) +
								((player->cmdbytes[1] & 0xf) * 100) +
								((player->cmdbytes[2] >> 4)  * 10) +
								(player->cmdbytes[2] & 0xf);

			CMDPRINTF(("Simutrek: Seek to frame %d\n", player->parameter));

			pr8210_switch_state(ld, LDSTATE_SEEKING, player->parameter);
			player->cmdcnt = 0;
		}
	}
	else if (data == 0)
	{
		CMDPRINTF(("Simutrek: 0 ?\n"));
	}
	else if ((data & 0xf0) == 0xf0)
	{
		CMDPRINTF(("Simutrek: Seek to frame byte 1 of 3\n"));
		player->cmdbytes[player->cmdcnt++] = data;
	}
	else if (data >= 1 && data <= 0x19)
	{
		int count = ((data >> 4) * 10) + (data & 0xf);
		CMDPRINTF(("Simutrek: Step forwards by %d frame(s)\n", count));
		pr8210_switch_state(ld, LDSTATE_STEPPING_BY_PARAMETER, count);
	}
	else if (data >= 0x81 && data <= 0x99)
	{
		int count = (((data >> 4) * 10) + (data & 0xf)) - 100;
		CMDPRINTF(("Simutrek: Step backwards by %d frame(s)\n", count));
		pr8210_switch_state(ld, LDSTATE_STEPPING_BY_PARAMETER, count);
	}
	else if (data == 0x5a)
	{
		CMDPRINTF(("Simutrek: Frame window toggle\n"));
		player->framedisplay ^= 1;
	}
	else
	{
		CMDPRINTF(("Simutrek: Unknown command (%.2x)\n", data));
	}
}


/*-------------------------------------------------
    simutrek_set_audio_squelch - Simutrek-specific
    command to enable/disable audio squelch
-------------------------------------------------*/

void simutrek_set_audio_squelch(const device_config *device, int state)
{
	laserdisc_state *ld = ldcore_get_safe_token(device);
	int squelch = (state == 0);
	if (squelch)
		ldcore_set_audio_squelch(ld, squelch, squelch);
	else
		ldcore_set_audio_squelch(ld, squelch, squelch);
}


/*-------------------------------------------------
    simutrek_set_audio_squelch - Simutrek-specific
    command to set callback function for
    player/interface command acknowledge
-------------------------------------------------*/

void simutrek_set_cmd_ack_callback(const device_config *device, void (*callback)(void))
{
	laserdisc_state *ld = ldcore_get_safe_token(device);
	ldplayer_data *player = ld->player;

	player->cmd_ack_callback = callback;
}



/***************************************************************************
    RE-IMPLEMENTATION USING ACTUAL ROMS
***************************************************************************/

/*************************************
 *
 *  Test PR-8210 ROM emulation
 *
 *************************************/

static laserdisc_state *find_pr8210(running_machine *machine)
{
	return ldcore_get_safe_token(device_list_first(machine->config->devicelist, LASERDISC));
}


static READ8_HANDLER( pr8210_pia_r )
{
	laserdisc_state *ld = find_pr8210(machine);
	ldplayer_data *player = ld->player;
	UINT8 result = player->pia[offset];
	switch (offset)
	{
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0xc0:
		case 0xe0:
			break;
	
		case 0xa0:
//          printf("%03X:pia_r(%02X) = %02X\n", activecpu_get_pc(), offset, player->pia_porta);
			result = player->pia_porta;
//			player->pia_porta = 0;
			break;

		default:
			printf("%03X:pia_r(%02X)\n", activecpu_get_pc(), offset);
			break;
	}
	return result;
}

static WRITE8_HANDLER( pr8210_pia_w )
{
	/*
       $22-26 (R) = read and copied to memory $23-27
       $23 (R) = something compared against $F4
       $22-26 (W) = SRCH. text
       $27-2B (W) = FRAME/CHAP. text
       $2C-30 (W) = frame or chapter number
       $40 (W) = $CF at initialization, tracked by ($78)
       $60 (W) = port B output, tracked by ($77)
           $80 = n/c
           $40 = (out) LED3
           $20 = (out) LED2
           $10 = (out) LED1
                    123 -> LHL = Play
                        -> HLL = Slow fwd
                        -> LLL = Slow rev
                        -> HHL = Still
                        -> LLH = Pause
                        -> HHH = all off
           $08 = (out) CAV LED
           $04 = (out) CLV LED
           $02 = (out) A2 LED/AUDIO 2
           $01 = (out) A1 LED/AUDIO 1
       $80 (W) = 0 or 1
       $A0 (R) = port A input
       $C0 (R) = stored to ($2E)
       $E0 (R) = stored to ($2F)
    */
	laserdisc_state *ld = find_pr8210(machine);
	ldplayer_data *player = ld->player;
	if (player->pia[offset] != data)
	{
		switch (offset)
		{
			case 0x40:
				if (!(data & 0x02) && (player->pia[offset] & 0x02))
					player->framedisplay = 1;
				printf("%03X:pia_w(%02X) = %02X\n", activecpu_get_pc(), offset, data);
				break;
				
			case 0x60:
				printf("%03X:pia_w(%02X) = %02X (PORT B LEDS:", activecpu_get_pc(), offset, data);
				output_set_value("pr8210_audio1", (data & 0x01) != 0);
				output_set_value("pr8210_audio2", (data & 0x02) != 0);
				output_set_value("pr8210_clv", (data & 0x04) != 0);
				output_set_value("pr8210_cav", (data & 0x08) != 0);
				output_set_value("pr8210_led1", (data & 0x10) == 0);
				output_set_value("pr8210_led2", (data & 0x20) == 0);
				output_set_value("pr8210_led3", (data & 0x40) == 0);
				if (!(data & 0x80)) printf(" ???");
				printf(")\n");
				player->pia_portb = data;
				update_audio_squelch(ld);
				break;

			default:
				printf("%03X:pia_w(%02X) = %02X\n", activecpu_get_pc(), offset, data);
				break;
		}
		player->pia[offset] = data;
	}
}

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
	UINT8 focus_on = !(player->porta & 0x08);
	UINT8 spdl_on = !(player->porta & 0x10);
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

	/* loop at beginning waits for $40=0, $02=1 */
	return result;
}

static WRITE8_HANDLER( pr8210_porta_w )
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
	if ((data & 0xfe) != (player->porta & 0xfe))
	{
		int direction = (data & 0x80) ? 1 : -1;

		printf("%03X:porta_w = %02X", activecpu_get_pc(), data);
		if (!(data & 0x01) && (player->porta & 0x01))
			ldcore_advance_slider(ld, direction);
		if (!(data & 0x02))
			printf(" SCAN:%c:%c", (data & 0x80) ? 'F' : 'R', (data & 0x04) ? 'L' : 'H');
		if (!(data & 0x08)) printf(" /FOCUSON");
		if (!(data & 0x10)) printf(" /SPDLON");
		if (data & 0x20) printf(" VIDEOSQ");
		if (data & 0x40) printf(" AUDIOSQ");
		printf("\n");
		player->porta = data;

		ldcore_set_video_squelch(ld, (data & 0x20) != 0);
		update_audio_squelch(ld);
		if (!(data & 0x02))
		{
			int delta = (data & 0x04) ? PR8210_SCAN_SPEED : PR8210_SEEK_FAST_SPEED;
			ldcore_set_slider_speed(ld, delta * direction);
		}
		else
			ldcore_set_slider_speed(ld, 0);
	}
}

static WRITE8_HANDLER( pr8210_portb_w )
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
	cpunum_set_input_line(machine, player->cpunum, 0, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	if ((data & 0x7f) != (player->portb & 0x7f))
	{
		printf("%03X:portb_w = %02X", activecpu_get_pc(), data);
		if (!(data & 0x01)) printf(" LASERON");
		if (!(data & 0x02)) printf(" ???");
		if (!(data & 0x04)) printf(" TP1");
		if (!(data & 0x08)) printf(" TP2");
		output_set_value("pr8210_standby", !(data & 0x10));
		if (!(data & 0x20)) printf(" SLOWTRG");
		if (!(data & 0x40)) printf(" IRQGEN");
//      if (data & 0x80) printf(" PIASEL");
		printf("\n");
		player->portb = data;
	}
}


static READ8_HANDLER( pr8210_t0_r )
{
	/* returns VSYNC state */
	laserdisc_state *ld = find_pr8210(machine);
	return !ld->player->vsync;
}


static READ8_HANDLER( pr8210_t1_r )
{
	/* must return 1 or else it tries to jump to an external ROM */
	return 1;
}


static ADDRESS_MAP_START( pr8210_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( pr8210_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(pr8210_pia_r, pr8210_pia_w)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(pr8210_bus_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(pr8210_porta_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(pr8210_portb_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(pr8210_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(pr8210_t1_r)
ADDRESS_MAP_END


MACHINE_DRIVER_START( pr8210 )
	MDRV_CPU_ADD("pr8210", I8049, XTAL_4_41MHz)
	MDRV_CPU_PROGRAM_MAP(pr8210_map,0)
	MDRV_CPU_IO_MAP(pr8210_portmap,0)
MACHINE_DRIVER_END
