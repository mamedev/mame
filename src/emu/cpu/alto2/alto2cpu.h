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

#include "machine/diablo_hd.h"

#define	ALTO2_TAG "alto2"

/**
 * \brief AltoII register names
 */
enum {
	// micro code task, micro program counter, next and next2
	A2_TASK, A2_MPC, A2_NEXT, A2_NEXT2,
	// BUS, ALU, temp, latch, memory latch and carry flags
	A2_BUS, A2_T, A2_ALU, A2_ALUC0,	A2_L, A2_SHIFTER, A2_LALUC0, A2_M,
	A2_R,	// 32 R registers
	A2_AC3 = A2_R, A2_AC2, A2_AC1, A2_AC0, A2_R04, A2_R05, A2_PC,  A2_R07,
	A2_R10, A2_R11, A2_R12, A2_R13, A2_R14, A2_R15, A2_R16, A2_R17,
	A2_R20, A2_R21, A2_R22, A2_R23, A2_R24, A2_R25, A2_R26, A2_R27,
	A2_R30, A2_R31, A2_R32, A2_R33, A2_R34, A2_R35, A2_R36, A2_R37,
	A2_S,	// 32 S registers
	A2_S00 = A2_S, A2_S01, A2_S02, A2_S03, A2_S04, A2_S05, A2_S06, A2_S07,
	A2_S10, A2_S11, A2_S12, A2_S13, A2_S14, A2_S15, A2_S16, A2_S17,
	A2_S20, A2_S21, A2_S22, A2_S23, A2_S24, A2_S25, A2_S26, A2_S27,
	A2_S30, A2_S31, A2_S32, A2_S33, A2_S34, A2_S35, A2_S36, A2_S37,
	// DISK controller registers
	A2_DRIVE, A2_KADDR, A2_KADR, A2_KSTAT, A2_KCOM, A2_KRECNO,
	A2_SHIFTIN, A2_SHIFTOUT, A2_DATAIN, A2_DATAOUT, A2_KRWC,
	A2_KFER, A2_WDTSKENA, A2_WDINIT0, A2_WDINIT, A2_STROBE,
	A2_BITCLK, A2_DATIN, A2_BITCNT, A2_CARRY, A2_SECLATE,
	A2_SEEKOK, A2_OKTORUN, A2_READY
};

#ifndef	ALTO2_DEBUG
#define	ALTO2_DEBUG             0   //!< define to 1 to enable logerror() output
#endif

#define	USE_PRIO_F9318			0	//!< define to 1 to use the F9318 priority encoder code
#define	USE_ALU_74181			1	//!< define to 1 to use the SN74181 ALU code
#define	DEBUG_DISPLAY_TIMING	0	//!< define to 1 to debug the display timing
#define	USE_BITCLK_TIMER		0	//!< define to 1 to use a very high rate timer for the disk bit clock
#define	ALTO2_HAMMING_CHECK		0	//!< define to 1 to incorporate the Hamming code and Parity check

#define	ALTO2_TASKS		16			//!< 16 task slots
#define	ALTO2_REGS		32			//!< 32 16-bit words in the R register file
#define	ALTO2_ALUF		16			//!< 16 ALU functions (74181)
#define	ALTO2_BUSSRC	8			//!< 8 bus sources
#define	ALTO2_F1MAX		16			//!< 16 F1 functions
#define	ALTO2_F2MAX		16			//!< 16 F2 functions
#define	ALTO2_UCYCLE	169542		//!< time in pico seconds for a CPU micro cycle: 29.4912MHz/5 -> 5.898240Hz ~= 169.542ns/clock

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

/********************************************************************************
 * Bit field primitives
 * These are some macros to make it easier to access variable by the bit-
 * reversed notation that the Xerox Alto documents use all over the place.
 * Bit number 0 is the most significant there,
 * and bit number (width - 1) is the least significant.
 * The X_ is for Xerox and to avoid collisions with MAME generic macros.
 ********************************************************************************/

//! get the left shift required to access bit %to in a word of %width bits
#define X_BITSHIFT(width,to) ((width) - 1 - (to))

//! build a least significant bit mask for bits %from to %to (inclusive)
#define X_BITMASK(from,to) ((1ul << ((to) + 1 - (from))) - 1)

//! get a single bit number %bit value from %reg, a word of %width bits
#define	X_BIT(reg,width,bit) (((reg) >> X_BITSHIFT(width,bit)) & 1)

//! get a bit field from %reg, a word of %width bits, starting at bit %from until bit %to
#define X_RDBITS(reg,width,from,to) (((reg) >> X_BITSHIFT(width,to)) & X_BITMASK(from,to))

