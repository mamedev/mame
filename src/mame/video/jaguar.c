// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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

#include "emu.h"
#include "machine/atarigen.h"
#include "cpu/mips/r3000.h"
#include "cpu/m68000/m68000.h"
#include "includes/jaguar.h"
#include "jagblit.h"


#define ENABLE_BORDERS      0

#define LOG_BLITS           0
#define LOG_BAD_BLITS       0
#define LOG_BLITTER_STATS   0
#define LOG_BLITTER_WRITE   0
#define LOG_UNHANDLED_BLITS 0




// interrupts to main CPU:
//  0 = video (on the VI scanline)
//  1 = GPU (write from GPU coprocessor)
//  2 = object (stop object interrupt in display list)
//  3 = timer (from the PIT)
//  4 = jerry


/* GPU registers */
enum
{
	MEMCON1,    MEMCON2,    HC,         VC,
	LPH,        LPV,        GPU0,       GPU1,
	OB_HH,      OB_HL,      OB_LH,      OB_LL,
	GPU2,       GPU3,       GPU4,       GPU5,
	OLP_L,      OLP_H,      GPU6,       OBF,
	VMODE,      BORD1,      BORD2,      HP,
	HBB,        HBE,        HSYNC,      HVS,
	HDB1,       HDB2,       HDE,        VP,
	VBB,        VBE,        VS,         VDB,
	VDE,        VEB,        VEE,        VI,
	PIT0,       PIT1,       HEQ,        GPU7,
	BG,
	INT1 = 0xe0/2,
	INT2,
	GPU_REGS
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/



/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

inline void jaguar_state::get_crosshair_xy(int player, int &x, int &y)
{
	const rectangle &visarea = m_screen->visible_area();

	/* only 2 lightguns are connected */
	x = visarea.min_x + (((ioport(player ? "FAKE2_X" : "FAKE1_X")->read() & 0xff) * visarea.width()) >> 8);
	y = visarea.min_y + (((ioport(player ? "FAKE2_Y" : "FAKE1_Y")->read() & 0xff) * visarea.height()) >> 8);
}



/*************************************
 *
 *  Horizontal display values
 *
 *************************************/

inline int jaguar_state::effective_hvalue(int value)
{
	if (!(value & 0x400))
		return value & 0x3ff;
	else
		return (value & 0x3ff) + (m_gpu_regs[HP] & 0x3ff) + 1;
}



/*************************************
 *
 *  Object processor timer
 *
 *************************************/

inline bool jaguar_state::adjust_object_timer(int vc)
{
	/* extract the display begin registers */
	int hdbpix[2];
	hdbpix[0] = (m_gpu_regs[HDB1] & 0x7ff) / 2;
	hdbpix[1] = (m_gpu_regs[HDB2] & 0x7ff) / 2;

	/* sort */
	if (hdbpix[0] > hdbpix[1])
	{
		int temp = hdbpix[0];
		hdbpix[0] = hdbpix[1];
		hdbpix[1] = temp;
	}

	/* select the target one */
	int hdb = hdbpix[vc % 2];

	/* if setting the second one in a line, make sure we will ever actually hit it */
	if (vc % 2 == 1 && (hdbpix[1] == hdbpix[0] || hdbpix[1] >= m_screen->width()))
		return false;

	/* adjust the timer */
	m_object_timer->adjust(m_screen->time_until_pos(vc / 2, hdb), vc | (hdb << 16));
	return true;
}



/*************************************
 *
 *  Main CPU interrupts
 *
 *************************************/

void jaguar_state::update_cpu_irq()
{
	if ((m_cpu_irq_state & m_gpu_regs[INT1] & 0x1f) != 0)
		m_maincpu->set_input_line(m_is_r3000 ? R3000_IRQ4 : M68K_IRQ_6, ASSERT_LINE);
	else
		m_maincpu->set_input_line(m_is_r3000 ? R3000_IRQ4 : M68K_IRQ_6, CLEAR_LINE);
}


WRITE_LINE_MEMBER( jaguar_state::gpu_cpu_int )
{
	m_cpu_irq_state |= 2;
	update_cpu_irq();
}

WRITE_LINE_MEMBER( jaguar_state::dsp_cpu_int )
{
	m_cpu_irq_state |= 16;
	update_cpu_irq();
}



/*************************************
 *
 *  Palette init
 *
 *************************************/

void jaguar_state::set_palette(UINT16 vmode)
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

