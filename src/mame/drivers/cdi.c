/******************************************************************************


    Philips CD-I-based games
    ------------------------

    Preliminary MAME driver by Harmony
    Help provided by CD-i Fan


*******************************************************************************

STATUS:

Quizard does not work for unknown reasons.

TODO:

- Proper handling of the 68070's internal devices (UART,DMA,Timers etc.)

- Full emulation of the CDIC, SLAVE and/or MCD212 customs

*******************************************************************************/

#define CLOCK_A XTAL_30MHz
#define CLOCK_B XTAL_19_6608MHz

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/dmadac.h"
#include "cdrom.h"
#include "machine/timekpr.h"
#include "sound/cdda.h"
#include "cdi.lh"

#define ENABLE_UART_PRINTING (0)

#define VERBOSE_LEVEL   (5)

#define ENABLE_VERBOSE_LOG (0)

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine *machine, int n_level, const char *s_fmt, ...)
{
    if( VERBOSE_LEVEL >= n_level )
    {
        va_list v;
        char buf[ 32768 ];
        va_start( v, s_fmt );
        vsprintf( buf, s_fmt, v );
        va_end( v );
        logerror( "%08x: %s", cpu_get_pc(machine->device("maincpu")), buf );
    }
}
#else
#define verboselog(x,y,z,...)
#endif

/***********************
* Forward declarations *
***********************/

// SCC68070
//static void scc68070_set_timer_callback(int channel);
//static TIMER_CALLBACK( scc68070_timer0_callback );
//static READ16_HANDLER( scc68070_periphs_r );
//static WRITE16_HANDLER( scc68070_periphs_w );

// CDIC
//static READ16_HANDLER( cdic_r );
//static WRITE16_HANDLER( cdic_w );

// SLAVE
//static TIMER_CALLBACK( slave_trigger_readback_int );
//static void slave_prepare_readback(running_machine *machine, attotime delay, UINT8 channel, UINT8 count, UINT8 data0, UINT8 data1, UINT8 data2, UINT8 data3, UINT8 cmd);
//static void perform_mouse_update(running_machine *machine);
//static INPUT_CHANGED( mouse_update );
//static READ16_HANDLER( slave_r );
//static WRITE16_HANDLER( slave_w );

// Platform-specific
//static void cdi220_draw_lcd(running_machine *machine, int y);

// MCD212
//static READ16_HANDLER(mcd212_r);
//static WRITE16_HANDLER(mcd212_w);
//static void mcd212_set_register(running_machine *machine, int channel, UINT8 reg, UINT32 value);
//static void mcd212_set_vsr(int channel, UINT32 value);
//static UINT32 mcd212_get_vsr(int channel);
//static void mcd212_set_dcp(int channel, UINT32 value);
//static UINT32 mcd212_get_dcp(int channel);
//static void mcd212_set_display_parameters(int channel, UINT8 value);
//static void mcd212_process_ica(running_machine *machine, int channel);
//static void mcd212_process_dca(running_machine *machine, int channel);
static void mcd212_update_region_arrays(running_machine *machine);
//static void mcd212_process_vsr(running_machine *machine, int channel, UINT8 *pixels_r, UINT8 *pixels_g, UINT8 *pixels_b);
//static void mcd212_draw_cursor(running_machine *machine, UINT32 *scanline, int y);
//static void mcd212_mix_lines(running_machine *machine, UINT8 *plane_a_r, UINT8 *plane_a_g, UINT8 *plane_a_b, UINT8 *plane_b_r, UINT8 *plane_b_g, UINT8 *plane_b_b, UINT32 *out);
//static void mcd212_draw_scanline(running_machine *machine, int y);
//static TIMER_CALLBACK( mcd212_perform_scan );
//static VIDEO_START(cdi);
//static VIDEO_UPDATE(cdi);

// Miscellaneous
//static TIMER_CALLBACK( test_timer_callback );

/***********************
* On-board peripherals *
***********************/

typedef struct
{
    UINT8 reserved0;
    UINT8 data_register;
    UINT8 reserved1;
    UINT8 address_register;
    UINT8 reserved2;
    UINT8 status_register;
    UINT8 reserved3;
    UINT8 control_register;
    UINT8 reserved;
    UINT8 clock_control_register;
} scc68070_i2c_regs_t;

#define ISR_MST     0x80    // Master
#define ISR_TRX     0x40    // Transmitter
#define ISR_BB      0x20    // Busy
#define ISR_PIN     0x10    // No Pending Interrupt
#define ISR_AL      0x08    // Arbitration Lost
#define ISR_AAS     0x04    // Addressed As Slave
#define ISR_AD0     0x02    // Address Zero
#define ISR_LRB     0x01    // Last Received Bit

typedef struct
{
    UINT8 reserved0;
    UINT8 mode_register;
    UINT8 reserved1;
    UINT8 status_register;
    UINT8 reserved2;
    UINT8 clock_select;
    UINT8 reserved3;
    UINT8 command_register;
    UINT8 reserved4;
    UINT8 transmit_holding_register;
    UINT8 reserved5;
    UINT8 receive_holding_register;
} scc68070_uart_regs_t;

#define UMR_OM          0xc0
#define UMR_OM_NORMAL   0x00
#define UMR_OM_ECHO     0x40
#define UMR_OM_LOOPBACK 0x80
#define UMR_OM_RLOOP    0xc0
#define UMR_TXC         0x10
#define UMR_PC          0x08
#define UMR_P           0x04
#define UMR_SB          0x02
#define UMR_CL          0x01

#define USR_RB          0x80
#define USR_FE          0x40
#define USR_PE          0x20
#define USR_OE          0x10
#define USR_TXEMT       0x08
#define USR_TXRDY       0x04
#define USR_RXRDY       0x01

typedef struct
{
    UINT8 timer_status_register;
    UINT8 timer_control_register;
    UINT16 reload_register;
    UINT16 timer0;
    UINT16 timer1;
    UINT16 timer2;
    emu_timer* timer0_timer;
} scc68070_timer_regs_t;

#define TSR_OV0         0x80
#define TSR_MA1         0x40
#define TSR_CAP1        0x20
#define TSR_OV1         0x10
#define TSR_MA2         0x08
#define TSR_CAP2        0x04
#define TSR_OV2         0x02

#define TCR_E1          0xc0
#define TCR_E1_NONE     0x00
#define TCR_E1_RISING   0x40
#define TCR_E1_FALLING  0x80
#define TCR_E1_BOTH     0xc0
#define TCR_M1          0x30
#define TCR_M1_NONE     0x00
#define TCR_M1_MATCH    0x10
#define TCR_M1_CAPTURE  0x20
#define TCR_M1_COUNT    0x30
#define TCR_E2          0x0c
#define TCR_E2_NONE     0x00
#define TCR_E2_RISING   0x04
#define TCR_E2_FALLING  0x08
#define TCR_E2_BOTH     0x0c
#define TCR_M2          0x03
#define TCR_M2_NONE     0x00
#define TCR_M2_MATCH    0x01
#define TCR_M2_CAPTURE  0x02
#define TCR_M2_COUNT    0x03

typedef struct
{
    UINT8 channel_status;
    UINT8 channel_error;

    UINT8 reserved0[2];

    UINT8 device_control;
    UINT8 operation_control;
    UINT8 sequence_control;
    UINT8 channel_control;

    UINT8 reserved1[3];

    UINT16 transfer_counter;

    UINT32 memory_address_counter;

    UINT8 reserved2[4];

    UINT32 device_address_counter;

    UINT8 reserved3[40];
} scc68070_dma_channel_t;

#define CSR_COC         0x80
#define CSR_NDT         0x20
#define CSR_ERR         0x10
#define CSR_CA          0x08

#define CER_EC          0x1f
#define CER_NONE        0x00
#define CER_TIMING      0x02
#define CER_BUSERR_MEM  0x09
#define CER_BUSERR_DEV  0x0a
#define CER_SOFT_ABORT  0x11

#define DCR1_ERM        0x80
#define DCR1_DT         0x30

#define DCR2_ERM        0x80
#define DCR2_DT         0x30
#define DCR2_DS         0x08

#define OCR_D           0x80
#define OCR_D_M2D       0x00
#define OCR_D_D2M       0x80
#define OCR_OS          0x30
#define OCR_OS_BYTE     0x00
#define OCR_OS_WORD     0x10

#define SCR2_MAC        0x0c
#define SCR2_MAC_NONE   0x00
#define SCR2_MAC_INC    0x04
#define SCR2_DAC        0x03
#define SCR2_DAC_NONE   0x00
#define SCR2_DAC_INC    0x01

#define CCR_SO          0x80
#define CCR_SA          0x10
#define CCR_INE         0x08
#define CCR_IPL         0x07

typedef struct
{
    scc68070_dma_channel_t channel[2];
} scc68070_dma_regs_t;

typedef struct
{
    UINT16 attr;
    UINT16 length;
    UINT8  undefined;
    UINT8  segment;
    UINT16 base;
} scc68070_mmu_desc_t;

typedef struct
{
    UINT8 status;
    UINT8 control;

    UINT8 reserved[0x3e];

    scc68070_mmu_desc_t desc[8];
} scc68070_mmu_regs_t;

typedef struct
{
    UINT16 lir;
    UINT8 picr1;
    UINT8 picr2;

    scc68070_i2c_regs_t i2c;
    scc68070_uart_regs_t uart;
    scc68070_timer_regs_t timers;
    scc68070_dma_regs_t dma;
    scc68070_mmu_regs_t mmu;
} scc68070_regs_t;

#define CDIC_BUFFERED_SECTORS   2

typedef struct
{
    UINT16 command;             // CDIC Command Register (0x303c00)
    UINT32 time;                // CDIC Time Register (0x303c02)
    UINT16 file;                // CDIC File Register (0x303c06)
    UINT32 channel;             // CDIC Channel Register (0x303c08)
    UINT16 audio_channel;       // CDIC Audio Channel Register (0x303c0c)

    UINT16 audio_buffer;        // CDIC Audio Buffer Register (0x303ff4)
    UINT16 x_buffer;            // CDIC X-Buffer Register (0x303ff6)
    UINT16 dma_control;         // CDIC DMA Control Register (0x303ff8)
    UINT16 z_buffer;            // CDIC Z-Buffer Register (0x303ffa)
    UINT16 interrupt_vector;    // CDIC Interrupt Vector Register (0x303ffc)
    UINT16 data_buffer;         // CDIC Data Buffer Register (0x303ffe)

    emu_timer *interrupt_timer;
    cdrom_file *cd;

    emu_timer *audio_sample_timer;
    INT32 audio_sample_freq;
    INT32 audio_sample_size;

    UINT16 decode_addr;
    UINT8 decode_delay;
    attotime decode_period;

    int xa_last[4];
    UINT16 *ram;
} cdic_regs_t;

#define CDIC_SECTOR_SYNC        0

#define CDIC_SECTOR_HEADER      12

#define CDIC_SECTOR_MODE        15

#define CDIC_SECTOR_FILE1       16
#define CDIC_SECTOR_CHAN1       17
#define CDIC_SECTOR_SUBMODE1    18
#define CDIC_SECTOR_CODING1     19

#define CDIC_SECTOR_FILE2       20
#define CDIC_SECTOR_CHAN2       21
#define CDIC_SECTOR_SUBMODE2    22
#define CDIC_SECTOR_CODING2     23

#define CDIC_SECTOR_DATA        24

#define CDIC_SECTOR_SIZE        2352

#define CDIC_SECTOR_DATASIZE    2048
#define CDIC_SECTOR_AUDIOSIZE   2304
#define CDIC_SECTOR_VIDEOSIZE   2324

#define CDIC_SUBMODE_EOF        0x80
#define CDIC_SUBMODE_RT         0x40
#define CDIC_SUBMODE_FORM       0x20
#define CDIC_SUBMODE_TRIG       0x10
#define CDIC_SUBMODE_DATA       0x08
#define CDIC_SUBMODE_AUDIO      0x04
#define CDIC_SUBMODE_VIDEO      0x02
#define CDIC_SUBMODE_EOR        0x01

typedef struct
{
    UINT8 out_buf[4];
    UINT8 out_index;
    UINT8 out_count;
    UINT8 out_cmd;
} slave_channel_t;

typedef struct
{
    slave_channel_t channel[4];
    emu_timer *interrupt_timer;

    UINT8 in_buf[17];
    UINT8 in_index;
    UINT8 in_count;

    UINT8 polling_active;

    UINT8 xbus_interrupt_enable;

    UINT8 lcd_state[16];

    UINT16 real_mouse_x;
    UINT16 real_mouse_y;

    UINT16 fake_mouse_x;
    UINT16 fake_mouse_y;
} slave_regs_t;


typedef struct
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
} mcd212_channel_t;

typedef struct
{
    mcd212_channel_t channel[2];
    emu_timer *scan_timer;
    UINT8 region_flag_0[768];
    UINT8 region_flag_1[768];
} mcd212_regs_t;

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



typedef struct _mcd212_ab_t mcd212_ab_t;
struct _mcd212_ab_t
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

class cdi_state : public driver_data_t
{
public:
    static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cdi_state(machine)); }

    cdi_state(running_machine &machine)
		: driver_data_t(machine) { }

    UINT16 *planea;
    UINT16 *planeb;

    dmadac_sound_device *dmadac[2];

    UINT8 timer_set;
    emu_timer *test_timer;
    bitmap_t* lcdbitmap;
    scc68070_regs_t scc68070_regs;
    cdic_regs_t cdic_regs;
    slave_regs_t slave_regs;
    mcd212_regs_t mcd212_regs;
    mcd212_ab_t mcd212_ab;
};

static void scc68070_set_timer_callback(scc68070_regs_t *scc68070, int channel)
{
    UINT32 compare = 0;
    attotime period;
    switch(channel)
    {
        case 0:
            compare = 0x10000 - scc68070->timers.timer0;
            period = attotime_mul(ATTOTIME_IN_HZ(CLOCK_A/192), compare);
            timer_adjust_oneshot(scc68070->timers.timer0_timer, period, 0);
            break;
        default:
            fatalerror( "Unsupported timer channel to scc68070_set_timer_callback!\n" );
    }
}

static TIMER_CALLBACK( scc68070_timer0_callback )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;

    scc68070->timers.timer0 = scc68070->timers.reload_register;
    scc68070->timers.timer_status_register |= TSR_OV0;
    if(scc68070->picr1 & 7)
    {
        UINT8 interrupt = scc68070->picr1 & 7;
        scc68070->timers.timer_status_register |= TSR_OV0;
        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
        cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
    }
    scc68070_set_timer_callback(&state->scc68070_regs, 0);
}

