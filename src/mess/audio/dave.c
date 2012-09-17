/**********************************************************************

    "Dave" Sound Chip

    DAVE SOUND CHIP FOUND IN ENTERPRISE

     working:

    - pure tone
    - sampled sounds
    - 1 kHz, 50 Hz and 1 Hz ints
    - external ints (int1 and int2) - not correct speed yet

**********************************************************************/

#include "emu.h"
#include "audio/dave.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define STEP 0x08000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct dave_t
{
	devcb_resolved_read8 reg_r;
	devcb_resolved_write8 reg_w;
	devcb_resolved_write_line int_callback;

	unsigned char Regs[32];

	/* int latches (used by 1Hz, int1 and int2) */
	unsigned long int_latch;
	/* int enables */
	unsigned long int_enable;
	/* int inputs */
	unsigned long int_input;

	unsigned long int_irq;

	/* INTERRUPTS */

	/* internal timer */
	/* bit 2: 1kHz timer irq */
	/* bit 1: 50kHz timer irq */
	int timer_irq;
	/* 1khz timer - divided into 1kHz, 50Hz and 1Hz timer */
	emu_timer	*int_timer;
	/* state of 1kHz timer */
	unsigned long one_khz_state;
	/* state of 50Hz timer */
	unsigned long fifty_hz_state;

	/* counter used to trigger 50Hz from 1kHz timer */
	unsigned long fifty_hz_count;
	/* counter used to trigger 1Hz from 1kHz timer */
	unsigned long one_hz_count;


	/* SOUND SYNTHESIS */
	int Period[4];
	int Count[4];
	int	level[4];

	/* these are used to force channels on/off */
	/* if one of the or values is 0x0ff, this means
    the volume will be forced on,else it is dependant on
    the state of the wave */
	int level_or[8];
	/* if one of the values is 0x00, this means the
    volume is forced off, else it is dependant on the wave */
	int level_and[8];

	/* these are the current channel volumes in MAME form */
	int mame_volumes[8];

	/* update step */
	int UpdateStep;

	sound_stream *sound_stream_var;

	/* temp here */
	int nick_virq;
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE dave_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DAVE);
	return (dave_t *) downcast<dave_sound_device *>(device)->token();
}



INLINE const dave_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DAVE);
	return (const dave_interface *) device->static_config();
}


/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void dave_set_external_int_state(device_t *device, int IntID, int State);
static TIMER_CALLBACK(dave_1khz_callback);
static STREAM_UPDATE(dave_update_sound);


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START( dave_sound )
-------------------------------------------------*/

static DEVICE_START( dave_sound )
{
	int i;
	dave_t *dave = get_token(device);
	const dave_interface *intf = get_interface(device);

	memset(dave, 0, sizeof(*dave));

	/* resolve callbacks */
	dave->reg_r.resolve(intf->reg_r, *device);
	dave->reg_w.resolve(intf->reg_w, *device);
	dave->int_callback.resolve(intf->int_callback, *device);

	/* temp! */
	dave->nick_virq = 0;

	/* initialise 1kHz timer */
	dave->int_latch = 0;
	dave->int_input = 0;
	dave->int_enable = 0;
	dave->timer_irq = 0;
	dave->fifty_hz_state = 0;
	dave->one_khz_state = 0;
	dave->fifty_hz_count = DAVE_FIFTY_HZ_COUNTER_RELOAD;
	dave->one_hz_count = DAVE_ONE_HZ_COUNTER_RELOAD;
	device->machine().scheduler().timer_pulse(attotime::from_hz(1000), FUNC(dave_1khz_callback), 0, (void *) device);

	for (i=0; i<3; i++)
	{
		dave->Period[i] = (STEP * device->machine().sample_rate()) / 125000;
		dave->Count[i] = (STEP * device->machine().sample_rate()) / 125000;
		dave->level[i] = 0;
	}

	/* dave has 3 tone channels and 1 noise channel.
    the volumes are mixed internally and output as left and right volume */

	/* 3 tone channels + 1 noise channel */
	dave->sound_stream_var = device->machine().sound().stream_alloc(*device, 0, 2, device->machine().sample_rate(), NULL, dave_update_sound);
}


