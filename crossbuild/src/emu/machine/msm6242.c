/***************************************************************************

    MSM6242 Real Time Clock

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "machine/msm6242.h"

static UINT8 msm6264_reg[3] = { 0, 0, 0 };
static mame_system_time msm6264_hold_time = { 0 };

enum
{
	MSM6264_REG_S1		= 0,
	MSM6264_REG_S10,
	MSM6264_REG_MI1,
	MSM6264_REG_MI10,
	MSM6264_REG_H1,
	MSM6264_REG_H10,
	MSM6264_REG_D1,
	MSM6264_REG_D10,
	MSM6264_REG_MO1,
	MSM6264_REG_MO10,
	MSM6264_REG_Y1,
	MSM6264_REG_Y10,
	MSM6264_REG_W,
	MSM6264_REG_CD,
	MSM6264_REG_CE,
	MSM6264_REG_CF
};

READ8_HANDLER( msm6242_r )
{
	mame_system_time curtime, *systime = &curtime;

	if ( msm6264_reg[0] & 1 ) /* if HOLD is set, use the hold time */
	{
		systime = &msm6264_hold_time;
	}
	else /* otherwise, use the current time */
	{
		mame_get_current_datetime(Machine, &curtime);
	}

	switch(offset)
	{
		case MSM6264_REG_S1: return systime->local_time.second % 10;
		case MSM6264_REG_S10: return systime->local_time.second / 10;
		case MSM6264_REG_MI1: return systime->local_time.minute % 10;
		case MSM6264_REG_MI10: return systime->local_time.minute / 10;
		case MSM6264_REG_H1:
		case MSM6264_REG_H10:
		{
			/* check for 12/24 hour mode */
			if ( (msm6264_reg[2] & 0x04) == 0 ) /* 12 hour mode? */
			{
				int	hour = systime->local_time.hour;
				int	pm = (hour >= 12) ? 1 : 0;

				if ( pm ) hour -= 12;
				if ( hour == 0 ) hour = 12;

				if ( offset == MSM6264_REG_H1 )
					return hour % 10;

				return (hour / 10) | (pm << 2);
			}

			if ( offset == MSM6264_REG_H1 )
				return systime->local_time.hour % 10;

			return systime->local_time.hour / 10;
		}

		case MSM6264_REG_D1: return systime->local_time.mday % 10;
		case MSM6264_REG_D10: return systime->local_time.mday / 10;
		case MSM6264_REG_MO1: return (systime->local_time.month+1) % 10;
		case MSM6264_REG_MO10: return (systime->local_time.month+1) / 10;
		case MSM6264_REG_Y1: return (systime->local_time.year - 1900) % 10;
		case MSM6264_REG_Y10: return (systime->local_time.year - 1900) / 10;
		case MSM6264_REG_W: return systime->local_time.weekday;
		case MSM6264_REG_CD: return msm6264_reg[0];
		case MSM6264_REG_CE: return msm6264_reg[1];
		case MSM6264_REG_CF: return msm6264_reg[2];
	}

	logerror("%04x: MSM6242 unmapped offset %02X read\n",activecpu_get_pc(),offset);
	return 0;
}

WRITE8_HANDLER( msm6242_w )
{
	switch(offset)
	{
		case MSM6264_REG_CD:
		{
			 msm6264_reg[0] = data;

			 if ( data & 1 )	/* was Hold set? */
			 {
			 	mame_get_current_datetime(Machine, &msm6264_hold_time);
			 }

			 return;
		}

		case MSM6264_REG_CE: msm6264_reg[1] = data; return;

		case MSM6264_REG_CF:
		{
			/* the 12/24 mode bit can only be changed while REST is 1 */
			if ( (data ^ msm6264_reg[2]) & 0x04 )
			{
				if ( msm6264_reg[2] & 1 )
					msm6264_reg[2] = data;
			}
			else
			{
				msm6264_reg[2] = data;
			}
			return;
		}
	}

	logerror("%04x: MSM6242 unmapped offset %02X written with %02X\n",activecpu_get_pc(),offset,data);
}


READ16_HANDLER( msm6242_lsb_r )
{
	return msm6242_r(offset);
}

WRITE16_HANDLER( msm6242_lsb_w )
{
	if (ACCESSING_LSB)
		msm6242_w(offset,data);
}
