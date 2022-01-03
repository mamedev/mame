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

#ifndef MAME_VIDEO_MCD212_H
#define MAME_VIDEO_MCD212_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mcd212_device

class mcd212_device : public device_t,
						public device_video_interface
{
public:
	typedef device_delegate<void (int)> scanline_callback_delegate;

	// construction/destruction
	mcd212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_int_callback.bind(); }

	// device members
	void map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	struct channel_t
	{
		uint8_t csrr;
		uint16_t csrw;
		uint16_t dcr;
		uint16_t vsr;
		uint16_t ddr;
		uint16_t dcp;
		uint32_t dca;
		uint8_t clut_r[256];
		uint8_t clut_g[256];
		uint8_t clut_b[256];
		uint32_t image_coding_method;
		uint32_t transparency_control;
		uint32_t plane_order;
		uint32_t clut_bank;
		uint32_t transparent_color_a;
		uint32_t reserved0;
		uint32_t transparent_color_b;
		uint32_t mask_color_a;
		uint32_t reserved1;
		uint32_t mask_color_b;
		uint32_t dyuv_abs_start_a;
		uint32_t dyuv_abs_start_b;
		uint32_t reserved2;
		uint32_t cursor_position;
		uint32_t cursor_control;
		uint32_t cursor_pattern[16];
		uint32_t region_control[8];
		uint32_t backdrop_color;
		uint32_t mosaic_hold_a;
		uint32_t mosaic_hold_b;
		uint8_t weight_factor_a[768];
		uint8_t weight_factor_b[768];
	};

	struct ab_t
	{
		//* Color limit array.
		uint8_t limit[3 * 0xff];

		//* Color clamp array.
		uint8_t clamp[3 * 0xff];

		//* U-to-B matrix array.
		int16_t matrixUB[0x100];

		//* U-to-G matrix array.
		int16_t matrixUG[0x100];

		//* V-to-G matrix array.
		int16_t matrixVG[0x100];

		//* V-to-R matrix array.
		int16_t matrixVR[0x100];

		//* Delta-Y decoding array.
		uint8_t deltaY[0x100];

		//* Delta-U/V decoding array.
		uint8_t deltaUV[0x100];
	};

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

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

		ICM_CS               = 0x400000,    // CLUT select
		ICM_NR               = 0x080000,    // Number of region flags
		ICM_NR_BIT           = 19,
		ICM_EV               = 0x040000,    // External video
		ICM_MODE2            = 0x000f00,    // Plane 2
		ICM_MODE2_SHIFT      = 8,
		ICM_MODE1            = 0x00000f,    // Plane 1
		ICM_MODE1_SHIFT      = 0,

		TCR_DISABLE_MX       = 0x800000,    // Mix disable
		TCR_TB               = 0x000f00,    // Plane B
		TCR_TB_SHIFT         = 8,
		TCR_TA               = 0x00000f,    // Plane A
		TCR_COND_1           = 0x0,         // Transparent if: Always (Plane Disabled)
		TCR_COND_KEY_1       = 0x1,         // Transparent if: Color Key = True
		TCR_COND_XLU_1       = 0x2,         // Transparent if: Transparency Bit = 1
		TCR_COND_RF0_1       = 0x3,         // Transparent if: Region Flag 0 = True
		TCR_COND_RF1_1       = 0x4,         // Transparent if: Region Flag 1 = True
		TCR_COND_RF0KEY_1    = 0x5,         // Transparent if: Region Flag 0 = True || Color Key = True
		TCR_COND_RF1KEY_1    = 0x6,         // Transparent if: Region Flag 1 = True || Color Key = True
		TCR_COND_UNUSED0     = 0x7,         // Unused
		TCR_COND_0           = 0x8,         // Transparent if: Never (No Transparent Area)
		TCR_COND_KEY_0       = 0x9,         // Transparent if: Color Key = False
		TCR_COND_XLU_0       = 0xa,         // Transparent if: Transparency Bit = 0
		TCR_COND_RF0_0       = 0xb,         // Transparent if: Region Flag 0 = False
		TCR_COND_RF1_0       = 0xc,         // Transparent if: Region Flag 1 = False
		TCR_COND_RF0KEY_0    = 0xd,         // Transparent if: Region Flag 0 = False && Color Key = False
		TCR_COND_RF1KEY_0    = 0xe,         // Transparent if: Region Flag 1 = False && Color Key = False
		TCR_COND_UNUSED1     = 0xf,         // Unused

		POR_AB               = 0,           // Plane A in front of Plane B
		POR_BA               = 1,           // Plane B in front of Plane A

		RC_X                 = 0x0003ff,    // X position
		RC_WF                = 0x00fc00,    // Weight position
		RC_WF_SHIFT          = 10,
		RC_RF_BIT            = 16,          // Region flag bit
		RC_OP                = 0xf00000,    // Operation
		RC_OP_SHIFT          = 20,

		CSR1W_ST             = 0x0002,  // Standard
		CSR1W_BE             = 0x0001,  // Bus Error

		CSR2R_IT1            = 0x0004,  // Interrupt 1
		CSR2R_IT2            = 0x0002,  // Interrupt 2
		CSR2R_BE             = 0x0001,  // Bus Error

		DCR_DE               = 0x8000,  // Display Enable
		DCR_CF               = 0x4000,  // Crystal Frequency
		DCR_FD               = 0x2000,  // Frame Duration
		DCR_SM               = 0x1000,  // Scan Mode
		DCR_CM               = 0x0800,  // Color Mode Ch.1/2
		DCR_ICA              = 0x0200,  // ICA Enable Ch.1/2
		DCR_DCA              = 0x0100,  // DCA Enable Ch.1/2

		DDR_FT               = 0x0300,  // Display File Type
		DDR_FT_BMP           = 0x0000,  // Bitmap
		DDR_FT_BMP2          = 0x0100,  // Bitmap (alt.)
		DDR_FT_RLE           = 0x0200,  // Run-Length Encoded
		DDR_FT_MOSAIC        = 0x0300,  // Mosaic
		DDR_MT               = 0x0c00,  // Mosaic File Type
		DDR_MT_2             = 0x0000,  // 2x1
		DDR_MT_4             = 0x0400,  // 4x1
		DDR_MT_8             = 0x0800,  // 8x1
		DDR_MT_16            = 0x0c00,  // 16x1
		DDR_MT_SHIFT         = 10
	};

	// interrupt callbacks
	devcb_write_line m_int_callback;

	required_shared_ptr<uint16_t> m_planea;
	required_shared_ptr<uint16_t> m_planeb;

	// internal state
	channel_t m_channel[2];
	uint8_t m_region_flag_0[768];
	uint8_t m_region_flag_1[768];
	int m_ica_height;
	int m_total_height;

	static const uint32_t s_4bpp_color[16];

	ab_t m_ab;

	uint8_t get_weight_factor(const uint32_t region_idx);
	uint8_t get_region_op(const uint32_t region_idx);
	void update_region_arrays();

	void set_vsr(int channel, uint32_t value);
	uint32_t get_vsr(int channel);

	void set_dcp(int channel, uint32_t value);
	uint32_t get_dcp(int channel);

	void set_display_parameters(int channel, uint8_t value);
	void update_visible_area();
	uint32_t get_screen_width();

	void process_ica(int channel);
	void process_dca(int channel);

	uint8_t get_icm(const int channel);
	bool get_mosaic_enable(const int channel);
	uint8_t get_mosaic_factor(const int channel);
	void process_vsr(int channel, uint8_t *pixels_r, uint8_t *pixels_g, uint8_t *pixels_b);

	void set_register(int channel, uint8_t reg, uint32_t value);

	void mix_lines(uint8_t *plane_a_r, uint8_t *plane_a_g, uint8_t *plane_a_b, uint8_t *plane_b_r, uint8_t *plane_b_g, uint8_t *plane_b_b, uint32_t *out);

	void draw_cursor(uint32_t *scanline);
	void draw_scanline(bitmap_rgb32 &bitmap);

	void ab_init();
};

// device type definition
DECLARE_DEVICE_TYPE(MCD212, mcd212_device)

#endif // MAME_VIDEO_MCD212_H
