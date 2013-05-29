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
    IMPLEMENTATION
***************************************************************************/

const device_type DAVE = &device_creator<dave_sound_device>;

dave_sound_device::dave_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, DAVE, "Dave", tag, owner, clock),
						device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void dave_sound_device::device_config_complete()
{
	// inherit a copy of the static data
	const dave_interface *intf = reinterpret_cast<const dave_interface *>(static_config());
	if (intf != NULL)
		*static_cast<dave_interface *>(this) = *intf;
	
	// or initialize to defaults if none provided
	else
	{
		memset(&m_reg_r_cb, 0, sizeof(m_reg_r_cb));
		memset(&m_reg_w_cb, 0, sizeof(m_reg_w_cb));
		memset(&m_int_cb, 0, sizeof(m_int_cb));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dave_sound_device::device_start()
{
	m_reg_r.resolve(m_reg_r_cb, *this);
	m_reg_w.resolve(m_reg_w_cb, *this);
	m_int_callback.resolve(m_int_cb, *this);
	
	/* temp! */
	m_nick_virq = 0;
	
	/* initialise 1kHz timer */
	m_int_latch = 0;
	m_int_input = 0;
	m_int_enable = 0;
	m_timer_irq = 0;
	m_fifty_hz_state = 0;
	m_one_khz_state = 0;
	m_fifty_hz_count = DAVE_FIFTY_HZ_COUNTER_RELOAD;
	m_one_hz_count = DAVE_ONE_HZ_COUNTER_RELOAD;
	machine().scheduler().timer_pulse(attotime::from_hz(1000), timer_expired_delegate(FUNC(dave_sound_device::dave_1khz_callback),this));
	
	for (int i = 0; i < 3; i++)
	{
		m_period[i] = (STEP * machine().sample_rate()) / 125000;
		m_count[i] = (STEP * machine().sample_rate()) / 125000;
		m_level[i] = 0;
	}
	
	/* dave has 3 tone channels and 1 noise channel.
	 the volumes are mixed internally and output as left and right volume */
	
	/* 3 tone channels + 1 noise channel */
	m_sound_stream_var = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate(), this);

	save_item(NAME(m_regs));
	save_item(NAME(m_int_latch));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_input));
	save_item(NAME(m_int_irq));
	save_item(NAME(m_timer_irq));
	save_item(NAME(m_one_khz_state));
	save_item(NAME(m_one_hz_count));
	save_item(NAME(m_fifty_hz_state));
	save_item(NAME(m_fifty_hz_count));
	save_item(NAME(m_period));
	save_item(NAME(m_count));
	save_item(NAME(m_level));
	save_item(NAME(m_level_or));
	save_item(NAME(m_level_and));
	save_item(NAME(m_mame_volumes));
	save_item(NAME(m_nick_virq));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dave_sound_device::device_reset()
{
	for (int i = 0; i < 32; i++)
		m_regs[i] = 0;
	
	address_space &space = machine().driver_data()->generic_space();
	reg_w(space, 0x10, 0);
	reg_w(space, 0x11, 0);
	reg_w(space, 0x12, 0);
	reg_w(space, 0x13, 0);
}


/*-------------------------------------------------
    dave_refresh_ints
-------------------------------------------------*/

void dave_sound_device::refresh_ints()
{
	int int_wanted;

	logerror("int latch: %02x enable: %02x input: %02x\n", (int) m_int_latch, (int) m_int_enable, (int) m_int_input);

	int_wanted = ((m_int_enable << 1) & m_int_latch) != 0;
	m_int_callback(int_wanted);
}


/*-------------------------------------------------
    dave_refresh_selectable_int
-------------------------------------------------*/

void dave_sound_device::refresh_selectable_int()
{
	/* update 1kHz/50Hz/tg latch and int input */
	switch ((m_regs[7]>>5) & 0x03)
	{
		/* 1kHz */
		case 0:
			m_int_latch &= ~(1<<1);
			m_int_latch |= (m_int_irq >> 1) & 0x02;

			/* set int input state */
			m_int_input &= ~(1<<0);
			m_int_input |= (m_one_khz_state & 0x01) << 0;
			break;

		/* 50Hz */
		case 1:
			m_int_latch &= ~(1<<1);
			m_int_latch |= m_int_irq & 0x02;

			/* set int input state */
			m_int_input &= ~(1<<0);
			m_int_input |= (m_fifty_hz_state & 0x01) << 0;
			break;


		default:
			break;
	}

	refresh_ints();
}


