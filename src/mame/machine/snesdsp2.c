/***************************************************************************

  snesdsp2.c

  File to handle emulation of the SNES "DSP-2" add-on chip.

  Original C++ code by byuu, based on research by Overload.
  Byuu's code is released under GNU General Public License
  version 2 as published by the Free Software Foundation.
  The implementation below is released under the MAME license
  for use in MAME, MESS and derivatives by permission of the
  author

***************************************************************************/

struct _snes_dsp2_state
{
	int waiting_for_command;
	int command;
	int in_count, in_index;
	int out_count, out_index;

	UINT8 parameters[512];
	UINT8 output[512];

	UINT8 op05transparent;
	int op05haslen;
	int op05len;
	int op06haslen;
	int op06len;
	UINT16 op09word1;
	UINT16 op09word2;
	int op0dhaslen;
	int op0doutlen;
	int op0dinlen;
};

static struct _snes_dsp2_state  dsp2_state;


static void dsp2_register_save(running_machine *machine);

//convert bitmap to bitplane tile
static void dsp2_op01( void )
{
//op01 size is always 32 bytes input and output
//the hardware does strange things if you vary the size

	unsigned char c0, c1, c2, c3;
	unsigned char *p1  = dsp2_state.parameters;
	unsigned char *p2a = dsp2_state.output;
	unsigned char *p2b = dsp2_state.output + 16; //halfway
	int j;

//process 8 blocks of 4 bytes each
	for (j = 0; j < 8; j++)
	{
		c0 = *p1++;
		c1 = *p1++;
		c2 = *p1++;
		c3 = *p1++;

		*p2a++ = (c0 & 0x10) << 3 |
				(c0 & 0x01) << 6 |
				(c1 & 0x10) << 1 |
				(c1 & 0x01) << 4 |
				(c2 & 0x10) >> 1 |
				(c2 & 0x01) << 2 |
				(c3 & 0x10) >> 3 |
				(c3 & 0x01);

		*p2a++ = (c0 & 0x20) << 2 |
				(c0 & 0x02) << 5 |
				(c1 & 0x20)      |
				(c1 & 0x02) << 3 |
				(c2 & 0x20) >> 2 |
				(c2 & 0x02) << 1 |
				(c3 & 0x20) >> 4 |
				(c3 & 0x02) >> 1;

		*p2b++ = (c0 & 0x40) << 1 |
				(c0 & 0x04) << 4 |
				(c1 & 0x40) >> 1 |
				(c1 & 0x04) << 2 |
				(c2 & 0x40) >> 3 |
				(c2 & 0x04)      |
				(c3 & 0x40) >> 5 |
				(c3 & 0x04) >> 2;

		*p2b++ = (c0 & 0x80)      |
				(c0 & 0x08) << 3 |
				(c1 & 0x80) >> 2 |
				(c1 & 0x08) << 1 |
				(c2 & 0x80) >> 4 |
				(c2 & 0x08) >> 1 |
				(c3 & 0x80) >> 6 |
				(c3 & 0x08) >> 3;
	}
}

//set transparent color
static void dsp2_op03( void )
{
	dsp2_state.op05transparent = dsp2_state.parameters[0];
}

//replace bitmap using transparent color
static void dsp2_op05( void )
{
	UINT8 color;
// Overlay bitmap with transparency.
// Input:
//
//   Bitmap 1:  i[0] <=> i[size-1]
//   Bitmap 2:  i[size] <=> i[2*size-1]
//
// Output:
//
//   Bitmap 3:  o[0] <=> o[size-1]
//
// Processing:
//
//   Process all 4-bit pixels (nibbles) in the bitmap
//
//   if ( BM2_pixel == transparent_color )
//      pixelout = BM1_pixel
//   else
//      pixelout = BM2_pixel

// The max size bitmap is limited to 255 because the size parameter is a byte
// I think size=0 is an error.  The behavior of the chip on size=0 is to
// return the last value written to DR if you read DR on Op05 with
// size = 0.  I don't think it's worth implementing this quirk unless it's
// proven necessary.

	unsigned char c1, c2;
	unsigned char *p1 = dsp2_state.parameters;
	unsigned char *p2 = dsp2_state.parameters + dsp2_state.op05len;
	unsigned char *p3 = dsp2_state.output;
	int n;

	color = dsp2_state.op05transparent & 0x0f;

	for (n = 0; n < dsp2_state.op05len; n++)
	{
		c1 = *p1++;
		c2 = *p2++;
		*p3++ = (((c2 >> 4) == color) ? c1 & 0xf0 : c2 & 0xf0) |
				(((c2 & 0x0f) == color) ? c1 & 0x0f : c2 & 0x0f);
	}
}