static READ16_HANDLER( scc68070_periphs_r )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;

    switch(offset)
    {
        // Interupts: 80001001
        case 0x1000/2: // LIR priority level
            return scc68070->lir;

        // I2C interface: 80002001 to 80002009
        case 0x2000/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Data Register: %04x & %04x\n", scc68070->i2c.data_register, mem_mask);
            }
            return scc68070->i2c.data_register;
        case 0x2002/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Address Register: %04x & %04x\n", scc68070->i2c.address_register, mem_mask);
            }
            return scc68070->i2c.address_register;
        case 0x2004/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Status Register: %04x & %04x\n", scc68070->i2c.status_register, mem_mask);
            }
            return scc68070->i2c.status_register;
        case 0x2006/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Control Register: %04x & %04x\n", scc68070->i2c.control_register, mem_mask);
            }
            return scc68070->i2c.control_register;
        case 0x2008/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Clock Control Register: %04x & %04x\n", scc68070->i2c.clock_control_register, mem_mask);
            }
            return scc68070->i2c.clock_control_register;

        // UART interface: 80002011 to 8000201b
        case 0x2010/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Mode Register: %04x & %04x\n", scc68070->uart.mode_register, mem_mask);
            }
            return scc68070->uart.mode_register | 0x20;
        case 0x2012/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Status Register: %04x & %04x\n", scc68070->uart.status_register, mem_mask);
            }
            return scc68070->uart.status_register /*| USR_TXEMT*/ | USR_TXRDY | (1 << 1) | USR_RXRDY;
        case 0x2014/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Clock Select: %04x & %04x\n", scc68070->uart.clock_select, mem_mask);
            }
            return scc68070->uart.clock_select | 0x08;
        case 0x2016/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Command Register: %02x & %04x\n", scc68070->uart.command_register, mem_mask);
            }
            return scc68070->uart.command_register | 0x80;
        case 0x2018/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Transmit Holding Register: %02x & %04x\n", scc68070->uart.transmit_holding_register, mem_mask);
            }
            return scc68070->uart.transmit_holding_register;
        case 0x201a/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Receive Holding Register: %02x & %04x\n", scc68070->uart.receive_holding_register, mem_mask);
                return scc68070->uart.receive_holding_register;
            }
            return 0;

        // Timers: 80002020 to 80002029
        case 0x2020/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: Timer Control Register: %02x & %04x\n", scc68070->timers.timer_control_register, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 12, "scc68070_periphs_r: Timer Status Register: %02x & %04x\n", scc68070->timers.timer_status_register, mem_mask);
            }
            return (scc68070->timers.timer_status_register << 8) | scc68070->timers.timer_control_register;
        case 0x2022/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer Reload Register: %04x & %04x\n", scc68070->timers.reload_register, mem_mask);
            return scc68070->timers.reload_register;
        case 0x2024/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer 0: %04x & %04x\n", scc68070->timers.timer0, mem_mask);
            return scc68070->timers.timer0;
        case 0x2026/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer 1: %04x & %04x\n", scc68070->timers.timer1, mem_mask);
            printf( "Timer 1 read\n" );
            return scc68070->timers.timer1;
        case 0x2028/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer 2: %04x & %04x\n", scc68070->timers.timer2, mem_mask);
            printf( "Timer 2 read\n" );
            return scc68070->timers.timer2;

        // PICR1: 80002045
        case 0x2044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: Peripheral Interrupt Control Register 1: %02x & %04x\n", scc68070->picr1, mem_mask);
            }
            return scc68070->picr1;

        // PICR2: 80002047
        case 0x2046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: Peripheral Interrupt Control Register 2: %02x & %04x\n", scc68070->picr2, mem_mask);
            }
            return scc68070->picr2;

        // DMA controller: 80004000 to 8000406d
        case 0x4000/2:
        case 0x4040/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Error Register: %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].channel_error, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Status Register: %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].channel_status, mem_mask);
            }
            return (scc68070->dma.channel[(offset - 0x2000) / 32].channel_status << 8) | scc68070->dma.channel[(offset - 0x2000) / 32].channel_error;
        case 0x4004/2:
        case 0x4044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Operation Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].operation_control, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Device Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].device_control, mem_mask);
            }
            return (scc68070->dma.channel[(offset - 0x2000) / 32].device_control << 8) | scc68070->dma.channel[(offset - 0x2000) / 32].operation_control;
        case 0x4006/2:
        case 0x4046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Channel Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].channel_control, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Sequence Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].sequence_control, mem_mask);
            }
            return (scc68070->dma.channel[(offset - 0x2000) / 32].sequence_control << 8) | scc68070->dma.channel[(offset - 0x2000) / 32].channel_control;
        case 0x400a/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Memory Transfer Counter: %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].transfer_counter, mem_mask);
            return scc68070->dma.channel[(offset - 0x2000) / 32].transfer_counter;
        case 0x400c/2:
        case 0x404c/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Memory Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, (scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter >> 16), mem_mask);
            return (scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter >> 16);
        case 0x400e/2:
        case 0x404e/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Memory Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter, mem_mask);
            return scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter;
        case 0x4014/2:
        case 0x4054/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Device Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, (scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter >> 16), mem_mask);
            return (scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter >> 16);
        case 0x4016/2:
        case 0x4056/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Device Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter, mem_mask);
            return scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter;

        // MMU: 80008000 to 8000807f
        case 0x8000/2:  // Status / Control register
            if(ACCESSING_BITS_0_7)
            {   // Control
                verboselog(space->machine, 2, "scc68070_periphs_r: MMU Control: %02x & %04x\n", scc68070->mmu.control, mem_mask);
                return scc68070->mmu.control;
            }   // Status
            else
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: MMU Status: %02x & %04x\n", scc68070->mmu.status, mem_mask);
                return scc68070->mmu.status;
            }
            break;
        case 0x8040/2:
        case 0x8048/2:
        case 0x8050/2:
        case 0x8058/2:
        case 0x8060/2:
        case 0x8068/2:
        case 0x8070/2:
        case 0x8078/2:  // Attributes (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d attributes: %04x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].attr, mem_mask);
            return scc68070->mmu.desc[(offset - 0x4020) / 4].attr;
        case 0x8042/2:
        case 0x804a/2:
        case 0x8052/2:
        case 0x805a/2:
        case 0x8062/2:
        case 0x806a/2:
        case 0x8072/2:
        case 0x807a/2:  // Segment Length (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d length: %04x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].length, mem_mask);
            return scc68070->mmu.desc[(offset - 0x4020) / 4].length;
        case 0x8044/2:
        case 0x804c/2:
        case 0x8054/2:
        case 0x805c/2:
        case 0x8064/2:
        case 0x806c/2:
        case 0x8074/2:
        case 0x807c/2:  // Segment Number (SD0-7, A0=1 only)
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d segment: %02x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].segment, mem_mask);
                return scc68070->mmu.desc[(offset - 0x4020) / 4].segment;
            }
            break;
        case 0x8046/2:
        case 0x804e/2:
        case 0x8056/2:
        case 0x805e/2:
        case 0x8066/2:
        case 0x806e/2:
        case 0x8076/2:
        case 0x807e/2:  // Base Address (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d base: %04x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].base, mem_mask);
            return scc68070->mmu.desc[(offset - 0x4020) / 4].base;
        default:
            verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
            break;
    }

    return 0;
}

static WRITE16_HANDLER( scc68070_periphs_w )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;

    switch(offset)
    {
        // Interupts: 80001001
        case 0x1000/2: // LIR priority level
            verboselog(space->machine, 2, "scc68070_periphs_w: LIR: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->lir);
            break;

        // I2C interface: 80002001 to 80002009
        case 0x2000/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Data Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.data_register = data & 0x00ff;
            }
            break;
        case 0x2002/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Address Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.address_register = data & 0x00ff;
            }
            break;
        case 0x2004/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Status Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.status_register = data & 0x00ff;
            }
            break;
        case 0x2006/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Control Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.control_register = data & 0x00ff;
            }
            break;
        case 0x2008/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Clock Control Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.clock_control_register = data & 0x00ff;
            }
            break;

        // UART interface: 80002011 to 8000201b
        case 0x2010/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Mode Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.mode_register = data & 0x00ff;
            }
            break;
        case 0x2012/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Status Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.status_register = data & 0x00ff;
            }
            break;
        case 0x2014/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Clock Select: %04x & %04x\n", data, mem_mask);
                scc68070->uart.clock_select = data & 0x00ff;
            }
            break;
        case 0x2016/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Command Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.command_register = data & 0x00ff;
            }
            break;
        case 0x2018/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Transmit Holding Register: %04x & %04x: %c\n", data, mem_mask, (data >= 0x20 && data < 0x7f) ? (data & 0x00ff) : ' ');
                if((data >= 0x20 && data < 0x7f) || data == 0x08)
                {
                    printf( "%c", data & 0x00ff );
                }
                if(data == 0x0d)
                {
                    printf( "\n" );
                }
                scc68070->uart.transmit_holding_register = data & 0x00ff;
            }
            break;
        case 0x201a/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Receive Holding Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.receive_holding_register = data & 0x00ff;
            }
            break;

        // Timers: 80002020 to 80002029
        case 0x2020/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: Timer Control Register: %04x & %04x\n", data, mem_mask);
                scc68070->timers.timer_control_register = data & 0x00ff;
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 12, "scc68070_periphs_w: Timer Status Register: %04x & %04x\n", data, mem_mask);
                scc68070->timers.timer_status_register &= ~(data >> 8);
                if(!scc68070->timers.timer_status_register)
                {
                    UINT8 interrupt = scc68070->picr1 & 7;
                    cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), CLEAR_LINE);
                }
            }
            break;
        case 0x2022/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer Reload Register: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.reload_register);
            break;
        case 0x2024/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer 0: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.timer0);
            scc68070_set_timer_callback(&state->scc68070_regs, 0);
            break;
        case 0x2026/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer 1: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.timer1);
            printf( "Timer 1 write: %04x\n", data );
            break;
        case 0x2028/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer 2: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.timer2);
            printf( "Timer 2 write: %04x\n", data );
            break;

        // PICR1: 80002045
        case 0x2044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: Peripheral Interrupt Control Register 1: %04x & %04x\n", data, mem_mask);
                scc68070->picr1 = data & 0x00ff;
            }
            break;

        // PICR2: 80002047
        case 0x2046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: Peripheral Interrupt Control Register 2: %04x & %04x\n", data, mem_mask);
                scc68070->picr2 = data & 0x00ff;
            }
            break;

        // DMA controller: 80004000 to 8000406d
        case 0x4000/2:
        case 0x4040/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Error (invalid): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Status: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].channel_status &= ~(data & 0xb0);
            }
            break;
        case 0x4004/2:
        case 0x4044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Operation Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].operation_control = data & 0x00ff;
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Device Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].device_control = data >> 8;
            }
            break;
        case 0x4006/2:
        case 0x4046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Channel Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].channel_control = data & 0x007f;
                if(data & CCR_SO)
                {
                    scc68070->dma.channel[(offset - 0x2000) / 32].channel_status |= CSR_COC;
                }
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Sequence Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].sequence_control = data >> 8;
            }
            break;
        case 0x400a/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Memory Transfer Counter: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            COMBINE_DATA(&scc68070->dma.channel[(offset - 0x2000) / 32].transfer_counter);
            break;
        case 0x400c/2:
        case 0x404c/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Memory Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter &= ~(mem_mask << 16);
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter |= data << 16;
            break;
        case 0x400e/2:
        case 0x404e/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Memory Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter &= ~mem_mask;
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter |= data;
            break;
        case 0x4014/2:
        case 0x4054/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Device Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter &= ~(mem_mask << 16);
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter |= data << 16;
            break;
        case 0x4016/2:
        case 0x4056/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Device Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter &= ~mem_mask;
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter |= data;
            break;

        // MMU: 80008000 to 8000807f
        case 0x8000/2:  // Status / Control register
            if(ACCESSING_BITS_0_7)
            {   // Control
                verboselog(space->machine, 2, "scc68070_periphs_w: MMU Control: %04x & %04x\n", data, mem_mask);
                scc68070->mmu.control = data & 0x00ff;
            }   // Status
            else
            {
                verboselog(space->machine, 0, "scc68070_periphs_w: MMU Status (invalid): %04x & %04x\n", data, mem_mask);
            }
            break;
        case 0x8040/2:
        case 0x8048/2:
        case 0x8050/2:
        case 0x8058/2:
        case 0x8060/2:
        case 0x8068/2:
        case 0x8070/2:
        case 0x8078/2:  // Attributes (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d attributes: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
            COMBINE_DATA(&scc68070->mmu.desc[(offset - 0x4020) / 4].attr);
            break;
        case 0x8042/2:
        case 0x804a/2:
        case 0x8052/2:
        case 0x805a/2:
        case 0x8062/2:
        case 0x806a/2:
        case 0x8072/2:
        case 0x807a/2:  // Segment Length (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d length: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
            COMBINE_DATA(&scc68070->mmu.desc[(offset - 0x4020) / 4].length);
            break;
        case 0x8044/2:
        case 0x804c/2:
        case 0x8054/2:
        case 0x805c/2:
        case 0x8064/2:
        case 0x806c/2:
        case 0x8074/2:
        case 0x807c/2:  // Segment Number (SD0-7, A0=1 only)
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d segment: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
                scc68070->mmu.desc[(offset - 0x4020) / 4].segment = data & 0x00ff;
            }
            break;
        case 0x8046/2:
        case 0x804e/2:
        case 0x8056/2:
        case 0x805e/2:
        case 0x8066/2:
        case 0x806e/2:
        case 0x8076/2:
        case 0x807e/2:  // Base Address (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d base: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
            COMBINE_DATA(&scc68070->mmu.desc[(offset - 0x4020) / 4].base);
            break;
        default:
            verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
            break;
    }
}

static void scc68070_init(running_machine *machine, scc68070_regs_t *scc68070)
{
    int index = 0;
    scc68070->lir = 0;

    scc68070->picr1 = 0;
    scc68070->picr2 = 0;

    scc68070->i2c.data_register = 0;
    scc68070->i2c.address_register = 0;
    scc68070->i2c.status_register = 0;
    scc68070->i2c.control_register = 0;
    scc68070->i2c.clock_control_register = 0;

    scc68070->uart.mode_register = 0;
    scc68070->uart.status_register = 0;
    scc68070->uart.clock_select = 0;
    scc68070->uart.command_register = 0;
    scc68070->uart.transmit_holding_register = 0;
    scc68070->uart.receive_holding_register = 0;

    scc68070->timers.timer_status_register = 0;
    scc68070->timers.timer_control_register = 0;
    scc68070->timers.reload_register = 0;
    scc68070->timers.timer0 = 0;
    scc68070->timers.timer1 = 0;
    scc68070->timers.timer2 = 0;

    for(index = 0; index < 2; index++)
    {
        scc68070->dma.channel[index].channel_status = 0;
        scc68070->dma.channel[index].channel_error = 0;
        scc68070->dma.channel[index].device_control = 0;
        scc68070->dma.channel[index].operation_control = 0;
        scc68070->dma.channel[index].sequence_control = 0;
        scc68070->dma.channel[index].channel_control = 0;
        scc68070->dma.channel[index].transfer_counter = 0;
        scc68070->dma.channel[index].memory_address_counter = 0;
        scc68070->dma.channel[index].device_address_counter = 0;
    }

    scc68070->mmu.status = 0;
    scc68070->mmu.control = 0;
    for(index = 0; index < 8; index++)
    {
        scc68070->mmu.desc[index].attr = 0;
        scc68070->mmu.desc[index].length = 0;
        scc68070->mmu.desc[index].segment = 0;
        scc68070->mmu.desc[index].base = 0;
    }
}

