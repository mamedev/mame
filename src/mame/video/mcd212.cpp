// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i MCD212 Video Decoder and System Controller emulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Unknown yet.

*******************************************************************************/

#include "emu.h"
#include "video/mcd212.h"

#include "screen.h"

#define LOG_UNKNOWNS		(1U << 1)
#define LOG_REGISTERS		(1U << 2)
#define LOG_ICA				(1U << 3)
#define LOG_DCA				(1U << 4)
#define LOG_VSR				(1U << 5)
#define LOG_STATUS			(1U << 6)
#define LOG_MAIN_REG_READS	(1U << 7)
#define LOG_MAIN_REG_WRITES	(1U << 8)
#define LOG_CLUT			(1U << 9)
#define LOG_ALL				(LOG_UNKNOWNS | LOG_REGISTERS | LOG_ICA | LOG_DCA | LOG_VSR | LOG_STATUS | LOG_MAIN_REG_READS | LOG_MAIN_REG_WRITES | LOG_CLUT)

#define VERBOSE				(LOG_ALL)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(MCD212, mcd212_device, "mcd212", "MCD212 VDSC")

inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_weight_factor(const uint32_t region_idx)
{
	return (m_channel[0].region_control[region_idx] & RC_WF) >> RC_WF_SHIFT;
}

inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_region_op(const uint32_t region_idx)
{
	return (m_channel[0].region_control[region_idx] & RC_OP) >> RC_OP_SHIFT;
}

void mcd212_device::update_region_arrays()
{
	int latched_rf[2] { 0, 0 };
	uint8_t latched_wfa = m_channel[0].weight_factor[0];
	uint8_t latched_wfb = m_channel[1].weight_factor[0];

	if (BIT(m_channel[0].image_coding_method, ICM_NR_BIT))
	{
		if (get_region_op(0) == 0 && get_region_op(4) == 0)
		{
			std::fill_n(m_channel[0].weight_factor, std::size(m_channel[0].weight_factor), latched_wfa);
			std::fill_n(m_channel[1].weight_factor, std::size(m_channel[1].weight_factor), latched_wfb);
			std::fill_n(m_region_flag_0, std::size(m_region_flag_0), 0);
			std::fill_n(m_region_flag_1, std::size(m_region_flag_1), 0);
			return;
		}

		for (int x = 0; x < 768; x++)
		{
			for (int flag = 0; flag < 2; flag++)
			{
				for (int region = 0; region < 4; region++)
				{
					const int region_idx = (flag << 2) + region;
					const uint32_t region_ctrl = m_channel[0].region_control[region_idx];
					const uint32_t region_op = get_region_op(region_idx);
					if (region_op == 0)
					{
						break;
					}
					if (x == (region_ctrl & RC_X))
					{
						switch (region_op)
						{
							case 0: // End of region control for line
								break;
							case 1:
							case 2:
							case 3: // Not used
								break;
							case 4: // Change weight of plane A
								latched_wfa = get_weight_factor(region_idx);
								break;
							case 5: // Not used
								break;
							case 6: // Change weight of plane B
								latched_wfb = get_weight_factor(region_idx);
								break;
							case 7: // Not used
								break;
							case 8: // Reset region flag
								latched_rf[flag] = 0;
								break;
							case 9: // Set region flag
								latched_rf[flag] = 1;
								break;
							case 10:    // Not used
							case 11:    // Not used
								break;
							case 12: // Reset region flag and change weight of plane A
								latched_wfa = get_weight_factor(region_idx);
								latched_rf[flag] = 0;
								break;
							case 13: // Set region flag and change weight of plane A
								latched_wfa = get_weight_factor(region_idx);
								latched_rf[flag] = 1;
								break;
							case 14: // Reset region flag and change weight of plane B
								latched_wfb = get_weight_factor(region_idx);
								latched_rf[flag] = 0;
								break;
							case 15: // Set region flag and change weight of plane B
								latched_wfb = get_weight_factor(region_idx);
								latched_rf[flag] = 1;
								break;
						}
					}
				}
			}
			m_channel[0].weight_factor[x] = latched_wfa;
			m_channel[1].weight_factor[x] = latched_wfb;
			m_region_flag_0[x] = latched_rf[0];
			m_region_flag_1[x] = latched_rf[1];
		}
	}
	else
	{
		int region_idx = 0;
		for (int x = 0; x < 768; x++)
		{
			if (region_idx < 8)
			{
				const int flag = BIT(m_channel[0].region_control[region_idx], RC_RF_BIT);
				const uint32_t region_ctrl = m_channel[0].region_control[region_idx];
				const uint32_t region_op = get_region_op(region_idx);
				if (region_op == 0)
				{
					std::fill_n(m_channel[0].weight_factor + x, std::size(m_channel[0].weight_factor) - x, latched_wfa);
					std::fill_n(m_channel[1].weight_factor + x, std::size(m_channel[1].weight_factor) - x, latched_wfb);
					std::fill_n(m_region_flag_0 + x, std::size(m_region_flag_0) - x, 0);
					std::fill_n(m_region_flag_1 + x, std::size(m_region_flag_1) - x, 0);
					return;
				}
				if (x == (region_ctrl & RC_X))
				{
					switch (region_op)
					{
						case 0: // End of region control for line
							break;
						case 1:
						case 2:
						case 3: // Not used
							break;
						case 4: // Change weight of plane A
							latched_wfa = get_weight_factor(region_idx);
							break;
						case 5: // Not used
							break;
						case 6: // Change weight of plane B
							latched_wfb = get_weight_factor(region_idx);
							break;
						case 7: // Not used
							break;
						case 8: // Reset region flag
							latched_rf[flag] = 0;
							break;
						case 9: // Set region flag
							latched_rf[flag] = 1;
							break;
						case 10:    // Not used
						case 11:    // Not used
							break;
						case 12: // Reset region flag and change weight of plane A
							latched_wfa = get_weight_factor(region_idx);
							latched_rf[flag] = 0;
							break;
						case 13: // Set region flag and change weight of plane A
							latched_wfa = get_weight_factor(region_idx);
							latched_rf[flag] = 1;
							break;
						case 14: // Reset region flag and change weight of plane B
							latched_wfb = get_weight_factor(region_idx);
							latched_rf[flag] = 0;
							break;
						case 15: // Set region flag and change weight of plane B
							latched_wfb = get_weight_factor(region_idx);
							latched_rf[flag] = 1;
							break;
					}
					region_idx++;
				}
			}
			m_channel[0].weight_factor[x] = latched_wfa;
			m_channel[1].weight_factor[x] = latched_wfb;
			m_region_flag_0[x] = latched_rf[0];
			m_region_flag_1[x] = latched_rf[1];
		}
	}
}

