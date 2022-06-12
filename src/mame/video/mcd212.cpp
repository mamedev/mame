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

#define LOG_UNKNOWNS        (1U << 1)
#define LOG_REGISTERS       (1U << 2)
#define LOG_ICA             (1U << 3)
#define LOG_DCA             (1U << 4)
#define LOG_VSR             (1U << 5)
#define LOG_STATUS          (1U << 6)
#define LOG_MAIN_REG_READS  (1U << 7)
#define LOG_MAIN_REG_WRITES (1U << 8)
#define LOG_CLUT            (1U << 9)
#define LOG_ALL             (LOG_UNKNOWNS | LOG_REGISTERS | LOG_ICA | LOG_DCA | LOG_VSR | LOG_STATUS | LOG_MAIN_REG_READS | LOG_MAIN_REG_WRITES | LOG_CLUT)

#define VERBOSE             (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(MCD212, mcd212_device, "mcd212", "MCD212 VDSC")

inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_weight_factor(const uint32_t region_idx)
{
	return (uint8_t)((m_region_control[region_idx] & RC_WF) >> RC_WF_SHIFT);
}

inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_region_op(const uint32_t region_idx)
{
	return (m_region_control[region_idx] & RC_OP) >> RC_OP_SHIFT;
}

void mcd212_device::update_region_arrays()
{
	bool latched_rf[2] { false, false };
	uint8_t latched_wfa = m_weight_factor[0][0];
	uint8_t latched_wfb = m_weight_factor[1][0];
	const int width = get_screen_width();

	if (BIT(m_image_coding_method, ICM_NR_BIT))
	{
		if (get_region_op(0) == 0 && get_region_op(4) == 0)
		{
			std::fill_n(m_weight_factor[0], std::size(m_weight_factor[0]), latched_wfa);
			std::fill_n(m_weight_factor[1], std::size(m_weight_factor[1]), latched_wfb);
			std::fill_n(m_region_flag[0], std::size(m_region_flag[0]), false);
			std::fill_n(m_region_flag[1], std::size(m_region_flag[1]), false);
			return;
		}

		for (int x = 0; x < width; x++)
		{
			for (int flag = 0; flag < 2; flag++)
			{
				for (int region = 0; region < 4; region++)
				{
					const int region_idx = (flag << 2) + region;
					const uint32_t region_ctrl = m_region_control[region_idx];
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
								latched_rf[flag] = false;
								break;
							case 9: // Set region flag
								latched_rf[flag] = true;
								break;
							case 10:    // Not used
							case 11:    // Not used
								break;
							case 12: // Reset region flag and change weight of plane A
								latched_wfa = get_weight_factor(region_idx);
								latched_rf[flag] = false;
								break;
							case 13: // Set region flag and change weight of plane A
								latched_wfa = get_weight_factor(region_idx);
								latched_rf[flag] = true;
								break;
							case 14: // Reset region flag and change weight of plane B
								latched_wfb = get_weight_factor(region_idx);
								latched_rf[flag] = false;
								break;
							case 15: // Set region flag and change weight of plane B
								latched_wfb = get_weight_factor(region_idx);
								latched_rf[flag] = true;
								break;
						}
					}
				}
			}
			m_weight_factor[0][x] = latched_wfa;
			m_weight_factor[1][x] = latched_wfb;
			m_region_flag[0][x] = latched_rf[0];
			m_region_flag[1][x] = latched_rf[1];
		}
	}
	else
	{
		int region_idx = 0;
		for (int x = 0; x < width; x++)
		{
			if (region_idx < 8)
			{
				const int flag = BIT(m_region_control[region_idx], RC_RF_BIT);
				const uint32_t region_ctrl = m_region_control[region_idx];
				const uint32_t region_op = get_region_op(region_idx);
				if (region_op == 0)
				{
					std::fill_n(m_weight_factor[0] + x, std::size(m_weight_factor[0]) - x, latched_wfa);
					std::fill_n(m_weight_factor[1] + x, std::size(m_weight_factor[1]) - x, latched_wfb);
					std::fill_n(m_region_flag[0] + x, std::size(m_region_flag[0]) - x, latched_rf[0]);
					std::fill_n(m_region_flag[1] + x, std::size(m_region_flag[1]) - x, latched_rf[1]);
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
							latched_rf[flag] = false;
							break;
						case 9: // Set region flag
							latched_rf[flag] = true;
							break;
						case 10:    // Not used
						case 11:    // Not used
							break;
						case 12: // Reset region flag and change weight of plane A
							latched_wfa = get_weight_factor(region_idx);
							latched_rf[flag] = false;
							break;
						case 13: // Set region flag and change weight of plane A
							latched_wfa = get_weight_factor(region_idx);
							latched_rf[flag] = true;
							break;
						case 14: // Reset region flag and change weight of plane B
							latched_wfb = get_weight_factor(region_idx);
							latched_rf[flag] = false;
							break;
						case 15: // Set region flag and change weight of plane B
							latched_wfb = get_weight_factor(region_idx);
							latched_rf[flag] = true;
							break;
					}
					region_idx++;
				}
			}
			m_weight_factor[0][x] = latched_wfa;
			m_weight_factor[1][x] = latched_wfb;
			m_region_flag[0][x] = latched_rf[0];
			m_region_flag[1][x] = latched_rf[1];
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
			{
				const uint8_t clut_index = m_clut_bank[Channel] * 0x40 + (reg - 0x80);
				LOGMASKED(LOG_CLUT, "%s: Channel %d: CLUT[%d] = %08x\n", machine().describe_context(), Channel, clut_index, value);
				m_clut[clut_index] = value & 0x00fcfcfc;
			}
			break;
		case 0xc0: // Image Coding Method
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Channel 0: Image Coding Method = %08x\n", machine().describe_context(), value);
				m_image_coding_method = value;
			}
			break;
		case 0xc1: // Transparency Control
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Transparency Control = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_transparency_control = value;
			}
			break;
		case 0xc2: // Plane Order
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Plane Order = %08x\n", machine().describe_context(), screen().vpos(), value & 7);
				m_plane_order = value & 0x00000007;
			}
			break;
		case 0xc3: // CLUT Bank Register
			LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel %d: CLUT Bank Register = %08x\n", machine().describe_context(), screen().vpos(), Channel, value & 3);
			m_clut_bank[Channel] = Channel ? (2 | (value & 0x00000001)) : (value & 0x00000003);
			break;
		case 0xc4: // Transparent Color A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Transparent Color A = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_transparent_color[0] = value & 0x00fcfcfc;
			}
			break;
		case 0xc6: // Transparent Color B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Transparent Color B = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_transparent_color[1] = value & 0x00fcfcfc;
			}
			break;
		case 0xc7: // Mask Color A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Mask Color A = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_mask_color[0] = value & 0x00fcfcfc;
			}
			break;
		case 0xc9: // Mask Color B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Mask Color B = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_mask_color[1] = value & 0x00fcfcfc;
			}
			break;
		case 0xca: // Delta YUV Absolute Start Value A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Delta YUV Absolute Start Value A = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_dyuv_abs_start[0] = value;
			}
			break;
		case 0xcb: // Delta YUV Absolute Start Value B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Delta YUV Absolute Start Value B = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_dyuv_abs_start[1] = value;
			}
			break;
		case 0xcd: // Cursor Position
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Cursor Position = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_cursor_position = value;
			}
			break;
		case 0xce: // Cursor Control
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Cursor Control = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_cursor_control = value;
			}
			break;
		case 0xcf: // Cursor Pattern
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Cursor Pattern[%d] = %04x\n", machine().describe_context(), screen().vpos(), (value >> 16) & 0x000f, value & 0x0000ffff);
				m_cursor_pattern[(value >> 16) & 0x000f] = value & 0x0000ffff;
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
			LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel %d: Region Control %d = %08x\n", machine().describe_context(), screen().vpos(), Channel, reg & 7, value);
			m_region_control[reg & 7] = value;
			update_region_arrays();
			break;
		case 0xd8: // Backdrop Color
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Backdrop Color = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_backdrop_color = value;
			}
			break;
		case 0xd9: // Mosaic Pixel Hold Factor A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Mosaic Pixel Hold Factor A = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_mosaic_hold[0] = value;
			}
			break;
		case 0xda: // Mosaic Pixel Hold Factor B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Mosaic Pixel Hold Factor B = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_mosaic_hold[1] = value;
			}
			break;
		case 0xdb: // Weight Factor A
			if (Channel == 0)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 0: Weight Factor A = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_weight_factor[0][0] = (uint8_t)value;
				update_region_arrays();
			}
			break;
		case 0xdc: // Weight Factor B
			if (Channel == 1)
			{
				LOGMASKED(LOG_REGISTERS, "%s: Scanline %d, Channel 1: Weight Factor B = %08x\n", machine().describe_context(), screen().vpos(), value);
				m_weight_factor[1][0] = (uint8_t)value;
				update_region_arrays();
			}
			break;
	}
}

