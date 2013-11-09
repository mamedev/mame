/*****************************************************************************
 *
 *   Portable Xerox AltoII CPU core interface
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"

#pragma once

#ifndef _CPU_ALTO2_H_
#define _CPU_ALTO2_H

#define	ALTO2_TAG "alto2"

#ifndef	ALTO2_DEBUG
#define	ALTO2_DEBUG			1
#endif
#if	ALTO2_DEBUG
#include "debug/debugcon.h"
#endif

#define	USE_PRIO_F9318	0			//!< define to 1 to use the F9318 priority encoder code
#define	USE_ALU_74181	1			//!< define to 1 to use the SN74181 ALU code
#define	DEBUG_DISPLAY_TIMING	0	//!< define to 1 to debug the display timing

#define	ALTO2_TASKS		16			//!< 16 task slots
#define	ALTO2_REGS		32			//!< 32 16-bit words in the R register file
#define	ALTO2_ALUF		16			//!< 16 ALU functions (74181)
#define	ALTO2_BUSSRC	8			//!< 8 bus sources
#define	ALTO2_F1MAX		16			//!< 16 F1 functions
#define	ALTO2_F2MAX		16			//!< 16 F2 functions
#define	ALTO2_UCYCLE	170			//!< time in nano seconds for a CPU micro cycle

#define	ALTO2_ETHER_FIFO_SIZE	16

#ifndef	ALTO2_CRAM_CONFIG
#define	ALTO2_CRAM_CONFIG	2		//!< use default configuration 2
#endif

#if	(ALTO2_CRAM_CONFIG==1)
#define	ALTO2_UCODE_ROM_PAGES	1		//!< number of microcode ROM pages
#define	ALTO2_UCODE_RAM_PAGES	1		//!< number of microcode RAM pages
#elif (ALTO2_CRAM_CONFIG==2)
#define	ALTO2_UCODE_ROM_PAGES	2		//!< number of microcode ROM pages
#define	ALTO2_UCODE_RAM_PAGES	1		//!< number of microcode RAM pages
#elif (ALTO2_CRAM_CONFIG==3)
#define	ALTO2_UCODE_ROM_PAGES	1		//!< number of microcode ROM pages
#define	ALTO2_UCODE_RAM_PAGES	3		//!< number of microcode RAM pages
#else
#error "Undefined CROM/CRAM configuration"
#endif

/**
 * \brief number of S register banks
 * This depends on the number of RAM pages
 *   8 pages in 3K CRAM configuration
 *   1 page in 1K CRAM configurations
 */
#if	(ALTO2_UCODE_RAM_PAGES == 3)
#define	ALTO2_SREG_BANKS	8
#else
#define	ALTO2_SREG_BANKS	1
#endif

#define	ALTO2_UCODE_PAGE_SIZE	1024						//!< number of words of microcode
#define	ALTO2_UCODE_PAGE_MASK	(ALTO2_UCODE_PAGE_SIZE-1)	//!< mask for microcode ROM/RAM address
#define	ALTO2_UCODE_SIZE		((ALTO2_UCODE_ROM_PAGES + ALTO2_UCODE_RAM_PAGES) * ALTO2_UCODE_PAGE_SIZE)	//!< total number of words of microcode
#define	ALTO2_UCODE_RAM_BASE	(ALTO2_UCODE_ROM_PAGES * ALTO2_UCODE_PAGE_SIZE)	//!< base offset for the RAM page(s)
#define	ALTO2_CONST_SIZE		256							//!< number words in the constant ROM
#define	ALTO2_RAM_SIZE			0200000						//!< size of main memory in words
#define	ALTO2_IO_PAGE_BASE		0177000						//!< base address of the memory mapped io range
#define	ALTO2_IO_PAGE_SIZE		01000						//!< size of the memory mapped io range

//! inverted bits in the micro instruction 32 bit word
#define	ALTO2_UCODE_INVERTED	((1 << 10) | (1 << 15) | (1 << 19))

/**
 * @brief start value for the horizontal line counter
 *
 * This value is loaded into the three 4 bit counters (type 9316)
 * with numbers 65, 67, and 75.
 * 65: A=0 B=1 C=1 D=0
 * 67: A=1 B=0 C=0 D=1
 * 75: A=0 B=0 C=0 D=0
 *
 * The value is 150
 */
#define	ALTO2_DISPLAY_HLC_START (2+4+16+128)

/**
 * @brief end value for the horizontal line counter
 *
 * This is decoded by H30, an 8 input NAND gate.
 * The value is 1899; horz. line count range 150…1899 = 1750.
 *
 * There are 1750 / 2 = 875 total scanlines.
 */
#define	ALTO2_DISPLAY_HLC_END (1+2+8+32+64+256+512+1024)

/**
 * @brief display total height, including overscan (vertical blanking and synch)
 *
 * The display is interleaved in two fields, alternatingly drawing the even and odd
 * scanlines to the monitor. The frame rate is 60Hz, which is actually the rate
 * of the half-frames. The rate for full frames is thus 30Hz.
 */
#define	ALTO2_DISPLAY_TOTAL_HEIGHT ((ALTO2_DISPLAY_HLC_END + 1 - ALTO2_DISPLAY_HLC_START) / 2)

/**
 * @brief display total width, including horizontal blanking
 *
 * Known facts:
 *
 * We have 606x808 visible pixels, and the pixel clock is said to be 50ns
 * (20MHz), while the crystal in the schematics is labeled 20.16 MHz,
 * so the pixel clock would actually be 49.6031ns.
 *
 * The total number of scanlines is, according to the docs, 875.
 *
 * 875 scanlines at 30 frames per second, thus the scanline rate is 26.250 kHz.
 *
 * If I divide 20.16 MHz by 26.250 kHz, I get 768 pixels for the total width
 * of a scanline in pixels.
 *
 * The horizontal blanking period would then be 768 - 606 = 162 pixels, and
 * thus 162 * 49.6031ns ~= 8036ns = 8.036us for the HBLANK time.
 *
 * In the display schematics there is a divide by 24 logic, and when
 * dividing the 768 pixels per scanline by 24, we have 32 phases of a scanline.
 *
 * A S8223 PROM (a63) with 32x8 bits contains the status of the HBLANK and
 * HSYNC signals for these phases, the SCANEND and HLCGATE signals, as well
 * as its own next address A0-A3!
 *
 */
#define	ALTO2_DISPLAY_TOTAL_WIDTH 768


#define	ALTO2_DISPLAY_SCANLINE_WORDS (ALTO2_DISPLAY_TOTAL_WIDTH/16)		//!< words per scanline
#define	ALTO2_DISPLAY_HEIGHT 808										//!< number of visible scanlines per frame; 808 really, but there are some empty lines?
#define	ALTO2_DISPLAY_WIDTH 606											//!< visible width of the display
#define	ALTO2_DISPLAY_VISIBLE_WORDS ((ALTO2_DISPLAY_WIDTH+15)/16)		//!< visible words per scanline
#define	ALTO2_DISPLAY_BITCLOCK 20160000ll								//!< display bit clock in in Hertz (20.16MHz)
#define	ALTO2_DISPLAY_BITTIME(n) (HZ_TO_ATTOSECONDS(ALTO2_DISPLAY_BITCLOCK)*(n))		//!< display bit time in in atto seconds (~= 49.6031ns)
#define	ALTO2_DISPLAY_SCANLINE_TIME	ALTO2_DISPLAY_BITTIME(ALTO2_DISPLAY_TOTAL_WIDTH)	//!< time for a scanline in nano seconds (768 * 49.6031ns)
#define	ALTO2_DISPLAY_VISIBLE_TIME ALTO2_DISPLAY_BITTIME(ALTO2_DISPLAY_WIDTH)	//!< time of the visible part of a scanline (606 * 49.6031ns)
#define	ALTO2_DISPLAY_WORD_TIME	ALTO2_DISPLAY_BITTIME(16)				//!< time for a word (16 pixels * 49.6031ns)
#define	ALTO2_DISPLAY_VBLANK_TIME ((ALTO2_DISPLAY_TOTAL_HEIGHT-ALTO2_DISPLAY_HEIGHT)*HZ_TO_ATTOSECONDS(26250))
#define	ALTO2_DISPLAY_FIFO 16											//!< the display fifo has 16 words

enum {
	A2_TASK, A2_BUS, A2_T, A2_ALU, A2_ALUC0,
	A2_L, A2_SHIFTER, A2_LALUC0, A2_M,
	A2_R,	// 32 R registers
	A2_AC3 = A2_R, A2_AC2, A2_AC1, A2_AC0, A2_R04, A2_R05, A2_PC,  A2_R07,
	A2_R10, A2_R11, A2_R12, A2_R13, A2_R14, A2_R15, A2_R16, A2_R17,
	A2_R20, A2_R21, A2_R22, A2_R23, A2_R24, A2_R25, A2_R26, A2_R27,
	A2_R30, A2_R31, A2_R32, A2_R33, A2_R34, A2_R35, A2_R36, A2_R37,
	A2_S,	// 32 S registers
	A2_S00 = A2_S, A2_S01, A2_S02, A2_S03, A2_S04, A2_S05, A2_S06, A2_S07,
	A2_S10, A2_S11, A2_S12, A2_S13, A2_S14, A2_S15, A2_S16, A2_S17,
	A2_S20, A2_S21, A2_S22, A2_S23, A2_S24, A2_S25, A2_S26, A2_S27,
	A2_S30, A2_S31, A2_S32, A2_S33, A2_S34, A2_S35, A2_S36, A2_S37
};

/**
 * @brief enumeration of the inputs and outputs of a JK flip-flop type 74109
 * <PRE>
 * 74109
 * Dual J-/K flip-flops with set and reset.
 *
 *       +----------+           +-----------------------------+
 * /1RST |1  +--+ 16| VCC       | J |/K |CLK|/SET|/RST| Q |/Q |
 *    1J |2       15| /2RST     |---+---+---+----+----+---+---|
 *   /1K |3       14| 2J        | X | X | X |  0 |  0 | 1 | 1 |
 *  1CLK |4   74  13| /2K       | X | X | X |  0 |  1 | 1 | 0 |
 * /1SET |5  109  12| 2CLK      | X | X | X |  1 |  0 | 0 | 1 |
 *    1Q |6       11| /2SET     | 0 | 0 | / |  1 |  1 | 0 | 1 |
 *   /1Q |7       10| 2Q        | 0 | 1 | / |  1 |  1 | - | - |
 *   GND |8        9| /2Q       | 1 | 0 | / |  1 |  1 |/Q | Q |
 *       +----------+           | 1 | 1 | / |  1 |  1 | 1 | 0 |
 *                              | X | X |!/ |  1 |  1 | - | - |
 *                              +-----------------------------+
 *
 * [This information is part of the GIICM]
 * </PRE>
 */
typedef enum {
	JKFF_0,					//!< no inputs or outputs
	JKFF_CLK	= (1 << 0),	//!< clock signal
	JKFF_J		= (1 << 1),	//!< J input
	JKFF_K		= (1 << 2),	//!< K' input
	JKFF_S		= (1 << 3),	//!< S' input
	JKFF_C		= (1 << 4),	//!< C' input
	JKFF_Q		= (1 << 5),	//!< Q  output
	JKFF_Q0		= (1 << 6)	//!< Q' output
}	jkff_t;

class alto2_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	alto2_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	//! device-level override for start
	virtual void device_start();
	//! device-level override for reset
	virtual void device_reset();

	//! device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 1; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	//! device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		switch (spacenum) {
		case AS_PROGRAM:
			return &m_ucode_config;
		case AS_DATA:
			return &m_const_config;
		case AS_IO:
			return &m_ram_config;
		default:
			return NULL;
		}
	}

	//! device (P)ROMs
	virtual const rom_entry *device_rom_region() const;

	//! device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	//! device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
#if	ALTO2_DEBUG
	enum {
		LOG_0,
		LOG_CPU		= (1 <<  0),
		LOG_EMU		= (1 <<  1),
		LOG_T01		= (1 <<  2),
		LOG_T02		= (1 <<  3),
		LOG_T03		= (1 <<  4),
		LOG_KSEC	= (1 <<  5),
		LOG_T05		= (1 <<  6),
		LOG_T06		= (1 <<  7),
		LOG_ETH		= (1 <<  8),
		LOG_MRT		= (1 <<  9),
		LOG_DWT		= (1 << 10),
		LOG_CURT	= (1 << 11),
		LOG_DHT		= (1 << 12),
		LOG_DVT		= (1 << 13),
		LOG_PART	= (1 << 14),
		LOG_KWD		= (1 << 15),
		LOG_T17		= (1 << 16),
		LOG_MEM		= (1 << 17),
		LOG_RAM		= (1 << 18),
		LOG_DRIVE	= (1 << 19),
		LOG_DISK	= (1 << 20),
		LOG_DISPL	= (1 << 21),
		LOG_MOUSE	= (1 << 22),
		LOG_ALL		= ((1 << 23) - 1)
	};
	int m_log_types;
	int m_log_level;
	bool m_log_newline;
	void logprintf(int type, int level, const char* format, ...);
