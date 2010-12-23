/*

-Galaxy Force
-Run Away
--------------------------
Dai San Wakusei Meteor
(c)1979 Sun Electronics
SIV-01-B

TVG_13.6     [8e8b40b1]
TVG_14.7     [d48cbabe]
TVG_15.8     [cf44bd60]
TVG_16.9     [ae723f56]
--------------------------
-Warp 1


Dumped by Chack'n
01/04/2009

Written by Hau
02/18/2009
12/14/2010

Discrete by Andy
11/11/2009


Driver Notes:

- Two player games are automatically displayed in cocktail mode.
  Is this by design (a cocktail only romset)?

- Discrete audio needs adding to replace hardcoded samples

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/samples.h"
#include "machine/rescap.h"
#include "sound/sn76477.h"


class dai3wksi_state : public driver_device
{
public:
	dai3wksi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* video */
	UINT8 *     dai3wksi_videoram;
	size_t      dai3wksi_videoram_size;
	int         dai3wksi_flipscreen;
	int         dai3wksi_redscreen;
	int         dai3wksi_redterop;

	/* sound */
	UINT8       port_last1;
	UINT8       port_last2;
	int         enabled_sound;
	int         sound3_counter;
};


/*************************************
 *
 *  Video system
 *
 *************************************/


static const UINT8 vr_prom1[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};

static const UINT8 vr_prom2[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};


static void dai3wksi_get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i <= 7; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
	}
}


static VIDEO_UPDATE( dai3wksi )
{
	dai3wksi_state *state = screen->machine->driver_data<dai3wksi_state>();
	offs_t offs;
	pen_t pens[8];

	dai3wksi_get_pens(pens);

	for (offs = 0; offs < state->dai3wksi_videoram_size; offs++)
	{
		offs_t i;

		UINT8 x = offs << 2;
		UINT8 y = offs >> 6;
		UINT8 data = state->dai3wksi_videoram[offs];
		UINT8 color;
		int value = (x >> 2) + ((y >> 5) << 6) + 64 * 8 * (state->dai3wksi_redterop ? 1 : 0);

		if (state->dai3wksi_redscreen)
		{
			color = 0x02;
		} else {
			if (input_port_read(screen->machine, "IN2") & 0x03)
				color = vr_prom2[value];
			else
				color = vr_prom1[value];
		}

		for (i = 0; i <= 3; i++)
		{
			pen_t pen = (data & (1 << i)) ? pens[color] : pens[0];

			if (state->dai3wksi_flipscreen)
				*BITMAP_ADDR32(bitmap, 255-y, 255-x) = pen;
			else
				*BITMAP_ADDR32(bitmap, y, x) = pen;

			x++;
		}
	}

	return 0;
}


/*************************************
 *
 *  Audio system
 *
 *************************************/

#define SAMPLE_SOUND1		0
#define SAMPLE_SOUND2		1
#define SAMPLE_SOUND3_1		2
#define SAMPLE_SOUND3_2		3
#define SAMPLE_SOUND4		4
#define SAMPLE_SOUND5		5
#define SAMPLE_SOUND6_1		6
#define SAMPLE_SOUND6_2		7

#define CHANNEL_SOUND1		0
#define CHANNEL_SOUND2		1
#define CHANNEL_SOUND3		2
#define CHANNEL_SOUND4		3
#define CHANNEL_SOUND5		4
#define CHANNEL_SOUND6		5


static WRITE8_HANDLER( dai3wksi_audio_1_w )
{
	dai3wksi_state *state = space->machine->driver_data<dai3wksi_state>();
	running_device *samples = space->machine->device("samples");
	UINT8 rising_bits = data & ~state->port_last1;

	state->enabled_sound = data & 0x80;

	if ((rising_bits & 0x20) && state->enabled_sound)
	{
		if (data & 0x04)
			sample_start(samples, CHANNEL_SOUND5, SAMPLE_SOUND5, 0);
		else
			sample_start(samples, CHANNEL_SOUND5, SAMPLE_SOUND5, 1);
	}
	if (!(data & 0x20) && (state->port_last1 & 0x20))
		sample_stop(samples, CHANNEL_SOUND5);

	state->port_last1 = data;
}

