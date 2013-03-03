/***********************************************************************************************************
 
 SPC-7110 add-on chip emulation (for SNES/SFC)
 
 Based on C++ implementation by Byuu in BSNES.
 
 Byuu's code is released under GNU General Public License
 version 2 as published by the Free Software Foundation.

 The implementation below is released under the MAME license
 for use in MAME, MESS and derivatives by permission of Byuu
 
 Copyright (for the implementation below) MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.
 
 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sns_spc7110.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type SNS_HIROM_SPC7110 = &device_creator<sns_rom_spc7110_device>;
const device_type SNS_HIROM_SPC7110_RTC = &device_creator<sns_rom_spc7110rtc_device>;


sns_rom_spc7110_device::sns_rom_spc7110_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
			: sns_rom21_device(mconfig, type, name, tag, owner, clock)
{
}

sns_rom_spc7110_device::sns_rom_spc7110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: sns_rom21_device(mconfig, SNS_HIROM_SPC7110, "SNES Cart + SPC-7110", tag, owner, clock)
{
}

sns_rom_spc7110rtc_device::sns_rom_spc7110rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: sns_rom_spc7110_device(mconfig, SNS_HIROM_SPC7110_RTC, "SNES Cart + SPC-7110 + RTC", tag, owner, clock)
{
}


void sns_rom_spc7110_device::spc7110_start()
{
	memset(m_ram, 0, sizeof(m_ram));
	m_r4801 = 0x00;
	m_r4802 = 0x00;
	m_r4803 = 0x00;
	m_r4804 = 0x00;
	m_r4805 = 0x00;
	m_r4806 = 0x00;
	m_r4807 = 0x00;
	m_r4808 = 0x00;
	m_r4809 = 0x00;
	m_r480a = 0x00;
	m_r480b = 0x00;
	m_r480c = 0x00;
	
	m_r4811 = 0x00;
	m_r4812 = 0x00;
	m_r4813 = 0x00;
	m_r4814 = 0x00;
	m_r4815 = 0x00;
	m_r4816 = 0x00;
	m_r4817 = 0x00;
	m_r4818 = 0x00;
	
	m_r481x = 0x00;
	m_r4814_latch = 0;
	m_r4815_latch = 0;
	
	m_r4820 = 0x00;
	m_r4821 = 0x00;
	m_r4822 = 0x00;
	m_r4823 = 0x00;
	m_r4824 = 0x00;
	m_r4825 = 0x00;
	m_r4826 = 0x00;
	m_r4827 = 0x00;
	m_r4828 = 0x00;
	m_r4829 = 0x00;
	m_r482a = 0x00;
	m_r482b = 0x00;
	m_r482c = 0x00;
	m_r482d = 0x00;
	m_r482e = 0x00;
	m_r482f = 0x00;
	
	m_r4830 = 0x00;
	m_r4831 = 0;
	m_dx_offset = spc7110_datarom_addr(0 * 0x100000, 0x200000);	// we would need the rom length here...
	m_r4832 = 1;
	m_ex_offset = spc7110_datarom_addr(1 * 0x100000, 0x200000);	// we would need the rom length here...
	m_r4833 = 2;
	m_fx_offset = spc7110_datarom_addr(2 * 0x100000, 0x200000);	// we would need the rom length here...
	m_r4834 = 0x00;
	
	m_r4840 = 0x00;
	m_r4841 = 0x00;
	m_r4842 = 0x00;
	
	m_decomp = auto_alloc(machine(), SPC7110_Decomp(machine()));

	save_item(NAME(m_ram));
	save_item(NAME(m_r4801));
	save_item(NAME(m_r4802));
	save_item(NAME(m_r4803));
	save_item(NAME(m_r4804));
	save_item(NAME(m_r4805));
	save_item(NAME(m_r4806));
	save_item(NAME(m_r4807));
	save_item(NAME(m_r4808));
	save_item(NAME(m_r4809));
	save_item(NAME(m_r480a));
	save_item(NAME(m_r480b));
	save_item(NAME(m_r480c));
	save_item(NAME(m_r4811));
	save_item(NAME(m_r4812));
	save_item(NAME(m_r4813));
	save_item(NAME(m_r4814));
	save_item(NAME(m_r4815));
	save_item(NAME(m_r4816));
	save_item(NAME(m_r4817));
	save_item(NAME(m_r4818));
	save_item(NAME(m_r481x));
	save_item(NAME(m_r4814_latch));
	save_item(NAME(m_r4815_latch));
	save_item(NAME(m_r4820));
	save_item(NAME(m_r4821));
	save_item(NAME(m_r4822));
	save_item(NAME(m_r4823));
	save_item(NAME(m_r4824));
	save_item(NAME(m_r4825));
	save_item(NAME(m_r4826));
	save_item(NAME(m_r4827));
	save_item(NAME(m_r4828));
	save_item(NAME(m_r4829));
	save_item(NAME(m_r482a));
	save_item(NAME(m_r482b));
	save_item(NAME(m_r482c));
	save_item(NAME(m_r482d));
	save_item(NAME(m_r482e));
	save_item(NAME(m_r482f));
	save_item(NAME(m_r4830));
	save_item(NAME(m_r4831));
	save_item(NAME(m_r4832));
	save_item(NAME(m_r4833));
	save_item(NAME(m_r4834));
	save_item(NAME(m_r4840));
	save_item(NAME(m_r4841));
	save_item(NAME(m_r4842));
	save_item(NAME(m_dx_offset));
	save_item(NAME(m_ex_offset));
	save_item(NAME(m_fx_offset));
	// TODO: save decomp-related items and fix their restore...
}

void sns_rom_spc7110_device::device_start()
{
	memset(rom_bank_map, 0, sizeof(rom_bank_map));

	spc7110_start();
}

void sns_rom_spc7110rtc_device::device_start()
{
	memset(rom_bank_map, 0, sizeof(rom_bank_map));
	
	spc7110_start();

	// RTC
	m_rtc_state = RTCS_Inactive;
	m_rtc_mode  = RTCM_Linear;
	m_rtc_index = 0;
	m_rtc_offset = 0;

// at this stage, rtc_ram is not yet allocated. this will be fixed when converting RTC to be a separate device.
//	spc7110_update_time(0);
	
	save_item(NAME(m_rtc_state));
	save_item(NAME(m_rtc_mode));
	save_item(NAME(m_rtc_index));
	save_item(NAME(m_rtc_offset));
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

#define SPC7110_DECOMP_BUFFER_SIZE 64

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

SPC7110_Decomp::SPC7110_Decomp(running_machine &machine)
				:  m_machine(machine)
{
	m_decomp_buffer = (UINT8*)auto_alloc_array(machine, UINT8, SPC7110_DECOMP_BUFFER_SIZE);
	reset();
	
	for (int i = 0; i < 256; i++)
	{
#define map(x, y) (((i >> x) & 1) << y)
		//2x8-bit
		m_morton16[1][i] = map(7, 15) + map(6,  7) + map(5, 14) + map(4,  6)
		+ map(3, 13) + map(2,  5) + map(1, 12) + map(0,  4);
		m_morton16[0][i] = map(7, 11) + map(6,  3) + map(5, 10) + map(4,  2)
		+ map(3,  9) + map(2,  1) + map(1,  8) + map(0,  0);
		//4x8-bit
		m_morton32[3][i] = map(7, 31) + map(6, 23) + map(5, 15) + map(4,  7)
		+ map(3, 30) + map(2, 22) + map(1, 14) + map(0,  6);
		m_morton32[2][i] = map(7, 29) + map(6, 21) + map(5, 13) + map(4,  5)
		+ map(3, 28) + map(2, 20) + map(1, 12) + map(0,  4);
		m_morton32[1][i] = map(7, 27) + map(6, 19) + map(5, 11) + map(4,  3)
		+ map(3, 26) + map(2, 18) + map(1, 10) + map(0,  2);
		m_morton32[0][i] = map(7, 25) + map(6, 17) + map(5,  9) + map(4,  1)
		+ map(3, 24) + map(2, 16) + map(1,  8) + map(0,  0);
#undef map
	}
}

void SPC7110_Decomp::reset()
{
	//mode 3 is invalid; this is treated as a special case to always return 0x00
	//set to mode 3 so that reading decomp port before starting first decomp will return 0x00
	m_decomp_mode = 3;
	
	m_decomp_buffer_rdoffset = 0;
	m_decomp_buffer_wroffset = 0;
	m_decomp_buffer_length   = 0;
}

void SPC7110_Decomp::init(running_machine &machine, UINT8 *ROM, UINT32 len, UINT32 mode, UINT32 offset, UINT32 index)
{
	m_decomp_mode = mode;
	m_decomp_offset = offset;
	
	m_decomp_buffer_rdoffset = 0;
	m_decomp_buffer_wroffset = 0;
	m_decomp_buffer_length   = 0;
	
	//reset context states
	for (int i = 0; i < 32; i++)
	{
		m_context[i].index  = 0;
		m_context[i].invert = 0;
	}
	
	switch (m_decomp_mode)
	{
		case 0: mode0(1, ROM, len); break;
		case 1: mode1(1, ROM, len); break;
		case 2: mode2(1, ROM, len); break;
	}
	
	//decompress up to requested output data index
	while (index--)
	{
		read(ROM, len);
	}
}

UINT8 SPC7110_Decomp::read(UINT8 *ROM, UINT32 len)
{
	UINT8 data;
	
	if (m_decomp_buffer_length == 0)
	{
		//decompress at least (SPC7110_DECOMP_BUFFER_SIZE / 2) bytes to the buffer
		switch (m_decomp_mode)
		{
			case 0:
				mode0(0, ROM, len);
				break;
				
			case 1:
				mode1(0, ROM, len);
				break;
				
			case 2:
				mode2(0, ROM, len);
				break;
				
			default:
				return 0x00;
		}
	}
	
	data = m_decomp_buffer[m_decomp_buffer_rdoffset++];
	m_decomp_buffer_rdoffset &= SPC7110_DECOMP_BUFFER_SIZE - 1;
	m_decomp_buffer_length--;
	return data;
}

void SPC7110_Decomp::write(UINT8 data)
{
	m_decomp_buffer[m_decomp_buffer_wroffset++] = data;
	m_decomp_buffer_wroffset &= SPC7110_DECOMP_BUFFER_SIZE - 1;
	m_decomp_buffer_length++;
}

UINT8 SPC7110_Decomp::dataread(UINT8 *ROM, UINT32 len)
{
	UINT32 size = len - 0x100000;
	while (m_decomp_offset >= size)
	{
		m_decomp_offset -= size;
	}
	return ROM[0x100000 + m_decomp_offset++];
}

void SPC7110_Decomp::mode0(UINT8 init, UINT8 *ROM, UINT32 len)
{
	static UINT8 val, in, span;
	static INT32 out, inverts, lps, in_count;
	
	if (init == 1)
	{
		out = inverts = lps = 0;
		span = 0xff;
		val = dataread(ROM, len);
		in = dataread(ROM, len);
		in_count = 8;
		return;
	}
	
	while (m_decomp_buffer_length < (SPC7110_DECOMP_BUFFER_SIZE >> 1))
	{
		for (int bit = 0; bit < 8; bit++)
		{
			//get context
			UINT8 mask = (1 << (bit & 3)) - 1;
			UINT8 con = mask + ((inverts & mask) ^ (lps & mask));
			UINT32 prob, mps, flag_lps;
			UINT32 shift = 0;
			if (bit > 3)
			{
				con += 15;
			}
			
			//get prob and mps
			prob = probability(con);
			mps = (((out >> 15) & 1) ^ m_context[con].invert);
			
			//get bit
			if (val <= span - prob) //mps
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
			while (span < 0x7f)
			{
				shift++;
				
				span = (span << 1) + 1;
				val = (val << 1) + (in >> 7);
				
				in <<= 1;
				if (--in_count == 0)
				{
					in = dataread(ROM, len);
					in_count = 8;
				}
			}
			
			//update processing info
			lps = (lps << 1) + flag_lps;
			inverts = (inverts << 1) + m_context[con].invert;
			
			//update context state
			if (flag_lps & toggle_invert(con))
			{
				m_context[con].invert ^= 1;
			}
			if (flag_lps)
			{
				m_context[con].index = next_lps(con);
			}
			else if (shift)
			{
				m_context[con].index = next_mps(con);
			}
		}
		
		//save byte
		write(out);
	}
}

void SPC7110_Decomp::mode1(UINT8 init, UINT8 *ROM, UINT32 len)
{
	static INT32 pixelorder[4], realorder[4];
	static UINT8 in, val, span;
	static INT32 out, inverts, lps, in_count;
	
	if (init == 1)
	{
		for (int i = 0; i < 4; i++)
		{
			pixelorder[i] = i;
		}
		out = inverts = lps = 0;
		span = 0xff;
		val = dataread(ROM, len);
		in = dataread(ROM, len);
		in_count = 8;
		return;
	}
	
	while (m_decomp_buffer_length < (SPC7110_DECOMP_BUFFER_SIZE >> 1))
	{
		UINT16 data;
		for (int pixel = 0; pixel < 8; pixel++)
		{
			//get first symbol context
			UINT32 a = ((out >> (1 * 2)) & 3);
			UINT32 b = ((out >> (7 * 2)) & 3);
			UINT32 c = ((out >> (8 * 2)) & 3);
			UINT32 con = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);
			
			//update pixel order
			UINT32 m, n;
			for (m = 0; m < 4; m++)
			{
				if (pixelorder[m] == a)
				{
					break;
				}
			}
			for (n = m; n > 0; n--)
			{
				pixelorder[n] = pixelorder[n - 1];
			}
			pixelorder[0] = a;
			
			//calculate the real pixel order
			for (m = 0; m < 4; m++)
			{
				realorder[m] = pixelorder[m];
			}
			
			//rotate reference pixel c value to top
			for (m = 0; m < 4; m++)
			{
				if (realorder[m] == c)
				{
					break;
				}
			}
			for (n = m; n > 0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = c;
			
			//rotate reference pixel b value to top
			for (m = 0; m < 4; m++)
			{
				if (realorder[m] == b)
				{
					break;
				}
			}
			for (n = m; n > 0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = b;
			
			//rotate reference pixel a value to top
			for (m = 0; m < 4; m++)
			{
				if (realorder[m] == a)
				{
					break;
				}
			}
			for (n = m; n > 0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = a;
			
			//get 2 symbols
			for (int bit = 0; bit < 2; bit++)
			{
				//get prob
				UINT32 prob = probability(con);
				UINT32 shift = 0;
				
				//get symbol
				UINT32 flag_lps;
				if (val <= span - prob) //mps
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
				while (span < 0x7f)
				{
					shift++;
					
					span = (span << 1) + 1;
					val = (val << 1) + (in >> 7);
					
					in <<= 1;
					if (--in_count == 0)
					{
						in = dataread(ROM, len);
						in_count = 8;
					}
				}
				
				//update processing info
				lps = (lps << 1) + flag_lps;
				inverts = (inverts << 1) + m_context[con].invert;
				
				//update context state
				if (flag_lps & toggle_invert(con))
				{
					m_context[con].invert ^= 1;
				}
				if (flag_lps)
				{
					m_context[con].index = next_lps(con);
				}
				else if (shift)
				{
					m_context[con].index = next_mps(con);
				}
				
				//get next context
				con = 5 + (con << 1) + ((lps ^ inverts) & 1);
			}
			
			//get pixel
			b = realorder[(lps ^ inverts) & 3];
			out = (out << 2) + b;
		}
		
		//turn pixel data into bitplanes
		data = morton_2x8(out);
		write(data >> 8);
		write(data >> 0);
	}
}

void SPC7110_Decomp::mode2(UINT8 init, UINT8 *ROM, UINT32 len)
{
	static INT32 pixelorder[16], realorder[16];
	static UINT8 bitplanebuffer[16], buffer_index;
	static UINT8 in, val, span;
	static INT32 out0, out1, inverts, lps, in_count;
	
	if (init == 1)
	{
		for (int i = 0; i < 16; i++)
		{
			pixelorder[i] = i;
		}
		buffer_index = 0;
		out0 = out1 = inverts = lps = 0;
		span = 0xff;
		val = dataread(ROM, len);
		in = dataread(ROM, len);
		in_count = 8;
		return;
	}
	
	while (m_decomp_buffer_length < (SPC7110_DECOMP_BUFFER_SIZE >> 1))
	{
		UINT32 data;
		for (int pixel = 0; pixel < 8; pixel++)
		{
			//get first symbol context
			UINT32 a = ((out0 >> (0 * 4)) & 15);
			UINT32 b = ((out0 >> (7 * 4)) & 15);
			UINT32 c = ((out1 >> (0 * 4)) & 15);
			UINT32 con = 0;
			UINT32 refcon = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);
			
			//update pixel order
			UINT32 m, n;
			for (m = 0; m < 16; m++)
			{
				if (pixelorder[m] == a)
				{
					break;
				}
			}
			for (n = m; n >  0; n--)
			{
				pixelorder[n] = pixelorder[n - 1];
			}
			pixelorder[0] = a;
			
			//calculate the real pixel order
			for (m = 0; m < 16; m++)
			{
				realorder[m] = pixelorder[m];
			}
			
			//rotate reference pixel c value to top
			for (m = 0; m < 16; m++)
			{
				if (realorder[m] == c)
				{
					break;
				}
			}
			for (n = m; n >  0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = c;
			
			//rotate reference pixel b value to top
			for (m = 0; m < 16; m++)
			{
				if (realorder[m] == b)
				{
					break;
				}
			}
			for (n = m; n >  0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = b;
			
			//rotate reference pixel a value to top
			for (m = 0; m < 16; m++)
			{
				if (realorder[m] == a)
				{
					break;
				}
			}
			for (n = m; n >  0; n--)
			{
				realorder[n] = realorder[n - 1];
			}
			realorder[0] = a;
			
			//get 4 symbols
			for (int bit = 0; bit < 4; bit++)
			{
				UINT32 invertbit, shift;
				
				//get prob
				UINT32 prob = probability(con);
				
				//get symbol
				UINT32 flag_lps;
				if (val <= span - prob) //mps
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
				while (span < 0x7f)
				{
					shift++;
					
					span = (span << 1) + 1;
					val = (val << 1) + (in >> 7);
					
					in <<= 1;
					if (--in_count == 0)
					{
						in = dataread(ROM, len);
						in_count = 8;
					}
				}
				
				//update processing info
				lps = (lps << 1) + flag_lps;
				invertbit = m_context[con].invert;
				inverts = (inverts << 1) + invertbit;
				
				//update context state
				if (flag_lps & toggle_invert(con))
				{
					m_context[con].invert ^= 1;
				}
				if (flag_lps)
				{
					m_context[con].index = next_lps(con);
				}
				else if (shift)
				{
					m_context[con].index = next_mps(con);
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
		data = morton_4x8(out0);
		write(data >> 24);
		write(data >> 16);
		bitplanebuffer[buffer_index++] = data >> 8;
		bitplanebuffer[buffer_index++] = data >> 0;
		
		if (buffer_index == 16)
		{
			for (int i = 0; i < 16; i++)
			{
				write(bitplanebuffer[i]);
			}
			buffer_index = 0;
		}
	}
}

UINT8 SPC7110_Decomp::probability(UINT32 n)
{
	return spc7110_evolution_table[m_context[n].index][0];
}

UINT8 SPC7110_Decomp::next_lps(UINT32 n)
{
	return spc7110_evolution_table[m_context[n].index][1];
}

UINT8 SPC7110_Decomp::next_mps(UINT32 n)
{
	return spc7110_evolution_table[m_context[n].index][2];
}

UINT8 SPC7110_Decomp::toggle_invert(UINT32 n)
{
	return spc7110_evolution_table[m_context[n].index][3];
}

UINT32 SPC7110_Decomp::morton_2x8(UINT32 data)
{
	//reverse morton lookup: de-interleave two 8-bit values
	//15, 13, 11,  9,  7,  5,  3,  1 -> 15- 8
	//14, 12, 10,  8,  6,  4,  2,  0 ->  7- 0
	return m_morton16[0][(data >>  0) & 255] + m_morton16[1][(data >>  8) & 255];
}

UINT32 SPC7110_Decomp::morton_4x8(UINT32 data)
{
	//reverse morton lookup: de-interleave four 8-bit values
	//31, 27, 23, 19, 15, 11,  7,  3 -> 31-24
	//30, 26, 22, 18, 14, 10,  6,  2 -> 23-16
	//29, 25, 21, 17, 13,  9,  5,  1 -> 15- 8
	//28, 24, 20, 16, 12,  8,  4,  0 ->  7- 0
	return m_morton32[0][(data >>  0) & 255] + m_morton32[1][(data >>  8) & 255]
	+ m_morton32[2][(data >> 16) & 255] + m_morton32[3][(data >> 24) & 255];
}


static const UINT32 spc7110_months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

UINT32 sns_rom_spc7110_device::spc7110_datarom_addr(UINT32 addr, UINT32 rom_len)
{
	UINT32 size = rom_len - 0x100000;
	while (addr >= size)
	{
		addr -= size;
	}
	return addr + 0x100000;
}

UINT32 sns_rom_spc7110_device::spc7110_data_pointer(void)
{
	return m_r4811 + (m_r4812 << 8) + (m_r4813 << 16);
}

UINT32 sns_rom_spc7110_device::spc7110_data_adjust(void)
{
	return m_r4814 + (m_r4815 << 8);
}

UINT32 sns_rom_spc7110_device::spc7110_data_increment(void)
{
	return m_r4816 + (m_r4817 << 8);
}

void sns_rom_spc7110_device::spc7110_set_data_pointer(UINT32 addr)
{
	m_r4811 = addr;
	m_r4812 = addr >> 8;
	m_r4813 = addr >> 16;
}

void sns_rom_spc7110_device::spc7110_set_data_adjust(UINT32 addr)
{
	m_r4814 = addr;
	m_r4815 = addr >> 8;
}

// FIXME: SPC7110 RTC is capable of rounding/adding/zero-ing seconds, so
// we should probably keep track internally of the time rather than updating
// to the system time at each call with a "offset" tracking as we do now...
// (and indeed current code fails to pass Tengai Makyou Zero tests)
void sns_rom_spc7110_device::spc7110_update_time(UINT8 offset)
{
	system_time curtime, *systime = &curtime;
	machine().current_datetime(curtime);
	int update = 1;
	
	m_rtc_offset += offset;
	
	// TEST: can we go beyond 24hrs of rounding?!? I doubt it will ever go beyond 3600, but I could be wrong...
	assert(m_rtc_offset < 86400);
	
	/* do not update if CR0 or CR2 timer disable flags are set */
	if ((m_rtc_ram[13] & 0x01) || (m_rtc_ram[15] & 0x03))
		update = 0;
	
	if (update)
	{
		/* update time with offset, assuming offset < 3600s */
		UINT8 second = systime->local_time.second;
		UINT8 minute = systime->local_time.minute;
		UINT8 hour = systime->local_time.hour;
		UINT8 mday = systime->local_time.mday;
		
		while (m_rtc_offset >= 3600)
		{
			m_rtc_offset -= 3600;
			hour++;
			
			if (hour == 24)
			{
				mday++;
				hour = 0;
			}
		}
		
		while (m_rtc_offset >= 60)
		{
			m_rtc_offset -= 60;
			minute++;
			
			if (minute == 60)
			{
				hour++;
				minute = 0;
			}
		}
		
		while (m_rtc_offset)
		{
			m_rtc_offset -= 1;
			second++;
			
			if (second == 60)
			{
				minute++;
				second = 0;
			}
		}
		
		m_rtc_ram[0] = second % 10;
		m_rtc_ram[1] = second / 10;
		m_rtc_ram[2] = minute % 10;
		m_rtc_ram[3] = minute / 10;
		m_rtc_ram[4] = hour % 10;
		m_rtc_ram[5] = hour / 10;
		m_rtc_ram[6] = mday % 10;
		m_rtc_ram[7] = mday / 10;
		m_rtc_ram[8] = systime->local_time.month % 10;
		m_rtc_ram[9] = systime->local_time.month / 10;
		m_rtc_ram[8] = systime->local_time.month;
		m_rtc_ram[10] = (systime->local_time.year - 1900) % 10;
		m_rtc_ram[11] = ((systime->local_time.year - 1900) / 10) % 10;
		m_rtc_ram[12] = systime->local_time.weekday % 7;
	}
}

