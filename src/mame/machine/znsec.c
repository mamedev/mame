/*

  CAT702 ZN security chip

  A serial magic latch.

  It's a DIP20 chip with a sticker of the form XXnn, where XX is the
  company and nn a number:
    AC = Acclaim
    AT = Atlus
    CP = Capcom
    ET = Raizing
    KN = Konami
    MG = Tecmo
    TT = Taito
    TW = Atari

  There usually are 2 of them, one on the cpu board and one on the rom
  board.  The cpu board one is usually numbered 01.

  Pinout:             GND -11  10- GND
                        ? -12   9- +5V
                      +5V -13   8- Data in
                 Data out- 14   7- Clock
                      +5V -15   6- Select
                        ? -16   5- Select
                      +5V -17   4- +5V
                      +5V -18   3- +5V
                      +5V -19   2- +5V
                      +5V -20   1- ?

  The chip works with the '?' lines left unconnected.

  The communication protocol is serial, and in practice the standard
  psx controller communication protocol minus the ack.  Drive both
  select to ground to start a communication, send bits and get the
  results on the raising clock.  Put both select back to +5V when
  finished.  The bios seems to use two communication clock speeds,
  ~300KHz (standard psx) and ~2MHz.  Driving it with lower clocks
  works reasonably, at least at 1KHz.

  The data is divided in bytes but there is no signal for end-of-byte.
  In all of the following the data will be considered coming and going
  lower-bit first.

  Internally the chip has a 8-bit state, initialized at communication
  start to 0xfc.  The structure is simple:


                  +---------+         bit number        +--------+
  Clock   ------->| bit     |-----+-------------------->| bit    |---------> Data out
                  | counter |     |                     | select |
                  +---------+     v      +-------+ out  |        |
                      |        +-----+   | 8bit  |=====>|        |
  Data in ------------|------->| TF1 |<=>| state |      +--------+
                      |        +-----+   |       |
                      |                  |       |
                      | start  +-----+   |       |
                      +------->| TF2 |<=>|       |
                               +-----+   +-------+

  The chip starts by tranforming the state with TF2.  Then, for each
  input bit from 0 to 7:
    - the nth bit from the state is sent to the output
    - the state is transformed by TF1 if the input bit is 0

  TF2 is a fixed linear substitution box (* = and, + = xor):
    o = ff*s0 + fe*s1 + fc*s2 + f8*s3 + f0*s4 + e0*s5 + c0*s6 + 7f*s7

  TF1 is a chip-dependent set of 8 linear sboxes, one per bit number.
  In practice, only the sbox for bit 0 is defined for the chip, the 7
  other are derived from it.  Defining the byte transformation Shift
  as:
       Shift(i7..i0) = i6..i0, i7^i6

  and noting the sboxes as:
       Sbox(n, i7..i0) =    Xor(    c[n, bit]*i[bit])
                         0<=bit<=7
  then
       c[n, bit=0..6] = Shift(c[n-1, (bit-1)&7])
       c[n, 7]        = Shift(c[n-1, 6])^c[n, 0]
                      = Shift(c[n-1, 6])^Shift(c[n-1, 7])
*/

#include "emu.h"
#include "znsec.h"

struct znsec_state {
	const UINT8 *transform;
	UINT8 state;
	UINT8 bit;
};

static znsec_state zns[2];


// Given the value for x7..x0 and linear transform coefficients a7..a0
// compute the value of the transform
#if 0
static int c_linear(UINT8 x, UINT8 a)
{
	int i;
	UINT8 r;
	x &= a;
	r = 0;
	for(i=0; i<8; i++)
		if(x & (1<<i))
			r = !r;
	return r;
}
#endif

// Derive the sbox xor mask for a given input and select bit
static UINT8 compute_sbox_coef(int chip, int sel, int bit)
{
	UINT8 r;
	if(!sel)
		return zns[chip].transform[bit];
	r = compute_sbox_coef(chip, (sel-1) & 7, (bit-1) & 7);
	r = (r << 1)|(((r >> 7)^(r >> 6)) & 1);
	if(bit != 7)
		return r;

	return r ^ compute_sbox_coef(chip, sel, 0);
}

// Apply the sbox for a input 0 bit
static UINT8 apply_bit_sbox(int chip, UINT8 state, int sel)
{
	int i;
	UINT8 r = 0;
	for(i=0; i<8; i++)
		if(state & (1<<i))
			r ^= compute_sbox_coef(chip, sel, i);
	return r;
}

// Apply a sbox
static UINT8 apply_sbox(UINT8 state, const UINT8 *sbox)
{
	int i;
	UINT8 r = 0;
	for(i=0; i<8; i++)
		if(state & (1<<i))
			r ^= sbox[i];
	return r;
}

void znsec_init(int chip, const UINT8 *transform)
{
	zns[chip].transform = transform;
	zns[chip].state = 0xfc;
	zns[chip].bit = 0;
}

void znsec_start(int chip)
{
	zns[chip].state = 0xfc;
	zns[chip].bit = 0;
}

UINT8 znsec_step(int chip, UINT8 input)
{
	UINT8 res;
	static const UINT8 initial_sbox[8] = { 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x7f };

	if (zns[chip].bit==0)
	{
		// Apply the initial xbox
		zns[chip].state = apply_sbox(zns[chip].state, initial_sbox);
	}

	// Compute the output and change the state
	res = (zns[chip].state>>zns[chip].bit) & 1;
	if((input & 1)==0)
		zns[chip].state = apply_bit_sbox(chip, zns[chip].state, zns[chip].bit);

	zns[chip].bit++;
	zns[chip].bit&=7;
	return res;
}