//reverse bitmap
static void dsp2_op06( void )
{
// Input:
//    size
//    bitmap
	int i, j;
	for (i = 0, j = dsp2_state.op06len - 1; i < dsp2_state.op06len; i++, j--)
	{
		dsp2_state.output[j] = (dsp2_state.parameters[i] << 4) | (dsp2_state.parameters[i] >> 4);
	}
}

//multiply
static void dsp2_op09( void )
{
	UINT32 r = 0;
	dsp2_state.out_count = 4;

	dsp2_state.op09word1 = dsp2_state.parameters[0] | (dsp2_state.parameters[1] << 8);
	dsp2_state.op09word2 = dsp2_state.parameters[2] | (dsp2_state.parameters[3] << 8);

	r = dsp2_state.op09word1 * dsp2_state.op09word2;
	dsp2_state.output[0] = r;
	dsp2_state.output[1] = r >> 8;
	dsp2_state.output[2] = r >> 16;
	dsp2_state.output[3] = r >> 24;
}

//scale bitmap
static void dsp2_op0d( void )
{
// Bit accurate hardware algorithm - uses fixed point math
// This should match the DSP2 Op0D output exactly
// I wouldn't recommend using this unless you're doing hardware debug.
// In some situations it has small visual artifacts that
// are not readily apparent on a TV screen but show up clearly
// on a monitor.  Use Overload's scaling instead.
// This is for hardware verification testing.
//
// One note:  the HW can do odd byte scaling but since we divide
// by two to get the count of bytes this won't work well for
// odd byte scaling (in any of the current algorithm implementations).
// So far I haven't seen Dungeon Master use it.
// If it does we can adjust the parameters and code to work with it

	UINT32 multiplier; // Any size int >= 32-bits
	UINT32 pixloc;     // match size of multiplier
	int    i, j;
	UINT8  pixelarray[512];
	if (dsp2_state.op0dinlen <= dsp2_state.op0doutlen)
	{
		multiplier = 0x10000; // In our self defined fixed point 0x10000 == 1
	}
	else
	{
		multiplier = (dsp2_state.op0dinlen << 17) / ((dsp2_state.op0doutlen << 1) + 1);
	}

	pixloc = 0;
	for (i = 0; i < dsp2_state.op0doutlen * 2; i++)
	{
		j = pixloc >> 16;

		if (j & 1)
			pixelarray[i] = (dsp2_state.parameters[j >> 1] & 0x0f);
		else
			pixelarray[i] = (dsp2_state.parameters[j >> 1] & 0xf0) >> 4;

		pixloc += multiplier;
	}

	for (i = 0; i < dsp2_state.op0doutlen; i++)
	{
		dsp2_state.output[i] = (pixelarray[i << 1] << 4) | pixelarray[(i << 1) + 1];
	}
}


static void dsp2_init( running_machine *machine )
{
	dsp2_state.waiting_for_command = 1;
	dsp2_state.in_count  = 0;
	dsp2_state.in_index  = 0;
	dsp2_state.out_count = 0;
	dsp2_state.out_index = 0;

	memset(dsp2_state.parameters, 0 , ARRAY_LENGTH(dsp2_state.parameters));
	memset(dsp2_state.output, 0 , ARRAY_LENGTH(dsp2_state.output));

	dsp2_state.op05transparent = 0;
	dsp2_state.op05haslen      = 0;
	dsp2_state.op05len         = 0;
	dsp2_state.op06haslen      = 0;
	dsp2_state.op06len         = 0;
	dsp2_state.op09word1       = 0;
	dsp2_state.op09word2       = 0;
	dsp2_state.op0dhaslen      = 0;
	dsp2_state.op0doutlen      = 0;
	dsp2_state.op0dinlen       = 0;

	dsp2_register_save(machine);
}

static UINT8 dsp2_read( void )
{
	UINT8 r = 0xff;
	if (dsp2_state.out_count)
	{
		r = dsp2_state.output[dsp2_state.out_index++];
		dsp2_state.out_index &= 511;
		if (dsp2_state.out_count == dsp2_state.out_index)
			dsp2_state.out_count = 0;
	}
	return r;
}