READ8_MEMBER(sns_rom_spc7110_device::read_l)
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset < 0x10000)
				return m_ram[offset & 0x1fff];
			if (offset >= 0x300000 && offset < 0x310000)
				return m_ram[offset & 0x1fff];
		}
		if (address >= 0x8000)
			return m_rom[rom_bank_map[offset / 0x8000] * 0x8000 + (offset & 0x7fff)];
	}
	
	return 0xff;
}

READ8_MEMBER(sns_rom_spc7110_device::read_h)
{
	UINT16 address = offset & 0xfffff;
	
	if (offset < 0x400000)
	{
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset < 0x10000)
				return m_ram[offset & 0x1fff];
			if (offset >= 0x300000 && offset < 0x310000)
				return m_ram[offset & 0x1fff];
		}
		if (address >= 0x8000)
			return m_rom[rom_bank_map[offset / 0x8000] * 0x8000 + (offset & 0x7fff)];
	}
	else
	{
		switch (offset & 0x300000)
		{
			case 0x000000:
				return m_rom[rom_bank_map[(offset - 0x400000) / 0x8000] * 0x8000 + (offset & 0x7fff)];
			case 0x100000:
				return m_rom[m_dx_offset + address];
			case 0x200000:
				return m_rom[m_ex_offset + address];
			case 0x300000:
				return m_rom[m_fx_offset + address];
			default:
				break;
		}
	}

	return 0xff;
}