/*-------------------------------------------------
    dave_1khz_callback
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(dave_sound_device::dave_1khz_callback)
{
	/* time over - want int */
	m_one_khz_state ^= 0x0ffffffff;

	/* lo-high transition causes int */
	if (m_one_khz_state != 0)
	{
		m_int_irq |= (1<<2);
	}


	/* update fifty Hz counter */
	m_fifty_hz_count--;

	if (m_fifty_hz_count == 0)
	{
		/* these two lines are temp here */
		m_nick_virq ^= 0x0ffffffff;
		set_external_int_state(DAVE_INT1_ID, m_nick_virq);


		m_fifty_hz_count = DAVE_FIFTY_HZ_COUNTER_RELOAD;
		m_fifty_hz_state ^= 0x0ffffffff;

		if (m_fifty_hz_state != 0)
		{
			m_int_irq |= (1<<1);
		}
	}

	m_one_hz_count--;

	if (m_one_hz_count == 0)
	{
		/* reload counter */
		m_one_hz_count = DAVE_ONE_HZ_COUNTER_RELOAD;

		/* change state */
		m_int_input ^= (1<<2);

		if (m_int_input & (1<<2))
		{
			/* transition from 0->1 */
			/* int requested */
			m_int_latch |=(1<<3);
		}
	}

	refresh_selectable_int();
}


/*-------------------------------------------------
    dave_sound_w - used to update sound output
    based on data writes
-------------------------------------------------*/

WRITE8_MEMBER(dave_sound_device::sound_w)
{
	/* update stream */
	m_sound_stream_var->update();

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
					count = (data & 0x0ff) | ((m_regs[offset + 1] & 0x0f)<<8);
				}
				break;

				case 1:
				{
					count = (m_regs[offset - 1] & 0x0ff) | ((data & 0x0f)<<8);

				}
				break;
			}

			count++;


			m_period[channel_index] = ((STEP  * machine().sample_rate())/125000) * count;

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
			m_mame_volumes[offset - 8] = (data & 0x03f) << 9;
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
				m_level_or[0] = 0x0ffff;
				m_level_and[0] = 0x00;

				/* remove r9 value */
				m_level_or[2] = 0x000;
				m_level_and[2] = 0x00;

				/* remove r10 value */
				m_level_or[4] = 0x000;
				m_level_and[4] = 0x00;

				/* remove r11 value */
				m_level_or[6] = 0x000;
				m_level_and[6] = 0x00;
			}
			else
			{
				/* use r8 value */
				m_level_or[0] = 0x000;
				m_level_and[0] = 0xffff;

				/* use r9 value */
				m_level_or[2] = 0x000;
				m_level_and[2] = 0xffff;

				/* use r10 value */
				m_level_or[4] = 0x000;
				m_level_and[4] = 0xffff;

				/* use r11 value */
				m_level_or[6] = 0x000;
				m_level_and[6] = 0xffff;
			}

			/* turn L.H audio output into D/A, outputting value in R12 */
			if (data & (1<<4))
			{
				/* force r12 value */
				m_level_or[1] = 0x0ffff;
				m_level_and[1] = 0x00;

				/* remove r13 value */
				m_level_or[3] = 0x000;
				m_level_and[3] = 0x00;

				/* remove r14 value */
				m_level_or[5] = 0x000;
				m_level_and[5] = 0x00;

				/* remove r15 value */
				m_level_or[7] = 0x000;
				m_level_and[7] = 0x00;
			}
			else
			{
				/* use r12 value */
				m_level_or[1] = 0x000;
				m_level_and[1] = 0xffff;

				/* use r13 value */
				m_level_or[3] = 0x000;
				m_level_and[3] = 0xffff;

				/* use r14 value */
				m_level_or[5] = 0x000;
				m_level_and[5] = 0xffff;

				/* use r15 value */
				m_level_or[7] = 0x000;
				m_level_and[7] = 0xffff;
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

WRITE8_MEMBER( dave_sound_device::reg_w )
{
	logerror("dave w: %04x %02x\n",offset,data);

	sound_w(space, offset, data, mem_mask);

	m_regs[offset & 0x01f] = data;

	switch (offset)
	{
		case 0x07:
			refresh_selectable_int();
			break;

		case 0x014:
			/* enabled ints */
			m_int_enable = data & 0x055;
			/* clear latches */
			m_int_latch &=~(data & 0x0aa);

			/* reset 1kHz, 50Hz latch */
			if (data & (1<<1))
			{
				m_int_irq = 0;
			}

			/* refresh ints */
			refresh_ints();
			break;

		default:
			break;
	}

	m_reg_w(offset, data);
}


/*-------------------------------------------------
    dave_set_reg
-------------------------------------------------*/

void dave_sound_device::set_reg(offs_t offset, UINT8 data)
{
	m_regs[offset & 0x01f] = data;
}


/*-------------------------------------------------
    dave_reg_r
-------------------------------------------------*/

READ8_MEMBER( dave_sound_device::reg_r )
{
	logerror("dave r: %04x\n",offset);

	m_reg_r(offset);

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
			return (m_int_latch & 0x0aa) | (m_int_input & 0x055);


		default:
			break;
	}

	return m_regs[offset & 0x01f];
}


