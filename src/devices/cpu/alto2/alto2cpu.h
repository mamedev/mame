// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII CPU core interface
 *
 *****************************************************************************/
#ifndef _CPU_ALTO2_H_
#define _CPU_ALTO2_H_

#define ALTO2_TAG "alto2"

#include "emu.h"
#include "debugger.h"
#include "machine/diablo_hd.h"

/**
 * \brief AltoII register names
 */
enum {
	// micro code task, micro program counter, next and next2
	A2_TASK, A2_MPC, A2_NEXT, A2_NEXT2,
	// BUS, ALU, temp, latch, memory latch and carry flags
	A2_BUS, A2_T, A2_ALU, A2_ALUC0, A2_L, A2_SHIFTER, A2_LALUC0, A2_M,
	A2_R,   // 32 R registers
	A2_AC3 = A2_R, A2_AC2, A2_AC1, A2_AC0, A2_R04, A2_R05, A2_PC,  A2_R07,
	A2_R10, A2_R11, A2_R12, A2_R13, A2_R14, A2_R15, A2_R16, A2_R17,
	A2_R20, A2_R21, A2_R22, A2_R23, A2_R24, A2_R25, A2_R26, A2_R27,
	A2_R30, A2_R31, A2_R32, A2_R33, A2_R34, A2_R35, A2_R36, A2_R37,
	A2_S,   // 32 S registers
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

#ifndef ALTO2_DEBUG
#define ALTO2_DEBUG             1           //!< define to 1 to enable logerror() output
#endif

#ifndef ALTO2_CRAM_CONFIG
#define ALTO2_CRAM_CONFIG       2           //!< use default CROM/CRAM configuration 2
#endif

#define ALTO2_FAKE_STATUS_H     12          //!< number of extra scanlines to display some status info

#define USE_PRIO_F9318          0           //!< define to 1 to use the F9318 priority encoder code
#define USE_ALU_74181           1           //!< define to 1 to use the SN74181 ALU code
#define USE_BITCLK_TIMER        0           //!< define to 1 to use a very high rate timer for the disk bit clock
#define USE_HAMMING_CHECK       1           //!< define to 1 to use the Hamming code and Parity check in a2mem

#define ALTO2_TASKS             16          //!< 16 task slots
#define ALTO2_REGS              32          //!< 32 16-bit words in the R register file
#define ALTO2_ALUF              16          //!< 16 ALU functions (74181)
#define ALTO2_BUSSRC            8           //!< 8 bus sources
#define ALTO2_F1MAX             16          //!< 16 F1 functions
#define ALTO2_F2MAX             16          //!< 16 F2 functions
#define ALTO2_UCYCLE            169542      //!< time in pico seconds for a CPU micro cycle: 29.4912MHz/5 -> 5.898240Hz ~= 169.542ns/clock

#define ALTO2_CONST_SIZE        256         //!< number words in the constant ROM

//! inverted bits in the micro instruction 32 bit word
#define ALTO2_UCODE_INVERTED    ((1 << 10) | (1 << 15) | (1 << 19))

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
#define X_BIT(reg,width,bit) (((reg) >> X_BITSHIFT(width,bit)) & 1)

//! get a bit field from %reg, a word of %width bits, starting at bit %from until bit %to
#define X_RDBITS(reg,width,from,to) (((reg) >> X_BITSHIFT(width,to)) & X_BITMASK(from,to))

//! put a value %val into %reg, a word of %width bits, starting at bit %from until bit %to
#define X_WRBITS(reg,width,from,to,val) do { \
	UINT32 mask = X_BITMASK(from,to) << X_BITSHIFT(width,to); \
	reg = ((reg) & ~mask) | (((val) << X_BITSHIFT(width,to)) & mask); \
} while (0)

