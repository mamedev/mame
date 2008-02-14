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

#include "driver.h"
#include "machine/74153.h"


#define MAX_TTL74153 9

struct TTL74153
{
	/* callback */
	void (*output_cb)(void);

	/* inputs */
	int a;					/* pin 14 */
	int b;					/* pin 2 */
	int input_lines[2][4];	/* pins 3-6,10-13 */
	int enable[2];			/* pins 1,15 */

	/* output */
	int output[2];			/* pins 7,9 */

	/* internals */
	int last_output[2];
};

static struct TTL74153 chips[MAX_TTL74153];


void TTL74153_update(int which)
{
	int sel;
	int section;


	sel = (chips[which].b << 1) | chips[which].a;


	/* process both sections */
	for (section = 0; section < 2; section++)
	{
		if (chips[which].enable[section])
			chips[which].output[section] = 0;	// row 1 in truth table
		else
			chips[which].output[section] = chips[which].input_lines[section][sel];
	}


	/* call callback if either of the outputs changed */
	if (  chips[which].output_cb &&
		((chips[which].output[0] != chips[which].last_output[0]) ||
		 (chips[which].output[1] != chips[which].last_output[1])))
	{
		chips[which].last_output[0] = chips[which].output[0];
		chips[which].last_output[1] = chips[which].output[1];

		chips[which].output_cb();
	}
}


void TTL74153_a_w(int which, int data)
{
	chips[which].a = data ? 1 : 0;
}


void TTL74153_b_w(int which, int data)
{
	chips[which].b = data ? 1 : 0;
}


void TTL74153_input_line_w(int which, int section, int input_line, int data)
{
	chips[which].input_lines[section][input_line] = data ? 1 : 0;
}


void TTL74153_enable_w(int which, int section, int data)
{
	chips[which].enable[section] = data ? 1 : 0;
}


int TTL74153_output_r(int which, int section)
{
	return chips[which].output[section];
}



void TTL74153_config(int which, const struct TTL74153_interface *intf)
{
	if (which >= MAX_TTL74153)
	{
		logerror("Only %d 74153's are supported at this time.\n", MAX_TTL74153);
		return;
	}


	chips[which].output_cb = (intf ? intf->output_cb : 0);
	chips[which].a = 1;
	chips[which].b = 1;
	chips[which].enable[0] = 1;
	chips[which].enable[1] = 1;
	chips[which].input_lines[0][0] = 1;
	chips[which].input_lines[0][1] = 1;
	chips[which].input_lines[0][2] = 1;
	chips[which].input_lines[0][3] = 1;
	chips[which].input_lines[1][0] = 1;
	chips[which].input_lines[1][1] = 1;
	chips[which].input_lines[1][2] = 1;
	chips[which].input_lines[1][3] = 1;

	chips[which].last_output[0] = -1;
	chips[which].last_output[1] = -1;
}