static void scc68070_register_globals(running_machine *machine, scc68070_regs_t *scc68070)
{
    state_save_register_global(machine, scc68070->lir);

    state_save_register_global(machine, scc68070->picr1);
    state_save_register_global(machine, scc68070->picr2);

    state_save_register_global(machine, scc68070->i2c.data_register);
    state_save_register_global(machine, scc68070->i2c.address_register);
    state_save_register_global(machine, scc68070->i2c.status_register);
    state_save_register_global(machine, scc68070->i2c.control_register);
    state_save_register_global(machine, scc68070->i2c.clock_control_register);

    state_save_register_global(machine, scc68070->uart.mode_register);
    state_save_register_global(machine, scc68070->uart.status_register);
    state_save_register_global(machine, scc68070->uart.clock_select);
    state_save_register_global(machine, scc68070->uart.command_register);
    state_save_register_global(machine, scc68070->uart.transmit_holding_register);
    state_save_register_global(machine, scc68070->uart.receive_holding_register);

    state_save_register_global(machine, scc68070->timers.timer_status_register);
    state_save_register_global(machine, scc68070->timers.timer_control_register);
    state_save_register_global(machine, scc68070->timers.reload_register);
    state_save_register_global(machine, scc68070->timers.timer0);
    state_save_register_global(machine, scc68070->timers.timer1);
    state_save_register_global(machine, scc68070->timers.timer2);

    state_save_register_global(machine, scc68070->dma.channel[0].channel_status);
    state_save_register_global(machine, scc68070->dma.channel[0].channel_error);
    state_save_register_global(machine, scc68070->dma.channel[0].device_control);
    state_save_register_global(machine, scc68070->dma.channel[0].operation_control);
    state_save_register_global(machine, scc68070->dma.channel[0].sequence_control);
    state_save_register_global(machine, scc68070->dma.channel[0].channel_control);
    state_save_register_global(machine, scc68070->dma.channel[0].transfer_counter);
    state_save_register_global(machine, scc68070->dma.channel[0].memory_address_counter);
    state_save_register_global(machine, scc68070->dma.channel[0].device_address_counter);
    state_save_register_global(machine, scc68070->dma.channel[1].channel_status);
    state_save_register_global(machine, scc68070->dma.channel[1].channel_error);
    state_save_register_global(machine, scc68070->dma.channel[1].device_control);
    state_save_register_global(machine, scc68070->dma.channel[1].operation_control);
    state_save_register_global(machine, scc68070->dma.channel[1].sequence_control);
    state_save_register_global(machine, scc68070->dma.channel[1].channel_control);
    state_save_register_global(machine, scc68070->dma.channel[1].transfer_counter);
    state_save_register_global(machine, scc68070->dma.channel[1].memory_address_counter);
    state_save_register_global(machine, scc68070->dma.channel[1].device_address_counter);

    state_save_register_global(machine, scc68070->mmu.status);
    state_save_register_global(machine, scc68070->mmu.control);
    state_save_register_global(machine, scc68070->mmu.desc[0].attr);
    state_save_register_global(machine, scc68070->mmu.desc[0].length);
    state_save_register_global(machine, scc68070->mmu.desc[0].segment);
    state_save_register_global(machine, scc68070->mmu.desc[0].base);
    state_save_register_global(machine, scc68070->mmu.desc[1].attr);
    state_save_register_global(machine, scc68070->mmu.desc[1].length);
    state_save_register_global(machine, scc68070->mmu.desc[1].segment);
    state_save_register_global(machine, scc68070->mmu.desc[1].base);
    state_save_register_global(machine, scc68070->mmu.desc[2].attr);
    state_save_register_global(machine, scc68070->mmu.desc[2].length);
    state_save_register_global(machine, scc68070->mmu.desc[2].segment);
    state_save_register_global(machine, scc68070->mmu.desc[2].base);
    state_save_register_global(machine, scc68070->mmu.desc[3].attr);
    state_save_register_global(machine, scc68070->mmu.desc[3].length);
    state_save_register_global(machine, scc68070->mmu.desc[3].segment);
    state_save_register_global(machine, scc68070->mmu.desc[3].base);
    state_save_register_global(machine, scc68070->mmu.desc[4].attr);
    state_save_register_global(machine, scc68070->mmu.desc[4].length);
    state_save_register_global(machine, scc68070->mmu.desc[4].segment);
    state_save_register_global(machine, scc68070->mmu.desc[4].base);
    state_save_register_global(machine, scc68070->mmu.desc[5].attr);
    state_save_register_global(machine, scc68070->mmu.desc[5].length);
    state_save_register_global(machine, scc68070->mmu.desc[5].segment);
    state_save_register_global(machine, scc68070->mmu.desc[5].base);
    state_save_register_global(machine, scc68070->mmu.desc[6].attr);
    state_save_register_global(machine, scc68070->mmu.desc[6].length);
    state_save_register_global(machine, scc68070->mmu.desc[6].segment);
    state_save_register_global(machine, scc68070->mmu.desc[6].base);
    state_save_register_global(machine, scc68070->mmu.desc[7].attr);
    state_save_register_global(machine, scc68070->mmu.desc[7].length);
    state_save_register_global(machine, scc68070->mmu.desc[7].segment);
    state_save_register_global(machine, scc68070->mmu.desc[7].base);
}

#if ENABLE_UART_PRINTING
static READ16_HANDLER( uart_loopback_enable )
{
    return 0x1234;
}
#endif




static const INT32 cdic_adpcm_filter_coef[5][2] =
{
    { 0,0 },
    { 60,0 },
    { 115,-52 },
    { 98,-55 },
    { 122,-60 },
};

INLINE int CDIC_IS_VALID_SAMPLE_BUF(UINT16 *cdram, UINT16 addr)
{
    UINT8 *cdram8 = ((UINT8*)cdram) + addr + 8;
    if(cdram8[2] != 0xff)
    {
        return 1;
    }
    return 0;
}

INLINE double CDIC_SAMPLE_BUF_FREQ(UINT16 *cdram, UINT16 addr)
{
    UINT8 *cdram8 = ((UINT8*)cdram) + addr + 8;
    switch(cdram8[2] & 0x3f)
    {
        case 0:
        case 1:
        case 16:
        case 17:
            return 37800.0f;

        case 4:
        case 5:
            return 18900.0f;

        default:
            return 18900.0f;
    }
}

INLINE int CDIC_SAMPLE_BUF_SIZE(UINT16 *cdram, UINT16 addr)
{
    UINT8 *cdram8 = ((UINT8*)cdram) + addr + 8;
    switch(cdram8[2] & 0x3f)
    {
        case 0:
        case 4:
            return 4;

        case 1:
        case 5:
        case 16:
            return 2;

        case 17:
            return 1;

        default:
            return 2;
    }
}

INLINE INT16 clamp(INT16 in)
{
    return in;
}

static void cdic_decode_xa_mono(int *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
    int l0=cdic_xa_last[0],
            l1=cdic_xa_last[1];
    int b=0;
    int s=0;

    for (b=0; b<18; b++)
    {
        for (s=0; s<4; s++)
        {
            unsigned char flags=xa[(4+(s<<1))^1],
                                        shift=flags&0xf,
                                        filter=flags>>4;
            int f0=cdic_adpcm_filter_coef[filter][0],
                    f1=cdic_adpcm_filter_coef[filter][1];
            int i=0;

            for (i=0; i<28; i++)
            {
                short d=(xa[(16+(i<<2)+s)^1]&0xf)<<12;
                d=clamp((d>>shift)+(((l0*f0)+(l1*f1)+32)>>6));
                *dp++=d;
                l1=l0;
                l0=d;
            }

            flags=xa[(5+(s<<1))^1];
            shift=flags&0xf;
            filter=flags>>4;
            f0=cdic_adpcm_filter_coef[filter][0];
            f1=cdic_adpcm_filter_coef[filter][1];

            for (i=0; i<28; i++)
            {
                short d=(xa[(16+(i<<2)+s)^1]>>4)<<12;
                d=clamp((d>>shift)+(((l0*f0)+(l1*f1)+32)>>6));
                *dp++=d;
                l1=l0;
                l0=d;
            }
        }

        xa+=128;
    }

    cdic_xa_last[0]=l0;
    cdic_xa_last[1]=l1;
}

static void cdic_decode_xa_mono8(int *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
    int l0=cdic_xa_last[0],
            l1=cdic_xa_last[1];
    int b=0;
    int s=0;

    for (b=0; b<18; b++)
    {
        for (s=0; s<4; s++)
        {
            unsigned char flags=xa[(4+s)^1],
                                        shift=flags&0xf,
                                        filter=flags>>4;
            int f0=cdic_adpcm_filter_coef[filter][0],
                    f1=cdic_adpcm_filter_coef[filter][1];
            int i=0;

            for (i=0; i<28; i++)
            {
                short d=(xa[(16+(i<<2)+s)^1]<<8);
                d=clamp((d>>shift)+(((l0*f0)+(l1*f1)+32)>>6));
                *dp++=d;
                l1=l0;
                l0=d;
            }
        }

        xa+=128;
    }

    cdic_xa_last[0]=l0;
    cdic_xa_last[1]=l1;
}

static void cdic_decode_xa_stereo(int *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
    int l0=cdic_xa_last[0],
            l1=cdic_xa_last[1],
            l2=cdic_xa_last[2],
            l3=cdic_xa_last[3];
    int b=0;
    int s=0;
    //int decoded = 0;

    for (b=0; b<18; b++)
    {
        for (s=0; s<4; s++)
        {
            unsigned char flags0=xa[(4+(s<<1))^1],
                                        shift0=flags0&0xf,
                                        filter0=flags0>>4,
                                        flags1=xa[(5+(s<<1))^1],
                                        shift1=flags1&0xf,
                                        filter1=flags1>>4;

            int f0=cdic_adpcm_filter_coef[filter0][0],
                    f1=cdic_adpcm_filter_coef[filter0][1],
                    f2=cdic_adpcm_filter_coef[filter1][0],
                    f3=cdic_adpcm_filter_coef[filter1][1];
            int i=0;

            for (i=0; i<28; i++)
            {
                short d=xa[(16+(i<<2)+s)^1],
                            d0=(d&0xf)<<12,
                            d1=(d>>4)<<12;
                d0=clamp((d0>>shift0)+(((l0*f0)+(l1*f1)+32)>>6));
                *dp++=d0;
                l1=l0;
                l0=d0;

                d1=clamp((d1>>shift1)+(((l2*f2)+(l3*f3)+32)>>6));
                *dp++=d1;
                l3=l2;
                l2=d1;
                //decoded += 2;
            }
        }

        xa+=128;
    }

    cdic_xa_last[0]=l0;
    cdic_xa_last[1]=l1;
    cdic_xa_last[2]=l2;
    cdic_xa_last[3]=l3;

    //printf( "Decoded %d samples\n", decoded );
}

static void cdic_decode_xa_stereo8(int *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
    int l0=cdic_xa_last[0],
            l1=cdic_xa_last[1],
            l2=cdic_xa_last[2],
            l3=cdic_xa_last[3];
    int b=0;
    int s=0;

    for (b=0; b<18; b++)
    {
        for (s=0; s<4; s += 2)
        {
            unsigned char flags0=xa[(4+s)^1],
                                        shift0=flags0&0xf,
                                        filter0=flags0>>4,
                                        flags1=xa[(5+s)^1],
                                        shift1=flags1&0xf,
                                        filter1=flags1>>4;
            int f0=cdic_adpcm_filter_coef[filter0][0],
                    f1=cdic_adpcm_filter_coef[filter0][1],
                    f2=cdic_adpcm_filter_coef[filter1][0],
                    f3=cdic_adpcm_filter_coef[filter1][1];
            int i=0;

            for (i=0; i<28; i++)
            {
                //short d0=(xa[(16+((i+0)<<2)+s)^1]<<8);
                //short d1=(xa[(16+((i+1)<<2)+s)^1]<<8);
                short d0=(xa[(16+(i<<2)+s+0)^1]<<8);
                short d1=(xa[(16+(i<<2)+s+1)^1]<<8);

                d0=clamp((d0>>shift0)+(((l0*f0)+(l1*f1)+32)>>6));
                *dp++=d0;
                l1=l0;
                l0=d0;

                d1=clamp((d1>>shift1)+(((l2*f2)+(l3*f3)+32)>>6));
                *dp++=d1;
                l3=l2;
                l2=d1;
            }
        }

        xa+=128;
    }

    cdic_xa_last[0]=l0;
    cdic_xa_last[1]=l1;
    cdic_xa_last[2]=l2;
    cdic_xa_last[3]=l3;
}