#if ALTO2_DEBUG
	enum LOG_TYPE_ENUM {
		LOG_0,
		LOG_CPU     = (1 <<  0),
		LOG_EMU     = (1 <<  1),
		LOG_T01     = (1 <<  2),
		LOG_T02     = (1 <<  3),
		LOG_T03     = (1 <<  4),
		LOG_KSEC    = (1 <<  5),
		LOG_T05     = (1 <<  6),
		LOG_T06     = (1 <<  7),
		LOG_ETH     = (1 <<  8),
		LOG_MRT     = (1 <<  9),
		LOG_DWT     = (1 << 10),
		LOG_CURT    = (1 << 11),
		LOG_DHT     = (1 << 12),
		LOG_DVT     = (1 << 13),
		LOG_PART    = (1 << 14),
		LOG_KWD     = (1 << 15),
		LOG_T17     = (1 << 16),
		LOG_MEM     = (1 << 17),
		LOG_RAM     = (1 << 18),
		LOG_DRIVE   = (1 << 19),
		LOG_DISK    = (1 << 20),
		LOG_DISPL   = (1 << 21),
		LOG_MOUSE   = (1 << 22),
		LOG_HW      = (1 << 23),
		LOG_KBD     = (1 << 24),
		LOG_ALL     = ((1 << 25) - 1)
	};
	extern int m_log_types;
	extern int m_log_level;
	extern bool m_log_newline;
	void logprintf(device_t *device, int type, int level, const char* format, ...);
#   define  LOG(x) logprintf x
#else
#   define  LOG(x)
#endif

