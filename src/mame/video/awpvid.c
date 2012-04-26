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

static UINT8 steps[MAX_STEPPERS];
static UINT8 symbols[MAX_STEPPERS];
static UINT16 reelpos[MAX_STEPPERS];

void awp_reel_setup(void)
{
	int x,reels;
	char rstep[16],rsym[16];

	if (!output_get_value("TotalReels"))
	{
		reels = 6 ;
	}
	else
	{
		reels = output_get_value("TotalReels");
	}

	for ( x = 0; x < reels; x++ )
	{
		sprintf(rstep, "ReelSteps%d",x+1);
		sprintf(rsym, "ReelSymbols%d",x+1);

		if (!output_get_value(rstep))
		{
			steps[x] = 6 ;
		}
		else
		{
			steps[x] = output_get_value(rstep);
		}

		if (!output_get_value(rsym))
		{
			symbols[x] = 1 ;
		}
		else
		{
			symbols[x] = output_get_value(rsym);
		}
	}
}

void awp_draw_reel(int rno)
{
	int rsteps = steps[rno];
	int rsymbols = symbols[rno];
	int m;
	int x = rno + 1;
	char rg[16], rga[16], rgb[16];

	sprintf(rg,"reel%d", x);
	reelpos[rno] = stepper_get_position(rno);
	if (reelpos[rno] == output_get_value(rg))
	{
		// Not moved, no need to update.
	}
	else
	{
		/* legacy symbol support - should be possible to remove this once the layouts depending on it are converted to scrolling type */
		for ( m = 0; m < (rsymbols-1); m++ )
		{
			{
				sprintf(rga,"reel%da%d", x, m);
				output_set_value(rga,(reelpos[rno] + (rsteps * m) + stepper_get_max(rno)) % stepper_get_max(rno));
			}

			{
				sprintf(rgb,"reel%db%d", x, m);
				output_set_value(rgb,(reelpos[rno] - (rsteps * m) + stepper_get_max(rno)) % stepper_get_max(rno));
			}
		}

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