static void cdic_decode_audio_sector(running_machine *machine, const unsigned char *xa, int triggered)
{
    // Get XA format from sector header

    cdi_state *state = machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;
    const unsigned char *hdr = xa + 4;
    int channels;
    int bits = 4;
    int index = 0;
    INT16 samples[18*28*16+16];
    //FILE* temp_adpcm = fopen("temp_adpcm.bin","ab");

    //printf( "%02x\n", hdr[2] & 0x3f );

    if(hdr[2] == 0xff && triggered == 1)
    {
        // Don't play
        //timer_adjust_oneshot(cdic->audio_sample_timer, attotime_never, 0);
        //fclose(temp_adpcm);
        return;
    }

    verboselog(machine, 0, "cdic_decode_audio_sector, got header type %02x\n", hdr[2] );

    //fseek(temp_adpcm, 0, SEEK_END);
    switch(hdr[2] & 0x3f)   // ignore emphasis and reserved bits
    {
        case 0:
            channels=1;
            cdic->audio_sample_freq=37800.0f;   //18900.0f;
            bits=4;
            cdic->audio_sample_size=4;
            break;

        case 1:
            channels=2;
            cdic->audio_sample_freq=37800.0f;
            bits=4;
            cdic->audio_sample_size=2;
            break;

        case 4:
            channels=1;
            cdic->audio_sample_freq=18900.0f;   ///2.0f;
            bits=4;
            cdic->audio_sample_size=4;
            break;

        case 5:
            channels=2;
            cdic->audio_sample_freq=18900.0f;   //37800.0f/2.0f;
            bits=4;
            cdic->audio_sample_size=2;
            break;

        case 16:
            channels=1;
            cdic->audio_sample_freq=37800.0f;
            bits=8;
            cdic->audio_sample_size=2;
            break;

        case 17:
            channels=2;
            cdic->audio_sample_freq=37800.0f;
            bits=8;
            cdic->audio_sample_size=1;
            break;

        default:
            fatalerror("play_xa: unhandled xa mode %08x\n",hdr[2]);
            return;
    }

    dmadac_set_frequency(&state->dmadac[0], 2, cdic->audio_sample_freq);
    dmadac_enable(&state->dmadac[0], 2, 1);

    switch(channels)
    {
        case 1:
            switch(bits)
            {
                case 4:
                    cdic_decode_xa_mono(cdic->xa_last, hdr + 4, samples);
                    for(index = 18*28*8 - 1; index >= 0; index--)
                    {
                        samples[index*2 + 1] = samples[index];
                        samples[index*2 + 0] = samples[index];
                    }
                    samples[18*28*16 + 0] = samples[18*28*16 + 2] = samples[18*28*16 + 4] = samples[18*28*16 + 6] = samples[18*28*16 + 8] = samples[18*28*16 + 10] = samples[18*28*16 + 12] = samples[18*28*16 + 14] = samples[18*28*16 - 2];
                    samples[18*28*16 + 1] = samples[18*28*16 + 3] = samples[18*28*16 + 5] = samples[18*28*16 + 7] = samples[18*28*16 + 9] = samples[18*28*16 + 11] = samples[18*28*16 + 13] = samples[18*28*16 + 15] = samples[18*28*16 - 1];
                    break;
                case 8:
                    cdic_decode_xa_mono8(cdic->xa_last, hdr + 4, samples);
                    for(index = 18*28*8 - 1; index >= 0; index--)
                    {
                        samples[index*2 + 1] = samples[index];
                        samples[index*2 + 0] = samples[index];
                    }
                    samples[18*28*8 + 0] = samples[18*28*8 + 2] = samples[18*28*8 + 4] = samples[18*28*8 + 6] = samples[18*28*8 + 8] = samples[18*28*8 + 10] = samples[18*28*8 + 12] = samples[18*28*8 + 14] = samples[18*28*8 - 2];
                    samples[18*28*8 + 1] = samples[18*28*8 + 3] = samples[18*28*8 + 5] = samples[18*28*8 + 7] = samples[18*28*8 + 9] = samples[18*28*8 + 11] = samples[18*28*8 + 13] = samples[18*28*8 + 15] = samples[18*28*8 - 1];
                    break;
            }
            break;
        case 2:
            switch(bits)
            {
                case 4:
                    cdic_decode_xa_stereo(cdic->xa_last, hdr + 4, samples);
                    samples[18*28*8 + 0] = samples[18*28*8 + 2] = samples[18*28*8 + 4] = samples[18*28*8 + 6] = samples[18*28*8 + 8] = samples[18*28*8 + 10] = samples[18*28*8 + 12] = samples[18*28*8 + 14] = samples[18*28*8 - 2];
                    samples[18*28*8 + 1] = samples[18*28*8 + 3] = samples[18*28*8 + 5] = samples[18*28*8 + 7] = samples[18*28*8 + 9] = samples[18*28*8 + 11] = samples[18*28*8 + 13] = samples[18*28*8 + 15] = samples[18*28*8 - 1];
                    //fwrite(samples, 1, 18*28*4*cdic->audio_sample_size, temp_adpcm);
                    break;
                case 8:
                    cdic_decode_xa_stereo8(cdic->xa_last, hdr + 4, samples);
                    samples[18*28*4 + 0] = samples[18*28*4 + 2] = samples[18*28*4 + 4] = samples[18*28*4 + 6] = samples[18*28*4 + 8] = samples[18*28*4 + 10] = samples[18*28*4 + 12] = samples[18*28*4 + 14] = samples[18*28*4 - 2];
                    samples[18*28*4 + 1] = samples[18*28*4 + 3] = samples[18*28*4 + 5] = samples[18*28*4 + 7] = samples[18*28*4 + 9] = samples[18*28*4 + 11] = samples[18*28*4 + 13] = samples[18*28*4 + 15] = samples[18*28*4 - 1];
                    break;
            }
            break;
    }

    dmadac_transfer(&state->dmadac[0], 2, 1, 2, 18*28*2*cdic->audio_sample_size, samples);

    //fclose(temp_adpcm);
}

// After an appropriate delay for decoding to take place...
static TIMER_CALLBACK( audio_sample_trigger )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;

    if(cdic->decode_addr == 0xffff)
    {
        verboselog(machine, 0, "Decode stop requested, stopping playback\n" );
        timer_adjust_oneshot(cdic->audio_sample_timer, attotime_never, 0);
        return;
    }

    if(!cdic->decode_delay)
    {
        // Indicate that data has been decoded
        verboselog(machine, 0, "Flagging that audio data has been decoded\n" );
        cdic->audio_buffer |= 0x8000;

        // Set the CDIC interrupt line
        verboselog(machine, 0, "Setting CDIC interrupt line for soundmap decode\n" );
        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 128);
        cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
    }
    else
    {
        cdic->decode_delay = 0;
    }

    if(CDIC_IS_VALID_SAMPLE_BUF(cdic->ram, cdic->decode_addr & 0x3ffe))
    {
        verboselog(machine, 0, "Hit audio_sample_trigger, with cdic->decode_addr == %04x, calling cdic_decode_audio_sector\n", cdic->decode_addr );

        // Decode the data at Z+4, the same offset as a normal CD sector.
        cdic_decode_audio_sector(machine, ((UINT8*)cdic->ram) + (cdic->decode_addr & 0x3ffe) + 4, 1);

        // Swap buffer positions to indicate our new buffer position at the next read
        cdic->decode_addr ^= 0x1a00;

        verboselog(machine, 0, "Updated cdic->decode_addr, new value is %04x\n", cdic->decode_addr );

        //// Delay for Frequency * (18*28*2*size in bytes) before requesting more data
        verboselog(machine, 0, "Data is valid, setting up a new callback\n" );
        cdic->decode_period = attotime_mul(ATTOTIME_IN_HZ(CDIC_SAMPLE_BUF_FREQ(cdic->ram, cdic->decode_addr & 0x3ffe)), 18*28*2*CDIC_SAMPLE_BUF_SIZE(cdic->ram, cdic->decode_addr & 0x3ffe));
        timer_adjust_oneshot(cdic->audio_sample_timer, cdic->decode_period, 0);
        //dmadac_enable(&dmadac[0], 2, 0);
    }
    else
    {
        // Swap buffer positions to indicate our new buffer position at the next read
        cdic->decode_addr ^= 0x1a00;

        verboselog(machine, 0, "Data is not valid, indicating to shut down on the next audio sample\n" );
        cdic->decode_addr = 0xffff;
        timer_adjust_oneshot(cdic->audio_sample_timer, cdic->decode_period, 0);
    }
}

static UINT32 increment_cdda_frame_bcd(UINT32 bcd)
{
    UINT8 nybbles[6] =
    {
         bcd & 0x0000000f,
        (bcd & 0x000000f0) >> 4,
        (bcd & 0x00000f00) >> 8,
        (bcd & 0x0000f000) >> 12,
        (bcd & 0x000f0000) >> 16,
        (bcd & 0x00f00000) >> 20
    };
    nybbles[0]++;
    if(nybbles[0] == 5 && nybbles[1] == 7)
    {
        nybbles[0] = 0;
        nybbles[1] = 0;
        nybbles[2]++;
    }
    else if(nybbles[0] == 10)
    {
        nybbles[1]++;
    }
    if(nybbles[2] == 10)
    {
        nybbles[3]++;
        nybbles[2] = 0;
    }
    if(nybbles[3] == 6)
    {
        nybbles[4]++;
        nybbles[3] = 0;
    }
    if(nybbles[4] == 10)
    {
        nybbles[5]++;
        nybbles[4] = 0;
    }
    return (nybbles[5] << 20) | (nybbles[4] << 16) | (nybbles[3] << 12) | (nybbles[2] << 8) | (nybbles[1] << 4) | nybbles[0];
}

static UINT32 increment_cdda_sector_bcd(UINT32 bcd)
{
    UINT8 nybbles[6] =
    {
         bcd & 0x0000000f,
        (bcd & 0x000000f0) >> 4,
        (bcd & 0x00000f00) >> 8,
        (bcd & 0x0000f000) >> 12,
        (bcd & 0x000f0000) >> 16,
        (bcd & 0x00f00000) >> 20
    };
    nybbles[2]++;
    if(nybbles[2] == 10)
    {
        nybbles[3]++;
        nybbles[2] = 0;
    }
    if(nybbles[3] == 6)
    {
        nybbles[4]++;
        nybbles[3] = 0;
    }
    if(nybbles[4] == 10)
    {
        nybbles[5]++;
        nybbles[4] = 0;
    }
    return (nybbles[5] << 20) | (nybbles[4] << 16) | (nybbles[3] << 12) | (nybbles[2] << 8) | (nybbles[1] << 4) | nybbles[0];
}

static TIMER_CALLBACK( cdic_trigger_readback_int )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;

    switch(cdic->command)
    {
        case 0x23: // Reset Mode 1
        case 0x24: // Reset Mode 2
        case 0x29: // Read Mode 1
        case 0x2a: // Read Mode 2
        //case 0x2c: // Seek
        {
            UINT8 buffer[2560] = { 0 };
            UINT32 msf = cdic->time >> 8;
            UINT32 lba = 0;
            int index = 0;
            UINT8 nybbles[6] =
            {
                 msf & 0x0000000f,
                (msf & 0x000000f0) >> 4,
                (msf & 0x00000f00) >> 8,
                (msf & 0x0000f000) >> 12,
                (msf & 0x000f0000) >> 16,
                (msf & 0x00f00000) >> 20
            };
            if(msf & 0x000080)
            {
                msf &= 0xffff00;
                nybbles[0] = 0;
                nybbles[1] = 0;
            }
            if(nybbles[2] >= 2)
            {
                nybbles[2] -= 2;
            }
            else
            {
                nybbles[2] = 8 + nybbles[2];
                if(nybbles[3] > 0)
                {
                    nybbles[3]--;
                }
                else
                {
                    nybbles[3] = 5;
                    if(nybbles[4] > 0)
                    {
                        nybbles[4]--;
                    }
                    else
                    {
                        nybbles[4] = 9;
                        nybbles[5]--;
                    }
                }
            }
            lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

            //printf( "Reading Mode %d sector from MSF location %06x\n", cdic->command - 0x28, cdic->time | 2 );
            verboselog(machine, 0, "Reading Mode %d sector from MSF location %06x\n", cdic->command - 0x28, cdic->time | 2 );


            cdrom_read_data(cdic->cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

            cdic->time += 0x100;
            if((cdic->time & 0x00000f00) == 0x00000a00)
            {
                cdic->time &= 0xfffff0ff;
                cdic->time += 0x00001000;
            }
            if((cdic->time & 0x0000ff00) == 0x00007500)
            {
                cdic->time &= 0xffff00ff;
                cdic->time += 0x00010000;
                if((cdic->time & 0x000f0000) == 0x000a0000)
                {
                    cdic->time &= 0xfff0ffff;
                    cdic->time += 0x00100000;
                }
            }
            if((cdic->time & 0x00ff0000) == 0x00600000)
            {
                cdic->time &= 0xff00ffff;
                cdic->time += 0x01000000;
                if((cdic->time & 0x0f000000) == 0x0a000000)
                {
                    cdic->time &= 0xf0ffffff;
                    cdic->time += 0x10000000;
                }
            }

            cdic->data_buffer &= ~0x0004;
            cdic->data_buffer ^= 0x0001;

            //printf( "%02x\n", buffer[CDIC_SECTOR_SUBMODE2] );
            if((buffer[CDIC_SECTOR_FILE2] << 8) == cdic->file)
            {
                //if((1 << buffer[CDIC_SECTOR_CHAN2]))
                {
                    if(((buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_FORM | CDIC_SUBMODE_DATA | CDIC_SUBMODE_AUDIO | CDIC_SUBMODE_VIDEO)) == (CDIC_SUBMODE_FORM | CDIC_SUBMODE_AUDIO)) &&
                       (cdic->channel & cdic->audio_channel & (1 << buffer[CDIC_SECTOR_CHAN2])))
                    {
                        {
                            verboselog(machine, 0, "Audio sector\n" );

                            cdic->x_buffer |= 0x8000;
                            //cdic->data_buffer |= 0x4000;
                            cdic->data_buffer |= 0x0004;

                            for(index = 6; index < 2352/2; index++)
                            {
                                cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
                            }

                            cdic_decode_audio_sector(machine, ((UINT8*)cdic->ram) + ((cdic->data_buffer & 5) * 0xa00 + 4), 0);

                            //printf( "Setting CDIC interrupt line\n" );
                            verboselog(machine, 0, "Setting CDIC interrupt line for audio sector\n" );
                            cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 128);
                            cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
                        }
                    }
                    else if((buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_DATA | CDIC_SUBMODE_AUDIO | CDIC_SUBMODE_VIDEO)) == 0x00)
                    {
                        cdic->x_buffer |= 0x8000;
                        //cdic->data_buffer |= 0x4000;

                        for(index = 6; index < 2352/2; index++)
                        {
                            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
                        }

                        if((buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_TRIG) == CDIC_SUBMODE_TRIG ||
                           (buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOR) == CDIC_SUBMODE_EOR ||
                           (buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOF) == CDIC_SUBMODE_EOF)
                        {
                            //printf( "Setting CDIC interrupt line\n" );
                            verboselog(machine, 0, "Setting CDIC interrupt line for message sector\n" );
                            cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 128);
                            cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
                        }
                        else
                        {
                            verboselog(machine, 0, "Message sector, ignored\n" );
                        }
                    }
                    else /*if(buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_DATA | CDIC_SUBMODE_VIDEO))*/
                    {
                        cdic->x_buffer |= 0x8000;
                        //cdic->data_buffer |= 0x4000;

                        for(index = 6; index < 2352/2; index++)
                        {
                            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
                        }

                        //printf( "Setting CDIC interrupt line\n" );
                        verboselog(machine, 0, "Setting CDIC interrupt line for data sector\n" );
                        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 128);
                        cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
                    }
                }

                if((buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOF) == 0 && cdic->command != 0x23)
                {
                    timer_adjust_oneshot(cdic->interrupt_timer, ATTOTIME_IN_HZ(75), 0); // 75Hz = 1x CD-ROM speed
                }
                else
                {
                    if(cdic->command == 0x23) // Mode 1 Reset
                    {
                        timer_adjust_oneshot(cdic->interrupt_timer, attotime_never, 0);
                    }
                }
            }

            break;
        }
        //case 0x24: // Mode 2 Reset
        case 0x2e: // Abort
            timer_adjust_oneshot(cdic->interrupt_timer, attotime_never, 0);
            //cdic->data_buffer &= ~4;
            break;
        case 0x28: // Play CDDA audio
        {
            UINT8 buffer[2560] = { 0 };
            int index = 0;
            UINT32 msf = (cdic->time & 0xffff7f00) >> 8;
            UINT32 next_msf = increment_cdda_frame_bcd((cdic->time & 0xffff7f00) >> 8);
            UINT32 rounded_next_msf = increment_cdda_sector_bcd((cdic->time & 0xffff0000) >> 8);
            UINT32 lba = 0;
            UINT32 next_lba = 0;
            UINT8 nybbles[6] =
            {
                 msf & 0x0000000f,
                (msf & 0x000000f0) >> 4,
                (msf & 0x00000f00) >> 8,
                (msf & 0x0000f000) >> 12,
                (msf & 0x000f0000) >> 16,
                (msf & 0x00f00000) >> 20
            };
            UINT8 next_nybbles[6] =
            {
                 rounded_next_msf & 0x0000000f,
                (rounded_next_msf & 0x000000f0) >> 4,
                (rounded_next_msf & 0x00000f00) >> 8,
                (rounded_next_msf & 0x0000f000) >> 12,
                (rounded_next_msf & 0x000f0000) >> 16,
                (rounded_next_msf & 0x00f00000) >> 20
            };

            lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

            cdrom_read_data(cdic->cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

            if(!(msf & 0x0000ff))
            {
                next_lba = next_nybbles[0] + next_nybbles[1]*10 + ((next_nybbles[2] + next_nybbles[3]*10)*75) + ((next_nybbles[4] + next_nybbles[5]*10)*75*60);

                verboselog(machine, 0, "Playing CDDA sector from MSF location %06x\n", cdic->time | 2 );

                cdda_start_audio(machine->device("cdda"), lba, rounded_next_msf);

            }

            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x924/2] = 0x0001;                              //  CTRL
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x926/2] = 0x0001;                              //  TRACK
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x928/2] = 0x0000;                              //  INDEX
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x92a/2] = (cdic->time >> 24) & 0x000000ff; //  MIN
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x92c/2] = (cdic->time >> 16) & 0x000000ff; //  SEC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x92e/2] = (cdic->time >>  8) & 0x0000007f; //  FRAC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x930/2] = 0x0000;                              //  ZERO
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x932/2] = (cdic->time >> 24) & 0x000000ff; //  AMIN
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x934/2] = (cdic->time >> 16) & 0x000000ff; //  ASEC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x936/2] = (cdic->time >>  8) & 0x0000007f; //  AFRAC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x938/2] = 0x0000;                              //  CRC1
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x93a/2] = 0x0000;                              //  CRC2

            cdic->time = next_msf << 8;

            timer_adjust_oneshot(cdic->interrupt_timer, ATTOTIME_IN_HZ(75), 0);

            cdic->x_buffer |= 0x8000;
            //cdic->data_buffer |= 0x4000;

            for(index = 6; index < 2352/2; index++)
            {
                cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
            }

            verboselog(machine, 0, "Setting CDIC interrupt line for CDDA sector\n" );
            cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 128);
            cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
            break;
        }
        case 0x2c: // Seek
        {
            UINT8 buffer[2560] = { 0 };
            int index = 0;
            UINT32 msf = (cdic->time & 0xffff7f00) >> 8;
            UINT32 next_msf = increment_cdda_frame_bcd((cdic->time & 0xffff7f00) >> 8);
            UINT32 lba = 0;
            UINT8 nybbles[6] =
            {
                 msf & 0x0000000f,
                (msf & 0x000000f0) >> 4,
                (msf & 0x00000f00) >> 8,
                (msf & 0x0000f000) >> 12,
                (msf & 0x000f0000) >> 16,
                (msf & 0x00f00000) >> 20
            };
            lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

            timer_adjust_oneshot(cdic->interrupt_timer, ATTOTIME_IN_HZ(75), 0);

            cdrom_read_data(cdic->cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

            cdic->data_buffer ^= 0x0001;
            cdic->x_buffer |= 0x8000;
            cdic->data_buffer |= 0x4000;

            for(index = 6; index < 2352/2; index++)
            {
                cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
            }

            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x924/2] = 0x0041;                              //  CTRL
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x926/2] = 0x0001;                              //  TRACK
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x928/2] = 0x0000;                              //  INDEX
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x92a/2] = (cdic->time >> 24) & 0x000000ff; //  MIN
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x92c/2] = (cdic->time >> 16) & 0x000000ff; //  SEC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x92e/2] = (cdic->time >>  8) & 0x0000007f; //  FRAC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x930/2] = 0x0000;                              //  ZERO
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x932/2] = (cdic->time >> 24) & 0x000000ff; //  AMIN
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x934/2] = (cdic->time >> 16) & 0x000000ff; //  ASEC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x936/2] = (cdic->time >>  8) & 0x0000007f; //  AFRAC
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x938/2] = 0x0000;                              //  CRC1
            cdic->ram[(cdic->data_buffer & 5) * (0xa00/2) + 0x93a/2] = 0x0000;                              //  CRC2

            cdic->time = next_msf << 8;

            verboselog(machine, 0, "Setting CDIC interrupt line for Seek sector\n" );
            cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 128);
            cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
            break;
        }
    }
}

