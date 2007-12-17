/*****************************************************************************

  MB87078 6-bit,4-channel electronic volume controller emulator

  An excerpt from the datasheet about the chip functionality:
  "A digital signal input controls gain every 0.5 dB step from 0dB to -32dB.
  - Gain variable range: 0 dB to -32 dB by 0.5dB or -infinity
  - Gain variable range is expanded to connect two channels serially (0 dB to -64 dB)
  - Each channel gain can be set respectively
  - Test function is provided (to confirm internal data)
  - Data is initialized by reset signal (all channels are set to 0dB)
  - Logic I/O is TTL comatible"

  There are 6 digital data input/output pins and DSEL pin that selects
  the group (there are two) of internal registers to be read/written.

  Group 0 is 6-bit gain latch
  Group 1 is 5-bit control latch (2-bits are channel select and 3-bits are volume control)

  Digital I/O Setting:
  /TC   DSEL    D0      D1      D2      D3      D4      D5      I/O MODES (when /TC==H ->write)
  H     H       DSC1    DSC2    EN      C0      C32     X       Input mode
  H     L       GD0     GD1     GD2     GD3     GD4     GD5     (set)
  L     H       DSC1    DSC2    EN      C0      C32     L       Output mode
  L     L       GD0     GD1     GD2     GD3     GD4     GD5     (check)

  Channel Setting:
  DSC2  DSC1    CHANNEL
  L      L         0
  L      H         1
  H      L         2
  H      H         3

  Electrical Volume Setting:
                 DATA*                  GAIN
  GD5 GD4 GD3 GD2 GD1 GD0  EN  C0  C32  (dB)
   1   1   1   1   1   1   1   0   0     0
   1   1   1   1   1   0   1   0   0    -0.5
   1   1   1   1   0   1   1   0   0    -1
   1   1   1   1   0   0   1   0   0    -1.5
   1   1   1   0   1   1   1   0   0    -2
  [..........................................]
   0   0   0   0   0   1   1   0   0    -31
   0   0   0   0   0   0   1   0   0    -31.5
   X   X   X   X   X   X   1   X   1    -32
   X   X   X   X   X   X   1   1   0     0
   X   X   X   X   X   X   0   X   X    -infinity

   X=don't care
   * When reset, DATA is set to 0 dB (code 111111 100)


  MB87078 pins and assigned interface variables/functions

                   /[ 1] D0        /TC [24]
                  | [ 2] D1        /WR [23]
  MB87078_data_w()| [ 3] D2        /CE [22]
  MB87078_data_r()| [ 4] D3       DSEL [21]-MB87078_data_w()/data_r() parameter
                  | [ 5] D4     /RESET [20]-MB87078_reset_comp_w()
                   \[ 6] D5        /PD [19]
                    [ 7] DGND      VDD [18]
                    [ 8] AGND  1/2 VDD [17]
                    [ 9] AIN0    AOUT3 [16]
                    [10] AOUT0    AIN3 [15]
                    [11] AIN1    AOUT2 [14]
                    [12] AOUT1    AIN2 [13]

*****************************************************************************/

#ifndef MB87078_H
#define MB87078_H

#define MAX_MB87078 4

/* The interface structure */
struct MB87078interface {
	void (*gain_changed_cb)(int channel, int percent /*, float decibels*/);
};


void MB87078_stop(void);
void MB87078_start(int which, const struct MB87078interface *intf);

void MB87078_data_w(int which, int data, int dsel);
void MB87078_reset_comp_w(int which, int level);


/* MB87078_gain_decibel_r will return 'channel' gain on chip 'which'.
   Returned value represnts channel gain expressed in decibels,
   Range from 0 to -32.0 (or -256.0 for -infinity)
*/
float MB87078_gain_decibel_r(int which, int channel);


/* MB87078_gain_percent_r will return 'channel' gain on chip 'which'.
   Returned value represents channel gain expressed in percents of maximum volume.
   Range from 100 to 0. (100 = 0dB; 50 = -6dB; 0 = -infinity)

   This function is designed for use with MAME mixer_xxx() functions.
*/
int   MB87078_gain_percent_r(int which, int channel);

#endif
