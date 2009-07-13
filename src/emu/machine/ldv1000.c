/*************************************************************************

    ldv1000.c

    Pioneer LD-V1000 laserdisc emulation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************

    Still to do:

        * fix issues
        * add OSD

*************************************************************************/

#include "ldcore.h"
#include "machine/8255ppi.h"
#include "machine/z80ctc.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_PORT_IO					0
#define LOG_STATUS_CHANGES			0
#define LOG_FRAMES_SEEN				0
#define LOG_COMMANDS				0



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SCAN_SPEED						(2000 / 30)			/* 2000 frames/second */
#define SEEK_FAST_SPEED					(4000 / 30)			/* 4000 frames/second */

#define MULTIJUMP_TRACK_TIME			ATTOTIME_IN_USEC(50)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* player-specific data */
struct _ldplayer_data
{
	/* low-level emulation data */
	const device_config *cpu;					/* CPU index of the Z80 */
	const device_config *ctc;					/* CTC device */
	const device_config *multitimer;			/* multi-jump timer device */

	/* communication status */
	UINT8				command;				/* command byte to the player */
	UINT8				status;					/* status byte from the player */
	UINT8				vsync;					/* VSYNC state */

	/* I/O port states */
	UINT8				counter_start;			/* starting value for counter */
	UINT8 				counter;				/* current counter value */
	UINT8				portc0;					/* port C on PPI 0 */
	UINT8				portb1;					/* port B on PPI 1 */
	UINT8				portc1;					/* port C on PPI 1 */

	/* display/decode circuit emulation */
	UINT8				portselect;				/* selection of which port to access */
	UINT8				display[2][20];			/* display lines */
	UINT8				dispindex;				/* index within the display line */
	UINT8				vbi[7*3];				/* VBI data */
	UINT8				vbiready;				/* VBI ready flag */
	UINT8				vbiindex;				/* index within the VBI data */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ldv1000_init(laserdisc_state *ld);
static void ldv1000_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static INT32 ldv1000_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime);
static void ldv1000_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data);
static UINT8 ldv1000_status_strobe_r(laserdisc_state *ld);
static UINT8 ldv1000_command_strobe_r(laserdisc_state *ld);
static UINT8 ldv1000_status_r(laserdisc_state *ld);

static TIMER_CALLBACK( vsync_off );
static TIMER_CALLBACK( vbi_data_fetch );
static TIMER_DEVICE_CALLBACK( multijump_timer );

static void ctc_interrupt(const device_config *device, int state);

static WRITE8_HANDLER( decoder_display_port_w );
static READ8_HANDLER( decoder_display_port_r );
static READ8_HANDLER( controller_r );
static WRITE8_HANDLER( controller_w );
static WRITE8_DEVICE_HANDLER( ppi0_porta_w );
static READ8_DEVICE_HANDLER( ppi0_portb_r );
static READ8_DEVICE_HANDLER( ppi0_portc_r );
static WRITE8_DEVICE_HANDLER( ppi0_portc_w );
static READ8_DEVICE_HANDLER( ppi1_porta_r );
static WRITE8_DEVICE_HANDLER( ppi1_portb_w );
static WRITE8_DEVICE_HANDLER( ppi1_portc_w );



/***************************************************************************
    LD-V1000 ROM AND MACHINE INTERFACES
***************************************************************************/

