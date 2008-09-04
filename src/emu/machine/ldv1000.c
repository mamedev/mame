/*************************************************************************

    ldv1000.c

    Pioneer LD-V1000 laserdisc emulation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#include "ldcore.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_COMMANDS				1
#define CMDPRINTF(x)				do { if (PRINTF_COMMANDS) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Player-specific states */
enum
{
	LDSTATE_SKIPPING = LDSTATE_OTHER,
	LDSTATE_WAITING
};

/* Pioneer LD-V1000 specific information */
enum
{
	LDV1000_MODE_STATUS,
	LDV1000_MODE_GET_FRAME,
	LDV1000_MODE_GET_1ST,
	LDV1000_MODE_GET_2ND,
	LDV1000_MODE_GET_RAM
};

#define LDV1000_SCAN_SPEED			(2000 / 30)			/* 2000 frames/second */
#define LDV1000_SCAN_DURATION		(4)					/* scan for 4 vsyncs each time */



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

	UINT8				mode;					/* current mode */
	UINT8				lastchapter;			/* last chapter seen */
	UINT16				lastframe;				/* last frame seen */
	UINT32				activereg;				/* active register index */
	UINT8				ram[1024];				/* RAM */
	UINT32				readpos;				/* current read position */
	UINT32				readtotal;				/* current read position */
	UINT8				readbuf[256];			/* temporary read buffer */
	attotime			lastvsynctime;			/* time of last update/vsync */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ldv1000_init(laserdisc_state *ld);
static void ldv1000_soft_reset(laserdisc_state *ld);
static void ldv1000_update_squelch(laserdisc_state *ld);
static int ldv1000_switch_state(laserdisc_state *ld, UINT8 newstate, INT32 stateparam);
static INT32 ldv1000_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static void ldv1000_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data);
static UINT8 ldv1000_status_strobe_r(laserdisc_state *ld);
static UINT8 ldv1000_command_strobe_r(laserdisc_state *ld);
static UINT8 ldv1000_status_r(laserdisc_state *ld);



/***************************************************************************
    INTERFACES
***************************************************************************/

const ldplayer_interface ldv1000_interface =
{
	LASERDISC_TYPE_PIONEER_LDV1000,				/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Pioneer LD-V1000",							/* name of the player */
	ldv1000_init,								/* initialization callback */
	ldv1000_update,								/* update callback */
	ldv1000_data_w,								/* parallel data write */
	{											/* single line write: */
		NULL,									/*    LASERDISC_LINE_ENTER */
		NULL									/*    LASERDISC_LINE_CONTROL */
	},
	ldv1000_status_r,							/* parallel data read */
	{											/* single line read: */
		NULL,									/*    LASERDISC_LINE_READY */
		ldv1000_status_strobe_r,				/*    LASERDISC_LINE_STATUS */
		ldv1000_command_strobe_r,				/*    LASERDISC_LINE_COMMAND */
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
	return (state == LDSTATE_SCANNING ||
			state == LDSTATE_SKIPPING ||
			state == LDSTATE_WAITING);
}


/*-------------------------------------------------
    read_16bits_from_ram_be - read 16 bits from
    player RAM in big-endian format
-------------------------------------------------*/

INLINE UINT16 read_16bits_from_ram_be(UINT8 *ram, UINT32 offset)
{
	return (ram[offset + 0] << 8) | ram[offset + 1];
}


/*-------------------------------------------------
    write_16bits_to_ram_be - write 16 bits to
    player RAM in big endian format
-------------------------------------------------*/

INLINE void write_16bits_to_ram_be(UINT8 *ram, UINT32 offset, UINT16 data)
{
	ram[offset + 0] = data >> 8;
	ram[offset + 1] = data >> 0;
}



/***************************************************************************
    PIONEER LD-V1000 IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ldv1000_init - Pioneer PR-8210-specific
    initialization
-------------------------------------------------*/

static void ldv1000_init(laserdisc_state *ld)
{
	/* do a soft reset */
	ldv1000_soft_reset(ld);
}


/*-------------------------------------------------
    ldv1000_soft_reset - Pioneer LDV-1000-specific
    soft reset
-------------------------------------------------*/

static void ldv1000_soft_reset(laserdisc_state *ld)
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

	player->mode = 0;
	player->activereg = 0;
	memset(player->ram, 0, sizeof(player->ram));
	player->readpos = 0;
	player->readtotal = 0;
	memset(player->readbuf, 0, sizeof(player->readbuf));
	player->lastvsynctime = curtime;

	ldv1000_switch_state(ld, LDSTATE_PARKED, 0);
}


