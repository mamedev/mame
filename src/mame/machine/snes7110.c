/***************************************************************************

  snes7110.c

  File to handle emulation of the SNES "SPC7110" add-on chip.

  Based on C++ implementation by Byuu in BSNES.

  Byuu's code is released under GNU General Public License
  version 2 as published by the Free Software Foundation.
  The implementation below is released under the MAME license
  for use in MAME, MESS and derivatives by permission of the
  author

***************************************************************************/


static const UINT32 spc7110_decomp_buffer_size = 64;

static const UINT8 spc7110_evolution_table[53][4] =
{
	{ 0x5a,  1,  1, 1 },
	{ 0x25,  6,  2, 0 },
	{ 0x11,  8,  3, 0 },
	{ 0x08, 10,  4, 0 },
	{ 0x03, 12,  5, 0 },
	{ 0x01, 15,  5, 0 },

	{ 0x5a,  7,  7, 1 },
	{ 0x3f, 19,  8, 0 },
	{ 0x2c, 21,  9, 0 },
	{ 0x20, 22, 10, 0 },
	{ 0x17, 23, 11, 0 },
	{ 0x11, 25, 12, 0 },
	{ 0x0c, 26, 13, 0 },
	{ 0x09, 28, 14, 0 },
	{ 0x07, 29, 15, 0 },
	{ 0x05, 31, 16, 0 },
	{ 0x04, 32, 17, 0 },
	{ 0x03, 34, 18, 0 },
	{ 0x02, 35,  5, 0 },

	{ 0x5a, 20, 20, 1 },
	{ 0x48, 39, 21, 0 },
	{ 0x3a, 40, 22, 0 },
	{ 0x2e, 42, 23, 0 },
	{ 0x26, 44, 24, 0 },
	{ 0x1f, 45, 25, 0 },
	{ 0x19, 46, 26, 0 },
	{ 0x15, 25, 27, 0 },
	{ 0x11, 26, 28, 0 },
	{ 0x0e, 26, 29, 0 },
	{ 0x0b, 27, 30, 0 },
	{ 0x09, 28, 31, 0 },
	{ 0x08, 29, 32, 0 },
	{ 0x07, 30, 33, 0 },
	{ 0x05, 31, 34, 0 },
	{ 0x04, 33, 35, 0 },
	{ 0x04, 33, 36, 0 },
	{ 0x03, 34, 37, 0 },
	{ 0x02, 35, 38, 0 },
	{ 0x02, 36,  5, 0 },

	{ 0x58, 39, 40, 1 },
	{ 0x4d, 47, 41, 0 },
	{ 0x43, 48, 42, 0 },
	{ 0x3b, 49, 43, 0 },
	{ 0x34, 50, 44, 0 },
	{ 0x2e, 51, 45, 0 },
	{ 0x29, 44, 46, 0 },
	{ 0x25, 45, 24, 0 },

	{ 0x56, 47, 48, 1 },
	{ 0x4f, 47, 49, 0 },
	{ 0x47, 48, 50, 0 },
	{ 0x41, 49, 51, 0 },
	{ 0x3c, 50, 52, 0 },
	{ 0x37, 51, 43, 0 },
};

static const UINT8 spc7110_mode2_context_table[32][2] =
{
	{  1,  2 },

	{  3,  8 },
	{ 13, 14 },

	{ 15, 16 },
	{ 17, 18 },
	{ 19, 20 },
	{ 21, 22 },
	{ 23, 24 },
	{ 25, 26 },
	{ 25, 26 },
	{ 25, 26 },
	{ 25, 26 },
	{ 25, 26 },
	{ 27, 28 },
	{ 29, 30 },

	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },
	{ 31, 31 },

	{ 31, 31 },
};

typedef struct
{
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	running_machine *m_machine;

	UINT32 decomp_mode;
	UINT32 decomp_offset;

	UINT8 *decomp_buffer;
	UINT32 decomp_buffer_rdoffset;
	UINT32 decomp_buffer_wroffset;
	UINT32 decomp_buffer_length;

	struct ContextState
	{
		UINT8 index;
		UINT8 invert;
	} context[32];

	UINT32 morton16[2][256];
	UINT32 morton32[4][256];

	UINT32 rom_size;
} SPC7110Decomp;

static SPC7110Decomp* SPC7110Decomp_ctor(running_machine &machine, UINT32 size);
static void SPC7110Decomp_reset(SPC7110Decomp *thisptr);
static void SPC7110Decomp_init(SPC7110Decomp *thisptr, running_machine &machine, UINT32 mode, UINT32 offset, UINT32 index);
static UINT8 SPC7110Decomp_read(SPC7110Decomp *thisptr);
static void SPC7110Decomp_write(SPC7110Decomp *thisptr, UINT8 data);
static UINT8 SPC7110Decomp_dataread(SPC7110Decomp *thisptr);
static void SPC7110Decomp_mode0(SPC7110Decomp *thisptr, UINT8 init);
static void SPC7110Decomp_mode1(SPC7110Decomp *thisptr, UINT8 init);
static void SPC7110Decomp_mode2(SPC7110Decomp *thisptr, UINT8 init);
static UINT8 SPC7110Decomp_probability(SPC7110Decomp *thisptr, UINT32 n);
static UINT8 SPC7110Decomp_next_lps(SPC7110Decomp *thisptr, UINT32 n);
static UINT8 SPC7110Decomp_next_mps(SPC7110Decomp *thisptr, UINT32 n);
static UINT8 SPC7110Decomp_toggle_invert(SPC7110Decomp *thisptr, UINT32 n);
static UINT32 SPC7110Decomp_morton_2x8(SPC7110Decomp *thisptr, UINT32 data);
static UINT32 SPC7110Decomp_morton_4x8(SPC7110Decomp *thisptr, UINT32 data);