/*-------------------------------------------------
    DEVICE_RESET( dave_sound )
-------------------------------------------------*/

static DEVICE_RESET( dave_sound )
{
	dave_t *dave = get_token(device);
	int i;

	for (i = 0; i < 32; i++)
		dave->Regs[i] = 0;

	address_space &space = device->machine().driver_data()->generic_space();
	dave_reg_w(device, space, 0x10, 0);
	dave_reg_w(device, space, 0x11, 0);
	dave_reg_w(device, space, 0x12, 0);
	dave_reg_w(device, space, 0x13, 0);

}


/*-------------------------------------------------
    dave_refresh_ints
-------------------------------------------------*/

static void dave_refresh_ints(device_t *device)
{
	dave_t *dave = get_token(device);
	int int_wanted;

	logerror("int latch: %02x enable: %02x input: %02x\n", (int) dave->int_latch, (int) dave->int_enable, (int) dave->int_input);

	int_wanted = ((dave->int_enable<<1) & dave->int_latch) != 0;
	dave->int_callback(int_wanted);
}


/*-------------------------------------------------
    dave_refresh_selectable_int
-------------------------------------------------*/

static void dave_refresh_selectable_int(device_t *device)
{
	dave_t *dave = get_token(device);

	/* update 1kHz/50Hz/tg latch and int input */
	switch ((dave->Regs[7]>>5) & 0x03)
	{
		/* 1kHz */
		case 0:
		{
			dave->int_latch &=~(1<<1);
			dave->int_latch |= (dave->int_irq>>1) & 0x02;

			/* set int input state */
			dave->int_input &= ~(1<<0);
			dave->int_input |= (dave->one_khz_state & 0x01)<<0;
		}
		break;

		/* 50Hz */
		case 1:
		{
			dave->int_latch &=~(1<<1);
			dave->int_latch |= dave->int_irq & 0x02;

			/* set int input state */
			dave->int_input &= ~(1<<0);
			dave->int_input |= (dave->fifty_hz_state & 0x01)<<0;
		}
		break;


		default:
			break;
	}

	dave_refresh_ints(device);
}


/*-------------------------------------------------
    dave_1khz_callback
-------------------------------------------------*/

static TIMER_CALLBACK(dave_1khz_callback)
{
	device_t *device = (device_t *)ptr;
	dave_t *dave = get_token(device);

	/* time over - want int */
	dave->one_khz_state ^= 0x0ffffffff;

	/* lo-high transition causes int */
	if (dave->one_khz_state!=0)
	{
		dave->int_irq |=(1<<2);
	}


	/* update fifty Hz counter */
	dave->fifty_hz_count--;

	if (dave->fifty_hz_count==0)
	{
		/* these two lines are temp here */
		dave->nick_virq ^= 0x0ffffffff;
		dave_set_external_int_state(device, DAVE_INT1_ID, dave->nick_virq);


		dave->fifty_hz_count = DAVE_FIFTY_HZ_COUNTER_RELOAD;
		dave->fifty_hz_state^=0x0ffffffff;

		if (dave->fifty_hz_state!=0)
		{
			dave->int_irq |= (1<<1);
		}
	}

	dave->one_hz_count--;

	if (dave->one_hz_count==0)
	{
		/* reload counter */
		dave->one_hz_count = DAVE_ONE_HZ_COUNTER_RELOAD;

		/* change state */
		dave->int_input ^= (1<<2);

		if (dave->int_input & (1<<2))
		{
			/* transition from 0->1 */
			/* int requested */
			dave->int_latch |=(1<<3);
		}
	}

	dave_refresh_selectable_int(device);
}


/*-------------------------------------------------
    dave_update_sound
-------------------------------------------------*/