template <int Channel>
void mcd212_device::set_register(uint8_t reg, uint32_t value)
{
	switch (reg)
	{
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: // CLUT 0 - 63
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			LOGMASKED(LOG_CLUT, "%s: Channel %d: CLUT[%d] = %08x\n", machine().describe_context(), Channel, m_channel[Channel].clut_bank * 0x40 + (reg - 0x80), value );
			m_channel[0].clut_r[m_channel[Channel].clut_bank * 0x40 + (reg - 0x80)] = (uint8_t)(value >> 16) & 0xfc;
			m_channel[0].clut_g[m_channel[Channel].clut_bank * 0x40 + (reg - 0x80)] = (uint8_t)(value >>  8) & 0xfc;
			m_channel[0].clut_b[m_channel[Channel].clut_bank * 0x40 + (reg - 0x80)] = (uint8_t)(value >>  0) & 0xfc;
			break;
		case 0xc0: // Image Coding Method
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Channel 0: Image Coding Method = %08x\n", machine().describe_context(), value );
				m_channel[0].image_coding_method = value;
			}
			break;
		case 0xc1: // Transparency Control
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Transparency Control = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].transparency_control = value;
			}
			break;
		case 0xc2: // Plane Order
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Plane Order = %08x\n", machine().describe_context(), screen().vpos(), value & 7);
				m_channel[0].plane_order = value & 0x00000007;
			}
			break;
		case 0xc3: // CLUT Bank Register
			LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel %d: CLUT Bank Register = %08x\n", machine().describe_context(), screen().vpos(), Channel, value & 3);
			m_channel[Channel].clut_bank = Channel ? (2 | (value & 0x00000001)) : (value & 0x00000003);
			break;
		case 0xc4: // Transparent Color A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Transparent Color A = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].transparent_color = value & 0xfcfcfc;
			}
			break;
		case 0xc6: // Transparent Color B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Transparent Color B = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[1].transparent_color = value & 0xfcfcfc;
			}
			break;
		case 0xc7: // Mask Color A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Mask Color A = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].mask_color = value & 0xfcfcfc;
			}
			break;
		case 0xc9: // Mask Color B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Mask Color B = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[1].mask_color = value & 0xfcfcfc;
			}
			break;
		case 0xca: // Delta YUV Absolute Start Value A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Delta YUV Absolute Start Value A = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].dyuv_abs_start = value;
			}
			break;
		case 0xcb: // Delta YUV Absolute Start Value B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Delta YUV Absolute Start Value B = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[1].dyuv_abs_start = value;
			}
			break;
		case 0xcd: // Cursor Position
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Cursor Position = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].cursor_position = value;
			}
			break;
		case 0xce: // Cursor Control
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Cursor Control = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].cursor_control = value;
			}
			break;
		case 0xcf: // Cursor Pattern
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Cursor Pattern[%d] = %04x\n", machine().describe_context(), screen().vpos(), (value >> 16) & 0x000f, value & 0x0000ffff);
				m_channel[0].cursor_pattern[(value >> 16) & 0x000f] = value & 0x0000ffff;
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
			LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel %d: Region Control %d = %08x\n", machine().describe_context(), screen().vpos(), Channel, reg & 7, value );
			m_channel[0].region_control[reg & 7] = value;
			update_region_arrays();
			break;
		case 0xd8: // Backdrop Color
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Backdrop Color = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].backdrop_color = value;
			}
			break;
		case 0xd9: // Mosaic Pixel Hold Factor A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Mosaic Pixel Hold Factor A = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[0].mosaic_hold = value;
			}
			break;
		case 0xda: // Mosaic Pixel Hold Factor B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Mosaic Pixel Hold Factor B = %08x\n", machine().describe_context(), screen().vpos(), value );
				m_channel[1].mosaic_hold = value;
			}
			break;
		case 0xdb: // Weight Factor A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Weight Factor A = %08x\n", machine().describe_context(), screen().vpos(), value );
				memset(m_channel[0].weight_factor, value & 0x000000ff, 768);
				update_region_arrays();
			}
			break;
		case 0xdc: // Weight Factor B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Weight Factor B = %08x\n", machine().describe_context(), screen().vpos(), value );
				memset(m_channel[1].weight_factor, value & 0x000000ff, 768);
				update_region_arrays();
			}
			break;
	}
}

template <int Channel>
inline ATTR_FORCE_INLINE uint32_t mcd212_device::get_vsr()
{
	return ((m_channel[Channel].dcr & 0x3f) << 16) | m_channel[Channel].vsr;
}

