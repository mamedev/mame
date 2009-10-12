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

static int DSP2_waiting_for_command = 0;
static int DSP2_command = 0;
static int DSP2_in_count = 0,  DSP2_in_index = 0;
static int DSP2_out_count = 0, DSP2_out_index = 0;

static UINT8 DSP2_parameters[512] = {0};
static UINT8 DSP2_output[512] = {0};

static UINT8 DSP2_op05transparent = 0;
static int DSP2_op05haslen = 0;
static int DSP2_op05len = 0;
static int DSP2_op06haslen = 0;
static int DSP2_op06len = 0;
static UINT16 DSP2_op09word1 = 0;
static UINT16 DSP2_op09word2 = 0;
static int DSP2_op0dhaslen = 0;
static int DSP2_op0doutlen = 0;
static int DSP2_op0dinlen = 0;


//convert bitmap to bitplane tile
static void DSP2_op01( void )
{
//op01 size is always 32 bytes input and output
//the hardware does strange things if you vary the size

	unsigned char c0, c1, c2, c3;
	unsigned char *p1  = DSP2_parameters;
	unsigned char *p2a = DSP2_output;
	unsigned char *p2b = DSP2_output + 16; //halfway
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
static void DSP2_op03( void )
{
	DSP2_op05transparent = DSP2_parameters[0];
}

//replace bitmap using transparent color
static void DSP2_op05( void )
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
	unsigned char *p1 = DSP2_parameters;
	unsigned char *p2 = DSP2_parameters + DSP2_op05len;
	unsigned char *p3 = DSP2_output;
	int n;

	color = DSP2_op05transparent & 0x0f;

	for (n = 0; n < DSP2_op05len; n++)
	{
		c1 = *p1++;
		c2 = *p2++;
		*p3++ = ( ((c2 >> 4)   == color ) ? c1 & 0xf0 : c2 & 0xf0 ) |
				( ((c2 & 0x0f) == color ) ? c1 & 0x0f : c2 & 0x0f );
	}
}

//reverse bitmap
static void DSP2_op06( void )
{
// Input:
//    size
//    bitmap
	int i, j;
	for (i = 0, j = DSP2_op06len - 1; i < DSP2_op06len; i++, j--)
	{
		DSP2_output[j] = (DSP2_parameters[i] << 4) | (DSP2_parameters[i] >> 4);
	}
}

//multiply
static void DSP2_op09( void )
{
	UINT32 r = 0;
	DSP2_out_count = 4;

	DSP2_op09word1 = DSP2_parameters[0] | (DSP2_parameters[1] << 8);
	DSP2_op09word2 = DSP2_parameters[2] | (DSP2_parameters[3] << 8);

	r = DSP2_op09word1 * DSP2_op09word2;
	DSP2_output[0] = r;
	DSP2_output[1] = r >> 8;
	DSP2_output[2] = r >> 16;
	DSP2_output[3] = r >> 24;
}

//scale bitmap
static void DSP2_op0d( void )
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
	if (DSP2_op0dinlen <= DSP2_op0doutlen)
	{
		multiplier = 0x10000; // In our self defined fixed point 0x10000 == 1
	}
	else
	{
		multiplier = (DSP2_op0dinlen << 17) / ((DSP2_op0doutlen << 1) + 1);
	}

	pixloc = 0;
	for (i = 0; i < DSP2_op0doutlen * 2; i++)
	{
		j = pixloc >> 16;

		if (j & 1)
			pixelarray[i] = (DSP2_parameters[j >> 1] & 0x0f);
		else
			pixelarray[i] = (DSP2_parameters[j >> 1] & 0xf0) >> 4;

		pixloc += multiplier;
	}

	for (i = 0; i < DSP2_op0doutlen; i++)
	{
		DSP2_output[i] = (pixelarray[i << 1] << 4) | pixelarray[(i << 1) + 1];
	}
}


static void DSP2_reset( void )
{
	DSP2_waiting_for_command = 1;
	DSP2_in_count  = 0;
	DSP2_in_index  = 0;
	DSP2_out_count = 0;
	DSP2_out_index = 0;

	DSP2_op05transparent = 0;
	DSP2_op05haslen      = 0;
	DSP2_op05len         = 0;
	DSP2_op06haslen      = 0;
	DSP2_op06len         = 0;
	DSP2_op09word1       = 0;
	DSP2_op09word2       = 0;
	DSP2_op0dhaslen      = 0;
	DSP2_op0doutlen      = 0;
	DSP2_op0dinlen       = 0;
}

static UINT8 DSP2_read( void )
{
	UINT8 r = 0xff;
	if (DSP2_out_count)
	{
		r = DSP2_output[DSP2_out_index++];
		DSP2_out_index &= 511;
		if (DSP2_out_count == DSP2_out_index)
			DSP2_out_count = 0;
	}
	return r;
}

static void DSP2_write(UINT8 data)
{
	if(DSP2_waiting_for_command)
	{
		DSP2_command  = data;
		DSP2_in_index = 0;
		DSP2_waiting_for_command = 0;

		switch (data)
		{
			case 0x01: DSP2_in_count = 32; break;
			case 0x03: DSP2_in_count =  1; break;
			case 0x05: DSP2_in_count =  1; break;
			case 0x06: DSP2_in_count =  1; break;
			case 0x07: break;
			case 0x08: break;
			case 0x09: DSP2_in_count =  4; break;
			case 0x0d: DSP2_in_count =  2; break;
			case 0x0f: DSP2_in_count =  0; break;
		}
	}
	else
	{
		DSP2_parameters[DSP2_in_index++] = data;
		DSP2_in_index &= 511;
	}

	if (DSP2_in_count == DSP2_in_index)
	{
		DSP2_waiting_for_command = 1;
		DSP2_out_index = 0;
		switch (DSP2_command)
		{
			case 0x01:
				DSP2_out_count = 32;
				DSP2_op01();
				break;

			case 0x03:
				DSP2_op03();
				break;

			case 0x05:
				if (DSP2_op05haslen)
				{
					DSP2_op05haslen = 0;
					DSP2_out_count  = DSP2_op05len;
					DSP2_op05();
				}
				else
				{
					DSP2_op05len    = DSP2_parameters[0];
					DSP2_in_index   = 0;
					DSP2_in_count   = DSP2_op05len * 2;
					DSP2_op05haslen = 1;
					if (data)
						DSP2_waiting_for_command = 0;
				}
				break;

			case 0x06:
				if (DSP2_op06haslen)
				{
					DSP2_op06haslen = 0;
					DSP2_out_count  = DSP2_op06len;
					DSP2_op06();
				}
				else
				{
					DSP2_op06len    = DSP2_parameters[0];
					DSP2_in_index   = 0;
					DSP2_in_count   = DSP2_op06len;
					DSP2_op06haslen = 1;
					if (data)
						DSP2_waiting_for_command = 0;
				}
				break;

			case 0x07: break;
			case 0x08: break;

			case 0x09:
				DSP2_op09();
				break;

			case 0x0d:
				if (DSP2_op0dhaslen)
				{
					DSP2_op0dhaslen = 0;
					DSP2_out_count  = DSP2_op0doutlen;
					DSP2_op0d();
				}
				else
				{
					DSP2_op0dinlen  = DSP2_parameters[0];
					DSP2_op0doutlen = DSP2_parameters[1];
					DSP2_in_index   = 0;
					DSP2_in_count   = (DSP2_op0dinlen + 1) >> 1;
					DSP2_op0dhaslen = 1;
					if(data)
						DSP2_waiting_for_command = 0;
				}
				break;

			case 0x0f:
				break;
		}
	}
}