static READ16_HANDLER( cdic_r )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;

    offset += 0x3c00/2;
    switch(offset)
    {
        case 0x3c00/2: // Command register
            verboselog(space->machine, 0, "cdic_r: Command Register = %04x & %04x\n", cdic->command, mem_mask);
            return cdic->command;

        case 0x3c02/2: // Time register (MSW)
            verboselog(space->machine, 0, "cdic_r: Time Register (MSW) = %04x & %04x\n", cdic->time >> 16, mem_mask);
            return cdic->time >> 16;

        case 0x3c04/2: // Time register (LSW)
            verboselog(space->machine, 0, "cdic_r: Time Register (LSW) = %04x & %04x\n", (UINT16)(cdic->time & 0x0000ffff), mem_mask);
            return cdic->time & 0x0000ffff;

        case 0x3c06/2: // File register
            verboselog(space->machine, 0, "cdic_r: File Register = %04x & %04x\n", cdic->file, mem_mask);
            return cdic->file;

        case 0x3c08/2: // Channel register (MSW)
            verboselog(space->machine, 0, "cdic_r: Channel Register (MSW) = %04x & %04x\n", cdic->channel >> 16, mem_mask);
            return cdic->channel >> 16;

        case 0x3c0a/2: // Channel register (LSW)
            verboselog(space->machine, 0, "cdic_r: Channel Register (LSW) = %04x & %04x\n", cdic->channel & 0x0000ffff, mem_mask);
            return cdic->channel & 0x0000ffff;

        case 0x3c0c/2: // Audio Channel register
            verboselog(space->machine, 0, "cdic_r: Audio Channel Register = %04x & %04x\n", cdic->audio_channel, mem_mask);
            return cdic->audio_channel;

        case 0x3ff4/2: // ABUF
        {
            UINT16 temp = cdic->audio_buffer;
            cdic->audio_buffer &= 0x7fff;
            if(!((cdic->audio_buffer | cdic->x_buffer) & 0x8000))
            {
                cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_4, CLEAR_LINE);
                verboselog(space->machine, 0, "Clearing CDIC interrupt line\n" );
                //printf("Clearing CDIC interrupt line\n" );
            }
            verboselog(space->machine, 0, "cdic_r: Audio Buffer Register = %04x & %04x\n", temp, mem_mask);
            return temp;
        }

        case 0x3ff6/2: // XBUF
        {
            UINT16 temp = cdic->x_buffer;
            cdic->x_buffer &= 0x7fff;
            if(!((cdic->audio_buffer | cdic->x_buffer) & 0x8000))
            {
                cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_4, CLEAR_LINE);
                verboselog(space->machine, 0, "Clearing CDIC interrupt line\n" );
                //printf("Clearing CDIC interrupt line\n" );
            }
            verboselog(space->machine, 0, "cdic_r: X-Buffer Register = %04x & %04x\n", temp, mem_mask);
            return temp;
        }

        case 0x3ffa/2: // AUDCTL
        {
            if(attotime_is_never(timer_timeleft(cdic->audio_sample_timer)))
            {
                cdic->z_buffer ^= 0x0001;
            }
            verboselog(space->machine, 0, "cdic_r: Z-Buffer Register = %04x & %04x\n", cdic->z_buffer, mem_mask);
            return cdic->z_buffer;
        }

        case 0x3ffe/2:
        {
            verboselog(space->machine, 0, "cdic_r: Data buffer Register = %04x & %04x\n", cdic->data_buffer, mem_mask);
            return cdic->data_buffer;
        }
        default:
            verboselog(space->machine, 0, "cdic_r: UNIMPLEMENTED: Unknown address: %04x & %04x\n", offset*2, mem_mask);
            return 0;
    }
}