template <int Channel>
inline ATTR_FORCE_INLINE void mcd212_device::set_vsr(uint32_t value)
{
	m_channel[Channel].vsr = value & 0x0000ffff;
	m_channel[Channel].dcr &= 0xffc0;
	m_channel[Channel].dcr |= (value >> 16) & 0x003f;
}

template <int Channel>
inline ATTR_FORCE_INLINE void mcd212_device::set_dcp(uint32_t value)
{
	m_channel[Channel].dcp = value & 0x0000ffff;
	m_channel[Channel].ddr &= 0xffc0;
	m_channel[Channel].ddr |= (value >> 16) & 0x003f;
}

template <int Channel>
inline ATTR_FORCE_INLINE uint32_t mcd212_device::get_dcp()
{
	return ((m_channel[Channel].ddr & 0x3f) << 16) | m_channel[Channel].dcp;
}

template <int Channel>
inline ATTR_FORCE_INLINE void mcd212_device::set_display_parameters(uint8_t value)
{
	m_channel[Channel].ddr &= 0xf0ff;
	m_channel[Channel].ddr |= (value & 0x0f) << 8;
	m_channel[Channel].dcr &= 0xf7ff;
	m_channel[Channel].dcr |= (value & 0x10) << 7;
}

void mcd212_device::update_visible_area()
{
	rectangle visarea;
	attoseconds_t period = screen().frame_period().attoseconds();

	const bool st_set = (m_channel[0].csrw & CSR1W_ST) != 0;
	const bool fd_set = (m_channel[0].dcr & (DCR_CF | DCR_FD)) != 0;
	int total_width = 384;
	if (fd_set && st_set)
		total_width = 360;

	const bool pal = !(m_channel[0].dcr & DCR_FD);
	const int total_height = (pal ? 312 : 262);

	int visible_height = 240;
	if (pal && !st_set)
		visible_height = 280;

	m_ica_height = total_height - visible_height;
	m_total_height = total_height;

	visarea.min_y = m_ica_height;
	visarea.max_y = total_height - 1;
	visarea.min_x = 0;
	visarea.max_x = total_width - 1;

	screen().configure(total_width, total_height, visarea, period);
}

uint32_t mcd212_device::get_screen_width()
{
	if ((m_channel[0].dcr & (DCR_CF | DCR_FD)) && (m_channel[0].csrw & CSR1W_ST))
	{
		return 720;
	}
	return 768;
}

template <int Channel>
void mcd212_device::process_ica()
{
	uint16_t *ica = Channel ? m_planeb.target() : m_planea.target();
	uint32_t addr = 0x200;
	uint32_t cmd = 0;

	const int max_to_process = m_ica_height * 120;
	for (int i = 0; i < max_to_process; i++)
	{
		cmd = ica[addr++] << 16;
		cmd |= ica[addr++];
		switch ((cmd & 0xff000000) >> 24)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // STOP
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: STOP\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				return;
			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // NOP
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: NOP\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				break;
			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // RELOAD DCP
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: RELOAD DCP: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_dcp<Channel>(cmd & 0x003ffffc);
				break;
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // RELOAD DCP and STOP
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: RELOAD DCP and STOP: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_dcp<Channel>(cmd & 0x003ffffc);
				return;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD VSR (ICA)
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: RELOAD VSR: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				addr = (cmd & 0x0007ffff) / 2;
				break;
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: RELOAD VSR and STOP: VSR = %05x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_vsr<Channel>(cmd & 0x003fffff);
				return;
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: INTERRUPT\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				m_channel[1].csrr |= 1 << (2 - Channel);
				if (m_channel[1].csrr & (CSR2R_IT1 | CSR2R_IT2))
					m_int_callback(ASSERT_LINE);
				break;
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: RELOAD DISPLAY PARAMETERS\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				set_display_parameters<Channel>(cmd & 0x1f);
				break;
			default:
				LOGMASKED(LOG_ICA, "%08x: %08x: ICA %d: SET REGISTER %02x = %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd >> 24, cmd & 0x00ffffff );
				set_register<Channel>(cmd >> 24, cmd & 0x00ffffff);
				break;
		}
	}
}

template <int Channel>
void mcd212_device::process_dca()
{
	if (screen().vpos() == m_ica_height)
	{
		m_channel[Channel].dca = get_dcp<Channel>();
	}

	uint16_t *dca = Channel ? m_planeb.target() : m_planea.target();
	uint32_t addr = (m_channel[Channel].dca & 0x0007ffff) / 2;
	uint32_t cmd = 0;
	uint32_t count = 0;
	uint32_t max = 64;
	uint8_t addr_changed = 0;

	while(1)
	{
		uint8_t stop = 0;
		cmd = dca[addr++] << 16;
		cmd |= dca[addr++];
		count += 4;
		switch ((cmd & 0xff000000) >> 24)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // STOP
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: STOP\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				stop = 1;
				break;
			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // NOP
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: NOP\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				break;
			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // RELOAD DCP
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD DCP (NOP)\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				break;
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // RELOAD DCP and STOP
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD DCP and STOP\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				set_dcp<Channel>(cmd & 0x003ffffc);
				addr = (cmd & 0x0007fffc) / 2;
				addr_changed = 1;
				stop = 1;
				break;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD VSR
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD VSR: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_vsr<Channel>(cmd & 0x003fffff);
				break;
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD VSR and STOP: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_vsr<Channel>(cmd & 0x003fffff);
				stop = 1;
				break;
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: INTERRUPT\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				m_channel[1].csrr |= 1 << (2 - Channel);
				if (m_channel[1].csrr & (CSR2R_IT1 | CSR2R_IT2))
					m_int_callback(ASSERT_LINE);
				break;
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD DISPLAY PARAMETERS\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				set_display_parameters<Channel>(cmd & 0x1f);
				break;
			default:
				set_register<Channel>(cmd >> 24, cmd & 0x00ffffff);
				break;
		}
		if (stop != 0 || count == max)
		{
			break;
		}
	}
	if (!addr_changed)
	{
		if (count < max)
		{
			addr += (max - count) >> 1;
		}
	}
	m_channel[Channel].dca = addr * 2;
}

