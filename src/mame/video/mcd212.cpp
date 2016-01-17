// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i MCD212 video emulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Unknown yet.

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mcd212.h"
#include "includes/cdi.h"

// device type definition
const device_type MACHINE_MCD212 = &device_creator<mcd212_device>;

#if ENABLE_VERBOSE_LOG
static inline void ATTR_PRINTF(3,4) verboselog(device_t& device, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror("%s: %s", device.machine().describe_context(), buf );
	}
}
#else
#define verboselog(x,y,z, ...)
#endif

static const UINT16 cdi220_lcd_char[20*22] =
{
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0002, 0x0002, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0002, 0x0002, 0x0002, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0002, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0000, 0x8000, 0x8000, 0x8000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x0000, 0x0000, 0x8000, 0x8000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0000, 0x0000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x2000, 0x2000, 0x2000, 0x2000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0200, 0x0200, 0x0200, 0x0200,
	0x1000, 0x1000, 0x1000, 0x1000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0000, 0x0000, 0x0010, 0x0010, 0x0001, 0x0001, 0x0001, 0x0001, 0x0008, 0x0008, 0x0000, 0x0000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0000, 0x0010, 0x0010, 0x0010, 0x0001, 0x0001, 0x0001, 0x0001, 0x0008, 0x0008, 0x0008, 0x0000, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0010, 0x0010, 0x0010, 0x0010, 0x0001, 0x0001, 0x0001, 0x0001, 0x0008, 0x0008, 0x0008, 0x0008, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0010, 0x0010, 0x0010, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0008, 0x0008, 0x0008, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0010, 0x0010, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0008, 0x0008, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400,
	0x1000, 0x1000, 0x1000, 0x1000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0400, 0x0400, 0x0400, 0x0400
};

void mcd212_device::draw_lcd(int y)
{
	cdi_state *state = machine().driver_data<cdi_state>();
	if (state->m_slave_hle == nullptr)
	{
		return;
	}
	bitmap_rgb32 &bitmap = state->m_lcdbitmap;
	UINT32 *scanline = &bitmap.pix32(y);
	int x = 0;
	int lcd = 0;

	for(lcd = 0; lcd < 8; lcd++)
	{
		UINT16 data = (state->m_slave_hle->get_lcd_state()[lcd*2] << 8) |
						state->m_slave_hle->get_lcd_state()[lcd*2 + 1];
		for(x = 0; x < 20; x++)
		{
			if(data & cdi220_lcd_char[y*20 + x])
			{
				scanline[(7 - lcd)*24 + x] = 0x00ffffff;
			}
			else
			{
				scanline[(7 - lcd)*24 + x] = 0;
			}
		}
	}
}