#	define	LOG(x) logprintf x
#else
#	define	LOG(x)
#endif

	void fatal(int level, const char *format, ...);

	address_space_config m_ucode_config;
	address_space_config m_const_config;
	address_space_config m_ram_config;

	address_space *m_ucode;
	address_space *m_const;
	address_space *m_ram;
	int m_icount;

	static const UINT8 m_ether_id = 0121;

	typedef void (alto2_cpu_device::*a2func)();
	typedef void (alto2_cpu_device::*a2cb)(int unit);
	typedef void (alto2_cpu_device::*a2io_wr)(UINT32 addr, UINT16 data);
	typedef UINT16 (alto2_cpu_device::*a2io_rd)(UINT32 addr);

	//! task numbers
	enum {
		task_emu,		//!< emulator task
		task_1,			//!< unused
		task_2,			//!< unused
		task_3,			//!< unused
		task_ksec,		//!< disk sector task
		task_5,			//!< unused
		task_6,			//!< unused
		task_ether,		//!< ethernet task
		task_mrt,		//!< memory refresh task
		task_dwt,		//!< display word task
		task_curt,		//!< cursor task
		task_dht,		//!< display horizontal task
		task_dvt,		//!< display vertical task
		task_part,		//!< parity task
		task_kwd,		//!< disk word task
		task_17			//!< unused task slot 017
	};

	//! register select values accessing R (Note: register numbers are octal)
	enum {
		rsel_ac3,		//!< AC3 used by emulator as accu 3. Also used by Mesa emulator to keep bytecode to execute after breakpoint
		rsel_ac2,		//!< AC2 used by emulator as accu 2. Also used by Mesa emulator as x register for xfer
		rsel_ac1,		//!< AC1 used by emulator as accu 1. Also used by Mesa emulator as r-temporary for return indices and values
		rsel_ac0,		//!< AC0 used by emulator as accu 0. Also used by Mesa emulator as new field bits for WF and friends
		rsel_r04,		//!< NWW state of the interrupt system
		rsel_r05,		//!< SAD. Also used by Mesa emulator as scratch R-register for counting
		rsel_pc,		//!< PC used by emulator as program counter
		rsel_r07,		//!< XREG. Also used by Mesa emulator as task hole, i.e. pigeonhole for saving things across tasks.
		rsel_r10,		//!< XH. Also used by Mesa emulator as instruction byte register
		rsel_r11,		//!< CLOCKTEMP - used in the MRT
		rsel_r12,		//!< ECNTR remaining words in buffer - ETHERNET
		rsel_r13,		//!< EPNTR points BEFORE next word in buffer - ETHERNET
		rsel_r14,
		rsel_r15,		//!< MPC. Used by the Mesa emulator as program counter
		rsel_r16,		//!< STKP. Used by the Mesa emulator as stack pointer [0-10] 0 empty, 10 full
		rsel_r17,		//!< XTSreg. Used by the Mesa emulator to xfer trap state
		rsel_r20,		//!< CURX. Holds cursor X; used by the cursor task
		rsel_r21,		//!< CURDATA. Holds the cursor data; used by the cursor task
		rsel_r22,		//!< CBA. Holds the address of the currently active DCB+1
		rsel_r23,		//!< AECL. Holds the address of the end of the current scanline's bitmap
		rsel_r24,		//!< SLC. Holds the number of scanlines remaining in currently active DCB
		rsel_r25,		//!< MTEMP. Holds the temporary cell
		rsel_r26,		//!< HTAB. Holds the number of tab words remaining on current scanline
		rsel_r27,		//!< YPOS
		rsel_r30,		//!< DWA. Holds the address of the bit map doubleword currently being fetched for transmission to the hardware buffer.
		rsel_r31,		//!< KWDCT. Used by the disk tasks as word counter
		rsel_r32,		//!< CKSUMR. Used by the disk tasks as checksum register (and *amble counter?)
		rsel_r33,		//!< KNMAR. Used by the disk tasks as transfer memory address register
		rsel_r34,		//!< DCBR. Used by the disk tasks to keep the current device control block
		rsel_r35,		//!< TEMP. Used by the Mesa emulator, and also by BITBLT
		rsel_r36,		//!< TEMP2. Used by the Mesa emulator, and also by BITBLT
		rsel_r37		//!< CLOCKREG. Low order bits of the real time clock
	};

	//! ALU function numbers
	enum {
		/**
		 * \brief 00: ALU <- BUS
		 * PROM data for S3-0,M,C: 1111/1/0
		 * function F=A
		 * T source is ALU
		 */
		aluf_bus__alut,
		/**
		 * \brief 01: ALU <- T
		 * PROM data for S3-0,M,C: 1010/1/0
		 * function F=B
		 * T source is BUS
		 */
		aluf_treg,
		/**
		 * \brief 02: ALU <- BUS | T
		 * PROM data for S3-0,M,C: 1110/1/0
		 * function F=A|B
		 * T source is ALU
		 */
		aluf_bus_or_t__alut,
		/**
		 * \brief 03: ALU <- BUS & T
		 * PROM data for S3-0,M,C: 1011/1/0
		 * function F=A&B
		 * T source is BUS
		 */
		aluf_bus_and_t,
		/**
		 * \brief 04: ALU <- BUS ^ T
		 * PROM data for S3-0,M,C: 0110/1/0
		 * function F=A^B
		 * T source is BUS
		 */
		aluf_bus_xor_t,
		/**
		 * \brief 05: ALU <- BUS + 1
		 * PROM data for S3-0,M,C: 0000/0/0
		 * function F=A+1
		 * T source is ALU
		 */
		aluf_bus_plus_1__alut,
		/**
		 * \brief 06: ALU <- BUS - 1
		 * PROM data for S3-0,M,C: 1111/0/1
		 * function F=A-1
		 * T source is ALU
		 */
		aluf_bus_minus_1__alut,
		/**
		 * \brief 07: ALU <- BUS + T
		 * PROM data for S3-0,M,C: 1001/0/1
		 * function F=A+B
		 * T source is BUS
		 */
		aluf_bus_plus_t,
		/**
		 * \brief 10: ALU <- BUS - T
		 * PROM data for S3-0,M,C: 0110/0/0
		 * function F=A-B
		 * T source is BUS
		 */
		aluf_bus_minus_t,
		/**
		 * \brief 11: ALU <- BUS - T - 1
		 * PROM data for S3-0,M,C: 0110/0/1
		 * function F=A-B-1
		 * T source is BUS
		 */
		aluf_bus_minus_t_minus_1,
		/**
		 * \brief 12: ALU <- BUS + T + 1
		 * PROM data for S3-0,M,C: 1001/0/0
		 * function F=A+B+1
		 * T source is ALU
		 */
		aluf_bus_plus_t_plus_1__alut,
		/**
		 * \brief 13: ALU <- BUS + SKIP
		 * PROM data for S3-0,M,C: 0000/0/SKIP
		 * function F=A (SKIP=1) or F=A+1 (SKIP=0)
		 * T source is ALU
		 */
		aluf_bus_plus_skip__alut,
		/**
		 * \brief 14: ALU <- BUS & T
		 * PROM data for S3-0,M,C: 1011/1/0
		 * function F=A&B
		 * T source is ALU
		 */
		aluf_bus_and_t__alut,
		/**
		 * \brief 15: ALU <- BUS & ~T
		 * PROM data for S3-0,M,C: 0111/1/0
		 * function F=A&~B
		 * T source is BUS
		 */
		aluf_bus_and_not_t,
		/**
		 * \brief 16: ALU <- ???
		 * PROM data for S3-0,M,C: ????/?/?
		 * perhaps F=0 (0011/0/0)
		 * T source is BUS
		 */
		aluf_undef_16,
		/**
		 * \brief 17: ALU <- ???
		 * PROM data for S3-0,M,C: ????/?/?
		 * perhaps F=0 (0011/0/0)
		 * T source is BUS
		 */
		aluf_undef_17
	};

	//! BUS source selection numbers
	enum {
		bs_read_r,							//!< BUS source is R register
		bs_load_r,							//!< load R register from BUS
		bs_no_source,						//!< BUS is open (0177777)
		bs_task_3,							//!< BUS source is task specific
		bs_task_4,							//!< BUS source is task specific
		bs_read_md,							//!< BUS source is memory data
		bs_mouse,							//!< BUS source is mouse data
		bs_disp,							//!< BUS source displacement

		bs_ram_read_slocation= bs_task_3,	//!< ram related: read S register
		bs_ram_load_slocation= bs_task_4,	//!< ram related: load S register

		bs_emu_read_sreg	= bs_task_3,	//!< emulator task: read S register
		bs_emu_load_sreg	= bs_task_4,	//!< emulator task: load S register from BUS

		bs_ksec_read_kstat	= bs_task_3,	//!< disk sector task: read status register
		bs_ksec_read_kdata	= bs_task_4,	//!< disk sector task: read data register

		bs_ether_eidfct		= bs_task_3,	//!< ethernet task: Ethernet input data function

		bs_kwd_read_kstat	= bs_task_3,	//!< disk word task: read status register
		bs_kwd_read_kdata	= bs_task_4		//!< disk word task: read data register
	};

	//! Function 1 numbers
	enum {
		f1_nop,								//!< f1 (0000) no operation
		f1_load_mar,						//!< f1 (0001) load memory address register
		f1_task,							//!< f1 (0010) task switch
		f1_block,							//!< f1 (0011) block task
		f1_l_lsh_1,							//!< f1 (0100) left shift L once
		f1_l_rsh_1,							//!< f1 (0101) right shift L once
		f1_l_lcy_8,							//!< f1 (0110) cycle L 8 times
		f1_const,							//!< f1 (0111) constant from PROM

		f1_task_10,							//!< f1 (1000) task specific
		f1_task_11,							//!< f1 (1001) task specific
		f1_task_12,							//!< f1 (1010) task specific
		f1_task_13,							//!< f1 (1011) task specific
		f1_task_14,							//!< f1 (1100) task specific
		f1_task_15,							//!< f1 (1101) task specific
		f1_task_16,							//!< f1 (1110) task specific
		f1_task_17,							//!< f1 (1111) task specific

		f1_ram_swmode		= f1_task_10,	//!< f1 (1000) ram related: switch mode to CROM/CRAM in same page
		f1_ram_wrtram		= f1_task_11,	//!< f1 (1001) ram related: start WRTRAM cycle
		f1_ram_rdram		= f1_task_12,	//!< f1 (1010) ram related: start RDRAM cycle
#if	(ALTO2_UCODE_RAM_PAGES == 3)
		f1_ram_load_rmr		= f1_task_13,	//!< f1 (1011) ram related: load the reset mode register
#else	// ALTO2_UCODE_RAM_PAGES != 3
		f1_ram_load_srb		= f1_task_13,	//!< f1 (1011) ram related: load the S register bank from BUS[12-14]
#endif

		f1_emu_swmode		= f1_task_10,	//!< f1 (1000) emu: switch mode; branch to ROM/RAM microcode
		f1_emu_wrtram		= f1_task_11,	//!< f1 (1001) emu: write microcode RAM
		f1_emu_rdram		= f1_task_12,	//!< f1 (1010) emu: read microcode RAM
		f1_emu_load_rmr		= f1_task_13,	//!< f1 (1011) emu: load reset mode register
											//!< f1 (1100) emu: undefined
		f1_emu_load_esrb	= f1_task_15,	//!< f1 (1101) emu: load extended S register bank
		f1_emu_rsnf			= f1_task_16,	//!< f1 (1110) emu: read serial number (Ethernet ID)
		f1_emu_startf		= f1_task_17,	//!< f1 (1111) emu: start I/O hardware (Ethernet)

		f1_ksec_strobe		= f1_task_11,	//!< f1 (1001) ksec: strobe
		f1_ksec_load_kstat	= f1_task_12,	//!< f1 (1010) ksec: load kstat register
		f1_ksec_increcno	= f1_task_13,	//!< f1 (1011) ksec: increment record number
		f1_ksec_clrstat		= f1_task_14,	//!< f1 (1100) ksec: clear status register
		f1_ksec_load_kcom	= f1_task_15,	//!< f1 (1101) ksec: load kcom register
		f1_ksec_load_kadr	= f1_task_16,	//!< f1 (1110) ksec: load kadr register
		f1_ksec_load_kdata	= f1_task_17,	//!< f1 (1111) ksec: load kdata register

		f1_ether_eilfct		= f1_task_13,	//!< f1 (1011) ether: Ethernet input look function
		f1_ether_epfct		= f1_task_14,	//!< f1 (1100) ether: Ethernet post function
		f1_ether_ewfct		= f1_task_15,	//!< f1 (1101) ether: Ethernet countdown wakeup function

		f1_kwd_strobe		= f1_task_11,	//!< f1 (1001) kwd: strobe
		f1_kwd_load_kstat	= f1_task_12,	//!< f1 (1010) kwd: load kstat register
		f1_kwd_increcno		= f1_task_13,	//!< f1 (1011) kwd: increment record number
		f1_kwd_clrstat		= f1_task_14,	//!< f1 (1100) kwd: clear status register
		f1_kwd_load_kcom	= f1_task_15,	//!< f1 (1101) kwd: load kcom register
		f1_kwd_load_kadr	= f1_task_16,	//!< f1 (1110) kwd: load kadr register
		f1_kwd_load_kdata	= f1_task_17,	//!< f1 (1111) kwd: load kdata register
	};

	//! Function 2 numbers
	enum {
		f2_nop,								//!< f2 00 no operation
		f2_bus_eq_zero,						//!< f2 01 branch on bus equals 0
		f2_shifter_lt_zero,					//!< f2 02 branch on shifter less than 0
		f2_shifter_eq_zero,					//!< f2 03 branch on shifter equals 0
		f2_bus,								//!< f2 04 branch on BUS[6-15]
		f2_alucy,							//!< f2 05 branch on (latched) ALU carry
		f2_load_md,							//!< f2 06 load memory data
		f2_const,							//!< f2 07 constant from PROM
		f2_task_10,							//!< f2 10 task specific
		f2_task_11,							//!< f2 11 task specific
		f2_task_12,							//!< f2 12 task specific
		f2_task_13,							//!< f2 13 task specific
		f2_task_14,							//!< f2 14 task specific
		f2_task_15,							//!< f2 15 task specific
		f2_task_16,							//!< f2 16 task specific
		f2_task_17,							//!< f2 17 task specific

		f2_emu_busodd		= f2_task_10,	//!< f2 (1000) emu: branch on bus odd
		f2_emu_magic		= f2_task_11,	//!< f2 (1001) emu: magic shifter (MRSH 1: shifter[15]=T[0], MLSH 1: shifter[015])
		f2_emu_load_dns		= f2_task_12,	//!< f2 (1010) emu: do novel shift (RSH 1: shifter[15]=XC, LSH 1: shifer[0]=XC)
		f2_emu_acdest		= f2_task_13,	//!< f2 (1011) emu: destination accu
		f2_emu_load_ir		= f2_task_14,	//!< f2 (1100) emu: load instruction register and branch
		f2_emu_idisp		= f2_task_15,	//!< f2 (1101) emu: load instruction displacement and branch
		f2_emu_acsource		= f2_task_16,	//!< f2 (1110) emu: source accu

		f2_ksec_init		= f2_task_10,	//!< f2 (1000) ksec: branches NEXT[5-9] on WDTASKACT && WDINIT
		f2_ksec_rwc			= f2_task_11,	//!< f2 (1001) ksec: branches NEXT[8-9] on READ/WRITE/CHECK for record
		f2_ksec_recno		= f2_task_12,	//!< f2 (1010) ksec: branches NEXT[8-9] on RECNO[0-1]
		f2_ksec_xfrdat		= f2_task_13,	//!< f2 (1011) ksec: branches NEXT[9] on !SEEKONLY
		f2_ksec_swrnrdy		= f2_task_14,	//!< f2 (1100) ksec: branches NEXT[9] on !SWRDY
		f2_ksec_nfer		= f2_task_15,	//!< f2 (1101) ksec: branches NEXT[9] on !KFER
		f2_ksec_strobon		= f2_task_16,	//!< f2 (1110) ksec: branches NEXT[9] on STROBE

		f2_ether_eodfct		= f2_task_10,	//!< f2 (1000) ether: Ethernet output data function
		f2_ether_eosfct		= f2_task_11,	//!< f2 (1001) ether: Ethernet output start function
		f2_ether_erbfct		= f2_task_12,	//!< f2 (1010) ether: Ethernet reset branch function
		f2_ether_eefct		= f2_task_13,	//!< f2 (1011) ether: Ethernet end of transmission function
		f2_ether_ebfct		= f2_task_14,	//!< f2 (1100) ether: Ethernet branch function
		f2_ether_ecbfct		= f2_task_15,	//!< f2 (1101) ether: Ethernet countdown branch function
		f2_ether_eisfct		= f2_task_16,	//!< f2 (1110) ether: Ethernet input start function

		f2_dwt_load_ddr		= f2_task_10,	//!< f2 (1000) dwt: load display data register

		f2_curt_load_xpreg	= f2_task_10,	//!< f2 (1000) curt: load x position register
		f2_curt_load_csr	= f2_task_11,	//!< f2 (1001) curt: load cursor shift register

		f2_dht_evenfield	= f2_task_10,	//!< f2 (1000) dht: load even field
		f2_dht_setmode		= f2_task_11,	//!< f2 (1001) dht: set mode

		f2_dvt_evenfield	= f2_task_10,	//!< f2 (1000) dvt: load even field

		f2_kwd_init			= f2_task_10,	//!< f2 (1000) kwd: branches NEXT[5-9] on WDTASKACT && WDINIT
		f2_kwd_rwc			= f2_task_11,	//!< f2 (1001) kwd: branches NEXT[8-9] on READ/WRITE/CHECK for record
		f2_kwd_recno		= f2_task_12,	//!< f2 (1010) kwd: branches NEXT[8-9] on RECNO[0-1]
		f2_kwd_xfrdat		= f2_task_13,	//!< f2 (1011) kwd: branches NEXT[9] on !SEEKONLY
		f2_kwd_swrnrdy		= f2_task_14,	//!< f2 (1100) kwd: branches NEXT[9] on !SWRDY
		f2_kwd_nfer			= f2_task_15,	//!< f2 (1101) kwd: branches NEXT[9] on !KFER
		f2_kwd_strobon		= f2_task_16,	//!< f2 (1110) kwd: branches NEXT[9] on STROBE
	};

	enum {
		p_dynamic,			//!< dynamic functions (e.g. ALU and SHIFTER operations)
		p_latches			//!< latching function (T, L, M, R, etc. register latch)
	};


	/**
	 * Bit field primitives
	 * These are some inline functions to make it easier to access variable by the
	 * bit-reversed notation that the Xerox Alto documents use all over the place.
	 * Bit number 0 is the most significant there, and bit number (width - 1)
	 * is the least significant.
	 */

	//! get the left shift required to access bit %to in a word of %width bits
	static inline UINT8 A2_BITSHIFT(UINT8 width, UINT8 to) { return width - 1 - to; }

	//! build a least significant bit mask for bits %from to %to (inclusive)
	static inline UINT32 A2_BITMASK(UINT8 from, UINT8 to) { return (1ul << (to + 1 - from)) - 1; }

	//! get a single bit number %bit value from %reg, a word of %width bits
	static inline UINT8 A2_BIT8(UINT8 reg, UINT8 width, UINT8 bit) { return (reg >> A2_BITSHIFT(width,bit)) & 1; }

	//! get a bit field from %reg, a word of %width bits, starting at bit %from until bit %to
	static inline UINT8 A2_GET8(UINT8 reg, UINT8 width, UINT8 from, UINT8 to) { return (reg >> A2_BITSHIFT(width,to)) & A2_BITMASK(from,to); }

	//! put a value %val into %reg, a word of %width bits, starting at bit %from until bit %to
	static inline void A2_PUT8(UINT8& reg, UINT8 width, UINT8 from, UINT8 to, UINT8 val) {
		UINT8 mask = A2_BITMASK(from,to) << A2_BITSHIFT(width,to);
		reg = (reg & ~mask) | ((val << A2_BITSHIFT(width,to)) & mask);
	}

	//! get a single bit number %bit value from %reg, a word of %width bits
	static inline UINT16 A2_BIT16(UINT16 reg, UINT8 width, UINT8 bit) { return (reg >> A2_BITSHIFT(width,bit)) & 1; }

	//! get a bit field from %reg, a word of %width bits, starting at bit %from until bit %to
	static inline UINT16 A2_GET16(UINT16 reg, UINT8 width, UINT8 from, UINT8 to) { return (reg >> A2_BITSHIFT(width,to)) & A2_BITMASK(from,to); }

	//! put a value %val into %reg, a word of %width bits, starting at bit %from until bit %to
	static inline void A2_PUT16(UINT16& reg, UINT8 width, UINT8 from, UINT8 to, UINT16 val) {
		UINT16 mask = A2_BITMASK(from,to) << A2_BITSHIFT(width,to);
		reg = (reg & ~mask) | ((val << A2_BITSHIFT(width,to)) & mask);
	}

	//! get a single bit number %bit value from %reg, a word of %width bits
	static inline UINT32 A2_BIT32(UINT32 reg, UINT8 width, UINT8 bit) { return (reg >> A2_BITSHIFT(width,bit)) & 1; }

	//! get a bit field from %reg, a word of %width bits, starting at bit %from until bit %to
	static inline UINT32 A2_GET32(UINT32 reg, UINT8 width, UINT8 from, UINT8 to) { return (reg >> A2_BITSHIFT(width,to)) & A2_BITMASK(from,to); }

	//! put a value %val into %reg, a word of %width bits, starting at bit %from until bit %to
	static inline void A2_PUT32(UINT32& reg, UINT8 width, UINT8 from, UINT8 to, UINT32 val) {
		UINT32 mask = A2_BITMASK(from,to) << A2_BITSHIFT(width,to);
		reg = (reg & ~mask) | ((val << A2_BITSHIFT(width,to)) & mask);
	}

	//! enumeration of the micro code word bits
	//! Note: The Alto documents enumerate bits from left (MSB = 0) to right (LSB = 31)
	enum {
		DRSEL0, DRSEL1, DRSEL2, DRSEL3, DRSEL4,
		DALUF0, DALUF1, DALUF2, DALUF3,
		DBS0, DBS1, DBS2,
		DF1_0, DF1_1, DF1_2, DF1_3,
		DF2_0, DF2_1, DF2_2, DF2_3,
		LOADT,
		LOADL,
		NEXT0, NEXT1, NEXT2, NEXT3, NEXT4, NEXT5, NEXT6, NEXT7, NEXT8, NEXT9
	};

	//! get the RSEL value from a micro instruction word
	static inline UINT32 MIR_RSEL(UINT32 mir) { return A2_GET32(mir, 32, DRSEL0, DRSEL4); }

	//! get the ALUF value from a micro instruction word
	static inline UINT32 MIR_ALUF(UINT32 mir) { return A2_GET32(mir, 32, DALUF0, DALUF3); }

	//! get the BS value from a micro instruction word
	static inline UINT32 MIR_BS(UINT32 mir) { return A2_GET32(mir, 32, DBS0, DBS2); }

	//! get the F1 value from a micro instruction word
	static inline UINT32 MIR_F1(UINT32 mir) { return A2_GET32(mir, 32, DF1_0, DF1_3); }

	//! get the F2 value from a micro instruction word
	static inline UINT32 MIR_F2(UINT32 mir) { return A2_GET32(mir, 32, DF2_0, DF2_3); }

	//! get the T value from a micro instruction word
	static inline UINT32 MIR_T(UINT32 mir) { return A2_BIT32(mir, 32, LOADT); }

	//! get the L value from a micro instruction word
	static inline UINT32 MIR_L(UINT32 mir) { return A2_BIT32(mir, 32, LOADL); }

	//! get the NEXT value from a micro instruction word
	static inline UINT32 MIR_NEXT(UINT32 mir) { return A2_GET32(mir, 32, NEXT0, NEXT9); }

	//! get the normally accessed bank number from a bank register
	static inline UINT16 GET_BANK_NORMAL(UINT16 breg) { return A2_GET16(breg,16,12,13); }

	//! get the extended bank number (accessed via XMAR) from a bank register
	static inline UINT16 GET_BANK_EXTENDED(UINT16 breg) { return A2_GET16(breg,16,14,15); }

	//! get an ignored bit field from a control RAM address
	static inline UINT16 GET_CRAM_IGNORE(UINT16 addr) { return A2_GET16(addr,16,0,1); }

	//! get the bank select bit field from a control RAM address
	static inline UINT16 GET_CRAM_BANKSEL(UINT16 addr) { return A2_GET16(addr,16,2,3); }

	//! get the ROM/RAM flag from a control RAM address
	static inline UINT16 GET_CRAM_RAMROM(UINT16 addr) { return A2_GET16(addr,16,4,4); }

	//! get the half select flag from a control RAM address
	static inline UINT16 GET_CRAM_HALFSEL(UINT16 addr) { return A2_GET16(addr,16,5,5); }

	//! get the word address bit field from a control RAM address
	static inline UINT16 GET_CRAM_WORDADDR(UINT16 addr)	{ return A2_GET16(addr,16,6,15); }

	UINT16 m_task_mpc[ALTO2_TASKS];					//!< per task micro program counter
	UINT16 m_task_next2[ALTO2_TASKS];				//!< per task address modifier
	attoseconds_t m_ntime[ALTO2_TASKS];				//!< per task atto seconds executed
	UINT8 m_task;									//!< active task
	UINT8 m_next_task;								//!< next micro instruction's task
	UINT8 m_next2_task;								//!< next but one micro instruction's task
	UINT16 m_mpc;									//!< micro program counter
	UINT32 m_mir;									//!< micro instruction register

	/**
	 * \brief current micro instruction's register selection
	 * The emulator F2s ACSOURCE and ACDEST modify this.
	 * Note: S registers are addressed by the original RSEL[0-4],
	 * even when the the emulator modifies this.
	 */
	UINT8 m_rsel;

	UINT16 m_next;									//!< current micro instruction's next
	UINT16 m_next2;									//!< next micro instruction's next
	UINT16 m_r[ALTO2_REGS];							//!< R register file
	UINT16 m_s[ALTO2_SREG_BANKS][ALTO2_REGS];		//!< S register file(s)
	UINT16 m_bus;									//!< wired-AND bus
	UINT16 m_t;										//!< T register
	UINT16 m_alu;									//!< the current ALU
	UINT16 m_aluc0;									//!< the current ALU carry output
	UINT16 m_l;										//!< L register
	UINT16 m_shifter;								//!< shifter output
	UINT16 m_laluc0;								//!< the latched ALU carry output
	UINT16 m_m;										//!< M register of RAM related tasks (MYL latch in the schematics)
	UINT16 m_cram_addr;								//!< constant RAM address
	UINT16 m_task_wakeup;							//!< task wakeup: bit 1<<n set if task n requesting service
	a2func m_active_callback[ALTO2_TASKS];			//!< task activation callbacks

	UINT16 m_reset_mode;							//!< reset mode register: bit 1<<n set if task n starts in ROM
	bool m_rdram_flag;								//!< set by rdram, action happens on next cycle
	bool m_wrtram_flag;								//!< set by wrtram, action happens on next cycle

	UINT8 m_s_reg_bank[ALTO2_TASKS];				//!< active S register bank per task
	UINT8 m_bank_reg[ALTO2_TASKS];					//!< normal and extended RAM banks per task
	bool m_ether_enable;							//!< set to true, if the ethernet should be simulated
	bool m_ewfct;									//!< set by Ether task when it want's a wakeup at switch to task_mrt
	int m_dsp_time;									//!< display_state_machine() time accu
	int m_dsp_state;								//!< display_state_machine() previous state
	int m_unload_time;								//!< unload word time accu
	int m_unload_word;								//!< unload word number

	static const char *task_name(int task);			//!< human readable task names
	static const char *r_name(UINT8 reg);			//!< human readable register names
	static const char *aluf_name(UINT8 aluf);		//!< human readable ALU function names
	static const char *bs_name(UINT8 bs);			//!< human readable bus source names
	static const char *f1_name(UINT8 f1);			//!< human readable F1 function names
	static const char *f2_name(UINT8 f2);			//!< human readable F2 function names

	/**
	 * @brief 2KCTL PROM u3 - 256x4
	 * <PRE>
	 * PROM u3 is 256x4 type 3601-1, looks like SN74387, and it
	 * controls NEXT[6-9]', i.e. the outputs are wire-AND to NEXT
	 *
	 *           SN74387
	 *         +---+-+---+
	 *         |   +-+   |
	 *    A6  -|1      16|-  Vcc
	 *         |         |
	 *    A5  -|2      15|-  A7
	 *         |         |
	 *    A4  -|3      14|-  FE1'
	 *         |         |
	 *    A3  -|4      13|-  FE2'
	 *         |         |
	 *    A0  -|5      12|-  D0
	 *         |         |
	 *    A1  -|6      11|-  D1
	 *         |         |
	 *    A2  -|7      10|-  D2
	 *         |         |
	 *   GND  -|8       9|-  D3
	 *         |         |
	 *         +---------+
	 *
	 *
	 * It is enabled whenever the Emulator task is active and:
	 *	both F2[0] and F2[1] are 1  F2 functions 014, 015, 016, 017
	 *	F2=14 is 0                  not for F2 = 14 (load IR<-)
	 *	IR[0] is 0                  not for arithmetic group
	 *
	 * This means it controls the F2 functions 015:IDISP<- and 016:<-ACSOURCE
	 *
	 * Its address lines are:
	 *	line   pin   connected to         load swap
	 *	-------------------------------------------------------------------
	 *	A0     5     F2[2] (i.e. MIR[18]) IR[07]
	 *	A1     6     IR[01]               IR[06]
	 *	A2     7     IR[02]               IR[05]
	 *	A3     4     IR[03]               IR[04]
	 *	A4     3     IR[04]               IR[03]
	 *	A5     2     IR[05]               IR[02]
	 *	A6     1     IR[06]               IR[01]
	 *	A7     15    IR[07]               F2[2]
	 *
	 * Its data lines are:
	 *	line   pin   connected to         load
	 *	-------------------------------------------------------------------
	 *	D3     9     NEXT[06]'            NEXT[06]
	 *	D2     10    NEXT[07]'            NEXT[07]
	 *	D1     11    NEXT[08]'            NEXT[08]
	 *	D0     12    NEXT[09]'            NEXT[09]
	 *
	 * Its address lines are reversed at load time to make it easier to
	 * access it. Also both, address and data lines, are inverted.
	 * </PRE>
	 */
	UINT8* m_ctl2k_u3;

	/**
	 * @brief 2KCTL PROM u38; 82S23; 32x8 bit
	 * <PRE>
	 *
	 *            82S23
	 *         +---+-+---+
	 *         |   +-+   |
	 *    B0  -|1      16|-  Vcc
	 *         |         |
	 *    B1  -|2      15|-  EN'
	 *         |         |
	 *    B2  -|3      14|-  A4
	 *         |         |
	 *    B3  -|4      13|-  A3
	 *         |         |
	 *    B4  -|5      12|-  A2
	 *         |         |
	 *    B5  -|6      11|-  A1
	 *         |         |
	 *    B6  -|7      10|-  A0
	 *         |         |
	 *   GND  -|8       9|-  B7
	 *         |         |
	 *         +---------+
	 *
	 * Task priority encoder
	 *
	 * 	line   pin    signal
	 *	-------------------------------
	 *	A0     10     CT1 (current task LSB)
	 *	A1     11     CT2
	 *	A2     12     CT4
	 *	A3     13     CT8 (current task MSB)
	 *	A4     14     0 (GND)
	 *
	 *	line   pin    signal
	 *	-------------------------------
	 *	B0     1      RDCT8'
	 *	B1     2      RDCT4'
	 *	B2     3      RDCT2'
	 *	B3     4      RDCT1'
	 *	B4     5      NEXT[09]'
	 *	B5     6      NEXT[08]'
	 *	B6     7      NEXT[07]'
	 *	B7     9      NEXT[06]'
	 * </PRE>
	 */
	UINT8* m_ctl2k_u38;

	//! output lines of the 2KCTL U38 PROM
	enum {
		U38_RDCT8,
		U38_RDCT4,
		U38_RDCT2,
		U38_RDCT1,
		U38_NEXT09,
		U38_NEXT08,
		U38_NEXT07,
		U38_NEXT06
	};

	/**
	 * @brief 2KCTL PROM u76; P3601-1; 256x4; PC0I and PC1I decoding
	 * <PRE>
	 * Replacement for u51, which is used in 1KCTL
	 *
	 *           SN74387
	 *         +---+-+---+
	 *         |   +-+   |
	 *    A6  -|1      16|-  Vcc
	 *         |         |
	 *    A5  -|2      15|-  A7
	 *         |         |
	 *    A4  -|3      14|-  FE1'
	 *         |         |
	 *    A3  -|4      13|-  FE2'
	 *         |         |
	 *    A0  -|5      12|-  D0
	 *         |         |
	 *    A1  -|6      11|-  D1
	 *         |         |
	 *    A2  -|7      10|-  D2
	 *         |         |
	 *   GND  -|8       9|-  D3
	 *         |         |
	 *         +---------+
	 *
	 *	input line    signal
	 *	----------------------------
	 *	A7    15      EMACT'
	 *	A6    1       F1(0)
	 *	A5    2       F1(1)'
	 *	A4    3       F1(2)'
	 *	A3    4       F1(3)'
	 *	A2    7       0 (GND)
	 *	A1    6       PC1O
	 *	A0    5       PC0O
	 *
	 *	output line   signal
	 *	----------------------------
	 *	D0     12     PC1T
	 *	D1     11     PC1F
	 *	D2     10     PC0T
	 *	D3     9      PC0F
	 *
	 * The outputs are connected to a dual 4:1 demultiplexer 74S153, so that
	 * depending on NEXT01' and RESET the following signals are passed through:
	 *
	 *	RESET  NEXT[01]'  PC0I    PC1I
	 *	--------------------------------------
	 *	0      0          PC0T    PC1T
	 *	0      1          PC0F    PC1F
	 *	1      0          PC0I4   T14 (?)
	 *	1      1          -"-     -"-
	 *
	 * This selects the microcode "page" to jump to on SWMODE (F1 = 010)
	 * depending on the current NEXT[01]' level.
	 * </PRE>
	 */
	UINT8* m_ctl2k_u76;

	/**
	 * @brief 3k CRAM PROM a37
	 */
	UINT8* m_cram3k_a37;

	/**
	 * @brief memory addressing PROM a64
	 */
	UINT8* m_madr_a64;

	/**
	 * @brief memory addressing PROM a65
	 */
	UINT8* m_madr_a65;

	//! per task bus source function pointers, early (0) and late (1)
	a2func m_bs[2][ALTO2_TASKS][ALTO2_BUSSRC];
	void set_bs(UINT8 task, UINT8 fn, a2func f0, a2func f1) {
		m_bs[0][task][fn] = f0;
		m_bs[1][task][fn] = f1;
	}

	//! per task f1 function pointers, early (0) and late (1)
	a2func m_f1[2][ALTO2_TASKS][ALTO2_F1MAX];
	void set_f1(UINT8 task, UINT8 fn, a2func f0, a2func f1) {
		m_f1[0][task][fn] = f0;
		m_f1[1][task][fn] = f1;
	}

	//! per task f2 function pointers, early (0) and late (1)
	a2func m_f2[2][ALTO2_TASKS][ALTO2_F2MAX];
	void set_f2(UINT8 task, UINT8 fn, a2func f0, a2func f1) {
		m_f2[0][task][fn] = f0;
		m_f2[1][task][fn] = f1;
	}

	bool m_ram_related[ALTO2_TASKS];				//!< set when task is RAM related

	UINT64 m_cycle;									//!< number of cycles executed in the current slice

	UINT64 cycle() { return m_cycle; }				//!< return the current CPU cycle
	UINT64 ntime() { return m_cycle*ALTO2_UCYCLE; }	//!< return the current nano seconds

	void hard_reset();								//!< reset the various registers
	int soft_reset();								//!< soft reset

	void fn_bs_bad_0();								//! bs dummy early function
	void fn_bs_bad_1();								//! bs dummy late function

	void fn_f1_bad_0();								//! f1 dummy early function
	void fn_f1_bad_1();								//! f1 dummy late function

	void fn_f2_bad_0();								//! f2 dummy early function
	void fn_f2_bad_1();								//! f2 dummy late function

	UINT16 bank_reg_r(UINT32 address);				//!< read bank register in memory mapped I/O range
	void bank_reg_w(UINT32 address, UINT16 data);	//!< write bank register in memory mapped I/O range

	void bs_read_r_0();								//!< bs_read_r early: drive bus by R register
	void bs_load_r_0();								//!< bs_load_r early: load R places 0 on the BUS
	void bs_load_r_1();								//!< bs_load_r late: load R from SHIFTER
	void bs_read_md_0();							//!< bs_read_md early: drive BUS from read memory data
	void bs_mouse_0();								//!< bs_mouse early: drive bus by mouse
	void bs_disp_0();								//!< bs_disp early: drive bus by displacement (which?)
	void f1_load_mar_1();							//!< f1_load_mar late: load memory address register
	void f1_task_0();								//!< f1_task early: task switch
	void f2_bus_eq_zero_1();						//!< f2_bus_eq_zero late: branch on bus equals zero
	void f2_shifter_lt_zero_1();					//!< f2_shifter_lt_zero late: branch on shifter less than zero
	void f2_shifter_eq_zero_1();					//!< f2_shifter_eq_zero late: branch on shifter equals zero
	void f2_bus_1();								//!< f2_bus late: branch on bus bits BUS[6-15]
	void f2_alucy_1();								//!< f2_alucy late: branch on latched ALU carry
	void f2_load_md_1();							//!< f2_load_md late: load memory data

	UINT8* m_alu_a10;								//!< ALU function to 74181 operation lookup PROM
	UINT32 alu_74181(UINT32 smc);

	void rdram();									//!< read the microcode ROM/RAM halfword
	void wrtram();									//!< write the microcode RAM from M register and ALU

	// ************************************************
	// ram related stuff
	// ************************************************
	void bs_read_sreg_0();							//!< bs_read_sreg early: drive bus by S register or M (MYL), if rsel is = 0
	void bs_load_sreg_0();							//!< bs_load_sreg early: load S register puts garbage on the bus
	void bs_load_sreg_1();							//!< bs_load_sreg late: load S register from M
	void branch_ROM(const char *from, int page);	//!< branch to ROM page
	void branch_RAM(const char *from, int page);	//!< branch to RAM page
	void f1_swmode_1();								//!< f1_swmode early: switch to micro program counter BUS[6-15] in other bank
	void f1_wrtram_1();								//!< f1_wrtram late: start WRTRAM cycle
	void f1_rdram_1();								//!< f1_rdram late: start RDRAM cycle