static inline uint8_t BYTE_TO_CLUT(int channel, int icm, uint8_t byte)
{
	switch (icm)
	{
		case 1:
			return byte;
		case 3:
			if (channel)
			{
				return 0x80 + (byte & 0x7f);
			}
			else
			{
				return byte & 0x7f;
			}
		case 4:
			if (!channel)
			{
				return byte & 0x7f;
			}
			break;
		case 11:
			if (channel)
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

template <int Channel>
inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_icm()
{
	const uint32_t mask = Channel ? ICM_MODE2 : ICM_MODE1;
	const uint32_t shift = Channel ? ICM_MODE2_SHIFT : ICM_MODE1_SHIFT;
	return (m_channel[0].image_coding_method & mask) >> shift;
}

template <int Channel>
inline ATTR_FORCE_INLINE bool mcd212_device::get_mosaic_enable()
{
	return (m_channel[Channel].ddr & DDR_FT) == DDR_FT_MOSAIC;
}

template <int Channel>
inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_mosaic_factor()
{
	return 1 << (((m_channel[Channel].ddr & DDR_MT) >> DDR_MT_SHIFT) + 1);
}

template <int Channel>
void mcd212_device::process_vsr(uint8_t *pixels_r, uint8_t *pixels_g, uint8_t *pixels_b)
{
	uint8_t *data = reinterpret_cast<uint8_t *>(Channel ? m_planeb.target() : m_planea.target());
	uint32_t vsr = get_vsr<Channel>();
	uint8_t done = 0;
	uint32_t x = 0;
	uint8_t icm = get_icm<Channel>();
	uint8_t *clut_r = m_channel[0].clut_r;
	uint8_t *clut_g = m_channel[0].clut_g;
	uint8_t *clut_b = m_channel[0].clut_b;
	uint8_t mosaic_enable = get_mosaic_enable<Channel>();
	uint8_t mosaic_factor = get_mosaic_factor<Channel>();
	int mosaic_index = 0;
	uint32_t width = get_screen_width();

	if (!icm || !vsr)
	{
		memset(pixels_r, 0x10, width);
		memset(pixels_g, 0x10, width);
		memset(pixels_b, 0x10, width);
		return;
	}

	LOGMASKED(LOG_VSR, "Scanline %d: VSR Channel %d, ICM (%02x), VSR (%08x)\n", screen().vpos(), Channel, icm, vsr);

	while(!done)
	{
		uint8_t byte = data[(vsr & 0x0007ffff) ^ 1];
		LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: VSR[%05x] = %02x\n", screen().vpos(), Channel, (vsr & 0x0007ffff), byte);
		vsr++;
		switch (m_channel[Channel].ddr & DDR_FT)
		{
			case DDR_FT_BMP:
			case DDR_FT_BMP2:
			case DDR_FT_MOSAIC:
				if ((m_channel[Channel].ddr & DDR_FT) == DDR_FT_BMP)
				{
					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: BMP\n", screen().vpos(), Channel);
				}
				else if ((m_channel[Channel].ddr & DDR_FT) == DDR_FT_BMP2)
				{
					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: BMP2\n", screen().vpos(), Channel);
				}
				else if ((m_channel[Channel].ddr & DDR_FT) == DDR_FT_MOSAIC)
				{
					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: MOSAIC\n", screen().vpos(), Channel);
				}
				if (m_channel[Channel].dcr & DCR_CM)
				{
					// 4-bit Bitmap
					LOGMASKED(LOG_UNKNOWNS, "%s", "Unsupported display mode: 4-bit Bitmap\n" );
				}
				else
				{
					// 8-bit Bitmap
					if (icm == 5)
					{
						LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: DYUV\n", screen().vpos(), Channel);
						uint8_t bY = (m_channel[Channel].dyuv_abs_start >> 16) & 0x000000ff;
						uint8_t bU = (m_channel[Channel].dyuv_abs_start >>  8) & 0x000000ff;
						uint8_t bV = (m_channel[Channel].dyuv_abs_start >>  0) & 0x000000ff;
						for (; x < width; x += 2)
						{
							uint8_t b0 = byte;
							uint8_t bU1 = bU + m_ab.deltaUV[b0];
							uint8_t bY0 = bY + m_ab.deltaY[b0];

							uint8_t b1 = data[(vsr & 0x0007ffff) ^ 1];
							uint8_t bV1 = bV + m_ab.deltaUV[b1];
							uint8_t bY1 = bY0 + m_ab.deltaY[b1];

							uint8_t bU0 = (bU + bU1) >> 1;
							uint8_t bV0 = (bV + bV1) >> 1;

							uint8_t *pbLimit;

							vsr++;

							bY = bY0;
							bU = bU0;
							bV = bV0;

							pbLimit = m_ab.limit + bY + 0xff;

							pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[m_ab.matrixVR[bV]];
							pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[m_ab.matrixUG[bU] + m_ab.matrixVG[bV]];
							pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[m_ab.matrixUB[bU]];

							if (mosaic_enable)
							{
								for (mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
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

							pbLimit = m_ab.limit + bY + 0xff;

							pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[m_ab.matrixVR[bV]];
							pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[m_ab.matrixUG[bU] + m_ab.matrixVG[bV]];
							pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[m_ab.matrixUB[bU]];

							if (mosaic_enable)
							{
								for (mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
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
						set_vsr<Channel>(vsr - 1);
					}
					else if (icm == 1 || icm == 3 || icm == 4)
					{
						for (; x < width; x += 2)
						{
							uint8_t clut_entry = BYTE_TO_CLUT(Channel, icm, byte);
							pixels_r[x + 0] = clut_r[clut_entry];
							pixels_g[x + 0] = clut_g[clut_entry];
							pixels_b[x + 0] = clut_b[clut_entry];
							pixels_r[x + 1] = clut_r[clut_entry];
							pixels_g[x + 1] = clut_g[clut_entry];
							pixels_b[x + 1] = clut_b[clut_entry];
							if (mosaic_enable)
							{
								for (mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
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
						set_vsr<Channel>(vsr - 1);
					}
					else if (icm == 11)
					{
						for (; x < width; x += 2)
						{
							uint8_t even_entry = BYTE_TO_CLUT(Channel, icm, byte >> 4);
							uint8_t odd_entry = BYTE_TO_CLUT(Channel, icm, byte);
							if (mosaic_enable)
							{
								for (mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
								{
									pixels_r[x + mosaic_index] = clut_r[even_entry];
									pixels_g[x + mosaic_index] = clut_g[even_entry];
									pixels_b[x + mosaic_index] = clut_b[even_entry];
								}
								for (mosaic_index = 0; mosaic_index < mosaic_factor; mosaic_index++)
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
						set_vsr<Channel>(vsr - 1);
					}
					else
					{
						for (; x < width; x++)
						{
							pixels_r[x] = 0x10;
							pixels_g[x] = 0x10;
							pixels_b[x] = 0x10;
						}
					}
				}
				done = 1;
				break;
			case DDR_FT_RLE:
				LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: RLE\n", screen().vpos(), Channel);
				if (m_channel[Channel].dcr & DCR_CM)
				{
					LOGMASKED(LOG_UNKNOWNS, "%s", "Unsupported display mode: 4-bit RLE\n" );
					done = 1;
				}
				else
				{
					if (byte & 0x80)
					{
						// Run length
						uint8_t length = data[((vsr++) & 0x0007ffff) ^ 1];
						LOGMASKED(LOG_VSR, "Byte %02x w/ run length %02x at %d\n", byte, length, x);
						if (!length)
						{
							uint8_t clut_entry = BYTE_TO_CLUT(Channel, icm, byte & 0x7f);
							uint8_t r = clut_r[clut_entry];
							uint8_t g = clut_g[clut_entry];
							uint8_t b = clut_b[clut_entry];
							// Go to the end of the line
							for (; x < width; x++)
							{
								pixels_r[x] = r;
								pixels_g[x] = g;
								pixels_b[x] = b;
							}
							done = 1;
							set_vsr<Channel>(vsr);
						}
						else
						{
							int end = x + (length * 2);
							uint8_t clut_entry = BYTE_TO_CLUT(Channel, icm, byte & 0x7f);
							uint8_t r = clut_r[clut_entry];
							uint8_t g = clut_g[clut_entry];
							uint8_t b = clut_b[clut_entry];
							for (; x < end && x < width; x++)
							{
								pixels_r[x] = r;
								pixels_g[x] = g;
								pixels_b[x] = b;
							}
							if (x >= width)
							{
								done = 1;
								set_vsr<Channel>(vsr);
							}
						}
					}
					else
					{
						LOGMASKED(LOG_VSR, "Byte %02x, single at %d\n", byte, x);
						// Single pixel
						uint8_t clut_entry = BYTE_TO_CLUT(Channel, icm, byte);
						pixels_r[x] = clut_r[clut_entry];
						pixels_g[x] = clut_g[clut_entry];
						pixels_b[x] = clut_b[clut_entry];
						x++;
						pixels_r[x] = clut_r[clut_entry];
						pixels_g[x] = clut_g[clut_entry];
						pixels_b[x] = clut_b[clut_entry];
						x++;
						if (x >= width)
						{
							done = 1;
							set_vsr<Channel>(vsr);
						}
					}
				}
				break;
		}
	}
}

const uint32_t mcd212_device::s_4bpp_color[16] =
{
	0x00101010, 0x0010107a, 0x00107a10, 0x00107a7a, 0x007a1010, 0x007a107a, 0x007a7a10, 0x007a7a7a,
	0x00101010, 0x001010e6, 0x0010e610, 0x0010e6e6, 0x00e61010, 0x00e610e6, 0x00e6e610, 0x00e6e6e6
};

void mcd212_device::mix_lines(uint8_t *plane_a_r, uint8_t *plane_a_g, uint8_t *plane_a_b, uint8_t *plane_b_r, uint8_t *plane_b_g, uint8_t *plane_b_b, uint32_t *out)
{
	const uint32_t backdrop = s_4bpp_color[m_channel[0].backdrop_color];
	const uint8_t transparency_mode_a = (m_channel[0].transparency_control >> 0) & 0x0f;
	const uint8_t transparency_mode_b = (m_channel[0].transparency_control >> 8) & 0x0f;
	const uint8_t transparent_color_a_r = (uint8_t)(m_channel[0].transparent_color >> 16);
	const uint8_t transparent_color_a_g = (uint8_t)(m_channel[0].transparent_color >>  8);
	const uint8_t transparent_color_a_b = (uint8_t)(m_channel[0].transparent_color >>  0);
	const uint8_t transparent_color_b_r = (uint8_t)(m_channel[1].transparent_color >> 16);
	const uint8_t transparent_color_b_g = (uint8_t)(m_channel[1].transparent_color >>  8);
	const uint8_t transparent_color_b_b = (uint8_t)(m_channel[1].transparent_color >>  0);
	const uint8_t image_coding_method_a = m_channel[0].image_coding_method & 0x0000000f;
	const uint8_t image_coding_method_b = (m_channel[0].image_coding_method >> 8) & 0x0000000f;
	const bool dyuv_enable_a = (image_coding_method_a == 5);
	const bool dyuv_enable_b = (image_coding_method_b == 5);
	const uint8_t mosaic_enable_a = (m_channel[0].mosaic_hold & 0x800000) >> 23;
	const uint8_t mosaic_enable_b = (m_channel[1].mosaic_hold & 0x800000) >> 23;
	const uint8_t mosaic_count_a = (m_channel[0].mosaic_hold & 0x0000ff) << 1;
	const uint8_t mosaic_count_b = (m_channel[1].mosaic_hold & 0x0000ff) << 1;

	for (int x = 0; x < 768; x++)
	{
		out[x] = backdrop;
		if (!(m_channel[0].transparency_control & TCR_DISABLE_MX))
		{
			uint8_t abr = std::clamp(((std::clamp((int32_t)plane_a_r[x] - 16, 0, 255) * m_channel[0].weight_factor[x]) >> 6) + ((std::clamp((int32_t)plane_b_r[x] - 16, 0, 255) * m_channel[1].weight_factor[x]) >> 6) + 16, 0, 255);
			uint8_t abg = std::clamp(((std::clamp((int32_t)plane_a_g[x] - 16, 0, 255) * m_channel[0].weight_factor[x]) >> 6) + ((std::clamp((int32_t)plane_b_g[x] - 16, 0, 255) * m_channel[1].weight_factor[x]) >> 6) + 16, 0, 255);
			uint8_t abb = std::clamp(((std::clamp((int32_t)plane_a_b[x] - 16, 0, 255) * m_channel[0].weight_factor[x]) >> 6) + ((std::clamp((int32_t)plane_b_b[x] - 16, 0, 255) * m_channel[1].weight_factor[x]) >> 6) + 16, 0, 255);
			out[x] = (abr << 16) | (abg << 8) | abb;
		}
		else
		{
			uint8_t plane_enable_a = 0;
			uint8_t plane_enable_b = 0;
			uint8_t plane_a_r_cur = mosaic_enable_a ? plane_a_r[x - (x % mosaic_count_a)] : plane_a_r[x];
			uint8_t plane_a_g_cur = mosaic_enable_a ? plane_a_g[x - (x % mosaic_count_a)] : plane_a_g[x];
			uint8_t plane_a_b_cur = mosaic_enable_a ? plane_a_b[x - (x % mosaic_count_a)] : plane_a_b[x];
			uint8_t plane_b_r_cur = mosaic_enable_b ? plane_b_r[x - (x % mosaic_count_b)] : plane_b_r[x];
			uint8_t plane_b_g_cur = mosaic_enable_b ? plane_b_g[x - (x % mosaic_count_b)] : plane_b_g[x];
			uint8_t plane_b_b_cur = mosaic_enable_b ? plane_b_b[x - (x % mosaic_count_b)] : plane_b_b[x];
			switch (transparency_mode_a)
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
					LOGMASKED(LOG_UNKNOWNS, "Unhandled transparency mode for plane A: %d\n", transparency_mode_a);
					plane_enable_a = 1;
					break;
			}
			switch (transparency_mode_b)
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
					LOGMASKED(LOG_UNKNOWNS, "Unhandled transparency mode for plane B: %d\n", transparency_mode_b);
					plane_enable_b = 1;
					break;
			}
			plane_a_r_cur = std::clamp(((std::clamp((int32_t)plane_a_r_cur - 16, 0, 255) * m_channel[0].weight_factor[x]) >> 6) + 16, 0, 255);
			plane_a_g_cur = std::clamp(((std::clamp((int32_t)plane_a_g_cur - 16, 0, 255) * m_channel[0].weight_factor[x]) >> 6) + 16, 0, 255);
			plane_a_b_cur = std::clamp(((std::clamp((int32_t)plane_a_b_cur - 16, 0, 255) * m_channel[0].weight_factor[x]) >> 6) + 16, 0, 255);
			plane_b_r_cur = std::clamp(((std::clamp((int32_t)plane_b_r_cur - 16, 0, 255) * m_channel[1].weight_factor[x]) >> 6) + 16, 0, 255);
			plane_b_g_cur = std::clamp(((std::clamp((int32_t)plane_b_g_cur - 16, 0, 255) * m_channel[1].weight_factor[x]) >> 6) + 16, 0, 255);
			plane_b_b_cur = std::clamp(((std::clamp((int32_t)plane_b_b_cur - 16, 0, 255) * m_channel[1].weight_factor[x]) >> 6) + 16, 0, 255);
			switch (m_channel[0].plane_order)
			{
				case POR_AB:
					if (plane_enable_a)
					{
						out[x] = (plane_a_r_cur << 16) | (plane_a_g_cur << 8) | plane_a_b_cur;
					}
					else if (plane_enable_b)
					{
						out[x] = (plane_b_r_cur << 16) | (plane_b_g_cur << 8) | plane_b_b_cur;
					}
					break;
				case POR_BA:
					if (plane_enable_b)
					{
						out[x] = (plane_b_r_cur << 16) | (plane_b_g_cur << 8) | plane_b_b_cur;
					}
					else if (plane_enable_a)
					{
						out[x] = (plane_a_r_cur << 16) | (plane_a_g_cur << 8) | plane_a_b_cur;
					}
					break;
			}
		}
	}
}

void mcd212_device::draw_cursor(uint32_t *scanline)
{
	if (m_channel[0].cursor_control & CURCNT_EN)
	{
		uint16_t y = (uint16_t)screen().vpos();
		const uint16_t cursor_x =  m_channel[0].cursor_position        & 0x3ff;
		const uint16_t cursor_y = ((m_channel[0].cursor_position >> 12) & 0x3ff) + m_ica_height;
		if (y >= cursor_y && y < (cursor_y + 16))
		{
			uint32_t color = s_4bpp_color[m_channel[0].cursor_control & CURCNT_COLOR];
			y -= cursor_y;
			if (m_channel[0].cursor_control & CURCNT_CUW)
			{
				for (int x = cursor_x; x < cursor_x + 64 && x < 768; x++)
				{
					if (m_channel[0].cursor_pattern[y] & (1 << (15 - ((x - cursor_x) >> 2))))
					{
						scanline[(x++)/2] = color;
						scanline[(x++)/2] = color;
						scanline[(x++)/2] = color;
						scanline[(x/2)] = color;
					}
				}
			}
			else
			{
				for (int x = cursor_x; x < cursor_x + 32 && x < 768; x++)
				{
					if (m_channel[0].cursor_pattern[y] & (1 << (15 - ((x - cursor_x) >> 1))))
					{
						scanline[(x++)/2] = color;
						scanline[x/2] = color;
					}
				}
			}
		}
	}
}

void mcd212_device::draw_scanline(bitmap_rgb32 &bitmap)
{
	uint8_t plane_a_r[768], plane_a_g[768], plane_a_b[768];
	uint8_t plane_b_r[768], plane_b_g[768], plane_b_b[768];
	uint32_t out[768];

	process_vsr<0>(plane_a_r, plane_a_g, plane_a_b);
	process_vsr<1>(plane_b_r, plane_b_g, plane_b_b);

	mix_lines(plane_a_r, plane_a_g, plane_a_b, plane_b_r, plane_b_g, plane_b_b, out);

	uint32_t *const scanline = &bitmap.pix(screen().vpos());
	for (int x = 0; x < 384; x++)
	{
		scanline[x] = out[x*2];
	}

	draw_cursor(scanline);
}

void mcd212_device::map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(mcd212_device::csr2_w));
	map(0x01, 0x01).r(FUNC(mcd212_device::csr2_r));
	map(0x02, 0x03).rw(FUNC(mcd212_device::dcr2_r), FUNC(mcd212_device::dcr2_w));
	map(0x04, 0x05).rw(FUNC(mcd212_device::vsr2_r), FUNC(mcd212_device::vsr2_w));
	map(0x08, 0x09).rw(FUNC(mcd212_device::ddr2_r), FUNC(mcd212_device::ddr2_w));
	map(0x0a, 0x0b).rw(FUNC(mcd212_device::dca2_r), FUNC(mcd212_device::dca2_w));

	map(0x10, 0x11).w(FUNC(mcd212_device::csr1_w));
	map(0x11, 0x11).r(FUNC(mcd212_device::csr1_r));
	map(0x12, 0x13).rw(FUNC(mcd212_device::dcr1_r), FUNC(mcd212_device::dcr1_w));
	map(0x14, 0x15).rw(FUNC(mcd212_device::vsr1_r), FUNC(mcd212_device::vsr1_w));
	map(0x18, 0x19).rw(FUNC(mcd212_device::ddr1_r), FUNC(mcd212_device::ddr1_w));
	map(0x1a, 0x1b).rw(FUNC(mcd212_device::dca1_r), FUNC(mcd212_device::dca1_w));
}

uint8_t mcd212_device::csr1_r()
{
	LOGMASKED(LOG_STATUS, "%s: Control/Status Register 1 Read: %02x\n", machine().describe_context(), m_channel[0].csrr);
	return m_channel[0].csrr;
}

void mcd212_device::csr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Control/Status Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[0].csrw);
	update_visible_area();
}

uint16_t mcd212_device::dcr1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Command Register 1 Read: %04x & %08x\n", machine().describe_context(), m_channel[0].dcr, mem_mask);
	return m_channel[0].dcr;
}

void mcd212_device::dcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Command Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[0].dcr);
	update_visible_area();
}

uint16_t mcd212_device::vsr1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Video Start Register 1 Read: %04x & %08x\n", machine().describe_context(), m_channel[0].vsr, mem_mask);
	return m_channel[0].vsr;
}

void mcd212_device::vsr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Video Start Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[0].vsr);
}

uint16_t mcd212_device::ddr1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Decoder Register 1 Read: %04x & %08x\n", machine().describe_context(), m_channel[0].ddr, mem_mask);
	return m_channel[0].ddr;
}