static STREAM_UPDATE( dave_update_sound )
{
	dave_t *dave = get_token(device);
	stream_sample_t *buffer1, *buffer2;
	/* 0 = channel 0 left volume, 1 = channel 0 right volume,
    2 = channel 1 left volume, 3 = channel 1 right volume,
    4 = channel 2 left volume, 5 = channel 2 right volume
    6 = noise channel left volume, 7 = noise channel right volume */
	int output_volumes[8];
	int left_volume;
	int right_volume;

	//logerror("sound update!\n");

	buffer1 = outputs[0];
	buffer2 = outputs[1];

	while (samples)
	{
		int vol[4];
		int i;

		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = 0;

		for (i = 0;i < 3;i++)
		{
			if ((dave->Regs[7] & (1<<i))==0)
			{

				if (dave->level[i]) vol[i] += dave->Count[i];
				dave->Count[i] -= STEP;
				/* Period[i] is the half period of the square wave. Here, in each */
				/* loop I add Period[i] twice, so that at the end of the loop the */
				/* square wave is in the same status (0 or 1) it was at the start. */
				/* vol[i] is also incremented by Period[i], since the wave has been 1 */
				/* exactly half of the time, regardless of the initial position. */
				/* If we exit the loop in the middle, Output[i] has to be inverted */
				/* and vol[i] incremented only if the exit status of the square */
				/* wave is 1. */
				while (dave->Count[i] <= 0)
				{
					dave->Count[i] += dave->Period[i];
					if (dave->Count[i] > 0)
					{
						dave->level[i] ^= 0x0ffffffff;
						if (dave->level[i]) vol[i] += dave->Period[i];
							break;
					}
					dave->Count[i] += dave->Period[i];
					vol[i] += dave->Period[i];
				}
				if (dave->level[i])
					vol[i] -= dave->Count[i];
			}
		}

		/* update volume outputs */

		/* setup output volumes for each channel */
		/* channel 0 */
		output_volumes[0] = ((dave->level[0] & dave->level_and[0]) | dave->level_or[0]) & dave->mame_volumes[0];
		output_volumes[1] = ((dave->level[0] & dave->level_and[1]) | dave->level_or[1]) & dave->mame_volumes[4];
		/* channel 1 */
		output_volumes[2] = ((dave->level[1] & dave->level_and[2]) | dave->level_or[2]) & dave->mame_volumes[1];
		output_volumes[3] = ((dave->level[1] & dave->level_and[3]) | dave->level_or[3]) & dave->mame_volumes[5];
		/* channel 2 */
		output_volumes[4] = ((dave->level[2] & dave->level_and[4]) | dave->level_or[4]) & dave->mame_volumes[2];
		output_volumes[5] = ((dave->level[2] & dave->level_and[5]) | dave->level_or[5]) & dave->mame_volumes[6];
		/* channel 3 */
		output_volumes[6] = ((dave->level[3] & dave->level_and[6]) | dave->level_or[6]) & dave->mame_volumes[3];
		output_volumes[7] = ((dave->level[3] & dave->level_and[7]) | dave->level_or[7]) & dave->mame_volumes[7];

		left_volume = (output_volumes[0] + output_volumes[2] + output_volumes[4] + output_volumes[6])>>2;
		right_volume = (output_volumes[1] + output_volumes[3] + output_volumes[5] + output_volumes[7])>>2;

		*(buffer1++) = left_volume;
		*(buffer2++) = right_volume;

		samples--;
	}
}


/*-------------------------------------------------
    dave_sound_w - used to update sound output
    based on data writes
-------------------------------------------------*/