#if	(ALTO2_UCODE_RAM_PAGES == 3)
	void f1_load_rmr_1();							//!< f1_load_rmr late: load the reset mode register
#else	// ALTO2_UCODE_RAM_PAGES != 3
	void f1_load_srb_1();							//!< f1_load_srb late: load the S register bank from BUS[12-14]
#endif
	void init_ram(int task);						//!<

	// ************************************************
	// memory mapped i/o stuff
	// ************************************************
	/**
	 * @brief get printer paper ready bit
	 * Paper ready bit. 0 when the printer is ready for a paper scrolling operation.
	 */
	static inline UINT16 GET_PPRDY(UINT16 utilin) { return A2_GET16(utilin,16,0,0); }

	/** @brief put printer paper ready bit */
	static inline void PUT_PPRDY(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,0,0,val); }

	/**
	 * @brief get printer check bit
	 * Printer check bit bit. Should the printer find itself in an abnormal state,
	 * it sets this bit to 0
	 */
	static inline UINT16 GET_PCHECK(UINT16 utilin) { return A2_GET16(utilin,16,1,1); }

	/** @brief put printer check bit */
	static inline void PUT_PCHECK(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,1,1,val); }

	/** @brief get unused bit 2 */
	static inline UINT16 GET_UNUSED_2(UINT16 utilin) { return A2_GET16(utilin,16,2,2); }

	/** @brief put unused bit 2 */
	static inline void PUT_UNUSED_2(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,2,2,val); }

	/**
	 * @brief get printer daisy ready bit
	 * Daisy ready bit. 0 when the printer is ready to print a character.
	 */
	static inline UINT16 GET_PCHRDY(UINT16 utilin) { return A2_GET16(utilin,16,3,3); }

	/** @brief put printer daisy ready bit */
	static inline void PUT_PCHRDY(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,3,3,val); }

	/**
	 * @brief get printer carriage ready bit
	 * Carriage ready bit. 0 when the printer is ready for horizontal positioning.
	 */
	static inline UINT16 GET_PCARRDY(UINT16 utilin) { return A2_GET16(utilin,16,4,4); }

	/** @brief put printer carriage ready bit */
	static inline void PUT_PCARRDY(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,4,4,val); }

	/**
	 * @brief get printer ready bit
	 * Ready bit. Both this bit and the appropriate other ready bit (carriage,
	 * daisy, etc.) must be 0 before attempting any output operation.
	 */
	static inline UINT16 GET_PREADY(UINT16 utilin) { return A2_GET16(utilin,16,5,5); }

	/** @brief put printer ready bit */
	static inline void PUT_PREADY(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,5,5,val); }

	/**
	 * @brief memory configuration switch
	 */
	static inline UINT16 GET_MEMCONFIG(UINT16 utilin) { return A2_GET16(utilin,16,6,6); }

	/** @brief put memory configuration switch */
	static inline void PUT_MEMCONFIG(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,6,6,val); }

	/** @brief get unused bit 7 */
	static inline UINT16 GET_UNUSED_7(UINT16 utilin) { return A2_GET16(utilin,16,7,7); }
	/** @brief put unused bit 7 */
	static inline void PUT_UNUSED_7(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,7,7,val); }

	/** @brief get key set key 0 */
	static inline UINT16 GET_KEYSET_KEY0(UINT16 utilin) { return A2_GET16(utilin,16,8,8); }
	/** @brief put key set key 0 */
	static inline void PUT_KEYSET_KEY0(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,8,8,val); }

	/** @brief get key set key 1 */
	static inline UINT16 GET_KEYSET_KEY1(UINT16 utilin) { return A2_GET16(utilin,16,9,9); }
	/** @brief put key set key 1 */
	static inline void PUT_KEYSET_KEY1(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,9,9,val); }

	/** @brief get key set key 2 */
	static inline UINT16 GET_KEYSET_KEY2(UINT16 utilin) { return A2_GET16(utilin,16,10,10); }
	/** @brief put key set key 2 */
	static inline void PUT_KEYSET_KEY2(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,10,10,val); }

	/** @brief get key set key 3 */
	static inline UINT16 GET_KEYSET_KEY3(UINT16 utilin) { return A2_GET16(utilin,16,11,11); }
	/** @brief put key set key 3 */
	static inline void PUT_KEYSET_KEY3(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,11,11,val); }

	/** @brief get key set key 4 */
	static inline UINT16 GET_KEYSET_KEY4(UINT16 utilin) { return A2_GET16(utilin,16,12,12); }
	/** @brief put key set key 4 */
	static inline void PUT_KEYSET_KEY4(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,12,12,val); }

	/** @brief get mouse red button bit */
	static inline UINT16 GET_MOUSE_RED(UINT16 utilin) { return A2_GET16(utilin,16,13,13); }
	/** @brief put mouse red button bit */
	static inline void PUT_MOUSE_RED(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,13,13,val); }

	/** @brief get mouse blue button bit */
	static inline UINT16 GET_MOUSE_BLUE(UINT16 utilin) { return A2_GET16(utilin,16,14,14); }
	/** @brief put mouse blue button bit */
	static inline void PUT_MOUSE_BLUE(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,14,14,val); }

	/** @brief get mouse yellow button bit */
	static inline UINT16 GET_MOUSE_YELLOW(UINT16 utilin) { return A2_GET16(utilin,16,15,15); }
	/** @brief put mouse yellow button bit */
	static inline void PUT_MOUSE_YELLOW(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,15,15,val); }

	/** @brief put mouse bits */
	static inline void PUT_MOUSE(UINT16& utilin, UINT16 val) { A2_PUT16(utilin,16,13,15,val); }

	/**
	 * @brief printer paper strobe bit
	 * Paper strobe bit. Toggling this bit causes a paper scrolling operation.
	 */
	static inline UINT16 GET_PPPSTR(UINT16 utilout) { return A2_GET16(utilout,16,0,0); }

	/**
	 * @brief printer retstore bit
	 * Restore bit. Toggling this bit resets the printer (including clearing
	 * the "check" condition if present) and moves the carriage to the
	 * left margin.
	 */
	static inline UINT16 GET_PREST(UINT16 utilout) { return A2_GET16(utilout,16,1,1); }

	/**
	 * @brief printer ribbon bit
	 * Ribbon bit. When this bit is 1 the ribbon is up (in printing
	 * position); when 0, it is down.
	 */
	static inline UINT16 GET_PRIB(UINT16 utilout) { return A2_GET16(utilout,16,2,2); }

	/**
	 * @brief printer daisy strobe bit
	 * Daisy strobe bit. Toggling this bit causes a character to be printed.
	 */
	static inline UINT16 GET_PCHSTR(UINT16 utilout) { return A2_GET16(utilout,16,3,3); }

	/**
	 * @brief printer carriage strobe bit
	 * Carriage strobe bit. Toggling this bit causes a horizontal position operation.
	 */
	static inline UINT16 GET_PCARSTR(UINT16 utilout) { return A2_GET16(utilout,16,4,4); }

	/**
	 * @brief printer data
	 * Argument to various output operations:
	 * 1. Printing characters. When the daisy bit is toggled bits 9-15 of this field
	 * are interpreted as an ASCII character code to be printed (it should be noted
	 * that all codes less than 040 print as lower case "w").
	 * 2. For paper and carriage operations the field is interpreted as a displacement
	 * (-1024 to +1023), in units of 1/48 inch for paper and 1/60 inch for carriage.
	 * Positive is down or to the right, negative up or to the left. The value is
	 * represented as sign-magnitude (i.e., bit 5 is 1 for negative numbers, 0 for
	 * positive; bits 6-15 are the absolute value of the number).
	 */
	static inline UINT16 GET_PDATA(UINT16 utilout) { return A2_GET16(utilout,16,5,15); }

	/** @brief miscellaneous hardware registers in the memory mapped I/O range */
	struct {
		UINT16 eia;				//!< the EIA port at 0177001
		UINT16 utilout;			//!< the UTILOUT port at 0177016 (active-low outputs) */
		UINT16 xbus[4];			//!< the XBUS port at 0177020 to 0177023
		UINT16 utilin;			//!< the UTILIN port at 0177030 to 0177033 (same value on all addresses)
	}	m_hw;

	UINT16 utilin_r(UINT32 addr);				//!< read an UTILIN address
	UINT16 utilout_r(UINT32 addr);				//!< read the UTILOUT address
	void utilout_w(UINT32 addr, UINT16 data);	//!< write the UTILOUT address
	UINT16 xbus_r(UINT32 addr);					//!< read an XBUS address
	void xbus_w(UINT32 addr, UINT16 data);		//!< write an XBUS address (?)
	void init_hardware();						//!< initialize miscellaneous hardware

	// ************************************************
	// mouse stuff
	// ************************************************
	/**
	 * @brief PROM madr.a32 contains a lookup table to translate mouse motions
	 *
	 * <PRE>
	 * The 4 mouse motion signals MX1, MX2, MY1, and MY2 are connected
	 * to a 256x4 PROM's (3601, SN74387) address lines A0, A2, A4, and A6.
	 * The previous (latched) state of the 4 signals is connected to the
	 * address lines A1, A3, A5, and A7.
	 *
	 *                  SN74387
	 *               +---+--+---+
	 *               |   +--+   |
	 *  MY2     A6  -|1       16|-  Vcc
	 *               |          |
	 *  LMY1    A5  -|2       15|-  A7     LMY2
	 *               |          |
	 *  MY1     A4  -|3       14|-  FE1'   0
	 *               |          |
	 *  LMX2    A3  -|4       13|-  FE2'   0
	 *               |          |
	 *  MX1     A0  -|5       12|-  D0     BUS[12]
	 *               |          |
	 *  LMX1    A1  -|6       11|-  D1     BUS[13]
	 *               |          |
	 *  MX2     A2  -|7       10|-  D2     BUS[14]
	 *               |          |
	 *         GND  -|8        9|-  D3     BUS[15]
	 *               |          |
	 *               +----------+
	 *
	 * A motion to the west will first toggle MX2, then MX1.
	 * sequence: 04 -> 0d -> 0b -> 02
	 * A motion to the east will first toggle MX1, then MX2.
	 * sequence: 01 -> 07 -> 0e -> 08
	 *
	 * A motion to the north will first toggle MY2, then MY1.
	 * sequence: 40 -> d0 -> b0 -> 20
	 * A motion to the south will first toggle MY1, then MY2.
	 * sequence: 10 -> 70 -> e0 -> 80
	 *
	 * This dump is from PROM madr.a32:
	 * 0000: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0020: 003,015,005,003,005,003,003,015,015,003,003,005,003,005,015,003,
	 * 0040: 011,001,016,011,016,011,011,001,001,011,011,016,011,016,001,011,
	 * 0060: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0100: 011,001,016,011,016,011,011,001,001,011,011,016,011,016,001,011,
	 * 0120: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0140: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0160: 003,015,005,003,005,003,003,015,015,003,003,005,003,005,015,003,
	 * 0200: 003,015,005,003,005,003,003,015,015,003,003,005,003,005,015,003,
	 * 0220: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0240: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0260: 011,001,016,011,016,011,011,001,001,011,011,016,011,016,001,011,
	 * 0300: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017,
	 * 0320: 011,001,016,011,016,011,011,001,001,011,011,016,011,016,001,011,
	 * 0340: 003,015,005,003,005,003,003,015,015,003,003,005,003,005,015,003,
	 * 0360: 017,007,013,017,013,017,017,007,007,017,017,013,017,013,007,017
	 * </PRE>
	 */
	UINT8* m_madr_a32;
	struct {
		int x;
		int y;
		int dx;
		int dy;
		UINT8 latch;
	}	m_mouse;

	UINT16 mouse_read();							//!< return the mouse motion flags
	void mouse_motion(int x, int y);				//!< register a mouse motion
	void mouse_button(int b);						//!< register a mouse button change
	void mouse_init();								//!< initialize the mouse context to useful values

	// ************************************************
	// diablo31 drive stuff
	// ************************************************
	#define	DIABLO31 1
	static const int DIABLO_DRIVE_MAX = 2;					//!< max number of drive units
	static const int DIABLO_DRIVE_CYLINDERS = 203;			//!< number of cylinders per drive
	static const int DIABLO_DRIVE_CYLINDER_MASK = 0777;		//!< bit maks for cylinder number (9 bits)
	static const int DIABLO_DRIVE_SPT = 12;					//!< number of sectors per track
	static const int DIABLO_DRIVE_SECTOR_MASK = 017;		//!< bit maks for cylinder number (4 bits)
	static const int DIABLO_DRIVE_HEADS = 2;				//!< number of heads per drive
	static const int DIABLO_DRIVE_PAGES = 203*2*12;			//!< number of pages per drive
	static const int DIABLO_DRIVE_HEAD_MASK = 1;			//!< bit maks for cylinder number (4 bits)
	//! convert a cylinder/head/sector to a logical block address (page)
	static inline int DRIVE_PAGE(int c,int h,int s)	{ return (c * DIABLO_DRIVE_HEADS + h) * DIABLO_DRIVE_SPT + s; }
	/**
	 * @brief description of the sector layout
	 * <PRE>
	 *
	 *                                   xx.x msec sector mark pulses
	 * -+   +-------------------------------------------------------------------------------+   +--
	 *  |   |                                                                               |   |
	 *  +---+                                                                               +---+
	 *
	 *    |                                                                                   |
	 *
	 *    +------+----+------+-----+------+----+-------+-----+------+----+-------+-----+------+
	 *    | PRE- |SYNC|HEADER|CKSUM| PRE- |SYNC| LABEL |CKSUM| PRE- |SYNC| DATA  |CKSUM| POST |
	 *    |AMBLE1|  1 |      |  1  |AMBLE2|  2 |       |  2  |AMBLE3|  3 |       |  3  |AMBLE |
	 *    +------+----+------+-----+------+----+-------+-----+------+----+-------+-----+------+
	 *
	 *    |                                                                                   |
	 *
	 *    +-----------------------------------------------------------------------------------+
	 *    |                                                                                   |
	 * ---+                                                                                   +----
	 *      FORMAT WRITE GATE FOR INITIALIZING
	 *    |                                                                                   |
	 *
	 *    |                                                    +------------------------------+
	 *                                                         |                              |
	 * ---|----------------------------------------------------+                              +----
	 *      WRITE GATE FOR DATA XFER (*)
	 *    |                                                                                   |
	 *
	 *    |                          +-----------------------+-+------------------------------+
	 *                               |                       | | may be continuous (?)        |
	 * ------------------------------+                       +-+                              +----
	 * ???  WRITE GATE FOR LABEL AND DATA XFER (*)
	 *    |                                                                                   |
	 *
	 *    |   +--------------------+   +---------------------+   +----------------------------+
	 *        |                    |   |                     |   |                            |
	 * -------+                    +---+                     +---+                            +----
	 *      READ GATE FOR INITIALIZING OR DATA XFER (**)
	 *
	 *
	 *  (*) Enable should be delayed 1 byte/word time from last bit of checks sum.
	 *  (**) Read Gate should be enabled half way through the preamble area. This
	 *       ensures reading a zero field for data separator synchronization.
	 *
	 * </PRE>
	 */
	static const int DIABLO_PAGENO_WORDS = 1;		//!< number of words in a page number (this doesn't really belong here)
	static const int DIABLO_HEADER_WORDS = 2;		//!< number of words in a header (this doesn't really belong here)
	static const int DIABLO_LABEL_WORDS = 8;		//!< number of words in a label (this doesn't really belong here)
	static const int DIABLO_DATA_WORDS = 256;		//!< number of data words (this doesn't really belong here)
	static const int DIABLO_CKSUM_WORDS = 1;		//!< number of words for a checksum (this doesn't really belong here)

	#if	DIABLO31
	/** @brief DIABLO 31 rotation time is approx. 40ms */
	#define	DIABLO_ROTATION_TIME attotime::from_msec(39.9999)

	/** @brief DIABLO 31 sector time */
	#define	DIABLO_SECTOR_TIME attotime::from_msec(39.9999/DIABLO_DRIVE_SPT)

	/** @brief DIABLO 31 bit clock is 3330kHz ~= 300ns per bit
	 * ~= 133333 bits/track (?)
	 * ~= 11111 bits/sector
	 * ~= 347 words/sector
	 */
	#define	DIABLO_BIT_TIME(bits) attotime::from_nsec(300*(bits))

	/** @brief DIABLO 31 possible sector words */
	#define	DIABLO_SECTOR_WORDS 347

	/** @brief pulse width of sector mark before the next sector begins */
	#define	DIABLO_SECTOR_MARK_PULSE_PRE DIABLO_BIT_TIME(16)

	/** @brief pulse width of sector mark after the next sector began */
	#define	DIABLO_SECTOR_MARK_PULSE_POST DIABLO_BIT_TIME(16)

	#else	/* DIABLO31 */

	/** @brief DIABLO 44 rotation time is approx. 25ms */
	#define	DIABLO_ROTATION_TIME attotime::from_msec(25)

	/** @brief DIABLO 44 sector time */
	#define	DIABLO_SECTOR_TIME	attotime::from_msec(25/DIABLO_DRIVE_SPT)

	/** @brief DIABLO 44 bit clock is 5000kHz ~= 200ns per bit
	 * ~= 125184 bits/track (?)
	 * ~= 10432 bits/sector
	 * ~= 325 words/sector
	 */
	#define	DIABLO_BIT_TIME attotime::from_nsec(200)

	/** @brief DIABLO 44 possible sector words */
	#define	DIABLO_SECTOR_WORDS	325

	/** @brief pulse width of sector mark before the next sector begins */
	#define	DIABLO_SECTOR_MARK_PULSE_PRE DIABLO_BIT_TIME(16)

	/** @brief pulse width of sector mark after the next sector began */
	#define	DIABLO_SECTOR_MARK_PULSE_POST DIABLO_BIT_TIME(16)
	#endif

	/**
	 * @brief format of the cooked disk image sectors, i.e. pure data
	 *
	 * The available images are a multiple of 267 words per sector,
	 * 1 word page number
	 * 2 words header
	 * 8 words label
	 * 256 words data
	 */
	typedef struct {
		UINT8 pageno[2*DIABLO_PAGENO_WORDS];	//!< sector page number
		UINT8 header[2*DIABLO_HEADER_WORDS];	//!< sector header words
		UINT8 label[2*DIABLO_LABEL_WORDS];		//!< sector label words
		UINT8 data[2*DIABLO_DATA_WORDS];		//!< sector data words
	}	diablo_sector_t;

	/**
	 * @brief Structure of the disk drive context (2 drives or packs per system)
	 */
	typedef struct {
		diablo_sector_t *image;		//!< disk image, made up of 203 x 2 x 12 sectors
		int unit;					//!< drive unit number (0 or 1)
		char description[32];		//!< description of the drive(s)
		char basename[80];			//!< basename of the drive image
		int packs;					//!< number of packs in drive (1 or 2)
		attotime rotation_time;		//!< rotation time
		attotime bit_time;			//!< bit time in atto seconds
		int s_r_w_0;				//!< drive seek/read/write signal (active 0)
		int ready_0;				//!< drive ready signal (active 0)
		int sector_mark_0;			//!< sector mark (0 if new sector)
		int addx_acknowledge_0;		//!< address acknowledge, i.e. seek successful (active 0)
		int log_addx_interlock_0;	//!< log address interlock, i.e. seek in progress (active 0)
		int seek_incomplete_0;		//!< seek incomplete, i.e. seek in progress (active 0)
		int egate_0;				//!< erase gate
		int wrgate_0;				//!< write gate
		int rdgate_0;				//!< read gate
		int cylinder;				//!< current cylinder number
		int head;					//!< current head (track) number on cylinder
		int sector;					//!< current sector number in track
		UINT32 *bits[DIABLO_DRIVE_CYLINDERS * DIABLO_DRIVE_HEADS * DIABLO_DRIVE_SPT];		//!< sectors expanded to bits
		int page;					//!< current page = (cylinder * HEADS + head) * SPT + sector
		int rdfirst;				//!< set to first bit of a sector that is read from
		int rdlast;					//!< set to last bit of a sector that was read from
		int wrfirst;				//!< set to non-zero if a sector is written to
		int wrlast;					//!< set to last bit of a sector that was written to
	}	diablo_drive_t;

	diablo_drive_t* m_drive[2];							//!< per drive data
	int m_unit_selected;								//!< selected drive unit
	int m_head_selected;								//!< selected drive head
	a2cb m_sector_callback;								//!< callback to call at the start of each sector
	emu_timer* m_sector_timer;							//!< sector timer