void mcd212_device::ddr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Decoder Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[0].ddr);
}

uint16_t mcd212_device::dca1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: DCA Pointer 1 Read: %04x & %08x\n", machine().describe_context(), m_channel[0].dca, mem_mask);
	return m_channel[0].dca;
}

void mcd212_device::dca1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: DCA Pointer 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[0].dca);
}

uint8_t mcd212_device::csr2_r()
{
	if (machine().side_effects_disabled())
	{
		return m_channel[1].csrr;
	}

	const uint8_t data = m_channel[1].csrr;
	LOGMASKED(LOG_STATUS, "%s: Status Register 2: %02x\n", machine().describe_context(), data);

	m_channel[1].csrr &= ~(CSR2R_IT1 | CSR2R_IT2);
	if (data & (CSR2R_IT1 | CSR2R_IT2))
		m_int_callback(CLEAR_LINE);

	return data;
}

void mcd212_device::csr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Control/Status Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[1].csrw);
}

uint16_t mcd212_device::dcr2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Command Register 2 Read: %04x & %08x\n", machine().describe_context(), m_channel[1].dcr, mem_mask);
	return m_channel[1].dcr;
}

void mcd212_device::dcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Command Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[1].dcr);
}

uint16_t mcd212_device::vsr2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Video Start Register 2 Read: %04x & %08x\n", machine().describe_context(), m_channel[1].vsr, mem_mask);
	return m_channel[1].vsr;
}