//! put a value %val into %reg, a word of %width bits, starting at bit %from until bit %to
#define X_WRBITS(reg,width,from,to,val) do { \
	UINT32 mask = X_BITMASK(from,to) << X_BITSHIFT(width,to); \
	reg = ((reg) & ~mask) | (((val) << X_BITSHIFT(width,to)) & mask); \
} while (0)

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


#define	ALTO2_DISPLAY_FIFO 16														//!< the display fifo has 16 words
#define	ALTO2_DISPLAY_SCANLINE_WORDS (ALTO2_DISPLAY_TOTAL_WIDTH/16)         		//!< words per scanline
#define	ALTO2_DISPLAY_HEIGHT 808                                                    //!< number of visible scanlines per frame; 808 really, but there are some empty lines?
#define	ALTO2_DISPLAY_WIDTH 606                                                     //!< visible width of the display; 38 x 16 bit words - 2 pixels
#define	ALTO2_DISPLAY_VISIBLE_WORDS ((ALTO2_DISPLAY_WIDTH+15)/16)                   //!< visible words per scanline
#define	ALTO2_DISPLAY_BITCLOCK 20160000ll                                           //!< display bit clock in in Hertz (20.16MHz)
#define	ALTO2_DISPLAY_BITTIME(n) (U64(1000000000000)*(n)/ALTO2_DISPLAY_BITCLOCK)	//!< display bit time in in pico seconds (~= 49.6031ns)
#define	ALTO2_DISPLAY_SCANLINE_TIME	ALTO2_DISPLAY_BITTIME(ALTO2_DISPLAY_TOTAL_WIDTH)//!< time for a scanline in pico seconds (768 * 49.6031ns ~= 38095.1808ns)
#define	ALTO2_DISPLAY_VISIBLE_TIME ALTO2_DISPLAY_BITTIME(ALTO2_DISPLAY_WIDTH)		//!< time of the visible part of a scanline in pico seconds (606 * 49.6031ns ~= 30059.4786ns)
#define	ALTO2_DISPLAY_WORD_TIME	ALTO2_DISPLAY_BITTIME(16)							//!< time for a word in pico seconds (16 pixels * 49.6031ns ~= 793.6496ns)
#define	ALTO2_DISPLAY_VBLANK_TIME ((ALTO2_DISPLAY_TOTAL_HEIGHT-ALTO2_DISPLAY_HEIGHT)*HZ_TO_ATTOSECONDS(26250))

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
	~alto2_cpu_device();

	//! driver interface to set diablo_hd_device
	void set_diablo(int unit, diablo_hd_device* ptr);

	//! call in for the next sector callback
	void next_sector(int unit);

	//! return the display bitmap
	bitmap_ind16& display() { return *m_displ_bitmap; }

	DECLARE_ADDRESS_MAP( ucode_map, 32 );
	DECLARE_ADDRESS_MAP( const_map, 16 );
	DECLARE_ADDRESS_MAP( iomem_map, 16 );

	//! register a mouse motion in x direction
	DECLARE_INPUT_CHANGED_MEMBER( mouse_motion_x );
	//! register a mouse motion in y direction
	DECLARE_INPUT_CHANGED_MEMBER( mouse_motion_y );
	//! register a mouse button change
	DECLARE_INPUT_CHANGED_MEMBER( mouse_buttons );