	int i;

	/* switch off the mode */
	switch (vmode & 0x106)
	{
		/* YCC full */
		case 0x000:
		/* RGB24 */
		case 0x002:
			for (i = 0; i < 65536; i++)
			{
				UINT8 r = (red_lookup[i >> 8] * (i & 0xff)) >> 8;
				UINT8 g = (grn_lookup[i >> 8] * (i & 0xff)) >> 8;
				UINT8 b = (blu_lookup[i >> 8] * (i & 0xff)) >> 8;
				m_pen_table[i] = rgb_t(r, g, b);
			}
			break;

		/* YCC VARMOD */
		case 0x100:
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
				m_pen_table[i] = rgb_t(r, g, b);
			}
			break;

		/* RGB VARMOD */
		case 0x106:
			for (i = 0; i < 65536; i++)
			{
				if (i & 1) // FIXME: controls RGB 5-5-5 / 5-6-5 format or it's just ignored? Used by UBI Soft logo in Rayman
					m_pen_table[i] = rgb_t(pal5bit(i >> 11), pal5bit(i >> 1), pal5bit(i >> 6));
				else
					m_pen_table[i] = rgb_t(pal5bit(i >> 11), pal6bit(i >> 0), pal5bit(i >> 6));
			}
			break;

		/* RGB full */
		case 0x006:
			for (i = 0; i < 65536; i++)
				m_pen_table[i] = rgb_t(pal5bit(i >> 11), pal6bit(i >> 0), pal5bit(i >> 6));
			break;

		/* others */
		default:
			logerror("Can't handle mode %X\n", vmode);
			fprintf(stderr, "Can't handle mode %X\n", vmode);
			break;
	}
}



/*************************************
 *
 *  32-bit access to the blitter
 *
 *************************************/