void mcd212_device::vsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Video Start Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[1].vsr);
}

uint16_t mcd212_device::ddr2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Decoder Register 2 Read: %04x & %08x\n", machine().describe_context(), m_channel[1].ddr, mem_mask);
	return m_channel[1].ddr;
}

void mcd212_device::ddr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: Display Decoder Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[1].ddr);
}

uint16_t mcd212_device::dca2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: DCA Pointer 2 Read: %04x & %08x\n", machine().describe_context(), m_channel[1].dca, mem_mask);
	return m_channel[1].dca;
}

void mcd212_device::dca2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_STATUS, "%s: DCA Pointer 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_channel[1].dca);
}

WRITE_LINE_MEMBER(mcd212_device::screen_vblank)
{
	if (state)
	{
		// Process ICA
		m_channel[0].csrr &= 0x7f;
		if (m_channel[0].dcr & DCR_ICA)
			process_ica<0>();
		if (m_channel[1].dcr & DCR_ICA)
			process_ica<1>();
	}
}

uint32_t mcd212_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scanline = screen.vpos();

	if (scanline >= m_ica_height)
	{
		m_channel[0].csrr |= 0x80;

		// Process DCA
		if (m_channel[0].dcr & DCR_DCA)
			process_dca<0>();
		if (m_channel[1].dcr & DCR_DCA)
			process_dca<1>();

		// Process VSR
		draw_scanline(bitmap);
	}

	if (scanline == (m_total_height - 1))
	{
		m_channel[0].csrr ^= 0x20;
	}

	return 0;
}

