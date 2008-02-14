/*****************************************************************************

  7474 positive-edge-triggered D-type flip-flop with preset, clear and
       complementary outputs.  There are 2 flip-flops per chips


  Pin layout and functions to access pins:

  clear_w        [1] /1CLR         VCC [14]
  d_w            [2]  1D         /2CLR [13]  clear_w
  clock_w        [3]  1CLK          2D [12]  d_w
  preset_w       [4] /1PR         2CLK [11]  clock_w
  output_r       [5]  1Q          /2PR [10]  preset_w
  output_comp_r  [6] /1Q            2Q [9]   output_r
                 [7]  GND          /2Q [8]   output_comp_r


  Truth table (all logic levels indicate the actual voltage on the line):

        INPUTS    | OUTPUTS
                  |
    PR  CLR CLK D | Q  /Q
    --------------+-------
 1  L   H   X   X | H   L
 2  H   L   X   X | L   H
 3  L   L   X   X | H   H  (Note 1)
 4  H   H  _-   X | D  /D
 5  H   H   L   X | Q0 /Q0
    --------------+-------
    L   = lo (0)
    H   = hi (1)
    X   = any state
    _-  = raising edge
    Q0  = previous state

    Note 1: Non-stable configuration

*****************************************************************************/

#include "driver.h"
#include "7474.h"


#define MAX_TTL7474  12

struct TTL7474
{
	/* callback */
	void (*output_cb)(void);

	/* inputs */
	UINT8 clear;			/* pin 1/13 */
	UINT8 preset;			/* pin 4/10 */
	UINT8 clock;			/* pin 3/11 */
	UINT8 d;				/* pin 2/12 */

	/* outputs */
	UINT8 output;			/* pin 5/9 */
	UINT8 output_comp;	/* pin 6/8 */

	/* internal */
	UINT8 last_clock;
	UINT8 last_output;
	UINT8 last_output_comp;
};

static struct TTL7474 chips[MAX_TTL7474];


void TTL7474_update(int which)
{
	if (!chips[which].preset && chips[which].clear)			  /* line 1 in truth table */
	{
		chips[which].output 	 = 1;
		chips[which].output_comp = 0;
	}
	else if (chips[which].preset && !chips[which].clear)	  /* line 2 in truth table */
	{
		chips[which].output 	 = 0;
		chips[which].output_comp = 1;
	}
	else if (!chips[which].preset && !chips[which].clear)	  /* line 3 in truth table */
	{
		chips[which].output 	 = 1;
		chips[which].output_comp = 1;
	}
	else if (!chips[which].last_clock && chips[which].clock)  /* line 4 in truth table */
	{
		chips[which].output 	 =  chips[which].d;
		chips[which].output_comp = !chips[which].d;
	}

	chips[which].last_clock = chips[which].clock;


	/* call callback if any of the outputs changed */
	if (  chips[which].output_cb &&
	    ((chips[which].output      != chips[which].last_output) ||
	     (chips[which].output_comp != chips[which].last_output_comp)))
	{
		chips[which].last_output = chips[which].output;
		chips[which].last_output_comp = chips[which].output_comp;

		chips[which].output_cb();
	}
}


void TTL7474_clear_w(int which, int data)
{
	chips[which].clear = data ? 1 : 0;
}

void TTL7474_preset_w(int which, int data)
{
	chips[which].preset = data ? 1 : 0;
}

void TTL7474_clock_w(int which, int data)
{
	chips[which].clock = data ? 1 : 0;
}

void TTL7474_d_w(int which, int data)
{
	chips[which].d = data ? 1 : 0;
}


int TTL7474_output_r(int which)
{
	return chips[which].output;
}

int TTL7474_output_comp_r(int which)
{
	return chips[which].output_comp;
}


void TTL7474_config(int which, const struct TTL7474_interface *intf)
{
	struct TTL7474 *chip = &chips[which];

	if (which >= MAX_TTL7474)
	{
		logerror("Only %d 7474's are supported at this time.\n", MAX_TTL7474);
		return;
	}


	chip->output_cb = (intf ? intf->output_cb : 0);

	/* all inputs are open first */
    chip->clear = 1;
    chip->preset = 1;
    chip->clock = 1;
    chip->d = 1;

    chip->last_clock = 1;
    chip->last_output = -1;
    chip->last_output_comp = -1;

    state_save_register_item("ttl7474", which, chip->clear);
    state_save_register_item("ttl7474", which, chip->preset);
    state_save_register_item("ttl7474", which, chip->clock);
    state_save_register_item("ttl7474", which, chip->d);
    state_save_register_item("ttl7474", which, chip->output);
    state_save_register_item("ttl7474", which, chip->output_comp);
    state_save_register_item("ttl7474", which, chip->last_clock);
    state_save_register_item("ttl7474", which, chip->last_output);
    state_save_register_item("ttl7474", which, chip->last_output_comp);
}
