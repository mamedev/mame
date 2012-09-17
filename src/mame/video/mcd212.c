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

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mcd212.h"
#include "includes/cdi.h"

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
    if( VERBOSE_LEVEL >= n_level )
    {
        va_list v;
        char buf[ 32768 ];
        va_start( v, s_fmt );
        vsprintf( buf, s_fmt, v );
        va_end( v );
        logerror( "%08x: %s", machine.device("maincpu")->safe_pc(), buf );
    }
}
#else
#define verboselog(x,y,z,...)
#endif

static void cdi220_draw_lcd(running_machine &machine, int y);
static void mcd212_update_region_arrays(mcd212_regs_t *mcd212);
static void mcd212_set_display_parameters(mcd212_regs_t *mcd212, int channel, UINT8 value);
static void mcd212_update_visible_area(running_machine &machine);
static void mcd212_set_vsr(mcd212_regs_t *mcd212, int channel, UINT32 value);
static void mcd212_set_dcp(mcd212_regs_t *mcd212, int channel, UINT32 value);
static UINT32 mcd212_get_vsr(mcd212_regs_t *mcd212, int channel);
static UINT32 mcd212_get_dcp(mcd212_regs_t *mcd212, int channel);
static UINT32 mcd212_get_screen_width(mcd212_regs_t *mcd212);
static void mcd212_set_register(running_machine &machine, int channel, UINT8 reg, UINT32 value);
static void mcd212_process_ica(mcd212_regs_t *mcd212, int channel);
static void mcd212_process_dca(mcd212_regs_t *mcd212, int channel);
static void mcd212_process_vsr(mcd212_regs_t *mcd212, int channel, UINT8 *pixels_r, UINT8 *pixels_g, UINT8 *pixels_b);
static void mcd212_mix_lines(mcd212_regs_t *mcd212, UINT8 *plane_a_r, UINT8 *plane_a_g, UINT8 *plane_a_b, UINT8 *plane_b_r, UINT8 *plane_b_g, UINT8 *plane_b_b, UINT32 *out);
static void mcd212_draw_cursor(mcd212_regs_t *mcd212, UINT32 *scanline, int y);
static void mcd212_draw_scanline(mcd212_regs_t *mcd212, int y);

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

