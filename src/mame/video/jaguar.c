/***************************************************************************

    Atari Jaguar hardware

****************************************************************************

    ------------------------------------------------------------
    TOM REGISTERS
    ------------------------------------------------------------
    F00000-F0FFFF   R/W   xxxxxxxx xxxxxxxx   Internal Registers
    F00000          R/W   -x-xx--- xxxxxxxx   MEMCON1 - memory config reg 1
                          -x------ --------      (CPU32 - is the CPU 32bits?)
                          ---xx--- --------      (IOSPEED - external I/O clock cycles)
                          -------- x-------      (FASTROM - reduces ROM clock cycles)
                          -------- -xx-----      (DRAMSPEED - sets RAM clock cycles)
                          -------- ---xx---      (ROMSPEED - sets ROM clock cycles)
                          -------- -----xx-      (ROMWIDTH - sets width of ROM: 8,16,32,64 bits)
                          -------- -------x      (ROMHI - controls ROM mapping)
    F00002          R/W   --xxxxxx xxxxxxxx   MEMCON2 - memory config reg 2
                          --x----- --------      (HILO - image display bit order)
                          ---x---- --------      (BIGEND - big endian addressing?)
                          ----xxxx --------      (REFRATE - DRAM refresh rate)
                          -------- xx------      (DWIDTH1 - DRAM1 width: 8,16,32,64 bits)
                          -------- --xx----      (COLS1 - DRAM1 columns: 256,512,1024,2048)
                          -------- ----xx--      (DWIDTH0 - DRAM0 width: 8,16,32,64 bits)
                          -------- ------xx      (COLS0 - DRAM0 columns: 256,512,1024,2048)
    F00004          R/W   -----xxx xxxxxxxx   HC - horizontal count
                          -----x-- --------      (which half of the display)
                          ------xx xxxxxxxx      (10-bit counter)
    F00006          R/W   ----xxxx xxxxxxxx   VC - vertical count
                          ----x--- --------      (which field is being generated)
                          -----xxx xxxxxxxx      (11-bit counter)
    F00008          R     -----xxx xxxxxxxx   LPH - light pen horizontal position
    F0000A          R     -----xxx xxxxxxxx   LPV - light pen vertical position
    F00010-F00017   R     xxxxxxxx xxxxxxxx   OB - current object code from the graphics processor
    F00020-F00023     W   xxxxxxxx xxxxxxxx   OLP - start of the object list
    F00026            W   -------- -------x   OBF - object processor flag
    F00028            W   ----xxxx xxxxxxxx   VMODE - video mode
                      W   ----xxx- --------      (PWIDTH1-8 - width of pixel in video clock cycles)
                      W   -------x --------      (VARMOD - enable variable color resolution)
                      W   -------- x-------      (BGEN - clear line buffere to BG color)
                      W   -------- -x------      (CSYNC - enable composite sync on VSYNC)
                      W   -------- --x-----      (BINC - local border color if INCEN)
                      W   -------- ---x----      (INCEN - encrustation enable)
                      W   -------- ----x---      (GENLOCK - enable genlock)
                      W   -------- -----xx-      (MODE - CRY16,RGB24,DIRECT16,RGB16)
                      W   -------- -------x      (VIDEN - enables video)
    F0002A            W   xxxxxxxx xxxxxxxx   BORD1 - border color (red/green)
    F0002C            W   -------- xxxxxxxx   BORD2 - border color (blue)
    F0002E            W   ------xx xxxxxxxx   HP - horizontal period
    F00030            W   -----xxx xxxxxxxx   HBB - horizontal blanking begin
    F00032            W   -----xxx xxxxxxxx   HBE - horizontal blanking end
    F00034            W   -----xxx xxxxxxxx   HSYNC - horizontal sync
    F00036            W   ------xx xxxxxxxx   HVS - horizontal vertical sync
    F00038            W   -----xxx xxxxxxxx   HDB1 - horizontal display begin 1
    F0003A            W   -----xxx xxxxxxxx   HDB2 - horizontal display begin 2
    F0003C            W   -----xxx xxxxxxxx   HDE - horizontal display end
    F0003E            W   -----xxx xxxxxxxx   VP - vertical period
    F00040            W   -----xxx xxxxxxxx   VBB - vertical blanking begin
    F00042            W   -----xxx xxxxxxxx   VBE - vertical blanking end
    F00044            W   -----xxx xxxxxxxx   VS - vertical sync
    F00046            W   -----xxx xxxxxxxx   VDB - vertical display begin
    F00048            W   -----xxx xxxxxxxx   VDE - vertical display end
    F0004A            W   -----xxx xxxxxxxx   VEB - vertical equalization begin
    F0004C            W   -----xxx xxxxxxxx   VEE - vertical equalization end
    F0004E            W   -----xxx xxxxxxxx   VI - vertical interrupt
    F00050            W   xxxxxxxx xxxxxxxx   PIT0 - programmable interrupt timer 0
    F00052            W   xxxxxxxx xxxxxxxx   PIT1 - programmable interrupt timer 1
    F00054            W   ------xx xxxxxxxx   HEQ - horizontal equalization end
    F00058            W   xxxxxxxx xxxxxxxx   BG - background color
    F000E0          R/W   ---xxxxx ---xxxxx   INT1 - CPU interrupt control register
                          ---x---- --------      (C_JERCLR - clear pending Jerry ints)
                          ----x--- --------      (C_PITCLR - clear pending PIT ints)
                          -----x-- --------      (C_OPCLR - clear pending object processor ints)
                          ------x- --------      (C_GPUCLR - clear pending graphics processor ints)
                          -------x --------      (C_VIDCLR - clear pending video timebase ints)
                          -------- ---x----      (C_JERENA - enable Jerry ints)
                          -------- ----x---      (C_PITENA - enable PIT ints)
                          -------- -----x--      (C_OPENA - enable object processor ints)
                          -------- ------x-      (C_GPUENA - enable graphics processor ints)
                          -------- -------x      (C_VIDENA - enable video timebase ints)
    F000E2            W   -------- --------   INT2 - CPU interrupt resume register
    F00400-F005FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table A
    F00600-F007FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table B
    F00800-F00D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer A
    F01000-F0159F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer B
    F01800-F01D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer currently selected
    ------------------------------------------------------------
    F02000-F021FF   R/W   xxxxxxxx xxxxxxxx   GPU control registers
    F02100          R/W   xxxxxxxx xxxxxxxx   G_FLAGS - GPU flags register
                    R/W   x------- --------      (DMAEN - DMA enable)
                    R/W   -x------ --------      (REGPAGE - register page)
                      W   --x----- --------      (G_BLITCLR - clear blitter interrupt)
                      W   ---x---- --------      (G_OPCLR - clear object processor int)
                      W   ----x--- --------      (G_PITCLR - clear PIT interrupt)
                      W   -----x-- --------      (G_JERCLR - clear Jerry interrupt)
                      W   ------x- --------      (G_CPUCLR - clear CPU interrupt)
                    R/W   -------x --------      (G_BLITENA - enable blitter interrupt)
                    R/W   -------- x-------      (G_OPENA - enable object processor int)
                    R/W   -------- -x------      (G_PITENA - enable PIT interrupt)
                    R/W   -------- --x-----      (G_JERENA - enable Jerry interrupt)
                    R/W   -------- ---x----      (G_CPUENA - enable CPU interrupt)
                    R/W   -------- ----x---      (IMASK - interrupt mask)
                    R/W   -------- -----x--      (NEGA_FLAG - ALU negative)
                    R/W   -------- ------x-      (CARRY_FLAG - ALU carry)
                    R/W   -------- -------x      (ZERO_FLAG - ALU zero)
    F02104            W   -------- ----xxxx   G_MTXC - matrix control register
                      W   -------- ----x---      (MATCOL - column/row major)
                      W   -------- -----xxx      (MATRIX3-15 - matrix width)
    F02108            W   ----xxxx xxxxxx--   G_MTXA - matrix address register
    F0210C            W   -------- -----xxx   G_END - data organization register
                      W   -------- -----x--      (BIG_INST - big endian instruction fetch)
                      W   -------- ------x-      (BIG_PIX - big endian pixels)
                      W   -------- -------x      (BIG_IO - big endian I/O)
    F02110          R/W   xxxxxxxx xxxxxxxx   G_PC - GPU program counter
    F02114          R/W   xxxxxxxx xx-xxxxx   G_CTRL - GPU control/status register
                    R     xxxx---- --------      (VERSION - GPU version code)
                    R/W   ----x--- --------      (BUS_HOG - hog the bus!)
                    R/W   -----x-- --------      (G_BLITLAT - blitter interrupt latch)
                    R/W   ------x- --------      (G_OPLAT - object processor int latch)
                    R/W   -------x --------      (G_PITLAT - PIT interrupt latch)
                    R/W   -------- x-------      (G_JERLAT - Jerry interrupt latch)
                    R/W   -------- -x------      (G_CPULAT - CPU interrupt latch)
                    R/W   -------- ---x----      (SINGLE_GO - single step one instruction)
                    R/W   -------- ----x---      (SINGLE_STEP - single step mode)
                    R/W   -------- -----x--      (FORCEINT0 - cause interrupt 0 on GPU)
                    R/W   -------- ------x-      (CPUINT - send GPU interrupt to CPU)
                    R/W   -------- -------x      (GPUGO - enable GPU execution)
    F02118-F0211B   R/W   xxxxxxxx xxxxxxxx   G_HIDATA - high data register
    F0211C-F0211F   R     xxxxxxxx xxxxxxxx   G_REMAIN - divide unit remainder
    F0211C            W   -------- -------x   G_DIVCTRL - divide unit control
                      W   -------- -------x      (DIV_OFFSET - 1=16.16 divide, 0=32-bit divide)
    ------------------------------------------------------------

****************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "machine/atarigen.h"
#include "cpu/mips/r3000.h"
#include "cpu/m68000/m68000.h"
#include "includes/jaguar.h"
#include "jagblit.h"


#define LOG_BLITS			0
#define LOG_BAD_BLITS		0
#define LOG_BLITTER_STATS	0
#define LOG_BLITTER_WRITE	0
#define LOG_UNHANDLED_BLITS	0


// interrupts to main CPU:
//  0 = video (on the VI scanline)
//  1 = GPU (write from GPU coprocessor)
//  2 = object (stop object interrupt in display list)
//  3 = timer (from the PIT)
//  4 = jerry


/* GPU registers */
enum
{
	MEMCON1,	MEMCON2,	HC,			VC,
	LPH,		LPV,		GPU0,		GPU1,
	OB_HH,		OB_HL,		OB_LH,		OB_LL,
	GPU2,		GPU3,		GPU4,		GPU5,
	OLP_L,		OLP_H,		GPU6,		OBF,
	VMODE,		BORD1,		BORD2,		HP,
	HBB,		HBE,		HSYNC,		HVS,
	HDB1,		HDB2,		HDE,		VP,
	VBB,		VBE,		VS,			VDB,
	VDE,		VEB,		VEE,		VI,
	PIT0,		PIT1,		HEQ,		GPU7,
	BG,
	INT1 = 0xe0/2,
	INT2,
	GPU_REGS
};