void mcd212_device::update_region_arrays()
{
	int latched_rf0 = 0;
	int latched_rf1 = 0;
	int latched_wfa = m_channel[0].weight_factor_a[0];
	int latched_wfb = m_channel[1].weight_factor_b[0];
	int reg = 0;

	for(int x = 0; x < 768; x++)
	{
		if(m_channel[0].image_coding_method & MCD212_ICM_NR)
		{
			for(int flag = 0; flag < 2; flag++)
			{
				for(int reg_ = 0; reg_ < 4; reg_++)
				{
					if(m_channel[0].region_control[reg_] == 0)
					{
						break;
					}
					if(x == (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_X))
					{
						switch((m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_OP) >> MCD212_RC_OP_SHIFT)
						{
							case 0: // End of region control for line
								break;
							case 1:
							case 2:
							case 3: // Not used
								break;
							case 4: // Change weight of plane A
								latched_wfa = (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
								break;
							case 5: // Not used
								break;
							case 6: // Change weight of plane B
								latched_wfb = (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
								break;
							case 7: // Not used
								break;
							case 8: // Reset region flag
								if(flag)
								{
									latched_rf1 = 0;
								}
								else
								{
									latched_rf0 = 0;
								}
								break;
							case 9: // Set region flag
								if(flag)
								{
									latched_rf1 = 1;
								}
								else
								{
									latched_rf0 = 1;
								}
								break;
							case 10:    // Not used
							case 11:    // Not used
								break;
							case 12: // Reset region flag and change weight of plane A
								latched_wfa = (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
								if(flag)
								{
									latched_rf1 = 0;
								}
								else
								{
									latched_rf0 = 0;
								}
								break;
							case 13: // Set region flag and change weight of plane A
								latched_wfa = (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
								if(flag)
								{
									latched_rf1 = 1;
								}
								else
								{
									latched_rf0 = 1;
								}
								break;
							case 14: // Reset region flag and change weight of plane B
								latched_wfb = (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
								if(flag)
								{
									latched_rf1 = 0;
								}
								else
								{
									latched_rf0 = 0;
								}
								break;
							case 15: // Set region flag and change weight of plane B
								latched_wfb = (m_channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
								if(flag)
								{
									latched_rf1 = 1;
								}
								else
								{
									latched_rf0 = 1;
								}
								break;
						}
					}
				}
			}
		}
		else
		{
			if(reg < 8)
			{
				int flag = (m_channel[0].region_control[reg] & MCD212_RC_RF) >> MCD212_RC_RF_SHIFT;
				if(!(m_channel[0].region_control[reg] & MCD212_RC_OP))
				{
					for(; x < 768; x++)
					{
						m_channel[0].weight_factor_a[x] = latched_wfa;
						m_channel[1].weight_factor_b[x] = latched_wfb;
						m_region_flag_0[x] = latched_rf0;
						m_region_flag_1[x] = latched_rf1;
					}
					break;
				}
				if(x == (m_channel[0].region_control[reg] & MCD212_RC_X))
				{
					switch((m_channel[0].region_control[reg] & MCD212_RC_OP) >> MCD212_RC_OP_SHIFT)
					{
						case 0: // End of region control for line
							break;
						case 1:
						case 2:
						case 3: // Not used
							break;
						case 4: // Change weight of plane A
							latched_wfa = (m_channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
							break;
						case 5: // Not used
							break;
						case 6: // Change weight of plane B
							latched_wfb = (m_channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
							break;
						case 7: // Not used
							break;
						case 8: // Reset region flag
							if(flag)
							{
								latched_rf1 = 0;
							}
							else
							{
								latched_rf0 = 0;
							}
							break;
						case 9: // Set region flag
							if(flag)
							{
								latched_rf1 = 1;
							}
							else
							{
								latched_rf0 = 1;
							}
							break;
						case 10:    // Not used
						case 11:    // Not used
							break;
						case 12: // Reset region flag and change weight of plane A
							latched_wfa = (m_channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
							if(flag)
							{
								latched_rf1 = 0;
							}
							else
							{
								latched_rf0 = 0;
							}
							break;
						case 13: // Set region flag and change weight of plane A
							latched_wfa = (m_channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
							if(flag)
							{
								latched_rf1 = 1;
							}
							else
							{
								latched_rf0 = 1;
							}
							break;
						case 14: // Reset region flag and change weight of plane B
							latched_wfb = (m_channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
							if(flag)
							{
								latched_rf1 = 0;
							}
							else
							{
								latched_rf0 = 0;
							}
							break;
						case 15: // Set region flag and change weight of plane B
							latched_wfb = (m_channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
							if(flag)
							{
								latched_rf1 = 1;
							}
							else
							{
								latched_rf0 = 1;
							}
							break;
					}
					reg++;
				}
			}
		}
		m_channel[0].weight_factor_a[x] = latched_wfa;
		m_channel[1].weight_factor_b[x] = latched_wfb;
		m_region_flag_0[x] = latched_rf0;
		m_region_flag_1[x] = latched_rf1;
	}
}

void mcd212_device::set_vsr(int channel, UINT32 value)
{
	m_channel[channel].vsr = value & 0x0000ffff;
	m_channel[channel].dcr &= 0xffc0;
	m_channel[channel].dcr |= (value >> 16) & 0x003f;
}

void mcd212_device::set_register(int channel, UINT8 reg, UINT32 value)
{
	switch(reg)
	{
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: // CLUT 0 - 63
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			verboselog(*this, 11, "          %04xxxxx: %d: CLUT[%d] = %08x\n", channel * 0x20, channel, m_channel[channel].clut_bank * 0x40 + (reg - 0x80), value );
			m_channel[0].clut_r[m_channel[channel].clut_bank * 0x40 + (reg - 0x80)] = (UINT8)(value >> 16) & 0xfc;
			m_channel[0].clut_g[m_channel[channel].clut_bank * 0x40 + (reg - 0x80)] = (UINT8)(value >>  8) & 0xfc;
			m_channel[0].clut_b[m_channel[channel].clut_bank * 0x40 + (reg - 0x80)] = (UINT8)(value >>  0) & 0xfc;
			break;
		case 0xc0: // Image Coding Method
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Image Coding Method = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].image_coding_method = value;
			}
			break;
		case 0xc1: // Transparency Control
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Transparency Control = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].transparency_control = value;
			}
			break;
		case 0xc2: // Plane Order
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Plane Order = %08x\n", channel * 0x20, channel, value & 7);
				m_channel[channel].plane_order = value & 0x00000007;
			}
			break;
		case 0xc3: // CLUT Bank Register
			verboselog(*this, 6, "          %04xxxxx: %d: CLUT Bank Register = %08x\n", channel * 0x20, channel, value & 3);
			m_channel[channel].clut_bank = channel ? (2 | (value & 0x00000001)) : (value & 0x00000003);
			break;
		case 0xc4: // Transparent Color A
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Transparent Color A = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].transparent_color_a = value & 0xfcfcfc;
			}
			break;
		case 0xc6: // Transparent Color B
			if(channel == 1)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Transparent Color B = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].transparent_color_b = value & 0xfcfcfc;
			}
			break;
		case 0xc7: // Mask Color A
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Mask Color A = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].mask_color_a = value & 0xfcfcfc;
			}
			break;
		case 0xc9: // Mask Color B
			if(channel == 1)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Mask Color B = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].mask_color_b = value & 0xfcfcfc;
			}
			break;
		case 0xca: // Delta YUV Absolute Start Value A
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Delta YUV Absolute Start Value A = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].dyuv_abs_start_a = value;
			}
			break;
		case 0xcb: // Delta YUV Absolute Start Value B
			if(channel == 1)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Delta YUV Absolute Start Value B = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].dyuv_abs_start_b = value;
			}
			break;
		case 0xcd: // Cursor Position
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Cursor Position = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].cursor_position = value;
			}
			break;
		case 0xce: // Cursor Control
			if(channel == 0)
			{
				verboselog(*this, 11, "          %04xxxxx: %d: Cursor Control = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].cursor_control = value;
			}
			break;
		case 0xcf: // Cursor Pattern
			if(channel == 0)
			{
				verboselog(*this, 11, "          %04xxxxx: %d: Cursor Pattern[%d] = %04x\n", channel * 0x20, channel, (value >> 16) & 0x000f, value & 0x0000ffff);
				m_channel[channel].cursor_pattern[(value >> 16) & 0x000f] = value & 0x0000ffff;
			}
			break;
		case 0xd0: // Region Control 0-7
		case 0xd1:
		case 0xd2:
		case 0xd3:
		case 0xd4:
		case 0xd5:
		case 0xd6:
		case 0xd7:
			verboselog(*this, 6, "          %04xxxxx: %d: Region Control %d = %08x\n", channel * 0x20, channel, reg & 7, value );
			m_channel[0].region_control[reg & 7] = value;
			update_region_arrays();
			break;
		case 0xd8: // Backdrop Color
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Backdrop Color = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].backdrop_color = value;
			}
			break;
		case 0xd9: // Mosaic Pixel Hold Factor A
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Mosaic Pixel Hold Factor A = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].mosaic_hold_a = value;
			}
			break;
		case 0xda: // Mosaic Pixel Hold Factor B
			if(channel == 1)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Mosaic Pixel Hold Factor B = %08x\n", channel * 0x20, channel, value );
				m_channel[channel].mosaic_hold_b = value;
			}
			break;
		case 0xdb: // Weight Factor A
			if(channel == 0)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Weight Factor A = %08x\n", channel * 0x20, channel, value );
				memset(m_channel[channel].weight_factor_a, value & 0x000000ff, 768);
				update_region_arrays();
			}
			break;
		case 0xdc: // Weight Factor B
			if(channel == 1)
			{
				verboselog(*this, 6, "          %04xxxxx: %d: Weight Factor B = %08x\n", channel * 0x20, channel, value );
				memset(m_channel[channel].weight_factor_b, value & 0x000000ff, 768);
				update_region_arrays();
			}
			break;
	}
}

UINT32 mcd212_device::get_vsr(int channel)
{
	return ((m_channel[channel].dcr & 0x3f) << 16) | m_channel[channel].vsr;
}

void mcd212_device::set_dcp(int channel, UINT32 value)
{
	m_channel[channel].dcp = value & 0x0000ffff;
	m_channel[channel].ddr &= 0xffc0;
	m_channel[channel].ddr |= (value >> 16) & 0x003f;
}

UINT32 mcd212_device::get_dcp(int channel)
{
	return ((m_channel[channel].ddr & 0x3f) << 16) | m_channel[channel].dcp;
}

void mcd212_device::set_display_parameters(int channel, UINT8 value)
{
	m_channel[channel].ddr &= 0xf0ff;
	m_channel[channel].ddr |= (value & 0x0f) << 8;
	m_channel[channel].dcr &= 0xf7ff;
	m_channel[channel].dcr |= (value & 0x10) << 7;
}

void mcd212_device::update_visible_area()
{
	const rectangle &visarea = m_screen->visible_area();
	rectangle visarea1;
	attoseconds_t period = m_screen->frame_period().attoseconds();
	int width = 0;

	if((m_channel[0].dcr & (MCD212_DCR_CF | MCD212_DCR_FD)) && (m_channel[0].csrw & MCD212_CSR1W_ST))
	{
		width = 360;
	}
	else
	{
		width = 384;
	}

	visarea1.max_x = width-1;
	visarea1.min_x = visarea.min_x;
	visarea1.min_y = visarea.min_y;
	visarea1.max_y = visarea.max_y;

	m_screen->configure(width, 302, visarea1, period);
}