static void cdi220_draw_lcd(running_machine &machine, int y)
{
    cdi_state *state = machine.driver_data<cdi_state>();
    bitmap_rgb32 &bitmap = state->m_lcdbitmap;
    UINT32 *scanline = &bitmap.pix32(y);
    int x = 0;
    int lcd = 0;

    for(lcd = 0; lcd < 8; lcd++)
    {
        cdislave_device *slave = downcast<cdislave_device *>(machine.device("slave"));
        UINT16 data = (slave->get_lcd_state()[lcd*2] << 8) |
                       slave->get_lcd_state()[lcd*2 + 1];
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

static void mcd212_update_region_arrays(mcd212_regs_t *mcd212)
{
    int x = 0;
    int latched_rf0 = 0;
    int latched_rf1 = 0;
    int latched_wfa = mcd212->channel[0].weight_factor_a[0];
    int latched_wfb = mcd212->channel[1].weight_factor_b[0];
    int reg = 0;

    for(x = 0; x < 768; x++)
    {
        if(mcd212->channel[0].image_coding_method & MCD212_ICM_NR)
        {
            int reg_ = 0;
            int flag = 0;

            for(flag = 0; flag < 2; flag++)
            {
                for(reg_ = 0; reg_ < 4; reg_++)
                {
                    if(mcd212->channel[0].region_control[reg_] == 0)
                    {
                        break;
                    }
                    if(x == (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_X))
                    {
                        switch((mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_OP) >> MCD212_RC_OP_SHIFT)
                        {
                            case 0: // End of region control for line
                                break;
                            case 1:
                            case 2:
                            case 3: // Not used
                                break;
                            case 4: // Change weight of plane A
                                latched_wfa = (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
                                break;
                            case 5: // Not used
                                break;
                            case 6: // Change weight of plane B
                                latched_wfb = (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                                latched_wfa = (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                                latched_wfa = (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                                latched_wfb = (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                                latched_wfb = (mcd212->channel[0].region_control[flag*4 + reg_] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                int flag = (mcd212->channel[0].region_control[reg] & MCD212_RC_RF) >> MCD212_RC_RF_SHIFT;
                if(!(mcd212->channel[0].region_control[reg] & MCD212_RC_OP))
                {
                    for(; x < 768; x++)
                    {
                        mcd212->channel[0].weight_factor_a[x] = latched_wfa;
                        mcd212->channel[1].weight_factor_b[x] = latched_wfb;
                        mcd212->region_flag_0[x] = latched_rf0;
                        mcd212->region_flag_1[x] = latched_rf1;
                    }
                    break;
                }
                if(x == (mcd212->channel[0].region_control[reg] & MCD212_RC_X))
                {
                    switch((mcd212->channel[0].region_control[reg] & MCD212_RC_OP) >> MCD212_RC_OP_SHIFT)
                    {
                        case 0: // End of region control for line
                            break;
                        case 1:
                        case 2:
                        case 3: // Not used
                            break;
                        case 4: // Change weight of plane A
                            latched_wfa = (mcd212->channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
                            break;
                        case 5: // Not used
                            break;
                        case 6: // Change weight of plane B
                            latched_wfb = (mcd212->channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                            latched_wfa = (mcd212->channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                            latched_wfa = (mcd212->channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                            latched_wfb = (mcd212->channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
                            latched_wfb = (mcd212->channel[0].region_control[reg] & MCD212_RC_WF) >> MCD212_RC_WF_SHIFT;
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
        mcd212->channel[0].weight_factor_a[x] = latched_wfa;
        mcd212->channel[1].weight_factor_b[x] = latched_wfb;
        mcd212->region_flag_0[x] = latched_rf0;
        mcd212->region_flag_1[x] = latched_rf1;
    }
}

static void mcd212_set_vsr(mcd212_regs_t *mcd212, int channel, UINT32 value)
{
    mcd212->channel[channel].vsr = value & 0x0000ffff;
    mcd212->channel[channel].dcr &= 0xffc0;
    mcd212->channel[channel].dcr |= (value >> 16) & 0x003f;
}

static void mcd212_set_register(running_machine &machine, int channel, UINT8 reg, UINT32 value)
{
    cdi_state *state = machine.driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->m_mcd212_regs;

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
            verboselog(machine, 11, "          %04xxxxx: %d: CLUT[%d] = %08x\n", channel * 0x20, channel, mcd212->channel[channel].clut_bank * 0x40 + (reg - 0x80), value );
            mcd212->channel[0].clut_r[mcd212->channel[channel].clut_bank * 0x40 + (reg - 0x80)] = (UINT8)(value >> 16) & 0xfc;
            mcd212->channel[0].clut_g[mcd212->channel[channel].clut_bank * 0x40 + (reg - 0x80)] = (UINT8)(value >>  8) & 0xfc;
            mcd212->channel[0].clut_b[mcd212->channel[channel].clut_bank * 0x40 + (reg - 0x80)] = (UINT8)(value >>  0) & 0xfc;
            break;
        case 0xc0: // Image Coding Method
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Image Coding Method = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].image_coding_method = value;
            }
            break;
        case 0xc1: // Transparency Control
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Transparency Control = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].transparency_control = value;
            }
            break;
        case 0xc2: // Plane Order
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Plane Order = %08x\n", channel * 0x20, channel, value & 7);
                mcd212->channel[channel].plane_order = value & 0x00000007;
            }
            break;
        case 0xc3: // CLUT Bank Register
            verboselog(machine, 6, "          %04xxxxx: %d: CLUT Bank Register = %08x\n", channel * 0x20, channel, value & 3);
            mcd212->channel[channel].clut_bank = channel ? (2 | (value & 0x00000001)) : (value & 0x00000003);
            break;
        case 0xc4: // Transparent Color A
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Transparent Color A = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].transparent_color_a = value & 0xfcfcfc;
            }
            break;
        case 0xc6: // Transparent Color B
            if(channel == 1)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Transparent Color B = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].transparent_color_b = value & 0xfcfcfc;
            }
            break;
        case 0xc7: // Mask Color A
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Mask Color A = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].mask_color_a = value & 0xfcfcfc;
            }
            break;
        case 0xc9: // Mask Color B
            if(channel == 1)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Mask Color B = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].mask_color_b = value & 0xfcfcfc;
            }
            break;
        case 0xca: // Delta YUV Absolute Start Value A
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Delta YUV Absolute Start Value A = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].dyuv_abs_start_a = value;
            }
            break;
        case 0xcb: // Delta YUV Absolute Start Value B
            if(channel == 1)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Delta YUV Absolute Start Value B = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].dyuv_abs_start_b = value;
            }
            break;
        case 0xcd: // Cursor Position
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Cursor Position = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].cursor_position = value;
            }
            break;
        case 0xce: // Cursor Control
            if(channel == 0)
            {
                verboselog(machine, 11, "          %04xxxxx: %d: Cursor Control = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].cursor_control = value;
            }
            break;
        case 0xcf: // Cursor Pattern
            if(channel == 0)
            {
                verboselog(machine, 11, "          %04xxxxx: %d: Cursor Pattern[%d] = %04x\n", channel * 0x20, channel, (value >> 16) & 0x000f, value & 0x0000ffff);
                mcd212->channel[channel].cursor_pattern[(value >> 16) & 0x000f] = value & 0x0000ffff;
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
            verboselog(machine, 6, "          %04xxxxx: %d: Region Control %d = %08x\n", channel * 0x20, channel, reg & 7, value );
            mcd212->channel[0].region_control[reg & 7] = value;
            mcd212_update_region_arrays(mcd212);
            break;
        case 0xd8: // Backdrop Color
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Backdrop Color = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].backdrop_color = value;
            }
            break;
        case 0xd9: // Mosaic Pixel Hold Factor A
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Mosaic Pixel Hold Factor A = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].mosaic_hold_a = value;
            }
            break;
        case 0xda: // Mosaic Pixel Hold Factor B
            if(channel == 1)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Mosaic Pixel Hold Factor B = %08x\n", channel * 0x20, channel, value );
                mcd212->channel[channel].mosaic_hold_b = value;
            }
            break;
        case 0xdb: // Weight Factor A
            if(channel == 0)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Weight Factor A = %08x\n", channel * 0x20, channel, value );
                memset(mcd212->channel[channel].weight_factor_a, value & 0x000000ff, 768);
                mcd212_update_region_arrays(mcd212);
            }
            break;
        case 0xdc: // Weight Factor B
            if(channel == 1)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Weight Factor B = %08x\n", channel * 0x20, channel, value );
                memset(mcd212->channel[channel].weight_factor_b, value & 0x000000ff, 768);
                mcd212_update_region_arrays(mcd212);
            }
            break;
    }
}

static UINT32 mcd212_get_vsr(mcd212_regs_t *mcd212, int channel)
{
    return ((mcd212->channel[channel].dcr & 0x3f) << 16) | mcd212->channel[channel].vsr;
}

static void mcd212_set_dcp(mcd212_regs_t *mcd212, int channel, UINT32 value)
{
    mcd212->channel[channel].dcp = value & 0x0000ffff;
    mcd212->channel[channel].ddr &= 0xffc0;
    mcd212->channel[channel].ddr |= (value >> 16) & 0x003f;
}

static UINT32 mcd212_get_dcp(mcd212_regs_t *mcd212, int channel)
{
    return ((mcd212->channel[channel].ddr & 0x3f) << 16) | mcd212->channel[channel].dcp;
}

static void mcd212_set_display_parameters(mcd212_regs_t *mcd212, int channel, UINT8 value)
{
    mcd212->channel[channel].ddr &= 0xf0ff;
    mcd212->channel[channel].ddr |= (value & 0x0f) << 8;
    mcd212->channel[channel].dcr &= 0xf7ff;
    mcd212->channel[channel].dcr |= (value & 0x10) << 7;
}