#if	ALTO2_DEBUG
	void drive_dump_ascii(UINT8 *src, size_t size);
	size_t dump_record(UINT8 *src, size_t addr, size_t size, const char *name, int cr);
#endif
	void drive_get_sector(int unit);					//!< calculate the sector from the logical block address
	void expand_sector(int unit, int page);				//!< Expand a sector into an array of clock and data bits
	size_t squeeze_sync(UINT32 *bits, size_t src, size_t size);
	size_t squeeze_unsync(UINT32 *bits, size_t src, size_t size);
	size_t squeeze_record(UINT32 *bits, size_t src, UINT8 *field, size_t size);
	size_t squeeze_cksum(UINT32 *bits, size_t src, int *cksum);
	void squeeze_sector(int unit);						//!< Squeeze a array of clock and data bits into a sector's data
	int drive_bits_per_sector() const;					//!< return number of bitclk edges for a sector
	const char* drive_description(int unit);			//!< return a pointer to a drive's description
	const char* drive_basename(int unit);				//!< return a pointer to a drive's image basename
	int drive_unit(int unit);							//!< return the number of a drive unit
	attotime drive_rotation_time(int unit);				//!< return the atto time for a full rotation
	attotime drive_bit_time(int unit);					//!< return the atto time for a data bit
	int drive_seek_read_write_0(int unit);				//!< return the seek/read/write status of a drive
	int drive_ready_0(int unit);						//!< return the ready status of a drive
	int drive_sector_mark_0(int unit);					//!< return the current sector mark status of a drive
	int drive_addx_acknowledge_0(int unit);				//!< return the address acknowledge state
	int drive_log_addx_interlock_0(int unit);			//!< return the log address interlock state
	int drive_seek_incomplete_0(int unit);				//!< return the seek incomplete state
	int drive_cylinder(int unit);						//!< return the current cylinder of a drive unit
	int drive_head(int unit);							//!< return the current head of a drive unit
	int drive_sector(int unit);							//!< return the current sector of a drive unit
	int drive_page(int unit);							//!< return the current page of a drive unit
	void drive_select(int unit, int head);				//!< select a drive unit and head
	void drive_sector_callback(a2cb callback);			//!< set a new drive sector callback
	void drive_strobe(int unit, int cylinder, int restore, int strobe);	//!< strobe a seek operation
	void drive_egate(int unit, int gate);				//!< set the drive erase gate
	void drive_wrgate(int unit, int gate);				//!< set the drive write gate
	void drive_rdgate(int unit, int gate);				//!< set the drive read gate
	void drive_wrdata(int unit, int index, int wrdata);	//!< write the sector relative bit at index
	int drive_rddata(int unit, int index);				//!< read the sector relative bit at index
	int drive_rdclk(int unit, int index);				//!< get the sector relative clock at index
	int debug_read_sync(int unit, int page, int offs);	//!< debug function to read sync bit position from a sector
	int debug_read_sec(int unit, int page, int offs);	//!< debug function to read bits from a drive sector
	TIMER_CALLBACK_MEMBER( drive_next_sector );			//!< timer callback that is called once per sector in the rotation
	void sector_mark_1(int unit);						//!< deasserts the sector mark
	void sector_mark_0(int unit);						//!< asserts the sector mark
	void drive_init();									//!< initialize the disk drive context and insert a disk word timer

	// ************************************************
	// disk controller stuff
	// ************************************************

	//! disk controller context
	struct {
		UINT8 drive;					//!< selected drive from KADDR[14] (written to data out with SENDADR)
		UINT16 kaddr;					//!< A[0-15] disk hardware address (sector, cylinder, head, drive, restore)
		UINT16 kadr;					//!< C[0-15] with read/write/check modes for header, label and data
		UINT16 kstat;					//!< S[0-15] disk status
		UINT16 kcom;					//!< disk command (5 bits kcom[1-5])
		UINT8 krecno;					//!< record number (2 bits indexing header, label, data, -/-)
		UINT32 shiftin;					//!< input shift register
		UINT32 shiftout;				//!< output shift register
		UINT32 datain;					//!< disk data in latch
		UINT32 dataout;					//!< disk data out latch
		UINT8 krwc;						//!< read/write/check for current record
		UINT8 kfer;						//!< disk fatal error signal state
		UINT8 wdtskena;					//!< disk word task enable (active low)
		UINT8 wdinit0;					//!< disk word task init at the early microcycle
		UINT8 wdinit;					//!< disk word task init at the late microcycle
		UINT8 strobe;					//!< strobe (still) active
		emu_timer* strobon_timer;		//!< set strobe on timer
		UINT8 bitclk;					//!< current bitclk state (either crystal clock, or rdclk from the drive)
		emu_timer* bitclk_timer;		//!< bit clock timer
		UINT8 datin;					//!< current datin from the drive
		UINT8 bitcount;					//!< bit counter
		UINT8 carry;					//!< carry output of the bitcounter
		UINT8 seclate;					//!< sector late (monoflop output)
		emu_timer* seclate_timer;		//!< sector late timer
		UINT8 seekok;					//!< seekok state (SKINC' & LAI' & ff_44a.Q')
		UINT8 ok_to_run;				//!< ok to run signal (set to 1 some time after reset)
		emu_timer* ok_to_run_timer;		//!< ok to run timer
		UINT8 ready_mf31a;				//!< ready monoflop 31a
		emu_timer* ready_timer;			//!< ready timer
		UINT8 seclate_mf31b;			//!< seclate monoflop 31b
		jkff_t ff_21a;					//!< JK flip-flop 21a (sector task)
		jkff_t ff_21a_old;				//!< -"- previous state
		jkff_t ff_21b;					//!< JK flip-flop 21b (sector task)
		jkff_t ff_22a;					//!< JK flip-flop 22a (sector task)
		jkff_t ff_22b;					//!< JK flip-flop 22b (sector task)
		jkff_t ff_43b;					//!< JK flip-flop 43b (word task)
		jkff_t ff_53a;					//!< JK flip-flop 53a (word task)
		jkff_t ff_43a;					//!< JK flip-flop 43a (word task)
		jkff_t ff_53b;					//!< brief JK flip-flop 53b (word task)
		jkff_t ff_44a;					//!< JK flip-flop 44a (LAI' clocked)
		jkff_t ff_44b;					//!< JK flip-flop 44b (CKSUM)
		jkff_t ff_45a;					//!< JK flip-flop 45a (ready latch)
		jkff_t ff_45b;					//!< JK flip-flop 45b (seqerr latch)
	}	m_dsk;

	jkff_t m_sysclka0[4];				//!< simulate previous sysclka
	jkff_t m_sysclka1[4];				//!< simulate current sysclka
	jkff_t m_sysclkb0[4];				//!< simulate previous sysclkb
	jkff_t m_sysclkb1[4];				//!< simulate current sysclkb

	void kwd_timing(int bitclk, int datin, int block);	//!< disk word timing
	TIMER_CALLBACK_MEMBER( disk_seclate );			//!< timer callback to take away the SECLATE pulse (monoflop)
	TIMER_CALLBACK_MEMBER( disk_ok_to_run );		//!< timer callback to take away the OK TO RUN pulse (reset)
	TIMER_CALLBACK_MEMBER( disk_strobon );			//!< timer callback to pulse the STROBE' signal to the drive
	TIMER_CALLBACK_MEMBER( disk_ready_mf31a );		//!< timer callback to change the READY monoflop 31a
	void disk_block(int task);						//!< called if one of the disk tasks (task_kwd or task_ksec) blocks
	void bs_read_kstat_0();							//!< bs_read_kstat early: bus driven by disk status register KSTAT
	void bs_read_kdata_0();							//!< bs_read_kdata early: bus driven by disk data register KDATA input
	void f1_strobe_1();								//!< f1_strobe late: initiates a disk seek
	void f1_load_kstat_1();							//!< f1_load_kstat late: load disk status register
	void f1_load_kdata_1();							//!< f1_load_kdata late: load data out register, or the disk address register
	void f1_increcno_1();							//!< f1_increcno late: advances shift registers holding KADR
	void f1_clrstat_1();							//!< f1_clrstat late: reset all error latches
	void f1_load_kcom_1();							//!< f1_load_kcom late: load the KCOM register from bus
	void f1_load_kadr_1();							//!< f1_load_kadr late: load the KADR register from bus
	void f2_init_1();								//!< f2_init late: branch on disk word task active and init
	void f2_rwc_1();								//!< f2_rwc late: branch on read/write/check state of the current record
	void f2_recno_1();								//!< f2_recno late: branch on the current record number by a lookup table
	void f2_xfrdat_1();								//!< f2_xfrdat late: branch on the data transfer state
	void f2_swrnrdy_1();							//!< f2_swrnrdy late: branch on the disk ready signal
	void f2_nfer_1();								//!< f2_nfer late: branch on the disk fatal error condition
	void f2_strobon_1();							//!< f2_strobon late: branch on the seek busy status
	TIMER_CALLBACK_MEMBER( disk_bitclk );			//!< callback to update the disk controller with a new bitclk
	void disk_sector_start(int unit);				//!< callback is called by the drive timer whenever a new sector starts
	void init_disk();								//!< initialize the disk context and insert a disk wort timer

	// ************************************************
	// display stuff
	// ************************************************
	/**
	 * @brief structure of the display context
	 *
	 * Schematics of the task clear and wakeup signal generators
	 * <PRE>
	 * A quote (') appended to a signal name means inverted signal.
	 *
	 *  AND |     NAND|      NOR |       FF | Q    N174
	 * -----+--- -----+---  -----+---  -----+---   -----
	 *  0 0 | 0   0 0 | 1    0 0 | 1    S'\0| 1    delay
	 *  0 1 | 0   0 1 | 1    0 1 | 0    R'\0| 0
	 *  1 0 | 0   1 0 | 1    1 0 | 0
	 *  1 1 | 1   1 1 | 0    1 1 | 0
	 *
	 *
	 *                                                       DVTAC'
	 *                                                      >-------·  +-----+
	 *                                                              |  |  FF |
	 * VBLANK'+----+ DELVBLANK' +---+  DELVBLANK   +----+           ·--|S'   |
	 * >------|N174|------+-----|inv|--------------|NAND| VBLANKPULSE  |     |              WAKEDVT'
	 *        +----+      |     +---+              |    o--+-----------|R'  Q|---------------------->
	 *                    |                      ·-|    |  |           |     |
	 *        +----+      |     DDELVBLANK'      | +----+  |           +-----+
	 *      ·-|N174|-----------------------------·         |      +---+
	 *      | +----+      |                                +------oAND|
	 *      |             |                      DSP07.01  |      |   o----------·
	 *      ·-------------·                      >---------|------o   |          |
	 *                                                     |      +---+          |
	 *                                                     |                     |
	 *                                                     | +-----+             |
	 *                                                     | |  FF |             |  +-----+
	 *        DHTAC'       +---+                           | |     |             |  |  FF |
	 *      >--------------oNOR|  *07.25       +----+      ·-|S'   |   DHTBLK'   |  |     |
	 *        BLOCK'       |   |---------------|NAND|        |    Q|--+----------|--|S1'  | WAKEDHT'
	 *      >--------------o   |     DCSYSCLK  |    o--------|R'   |  | >--------|--|S2' Q|--------->
	 *                     +---+     >---------|    |        +-----+  |  DHTAC'  ·--|R'   |
	 *                                         +----+                 |             +-----+
	 *                                                   ·------------·
	 *                                                   |
	 *        DWTAC'       +---+                         |   +-----+
	 *      >--------------oNOR|  *07.26 +----+          |   |  FF |
	 *        BLOCK'       |   |---------|NAND| DSP07.01 |   |     |
	 *      >--------------o   | DCSYSCLK|    o----------|---|S1'  | DWTCN' +---+        DWTCN
	 *                     +---+ >-------|    |          ·---|S2' Q|--------|inv|-----------+----
	 *                                   +----+          ·---|R'   |        +---+           |
	 *                                                   |   +-----+                        |
	 *                 SCANEND     +----+                |                                  |
	 *               >-------------|NAND|  CLRBUF'       |           .----------------------·
	 *                 DCLK        |    o----------------·           |
	 *               >-------------|    |                            |  +-----+
	 *                             +----+                            ·--| NAND|
	 *                                                       STOPWAKE'  |     |preWake +----+ WAKEDWT'
	 *                                                      >-----------|     o--------|N174|--------->
	 *                                                        VBLANK'   |     |        +----+
	 *                                                      >-----------|     |
	 *                                                                  +-----+
	 *                                                     a40c
	 *                                        VBLANKPULSE +----+
	 *                                       -------------|NAND|
	 *                                                    |    o--·
	 *                                                 ·--|    |  |
	 *                                                 |  +----+  |
	 *                                                 ·----------|-·
	 *                                                 ·----------· |
	 *        CURTAC'      +---+                       |  +----+    |     a20d
	 *      >--------------oNOR|  *07.27 +----+        ·--|NAND|    |    +----+
	 *        BLOCK'       |   |---------|NAND| DSP07.07  |    o----+----o NOR| preWK  +----+ WAKECURT'
	 *      >--------------o   | DCSYSCLK|    o-----------|    |         |    |--------|N174|--------->
	 *                     +---+ >-------|    |           +----+    +----o    |        +----+
	 *                                   +----+            a40d     |    +----+
	 *                                          a30c                |
	 *                              CURTAC'    +----+               |
	 *                            >------------|NAND|    DSP07.03   |
	 *                                         |    o--+------------·
	 *                                      ·--|    |  |
	 *                                      |  +----+  |
	 *                                      ·----------|-·
	 *                                      ·----------· |
	 *                                      |  +----+    |
	 *                                      ·--|NAND|    |
	 *                              CLRBUF'    |    o----·
	 *                            >------------|    |
	 *                                         +----+
	 *                                          a30d
	 * </PRE>
	 */
	struct {
		UINT16 hlc;							//!< horizontal line counter
		UINT8 a63;							//!< most recent value read from the PROM a63
		UINT8 a66;							//!< most recent value read from the PROM a66
		UINT16 setmode;						//!< value written by last SETMODE<-
		UINT16 inverse;						//!< set to 0xffff if line is inverse, 0x0000 otherwise
		UINT8 halfclock;					//!< set 0 for normal pixel clock, 1 for half pixel clock
		UINT8 clr;							//!< set non-zero if any of VBLANK or HBLANK is active (a39a 74S08)
		UINT16 fifo[ALTO2_DISPLAY_FIFO];	//!< display word fifo
		UINT8 fifo_wr;						//!< fifo input pointer (4-bit)
		UINT8 fifo_rd;						//!< fifo output pointer (4-bit)
		UINT8 dht_blocks;					//!< set non-zero, if the DHT executed BLOCK
		UINT8 dwt_blocks;					//!< set non-zero, if the DWT executed BLOCK
		UINT8 curt_blocks;					//!< set non-zero, if the CURT executed BLOCK
		UINT8 curt_wakeup;					//!< set non-zero, if CURT wakeups are generated
		UINT16 vblank;						//!< most recent HLC with VBLANK still high (11-bit)
		UINT16 xpreg;						//!< cursor cursor x position register (10-bit)
		UINT16 csr;							//!< cursor shift register (16-bit)
		UINT32 curword;						//!< helper: first cursor word in current scanline
		UINT32 curdata;						//!< helper: shifted cursor data (32-bit)
		UINT16 *raw_bitmap;					//!< array of words of the raw bitmap that is displayed
	}	m_dsp;

	//! horizontal line counter bit 0
	inline int HLC1() { return A2_BIT16(m_dsp.hlc,11,10); }
	//! horizontal line counter bit 1
	inline int HLC2() { return A2_BIT16(m_dsp.hlc,11,9); }
	//! horizontal line counter bit 2
	inline int HLC4() { return A2_BIT16(m_dsp.hlc,11,8); }
	//! horizontal line counter bit 3
	inline int HLC8() { return A2_BIT16(m_dsp.hlc,11,7); }
	//! horizontal line counter bit 4
	inline int HLC16() { return A2_BIT16(m_dsp.hlc,11,6); }
	//! horizontal line counter bit 5
	inline int HLC32() { return A2_BIT16(m_dsp.hlc,11,5); }
	//! horizontal line counter bit 6
	inline int HLC64() { return A2_BIT16(m_dsp.hlc,11,4); }
	//! horizontal line counter bit 7
	inline int HLC128() { return A2_BIT16(m_dsp.hlc,11,3); }
	//! horizontal line counter bit 8
	inline int HLC256() { return A2_BIT16(m_dsp.hlc,11,2); }
	//! horizontal line counter bit 9
	inline int HLC512() { return A2_BIT16(m_dsp.hlc,11,1); }
	//! horizontal line counter bit 10
	inline int HLC1024() { return A2_BIT16(m_dsp.hlc,11,0); }

	//! get the pixel clock speed from a SETMODE<- bus value
	static inline UINT16 GET_SETMODE_SPEEDY(UINT16 mode) { return A2_GET16(mode,16,0,0); }
	//! get the inverse video flag from a SETMODE<- bus value
	static inline UINT16 GET_SETMODE_INVERSE(UINT16 mode) { return A2_GET16(mode,16,1,1); }

	/**
	 * @brief PROM a38 contains the STOPWAKE' and MBEMBPTY' signals for the FIFO
	 * <PRE>
	 * The inputs to a38 are the UNLOAD counter RA[0-3] and the DDR<- counter
	 * WA[0-3], and the designer decided to reverse the address lines :-)
	 *
	 *	a38  counter FIFO counter
	 *	--------------------------
	 *	 A0  RA[0]   fifo_rd
	 *	 A1  RA[1]
	 *	 A2  RA[2]
	 *	 A3  RA[3]
	 *	 A4  WA[0]   fifo_wr
	 *	 A5  WA[1]
	 *	 A6  WA[2]
	 *	 A7  WA[3]
	 *
	 * Only two bits of a38 are used:
	 * 	O1 (002) = STOPWAKE'
	 * 	O3 (010) = MBEMPTY'
	 * </PRE>
	 */
	UINT8* m_disp_a38;

	//! PROM a38 bit O1 is STOPWAKE' (stop DWT if bit is zero)
	inline int FIFO_STOPWAKE_0() { return m_disp_a38[m_dsp.fifo_rd * 16 + m_dsp.fifo_wr] & 002; }

	//! PROM a38 bit O3 is MBEMPTY' (FIFO is empty if bit is zero)
	inline int FIFO_MBEMPTY_0() { return m_disp_a38[m_dsp.fifo_rd * 16 + m_dsp.fifo_wr] & 010; }

	/**
	 * @brief emulation of PROM a63 in the display schematics page 8
	 * <PRE>
	 * The PROM's address lines are driven by a clock CLK, which is
	 * pixel clock / 24, and an inverted half-scanline signal H[1]'.
	 *
	 * It is 32x8 bits and its output bits (B) are connected to the
	 * signals, as well as its own address lines (A) through a latch
	 * of the type SN74774 like this:
	 *
	 * B    174     A   others
	 * ------------------------
	 * 0     5      -   HBLANK
	 * 1     0      -   HSYNC
	 * 2     4      0
	 * 3     1      1
	 * 4     3      2
	 * 5     2      3
	 * 6     -      -   SCANEND
	 * 7     -      -   HLCGATE
	 * ------------------------
	 * H[1]' -      4
	 *
	 * The display_state_machine() is called by the CPU at a rate of pixelclock/24,
	 * which happens to be very close to every 7th CPU micrcocycle.
	 * </PRE>
	 */
	UINT8* m_disp_a63;

	enum {
		A63_HBLANK	= (1 << 0),				//!< PROM a63 B0 is latched as HBLANK signal
		A63_HSYNC	= (1 << 1),				//!< PROM a63 B1 is latched as HSYNC signal
		A63_A0		= (1 << 2),				//!< PROM a63 B2 is the latched next address bit A0
		A63_A1		= (1 << 3),				//!< PROM a63 B3 is the latched next address bit A1
		A63_A2		= (1 << 4),				//!< PROM a63 B4 is the latched next address bit A2
		A63_A3		= (1 << 5),				//!< PROM a63 B5 is the latched next address bit A3
		A63_SCANEND	= (1 << 6),				//!< PROM a63 B6 SCANEND signal, which resets the FIFO counters
		A63_HLCGATE	= (1 << 7)				//!< PROM a63 B7 HLCGATE signal, which enables counting the HLC
	};
	//!< helper to extract A3-A0 from a PROM a63 value
	inline UINT8 A63_NEXT(UINT8 n) { return (n >> 2) & 017; }

	//!< test the HBLANK (horizontal blanking) signal in PROM a63 being high
	static inline bool A2_HBLANK_HI(UINT8 a) { return (a & A63_HBLANK) ? true : false; }
	//!< test the HBLANK (horizontal blanking) signal in PROM a63 being low
	static inline bool A2_HBLANK_LO(UINT8 a) { return !A2_HBLANK_HI(a); }
	//!< test the HSYNC (horizontal synchonisation) signal in PROM a63 being high
	static inline bool A2_HSYNC_HI(UINT8 a) { return (a & A63_HSYNC) ? true : false; }
	//!< test the HSYNC (horizontal synchonisation) signal in PROM a63 being low
	static inline bool A2_HSYNC_LO(UINT8 a) { return !A2_HSYNC_HI(a); }
	//!< test the SCANEND (scanline end) signal in PROM a63 being high
	static inline bool A2_SCANEND_HI(UINT8 a) { return (a & A63_SCANEND) ? true : false; }
	//!< test the SCANEND (scanline end) signal in PROM a63 being low
	static inline bool A2_SCANEND_LO(UINT8 a) { return !A2_SCANEND_HI(a); }
	//!< test the HLCGATE (horz. line counter gate) signal in PROM a63 being high
	static inline bool A2_HLCGATE_HI(UINT8 a) { return (a & A63_HLCGATE) ? true : false; }
	//!< test the HLCGATE (horz. line counter gate) signal in PROM a63 being low
	static inline bool A2_HLCGATE_LO(UINT8 a) { return !A2_HLCGATE_HI(a); }

	/**
	 * @brief vertical blank and synch PROM
	 *
	 * PROM a66 is a 256x4 bit (type 3601), containing the vertical blank + synch.
	 * Address lines are driven by H[1] to H[128] of the the horz. line counters.
	 * The PROM is enabled whenever H[256] and H[512] are both 0.
	 *
	 * Q1 (001) is VSYNC for the odd field (with H1024=1)
	 * Q2 (002) is VSYNC for the even field (with H1024=0)
	 * Q3 (004) is VBLANK for the odd field (with H1024=1)
	 * Q4 (010) is VBLANK for the even field (with H1024=0)
	 */
	UINT8* m_disp_a66;

	enum {
		A66_VSYNC_ODD	= (1 << 0),
		A66_VSYNC_EVEN	= (1 << 1),
		A66_VBLANK_ODD	= (1 << 2),
		A66_VBLANK_EVEN	= (1 << 3)
	};

	//! test for the VSYNC signal (depends on "field", i.e. HLC1024)
	inline bool A66_VSYNC(UINT8 a) { return a & (HLC1024() ? A66_VSYNC_ODD : A66_VSYNC_EVEN) ? true : false; }
	//! test for the VBLANK signal (depends on "field", i.e. HLC1024)
	inline bool A66_VBLANK(UINT8 a) { return a & (HLC1024() ? A66_VBLANK_ODD : A66_VBLANK_EVEN) ? true : false; }
	//! test the VSYNC (vertical synchronisation) signal in PROM a66 being high
	inline bool A2_VSYNC_HI(UINT8 a) { return A66_VSYNC(a); }
	//! test the VSYNC (vertical synchronisation) signal in PROM a66 being low
	inline bool A2_VSYNC_LO(UINT8 a) { return !A66_VSYNC(a); }
	//! test the VBLANK (vertical blanking) signal in PROM a66 being high
	inline bool A2_VBLANK_HI(UINT8 a) { return A66_VBLANK(a); }
	//! test the VBLANK (vertical blanking) signal in PROM a66 being low
	inline bool A2_VBLANK_LO(UINT8 a) { return !A66_VBLANK(a); }

	/**
	 * @brief unload the next word from the display FIFO and shift it to the screen
	 */
	int unload_word(int x);

	/**
	 * @brief function called by the CPU to enter the next display state
	 *
	 * There are 32 states per scanline and 875 scanlines per frame.
	 *
	 * @param arg the current m_disp_a63 PROM address
	 * @result returns the next state of the display state machine
	 */
	int display_state_machine(int arg);

	/** @brief branch on the evenfield flip-flop */
	void f2_evenfield_1(void);

	/** @brief initialize the display context */
	int init_disp();

	// ************************************************
	// memory stuff
	// ************************************************
	/** @brief set non-zero to incorporate the Hamming code and Parity check */
	#define	ALTO2_HAMMING_CHECK	1

	enum {
		ALTO2_MEM_NONE,
		ALTO2_MEM_ODD	= (1 << 0),
		ALTO2_MEM_RAM	= (1 << 1),
		ALTO2_MEM_NIRVANA	= (1 << 2)
	};

	struct {
		UINT32 ram[ALTO2_RAM_SIZE/4];		//!< main memory organized as double-words
		UINT8 hpb[ALTO2_RAM_SIZE/4];		//!< Hamming code (6) and Parity bits (1) per double word
		UINT32 mar;							//!< memory address register
		UINT32 rmdd;						//!< read memory data double-word
		UINT32 wmdd;						//!< write memory data double-word
		UINT16 md;							//!< memory data register
		UINT64 cycle;						//!< cycle when the memory address register was loaded

		/**
		 * @brief memory access under the way if non-zero
		 * 0: no memory access (MEM_NONE)
		 * 1: invalid
		 * 2: memory access even word (MEM_RAM)
		 * 3: memory access odd word (MEM_RAM | MEM_ODD)
		 * 4: refresh even word (MEM_REFRESH)
		 * 5: refresh odd word (MEM_REFRESH | MEM_ODD)
		 */
		int access;
		int error;							//!< non-zero after a memory error was detected
		int mear;							//!< memory error address register
		UINT16 mesr;						//!< memory error status register
		UINT16 mecr;						//!< memory error control register
	}	m_mem;

	/**
	 * @brief check if memory address register load is yet possible
	 * suspend if accessing RAM and previous MAR<- was less than 5 cycles ago
	 *
	 * 1.  MAR<- ANY
	 * 2.  REQUIRED
	 * 3.  MD<- whatever
	 * 4.  SUSPEND
	 * 5.  SUSPEND
	 * 6.  MAR<- ANY
	 *
	 * @result returns 0, if memory address can be loaded
	 */
	inline bool check_mem_load_mar_stall(UINT8 rsel) {
		return (ALTO2_MEM_NONE == m_mem.access ? false : cycle() < m_mem.cycle+5);
	}

	/**
	 * @brief check if memory read is yet possible
	 * MAR<- = cycle #1, earliest read at cycle #5, i.e. + 4
	 *
	 * 1.  MAR<- ANY
	 * 2.  REQUIRED
	 * 3.  SUSPEND
	 * 4.  SUSPEND
	 * 5.  whereever <-MD
	 *
	 * @result returns 0, if memory can be read without wait cycle
	 */
	inline bool check_mem_read_stall() {
		return (ALTO2_MEM_NONE == m_mem.access ? false : cycle() < m_mem.cycle+4);
	}

	/**
	 * @brief check if memory write is yet possible
	 * MAR<- = cycle #1, earliest write at cycle #3, i.e. + 2
	 *
	 * 1.  MAR<- ANY
	 * 2.  REQUIRED
	 * 3.  OPTIONAL
	 * 4.  MD<- whatever
	 *
	 * @result returns 0, if memory can be written without wait cycle
	 */
	inline bool check_mem_write_stall() {
		return (ALTO2_MEM_NONE == m_mem.access ? false : cycle() < m_mem.cycle+2);
	}

	//! memory mapped I/O read functions - FIXME: use MAME memory handlers for AS_IO
	a2io_rd mmio_read_fn[ALTO2_IO_PAGE_SIZE];

	//! memory mapped I/O write functions - FIXME: use MAME memory handlers for AS_IO
	a2io_wr mmio_write_fn[ALTO2_IO_PAGE_SIZE];

	//! catch unmapped memory mapped I/O reads
	UINT16 bad_mmio_read_fn(UINT32 address);

	//! catch unmapped memory mapped I/O writes
	void bad_mmio_write_fn(UINT32 address, UINT16 data);

	//! memory error address register read
	UINT16 mear_r(UINT32 address);

	//! memory error status register read
	UINT16 mesr_r(UINT32 address);

	//! memory error status register write (clear)
	void mesr_w(UINT32 address, UINT16 data);

	//! memory error control register read
	UINT16 mecr_r(UINT32 address);

	//! memory error control register write
	void mecr_w(UINT32 address, UINT16 data);

	//! read or write a memory double-word and caluclate its Hamming code
	UINT32 hamming_code(int write, UINT32 dw_addr, UINT32 dw_data);

	//! load the memory address register with some value
	void load_mar(UINT8 rsel, UINT32 addr);

	//! read memory or memory mapped I/O from the address in mar to md
	UINT16 read_mem();

	//! write memory or memory mapped I/O from md to the address in mar
	void write_mem(UINT16 data);

	//! install read and/or write memory mapped I/O handler(s) for a range first to last
	void install_mmio_fn(int first, int last, a2io_rd rfn, a2io_wr wfn);

	//! debugger interface to read memory
	UINT16 debug_read_mem(UINT32 addr);

	//! debugger interface to write memory
	void debug_write_mem(UINT32 addr, UINT16 data);