/*-------------------------------------------------
    ldv1000_update_squelch - update the squelch
    settings based on the current state
-------------------------------------------------*/

static void ldv1000_update_squelch(laserdisc_state *ld)
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
		case LDSTATE_SKIPPING:
		case LDSTATE_WAITING:
			ldcore_set_audio_squelch(ld, TRUE, TRUE);
			ldcore_set_video_squelch(ld, FALSE);
			break;

		/* video off, audio off */
		case LDSTATE_NONE:
		case LDSTATE_EJECTING:
		case LDSTATE_EJECTED:
		case LDSTATE_PARKED:
		case LDSTATE_LOADING:
		case LDSTATE_SPINUP:
		case LDSTATE_SEEKING:
			ldcore_set_audio_squelch(ld, TRUE, TRUE);
			ldcore_set_video_squelch(ld, TRUE);
			break;
	}
}


/*-------------------------------------------------
    ldv1000_switch_state - attempt to switch states
    if appropriate, and ensure the squelch values
    are correct
-------------------------------------------------*/

static int ldv1000_switch_state(laserdisc_state *ld, UINT8 newstate, INT32 parameter)
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
	ldv1000_update_squelch(ld);

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

		case LDSTATE_WAITING:
			ld->state.endtime = attotime_add(curtime, attotime_mul(ATTOTIME_IN_MSEC(100), ld->state.param));
			break;
	}
	return TRUE;
}


/*-------------------------------------------------
    ldv1000_update - Pioneer PR-8210-specific
    update callback
-------------------------------------------------*/

static INT32 ldv1000_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	ldplayer_data *player = ld->player;
	ldplayer_state newstate;
	INT32 advanceby = 0;
	int frame, chapter;

	/* remember the last frame and chapter */
	player->lastvsynctime = curtime;
	frame = frame_from_metadata(vbi);
	if (frame != FRAME_NOT_PRESENT)
		player->lastframe = frame;
	chapter = chapter_from_metadata(vbi);
	if (chapter != CHAPTER_NOT_PRESENT)
		player->lastchapter = chapter;

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
		case LDSTATE_SEEKING:
			/* generic behaviors are appropriate for all of these commands */
			advanceby = ldcore_generic_update(ld, vbi, fieldnum, curtime, &newstate);
			ldv1000_switch_state(ld, newstate.state, newstate.param);
			break;

		case LDSTATE_SKIPPING:
			/* just advance and return to previous state */
			advanceby = ld->state.param;
			ldv1000_switch_state(ld, ld->savestate.state, ld->savestate.param);
			break;

		case LDSTATE_WAITING:
			/* keep trying to switch to the previous state until we succeed */
			ldv1000_switch_state(ld, ld->savestate.state, ld->savestate.param);
			break;
	}

	return advanceby;
}


/*-------------------------------------------------
    ldv1000_data_w - write callback when the
    ENTER state is written
-------------------------------------------------*/

