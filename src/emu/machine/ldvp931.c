/*************************************************************************

    ldvp931.c

    Philips 22VP931 laserdisc emulation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************

    Still to do:

        * determine actual slow/fast speeds
        *

*************************************************************************/

#include "ldcore.h"
#include "cpu/mcs48/mcs48.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_COMMANDS				0
#define LOG_PORTS					0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* scanning speeds */
#define SCAN_SPEED						(2000 / 30)			/* 2000 frames/second */
#define SCAN_FAST_SPEED					(4000 / 30)			/* 4000 frames/second */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* player-specific data */
struct _ldplayer_data
{
	/* low-level emulation data */
	const device_config *cpu;					/* CPU index of the 8049 */
	const device_config *tracktimer;			/* timer device */
	vp931_data_ready_func data_ready_cb; 		/* data ready callback */

	/* I/O port states */
	UINT8				out0;					/* output 0 state */
	UINT8				out1;					/* output 1 state */
	UINT8				port1;					/* port 1 state */

	/* DATIC circuit implementation */
	UINT8				daticval;				/* latched DATIC value */
	UINT8				daticerp;				/* /ERP value from DATIC */
	UINT8				datastrobe;				/* DATA STROBE line from DATIC */

	/* communication status */
	UINT8				fromcontroller;			/* command byte from the controller */
	UINT8				fromcontroller_pending;	/* TRUE if data is pending */
	UINT8				tocontroller;			/* command byte to the controller */
	UINT8				tocontroller_pending;	/* TRUE if data is pending */

	/* tracking */
	INT8				trackdir;				/* direction of tracking */
	UINT8				trackstate;				/* state of tracking */

	/* debugging */
	UINT8				cmdbuf[3];				/* 3 bytes worth of commands */
	UINT8				cmdcount;				/* number of command bytes seen */
	INT16				advanced;				/* number of frames advanced */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void vp931_init(laserdisc_state *ld);
static void vp931_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static INT32 vp931_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static void vp931_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data);
static UINT8 vp931_data_r(laserdisc_state *ld);
static UINT8 vp931_ready(laserdisc_state *ld);
static UINT8 vp931_data_ready(laserdisc_state *ld);

static TIMER_CALLBACK( vbi_data_fetch );
static TIMER_CALLBACK( deferred_data_w );
static TIMER_CALLBACK( irq_off );
static TIMER_CALLBACK( datastrobe_off );
static TIMER_CALLBACK( erp_off );
static TIMER_DEVICE_CALLBACK( track_timer );

static WRITE8_HANDLER( output0_w );
static WRITE8_HANDLER( output1_w );
static WRITE8_HANDLER( lcd_w );
static READ8_HANDLER( unknown_r );
static READ8_HANDLER( keypad_r );
static READ8_HANDLER( datic_r );
static READ8_HANDLER( from_controller_r );
static WRITE8_HANDLER( to_controller_w );
static READ8_HANDLER( port1_r );
static WRITE8_HANDLER( port1_w );
static READ8_HANDLER( port2_r );
static WRITE8_HANDLER( port2_w );
static READ8_HANDLER( t0_r );
static READ8_HANDLER( t1_r );



/***************************************************************************
    22VP931 ROM AND MACHINE INTERFACES
***************************************************************************/

