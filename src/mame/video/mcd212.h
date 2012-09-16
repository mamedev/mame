/******************************************************************************


    CD-i MCD212 video emulation
    -------------------

    MESS implementation by Harmony


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Unknown yet.

*******************************************************************************/

#ifndef _VIDEO_MCD212_H_
#define _VIDEO_MCD212_H_

#include "emu.h"

struct mcd212_channel_t 
{
    UINT8 csrr;
    UINT16 csrw;
    UINT16 dcr;
    UINT16 vsr;
    UINT16 ddr;
    UINT16 dcp;
    UINT32 dca;
    UINT8 clut_r[256];
    UINT8 clut_g[256];
    UINT8 clut_b[256];
    UINT32 image_coding_method;
    UINT32 transparency_control;
    UINT32 plane_order;
    UINT32 clut_bank;
    UINT32 transparent_color_a;
    UINT32 reserved0;
    UINT32 transparent_color_b;
    UINT32 mask_color_a;
    UINT32 reserved1;
    UINT32 mask_color_b;
    UINT32 dyuv_abs_start_a;
    UINT32 dyuv_abs_start_b;
    UINT32 reserved2;
    UINT32 cursor_position;
    UINT32 cursor_control;
    UINT32 cursor_pattern[16];
    UINT32 region_control[8];
    UINT32 backdrop_color;
    UINT32 mosaic_hold_a;
    UINT32 mosaic_hold_b;
    UINT8 weight_factor_a[768];
    UINT8 weight_factor_b[768];
};

struct mcd212_regs_t
{
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

    mcd212_channel_t channel[2];
    emu_timer *scan_timer;
    UINT8 region_flag_0[768];
    UINT8 region_flag_1[768];

    running_machine *m_machine;
    bitmap_rgb32 m_bitmap;
};

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

typedef UINT8 BYTE68K;
typedef UINT16 WORD68K;
typedef INT16 SWORD68K;

#define BYTE68K_MAX 255



struct mcd212_ab_t
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

// Member functions
READ16_HANDLER( mcd212_r );
WRITE16_HANDLER( mcd212_w );
TIMER_CALLBACK( mcd212_perform_scan );
VIDEO_START( cdimono1 );
SCREEN_UPDATE_RGB32( cdimono1 );
SCREEN_UPDATE_RGB32( cdimono1_lcd );

#endif // _VIDEO_MCD212_H_
