/*****************************************************************************
 *
 * machine/ps2.c
 *
 * IBM Personal System 2
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/ps2.h"


/*
ibm ps2 model 30
----------------
(postcode at 0x0190!)
f008a
f0112 "real mode switchback"
f0147
f0204 03
f02a5 04
f03b8 07
f0413 08
f0470 0a
f04a2 0b
f04e9 0e
f0525 0f
f088f
 f215c
f0928 12
f0bbf
f0c27
f0c96 24
f0e01 40
 e1cf0
  e2b2c
 e1d48
f0e45
 beep code
f0e8e 43
f0ec9 45
f0f19 48
f0f62 4a
f0f97 4c
f0ff7 4e
f1050 51
f118c 52
f11e3 53
f2393 f0
f23eb f2
f241f f3 task register usage!
f2468 f4
f24d2 f5
f2524 f6
f2574 f7
f25be f9
f264c 3e
f137e 55
f13a8 output of keyboard problem
f152b 5b
f1543 5c
f15c6 60
f16c9 65
 various error outputs
f176a 66
f1827 6a
f18ec 6c
f1a1e 6e
f2bf7 6f

f2dcc

f1db9 write byte to nvram
*/

static struct {
	UINT8 data[8];
} pos={
	{ 0 }
};

 READ8_HANDLER(ps2_pos_r)
{
	UINT8 data=pos.data[offset];
	switch (offset) {
	case 2:
		data|=8;
		break;
	}
	return data;
}

WRITE8_HANDLER(ps2_pos_w)
{
	pos.data[offset]=data;
}