static ADDRESS_MAP_START( vp931_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xcf) AM_READWRITE(keypad_r, output0_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0xcf) AM_READWRITE(unknown_r, output1_w)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0xcf) AM_READWRITE(datic_r, lcd_w)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0xcf) AM_READWRITE(from_controller_r, to_controller_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(port1_r, port1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(port2_r, port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( vp931 )
	MDRV_CPU_ADD("vp931", I8049, XTAL_11MHz)
	MDRV_CPU_IO_MAP(vp931_portmap)
	MDRV_TIMER_ADD("tracktimer", track_timer)
MACHINE_DRIVER_END


ROM_START( vp931 )
	ROM_REGION( 0x800, "vp931", ROMREGION_LOADBYNAME )
	ROM_LOAD( "at-6-1_a.bin", 0x000, 0x800, CRC(e11b3c8d) SHA1(ea2d7f6a044ed085ce5e09d8b1b1a21c37f0e9b8) )
ROM_END



/***************************************************************************
    22VP931 PLAYER INTERFACE
***************************************************************************/

const ldplayer_interface vp931_interface =
{
	LASERDISC_TYPE_PHILLIPS_22VP931,			/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Phillips 22VP931",							/* name of the player */
	ROM_NAME(vp931),							/* pointer to ROM region information */
	MACHINE_DRIVER_NAME(vp931),					/* pointer to machine configuration */
	vp931_init,									/* initialization callback */
	vp931_vsync,								/* vsync callback */
	vp931_update,								/* update callback */
	NULL,										/* overlay callback */
	vp931_data_w,								/* parallel data write */
	{											/* single line write: */
		NULL,									/*    LASERDISC_LINE_ENTER */
		NULL									/*    LASERDISC_LINE_CONTROL */
	},
	vp931_data_r,								/* parallel data read */
	{											/* single line read: */
		vp931_ready,							/*    LASERDISC_LINE_READY */
		NULL,									/*    LASERDISC_LINE_STATUS */
		NULL,									/*    LASERDISC_LINE_COMMAND */
		vp931_data_ready,						/*    LASERDISC_LINE_DATA_AVAIL */
	}
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    vp931_set_data_ready_callback - set the data
    ready callback
-------------------------------------------------*/

void vp931_set_data_ready_callback(const device_config *device, vp931_data_ready_func callback)
{
	laserdisc_state *ld = ldcore_get_safe_token(device);
	ld->player->data_ready_cb = callback;
}



/***************************************************************************
    PHILLIPS 22VP931 IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    vp931_init - player-specific initialization
-------------------------------------------------*/

static void vp931_init(laserdisc_state *ld)
{
	astring *tempstring = astring_alloc();
	ldplayer_data *player = ld->player;
	vp931_data_ready_func cbsave;

	/* reset our state */
	cbsave = player->data_ready_cb;
	memset(player, 0, sizeof(*player));
	player->data_ready_cb = cbsave;

	/* find our devices */
	player->cpu = cputag_get_cpu(ld->device->machine, device_build_tag(tempstring, ld->device, "vp931"));
	player->tracktimer = devtag_get_device(ld->device->machine, device_build_tag(tempstring, ld->device, "tracktimer"));
	timer_device_set_ptr(player->tracktimer, ld);
	astring_free(tempstring);
}


/*-------------------------------------------------
    vp931_vsync - VSYNC callback, called at the
    start of the blanking period
-------------------------------------------------*/

static void vp931_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	/* reset our command counter (debugging only) */
	ld->player->cmdcount = 0;

	/* set the ERP signal to 1 to indicate start of frame, and set a timer to turn it off */
	ld->player->daticerp = 1;
	timer_set(ld->device->machine, video_screen_get_time_until_pos(ld->screen, 15*2, 0), ld, 0, erp_off);
}


/*-------------------------------------------------
    vp931_update - update callback, called on
    the first visible line of the frame
-------------------------------------------------*/

static INT32 vp931_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	/* set the first VBI timer to go at the start of line 16 */
	timer_set(ld->device->machine, video_screen_get_time_until_pos(ld->screen, 16*2, 0), ld, LASERDISC_CODE_LINE16 << 2, vbi_data_fetch);

	/* play forward by default */
	return fieldnum;
}


/*-------------------------------------------------
    vp931_data_w - handle a parallel data write
    to the 22VP931
-------------------------------------------------*/

static void vp931_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	/* set a timer to synchronize execution before sending the data */
	timer_call_after_resynch(ld->device->machine, ld, data, deferred_data_w);
}


/*-------------------------------------------------
    vp931_data_r - handle a parallel data read
    from the 22VP931
-------------------------------------------------*/

static UINT8 vp931_data_r(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;

	/* if data is pending, clear the pending flag and notify any callbacks */
	if (player->tocontroller_pending)
	{
		player->tocontroller_pending = FALSE;
		if (player->data_ready_cb != NULL)
			(*player->data_ready_cb)(ld->device, FALSE);
	}

	/* also boost interleave for 4 scanlines to ensure proper communications */
	cpuexec_boost_interleave(ld->device->machine, attotime_zero, attotime_mul(video_screen_get_scan_period(ld->screen), 4));
	return player->tocontroller;
}


/*-------------------------------------------------
    vp931_ready - return the status of "ready"
    to the caller (ready to accept another
    command)
-------------------------------------------------*/

static UINT8 vp931_ready(laserdisc_state *ld)
{
	/* if data is pending, we are not ready */
	ldplayer_data *player = ld->player;
	return player->fromcontroller_pending ? CLEAR_LINE : ASSERT_LINE;
}


/*-------------------------------------------------
    vp931_data_ready - return the status of
    "data available" to the caller
-------------------------------------------------*/

static UINT8 vp931_data_ready(laserdisc_state *ld)
{
	ldplayer_data *player = ld->player;
	return player->tocontroller_pending ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    vbi_data_fetch - called 4 times per scanline
    on lines 16, 17, and 18 to feed the VBI data
    through one byte at a time
-------------------------------------------------*/

static TIMER_CALLBACK( vbi_data_fetch )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;
	int which = param & 3;
	int line = param >> 2;
	UINT32 code = 0;

	/* fetch the code and compute the DATIC latched value */
	if (line >= LASERDISC_CODE_LINE16 && line <= LASERDISC_CODE_LINE18)
		code = laserdisc_get_field_code(ld->device, line, FALSE);

	/* at the start of each line, signal an interrupt and use a timer to turn it off */
	if (which == 0)
	{
		cpu_set_input_line(player->cpu, MCS48_INPUT_IRQ, ASSERT_LINE);
		timer_set(machine, ATTOTIME_IN_NSEC(5580), ld, 0, irq_off);
	}

	/* clock the data strobe on each subsequent callback */
	else if (code != 0)
	{
		player->daticval = code >> (8 * (3 - which));
		player->datastrobe = 1;
		timer_set(machine, ATTOTIME_IN_NSEC(5000), ld, 0, datastrobe_off);
	}

	/* determine the next bit to fetch and reprime ourself */
	if (++which == 4)
	{
		which = 0;
		line++;
	}
	if (line <= LASERDISC_CODE_LINE18 + 1)
		timer_set(machine, video_screen_get_time_until_pos(ld->screen, line*2, which * 2 * video_screen_get_width(ld->screen) / 4), ld, (line << 2) | which, vbi_data_fetch);
}


/*-------------------------------------------------
    deferred_data_w - handle a write from the
    external controller
-------------------------------------------------*/

static TIMER_CALLBACK( deferred_data_w )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;

	/* set the value and mark it pending */
	if (LOG_COMMANDS && player->fromcontroller_pending)
		printf("Dropped previous command byte\n");
	player->fromcontroller = param;
	player->fromcontroller_pending = TRUE;

	/* track the commands for debugging purposes */
	if (player->cmdcount < ARRAY_LENGTH(player->cmdbuf))
	{
		player->cmdbuf[player->cmdcount++ % 3] = param;
		if (LOG_COMMANDS && player->cmdcount % 3 == 0)
			printf("Cmd: %02X %02X %02X\n", player->cmdbuf[0], player->cmdbuf[1], player->cmdbuf[2]);
	}
}