template <int Channel>
inline ATTR_FORCE_INLINE uint32_t mcd212_device::get_vsr()
{
	return ((m_dcr[Channel] & 0x3f) << 16) | m_vsr[Channel];
}

template <int Channel>
inline ATTR_FORCE_INLINE void mcd212_device::set_vsr(uint32_t value)
{
	m_vsr[Channel] = value & 0x0000ffff;
	m_dcr[Channel] &= 0xffc0;
	m_dcr[Channel] |= (value >> 16) & 0x003f;
}

template <int Channel>
inline ATTR_FORCE_INLINE void mcd212_device::set_dcp(uint32_t value)
{
	m_dcp[Channel] = value & 0x0000ffff;
	m_ddr[Channel] &= 0xffc0;
	m_ddr[Channel] |= (value >> 16) & 0x003f;
}

template <int Channel>
inline ATTR_FORCE_INLINE uint32_t mcd212_device::get_dcp()
{
	return ((m_ddr[Channel] & 0x3f) << 16) | m_dcp[Channel];
}

template <int Channel>
inline ATTR_FORCE_INLINE void mcd212_device::set_display_parameters(uint8_t value)
{
	m_ddr[Channel] &= 0xf0ff;
	m_ddr[Channel] |= (value & 0x0f) << 8;
	m_dcr[Channel] &= 0xf7ff;
	m_dcr[Channel] |= (value & 0x10) << 7;
}

int mcd212_device::get_screen_width()
{
	int width = 768;
	if (!BIT(m_dcr[0], DCR_CF_BIT) || BIT(m_csrw[0], CSR1W_ST_BIT))
		width = 720;
	return width;
}

int mcd212_device::get_border_width()
{
	int width = 0;
	if (!BIT(m_dcr[0], DCR_CF_BIT) || BIT(m_csrw[0], CSR1W_ST_BIT))
		width = 24;
	return width;
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
				m_csrr[1] |= 1 << (2 - Channel);
				if (m_csrr[1] & (CSR2R_IT1 | CSR2R_IT2))
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
	uint16_t *dca = Channel ? m_planeb.target() : m_planea.target();
	uint32_t addr = (m_dca[Channel] & 0x0007ffff) / 2;
	uint32_t cmd = 0;
	uint32_t count = 0;
	uint32_t max = 64;
	bool addr_changed = false;
	bool processing = true;

	LOGMASKED(LOG_DCA, "Scanline %d: Processing DCA %d\n", screen().vpos(), Channel );

	while (processing && count < max)
	{
		cmd = dca[addr++] << 16;
		cmd |= dca[addr++];
		count += 4;
		switch ((cmd & 0xff000000) >> 24)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // STOP
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: STOP\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				processing = false;
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
				m_dca[Channel] = cmd & 0x0007fffc;
				return;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD VSR
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD VSR: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_vsr<Channel>(cmd & 0x003fffff);
				break;
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: RELOAD VSR and STOP: %06x\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel, cmd & 0x001fffff );
				set_vsr<Channel>(cmd & 0x003fffff);
				processing = false;
				break;
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				LOGMASKED(LOG_DCA, "%08x: %08x: DCA %d: INTERRUPT\n", (addr - 2) * 2 + Channel * 0x200000, cmd, Channel );
				m_csrr[1] |= 1 << (2 - Channel);
				if (m_csrr[1] & (CSR2R_IT1 | CSR2R_IT2))
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
	}

	if (!addr_changed)
	{
		addr += (max - count) >> 1;
	}

	m_dca[Channel] = addr * 2;
}

