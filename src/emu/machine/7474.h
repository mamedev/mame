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
    H   H   L   X | Q0 /Q01
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

#include "devlegcy.h"


typedef struct _ttl7474_config ttl7474_config;
struct _ttl7474_config
{
	devcb_write_line output_cb;
	devcb_write_line comp_output_cb;
};


#define MDRV_7474_ADD(_tag, _target_tag, _output_cb, _comp_output_cb) \
	MDRV_DEVICE_ADD(_tag, TTL7474, 0) \
	MDRV_DEVICE_CONFIG_WRITE_LINE(ttl7474_config, output_cb, _target_tag, _output_cb) \
	MDRV_DEVICE_CONFIG_WRITE_LINE(ttl7474_config, comp_output_cb, _target_tag, _comp_output_cb)


WRITE_LINE_DEVICE_HANDLER( ttl7474_clear_w );
WRITE_LINE_DEVICE_HANDLER( ttl7474_preset_w );
WRITE_LINE_DEVICE_HANDLER( ttl7474_clock_w );
WRITE_LINE_DEVICE_HANDLER( ttl7474_d_w );
READ_LINE_DEVICE_HANDLER( ttl7474_output_r );
READ_LINE_DEVICE_HANDLER( ttl7474_output_comp_r );	/* NOT strictly the same as !ttl7474_output_r() */

DECLARE_LEGACY_DEVICE(TTL7474, ttl7474);

#endif