/*-------------------------------------------------
    irq_off - turn off the 8048 IRQ signal
-------------------------------------------------*/

static TIMER_CALLBACK( irq_off )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	cpu_set_input_line(ld->player->cpu, MCS48_INPUT_IRQ, CLEAR_LINE);
}


/*-------------------------------------------------
    datastrobe_off - turn off the DATIC data
    strobe signal
-------------------------------------------------*/

static TIMER_CALLBACK( datastrobe_off )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ld->player->datastrobe = 0;
}


/*-------------------------------------------------
    erp_off - turn off the DATIC ERP signal
-------------------------------------------------*/

static TIMER_CALLBACK( erp_off )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ld->player->daticerp = 0;
}


/*-------------------------------------------------
    track_timer - advance by one half-track
-------------------------------------------------*/

static TIMER_DEVICE_CALLBACK( track_timer )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;

	/* advance by the count and toggle the state */
	player->trackstate ^= 1;
	if ((player->trackdir < 0 && !player->trackstate) || (player->trackdir > 0 && player->trackstate))
	{
		ldcore_advance_slider(ld, player->trackdir);
		player->advanced += player->trackdir;
	}
}


/*-------------------------------------------------
    output0_w - controls audio/video squelch
    and other bits
-------------------------------------------------*/

static WRITE8_HANDLER( output0_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;

	/*
        $80 = n/c
        $40 = LED (?) -> C335
        $20 = LED (?)
        $10 = LED (?) -> CX
        $08 = EJECT
        $04 = inverted -> AUDIO MUTE II
        $02 = inverted -> AUDIO MUTE I
        $01 = inverted -> VIDEO MUTE
    */

	if (LOG_PORTS && (player->out0 ^ data) & 0xff)
	{
		printf("%03X:out0:", cpu_get_pc(space->cpu));
		if ( (data & 0x80)) printf(" ???");
		if ( (data & 0x40)) printf(" LED1");
		if ( (data & 0x20)) printf(" LED2");
		if ( (data & 0x10)) printf(" LED3");
		if ( (data & 0x08)) printf(" EJECT");
		if (!(data & 0x04)) printf(" AUDMUTE2");
		if (!(data & 0x02)) printf(" AUDMUTE1");
		if (!(data & 0x01)) printf(" VIDMUTE");
		printf("\n");
		player->out0 = data;
	}

	/* update a/v squelch */
	ldcore_set_audio_squelch(ld, !(data & 0x02), !(data & 0x04));
	ldcore_set_video_squelch(ld, !(data & 0x01));
}