static WRITE16_HANDLER( cdic_w )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;

    offset += 0x3c00/2;
    switch(offset)
    {
        case 0x3c00/2: // Command register
            verboselog(space->machine, 0, "cdic_w: Command Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->command);
            //printf( "cdic command: %04x\n", cdic->command );
            break;

        case 0x3c02/2: // Time register (MSW)
            cdic->time &= ~(mem_mask << 16);
            cdic->time |= (data & mem_mask) << 16;
            verboselog(space->machine, 0, "cdic_w: Time Register (MSW) = %04x & %04x\n", data, mem_mask);
            break;

        case 0x3c04/2: // Time register (LSW)
            cdic->time &= ~mem_mask;
            cdic->time |= data & mem_mask;
            verboselog(space->machine, 0, "cdic_w: Time Register (LSW) = %04x & %04x\n", data, mem_mask);
            break;

        case 0x3c06/2: // File register
            verboselog(space->machine, 0, "cdic_w: File Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->file);
            break;

        case 0x3c08/2: // Channel register (MSW)
            cdic->channel &= ~(mem_mask << 16);
            cdic->channel |= (data & mem_mask) << 16;
            verboselog(space->machine, 0, "cdic_w: Channel Register (MSW) = %04x & %04x\n", data, mem_mask);
            break;

        case 0x3c0a/2: // Channel register (LSW)
            cdic->channel &= ~mem_mask;
            cdic->channel |= data & mem_mask;
            verboselog(space->machine, 0, "cdic_w: Channel Register (LSW) = %04x & %04x\n", data, mem_mask);
            break;

        case 0x3c0c/2: // Audio Channel register
            verboselog(space->machine, 0, "cdic_w: Audio Channel Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->audio_channel);
            break;

        case 0x3ff4/2:
            verboselog(space->machine, 0, "cdic_w: Audio Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->audio_buffer);
            break;

        case 0x3ff6/2:
            verboselog(space->machine, 0, "cdic_w: X Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->x_buffer);
            break;

        case 0x3ff8/2:
        {
            scc68070_regs_t *scc68070 = &state->scc68070_regs;
            UINT32 start = scc68070->dma.channel[0].memory_address_counter;
            UINT32 count = scc68070->dma.channel[0].transfer_counter;
            UINT32 index = 0;
            UINT32 device_index = (data & 0x3fff) >> 1;
            UINT16 *memory = state->planea;
            verboselog(space->machine, 0, "memory address counter: %08x\n", scc68070->dma.channel[0].memory_address_counter);
            verboselog(space->machine, 0, "cdic_w: DMA Control Register = %04x & %04x\n", data, mem_mask);
            verboselog(space->machine, 0, "Doing copy, transferring %04x bytes\n", count * 2 );
            //printf("Doing copy, transferring %04x bytes\n", count * 2 );
            if((start & 0x00f00000) == 0x00200000)
            {
                start -= 0x00200000;
                memory = state->planeb;
            }
            for(index = start / 2; index < (start / 2 + count); index++)
            {
                if(scc68070->dma.channel[0].operation_control & OCR_D)
                {
                    memory[index] = cdic->ram[device_index++];
                }
                else
                {
                    cdic->ram[device_index++] = memory[index];
                }
            }
            scc68070->dma.channel[0].memory_address_counter += scc68070->dma.channel[0].transfer_counter * 2;
            break;
        }

        case 0x3ffa/2:
        {
            verboselog(space->machine, 0, "cdic_w: Z-Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->z_buffer);
            if(cdic->z_buffer & 0x2000)
            {
                attotime period = timer_timeleft(cdic->audio_sample_timer);
                if(attotime_is_never(period))
                {
                    cdic->decode_addr = cdic->z_buffer & 0x3a00;
                    cdic->decode_delay = 1;
                    timer_adjust_oneshot(cdic->audio_sample_timer, ATTOTIME_IN_HZ(75), 0);
                }
            }
            else
            {
                cdic->decode_addr = 0xffff;
                timer_adjust_oneshot(cdic->audio_sample_timer, attotime_never, 0);
            }
            break;
        }
        case 0x3ffc/2:
            verboselog(space->machine, 0, "cdic_w: Interrupt Vector Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->interrupt_vector);
            break;
        case 0x3ffe/2:
        {
            verboselog(space->machine, 0, "cdic_w: Data Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&cdic->data_buffer);
            if(cdic->data_buffer & 0x8000)
            {
                switch(cdic->command)
                {
                    //case 0x24: // Reset Mode 2
                    case 0x2e: // Abort
                    {
                        timer_adjust_oneshot(cdic->interrupt_timer, attotime_never, 0);
                        dmadac_enable(&state->dmadac[0], 2, 0);
                        //cdic->data_buffer &= 0xbfff;
                        break;
                    }
                    case 0x2b: // Stop CDDA
                        cdda_stop_audio(space->machine->device("cdda"));
                        timer_adjust_oneshot(cdic->interrupt_timer, attotime_never, 0);
                        break;
                    case 0x23: // Reset Mode 1
                    case 0x29: // Read Mode 1
                    case 0x2a: // Read Mode 2
                    case 0x28: // Play CDDA
                    case 0x2c: // Seek
                    {
                        attotime period = timer_timeleft(cdic->interrupt_timer);
                        if(!attotime_is_never(period))
                        {
                            timer_adjust_oneshot(cdic->interrupt_timer, period, 0);
                        }
                        else
                        {
                            if(cdic->command != 0x23 && cdic->command != 0x24)
                            {
                                timer_adjust_oneshot(cdic->interrupt_timer, ATTOTIME_IN_HZ(75), 0);
                            }
                        }
                        break;
                    }
                    default:
                        verboselog(space->machine, 0, "Unknown CDIC command: %02x\n", cdic->command );
                        break;
                }
            }
            cdic->data_buffer &= 0x7fff;
            break;
        }
        default:
            verboselog(space->machine, 0, "cdic_w: UNIMPLEMENTED: Unknown address: %04x = %04x & %04x\n", offset*2, data, mem_mask);
            break;
    }
}

static void cdic_init(running_machine *machine, cdic_regs_t *cdic)
{
    cdic->command = 0;
    cdic->time = 0;
    cdic->file = 0;
    cdic->channel = 0xffffffff;
    cdic->audio_channel = 0xffff;
    cdic->audio_buffer = 0;
    cdic->x_buffer = 0;
    cdic->dma_control = 0;
    cdic->z_buffer = 0;
    cdic->interrupt_vector = 0;
    cdic->data_buffer = 0;

    cdic->audio_sample_freq = 0;
    cdic->audio_sample_size = 0;

    cdic->decode_addr = 0;
    cdic->decode_delay = 0;
}

static void cdic_register_globals(running_machine *machine, cdic_regs_t *cdic)
{
    state_save_register_global(machine, cdic->command);
    state_save_register_global(machine, cdic->time);
    state_save_register_global(machine, cdic->file);
    state_save_register_global(machine, cdic->channel);
    state_save_register_global(machine, cdic->audio_channel);
    state_save_register_global(machine, cdic->audio_buffer);
    state_save_register_global(machine, cdic->x_buffer);
    state_save_register_global(machine, cdic->dma_control);
    state_save_register_global(machine, cdic->z_buffer);
    state_save_register_global(machine, cdic->interrupt_vector);
    state_save_register_global(machine, cdic->data_buffer);

    state_save_register_global(machine, cdic->audio_sample_freq);
    state_save_register_global(machine, cdic->audio_sample_size);
}

static TIMER_CALLBACK( slave_trigger_readback_int )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    slave_regs_t *slave = &state->slave_regs;

    verboselog(machine, 0, "Asserting IRQ2\n" );
    cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_2, 26);
    cputag_set_input_line(machine, "maincpu", M68K_IRQ_2, ASSERT_LINE);
    timer_adjust_oneshot(slave->interrupt_timer, attotime_never, 0);
}

static void slave_prepare_readback(running_machine *machine, attotime delay, UINT8 channel, UINT8 count, UINT8 data0, UINT8 data1, UINT8 data2, UINT8 data3, UINT8 cmd)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    slave_regs_t *slave = &state->slave_regs;

    slave->channel[channel].out_index = 0;
    slave->channel[channel].out_count = count;
    slave->channel[channel].out_buf[0] = data0;
    slave->channel[channel].out_buf[1] = data1;
    slave->channel[channel].out_buf[2] = data2;
    slave->channel[channel].out_buf[3] = data3;
    slave->channel[channel].out_cmd = cmd;

    timer_adjust_oneshot(slave->interrupt_timer, delay, 0);
}

static void perform_mouse_update(running_machine *machine)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    slave_regs_t *slave = &state->slave_regs;
    UINT16 x = input_port_read(machine, "MOUSEX");
    UINT16 y = input_port_read(machine, "MOUSEY");
    UINT8 buttons = input_port_read(machine, "MOUSEBTN");

    UINT16 old_mouse_x = slave->real_mouse_x;
    UINT16 old_mouse_y = slave->real_mouse_y;

    if(slave->real_mouse_x == 0xffff)
    {
        old_mouse_x = x & 0x3ff;
        old_mouse_y = y & 0x3ff;
    }

    slave->real_mouse_x = x & 0x3ff;
    slave->real_mouse_y = y & 0x3ff;

    slave->fake_mouse_x += (slave->real_mouse_x - old_mouse_x);
    slave->fake_mouse_y += (slave->real_mouse_y - old_mouse_y);

    while(slave->fake_mouse_x > 0x3ff)
    {
        slave->fake_mouse_x += 0x400;
    }

    while(slave->fake_mouse_y > 0x3ff)
    {
        slave->fake_mouse_y += 0x400;
    }

    x = slave->fake_mouse_x;
    y = slave->fake_mouse_y;

    if(slave->polling_active)
    {
        slave_prepare_readback(machine, attotime_zero, 0, 4, ((x & 0x380) >> 7) | (buttons << 4), x & 0x7f, (y & 0x380) >> 7, y & 0x7f, 0xf7);
    }
}

static INPUT_CHANGED( mouse_update )
{
    perform_mouse_update(field->port->machine);
}

static READ16_HANDLER( slave_r )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    slave_regs_t *slave = &state->slave_regs;

    if(slave->channel[offset].out_count)
    {
        UINT8 ret = slave->channel[offset].out_buf[slave->channel[offset].out_index];
        verboselog(space->machine, 0, "slave_r: Channel %d: %d, %02x\n", offset, slave->channel[offset].out_index, ret );
        if(slave->channel[offset].out_index == 0)
        {
            switch(slave->channel[offset].out_cmd)
            {
                case 0xb0:
                case 0xb1:
                case 0xf0:
                case 0xf3:
                case 0xf7:
                    verboselog(space->machine, 0, "slave_r: De-asserting IRQ2\n" );
                    cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_2, CLEAR_LINE);
                    break;
            }
        }
        slave->channel[offset].out_index++;
        slave->channel[offset].out_count--;
        if(!slave->channel[offset].out_count)
        {
            slave->channel[offset].out_index = 0;
            slave->channel[offset].out_cmd = 0;
            memset(slave->channel[offset].out_buf, 0, 4);
        }
        return ret;
    }
    verboselog(space->machine, 0, "slave_r: Channel %d: %d\n", offset, slave->channel[offset].out_index );
    return 0xff;
}

static void set_mouse_position(running_machine* machine)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    slave_regs_t *slave = &state->slave_regs;
    UINT16 x, y;

    //printf( "Set mouse position: %02x %02x %02x\n", slave->in_buf[0], slave->in_buf[1], slave->in_buf[2] );

    slave->fake_mouse_y = ((slave->in_buf[1] & 0x0f) << 6) | (slave->in_buf[0] & 0x3f);
    slave->fake_mouse_x = ((slave->in_buf[1] & 0x70) << 3) | slave->in_buf[2];

    x = slave->fake_mouse_x;
    y = slave->fake_mouse_y;

    if(slave->polling_active)
    {
        //slave_prepare_readback(machine, attotime_zero, 0, 4, (x & 0x380) >> 7, x & 0x7f, (y & 0x380) >> 7, y & 0x7f, 0xf7);
    }
}

static WRITE16_HANDLER( slave_w )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    slave_regs_t *slave = &state->slave_regs;

    switch(offset)
    {
        case 0:
            if(slave->in_index)
            {
                verboselog(space->machine, 0, "slave_w: Channel %d: %d = %02x\n", offset, slave->in_index, data & 0x00ff );
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                if(slave->in_index == slave->in_count)
                {
                    switch(slave->in_buf[0])
                    {
                        case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
                        case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
                        case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
                        case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
                        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
                        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
                        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
                        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: // Update Mouse Position
                            set_mouse_position(space->machine);
                            memset(slave->in_buf, 0, 17);
                            slave->in_index = 0;
                            slave->in_count = 0;
                            break;
                    }
                }
            }
            else
            {
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                switch(data & 0x00ff)
                {
                    case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
                    case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
                    case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
                    case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
                    case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
                    case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
                    case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
                    case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                        verboselog(space->machine, 0, "slave_w: Channel %d: Update Mouse Position (0x%02x)\n", offset, data & 0x00ff );
                        slave->in_count = 3;
                        break;
                    default:
                        verboselog(space->machine, 0, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff );
                        slave->in_index = 0;
                        break;
                }
            }
            break;
        case 1:
            if(slave->in_index)
            {
                verboselog(space->machine, 0, "slave_w: Channel %d: %d = %02x\n", offset, slave->in_index, data & 0x00ff );
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                if(slave->in_index == slave->in_count)
                {
                    switch(slave->in_buf[0])
                    {
                        case 0xf0: // Set Front Panel LCD
                            memcpy(slave->lcd_state, slave->in_buf + 1, 16);
                            memset(slave->in_buf, 0, 17);
                            slave->in_index = 0;
                            slave->in_count = 0;
                            break;
                        default:
                            memset(slave->in_buf, 0, 17);
                            slave->in_index = 0;
                            slave->in_count = 0;
                            break;
                    }
                }
            }
            else
            {
                switch(data & 0x00ff)
                {
                    default:
                        verboselog(space->machine, 0, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff );
                        memset(slave->in_buf, 0, 17);
                        slave->in_index = 0;
                        slave->in_count = 0;
                        break;
                }
            }
            break;
        case 2:
            if(slave->in_index)
            {
                verboselog(space->machine, 0, "slave_w: Channel %d: %d = %02x\n", offset, slave->in_index, data & 0x00ff );
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                if(slave->in_index == slave->in_count)
                {
                    switch(slave->in_buf[0])
                    {
                        case 0xf0: // Set Front Panel LCD
                            memset(slave->in_buf + 1, 0, 16);
                            slave->in_count = 17;
                            break;
                        default:
                            memset(slave->in_buf, 0, 17);
                            slave->in_index = 0;
                            slave->in_count = 0;
                            break;
                    }
                }
            }
            else
            {
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                switch(data & 0x00ff)
                {
                    case 0x82: // Mute audio
                        verboselog(space->machine, 0, "slave_w: Channel %d: Mute Audio (0x82)\n", offset );
                        dmadac_enable(&state->dmadac[0], 2, 0);
                        slave->in_index = 0;
                        slave->in_count = 0;
                        //timer_adjust_oneshot(cdic->audio_sample_timer, attotime_never, 0);
                        break;
                    case 0x83: // Unmute audio
                        verboselog(space->machine, 0, "slave_w: Channel %d: Unmute Audio (0x83)\n", offset );
                        dmadac_enable(&state->dmadac[0], 2, 1);
                        slave->in_index = 0;
                        slave->in_count = 0;
                        break;
                    case 0xf0: // Set Front Panel LCD
                        verboselog(space->machine, 0, "slave_w: Channel %d: Set Front Panel LCD (0xf0)\n", offset );
                        slave->in_count = 17;
                        break;
                    default:
                        verboselog(space->machine, 0, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff );
                        memset(slave->in_buf, 0, 17);
                        slave->in_index = 0;
                        slave->in_count = 0;
                        break;
                }
            }
            break;
        case 3:
            if(slave->in_index)
            {
                verboselog(space->machine, 0, "slave_w: Channel %d: %d = %02x\n", offset, slave->in_index, data & 0x00ff );
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                if(slave->in_index == slave->in_count)
                {
                    switch(slave->in_buf[0])
                    {
                        case 0xb0: // Request Disc Status
                            memset(slave->in_buf, 0, 17);
                            slave->in_index = 0;
                            slave->in_count = 0;
                            slave_prepare_readback(space->machine, ATTOTIME_IN_HZ(4), 3, 4, 0xb0, 0x00, 0x02, 0x15, 0xb0);
                            break;
                        //case 0xb1: // Request Disc Base
                            //memset(slave->in_buf, 0, 17);
                            //slave->in_index = 0;
                            //slave->in_count = 0;
                            //slave_prepare_readback(space->machine, ATTOTIME_IN_HZ(10000), 3, 4, 0xb1, 0x00, 0x00, 0x00, 0xb1);
                            //break;
                        default:
                            memset(slave->in_buf, 0, 17);
                            slave->in_index = 0;
                            slave->in_count = 0;
                            break;
                    }
                }
            }
            else
            {
                slave->in_buf[slave->in_index] = data & 0x00ff;
                slave->in_index++;
                switch(data & 0x00ff)
                {
                    case 0xb0: // Request Disc Status
                        verboselog(space->machine, 0, "slave_w: Channel %d: Request Disc Status (0xb0)\n", offset );
                        slave->in_count = 4;
                        break;
                    case 0xb1: // Request Disc Status
                        verboselog(space->machine, 0, "slave_w: Channel %d: Request Disc Base (0xb1)\n", offset );
                        slave->in_count = 4;
                        break;
                    case 0xf0: // Get SLAVE revision
                        verboselog(space->machine, 0, "slave_w: Channel %d: Get SLAVE Revision (0xf0)\n", offset );
                        slave_prepare_readback(space->machine, ATTOTIME_IN_HZ(10000), 2, 2, 0xf0, 0x32, 0x31, 0, 0xf0);
                        slave->in_index = 0;
                        break;
                    case 0xf3: // Query Pointer Type
                        verboselog(space->machine, 0, "slave_w: Channel %d: Query Pointer Type (0xf3)\n", offset );
                        slave->in_index = 0;
                        slave_prepare_readback(space->machine, ATTOTIME_IN_HZ(10000), 2, 2, 0xf3, 1, 0, 0, 0xf3);
                        break;
                    case 0xf6: // NTSC/PAL
                        verboselog(space->machine, 0, "slave_w: Channel %d: Check NTSC/PAL (0xf6)\n", offset );
                        slave_prepare_readback(space->machine, attotime_never, 2, 2, 0xf6, 1, 0, 0, 0xf6);
                        slave->in_index = 0;
                        break;
                    case 0xf7: // Activate input polling
                        verboselog(space->machine, 0, "slave_w: Channel %d: Activate Input Polling (0xf7)\n", offset );
                        slave->polling_active = 1;
                        slave->in_index = 0;
                        break;
                    case 0xfa: // Enable X-Bus interrupts
                        verboselog(space->machine, 0, "slave_w: Channel %d: X-Bus Interrupt Enable (0xfa)\n", offset );
                        slave->xbus_interrupt_enable = 1;
                        slave->in_index = 0;
                        break;
                    default:
                        verboselog(space->machine, 0, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff );
                        memset(slave->in_buf, 0, 17);
                        slave->in_index = 0;
                        slave->in_count = 0;
                        break;
                }
            }
            break;
    }
}

static void slave_init(running_machine *machine, slave_regs_t *slave)
{
    int index = 0;
    for(index = 0; index < 4; index++)
    {
        slave->channel[index].out_buf[0] = 0;
        slave->channel[index].out_buf[1] = 0;
        slave->channel[index].out_buf[2] = 0;
        slave->channel[index].out_buf[3] = 0;
        slave->channel[index].out_index = 0;
        slave->channel[index].out_count = 0;
        slave->channel[index].out_cmd = 0;
    }

    memset(slave->in_buf, 0, 17);
    slave->in_index = 0;
    slave->in_count = 0;

    slave->polling_active = 0;

    slave->xbus_interrupt_enable = 0;

    memset(slave->lcd_state, 0, 16);

    slave->real_mouse_x = 0;
    slave->real_mouse_y = 0;

    slave->fake_mouse_x = 0;
    slave->fake_mouse_y = 0;
}

