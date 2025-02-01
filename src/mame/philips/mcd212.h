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

#ifndef MAME_PHILIPS_MCD212_H
#define MAME_PHILIPS_MCD212_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mcd212_device : public device_t,
					  public device_video_interface
{
public:
	template <typename T, typename U>
	mcd212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&plane_a_tag, U &&plane_b_tag)
		: mcd212_device(mconfig, tag, owner, clock)
	{
		m_planea.set_tag(std::forward<T>(plane_a_tag));
		m_planeb.set_tag(std::forward<U>(plane_b_tag));
	}

	mcd212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_int_callback.bind(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void map(address_map &map) ATTR_COLD;

	template <int Path> int ram_dtack_cycle_count();
	int rom_dtack_cycle_count();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(ica_tick);
	TIMER_CALLBACK_MEMBER(dca_tick);

	uint8_t csr1_r();
	void csr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dcr1_r(offs_t offset, uint16_t mem_mask = ~0);
	void dcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vsr1_r(offs_t offset, uint16_t mem_mask = ~0);
	void vsr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ddr1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ddr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dca1_r(offs_t offset, uint16_t mem_mask = ~0);
	void dca1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t csr2_r();
	void csr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dcr2_r(offs_t offset, uint16_t mem_mask = ~0);
	void dcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vsr2_r(offs_t offset, uint16_t mem_mask = ~0);
	void vsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ddr2_r(offs_t offset, uint16_t mem_mask = ~0);
	void ddr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dca2_r(offs_t offset, uint16_t mem_mask = ~0);
	void dca2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	enum : uint32_t
	{
		CURCNT_COLOR         = 0x00000f,    // Cursor color
		CURCNT_CUW           = 0x008000,    // Cursor width
		CURCNT_COF           = 0x070000,    // Cursor off time
		CURCNT_COF_SHIFT     = 16,
		CURCNT_CON           = 0x280000,    // Cursor on time
		CURCNT_CON_SHIFT     = 19,
		CURCNT_BLKC          = 0x400000,    // Blink type
		CURCNT_EN            = 0x800000,    // Cursor enable

		ICM_MODE1            = 0x00000f,    // Plane 1
		ICM_MODE1_SHIFT      = 0,
		ICM_MODE2            = 0x000f00,    // Plane 2
		ICM_MODE2_SHIFT      = 8,
		ICM_EV               = 0x040000,    // External video
		ICM_NM               = 0x080000,    // Number of Matte flags
		ICM_NM_BIT           = 19,
		ICM_CS               = 0x400000,    // CLUT select

		TCR_TA               = 0x00000f,    // Plane A
		TCR_TB               = 0x000f00,    // Plane B
		TCR_TB_SHIFT         = 8,
		TCR_ALWAYS           = 0x0,         // Transparent if: Always (Plane Disabled)
		TCR_KEY              = 0x1,         // Transparent if: Color Key = True
		TCR_RGB              = 0x2,         // Transparent if: Transparency Bit = 1 (RGB Only)
		TCR_MF0              = 0x3,         // Transparent if: Matte Flag 0 = True
		TCR_MF1              = 0x4,         // Transparent if: Matte Flag 1 = True
		TCR_MF0_KEY1         = 0x5,         // Transparent if: Matte Flag 0 = True || Color Key = True
		TCR_MF1_KEY1         = 0x6,         // Transparent if: Matte Flag 1 = True || Color Key = True
		TCR_COND_UNUSED0     = 0x7,         // Unused
		TCR_NEVER            = 0x8,         // Transparent if: Never (No Transparent Area)
		TCR_NOT_KEY          = 0x9,         // Transparent if: Color Key = False
		TCR_NOT_RGB          = 0xa,         // Transparent if: Transparency Bit = 0 (RGB Only)
		TCR_NOT_MF0          = 0xb,         // Transparent if: Matte Flag 0 = False
		TCR_NOT_MF1          = 0xc,         // Transparent if: Matte Flag 1 = False
		TCR_NOT_MF0_KEY      = 0xd,         // Transparent if: Matte Flag 0 = False || Color Key = False
		TCR_NOT_MF1_KEY      = 0xe,         // Transparent if: Matte Flag 1 = False || Color Key = False
		TCR_COND_UNUSED1     = 0xf,         // Unused
		TCR_DISABLE_MX       = 0x800000,    // Mix disable

		POR_AB               = 0,           // Plane A in front of Plane B
		POR_BA               = 1,           // Plane B in front of Plane A

		MC_X                 = 0x0003ff,    // X position
		MC_WF                = 0x00fc00,    // Weight position
		MC_WF_SHIFT          = 10,
		MC_MF_BIT            = 16,          // Matte flag bit
		MC_OP                = 0xf00000,    // Operation
		MC_OP_SHIFT          = 20,

		CSR1R_PA             = 0x20,        // Parity
		CSR1R_DA             = 0x80,        // Display Active

		CSR1W_BE             = 0x0001,      // Bus Error
		CSR1W_ST_BIT         = 1,           // Standard
		CSR1W_DD_BIT         = 3,
		CSR1W_DD2            = 0x0300,      // /DTACK Delay
		CSR1W_DD2_SHIFT      = 8,
		CSR1W_DI1_BIT        = 15,

		CSR2R_BE             = 0x0001,      // Bus Error
		CSR2R_IT2            = 0x0002,      // Interrupt 2
		CSR2R_IT1            = 0x0004,      // Interrupt 1

		DCR_DCA_BIT          = 8,           // DCA Enable Ch.1/2
		DCR_ICA_BIT          = 9,           // ICA Enable Ch.1/2
		DCR_CM_BIT           = 11,          // Color Mode Ch.1/2
		DCR_SM_BIT           = 12,          // Scan Mode
		DCR_FD_BIT           = 13,          // Frame Duration
		DCR_CF_BIT           = 14,          // Crystal Frequency
		DCR_DE_BIT           = 15,          // Display Enable

		DDR_MT               = 0x0c00,      // Mosaic File Type
		DDR_MT_2             = 0x0000,      // 2x1
		DDR_MT_4             = 0x0400,      // 4x1
		DDR_MT_8             = 0x0800,      // 8x1
		DDR_MT_16            = 0x0c00,      // 16x1
		DDR_MT_SHIFT         = 10,
		DDR_FT               = 0x0300,      // Display File Type
		DDR_FT_BMP           = 0x0000,      // Bitmap
		DDR_FT_BMP2          = 0x0100,      // Bitmap (alt.)
		DDR_FT_RLE           = 0x0200,      // Run-Length Encoded
		DDR_FT_MOSAIC        = 0x0300,      // Mosaic

		ICM_OFF              = 0x0,
		ICM_CLUT8            = 0x1,
		ICM_RGB555           = 0x1,
		ICM_CLUT7            = 0x3,
		ICM_CLUT77           = 0x4,
		ICM_DYUV             = 0x5,
		ICM_CLUT4            = 0xb
	};

	uint8_t m_csrr[2]{};
	uint16_t m_csrw[2]{};
	uint16_t m_dcr[2]{};
	uint16_t m_vsr[2]{};
	uint16_t m_ddr[2]{};
	uint16_t m_dcp[2]{};
	uint32_t m_dca[2]{};
	uint32_t m_clut[256]{};
	uint32_t m_image_coding_method = 0;
	uint32_t m_transparency_control = 0;
	uint32_t m_plane_order = 0;
	uint32_t m_clut_bank[2]{};
	uint32_t m_transparent_color[2]{};
	uint32_t m_mask_color[2]{};
	uint32_t m_dyuv_abs_start[2]{};
	uint32_t m_cursor_position = 0;
	uint32_t m_cursor_control = 0;
	uint32_t m_cursor_pattern[16]{};
	uint32_t m_matte_control[8]{};
	uint32_t m_backdrop_color = 0;
	uint32_t m_mosaic_hold[2]{};
	uint8_t m_weight_factor[2][768]{};

	// DYUV color limit arrays.
	uint32_t m_dyuv_limit_lut[0x300];

	// DYUV delta-Y decoding array
	uint8_t m_delta_y_lut[0x100];

	// DYUV delta-UV decoding array
	uint8_t m_delta_uv_lut[0x100];

	// DYUV U-to-B decoding array
	int16_t m_dyuv_u_to_b[0x100];

	// U-to-G decoding array
	int16_t m_dyuv_u_to_g[0x100];

	// V-to-G decoding array
	int16_t m_dyuv_v_to_g[0x100];

	// V-to-R decoding array
	int16_t m_dyuv_v_to_r[0x100];

	// interrupt callbacks
	devcb_write_line m_int_callback;

	required_shared_ptr<uint16_t> m_planea;
	required_shared_ptr<uint16_t> m_planeb;

	// internal state
	bool m_matte_flag[2][768]{};
	int m_ica_height = 0;
	int m_total_height = 0;
	emu_timer *m_ica_timer = nullptr;
	emu_timer *m_dca_timer = nullptr;

	static const uint32_t s_4bpp_color[16];

	uint8_t get_weight_factor(const uint32_t Matte_idx);
	uint8_t get_matte_op(const uint32_t Matte_idx);
	void update_matte_arrays();

	int get_screen_width();
	int get_border_width();

	template <int Path> void set_vsr(uint32_t value);
	template <int Path> uint32_t get_vsr();

	template <int Path> void set_dcp(uint32_t value);
	template <int Path> uint32_t get_dcp();

	template <int Path> void set_display_parameters(uint8_t value);

	template <int Path> void process_ica();
	template <int Path> void process_dca();

	template <int Path> uint8_t get_transparency_control();
	template <int Path> uint8_t get_icm();
	template <int Path> bool get_mosaic_enable();
	template <int Path> uint8_t get_mosaic_factor();
	template <int Path> void process_vsr(uint32_t *pixels, bool *transparent);

	template <int Path> void set_register(uint8_t reg, uint32_t value);

	template <bool MosaicA, bool MosaicB, bool OrderAB> void mix_lines(uint32_t *plane_a, bool *transparent_a, uint32_t *plane_b, bool *transparent_b, uint32_t *out);

	void draw_cursor(uint32_t *scanline);
};

// device type definition
DECLARE_DEVICE_TYPE(MCD212, mcd212_device)

#endif // MAME_PHILIPS_MCD212_H