static void mcd212_update_visible_area(running_machine &machine)
{
    cdi_state *state = machine.driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->m_mcd212_regs;
    const rectangle &visarea = machine.primary_screen->visible_area();
    rectangle visarea1;
    attoseconds_t period = machine.primary_screen->frame_period().attoseconds;
    int width = 0;

    if((mcd212->channel[0].dcr & (MCD212_DCR_CF | MCD212_DCR_FD)) && (mcd212->channel[0].csrw & MCD212_CSR1W_ST))
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

    machine.primary_screen->configure(width, 302, visarea1, period);
}

static UINT32 mcd212_get_screen_width(mcd212_regs_t *mcd212)
{
    if((mcd212->channel[0].dcr & (MCD212_DCR_CF | MCD212_DCR_FD)) && (mcd212->channel[0].csrw & MCD212_CSR1W_ST))
    {
        return 720;
    }
    return 768;
}

static void mcd212_process_ica(mcd212_regs_t *mcd212, int channel)
{
    running_machine &machine = mcd212->machine();
    cdi_state *state = machine.driver_data<cdi_state>();
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
                verboselog(machine, 11, "%08x: %08x: ICA %d: STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                stop = 1;
                break;
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // NOP
            case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
                verboselog(machine, 12, "%08x: %08x: ICA %d: NOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                break;
            case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // RELOAD DCP
            case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
                verboselog(machine, 11, "%08x: %08x: ICA %d: RELOAD DCP\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_dcp(mcd212, channel, cmd & 0x001fffff);
                break;
            case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // RELOAD DCP and STOP
            case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
                verboselog(machine, 11, "%08x: %08x: ICA %d: RELOAD DCP and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_dcp(mcd212, channel, cmd & 0x001fffff);
                stop = 1;
                break;
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD ICA
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
                verboselog(machine, 11, "%08x: %08x: ICA %d: RELOAD ICA\n", addr * 2 + channel * 0x200000, cmd, channel );
                addr = (cmd & 0x001fffff) / 2;
                break;
            case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
            case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
                verboselog(machine, 11, "%08x: %08x: ICA %d: RELOAD VSR and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_vsr(mcd212, channel, cmd & 0x001fffff);
                stop = 1;
                break;
            case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
            case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
                verboselog(machine, 11, "%08x: %08x: ICA %d: INTERRUPT\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212->channel[1].csrr |= 1 << (2 - channel);
                if(mcd212->channel[1].csrr & (MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2))
                {
                    UINT8 interrupt = (state->m_scc68070_regs.lir >> 4) & 7;
                    if(interrupt)
                    {
                        machine.device("maincpu")->execute().set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
                        machine.device("maincpu")->execute().set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#if 0
                if(mcd212->channel[1].csrr & MCD212_CSR2R_IT2)
                {
                    UINT8 interrupt = state->m_scc68070_regs.lir & 7;
                    if(interrupt)
                    {
                        machine.device("maincpu")->execute().set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 24 + interrupt);
                        machine.device("maincpu")->execute().set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#endif
                break;
            case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
                verboselog(machine, 6, "%08x: %08x: ICA %d: RELOAD DISPLAY PARAMETERS\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_display_parameters(mcd212, channel, cmd & 0x1f);
                break;
            default:
                mcd212_set_register(machine, channel, cmd >> 24, cmd & 0x00ffffff);
                break;
        }
        if(stop)
        {
            break;
        }
    }
}

static void mcd212_process_dca(mcd212_regs_t *mcd212, int channel)
{
    running_machine &machine = mcd212->machine();
    cdi_state *state = machine.driver_data<cdi_state>();
    UINT16 *dca = channel ? state->m_planeb : state->m_planea;
    UINT32 addr = (mcd212->channel[channel].dca & 0x0007ffff) / 2; //(mcd212_get_dcp(mcd212, channel) & 0x0007ffff) / 2; // mcd212->channel[channel].dca / 2;
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
                verboselog(machine, 11, "%08x: %08x: DCA %d: STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                stop = 1;
                break;
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // NOP
            case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
                verboselog(machine, 12, "%08x: %08x: DCA %d: NOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                break;
            case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // RELOAD DCP
            case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: RELOAD DCP (NOP)\n", addr * 2 + channel * 0x200000, cmd, channel );
                break;
            case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // RELOAD DCP and STOP
            case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: RELOAD DCP and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_dcp(&state->m_mcd212_regs, channel, cmd & 0x001fffff);
                addr = (cmd & 0x0007ffff) / 2;
                addr_changed = 1;
                stop = 1;
                break;
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD VSR
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: RELOAD VSR\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_vsr(&state->m_mcd212_regs, channel, cmd & 0x001fffff);
                break;
            case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
            case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: RELOAD VSR and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_vsr(&state->m_mcd212_regs, channel, cmd & 0x001fffff);
                stop = 1;
                break;
            case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
            case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: INTERRUPT\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212->channel[1].csrr |= 1 << (2 - channel);
                if(mcd212->channel[1].csrr & (MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2))
                {
                    UINT8 interrupt = (state->m_scc68070_regs.lir >> 4) & 7;
                    if(interrupt)
                    {
                        machine.device("maincpu")->execute().set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
                        machine.device("maincpu")->execute().set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#if 0
                if(mcd212->channel[1].csrr & MCD212_CSR2R_IT2)
                {
                    UINT8 interrupt = state->m_scc68070_regs.lir & 7;
                    if(interrupt)
                    {
                        machine.device("maincpu")->execute().set_input_line_vector(M68K_IRQ_1 + (interrupt - 1), 24 + interrupt);
                        machine.device("maincpu")->execute().set_input_line(M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#endif
                break;
            case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
                verboselog(machine, 6, "%08x: %08x: DCA %d: RELOAD DISPLAY PARAMETERS\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_display_parameters(&state->m_mcd212_regs, channel, cmd & 0x1f);
                break;
            default:
                mcd212_set_register(machine, channel, cmd >> 24, cmd & 0x00ffffff);
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
    mcd212->channel[channel].dca = addr * 2;
}

INLINE UINT8 MCD212_LIM(INT32 in)
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

INLINE UINT8 BYTE_TO_CLUT(int channel, int icm, UINT8 byte)
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

static void mcd212_process_vsr(mcd212_regs_t *mcd212, int channel, UINT8 *pixels_r, UINT8 *pixels_g, UINT8 *pixels_b)
{
    running_machine &machine = mcd212->machine();
    cdi_state *state = machine.driver_data<cdi_state>();
    UINT8 *data = reinterpret_cast<UINT8 *>(channel ? state->m_planeb.target() : state->m_planea.target());
    UINT32 vsr = mcd212_get_vsr(mcd212, channel) & 0x0007ffff;
    UINT8 done = 0;
    UINT32 x = 0;
    UINT32 icm_mask = channel ? MCD212_ICM_MODE2 : MCD212_ICM_MODE1;
    UINT32 icm_shift = channel ? MCD212_ICM_MODE2_SHIFT : MCD212_ICM_MODE1_SHIFT;
    UINT8 icm = (mcd212->channel[0].image_coding_method & icm_mask) >> icm_shift;
    UINT8 *clut_r = mcd212->channel[0].clut_r;
    UINT8 *clut_g = mcd212->channel[0].clut_g;
    UINT8 *clut_b = mcd212->channel[0].clut_b;
    UINT8 mosaic_enable = ((mcd212->channel[channel].ddr & MCD212_DDR_FT) == MCD212_DDR_FT_MOSAIC);
    UINT8 mosaic_factor = 1 << (((mcd212->channel[channel].ddr & MCD212_DDR_MT) >> MCD212_DDR_MT_SHIFT) + 1);
    int mosaic_index = 0;
    UINT32 width = mcd212_get_screen_width(mcd212);

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
        switch(mcd212->channel[channel].ddr & MCD212_DDR_FT)
        {
            case MCD212_DDR_FT_BMP:
            case MCD212_DDR_FT_BMP2:
            case MCD212_DDR_FT_MOSAIC:
                if(mcd212->channel[channel].dcr & MCD212_DCR_CM)
                {
                    // 4-bit Bitmap
                    verboselog(machine, 0, "Unsupported display mode: 4-bit Bitmap\n" );
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
                                bY = (mcd212->channel[0].dyuv_abs_start_a >> 16) & 0x000000ff;
                                bU = (mcd212->channel[0].dyuv_abs_start_a >>  8) & 0x000000ff;
                                bV = (mcd212->channel[0].dyuv_abs_start_a >>  0) & 0x000000ff;
                                break;
                            case 1:
                                bY = (mcd212->channel[1].dyuv_abs_start_b >> 16) & 0x000000ff;
                                bU = (mcd212->channel[1].dyuv_abs_start_b >>  8) & 0x000000ff;
                                bV = (mcd212->channel[1].dyuv_abs_start_b >>  0) & 0x000000ff;
                                break;
                            default:
                                bY = bU = bV = 0x80;
                                break;
                        }
                        for(; x < width; x += 2)
                        {
                            BYTE68K b0 = byte;
                            BYTE68K bU1 = bU + state->m_mcd212_ab.deltaUV[b0];
                            BYTE68K bY0 = bY + state->m_mcd212_ab.deltaY[b0];

                            BYTE68K b1 = data[(vsr & 0x0007ffff) ^ 1];
                            BYTE68K bV1 = bV + state->m_mcd212_ab.deltaUV[b1];
                            BYTE68K bY1 = bY0 + state->m_mcd212_ab.deltaY[b1];

                            BYTE68K bU0 = (bU + bU1) >> 1;
                            BYTE68K bV0 = (bV + bV1) >> 1;

                            BYTE68K *pbLimit;

                            vsr++;

                            bY = bY0;
                            bU = bU0;
                            bV = bV0;

                            pbLimit = state->m_mcd212_ab.limit + bY + BYTE68K_MAX;

                            pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[state->m_mcd212_ab.matrixVR[bV]];
                            pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[state->m_mcd212_ab.matrixUG[bU] + state->m_mcd212_ab.matrixVG[bV]];
                            pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[state->m_mcd212_ab.matrixUB[bU]];

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

                            pbLimit = state->m_mcd212_ab.limit + bY + BYTE68K_MAX;

                            pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[state->m_mcd212_ab.matrixVR[bV]];
                            pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[state->m_mcd212_ab.matrixUG[bU] + state->m_mcd212_ab.matrixVG[bV]];
                            pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[state->m_mcd212_ab.matrixUB[bU]];

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
                        mcd212_set_vsr(&state->m_mcd212_regs, channel, (vsr - 1) & 0x0007ffff);
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
                        mcd212_set_vsr(&state->m_mcd212_regs, channel, (vsr - 1) & 0x0007ffff);
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
                        mcd212_set_vsr(&state->m_mcd212_regs, channel, (vsr - 1) & 0x0007ffff);
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
                if(mcd212->channel[channel].dcr & MCD212_DCR_CM)
                {
                    verboselog(machine, 0, "Unsupported display mode: 4-bit RLE\n" );
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
                            mcd212_set_vsr(&state->m_mcd212_regs, channel, vsr);
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
                                mcd212_set_vsr(&state->m_mcd212_regs, channel, vsr);
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
                            mcd212_set_vsr(&state->m_mcd212_regs, channel, vsr);
                        }
                    }
                }
                break;
        }
    }

    //printf( ": vsr after: %08x\n", vsr);
    //mcd212_set_vsr(&state->m_mcd212_regs, channel, vsr);
}

static const UINT32 mcd212_4bpp_color[16] =
{
    0x00101010, 0x0010107a, 0x00107a10, 0x00107a7a, 0x007a1010, 0x007a107a, 0x007a7a10, 0x007a7a7a,
    0x00101010, 0x001010e6, 0x0010e610, 0x0010e6e6, 0x00e61010, 0x00e610e6, 0x00e6e610, 0x00e6e6e6
};

static void mcd212_mix_lines(mcd212_regs_t *mcd212, UINT8 *plane_a_r, UINT8 *plane_a_g, UINT8 *plane_a_b, UINT8 *plane_b_r, UINT8 *plane_b_g, UINT8 *plane_b_b, UINT32 *out)
{
    running_machine &machine = mcd212->machine();
    int x = 0;
    UINT8 debug_mode = machine.root_device().ioport("DEBUG")->read();
    UINT8 global_plane_a_disable = debug_mode & 1;
    UINT8 global_plane_b_disable = debug_mode & 2;
    UINT8 debug_backdrop_enable = debug_mode & 4;
    UINT8 debug_backdrop_index = debug_mode >> 4;
    UINT32 backdrop = debug_backdrop_enable ? mcd212_4bpp_color[debug_backdrop_index] : mcd212_4bpp_color[mcd212->channel[0].backdrop_color];
    UINT8 transparency_mode_a = (mcd212->channel[0].transparency_control >> 0) & 0x0f;
    UINT8 transparency_mode_b = (mcd212->channel[0].transparency_control >> 8) & 0x0f;
    UINT8 transparent_color_a_r = (UINT8)(mcd212->channel[0].transparent_color_a >> 16);
    UINT8 transparent_color_a_g = (UINT8)(mcd212->channel[0].transparent_color_a >>  8);
    UINT8 transparent_color_a_b = (UINT8)(mcd212->channel[0].transparent_color_a >>  0);
    UINT8 transparent_color_b_r = (UINT8)(mcd212->channel[1].transparent_color_b >> 16);
    UINT8 transparent_color_b_g = (UINT8)(mcd212->channel[1].transparent_color_b >>  8);
    UINT8 transparent_color_b_b = (UINT8)(mcd212->channel[1].transparent_color_b >>  0);
    UINT8 image_coding_method_a = mcd212->channel[0].image_coding_method & 0x0000000f;
    UINT8 image_coding_method_b = (mcd212->channel[0].image_coding_method >> 8) & 0x0000000f;
    UINT8 dyuv_enable_a = (image_coding_method_a == 5);
    UINT8 dyuv_enable_b = (image_coding_method_b == 5);
    UINT8 mosaic_enable_a = (mcd212->channel[0].mosaic_hold_a & 0x800000) >> 23;
    UINT8 mosaic_enable_b = (mcd212->channel[1].mosaic_hold_b & 0x800000) >> 23;
    UINT8 mosaic_count_a = (mcd212->channel[0].mosaic_hold_a & 0x0000ff) << 1;
    UINT8 mosaic_count_b = (mcd212->channel[1].mosaic_hold_b & 0x0000ff) << 1;
    for(x = 0; x < 768; x++)
    {
        out[x] = backdrop;
        if(!(mcd212->channel[0].transparency_control & MCD212_TCR_DISABLE_MX))
        {
            UINT8 abr = MCD212_LIM(((MCD212_LIM((INT32)plane_a_r[x] - 16) * mcd212->channel[0].weight_factor_a[x]) >> 6) + ((MCD212_LIM((INT32)plane_b_r[x] - 16) * mcd212->channel[1].weight_factor_b[x]) >> 6) + 16);
            UINT8 abg = MCD212_LIM(((MCD212_LIM((INT32)plane_a_g[x] - 16) * mcd212->channel[0].weight_factor_a[x]) >> 6) + ((MCD212_LIM((INT32)plane_b_g[x] - 16) * mcd212->channel[1].weight_factor_b[x]) >> 6) + 16);
            UINT8 abb = MCD212_LIM(((MCD212_LIM((INT32)plane_a_b[x] - 16) * mcd212->channel[0].weight_factor_a[x]) >> 6) + ((MCD212_LIM((INT32)plane_b_b[x] - 16) * mcd212->channel[1].weight_factor_b[x]) >> 6) + 16);
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
                    plane_enable_a = !mcd212->region_flag_0[x];
                    break;
                case 4:
                    plane_enable_a = !mcd212->region_flag_1[x];
                    break;
                case 5:
                    plane_enable_a = (plane_a_r_cur != transparent_color_a_r || plane_a_g_cur != transparent_color_a_g || plane_a_b_cur != transparent_color_a_b) && (dyuv_enable_a || mcd212->region_flag_0[x] == 0);
                    break;
                case 6:
                    plane_enable_a = (plane_a_r_cur != transparent_color_a_r || plane_a_g_cur != transparent_color_a_g || plane_a_b_cur != transparent_color_a_b) && (dyuv_enable_a || mcd212->region_flag_1[x] == 0);
                    break;
                case 8:
                    plane_enable_a = 1;
                    break;
                case 9:
                    plane_enable_a = (plane_a_r_cur == transparent_color_a_r && plane_a_g_cur == transparent_color_a_g && plane_a_b_cur == transparent_color_a_b);
                    break;
                case 11:
                    plane_enable_a = mcd212->region_flag_0[x];
                    break;
                case 12:
                    plane_enable_a = mcd212->region_flag_1[x];
                    break;
                case 13:
                    plane_enable_a = (plane_a_r_cur == transparent_color_a_r && plane_a_g_cur == transparent_color_a_g && plane_a_b_cur == transparent_color_a_b) || dyuv_enable_a || mcd212->region_flag_0[x] == 1;
                    break;
                case 14:
                    plane_enable_a = (plane_a_r_cur == transparent_color_a_r && plane_a_g_cur == transparent_color_a_g && plane_a_b_cur == transparent_color_a_b) || dyuv_enable_a || mcd212->region_flag_1[x] == 1;
                    break;
                default:
                    verboselog(machine, 0, "Unhandled transparency mode for plane A: %d\n", transparency_mode_a);
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
                    plane_enable_b = !mcd212->region_flag_0[x];
                    break;
                case 4:
                    plane_enable_b = !mcd212->region_flag_1[x];
                    break;
                case 5:
                    plane_enable_b = (plane_b_r_cur != transparent_color_b_r || plane_b_g_cur != transparent_color_b_g || plane_b_b_cur != transparent_color_b_b) && (dyuv_enable_b || mcd212->region_flag_0[x] == 0);
                    break;
                case 6:
                    plane_enable_b = (plane_b_r_cur != transparent_color_b_r || plane_b_g_cur != transparent_color_b_g || plane_b_b_cur != transparent_color_b_b) && (dyuv_enable_b || mcd212->region_flag_1[x] == 0);
                    break;
                case 8:
                    plane_enable_b = 1;
                    break;
                case 9:
                    plane_enable_b = (plane_b_r_cur == transparent_color_b_r && plane_b_g_cur == transparent_color_b_g && plane_b_b_cur == transparent_color_b_b);
                    break;
                case 11:
                    plane_enable_b = mcd212->region_flag_0[x];
                    break;
                case 12:
                    plane_enable_b = mcd212->region_flag_1[x];
                    break;
                case 13:
                    plane_enable_b = (plane_b_r_cur == transparent_color_b_r && plane_b_g_cur == transparent_color_b_g && plane_b_b_cur == transparent_color_b_b) || dyuv_enable_b || mcd212->region_flag_0[x] == 1;
                    break;
                case 14:
                    plane_enable_b = (plane_b_r_cur == transparent_color_b_r && plane_b_g_cur == transparent_color_b_g && plane_b_b_cur == transparent_color_b_b) || dyuv_enable_b || mcd212->region_flag_1[x] == 1;
                    break;
                default:
                    verboselog(machine, 0, "Unhandled transparency mode for plane B: %d\n", transparency_mode_b);
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
            plane_a_r_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_a_r_cur - 16) * mcd212->channel[0].weight_factor_a[x]) >> 6) + 16);
            plane_a_g_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_a_g_cur - 16) * mcd212->channel[0].weight_factor_a[x]) >> 6) + 16);
            plane_a_b_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_a_b_cur - 16) * mcd212->channel[0].weight_factor_a[x]) >> 6) + 16);
            plane_b_r_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_b_r_cur - 16) * mcd212->channel[1].weight_factor_b[x]) >> 6) + 16);
            plane_b_g_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_b_g_cur - 16) * mcd212->channel[1].weight_factor_b[x]) >> 6) + 16);
            plane_b_b_cur = MCD212_LIM(((MCD212_LIM((INT32)plane_b_b_cur - 16) * mcd212->channel[1].weight_factor_b[x]) >> 6) + 16);
            switch(mcd212->channel[0].plane_order)
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