void mcd212_device::device_reset()
{
	for (auto & elem : m_channel)
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
		elem.transparent_color = 0;
		elem.mask_color = 0;
		elem.dyuv_abs_start = 0;
		elem.cursor_position = 0;
		elem.cursor_control = 0;
		memset((uint8_t*)&elem.cursor_pattern, 0, 16 * sizeof(uint32_t));
		memset((uint8_t*)&elem.region_control, 0, 8 * sizeof(uint32_t));
		elem.backdrop_color = 0;
		elem.mosaic_hold = 0;
		memset(elem.weight_factor, 0, 768);
	}
	memset(m_region_flag_0, 0, 768);
	memset(m_region_flag_1, 0, 768);
	m_ica_height = 32;
	m_total_height = 312;

	m_int_callback(CLEAR_LINE);
}

//-------------------------------------------------
//  mcd212_device - constructor
//-------------------------------------------------

mcd212_device::mcd212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MCD212, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_int_callback(*this)
	, m_planea(*this, "planea")
	, m_planeb(*this, "planeb")
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mcd212_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcd212_device::device_start()
{
	ab_init();

	save_item(NAME(m_region_flag_0));
	save_item(NAME(m_region_flag_1));
	save_item(NAME(m_ica_height));
	save_item(NAME(m_total_height));
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
	save_item(NAME(m_channel[0].transparent_color));
	save_item(NAME(m_channel[0].mask_color));
	save_item(NAME(m_channel[0].dyuv_abs_start));
	save_item(NAME(m_channel[0].cursor_position));
	save_item(NAME(m_channel[0].cursor_control));
	save_item(NAME(m_channel[0].cursor_pattern));
	save_item(NAME(m_channel[0].region_control));
	save_item(NAME(m_channel[0].backdrop_color));
	save_item(NAME(m_channel[0].mosaic_hold));
	save_item(NAME(m_channel[0].weight_factor));
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
	save_item(NAME(m_channel[1].transparent_color));
	save_item(NAME(m_channel[1].mask_color));
	save_item(NAME(m_channel[1].dyuv_abs_start));
	save_item(NAME(m_channel[1].cursor_position));
	save_item(NAME(m_channel[1].cursor_control));
	save_item(NAME(m_channel[1].cursor_pattern));
	save_item(NAME(m_channel[1].region_control));
	save_item(NAME(m_channel[1].backdrop_color));
	save_item(NAME(m_channel[1].mosaic_hold));
	save_item(NAME(m_channel[1].weight_factor));
}