WRITE8_MEMBER(sns_rom_spc7110_device::write_l)
{
	m_ram[offset & 0x1fff] = data;
}


READ8_MEMBER(sns_rom_spc7110_device::chip_read)
{
	UINT8 *ROM = get_rom_base();
	UINT32 len = get_rom_size();
	UINT16 addr = offset & 0xffff;
	
	switch (addr)
	{
		//==================
		//decompression unit
		//==================
			
		case 0x4800:
		{
			UINT16 counter = (m_r4809 + (m_r480a << 8));
			counter--;
			m_r4809 = counter;
			m_r480a = counter >> 8;
			return m_decomp->read(ROM, len);
		}
		case 0x4801: return m_r4801;
		case 0x4802: return m_r4802;
		case 0x4803: return m_r4803;
		case 0x4804: return m_r4804;
		case 0x4805: return m_r4805;
		case 0x4806: return m_r4806;
		case 0x4807: return m_r4807;
		case 0x4808: return m_r4808;
		case 0x4809: return m_r4809;
		case 0x480a: return m_r480a;
		case 0x480b: return m_r480b;
		case 0x480c:
		{
			UINT8 status = m_r480c;
			m_r480c &= 0x7f;
			return status;
		}
			
		//==============
		//data port unit
		//==============
			
		case 0x4810:
		{
			UINT8 data;
			UINT32 address, adjust, adjustaddr;
			
			if (m_r481x != 0x07) return 0x00;
			
			address = spc7110_data_pointer();
			adjust = spc7110_data_adjust();
			if (m_r4818 & 8)
			{
				adjust = (INT16)adjust;  //16-bit sign extend
			}
			
			adjustaddr = address;
			if (m_r4818 & 2)
			{
				adjustaddr += adjust;
				spc7110_set_data_adjust(adjust + 1);
			}
			
			data = ROM[spc7110_datarom_addr(adjustaddr, len)];
			if (!(m_r4818 & 2))
			{
				UINT32 increment = (m_r4818 & 1) ? spc7110_data_increment() : 1;
				if (m_r4818 & 4)
				{
					increment = (INT16)increment;  //16-bit sign extend
				}
				
				if ((m_r4818 & 16) == 0)
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
		case 0x4811: return m_r4811;
		case 0x4812: return m_r4812;
		case 0x4813: return m_r4813;
		case 0x4814: return m_r4814;
		case 0x4815: return m_r4815;
		case 0x4816: return m_r4816;
		case 0x4817: return m_r4817;
		case 0x4818: return m_r4818;
		case 0x481a:
		{
			UINT8 data;
			UINT32 address, adjust;
			if (m_r481x != 0x07)
			{
				return 0x00;
			}
			
			address = spc7110_data_pointer();
			adjust = spc7110_data_adjust();
			if (m_r4818 & 8)
			{
				adjust = (INT16)adjust;  //16-bit sign extend
			}
			
			data = ROM[spc7110_datarom_addr(address + adjust, len)];
			if ((m_r4818 & 0x60) == 0x60)
			{
				if ((m_r4818 & 16) == 0)
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
			
		case 0x4820: return m_r4820;
		case 0x4821: return m_r4821;
		case 0x4822: return m_r4822;
		case 0x4823: return m_r4823;
		case 0x4824: return m_r4824;
		case 0x4825: return m_r4825;
		case 0x4826: return m_r4826;
		case 0x4827: return m_r4827;
		case 0x4828: return m_r4828;
		case 0x4829: return m_r4829;
		case 0x482a: return m_r482a;
		case 0x482b: return m_r482b;
		case 0x482c: return m_r482c;
		case 0x482d: return m_r482d;
		case 0x482e: return m_r482e;
		case 0x482f:
		{
			UINT8 status = m_r482f;
			m_r482f &= 0x7f;
			return status;
		}
			
		//===================
		//memory mapping unit
		//===================
			
		case 0x4830: return m_r4830;
		case 0x4831: return m_r4831;
		case 0x4832: return m_r4832;
		case 0x4833: return m_r4833;
		case 0x4834: return m_r4834;
			
		//====================
		//real-time clock unit
		//====================
		case 0x4840: return m_r4840;
		case 0x4841:
		{
			UINT8 data = 0;
			if (m_rtc_state == RTCS_Inactive || m_rtc_state == RTCS_ModeSelect)
				return 0x00;
			
			m_r4842 = 0x80;
			data = m_rtc_ram[m_rtc_index];
			m_rtc_index = (m_rtc_index + 1) & 15;
			return data;
		}
		case 0x4842:
		{
			UINT8 status = m_r4842;
			m_r4842 &= 0x7f;
			return status;
		}
	}

	return 0xff;
}

WRITE8_MEMBER(sns_rom_spc7110_device::chip_write)
{
	UINT8 *ROM = get_rom_base();
	UINT32 len = get_rom_size();
	UINT16 addr = offset & 0xffff;
	
	switch (addr)
	{
		//==================
		//decompression unit
		//==================
			
		case 0x4801: m_r4801 = data; break;
		case 0x4802: m_r4802 = data; break;
		case 0x4803: m_r4803 = data; break;
		case 0x4804: m_r4804 = data; break;
		case 0x4805: m_r4805 = data; break;
		case 0x4806:
		{
			UINT32 table, index, address, mode, offset;
			m_r4806 = data;
			
			table   = (m_r4801 + (m_r4802 << 8) + (m_r4803 << 16));
			index   = (m_r4804 << 2);
			//length  = (m_r4809 + (m_r480a << 8));
			address = spc7110_datarom_addr(table + index, len);
			mode    = (ROM[address + 0]);
			offset  = (ROM[address + 1] << 16)
			+ (ROM[address + 2] <<  8)
			+ (ROM[address + 3] <<  0);
			
			m_decomp->init(machine(), ROM, len, mode, offset, (m_r4805 + (m_r4806 << 8)) << mode);
			m_r480c = 0x80;
		}
			break;
			
		case 0x4807: m_r4807 = data; break;
		case 0x4808: m_r4808 = data; break;
		case 0x4809: m_r4809 = data; break;
		case 0x480a: m_r480a = data; break;
		case 0x480b: m_r480b = data; break;
			
		//==============
		//data port unit
		//==============
			
		case 0x4811: m_r4811 = data; m_r481x |= 0x01; break;
		case 0x4812: m_r4812 = data; m_r481x |= 0x02; break;
		case 0x4813: m_r4813 = data; m_r481x |= 0x04; break;
		case 0x4814:
		{
			m_r4814 = data;
			m_r4814_latch = 1;
			if (!m_r4815_latch)
			{
				break;
			}
			if (!(m_r4818 & 2))
			{
				break;
			}
			if (m_r4818 & 0x10)
			{
				break;
			}
			
			if ((m_r4818 & 0x60) == 0x20)
			{
				UINT32 increment = spc7110_data_adjust() & 0xff;
				if (m_r4818 & 8)
				{
					increment = (INT8)increment;  //8-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			else if ((m_r4818 & 0x60) == 0x40)
			{
				UINT32 increment = spc7110_data_adjust();
				if (m_r4818 & 8)
				{
					increment = (INT16)increment;  //16-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			break;
		}
			
		case 0x4815:
		{
			m_r4815 = data;
			m_r4815_latch = 1;
			if (!m_r4814_latch)
			{
				break;
			}
			if (!(m_r4818 & 2))
			{
				break;
			}
			if (m_r4818 & 0x10)
			{
				break;
			}
			
			if ((m_r4818 & 0x60) == 0x20)
			{
				UINT32 increment = spc7110_data_adjust() & 0xff;
				if (m_r4818 & 8)
				{
					increment = (INT8)increment;  //8-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			else if ((m_r4818 & 0x60) == 0x40)
			{
				UINT32 increment = spc7110_data_adjust();
				if (m_r4818 & 8)
				{
					increment = (INT16)increment;  //16-bit sign extend
				}
				spc7110_set_data_pointer(spc7110_data_pointer() + increment);
			}
			break;
		}
			
		case 0x4816: m_r4816 = data; break;
		case 0x4817: m_r4817 = data; break;
		case 0x4818:
		{
			if (m_r481x != 0x07)
				break;
			
			m_r4818 = data;
			m_r4814_latch = m_r4815_latch = 0;
			break;
		}
			
		//=========
		//math unit
		//=========
			
		case 0x4820: m_r4820 = data; break;
		case 0x4821: m_r4821 = data; break;
		case 0x4822: m_r4822 = data; break;
		case 0x4823: m_r4823 = data; break;
		case 0x4824: m_r4824 = data; break;
		case 0x4825:
		{
			m_r4825 = data;
			
			if (m_r482e & 1)
			{
				//signed 16-bit x 16-bit multiplication
				INT16 r0 = (INT16)(m_r4824 + (m_r4825 << 8));
				INT16 r1 = (INT16)(m_r4820 + (m_r4821 << 8));
				
				INT32 result = r0 * r1;
				m_r4828 = result;
				m_r4829 = result >> 8;
				m_r482a = result >> 16;
				m_r482b = result >> 24;
			}
			else
			{
				//unsigned 16-bit x 16-bit multiplication
				UINT16 r0 = (UINT16)(m_r4824 + (m_r4825 << 8));
				UINT16 r1 = (UINT16)(m_r4820 + (m_r4821 << 8));
				
				UINT32 result = r0 * r1;
				m_r4828 = result;
				m_r4829 = result >> 8;
				m_r482a = result >> 16;
				m_r482b = result >> 24;
			}
			
			m_r482f = 0x80;
			break;
		}
			
		case 0x4826: m_r4826 = data; break;
		case 0x4827:
		{
			m_r4827 = data;
			
			if (m_r482e & 1)
			{
				//signed 32-bit x 16-bit division
				INT32 dividend = (INT32)(m_r4820 + (m_r4821 << 8) + (m_r4822 << 16) + (m_r4823 << 24));
				INT16 divisor  = (INT16)(m_r4826 + (m_r4827 << 8));
				
				INT32 quotient;
				INT16 remainder;
				
				if (divisor)
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
				
				m_r4828 = quotient;
				m_r4829 = quotient >> 8;
				m_r482a = quotient >> 16;
				m_r482b = quotient >> 24;
				
				m_r482c = remainder;
				m_r482d = remainder >> 8;
			}
			else
			{
				//unsigned 32-bit x 16-bit division
				UINT32 dividend = (UINT32)(m_r4820 + (m_r4821 << 8) + (m_r4822 << 16) + (m_r4823 << 24));
				UINT16 divisor  = (UINT16)(m_r4826 + (m_r4827 << 8));
				
				UINT32 quotient;
				UINT16 remainder;
				
				if (divisor)
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
				
				m_r4828 = quotient;
				m_r4829 = quotient >> 8;
				m_r482a = quotient >> 16;
				m_r482b = quotient >> 24;
				
				m_r482c = remainder;
				m_r482d = remainder >> 8;
			}
			
			m_r482f = 0x80;
			break;
		}
			
		case 0x482e:
		{
			//reset math unit
			m_r4820 = m_r4821 = m_r4822 = m_r4823 = 0;
			m_r4824 = m_r4825 = m_r4826 = m_r4827 = 0;
			m_r4828 = m_r4829 = m_r482a = m_r482b = 0;
			m_r482c = m_r482d = 0;
			
			m_r482e = data;
			break;
		}
			
		//===================
		//memory mapping unit
		//===================
			
		case 0x4830: m_r4830 = data; break;
			
		case 0x4831:
		{
			m_r4831 = data;
			m_dx_offset = spc7110_datarom_addr(data * 0x100000, len);
			break;
		}
			
		case 0x4832:
		{
			m_r4832 = data;
			m_ex_offset = spc7110_datarom_addr(data * 0x100000, len);
			break;
		}
			
		case 0x4833:
		{
			m_r4833 = data;
			m_fx_offset = spc7110_datarom_addr(data * 0x100000, len);
			break;
		}
			
		case 0x4834: m_r4834 = data; break;
			
		//====================
		//real-time clock unit
		//====================
			
		case 0x4840:
		{
			m_r4840 = data;
			
			if (!(m_r4840 & 1))
			{
				//disable RTC
				m_rtc_state = RTCS_Inactive;
				spc7110_update_time(0);
			}
			else
			{
				//enable RTC
				m_r4842 = 0x80;
				m_rtc_state = RTCS_ModeSelect;
			}
		}
			break;
			
		case 0x4841:
		{
			m_r4841 = data;
			
			switch (m_rtc_state)
			{
				case RTCS_ModeSelect:
					if (data == RTCM_Linear || data == RTCM_Indexed)
					{
						m_r4842 = 0x80;
						m_rtc_state = RTCS_IndexSelect;
						m_rtc_mode = (RTC_Mode)data;
						m_rtc_index = 0;
					}
					break;
					
				case RTCS_IndexSelect:
					m_r4842 = 0x80;
					m_rtc_index = data & 15;
					if (m_rtc_mode == RTCM_Linear)
						m_rtc_state = RTCS_Write;
					break;
					
				case RTCS_Write:
					m_r4842 = 0x80;
					
					//control register 0
					if (m_rtc_index == 13)
					{
						//increment second counter
						if (data & 2)
							spc7110_update_time(1);
						
						//round minute counter
						if (data & 8)
						{
							spc7110_update_time(0);
							
							UINT8 second = m_rtc_ram[0] + m_rtc_ram[1] * 10;
							//clear seconds
							m_rtc_ram[0] = 0;
							m_rtc_ram[1] = 0;
							
							if (second >= 30)
								spc7110_update_time(60);
						}
					}
					
					//control register 2
					if (m_rtc_index == 15)
					{
						//disable timer and clear second counter
						if ((data & 1) && !(m_rtc_ram[15]  & 1))
						{
							spc7110_update_time(0);
							
							//clear seconds
							m_rtc_ram[0] = 0;
							m_rtc_ram[1] = 0;
						}
						
						//disable timer
						if ((data & 2) && !(m_rtc_ram[15] & 2))
							spc7110_update_time(0);
					}
					
					m_rtc_ram[m_rtc_index] = data & 15;
					m_rtc_index = (m_rtc_index + 1) & 15;
					break;
			}
		}
			break;
	}
}