static void dsp2_write(UINT8 data)
{
	if (dsp2_state.waiting_for_command)
	{
		dsp2_state.command  = data;
		dsp2_state.in_index = 0;
		dsp2_state.waiting_for_command = 0;

		switch (data)
		{
			case 0x01: dsp2_state.in_count = 32; break;
			case 0x03: dsp2_state.in_count =  1; break;
			case 0x05: dsp2_state.in_count =  1; break;
			case 0x06: dsp2_state.in_count =  1; break;
			case 0x07: break;
			case 0x08: break;
			case 0x09: dsp2_state.in_count =  4; break;
			case 0x0d: dsp2_state.in_count =  2; break;
			case 0x0f: dsp2_state.in_count =  0; break;
		}
	}
	else
	{
		dsp2_state.parameters[dsp2_state.in_index++] = data;
		dsp2_state.in_index &= 511;
	}

	if (dsp2_state.in_count == dsp2_state.in_index)
	{
		dsp2_state.waiting_for_command = 1;
		dsp2_state.out_index = 0;
		switch (dsp2_state.command)
		{
		case 0x01:
			dsp2_state.out_count = 32;
			dsp2_op01();
			break;

		case 0x03:
			dsp2_op03();
			break;

		case 0x05:
			if (dsp2_state.op05haslen)
			{
				dsp2_state.op05haslen = 0;
				dsp2_state.out_count  = dsp2_state.op05len;
				dsp2_op05();
			}
			else
			{
				dsp2_state.op05len    = dsp2_state.parameters[0];
				dsp2_state.in_index   = 0;
				dsp2_state.in_count   = dsp2_state.op05len * 2;
				dsp2_state.op05haslen = 1;
				if (data)
					dsp2_state.waiting_for_command = 0;
			}
			break;

		case 0x06:
			if (dsp2_state.op06haslen)
			{
				dsp2_state.op06haslen = 0;
				dsp2_state.out_count  = dsp2_state.op06len;
				dsp2_op06();
			}
			else
			{
				dsp2_state.op06len    = dsp2_state.parameters[0];
				dsp2_state.in_index   = 0;
				dsp2_state.in_count   = dsp2_state.op06len;
				dsp2_state.op06haslen = 1;
				if (data)
					dsp2_state.waiting_for_command = 0;
			}
			break;

		case 0x07: break;
		case 0x08: break;

		case 0x09:
			dsp2_op09();
			break;

		case 0x0d:
			if (dsp2_state.op0dhaslen)
			{
				dsp2_state.op0dhaslen = 0;
				dsp2_state.out_count  = dsp2_state.op0doutlen;
				dsp2_op0d();
			}
			else
			{
				dsp2_state.op0dinlen  = dsp2_state.parameters[0];
				dsp2_state.op0doutlen = dsp2_state.parameters[1];
				dsp2_state.in_index   = 0;
				dsp2_state.in_count   = (dsp2_state.op0dinlen + 1) >> 1;
				dsp2_state.op0dhaslen = 1;
				if (data)
					dsp2_state.waiting_for_command = 0;
			}
			break;

		case 0x0f:
			break;
		}
	}
}

static void dsp2_register_save( running_machine *machine )
{
	state_save_register_global(machine, dsp2_state.waiting_for_command);
	state_save_register_global(machine, dsp2_state.command);
	state_save_register_global(machine, dsp2_state.in_count);
	state_save_register_global(machine, dsp2_state.in_index);
	state_save_register_global(machine, dsp2_state.out_count);
	state_save_register_global(machine, dsp2_state.out_index);

	state_save_register_global_array(machine, dsp2_state.parameters);
	state_save_register_global_array(machine, dsp2_state.output);

	state_save_register_global(machine, dsp2_state.op05transparent);
	state_save_register_global(machine, dsp2_state.op05haslen);
	state_save_register_global(machine, dsp2_state.op05len);
	state_save_register_global(machine, dsp2_state.op06haslen);
	state_save_register_global(machine, dsp2_state.op06len);
	state_save_register_global(machine, dsp2_state.op09word1);
	state_save_register_global(machine, dsp2_state.op09word2);
	state_save_register_global(machine, dsp2_state.op0dhaslen);
	state_save_register_global(machine, dsp2_state.op0doutlen);
	state_save_register_global(machine, dsp2_state.op0dinlen);
}