static SPC7110Decomp* SPC7110Decomp_ctor(running_machine &machine, UINT32 size)
{
	UINT32 i;
	SPC7110Decomp* newclass = (SPC7110Decomp*)auto_alloc_array(machine, UINT8, sizeof(SPC7110Decomp));
	newclass->decomp_buffer = (UINT8*)auto_alloc_array(machine, UINT8, spc7110_decomp_buffer_size);
	SPC7110Decomp_reset(newclass);

	for(i = 0; i < 256; i++)
	{
		#define map(x, y) (((i >> x) & 1) << y)
		//2x8-bit
		newclass->morton16[1][i] = map(7, 15) + map(6,  7) + map(5, 14) + map(4,  6)
                                 + map(3, 13) + map(2,  5) + map(1, 12) + map(0,  4);
		newclass->morton16[0][i] = map(7, 11) + map(6,  3) + map(5, 10) + map(4,  2)
                                 + map(3,  9) + map(2,  1) + map(1,  8) + map(0,  0);
		//4x8-bit
		newclass->morton32[3][i] = map(7, 31) + map(6, 23) + map(5, 15) + map(4,  7)
                                 + map(3, 30) + map(2, 22) + map(1, 14) + map(0,  6);
		newclass->morton32[2][i] = map(7, 29) + map(6, 21) + map(5, 13) + map(4,  5)
                                 + map(3, 28) + map(2, 20) + map(1, 12) + map(0,  4);
		newclass->morton32[1][i] = map(7, 27) + map(6, 19) + map(5, 11) + map(4,  3)
                                 + map(3, 26) + map(2, 18) + map(1, 10) + map(0,  2);
		newclass->morton32[0][i] = map(7, 25) + map(6, 17) + map(5,  9) + map(4,  1)
                                 + map(3, 24) + map(2, 16) + map(1,  8) + map(0,  0);
		#undef map
	}

	newclass->rom_size = size;

	return newclass;
}

static void SPC7110Decomp_reset(SPC7110Decomp *thisptr)
{
	//mode 3 is invalid; this is treated as a special case to always return 0x00
	//set to mode 3 so that reading decomp port before starting first decomp will return 0x00
	thisptr->decomp_mode = 3;

	thisptr->decomp_buffer_rdoffset = 0;
	thisptr->decomp_buffer_wroffset = 0;
	thisptr->decomp_buffer_length   = 0;
}

static void SPC7110Decomp_init(SPC7110Decomp *thisptr, running_machine &machine, UINT32 mode, UINT32 offset, UINT32 index)
{
	UINT32 i;

	thisptr->m_machine = &machine;

	thisptr->decomp_mode = mode;
	thisptr->decomp_offset = offset;

	thisptr->decomp_buffer_rdoffset = 0;
	thisptr->decomp_buffer_wroffset = 0;
	thisptr->decomp_buffer_length   = 0;

	//reset context states
	for(i = 0; i < 32; i++)
	{
		thisptr->context[i].index  = 0;
		thisptr->context[i].invert = 0;
	}

	switch(thisptr->decomp_mode)
	{
		case 0: SPC7110Decomp_mode0(thisptr, 1); break;
		case 1: SPC7110Decomp_mode1(thisptr, 1); break;
		case 2: SPC7110Decomp_mode2(thisptr, 1); break;
	}

	//decompress up to requested output data index
	while(index--)
	{
		SPC7110Decomp_read(thisptr);
	}
}

static UINT8 SPC7110Decomp_read(SPC7110Decomp *thisptr)
{
	UINT8 data;

	if(thisptr->decomp_buffer_length == 0) {
		//decompress at least (spc7110_decomp_buffer_size / 2) bytes to the buffer
		switch(thisptr->decomp_mode) {
			case 0:
				SPC7110Decomp_mode0(thisptr, 0);
				break;

			case 1:
				SPC7110Decomp_mode1(thisptr, 0);
				break;

			case 2:
				SPC7110Decomp_mode2(thisptr, 0);
				break;

			default:
				return 0x00;
		}
	}

	data = thisptr->decomp_buffer[thisptr->decomp_buffer_rdoffset++];
	thisptr->decomp_buffer_rdoffset &= spc7110_decomp_buffer_size - 1;
	thisptr->decomp_buffer_length--;
	return data;
}

static void SPC7110Decomp_write(SPC7110Decomp *thisptr, UINT8 data)
{
	thisptr->decomp_buffer[thisptr->decomp_buffer_wroffset++] = data;
	thisptr->decomp_buffer_wroffset &= spc7110_decomp_buffer_size - 1;
	thisptr->decomp_buffer_length++;
}

static UINT8 SPC7110Decomp_dataread(SPC7110Decomp *thisptr)
{
	UINT8 *ROM = thisptr->machine().root_device().memregion("cart")->base();
	UINT32 size = thisptr->rom_size - 0x100000;
	while(thisptr->decomp_offset >= size)
	{
		thisptr->decomp_offset -= size;
	}
	return ROM[0x100000 + thisptr->decomp_offset++];
}

static void SPC7110Decomp_mode0(SPC7110Decomp *thisptr, UINT8 init)
{
	static UINT8 val, in, span;
	static INT32 out, inverts, lps, in_count;

	if(init == 1)
	{
		out = inverts = lps = 0;
		span = 0xff;
		val = SPC7110Decomp_dataread(thisptr);
		in = SPC7110Decomp_dataread(thisptr);
		in_count = 8;
		return;
	}

	while(thisptr->decomp_buffer_length < (spc7110_decomp_buffer_size >> 1))
	{
		UINT32 bit;
		for(bit = 0; bit < 8; bit++)
		{
			//get context
			UINT8 mask = (1 << (bit & 3)) - 1;
			UINT8 con = mask + ((inverts & mask) ^ (lps & mask));
			UINT32 prob, mps, flag_lps;
			UINT32 shift = 0;
			if(bit > 3)
			{
				con += 15;
			}

			//get prob and mps
			prob = SPC7110Decomp_probability(thisptr, con);
			mps = (((out >> 15) & 1) ^ thisptr->context[con].invert);

			//get bit
			if(val <= span - prob) //mps
			{
				span = span - prob;
				out = (out << 1) + mps;
				flag_lps = 0;
			}
			else //lps
			{
				val = val - (span - (prob - 1));
				span = prob - 1;
				out = (out << 1) + 1 - mps;
				flag_lps = 1;
			}

			//renormalize
			while(span < 0x7f)
			{
				shift++;

				span = (span << 1) + 1;
				val = (val << 1) + (in >> 7);

				in <<= 1;
				if(--in_count == 0)
				{
					in = SPC7110Decomp_dataread(thisptr);
					in_count = 8;
				}
			}

			//update processing info
			lps = (lps << 1) + flag_lps;
			inverts = (inverts << 1) + thisptr->context[con].invert;

			//update context state
			if(flag_lps & SPC7110Decomp_toggle_invert(thisptr, con))
			{
				thisptr->context[con].invert ^= 1;
			}
			if(flag_lps)
			{
				thisptr->context[con].index = SPC7110Decomp_next_lps(thisptr, con);
			}
			else if(shift)
			{
				thisptr->context[con].index = SPC7110Decomp_next_mps(thisptr, con);
			}
		}

		//save byte
		SPC7110Decomp_write(thisptr, out);
	}
}