/*-------------------------------------------------
    dave_set_external_int_state - negative edge
    triggered
-------------------------------------------------*/

void dave_sound_device::set_external_int_state(int int_id, int state)
{
	switch (int_id)
	{
		/* connected to Nick virq */
		case DAVE_INT1_ID:
		{
			int previous_state;

			previous_state = m_int_input;

			m_int_input &= ~(1<<4);

			if (state)
			{
				m_int_input |= (1<<4);
			}

			if ((previous_state ^ m_int_input) & (1<<4))
			{
				/* changed state */

				if (m_int_input & (1<<4))
				{
					/* int request */
					m_int_latch |= (1<<5);

					refresh_ints();
				}
			}

		}
		break;

		case DAVE_INT2_ID:
		{
			int previous_state;

			previous_state = m_int_input;

			m_int_input &= ~(1<<6);

			if (state)
			{
				m_int_input |= (1<<6);
			}

			if ((previous_state ^ m_int_input) & (1<<6))
			{
				/* changed state */

				if (m_int_input & (1<<6))
				{
					/* int request */
					m_int_latch |= (1<<7);

					refresh_ints();
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



//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void dave_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
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
		
		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = 0;
		
		for (int i = 0; i < 3; i++)
		{
			if ((m_regs[7] & (1 << i))==0)
			{
				if (m_level[i]) vol[i] += m_count[i];
				m_count[i] -= STEP;
				/* Period[i] is the half period of the square wave. Here, in each */
				/* loop I add Period[i] twice, so that at the end of the loop the */
				/* square wave is in the same status (0 or 1) it was at the start. */
				/* vol[i] is also incremented by Period[i], since the wave has been 1 */
				/* exactly half of the time, regardless of the initial position. */
				/* If we exit the loop in the middle, Output[i] has to be inverted */
				/* and vol[i] incremented only if the exit status of the square */
				/* wave is 1. */
				while (m_count[i] <= 0)
				{
					m_count[i] += m_period[i];
					if (m_count[i] > 0)
					{
						m_level[i] ^= 0x0ffffffff;
						if (m_level[i]) vol[i] += m_period[i];
						break;
					}
					m_count[i] += m_period[i];
					vol[i] += m_period[i];
				}
				if (m_level[i])
					vol[i] -= m_count[i];
			}
		}
		
		/* update volume outputs */
		
		/* setup output volumes for each channel */
		/* channel 0 */
		output_volumes[0] = ((m_level[0] & m_level_and[0]) | m_level_or[0]) & m_mame_volumes[0];
		output_volumes[1] = ((m_level[0] & m_level_and[1]) | m_level_or[1]) & m_mame_volumes[4];
		/* channel 1 */
		output_volumes[2] = ((m_level[1] & m_level_and[2]) | m_level_or[2]) & m_mame_volumes[1];
		output_volumes[3] = ((m_level[1] & m_level_and[3]) | m_level_or[3]) & m_mame_volumes[5];
		/* channel 2 */
		output_volumes[4] = ((m_level[2] & m_level_and[4]) | m_level_or[4]) & m_mame_volumes[2];
		output_volumes[5] = ((m_level[2] & m_level_and[5]) | m_level_or[5]) & m_mame_volumes[6];
		/* channel 3 */
		output_volumes[6] = ((m_level[3] & m_level_and[6]) | m_level_or[6]) & m_mame_volumes[3];
		output_volumes[7] = ((m_level[3] & m_level_and[7]) | m_level_or[7]) & m_mame_volumes[7];
		
		left_volume = (output_volumes[0] + output_volumes[2] + output_volumes[4] + output_volumes[6])>>2;
		right_volume = (output_volumes[1] + output_volumes[3] + output_volumes[5] + output_volumes[7])>>2;
		
		*(buffer1++) = left_volume;
		*(buffer2++) = right_volume;
		
		samples--;
	}
}