protected:
	//! device-level override for start
	virtual void device_start();
	//! device-level override for reset
	virtual void device_reset();

	//! device-level override for post reset
	void interface_post_reset();

	//! device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 1; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	//! device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

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
		LOG_HW		= (1 << 23),
		LOG_KBD		= (1 << 24),
		LOG_ALL		= ((1 << 25) - 1)
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
	address_space_config m_iomem_config;

	address_space* m_iomem;

	UINT8* m_ucode_crom;
	UINT8* m_ucode_cram;
	UINT8* m_const_data;

	//! read microcode CROM
	DECLARE_READ32_MEMBER ( crom_r );

	//! read microcode CRAM
	DECLARE_READ32_MEMBER ( cram_r );

	//! write microcode CRAM
	DECLARE_WRITE32_MEMBER( cram_w );

	//! read constants PROM
	DECLARE_READ16_MEMBER ( const_r );

	//! read i/o space RAM
	DECLARE_READ16_MEMBER ( ioram_r );

	//!< write i/o space RAM
	DECLARE_WRITE16_MEMBER( ioram_w );

	//!< read memory mapped i/o
	DECLARE_READ16_MEMBER ( mmio_r );

	//!< write memory mapped i/o
	DECLARE_WRITE16_MEMBER( mmio_w );

	int m_icount;

	static const UINT8 m_ether_id = 0241;

	typedef void (alto2_cpu_device::*a2func)();

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
		 * PROM data for S3-0,M,C,T: 1111/1/0/0
		 * function F=A
		 * T source is ALU
		 */
		aluf_bus__alut,
		/**
		 * \brief 01: ALU <- T
		 * PROM data for S3-0,M,C,T: 1010/1/0/0
		 * function F=B
		 * T source is BUS
		 */
		aluf_treg,
		/**
		 * \brief 02: ALU <- BUS | T
		 * PROM data for S3-0,M,C,T: 1110/1/0/1
		 * function F=A|B
		 * T source is ALU
		 */
		aluf_bus_or_t__alut,
		/**
		 * \brief 03: ALU <- BUS & T
		 * PROM data for S3-0,M,C,T: 1011/1/0/0
		 * function F=A&B
		 * T source is BUS
		 */
		aluf_bus_and_t,
		/**
		 * \brief 04: ALU <- BUS ^ T
		 * PROM data for S3-0,M,C,T: 0110/1/0/0
		 * function F=A^B
		 * T source is BUS
		 */
		aluf_bus_xor_t,
		/**
		 * \brief 05: ALU <- BUS + 1
		 * PROM data for S3-0,M,C,T: 0000/0/0/1
		 * function F=A+1
		 * T source is ALU
		 */
		aluf_bus_plus_1__alut,
		/**
		 * \brief 06: ALU <- BUS - 1
		 * PROM data for S3-0,M,C,T: 1111/0/1/1
		 * function F=A-1
		 * T source is ALU
		 */
		aluf_bus_minus_1__alut,
		/**
		 * \brief 07: ALU <- BUS + T
		 * PROM data for S3-0,M,C,T: 1001/0/1/0
		 * function F=A+B
		 * T source is BUS
		 */
		aluf_bus_plus_t,
		/**
		 * \brief 10: ALU <- BUS - T
		 * PROM data for S3-0,M,C,T: 0110/0/0/0
		 * function F=A-B
		 * T source is BUS
		 */
		aluf_bus_minus_t,
		/**
		 * \brief 11: ALU <- BUS - T - 1
		 * PROM data for S3-0,M,C,T: 0110/0/1/0
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
		 * PROM data for S3-0,M,C,T: 0000/0/SKIP/1
		 * function F=A (SKIP=1) or F=A+1 (SKIP=0)
		 * T source is ALU
		 */
		aluf_bus_plus_skip__alut,
		/**
		 * \brief 14: ALU <- BUS & T
		 * PROM data for S3-0,M,C,T: 1011/1/0/1
		 * function F=A&B
		 * T source is ALU
		 */
		aluf_bus_and_t__alut,
		/**
		 * \brief 15: ALU <- BUS & ~T
		 * PROM data for S3-0,M,C,T: 0111/1/0/0
		 * function F=A&~B
		 * T source is BUS
		 */
		aluf_bus_and_not_t,
		/**
		 * \brief 16: ALU <- BUS
		 * PROM data for S3-0,M,C,T: 1111/1/0/1
		 * function F=A
		 * T source is ALU
		 */
		aluf_undef_16,
		/**
		 * \brief 17: ALU <- BUS
		 * PROM data for S3-0,M,C,T: 1111/1/0/1
		 * function F=A
		 * T source is ALU
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

	//! enumeration of the micro code word bits
	//! Note: The Alto documents enumerate bits from left (MSB = 0) to right (LSB = 31)
	enum {
		DRSEL0, DRSEL1, DRSEL2, DRSEL3, DRSEL4,
		DALUF0, DALUF1, DALUF2, DALUF3,
		DBS0, DBS1, DBS2,
		DF1_0, DF1_1, DF1_2, DF1_3,
		DF2_0, DF2_1, DF2_2, DF2_3,
		DLOADT,
		DLOADL,
		NEXT0, NEXT1, NEXT2, NEXT3, NEXT4, NEXT5, NEXT6, NEXT7, NEXT8, NEXT9
	};

	//! get the normally accessed bank number from a bank register
	static inline UINT16 GET_BANK_NORMAL(UINT16 breg) { return X_RDBITS(breg,16,12,13); }

	//! get the extended bank number (accessed via XMAR) from a bank register
	static inline UINT16 GET_BANK_EXTENDED(UINT16 breg) { return X_RDBITS(breg,16,14,15); }

	//! get an ignored bit field from a control RAM address
	static inline UINT16 GET_CRAM_IGNORE(UINT16 addr) { return X_RDBITS(addr,16,0,1); }

	//! get the bank select bit field from a control RAM address
	static inline UINT16 GET_CRAM_BANKSEL(UINT16 addr) { return X_RDBITS(addr,16,2,3); }

	//! get the ROM/RAM flag from a control RAM address
	static inline UINT16 GET_CRAM_RAMROM(UINT16 addr) { return X_RDBITS(addr,16,4,4); }

	//! get the half select flag from a control RAM address
	static inline UINT16 GET_CRAM_HALFSEL(UINT16 addr) { return X_RDBITS(addr,16,5,5); }

	//! get the word address bit field from a control RAM address
	static inline UINT16 GET_CRAM_WORDADDR(UINT16 addr)	{ return X_RDBITS(addr,16,6,15); }

	UINT16 m_task_mpc[ALTO2_TASKS];					//!< per task micro program counter
	UINT16 m_task_next2[ALTO2_TASKS];				//!< per task address modifier
	attoseconds_t m_pico_time[ALTO2_TASKS];				//!< per task atto seconds executed
	UINT8 m_task;									//!< active task
	UINT8 m_next_task;								//!< next micro instruction's task
	UINT8 m_next2_task;								//!< next but one micro instruction's task
	UINT16 m_mpc;									//!< micro program counter
	UINT32 m_mir;									//!< micro instruction register

	/**
	 * \brief current micro instruction's register selection
	 * The emulator F2s ACSOURCE and ACDEST modify this.
	 * Note: The S registers are addressed by the original RSEL[0-4],
	 * even when the the emulator modifies this.
	 */
	UINT8 m_rsel;
	UINT8 m_d_rsel;									//!< decoded RSEL[0-4]
	UINT8 m_d_aluf;									//!< decoded ALUF[0-3] function
	UINT8 m_d_bs;									//!< decoded BS[0-2] bus source
	UINT8 m_d_f1;									//!< decoded F1[0-3] function
	UINT8 m_d_f2;									//!< decoded F2[0-3] function
	UINT8 m_d_loadt;									//!< decoded LOADT flag
	UINT8 m_d_loadl;									//!< decoded LOADL flag
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
	int	m_bitclk_time;								//!< bitclk call time accu
	int m_bitclk_index;								//!< bitclk index (bit number)

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

	/**
	 * @brief unused PROM a90
	 */
	UINT8* m_madr_a90;

	/**
	 * @brief unused PROM a91
	 */
	UINT8* m_madr_a91;

	//! no operating function to put in the m_bs, m_f1 and m_f2 slots
	void noop() {}

	//! per task bus source function pointers, early (0) and late (1)
	a2func m_bs[2][ALTO2_TASKS][ALTO2_BUSSRC];
	void set_bs(UINT8 task, UINT8 fn, a2func f0, a2func f1) {
		m_bs[0][task][fn] = f0 ? f0 : &alto2_cpu_device::noop;
		m_bs[1][task][fn] = f1 ? f1 : &alto2_cpu_device::noop;
	}

	//! per task f1 function pointers, early (0) and late (1)
	a2func m_f1[2][ALTO2_TASKS][ALTO2_F1MAX];
	void set_f1(UINT8 task, UINT8 fn, a2func f0, a2func f1) {
		m_f1[0][task][fn] = f0 ? f0 : &alto2_cpu_device::noop;
		m_f1[1][task][fn] = f1 ? f1 : &alto2_cpu_device::noop;
	}

	//! per task f2 function pointers, early (0) and late (1)
	a2func m_f2[2][ALTO2_TASKS][ALTO2_F2MAX];
	void set_f2(UINT8 task, UINT8 fn, a2func f0, a2func f1) {
		m_f2[0][task][fn] = f0 ? f0 : &alto2_cpu_device::noop;
		m_f2[1][task][fn] = f1 ? f1 : &alto2_cpu_device::noop;
	}

	bool m_ram_related[ALTO2_TASKS];				//!< set when task is RAM related

	UINT64 m_cycle;									//!< number of cycles executed in the current slice

	UINT64 cycle() { return m_cycle; }				//!< return the current CPU cycle
	UINT64 ntime() { return m_cycle*ALTO2_UCYCLE/1000; }	//!< return the current nano seconds

	void hard_reset();								//!< reset the various registers
	int soft_reset();								//!< soft reset

	void fn_bs_bad_0();								//! bs dummy early function
	void fn_bs_bad_1();								//! bs dummy late function

	void fn_f1_bad_0();								//! f1 dummy early function
	void fn_f1_bad_1();								//! f1 dummy late function

	void fn_f2_bad_0();								//! f2 dummy early function
	void fn_f2_bad_1();								//! f2 dummy late function

	DECLARE_READ16_MEMBER( noop_r );				//!< read open bus (0177777)
	DECLARE_WRITE16_MEMBER( noop_w );				//!< write open bus

	DECLARE_READ16_MEMBER( bank_reg_r );			//!< read bank register in memory mapped I/O range
	DECLARE_WRITE16_MEMBER( bank_reg_w );			//!< write bank register in memory mapped I/O range

	void bs_early_read_r();							//!< bus source: drive bus by R register
	void bs_early_load_r();							//!< bus source: load R places 0 on the BUS
	void bs_late_load_r();							//!< bus source: load R from SHIFTER
	void bs_early_read_md();						//!< bus source: drive BUS from read memory data
	void bs_early_mouse();							//!< bus source: drive bus by mouse
	void bs_early_disp();							//!< bus source: drive bus by displacement (which?)
	void f1_early_block();							//!< F1 func: block active task
	void f1_late_load_mar();						//!< F1 func: load memory address register
	void f1_early_task();							//!< F1 func: task switch
	void f1_late_l_lsh_1();							//!< F1 func: SHIFTER = left shift L once
	void f1_late_l_rsh_1();							//!< F1 func: SHIFTER = right shift L once
	void f1_late_l_lcy_8();							//!< F1 func: SHIFTER = byte swap L
	void f2_late_bus_eq_zero();						//!< F2 func: branch on bus equals zero
	void f2_late_shifter_lt_zero();					//!< F2 func: branch on shifter less than zero
	void f2_late_shifter_eq_zero();					//!< F2 func: branch on shifter equals zero
	void f2_late_bus();								//!< F2 func: branch on bus bits BUS[6-15]
	void f2_late_alucy();							//!< F2 func: branch on latched ALU carry
	void f2_late_load_md();							//!< F2 func: load memory data

	UINT8* m_alu_a10;								//!< ALU function to 74181 operation lookup PROM