/*-------------------------------------------------
    output1_w - controls scanning behaviors
-------------------------------------------------*/

static WRITE8_HANDLER( output1_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;
	INT32 speed = 0;

	/*
        $80 = n/c
        $40 = n/c
        $20 = n/c
        $10 = n/c
        $08 = inverted -> SMS
        $04 = inverted -> SSS
        $02 = inverted -> SCAN CMD
        $01 = OSM
    */

	if (LOG_PORTS && (player->out1 ^ data) & 0x08)
	{
		mame_printf_debug("%03X:out1:", cpu_get_pc(space->cpu));
		if (!(data & 0x08)) mame_printf_debug(" SMS");
		mame_printf_debug("\n");
		player->out1 = data;
	}

	/* speed is 0 unless SCAN CMD is clear */
	speed = 0;
	if (!(data & 0x02))
	{
		/* fast/slow is based on bit 2 */
		speed = (data & 0x04) ? SCAN_FAST_SPEED : SCAN_SPEED;

		/* direction is based on bit 0 */
		if (data & 0x01)
			speed = -speed;
	}

	/* update the speed */
	ldcore_set_slider_speed(ld, speed);
}


/*-------------------------------------------------
    lcd_w - vestigial LCD frame display
-------------------------------------------------*/

static WRITE8_HANDLER( lcd_w )
{
	/*
        Frame number is written as 5 digits here; however, it is not actually
        connected
    */
}


/*-------------------------------------------------
    unknown_r - unknown input port
-------------------------------------------------*/

static READ8_HANDLER( unknown_r )
{
	/* only bit $80 is checked and its effects are minor */
	return 0x00;
}


/*-------------------------------------------------
    keypad_r - vestigial keypad/button controls
-------------------------------------------------*/

static READ8_HANDLER( keypad_r )
{
	/*
        From the code, this is apparently a vestigial keypad with basic controls:
            $01 = play
            $02 = still
            $04 = jump 25 frames backward
            $08 = jump 25 frames forward
            $10 = search for frame 50(?)
            $20 = search for frame 350(?)
            $40 = reset
            $80 = play reverse
    */
	return 0x00;
}


/*-------------------------------------------------
    datic_r - read the latched value from the
    DATIC circuit
-------------------------------------------------*/

static READ8_HANDLER( datic_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	return ld->player->daticval;
}


/*-------------------------------------------------
    from_controller_r - read the value the
    external controller wrote
-------------------------------------------------*/

static READ8_HANDLER( from_controller_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;

	/* clear the pending flag and return the data */
	player->fromcontroller_pending = FALSE;
	return player->fromcontroller;
}


/*-------------------------------------------------
    to_controller_w - write a value back to the
    external controller
-------------------------------------------------*/

static WRITE8_HANDLER( to_controller_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;

	/* set the pending flag and stash the data */
	player->tocontroller_pending = TRUE;
	player->tocontroller = data;

	/* signal to the callback if provided */
	if (player->data_ready_cb != NULL)
		(*player->data_ready_cb)(ld->device, TRUE);

	/* also boost interleave for 4 scanlines to ensure proper communications */
	cpuexec_boost_interleave(ld->device->machine, attotime_zero, attotime_mul(video_screen_get_scan_period(ld->screen), 4));
}