static void ldv1000_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	ldplayer_data *player = ld->player;
	int target;

	/* 0xff bytes are used for synchronization */
	if (data == 0xff)
		return;

	/* handle commands */
	switch (data)
	{
		case 0x3f:	CMDPRINTF(("ldv1000: 0\n"));
			player->parameter = (player->parameter == -1) ? 0 : (player->parameter * 10 + 0);
			return;

		case 0x0f:	CMDPRINTF(("ldv1000: 1\n"));
			player->parameter = (player->parameter == -1) ? 1 : (player->parameter * 10 + 1);
			return;

		case 0x8f:	CMDPRINTF(("ldv1000: 2\n"));
			player->parameter = (player->parameter == -1) ? 2 : (player->parameter * 10 + 2);
			return;

		case 0x4f:	CMDPRINTF(("ldv1000: 3\n"));
			player->parameter = (player->parameter == -1) ? 3 : (player->parameter * 10 + 3);
			return;

		case 0x2f:	CMDPRINTF(("ldv1000: 4\n"));
			player->parameter = (player->parameter == -1) ? 4 : (player->parameter * 10 + 4);
			return;

		case 0xaf:	CMDPRINTF(("ldv1000: 5\n"));
			player->parameter = (player->parameter == -1) ? 5 : (player->parameter * 10 + 5);
			return;

		case 0x6f:	CMDPRINTF(("ldv1000: 6\n"));
			player->parameter = (player->parameter == -1) ? 6 : (player->parameter * 10 + 6);
			return;

		case 0x1f:	CMDPRINTF(("ldv1000: 7\n"));
			player->parameter = (player->parameter == -1) ? 7 : (player->parameter * 10 + 7);
			return;

		case 0x9f:	CMDPRINTF(("ldv1000: 8\n"));
			player->parameter = (player->parameter == -1) ? 8 : (player->parameter * 10 + 8);
			return;

		case 0x5f:	CMDPRINTF(("ldv1000: 9\n"));
			player->parameter = (player->parameter == -1) ? 9 : (player->parameter * 10 + 9);
			return;

		case 0x7f:  CMDPRINTF(("ldv1000: %d Recall\n", player->parameter));
			/* set the active register */
			player->activereg = player->parameter;
			/* should also display the register value */
			break;

		case 0x20:  CMDPRINTF(("ldv1000: x0 reverse (stop) - Badlands special\n"));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_FAST_REVERSE, 0);
			break;

		case 0x21:  CMDPRINTF(("ldv1000: x1/4 reverse - Badlands special\n"));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_SLOW_REVERSE, 4);
			break;

		case 0x22:  CMDPRINTF(("ldv1000: x1/2 reverse - Badlands special\n"));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_SLOW_REVERSE, 2);
			break;

		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:  CMDPRINTF(("ldv1000: x%d reverse - Badlands special\n", (data & 0x07) - 2));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_FAST_REVERSE, (data & 0x07) - 2);
			break;

		case 0xa0:  CMDPRINTF(("ldv1000: x0 forward (stop)\n"));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_FAST_FORWARD, 0);
			break;

		case 0xa1:  CMDPRINTF(("ldv1000: x1/4 forward\n"));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_SLOW_FORWARD, 4);
			break;

		case 0xa2:  CMDPRINTF(("ldv1000: x1/2 forward\n"));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_SLOW_FORWARD, 2);
			break;

		case 0xa3:
		case 0xa4:
		case 0xa5:
		case 0xa6:
		case 0xa7:  CMDPRINTF(("ldv1000: x%d forward\n", (data & 0x07) - 2));
			ldv1000_switch_state(ld, LDSTATE_PLAYING_FAST_FORWARD, (data & 0x07) - 2);
			break;

		case 0xb1:
		case 0xb2:
		case 0xb3:
		case 0xb4:
		case 0xb5:
		case 0xb6:
		case 0xb7:
		case 0xb8:
		case 0xb9:
		case 0xba:  CMDPRINTF(("ldv1000: Skip forward %d0\n", data & 0x0f));
			/* note that this skips tracks, not frames */
			ldv1000_switch_state(ld, LDSTATE_SKIPPING, 10 * (data & 0x0f));
			break;

		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3a:  CMDPRINTF(("ldv1000: Skip backwards %d0 - Badlands special\n", data & 0x0f));
			/* note that this skips tracks, not frames */
			ldv1000_switch_state(ld, LDSTATE_SKIPPING, -10 * (data & 0x0f));
			break;

		case 0xbf:	CMDPRINTF(("ldv1000: %d Clear\n", player->parameter));
			/* clears register display and removes pending arguments */
			player->parameter = -1;
			break;

		case 0xc2:	CMDPRINTF(("ldv1000: Get frame no.\n"));
			/* returns the current frame number */
			player->mode = LDV1000_MODE_GET_FRAME;
			player->readpos = 0;
			player->readtotal = 5;
			sprintf((char *)player->readbuf, "%05d", player->lastframe);
			break;

		case 0xc3:	CMDPRINTF(("ldv1000: Get 2nd display\n"));
			/* returns the data from the 2nd display line */
			player->mode = LDV1000_MODE_GET_2ND;
			player->readpos = 0;
			player->readtotal = 8;
			sprintf((char *)player->readbuf, "	 \x1c\x1c\x1c");
			break;

		case 0xc4:	CMDPRINTF(("ldv1000: Get 1st display\n"));
			/* returns the data from the 1st display line */
			player->mode = LDV1000_MODE_GET_1ST;
			player->readpos = 0;
			player->readtotal = 8;
			sprintf((char *)player->readbuf, "	 \x1c\x1c\x1c");
			break;

		case 0xc8:	CMDPRINTF(("ldv1000: Transfer memory\n"));
			/* returns the data from RAM */
			player->mode = LDV1000_MODE_GET_RAM;
			player->readpos = 0;
			player->readtotal = 1024;
			break;

		case 0xcc:	CMDPRINTF(("ldv1000: Load\n"));
			/* load program from disc -- not implemented */
			break;

		case 0xcd:	CMDPRINTF(("ldv1000: Display disable\n"));
			/* disables the display of current command -- not implemented */
			break;

		case 0xce:	CMDPRINTF(("ldv1000: Display enable\n"));
			/* enables the display of current command -- not implemented */
			break;

		case 0xf0:	CMDPRINTF(("ldv1000: Scan forward\n"));
			if (ld->state.state != LDSTATE_SCANNING)
				ldv1000_switch_state(ld, LDSTATE_SCANNING, SCANNING_PARAM(LDV1000_SCAN_SPEED, LDV1000_SCAN_DURATION));
			else
				ld->state.substate = 0;
			break;

		case 0xf1:  CMDPRINTF(("ldv1000: %d Display\n", player->parameter));
			player->framedisplay = (player->parameter == -1) ? !player->framedisplay : (player->parameter & 1);
			break;

		case 0xf3:  CMDPRINTF(("ldv1000: %d Autostop\n", player->parameter));
			target = (player->parameter != -1) ? player->parameter : read_16bits_from_ram_be(player->ram, (player->activereg++ * 2) % 1024);
			if (target > player->lastframe)
				ldv1000_switch_state(ld, LDSTATE_PLAYING, target);
			else
				ldv1000_switch_state(ld, LDSTATE_SEEKING, target);
			break;

		case 0xf4:  CMDPRINTF(("ldv1000: %d Audio track 1\n", player->parameter));
			if (player->parameter == -1)
				player->audio1disable ^= 1;
			else
				player->audio1disable = ~player->parameter & 1;
			break;

		case 0xf5:	CMDPRINTF(("ldv1000: %d Store\n", player->parameter));
			/* store either the current frame number or an explicit value into the active register */
			if (player->parameter == -1)
				write_16bits_to_ram_be(player->ram, (player->activereg * 2) % 1024, player->lastframe);
			else
				write_16bits_to_ram_be(player->ram, (player->activereg * 2) % 1024, player->parameter);
			player->activereg++;
			break;

		case 0xf6:	CMDPRINTF(("ldv1000: Step forward\n"));
			ldv1000_switch_state(ld, LDSTATE_STEPPING_FORWARD, 0);
			break;

		case 0xf7:  CMDPRINTF(("ldv1000: %d Search\n", player->parameter));
			target = (player->parameter != -1) ? player->parameter : read_16bits_from_ram_be(player->ram, (player->activereg * 2) % 1024);
			player->activereg++;
			ldv1000_switch_state(ld, LDSTATE_SEEKING, target);
			break;

		case 0xf8:	CMDPRINTF(("ldv1000: Scan reverse\n"));
			if (ld->state.state != LDSTATE_SCANNING)
				ldv1000_switch_state(ld, LDSTATE_SCANNING, SCANNING_PARAM(-LDV1000_SCAN_SPEED, LDV1000_SCAN_DURATION));
			else
				ld->state.substate = 0;
			break;

		case 0xf9:	CMDPRINTF(("ldv1000: Reject\n"));
			ldv1000_switch_state(ld, LDSTATE_EJECTING, 0);
			break;

		case 0xfb:	CMDPRINTF(("ldv1000: %d Stop/Wait\n", player->parameter));
			ldv1000_switch_state(ld, LDSTATE_WAITING, player->parameter);
			break;

		case 0xfc:  CMDPRINTF(("ldv1000: %d Audio track 2\n", player->parameter));
			if (player->parameter == -1)
				player->audio2disable ^= 1;
			else
				player->audio2disable = ~player->parameter & 1;
			break;

		case 0xfd:  CMDPRINTF(("ldv1000: Play\n"));
			if (ld->state.state == LDSTATE_EJECTED)
				ldv1000_switch_state(ld, LDSTATE_LOADING, 0);
			else if (ld->state.state == LDSTATE_PARKED)
				ldv1000_switch_state(ld, LDSTATE_SPINUP, 0);
			else
				ldv1000_switch_state(ld, LDSTATE_PLAYING, -1);
			break;

		case 0xfe:	CMDPRINTF(("ldv1000: Step reverse\n"));
			ldv1000_switch_state(ld, LDSTATE_STEPPING_REVERSE, 0);
			break;

		default:	CMDPRINTF(("ldv1000: %d Unknown command %02X\n", player->parameter, data));
			/* unknown command */
			break;
	}

	/* reset the parameter after executing a command */
	player->parameter = -1;
}


