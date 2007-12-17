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
    L   H   X   X | H   L
    H   L   X   X | L   H
    L   L   X   X | H   H  (Note 1)
    H   H  _-   X | D  /D
    H   H   L   X | Q0 /Q0
    --------------+-------
    L   = lo (0)
    H   = hi (1)
    X   = any state
    _-  = raising edge
    Q0  = previous state

    Note 1: Non-stable configuration

*****************************************************************************/

#ifndef TTL7474_H
#define TTL7474_H


/* The interface structure */
struct TTL7474_interface
{
	void (*output_cb)(void);
};


void TTL7474_config(int which, const struct TTL7474_interface *intf);

/* must call TTL7474_update() after setting the inputs */
void TTL7474_update(int which);

void TTL7474_clear_w(int which, int data);
void TTL7474_preset_w(int which, int data);
void TTL7474_clock_w(int which, int data);
void TTL7474_d_w(int which, int data);
int  TTL7474_output_r(int which);
int  TTL7474_output_comp_r(int which);	/* NOT strictly the same as !TTL7474_output_r() */

#endif
