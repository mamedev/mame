/*****************************************************************************

  74153 Dual 4-line to 1-line data selectors/multiplexers


  Pin layout and functions to access pins:

  enable_w(0)        [1] /1G   VCC [16]
  b_w                [2] B     /2G [15]  enable_w(1)
  input_line_w(0,3)  [3] 1C3     A [14]  a_w
  input_line_w(0,2)  [4] 1C2   2C3 [13]  input_line_w(1,3)
  input_line_w(0,1)  [5] 1C1   2C2 [12]  input_line_w(1,2)
  input_line_w(0,0)  [6] 1C0   2C1 [11]  input_line_w(1,1)
  output_r(0)        [7] 1Y    2C0 [10]  input_line_w(1,0)
                     [8] GND    2Y [9]   output_r(1)


  Truth table (all logic levels indicate the actual voltage on the line):

            INPUTS         | OUTPUT
                           |
    G | B  A | C0 C1 C2 C3 | Y
    --+------+-------------+---
1   H | X  X | X  X  X  X  | L
2   L | L  L | X  X  X  X  | C0
3   L | L  H | X  X  X  X  | C1
4   L | H  L | X  X  X  X  | C2
5   L | H  H | X  X  X  X  | C3
    --+------+-------------+---
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#include "emu.h"
#include "machine/74153.h"


struct ttl74153_state
{
	/* callback */
	void (*output_cb)(device_t *device);

	/* inputs */
	int a;                  /* pin 14 */
	int b;                  /* pin 2 */
	int input_lines[2][4];  /* pins 3-6,10-13 */
	int enable[2];          /* pins 1,15 */

	/* output */
	int output[2];          /* pins 7,9 */

	/* internals */
	int last_output[2];
};

INLINE ttl74153_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TTL74153);

	return (ttl74153_state *)downcast<ttl74153_device *>(device)->token();
}



void ttl74153_update(device_t *device)
{
	ttl74153_state *state = get_safe_token(device);
	int sel;
	int section;


	sel = (state->b << 1) | state->a;


	/* process both sections */
	for (section = 0; section < 2; section++)
	{
		if (state->enable[section])
			state->output[section] = 0; // row 1 in truth table
		else
			state->output[section] = state->input_lines[section][sel];
	}


	/* call callback if either of the outputs changed */
	if (  state->output_cb &&
		((state->output[0] != state->last_output[0]) ||
			(state->output[1] != state->last_output[1])))
	{
		state->last_output[0] = state->output[0];
		state->last_output[1] = state->output[1];

		state->output_cb(device);
	}
}


void ttl74153_a_w(device_t *device, int data)
{
	ttl74153_state *state = get_safe_token(device);
	state->a = data ? 1 : 0;
}


void ttl74153_b_w(device_t *device, int data)
{
	ttl74153_state *state = get_safe_token(device);
	state->b = data ? 1 : 0;
}


void ttl74153_input_line_w(device_t *device, int section, int input_line, int data)
{
	ttl74153_state *state = get_safe_token(device);
	state->input_lines[section][input_line] = data ? 1 : 0;
}


void ttl74153_enable_w(device_t *device, int section, int data)
{
	ttl74153_state *state = get_safe_token(device);
	state->enable[section] = data ? 1 : 0;
}


int ttl74153_output_r(device_t *device, int section)
{
	ttl74153_state *state = get_safe_token(device);
	return state->output[section];
}


static DEVICE_START( ttl74153 )
{
	ttl74153_config *config = (ttl74153_config *)device->static_config();
	ttl74153_state *state = get_safe_token(device);
	state->output_cb = config->output_cb;

	device->save_item(NAME(state->enable));
	device->save_item(NAME(state->last_output));
	device->save_item(NAME(state->input_lines[0][0]));
	device->save_item(NAME(state->input_lines[0][1]));
	device->save_item(NAME(state->input_lines[0][2]));
	device->save_item(NAME(state->input_lines[0][3]));
	device->save_item(NAME(state->input_lines[1][0]));
	device->save_item(NAME(state->input_lines[1][1]));
	device->save_item(NAME(state->input_lines[1][2]));
	device->save_item(NAME(state->input_lines[1][3]));
	device->save_item(NAME(state->a));
	device->save_item(NAME(state->b));
}


static DEVICE_RESET( ttl74153 )
{
	ttl74153_state *state = get_safe_token(device);

	state->a = 1;
	state->b = 1;
	state->enable[0] = 1;
	state->enable[1] = 1;
	state->input_lines[0][0] = 1;
	state->input_lines[0][1] = 1;
	state->input_lines[0][2] = 1;
	state->input_lines[0][3] = 1;
	state->input_lines[1][0] = 1;
	state->input_lines[1][1] = 1;
	state->input_lines[1][2] = 1;
	state->input_lines[1][3] = 1;

	state->last_output[0] = -1;
	state->last_output[1] = -1;
}

const device_type TTL74153 = &device_creator<ttl74153_device>;

ttl74153_device::ttl74153_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TTL74153, "74153", tag, owner, clock)
{
	m_token = global_alloc_clear(ttl74153_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ttl74153_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74153_device::device_start()
{
	DEVICE_START_NAME( ttl74153 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl74153_device::device_reset()
{
	DEVICE_RESET_NAME( ttl74153 )(this);
}