static ADDRESS_MAP_START( ldv1000_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x6000) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x9ff0) AM_DEVREADWRITE("ldvppi0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x9ff0) AM_DEVREADWRITE("ldvppi1", ppi8255_r, ppi8255_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ldv1000_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x38) AM_READWRITE(decoder_display_port_r, decoder_display_port_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(controller_r)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3f) AM_WRITE(controller_w)
	AM_RANGE(0xc0, 0xc3) AM_MIRROR(0x3c) AM_DEVREADWRITE("ldvctc", z80ctc_r, z80ctc_w)
ADDRESS_MAP_END


static const ppi8255_interface ppi0intf =
{
	DEVCB_NULL,						DEVCB_HANDLER(ppi0_portb_r),	DEVCB_HANDLER(ppi0_portc_r),
	DEVCB_HANDLER(ppi0_porta_w),	DEVCB_NULL,						DEVCB_HANDLER(ppi0_portc_w)
};


static const ppi8255_interface ppi1intf =
{
	DEVCB_HANDLER(ppi1_porta_r),	DEVCB_NULL,						DEVCB_NULL,
	DEVCB_NULL,						DEVCB_HANDLER(ppi1_portb_w),	DEVCB_HANDLER(ppi1_portc_w)
};


static const z80ctc_interface ctcintf =
{
	0,
	ctc_interrupt
};


static const z80_daisy_chain daisy_chain[] =
{
	{ "ldvctc" },
	{ NULL }
};


static MACHINE_DRIVER_START( ldv1000 )
	MDRV_CPU_ADD("ldv1000", Z80, XTAL_5MHz/2)
	MDRV_CPU_CONFIG(daisy_chain)
	MDRV_CPU_PROGRAM_MAP(ldv1000_map)
	MDRV_CPU_IO_MAP(ldv1000_portmap)

	MDRV_Z80CTC_ADD("ldvctc", XTAL_5MHz/2 /* same as "ldv1000" */, ctcintf)
	MDRV_PPI8255_ADD("ldvppi0", ppi0intf)
	MDRV_PPI8255_ADD("ldvppi1", ppi1intf)
	MDRV_TIMER_ADD("multitimer", multijump_timer)
MACHINE_DRIVER_END


ROM_START( ldv1000 )
	ROM_REGION( 0x2000, "ldv1000", ROMREGION_LOADBYNAME )
	ROM_LOAD( "z03_1001_vyw-053_v1-0.bin", 0x0000, 0x2000, CRC(31ec4687) SHA1(52f91c304a878ba02b2fa1cda1a9489d6dd5a34f) )
ROM_END



/***************************************************************************
    INTERFACES
***************************************************************************/

const ldplayer_interface ldv1000_interface =
{
	LASERDISC_TYPE_PIONEER_LDV1000,				/* type of the player */
	sizeof(ldplayer_data),						/* size of the state */
	"Pioneer LD-V1000",							/* name of the player */
	ROM_NAME(ldv1000),							/* pointer to ROM region information */
	MACHINE_DRIVER_NAME(ldv1000),				/* pointer to machine configuration */
	ldv1000_init,								/* initialization callback */
	ldv1000_vsync,								/* vsync callback */
	ldv1000_update,								/* update callback */
	NULL,										/* overlay callback */
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
    PIONEER LD-V1000 IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ldv1000_init - player-specific initialization
-------------------------------------------------*/

static void ldv1000_init(laserdisc_state *ld)
{
	astring *tempstring = astring_alloc();
	ldplayer_data *player = ld->player;

	/* reset our state */
	memset(player, 0, sizeof(*player));

	/* find our devices */
	player->cpu = cputag_get_cpu(ld->device->machine, device_build_tag(tempstring, ld->device, "ldv1000"));
	player->ctc = devtag_get_device(ld->device->machine, device_build_tag(tempstring, ld->device, "ldvctc"));
	player->multitimer = devtag_get_device(ld->device->machine, device_build_tag(tempstring, ld->device, "multitimer"));
	timer_device_set_ptr(player->multitimer, ld);
	astring_free(tempstring);
}


/*-------------------------------------------------
    ldv1000_vsync - VSYNC callback, called at the
    start of the blanking period
-------------------------------------------------*/

static void ldv1000_vsync(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	slider_position sliderpos = ldcore_get_slider_position(ld);
	ldplayer_data *player = ld->player;

	/* generate interrupts if we hit the edges */
	z80ctc_trg1_w(player->ctc, 0, sliderpos == SLIDER_MINIMUM);
	z80ctc_trg2_w(player->ctc, 0, sliderpos == SLIDER_MAXIMUM);

	/* signal VSYNC and set a timer to turn it off */
	player->vsync = TRUE;
	timer_set(ld->device->machine, attotime_mul(video_screen_get_scan_period(ld->screen), 4), ld, 0, vsync_off);

	/* also set a timer to fetch the VBI data when it is ready */
	timer_set(ld->device->machine, video_screen_get_time_until_pos(ld->screen, 19*2, 0), ld, 0, vbi_data_fetch);

	/* boost interleave for the first 1ms to improve communications */
	cpuexec_boost_interleave(ld->device->machine, attotime_zero, ATTOTIME_IN_MSEC(1));
}


/*-------------------------------------------------
    ldv1000_update - update callback, called on
    the first visible line of the frame
-------------------------------------------------*/

static INT32 ldv1000_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime)
{
	if (LOG_FRAMES_SEEN)
	{
		int frame = frame_from_metadata(vbi);
		if (frame != FRAME_NOT_PRESENT) printf("== %d\n", frame);
	}
	return fieldnum;
}


/*-------------------------------------------------
    ldv1000_data_w - handle a parallel data write
    to the LD-V1000
-------------------------------------------------*/

static void ldv1000_data_w(laserdisc_state *ld, UINT8 prev, UINT8 data)
{
	ld->player->command = data;
	if (LOG_COMMANDS)
		printf("-> COMMAND = %02X (%s)\n", data, (ld->player->portc1 & 0x10) ? "valid" : "invalid");
}


/*-------------------------------------------------
    ldv1000_status_strobe_r - return state of the
    status strobe
-------------------------------------------------*/

static UINT8 ldv1000_status_strobe_r(laserdisc_state *ld)
{
	return (ld->player->portc1 & 0x20) ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    ldv1000_command_strobe_r - return state of the
    command strobe
-------------------------------------------------*/

static UINT8 ldv1000_command_strobe_r(laserdisc_state *ld)
{
	return (ld->player->portc1 & 0x10) ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    ldv1000_status_r - handle a parallel data read
    from the LD-V1000
-------------------------------------------------*/

static UINT8 ldv1000_status_r(laserdisc_state *ld)
{
	return ld->player->status;
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
	UINT8 focus_on = !(player->portb1 & 0x01);
	UINT8 laser_on = (player->portb1 & 0x40);
	UINT32 lines[3];

	/* appears to return data in reverse order */
	lines[0] = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE1718, FALSE);
	lines[1] = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE17, FALSE);
	lines[2] = laserdisc_get_field_code(ld->device, LASERDISC_CODE_LINE16, FALSE);

	/* fill in the details */
	memset(player->vbi, 0, sizeof(player->vbi));
	if (focus_on && laser_on)
	{
		int line;

		/* loop over lines */
		for (line = 0; line < 3; line++)
		{
			UINT8 *dest = &player->vbi[line * 7];
			UINT32 data = lines[line];

			/* the logic only processes leadin/leadout/frame number codes */
			if (data == VBI_CODE_LEADIN || data == VBI_CODE_LEADOUT || (data & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
			{
				*dest++ = 0x09 | (((data & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE) ? 0x02 : 0x00);
				*dest++ = 0x08;
				*dest++ = (data >> 16) & 0x0f;
				*dest++ = (data >> 12) & 0x0f;
				*dest++ = (data >>  8) & 0x0f;
				*dest++ = (data >>  4) & 0x0f;
				*dest++ = (data >>  0) & 0x0f;
			}
		}
	}

	/* signal that data is ready and reset the readback index */
	player->vbiready = TRUE;
	player->vbiindex = 0;
}


/*-------------------------------------------------
    multijump_timer - called once for each track
    in a multijump sequence; we decrement the
    down counter and autostop when we hit 0
-------------------------------------------------*/

static TIMER_DEVICE_CALLBACK( multijump_timer )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldplayer_data *player = ld->player;
	int direction;

	/* bit 5 of port B on PPI 1 selects the direction of slider movement */
	direction = (player->portb1 & 0x20) ? 1 : -1;
	ldcore_advance_slider(ld, direction);

	/* update down counter and reschedule */
	if (--player->counter != 0)
		timer_device_adjust_oneshot(timer, MULTIJUMP_TRACK_TIME, 0);
}


/*-------------------------------------------------
    ctc_interrupt - called when the CTC triggers
    an interrupt in the daisy chain
-------------------------------------------------*/

static void ctc_interrupt(const device_config *device, int state)
{
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	cpu_set_input_line(ld->player->cpu, 0, state ? ASSERT_LINE : CLEAR_LINE);
}


/*-------------------------------------------------
    decoder_display_port_w - handle writes to the
    decoder/display chips
-------------------------------------------------*/

static WRITE8_HANDLER( decoder_display_port_w )
{
	/*
        TX/RX = /A0 (A0=0 -> TX, A0=1 -> RX)

        Display is 6-bit
        Decoder is 4-bit
    */
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;

	/* writes to offset 0 select the target for reads/writes of actual data */
	if (offset == 0)
	{
		player->portselect = data;
		player->dispindex = 0;
	}

	/* writes to offset 2 constitute actual writes targeted toward the display and decoder chips */
	else if (offset == 2)
	{
		/* selections 0 and 1 represent the two display lines; only 6 bits are transferred */
		if (player->portselect < 2)
			player->display[player->portselect][player->dispindex++ % 20] = data & 0x3f;
	}
}


/*-------------------------------------------------
    decoder_display_port_r - handle reads from the
    decoder/display chips
-------------------------------------------------*/

static READ8_HANDLER( decoder_display_port_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	ldplayer_data *player = ld->player;
	UINT8 result = 0;

	/* reads from offset 3 constitute actual reads from the display and decoder chips */
	if (offset == 3)
	{
		/* selection 4 represents the VBI data reading */
		if (player->portselect == 4)
		{
			player->vbiready = FALSE;
			result = player->vbi[player->vbiindex++ % ARRAY_LENGTH(player->vbi)];
		}
	}
	return result;
}


/*-------------------------------------------------
    controller_r - handle read of the data from
    the controlling system
-------------------------------------------------*/

static READ8_HANDLER( controller_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);

	/* note that this is a cheesy implementation; the real thing relies on exquisite timing */
	UINT8 result = ld->player->command ^ 0xff;
	ld->player->command = 0xff;
	return result;
}


/*-------------------------------------------------
    controller_w - handle status latch writes
-------------------------------------------------*/

static WRITE8_HANDLER( controller_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(space->cpu->owner);
	if (LOG_STATUS_CHANGES && data != ld->player->status)
		printf("%04X:CONTROLLER.W=%02X\n", cpu_get_pc(space->cpu), data);
	ld->player->status = data;
}


/*-------------------------------------------------
    ppi0_porta_w - handle writes to port A of
    PPI #0
-------------------------------------------------*/

static WRITE8_DEVICE_HANDLER( ppi0_porta_w )
{
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	ld->player->counter_start = data;
	if (LOG_PORT_IO)
		printf("%s:PORTA.0=%02X\n", cpuexec_describe_context(device->machine), data);
}


/*-------------------------------------------------
    ppi0_portb_r - handle reads from port B of
    PPI #0
-------------------------------------------------*/

static READ8_DEVICE_HANDLER( ppi0_portb_r )
{
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	return ld->player->counter;
}


/*-------------------------------------------------
    ppi0_portc_r - handle reads from port C of
    PPI #0
-------------------------------------------------*/

static READ8_DEVICE_HANDLER( ppi0_portc_r )
{
	/*
        $10 = /VSYNC
        $20 = IRQ from decoder chip
        $40 = TRKG LOOP (N24-1)
        $80 = DUMP (N20-1) -- code reads the state and waits for it to change
    */
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	ldplayer_data *player = ld->player;
	UINT8 result = 0x00;

	if (!player->vsync)
		result |= 0x10;
	if (!player->vbiready)
		result |= 0x20;

	return result;
}


/*-------------------------------------------------
    ppi0_portc_w - handle writes to port C of
    PPI #0
-------------------------------------------------*/

static WRITE8_DEVICE_HANDLER( ppi0_portc_w )
{
	/*
        $01 = preload on up/down counters
        $02 = /MULTI JUMP TRIG
        $04 = SCAN MODE
        $08 = n/c
    */
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	ldplayer_data *player = ld->player;
	UINT8 prev = player->portc0;

	/* set the new value */
	player->portc0 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0x0f) != 0)
	{
		printf("%s:PORTC.0=%02X", cpuexec_describe_context(device->machine), data);
		if (data & 0x01) printf(" PRELOAD");
		if (!(data & 0x02)) printf(" /MULTIJUMP");
		if (data & 0x04) printf(" SCANMODE");
		printf("\n");
	}

	/* on the rising edge of bit 0, clock the down counter load */
	if ((data & 0x01) && !(prev & 0x01))
		player->counter = player->counter_start;

	/* on the falling edge of bit 1, start the multi-jump timer */
	if (!(data & 0x02) && (prev & 0x02))
		timer_device_adjust_oneshot(player->multitimer, MULTIJUMP_TRACK_TIME, 0);
}