void mcd212_device::ab_init()
{
	// Delta decoding array.
	static const uint8_t abDelta[16] = { 0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255 };

	// Initialize delta decoding arrays for each unsigned byte value b.
	for (uint16_t d = 0; d < 0x100; d++)
	{
		m_ab.deltaY[d] = abDelta[d & 15];
	}

	// Initialize delta decoding arrays for each unsigned byte value b.
	for (uint16_t d = 0; d < 0x100; d++)
	{
		m_ab.deltaUV[d] = abDelta[d >> 4];
	}

	// Initialize color limit and clamp arrays.
	for (uint16_t w = 0; w < 3 * 0xff; w++)
	{
		m_ab.limit[w] = (w < 0xff + 16) ?  0 : w <= 16 + 2 * 0xff ? w - 0xef : 0xff;
		m_ab.clamp[w] = (w < 0xff + 32) ? 16 : w <= 16 + 2 * 0xff ? w - 0xef : 0xff;
	}

	for (int16_t sw = 0; sw < 0x100; sw++)
	{
		m_ab.matrixUB[sw] = (444 * (sw - 128)) / 256;
		m_ab.matrixUG[sw] = - (86 * (sw - 128)) / 256;
		m_ab.matrixVG[sw] = - (179 * (sw - 128)) / 256;
		m_ab.matrixVR[sw] = (351 * (sw - 128)) / 256;
	}
}