static void slave_register_globals(running_machine *machine, slave_regs_t *slave)
{
    state_save_register_global(machine, slave->channel[0].out_buf[0]);
    state_save_register_global(machine, slave->channel[0].out_buf[1]);
    state_save_register_global(machine, slave->channel[0].out_buf[2]);
    state_save_register_global(machine, slave->channel[0].out_buf[3]);
    state_save_register_global(machine, slave->channel[0].out_index);
    state_save_register_global(machine, slave->channel[0].out_count);
    state_save_register_global(machine, slave->channel[0].out_cmd);
    state_save_register_global(machine, slave->channel[1].out_buf[0]);
    state_save_register_global(machine, slave->channel[1].out_buf[1]);
    state_save_register_global(machine, slave->channel[1].out_buf[2]);
    state_save_register_global(machine, slave->channel[1].out_buf[3]);
    state_save_register_global(machine, slave->channel[1].out_index);
    state_save_register_global(machine, slave->channel[1].out_count);
    state_save_register_global(machine, slave->channel[1].out_cmd);
    state_save_register_global(machine, slave->channel[2].out_buf[0]);
    state_save_register_global(machine, slave->channel[2].out_buf[1]);
    state_save_register_global(machine, slave->channel[2].out_buf[2]);
    state_save_register_global(machine, slave->channel[2].out_buf[3]);
    state_save_register_global(machine, slave->channel[2].out_index);
    state_save_register_global(machine, slave->channel[2].out_count);
    state_save_register_global(machine, slave->channel[2].out_cmd);
    state_save_register_global(machine, slave->channel[3].out_buf[0]);
    state_save_register_global(machine, slave->channel[3].out_buf[1]);
    state_save_register_global(machine, slave->channel[3].out_buf[2]);
    state_save_register_global(machine, slave->channel[3].out_buf[3]);
    state_save_register_global(machine, slave->channel[3].out_index);
    state_save_register_global(machine, slave->channel[3].out_count);
    state_save_register_global(machine, slave->channel[3].out_cmd);

    state_save_register_global_array(machine, slave->in_buf);
    state_save_register_global(machine, slave->in_index);
    state_save_register_global(machine, slave->in_count);

    state_save_register_global(machine, slave->polling_active);

    state_save_register_global(machine, slave->xbus_interrupt_enable);

    state_save_register_global_array(machine, slave->lcd_state);

    state_save_register_global(machine, slave->real_mouse_x);
    state_save_register_global(machine, slave->real_mouse_y);

    state_save_register_global(machine, slave->fake_mouse_x);
    state_save_register_global(machine, slave->fake_mouse_y);
}

/*************************
*     Video Hardware     *
*************************/

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

static void cdi220_draw_lcd(running_machine *machine, int y)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    bitmap_t *bitmap = state->lcdbitmap;
    UINT32 *scanline = BITMAP_ADDR32(bitmap, y, 0);
    int x = 0;
    int lcd = 0;

    for(lcd = 0; lcd < 8; lcd++)
    {
        UINT16 data = (state->slave_regs.lcd_state[lcd*2] << 8) | state->slave_regs.lcd_state[lcd*2 + 1];
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

static READ16_HANDLER(mcd212_r)
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    UINT8 channel = 1 - (offset / 8);

    switch(offset)
    {
        case 0x00/2:
        case 0x10/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 12, "mcd212_r: Status Register %d: %02x & %04x\n", channel + 1, mcd212->channel[1 - (offset / 8)].csrr, mem_mask);
                if(channel == 0)
                {
                    return mcd212->channel[0].csrr;
                }
                else
                {
                    UINT8 old_csr = mcd212->channel[1].csrr;
                    UINT8 interrupt1 = (state->scc68070_regs.lir >> 4) & 7;
                    //UINT8 interrupt2 = state->scc68070_regs.lir & 7;
                    mcd212->channel[1].csrr &= ~(MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2);
                    if(interrupt1)
                    {
                        cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_1 + (interrupt1 - 1), CLEAR_LINE);
                    }
                    //if(interrupt2)
                    //{
                    //  cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_1 + (interrupt2 - 1), CLEAR_LINE);
                    //}
                    return old_csr;
                }
            }
            else
            {
                verboselog(space->machine, 2, "mcd212_r: Unknown Register %d: %04x\n", channel + 1, mem_mask);
            }
            break;
        case 0x02/2:
        case 0x12/2:
            verboselog(space->machine, 2, "mcd212_r: Display Command Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].dcr, mem_mask);
            return mcd212->channel[1 - (offset / 8)].dcr;
        case 0x04/2:
        case 0x14/2:
            verboselog(space->machine, 2, "mcd212_r: Video Start Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].vsr, mem_mask);
            return mcd212->channel[1 - (offset / 8)].vsr;
        case 0x08/2:
        case 0x18/2:
            verboselog(space->machine, 2, "mcd212_r: Display Decoder Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].ddr, mem_mask);
            return mcd212->channel[1 - (offset / 8)].ddr;
        case 0x0a/2:
        case 0x1a/2:
            verboselog(space->machine, 2, "mcd212_r: DCA Pointer Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, mcd212->channel[1 - (offset / 8)].dcp, mem_mask);
            return mcd212->channel[1 - (offset / 8)].dcp;
        default:
            verboselog(space->machine, 2, "mcd212_r: Unknown Register %d & %04x\n", (1 - (offset / 8)) + 1, mem_mask);
            break;
    }

    return 0;
}

static void mcd212_update_visible_area(running_machine *machine)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    rectangle visarea = machine->primary_screen->visible_area();
    attoseconds_t period = machine->primary_screen->frame_period().attoseconds;
    int width = 0;

    if((mcd212->channel[0].dcr & (MCD212_DCR_CF | MCD212_DCR_FD)) && (mcd212->channel[0].csrw & MCD212_CSR1W_ST))
    {
        width = 360;
    }
    else
    {
        width = 384;
    }

    visarea.max_x = width-1;

    machine->primary_screen->configure(width, 262, visarea, period);
}

static WRITE16_HANDLER(mcd212_w)
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;

    switch(offset)
    {
        case 0x00/2:
        case 0x10/2:
            verboselog(space->machine, 2, "mcd212_w: Status Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].csrw);
            mcd212_update_visible_area(space->machine);
            break;
        case 0x02/2:
        case 0x12/2:
            verboselog(space->machine, 2, "mcd212_w: Display Command Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].dcr);
            mcd212_update_visible_area(space->machine);
            break;
        case 0x04/2:
        case 0x14/2:
            verboselog(space->machine, 2, "mcd212_w: Video Start Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].vsr);
            break;
        case 0x08/2:
        case 0x18/2:
            verboselog(space->machine, 2, "mcd212_w: Display Decoder Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].ddr);
            break;
        case 0x0a/2:
        case 0x1a/2:
            verboselog(space->machine, 2, "mcd212_w: DCA Pointer Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            COMBINE_DATA(&mcd212->channel[1 - (offset / 8)].dcp);
            break;
        default:
            verboselog(space->machine, 2, "mcd212_w: Unknown Register %d: %04x & %04x\n", (1 - (offset / 8)) + 1, data, mem_mask);
            break;
    }
}

static void mcd212_set_register(running_machine *machine, int channel, UINT8 reg, UINT32 value)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;

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
            mcd212_update_region_arrays(machine);
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
                mcd212_update_region_arrays(machine);
            }
            break;
        case 0xdc: // Weight Factor B
            if(channel == 1)
            {
                verboselog(machine, 6, "          %04xxxxx: %d: Weight Factor B = %08x\n", channel * 0x20, channel, value );
                memset(mcd212->channel[channel].weight_factor_b, value & 0x000000ff, 768);
                mcd212_update_region_arrays(machine);
            }
            break;
    }
}

static void mcd212_set_vsr(mcd212_regs_t *mcd212, int channel, UINT32 value)
{
    mcd212->channel[channel].vsr = value & 0x0000ffff;
    mcd212->channel[channel].dcr &= 0xffc0;
    mcd212->channel[channel].dcr |= (value >> 16) & 0x003f;
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

static void mcd212_process_ica(running_machine *machine, int channel)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    UINT16 *ica = channel ? state->planeb : state->planea;
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
                    UINT8 interrupt = (state->scc68070_regs.lir >> 4) & 7;
                    if(interrupt)
                    {
                        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
                        cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#if 0
                if(mcd212->channel[1].csrr & MCD212_CSR2R_IT2)
                {
                    UINT8 interrupt = state->scc68070_regs.lir & 7;
                    if(interrupt)
                    {
                        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 24 + interrupt);
                        cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
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

static void mcd212_process_dca(running_machine *machine, int channel)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    UINT16 *dca = channel ? state->planeb : state->planea;
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
                mcd212_set_dcp(&state->mcd212_regs, channel, cmd & 0x001fffff);
                addr = (cmd & 0x0007ffff) / 2;
                addr_changed = 1;
                stop = 1;
                break;
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // RELOAD VSR
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: RELOAD VSR\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_vsr(&state->mcd212_regs, channel, cmd & 0x001fffff);
                break;
            case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // RELOAD VSR and STOP
            case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: RELOAD VSR and STOP\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_vsr(&state->mcd212_regs, channel, cmd & 0x001fffff);
                stop = 1;
                break;
            case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // INTERRUPT
            case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
                verboselog(machine, 11, "%08x: %08x: DCA %d: INTERRUPT\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212->channel[1].csrr |= 1 << (2 - channel);
                if(mcd212->channel[1].csrr & (MCD212_CSR2R_IT1 | MCD212_CSR2R_IT2))
                {
                    UINT8 interrupt = (state->scc68070_regs.lir >> 4) & 7;
                    if(interrupt)
                    {
                        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
                        cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#if 0
                if(mcd212->channel[1].csrr & MCD212_CSR2R_IT2)
                {
                    UINT8 interrupt = state->scc68070_regs.lir & 7;
                    if(interrupt)
                    {
                        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 24 + interrupt);
                        cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
                    }
                }
#endif
                break;
            case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // RELOAD DISPLAY PARAMETERS
                verboselog(machine, 6, "%08x: %08x: DCA %d: RELOAD DISPLAY PARAMETERS\n", addr * 2 + channel * 0x200000, cmd, channel );
                mcd212_set_display_parameters(&state->mcd212_regs, channel, cmd & 0x1f);
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

static void mcd212_update_region_arrays(running_machine *machine)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
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

static int mcd212_get_screen_width(running_machine *machine)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;

    if((mcd212->channel[0].dcr & (MCD212_DCR_CF | MCD212_DCR_FD)) && (mcd212->channel[0].csrw & MCD212_CSR1W_ST))
    {
        return 720;
    }
    return 768;
}

static void mcd212_process_vsr(running_machine *machine, int channel, UINT8 *pixels_r, UINT8 *pixels_g, UINT8 *pixels_b)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    UINT8 *data = channel ? (UINT8*)state->planeb : (UINT8*)state->planea;
    UINT32 vsr = mcd212_get_vsr(mcd212, channel) & 0x0007ffff;
    UINT8 done = 0;
    int x = 0;
    UINT32 icm_mask = channel ? MCD212_ICM_MODE2 : MCD212_ICM_MODE1;
    UINT32 icm_shift = channel ? MCD212_ICM_MODE2_SHIFT : MCD212_ICM_MODE1_SHIFT;
    UINT8 icm = (mcd212->channel[0].image_coding_method & icm_mask) >> icm_shift;
    UINT8 *clut_r = mcd212->channel[0].clut_r;
    UINT8 *clut_g = mcd212->channel[0].clut_g;
    UINT8 *clut_b = mcd212->channel[0].clut_b;
    UINT8 mosaic_enable = ((mcd212->channel[channel].ddr & MCD212_DDR_FT) == MCD212_DDR_FT_MOSAIC);
    UINT8 mosaic_factor = 1 << (((mcd212->channel[channel].ddr & MCD212_DDR_MT) >> MCD212_DDR_MT_SHIFT) + 1);
    int mosaic_index = 0;
    int width = mcd212_get_screen_width(machine);

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
                            BYTE68K bU1 = bU + state->mcd212_ab.deltaUV[b0];
                            BYTE68K bY0 = bY + state->mcd212_ab.deltaY[b0];

                            BYTE68K b1 = data[(vsr & 0x0007ffff) ^ 1];
                            BYTE68K bV1 = bV + state->mcd212_ab.deltaUV[b1];
                            BYTE68K bY1 = bY0 + state->mcd212_ab.deltaY[b1];

                            BYTE68K bU0 = (bU + bU1) >> 1;
                            BYTE68K bV0 = (bV + bV1) >> 1;

                            BYTE68K *pbLimit;

                            vsr++;

                            bY = bY0;
                            bU = bU0;
                            bV = bV0;

                            pbLimit = state->mcd212_ab.limit + bY + BYTE68K_MAX;

                            pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[state->mcd212_ab.matrixVR[bV]];
                            pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[state->mcd212_ab.matrixUG[bU] + state->mcd212_ab.matrixVG[bV]];
                            pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[state->mcd212_ab.matrixUB[bU]];

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

                            pbLimit = state->mcd212_ab.limit + bY + BYTE68K_MAX;

                            pixels_r[x + 0] = pixels_r[x + 1] = pbLimit[state->mcd212_ab.matrixVR[bV]];
                            pixels_g[x + 0] = pixels_g[x + 1] = pbLimit[state->mcd212_ab.matrixUG[bU] + state->mcd212_ab.matrixVG[bV]];
                            pixels_b[x + 0] = pixels_b[x + 1] = pbLimit[state->mcd212_ab.matrixUB[bU]];

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
                        mcd212_set_vsr(&state->mcd212_regs, channel, (vsr - 1) & 0x0007ffff);
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
                        mcd212_set_vsr(&state->mcd212_regs, channel, (vsr - 1) & 0x0007ffff);
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
                        mcd212_set_vsr(&state->mcd212_regs, channel, (vsr - 1) & 0x0007ffff);
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
                            mcd212_set_vsr(&state->mcd212_regs, channel, vsr);
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
                                mcd212_set_vsr(&state->mcd212_regs, channel, vsr);
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
                            mcd212_set_vsr(&state->mcd212_regs, channel, vsr);
                        }
                    }
                }
                break;
        }
    }

    //printf( ": vsr after: %08x\n", vsr);
    //mcd212_set_vsr(&state->mcd212_regs, channel, vsr);
}

static const UINT32 mcd212_4bpp_color[16] =
{
    0x00101010, 0x0010107a, 0x00107a10, 0x00107a7a, 0x007a1010, 0x007a107a, 0x007a7a10, 0x007a7a7a,
    0x00101010, 0x001010e6, 0x0010e610, 0x0010e6e6, 0x00e61010, 0x00e610e6, 0x00e6e610, 0x00e6e6e6
};

static void mcd212_draw_cursor(running_machine *machine, UINT32 *scanline, int y)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;

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

static void mcd212_mix_lines(running_machine *machine, UINT8 *plane_a_r, UINT8 *plane_a_g, UINT8 *plane_a_b, UINT8 *plane_b_r, UINT8 *plane_b_g, UINT8 *plane_b_b, UINT32 *out)
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    int x = 0;
    UINT8 debug_mode = input_port_read(machine, "DEBUG");
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