/*-------------------------------------------------
    ppi1_porta_r - handle reads from port A of
    PPI #1
-------------------------------------------------*/

static READ8_DEVICE_HANDLER( ppi1_porta_r )
{
	/*
        $01 = /FOCS LOCK
        $02 = /SPDL LOCK
        $04 = INSIDE
        $08 = OUTSIDE
        $10 = MOTOR STOP
        $20 = +5V/test point
        $40 = /INT LOCK
        $80 = 8 INCH CHK
    */
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	ldplayer_data *player = ld->player;
	slider_position sliderpos = ldcore_get_slider_position(ld);
	UINT8 focus_on = !(player->portb1 & 0x01);
	UINT8 spdl_on = !(player->portb1 & 0x02);
	UINT8 result = 0x00;

	/* bit 0: /FOCUS LOCK */
	if (!focus_on)
		result |= 0x01;

	/* bit 1: /SPDL LOCK */
	if (!spdl_on)
		result |= 0x02;

	/* bit 2: INSIDE signal */
	if (sliderpos == SLIDER_MINIMUM)
		result |= 0x04;

	/* bit 3: OUTSIDE signal */
	if (sliderpos == SLIDER_MAXIMUM)
		result |= 0x08;

	/* bit 4: MOTOR STOP */

	/* bit 5: +5V/test point */
	result |= 0x20;

	/* bit 6: /INT LOCK */

	/* bit 7: 8 INCH CHK */

	return result;
}


