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


#define MCD212_CURCNT_COLOR         0x00000f    // Cursor color
#define MCD212_CURCNT_CUW           0x008000    // Cursor width
#define MCD212_CURCNT_COF           0x070000    // Cursor off time
#define MCD212_CURCNT_COF_SHIFT     16
#define MCD212_CURCNT_CON           0x280000    // Cursor on time
#define MCD212_CURCNT_CON_SHIFT     19
#define MCD212_CURCNT_BLKC          0x400000    // Blink type
#define MCD212_CURCNT_EN            0x800000    // Cursor enable

#define MCD212_ICM_CS               0x400000    // CLUT select
#define MCD212_ICM_NR               0x080000    // Number of region flags
#define MCD212_ICM_EV               0x040000    // External video
#define MCD212_ICM_MODE2            0x000f00    // Plane 2
#define MCD212_ICM_MODE2_SHIFT      8
#define MCD212_ICM_MODE1            0x00000f    // Plane 1
#define MCD212_ICM_MODE1_SHIFT      0

#define MCD212_TCR_DISABLE_MX       0x800000    // Mix disable
#define MCD212_TCR_TB               0x000f00    // Plane B
#define MCD212_TCR_TB_SHIFT         8
#define MCD212_TCR_TA               0x00000f    // Plane A
#define MCD212_TCR_COND_1           0x0         // Transparent if: Always (Plane Disabled)
#define MCD212_TCR_COND_KEY_1       0x1         // Transparent if: Color Key = True
#define MCD212_TCR_COND_XLU_1       0x2         // Transparent if: Transparency Bit = 1
#define MCD212_TCR_COND_RF0_1       0x3         // Transparent if: Region Flag 0 = True
#define MCD212_TCR_COND_RF1_1       0x4         // Transparent if: Region Flag 1 = True
#define MCD212_TCR_COND_RF0KEY_1    0x5         // Transparent if: Region Flag 0 = True || Color Key = True
#define MCD212_TCR_COND_RF1KEY_1    0x6         // Transparent if: Region Flag 1 = True || Color Key = True
#define MCD212_TCR_COND_UNUSED0     0x7         // Unused
#define MCD212_TCR_COND_0           0x8         // Transparent if: Never (No Transparent Area)
#define MCD212_TCR_COND_KEY_0       0x9         // Transparent if: Color Key = False
#define MCD212_TCR_COND_XLU_0       0xa         // Transparent if: Transparency Bit = 0
#define MCD212_TCR_COND_RF0_0       0xb         // Transparent if: Region Flag 0 = False
#define MCD212_TCR_COND_RF1_0       0xc         // Transparent if: Region Flag 1 = False
#define MCD212_TCR_COND_RF0KEY_0    0xd         // Transparent if: Region Flag 0 = False && Color Key = False
#define MCD212_TCR_COND_RF1KEY_0    0xe         // Transparent if: Region Flag 1 = False && Color Key = False
#define MCD212_TCR_COND_UNUSED1     0xf         // Unused

#define MCD212_POR_AB               0           // Plane A in front of Plane B
#define MCD212_POR_BA               1           // Plane B in front of Plane A

#define MCD212_RC_X                 0x0003ff    // X position
#define MCD212_RC_WF                0x00fc00    // Weight position
#define MCD212_RC_WF_SHIFT          10
#define MCD212_RC_RF                0x010000    // Region flag
#define MCD212_RC_RF_SHIFT          16
#define MCD212_RC_OP                0xf00000    // Operation
#define MCD212_RC_OP_SHIFT          20

#define MCD212_CSR1W_ST             0x0002  // Standard
#define MCD212_CSR1W_BE             0x0001  // Bus Error

#define MCD212_CSR2R_IT1            0x0004  // Interrupt 1
#define MCD212_CSR2R_IT2            0x0002  // Interrupt 2
#define MCD212_CSR2R_BE             0x0001  // Bus Error

