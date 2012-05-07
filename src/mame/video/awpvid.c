/*************************************************************************************

    AWP Hardware video simulation system
    originally written for AGEMAME by J.Wallace

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team.

    This is a primitive handler for generating reels with multiple symbols visible
    hanging off steppers.c .

**************************************************************************************/

#include "emu.h"
#include "awpvid.h"
#include "rendlay.h"
#include "machine/steppers.h"

static UINT16 reelpos[MAX_STEPPERS];

void awp_draw_reel(int rno)
{
	int x = rno + 1;
	char rg[16];

	sprintf(rg,"reel%d", x);
	reelpos[rno] = stepper_get_position(rno);
	if (reelpos[rno] == output_get_value(rg))
	{
		// Not moved, no need to update.
	}
	else
	{

		output_set_value(rg,(reelpos[rno]));

		// if the reel isn't configured don't do this, otherwise you'll get DIV0
		if (stepper_get_max(rno))
		{
			sprintf(rg,"sreel%d", x); // our new scrolling reels are called 'sreel'
			// normalize the value
			int sreelpos = (reelpos[rno] * 0x10000) / stepper_get_max(rno);

			output_set_value(rg,sreelpos);
		}
	}
}