/*-------------------------------------------------
    port1_r - read the 8048 I/O port 1
-------------------------------------------------*/

static READ8_HANDLER( port1_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;
	UINT8 result = 0x00;

	/*
        $80 = P17 = (in) unsure
        $40 = P16 = (in) /ERP from datic circuit
        $20 = P15 = (in) D105
    */

	if (!player->daticerp)
		result |= 0x40;

	return result;
}


/*-------------------------------------------------
    port1_w - write the 8048 I/O port 1
-------------------------------------------------*/

static WRITE8_HANDLER( port1_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;

	/*
        $10 = P14 = (out) D104 -> /SPEED
        $08 = P13 = (out) D103 -> /TIMER ENABLE
        $04 = P12 = (out) D102 -> /REV
        $02 = P11 = (out) D101 -> /FORW
        $01 = P10 = (out) D100 -> some op-amp then to C334, B56, B332
    */

	if (LOG_PORTS && (player->port1 ^ data) & 0x1f)
	{
		printf("%03X:port1:", cpu_get_pc(space->cpu));
		if (!(data & 0x10)) printf(" SPEED");
		if (!(data & 0x08)) printf(" TIMENABLE");
		if (!(data & 0x04)) printf(" REV");
		if (!(data & 0x02)) printf(" FORW");
		if (!(data & 0x01)) printf(" OPAMP");
		printf("\n");
	}

	/* if bit 0 is set, we are not tracking */
	if (data & 0x01)
		player->trackdir = 0;

	/* if bit 0 is clear and we weren't tracking before, initialize the state */
	else if (player->trackdir == 0)
	{
		player->advanced = 0;

		/* if bit 2 is clear, we are moving backwards */
		if (!(data & 0x04))
		{
			player->trackdir = -1;
			player->trackstate = 1;
		}

		/* if bit 1 is clear, we are moving forward */
		else if (!(data & 0x02))
		{
			player->trackdir = 1;
			player->trackstate = 0;
		}
	}

	/* if we have a timer, adjust it */
	if (player->tracktimer != NULL)
	{
		/* turn it off if we're not tracking */
		if (player->trackdir == 0)
			timer_device_adjust_periodic(player->tracktimer, attotime_never, 0, attotime_never);

		/* if we just started tracking, or if the speed was changed, reprime the timer */
		else if (((player->port1 ^ data) & 0x11) != 0)
		{
			/* speeds here are just guesses, but work with the player logic; this is the time per half-track */
			attotime speed = (data & 0x10) ? ATTOTIME_IN_USEC(60) : ATTOTIME_IN_USEC(10);

			/* always start with an initial long delay; the code expects this */
			timer_device_adjust_periodic(player->tracktimer, ATTOTIME_IN_USEC(100), 0, speed);
		}
	}

	player->port1 = data;
}


/*-------------------------------------------------
    port2_r - read from the 8048 I/O port 2
-------------------------------------------------*/

static READ8_HANDLER( port2_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;
	UINT8 result = 0x00;

	/*
        $80 = P27 = (in) set/reset latch; set by FOC LS, reset by IGR
        $20 = P25 = (in) D125 -> 0 when data written to controller is preset, reset to 1 when read
        $10 = P24 = (in) D124 -> 0 when data from controller is present, reset to 1 on a read
    */

	if (!player->tocontroller_pending)
		result |= 0x20;
	if (!player->fromcontroller_pending)
		result |= 0x10;

	return result;
}


/*-------------------------------------------------
    port2_w - write the 8048 I/O port 2
-------------------------------------------------*/

static WRITE8_HANDLER( port2_w )
{
	/*
        $40 = P26 = (out) cleared while data is sent back & forth; set afterwards
                    [Not actually connected, but this is done in the code]
    */
}


/*-------------------------------------------------
    t0_r - return the T0 line status, which is
    connected to the DATIC's data strobe line
-------------------------------------------------*/

static READ8_HANDLER( t0_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	return ld->player->datastrobe;
}


/*-------------------------------------------------
    t1_r - return the T1 line status, which is
    connected to the tracking state and is used
    to count the number of tracks advanced
-------------------------------------------------*/

static READ8_HANDLER( t1_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	return ld->player->trackstate;
}