#if	USE_ALU_74181
	UINT32 alu_74181(UINT32 a, UINT32 b, UINT8 smc);
#endif
	void rdram();									//!< read the microcode ROM/RAM halfword
	void wrtram();									//!< write the microcode RAM from M register and ALU

	// ************************************************
	// ram related stuff
	// ************************************************
	void bs_early_read_sreg();						//!< bus source: drive bus by S register or M (MYL), if rsel is = 0
	void bs_early_load_sreg();						//!< bus source: load S register puts garbage on the bus
	void bs_late_load_sreg();						//!< bus source: load S register from M
	void branch_ROM(const char *from, int page);	//!< branch to ROM page
	void branch_RAM(const char *from, int page);	//!< branch to RAM page
	void f1_late_swmode();							//!< F1 func: switch to micro program counter BUS[6-15] in other bank
	void f1_late_wrtram();							//!< F1 func: start WRTRAM cycle
	void f1_late_rdram();							//!< F1 func: start RDRAM cycle
#if	(ALTO2_UCODE_RAM_PAGES == 3)
	void f1_late_load_rmr();						//!< F1 func: load the reset mode register
#else	// ALTO2_UCODE_RAM_PAGES != 3
	void f1_late_load_srb();						//!< F1 func: load the S register bank from BUS[12-14]
