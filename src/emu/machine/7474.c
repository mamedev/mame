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


typedef struct _ttl7474_state ttl7474_state;
struct _ttl7474_state
{
	/* callback */
	void (*output_cb)(const device_config *);

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

INLINE ttl7474_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == TTL7474);

	return (ttl7474_state *)device->token;
}


void ttl7474_update(const device_config *device)
{
	ttl7474_state *state = get_safe_token(device);

	if (!state->preset && state->clear)			  /* line 1 in truth table */
	{
		state->output 	 = 1;
		state->output_comp = 0;
	}
	else if (state->preset && !state->clear)	  /* line 2 in truth table */
	{
		state->output 	 = 0;
		state->output_comp = 1;
	}
	else if (!state->preset && !state->clear)	  /* line 3 in truth table */
	{
		state->output 	 = 1;
		state->output_comp = 1;
	}
	else if (!state->last_clock && state->clock)  /* line 4 in truth table */
	{
		state->output 	 =  state->d;
		state->output_comp = !state->d;
	}

	state->last_clock = state->clock;


	/* call callback if any of the outputs changed */
	if (  state->output_cb &&
	    ((state->output      != state->last_output) ||
	     (state->output_comp != state->last_output_comp)))
	{
		state->last_output = state->output;
		state->last_output_comp = state->output_comp;

		state->output_cb(device);
	}
}


void ttl7474_clear_w(const device_config *device, int data)
{
	ttl7474_state *state = get_safe_token(device);
	state->clear = data ? 1 : 0;
}

void ttl7474_preset_w(const device_config *device, int data)
{
	ttl7474_state *state = get_safe_token(device);
	state->preset = data ? 1 : 0;
}

void ttl7474_clock_w(const device_config *device, int data)
{
	ttl7474_state *state = get_safe_token(device);
	state->clock = data ? 1 : 0;
}

void ttl7474_d_w(const device_config *device, int data)
{
	ttl7474_state *state = get_safe_token(device);
	state->d = data ? 1 : 0;
}


int ttl7474_output_r(const device_config *device)
{
	ttl7474_state *state = get_safe_token(device);
	return state->output;
}

int ttl7474_output_comp_r(const device_config *device)
{
	ttl7474_state *state = get_safe_token(device);
	return state->output_comp;
}


static DEVICE_START( ttl7474 )
{
	ttl7474_config *config = (ttl7474_config *)device->inline_config;
	ttl7474_state *state = get_safe_token(device);
    state->output_cb = config->output_cb;

    state_save_register_device_item(device, 0, state->clear);
    state_save_register_device_item(device, 0, state->preset);
    state_save_register_device_item(device, 0, state->clock);
    state_save_register_device_item(device, 0, state->d);
    state_save_register_device_item(device, 0, state->output);
    state_save_register_device_item(device, 0, state->output_comp);
    state_save_register_device_item(device, 0, state->last_clock);
    state_save_register_device_item(device, 0, state->last_output);
    state_save_register_device_item(device, 0, state->last_output_comp);
}


static DEVICE_RESET( ttl7474 )
{
	ttl7474_state *state = get_safe_token(device);

	/* all inputs are open first */
    state->clear = 1;
    state->preset = 1;
    state->clock = 1;
    state->d = 1;

    state->last_clock = 1;
    state->last_output = -1;
    state->last_output_comp = -1;
}


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##ttl7474##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_INLINE_CONFIG
#define DEVTEMPLATE_NAME		"7474"
#define DEVTEMPLATE_FAMILY		"TTL"
#include "devtempl.h"