/*-------------------------------------------------
    ppi1_portb_w - handle writes to port B of
    PPI #1
-------------------------------------------------*/

static WRITE8_DEVICE_HANDLER( ppi1_portb_w )
{
	/*
        $01 = /FOCS ON
        $02 = /SPDL RUN
        $04 = /JUMP TRIG
        $08 = /SCAN A
        $10 = SCAN B
        $20 = SCAN C
        $40 = /LASER ON
        $80 = /SYNC ST0
    */
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	ldplayer_data *player = ld->player;
	UINT8 prev = player->portb1;
	int direction;

	/* set the new value */
	player->portb1 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0xff) != 0)
	{
		printf("%s:PORTB.1=%02X:", cpuexec_describe_context(device->machine), data);
		if (!(data & 0x01)) printf(" FOCSON");
		if (!(data & 0x02)) printf(" SPDLRUN");
		if (!(data & 0x04)) printf(" JUMPTRIG");
		if (!(data & 0x08)) printf(" SCANA (%c %c)", (data & 0x10) ? 'L' : 'H', (data & 0x20) ? 'F' : 'R');
		if ( (data & 0x40)) printf(" LASERON");
		if (!(data & 0x80)) printf(" SYNCST0");
		printf("\n");
	}

	/* bit 5 selects the direction of slider movement for JUMP TRG and scanning */
	direction = (data & 0x20) ? 1 : -1;

	/* on the falling edge of bit 2, jump one track in either direction */
	if (!(data & 0x04) && (prev & 0x04))
		ldcore_advance_slider(ld, direction);

	/* bit 3 low enables scanning */
	if (!(data & 0x08))
	{
		/* bit 4 selects the speed */
		int delta = (data & 0x10) ? SCAN_SPEED : SEEK_FAST_SPEED;
		ldcore_set_slider_speed(ld, delta * direction);
	}

	/* bit 3 high stops scanning */
	else
		ldcore_set_slider_speed(ld, 0);
}