static void mcd212_draw_cursor(mcd212_regs_t *mcd212, UINT32 *scanline, int y)
{
    if(mcd212->channel[0].cursor_control & MCD212_CURCNT_EN)
    {
        UINT16 curx =  mcd212->channel[0].cursor_position        & 0x3ff;
        UINT16 cury = ((mcd212->channel[0].cursor_position >> 12) & 0x3ff) + 22;
        UINT32 x = 0;
        if(y >= cury && y < (cury + 16))
        {
            UINT32 color = mcd212_4bpp_color[mcd212->channel[0].cursor_control & MCD212_CURCNT_COLOR];
            y -= cury;
            if(mcd212->channel[0].cursor_control & MCD212_CURCNT_CUW)
            {
                for(x = curx; x < curx + 64 && x < 768; x++)
                {
                    if(mcd212->channel[0].cursor_pattern[y] & (1 << (15 - ((x - curx) >> 2))))
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
                for(x = curx; x < curx + 32 && x < 768; x++)
                {
                    if(mcd212->channel[0].cursor_pattern[y] & (1 << (15 - ((x - curx) >> 1))))
                    {
                        scanline[(x++)/2] = color;
                        scanline[x/2] = color;
                    }
                    else
                    {
                    }
                }
            }
        }
    }
}

static void mcd212_draw_scanline(mcd212_regs_t *mcd212, int y)
{
    bitmap_rgb32 &bitmap = mcd212->m_bitmap;
    UINT8 plane_a_r[768], plane_a_g[768], plane_a_b[768];
    UINT8 plane_b_r[768], plane_b_g[768], plane_b_b[768];
    UINT32 out[768];
    UINT32 *scanline = &bitmap.pix32(y);
    int x;

    mcd212_process_vsr(mcd212, 0, plane_a_r, plane_a_g, plane_a_b);
    mcd212_process_vsr(mcd212, 1, plane_b_r, plane_b_g, plane_b_b);

    mcd212_mix_lines(mcd212, plane_a_r, plane_a_g, plane_a_b, plane_b_r, plane_b_g, plane_b_b, out);

    for(x = 0; x < 384; x++)
    {
        scanline[x] = out[x*2];
    }

    mcd212_draw_cursor(mcd212, scanline, y);
}

READ16_HANDLER( mcd212_r )
{
    cdi_state *state = space.machine().driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->m_mcd212_regs;
    UINT8 channel = 1 - (offset / 8);

    switch(offset)
    {
        case 0x00/2:
        case 0x10/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space.machine(), 12, "mcd212_r: Status Register %d: %02x & %04x\n", channel + 1, mcd212->channel[1 - (offset / 8)].csrr, mem_mask);
                if(channel == 0)
                {
                    return mcd212->channel[0].csrr;
                }
                else
                {
                    UINT8 old_csr = mcd212->channel[1].csrr;
                    UINT8 interrupt1 = (state->m_scc68070_regs.lir >> 4) & 7;
                    //UINT8 interrupt2 = state->m_scc68070_regs.lir & 7;
                    mcd212->channel[1].csrr &= ~(MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2);
                    if(interrupt1)
                    {
                        space.machine().device("maincpu")->execute().set_input_line(M68K_IRQ_1 + (interrupt1 - 1), CLEAR_LINE);
                    }
                    //if(interrupt2)
                    //{
                    //  space.machine().device("maincpu")->execute().set_input_line(M68K_IRQ_1 + (interrupt2 - 1), CLEAR_LINE);
                    //}
                    return old_csr;
                }
            }
            else
            {
                verboselog(space.machine(), 2, "mcd212_r: Unknown Register %d: %04x\n", channel + 1, mem_mask);
            }
            break;
        case 0x02/2:
        case 0x12/2:
            verboselog(space.machine(), 2, "mcd212_r: Display Command Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].dcr, mem_mask);
            return mcd212->channel[1 - (offset / 8)].dcr;
        case 0x04/2:
        case 0x14/2:
            verboselog(space.machine(), 2, "mcd212_r: Video Start Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].vsr, mem_mask);
            return mcd212->channel[1 - (offset / 8)].vsr;
        case 0x08/2:
        case 0x18/2:
            verboselog(space.machine(), 2, "mcd212_r: Display Decoder Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].ddr, mem_mask);
            return mcd212->channel[1 - (offset / 8)].ddr;
        case 0x0a/2:
        case 0x1a/2:
            verboselog(space.machine(), 2, "mcd212_r: DCA Pointer Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].dcp, mem_mask);
            return mcd212->channel[1 - (offset / 8)].dcp;
        default:
            verboselog(space.machine(), 2, "mcd212_r: Unknown Register %d & %04x\n", (1 - (offset / 8)) + 1, mem_mask);
            break;
    }

    return 0;
}

WRITE16_HANDLER( mcd212_w )
{
    cdi_state *state = space.machine().driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->m_mcd212_regs;

    switch(offset)
    {
        case 0x00/2:
        case 0x10/2:
            verboselog(space.machine(), 2, "mcd212_w: Status Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].csrw);
            mcd212_update_visible_area(space.machine());
            break;
        case 0x02/2:
        case 0x12/2:
            verboselog(space.machine(), 2, "mcd212_w: Display Command Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].dcr);
            mcd212_update_visible_area(space.machine());
            break;
        case 0x04/2:
        case 0x14/2:
            verboselog(space.machine(), 2, "mcd212_w: Video Start Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].vsr);
            break;
        case 0x08/2:
        case 0x18/2:
            verboselog(space.machine(), 2, "mcd212_w: Display Decoder Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].ddr);
            break;
        case 0x0a/2:
        case 0x1a/2:
            verboselog(space.machine(), 2, "mcd212_w: DCA Pointer Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].dcp);
            break;
        default:
            verboselog(space.machine(), 2, "mcd212_w: Unknown Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            break;
    }
}

TIMER_CALLBACK( mcd212_perform_scan )
{
    cdi_state *state = machine.driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->m_mcd212_regs;
    int scanline = machine.primary_screen->vpos();

    if(/*mcd212->channel[0].dcr & MCD212_DCR_DE*/1)
    {
        if(scanline == 0)
        {
            // Process ICA
            int index = 0;
            verboselog(machine, 6, "Frame Start\n" );
            mcd212->channel[0].csrr &= 0x7f;
            for(index = 0; index < 2; index++)
            {
                if(mcd212->channel[index].dcr & MCD212_DCR_ICA)
                {
                    mcd212_process_ica(mcd212, index);
                }
            }
            cdi220_draw_lcd(machine, scanline);
        }
        else if(scanline < 22)
        {
            cdi220_draw_lcd(machine, scanline);
        }
        else if(scanline >= 22)
        {
            int index = 0;
            mcd212->channel[0].csrr |= 0x80;
            // Process VSR
            mcd212_draw_scanline(mcd212, scanline);
            // Process DCA
            for(index = 0; index < 2; index++)
            {
                if(mcd212->channel[index].dcr & MCD212_DCR_DCA)
                {
                    if(scanline == 22)
                    {
                        mcd212->channel[index].dca = mcd212_get_dcp(mcd212, index);
                    }
                    mcd212_process_dca(mcd212, index);
                }
            }
            if(scanline == 301)
            {
                mcd212->channel[0].csrr ^= 0x20;
            }
        }
    }
    mcd212->scan_timer->adjust(machine.primary_screen->time_until_pos(( scanline + 1 ) % 302, 0));
}

void mcd212_init(running_machine &machine, mcd212_regs_t *mcd212)
{
    mcd212->m_machine = &machine;
    machine.primary_screen->register_screen_bitmap(mcd212->m_bitmap);

    int index = 0;
    for(index = 0; index < 2; index++)
    {
        mcd212->channel[index].csrr = 0;
        mcd212->channel[index].csrw = 0;
        mcd212->channel[index].dcr = 0;
        mcd212->channel[index].vsr = 0;
        mcd212->channel[index].ddr = 0;
        mcd212->channel[index].dcp = 0;
        mcd212->channel[index].dca = 0;
        memset(mcd212->channel[index].clut_r, 0, 256);
        memset(mcd212->channel[index].clut_g, 0, 256);
        memset(mcd212->channel[index].clut_b, 0, 256);
        mcd212->channel[index].image_coding_method = 0;
        mcd212->channel[index].transparency_control = 0;
        mcd212->channel[index].plane_order = 0;
        mcd212->channel[index].clut_bank = 0;
        mcd212->channel[index].transparent_color_a = 0;
        mcd212->channel[index].transparent_color_b = 0;
        mcd212->channel[index].mask_color_a = 0;
        mcd212->channel[index].mask_color_b = 0;
        mcd212->channel[index].dyuv_abs_start_a = 0;
        mcd212->channel[index].dyuv_abs_start_b = 0;
        mcd212->channel[index].cursor_position = 0;
        mcd212->channel[index].cursor_control = 0;
        memset((UINT8*)&mcd212->channel[index].cursor_pattern, 0, 16 * sizeof(UINT32));
        memset((UINT8*)&mcd212->channel[index].region_control, 0, 8 * sizeof(UINT32));
        mcd212->channel[index].backdrop_color = 0;
        mcd212->channel[index].mosaic_hold_a = 0;
        mcd212->channel[index].mosaic_hold_b = 0;
        memset(mcd212->channel[index].weight_factor_a, 0, 768);
        memset(mcd212->channel[index].weight_factor_b, 0, 768);
    }
    memset(mcd212->region_flag_0, 0, 768);
    memset(mcd212->region_flag_1, 0, 768);

    state_save_register_global_array(machine, mcd212->region_flag_0);
    state_save_register_global_array(machine, mcd212->region_flag_1);
    state_save_register_global(machine, mcd212->channel[0].csrr);
    state_save_register_global(machine, mcd212->channel[0].csrw);
    state_save_register_global(machine, mcd212->channel[0].dcr);
    state_save_register_global(machine, mcd212->channel[0].vsr);
    state_save_register_global(machine, mcd212->channel[0].ddr);
    state_save_register_global(machine, mcd212->channel[0].dcp);
    state_save_register_global(machine, mcd212->channel[0].dca);
    state_save_register_global_array(machine, mcd212->channel[0].clut_r);
    state_save_register_global_array(machine, mcd212->channel[0].clut_g);
    state_save_register_global_array(machine, mcd212->channel[0].clut_b);
    state_save_register_global(machine, mcd212->channel[0].image_coding_method);
    state_save_register_global(machine, mcd212->channel[0].transparency_control);
    state_save_register_global(machine, mcd212->channel[0].plane_order);
    state_save_register_global(machine, mcd212->channel[0].clut_bank);
    state_save_register_global(machine, mcd212->channel[0].transparent_color_a);
    state_save_register_global(machine, mcd212->channel[0].transparent_color_b);
    state_save_register_global(machine, mcd212->channel[0].mask_color_a);
    state_save_register_global(machine, mcd212->channel[0].mask_color_b);
    state_save_register_global(machine, mcd212->channel[0].dyuv_abs_start_a);
    state_save_register_global(machine, mcd212->channel[0].dyuv_abs_start_b);
    state_save_register_global(machine, mcd212->channel[0].cursor_position);
    state_save_register_global(machine, mcd212->channel[0].cursor_control);
    state_save_register_global_array(machine, mcd212->channel[0].cursor_pattern);
    state_save_register_global_array(machine, mcd212->channel[0].region_control);
    state_save_register_global(machine, mcd212->channel[0].backdrop_color);
    state_save_register_global(machine, mcd212->channel[0].mosaic_hold_a);
    state_save_register_global(machine, mcd212->channel[0].mosaic_hold_b);
    state_save_register_global_array(machine, mcd212->channel[0].weight_factor_a);
    state_save_register_global_array(machine, mcd212->channel[0].weight_factor_b);
    state_save_register_global(machine, mcd212->channel[1].csrr);
    state_save_register_global(machine, mcd212->channel[1].csrw);
    state_save_register_global(machine, mcd212->channel[1].dcr);
    state_save_register_global(machine, mcd212->channel[1].vsr);
    state_save_register_global(machine, mcd212->channel[1].ddr);
    state_save_register_global(machine, mcd212->channel[1].dcp);
    state_save_register_global(machine, mcd212->channel[1].dca);
    state_save_register_global_array(machine, mcd212->channel[1].clut_r);
    state_save_register_global_array(machine, mcd212->channel[1].clut_g);
    state_save_register_global_array(machine, mcd212->channel[1].clut_b);
    state_save_register_global(machine, mcd212->channel[1].image_coding_method);
    state_save_register_global(machine, mcd212->channel[1].transparency_control);
    state_save_register_global(machine, mcd212->channel[1].plane_order);
    state_save_register_global(machine, mcd212->channel[1].clut_bank);
    state_save_register_global(machine, mcd212->channel[1].transparent_color_a);
    state_save_register_global(machine, mcd212->channel[1].transparent_color_b);
    state_save_register_global(machine, mcd212->channel[1].mask_color_a);
    state_save_register_global(machine, mcd212->channel[1].mask_color_b);
    state_save_register_global(machine, mcd212->channel[1].dyuv_abs_start_a);
    state_save_register_global(machine, mcd212->channel[1].dyuv_abs_start_b);
    state_save_register_global(machine, mcd212->channel[1].cursor_position);
    state_save_register_global(machine, mcd212->channel[1].cursor_control);
    state_save_register_global_array(machine, mcd212->channel[1].cursor_pattern);
    state_save_register_global_array(machine, mcd212->channel[1].region_control);
    state_save_register_global(machine, mcd212->channel[1].backdrop_color);
    state_save_register_global(machine, mcd212->channel[1].mosaic_hold_a);
    state_save_register_global(machine, mcd212->channel[1].mosaic_hold_b);
    state_save_register_global_array(machine, mcd212->channel[1].weight_factor_a);
    state_save_register_global_array(machine, mcd212->channel[1].weight_factor_b);
}

void mcd212_ab_init(mcd212_ab_t *mcd212_ab)
{
    WORD68K w = 0;
    SWORD68K sw = 0;
    WORD68K d = 0;

    //* Delta decoding array.
    static const BYTE68K mcd212_abDelta[16] = { 0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255 };

    // Initialize delta decoding arrays for each unsigned byte value b.
    for (d = 0; d < BYTE68K_MAX + 1; d++)
    {
        mcd212_ab->deltaY[d] = mcd212_abDelta[d & 15];
    }

    // Initialize delta decoding arrays for each unsigned byte value b.
    for (d = 0; d < (BYTE68K_MAX + 1); d++)
    {
        mcd212_ab->deltaUV[d] = mcd212_abDelta[d >> 4];
    }

    // Initialize color limit and clamp arrays.
    for (w = 0; w < 3 * BYTE68K_MAX; w++)
    {
        mcd212_ab->limit[w] = (w < BYTE68K_MAX + 16) ?  0 : w <= 16 + 2 * BYTE68K_MAX ? w - BYTE68K_MAX - 16 : BYTE68K_MAX;
        mcd212_ab->clamp[w] = (w < BYTE68K_MAX + 32) ? 16 : w <= 16 + 2 * BYTE68K_MAX ? w - BYTE68K_MAX - 16 : BYTE68K_MAX;
    }

    for (sw = 0; sw < 0x100; sw++)
    {
        mcd212_ab->matrixUB[sw] = (444 * (sw - 128)) / 256;
        mcd212_ab->matrixUG[sw] = - (86 * (sw - 128)) / 256;
        mcd212_ab->matrixVG[sw] = - (179 * (sw - 128)) / 256;
        mcd212_ab->matrixVR[sw] = (351 * (sw - 128)) / 256;
    }
}

void cdi_state::video_start()
{

    mcd212_ab_init(&m_mcd212_ab);
    mcd212_init(machine(), &m_mcd212_regs);
    m_mcd212_regs.scan_timer = machine().scheduler().timer_alloc(FUNC(mcd212_perform_scan));
    m_mcd212_regs.scan_timer->adjust(machine().primary_screen->time_until_pos(0, 0));

	screen_device *screen = downcast<screen_device *>(machine().device("lcd"));
    screen->register_screen_bitmap(m_lcdbitmap);
}

UINT32 cdi_state::screen_update_cdimono1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    copybitmap(bitmap, m_mcd212_regs.m_bitmap, 0, 0, 0, 0, cliprect);
    return 0;
}

UINT32 cdi_state::screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    copybitmap(bitmap, m_lcdbitmap, 0, 0, 0, 0, cliprect);
    return 0;
}