static void mcd212_draw_scanline(running_machine *machine, int y)
{
    bitmap_t *bitmap = machine->generic.tmpbitmap;
    UINT8 plane_a_r[768], plane_a_g[768], plane_a_b[768];
    UINT8 plane_b_r[768], plane_b_g[768], plane_b_b[768];
    UINT32 out[768];
    UINT32 *scanline = BITMAP_ADDR32(bitmap, y, 0);
    int x;

    mcd212_process_vsr(machine, 0, plane_a_r, plane_a_g, plane_a_b);
    mcd212_process_vsr(machine, 1, plane_b_r, plane_b_g, plane_b_b);

    mcd212_mix_lines(machine, plane_a_r, plane_a_g, plane_a_b, plane_b_r, plane_b_g, plane_b_b, out);

    for(x = 0; x < 384; x++)
    {
        scanline[x] = out[x*2];
    }

    mcd212_draw_cursor(machine, scanline, y);
}

static TIMER_CALLBACK( mcd212_perform_scan )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    mcd212_regs_t *mcd212 = &state->mcd212_regs;
    int scanline = machine->primary_screen->vpos();

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
                    mcd212_process_ica(machine, index);
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
            mcd212_draw_scanline(machine, scanline);
            // Process DCA
            for(index = 0; index < 2; index++)
            {
                if(mcd212->channel[index].dcr & MCD212_DCR_DCA)
                {
                    if(scanline == 22)
                    {
                        mcd212->channel[index].dca = mcd212_get_dcp(mcd212, index);
                    }
                    mcd212_process_dca(machine, index);
                }
            }
            if(scanline == 261)
            {
                mcd212->channel[0].csrr ^= 0x20;
            }
        }
    }
    timer_adjust_oneshot(mcd212->scan_timer, machine->primary_screen->time_until_pos(( scanline + 1 ) % 262, 0), 0);
}

static void mcd212_init(running_machine *machine, mcd212_regs_t *mcd212)
{
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
        memset(mcd212->channel[index].clut_r, 0, 768);
        memset(mcd212->channel[index].clut_g, 0, 768);
        memset(mcd212->channel[index].clut_b, 0, 768);
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

static void mcd212_ab_init(mcd212_ab_t *mcd212_ab)
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



static VIDEO_START(cdi)
{
    cdi_state *state = machine->driver_data<cdi_state>();

    VIDEO_START_CALL(generic_bitmapped);
    mcd212_ab_init(&state->mcd212_ab);
    mcd212_init(machine, &state->mcd212_regs);
    state->mcd212_regs.scan_timer = timer_alloc(machine, mcd212_perform_scan, 0);
    timer_adjust_oneshot(state->mcd212_regs.scan_timer, machine->primary_screen->time_until_pos(0, 0), 0);

    state->lcdbitmap = machine->primary_screen->alloc_compatible_bitmap();
}

static VIDEO_UPDATE(cdi)
{
    cdi_state *state = screen->machine->driver_data<cdi_state>();
    running_device *main_screen = screen->machine->device("screen");
    running_device *lcd_screen = screen->machine->device("lcd");

    if (screen == main_screen)
    {
        copybitmap(bitmap, screen->machine->generic.tmpbitmap, 0, 0, 0, 0, cliprect);
    }
    else if (screen == lcd_screen)
    {
        copybitmap(bitmap, state->lcdbitmap, 0, 0, 0, 0, cliprect);
    }

    return 0;
}

static TIMER_CALLBACK( test_timer_callback )
{
    cdi_state *state = machine->driver_data<cdi_state>();

    // This function manually triggers interrupt requests as a test.
    if(state->timer_set == 0)
    {
        state->timer_set = 1;
        cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_4, 60);
        cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
        timer_adjust_oneshot(state->test_timer, ATTOTIME_IN_HZ(10000), 0);
    }
    else
    {
        state->timer_set = 0;
        cputag_set_input_line(machine, "maincpu", M68K_IRQ_4, CLEAR_LINE);
        timer_adjust_oneshot(state->test_timer, attotime_never, 0);
    }
}

/*************************
*      Memory maps       *
*************************/

static WRITE16_HANDLER(cdic_ram_w)
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;

    verboselog(space->machine, 0, "cdic_ram_w: %08x = %04x & %04x\n", 0x00300000 + offset*2, data, mem_mask);
    COMBINE_DATA(&cdic->ram[offset]);
}

static READ16_HANDLER(cdic_ram_r)
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    cdic_regs_t *cdic = &state->cdic_regs;

    verboselog(space->machine, 0, "cdic_ram_r: %08x = %04x & %04x\n", 0x00300000 + offset*2, cdic->ram[offset], mem_mask);
    return cdic->ram[offset];
}

static ADDRESS_MAP_START( cdimono1_mem, ADDRESS_SPACE_PROGRAM, 16 )
    AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_BASE_MEMBER(cdi_state,planea)
    AM_RANGE(0x00200000, 0x0027ffff) AM_RAM AM_BASE_MEMBER(cdi_state,planeb)
#if ENABLE_UART_PRINTING
    AM_RANGE(0x00301400, 0x00301403) AM_READ(uart_loopback_enable)
#endif
    AM_RANGE(0x00300000, 0x00303bff) AM_READWRITE(cdic_ram_r, cdic_ram_w) AM_BASE_MEMBER(cdi_state,cdic_regs.ram)
    //AM_RANGE(0x00300000, 0x00303bff) AM_RAM AM_BASE_MEMBER(cdi_state,cdic_regs.ram)
    AM_RANGE(0x00303c00, 0x00303fff) AM_READWRITE(cdic_r, cdic_w)
    AM_RANGE(0x00310000, 0x00317fff) AM_READWRITE(slave_r, slave_w)
    //AM_RANGE(0x00318000, 0x0031ffff) AM_NOP
    AM_RANGE(0x00320000, 0x00323fff) AM_DEVREADWRITE8("mk48t08", timekeeper_r, timekeeper_w, 0xff00)    /* nvram (only low bytes used) */
    AM_RANGE(0x00400000, 0x0047ffff) AM_ROM AM_REGION("maincpu", 0)
    AM_RANGE(0x004fffe0, 0x004fffff) AM_READWRITE(mcd212_r, mcd212_w)
    AM_RANGE(0x00500000, 0x0057ffff) AM_RAM
    AM_RANGE(0x00580000, 0x00ffffff) AM_NOP
    AM_RANGE(0x00e00000, 0x00efffff) AM_RAM // DVC
    AM_RANGE(0x80000000, 0x8000807f) AM_READWRITE(scc68070_periphs_r, scc68070_periphs_w)
ADDRESS_MAP_END

/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( cdi )
    PORT_START("MOUSEX")
    PORT_BIT(0x3ff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(0) PORT_CHANGED(mouse_update, 0)

    PORT_START("MOUSEY")
    PORT_BIT(0x3ff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(0) PORT_CHANGED(mouse_update, 0)

    PORT_START("MOUSEBTN")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CHANGED(mouse_update, 0)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CHANGED(mouse_update, 0)
    PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("DEBUG")
    PORT_CONFNAME( 0x01, 0x00, "Plane A Disable")
    PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
    PORT_CONFSETTING(    0x01, DEF_STR( On ) )
    PORT_CONFNAME( 0x02, 0x00, "Plane B Disable")
    PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
    PORT_CONFSETTING(    0x02, DEF_STR( On ) )
    PORT_CONFNAME( 0x04, 0x00, "Force Backdrop Color")
    PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
    PORT_CONFSETTING(    0x04, DEF_STR( On ) )
    PORT_CONFNAME( 0xf0, 0x00, "Backdrop Color")
    PORT_CONFSETTING(    0x00, "Black" )
    PORT_CONFSETTING(    0x10, "Half-Bright Blue" )
    PORT_CONFSETTING(    0x20, "Half-Bright Green" )
    PORT_CONFSETTING(    0x30, "Half-Bright Cyan" )
    PORT_CONFSETTING(    0x40, "Half-Bright Red" )
    PORT_CONFSETTING(    0x50, "Half-Bright Magenta" )
    PORT_CONFSETTING(    0x60, "Half-Bright Yellow" )
    PORT_CONFSETTING(    0x70, "Half-Bright White" )
    PORT_CONFSETTING(    0x80, "Black (Alternate)" )
    PORT_CONFSETTING(    0x90, "Blue" )
    PORT_CONFSETTING(    0xa0, "Green" )
    PORT_CONFSETTING(    0xb0, "Cyan" )
    PORT_CONFSETTING(    0xc0, "Red" )
    PORT_CONFSETTING(    0xd0, "Magenta" )
    PORT_CONFSETTING(    0xe0, "Yellow" )
    PORT_CONFSETTING(    0xf0, "White" )
INPUT_PORTS_END

static MACHINE_START( cdi )
{
    cdi_state *state = machine->driver_data<cdi_state>();

    scc68070_register_globals(machine, &state->scc68070_regs);
    cdic_register_globals(machine, &state->cdic_regs);
    slave_register_globals(machine, &state->slave_regs);

    state->scc68070_regs.timers.timer0_timer = timer_alloc(machine, scc68070_timer0_callback, 0);
    timer_adjust_oneshot(state->scc68070_regs.timers.timer0_timer, attotime_never, 0);

    state->test_timer = timer_alloc(machine, test_timer_callback, 0);
    timer_adjust_oneshot(state->test_timer, attotime_never, 0);

    state->slave_regs.interrupt_timer = timer_alloc(machine, slave_trigger_readback_int, 0);
    timer_adjust_oneshot(state->slave_regs.interrupt_timer, attotime_never, 0);

    state->cdic_regs.interrupt_timer = timer_alloc(machine, cdic_trigger_readback_int, 0);
    timer_adjust_oneshot(state->cdic_regs.interrupt_timer, attotime_never, 0);

    state->cdic_regs.audio_sample_timer = timer_alloc(machine, audio_sample_trigger, 0);
    timer_adjust_oneshot(state->cdic_regs.audio_sample_timer, attotime_never, 0);
}

static MACHINE_RESET( cdi )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    UINT16 *src   = (UINT16*)memory_region(machine, "maincpu");
    UINT16 *dst   = state->planea;
    //running_device *cdrom_dev = machine->device("cdrom");
    memcpy(dst, src, 0x8);

    scc68070_init(machine, &state->scc68070_regs);
    cdic_init(machine, &state->cdic_regs);
    slave_init(machine, &state->slave_regs);

    state->cdic_regs.cd = cdrom_open(get_disk_handle(machine, "cdrom"));
    cdda_set_cdrom(machine->device("cdda"), state->cdic_regs.cd);

    machine->device("maincpu")->reset();

    state->dmadac[0] = machine->device<dmadac_sound_device>("dac1");
    state->dmadac[1] = machine->device<dmadac_sound_device>("dac2");

    state->slave_regs.real_mouse_x = 0xffff;
    state->slave_regs.real_mouse_y = 0xffff;
}

/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( cdi )

    MDRV_DRIVER_DATA( cdi_state )

    MDRV_CPU_ADD("maincpu", SCC68070, CLOCK_A/2)
    MDRV_CPU_PROGRAM_MAP(cdimono1_mem)

    MDRV_SCREEN_ADD("screen", RASTER)
    MDRV_SCREEN_REFRESH_RATE(60)
    MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
    MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
    MDRV_SCREEN_SIZE(384, 262)
    MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 22, 262-1) //dynamic resolution,TODO

    MDRV_SCREEN_ADD("lcd", RASTER)
    MDRV_SCREEN_REFRESH_RATE(60)
    MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
    MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
    MDRV_SCREEN_SIZE(192, 22)
    MDRV_SCREEN_VISIBLE_AREA(0, 192-1, 0, 22-1)

    MDRV_PALETTE_LENGTH(0x100)

    MDRV_DEFAULT_LAYOUT(layout_cdi)

    MDRV_VIDEO_START(cdi)
    MDRV_VIDEO_UPDATE(cdi)

    MDRV_MACHINE_RESET(cdi)
    MDRV_MACHINE_START(cdi)

    /* sound hardware */
    MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

    MDRV_SOUND_ADD( "dac1", DMADAC, 0 )
    MDRV_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )

    MDRV_SOUND_ADD( "dac2", DMADAC, 0 )
    MDRV_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

    MDRV_SOUND_ADD( "cdda", CDDA, 0 )
    MDRV_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )
    MDRV_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

    MDRV_MK48T08_ADD( "mk48t08" )
MACHINE_DRIVER_END

/*************************
*        Rom Load        *
*************************/

ROM_START( cdi )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping
ROM_END

ROM_START( quizard )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd32", 0, SHA1(31e9fa2169aa44d799c37170b238134ab738e1a1) )
ROM_END

ROM_START( quizrd22 )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd22", 0, SHA1(03c8fdcf27ead6e221691111e8c679b551099543) )
ROM_END

ROM_START( quizrd17 )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd17", 0, SHA1(4bd698f076505b4e17be978481bce027eb47123b) )
ROM_END

ROM_START( quizrd12 ) /* CD-ROM printed 01/95 */
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrd12", 0, SHA1(6e41683b96b74e903040842aeb18437ad7813c82) )
ROM_END

ROM_START( quizrr42 ) /* CD-ROM printed 09/98 */
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrr42", 0, SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )
ROM_END

ROM_START( quizrr41 )
    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

    ROM_REGION(0x2000, "cdic", 0)
    ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    ROM_REGION(0x2000, "slave", 0)
    ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

    DISK_REGION( "cdrom" )
    DISK_IMAGE_READONLY( "quizrr41", 0, SHA1(2c0484c6545aac8e00b318328c6edce6f5dde43d) )
ROM_END

/*************************
*      Game driver(s)    *
*************************/

/*          rom       parent    machine   inp       init */
GAME( 1991, cdi,      0,        cdi,      cdi,      0,        ROT0,     "Philips", "CD-i (Mono-I) BIOS", GAME_IS_BIOS_ROOT )
GAME( 1996, quizard,  cdi,      cdi,      cdi,      0,        ROT0,     "Disney",  "Quizard 3.2",   GAME_NOT_WORKING )
GAME( 1995, quizrd22, cdi,      cdi,      cdi,      0,        ROT0,     "Disney",  "Quizard 2.2",   GAME_NOT_WORKING )
GAME( 1995, quizrd17, cdi,      cdi,      cdi,      0,        ROT0,     "Disney",  "Quizard 1.7",   GAME_NOT_WORKING )
GAME( 1995, quizrd12, cdi,      cdi,      cdi,      0,        ROT0,     "Disney",  "Quizard 1.2",   GAME_NOT_WORKING )
GAME( 1998, quizrr42, cdi,      cdi,      cdi,      0,        ROT0,     "Disney",  "Quizard Rainbow 4.2",   GAME_NOT_WORKING )
GAME( 1998, quizrr41, cdi,      cdi,      cdi,      0,        ROT0,     "Disney",  "Quizard Rainbow 4.1",   GAME_NOT_WORKING )
