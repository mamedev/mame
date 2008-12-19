/*****************************************************************************

  MB87078 6-bit,4-channel electronic volume controller emulator

  (for more detailed chip description see the mb87078.h file)

 *****************************************************************************/

#include "driver.h"
#include "machine/mb87078.h"

struct MB87078 {
	const struct MB87078interface *intf;
	int gain[4];		/* gain index 0-63,64,65 */
	int channel_latch;	/* current channel */
	UINT8 latch[4*2];	/* 6bit+3bit 4 data latches */
	UINT8 reset_comp;
};

static struct MB87078 chip[MAX_MB87078];

static const float MB87078_gain_decibel[66]={
	0.0, -0.5, -1.0, -1.5, -2.0, -2.5, -3.0, -3.5,
   -4.0, -4.5, -5.0, -5.5, -6.0, -6.5, -7.0, -7.5,
   -8.0, -8.5, -9.0, -9.5,-10.0,-10.5,-11.0,-11.5,
  -12.0,-12.5,-13.0,-13.5,-14.0,-14.5,-15.0,-15.5,
  -16.0,-16.5,-17.0,-17.5,-18.0,-18.5,-19.0,-19.5,
  -20.0,-20.5,-21.0,-21.5,-22.0,-22.5,-23.0,-23.5,
  -24.0,-24.5,-25.0,-25.5,-26.0,-26.5,-27.0,-27.5,
  -28.0,-28.5,-29.0,-29.5,-30.0,-30.5,-31.0,-31.5,
  -32.0, -256.0
  };

static const int MB87078_gain_percent[66]={
   100,94,89,84,79,74,70,66,
    63,59,56,53,50,47,44,42,
    39,37,35,33,31,29,28,26,
    25,23,22,21,19,18,17,16,
    15,14,14,13,12,11,11,10,
    10, 9, 8, 8, 7, 7, 7, 6,
     6, 5, 5, 5, 5, 4, 4, 4,
     3, 3, 3, 3, 3, 2, 2, 2,
   2, 0
};

#define GAIN_MAX_INDEX 64
#define GAIN_INFINITY_INDEX 65



static int calc_gain_index(int data0, int data1)
{
//data 0: GD0-GD5
//data 1: 1  2  4  8  16
//        c1 c2 EN C0 C32

	if (!(data1&4))
	{
		return GAIN_INFINITY_INDEX;
	}
	else
	{
		if ((data1&16))
		{
			return GAIN_MAX_INDEX;
        }
		else
		{
			if ((data1&8))
			{
				return 0;
			}
			else
			{
				return (data0^0x3f);
			}
		}
	}
}


static void gain_recalc(running_machine *machine, int which)
{
	struct MB87078 *c = chip + which;
	int i;

	for (i=0; i<4; i++)
	{
		int old_index = c->gain[i];
		c->gain[i] = calc_gain_index(c->latch[i], c->latch[4+i]);
		if (old_index != c->gain[i])
		{
			(*c->intf->gain_changed_cb)(machine, i, MB87078_gain_percent[c->gain[i]] );
		}
	}
}


void MB87078_start(running_machine *machine, int which, const struct MB87078interface *intf)
{
	if (which >= MAX_MB87078) return;

	chip[which].intf = intf;

	/* reset chip */
	MB87078_reset_comp_w(machine,which,0);
	MB87078_reset_comp_w(machine,which,1);
}


void MB87078_stop(void)
{
	//int i;
	//for (i = 0; i < MAX_MB87078; i++){    };
}


void MB87078_reset_comp_w(running_machine *machine, int which, int level)
{
	struct MB87078 *c = chip + which;

	c->reset_comp = level;

	/*this seems to be true, according to the datasheets*/
	if (level==0)
	{
		c->latch[0] = 0x3f;
		c->latch[1] = 0x3f;
		c->latch[2] = 0x3f;
		c->latch[3] = 0x3f;
		c->latch[4] = 0x0 | 0x4;
		c->latch[5] = 0x1 | 0x4;
		c->latch[6] = 0x2 | 0x4;
		c->latch[7] = 0x3 | 0x4;
	}
	gain_recalc(machine, which);
}


void MB87078_data_w(running_machine *machine, int which, int data, int dsel)
{
	struct MB87078 *c = chip + which;

	if (c->reset_comp==0) return;

	if (dsel==0)
	{/*gd0-gd5*/
		c->latch[0+c->channel_latch] = data & 0x3f;
	}
	else
	{/*dcs1,dsc2,en,c0,c32,X*/
		c->channel_latch = data & 3;
		c->latch[4+c->channel_latch] = data & 0x1f; //always zero bit 5
	}
	gain_recalc(machine, which);
}


float MB87078_gain_decibel_r(int which, int channel)
{
	return MB87078_gain_decibel[ chip[which].gain[channel] ];
}


int MB87078_gain_percent_r(int which, int channel)
{
	return MB87078_gain_percent[ chip[which].gain[channel] ];
}