#if	ALTO2_DEBUG
	void watch_write(UINT32 addr, UINT32 data);
	void watch_read(UINT32 addr, UINT32 data);
#endif

	//! initialize the memory system
	void init_memory();

	// ************************************************
	// emulator task
	// ************************************************
	struct {
		UINT16 ir;									//!< emulator instruction register
		UINT8 skip;									//!< emulator skip
		UINT8 cy;									//!< emulator carry
	}	m_emu;
	void bs_emu_disp_0();							//!< bs_emu_disp early: drive bus by IR[8-15], possibly sign extended
	void f1_emu_block_0();							//!< f1_block early: block task
	void f1_emu_load_rmr_1();						//!< f1_load_rmr late: load the reset mode register
	void f1_emu_load_esrb_1();						//!< f1_load_esrb late: load the extended S register bank from BUS[12-14]
	void f1_rsnf_0();								//!< f1_rsnf early: drive the bus from the Ethernet node ID
	void f1_startf_0();								//!< f1_startf early: defines commands for for I/O hardware, including Ethernet
	void f2_busodd_1();								//!< f2_busodd late: branch on odd bus
	void f2_magic_1();								//!< f2_magic late: shift and use T
	void f2_load_dns_0();							//!< f2_load_dns early: modify RESELECT with DstAC = (3 - IR[3-4])
	void f2_load_dns_1();							//!< f2_load_dns late: do novel shifts
	void f2_acdest_0();								//!< f2_acdest early: modify RSELECT with DstAC = (3 - IR[3-4])
	void bitblt_info();								//!< debug bitblt opcode
	void f2_load_ir_1();							//!< f2_load_ir late: load instruction register IR and branch on IR[0,5-7]
	void f2_idisp_1();								//!< f2_idisp late: branch on: arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
	void f2_acsource_0();							//!< f2_acsource early: modify RSELECT with SrcAC = (3 - IR[1-2])
	void f2_acsource_1();							//!< f2_acsource late: branch on arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
	void init_emu(int task);						//!< 000 initialize emulator task

	// ksec task
	void f1_ksec_block_0(void);
	void init_ksec(int task);						//!< 004 initialize disk sector task

	// ************************************************
	// ethernet task
	// ************************************************
	/**
	 * @brief BPROMs P3601-1; 256x4; enet.a41 "PE1" and enet.a42 "PE2"
	 *
	 * Phase encoder
	 *
	 * a41: P3601-1; 256x4; "PE1"
	 * a42: P3601-1; 256x4; "PE2"
	 *
	 * PE1/PE2 inputs
	 * ----------------
	 * A0  (5) OUTGO
	 * A1  (6) XDATA
	 * A2  (7) OSDATAG
	 * A3  (4) XCLOCK
	 * A4  (3) OCNTR0
	 * A5  (2) OCNTR1
	 * A6  (1) OCNTR2
	 * A7 (15) OCNTR3
	 *
	 * PE1 outputs
	 * ----------------
	 * D0 (12) OCNTR0
	 * D1 (11) OCNTR1
	 * D2 (10) OCNTR2
	 * D3  (9) OCNTR3
	 *
	 * PE2 outputs
	 * ----------------
	 * D0 (12) n.c.
	 * D1 (11) to OSLOAD flip flop J and K'
	 * D2 (10) XDATA
	 * D3  (9) XCLOCK
	 */
	UINT8* m_ether_a41;
	UINT8* m_ether_a42;

	/**
	 * @brief BPROM; P3601-1; 265x4 enet.a49 "AFIFO"
	 *
	 * Perhaps try with the contents of the display FIFO, as it is
	 * the same type and the display FIFO has the same size.
	 *
	 * FIFO control
	 *
	 * a49: P3601-1; 256x4; "AFIFO"
	 *
	 * inputs
	 * ----------------
	 * A0  (5) fifo_wr[0]
	 * A1  (6) fifo_wr[1]
	 * A2  (7) fifo_wr[2]
	 * A3  (4) fifo_wr[3]
	 * A4  (3) fifo_rd[0]
	 * A5  (2) fifo_rd[1]
	 * A6  (1) fifo_rd[2]
	 * A7 (15) fifo_rd[3]
	 *
	 * outputs active low
	 * ----------------------------
	 * D0 (12) BE'    (buffer empty)
	 * D1 (11) BNE'   (buffer next empty ?)
	 * D2 (10) BNNE'  (buffer next next empty ?)
	 * D3  (9) BF'    (buffer full)
	 *
	 * Data from enet.a49 after address line reversal:
	 * 000: 010 007 017 017 017 017 017 017 017 017 017 017 017 017 013 011
	 * 020: 011 010 007 017 017 017 017 017 017 017 017 017 017 017 017 013
	 * 040: 013 011 010 007 017 017 017 017 017 017 017 017 017 017 017 017
	 * 060: 017 013 011 010 007 017 017 017 017 017 017 017 017 017 017 017
	 * 100: 017 017 013 011 010 007 017 017 017 017 017 017 017 017 017 017
	 * 120: 017 017 017 013 011 010 007 017 017 017 017 017 017 017 017 017
	 * 140: 017 017 017 017 013 011 010 007 017 017 017 017 017 017 017 017
	 * 160: 017 017 017 017 017 013 011 010 007 017 017 017 017 017 017 017
	 * 200: 017 017 017 017 017 017 013 011 010 007 017 017 017 017 017 017
	 * 220: 017 017 017 017 017 017 017 013 011 010 007 017 017 017 017 017
	 * 240: 017 017 017 017 017 017 017 017 013 011 010 007 017 017 017 017
	 * 260: 017 017 017 017 017 017 017 017 017 013 011 010 007 017 017 017
	 * 300: 017 017 017 017 017 017 017 017 017 017 013 011 010 007 017 017
	 * 320: 017 017 017 017 017 017 017 017 017 017 017 013 011 010 007 017
	 * 340: 017 017 017 017 017 017 017 017 017 017 017 017 013 011 010 007
	 * 360: 007 017 017 017 017 017 017 017 017 017 017 017 017 013 011 010
	 */
	UINT8* m_ether_a49;

	static const int m_duckbreath_sec = 15;			//!< send duckbreath every 15 seconds

	struct {
		UINT16 fifo[ALTO2_ETHER_FIFO_SIZE];			//!< FIFO buffer
		UINT16 fifo_rd;								//!< FIFO input pointer
		UINT16 fifo_wr;								//!< FIFO output pointer
		UINT16 status;								//!< status word
		UINT32 rx_crc;								//!< receiver CRC
		UINT32 tx_crc;								//!< transmitter CRC
		UINT32 rx_count;							//!< received words count
		UINT32 tx_count;							//!< transmitted words count
		emu_timer* tx_timer;						//!< transmitter timer
		int duckbreath;								//!< if non-zero, interval in seconds at which to broadcast the duckbreath
	}	m_eth;

	TIMER_CALLBACK_MEMBER( rx_duckbreath );			//!< HACK: pull the next word from the duckbreath in the fifo
	TIMER_CALLBACK_MEMBER( tx_packet );				//!< transmit data from the FIFO to <nirvana for now>
	void eth_wakeup();								//!< check for the various reasons to wakeup the Ethernet task
	void eth_startf();								//!< start input or output depending on m_bus
	void bs_eidfct_0();								//!< bs_eidfct early: Ethernet input data function
	void f1_eth_block_0();							//!< f1_eth_block early: block the Ether task
	void f1_eilfct_0();								//!< f1_eilfct early: Ethernet input look function
	void f1_epfct_0();								//!< f1_epfct early: Ethernet post function
	void f1_ewfct_1();								//!< f1_ewfct late: Ethernet countdown wakeup function
	void f2_eodfct_1();								//!< f2_eodfct late: Ethernet output data function
	void f2_eosfct_1();								//!< f2_eosfct late: Ethernet output start function
	void f2_erbfct_1();								//!< f2_erbfct late: Ethernet reset branch function
	void f2_eefct_1();								//!< f2_eefct late: Ethernet end of transmission function
	void f2_ebfct_1();								//!< f2_ebfct late: Ethernet branch function
	void f2_ecbfct_1();								//!< f2_ecbfct late: Ethernet countdown branch function
	void f2_eisfct_1();								//!< f2_eisfct late: Ethernet input start function
	void eth_activate();							//!< called by the CPU when the Ethernet task becomes active
	void init_ether(int task);						//!< 007 initialize ethernet task

	// memory refresh task
	void f1_mrt_block_0();							//!< f1_mrt_block early: block the display word task
	void mrt_activate();							//!< called by the CPU when MRT becomes active
	void init_mrt(int task);						//!< 010 initialize memory refresh task

	// display word task
	void f1_dwt_block_0();							//!< f1_dwt_block early: block the display word task
	void f2_dwt_load_ddr_1();						//!< f2_dwt_load_ddr late: load the display data register
	void init_dwt(int task);						//!< 011 initialize display word task

	// cursor task
	void f1_curt_block_0();							//!< f1_curt_block early: disable the cursor task and set the curt_blocks flag
	void f2_load_xpreg_1();							//!< f2_load_xpreg late: load the x position register from BUS[6-15]
	void f2_load_csr_1();							//!< f2_load_csr late: load the cursor shift register from BUS[0-15]
	void curt_activate();							//!< curt_activate: called by the CPU when the cursor task becomes active
	void init_curt(int task);					 	//!< 012 initialize cursor task

	// display horizontal task
	void f1_dht_block_0();							//!< f1_dht_block early: disable the display word task
	void f2_dht_setmode_1();						//!< f2_dht_setmode late: set the next scanline's mode inverse and half clock and branch
	void activate_dht();							//!< called by the CPU when the display horizontal task becomes active
	void init_dht(int task);						//!< 013 initialize display horizontal task

	// display vertical task
	void f1_dvt_block_0();							//!< f1_dvt_block early: disable the display word task
	void activate_dvt();							//!< called by the CPU when the display vertical task becomes active
	void init_dvt(int task);						//!< 014 initialize display vertical task

	// parity task
	void activate_part();
	void init_part(int task);						//!< 015 initialize parity task

	// disk word task
	void f1_kwd_block_0(void);
	void init_kwd(int task);						//!< 016 initialize disk word task

	void init_001(int task);						//!< 001 initialize unused task
	void init_002(int task);						//!< 002 initialize unused task
	void init_003(int task);						//!< 003 initialize unused task
	void init_005(int task);						//!< 005 initialize unused task
	void init_006(int task);						//!< 006 initialize unused task
	void init_017(int task);						//!< 017 initialize unused task
};

extern const device_type ALTO2;


#endif /* _CPU_ALTO2_H_ */