UINT32 mcd212_device::get_screen_width()
{
	if((m_channel[0].dcr & (MCD212_DCR_CF | MCD212_DCR_FD)) && (m_channel[0].csrw & MCD212_CSR1W_ST))
	{
		return 720;
	}
	return 768;
}

void mcd212_device::process_ica(int channel)
{
	cdi_state *state = machine().driver_data<cdi_state>();
	UINT16 *ica = channel ? state->m_planeb : state->m_planea;
	UINT32 addr = 0x000400/2;
	UINT32 cmd = 0;
	while(1)
	{
		UINT8 stop = 0;
		cmd = ica[addr++] << 16;
		cmd |= ica[addr++];
		switch((cmd & 0xff000000) >> 24)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // STOP
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				verboselog(*this, 11, "%08x: %08x: ICA %d: STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				stop = 1;
				break;
			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // NOP
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				verboselog(*this, 12, "%08x: %08x: ICA %d: NOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				break;
			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // RELOAD DCP
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				verboselog(*this, 11, "%08x: %08x: ICA %d: RELOAD DCP\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_dcp(channel, cmd & 0x001fffff);
				break;
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // RELOAD DCP and STOP
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				verboselog(*this, 11, "%08x: %08x: ICA %d: RELOAD DCP and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_dcp(channel, cmd & 0x001fffff);
				stop = 1;
				break;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD ICA
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				verboselog(*this, 11, "%08x: %08x: ICA %d: RELOAD ICA\n", addr * 2 + channel * 0x200000, cmd, channel );
				addr = (cmd & 0x001fffff) / 2;
				break;
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				verboselog(*this, 11, "%08x: %08x: ICA %d: RELOAD VSR and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_vsr(channel, cmd & 0x001fffff);
				stop = 1;
				break;
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				verboselog(*this, 11, "%08x: %08x: ICA %d: INTERRUPT\n", addr * 2 + channel * 0x200000, cmd, channel );
				m_channel[1].csrr |= 1 << (2 - channel);
				if(m_channel[1].csrr & (MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2))
				{
					UINT8 interrupt = (state->m_scc->get_lir() >> 4) & 7;
					if(interrupt)
					{
						state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
						state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
					}
				}
#if 0
				if(m_channel[1].csrr & MCD212_CSR2R_IT2)
				{
					UINT8 interrupt = state->m_scc68070_regs.lir & 7;
					if(interrupt)
					{
						state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 24 + interrupt);
						state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
					}
				}
#endif
				break;
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
				verboselog(*this, 6, "%08x: %08x: ICA %d: RELOAD DISPLAY PARAMETERS\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_display_parameters(channel, cmd & 0x1f);
				break;
			default:
				set_register(channel, cmd >> 24, cmd & 0x00ffffff);
				break;
		}
		if(stop)
		{
			break;
		}
	}
}

void mcd212_device::process_dca(int channel)
{
	cdi_state *state = machine().driver_data<cdi_state>();
	UINT16 *dca = channel ? state->m_planeb : state->m_planea;
	UINT32 addr = (m_channel[channel].dca & 0x0007ffff) / 2; //(get_dcp(mcd212, channel) & 0x0007ffff) / 2; // m_channel[channel].dca / 2;
	UINT32 cmd = 0;
	UINT32 count = 0;
	UINT32 max = 64;
	UINT8 addr_changed = 0;
	//printf( "max = %d\n", max );
	while(1)
	{
		UINT8 stop = 0;
		cmd = dca[addr++] << 16;
		cmd |= dca[addr++];
		count += 4;
		switch((cmd & 0xff000000) >> 24)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // STOP
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				verboselog(*this, 11, "%08x: %08x: DCA %d: STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				stop = 1;
				break;
			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // NOP
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				verboselog(*this, 12, "%08x: %08x: DCA %d: NOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				break;
			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // RELOAD DCP
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				verboselog(*this, 11, "%08x: %08x: DCA %d: RELOAD DCP (NOP)\n", addr * 2 + channel * 0x200000, cmd, channel );
				break;
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // RELOAD DCP and STOP
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				verboselog(*this, 11, "%08x: %08x: DCA %d: RELOAD DCP and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_dcp(channel, cmd & 0x001fffff);
				addr = (cmd & 0x0007ffff) / 2;
				addr_changed = 1;
				stop = 1;
				break;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD VSR
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				verboselog(*this, 11, "%08x: %08x: DCA %d: RELOAD VSR\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_vsr(channel, cmd & 0x001fffff);
				break;
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				verboselog(*this, 11, "%08x: %08x: DCA %d: RELOAD VSR and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_vsr(channel, cmd & 0x001fffff);
				stop = 1;
				break;
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				verboselog(*this, 11, "%08x: %08x: DCA %d: INTERRUPT\n", addr * 2 + channel * 0x200000, cmd, channel );
				m_channel[1].csrr |= 1 << (2 - channel);
				if(m_channel[1].csrr & (MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2))
				{
					UINT8 interrupt = (state->m_scc->get_lir() >> 4) & 7;
					if(interrupt)
					{
						state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
						state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
					}
				}
#if 0
				if(m_channel[1].csrr & MCD212_CSR2R_IT2)
				{
					UINT8 interrupt = state->m_scc68070_regs.lir & 7;
					if(interrupt)
					{
						state->m_maincpu->set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 24 + interrupt);
						state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
					}
				}
#endif
				break;
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
				verboselog(*this, 6, "%08x: %08x: DCA %d: RELOAD DISPLAY PARAMETERS\n", addr * 2 + channel * 0x200000, cmd, channel );
				set_display_parameters(channel, cmd & 0x1f);
				break;
			default:
				set_register(channel, cmd >> 24, cmd & 0x00ffffff);
				break;
		}
		if(stop != 0 || count == max)
		{
			break;
		}
	}
	if(!addr_changed)
	{
		if(count < max)
		{
			addr += (max - count) >> 1;
		}
	}
	m_channel[channel].dca = addr * 2;
}

static inline UINT8 MCD212_LIM(INT32 in)
{
	if(in < 0)
	{
		return 0;
	}
	else if(in > 255)
	{
		return 255;
	}
	return (UINT8)in;
}

static inline UINT8 BYTE_TO_CLUT(int channel, int icm, UINT8 byte)
{
	switch(icm)
	{
		case 1:
			return byte;
		case 3:
			if(channel)
			{
				return 0x80 + (byte & 0x7f);
			}
			else
			{
				return byte & 0x7f;
			}
		case 4:
			if(!channel)
			{
				return byte & 0x7f;
			}
			break;
		case 11:
			if(channel)
			{
				return 0x80 + (byte & 0x0f);
			}
			else
			{
				return byte & 0x0f;
			}
		default:
			break;
	}
	return 0;
}

void mcd212_device::process_vsr(int channel, UINT8 *pixels_r, UINT8 *pixels_g, UINT8 *pixels_b)
{
	cdi_state *state = machine().driver_data<cdi_state>();
	UINT8 *data = reinterpret_cast<UINT8 *>(channel ? state->m_planeb.target() : state->m_planea.target());
	UINT32 vsr = get_vsr(channel) & 0x0007ffff;
	UINT8 done = 0;
	UINT32 x = 0;
	UINT32 icm_mask = channel ? MCD212_ICM_MODE2 : MCD212_ICM_MODE1;
	UINT32 icm_shift = channel ? MCD212_ICM_MODE2_SHIFT : MCD212_ICM_MODE1_SHIFT;
	UINT8 icm = (m_channel[0].image_coding_method & icm_mask) >> icm_shift;
	UINT8 *clut_r = m_channel[0].clut_r;
	UINT8 *clut_g = m_channel[0].clut_g;
	UINT8 *clut_b = m_channel[0].clut_b;
	UINT8 mosaic_enable = ((m_channel[channel].ddr & MCD212_DDR_FT) == MCD212_DDR_FT_MOSAIC);
	UINT8 mosaic_factor = 1 << (((m_channel[channel].ddr & MCD212_DDR_MT) >> MCD212_DDR_MT_SHIFT) + 1);
	int mosaic_index = 0;
	UINT32 width = get_screen_width();

	//printf( "vsr before: %08x: ", vsr );
	//fflush(stdout);

	if(!icm || !vsr)
	{
		memset(pixels_r, 0x10, width);
		memset(pixels_g, 0x10, width);
		memset(pixels_b, 0x10, width);
		return;
	}

	while(!done)
	{
		UINT8 byte = data[(vsr & 0x0007ffff) ^ 1];
		vsr++;
		switch(m_channel[channel].ddr & MCD212_DDR_FT)
		{
			case MCD212_DDR_FT_BMP:
			case MCD212_DDR_FT_BMP2:
			case MCD212_DDR_FT_MOSAIC:
				if(m_channel[channel].dcr & MCD212_DCR_CM)
				{
					// 4-bit Bitmap
					verboselog(*this, 0, "%s", "Unsupported display mode: 4-bit Bitmap\n" );
				}
				else
				{
					// 8-bit Bitmap
					if(icm == 5)
					{
						BYTE68K bY;
						BYTE68K bU;
						BYTE68K bV;
						switch(channel)
						{
							case 0:
								bY = (m_channel[0].dyuv_abs_start_a >> 16) & 0x000000ff;
								bU = (m_channel[0].dyuv_abs_start_a >>  8) & 0x000000ff;
								bV = (m_channel[0].dyuv_abs_start_a >>  0) & 0x000000ff;
								break;
							case 1:
								bY = (m_channel[1].dyuv_abs_start_b >> 16) & 0x000000ff;
								bU = (m_channel[1].dyuv_abs_start_b >>  8) & 0x000000ff;
								bV = (m_channel[1].dyuv_abs_start_b >>  0) & 0x000000ff;
								break;
							default:
								bY = bU = bV = 0x80;
								break;
						}
						for(; x < width; x += 2)
						{
							BYTE68K b0 = byte;
							BYTE68K bU1 = bU + m_ab.deltaUV[b0];
							BYTE68K bY0 = bY + m_ab.deltaY[b0];

							BYTE68K b1 = data[(vsr & 0x0007ffff) ^ 1];
							BYTE68K bV1 = bV + m_ab.deltaUV[b1];
							BYTE68K bY1 = bY0 + m_ab.deltaY[b1];

							BYTE68K bU0 = (bU + bU1) >> 1;
							BYTE68K bV0 = (bV + bV1) >> 1;

							BYTE68K *pbLimit;

							vsr++;

							bY = bY0;
							bU = bU0;
							bV = bV0;

							pbLimit = m_ab.limit + bY + BYTE68K_MAX;

							pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[m_ab.matrixVR[bV]];
							pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[m_ab.matrixUG[bU] + m_ab.matrixVG[bV]];
							pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[m_ab.matrixUB[bU]];

							if(mosaic_enable)
							{
								for(mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
								{
									pixels_r[x + 0 + mosaic_index*2] = pixels_r[x + 0];
									pixels_g[x + 0 + mosaic_index*2] = pixels_g[x + 0];
									pixels_b[x + 0 + mosaic_index*2] = pixels_b[x + 0];
									pixels_r[x + 1 + mosaic_index*2] = pixels_r[x + 1];
									pixels_g[x + 1 + mosaic_index*2] = pixels_g[x + 1];
									pixels_b[x + 1 + mosaic_index*2] = pixels_b[x + 1];
								}
								x += mosaic_factor * 2;
							}
							else
							{
								x += 2;
							}

							bY = bY1;
							bU = bU1;
							bV = bV1;

							pbLimit = m_ab.limit + bY + BYTE68K_MAX;

							pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[m_ab.matrixVR[bV]];
							pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[m_ab.matrixUG[bU] + m_ab.matrixVG[bV]];
							pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[m_ab.matrixUB[bU]];

							if(mosaic_enable)
							{
								for(mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
								{
									pixels_r[x + 0 + mosaic_index*2] = pixels_r[x + 0];
									pixels_g[x + 0 + mosaic_index*2] = pixels_g[x + 0];
									pixels_b[x + 0 + mosaic_index*2] = pixels_b[x + 0];
									pixels_r[x + 1 + mosaic_index*2] = pixels_r[x + 1];
									pixels_g[x + 1 + mosaic_index*2] = pixels_g[x + 1];
									pixels_b[x + 1 + mosaic_index*2] = pixels_b[x + 1];
								}
								x += (mosaic_factor * 2) - 2;
							}

							byte = data[(vsr & 0x0007ffff) ^ 1];

							vsr++;
						}
						set_vsr(channel, (vsr - 1) & 0x0007ffff);
					}
					else if(icm == 1 || icm == 3 || icm == 4)
					{
						for(; x < width; x += 2)
						{
							UINT8 clut_entry = BYTE_TO_CLUT(channel, icm, byte);
							pixels_r[x + 0] = clut_r[clut_entry];
							pixels_g[x + 0] = clut_g[clut_entry];
							pixels_b[x + 0] = clut_b[clut_entry];
							pixels_r[x + 1] = clut_r[clut_entry];
							pixels_g[x + 1] = clut_g[clut_entry];
							pixels_b[x + 1] = clut_b[clut_entry];
							if(mosaic_enable)
							{
								for(mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
								{
									pixels_r[x + 0 + mosaic_index*2] = pixels_r[x + 0];
									pixels_g[x + 0 + mosaic_index*2] = pixels_g[x + 0];
									pixels_b[x + 0 + mosaic_index*2] = pixels_b[x + 0];
									pixels_r[x + 1 + mosaic_index*2] = pixels_r[x + 1];
									pixels_g[x + 1 + mosaic_index*2] = pixels_g[x + 1];
									pixels_b[x + 1 + mosaic_index*2] = pixels_b[x + 1];
								}
								x += (mosaic_factor * 2) - 2;
							}
							byte = data[(vsr & 0x0007ffff) ^ 1];
							vsr++;
						}
						set_vsr(channel, (vsr - 1) & 0x0007ffff);
					}
					else if(icm == 11)
					{
						for(; x < width; x += 2)
						{
							UINT8 even_entry = BYTE_TO_CLUT(channel, icm, byte >> 4);
							UINT8 odd_entry = BYTE_TO_CLUT(channel, icm, byte);
							if(mosaic_enable)
							{
								for(mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
								{
									pixels_r[x + mosaic_index] = clut_r[even_entry];
									pixels_g[x + mosaic_index] = clut_g[even_entry];
									pixels_b[x + mosaic_index] = clut_b[even_entry];
								}
								for(mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
								{
									pixels_r[x + mosaic_factor + mosaic_index] = clut_r[odd_entry];
									pixels_g[x + mosaic_factor + mosaic_index] = clut_g[odd_entry];
									pixels_b[x + mosaic_factor + mosaic_index] = clut_b[odd_entry];
								}
								x += (mosaic_factor * 2) - 2;
							}
							else
							{
								pixels_r[x + 0] = clut_r[even_entry];
								pixels_g[x + 0] = clut_g[even_entry];
								pixels_b[x + 0] = clut_b[even_entry];
								pixels_r[x + 1] = clut_r[odd_entry];
								pixels_g[x + 1] = clut_g[odd_entry];
								pixels_b[x + 1] = clut_b[odd_entry];
							}
							byte = data[(vsr & 0x0007ffff) ^ 1];
							vsr++;
						}
						set_vsr(channel, (vsr - 1) & 0x0007ffff);
					}
					else
					{
						for(; x < width; x++)
						{
							pixels_r[x] = 0x10;
							pixels_g[x] = 0x10;
							pixels_b[x] = 0x10;
						}
					}
				}
				done = 1;
				break;
			case MCD212_DDR_FT_RLE:
				if(m_channel[channel].dcr & MCD212_DCR_CM)
				{
					verboselog(*this, 0, "%s", "Unsupported display mode: 4-bit RLE\n" );
					done = 1;
				}
				else
				{
					if(byte & 0x80)
					{
						// Run length
						UINT8 length = data[((vsr++) & 0x0007ffff) ^ 1];
						if(!length)
						{
							UINT8 clut_entry = BYTE_TO_CLUT(channel, icm, byte);
							UINT8 r = clut_r[clut_entry];
							UINT8 g = clut_g[clut_entry];
							UINT8 b = clut_b[clut_entry];
							// Go to the end of the line
							for(; x < width; x++)
							{
								pixels_r[x] = r;
								pixels_g[x] = g;
								pixels_b[x] = b;
								x++;
								pixels_r[x] = r;
								pixels_g[x] = g;
								pixels_b[x] = b;
							}
							done = 1;
							set_vsr(channel, vsr);
						}
						else
						{
							int end = x + (length * 2);
							UINT8 clut_entry = BYTE_TO_CLUT(channel, icm, byte);
							UINT8 r = clut_r[clut_entry];
							UINT8 g = clut_g[clut_entry];
							UINT8 b = clut_b[clut_entry];
							for(; x < end && x < width; x++)
							{
								pixels_r[x] = r;
								pixels_g[x] = g;
								pixels_b[x] = b;
								x++;
								pixels_r[x] = r;
								pixels_g[x] = g;
								pixels_b[x] = b;
							}
							if(x >= width)
							{
								done = 1;
								set_vsr(channel, vsr);
							}
						}
					}
					else
					{
						// Single pixel
						UINT8 clut_entry = BYTE_TO_CLUT(channel, icm, byte);
						pixels_r[x] = clut_r[clut_entry];
						pixels_g[x] = clut_g[clut_entry];
						pixels_b[x] = clut_b[clut_entry];
						x++;
						pixels_r[x] = clut_r[clut_entry];
						pixels_g[x] = clut_g[clut_entry];
						pixels_b[x] = clut_b[clut_entry];
						x++;
						if(x >= width)
						{
							done = 1;
							set_vsr(channel, vsr);
						}
					}
				}
				break;
		}
	}

	//printf( ": vsr after: %08x\n", vsr);
	//mcd212_set_vsr(&state->m_mcd212_regs, channel, vsr);
}

const UINT32 mcd212_device::s_4bpp_color[16] =
{
	0x00101010, 0x0010107a, 0x00107a10, 0x00107a7a, 0x007a1010, 0x007a107a, 0x007a7a10, 0x007a7a7a,
	0x00101010, 0x001010e6, 0x0010e610, 0x0010e6e6, 0x00e61010, 0x00e610e6, 0x00e6e610, 0x00e6e6e6
};

void mcd212_device::mix_lines(UINT8 *plane_a_r, UINT8 *plane_a_g, UINT8 *plane_a_b, UINT8 *plane_b_r, UINT8 *plane_b_g, UINT8 *plane_b_b, UINT32 *out)
{
	UINT8 debug_mode = machine().root_device().ioport("DEBUG")->read();
	UINT8 global_plane_a_disable = debug_mode & 1;
	UINT8 global_plane_b_disable = debug_mode & 2;
	UINT8 debug_backdrop_enable = debug_mode & 4;
	UINT8 debug_backdrop_index = debug_mode >> 4;
	UINT32 backdrop = debug_backdrop_enable ? s_4bpp_color[debug_backdrop_index] : s_4bpp_color[m_channel[0].backdrop_color];
	UINT8 transparency_mode_a = (m_channel[0].transparency_control >> 0) & 0x0f;
	UINT8 transparency_mode_b = (m_channel[0].transparency_control >> 8) & 0x0f;
	UINT8 transparent_color_a_r = (UINT8)(m_channel[0].transparent_color_a >> 16);
	UINT8 transparent_color_a_g = (UINT8)(m_channel[0].transparent_color_a >>  8);
	UINT8 transparent_color_a_b = (UINT8)(m_channel[0].transparent_color_a >>  0);
	UINT8 transparent_color_b_r = (UINT8)(m_channel[1].transparent_color_b >> 16);
	UINT8 transparent_color_b_g = (UINT8)(m_channel[1].transparent_color_b >>  8);
	UINT8 transparent_color_b_b = (UINT8)(m_channel[1].transparent_color_b >>  0);
	UINT8 image_coding_method_a = m_channel[0].image_coding_method & 0x0000000f;
	UINT8 image_coding_method_b = (m_channel[0].image_coding_method >> 8) & 0x0000000f;
	bool dyuv_enable_a = (image_coding_method_a == 5);
	bool dyuv_enable_b = (image_coding_method_b == 5);
	UINT8 mosaic_enable_a = (m_channel[0].mosaic_hold_a & 0x800000) >> 23;
	UINT8 mosaic_enable_b = (m_channel[1].mosaic_hold_b & 0x800000) >> 23;
	UINT8 mosaic_count_a = (m_channel[0].mosaic_hold_a & 0x0000ff) << 1;
	UINT8 mosaic_count_b = (m_channel[1].mosaic_hold_b & 0x0000ff) << 1;
	for(int x = 0; x < 768; x++)
	{
		out[x] = backdrop;
		if(!(m_channel[0].transparency_control & MCD212_TCR_DISABLE_MX))
		{
			UINT8 abr = MCD212_LIM(((MCD212_LIM((INT32)plane_a_r[x] - 16) * m_channel[0].weight_factor_a[x]) >> 6) + ((MCD212_LIM((INT32)plane_b_r[x] - 16) * m_channel[1].weight_factor_b[x]) >> 6) + 16);
			UINT8 abg = MCD212_LIM(((MCD212_LIM((INT32)plane_a_g[x] - 16) * m_channel[0].weight_factor_a[x]) >> 6) + ((MCD212_LIM((INT32)plane_b_g[x] - 16) * m_channel[1].weight_factor_b[x]) >> 6) + 16);
			UINT8 abb = MCD212_LIM(((MCD212_LIM((INT32)plane_a_b[x] - 16) * m_channel[0].weight_factor_a[x]) >> 6) + ((MCD212_LIM((INT32)plane_b_b[x] - 16) * m_channel[1].weight_factor_b[x]) >> 6) + 16);
			out[x] = (abr << 16) | (abg << 8) | abb;
		}
		else
		{
			UINT8 plane_enable_a = 0;
			UINT8 plane_enable_b = 0;
			UINT8 plane_a_r_cur = mosaic_enable_a ? plane_a_r[x - (x % mosaic_count_a)] : plane_a_r[x];
			UINT8 plane_a_g_cur = mosaic_enable_a ? plane_a_g[x - (x % mosaic_count_a)] : plane_a_g[x];
			UINT8 plane_a_b_cur = mosaic_enable_a ? plane_a_b[x - (x % mosaic_count_a)] : plane_a_b[x];
			UINT8 plane_b_r_cur = mosaic_enable_b ? plane_b_r[x - (x % mosaic_count_b)] : plane_b_r[x];
			UINT8 plane_b_g_cur = mosaic_enable_b ? plane_b_g[x - (x % mosaic_count_b)] : plane_b_g[x];
			UINT8 plane_b_b_cur = mosaic_enable_b ? plane_b_b[x - (x % mosaic_count_b)] : plane_b_b[x];
			switch(transparency_mode_a)
			{
				case 0:
					plane_enable_a = 0;
					break;
				case 1:
					plane_enable_a = (plane_a_r_cur != transparent_color_a_r || plane_a_g_cur != transparent_color_a_g || plane_a_b_cur != transparent_color_a_b);
					break;
				case 3:
					plane_enable_a = !m_region_flag_0[x];
					break;
				case 4:
					plane_enable_a = !m_region_flag_1[x];
					break;
				case 5:
					plane_enable_a = (plane_a_r_cur != transparent_color_a_r || plane_a_g_cur != transparent_color_a_g || plane_a_b_cur != transparent_color_a_b) && (dyuv_enable_a || m_region_flag_0[x] == 0);
					break;
				case 6:
					plane_enable_a = (plane_a_r_cur != transparent_color_a_r || plane_a_g_cur != transparent_color_a_g || plane_a_b_cur != transparent_color_a_b) && (dyuv_enable_a || m_region_flag_1[x] == 0);
					break;
				case 8:
					plane_enable_a = 1;
					break;
				case 9:
					plane_enable_a = (plane_a_r_cur == transparent_color_a_r && plane_a_g_cur == transparent_color_a_g && plane_a_b_cur == transparent_color_a_b);
					break;
				case 11:
					plane_enable_a = m_region_flag_0[x];
					break;
				case 12:
					plane_enable_a = m_region_flag_1[x];
					break;
				case 13:
					plane_enable_a = (plane_a_r_cur == transparent_color_a_r && plane_a_g_cur == transparent_color_a_g && plane_a_b_cur == transparent_color_a_b) || dyuv_enable_a || m_region_flag_0[x] == 1;
					break;
				case 14:
					plane_enable_a = (plane_a_r_cur == transparent_color_a_r && plane_a_g_cur == transparent_color_a_g && plane_a_b_cur == transparent_color_a_b) || dyuv_enable_a || m_region_flag_1[x] == 1;
					break;
				default:
					verboselog(*this, 0, "Unhandled transparency mode for plane A: %d\n", transparency_mode_a);
					plane_enable_a = 1;
					break;
			}
			switch(transparency_mode_b)
			{
				case 0:
					plane_enable_b = 0;
					break;
				case 1:
					plane_enable_b = (plane_b_r_cur != transparent_color_b_r || plane_b_g_cur != transparent_color_b_g || plane_b_b_cur != transparent_color_b_b);
					break;
				case 3:
					plane_enable_b = !m_region_flag_0[x];
					break;
				case 4:
					plane_enable_b = !m_region_flag_1[x];
					break;
				case 5:
					plane_enable_b = (plane_b_r_cur != transparent_color_b_r || plane_b_g_cur != transparent_color_b_g || plane_b_b_cur != transparent_color_b_b) && (dyuv_enable_b || m_region_flag_0[x] == 0);
					break;
				case 6:
					plane_enable_b = (plane_b_r_cur != transparent_color_b_r || plane_b_g_cur != transparent_color_b_g || plane_b_b_cur != transparent_color_b_b) && (dyuv_enable_b || m_region_flag_1[x] == 0);
					break;
				case 8:
					plane_enable_b = 1;
					break;
				case 9:
					plane_enable_b = (plane_b_r_cur == transparent_color_b_r && plane_b_g_cur == transparent_color_b_g && plane_b_b_cur == transparent_color_b_b);
					break;
				case 11:
					plane_enable_b = m_region_flag_0[x];
					break;
				case 12:
					plane_enable_b = m_region_flag_1[x];
					break;
				case 13:
					plane_enable_b = (plane_b_r_cur == transparent_color_b_r && plane_b_g_cur == transparent_color_b_g && plane_b_b_cur == transparent_color_b_b) || dyuv_enable_b || m_region_flag_0[x] == 1;
					break;
				case 14:
					plane_enable_b = (plane_b_r_cur == transparent_color_b_r && plane_b_g_cur == transparent_color_b_g && plane_b_b_cur == transparent_color_b_b) || dyuv_enable_b || m_region_flag_1[x] == 1;
					break;
				default:
					verboselog(*this, 0, "Unhandled transparency mode for plane B: %d\n", transparency_mode_b);
					plane_enable_b = 1;
					break;
			}
			if(global_plane_a_disable)
			{
				plane_enable_a = 0;
			}
			if(global_plane_b_disable)
			{
				plane_enable_b = 0;
			}
			plane_a_r_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_a_r_cur - 16) * m_channel[0].weight_factor_a[x]) >> 6) + 16);
			plane_a_g_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_a_g_cur - 16) * m_channel[0].weight_factor_a[x]) >> 6) + 16);
			plane_a_b_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_a_b_cur - 16) * m_channel[0].weight_factor_a[x]) >> 6) + 16);
			plane_b_r_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_b_r_cur - 16) * m_channel[1].weight_factor_b[x]) >> 6) + 16);
			plane_b_g_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_b_g_cur - 16) * m_channel[1].weight_factor_b[x]) >> 6) + 16);
			plane_b_b_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_b_b_cur - 16) * m_channel[1].weight_factor_b[x]) >> 6) + 16);
			switch(m_channel[0].plane_order)
			{
				case MCD212_POR_AB:
					if(plane_enable_a)
					{
						out[x] = (plane_a_r_cur << 16) | (plane_a_g_cur << 8) | plane_a_b_cur;
					}
					else if(plane_enable_b)
					{
						out[x] = (plane_b_r_cur << 16) | (plane_b_g_cur << 8) | plane_b_b_cur;
					}
					break;
				case MCD212_POR_BA:
					if(plane_enable_b)
					{
						out[x] = (plane_b_r_cur << 16) | (plane_b_g_cur << 8) | plane_b_b_cur;
					}
					else if(plane_enable_a)
					{
						out[x] = (plane_a_r_cur << 16) | (plane_a_g_cur << 8) | plane_a_b_cur;
					}
					break;
			}
		}
	}
}