//*******************************************
// define constants from the sub-devices
//*******************************************
#define ALTO2_DEFINE_CONSTANTS 1
#include "a2jkff.h"
#include "a2ram.h"
#include "a2hw.h"
#include "a2kbd.h"
#include "a2mouse.h"
#include "a2disk.h"
#include "a2disp.h"
#include "a2mem.h"
#include "a2emu.h"
#include "a2ksec.h"
#include "a2ether.h"
#include "a2mrt.h"
#include "a2dwt.h"
#include "a2curt.h"
#include "a2dht.h"
#include "a2dvt.h"
#include "a2part.h"
#include "a2dwt.h"
#include "a2kwd.h"
#undef ALTO2_DEFINE_CONSTANTS

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

	//! update the screen bitmap
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	//! screen VBLANK handler
	void screen_eof(screen_device &screen, bool state);

	DECLARE_ADDRESS_MAP( ucode_map, 32 );
	DECLARE_ADDRESS_MAP( const_map, 16 );
	DECLARE_ADDRESS_MAP( iomem_map, 16 );

	//! register a mouse motion in x direction
	DECLARE_INPUT_CHANGED_MEMBER( mouse_motion_x );
	//! register a mouse motion in y direction
	DECLARE_INPUT_CHANGED_MEMBER( mouse_motion_y );
	//! register a mouse button change
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_0 );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_1 );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_2 );

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
	void state_string_export(const device_state_entry &entry, std::string &str);

	//! device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:

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

	typedef void (alto2_cpu_device::*a2func)();

	//! task numbers
	enum {
		task_emu,       //!< emulator task
		task_1,         //!< unused
		task_2,         //!< unused
		task_3,         //!< unused
		task_ksec,      //!< disk sector task
		task_5,         //!< unused
		task_6,         //!< unused
		task_ether,     //!< ethernet task
		task_mrt,       //!< memory refresh task
		task_dwt,       //!< display word task
		task_curt,      //!< cursor task
		task_dht,       //!< display horizontal task
		task_dvt,       //!< display vertical task
		task_part,      //!< parity task
		task_kwd,       //!< disk word task
		task_17         //!< unused task slot 017
	};

	//! register select values accessing R (Note: register numbers are octal)
	enum {
		rsel_ac3,       //!< AC3 used by emulator as accu 3. Also used by Mesa emulator to keep bytecode to execute after breakpoint
		rsel_ac2,       //!< AC2 used by emulator as accu 2. Also used by Mesa emulator as x register for xfer
		rsel_ac1,       //!< AC1 used by emulator as accu 1. Also used by Mesa emulator as r-temporary for return indices and values
		rsel_ac0,       //!< AC0 used by emulator as accu 0. Also used by Mesa emulator as new field bits for WF and friends
		rsel_r04,       //!< NWW state of the interrupt system
		rsel_r05,       //!< SAD. Also used by Mesa emulator as scratch R-register for counting
		rsel_pc,        //!< PC used by emulator as program counter
		rsel_r07,       //!< XREG. Also used by Mesa emulator as task hole, i.e. pigeonhole for saving things across tasks.
		rsel_r10,       //!< XH. Also used by Mesa emulator as instruction byte register
		rsel_r11,       //!< CLOCKTEMP - used in the MRT
		rsel_r12,       //!< ECNTR remaining words in buffer - ETHERNET
		rsel_r13,       //!< EPNTR points BEFORE next word in buffer - ETHERNET
		rsel_r14,
		rsel_r15,       //!< MPC. Used by the Mesa emulator as program counter
		rsel_r16,       //!< STKP. Used by the Mesa emulator as stack pointer [0-10] 0 empty, 10 full
		rsel_r17,       //!< XTSreg. Used by the Mesa emulator to xfer trap state
		rsel_r20,       //!< CURX. Holds cursor X; used by the cursor task
		rsel_r21,       //!< CURDATA. Holds the cursor data; used by the cursor task
		rsel_r22,       //!< CBA. Holds the address of the currently active DCB+1
		rsel_r23,       //!< AECL. Holds the address of the end of the current scanline's bitmap
		rsel_r24,       //!< SLC. Holds the number of scanlines remaining in currently active DCB
		rsel_r25,       //!< MTEMP. Holds the temporary cell
		rsel_r26,       //!< HTAB. Holds the number of tab words remaining on current scanline
		rsel_r27,       //!< YPOS
		rsel_r30,       //!< DWA. Holds the address of the bit map doubleword currently being fetched for transmission to the hardware buffer.
		rsel_r31,       //!< KWDCT. Used by the disk tasks as word counter
		rsel_r32,       //!< CKSUMR. Used by the disk tasks as checksum register (and *amble counter?)
		rsel_r33,       //!< KNMAR. Used by the disk tasks as transfer memory address register
		rsel_r34,       //!< DCBR. Used by the disk tasks to keep the current device control block
		rsel_r35,       //!< TEMP. Used by the Mesa emulator, and also by BITBLT
		rsel_r36,       //!< TEMP2. Used by the Mesa emulator, and also by BITBLT
		rsel_r37        //!< CLOCKREG. Low order bits of the real time clock
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
		bs_read_r,                          //!< BUS source is R register
		bs_load_r,                          //!< load R register from BUS
		bs_no_source,                       //!< BUS is open (0177777)
		bs_task_3,                          //!< BUS source is task specific
		bs_task_4,                          //!< BUS source is task specific
		bs_read_md,                         //!< BUS source is memory data
		bs_mouse,                           //!< BUS source is mouse data
		bs_disp                             //!< BUS source displacement (emulator task)
	};

	//! Function 1 numbers
	enum {
		f1_nop,                             //!< f1 00 no operation
		f1_load_mar,                        //!< f1 01 load memory address register
		f1_task,                            //!< f1 02 task switch
		f1_block,                           //!< f1 03 task block
		f1_l_lsh_1,                         //!< f1 04 left shift L once
		f1_l_rsh_1,                         //!< f1 05 right shift L once
		f1_l_lcy_8,                         //!< f1 06 cycle L 8 times
		f1_const,                           //!< f1 07 constant from PROM

		f1_task_10,                         //!< f1 10 task specific
		f1_task_11,                         //!< f1 11 task specific
		f1_task_12,                         //!< f1 12 task specific
		f1_task_13,                         //!< f1 13 task specific
		f1_task_14,                         //!< f1 14 task specific
		f1_task_15,                         //!< f1 15 task specific
		f1_task_16,                         //!< f1 16 task specific
		f1_task_17                          //!< f1 17 task specific
	};

	//! Function 2 numbers
	enum {
		f2_nop,                             //!< f2 00 no operation
		f2_bus_eq_zero,                     //!< f2 01 branch on bus equals 0
		f2_shifter_lt_zero,                 //!< f2 02 branch on shifter less than 0
		f2_shifter_eq_zero,                 //!< f2 03 branch on shifter equals 0
		f2_bus,                             //!< f2 04 branch on BUS[6-15]
		f2_alucy,                           //!< f2 05 branch on (latched) ALU carry
		f2_load_md,                         //!< f2 06 load memory data
		f2_const,                           //!< f2 07 constant from PROM

		f2_task_10,                         //!< f2 10 task specific
		f2_task_11,                         //!< f2 11 task specific
		f2_task_12,                         //!< f2 12 task specific
		f2_task_13,                         //!< f2 13 task specific
		f2_task_14,                         //!< f2 14 task specific
		f2_task_15,                         //!< f2 15 task specific
		f2_task_16,                         //!< f2 16 task specific
		f2_task_17                          //!< f2 17 task specific
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
	static inline UINT16 GET_CRAM_WORDADDR(UINT16 addr) { return X_RDBITS(addr,16,6,15); }

	UINT16 m_task_mpc[ALTO2_TASKS];                 //!< per task micro program counter
	UINT16 m_task_next2[ALTO2_TASKS];               //!< per task address modifier
	UINT8 m_task;                                   //!< active task
	UINT8 m_next_task;                              //!< next micro instruction's task
	UINT8 m_next2_task;                             //!< next but one micro instruction's task
	UINT16 m_mpc;                                   //!< micro program counter
	UINT32 m_mir;                                   //!< micro instruction register

	/**
	 * \brief current micro instruction's register selection
	 * The emulator F2s ACSOURCE and ACDEST modify this.
	 * Note: The S registers are addressed by the original RSEL[0-4],
	 * even when the emulator modifies this.
	 */
	UINT8 m_rsel;
	UINT8 m_d_rsel;                                 //!< decoded RSEL[0-4]
	UINT8 m_d_aluf;                                 //!< decoded ALUF[0-3] function
	UINT8 m_d_bs;                                   //!< decoded BS[0-2] bus source
	UINT8 m_d_f1;                                   //!< decoded F1[0-3] function
	UINT8 m_d_f2;                                   //!< decoded F2[0-3] function
	UINT8 m_d_loadt;                                    //!< decoded LOADT flag
	UINT8 m_d_loadl;                                    //!< decoded LOADL flag
	UINT16 m_next;                                  //!< current micro instruction's next
	UINT16 m_next2;                                 //!< next micro instruction's next
	UINT16 m_r[ALTO2_REGS];                         //!< R register file
	UINT16 m_s[ALTO2_SREG_BANKS][ALTO2_REGS];       //!< S register file(s)
	UINT16 m_bus;                                   //!< wired-AND bus
	UINT16 m_t;                                     //!< T register
	UINT16 m_alu;                                   //!< the current ALU
	UINT16 m_aluc0;                                 //!< the current ALU carry output
	UINT16 m_l;                                     //!< L register
	UINT16 m_shifter;                               //!< shifter output
	UINT16 m_laluc0;                                //!< the latched ALU carry output
	UINT16 m_m;                                     //!< M register of RAM related tasks (MYL latch in the schematics)
	UINT16 m_cram_addr;                             //!< constant RAM address
	UINT16 m_task_wakeup;                           //!< task wakeup: bit 1<<n set if task n requesting service
	a2func m_active_callback[ALTO2_TASKS];          //!< task activation callbacks

	UINT16 m_reset_mode;                            //!< reset mode register: bit 1<<n set if task n starts in ROM
	bool m_rdram_flag;                              //!< set by rdram, action happens on next cycle
	bool m_wrtram_flag;                             //!< set by wrtram, action happens on next cycle

	UINT8 m_s_reg_bank[ALTO2_TASKS];                //!< active S register bank per task
	UINT8 m_bank_reg[ALTO2_TASKS];                  //!< normal and extended RAM banks per task
	bool m_ether_enable;                            //!< set to true, if the ethernet should be simulated
	bool m_ewfct;                                   //!< set by Ether task when it want's a wakeup at switch to task_mrt
	int m_dsp_time;                                 //!< display_state_machine() time accu
	int m_unload_time;                              //!< unload word time accu
	int m_unload_word;                              //!< unload word number
#if (USE_BITCLK_TIMER == 0)
	int m_bitclk_time;                              //!< bitclk call time accu
	int m_bitclk_index;                             //!< bitclk index (bit number)
#endif

	static const char *task_name(int task);         //!< human readable task names
	static const char *r_name(UINT8 reg);           //!< human readable register names
	static const char *aluf_name(UINT8 aluf);       //!< human readable ALU function names
	static const char *bs_name(UINT8 bs);           //!< human readable bus source names
	static const char *f1_name(UINT8 f1);           //!< human readable F1 function names
	static const char *f2_name(UINT8 f2);           //!< human readable F2 function names

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
	 *  both F2[0] and F2[1] are 1  F2 functions 014, 015, 016, 017
	 *  F2=14 is 0                  not for F2 = 14 (load IR<-)
	 *  IR[0] is 0                  not for arithmetic group
	 *
	 * This means it controls the F2 functions 015:IDISP<- and 016:<-ACSOURCE
	 *
	 * Its address lines are:
	 *  line   pin   connected to         load swap
	 *  -------------------------------------------------------------------
	 *  A0     5     F2[2] (i.e. MIR[18]) IR[07]
	 *  A1     6     IR[01]               IR[06]
	 *  A2     7     IR[02]               IR[05]
	 *  A3     4     IR[03]               IR[04]
	 *  A4     3     IR[04]               IR[03]
	 *  A5     2     IR[05]               IR[02]
	 *  A6     1     IR[06]               IR[01]
	 *  A7     15    IR[07]               F2[2]
	 *
	 * Its data lines are:
	 *  line   pin   connected to         load
	 *  -------------------------------------------------------------------
	 *  D3     9     NEXT[06]'            NEXT[06]
	 *  D2     10    NEXT[07]'            NEXT[07]
	 *  D1     11    NEXT[08]'            NEXT[08]
	 *  D0     12    NEXT[09]'            NEXT[09]
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
	 *  line   pin    signal
	 *  -------------------------------
	 *  A0     10     CT1 (current task LSB)
	 *  A1     11     CT2
	 *  A2     12     CT4
	 *  A3     13     CT8 (current task MSB)
	 *  A4     14     0 (GND)
	 *
	 *  line   pin    signal
	 *  -------------------------------
	 *  B0     1      RDCT8'
	 *  B1     2      RDCT4'
	 *  B2     3      RDCT2'
	 *  B3     4      RDCT1'
	 *  B4     5      NEXT[09]'
	 *  B5     6      NEXT[08]'
	 *  B6     7      NEXT[07]'
	 *  B7     9      NEXT[06]'
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
	 *  input line    signal
	 *  ----------------------------
	 *  A7    15      EMACT'
	 *  A6    1       F1(0)
	 *  A5    2       F1(1)'
	 *  A4    3       F1(2)'
	 *  A3    4       F1(3)'
	 *  A2    7       0 (GND)
	 *  A1    6       PC1O
	 *  A0    5       PC0O
	 *
	 *  output line   signal
	 *  ----------------------------
	 *  D0     12     PC1T
	 *  D1     11     PC1F
	 *  D2     10     PC0T
	 *  D3     9      PC0F
	 *
	 * The outputs are connected to a dual 4:1 demultiplexer 74S153, so that
	 * depending on NEXT01' and RESET the following signals are passed through:
	 *
	 *  RESET  NEXT[01]'  PC0I    PC1I
	 *  --------------------------------------
	 *  0      0          PC0T    PC1T
	 *  0      1          PC0F    PC1F
	 *  1      0          PC0I4   T14 (?)
	 *  1      1          -"-     -"-
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
	 * Data sheet 05a_AIM.pdf page 14
	 * inputs A0-A7 from R0-R7 (?)
	 * output   signal
	 * -------------------
	 * Q0       KP3
	 * Q1       KP4
	 * Q2       KP5
	 * Q3       unused
	 *
	 * I haven't found yet where KP3-KP5 are used
	 */
	UINT8* m_madr_a90;

	/**
	 * @brief unused PROM a91
	 * Data sheet 05a_AIM.pdf page 14
	 * inputs A0-A7 from R0-R7 (?)
	 *
	 * Output   Signal
	 * -------------------
	 * Q0       KP0
	 * Q1       KP1
	 * Q2       KP2
	 * Q3       unused
	 * KP0-KP3 are decoded using 7442 a78 to select
	 * the keyboard row enable
	 *
	 * Enable   Key[0]  Key[1]  Key[2]  Key[3]  Key[4]  Key[5]  Key[6]  Key[7]
	 * ------------------------------------------------------------------------
	 * KE(0)    KB(R)   KB(1)   KB(3)   KB(5)   KB(T)   KB(ESC)  KB(2)  KB(4)
	 * KE(1)    KB(G)   KB(TAB) KB(W)   KB(6)   KB(Y)   KB(F)    KB(0)  KB(E)
	 * KE(2)    KB(H)   KB(CTL) KB(S)   KB(7)   KB(8)   KB(C)    KB(A)  KB(D)
	 * KE(3)    KB(N)   KB(J)   KB(9)   KB(U)   KB(M)   KB(B)    KB(I)  KB(V)
	 * KE(4)    KB(LCK) KB(Z)   KB(X)   KB(Q)   KB(SPC) KB(^R)   KB(O)  KB(K)
	 * KE(5)    KB([)   KB(.)   KB(L)   KB(-)   KB(+)   KB(;)    KB(,)  KB(P)
	 * KE(6)    KB(^L)  KB(RTN) KB(")   KB(/)   KB(S3)  KB(<-)    KB(])  KB(\)
	 * KE(7)    KB(S1)  KB(DEL) KB(S2)  KB(LF)  KB(S4)  KB(S5)   KB(BW) KB(BS)
	 */
	UINT8* m_madr_a91;

	/**
	 * @brief ALU function to 74181 operation lookup PROM
	 */
	UINT8* m_alu_a10;

	//! output lines of the ALU a10 PROM
	enum {
		A10_UNUSED  = (1 << 0),
		A10_TSELECT = (1 << 1),
		A10_ALUCI   = (1 << 2),
		A10_ALUM    = (1 << 3),
		A10_ALUS0   = (1 << 4),
		A10_ALUS1   = (1 << 5),
		A10_ALUS2   = (1 << 6),
		A10_ALUS3   = (1 << 7),
		A10_ALUIN   = (A10_ALUM|A10_ALUCI|A10_ALUS0|A10_ALUS1|A10_ALUS2|A10_ALUS3)
	};

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

	bool m_ram_related[ALTO2_TASKS];                //!< set when task is RAM related

	UINT64 m_cycle;                                 //!< number of cycles executed in the current slice

	UINT64 cycle() { return m_cycle; }              //!< return the current CPU cycle
	UINT64 ntime() { return m_cycle*ALTO2_UCYCLE/1000; }    //!< return the current nano seconds

	void hard_reset();                              //!< reset the various registers
	void soft_reset();                              //!< soft reset

	void fn_bs_bad_0();                             //! bs dummy early function
	void fn_bs_bad_1();                             //! bs dummy late function

	void fn_f1_bad_0();                             //! f1 dummy early function
	void fn_f1_bad_1();                             //! f1 dummy late function

	void fn_f2_bad_0();                             //! f2 dummy early function
	void fn_f2_bad_1();                             //! f2 dummy late function

	DECLARE_READ16_MEMBER( noop_r );                //!< read open bus (0177777)
	DECLARE_WRITE16_MEMBER( noop_w );               //!< write open bus

	DECLARE_READ16_MEMBER( bank_reg_r );            //!< read bank register in memory mapped I/O range
	DECLARE_WRITE16_MEMBER( bank_reg_w );           //!< write bank register in memory mapped I/O range

	void bs_early_read_r();                         //!< bus source: drive bus by R register
	void bs_early_load_r();                         //!< bus source: load R places 0 on the BUS
	void bs_late_load_r();                          //!< bus source: load R from SHIFTER
	void bs_early_read_md();                        //!< bus source: drive BUS from read memory data
	void bs_early_mouse();                          //!< bus source: drive bus by mouse
	void bs_early_disp();                           //!< bus source: drive bus by displacement (which?)
	void f1_early_block();                          //!< F1 func: block active task
	void f1_late_load_mar();                        //!< F1 func: load memory address register
	void f1_early_task();                           //!< F1 func: task switch
	void f1_late_l_lsh_1();                         //!< F1 func: SHIFTER = left shift L once
	void f1_late_l_rsh_1();                         //!< F1 func: SHIFTER = right shift L once
	void f1_late_l_lcy_8();                         //!< F1 func: SHIFTER = byte swap L
	void f2_late_bus_eq_zero();                     //!< F2 func: branch on bus equals zero
	void f2_late_shifter_lt_zero();                 //!< F2 func: branch on shifter less than zero
	void f2_late_shifter_eq_zero();                 //!< F2 func: branch on shifter equals zero
	void f2_late_bus();                             //!< F2 func: branch on bus bits BUS[6-15]
	void f2_late_alucy();                           //!< F2 func: branch on latched ALU carry
	void f2_late_load_md();                         //!< F2 func: load memory data

#if USE_ALU_74181
	UINT32 alu_74181(UINT32 a, UINT32 b, UINT8 smc);
#endif
	void rdram();                                   //!< read the microcode ROM/RAM halfword
	void wrtram();                                  //!< write the microcode RAM from M register and ALU

	UINT8 m_ether_id;                               //!< configured Ethernet ID for this machine

//*******************************************
// inline the sub-devices
//*******************************************
#include "a2jkff.h"
#include "a2ram.h"
#include "a2hw.h"
#include "a2kbd.h"
#include "a2mouse.h"
#include "a2disk.h"
#include "a2disp.h"
#include "a2mem.h"
#include "a2emu.h"
#include "a2ksec.h"
#include "a2ether.h"
#include "a2mrt.h"
#include "a2dwt.h"
#include "a2curt.h"
#include "a2dht.h"
#include "a2dvt.h"
#include "a2part.h"
#include "a2dwt.h"
#include "a2kwd.h"
};

extern const device_type ALTO2;


#endif /* _CPU_ALTO2_H_ */