void jaguar_state::blitter_run()
{
	UINT32 command = m_blitter_regs[B_CMD] & STATIC_COMMAND_MASK;
	UINT32 a1flags = m_blitter_regs[A1_FLAGS] & STATIC_FLAGS_MASK;
	UINT32 a2flags = m_blitter_regs[A2_FLAGS] & STATIC_FLAGS_MASK;

	g_profiler.start(PROFILER_USER1);

	if (a1flags == a2flags)
	{
		if (command == 0x09800001 && a1flags == 0x010020)
		{
			blitter_09800001_010020_010020(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
			return;
		}
		if (command == 0x09800009 && a1flags == 0x000020)
		{
			blitter_09800009_000020_000020(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
			return;
		}
		if (command == 0x01800009 && a1flags == 0x000028)
		{
			blitter_01800009_000028_000028(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
			return;
		}

		if (command == 0x01800001 && a1flags == 0x000018)
		{
			blitter_01800001_000018_000018(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
			return;
		}

		if (command == 0x01c00001 && a1flags == 0x000018)
		{
			blitter_01c00001_000018_000018(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
			return;
		}
	}

	if (command == 0x00010000)
	{
		blitter_00010000_xxxxxx_xxxxxx(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
		return;
	}

	if (command == 0x01800001)
	{
		blitter_01800001_xxxxxx_xxxxxx(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
		return;
	}

	if ((command & 0x0ffff0ff) == 0x01800001)
	{
		blitter_x1800x01_xxxxxx_xxxxxx(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
		return;
	}

if (LOG_BLITTER_STATS)
{
static UINT32 blitter_stats[1000][4];
static UINT64 blitter_pixels[1000];
static int blitter_count = 0;
static int reps = 0;
int i;
for (i = 0; i < blitter_count; i++)
	if (blitter_stats[i][0] == (m_blitter_regs[B_CMD] & STATIC_COMMAND_MASK) &&
		blitter_stats[i][1] == (m_blitter_regs[A1_FLAGS] & STATIC_FLAGS_MASK) &&
		blitter_stats[i][2] == (m_blitter_regs[A2_FLAGS] & STATIC_FLAGS_MASK))
		break;
if (i == blitter_count)
{
	blitter_stats[i][0] = m_blitter_regs[B_CMD] & STATIC_COMMAND_MASK;
	blitter_stats[i][1] = m_blitter_regs[A1_FLAGS] & STATIC_FLAGS_MASK;
	blitter_stats[i][2] = m_blitter_regs[A2_FLAGS] & STATIC_FLAGS_MASK;
	blitter_stats[i][3] = 0;
	blitter_pixels[i] = 0;
	blitter_count++;
}
blitter_stats[i][3]++;
blitter_pixels[i] += (m_blitter_regs[B_COUNT] & 0xffff) * (m_blitter_regs[B_COUNT] >> 16);
if (++reps % 100 == 99)
{
	osd_printf_debug("---\nBlitter stats:\n");
	for (i = 0; i < blitter_count; i++)
		osd_printf_debug("  CMD=%08X A1=%08X A2=%08X %6d times, %08X%08X pixels\n",
				blitter_stats[i][0], blitter_stats[i][1], blitter_stats[i][2],
				blitter_stats[i][3], (UINT32)(blitter_pixels[i] >> 32), (UINT32)(blitter_pixels[i]));
	osd_printf_debug("---\n");
}
}

	generic_blitter(m_blitter_regs[B_CMD], m_blitter_regs[A1_FLAGS], m_blitter_regs[A2_FLAGS]);
	g_profiler.stop();
}

READ32_MEMBER( jaguar_state::blitter_r )
{
	switch (offset)
	{
		case B_CMD: /* B_CMD */
			return m_blitter_status & 3;

		default:
			logerror("%08X:Blitter read register @ F022%02X\n", space.device().safe_pcbase(), offset * 4);
			return 0;
	}
}


WRITE32_MEMBER( jaguar_state::blitter_w )
{
	COMBINE_DATA(&m_blitter_regs[offset]);
	if ((offset == B_CMD) && (mem_mask & 0x0000ffff))
	{
		m_blitter_status = 0;
		int inner_count = m_blitter_regs[B_COUNT] & 0xffff;
		int outer_count = m_blitter_regs[B_COUNT] >> 16;
		timer_set(attotime::from_ticks(inner_count * outer_count, JAGUAR_CLOCK), TID_BLITTER_DONE);
		blitter_run();
	}

	if (LOG_BLITTER_WRITE)
	logerror("%08X:Blitter write register @ F022%02X = %08X\n", space.device().safe_pcbase(), offset * 4, data);
}



/*************************************
 *
 *  16-bit TOM register access
 *
 *************************************/

READ16_MEMBER( jaguar_state::tom_regs_r )
{
	if (offset != INT1 && offset != INT2 && offset != HC && offset != VC)
		logerror("%08X:TOM read register @ F00%03X\n", space.device().safe_pcbase(), offset * 2);

	switch (offset)
	{
		case INT1:
			return m_cpu_irq_state;

		case HC:
			return m_screen->hpos() % (m_screen->width() / 2);

		case VC:
		{
			UINT8 half_line;

			if(m_screen->hpos() >= (m_screen->width() / 2))
				half_line = 1;
			else
				half_line = 0;

			return m_screen->vpos() * 2 + half_line;
		}
	}

	return m_gpu_regs[offset];
}

WRITE16_MEMBER( jaguar_state::tom_regs_w )
{
	UINT32 reg_store = m_gpu_regs[offset];
	attotime sample_period;
	if (offset < GPU_REGS)
	{
		COMBINE_DATA(&m_gpu_regs[offset]);

		switch (offset)
		{
			case MEMCON1:
				if((m_gpu_regs[offset] & 1) == 0)
					printf("Warning: ROMHI = 0!\n");

				break;
			case PIT0:
			case PIT1:
				if (m_gpu_regs[PIT0] && m_gpu_regs[PIT0] != 0xffff) //FIXME: avoid too much small timers for now
				{
					sample_period = attotime::from_ticks((1+m_gpu_regs[PIT0]) * (1+m_gpu_regs[PIT1]), JAGUAR_CLOCK/2);
					timer_set(sample_period, TID_PIT);
				}
				break;

			case INT1:
				m_cpu_irq_state &= ~(m_gpu_regs[INT1] >> 8);
				update_cpu_irq();
				break;

			case VMODE:
				if (reg_store != m_gpu_regs[offset])
					set_palette(m_gpu_regs[VMODE]);
				break;

			case OBF:   /* clear GPU interrupt */
				m_cpu_irq_state &= 0xfd;
				update_cpu_irq();
				break;

			case HP:
			case HBB:
			case HBE:
			case HDB1:
			case HDB2:
			case HDE:
			case VP:
			case VBB:
			case VBE:
			case VDB:
			case VDE:
			{
				if (reg_store != m_gpu_regs[offset])
				{
					int hperiod = 2 * ((m_gpu_regs[HP] & 0x3ff) + 1);
					int hbend = effective_hvalue(ENABLE_BORDERS ? m_gpu_regs[HBE] : MIN(m_gpu_regs[HDB1], m_gpu_regs[HDB2]));
					int hbstart = effective_hvalue(m_gpu_regs[ENABLE_BORDERS ? HBB : HDE]);
					int vperiod = (m_gpu_regs[VP] & 0x7ff) + 1;
					int vbend = MAX(m_gpu_regs[VBE],m_gpu_regs[VDB]) & 0x7ff;
					int vbstart = m_gpu_regs[VBB] & 0x7ff;

					/* adjust for the half-lines */
					if (hperiod != 0 && vperiod != 0 && hbend < hbstart && vbend < vbstart && hbstart < hperiod)
					{
						rectangle visarea(hbend / 2, hbstart / 2 - 1, vbend / 2, vbstart / 2 - 1);
						m_screen->configure(hperiod / 2, vperiod / 2, visarea, HZ_TO_ATTOSECONDS(double(m_pixel_clock) * 2 / hperiod / vperiod));
					}
				}
				break;
			}
		}
	}

	if (offset != INT2 && offset != VI && offset != INT1)
		logerror("%08X:TOM write register @ F00%03X = %04X\n", space.device().safe_pcbase(), offset * 2, data);
}



/*************************************
 *
 *  Gun input
 *
 *************************************/

READ32_MEMBER( jaguar_state::cojag_gun_input_r )
{
	int beamx, beamy;

	switch (offset)
	{
		case 0:
			get_crosshair_xy(1, beamx, beamy);
			return (beamy << 16) | (beamx ^ 0x1ff);

		case 1:
			get_crosshair_xy(0, beamx, beamy);
			return (beamy << 16) | (beamx ^ 0x1ff);

		case 2:
			return ioport("IN3")->read();
	}
	return 0;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void jaguar_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SCANLINE:
			scanline_update(param);
			break;

		case TID_BLITTER_DONE:
			m_blitter_status = 1;
			break;

		case TID_PIT:
			if (m_gpu_regs[INT1] & 0x8)
			{
				m_cpu_irq_state |= 8;
				update_cpu_irq();
			}
			if (m_gpu_regs[PIT0] != 0)
			{
				attotime sample_period = attotime::from_ticks((1+m_gpu_regs[PIT0]) * (1+m_gpu_regs[PIT1]), JAGUAR_CLOCK/2);
				timer_set(sample_period, TID_PIT);
			}
			break;

		case TID_SERIAL:
			serial_update();
			break;

		case TID_GPU_SYNC:
			// if a command is still pending, and we haven't maxed out our timer, set a new one
			if (m_gpu_command_pending && param < 1000)
				timer_set(attotime::from_usec(50), TID_GPU_SYNC, ++param);
			break;
	}
}

void jaguar_state::scanline_update(int param)
{
	int vc = param & 0xffff;
	int hdb = param >> 16;
	const rectangle &visarea = m_screen->visible_area();

	/* only run if video is enabled and we are past the "display begin" */
	if ((m_gpu_regs[VMODE] & 1) && vc >= (m_gpu_regs[VDB] & 0x7ff))
	{
		UINT32 *dest = &m_screen_bitmap.pix32(vc >> 1);
		int maxx = visarea.max_x;
		int hde = effective_hvalue(m_gpu_regs[HDE]) >> 1;
		UINT16 x,scanline[760];
		UINT8 y,pixel_width = ((m_gpu_regs[VMODE]>>10)&3)+1;

		/* if we are first on this scanline, clear to the border color */
		if (ENABLE_BORDERS && vc % 2 == 0)
		{
			rgb_t border = rgb_t(m_gpu_regs[BORD1] & 0xff, m_gpu_regs[BORD1] >> 8, m_gpu_regs[BORD2] & 0xff);
			for (x = visarea.min_x; x <= visarea.max_x; x++)
				dest[x] = border;
		}

		/* process the object list for this counter value */
		process_object_list(vc, scanline);

		/* copy the data to the target, clipping */
		if ((m_gpu_regs[VMODE] & 0x106) == 0x002)   /* RGB24 */
		{
			for (x = 0; x < 760 && hdb <= maxx && hdb < hde; x+=2)
				for (y = 0; y < pixel_width; y++)
				{
					UINT8 r = m_pen_table[(scanline[x]&0xff)|256];
					UINT8 g = m_pen_table[(scanline[x]>>8)|512];
					UINT8 b = m_pen_table[scanline[x+1]&0xff];
					dest[hdb++] = rgb_t(r, g, b);
				}
		}
		else
		{
			for (x = 0; x < 760 && hdb <= maxx && hdb < hde; x++)
				for (y = 0; y < pixel_width; y++)
					dest[hdb++] = m_pen_table[scanline[x]];
		}
	}

	/* adjust the timer in a loop, to handle missed cases */
	do
	{
		/* handle vertical interrupts */
		if (vc == m_gpu_regs[VI])
		{
			m_cpu_irq_state |= 1;
			update_cpu_irq();
		}

		/* point to the next counter value */
		if (++vc / 2 >= m_screen->height())
			vc = 0;

	} while (!adjust_object_timer(vc));
}

void jaguar_state::video_start()
{
	memset(&m_blitter_regs, 0, sizeof(m_blitter_regs));
	memset(&m_gpu_regs, 0, sizeof(m_gpu_regs));
	m_cpu_irq_state = 0;

	m_object_timer = timer_alloc(TID_SCANLINE);
	adjust_object_timer(0);

	m_screen_bitmap.allocate(760, 512);

	jagobj_init();

	save_item(NAME(m_pen_table));
	save_item(NAME(m_blitter_regs));
	save_item(NAME(m_gpu_regs));
	save_item(NAME(m_cpu_irq_state));
	m_pixel_clock = m_is_cojag ? COJAG_PIXEL_CLOCK : JAGUAR_CLOCK;
}


void jaguar_state::device_postload()
{
	update_cpu_irq();
}


/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 jaguar_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* if not enabled, just blank */
	if (!(m_gpu_regs[VMODE] & 1))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* render the object list */
	copybitmap(bitmap, m_screen_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}



/*************************************
 *
 *  Object processor
 *
 *************************************/

#define INCLUDE_OBJECT_PROCESSOR
#include "jagobj.inc"



/*************************************
 *
 *  Blitter macros
 *
 *************************************/

/* generic blitters */
#define FUNCNAME    generic_blitter
#define COMMAND     command
#define A1FIXED     a1flags
#define A2FIXED     a2flags
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

/* optimized common blitters */
#define FUNCNAME    blitter_09800001_010020_010020
#define COMMAND     0x09800001
#define A1FIXED     0x010020
#define A2FIXED     0x010020
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME    blitter_09800009_000020_000020
#define COMMAND     0x09800009
#define A1FIXED     0x000020
#define A2FIXED     0x000020
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME    blitter_01800009_000028_000028
#define COMMAND     0x01800009
#define A1FIXED     0x000028
#define A2FIXED     0x000028
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME    blitter_01800001_000018_000018
#define COMMAND     0x01800001
#define A1FIXED     0x000018
#define A2FIXED     0x000018
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME    blitter_01c00001_000018_000018
#define COMMAND     0x01c00001
#define A1FIXED     0x000018
#define A2FIXED     0x000018
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME


#define FUNCNAME    blitter_00010000_xxxxxx_xxxxxx
#define COMMAND     0x00010000
#define A1FIXED     a1flags
#define A2FIXED     a2flags
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME    blitter_01800001_xxxxxx_xxxxxx
#define COMMAND     0x01800001
#define A1FIXED     a1flags
#define A2FIXED     a2flags
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME

#define FUNCNAME    blitter_x1800x01_xxxxxx_xxxxxx
#define COMMAND     ((command & 0xf0000f00) | 0x01800001)
#define A1FIXED     a1flags
#define A2FIXED     a2flags
#include "jagblit.inc"
#undef A2FIXED
#undef A1FIXED
#undef COMMAND
#undef FUNCNAME