template <int Channel>
static inline uint8_t BYTE_TO_CLUT(int icm, uint8_t byte)
{
	switch (icm)
	{
		case 1:
			return byte;
		case 3:
			if (Channel == 1)
			{
				return 0x80 + (byte & 0x7f);
			}
			else
			{
				return byte & 0x7f;
			}
		case 4:
			if (Channel == 0)
			{
				return byte & 0x7f;
			}
			break;
		case 11:
			if (Channel == 1)
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
inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_transparency_control()
{
	return (m_transparency_control >> (Channel ? 8 : 0)) & 0x0f;
}

template <int Channel>
inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_icm()
{
	const uint32_t mask = Channel ? ICM_MODE2 : ICM_MODE1;
	const uint32_t shift = Channel ? ICM_MODE2_SHIFT : ICM_MODE1_SHIFT;
	return (m_image_coding_method & mask) >> shift;
}

template <int Channel>
inline ATTR_FORCE_INLINE bool mcd212_device::get_mosaic_enable()
{
	return (m_ddr[Channel] & DDR_FT) == DDR_FT_MOSAIC;
}

template <int Channel>
inline ATTR_FORCE_INLINE uint8_t mcd212_device::get_mosaic_factor()
{
	return 1 << (((m_ddr[Channel] & DDR_MT) >> DDR_MT_SHIFT) + 1);
}

template <int Channel>
int mcd212_device::get_plane_width()
{
	const int width = get_screen_width();
	const uint8_t icm = get_icm<Channel>();
	if (icm == ICM_CLUT4)
		return width;
	return width >> 1;
}

template <int Channel>
void mcd212_device::process_vsr(uint32_t *pixels, bool *transparent)
{
	const uint8_t *data = reinterpret_cast<uint8_t *>(Channel ? m_planeb.target() : m_planea.target());
	const uint8_t icm = get_icm<Channel>();
	const uint8_t transp_ctrl = get_transparency_control<Channel>();
	const int width = get_plane_width<Channel>();

	uint32_t vsr = get_vsr<Channel>();

	if (transp_ctrl == TCR_COND_1)
	{
		std::fill_n(pixels, get_screen_width(), 0x00101010);
		std::fill_n(transparent, get_screen_width(), true);
		return;
	}

	if (!icm || !vsr)
	{
		std::fill_n(pixels, get_screen_width(), 0x00101010);
		return;
	}

	const uint8_t mosaic_enable = get_mosaic_enable<Channel>();
	const uint8_t mosaic_factor = get_mosaic_factor<Channel>();

	const uint32_t dyuv_abs_start = m_dyuv_abs_start[Channel];
	const uint8_t start_y = (dyuv_abs_start >> 16) & 0x000000ff;
	const uint8_t start_u = (dyuv_abs_start >>  8) & 0x000000ff;
	const uint8_t start_v = (dyuv_abs_start >>  0) & 0x000000ff;

	const uint32_t transparent_color = m_transparent_color[Channel];
	const uint8_t transp_ctrl_masked = transp_ctrl & 0x07;
	const bool transp_always = (transp_ctrl_masked == TCR_COND_1);
	const bool invert_transp_condition = BIT(transp_ctrl, 3);
	const int region_flag_index = 1 - (transp_ctrl_masked & 1);
	const bool *region_flags = m_region_flag[region_flag_index];
	const bool use_region_flag = (transp_ctrl_masked >= TCR_COND_RF0_1 && transp_ctrl_masked <= TCR_COND_RF1KEY_1);
	bool use_color_key = (transp_ctrl_masked == TCR_COND_KEY_1 || transp_ctrl_masked == TCR_COND_RF0KEY_1 || transp_ctrl_masked == TCR_COND_RF1KEY_1);

	bool done = false;
	int x = 0;

	LOGMASKED(LOG_VSR, "Scanline %d: VSR Channel %d, ICM (%02x), VSR (%08x)\n", screen().vpos(), Channel, icm, vsr);

	while (!done)
	{
		uint8_t byte = data[(vsr & 0x0007ffff) ^ 1];
		LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: VSR[%05x] = %02x\n", screen().vpos(), Channel, (vsr & 0x0007ffff), byte);
		vsr++;
		switch (m_ddr[Channel] & DDR_FT)
		{
			case DDR_FT_BMP:
			case DDR_FT_BMP2:
			case DDR_FT_MOSAIC:
				if ((m_ddr[Channel] & DDR_FT) == DDR_FT_BMP)
				{
					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: BMP\n", screen().vpos(), Channel);
				}
				else if ((m_ddr[Channel] & DDR_FT) == DDR_FT_BMP2)
				{
					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: BMP2\n", screen().vpos(), Channel);
				}
				else if ((m_ddr[Channel] & DDR_FT) == DDR_FT_MOSAIC)
				{
					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: MOSAIC\n", screen().vpos(), Channel);
				}

				if (icm == ICM_DYUV)
				{
					use_color_key = false;

					LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: DYUV\n", screen().vpos(), Channel);
					uint8_t y = start_y;
					uint8_t u = start_u;
					uint8_t v = start_v;
					for (; x < width; x++)
					{
						const uint8_t byte1 = data[(vsr++ & 0x0007ffff) ^ 1];
						const uint8_t u1 = u + m_delta_uv_lut[byte];
						const uint8_t y0 = y + m_delta_y_lut[byte];

						const uint8_t v1 = v + m_delta_uv_lut[byte1];
						const uint8_t y1 = y0 + m_delta_y_lut[byte1];

						const uint8_t u0 = (u + u1) >> 1;
						const uint8_t v0 = (v + v1) >> 1;

						uint32_t *limit_r = m_dyuv_limit_r_lut + y0 + 0xff;
						uint32_t *limit_g = m_dyuv_limit_g_lut + y0 + 0xff;
						uint32_t *limit_b = m_dyuv_limit_b_lut + y0 + 0xff;

						uint32_t entry = limit_r[m_dyuv_v_to_r[v0]] | limit_g[m_dyuv_u_to_g[u0] + m_dyuv_v_to_g[v0]] | limit_b[m_dyuv_u_to_b[u0]];
						pixels[x] = entry;
						transparent[x] = (transp_always || (use_region_flag && region_flags[x << 1])) != invert_transp_condition;

						if (mosaic_enable)
						{
							for (int mosaic_index = 1; mosaic_index < mosaic_factor && (x + mosaic_index) < width; mosaic_index++)
							{
								pixels[x + mosaic_index] = pixels[x];
								transparent[x + mosaic_index] = transparent[x << 1];
							}
							x += mosaic_factor;
						}
						else
						{
							x++;
						}

						limit_r = m_dyuv_limit_r_lut + y1 + 0xff;
						limit_g = m_dyuv_limit_g_lut + y1 + 0xff;
						limit_b = m_dyuv_limit_b_lut + y1 + 0xff;

						entry = limit_r[m_dyuv_v_to_r[v1]] | limit_g[m_dyuv_u_to_g[u1] + m_dyuv_v_to_g[v1]] | limit_b[m_dyuv_u_to_b[u1]];
						pixels[x] = entry;
						transparent[x] = (transp_always || (use_region_flag && region_flags[x << 1])) != invert_transp_condition;

						if (mosaic_enable)
						{
							for (int mosaic_index = 1; mosaic_index < mosaic_factor && (x + mosaic_index) < width; mosaic_index++)
							{
								pixels[x + mosaic_index] = pixels[x];
								transparent[x + mosaic_index] = transparent[x];
							}
							x += mosaic_factor - 1;
						}

						byte = data[(vsr++ & 0x0007ffff) ^ 1];

						y = y1;
						u = u1;
						v = v1;
					}
					set_vsr<Channel>(vsr - 1);
				}
				else if (icm == ICM_CLUT8 || icm == ICM_CLUT7 || icm == ICM_CLUT77)
				{
					for (; x < width; x++)
					{
						uint32_t entry = m_clut[BYTE_TO_CLUT<Channel>(icm, byte)];
						pixels[x] = entry;
						transparent[x] = (transp_always || (use_color_key && (entry == transparent_color)) || (use_region_flag && region_flags[x << 1])) != invert_transp_condition;
						if (mosaic_enable)
						{
							for (int mosaic_index = 1; mosaic_index < mosaic_factor && (x + mosaic_index) < width; mosaic_index++)
							{
								pixels[x + mosaic_index] = pixels[x];
								transparent[x + mosaic_index] = transparent[x];
							}
							x += mosaic_factor - 1;
						}
						byte = data[(vsr & 0x0007ffff) ^ 1];
						vsr++;
					}
					set_vsr<Channel>(vsr - 1);
				}
				else if (icm == ICM_CLUT4)
				{
					for (; x < width - 1; x += 2)
					{
						const uint32_t even_entry = m_clut[BYTE_TO_CLUT<Channel>(icm, byte >> 4)];
						const uint32_t odd_entry = m_clut[BYTE_TO_CLUT<Channel>(icm, byte)];
						const bool even_pre_transparent = transp_always || (use_color_key && (even_entry == transparent_color));
						const bool odd_pre_transparent = transp_always || (use_color_key && (odd_entry == transparent_color));
						if (mosaic_enable)
						{
							for (int mosaic_index = 0; mosaic_index < mosaic_factor && (x + mosaic_index) < (width - 1); mosaic_index += 2)
							{
								pixels[x + mosaic_index] = even_entry;
								transparent[x + mosaic_index] = (even_pre_transparent || (use_region_flag && region_flags[x + mosaic_index])) != invert_transp_condition;
								pixels[x + mosaic_index + 1] = odd_entry;
								transparent[x + mosaic_index + 1] = (odd_pre_transparent || (use_region_flag && region_flags[x + mosaic_index + 1])) != invert_transp_condition;
							}
							x += mosaic_factor - 2;
						}
						else
						{
							pixels[x] = even_entry;
							transparent[x] = (even_pre_transparent || (use_region_flag && region_flags[x])) != invert_transp_condition;

							pixels[x + 1] = odd_entry;
							transparent[x + 1] = (odd_pre_transparent || (use_region_flag && region_flags[x + 1])) != invert_transp_condition;
						}
						byte = data[(vsr & 0x0007ffff) ^ 1];
						vsr++;
					}
					set_vsr<Channel>(vsr - 1);
				}
				else
				{
					std::fill_n(pixels + x, width - x, 0x00101010);
					std::fill_n(transparent + x, width - x, true);
				}
				done = true;
				break;
			case DDR_FT_RLE:
				LOGMASKED(LOG_VSR, "Scanline %d: Chan %d: RLE\n", screen().vpos(), Channel);
				if (byte & 0x80)
				{
					// Run length
					uint8_t length = data[((vsr++) & 0x0007ffff) ^ 1];
					LOGMASKED(LOG_VSR, "Byte %02x w/ run length %02x at %d\n", byte, length, x);
					const uint32_t entry = m_clut[BYTE_TO_CLUT<Channel>(icm, byte & 0x7f)];
					const bool pre_transparent = (transp_always || (use_color_key && entry == transparent_color));
					if (!length)
					{
						// Go to the end of the line
						std::fill_n(pixels + x, width - x, entry);
						for (int transp_index = x; transp_index < width; transp_index++)
						{
							transparent[transp_index] = (pre_transparent || (use_region_flag && region_flags[transp_index << 1])) != invert_transp_condition;
						}
						done = true;
						set_vsr<Channel>(vsr);
					}
					else
					{
						int end = std::min(width, x + length);
						std::fill_n(pixels + x, end - x, entry);
						for (int transp_index = x; transp_index < end; transp_index++)
						{
							transparent[transp_index] = (pre_transparent || (use_region_flag && region_flags[transp_index << 1])) != invert_transp_condition;
						}
						x = end;
						if (x >= width)
						{
							done = true;
							set_vsr<Channel>(vsr);
						}
					}
				}
				else
				{
					LOGMASKED(LOG_VSR, "Byte %02x, single at %d\n", byte, x);
					// Single pixel
					const uint32_t entry = m_clut[BYTE_TO_CLUT<Channel>(icm, byte)];
					const bool pre_transparent = (transp_always || (use_color_key && entry == transparent_color));

					pixels[x] = entry;
					transparent[x] = (pre_transparent || (use_region_flag && region_flags[x << 1])) != invert_transp_condition;
					x++;

					if (x >= width)
					{
						done = true;
						set_vsr<Channel>(vsr);
					}
				}
				break;
		}
	}

	if (icm != ICM_CLUT4)
	{
		for (int i = width - 1; i >= 0; i--)
		{
			pixels[i * 2] = pixels[i * 2 + 1] = pixels[i];
			transparent[i * 2] = transparent[i * 2 + 1] = transparent[i];
		}
	}
}

const uint32_t mcd212_device::s_4bpp_color[16] =
{
	0xff101010, 0xff10107a, 0xff107a10, 0xff107a7a, 0xff7a1010, 0xff7a107a, 0xff7a7a10, 0xff7a7a7a,
	0xff101010, 0xff1010e6, 0xff10e610, 0xff10e6e6, 0xffe61010, 0xffe610e6, 0xffe6e610, 0xffe6e6e6
};

template <bool MosaicA, bool MosaicB, bool OrderAB>
void mcd212_device::mix_lines(uint32_t *plane_a, bool *transparent_a, uint32_t *plane_b, bool *transparent_b, uint32_t *out)
{
	const uint32_t backdrop = s_4bpp_color[m_backdrop_color];
	const uint8_t mosaic_count_a = (m_mosaic_hold[0] & 0x0000ff) << 1;
	const uint8_t mosaic_count_b = (m_mosaic_hold[1] & 0x0000ff) << 1;
	const int width = get_screen_width();
	const int border_width = get_border_width();

	uint8_t *weight_a = &m_weight_factor[0][0];
	uint8_t *weight_b = &m_weight_factor[1][0];

	if (!(m_transparency_control & TCR_DISABLE_MX))
	{
		for (int x = 0; x < width; x++, weight_a++, transparent_a++, weight_b++, transparent_b++)
		{
			const uint8_t weight_a_cur = *weight_a;
			const uint8_t weight_b_cur = *weight_b;

			const uint32_t plane_a_cur = plane_a[x];
			const uint32_t plane_b_cur = plane_b[x];

			const int32_t plane_a_r = (int32_t)(uint8_t)(plane_a_cur >> 16);
			const int32_t plane_b_r = (int32_t)(uint8_t)(plane_b_cur >> 16);
			const int32_t plane_a_g = (int32_t)(uint8_t)(plane_a_cur >> 8);
			const int32_t plane_b_g = (int32_t)(uint8_t)(plane_b_cur >> 8);
			const int32_t plane_a_b = (int32_t)(uint8_t)plane_a_cur;
			const int32_t plane_b_b = (int32_t)(uint8_t)plane_b_cur;
			const int32_t weighted_a_r = (plane_a_r > 16) ? (((plane_a_r - 16) * weight_a_cur) >> 6) : 0;
			const int32_t weighted_a_g = (plane_a_g > 16) ? (((plane_a_g - 16) * weight_a_cur) >> 6) : 0;
			const int32_t weighted_a_b = (plane_a_b > 16) ? (((plane_a_b - 16) * weight_a_cur) >> 6) : 0;
			const int32_t weighted_b_r = ((plane_b_r > 16) ? (((plane_b_r - 16) * weight_b_cur) >> 6) : 0) + weighted_a_r;
			const int32_t weighted_b_g = ((plane_b_g > 16) ? (((plane_b_g - 16) * weight_b_cur) >> 6) : 0) + weighted_a_g;
			const int32_t weighted_b_b = ((plane_b_b > 16) ? (((plane_b_b - 16) * weight_b_cur) >> 6) : 0) + weighted_a_b;
			const uint8_t out_r = (weighted_b_r > 255) ? 255 : (uint8_t)weighted_b_r;
			const uint8_t out_g = (weighted_b_g > 255) ? 255 : (uint8_t)weighted_b_g;
			const uint8_t out_b = (weighted_b_b > 255) ? 255 : (uint8_t)weighted_b_b;
			*out++ = 0xff000000 | (out_r << 16) | (out_g << 8) | out_b;
		}
	}
	else
	{
		for (int x = 0; x < width; x++, weight_a++, transparent_a++, weight_b++, transparent_b++)
		{
			if (OrderAB)
			{
				if (!(*transparent_a))
				{
					const uint32_t plane_a_cur = MosaicA ? plane_a[x - (x % mosaic_count_a)] : plane_a[x];
					const uint8_t weight_a_cur = *weight_a;
					const int32_t plane_a_r = (int32_t)(uint8_t)(plane_a_cur >> 16);
					const int32_t plane_a_g = (int32_t)(uint8_t)(plane_a_cur >> 8);
					const int32_t plane_a_b = (int32_t)(uint8_t)plane_a_cur;
					const uint8_t weighted_a_r = std::clamp(((plane_a_r > 16) ? (((plane_a_r - 16) * weight_a_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_a_g = std::clamp(((plane_a_g > 16) ? (((plane_a_g - 16) * weight_a_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_a_b = std::clamp(((plane_a_b > 16) ? (((plane_a_b - 16) * weight_a_cur) >> 6) : 0) + 16, 0, 255);
					*out++ = 0xff000000 | (weighted_a_r << 16) | (weighted_a_g << 8) | weighted_a_b;
				}
				else if (!(*transparent_b))
				{
					const uint32_t plane_b_cur = MosaicB ? plane_b[x - (x % mosaic_count_b)] : plane_b[x];
					const uint8_t weight_b_cur = *weight_b;
					const int32_t plane_b_r = (int32_t)(uint8_t)(plane_b_cur >> 16);
					const int32_t plane_b_g = (int32_t)(uint8_t)(plane_b_cur >> 8);
					const int32_t plane_b_b = (int32_t)(uint8_t)plane_b_cur;
					const uint8_t weighted_b_r = std::clamp(((plane_b_r > 16) ? (((plane_b_r - 16) * weight_b_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_b_g = std::clamp(((plane_b_g > 16) ? (((plane_b_g - 16) * weight_b_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_b_b = std::clamp(((plane_b_b > 16) ? (((plane_b_b - 16) * weight_b_cur) >> 6) : 0) + 16, 0, 255);
					*out++ = 0xff000000 | (weighted_b_r << 16) | (weighted_b_g << 8) | weighted_b_b;
				}
				else
				{
					*out++ = backdrop;
				}
			}
			else
			{
				if (!(*transparent_b))
				{
					const uint32_t plane_b_cur = MosaicB ? plane_b[x - (x % mosaic_count_b)] : plane_b[x];
					const uint8_t weight_b_cur = *weight_b;
					const int32_t plane_b_r = (int32_t)(uint8_t)(plane_b_cur >> 16);
					const int32_t plane_b_g = (int32_t)(uint8_t)(plane_b_cur >> 8);
					const int32_t plane_b_b = (int32_t)(uint8_t)plane_b_cur;
					const uint8_t weighted_b_r = std::clamp(((plane_b_r > 16) ? (((plane_b_r - 16) * weight_b_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_b_g = std::clamp(((plane_b_g > 16) ? (((plane_b_g - 16) * weight_b_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_b_b = std::clamp(((plane_b_b > 16) ? (((plane_b_b - 16) * weight_b_cur) >> 6) : 0) + 16, 0, 255);
					*out++ = 0xff000000 | (weighted_b_r << 16) | (weighted_b_g << 8) | weighted_b_b;
				}
				else if (!(*transparent_a))
				{
					const uint32_t plane_a_cur = MosaicA ? plane_a[x - (x % mosaic_count_a)] : plane_a[x];
					const uint8_t weight_a_cur = *weight_a;
					const int32_t plane_a_r = (int32_t)(uint8_t)(plane_a_cur >> 16);
					const int32_t plane_a_g = (int32_t)(uint8_t)(plane_a_cur >> 8);
					const int32_t plane_a_b = (int32_t)(uint8_t)plane_a_cur;
					const uint8_t weighted_a_r = std::clamp(((plane_a_r > 16) ? (((plane_a_r - 16) * weight_a_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_a_g = std::clamp(((plane_a_g > 16) ? (((plane_a_g - 16) * weight_a_cur) >> 6) : 0) + 16, 0, 255);
					const uint8_t weighted_a_b = std::clamp(((plane_a_b > 16) ? (((plane_a_b - 16) * weight_a_cur) >> 6) : 0) + 16, 0, 255);
					*out++ = 0xff000000 | (weighted_a_r << 16) | (weighted_a_g << 8) | weighted_a_b;
				}
				else
				{
					*out++ = backdrop;
				}
			}
		}
	}

	if (border_width)
	{
		std::fill_n(out, border_width, 0xff101010);
	}
}

void mcd212_device::draw_cursor(uint32_t *scanline)
{
	if (m_cursor_control & CURCNT_EN)
	{
		uint16_t y = (uint16_t)screen().vpos();
		const uint16_t cursor_x =  m_cursor_position & 0x3ff;
		const uint16_t cursor_y = ((m_cursor_position >> 12) & 0x3ff) + m_ica_height;
		if (y >= cursor_y && y < (cursor_y + 16))
		{
			const int width = get_screen_width();
			uint32_t color = s_4bpp_color[m_cursor_control & CURCNT_COLOR];
			y -= cursor_y;
			if (m_cursor_control & CURCNT_CUW)
			{
				for (int x = cursor_x; x < cursor_x + 64 && x < width; x++)
				{
					if (m_cursor_pattern[y] & (1 << (15 - ((x - cursor_x) >> 2))))
					{
						scanline[x++] = color;
						scanline[x++] = color;
						scanline[x++] = color;
						scanline[x] = color;
					}
				}
			}
			else
			{
				for (int x = cursor_x; x < cursor_x + 32 && x < width; x++)
				{
					if (m_cursor_pattern[y] & (1 << (15 - ((x - cursor_x) >> 1))))
					{
						scanline[x++] = color;
						scanline[x] = color;
					}
				}
			}
		}
	}
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
	LOGMASKED(LOG_STATUS, "%s: Control/Status Register 1 Read: %02x\n", machine().describe_context(), m_csrr[0]);
	return m_csrr[0];
}

void mcd212_device::csr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Control/Status Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_csrw[0]);
}

uint16_t mcd212_device::dcr1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: Display Command Register 1 Read: %04x & %08x\n", machine().describe_context(), m_dcr[0], mem_mask);
	return m_dcr[0];
}

void mcd212_device::dcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Display Command Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dcr[0]);
}

uint16_t mcd212_device::vsr1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: Video Start Register 1 Read: %04x & %08x\n", machine().describe_context(), m_vsr[0], mem_mask);
	return m_vsr[0];
}

void mcd212_device::vsr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Video Start Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_vsr[0]);
}

uint16_t mcd212_device::ddr1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: Display Decoder Register 1 Read: %04x & %08x\n", machine().describe_context(), m_ddr[0], mem_mask);
	return m_ddr[0];
}

void mcd212_device::ddr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Display Decoder Register 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ddr[0]);
}

uint16_t mcd212_device::dca1_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: DCA Pointer 1 Read: %04x & %08x\n", machine().describe_context(), m_dca[0], mem_mask);
	return m_dca[0];
}

void mcd212_device::dca1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: DCA Pointer 1 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dca[0]);
}

uint8_t mcd212_device::csr2_r()
{
	if (machine().side_effects_disabled())
	{
		return m_csrr[1];
	}

	const uint8_t data = m_csrr[1];
	LOGMASKED(LOG_STATUS, "%s: Status Register 2: %02x\n", machine().describe_context(), data);

	m_csrr[1] &= ~(CSR2R_IT1 | CSR2R_IT2);
	if (data & (CSR2R_IT1 | CSR2R_IT2))
		m_int_callback(CLEAR_LINE);

	return data;
}

void mcd212_device::csr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Control/Status Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_csrw[1]);
}

uint16_t mcd212_device::dcr2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: Display Command Register 2 Read: %04x & %08x\n", machine().describe_context(), m_dcr[1], mem_mask);
	return m_dcr[1];
}

void mcd212_device::dcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Display Command Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dcr[1]);
}

uint16_t mcd212_device::vsr2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: Video Start Register 2 Read: %04x & %08x\n", machine().describe_context(), m_vsr[1], mem_mask);
	return m_vsr[1];
}

void mcd212_device::vsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Video Start Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_vsr[1]);
}

uint16_t mcd212_device::ddr2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: Display Decoder Register 2 Read: %04x & %08x\n", machine().describe_context(), m_ddr[1], mem_mask);
	return m_ddr[1];
}

void mcd212_device::ddr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: Display Decoder Register 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ddr[1]);
}

uint16_t mcd212_device::dca2_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_READS, "%s: DCA Pointer 2 Read: %04x & %08x\n", machine().describe_context(), m_dca[1], mem_mask);
	return m_dca[1];
}

void mcd212_device::dca2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_MAIN_REG_WRITES, "%s: DCA Pointer 2 Write: %04x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_dca[1]);
}

TIMER_CALLBACK_MEMBER(mcd212_device::ica_tick)
{
	m_csrr[0] &= ~CSR1R_DA;

	// Process ICA
	if (BIT(m_dcr[0], DCR_ICA_BIT))
		process_ica<0>();
	if (BIT(m_dcr[1], DCR_ICA_BIT))
		process_ica<1>();

	if (BIT(m_dcr[0], DCR_DCA_BIT))
		m_dca[0] = get_dcp<0>();
	if (BIT(m_dcr[1], DCR_DCA_BIT))
		m_dca[1] = get_dcp<1>();

	m_ica_timer->adjust(screen().time_until_pos(0, 0));
}

TIMER_CALLBACK_MEMBER(mcd212_device::dca_tick)
{
	// Process DCA
	if (BIT(m_dcr[0], DCR_DCA_BIT))
		process_dca<0>();
	if (BIT(m_dcr[1], DCR_DCA_BIT))
		process_dca<1>();

	int scanline = screen().vpos();
	if (scanline == m_total_height - 1)
		m_dca_timer->adjust(screen().time_until_pos(m_ica_height, 784));
	else
		m_dca_timer->adjust(screen().time_until_pos(scanline + 1, 784));
}

uint32_t mcd212_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t plane_a[768];
	uint32_t plane_b[768];
	bool transparent_a[768];
	bool transparent_b[768];

	int scanline = screen.vpos();

	// Process VSR and mix if we're in the visible region
	if (scanline >= m_ica_height)
	{
		uint32_t *out = &bitmap.pix(scanline);

		bool draw_line = true;
		if (!BIT(m_dcr[0], DCR_FD_BIT) && BIT(m_csrw[0], CSR1W_ST_BIT))
		{
			// If PAL and 'Standard' bit set, insert a 20-line border on the top/bottom
			if ((scanline - m_ica_height < 20) || (scanline >= (m_total_height - 20)))
			{
				std::fill_n(out, 768, 0xff101010);
				draw_line = false;
			}
		}

		m_csrr[0] |= CSR1R_DA;

		if (draw_line)
		{
			// If PAL and 'Standard' bit set, insert a 24px border on the left/right
			if (!BIT(m_dcr[0], DCR_CF_BIT) || BIT(m_csrw[0], CSR1W_ST_BIT))
			{
				std::fill_n(out, 24, 0xff101010);
				out += 24;
			}

			process_vsr<0>(plane_a, transparent_a);
			process_vsr<1>(plane_b, transparent_b);

			const uint8_t mosaic_enable_a = (m_mosaic_hold[0] & 0x800000) >> 23;
			const uint8_t mosaic_enable_b = (m_mosaic_hold[1] & 0x800000) >> 22;
			const uint8_t mixing_mode = (mosaic_enable_a | mosaic_enable_b) | (BIT(m_plane_order, 0) << 2);
			switch (mixing_mode & 7)
			{
				case 0: // No Mosaic A/B, A->B->Backdrop plane ordering
					mix_lines<false, false, true>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 1: // Mosaic A, No Mosaic B, A->B->Backdrop plane ordering
					mix_lines<true, false, true>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 2: // No Mosaic A, Mosaic B, A->B->Backdrop plane ordering
					mix_lines<false, true, true>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 3: // Mosaic A/B, A->B->Backdrop plane ordering
					mix_lines<true, true, true>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 4: // No Mosaic A/B, B->A->Backdrop plane ordering
					mix_lines<false, false, false>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 5: // Mosaic A, No Mosaic B, B->A->Backdrop plane ordering
					mix_lines<true, false, false>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 6: // No Mosaic A, Mosaic B, B->A->Backdrop plane ordering
					mix_lines<false, true, false>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
				case 7: // Mosaic A/B, B->A->Backdrop plane ordering
					mix_lines<true, true, false>(plane_a, transparent_a, plane_b, transparent_b, out);
					break;
			}

			draw_cursor(out);
		}
	}

	// Toggle frame parity at the end of the visible frame (even in non-interlaced mode).
	if (scanline == (m_total_height - 1))
	{
		m_csrr[0] ^= CSR1R_PA;
	}

	return 0;
}

template int mcd212_device::ram_dtack_cycle_count<0>();
template int mcd212_device::ram_dtack_cycle_count<1>();

template <int Channel>
int mcd212_device::ram_dtack_cycle_count()
{
	// Per MCD-212 documentation, it takes 4 CLKs (2 SCC68070 clocks) for a VRAM access during the System timing slot.

	// No contending for Ch.1/Ch.2 timing slots if display is disabled
	if (!BIT(m_dcr[0], DCR_DE_BIT))
		return 2;

	// No contending for Ch.1/Ch.2 timing slots if a relevant channel is disabled
	if (!BIT(m_dcr[Channel], DCR_ICA_BIT))
		return 2;

	const int x = screen().hpos();
	const int y = screen().vpos();
	const bool x_outside_active_display = (x >= 408);

	// No contending for Ch.1/Ch.2 timing slots during the final 8-pixel area on all lines
	if (x >= 472)
		return 2;

	// No contending for Ch.1/Ch.2 timing slots during the free-run area of ICA lines
	if (y < m_ica_height && x_outside_active_display)
		return 2;

	// No contending for Ch.1/Ch.2 timing slots during the free-run area of DCA lines if DCA is disabled
	if (!BIT(m_dcr[Channel], DCR_DCA_BIT) && x_outside_active_display)
		return 2;

	// System access is restricted to the last 5 out of every 16 CLKs.
	const int slot_cycle = (int)(machine().time().as_ticks(clock()) & 0xf);
	if (slot_cycle >= 11)
		return 2;

	return 2 + std::max((11 - slot_cycle) >> 1, 1);
}

int mcd212_device::rom_dtack_cycle_count()
{
	static const int s_dd_values[4] = { 2, 3, 4, 5 };
	if (!BIT(m_csrw[0], CSR1W_DD_BIT))
		return 7;
	return s_dd_values[(m_csrw[0] & CSR1W_DD2) >> CSR1W_DD2_SHIFT];
}

void mcd212_device::device_reset()
{
	std::fill_n(m_csrr, 2, 0);
	std::fill_n(m_csrw, 2, 0);
	std::fill_n(m_dcr, 2, 0);
	std::fill_n(m_vsr, 2, 0);
	std::fill_n(m_ddr, 2, 0);
	std::fill_n(m_dcp, 2, 0);
	std::fill_n(m_dca, 2, 0);
	std::fill_n(m_clut, 256, 0);
	m_image_coding_method = 0;
	m_transparency_control = 0;
	m_plane_order = 0;
	std::fill_n(m_clut_bank, 2, 0);
	std::fill_n(m_transparent_color, 2, 0);
	std::fill_n(m_mask_color, 2, 0);
	std::fill_n(m_dyuv_abs_start, 2, 0);
	m_cursor_position = 0;
	m_cursor_control = 0;
	std::fill_n(m_cursor_pattern, std::size(m_cursor_pattern), 0);
	std::fill_n(m_region_control, 8, 0);
	m_backdrop_color = 0;
	std::fill_n(m_mosaic_hold, 2, 0);
	std::fill_n(m_weight_factor[0], std::size(m_weight_factor[0]), 0);
	std::fill_n(m_weight_factor[1], std::size(m_weight_factor[1]), 0);
	std::fill_n(m_region_flag[0], std::size(m_region_flag[0]), false);
	std::fill_n(m_region_flag[1], std::size(m_region_flag[1]), false);

	m_ica_height = 32;
	m_total_height = 312;

	m_int_callback(CLEAR_LINE);

	m_dca_timer->adjust(screen().time_until_pos(m_ica_height, 784));
	m_ica_timer->adjust(screen().time_until_pos(m_ica_height, 0));
}

//-------------------------------------------------
//  mcd212_device - constructor
//-------------------------------------------------

mcd212_device::mcd212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MCD212, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_int_callback(*this)
	, m_planea(*this, finder_base::DUMMY_TAG)
	, m_planeb(*this, finder_base::DUMMY_TAG)
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
	static const uint8_t s_dyuv_deltas[16] = { 0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255 };

	for (uint16_t d = 0; d < 0x100; d++)
	{
		m_delta_y_lut[d] = s_dyuv_deltas[d & 15];
		m_delta_uv_lut[d] = s_dyuv_deltas[d >> 4];
	}

	for (uint16_t w = 0; w < 3 * 0xff; w++)
	{
		const uint8_t limit = (w < 0xff + 16) ?  0 : w <= 16 + 2 * 0xff ? w - 0x10f : 0xff;
		m_dyuv_limit_r_lut[w] = limit << 16;
		m_dyuv_limit_g_lut[w] = limit << 8;
		m_dyuv_limit_b_lut[w] = limit;
	}

	for (int16_t sw = 0; sw < 0x100; sw++)
	{
		m_dyuv_u_to_b[sw] = (444 * (sw - 128)) / 256;
		m_dyuv_u_to_g[sw] = - (86 * (sw - 128)) / 256;
		m_dyuv_v_to_g[sw] = - (179 * (sw - 128)) / 256;
		m_dyuv_v_to_r[sw] = (351 * (sw - 128)) / 256;
	}

	save_item(NAME(m_region_flag[0]));
	save_item(NAME(m_region_flag[1]));
	save_item(NAME(m_ica_height));
	save_item(NAME(m_total_height));
	save_item(NAME(m_csrr));
	save_item(NAME(m_csrw));
	save_item(NAME(m_dcr));
	save_item(NAME(m_vsr));
	save_item(NAME(m_ddr));
	save_item(NAME(m_dcp));
	save_item(NAME(m_dca));
	save_item(NAME(m_clut));
	save_item(NAME(m_image_coding_method));
	save_item(NAME(m_transparency_control));
	save_item(NAME(m_plane_order));
	save_item(NAME(m_clut_bank));
	save_item(NAME(m_transparent_color));
	save_item(NAME(m_mask_color));
	save_item(NAME(m_dyuv_abs_start));
	save_item(NAME(m_cursor_position));
	save_item(NAME(m_cursor_control));
	save_item(NAME(m_cursor_pattern));
	save_item(NAME(m_region_control));
	save_item(NAME(m_backdrop_color));
	save_item(NAME(m_mosaic_hold));
	save_item(NAME(m_weight_factor[0]));
	save_item(NAME(m_weight_factor[1]));

	m_dca_timer = timer_alloc(FUNC(mcd212_device::dca_tick), this);
	m_dca_timer->adjust(attotime::never);

	m_ica_timer = timer_alloc(FUNC(mcd212_device::ica_tick), this);
	m_ica_timer->adjust(attotime::never);
}