static WRITE8_DEVICE_HANDLER(dave_sound_w)
{
	dave_t *dave = get_token(device);

	/* update stream */
	dave->sound_stream_var->update();

	/* new write */
	switch (offset)
	{
		/* channel 0 down-counter */
		case 0:
		case 1:
		/* channel 1 down-counter */
		case 2:
		case 3:
		/* channel 2 down-counter */
		case 4:
		case 5:
		{
			int count = 0;
			int channel_index = offset>>1;

			/* Fout = 125,000 / (n+1) Hz */

			/* sample rate/clock */


			/* get down-count */
			switch (offset & 0x01)
			{
				case 0:
				{
					count = (data & 0x0ff) | ((dave->Regs[offset+1] & 0x0f)<<8);
				}
				break;

				case 1:
				{
					count = (dave->Regs[offset-1] & 0x0ff) | ((data & 0x0f)<<8);

				}
				break;
			}

			count++;


			dave->Period[channel_index] = ((STEP  * device->machine().sample_rate())/125000) * count;

		}
		break;

		/* channel 0 left volume */
		case 8:
		/* channel 1 left volume */
		case 9:
		/* channel 2 left volume */
		case 10:
		/* noise channel left volume */
		case 11:
		/* channel 0 right volume */
		case 12:
		/* channel 1 right volume */
		case 13:
		/* channel 2 right volume */
		case 14:
		/* noise channel right volume */
		case 15:
		{
			/* update mame version of volume from data written */
			/* 0x03f->0x07e00. Max is 0x07fff */
			/* I believe the volume is linear - to be checked! */
			dave->mame_volumes[offset-8] = (data & 0x03f)<<9;
		}
		break;

		case 7:
		{
			/*  force => the value of this register is forced regardless of the wave
                    state,
                remove => this value is force to zero so that it has no influence over
                    the final volume calculation, regardless of wave state
                use => the volume value is dependant on the wave state and is included
                    in the final volume calculation */

			logerror("selectable int ");
			switch ((data>>5) & 0x03)
			{
				case 0:
				{
					logerror("1kHz\n");
				}
				break;

				case 1:
				{
					logerror("50Hz\n");
				}
				break;

				case 2:
				{
					logerror("tone channel 0\n");
				}
				break;

				case 3:
				{
					logerror("tone channel 1\n");
				}
				break;
			}



			/* turn L.H audio output into D/A, outputting value in R8 */
			if (data & (1<<3))
			{
				/* force r8 value */
				dave->level_or[0] = 0x0ffff;
				dave->level_and[0] = 0x00;

				/* remove r9 value */
				dave->level_or[2] = 0x000;
				dave->level_and[2] = 0x00;

				/* remove r10 value */
				dave->level_or[4] = 0x000;
				dave->level_and[4] = 0x00;

				/* remove r11 value */
				dave->level_or[6] = 0x000;
				dave->level_and[6] = 0x00;
			}
			else
			{
				/* use r8 value */
				dave->level_or[0] = 0x000;
				dave->level_and[0] = 0xffff;

				/* use r9 value */
				dave->level_or[2] = 0x000;
				dave->level_and[2] = 0xffff;

				/* use r10 value */
				dave->level_or[4] = 0x000;
				dave->level_and[4] = 0xffff;

				/* use r11 value */
				dave->level_or[6] = 0x000;
				dave->level_and[6] = 0xffff;
			}

			/* turn L.H audio output into D/A, outputting value in R12 */
			if (data & (1<<4))
			{
				/* force r12 value */
				dave->level_or[1] = 0x0ffff;
				dave->level_and[1] = 0x00;

				/* remove r13 value */
				dave->level_or[3] = 0x000;
				dave->level_and[3] = 0x00;

				/* remove r14 value */
				dave->level_or[5] = 0x000;
				dave->level_and[5] = 0x00;

				/* remove r15 value */
				dave->level_or[7] = 0x000;
				dave->level_and[7] = 0x00;
			}
			else
			{
				/* use r12 value */
				dave->level_or[1] = 0x000;
				dave->level_and[1] = 0xffff;

				/* use r13 value */
				dave->level_or[3] = 0x000;
				dave->level_and[3] = 0xffff;

				/* use r14 value */
				dave->level_or[5] = 0x000;
				dave->level_and[5] = 0xffff;

				/* use r15 value */
				dave->level_or[7] = 0x000;
				dave->level_and[7] = 0xffff;
			}
		}
		break;

		default:
			break;
	}
}


