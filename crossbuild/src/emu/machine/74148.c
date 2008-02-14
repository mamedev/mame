/*****************************************************************************

  74148 8-line-to-3-line priority encoder


  Pin layout and functions to access pins:

  input_line_w(4)  [1] /IN4   VCC [16]
  input_line_w(5)  [2] /IN5   /EO [15]  enable_output_r
  input_line_w(6)  [3] /IN6   /GS [14]  output_valid_r
  input_line_w(7)  [4] /IN7  /IN3 [13]  input_line_w(3)
  enable_input_w   [5] /EI   /IN2 [12]  input_line_w(2)
  output_r         [6] /A2   /IN1 [11]  input_line_w(1)
  output_r         [7] /A1   /IN0 [10]  input_line_w(0)
                   [8] GND   /A0  [9]   output_r


  Truth table (all logic levels indicate the actual voltage on the line):

              INPUTS            |     OUTPUTS
                                |
    EI  I0 I1 I2 I3 I4 I5 I6 I7 | A2 A1 A0 | GS EO
    ----------------------------+----------+------
 1  H   X  X  X  X  X  X  X  X  | H  H  H  | H  H
 2  L   H  H  H  H  H  H  H  H  | H  H  H  | H  L
 3  L   X  X  X  X  X  X  X  L  | L  L  L  | L  H
 4  L   X  X  X  X  X  X  L  H  | L  L  H  | L  H
 5  L   X  X  X  X  X  L  H  H  | L  H  L  | L  H
 6  L   X  X  X  X  L  H  H  H  | L  H  H  | L  H
 7  L   X  X  X  L  H  H  H  H  | H  L  L  | L  H
 8  L   X  X  L  H  H  H  H  H  | H  L  H  | L  H
 9  L   X  L  H  H  H  H  H  H  | H  H  L  | L  H
 10 L   L  H  H  H  H  H  H  H  | H  H  H  | L  H
    ----------------------------+----------+------
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#include "driver.h"
#include "machine/74148.h"


#define MAX_TTL74148 4

struct TTL74148
{
	/* callback */
	void (*output_cb)(void);

	/* inputs */
	int input_lines[8];	/* pins 1-4,10-13 */
	int enable_input;	/* pin 5 */

	/* outputs */
	int output;			/* pins 6,7,9 */
	int output_valid;	/* pin 14 */
	int enable_output;	/* pin 15 */

	/* internals */
	int last_output;
	int last_output_valid;
	int last_enable_output;
};

static struct TTL74148 chips[MAX_TTL74148];


void TTL74148_update(int which)
{
	if (chips[which].enable_input)
	{
		// row 1 in truth table
		chips[which].output = 0x07;
		chips[which].output_valid = 1;
		chips[which].enable_output = 1;
	}
	else
	{
		int bit0, bit1, bit2;

		/* this comes straight off the data sheet schematics */
		bit0 = !((!chips[which].input_lines[1] &
		           chips[which].input_lines[2] &
		           chips[which].input_lines[4] &
		           chips[which].input_lines[6])  |
		         (!chips[which].input_lines[3] &
		           chips[which].input_lines[4] &
		           chips[which].input_lines[6])  |
		         (!chips[which].input_lines[5] &
		           chips[which].input_lines[6])  |
		         (!chips[which].input_lines[7]));

		bit1 = !((!chips[which].input_lines[2] &
		           chips[which].input_lines[4] &
		           chips[which].input_lines[5])  |
		         (!chips[which].input_lines[3] &
		           chips[which].input_lines[4] &
		           chips[which].input_lines[5])  |
		         (!chips[which].input_lines[6])  |
		         (!chips[which].input_lines[7]));

		bit2 = !((!chips[which].input_lines[4])  |
		         (!chips[which].input_lines[5])  |
		         (!chips[which].input_lines[6])  |
		         (!chips[which].input_lines[7]));

		chips[which].output = (bit2 << 2) | (bit1 << 1) | bit0;

		chips[which].output_valid = (chips[which].input_lines[0] &
									 chips[which].input_lines[1] &
									 chips[which].input_lines[2] &
									 chips[which].input_lines[3] &
									 chips[which].input_lines[4] &
									 chips[which].input_lines[5] &
									 chips[which].input_lines[6] &
									 chips[which].input_lines[7]);

		chips[which].enable_output = !chips[which].output_valid;
	}


	/* call callback if any of the outputs changed */
	if (  chips[which].output_cb &&
		((chips[which].output        != chips[which].last_output) ||
	     (chips[which].output_valid  != chips[which].last_output_valid) ||
	     (chips[which].enable_output != chips[which].last_enable_output)))
	{
		chips[which].last_output = chips[which].output;
		chips[which].last_output_valid = chips[which].output_valid;
		chips[which].last_enable_output = chips[which].enable_output;

		chips[which].output_cb();
	}
}


void TTL74148_input_line_w(int which, int input_line, int data)
{
	chips[which].input_lines[input_line] = data ? 1 : 0;
}


void TTL74148_enable_input_w(int which, int data)
{
	chips[which].enable_input = data ? 1 : 0;
}


int TTL74148_output_r(int which)
{
	return chips[which].output;
}


int TTL74148_output_valid_r(int which)
{
	return chips[which].output_valid;
}


int TTL74148_enable_output_r(int which)
{
	return chips[which].enable_output;
}



void TTL74148_config(int which, const struct TTL74148_interface *intf)
{
	if (which >= MAX_TTL74148)
	{
		logerror("Only %d 74148's are supported at this time.\n", MAX_TTL74148);
		return;
	}


	chips[which].output_cb = (intf ? intf->output_cb : 0);
	chips[which].enable_input = 1;
	chips[which].input_lines[0] = 1;
	chips[which].input_lines[1] = 1;
	chips[which].input_lines[2] = 1;
	chips[which].input_lines[3] = 1;
	chips[which].input_lines[4] = 1;
	chips[which].input_lines[5] = 1;
	chips[which].input_lines[6] = 1;
	chips[which].input_lines[7] = 1;

	chips[which].last_output = -1;
	chips[which].last_output_valid = -1;
	chips[which].last_enable_output = -1;
}