/*-------------------------------------------------
    ldv1000_status_strobe_r - return state of the
    status strobe
-------------------------------------------------*/

static UINT8 ldv1000_status_strobe_r(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;

	/* the status strobe is asserted (active low) 500-650usec after VSYNC */
	/* for a duration of 26usec; we pick 600-626usec */
	attotime delta = attotime_sub(timer_get_time(), player->lastvsynctime);
	if (delta.attoseconds >= ATTOTIME_IN_USEC(600).attoseconds && delta.attoseconds < ATTOTIME_IN_USEC(626).attoseconds)
		return ASSERT_LINE;

	return CLEAR_LINE;
}


/*-------------------------------------------------
    ldv1000_command_strobe_r - return state of the
    command strobe
-------------------------------------------------*/

static UINT8 ldv1000_command_strobe_r(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;

	/* the command strobe is asserted (active low) 54 or 84usec after the status */
	/* strobe for a duration of 25usec; we pick 600+84 = 684-709usec */
	/* for a duration of 26usec; we pick 600-626usec */
	attotime delta = attotime_sub(timer_get_time(), player->lastvsynctime);
	if (delta.attoseconds >= ATTOTIME_IN_USEC(684).attoseconds && delta.attoseconds < ATTOTIME_IN_USEC(709).attoseconds)
		return ASSERT_LINE;

	return CLEAR_LINE;
}


