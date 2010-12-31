/***************************************************************************

    MSM6242 Real Time Clock

***************************************************************************/

#include "emu.h"
#include "machine/msm6242.h"


enum
{
	MSM6242_REG_S1		= 0,
	MSM6242_REG_S10,
	MSM6242_REG_MI1,
	MSM6242_REG_MI10,
	MSM6242_REG_H1,
	MSM6242_REG_H10,
	MSM6242_REG_D1,
	MSM6242_REG_D10,
	MSM6242_REG_MO1,
	MSM6242_REG_MO10,
	MSM6242_REG_Y1,
	MSM6242_REG_Y10,
	MSM6242_REG_W,
	MSM6242_REG_CD,
	MSM6242_REG_CE,
	MSM6242_REG_CF
};


typedef struct _msm6242_t msm6242_t;
struct _msm6242_t
{
	UINT8 reg[3];
	system_time hold_time;
};


/* makes sure that the passed in device is the right type */
INLINE msm6242_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MSM6242);

	return (msm6242_t *)downcast<legacy_device_base *>(device)->token();
}


READ8_DEVICE_HANDLER( msm6242_r )
{
	system_time curtime, *systime = &curtime;
	msm6242_t *msm6242 = get_safe_token(device);

	if (msm6242->reg[0] & 1) /* if HOLD is set, use the hold time */
	{
		systime = &msm6242->hold_time;
	}
	else /* otherwise, use the current time */
	{
		device->machine->current_datetime(curtime);
	}

	switch(offset)
	{
		case MSM6242_REG_S1: return systime->local_time.second % 10;
		case MSM6242_REG_S10: return systime->local_time.second / 10;
		case MSM6242_REG_MI1: return systime->local_time.minute % 10;
		case MSM6242_REG_MI10: return systime->local_time.minute / 10;
		case MSM6242_REG_H1:
		case MSM6242_REG_H10:
		{
			int	hour = systime->local_time.hour;
			int pm = 0;

			/* check for 12/24 hour mode */
			if ((msm6242->reg[2] & 0x04) == 0) /* 12 hour mode? */
			{
				if (hour >= 12)
					pm = 1;

				hour %= 12;

				if ( hour == 0 )
				hour = 12;
			}

			if ( offset == MSM6242_REG_H1 )
				return hour % 10;

			return (hour / 10) | (pm <<2);
		}

		case MSM6242_REG_D1: return systime->local_time.mday % 10;
		case MSM6242_REG_D10: return systime->local_time.mday / 10;
		case MSM6242_REG_MO1: return (systime->local_time.month+1) % 10;
		case MSM6242_REG_MO10: return (systime->local_time.month+1) / 10;
		case MSM6242_REG_Y1: return systime->local_time.year % 10;
		case MSM6242_REG_Y10: return (systime->local_time.year % 100) / 10;
		case MSM6242_REG_W: return systime->local_time.weekday;
		case MSM6242_REG_CD: return msm6242->reg[0];
		case MSM6242_REG_CE: return msm6242->reg[1];
		case MSM6242_REG_CF: return msm6242->reg[2];
	}

	logerror("%s: MSM6242 unmapped offset %02x read\n", cpuexec_describe_context(device->machine), offset);
	return 0;
}


WRITE8_DEVICE_HANDLER( msm6242_w )
{
	msm6242_t *msm6242 = get_safe_token(device);

	switch(offset)
	{
		case MSM6242_REG_CD:
		{
			msm6242->reg[0] = data & 0x0f;

			if (data & 1)	/* was Hold set? */
			{
				device->machine->current_datetime(msm6242->hold_time);
			}

			return;
		}

		case MSM6242_REG_CE: msm6242->reg[1] = data & 0x0f; return;

		case MSM6242_REG_CF:
		{
			/* the 12/24 mode bit can only be changed while REST is 1 */
			if ((data ^ msm6242->reg[2]) & 0x04)
			{
				msm6242->reg[2] = (msm6242->reg[2] & 0x04) | (data & ~0x04);

				if (msm6242->reg[2] & 1)
					msm6242->reg[2] = (msm6242->reg[2] & ~0x04) | (data & 0x04);
			}
			else
			{
				msm6242->reg[2] = data & 0x0f;
			}
			return;
		}
	}

	logerror("%s: MSM6242 unmapped offset %02x written with %02x\n", cpuexec_describe_context(device->machine), offset, data);
}


static DEVICE_START( msm6242 )
{
	msm6242_t *msm6242 = get_safe_token(device);

	msm6242->reg[0] = 0;
	msm6242->reg[1] = 0;
	msm6242->reg[2] = 0;
	memset(&msm6242->hold_time, 0, sizeof(system_time));
}


DEVICE_GET_INFO( msm6242 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(msm6242_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(msm6242);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "OKI MSM6242");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MSM6242 RTC");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.00");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}





#if 0
READ16_HANDLER( msm6242_lsb_r )
{
	return msm6242_r(space, offset);
}

WRITE16_HANDLER( msm6242_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		msm6242_w(space, offset, data);
}
#endif


DEFINE_LEGACY_DEVICE(MSM6242, msm6242);