/*-------------------------------------------------
    dave_reg_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER ( dave_reg_w )
{
	dave_t *dave = get_token(device);

	logerror("dave w: %04x %02x\n",offset,data);

	dave_sound_w(device, space, offset, data, mem_mask);

	dave->Regs[offset & 0x01f] = data;

	switch (offset)
	{
		case 0x07:
		{
			dave_refresh_selectable_int(device);
		}
		break;

		case 0x014:
		{
			/* enabled ints */
			dave->int_enable = data & 0x055;
			/* clear latches */
			dave->int_latch &=~(data & 0x0aa);

			/* reset 1kHz, 50Hz latch */
			if (data & (1<<1))
			{
				dave->int_irq = 0;
			}

			/* refresh ints */
			dave_refresh_ints(device);
		}
		break;

		default:
			break;
	}

	dave->reg_w(offset, data);
}


/*-------------------------------------------------
    dave_set_reg
-------------------------------------------------*/

void dave_set_reg(device_t *device, offs_t offset, UINT8 data)
{
	dave_t *dave = get_token(device);
	dave->Regs[offset & 0x01f] = data;
}


/*-------------------------------------------------
    dave_reg_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( dave_reg_r )
{
	dave_t *dave = get_token(device);

	logerror("dave r: %04x\n",offset);

	dave->reg_r(offset);

	switch (offset)
	{
		case 0x000:
		case 0x001:
		case 0x002:
		case 0x003:
		case 0x004:
		case 0x005:
		case 0x006:
		case 0x007:
		case 0x008:
		case 0x009:
		case 0x00a:
		case 0x00b:
		case 0x00c:
		case 0x00d:
		case 0x00e:
		case 0x00f:
		case 0x018:
		case 0x019:
		case 0x01a:
		case 0x01b:
		case 0x01c:
		case 0x01d:
		case 0x01e:
		case 0x01f:
			return 0x0ff;

		case 0x014:
			return (dave->int_latch & 0x0aa) | (dave->int_input & 0x055);


		default:
			break;
	}

	return dave->Regs[offset & 0x01f];
}


/*-------------------------------------------------
    dave_set_external_int_state - negative edge
    triggered
-------------------------------------------------*/

static void dave_set_external_int_state(device_t *device, int IntID, int State)
{
	dave_t *dave = get_token(device);

	switch (IntID)
	{
		/* connected to Nick virq */
		case DAVE_INT1_ID:
		{
			int previous_state;

			previous_state = dave->int_input;

			dave->int_input &=~(1<<4);

			if (State)
			{
				dave->int_input |=(1<<4);
			}

			if ((previous_state ^ dave->int_input) & (1<<4))
			{
				/* changed state */

				if (dave->int_input & (1<<4))
				{
					/* int request */
					dave->int_latch |= (1<<5);

					dave_refresh_ints(device);
				}
			}

		}
		break;

		case DAVE_INT2_ID:
		{
			int previous_state;

			previous_state = dave->int_input;

			dave->int_input &= ~(1<<6);

			if (State)
			{
				dave->int_input |=(1<<6);
			}

			if ((previous_state ^ dave->int_input) & (1<<6))
			{
				/* changed state */

				if (dave->int_input & (1<<6))
				{
					/* int request */
					dave->int_latch|=(1<<7);

					dave_refresh_ints(device);
				}
			}
		}
		break;

		default:
			break;
	}
}

/*
Reg 4 READ:

b7 = 1: INT2 latch set
b6 = INT2 input pin
b5 = 1: INT1 latch set
b4 = INT1 input pin
b3 = 1: 1Hz latch set
b2 = 1Hz input pin
b1 = 1: 1kHz/50Hz/TG latch set
b0 = 1kHz/50Hz/TG input

Reg 4 WRITE:

b7 = 1: Reset INT2 latch
b6 = 1: Enable INT2
b5 = 1: Reset INT1 latch
b4 = 1: Enable INT1
b3 = 1: Reset 1Hz interrupt latch
b2 = 1: Enable 1Hz interrupt
b1 = 1: Reset 1kHz/50Hz/TG latch
b0 = 1: Enable 1kHz/50Hz/TG latch
*/



const device_type DAVE = &device_creator<dave_sound_device>;

dave_sound_device::dave_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DAVE, "Dave", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(dave_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void dave_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dave_sound_device::device_start()
{
	DEVICE_START_NAME( dave_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dave_sound_device::device_reset()
{
	DEVICE_RESET_NAME( dave_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void dave_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