void mcd212_device::draw_cursor(UINT32 *scanline, int y)
{
	if(m_channel[0].cursor_control & MCD212_CURCNT_EN)
	{
		UINT16 curx =  m_channel[0].cursor_position        & 0x3ff;
		UINT16 cury = ((m_channel[0].cursor_position >> 12) & 0x3ff) + 22;
		if(y >= cury && y < (cury + 16))
		{
			UINT32 color = s_4bpp_color[m_channel[0].cursor_control & MCD212_CURCNT_COLOR];
			y -= cury;
			if(m_channel[0].cursor_control & MCD212_CURCNT_CUW)
			{
				for(int x = curx; x < curx + 64 && x < 768; x++)
				{
					if(m_channel[0].cursor_pattern[y] & (1 << (15 - ((x - curx) >> 2))))
					{
						scanline[(x++)/2] = color;
						scanline[(x++)/2] = color;
						scanline[(x++)/2] = color;
						scanline[(x/2)] = color;
					}
					else
					{
					}
				}
			}
			else
			{
				for(int x = curx; x < curx + 32 && x < 768; x++)
				{
					if(m_channel[0].cursor_pattern[y] & (1 << (15 - ((x - curx) >> 1))))
					{
						scanline[(x++)/2] = color;
						scanline[x/2] = color;
					}
				}
			}
		}
	}
}