/*-------------------------------------------------
    ppi1_portc_w - handle writes to port C of
    PPI #1
-------------------------------------------------*/

static WRITE8_DEVICE_HANDLER( ppi1_portc_w )
{
	/*
        $01 = AUD 1
        $02 = AUD 2
        $04 = AUDIO ENABLE
        $08 = /VIDEO SQ
        $10 = COMMAND
        $20 = STATUS
        $40 = SIZE 8/12
        $80 = /LED CAV
    */
	laserdisc_state *ld = ldcore_get_safe_token(device->owner);
	ldplayer_data *player = ld->player;
	UINT8 prev = player->portc1;

	/* set the new value */
	player->portc1 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0xcf) != 0)
	{
		printf("%s:PORTC.1=%02X", cpuexec_describe_context(device->machine), data);
		if (data & 0x01) printf(" AUD1");
		if (data & 0x02) printf(" AUD2");
		if (data & 0x04) printf(" AUDEN");
		if (!(data & 0x08)) printf(" VIDEOSQ");
		if (data & 0x10) printf(" COMMAND");
		if (data & 0x20) printf(" STATUS");
		if (data & 0x40) printf(" SIZE8");
		if (!(data & 0x80)) printf(" CAV");
		printf("\n");
	}

	/* video squelch is controlled by bit 3 */
	ldcore_set_video_squelch(ld, (data & 0x08) == 0);

	/* audio squelch is controlled by bits 0-2 */
	ldcore_set_audio_squelch(ld, !(data & 0x04) || !(data & 0x01), !(data & 0x04) || !(data & 0x02));
}