/*-------------------------------------------------
    ldv1000_status_r - return state of the ready
    line
-------------------------------------------------*/

static UINT8 ldv1000_status_r(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	UINT8 status = 0xff;

	/* switch off the current mode */
	switch (player->mode)
	{
		/* reading frame number returns 5 characters */
		/* reading display lines returns 8 characters */
		case LDV1000_MODE_GET_FRAME:
		case LDV1000_MODE_GET_1ST:
		case LDV1000_MODE_GET_2ND:
			assert(player->readpos < player->readtotal);
			status = player->readbuf[player->readpos++];
			if (player->readpos == player->readtotal)
				player->mode = LDV1000_MODE_STATUS;
			break;

		/* reading RAM returns 1024 bytes */
		case LDV1000_MODE_GET_RAM:
			assert(player->readpos < player->readtotal);
			status = player->ram[1023 - player->readpos++];
			if (player->readpos == player->readtotal)
				player->mode = LDV1000_MODE_STATUS;
			break;

		/* otherwise, we just compute a status code */
		default:
		case LDV1000_MODE_STATUS:
			switch (ld->state.state)
			{
				case LDSTATE_EJECTING:				status = 0x60;		break;
				case LDSTATE_EJECTED:				status = 0xe0;		break;
				case LDSTATE_PARKED:				status = 0xfc;		break;
				case LDSTATE_LOADING:				status = 0x48;		break;
				case LDSTATE_SPINUP:				status = 0x48;		break;
				case LDSTATE_PAUSING:				status = 0xe5;		break;
				case LDSTATE_PAUSED:				status = 0xe5;		break;
				case LDSTATE_PLAYING:				status = 0xe4;		break;
				case LDSTATE_PLAYING_SLOW_REVERSE:	status = 0xae;		break;
				case LDSTATE_PLAYING_SLOW_FORWARD:	status = 0xae;		break;
				case LDSTATE_PLAYING_FAST_REVERSE:	status = 0xae;		break;
				case LDSTATE_PLAYING_FAST_FORWARD:	status = 0xae;		break;
				case LDSTATE_STEPPING_FORWARD:		status = 0xe5;		break;
				case LDSTATE_STEPPING_REVERSE:		status = 0xe5;		break;
				case LDSTATE_SCANNING:				status = 0x4c;		break;
				case LDSTATE_SEEKING:				status = 0x50;		break;
//              case LASERDISC_SEARCH_FINISHED:     status = 0xd0;      break;
//              case LASERDISC_AUTOSTOPPED:         status = 0x54;      break;
				default:
					fatalerror("Unexpected disc state in ldv1000_status_r\n");
					break;
			}
			break;
	}

	/* bit 7 indicates our busy status */
	return status;
}