#endif
	void init_ram(int task);						//!< called by RAM related tasks
	void exit_ram();

#	include "a2hw.h"
	// ************************************************
	// keyboard stuff
	// ************************************************
	struct {
		UINT16 bootkey;							//!< boot key - key code pressed before power on
		UINT16 matrix[4];						//!< a bit map of the keys pressed (ioports ROW0 ... ROW3)
	}	m_kbd;
	DECLARE_READ16_MEMBER( kbd_ad_r );			//!< read the keyboard matrix
	void init_kbd(UINT16 bootkey = 0177777);	//!< initialize the keyboard hardware, optinally set the boot key
	void exit_kbd();							//!< deinitialize the keyboard hardware

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
	 * </PRE>
	 */
	UINT8* m_madr_a32;

	//! mouse context
	struct {
		int x;										//!< current X coordinate
		int y;										//!< current Y coordinate
		int dx;										//!< destination X coordinate (real mouse X)
		int dy;										//!< destination Y coordinate (real mouse Y)
		UINT8 latch;								//!< current latch value
		UINT8 phase;								//!< current read latch phase
	}	m_mouse;

	UINT16 mouse_read();							//!< return the mouse motion flags
	void init_mouse();								//!< initialize the mouse context to useful values
	void exit_mouse();								//!< deinitialize the mouse

	// ************************************************
	// disk controller stuff
	// ************************************************
	diablo_hd_device* m_drive[2];		//!< two diablo_hd_device drives

	//! disk controller context
	struct {
		UINT8 drive;					//!< selected drive from KADDR[14] (written to data out with SENDADR)
		UINT16 kaddr;					//!< A[0-15] disk hardware address (sector, cylinder, head, drive, restore)
		UINT16 kadr;					//!< C[0-15] with read/write/check modes for header, label and data
		UINT16 kstat;					//!< S[0-15] disk status
		UINT16 kcom;					//!< disk command (5 bits kcom[1-5])
		UINT8 krecno;					//!< record number (2 bits indexing header, label, data, -/-)
		UINT8 egate;					//!< current erase gate signal to the DIABLO hd
		UINT8 wrgate;					//!< current write gate signal to the DIABLO hd
		UINT8 rdgate;					//!< current read gate signal to the DIABLO hd
		UINT32 shiftin;					//!< input shift register
		UINT32 shiftout;				//!< output shift register
		UINT32 datain;					//!< disk data in latch
		UINT32 dataout;					//!< disk data out latch
		UINT8 krwc;						//!< read/write/check for current record
		UINT8 kfer;						//!< disk fatal error signal state
		UINT8 wdtskena;					//!< disk word task enable (active low)
		UINT8 wddone;					//!< previous state of WDDONE
		UINT8 wdinit0;					//!< disk word task init at the early microcycle
		UINT8 wdinit;					//!< disk word task init at the late microcycle
		UINT8 strobe;					//!< strobe (still) active
		emu_timer* strobon_timer;		//!< set strobe on timer
		UINT8 bitclk;					//!< current bitclk state (either crystal clock, or rdclk from the drive)
#if	USE_BITCLK_TIMER
		emu_timer* bitclk_timer;		//!< bit clock timer
#else
		int bitclk_time;				//!< time in clocks per bit
#endif
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
	//! lookup JK flip-flop state change from s0 to s1
	inline jkff_t update_jkff(UINT8 s0, UINT8 s1);

	void kwd_timing(int bitclk, int datin, int block);	//!< disk word timing
	TIMER_CALLBACK_MEMBER( disk_seclate );			//!< timer callback to take away the SECLATE pulse (monoflop)
	TIMER_CALLBACK_MEMBER( disk_ok_to_run );		//!< timer callback to take away the OK TO RUN pulse (reset)
	TIMER_CALLBACK_MEMBER( disk_strobon );			//!< timer callback to pulse the STROBE' signal to the drive
	TIMER_CALLBACK_MEMBER( disk_ready_mf31a );		//!< timer callback to change the READY monoflop 31a
#if	USE_BITCLK_TIMER
	TIMER_CALLBACK_MEMBER( disk_bitclk );			//!< callback to update the disk controller with a new bitclk
#else
	void disk_bitclk(void *ptr, int arg);			//!< function to update the disk controller with a new bitclk
#endif
	void disk_block(int task);						//!< called if one of the disk tasks (task_kwd or task_ksec) blocks
	void bs_early_read_kstat();						//!< bus source: bus driven by disk status register KSTAT
	void bs_early_read_kdata();						//!< bus source: bus driven by disk data register KDATA input
	void f1_late_strobe();							//!< F1 func: initiates a disk seek
	void f1_late_load_kstat();						//!< F1 func: load disk status register
	void f1_late_load_kdata();						//!< F1 func: load data out register, or the disk address register
	void f1_late_increcno();						//!< F1 func: advances shift registers holding KADR
	void f1_late_clrstat();							//!< F1 func: reset all error latches
	void f1_late_load_kcom();						//!< F1 func: load the KCOM register from bus
	void f1_late_load_kadr();						//!< F1 func: load the KADR register from bus
	void f2_late_init();							//!< F2 func: branch on disk word task active and init
	void f2_late_rwc();								//!< F2 func: branch on read/write/check state of the current record
	void f2_late_recno();							//!< F2 func: branch on the current record number by a lookup table
	void f2_late_xfrdat();							//!< F2 func: branch on the data transfer state
	void f2_late_swrnrdy();							//!< F2 func: branch on the disk ready signal
	void f2_late_nfer();							//!< f2_nfer late: branch on the disk fatal error condition
	void f2_late_strobon();							//!< f2_strobon late: branch on the seek busy status
	void init_disk();								//!< initialize the disk context
	void exit_disk();								//!< deinitialize the disk context

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
		UINT16 **scanline;					//!< array of pointers to the scanlines
	}	m_dsp;

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

	//! output bits of PROM A38
	enum {
		A38_STOPWAKE	= (1 << 1),
		A38_MBEMPTY		= (1 << 3)
	};

	//! PROM a38 bit O1 is STOPWAKE' (stop DWT if bit is zero)
	inline UINT8 FIFO_STOPWAKE_0() { return m_disp_a38[m_dsp.fifo_rd * 16 + m_dsp.fifo_wr] & A38_STOPWAKE; }

	//! PROM a38 bit O3 is MBEMPTY' (FIFO is empty if bit is zero)
	inline UINT8 FIFO_MBEMPTY_0() { return m_disp_a38[m_dsp.fifo_rd * 16 + m_dsp.fifo_wr] & A38_MBEMPTY; }

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

	//! test the VSYNC (vertical synchronisation) signal in PROM a66 being high
	static inline bool A66_VSYNC_HI(UINT8 a, int hlc1024) { return a & (hlc1024 ? A66_VSYNC_ODD : A66_VSYNC_EVEN) ? false : true; }
	//! test the VSYNC (vertical synchronisation) signal in PROM a66 being low
	static inline bool A66_VSYNC_LO(UINT8 a, int hlc1024) { return a & (hlc1024 ? A66_VSYNC_ODD : A66_VSYNC_EVEN) ? true : false; }
	//! test the VBLANK (vertical blanking) signal in PROM a66 being high
	static inline bool A66_VBLANK_HI(UINT8 a, int hlc1024) { return a & (hlc1024 ? A66_VBLANK_ODD : A66_VBLANK_EVEN) ? false : true; }
	//! test the VBLANK (vertical blanking) signal in PROM a66 being low
	static inline bool A66_VBLANK_LO(UINT8 a, int hlc1024) { return a & (hlc1024 ? A66_VBLANK_ODD : A66_VBLANK_EVEN) ? true : false; }

	//! screen bitmap
	bitmap_ind16* m_displ_bitmap;

	//! update a word in the screen bitmap
	void update_bitmap_word(int x, int y, UINT16 word);

	//! unload the next word from the display FIFO and shift it to the screen
	void unload_word();

	/**
	 * @brief function called by the CPU to enter the next display state
	 *
	 * There are 32 states per scanline and 875 scanlines per frame.
	 *
	 * @param arg the current m_disp_a63 PROM address
	 * @return next state of the display state machine
	 */
	void display_state_machine();

	//! branch on the evenfield flip-flop
	void f2_late_evenfield(void);

	//! initialize the display context
	void init_disp();
	//! deinitialize the display context
	void exit_disp();

	// ************************************************
	// memory stuff
	// ************************************************

	enum {
		ALTO2_MEM_NONE,
		ALTO2_MEM_ODD	= (1 << 0),
		ALTO2_MEM_RAM	= (1 << 1),
		ALTO2_MEM_NIRVANA	= (1 << 2)
	};

	struct {
		UINT32* ram;						//!< main memory organized as double-words
		UINT8* hpb;							//!< Hamming Code bits (6) and Parity bits (1) per double word
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

	//! memory error address register read
	DECLARE_READ16_MEMBER( mear_r );

	//! memory error status register read
	DECLARE_READ16_MEMBER( mesr_r );

	//! memory error status register write (clear)
	DECLARE_WRITE16_MEMBER( mesr_w );

	//! memory error control register read
	DECLARE_READ16_MEMBER( mecr_r );

	//! memory error control register write
	DECLARE_WRITE16_MEMBER( mecr_w );

	//! read or write a memory double-word and caluclate its Hamming code
	UINT32 hamming_code(int write, UINT32 dw_addr, UINT32 dw_data);

	//! load the memory address register with some value
	void load_mar(UINT8 rsel, UINT32 addr);

	//! read memory or memory mapped I/O from the address in mar to md
	UINT16 read_mem();

	//! write memory or memory mapped I/O from md to the address in mar
	void write_mem(UINT16 data);

	//! debugger interface to read memory
	UINT16 debug_read_mem(UINT32 addr);

	//! debugger interface to write memory
	void debug_write_mem(UINT32 addr, UINT16 data);

#if	ALTO2_DEBUG
	void watch_write(UINT32 addr, UINT32 data);
	void watch_read(UINT32 addr, UINT32 data);
#endif
	void init_memory();								//!< initialize the memory system
	void exit_memory();								//!< deinitialize the memory system

	// ************************************************
	// emulator task
	// ************************************************
	struct {
		UINT16 ir;									//!< emulator instruction register
		UINT8 skip;									//!< emulator skip
		UINT8 cy;									//!< emulator carry
	}	m_emu;
	void bs_early_emu_disp();						//!< bus source: drive bus by IR[8-15], possibly sign extended
	void f1_early_emu_block();						//!< F1 func: block task
	void f1_late_emu_load_rmr();					//!< F1 func: load the reset mode register
	void f1_late_emu_load_esrb();					//!< F1 func: load the extended S register bank from BUS[12-14]
	void f1_early_rsnf();							//!< F1 func: drive the bus from the Ethernet node ID
	void f1_early_startf();							//!< F1 func: defines commands for for I/O hardware, including Ethernet
	void f2_late_busodd();							//!< F2 func: branch on odd bus
	void f2_late_magic();							//!< F2 func: shift and use T
	void f2_early_load_dns();						//!< F2 func: modify RESELECT with DstAC = (3 - IR[3-4])
	void f2_late_load_dns();						//!< F2 func: do novel shifts
	void f2_early_acdest();							//!< F2 func: modify RSELECT with DstAC = (3 - IR[3-4])
	void bitblt_info();								//!< debug bitblt opcode
	void f2_late_load_ir();							//!< F2 func: load instruction register IR and branch on IR[0,5-7]
	void f2_late_idisp();							//!< F2 func: branch on: arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
	void f2_early_acsource();						//!< F2 func: modify RSELECT with SrcAC = (3 - IR[1-2])
	void f2_late_acsource();						//!< F2 func: branch on arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
	void init_emu(int task);						//!< 000 initialize emulator task
	void exit_emu();								//!< deinitialize emulator task

	// ************************************************
	// ksec task
	// ************************************************
	void f1_early_ksec_block(void);
	void init_ksec(int task);						//!< 004 initialize disk sector task
	void exit_ksec();

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
	void bs_early_eidfct();							//!< bus source: Ethernet input data function
	void f1_early_eth_block();						//!< F1 func: block the Ether task
	void f1_early_eilfct();							//!< F1 func: Ethernet input look function
	void f1_early_epfct();							//!< F1 func: Ethernet post function
	void f1_late_ewfct();							//!< F1 func: Ethernet countdown wakeup function
	void f2_late_eodfct();							//!< F2 func: Ethernet output data function
	void f2_late_eosfct();							//!< F2 func: Ethernet output start function
	void f2_late_erbfct();							//!< F2 func: Ethernet reset branch function
	void f2_late_eefct();							//!< F2 func: Ethernet end of transmission function
	void f2_late_ebfct();							//!< F2 func: Ethernet branch function
	void f2_late_ecbfct();							//!< F2 func: Ethernet countdown branch function
	void f2_late_eisfct();							//!< F2 func: Ethernet input start function
	void activate_eth();							//!< called by the CPU when the Ethernet task becomes active
	void init_ether(int task);						//!< 007 initialize ethernet task
	void exit_ether();								//!< deinitialize ethernet task

	// ************************************************
	// memory refresh task
	// ************************************************
	void f1_early_mrt_block();						//!< F1 func: block the display word task
	void activate_mrt();							//!< called by the CPU when MRT becomes active
	void init_mrt(int task);						//!< 010 initialize memory refresh task
	void exit_mrt();								//!< deinitialize memory refresh task

	// ************************************************
	// display word task
	// ************************************************
	void f1_early_dwt_block();						//!< F1 func: block the display word task
	void f2_dwt_load_ddr_1();						//!< F2 func: load the display data register
	void init_dwt(int task);						//!< 011 initialize display word task
	void exit_dwt();								//!< deinitialize display word task

	// ************************************************
	// cursor task
	// ************************************************
	void f1_early_curt_block();						//!< f1_curt_block early: disable the cursor task and set the curt_blocks flag
	void f2_late_load_xpreg();						//!< f2_load_xpreg late: load the x position register from BUS[6-15]
	void f2_late_load_csr();						//!< f2_load_csr late: load the cursor shift register from BUS[0-15]
	void activate_curt();							//!< curt_activate: called by the CPU when the cursor task becomes active
	void init_curt(int task);					 	//!< 012 initialize cursor task
	void exit_curt();								//!< deinitialize cursor task

	// ************************************************
	// display horizontal task
	// ************************************************
	void f1_early_dht_block();						//!< F1 func: disable the display word task
	void f2_late_dht_setmode();						//!< F2 func: set the next scanline's mode inverse and half clock and branch
	void activate_dht();							//!< called by the CPU when the display horizontal task becomes active
	void init_dht(int task);						//!< 013 initialize display horizontal task
	void exit_dht();								//!< deinitialize display horizontal task

	// ************************************************
	// display vertical task
	// ************************************************
	void f1_early_dvt_block();						//!< F1 func: disable the display word task
	void activate_dvt();							//!< called by the CPU when the display vertical task becomes active
	void init_dvt(int task);						//!< 014 initialize display vertical task
	void exit_dvt();								//!< deinitialize display vertical task

	// ************************************************
	// parity task
	// ************************************************
	void activate_part();
	void init_part(int task);						//!< 015 initialize parity task
	void exit_part();								//!< deinitialize parity task

	// ************************************************
	// disk word task
	// ************************************************
	void f1_early_kwd_block();						//!< F1 func: disable the disk word task
	void init_kwd(int task);						//!< 016 initialize disk word task
	void exit_kwd();								//!< deinitialize disk word task
};

extern const device_type ALTO2;


#endif /* _CPU_ALTO2_H_ */
