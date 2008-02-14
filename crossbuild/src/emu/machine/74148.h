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
    H   X  X  X  X  X  X  X  X  | H  H  H  | H  H
    L   H  H  H  H  H  H  H  H  | H  H  H  | H  L
    L   X  X  X  X  X  X  X  L  | L  L  L  | L  H
    L   X  X  X  X  X  X  L  H  | L  L  H  | L  H
    L   X  X  X  X  X  L  H  H  | L  H  L  | L  H
    L   X  X  X  X  L  H  H  H  | L  H  H  | L  H
    L   X  X  X  L  H  H  H  H  | H  L  L  | L  H
    L   X  X  L  H  H  H  H  H  | H  L  H  | L  H
    L   X  L  H  H  H  H  H  H  | H  H  L  | L  H
    L   L  H  H  H  H  H  H  H  | H  H  H  | L  H
    ----------------------------+----------+------
    L   = lo (0)
    H   = hi (1)
    X   = any state

*****************************************************************************/

#ifndef TTL74148_H
#define TTL74148_H


/* The interface structure */
struct TTL74148_interface
{
	void (*output_cb)(void);
};


void TTL74148_config(int which, const struct TTL74148_interface *intf);

/* must call TTL74148_update() after setting the inputs */
void TTL74148_update(int which);

void TTL74148_input_line_w(int which, int input_line, int data);
void TTL74148_enable_input_w(int which, int data);
int  TTL74148_output_r(int which);
int  TTL74148_output_valid_r(int which);
int  TTL74148_enable_output_r(int which);

#endif