static WRITE8_HANDLER( dai3wksi_audio_2_w )
{
	dai3wksi_state *state = space->machine->driver_data<dai3wksi_state>();
	running_device *samples = space->machine->device("samples");
	UINT8 rising_bits = data & ~state->port_last2;

	state->dai3wksi_flipscreen = data & 0x10;
	state->dai3wksi_redscreen  = ~data & 0x20;
	state->dai3wksi_redterop   = data & 0x40;

	if (state->enabled_sound)
	{
		if (rising_bits & 0x01) sample_start(samples, CHANNEL_SOUND1, SAMPLE_SOUND1, 0);
		if (rising_bits & 0x02) sample_start(samples, CHANNEL_SOUND2, SAMPLE_SOUND2, 0);
		if (rising_bits & 0x08) sample_start(samples, CHANNEL_SOUND4, SAMPLE_SOUND4, 0);
		if (rising_bits & 0x04)
		{
			if (!state->sound3_counter)
				sample_start(samples, CHANNEL_SOUND3, SAMPLE_SOUND3_1, 0);
			else
				sample_start(samples, CHANNEL_SOUND3, SAMPLE_SOUND3_2, 0);

			state->sound3_counter ^= 1;
		}
	}

	state->port_last2 = data;
}

static WRITE8_HANDLER( dai3wksi_audio_3_w )
{
	dai3wksi_state *state = space->machine->driver_data<dai3wksi_state>();
	running_device *samples = space->machine->device("samples");

	if (state->enabled_sound)
	{
		if (data & 0x40)
			sample_start(samples, CHANNEL_SOUND6, SAMPLE_SOUND6_1, 0);
		else if (data & 0x80)
			sample_start(samples, CHANNEL_SOUND6, SAMPLE_SOUND6_2, 0);
	}
}


static const char *const dai3wksi_sample_names[] =
{
	"*dai3wksi",
	"1.wav",
	"2.wav",
	"3.wav",
	"3-2.wav",
	"4.wav",
	"5.wav",
	"6.wav",
	"6-2.wav",
	0
};


static const samples_interface dai3wksi_samples_interface =
{
	6,	/* 6 channels */
	dai3wksi_sample_names
};


static MACHINE_CONFIG_FRAGMENT( dai3wksi_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(dai3wksi_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1bff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0x24ff) AM_MIRROR(0x100) AM_READ_PORT("IN0")
	AM_RANGE(0x2800, 0x28ff) AM_MIRROR(0x100) AM_READ_PORT("IN1")
	AM_RANGE(0x3000, 0x3000) AM_WRITE(dai3wksi_audio_1_w)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(dai3wksi_audio_2_w)
	AM_RANGE(0x3800, 0x3800) AM_WRITE(dai3wksi_audio_3_w)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_BASE_SIZE_MEMBER(dai3wksi_state, dai3wksi_videoram, dai3wksi_videoram_size)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( dai3wksi )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x10, 0x00, "DIPSW #7" )						PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW #8" )						PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	/* dummy port (colormap) */
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW #1" )						PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW #2" )						PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_START( dai3wksi )
{
	dai3wksi_state *state = machine->driver_data<dai3wksi_state>();

	state_save_register_global(machine, state->port_last1);
	state_save_register_global(machine, state->port_last2);
	state_save_register_global(machine, state->enabled_sound);
	state_save_register_global(machine, state->sound3_counter);
}

static MACHINE_RESET( dai3wksi )
{
	dai3wksi_state *state = machine->driver_data<dai3wksi_state>();

	state->port_last1 = 0;
	state->port_last2 = 0;
	state->enabled_sound = 0;
	state->sound3_counter = 0;
}


static MACHINE_CONFIG_START( dai3wksi, dai3wksi_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_10MHz/4 )	/* Confirmed on PCB */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(dai3wksi)
	MDRV_MACHINE_RESET(dai3wksi)

	/* video hardware */
	MDRV_VIDEO_UPDATE(dai3wksi)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(4, 251, 8, 247)
	MDRV_SCREEN_REFRESH_RATE(60)

	/* audio hardware */
	MDRV_FRAGMENT_ADD(dai3wksi_audio)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( dai3wksi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg_13.6",  0x0000, 0x0800, CRC(8e8b40b1) SHA1(25b9223486dd348ea302e8e8f1d47c804a88b142) )
	ROM_LOAD( "tvg_14.7",  0x0800, 0x0800, CRC(d48cbabe) SHA1(64b571cd778fc7d67a5fa998a0defd36c04f111f) )
	ROM_LOAD( "tvg_15.8",  0x1000, 0x0800, CRC(cf44bd60) SHA1(61e0b3f9c4a1f9da1de57fb8276d4fc9e43b8f24) )
	ROM_LOAD( "tvg_16.9",  0x1800, 0x0400, CRC(ae723f56) SHA1(c25c27d6144533b2b2a888bfa8dbf48ed8d8b09a) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME(1979, dai3wksi, 0,        dai3wksi, dai3wksi, 0, ROT270, "Sun Electronics", "Dai San Wakusei Meteor", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
