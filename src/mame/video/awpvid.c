/*************************************************************************************

    AWP Hardware video simulation system

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://www.mameworld.net/agemame/ for more information.

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org

**************************************************************************************

   NOTE: Fading lamp system currently only recognises three lamp states:

   0=off, 1, 2= fully on

   Based on evidence, newer techs may need more states, but we can give them their own
   handlers at some stage in the distant future.

   Instructions:
   In order to set up displays (dot matrices, etc) we normally set up the unique
   displays first, and then add the remainder in order.

   The priorities (in descending order) are:

   Full video screens
   Dot matrix displays
   Other, as yet unknown devices

**************************************************************************************/

#include "emu.h"
#include "awpvid.h"
#include "rendlay.h"
#include "machine/steppers.h"

static UINT8 steps[MAX_STEPPERS];
static UINT8 symbols[MAX_STEPPERS];
static UINT8 reelpos[MAX_STEPPERS];

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
		reelpos[rno] = stepper_get_position(rno)%(stepper_get_max(rno)-1);
		for ( m = 0; m < (rsymbols-1); m++ )
		{
			{
				sprintf(rga,"reel%da%d", x, m);
				output_set_value(rga,(reelpos[rno] + rsteps * m)%stepper_get_max(rno));
			}

			{
				if ((reelpos[rno] - rsteps * m) < 0)
				{
					sprintf(rgb,"reel%db%d", x, m);
					output_set_value(rgb,(reelpos[rno] - (rsteps * m - stepper_get_max(rno))));
				}
				else
				{
					sprintf(rgb,"reel%db%d", x, m);
					output_set_value(rgb,(reelpos[rno] - rsteps * m));
				}
			}
		}
		output_set_value(rg,(reelpos[rno]));
	}
}
