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

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type msm6242 = &device_creator<msm6242_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  xxx_device - constructor
//-------------------------------------------------

msm6242_device::msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, msm6242, "msm6242", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool msm6242_device::device_validity_check(emu_options &options, const game_driver &driver) const
{
	bool error = false;
	return error;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6242_device::device_start()
{
	reg[0] = 0;
	reg[1] = 0;
	reg[2] = 0;
	memset(&hold_time, 0, sizeof(system_time));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm6242_device::device_reset()
{
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( msm6242_device::read )
{
	system_time curtime, *systime = &curtime;

	if (reg[0] & 1) /* if HOLD is set, use the hold time */
	{
		systime = &hold_time;
	}
	else /* otherwise, use the current time */
	{
		machine().current_datetime(curtime);
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
			if ((reg[2] & 0x04) == 0) /* 12 hour mode? */
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
		case MSM6242_REG_CD: return reg[0];
		case MSM6242_REG_CE: return reg[1];
		case MSM6242_REG_CF: return reg[2];
	}

	logerror("%s: MSM6242 unmapped offset %02x read\n", machine().describe_context(), offset);
	return 0;
}

WRITE8_MEMBER( msm6242_device::write )
{
	switch(offset)
	{
		case MSM6242_REG_CD:
		{
			reg[0] = data & 0x0f;

			if (data & 1)	/* was Hold set? */
			{
				machine().current_datetime(hold_time);
			}

			return;
		}

		case MSM6242_REG_CE: reg[1] = data & 0x0f; return;

		case MSM6242_REG_CF:
		{
			/* the 12/24 mode bit can only be changed while REST is 1 */
			if ((data ^ reg[2]) & 0x04)
			{
				reg[2] = (reg[2] & 0x04) | (data & ~0x04);

				if (reg[2] & 1)
					reg[2] = (reg[2] & ~0x04) | (data & 0x04);
			}
			else
			{
				reg[2] = data & 0x0f;
			}
			return;
		}
	}

	logerror("%s: MSM6242 unmapped offset %02x written with %02x\n", machine().describe_context(), offset, data);
}