#define MCD212_DCR_DE               0x8000  // Display Enable
#define MCD212_DCR_CF               0x4000  // Crystal Frequency
#define MCD212_DCR_FD               0x2000  // Frame Duration
#define MCD212_DCR_SM               0x1000  // Scan Mode
#define MCD212_DCR_CM               0x0800  // Color Mode Ch.1/2
#define MCD212_DCR_ICA              0x0200  // ICA Enable Ch.1/2
#define MCD212_DCR_DCA              0x0100  // DCA Enable Ch.1/2

#define MCD212_DDR_FT               0x0300  // Display File Type
#define MCD212_DDR_FT_BMP           0x0000  // Bitmap
#define MCD212_DDR_FT_BMP2          0x0100  // Bitmap (alt.)
#define MCD212_DDR_FT_RLE           0x0200  // Run-Length Encoded
#define MCD212_DDR_FT_MOSAIC        0x0300  // Mosaic
#define MCD212_DDR_MT               0x0c00  // Mosaic File Type
#define MCD212_DDR_MT_2             0x0000  // 2x1
#define MCD212_DDR_MT_4             0x0400  // 4x1
#define MCD212_DDR_MT_8             0x0800  // 8x1
#define MCD212_DDR_MT_16            0x0c00  // 16x1
#define MCD212_DDR_MT_SHIFT         10

typedef uint8_t BYTE68K;
typedef uint16_t WORD68K;
typedef int16_t SWORD68K;

#define BYTE68K_MAX 255

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

	template <typename Object> void set_scanline_callback(Object &&cb) { m_scanline_callback = std::forward<Object>(cb); }
	void set_scanline_callback(scanline_callback_delegate callback) { m_scanline_callback = callback; }
	template <class FunctionClass> void set_scanline_callback(const char *devname, void (FunctionClass::*callback)(int), const char *name)
	{
		set_scanline_callback(scanline_callback_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_scanline_callback(void (FunctionClass::*callback)(int), const char *name)
	{
		set_scanline_callback(scanline_callback_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}

	// device members
	DECLARE_READ16_MEMBER( regs_r );
	DECLARE_WRITE16_MEMBER( regs_w );
	TIMER_CALLBACK_MEMBER( perform_scan );

	bitmap_rgb32& get_bitmap() { return m_bitmap; }

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
		BYTE68K limit[3 * BYTE68K_MAX];

		//* Color clamp array.
		BYTE68K clamp[3 * BYTE68K_MAX];

		//* U-to-B matrix array.
		SWORD68K matrixUB[BYTE68K_MAX + 1];

		//* U-to-G matrix array.
		SWORD68K matrixUG[BYTE68K_MAX + 1];

		//* V-to-G matrix array.
		SWORD68K matrixVG[BYTE68K_MAX + 1];

		//* V-to-R matrix array.
		SWORD68K matrixVR[BYTE68K_MAX + 1];

		//* Delta-Y decoding array.
		BYTE68K deltaY[BYTE68K_MAX + 1];

		//* Delta-U/V decoding array.
		BYTE68K deltaUV[BYTE68K_MAX + 1];
	};

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// interrupt callbacks
	devcb_write_line m_int_callback;

	scanline_callback_delegate m_scanline_callback;

	required_shared_ptr<uint16_t> m_planea;
	required_shared_ptr<uint16_t> m_planeb;

	// internal state
	channel_t m_channel[2];
	emu_timer *m_scan_timer;
	uint8_t m_region_flag_0[768];
	uint8_t m_region_flag_1[768];

	bitmap_rgb32 m_bitmap;

	static const uint32_t s_4bpp_color[16];

	ab_t m_ab;

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
	void process_vsr(int channel, uint8_t *pixels_r, uint8_t *pixels_g, uint8_t *pixels_b);

	void set_register(int channel, uint8_t reg, uint32_t value);

	void mix_lines(uint8_t *plane_a_r, uint8_t *plane_a_g, uint8_t *plane_a_b, uint8_t *plane_b_r, uint8_t *plane_b_g, uint8_t *plane_b_b, uint32_t *out);

	void draw_cursor(uint32_t *scanline, int y);
	void draw_scanline(int y);

	void ab_init();
};

// device type definition
DECLARE_DEVICE_TYPE(MCD212, mcd212_device)

#endif // MAME_VIDEO_MCD212_H