void mcd212_device::draw_scanline(int y)
{
	UINT8 plane_a_r[768], plane_a_g[768], plane_a_b[768];
	UINT8 plane_b_r[768], plane_b_g[768], plane_b_b[768];
	UINT32 out[768];
	UINT32 *scanline = &m_bitmap.pix32(y);
	int x;

	process_vsr(0, plane_a_r, plane_a_g, plane_a_b);
	process_vsr(1, plane_b_r, plane_b_g, plane_b_b);

	mix_lines(plane_a_r, plane_a_g, plane_a_b, plane_b_r, plane_b_g, plane_b_b, out);

	for(x = 0; x < 384; x++)
	{
		scanline[x] = out[x*2];
	}

	draw_cursor(scanline, y);
}

READ16_MEMBER( mcd212_device::regs_r )
{
	cdi_state *state = machine().driver_data<cdi_state>();
	UINT8 channel = 1 - (offset / 8);

	switch(offset)
	{
		case 0x00/2:
		case 0x10/2:
			if(ACCESSING_BITS_0_7)
			{
				verboselog(*this, 12, "mcd212_r: Status Register %d: %02x & %04x\n", channel + 1, m_channel[1 - (offset / 8)].csrr, mem_mask);
				if(channel == 0)
				{
					return m_channel[0].csrr;
				}
				else
				{
					UINT8 old_csr = m_channel[1].csrr;
					UINT8 interrupt1 = (state->m_scc->get_lir() >> 4) & 7;
					//UINT8 interrupt2 = state->m_scc68070_regs.lir & 7;
					m_channel[1].csrr &= ~(MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2);
					if(interrupt1)
					{
						state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt1 - 1), CLEAR_LINE);
					}
					//if(interrupt2)
					//{
					//  state->m_maincpu->set_input_line(M68K_IRQ_1 + (interrupt2 - 1), CLEAR_LINE);
					//}
					return old_csr;
				}
			}
			else
			{
				verboselog(*this, 2, "mcd212_r: Unknown Register %d: %04x\n", channel + 1, mem_mask);
			}
			break;
		case 0x02/2:
		case 0x12/2:
			verboselog(*this, 2, "mcd212_r: Display Command Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, m_channel[1 - (offset / 8)].dcr, mem_mask);
			return m_channel[1 - (offset / 8)].dcr;
		case 0x04/2:
		case 0x14/2:
			verboselog(*this, 2, "mcd212_r: Video Start Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, m_channel[1 - (offset / 8)].vsr, mem_mask);
			return m_channel[1 - (offset / 8)].vsr;
		case 0x08/2:
		case 0x18/2:
			verboselog(*this, 2, "mcd212_r: Display Decoder Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, m_channel[1 - (offset / 8)].ddr, mem_mask);
			return m_channel[1 - (offset / 8)].ddr;
		case 0x0a/2:
		case 0x1a/2:
			verboselog(*this, 2, "mcd212_r: DCA Pointer Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, m_channel[1 - (offset / 8)].dcp, mem_mask);
			return m_channel[1 - (offset / 8)].dcp;
		default:
			verboselog(*this, 2, "mcd212_r: Unknown Register %d & %04x\n", (1 - (offset / 8)) + 1, mem_mask);
			break;
	}

	return 0;
}

WRITE16_MEMBER( mcd212_device::regs_w )
{
	switch(offset)
	{
		case 0x00/2:
		case 0x10/2:
			verboselog(*this, 2, "mcd212_w: Status Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
			COMBINE_DATA(&m_channel[1 - (offset / 8)].csrw);
			update_visible_area();
			break;
		case 0x02/2:
		case 0x12/2:
			verboselog(*this, 2, "mcd212_w: Display Command Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
			COMBINE_DATA(&m_channel[1 - (offset / 8)].dcr);
			update_visible_area();
			break;
		case 0x04/2:
		case 0x14/2:
			verboselog(*this, 2, "mcd212_w: Video Start Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
			COMBINE_DATA(&m_channel[1 - (offset / 8)].vsr);
			break;
		case 0x08/2:
		case 0x18/2:
			verboselog(*this, 2, "mcd212_w: Display Decoder Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
			COMBINE_DATA(&m_channel[1 - (offset / 8)].ddr);
			break;
		case 0x0a/2:
		case 0x1a/2:
			verboselog(*this, 2, "mcd212_w: DCA Pointer Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
			COMBINE_DATA(&m_channel[1 - (offset / 8)].dcp);
			break;
		default:
			verboselog(*this, 2, "mcd212_w: Unknown Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
			break;
	}
}

TIMER_CALLBACK_MEMBER( mcd212_device::perform_scan )
{
	int scanline = m_screen->vpos();

	if(1)
	{
		if(scanline == 0)
		{
			// Process ICA
			verboselog(*this, 6, "%s", "Frame Start\n" );
			m_channel[0].csrr &= 0x7f;
			for(int index = 0; index < 2; index++)
			{
				if(m_channel[index].dcr & MCD212_DCR_ICA)
				{
					process_ica(index);
				}
			}
			draw_lcd(scanline);
		}
		else if(scanline < 22)
		{
			draw_lcd(scanline);
		}
		else if(scanline >= 22)
		{
			m_channel[0].csrr |= 0x80;
			// Process VSR
			draw_scanline(scanline);
			// Process DCA
			for(int index = 0; index < 2; index++)
			{
				if(m_channel[index].dcr & MCD212_DCR_DCA)
				{
					if(scanline == 22)
					{
						m_channel[index].dca = get_dcp(index);
					}
					process_dca(index);
				}
			}
			if(scanline == 301)
			{
				m_channel[0].csrr ^= 0x20;
			}
		}
	}
	m_scan_timer->adjust(m_screen->time_until_pos(( scanline + 1 ) % 302, 0));
}

void mcd212_device::device_reset()
{
	for(auto & elem : m_channel)
	{
		elem.csrr = 0;
		elem.csrw = 0;
		elem.dcr = 0;
		elem.vsr = 0;
		elem.ddr = 0;
		elem.dcp = 0;
		elem.dca = 0;
		memset(elem.clut_r, 0, 256);
		memset(elem.clut_g, 0, 256);
		memset(elem.clut_b, 0, 256);
		elem.image_coding_method = 0;
		elem.transparency_control = 0;
		elem.plane_order = 0;
		elem.clut_bank = 0;
		elem.transparent_color_a = 0;
		elem.transparent_color_b = 0;
		elem.mask_color_a = 0;
		elem.mask_color_b = 0;
		elem.dyuv_abs_start_a = 0;
		elem.dyuv_abs_start_b = 0;
		elem.cursor_position = 0;
		elem.cursor_control = 0;
		memset((UINT8*)&elem.cursor_pattern, 0, 16 * sizeof(UINT32));
		memset((UINT8*)&elem.region_control, 0, 8 * sizeof(UINT32));
		elem.backdrop_color = 0;
		elem.mosaic_hold_a = 0;
		elem.mosaic_hold_b = 0;
		memset(elem.weight_factor_a, 0, 768);
		memset(elem.weight_factor_b, 0, 768);
	}
	memset(m_region_flag_0, 0, 768);
	memset(m_region_flag_1, 0, 768);
}

//-------------------------------------------------
//  mcd212_device - constructor
//-------------------------------------------------

mcd212_device::mcd212_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MACHINE_MCD212, "MCD212 Video", tag, owner, clock, "mcd212", __FILE__),
		device_video_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcd212_device::device_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	m_scan_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mcd212_device::perform_scan), this));
	m_scan_timer->adjust(m_screen->time_until_pos(0, 0));

	save_item(NAME(m_region_flag_0));
	save_item(NAME(m_region_flag_1));
	save_item(NAME(m_channel[0].csrr));
	save_item(NAME(m_channel[0].csrw));
	save_item(NAME(m_channel[0].dcr));
	save_item(NAME(m_channel[0].vsr));
	save_item(NAME(m_channel[0].ddr));
	save_item(NAME(m_channel[0].dcp));
	save_item(NAME(m_channel[0].dca));
	save_item(NAME(m_channel[0].clut_r));
	save_item(NAME(m_channel[0].clut_g));
	save_item(NAME(m_channel[0].clut_b));
	save_item(NAME(m_channel[0].image_coding_method));
	save_item(NAME(m_channel[0].transparency_control));
	save_item(NAME(m_channel[0].plane_order));
	save_item(NAME(m_channel[0].clut_bank));
	save_item(NAME(m_channel[0].transparent_color_a));
	save_item(NAME(m_channel[0].transparent_color_b));
	save_item(NAME(m_channel[0].mask_color_a));
	save_item(NAME(m_channel[0].mask_color_b));
	save_item(NAME(m_channel[0].dyuv_abs_start_a));
	save_item(NAME(m_channel[0].dyuv_abs_start_b));
	save_item(NAME(m_channel[0].cursor_position));
	save_item(NAME(m_channel[0].cursor_control));
	save_item(NAME(m_channel[0].cursor_pattern));
	save_item(NAME(m_channel[0].region_control));
	save_item(NAME(m_channel[0].backdrop_color));
	save_item(NAME(m_channel[0].mosaic_hold_a));
	save_item(NAME(m_channel[0].mosaic_hold_b));
	save_item(NAME(m_channel[0].weight_factor_a));
	save_item(NAME(m_channel[0].weight_factor_b));
	save_item(NAME(m_channel[1].csrr));
	save_item(NAME(m_channel[1].csrw));
	save_item(NAME(m_channel[1].dcr));
	save_item(NAME(m_channel[1].vsr));
	save_item(NAME(m_channel[1].ddr));
	save_item(NAME(m_channel[1].dcp));
	save_item(NAME(m_channel[1].dca));
	save_item(NAME(m_channel[1].clut_r));
	save_item(NAME(m_channel[1].clut_g));
	save_item(NAME(m_channel[1].clut_b));
	save_item(NAME(m_channel[1].image_coding_method));
	save_item(NAME(m_channel[1].transparency_control));
	save_item(NAME(m_channel[1].plane_order));
	save_item(NAME(m_channel[1].clut_bank));
	save_item(NAME(m_channel[1].transparent_color_a));
	save_item(NAME(m_channel[1].transparent_color_b));
	save_item(NAME(m_channel[1].mask_color_a));
	save_item(NAME(m_channel[1].mask_color_b));
	save_item(NAME(m_channel[1].dyuv_abs_start_a));
	save_item(NAME(m_channel[1].dyuv_abs_start_b));
	save_item(NAME(m_channel[1].cursor_position));
	save_item(NAME(m_channel[1].cursor_control));
	save_item(NAME(m_channel[1].cursor_pattern));
	save_item(NAME(m_channel[1].region_control));
	save_item(NAME(m_channel[1].backdrop_color));
	save_item(NAME(m_channel[1].mosaic_hold_a));
	save_item(NAME(m_channel[1].mosaic_hold_b));
	save_item(NAME(m_channel[1].weight_factor_a));
	save_item(NAME(m_channel[1].weight_factor_b));
}

void mcd212_device::ab_init()
{
	// Delta decoding array.
	static const BYTE68K abDelta[16] = { 0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255 };

	// Initialize delta decoding arrays for each unsigned byte value b.
	for (WORD68K d = 0; d < BYTE68K_MAX + 1; d++)
	{
		m_ab.deltaY[d] = abDelta[d & 15];
	}

	// Initialize delta decoding arrays for each unsigned byte value b.
	for (WORD68K d = 0; d < (BYTE68K_MAX + 1); d++)
	{
		m_ab.deltaUV[d] = abDelta[d >> 4];
	}

	// Initialize color limit and clamp arrays.
	for (WORD68K w = 0; w < 3 * BYTE68K_MAX; w++)
	{
		m_ab.limit[w] = (w < BYTE68K_MAX + 16) ?  0 : w <= 16 + 2 * BYTE68K_MAX ? w - BYTE68K_MAX - 16 : BYTE68K_MAX;
		m_ab.clamp[w] = (w < BYTE68K_MAX + 32) ? 16 : w <= 16 + 2 * BYTE68K_MAX ? w - BYTE68K_MAX - 16 : BYTE68K_MAX;
	}

	for (SWORD68K sw = 0; sw < 0x100; sw++)
	{
		m_ab.matrixUB[sw] = (444 * (sw - 128)) / 256;
		m_ab.matrixUG[sw] = - (86 * (sw - 128)) / 256;
		m_ab.matrixVG[sw] = - (179 * (sw - 128)) / 256;
		m_ab.matrixVR[sw] = (351 * (sw - 128)) / 256;
	}
}

void cdi_state::video_start()
{
	m_mcd212->ab_init();

	screen_device *screen = downcast<screen_device *>(machine().device("lcd"));
	screen->register_screen_bitmap(m_lcdbitmap);
}

UINT32 cdi_state::screen_update_cdimono1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_mcd212->get_bitmap(), 0, 0, 0, 0, cliprect);
	return 0;
}

UINT32 cdi_state::screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_lcdbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