/*************************************
 *
 *  Local variables
 *
 *************************************/

/* blitter variables */
static UINT32 blitter_regs[BLITTER_REGS];
static UINT16 gpu_regs[GPU_REGS];

static emu_timer *vi_timer;
static UINT8 cpu_irq_state;

static pen_t *pen_table;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

/* from jagobj.c */
static void jagobj_init(void);
static void process_object_list(mame_bitmap *bitmap, const rectangle *cliprect);

/* from jagblit.c */
static void generic_blitter(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_09800001_010020_010020(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_09800009_000020_000020(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_01800009_000028_000028(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_01800001_000018_000018(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_01c00001_000018_000018(UINT32 command, UINT32 a1flags, UINT32 a2flags);

#ifdef MESS
static void blitter_00010000_xxxxxx_xxxxxx(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_01800001_xxxxxx_xxxxxx(UINT32 command, UINT32 a1flags, UINT32 a2flags);
static void blitter_x1800x01_xxxxxx_xxxxxx(UINT32 command, UINT32 a1flags, UINT32 a2flags);
#endif



/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

INLINE void get_crosshair_xy(int player, int *x, int *y)
{
	*x = ((readinputport(3 + player * 2) & 0xff) * Machine->screen[0].width) >> 8;
	*y = ((readinputport(4 + player * 2) & 0xff) * Machine->screen[0].height) >> 8;
}



/*************************************
 *
 *  GPU optimization control
 *
 *************************************/

void jaguar_gpu_suspend(void)
{
	cpunum_suspend(1, SUSPEND_REASON_SPIN, 1);
}


void jaguar_gpu_resume(void)
{
	cpunum_resume(1, SUSPEND_REASON_SPIN);
}



/*************************************
 *
 *  Main CPU interrupts
 *
 *************************************/

static void update_cpu_irq(void)
{
	if (cpu_irq_state & gpu_regs[INT1] & 0x1f)
		cpunum_set_input_line(0, cojag_is_r3000 ? R3000_IRQ4 : MC68000_IRQ_6, ASSERT_LINE);
	else
		cpunum_set_input_line(0, cojag_is_r3000 ? R3000_IRQ4 : MC68000_IRQ_6, CLEAR_LINE);
}


static TIMER_CALLBACK( vi_callback )
{
	int scanline = param;

	cpu_irq_state |= 1;
	update_cpu_irq();
	timer_adjust(vi_timer, video_screen_get_time_until_pos(0, scanline, 0), scanline, attotime_zero);
}


void jaguar_gpu_cpu_int(void)
{
	cpu_irq_state |= 2;
	update_cpu_irq();
}


void jaguar_dsp_cpu_int(void)
{
	cpu_irq_state |= 16;
	update_cpu_irq();
}



/*************************************
 *
 *  Palette init
 *
 *************************************/

static void jaguar_set_palette(UINT16 vmode)
{
	static const UINT8 red_lookup[256] =
	{
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 19,  0,
		 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 64, 43, 21,  0,
		102,102,102,102,102,102,102,102,102,102,102, 95, 71, 47, 23,  0,
		135,135,135,135,135,135,135,135,135,135,130,104, 78, 52, 26,  0,
		169,169,169,169,169,169,169,169,169,170,141,113, 85, 56, 28,  0,
		203,203,203,203,203,203,203,203,203,183,153,122, 91, 61, 30,  0,
		237,237,237,237,237,237,237,237,230,197,164,131, 98, 65, 32,  0,
		255,255,255,255,255,255,255,255,247,214,181,148,115, 82, 49, 17,
		255,255,255,255,255,255,255,255,255,235,204,173,143,112, 81, 51,
		255,255,255,255,255,255,255,255,255,255,227,198,170,141,113, 85,
		255,255,255,255,255,255,255,255,255,255,249,223,197,171,145,119,
		255,255,255,255,255,255,255,255,255,255,255,248,224,200,177,153,
		255,255,255,255,255,255,255,255,255,255,255,255,252,230,208,187,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,221,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
	};

	static const UINT8 grn_lookup[256] =
	{
		  0, 17, 34, 51, 68, 85,102,119,136,153,170,187,204,221,238,255,
		  0, 19, 38, 57, 77, 96,115,134,154,173,182,211,231,250,255,255,
		  0, 21, 43, 64, 86,107,129,150,172,193,215,236,255,255,255,255,
		  0, 23, 47, 71, 96,119,142,166,190,214,238,255,255,255,255,255,
		  0, 26, 52, 78,104,130,156,182,208,234,255,255,255,255,255,255,
		  0, 28, 56, 85,113,141,170,198,226,255,255,255,255,255,255,255,
		  0, 30, 61, 91,122,153,183,214,244,255,255,255,255,255,255,255,
		  0, 32, 65, 98,131,164,197,230,255,255,255,255,255,255,255,255,
		  0, 32, 65, 98,131,164,197,230,255,255,255,255,255,255,255,255,
		  0, 30, 61, 91,122,153,183,214,244,255,255,255,255,255,255,255,
		  0, 28, 56, 85,113,141,170,198,226,255,255,255,255,255,255,255,
		  0, 26, 52, 78,104,130,156,182,208,234,255,255,255,255,255,255,
		  0, 23, 47, 71, 96,119,142,166,190,214,238,255,255,255,255,255,
		  0, 21, 43, 64, 86,107,129,150,172,193,215,236,255,255,255,255,
		  0, 19, 38, 57, 77, 96,115,134,154,173,182,211,231,250,255,255,
		  0, 17, 34, 51, 68, 85,102,119,136,153,170,187,204,221,238,255
	};

	static const UINT8 blu_lookup[256] =
	{
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,221,
		255,255,255,255,255,255,255,255,255,255,255,255,252,230,208,187,
		255,255,255,255,255,255,255,255,255,255,255,248,224,200,177,153,
		255,255,255,255,255,255,255,255,255,255,249,223,197,171,145,119,
		255,255,255,255,255,255,255,255,255,255,227,198,170,141,113, 85,
		255,255,255,255,255,255,255,255,255,235,204,173,143,112, 81, 51,
		255,255,255,255,255,255,255,255,247,214,181,148,115, 82, 49, 17,
		237,237,237,237,237,237,237,237,230,197,164,131, 98, 65, 32,  0,
		203,203,203,203,203,203,203,203,203,183,153,122, 91, 61, 30,  0,
		169,169,169,169,169,169,169,169,169,170,141,113, 85, 56, 28,  0,
		135,135,135,135,135,135,135,135,135,135,130,104, 78, 52, 26,  0,
		102,102,102,102,102,102,102,102,102,102,102, 95, 71, 47, 23,  0,
		 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 64, 43, 21,  0,
		 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 19,  0,
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	};

	int i, colors = 0;

	/* switch off the mode */
	switch (vmode & 0x106)
	{
		/* YCC full */
		case 0x000:
		{
			/* set color 0 to black */
			palette_set_color(Machine, colors++, MAKE_RGB(0, 0, 0));

			/* due to the way YCC colorspace maps onto RGB, there are multiple entries for black */
			/* take advantage of this so we don't have to use all 64k colors */
			for (i = 0; i < 65536; i++)
			{
				UINT8 r = (red_lookup[i >> 8] * (i & 0xff)) >> 8;
				UINT8 g = (grn_lookup[i >> 8] * (i & 0xff)) >> 8;
				UINT8 b = (blu_lookup[i >> 8] * (i & 0xff)) >> 8;

				if (r == 0 && g == 0 && b == 0)
					pen_table[i] = 0;
				else
				{
					pen_table[i] = colors;
					palette_set_color(Machine, colors++, MAKE_RGB(r, g, b));
				}
			}
			break;
		}

		/* YCC/RGB VARMOD */
		case 0x100:
		case 0x107:
		{
			/* set color 0 to black */
			palette_set_color(Machine, colors++, MAKE_RGB(0, 0, 0));

			/* due to the way YCC colorspace maps onto RGB, there are multiple entries for black */
			/* take advantage of this so we don't have to use all 64k colors */
			for (i = 0; i < 65536; i++)
			{
				UINT8 r = (red_lookup[i >> 8] * (i & 0xff)) >> 8;
				UINT8 g = (grn_lookup[i >> 8] * (i & 0xff)) >> 8;
				UINT8 b = (blu_lookup[i >> 8] * (i & 0xff)) >> 8;

				/* if the low bit is set, treat it as 5-5-5 RGB instead */
				if (i & 1)
				{
					r = (i >> 11) & 31;
					g = (i >> 1) & 31;
					b = (i >> 6) & 31;
					r = (r << 3) | (r >> 2);
					g = (g << 3) | (g >> 2);
					b = (b << 3) | (b >> 2);
				}

				if (r == 0 && g == 0 && b == 0)
					pen_table[i] = 0;
				else
				{
					pen_table[i] = colors;
					palette_set_color(Machine, colors++, MAKE_RGB(r, g, b));
				}
			}
			break;
		}

		/* RGB full */
		case 0x006:
		{
			/* we cheat a little here to squeeze into 65534 colors */
			palette_set_color(Machine, colors++, MAKE_RGB(0,  0, 0));
			palette_set_color(Machine, colors++, MAKE_RGB(0,  8, 0));
			palette_set_color(Machine, colors++, MAKE_RGB(0, 16, 0));
			pen_table[0] = 0;
			pen_table[1] = 1;
			pen_table[2] = 1;
			pen_table[3] = 2;
			pen_table[4] = 2;

			/* map the remaining colors normally */
			for (i = 5; i < 65536; i++)
			{
				pen_table[i] = colors;
				palette_set_color_rgb(Machine, colors++, pal5bit(i >> 11), pal6bit(i >> 0), pal5bit(i >> 6));
			}
			break;
		}

		/* others */
		default:
			logerror("Can't handle mode %X\n", vmode);
			break;
	}
}



/*************************************
 *
 *  Fast memory accessors
 *
 *************************************/

static UINT8 *get_jaguar_memory(UINT32 offset)
{
	return memory_get_read_ptr(1, ADDRESS_SPACE_PROGRAM, offset);
}



/*************************************
 *
 *  32-bit access to the blitter
 *
 *************************************/

static void blitter_run(void)
{
	UINT32 command = blitter_regs[B_CMD] & STATIC_COMMAND_MASK;
	UINT32 a1flags = blitter_regs[A1_FLAGS] & STATIC_FLAGS_MASK;
	UINT32 a2flags = blitter_regs[A2_FLAGS] & STATIC_FLAGS_MASK;

	profiler_mark(PROFILER_USER1);

	if (a1flags == a2flags)
	{
		if (command == 0x09800001 && a1flags == 0x010020)
		{
			blitter_09800001_010020_010020(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
			return;
		}
		if (command == 0x09800009 && a1flags == 0x000020)
		{
			blitter_09800009_000020_000020(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
			return;
		}
		if (command == 0x01800009 && a1flags == 0x000028)
		{
			blitter_01800009_000028_000028(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
			return;
		}

		if (command == 0x01800001 && a1flags == 0x000018)
		{
			blitter_01800001_000018_000018(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
			return;
		}

		if (command == 0x01c00001 && a1flags == 0x000018)
		{
			blitter_01c00001_000018_000018(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
			return;
		}
	}

#ifdef MESS
	if (command == 0x00010000)
	{
		blitter_00010000_xxxxxx_xxxxxx(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
		return;
	}

	if (command == 0x01800001)
	{
		blitter_01800001_xxxxxx_xxxxxx(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
		return;
	}

	if ((command & 0x0ffff0ff) == 0x01800001)
	{
		blitter_x1800x01_xxxxxx_xxxxxx(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
		return;
	}
#endif


#if LOG_BLITTER_STATS
{
static UINT32 blitter_stats[1000][4];
static UINT64 blitter_pixels[1000];
static int blitter_count = 0;
static int reps = 0;
int i;
for (i = 0; i < blitter_count; i++)
	if (blitter_stats[i][0] == (blitter_regs[B_CMD] & STATIC_COMMAND_MASK) &&
		blitter_stats[i][1] == (blitter_regs[A1_FLAGS] & STATIC_FLAGS_MASK) &&
		blitter_stats[i][2] == (blitter_regs[A2_FLAGS] & STATIC_FLAGS_MASK))
		break;
if (i == blitter_count)
{
	blitter_stats[i][0] = blitter_regs[B_CMD] & STATIC_COMMAND_MASK;
	blitter_stats[i][1] = blitter_regs[A1_FLAGS] & STATIC_FLAGS_MASK;
	blitter_stats[i][2] = blitter_regs[A2_FLAGS] & STATIC_FLAGS_MASK;
	blitter_stats[i][3] = 0;
	blitter_pixels[i] = 0;
	blitter_count++;
}
blitter_stats[i][3]++;
blitter_pixels[i] += (blitter_regs[B_COUNT] & 0xffff) * (blitter_regs[B_COUNT] >> 16);
if (++reps % 100 == 99)
{
	mame_printf_debug("---\nBlitter stats:\n");
	for (i = 0; i < blitter_count; i++)
		mame_printf_debug("  CMD=%08X A1=%08X A2=%08X %6d times, %08X%08X pixels\n",
				blitter_stats[i][0], blitter_stats[i][1], blitter_stats[i][2],
				blitter_stats[i][3], (UINT32)(blitter_pixels[i] >> 32), (UINT32)(blitter_pixels[i]));
	mame_printf_debug("---\n");
}
}
#endif

	generic_blitter(blitter_regs[B_CMD], blitter_regs[A1_FLAGS], blitter_regs[A2_FLAGS]);
	profiler_mark(PROFILER_END);
}


READ32_HANDLER( jaguar_blitter_r )
{
	switch (offset)
	{
		case B_CMD:	/* B_CMD */
			return 0x00000001;

		default:
			logerror("%08X:Blitter read register @ F022%02X\n", activecpu_get_previouspc(), offset * 4);
			return 0;
	}
}


WRITE32_HANDLER( jaguar_blitter_w )
{
	COMBINE_DATA(&blitter_regs[offset]);
	if (offset == B_CMD)
		blitter_run();

	if (LOG_BLITTER_WRITE)
	logerror("%08X:Blitter write register @ F022%02X = %08X\n", activecpu_get_previouspc(), offset * 4, data);
}



/*************************************
 *
 *  16-bit TOM register access
 *
 *************************************/

READ16_HANDLER( jaguar_tom_regs_r )
{
	if (offset != INT1 && offset != INT2 && offset != HC && offset != VC)
		logerror("%08X:TOM read register @ F00%03X\n", activecpu_get_previouspc(), offset * 2);

	switch (offset)
	{
		case INT1:
			return cpu_irq_state;

		case HC:
			return video_screen_get_hpos(0) % (Machine->screen[0].width / 2);

		case VC:
			return video_screen_get_vpos(0) * 2 + gpu_regs[VBE];

	}

	return gpu_regs[offset];
}


WRITE16_HANDLER( jaguar_tom_regs_w )
{
	int scanline;

	if (offset < GPU_REGS)
	{
		COMBINE_DATA(&gpu_regs[offset]);

		switch (offset)
		{
			case VI:
				scanline = (gpu_regs[VI] - gpu_regs[VBE]) / 2;
				timer_adjust(vi_timer, video_screen_get_time_until_pos(0, scanline, 0), scanline, attotime_zero);
				break;

			case INT1:
				cpu_irq_state &= ~(gpu_regs[INT1] >> 8);
				update_cpu_irq();
				break;

			case VMODE:
				jaguar_set_palette(gpu_regs[VMODE]);
				break;
		}
	}

	if (offset != INT2 && offset != VI)
		logerror("%08X:TOM write register @ F00%03X = %04X\n", activecpu_get_previouspc(), offset * 2, data);
}



/*************************************
 *
 *  32-bit TOM register access
 *
 *************************************/

READ32_HANDLER( jaguar_tom_regs32_r )
{
	return (jaguar_tom_regs_r(offset * 2, 0) << 16) | jaguar_tom_regs_r(offset * 2 + 1, 0);
}


WRITE32_HANDLER( jaguar_tom_regs32_w )
{
	if (ACCESSING_MSW32)
		jaguar_tom_regs_w(offset * 2, data >> 16, mem_mask >> 16);
	if (ACCESSING_LSW32)
		jaguar_tom_regs_w(offset * 2 + 1, data, mem_mask);
}



/*************************************
 *
 *  Gun input
 *
 *************************************/

READ32_HANDLER( cojag_gun_input_r )
{
	int beamx, beamy;

	switch (offset)
	{
		case 0:
			get_crosshair_xy(1, &beamx, &beamy);
			beamx += 52;
			beamy += 17;
			return (beamy << 16) | (beamx ^ 0x1ff);

		case 1:
			get_crosshair_xy(0, &beamx, &beamy);
			beamx += 52;
			beamy += 17;
			return (beamy << 16) | (beamx ^ 0x1ff);

		case 2:
			return readinputport(7) << 16;
	}
	return 0;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( cojag )
{
	jagobj_init();

	pen_table = auto_malloc(65536 * sizeof(pen_t));

	vi_timer = timer_alloc(vi_callback, NULL);

	state_save_register_global_pointer(pen_table, 65536);
	state_save_register_global_array(blitter_regs);
	state_save_register_global_array(gpu_regs);
	state_save_register_global(cpu_irq_state);
	state_save_register_func_postload(update_cpu_irq);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( cojag )
{
	/* if not enabled, just blank */
	if (!(gpu_regs[VMODE] & 1))
	{
		fillbitmap(bitmap, 0, cliprect);
		return 0;
	}

	/* render the object list */
	process_object_list(bitmap, cliprect);
	return 0;
}



/*************************************
 *
 *  Object processor
 *
 *************************************/

#define INCLUDE_OBJECT_PROCESSOR
#include "jagobj.c"



/*************************************
 *
 *  Blitter macros
 *
 *************************************/

/* generic blitters */
#define FUNCNAME	generic_blitter
#define COMMAND		command
#define A1FIXED		a1flags
#define A2FIXED		a2flags
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

/* optimized common blitters */
#define FUNCNAME	blitter_09800001_010020_010020
#define COMMAND		0x09800001
#define A1FIXED		0x010020
#define A2FIXED		0x010020
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME	blitter_09800009_000020_000020
#define COMMAND		0x09800009
#define A1FIXED		0x000020
#define A2FIXED		0x000020
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME	blitter_01800009_000028_000028
#define COMMAND		0x01800009
#define A1FIXED		0x000028
#define A2FIXED		0x000028
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME	blitter_01800001_000018_000018
#define COMMAND		0x01800001
#define A1FIXED		0x000018
#define A2FIXED		0x000018
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME	blitter_01c00001_000018_000018
#define COMMAND		0x01c00001
#define A1FIXED		0x000018
#define A2FIXED		0x000018
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#ifdef MESS

#define FUNCNAME	blitter_00010000_xxxxxx_xxxxxx
#define COMMAND		0x00010000
#define A1FIXED		a1flags
#define A2FIXED		a2flags
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME	blitter_01800001_xxxxxx_xxxxxx
#define COMMAND		0x01800001
#define A1FIXED		a1flags
#define A2FIXED		a2flags
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME	blitter_x1800x01_xxxxxx_xxxxxx
#define COMMAND		((command & 0xf0000f00) | 0x01800001)
#define A1FIXED		a1flags
#define A2FIXED		a2flags
#include "jagblit.c"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#endif /* MESS */