static void SPC7110Decomp_mode1(SPC7110Decomp *thisptr, UINT8 init)
{
	static INT32 pixelorder[4], realorder[4];
	static UINT8 in, val, span;
	static INT32 out, inverts, lps, in_count;

	if(init == 1)
	{
		UINT32 i;
		for(i = 0; i < 4; i++)
		{
			pixelorder[i] = i;
		}
		out = inverts = lps = 0;
		span = 0xff;
		val = SPC7110Decomp_dataread(thisptr);
		in = SPC7110Decomp_dataread(thisptr);
		in_count = 8;
		return;
	}

	while(thisptr->decomp_buffer_length < (spc7110_decomp_buffer_size >> 1))
	{
		UINT16 data;
		UINT32 pixel;
		for(pixel = 0; pixel < 8; pixel++)
		{
			//get first symbol context
			UINT32 a = ((out >> (1 * 2)) & 3);
			UINT32 b = ((out >> (7 * 2)) & 3);
			UINT32 c = ((out >> (8 * 2)) & 3);
			UINT32 con = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);
			UINT32 bit;

			//update pixel order
			UINT32 m, n;
			for(m = 0; m < 4; m++)
			{
				if(pixelorder[m] == a)
				{
					break;
				}
			}
			for(n = m; n > 0; n--)
			{
				pixelorder[n] = pixelorder[n - 1];
			}
			pixelorder[0] = a;

			//calculate the real pixel order
			for(m = 0; m < 4; m++)
			{
				realorder[m] = pixelorder[m];
			}

			//rotate reference pixel c value to top
			for(m = 0; m < 4; m++)
			{
				if(realorder[m] == c)
				{
					break;
				}
			}
			for(n = m; n > 0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = c;

			//rotate reference pixel b value to top
			for(m = 0; m < 4; m++)
			{
				if(realorder[m] == b)
				{
					break;
				}
			}
			for(n = m; n > 0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = b;

			//rotate reference pixel a value to top
			for(m = 0; m < 4; m++)
			{
				if(realorder[m] == a)
				{
					break;
				}
			}
			for(n = m; n > 0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = a;

			//get 2 symbols
			for(bit = 0; bit < 2; bit++)
			{
				//get prob
				UINT32 prob = SPC7110Decomp_probability(thisptr, con);
				UINT32 shift = 0;

				//get symbol
				UINT32 flag_lps;
				if(val <= span - prob) //mps
				{
					span = span - prob;
					flag_lps = 0;
				}
				else //lps
				{
					val = val - (span - (prob - 1));
					span = prob - 1;
					flag_lps = 1;
				}

				//renormalize
				while(span < 0x7f)
				{
					shift++;

					span = (span << 1) + 1;
					val = (val << 1) + (in >> 7);

					in <<= 1;
					if(--in_count == 0)
					{
						in = SPC7110Decomp_dataread(thisptr);
						in_count = 8;
					}
				}

				//update processing info
				lps = (lps << 1) + flag_lps;
				inverts = (inverts << 1) + thisptr->context[con].invert;

				//update context state
				if(flag_lps & SPC7110Decomp_toggle_invert(thisptr, con))
				{
					thisptr->context[con].invert ^= 1;
				}
				if(flag_lps)
				{
					thisptr->context[con].index = SPC7110Decomp_next_lps(thisptr, con);
				}
				else if(shift)
				{
					thisptr->context[con].index = SPC7110Decomp_next_mps(thisptr, con);
				}

				//get next context
				con = 5 + (con << 1) + ((lps ^ inverts) & 1);
			}

			//get pixel
			b = realorder[(lps ^ inverts) & 3];
			out = (out << 2) + b;
		}

		//turn pixel data into bitplanes
		data = SPC7110Decomp_morton_2x8(thisptr, out);
		SPC7110Decomp_write(thisptr, data >> 8);
		SPC7110Decomp_write(thisptr, data >> 0);
	}
}

static void SPC7110Decomp_mode2(SPC7110Decomp *thisptr, UINT8 init)
{
	static INT32 pixelorder[16], realorder[16];
	static UINT8 bitplanebuffer[16], buffer_index;
	static UINT8 in, val, span;
	static INT32 out0, out1, inverts, lps, in_count;

	if(init == 1)
	{
		UINT32 i;
		for(i = 0; i < 16; i++)
		{
			pixelorder[i] = i;
		}
		buffer_index = 0;
		out0 = out1 = inverts = lps = 0;
		span = 0xff;
		val = SPC7110Decomp_dataread(thisptr);
		in = SPC7110Decomp_dataread(thisptr);
		in_count = 8;
		return;
	}

	while(thisptr->decomp_buffer_length < (spc7110_decomp_buffer_size >> 1))
	{
		UINT32 data;
		UINT32 pixel;
		for(pixel = 0; pixel < 8; pixel++)
		{
			//get first symbol context
			UINT32 a = ((out0 >> (0 * 4)) & 15);
			UINT32 b = ((out0 >> (7 * 4)) & 15);
			UINT32 c = ((out1 >> (0 * 4)) & 15);
			UINT32 con = 0;
			UINT32 refcon = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);
			UINT32 bit;

			//update pixel order
			UINT32 m, n;
			for(m = 0; m < 16; m++)
			{
				if(pixelorder[m] == a)
				{
					break;
				}
			}
			for(n = m; n >  0; n--)
			{
				pixelorder[n] = pixelorder[n - 1];
			}
			pixelorder[0] = a;

			//calculate the real pixel order
			for(m = 0; m < 16; m++)
			{
				realorder[m] = pixelorder[m];
			}

			//rotate reference pixel c value to top
			for(m = 0; m < 16; m++)
			{
				if(realorder[m] == c)
				{
					break;
				}
			}
			for(n = m; n >  0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = c;

			//rotate reference pixel b value to top
			for(m = 0; m < 16; m++)
			{
				if(realorder[m] == b)
				{
					break;
				}
			}
			for(n = m; n >  0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = b;

			//rotate reference pixel a value to top
			for(m = 0; m < 16; m++)
			{
				if(realorder[m] == a)
				{
					break;
				}
			}
			for(n = m; n >  0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = a;

			//get 4 symbols
			for(bit = 0; bit < 4; bit++)
			{
				UINT32 invertbit, shift;

				//get prob
				UINT32 prob = SPC7110Decomp_probability(thisptr, con);

				//get symbol
				UINT32 flag_lps;
				if(val <= span - prob) //mps
				{
					span = span - prob;
					flag_lps = 0;
				}
				else //lps
				{
					val = val - (span - (prob - 1));
					span = prob - 1;
					flag_lps = 1;
				}

				//renormalize
				shift = 0;
				while(span < 0x7f)
				{
					shift++;

					span = (span << 1) + 1;
					val = (val << 1) + (in >> 7);

					in <<= 1;
					if(--in_count == 0)
					{
						in = SPC7110Decomp_dataread(thisptr);
						in_count = 8;
					}
				}

				//update processing info
				lps = (lps << 1) + flag_lps;
				invertbit = thisptr->context[con].invert;
				inverts = (inverts << 1) + invertbit;

				//update context state
				if(flag_lps & SPC7110Decomp_toggle_invert(thisptr, con))
				{
					thisptr->context[con].invert ^= 1;
				}
				if(flag_lps)
				{
					thisptr->context[con].index = SPC7110Decomp_next_lps(thisptr, con);
				}
				else if(shift)
				{
					thisptr->context[con].index = SPC7110Decomp_next_mps(thisptr, con);
				}

				//get next context
				con = spc7110_mode2_context_table[con][flag_lps ^ invertbit] + (con == 1 ? refcon : 0);
			}

			//get pixel
			b = realorder[(lps ^ inverts) & 0x0f];
			out1 = (out1 << 4) + ((out0 >> 28) & 0x0f);
			out0 = (out0 << 4) + b;
		}

		//convert pixel data into bitplanes
		data = SPC7110Decomp_morton_4x8(thisptr, out0);
		SPC7110Decomp_write(thisptr, data >> 24);
		SPC7110Decomp_write(thisptr, data >> 16);
		bitplanebuffer[buffer_index++] = data >> 8;
		bitplanebuffer[buffer_index++] = data >> 0;

		if(buffer_index == 16)
		{
			UINT32 i;
			for(i = 0; i < 16; i++)
			{
				SPC7110Decomp_write(thisptr, bitplanebuffer[i]);
			}
			buffer_index = 0;
		}
	}
}

static UINT8 SPC7110Decomp_probability(SPC7110Decomp *thisptr, UINT32 n)
{
	return spc7110_evolution_table[thisptr->context[n].index][0];
}

static UINT8 SPC7110Decomp_next_lps(SPC7110Decomp *thisptr, UINT32 n)
{
	return spc7110_evolution_table[thisptr->context[n].index][1];
}

static UINT8 SPC7110Decomp_next_mps(SPC7110Decomp *thisptr, UINT32 n)
{
	return spc7110_evolution_table[thisptr->context[n].index][2];
}

static UINT8 SPC7110Decomp_toggle_invert(SPC7110Decomp *thisptr, UINT32 n)
{
	return spc7110_evolution_table[thisptr->context[n].index][3];
}

static UINT32 SPC7110Decomp_morton_2x8(SPC7110Decomp *thisptr, UINT32 data)
{
  //reverse morton lookup: de-interleave two 8-bit values
  //15, 13, 11,  9,  7,  5,  3,  1 -> 15- 8
  //14, 12, 10,  8,  6,  4,  2,  0 ->  7- 0
  return thisptr->morton16[0][(data >>  0) & 255] + thisptr->morton16[1][(data >>  8) & 255];
}

static UINT32 SPC7110Decomp_morton_4x8(SPC7110Decomp *thisptr, UINT32 data)
{
  //reverse morton lookup: de-interleave four 8-bit values
  //31, 27, 23, 19, 15, 11,  7,  3 -> 31-24
  //30, 26, 22, 18, 14, 10,  6,  2 -> 23-16
  //29, 25, 21, 17, 13,  9,  5,  1 -> 15- 8
  //28, 24, 20, 16, 12,  8,  4,  0 ->  7- 0
  return thisptr->morton32[0][(data >>  0) & 255] + thisptr->morton32[1][(data >>  8) & 255]
       + thisptr->morton32[2][(data >> 16) & 255] + thisptr->morton32[3][(data >> 24) & 255];
}

static void spc7110_mmio_write(running_machine &machine, UINT32 addr, UINT8 data);
static UINT8 spc7110_mmio_read(address_space *space, UINT32 addr);
static void spc7110_update_time(running_machine &machine, UINT8 offset);

enum RTC_State
{
	RTCS_Inactive,
	RTCS_ModeSelect,
	RTCS_IndexSelect,
	RTCS_Write
};

enum RTC_Mode
{
	RTCM_Linear = 0x03,
	RTCM_Indexed = 0x0c
};

static const UINT32 spc7110_months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct _snes_spc7110_t 
{
	//==================
	//decompression unit
	//==================
	UINT8 r4801;		// compression table low
	UINT8 r4802;		// compression table high
	UINT8 r4803;		// compression table bank
	UINT8 r4804;		// compression table index
	UINT8 r4805;		// decompression buffer index low
	UINT8 r4806;		// decompression buffer index high
	UINT8 r4807;		// ???
	UINT8 r4808;		// ???
	UINT8 r4809;		// compression length low
	UINT8 r480a;		// compression length high
	UINT8 r480b;		// decompression control register
	UINT8 r480c;		// decompression status

	SPC7110Decomp* decomp;

	UINT8 r4811;		// data pointer low
	UINT8 r4812;		// data pointer high
	UINT8 r4813;		// data pointer bank
	UINT8 r4814;		// data adjust low
	UINT8 r4815;		// data adjust high
	UINT8 r4816;		// data increment low
	UINT8 r4817;		// data increment high
	UINT8 r4818;		// data port control register

	UINT8 r481x;

	UINT8 r4814_latch;
	UINT8 r4815_latch;

	//=========
	//math unit
	//=========
	UINT8 r4820;		// 16-bit multiplicand B0, 32-bit dividend B0
	UINT8 r4821;		// 16-bit multiplicand B1, 32-bit dividend B1
	UINT8 r4822;		// 32-bit dividend B2
	UINT8 r4823;		// 32-bit dividend B3
	UINT8 r4824;		// 16-bit multiplier B0
	UINT8 r4825;		// 16-bit multiplier B1
	UINT8 r4826;		// 16-bit divisor B0
	UINT8 r4827;		// 16-bit divisor B1
	UINT8 r4828;		// 32-bit product B0, 32-bit quotient B0
	UINT8 r4829;		// 32-bit product B1, 32-bit quotient B1
	UINT8 r482a;		// 32-bit product B2, 32-bit quotient B2
	UINT8 r482b;		// 32-bit product B3, 32-bit quotient B3
	UINT8 r482c;		// 16-bit remainder B0
	UINT8 r482d;		// 16-bit remainder B1
	UINT8 r482e;		// math control register
	UINT8 r482f;		// math status

	//===================
	//memory mapping unit
	//===================
	UINT8 r4830;		// SRAM write enable
	UINT8 r4831;		// $[d0-df]:[0000-ffff] mapping
	UINT8 r4832;		// $[e0-ef]:[0000-ffff] mapping
	UINT8 r4833;		// $[f0-ff]:[0000-ffff] mapping
	UINT8 r4834;		// ???

	UINT32 dx_offset;
	UINT32 ex_offset;
	UINT32 fx_offset;

	//====================
	//real-time clock unit
	//====================
	UINT8 r4840;		// RTC latch
	UINT8 r4841;		// RTC index/data port
	UINT8 r4842;		// RTC status

	UINT32 rtc_state;
	UINT32 rtc_mode;
	UINT32 rtc_index;

	UINT64 rtc_offset;

	UINT8 rtc_ram[16];	// 0-12 secs, min, hrs, etc.; 13-14-15 control registers

	UINT32 size;
};

static _snes_spc7110_t snes_spc7110;

static void spc7110_init(running_machine& machine)
{
	snes_state *state = machine.driver_data<snes_state>();

	snes_spc7110.r4801 = 0x00;
	snes_spc7110.r4802 = 0x00;
	snes_spc7110.r4803 = 0x00;
	snes_spc7110.r4804 = 0x00;
	snes_spc7110.r4805 = 0x00;
	snes_spc7110.r4806 = 0x00;
	snes_spc7110.r4807 = 0x00;
	snes_spc7110.r4808 = 0x00;
	snes_spc7110.r4809 = 0x00;
	snes_spc7110.r480a = 0x00;
	snes_spc7110.r480b = 0x00;
	snes_spc7110.r480c = 0x00;

	snes_spc7110.r4811 = 0x00;
	snes_spc7110.r4812 = 0x00;
	snes_spc7110.r4813 = 0x00;
	snes_spc7110.r4814 = 0x00;
	snes_spc7110.r4815 = 0x00;
	snes_spc7110.r4816 = 0x00;
	snes_spc7110.r4817 = 0x00;
	snes_spc7110.r4818 = 0x00;

	snes_spc7110.r481x = 0x00;
	snes_spc7110.r4814_latch = 0;
	snes_spc7110.r4815_latch = 0;

	snes_spc7110.r4820 = 0x00;
	snes_spc7110.r4821 = 0x00;
	snes_spc7110.r4822 = 0x00;
	snes_spc7110.r4823 = 0x00;
	snes_spc7110.r4824 = 0x00;
	snes_spc7110.r4825 = 0x00;
	snes_spc7110.r4826 = 0x00;
	snes_spc7110.r4827 = 0x00;
	snes_spc7110.r4828 = 0x00;
	snes_spc7110.r4829 = 0x00;
	snes_spc7110.r482a = 0x00;
	snes_spc7110.r482b = 0x00;
	snes_spc7110.r482c = 0x00;
	snes_spc7110.r482d = 0x00;
	snes_spc7110.r482e = 0x00;
	snes_spc7110.r482f = 0x00;

	snes_spc7110.r4830 = 0x00;
	spc7110_mmio_write(machine, 0x4831, 0);
	spc7110_mmio_write(machine, 0x4832, 1);
	spc7110_mmio_write(machine, 0x4833, 2);
	snes_spc7110.r4834 = 0x00;

	snes_spc7110.r4840 = 0x00;
	snes_spc7110.r4841 = 0x00;
	snes_spc7110.r4842 = 0x00;

	snes_spc7110.size = state->m_cart_size;

	snes_spc7110.decomp = SPC7110Decomp_ctor(machine, snes_spc7110.size);
}

static void spc7110rtc_init(running_machine& machine)
{
	spc7110_init(machine);

	snes_spc7110.rtc_state = RTCS_Inactive;
	snes_spc7110.rtc_mode  = RTCM_Linear;
	snes_spc7110.rtc_index = 0;

	snes_spc7110.rtc_offset = 0;

	spc7110_update_time(machine, 0);
}

static UINT32 spc7110_datarom_addr(UINT32 addr)
{
	UINT32 size = snes_spc7110.size - 0x100000;
	while(addr >= size)
	{
		addr -= size;
	}
	return addr + 0x100000;
}

static UINT32 spc7110_data_pointer(void)
{
	return snes_spc7110.r4811 + (snes_spc7110.r4812 << 8) + (snes_spc7110.r4813 << 16);
}

static UINT32 spc7110_data_adjust(void)
{
	return snes_spc7110.r4814 + (snes_spc7110.r4815 << 8);
}

static UINT32 spc7110_data_increment(void)
{
	return snes_spc7110.r4816 + (snes_spc7110.r4817 << 8);
}

static void spc7110_set_data_pointer(UINT32 addr)
{
	snes_spc7110.r4811 = addr;
	snes_spc7110.r4812 = addr >> 8;
	snes_spc7110.r4813 = addr >> 16;
}

static void spc7110_set_data_adjust(UINT32 addr)
{
	snes_spc7110.r4814 = addr;
	snes_spc7110.r4815 = addr >> 8;
}

// FIXME: SPC7110 RTC is capable of rounding/adding/zero-ing seconds, so
// we should probably keep track internally of the time rather than updating
// to the system time at each call with a "offset" tracking as we do now...
// (and indeed current code fails to pass Tengai Makyou Zero tests)
static void spc7110_update_time(running_machine &machine, UINT8 offset)
{
	system_time curtime, *systime = &curtime;
	machine.current_datetime(curtime);
	int update = 1;

	snes_spc7110.rtc_offset += offset;

	// TEST: can we go beyond 24hrs of rounding?!? I doubt it will ever go beyond 3600, but I could be wrong...
	assert(snes_spc7110.rtc_offset < 86400);

	/* do not update if CR0 or CR2 timer disable flags are set */
	if ((snes_spc7110.rtc_ram[13] & 0x01) || (snes_spc7110.rtc_ram[15] & 0x03))
		update = 0;

	if (update)
	{
		/* update time with offset, assuming offset < 3600s */
		UINT8 second = systime->local_time.second;
		UINT8 minute = systime->local_time.minute;
		UINT8 hour = systime->local_time.hour;
		UINT8 mday = systime->local_time.mday;

		while (snes_spc7110.rtc_offset >= 3600)
		{
			snes_spc7110.rtc_offset -= 3600;
			hour++;

			if (hour == 24)
			{
				mday++;
				hour = 0;
			}
		}

		while (snes_spc7110.rtc_offset >= 60)
		{
			snes_spc7110.rtc_offset -= 60;
			minute++;

			if (minute == 60)
			{
				hour++;
				minute = 0;
			}
		}

		while (snes_spc7110.rtc_offset)
		{
			snes_spc7110.rtc_offset -= 1;
			second++;

			if (second == 60)
			{
				minute++;
				second = 0;
			}
		}

		snes_spc7110.rtc_ram[0] = second % 10;
		snes_spc7110.rtc_ram[1] = second / 10;
		snes_spc7110.rtc_ram[2] = minute % 10;
		snes_spc7110.rtc_ram[3] = minute / 10;
		snes_spc7110.rtc_ram[4] = hour % 10;
		snes_spc7110.rtc_ram[5] = hour / 10;
		snes_spc7110.rtc_ram[6] = mday % 10;
		snes_spc7110.rtc_ram[7] = mday / 10;
		snes_spc7110.rtc_ram[8] = systime->local_time.month % 10;
		snes_spc7110.rtc_ram[9] = systime->local_time.month / 10;
		snes_spc7110.rtc_ram[8] = systime->local_time.month;
		snes_spc7110.rtc_ram[10] = (systime->local_time.year - 1900) % 10;
		snes_spc7110.rtc_ram[11] = ((systime->local_time.year - 1900) / 10) % 10;
		snes_spc7110.rtc_ram[12] = systime->local_time.weekday % 7;
	}
}

static UINT8 spc7110_mmio_read(address_space *space, UINT32 addr)
{
	running_machine &machine = space->machine();
	UINT8 *ROM = machine.root_device().memregion("cart")->base();

	addr &= 0xffff;

	switch(addr)
	{
	//==================
	//decompression unit
	//==================

	case 0x4800:
		{
			UINT16 counter = (snes_spc7110.r4809 + (snes_spc7110.r480a << 8));
			counter--;
			snes_spc7110.r4809 = counter;
			snes_spc7110.r480a = counter >> 8;
			return SPC7110Decomp_read(snes_spc7110.decomp);
		}
	case 0x4801: return snes_spc7110.r4801;
	case 0x4802: return snes_spc7110.r4802;
	case 0x4803: return snes_spc7110.r4803;
	case 0x4804: return snes_spc7110.r4804;
	case 0x4805: return snes_spc7110.r4805;
	case 0x4806: return snes_spc7110.r4806;
	case 0x4807: return snes_spc7110.r4807;
	case 0x4808: return snes_spc7110.r4808;
	case 0x4809: return snes_spc7110.r4809;
	case 0x480a: return snes_spc7110.r480a;
	case 0x480b: return snes_spc7110.r480b;
	case 0x480c:
		{
			UINT8 status = snes_spc7110.r480c;
			snes_spc7110.r480c &= 0x7f;
			return status;
		}

	//==============
	//data port unit
	//==============

	case 0x4810:
		{
			UINT8 data;
			UINT32 address, adjust, adjustaddr;

			if(snes_spc7110.r481x != 0x07) return 0x00;

			address = spc7110_data_pointer();
			adjust = spc7110_data_adjust();
			if(snes_spc7110.r4818 & 8)
			{
				adjust = (INT16)adjust;  //16-bit sign extend
			}

			adjustaddr = address;
			if(snes_spc7110.r4818 & 2)
			{
				adjustaddr += adjust;
				spc7110_set_data_adjust(adjust + 1);
			}

			data = ROM[spc7110_datarom_addr(adjustaddr)];
			if(!(snes_spc7110.r4818 & 2))
			{
				UINT32 increment = (snes_spc7110.r4818 & 1) ? spc7110_data_increment() : 1;
				if(snes_spc7110.r4818 & 4)
				{
					increment = (INT16)increment;  //16-bit sign extend
				}

				if((snes_spc7110.r4818 & 16) == 0)
				{
					spc7110_set_data_pointer(address + increment);
				}
				else
				{
					spc7110_set_data_adjust(adjust + increment);
				}
			}

			return data;
		}
	case 0x4811: return snes_spc7110.r4811;
	case 0x4812: return snes_spc7110.r4812;
	case 0x4813: return snes_spc7110.r4813;
	case 0x4814: return snes_spc7110.r4814;
	case 0x4815: return snes_spc7110.r4815;
	case 0x4816: return snes_spc7110.r4816;
	case 0x4817: return snes_spc7110.r4817;
	case 0x4818: return snes_spc7110.r4818;
	case 0x481a:
		{
			UINT8 data;
			UINT32 address, adjust;
			if(snes_spc7110.r481x != 0x07)
			{
				return 0x00;
			}

			address = spc7110_data_pointer();
			adjust = spc7110_data_adjust();
			if(snes_spc7110.r4818 & 8)
			{
				adjust = (INT16)adjust;  //16-bit sign extend
			}

			data = ROM[spc7110_datarom_addr(address + adjust)];
			if((snes_spc7110.r4818 & 0x60) == 0x60)
			{
				if((snes_spc7110.r4818 & 16) == 0)
				{
					spc7110_set_data_pointer(address + adjust);
				}
				else
				{
					spc7110_set_data_adjust(adjust + adjust);
				}
			}

			return data;
		}

	//=========
	//math unit
	//=========

	case 0x4820: return snes_spc7110.r4820;
	case 0x4821: return snes_spc7110.r4821;
	case 0x4822: return snes_spc7110.r4822;
	case 0x4823: return snes_spc7110.r4823;
	case 0x4824: return snes_spc7110.r4824;
	case 0x4825: return snes_spc7110.r4825;
	case 0x4826: return snes_spc7110.r4826;
	case 0x4827: return snes_spc7110.r4827;
	case 0x4828: return snes_spc7110.r4828;
	case 0x4829: return snes_spc7110.r4829;
	case 0x482a: return snes_spc7110.r482a;
	case 0x482b: return snes_spc7110.r482b;
	case 0x482c: return snes_spc7110.r482c;
	case 0x482d: return snes_spc7110.r482d;
	case 0x482e: return snes_spc7110.r482e;
	case 0x482f:
		{
			UINT8 status = snes_spc7110.r482f;
			snes_spc7110.r482f &= 0x7f;
			return status;
		}

	//===================
	//memory mapping unit
	//===================

	case 0x4830: return snes_spc7110.r4830;
	case 0x4831: return snes_spc7110.r4831;
	case 0x4832: return snes_spc7110.r4832;
	case 0x4833: return snes_spc7110.r4833;
	case 0x4834: return snes_spc7110.r4834;

	//====================
	//real-time clock unit
	//====================
	case 0x4840: return snes_spc7110.r4840;
	case 0x4841:
		{
			UINT8 data = 0;
			if (snes_spc7110.rtc_state == RTCS_Inactive || snes_spc7110.rtc_state == RTCS_ModeSelect)
				return 0x00;

			snes_spc7110.r4842 = 0x80;
			data = snes_spc7110.rtc_ram[snes_spc7110.rtc_index];
			snes_spc7110.rtc_index = (snes_spc7110.rtc_index + 1) & 15;
			return data;
		}
	case 0x4842:
		{
			UINT8 status = snes_spc7110.r4842;
			snes_spc7110.r4842 &= 0x7f;
			return status;
		}
	}

	return snes_open_bus_r(space, 0);
}

static void spc7110_mmio_write(running_machine &machine, UINT32 addr, UINT8 data)
{
	UINT8 *ROM = machine.root_device().memregion("cart")->base();

	addr &= 0xffff;

	switch(addr)
	{
	//==================
	//decompression unit
	//==================

	case 0x4801: snes_spc7110.r4801 = data; break;
	case 0x4802: snes_spc7110.r4802 = data; break;
	case 0x4803: snes_spc7110.r4803 = data; break;
	case 0x4804: snes_spc7110.r4804 = data; break;
	case 0x4805: snes_spc7110.r4805 = data; break;
	case 0x4806:
		{
			UINT32 table, index, address, mode, offset;
			snes_spc7110.r4806 = data;

			table   = (snes_spc7110.r4801 + (snes_spc7110.r4802 << 8) + (snes_spc7110.r4803 << 16));
			index   = (snes_spc7110.r4804 << 2);
			//length  = (snes_spc7110.r4809 + (snes_spc7110.r480a << 8));
			address = spc7110_datarom_addr(table + index);
			mode    = (ROM[address + 0]);
			offset  = (ROM[address + 1] << 16)
                    + (ROM[address + 2] <<  8)
                    + (ROM[address + 3] <<  0);

			SPC7110Decomp_init(snes_spc7110.decomp, machine, mode, offset, (snes_spc7110.r4805 + (snes_spc7110.r4806 << 8)) << mode);
			snes_spc7110.r480c = 0x80;
		}
		break;

	case 0x4807: snes_spc7110.r4807 = data; break;
	case 0x4808: snes_spc7110.r4808 = data; break;
	case 0x4809: snes_spc7110.r4809 = data; break;
	case 0x480a: snes_spc7110.r480a = data; break;
	case 0x480b: snes_spc7110.r480b = data; break;

	//==============
	//data port unit
	//==============

	case 0x4811: snes_spc7110.r4811 = data; snes_spc7110.r481x |= 0x01; break;
	case 0x4812: snes_spc7110.r4812 = data; snes_spc7110.r481x |= 0x02; break;
	case 0x4813: snes_spc7110.r4813 = data; snes_spc7110.r481x |= 0x04; break;
	case 0x4814:
		{
			snes_spc7110.r4814 = data;
			snes_spc7110.r4814_latch = 1;
			if(!snes_spc7110.r4815_latch)
			{
				break;
			}
			if(!(snes_spc7110.r4818 & 2))
			{
				break;
			}
			if(snes_spc7110.r4818 & 0x10)
			{
				break;
			}

			if((snes_spc7110.r4818 & 0x60) == 0x20)
			{
				UINT32 increment = spc7110_data_adjust() & 0xff;
				if(snes_spc7110.r4818 & 8)
				{
					increment = (INT8)increment;  //8-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			else if((snes_spc7110.r4818 & 0x60) == 0x40)
			{
				UINT32 increment = spc7110_data_adjust();
				if(snes_spc7110.r4818 & 8)
				{
					increment = (INT16)increment;  //16-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			break;
		}

	case 0x4815:
		{
			snes_spc7110.r4815 = data;
			snes_spc7110.r4815_latch = 1;
			if(!snes_spc7110.r4814_latch)
			{
				break;
			}
			if(!(snes_spc7110.r4818 & 2))
			{
				break;
			}
			if(snes_spc7110.r4818 & 0x10)
			{
				break;
			}

			if((snes_spc7110.r4818 & 0x60) == 0x20)
			{
				UINT32 increment = spc7110_data_adjust() & 0xff;
				if(snes_spc7110.r4818 & 8)
				{
					increment = (INT8)increment;  //8-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			else if((snes_spc7110.r4818 & 0x60) == 0x40)
			{
				UINT32 increment = spc7110_data_adjust();
				if(snes_spc7110.r4818 & 8)
				{
					increment = (INT16)increment;  //16-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			break;
		}

	case 0x4816: snes_spc7110.r4816 = data; break;
	case 0x4817: snes_spc7110.r4817 = data; break;
	case 0x4818:
		{
    			if(snes_spc7110.r481x != 0x07)
				break;

	    		snes_spc7110.r4818 = data;
    			snes_spc7110.r4814_latch = snes_spc7110.r4815_latch = 0;
    			break;
    		}

	//=========
	//math unit
	//=========

	case 0x4820: snes_spc7110.r4820 = data; break;
	case 0x4821: snes_spc7110.r4821 = data; break;
	case 0x4822: snes_spc7110.r4822 = data; break;
	case 0x4823: snes_spc7110.r4823 = data; break;
	case 0x4824: snes_spc7110.r4824 = data; break;
	case 0x4825:
		{
	    		snes_spc7110.r4825 = data;

    			if(snes_spc7110.r482e & 1)
			{
    				//signed 16-bit x 16-bit multiplication
    				INT16 r0 = (INT16)(snes_spc7110.r4824 + (snes_spc7110.r4825 << 8));
    				INT16 r1 = (INT16)(snes_spc7110.r4820 + (snes_spc7110.r4821 << 8));

	    			INT32 result = r0 * r1;
	    			snes_spc7110.r4828 = result;
	    			snes_spc7110.r4829 = result >> 8;
	    			snes_spc7110.r482a = result >> 16;
	    			snes_spc7110.r482b = result >> 24;
			}
			else
			{
	    			//unsigned 16-bit x 16-bit multiplication
	    			UINT16 r0 = (UINT16)(snes_spc7110.r4824 + (snes_spc7110.r4825 << 8));
	    			UINT16 r1 = (UINT16)(snes_spc7110.r4820 + (snes_spc7110.r4821 << 8));

	    			UINT32 result = r0 * r1;
    				snes_spc7110.r4828 = result;
    				snes_spc7110.r4829 = result >> 8;
    				snes_spc7110.r482a = result >> 16;
    				snes_spc7110.r482b = result >> 24;
			}

			snes_spc7110.r482f = 0x80;
			break;
		}

	case 0x4826: snes_spc7110.r4826 = data; break;
	case 0x4827:
		{
			snes_spc7110.r4827 = data;

			if(snes_spc7110.r482e & 1)
			{
				//signed 32-bit x 16-bit division
				INT32 dividend = (INT32)(snes_spc7110.r4820 + (snes_spc7110.r4821 << 8) + (snes_spc7110.r4822 << 16) + (snes_spc7110.r4823 << 24));
				INT16 divisor  = (INT16)(snes_spc7110.r4826 + (snes_spc7110.r4827 << 8));

				INT32 quotient;
				INT16 remainder;

				if(divisor)
				{
					quotient  = (INT32)(dividend / divisor);
					remainder = (INT32)(dividend % divisor);
				}
				else
				{
					//illegal division by zero
					quotient  = 0;
					remainder = dividend & 0xffff;
				}

				snes_spc7110.r4828 = quotient;
				snes_spc7110.r4829 = quotient >> 8;
				snes_spc7110.r482a = quotient >> 16;
				snes_spc7110.r482b = quotient >> 24;

				snes_spc7110.r482c = remainder;
				snes_spc7110.r482d = remainder >> 8;
			}
			else
			{
				//unsigned 32-bit x 16-bit division
				UINT32 dividend = (UINT32)(snes_spc7110.r4820 + (snes_spc7110.r4821 << 8) + (snes_spc7110.r4822 << 16) + (snes_spc7110.r4823 << 24));
				UINT16 divisor  = (UINT16)(snes_spc7110.r4826 + (snes_spc7110.r4827 << 8));

				UINT32 quotient;
				UINT16 remainder;

				if(divisor)
				{
					quotient  = (UINT32)(dividend / divisor);
					remainder = (UINT16)(dividend % divisor);
				}
				else
				{
					//illegal division by zero
					quotient  = 0;
					remainder = dividend & 0xffff;
				}

				snes_spc7110.r4828 = quotient;
				snes_spc7110.r4829 = quotient >> 8;
				snes_spc7110.r482a = quotient >> 16;
				snes_spc7110.r482b = quotient >> 24;

				snes_spc7110.r482c = remainder;
				snes_spc7110.r482d = remainder >> 8;
			}

			snes_spc7110.r482f = 0x80;
			break;
		}

	case 0x482e:
		{
			//reset math unit
			snes_spc7110.r4820 = snes_spc7110.r4821 = snes_spc7110.r4822 = snes_spc7110.r4823 = 0;
			snes_spc7110.r4824 = snes_spc7110.r4825 = snes_spc7110.r4826 = snes_spc7110.r4827 = 0;
			snes_spc7110.r4828 = snes_spc7110.r4829 = snes_spc7110.r482a = snes_spc7110.r482b = 0;
			snes_spc7110.r482c = snes_spc7110.r482d = 0;

			snes_spc7110.r482e = data;
			break;
		}

	//===================
	//memory mapping unit
	//===================

	case 0x4830: snes_spc7110.r4830 = data; break;

	case 0x4831:
		{
			snes_spc7110.r4831 = data;
			snes_spc7110.dx_offset = spc7110_datarom_addr(data * 0x100000);
			break;
		}

	case 0x4832:
		{
			snes_spc7110.r4832 = data;
			snes_spc7110.ex_offset = spc7110_datarom_addr(data * 0x100000);
			break;
		}

	case 0x4833:
		{
			snes_spc7110.r4833 = data;
			snes_spc7110.fx_offset = spc7110_datarom_addr(data * 0x100000);
			break;
		}

	case 0x4834: snes_spc7110.r4834 = data; break;

	//====================
	//real-time clock unit
	//====================

	case 0x4840:
		{
			snes_spc7110.r4840 = data;

			if (!(snes_spc7110.r4840 & 1))
			{
				//disable RTC
				snes_spc7110.rtc_state = RTCS_Inactive;
				spc7110_update_time(machine, 0);
			}
			else
			{
				//enable RTC
				snes_spc7110.r4842 = 0x80;
				snes_spc7110.rtc_state = RTCS_ModeSelect;
			}
		}
		break;

	case 0x4841:
		{
			snes_spc7110.r4841 = data;

			switch (snes_spc7110.rtc_state)
			{
			case RTCS_ModeSelect:
				if (data == RTCM_Linear || data == RTCM_Indexed)
				{
					snes_spc7110.r4842 = 0x80;
					snes_spc7110.rtc_state = RTCS_IndexSelect;
					snes_spc7110.rtc_mode = (RTC_Mode)data;
					snes_spc7110.rtc_index = 0;
				}
				break;

			case RTCS_IndexSelect:
				snes_spc7110.r4842 = 0x80;
				snes_spc7110.rtc_index = data & 15;
				if (snes_spc7110.rtc_mode == RTCM_Linear)
					snes_spc7110.rtc_state = RTCS_Write;
				break;

			case RTCS_Write:
				snes_spc7110.r4842 = 0x80;

				//control register 0
				if (snes_spc7110.rtc_index == 13)
				{
					//increment second counter
					if (data & 2)
						spc7110_update_time(machine, 1);

					//round minute counter
					if (data & 8)
					{
						spc7110_update_time(machine, 0);

						UINT8 second = snes_spc7110.rtc_ram[0] + snes_spc7110.rtc_ram[1] * 10;
						//clear seconds
						snes_spc7110.rtc_ram[0] = 0;
						snes_spc7110.rtc_ram[1] = 0;

						if (second >= 30)
							spc7110_update_time(machine, 60);
					}
				}

				//control register 2
				if (snes_spc7110.rtc_index == 15)
				{
					//disable timer and clear second counter
					if ((data & 1) && !(snes_spc7110.rtc_ram[15]  & 1))
					{
						spc7110_update_time(machine, 0);

						//clear seconds
						snes_spc7110.rtc_ram[0] = 0;
						snes_spc7110.rtc_ram[1] = 0;
					}

					//disable timer
					if((data & 2) && !(snes_spc7110.rtc_ram[15] & 2))
						spc7110_update_time(machine, 0);
				}

				snes_spc7110.rtc_ram[snes_spc7110.rtc_index] = data & 15;
				snes_spc7110.rtc_index = (snes_spc7110.rtc_index + 1) & 15;
				break;
			}
		}
		break;
	}
}

static UINT8 spc7110_bank7_read(address_space *space, UINT32 offset)
{
	UINT8 *ROM = space->machine().root_device().memregion("cart")->base();
	UINT32 addr = offset & 0x0fffff;

	switch (offset & 0xf00000)
	{
	case 0x100000:
		return ROM[snes_spc7110.dx_offset + addr];
	case 0x200000:
		return ROM[snes_spc7110.ex_offset + addr];
	case 0x300000:
		return ROM[snes_spc7110.fx_offset + addr];
	default:
		break;
	}
	return snes_open_bus_r(space, 0);
}
