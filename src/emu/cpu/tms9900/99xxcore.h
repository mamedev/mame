/*
    99xxcore.h : generic tms99xx emulation

    The TMS99XX_MODEL switch tells which emulator we want to build.  Set the
    switch, then include 99xxcore.h, and you will have an emulator for this
    processor.

    Only ti990/10, tms9900, tms9980a/9981, and tms9995 work OK for now.  Note
    that tms9995 has not been tested extensively.

    tms9940 is WIP: it is probably still buggy (particularily the BCD support),
    as it has not been tested.  tms9985 has been implemented as a 9940 with
    a data bus, which should be mostly correct.

    I think all software aspects of tms9985 and tms9989 are implemented.
    You just need to implement bus interfaces, provided you know them.
    (I don't...)

    tms99000 cannot be implemented fully yet, due to lack of documentation.

    ti990/12 is not supported at all, and it should probably be implemented as
    a separate processor core, anyway.

    Original tms9900 emulator by Edward Swartz
    Smoothed out by Raphael Nabet
    Originally converted for Mame by M.Coates
    Processor timing, support for tms9980 and tms9995, and many bug fixes by R Nabet
*/

/*
    The first member of the family was actually the ti990/10 minicomputer,
    released in 1975.  tms9900 was released in 1976, and has the same
    instruction set as ti990/10: however, tms9900 is slower, it does not
    support privileges and memory mapping, and illegal instructions do not
    cause an error interrupt.

    The ti990 family later evoluted into the huge ti990/12 system, with support
    for 144 different instructions, and microcode programming in case some user
    found it was not enough.  ti990/10 was eventually replaced by a cheaper
    ti990/10a board, built around a tms99000 microprocessor.

    The tms9980 processor is merely a tms9900 with a 8-bit data bus (instead of
    16-bit on tms9900).

    tms9940 is a microcontroller, and is mostly similar to 9900/9980.  The
    variant I know has 2kb of ROM, 128 bytes of RAM, a timer, 32 I/O line, some
    of which can be reconfigured as a CRU bus, but no external memory bus.  It
    includes three additional opcodes, which are not supported by any other
    member of the family (with the probable exception of TMS9985).

    tms9985 is similar to tms9940, but it supports an external 8-bit-wide
    memory bus.  At least one variant included 8kb of ROM, 256 bytes of RAM.
    It was ill-fated, as it was never released due to technical problems.

    tms9989 is mostly alien to me.  I guess it is a close relative of tms9995,
    although I am not sure.  I have read that the SBP68689 supports tms9995
    opcodes, but that tms9989 does not.

    tms9995 belongs to another generation.  It is quite faster than tms9900,
    and supports 4 extra opcodes.  Its external bus is 8-bit-wide, and it has
    256 bytes of internal 16-bit RAM.

    tms99000 is the successor to both ti9900 and ti990/10.  It supports
    privileges, and has a coprocessor interface which enables the use of an
    external memory mapper.  Additionnally,  it can use a Macrostore ROM to
    emulate additional instructions.

    This feature allowed TI to expand the 99000 family with the tms99105 (which
    was said to support 84 instructions types), the tms99110 (which supported
    floating point instructions), and possibly another chip (tms99220???) which
    included parts of the UCSD P-system in Macrostore.

References :
* 9900 family systems design, chapter 6, 7, 8
* TMS 9980A/ TMS 9981 Product Data Book
* TMS 9995 16-Bit Microcomputer Data Manual
* Model 990/10A computer general description, section 4 (p/n 2302633-9701 A)
* 990/99000 assembly language reference manual (p/n 2270509-9701 A)
* Chapter 18 of unidentified book is the only reference on tms9940 I have found so far (Paperport format)
    <ftp://ftp.whtech.com//datasheets/Hardware manuals/tms9900 9901 9904 9940 9980 (by a osborne).max>

Other references can be found on spies.com:
<http://www.spies.com/arcade/simulation/processors/index.html>
<http://www.spies.com/~aek/pdf/ti/>

*/


#include "debugger.h"
#include "deprecat.h"
#include "tms9900.h"


#if (TMS99XX_MODEL == TI990_10_ID)

	#define TMS99XX_PREFIX ti990_10
	#define TMS99XX_CPU_NAME "TI990/10"

#elif (TMS99XX_MODEL == TMS9900_ID)

	#define TMS99XX_PREFIX tms9900
	#define TMS99XX_CPU_NAME "TMS9900"

#elif (TMS99XX_MODEL == TMS9940_ID)

	#define TMS99XX_PREFIX tms9940
	#define TMS99XX_CPU_NAME "TMS9940"

	#error "tms9940 is not yet supported"

#elif (TMS99XX_MODEL == TMS9980_ID)

	#define TMS99XX_PREFIX tms9980a
	#define TMS99XX_CPU_NAME "TMS9980A/TMS9981"

#elif (TMS99XX_MODEL == TMS9985_ID)

	#define TMS99XX_PREFIX tms9985
	#define TMS99XX_CPU_NAME "TMS9985"

	#error "tms9985 is not yet supported"

#elif (TMS99XX_MODEL == TMS9989_ID)

	#define TMS99XX_PREFIX tms9989
	#define TMS99XX_CPU_NAME "TMS9989"

	#error "tms9989 is not yet supported"

#elif (TMS99XX_MODEL == TMS9995_ID)

	#define TMS99XX_PREFIX tms9995
	#define TMS99XX_CPU_NAME "TMS9995"

#elif (TMS99XX_MODEL == TMS99000_ID)

	#define TMS99XX_PREFIX tms99000
	#define TMS99XX_CPU_NAME "TMS99000"

	#error "tms99000 is not yet supported"

#elif (TMS99XX_MODEL == TMS99105A_ID)

	#define TMS99XX_PREFIX tms99105a
	#define TMS99XX_CPU_NAME "TMS99105A"

	#error "tms99105a is not yet supported"

#elif (TMS99XX_MODEL == TMS99110A_ID)

	#define TMS99XX_PREFIX tms99110a
	#define TMS99XX_CPU_NAME "TMS99110A"

	#error "tms99110a is not yet supported"

#endif

/*
    Now for some preprocessor wizardry.
*/
#define concat2(a,b) a##b

#define ICOUNT(prefix)				concat2(prefix,_ICount)
#define GET_INFO(prefix)			concat2(prefix,_get_info)
#define RESET_PARAM(prefix)			concat2(prefix,reset_param)

#define TMS99XX_ICOUNT				ICOUNT(TMS99XX_PREFIX)
#define TMS99XX_GET_INFO			GET_INFO(TMS99XX_PREFIX)
#define TMS99XX_RESET_PARAM			RESET_PARAM(TMS99XX_PREFIX)



/*
    I include this macro because we may eventually support other 99000 variants such as tms99110,
    and this macro will remain true for every 99000 family member, even when we have
    (TMS99XX_MODEL != TMS99000_ID).
*/
#define IS_99000 (TMS99XX_MODEL == TMS99000_ID)

/*
    On microprocessor implementations (other than TMS9940 and, probably, TMS9985), the CKOF, CKON,
    IDLE, LREX and RSET cause an external CRU write.  CKOF, CKON and LREX do nothing apart of this,
    therefore they must be implemented with external hardware (CKON and CKOF are supposed to
    enable/disable a line clock interrupt, and LREX to trigger a LOAD interrupt).  IDLE and RSET
    are functional, but, on the one hand, the design allowed to light a diagnostic LED when
    the processor is in IDLE state, and, on the other hand, the RSET instruction is supposed
    to reset external devices as well.

    On the TI990/10 and TI990/12 minicomputers, there is no such CRU write.  The line clock
    interrupt latch is part of the CPU board, LREX is fully functional, the IDLE led is connected
    to the board, and the RSET line is part of the TILINE bus connector.

    On the TMS9940, CKOF, CKON, LREX and RSET are not supported.  IDLE, on the other hand, is
    supported, and the CPU can be configured to output its IDLE state on the P16 I/O pin.
*/
#define EXTERNAL_INSTRUCTION_DECODING (TMS99XX_MODEL != TI990_10_ID) && (TMS99XX_MODEL != TI9940_ID) && (TMS99XX_MODEL != TI9985_ID)
#define EXTERNAL_INSTRUCTION_CALLBACK (TMS99XX_MODEL == TI990_10_ID)

/*
    ti990/10, ti990/12 and tms99000 support privileges

    privileged instructions:
    CKOF, CKON, IDLE, LIMI, LREX, RSET,
    LDD, LDS, LMF, (memory mapping instructions)
    DINT, EINT, EMD , LCS, LIM, SLSP. (990/12 instructions)

    instructions which are privileged when the effective CRU address is higher than 0xE00:
    LDCR, SBO, SBZ, STCR, TB.

    instructions whose behaviour is modified in user mode (only user flags in ST are affected):
    RTWP,
    LST. (ti990/12 and tms99000 instruction)
*/
#define HAS_PRIVILEGE ((TMS99XX_MODEL == TI990_10_ID) || IS_99000)

/*
    opcode groups

    * 69 basic opcodes implemented on all family members
    * 3 memory mapping opcodes implemented on ti990/10 with mapping option, ti990/12, and
      the tim99610 mapper in conjunction with any tms99000 family member: LMF, LDS, LDD
    * 3 opcodes implemented on tms9940 (and probably tms9985) only: DCA, DCS, LIIM
    * 4 opcodes implemented on ti990/12, tms9989 and above: MPYS, DIVS, LST, LWP
    * 1 opcode implemented on ti990/12, and tms99000: BIND
    * 72 opcodes implemented on ti990/12 only (some of which are emulated by tms99105 & tms99110)
*/

#define HAS_MAPPING ((TMS99XX_MODEL == TI990_10_ID) /*|| IS_99000*/)
#define HAS_9995_OPCODES ((TMS99XX_MODEL == TMS9989_ID) || (TMS99XX_MODEL == TMS9995_ID) || IS_99000)
#define HAS_BIND_OPCODE IS_99000

#define HAS_OVERFLOW_INTERRUPT ((TMS99XX_MODEL == TMS9995_ID) || IS_99000)

/*
    Under tms9900, opcodes >0200->03FF are incompletely decoded: bits 11 is ignored, and so are
    bits 12-15 for instructions which do not require a register.  On the other hand, ti990/10
    generates an illegal instruction error when bit 11 is set, but still ignores bits 12-15.
    Additionally, ti990/12 and tms9995 will generate an illegal error when bits 12-15 are
    non-zero.
*/
#define BETTER_0200_DECODING (TMS99XX_MODEL == TI990_10_ID)
#define COMPLETE_0200_DECODING (TMS99XX_MODEL >= TMS9995_ID)

/*
    TI990/10 and tms9900 force unused bits in the ST register to 0.
    TMS9995 does not.
*/
#define USE_ST_MASK (TMS99XX_MODEL <= TMS9985_ID)

/*
    TI990/10, TMS9900 and TMS99000 have a 16-bit-wide memory bus, and use 16-bus accessors.
    TMS9940 use 16-bit accessors, too, as it has an internal 16-bit bus, and no external bus.
*/
#define USE_16_BIT_ACCESSORS ((TMS99XX_MODEL == TI990_10_ID) || (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9940_ID) || (IS_99000))


INLINE void execute(UINT16 opcode);

#if EXTERNAL_INSTRUCTION_DECODING
static void external_instruction_notify(int ext_op_ID);
#endif
static UINT16 decipheraddr(UINT16 opcode);
static UINT16 decipheraddrbyte(UINT16 opcode);
static void contextswitch(UINT16 addr);
#if HAS_MAPPING || HAS_PRIVILEGE
static void contextswitchX(UINT16 addr);
#else
#define contextswitchX(addr) contextswitch(addr)
#endif
static void field_interrupt(void);

/***************************/
/* Mame Interface Routines */
/***************************/


static int TMS99XX_ICOUNT = 0;


/* tms9900 ST register bits. */

/* These bits are set by every compare, move and arithmetic or logical operation : */
/* (Well, COC, CZC and TB only set the E bit, but these are kind of exceptions.) */
#define ST_LGT 0x8000 /* Logical Greater Than (strictly) */
#define ST_AGT 0x4000 /* Arithmetical Greater Than (strictly) */
#define ST_EQ  0x2000 /* Equal */

/* These bits are set by arithmetic operations, when it makes sense to update them. */
#define ST_C   0x1000 /* Carry */
#define ST_OV  0x0800 /* OVerflow (overflow with operations on signed integers, */
                      /* and when the result of a 32bits:16bits division cannot fit in a 16-bit word.) */

/* This bit is set by move and arithmetic operations WHEN THEY USE BYTE OPERANDS. */
#define ST_OP  0x0400 /* Odd Parity */

#if (TMS99XX_MODEL != TMS9940_ID)

/* This bit is set by the XOP instruction. */
#define ST_X   0x0200 /* Xop */

#endif

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)

/* This bit is set by arithmetic operations to support BCD */
#define ST_DC  0x0100 /* Digit Carry */

#endif

#if HAS_PRIVILEGE

/* This bit is set in user (i.e. non-supervisor) mode */
#define ST_PR  0x0100 /* PRivilege */

#endif

#if HAS_MAPPING

/* This tells which map is currently in use */
#define ST_MF  0x0080 /* Map File */

#endif

#if (HAS_OVERFLOW_INTERRUPT)

/* This bit is set in ti990/12, TMS9995 and later chips to generate a level-2 interrupt when
the Overflow status bit is set */
#define ST_OVIE 0x0020 /* OVerflow Interrupt Enable */

#endif

#if (IS_99000)

/* This bit enables the macrostore feature on tms99000 */
/* (It is used on ti990/12 with a different meaning.) */
#define ST_EMR 0x0010 /* Enable Macrostore Rom */

#endif

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)

#define ST_IM 0x0003 /* Interrupt Mask */

#else

#define ST_IM 0x000F /* Interrupt Mask */

#endif

/* On models before TMS9995 (TMS9989 ?), unused ST bits are always forced to 0, so we define
a ST_MASK */
#if (USE_ST_MASK)
	#if (TMS99XX_MODEL == TI990_10_ID)
		#define ST_MASK 0xFF8F
	#elif (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
#define ST_MASK 0xFD03
	#else
		#define ST_MASK 0xFE0F
	#endif
#endif


/* error interrupt register bits */

#if (TMS99XX_MODEL == TI990_10_ID)

/* This bit is set in user (i.e. non-supervisor) mode */
#define EIR_MAPERR	0x0800	/* memory MAPping ERRor */
#define EIR_MER		0x1000	/* Memory parity ERRor */
#define EIR_ILLOP	0x2000	/* ILLegal OPcode */
#define EIR_PRIVOP	0x4000	/* PRiviledged OPeration */
#define EIR_TIMEOUT	0x8000	/* TILINE TIMEOUT */

#endif


/* Offsets for registers. */
#define R0   0
#define R1   2
#define R2   4
#define R3   6
#define R4   8
#define R5  10
#define R6  12
#define R7  14
#define R8  16
#define R9  18
#define R10 20
#define R11 22
#define R12 24
#define R13 26
#define R14 28
#define R15 30

typedef struct map_file_t
{
	UINT16 L[3], B[3];			/* actual registers */
	UINT32 limit[3], bias[3];	/* equivalent in a more convenient form */
} map_file_t;

typedef struct
{
/* "actual" tms9900 registers : */
	UINT16 WP;  /* Workspace pointer */
	UINT16 PC;  /* Program counter */
	UINT16 STATUS;  /* STatus register */

/* Now, data used for emulation */
	UINT16 IR;  /* Instruction register, with the currently parsed opcode */

	UINT8 interrupt_pending;  /* true if an interrupt must be honored... */

#if ! ((TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID))
	UINT8 load_state; /* nonzero if the LOAD* line is active (low) */
#endif

#if (TMS99XX_MODEL == TI990_10_ID) || (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID)
	/* On tms9900, we cache the state of INTREQ* and IC0-IC3 here */
	/* On tms9980/9981, we translate the state of IC0-IC2 to the equivalent state for a tms9900,
    and store the result here */
	UINT8 irq_level;	/* when INTREQ* is active, interrupt level on IC0-IC3 ; else always 16 */
	UINT8 irq_state;	/* nonzero if the INTREQ* line is active (low) */
					/* with TMS9940, bit 0 means INT1, bit 1 decrementer, bit 2 INT2 */
#elif (TMS99XX_MODEL == TMS9995_ID)
	/* tms9995 is quite different : it latches the interrupt inputs */
	UINT8 irq_level;    /* We store the level of the request with the highest level here */
	UINT8 int_state;    /* interrupt lines state */
	UINT8 int_latch;	  /* interrupt latches state */
#endif

	/* interrupt callback */
	/* note that this callback is used by tms9900_set_irq_line() and tms9980a_set_irq_line() to
    retreive the value on IC0-IC3 (non-standard behaviour) */
	int (*irq_callback)(int irq_line);

	UINT8 IDLE;       /* nonzero if processor is IDLE - i.e waiting for interrupt while writing
                        special data on CRU bus */

#if HAS_MAPPING
	UINT8 mapping_on;			/* set by a CRU write */
	map_file_t map_files[3];	/* internal mapper registers */
	UINT8 cur_map;			/* equivalent to ST_MF status bit */
	UINT8 cur_src_map;		/* set to 2 by LDS */
	UINT8 cur_dst_map;		/* set to 2 by LDD */

#if (TMS99XX_MODEL == TI990_10_ID)
	UINT8 reset_maperr;		/* reset mapper error flag line (reset flags in 945417-9701 p. 3-90) */

	UINT32 mapper_address_latch;	/* used to load the map file and for diagnostic purpose */
	UINT16 mapper_cru_read_register;	/* read register select code for mapper cru interface */
	UINT8 diaglat;					/* set when diagnostic address latch is done */
	UINT8 latch_control[3];			/* latch control */

#endif
#endif

#if (TMS99XX_MODEL == TI990_10_ID)
	UINT16 error_interrupt_register;	/* one flag for each interrupt condition */
	void (*error_interrupt_callback)(int state);
#endif

#if (TMS99XX_MODEL == TMS9985_ID) || (TMS99XX_MODEL == TMS9995_ID)
	UINT8 RAM[256]; /* on-chip RAM (I know this is weird, but the internal bus is 16-bit-wide, whereas the external bus is 8-bit-wide) */
#endif

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID) || (TMS99XX_MODEL == TMS9995_ID)
	/* on-chip event counter/timer*/
	UINT8 decrementer_enabled;
	UINT16 decrementer_interval;
	UINT16 decrementer_count;	/* used in event counter mode*/
	emu_timer *timer;  /* used in timer mode */
#endif

#if (TMS99XX_MODEL == TMS9995_ID)
	/* additionnal registers */
	UINT16 flag; 	  /* flag register */
	UINT8 MID_flag;   /* MID flag register */

	/* chip config, which can be set on reset */
	UINT8 memory_wait_states_byte;
	UINT8 memory_wait_states_word;

	/* mask option (off on normal tms9995) */
	UINT8 is_mp9537;
#endif

	/* Some instructions (i.e. XOP, BLWP, and MID) disable interrupt recognition until another
    instruction is executed : so they set this flag */
	UINT8 disable_interrupt_recognition;

	/* notify the driver of changes in IDLE state */
	void (*idle_callback)(int state);

#if EXTERNAL_INSTRUCTION_CALLBACK
	void (*rset_callback)(void);
	void (*lrex_callback)(void);
	void (*ckon_ckof_callback)(int state);
#endif

}	tms99xx_Regs;

static tms99xx_Regs I;
static UINT8 lastparity;  /* rather than handling ST_OP directly, we copy the last value which
                                  would set it here */

static char lds_flag, ldd_flag;

#if (TMS99XX_MODEL == TMS9995_ID)
static void reset_decrementer(void);
#endif


#if (TMS99XX_MODEL == TI990_10_ID)

/*
    accessor for the internal ROM
*/
READ16_HANDLER(ti990_10_internal_r)
{
	//return I.ROM[offset];
	return program_read_word_16be(0x1ffc00+offset);
}

#endif

#if (TMS99XX_MODEL == TMS9995_ID)

/*
    accessor for the first 252 bytes of internal RAM
*/
READ8_HANDLER(tms9995_internal1_r)
{
	return I.RAM[offset];
}

WRITE8_HANDLER(tms9995_internal1_w)
{
	I.RAM[offset]=data;
}

/*
    accessor for the last 4 bytes of internal RAM
*/
READ8_HANDLER(tms9995_internal2_r)
{
	return I.RAM[offset+0xfc];
}

WRITE8_HANDLER(tms9995_internal2_w)
{
	I.RAM[offset+0xfc]=data;
}

#endif

#if (TMS99XX_MODEL == TI990_10_ID)

	/* on-board ROMs are not emulated (we use a hack) */

	static int write_inhibit;

	#define readword(addr) readwordX((addr), I.cur_map)
	static int readwordX(int addr, int map_file)
	{
		if ((map_file == 0) && (addr >= 0xf800))
		{	/* intercept TPCS and CPU ROM */
			if (addr < 0xfc00)
				/* TPCS */
				return program_read_word_16be(0x1f0000+addr);
			else
				/* CPU ROM */
				return program_read_word_16be(0x1f0000+addr);	/* hack... */
		}
		else if (! I.mapping_on)
		{
			return program_read_word_16be(addr);
		}
		else
		{
			int map_index;

			if (addr <= I.map_files[map_file].limit[0])
				map_index = 0;
			else if (addr <= I.map_files[map_file].limit[1])
				map_index = 1;
			else if (addr <= I.map_files[map_file].limit[2])
				map_index = 2;
			else
			{	/* mapping error */
				if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
					I.mapper_address_latch = addr;
				if ((! I.reset_maperr) && ! (I.error_interrupt_register & EIR_MAPERR))
				{
					I.error_interrupt_register |= EIR_MAPERR;
					write_inhibit = 1;
				}
				return program_read_word_16be(addr);
			}
			if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
				I.mapper_address_latch = I.map_files[map_file].bias[map_index]+addr;
			if ((I.latch_control[map_index]) && (! I.reset_maperr))
				I.diaglat = 1;
			return program_read_word_16be(I.map_files[map_file].bias[map_index]+addr);
		}
	}

	#define writeword(addr, data) writewordX((addr), (data), I.cur_map)
	static void writewordX(int addr, int data, int map_file)
	{
		if ((map_file == 0) && (addr >= 0xf800))
		{	/* intercept TPCS and CPU ROM */
			if (addr < 0xfc00)
				/* TPCS */
				program_write_word_16be(0x1f0000+addr, data);
			else
				/* CPU ROM */
				program_write_word_16be(0x1f0000+addr, data);	/* hack... */
		}
		else if (! I.mapping_on)
		{
			program_write_word_16be(addr, data);
		}
		else
		{
			int map_index;

			if (addr <= I.map_files[map_file].limit[0])
				map_index = 0;
			else if (addr <= I.map_files[map_file].limit[1])
				map_index = 1;
			else if (addr <= I.map_files[map_file].limit[2])
				map_index = 2;
			else
			{	/* mapping error */
				if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
					I.mapper_address_latch = addr;
				if ((! I.reset_maperr) && ! (I.error_interrupt_register & EIR_MAPERR))
				{
					I.error_interrupt_register |= EIR_MAPERR;
					write_inhibit = 1;
				}
				if (write_inhibit)
					(void)program_read_word_16be(addr);
				else
					program_write_word_16be(addr, data);
				return;
			}
			if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
				I.mapper_address_latch = I.map_files[map_file].bias[map_index]+addr;
			if ((I.latch_control[map_index]) && (! I.reset_maperr))
				I.diaglat = 1;
			program_write_word_16be(I.map_files[map_file].bias[map_index]+addr, data);
		}
	}

	#define readbyte(addr) readbyteX((addr), I.cur_map)
	static int readbyteX(int addr, int map_file)
	{
		if ((map_file == 0) && (addr >= 0xf800))
		{	/* intercept TPCS and CPU ROM */
			if (addr < 0xfc00)
				/* TPCS */
				return program_read_byte_16be(0x1f0000+addr);
			else
				/* CPU ROM */
				return program_read_byte_16be(0x1f0000+addr);	/* hack... */
		}
		else if (! I.mapping_on)
		{
			return program_read_byte_16be(addr);
		}
		else
		{
			int map_index;

			if (addr <= I.map_files[map_file].limit[0])
				map_index = 0;
			else if (addr <= I.map_files[map_file].limit[1])
				map_index = 1;
			else if (addr <= I.map_files[map_file].limit[2])
				map_index = 2;
			else
			{	/* mapping error */
				if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
					I.mapper_address_latch = addr;
				if ((! I.reset_maperr) && ! (I.error_interrupt_register & EIR_MAPERR))
				{
					I.error_interrupt_register |= EIR_MAPERR;
					write_inhibit = 1;
				}
				return program_read_byte_16be(addr);
			}
			if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
				I.mapper_address_latch = I.map_files[map_file].bias[map_index]+addr;
			if ((I.latch_control[map_index]) && (! I.reset_maperr))
				I.diaglat = 1;
			return program_read_byte_16be(I.map_files[map_file].bias[map_index]+addr);
		}
	}

	#define writebyte(addr, data) writebyteX((addr), (data), I.cur_map)
	static void writebyteX(int addr, int data, int map_file)
	{
		if ((map_file == 0) && (addr >= 0xf800))
		{	/* intercept TPCS and CPU ROM */
			if (addr < 0xfc00)
				/* TPCS */
				program_write_byte_16be(0x1f0000+addr, data);
			else
				/* CPU ROM */
				program_write_byte_16be(0x1f0000+addr, data);	/* hack... */
		}
		else if (! I.mapping_on)
		{
			program_write_byte_16be(addr, data);
		}
		else
		{
			int map_index;

			if (addr <= I.map_files[map_file].limit[0])
				map_index = 0;
			else if (addr <= I.map_files[map_file].limit[1])
				map_index = 1;
			else if (addr <= I.map_files[map_file].limit[2])
				map_index = 2;
			else
			{	/* mapping error */
				if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
					I.mapper_address_latch = addr;
				if ((! I.reset_maperr) && ! (I.error_interrupt_register & EIR_MAPERR))
				{
					I.error_interrupt_register |= EIR_MAPERR;
					write_inhibit = 1;
				}
				if (write_inhibit)
					(void)program_read_byte_16be(addr);
				else
					program_write_byte_16be(addr, data);
				return;
			}
			if ((! (I.error_interrupt_register & EIR_MAPERR)) && ! (I.diaglat))
				I.mapper_address_latch = I.map_files[map_file].bias[map_index]+addr;
			if ((I.latch_control[map_index]) && (! I.reset_maperr))
				I.diaglat = 1;
			program_write_byte_16be(I.map_files[map_file].bias[map_index]+addr, data);
		}
	}

#elif (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9940_ID)
	/*16-bit data bus, 16-bit address bus (internal bus in the case of TMS9940)*/
	/*Note that tms9900 actually never accesses a single byte : when performing byte operations,
    it reads a 16-bit word, changes the revelant byte, then write a complete word.  You should
    remember this when writing memory handlers.*/
	/*This does not apply to tms9995 and tms99xxx, but does apply to tms9980 (see below).*/

	#define readword(addr)        program_read_word_16be(addr)
	#define writeword(addr,data)  program_write_word_16be((addr), (data))

	#define readbyte(addr)        program_read_byte_16be(addr)
	#define writebyte(addr,data)  program_write_byte_16be((addr),(data))

#elif (TMS99XX_MODEL == TMS9980_ID)
	/*8-bit data bus, 14-bit address*/
	/*Note that tms9980 never accesses a single byte (however crazy it may seem).  Although this
    makes memory access slower, I have emulated this feature, because if I did otherwise,
    there would be some implementation problems in some driver sooner or later.*/

	INLINE int readword(int addr)
	{
		int val;

		TMS99XX_ICOUNT -= 2;
		val = program_read_byte_8be(addr);
		return (val << 8) | program_read_byte_8be(addr+1);
	}
	#define writeword(addr,data)  { TMS99XX_ICOUNT -= 2; program_write_byte_8be((addr), (data) >> 8); program_write_byte_8be((addr) + 1, (data) & 0xff); }

#if 0
	#define readbyte(addr)        (TMS99XX_ICOUNT -= 2, program_read_byte_8be(addr))
	#define writebyte(addr,data)  { TMS99XX_ICOUNT -= 2; program_write_byte_8be((addr),(data)); }
#else
	/*This is how it really works*/
	/*Note that every writebyte must match a readbyte (which is indeed the case)*/
	static int extra_byte;	/* buffer holding the unused byte in a word read */

	static int readbyte(int addr)
	{
		TMS99XX_ICOUNT -= 2;
		if (addr & 1)
		{
			extra_byte = program_read_byte_8be(addr-1);
			return program_read_byte_8be(addr);
		}
		else
		{
			int val = program_read_byte_8be(addr);
			extra_byte = program_read_byte_8be(addr+1);
			return val;
		}
	}
	static void writebyte (int addr, int data)
	{
		TMS99XX_ICOUNT -= 2;
		if (addr & 1)
		{
			program_write_byte_8be(addr-1, extra_byte);
			program_write_byte_8be(addr, data);
		}
		else
		{
			program_write_byte_8be(addr, data);
			program_write_byte_8be(addr+1, extra_byte);
		}
	}
#endif

#elif (TMS99XX_MODEL == TMS9985_ID)
	/*Note that every writebyte must match a readbyte (which is indeed the case)*/
	static int extra_byte;	/* buffer holding the unused byte in a word read */

	static int readword(int addr)
	{
		if (addr < 0x2000)
		{
		}
		else if ((addr >= 0x8300) && (addr < 0x8400))
		{
		}
		else
		{
			TMS99XX_ICOUNT -= 2;
			return (program_read_byte_8be(addr) << 8) + program_read_byte_8be(addr + 1);
		}
	}
	static void writeword(int addr, int data)
	{
		if ((addr >= 0x8300) && (addr < 0x8400))
		{
		}
		else if (!(addr < 0x2000))
		{
			TMS99XX_ICOUNT -= 2;
			program_write_byte_8be(addr, data >> 8);
			program_write_byte_8be(addr + 1, data & 0xff);
		}
	}

	static int readbyte(int addr)
	{
		if (addr < 0x2000)
		{
		}
		else if ((addr >= 0x8300) && (addr < 0x8400))
		{
		}
		else
		{
			TMS99XX_ICOUNT -= 2;
			if (addr & 1)
			{
				extra_byte = program_read_byte_8be(addr-1);
				return program_read_byte_8be(addr);
			}
			else
			{
				int val = program_read_byte_8be(addr);
				extra_byte = program_read_byte_8be(addr+1);
				return val;
			}
		}
	}
	static void writebyte(int addr, int data)
	{
		if ((addr >= 0x8300) && (addr < 0x8400))
		{
		}
		else if (!(addr < 0x2000))
		{
			TMS99XX_ICOUNT -= 2;
			if (addr & 1)
			{
				program_write_byte_8be(addr-1, extra_byte);
				program_write_byte_8be(addr, data);
			}
			else
			{
				program_write_byte_8be(addr, data);
				program_write_byte_8be(addr+1, extra_byte);
		}
	}
	}

#elif (TMS99XX_MODEL == TMS9995_ID)
	/*8-bit external data bus, with on-chip 16-bit RAM, and 16-bit address bus*/
	/*The code is complex, so we use functions rather than macros*/

	static TIMER_CALLBACK( decrementer_callback );

	static int readword(int addr)
	{
		if ((addr < 0xf000) || (I.is_mp9537))
		{
			int reply;
			TMS99XX_ICOUNT -= I.memory_wait_states_word;
			reply = program_read_byte_8be(addr);
			return (reply << 8) | program_read_byte_8be(addr + 1);
		}
		else if (addr < 0xf0fc)
		{
			return *(UINT16 *)(& I.RAM[addr - 0xf000]);
		}
		else if (addr < 0xfffa)
		{
			int reply;
			TMS99XX_ICOUNT -= I.memory_wait_states_word;
			reply = program_read_byte_8be(addr);
			return (reply << 8) | program_read_byte_8be(addr + 1);
		}
		else if (addr < 0xfffc)
		{
			/* read decrementer */
			if (I.decrementer_enabled && !(I.flag & 1))
				/* timer mode, timer enabled */
				return ceil(ATTOTIME_TO_CYCLES(cpu_getactivecpu(), attotime_div(timer_timeleft(I.timer), 16)));
			else
				/* event counter mode or timer mode, timer disabled */
				return I.decrementer_count;
		}
		else
		{
			return *(UINT16 *)(& I.RAM[addr - 0xff00]);
		}
	}

	static void writeword (int addr, int data)
	{
		if ((addr < 0xf000) || (I.is_mp9537))
		{
			TMS99XX_ICOUNT -= I.memory_wait_states_word;
			program_write_byte_8be(addr, data >> 8);
			program_write_byte_8be(addr + 1, data & 0xff);
		}
		else if (addr < 0xf0fc)
		{
			*(UINT16 *)(& I.RAM[addr - 0xf000]) = data;
		}
		else if (addr < 0xfffa)
		{
			TMS99XX_ICOUNT -= I.memory_wait_states_word;
			program_write_byte_8be(addr, data >> 8);
			program_write_byte_8be(addr + 1, data & 0xff);
		}
		else if (addr < 0xfffc)
		{
			/* write decrementer */
			I.decrementer_interval = data;
			reset_decrementer();
		}
		else
		{
			*(UINT16 *)(& I.RAM[addr - 0xff00]) = data;
		}
	}

	static int readbyte(int addr)
	{
		if ((addr < 0xf000) || (I.is_mp9537))
		{
			TMS99XX_ICOUNT -= I.memory_wait_states_byte;
			return program_read_byte_8be(addr);
		}
		else if (addr < 0xf0fc)
		{
			return I.RAM[BYTE_XOR_BE(addr - 0xf000)];
		}
		else if (addr < 0xfffa)
		{
			TMS99XX_ICOUNT -= I.memory_wait_states_byte;
			return program_read_byte_8be(addr);
		}
		else if (addr < 0xfffc)
		{
			/* read decrementer */
			int value;

			if (I.decrementer_enabled && !(I.flag & 1))
				/* timer mode, timer enabled */
				value = ceil(ATTOTIME_TO_CYCLES(cpu_getactivecpu(), attotime_div(timer_timeleft(I.timer), 16)));
			else
				/* event counter mode or timer mode, timer disabled */
				value = I.decrementer_count;

			if (addr & 1)
				return (value & 0xFF);
			else
				return (value >> 8);
		}
		else
		{
			return I.RAM[BYTE_XOR_BE(addr - 0xff00)];
		}
	}

	static void writebyte (int addr, int data)
	{
		if ((addr < 0xf000) || (I.is_mp9537))
		{
			TMS99XX_ICOUNT -= I.memory_wait_states_byte;
			program_write_byte_8be(addr, data);
		}
		else if (addr < 0xf0fc)
		{
			I.RAM[BYTE_XOR_BE(addr - 0xf000)] = data;
		}
		else if (addr < 0xfffa)
		{
			TMS99XX_ICOUNT -= I.memory_wait_states_byte;
			program_write_byte_8be(addr, data);
		}
		else if (addr < 0xfffc)
		{
			/* write decrementer */
			/* Note that a byte write to tms9995 timer messes everything up. */
			I.decrementer_interval = (data << 8) | data;
			reset_decrementer();
		}
		else
		{
			I.RAM[BYTE_XOR_BE(addr - 0xff00)] = data;
		}
	}

#else

	#error "memory access not implemented"

#endif

#if !HAS_MAPPING
	#define readwordX(addr, map_file) readword(addr)
	#define writewordX(addr, data, map_file) writeword((addr), (data))
	#define readbyteX(addr, map_file) readbyte(addr)
	#define writebyteX(addr, data, map_file) writebyte((addr), (data))
#endif

#define READREG(reg)         readword((I.WP+(reg)) & 0xffff)
#define WRITEREG(reg, data)  writeword((I.WP+(reg)) & 0xffff, (data))

INLINE UINT16 READREG_DEBUG(int reg)
{
	int temp = TMS99XX_ICOUNT;
	UINT16 result = READREG(reg);
	TMS99XX_ICOUNT = temp;
	return result;
}

INLINE void WRITEREG_DEBUG(int reg, UINT16 data)
{
	int temp = TMS99XX_ICOUNT;
	WRITEREG(reg, data);
	TMS99XX_ICOUNT = temp;
}

#if (TMS99XX_MODEL == TI990_10_ID)
	READ8_HANDLER(ti990_10_mapper_cru_r)
	{
		int reply = 0;

		switch(I.mapper_cru_read_register)
		{
		case 0xb000:
			reply = I.map_files[I.cur_map].B[0];
			break;
		case 0xb001:
			reply = I.map_files[I.cur_map].B[1];
			break;
		case 0xb010:
			reply = I.map_files[I.cur_map].B[2];
			break;
		case 0xb011:
			reply = I.map_files[I.cur_map].L[0];
			break;
		case 0xb100:
			reply = I.map_files[I.cur_map].L[1];
			break;
		case 0xb101:
			reply = I.map_files[I.cur_map].L[2];
			break;
		case 0xb110:
			reply = I.mapper_address_latch;
			break;
		case 0xb111:
			reply = I.mapper_address_latch >> 16;
			break;
		}

		if (offset)
			return (reply >> 8) & 0xff;
		else
			return reply & 0xff;
	}

	WRITE8_HANDLER(ti990_10_mapper_cru_w)
	{
		switch (offset)
		{
		case 0:
		case 1:
		case 2:
			/* read register select */
			if (data)
				I.mapper_cru_read_register |= (1 << offset);
			else
				I.mapper_cru_read_register &= ~ (1 << offset);
			break;
		case 3:
			/* enable mapping */
			I.mapping_on = data;
			break;
		case 4:
			/* reset flags */
			I.reset_maperr = data;
			if (data)
			{
				I.error_interrupt_register &= ~ EIR_MAPERR;
				I.diaglat = 0;
			}
			break;
		case 5:
		case 6:
		case 7:
			/* latch control */
			I.latch_control[7-offset] = data;
			break;
		}
		}

	INLINE void handle_error_interrupt(void)
		{
		if (I.error_interrupt_callback)
			(*I.error_interrupt_callback)(I.error_interrupt_register ? 1 : 0);
		}

	READ8_HANDLER(ti990_10_eir_cru_r)
		{
		return (offset == 1) ? (I.error_interrupt_register & 0xff) : 0;
		}

	WRITE8_HANDLER(ti990_10_eir_cru_w)
		{
		if (offset < 4)	/* does not work for EIR_MAPERR */
		{
			I.error_interrupt_register &= ~ (1 << offset);

			handle_error_interrupt();
		}
	}


#endif

/* Interrupt mask */
#define IMASK       (I.STATUS & ST_IM)

/*
    CYCLES macro : you provide timings for tms9900 and tms9995, and the macro chooses for you.

    BTW, I have no idea what the timings are for tms9989 and tms99xxx...
*/
#if TMS99XX_MODEL == TI990_10_ID
	/* Use TI990/10 timings*/
	#define CYCLES(a,b,c) TMS99XX_ICOUNT -= a
#elif TMS99XX_MODEL <= TMS9985_ID
	/* Use TMS9900/TMS9980 timings*/
	#define CYCLES(a,b,c) TMS99XX_ICOUNT -= b
#else
	/* Use TMS9995 timings*/
	#define CYCLES(a,b,c) TMS99XX_ICOUNT -= c*4
#endif

#if (TMS99XX_MODEL == TMS9995_ID)

static void set_flag0(int val);
static void set_flag1(int val);

#endif

/************************************************************************
 * Status register functions
 ************************************************************************/
#include "99xxstat.h"

/**************************************************************************/

static void register_for_save_state(int index)
{
	state_save_register_item("tms99xx", index, I.WP);
	state_save_register_item("tms99xx", index, I.PC);
	state_save_register_item("tms99xx", index, I.STATUS);
	state_save_register_item("tms99xx", index, I.interrupt_pending);

#if ! ((TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID))
	state_save_register_item("tms99xx", index, I.load_state);
#endif

#if (TMS99XX_MODEL == TI990_10_ID) || (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID)
	state_save_register_item("tms99xx", index, I.irq_level);
	state_save_register_item("tms99xx", index, I.irq_state);
#elif (TMS99XX_MODEL == TMS9995_ID)
	state_save_register_item("tms99xx", index, I.irq_level);
	state_save_register_item("tms99xx", index, I.int_state);
	state_save_register_item("tms99xx", index, I.int_latch);
#endif

	state_save_register_item("tms99xx", index, I.IDLE);

#if HAS_MAPPING
	state_save_register_item("tms99xx", index, I.mapping_on);
	state_save_register_item_array("tms99xx", index, I.map_files[0].L);
	state_save_register_item_array("tms99xx", index, I.map_files[0].B);
	state_save_register_item_array("tms99xx", index, I.map_files[0].limit);
	state_save_register_item_array("tms99xx", index, I.map_files[0].bias);
	state_save_register_item_array("tms99xx", index, I.map_files[1].L);
	state_save_register_item_array("tms99xx", index, I.map_files[1].B);
	state_save_register_item_array("tms99xx", index, I.map_files[1].limit);
	state_save_register_item_array("tms99xx", index, I.map_files[1].bias);
	state_save_register_item_array("tms99xx", index, I.map_files[2].L);
	state_save_register_item_array("tms99xx", index, I.map_files[2].B);
	state_save_register_item_array("tms99xx", index, I.map_files[2].limit);
	state_save_register_item_array("tms99xx", index, I.map_files[2].bias);
	state_save_register_item("tms99xx", index, I.cur_map);
	state_save_register_item("tms99xx", index, I.cur_src_map);
	state_save_register_item("tms99xx", index, I.cur_dst_map);

#if (TMS99XX_MODEL == TI990_10_ID)
	state_save_register_item("tms99xx", index, I.reset_maperr);
	state_save_register_item("tms99xx", index, I.mapper_address_latch);
	state_save_register_item("tms99xx", index, I.mapper_cru_read_register);
	state_save_register_item("tms99xx", index, I.diaglat);
	state_save_register_item_array("tms99xx", index, I.latch_control);
#endif
#endif

#if (TMS99XX_MODEL == TI990_10_ID)
	state_save_register_item("tms99xx", index, I.error_interrupt_register);
#endif

#if (TMS99XX_MODEL == TMS9985_ID) || (TMS99XX_MODEL == TMS9995_ID)
	state_save_register_item_array("tms99xx", index, I.RAM);
#endif

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID) || (TMS99XX_MODEL == TMS9995_ID)
	state_save_register_item("tms99xx", index, I.decrementer_enabled);
	state_save_register_item("tms99xx", index, I.decrementer_interval);
	state_save_register_item("tms99xx", index, I.decrementer_count);
#endif

#if (TMS99XX_MODEL == TMS9995_ID)
	state_save_register_item("tms99xx", index, I.flag);
	state_save_register_item("tms99xx", index, I.MID_flag);
	state_save_register_item("tms99xx", index, I.memory_wait_states_byte);
	state_save_register_item("tms99xx", index, I.memory_wait_states_word);
	state_save_register_item("tms99xx", index, I.is_mp9537);
#endif

	state_save_register_item("tms99xx", index, I.disable_interrupt_recognition);
}


static void tms99xx_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const TMS99XX_RESET_PARAM *param = (const TMS99XX_RESET_PARAM *) config;

	register_for_save_state(index);

	I.irq_level = 16;
	I.irq_callback = irqcallback;

#if (TMS99XX_MODEL == TMS9995_ID)
	I.timer = timer_alloc(decrementer_callback, NULL);
#endif

	I.idle_callback = param ? param->idle_callback : NULL;
	#if (TMS99XX_MODEL == TI990_10_ID)
		I.rset_callback = param ? param->rset_callback : NULL;
		I.lrex_callback = param ? param->lrex_callback : NULL;
		I.ckon_ckof_callback = param ? param->ckon_ckof_callback : NULL;

		I.error_interrupt_callback = param ? param->error_interrupt_callback : NULL;
	#endif

#if (TMS99XX_MODEL == TMS9995_ID)
	/* we can ask at reset time that the CPU always generates one wait state automatically */
	if (param == NULL)
	{	/* if no param, the default is currently "wait state added" */
		I.memory_wait_states_byte = 4;
		I.memory_wait_states_word = 12;
			I.is_mp9537 = 0;
	}
	else
	{
			I.memory_wait_states_byte = (param->auto_wait_state) ? 4 : 0;
			I.memory_wait_states_word = (param->auto_wait_state) ? 12 : 4;
			I.is_mp9537 = param->is_mp9537;
	}
#endif

}

/*
    TMS9900 hard reset
*/
static void tms99xx_reset(void)
{
	I.STATUS = 0; /* TMS9980 and TMS9995 Data Books say so */
	getstat();

	#if HAS_MAPPING
		I.mapping_on = 0;
{
			int i,j;

			for (i=0; i<3; i++)
				for (j=0; j<3; j++)
				{
					I.map_files[i].L[j] = 0;
					I.map_files[i].limit[j] = 0xffff;
					I.map_files[i].B[j] = 0;
					I.map_files[i].bias[j] = 0;
				}
		}
		I.cur_map = 0;			/* equivalent to ST_MF status bit */
		I.cur_src_map = 0;		/* set to 2 by LDS */
		I.cur_dst_map = 0;		/* set to 2 by LDD */

		I.reset_maperr = 0;
	#endif

	if (I.IDLE)
	{
	I.IDLE = 0;   /* clear IDLE condition */
		if (I.idle_callback)
			(*I.idle_callback)(0);
	}

#if (TMS99XX_MODEL == TMS9995_ID)
	I.MID_flag = 0;

	/* Clear flag bits 0 & 1 */
	set_flag0(0);
	set_flag1(0);

		/* Clear internal interrupt latches */
	I.int_latch = 0;
	I.flag &= 0xFFE3;
#endif

	/* The ST register and interrupt latches changed, didn't they ? */
	field_interrupt();

	contextswitchX(0x0000);

	CYCLES(6, 26, 14);
}

static void tms99xx_exit(void)
{
	/* nothing to do ? */
}

/* fetch : read one word at * PC, and increment PC. */
INLINE UINT16 fetch(void)
{
	UINT16 value = readword(I.PC);
	I.PC += 2;
	return value;
}


static int tms99xx_execute(int cycles)
			{
	TMS99XX_ICOUNT = cycles;

	lds_flag = 0;
	ldd_flag = 0;

	do
	{
		/* all TMS9900 chips I know do not honor interrupts after XOP, BLWP or MID (after any
          interrupt-like instruction, actually), and they do not either after LDS and LDD
          (There are good reasons for this). */
		if ((I.interrupt_pending) && (! I.disable_interrupt_recognition))
		{
			int level;

			level = I.irq_level;


			if (I.load_state)
			{	/* LOAD has the highest priority */

				contextswitchX(0xFFFC);  /* load vector, save PC, WP and ST */

				I.STATUS &= ~ST_IM;     /* clear interrupt mask */

				/* clear IDLE status if necessary */
				if (I.IDLE)
				{
					I.IDLE = 0;		/* clear IDLE condition */
					if (I.idle_callback)
						(*I.idle_callback)(0);
				}

				CYCLES(6/*to be confirmed*/, 22, 14);
			}
			else if (level <= IMASK)
			{	/* a maskable interrupt is honored only if its level isn't greater than IMASK */

			contextswitchX(level*4); /* load vector, save PC, WP and ST */

				/* change interrupt mask */
				if (level)
				{
				I.STATUS = (I.STATUS & ~ST_IM) | (level -1);  /* decrement mask */
					I.interrupt_pending = 0;  /* as a consequence, the interrupt request will be subsequently ignored */
				}
				else
				I.STATUS &= ~ST_IM; /* clear mask (is this correct???) */

#if (TMS99XX_MODEL == TMS9995_ID)
				I.STATUS &= 0xFE00;
#endif

				/* clear IDLE status if necessary */
			if (I.IDLE)
			{
				I.IDLE = 0;		/* clear IDLE condition */
				if (I.idle_callback)
					(*I.idle_callback)(0);
			}

#if (TMS99XX_MODEL == TMS9995_ID)
				/* Clear bit in latch */
				/* I think tms9989 does this, too */
				if (level != 2)
				{	/* Only do this on level 1, 3, 4 interrupts */
					int mask = 1 << level;
					int flag_mask = (level == 1) ? 4 : mask;

					I.int_latch &= ~ mask;
					I.flag &= ~ flag_mask;

					/* unlike tms9900, we can call the callback */
					if (level == 1)
						(* I.irq_callback)(0);
					else if (level == 4)
						(* I.irq_callback)(1);
				}
#endif

			CYCLES(6, 22, 14);
			}
			else
			{
				logerror("tms9900.c : the interrupt_pending flag was set incorrectly\n");
				I.interrupt_pending = 0;
			}
		}

		if ((Machine->debug_flags & DEBUG_FLAG_CALL_HOOK) != 0)
		{
			#if 0		/* Trace */
			logerror("> PC %4.4x :%4.4x %4.4x : R=%4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x %4.4x%4.4x %4.4x %4.4x %4.4x %4.4x %4.4x :T=%d\n",I.PC,I.STATUS,I.WP,I.FR[0],I.FR[1],I.FR[2],I.FR[3],I.FR[4],I.FR[5],I.FR[6],I.FR[7],I.FR[8],I.FR[9],I.FR[10],I.FR[11],I.FR[12],I.FR[13],I.FR[14],I.FR[15],TMS99XX_ICOUNT);
				#if 0	/* useful with TI99/4a driver */
				#ifdef MESS
				if (I.PC == 0x0078)
				{
					extern struct
					{
						/* pointer to GROM data */
						UINT8 *data_ptr;
						/* current address pointer for the active GROM in port (16 bits) */
						unsigned int addr;
						/* GROM data buffer */
						UINT8 buf;
						/* internal flip-flops that are set after the first access to the GROM
                       address so that next access is mapped to the LSB, and cleared after each
                       data access */
						char raddr_LSB, waddr_LSB;
					} console_GROMs;
					logerror("> GPL pointer %4.4X\n", console_GROMs.addr);
				}
				#endif
				#endif
			#endif

			debugger_instruction_hook(Machine, I.IR);
		}

		if (I.IDLE)
		{	/* IDLE instruction has halted execution */
			#if EXTERNAL_INSTRUCTION_DECODING
				external_instruction_notify(2);
				CYCLES(Moof!, 2, 2); /* 2 cycles per CRU write */
			#else
				TMS99XX_ICOUNT = 0;	/* much simpler... */
			#endif
		}
		else
		{	/* we execute an instruction */
			I.disable_interrupt_recognition = 0;  /* default value */
			I.IR = fetch();
			execute(I.IR);
			#if HAS_MAPPING
				if (lds_flag)
				{
					lds_flag = 0;
					I.cur_src_map = 2 /*(I.cur_src_map != 2) ? 2 : I.cur_map*/;
				}
				else
					I.cur_src_map = I.cur_map;
				if (ldd_flag)
				{
					ldd_flag = 0;
					I.cur_dst_map = 2 /*(I.cur_src_map != 2) ? 2 : I.cur_map*/;
				}
				else
					I.cur_dst_map = I.cur_map;
				#if (TMS99XX_MODEL == TI990_10_ID)
					write_inhibit = 0;
				#endif
			#endif

			#if (HAS_OVERFLOW_INTERRUPT)
				#if (TMS99XX_MODEL == TMS9995_ID)
					/* Note that TI had some problem implementing this...  I don't know if this feature works on
                    a real-world TMS9995. */
					if ((I.STATUS & ST_OVIE) && (I.STATUS & ST_OV) && (I.irq_level > 2))
						I.irq_level = 2;  /* interrupt request */
				#else
					#warning "todo..."
				#endif
			#endif
		}

	} while (TMS99XX_ICOUNT > 0);

	return cycles - TMS99XX_ICOUNT;
}

static void tms99xx_get_context(void *dst)
{
	setstat();

	if( dst )
		*(tms99xx_Regs*)dst = I;
}

static void tms99xx_set_context(void *src)
{
	if( src )
	{
		I = *(tms99xx_Regs*)src;
		/* We have to make additionnal checks, because Mame debugger can foolishly initialize
        the context to all 0s */
		#if (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TI990_10_ID)
		if (! I.irq_state)
			I.irq_level = 16;
#elif ((TMS99XX_MODEL == TMS9980_ID) || (TMS99XX_MODEL == TMS9995_ID))
		/* Our job is simpler, since there is no level-0 request... */
		if (! I.irq_level)
			I.irq_level = 16;
		#elif ((TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID))
			if (! I.irq_level)
				I.irq_level = 4;
#else
		#warning "You may want to have a look at this problem"
#endif

		getstat();  /* set last_parity */
	}
}

#if (TMS99XX_MODEL == TI990_10_ID)

static void tms99xx_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		I.load_state = state;   /* save new state */

		field_interrupt();  /* interrupt status changed */
	}
	else
	{
		/*if (I.irq_state == state)
            return;*/

		I.irq_state = state;

		if (state == CLEAR_LINE)
			I.irq_level = 16;
			/* trick : 16 will always be bigger than the IM (0-15), so there will never be interrupts */
		else
			I.irq_level = (* I.irq_callback)(0);

		field_interrupt();  /* interrupt state is likely to have changed */
	}
}

#elif (TMS99XX_MODEL == TMS9900_ID)

/*
void tms99xx_set_irq_line(INPUT_LINE_NMI, int state) : change the state of the LOAD* line

    state == 0 -> LOAD* goes high (inactive)
    state != 0 -> LOAD* goes low (active)

    While LOAD* is low, we keep triggering LOAD interrupts...

    A problem : some peripherals lower the LOAD* line for a fixed time interval (causing the 1st
    instruction of the LOAD interrupt routine to be repeated while the line is low), and will be
    perfectly happy with the current scheme, but others might be more clever and wait for the IAQ
    (Instruction acquisition) line to go high, and this needs a callback function to emulate.
*/

/*
void tms99xx_set_irq_line(int irqline, int state) : sets the state of the interrupt line.

    irqline is ignored, and should always be 0.

    state == 0 -> INTREQ* goes high (inactive)
    state != 0 -> INTREQ* goes low (active)
*/
/*
    R Nabet 991020, revised 991218 :
    In short : interrupt code should call "cpu_set_irq_line(0, 0, ASSERT_LINE);" to set an
    interrupt request (level-triggered interrupts).  Also, there MUST be a call to
    "cpu_set_irq_line(0, 0, CLEAR_LINE);" in the machine code, when the interrupt line is released by
    the hardware (generally in response to an action performed by the interrupt routines).
    On tms9995 (9989 ?), you can use PULSE_LINE, too, since the processor latches the line...

    **Note** : HOLD_LINE *NEVER* makes sense on the TMS9900 (or 9980, 9995...).  The reason is the
    TMS9900 does NOT tell the world it acknoledges an interrupt, so no matter how much hardware you
    use, you cannot know when the CPU takes the interrupt, hence you cannot release the line when
    the CPU takes the interrupt.  Generally, the interrupt condition is cleared by the interrupt
    routine (with some CRU or memory access).

    Note that cpu_generate_interrupt uses HOLD_LINE, so your driver interrupt code
    should always use the new style, i.e. return "ignore_interrupt()" and call
    "cpu_set_irq_line(0, 0, ASSERT_LINE);" explicitely.

    Last, many TMS9900-based hardware use a TMS9901 interrupt-handling chip.  If anybody wants
    to emulate some hardware which uses it, note that I am writing some emulation in the TI99/4(A)
    driver in MESS, so you should ask me.
*/
/*
 * HJB 990430: changed to use irq_callback() to retrieve the vector
 * instead of using 16 irqlines.
 *
 * R Nabet 990830 : My mistake, I rewrote all these once again ; I think it is now correct.
 * A driver using the TMS9900 should do :
 *      cpu_0_irq_line_vector_w(0, level);
 *      cpu_set_irq_line(0,0,ASSERT_LINE);
 *
 * R Nabet 991108 : revised once again, with advice from Juergen Buchmueller, after a discussion
 * with Nicola...
 * We use the callback to retreive the interrupt level as soon as INTREQ* is asserted.
 * As a consequence, I do not support HOLD_LINE normally...  However, we do not really have to
 * support HOLD_LINE, since no real world TMS9900-based system can support this.
 * FYI, there are two alternatives to retreiving the interrupt level with the callback :
 * a) using 16 pseudo-IRQ lines.  Mostly OK, though it would require a few core changes.
 *    However, this could cause some problems if someone tried to set two lines simulteanously...
 *    And TMS9900 did NOT have 16 lines ! This is why Juergen and I did not retain this solution.
 * b) modifying the interrupt system in order to provide an extra int to every xxx_set_irq_line
 *    function.  I think this solution would be fine, but it would require quite a number of
 *    changes in the MAME core.  (And I did not feel the courage to check out 4000 drivers and 25
 *    cpu cores ;-) .)
 *
 * Note that this does not apply to tms9995.
*/
static void tms99xx_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		I.load_state = state;   /* save new state */

		field_interrupt();  /* interrupt status changed */
	}
	else
	{
		/*if (I.irq_state == state)
            return;*/

		I.irq_state = state;

		if (state == CLEAR_LINE)
			I.irq_level = 16;
			/* trick : 16 will always be bigger than the IM (0-15), so there will never be interrupts */
		else
			I.irq_level = (* I.irq_callback)(0);

		field_interrupt();  /* interrupt state is likely to have changed */
	}
}

#elif (TMS99XX_MODEL == TMS9980_ID)
/*
    interrupt system similar to tms9900, but only 3 interrupt pins (IC0-IC2)
*/

static void tms99xx_set_irq_line(int irqline, int state)
{
	if (state == CLEAR_LINE)
	{
		I.load_state = 0;
		I.irq_state = 0;
		I.irq_level = 16;
		/* trick : 16 will always be bigger than the IM (0-15), so there will never be interrupts */
	}
	else
	{
		int level;

		if (irqline == INPUT_LINE_NMI)
			level = 2;	/* translate MAME's convention to CPU's representation */
		else
			level = (* I.irq_callback)(0);

		switch (level)
		{
		case 0:
		case 1:
			I.load_state = 0;
			I.irq_state = 0;
			I.irq_level = 16;
			tms99xx_reset();
			break;
		case 2:
			I.load_state = 1;
			I.irq_state = 0;
			I.irq_level = 16;
			break;
		case 7:
			I.load_state = 0;
			I.irq_state = 0;
			I.irq_level = 16;
			break;
		default:  /* external levels 1, 2, 3, 4 */
			I.load_state = 0;
			I.irq_state = 1;
			I.irq_level = level - 2;
			break;
		}
	}

	field_interrupt();  /* interrupt state is likely to have changed */
}

#elif (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
/*
    2 interrupt pins (int1 and int2)
*/

static void tms99xx_set_irq_line(int irqline, int state)
	{
	int mask;

	if (irqline == 0)
		/* INT1 */
		mask = 1;
	else if (irqline == 1)
		/* INT2 */
		mask = 4;
	else
		/* What on earth??? */
		return;

	if (state)
		I.irq_state |= mask;
	else
		I.irq_state &= ~mask;

	field_interrupt();  /* interrupt state is likely to have changed */
}

#elif (TMS99XX_MODEL == TMS9995_ID)
/*
  this call-back is called by MESS timer system when the timer reaches 0.
*/
static TIMER_CALLBACK( decrementer_callback )
{
	/* request decrementer interrupt */
	I.int_latch |= 0x8;
	I.flag |= 0x8;

	field_interrupt();
}


/*
    reset and load the timer/decrementer

    Note that I don't know whether toggling flag0/flag1 causes the decrementer to be reloaded or not
*/
static void reset_decrementer(void)
{
	timer_adjust_oneshot(I.timer, attotime_never, 0);

	/* reload count */
	I.decrementer_count = I.decrementer_interval;

	/* decrementer / timer enabled ? */
	I.decrementer_enabled = ((I.flag & 2) && (I.decrementer_interval));

	if (I.decrementer_enabled && ! (I.flag & 1))
		{	/* timer */
		attotime period = ATTOTIME_IN_CYCLES(I.decrementer_interval * 16L, cpu_getactivecpu());
		timer_adjust_periodic(I.timer, period, 0, period);
	}
}

/*
    You have two interrupt line : one triggers level-1 interrupts, the other triggers level-4
    interrupts (or decrements the decrementer register).

    According to the hardware, you may use PULSE_LINE (edge-triggered interrupts), or ASSERT_LINE
    (level-triggered interrupts).  Edge-triggered interrupts are way simpler, but if multiple devices
    share the same line, they must use level-triggered interrupts.
*/
static void tms99xx_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		I.load_state = state;   /* save new state */

		field_interrupt();  /* interrupt status changed */
	}
	else
	{
		int mask = (irqline == 0) ? 0x2 : 0x10;
		int flag_mask = (irqline == 0) ? 0x4 : 0x10;

		if (((I.int_state & mask) != 0) ^ (state != 0))
		{	/* only if state changes */
			if (state)
			{
				I.int_state |= mask;

				if ((irqline == 1) && (I.flag & 1))
				{	/* event counter mode : INT4* triggers no interrupt... */
					if (I.decrementer_enabled)
					{	/* decrement, then interrupt if reach 0 */
						if ((-- I.decrementer_count) == 0)
						{
							decrementer_callback(Machine, NULL, 0);
							I.decrementer_count = I.decrementer_interval;	/* reload */
						}
					}
				}
				else
				{	/* plain interrupt mode */
					I.int_latch |= mask;
					I.flag |= flag_mask;
				}
			}
			else
			{
				I.int_state &= ~ mask;
			}

			field_interrupt();  /* interrupt status changed */
		}
	}
}

#else

#error "interrupt system not implemented"

#endif

/*
 * field_interrupt
 *
 * Determines whether if an interrupt is pending, and sets the revelant flag.
 *
 * Called when an interrupt pin (LOAD*, INTREQ*, IC0-IC3) is changed, and when the interrupt mask
 * is modified.
 *
 * By using this flag, we save some compares in the execution loop.  Subtle, isn't it ;-) ?
 *
 * R Nabet.
 */
#if (TMS99XX_MODEL == TI990_10_ID) || (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID)

static void field_interrupt(void)
{
	I.interrupt_pending = ((I.irq_level <= IMASK) || (I.load_state));
}

#elif (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)

static void field_interrupt(void)
{
	if (I.irq_state & 1)
		I.irq_level = 1;
	else if (I.irq_state & 2)
		I.irq_level = 2;
	else if (I.irq_state & 4)
		I.irq_level = 3;
	else
		I.irq_level = 4;
		/* trick : 4 will always be bigger than the IM (0-3), so there will never be interrupts */

	I.interrupt_pending = ((I.irq_level <= IMASK) || (I.load_state));
}

#elif (TMS99XX_MODEL == TMS9995_ID)

static void field_interrupt(void)
{
	if (I.load_state)
	{
		I.interrupt_pending = 1;
	}
	else
	{
		int current_int;
		int level;

		if (I.flag & 1)
			/* event counter mode : ignore int4* line... */
			current_int = (I.int_state & ~0x10) | I.int_latch;
		else
			/* normal behavior */
			current_int = I.int_state | I.int_latch;

		if (current_int)
			/* find first bit to 1 */
			/* possible values : 1, 3, 4 */
			for (level=0; ! (current_int & 1); current_int >>= 1, level++)
				;
		else
			level=16;

		I.irq_level = level;

		I.interrupt_pending = (level <= IMASK);
	}
}

#else

#error "field_interrupt() not written"

#endif

static unsigned tms99xx_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return Dasm9900(buffer, pc, TMS99XX_MODEL, oprom, opram);
}


/*****************************************************************************/
/*
    CRU support code

    The CRU bus is a 1-bit-wide I/O bus.  The CPU can read or write bits at random address.
    Special instructions are dedicated to reading and writing one or several consecutive bits.



    Note that TMS99000 additionally supports parallel CRU operations, although I don't know how
    this feature is implemented.
*/

enum
{
#if (TMS99XX_MODEL == TI990_10_ID)
	/* 3 MSBs do exist, although they are not connected (don't ask...) */
	CRUAddrBit = 15
#elif (TMS99XX_MODEL == TMS9900_ID)
	/* 3 MSBs are always 0 to support external instructions */
	CRUAddrBit = 12
#elif (TMS99XX_MODEL == TMS9980_ID)
	/* 2 bits unused, and 2 MSBs are always 0 to support external instructions */
	CRUAddrBit = 11
#elif (TMS99XX_MODEL == TMS9940_ID)
	/* 9 internal address lines (8 external) */
	CRUAddrBit = 9
#elif (TMS99XX_MODEL == TMS9995_ID)
	/* no such problem here : data bus lines D0-D2 provide the external instruction code */
	CRUAddrBit = 15
#else
	#warning "I don't know how your processor handles CRU."
	CRUAddrBit = 15
#endif
};

enum
{
	rCRUAddrMask = (1 << (CRUAddrBit - 3)) - 1,
	wCRUAddrMask = (1 << CRUAddrBit) - 1
};


#if (TMS99XX_MODEL == TMS9995_ID)

/* set decrementer mode flag */
static void set_flag0(int val)
{
	if (val)
		I.flag |= 1;
	else
		I.flag &= ~ 1;

	reset_decrementer();
}

/* set decrementer enable flag */
static void set_flag1(int val)
{
	if (val)
		I.flag |= 2;
	else
		I.flag &= ~ 2;

	reset_decrementer();
}

#endif

typedef enum
{
	CRU_OK = 0,
	CRU_PRIVILEGE_VIOLATION = -1
} cru_error_code;

#define WRITEPORT(port, data) io_write_byte_8be(port, data)

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
/* on tms9940, we have to handle internal CRU ports */
static void write_single_CRU(int port, int data)
{
	int mask;

	if (! (port & 0x100))
	{
		/*if (I.config & CB0)*/
			/* External CRU */
			WRITEPORT(port, (data & 0x01));
	}
	else
	{
		/* internal CRU */
		switch ((port >> 4) & 0xf)
		{
		case 0x8:
			if (port == 0x181)
			{
				/* clear decrementer interrupt */
				I.irq_state &= ~2;
			}
			if (port >= 0x183) && (port <= 0x186)
			{
				/* write configuration register */
				mask = 1 << (port - 0x183);
				/* ... */
			}
			break;

		case 0x9:
			if (port <= 0x19D)
			{
				/* write decrementer latch */
				mask = 1 << (port - 0x190);
				/* ... */
			}
			else if (port == 0x19E)
			{
				/* set decrementer as timer (1) or event counter (0) */
				/* ... */
			}
			break;

		case 0xA:
			/* multiprocessor system interface */
			mask = 1 << (port - 0x1A0);
			/* ... */
			break;

		case 0xB:
			/* flags */
			mask = 1 << (port - 0x1B0);
			/* ... */
			break;

		case 0xC:
		case 0xD:
			/* direction for P0-P31 */
			mask = 1 << (port - 0x1C0);
			/* ... */
			break;

		case 0xE:
		case 0xF:
			/* data for P0-P31 */
			mask = 1 << (port - 0x1E0);
			/* ... */
			break;
		}
	}
}
#elif (TMS99XX_MODEL == TMS9995_ID)
/* on tms9995, we have to handle internal CRU ports */
static void write_single_CRU(int port, int data)
	{
	/* Internal CRU */
	switch (port)
	{
	case 0xF70:
		set_flag0(data & 0x01);
		break;
	case 0xF71:
		set_flag1(data & 0x01);
		break;
	case 0xF72:
	case 0xF73:
	case 0xF74:
		break;     /* ignored */
	case 0xF75:
	case 0xF76:
	case 0xF77:
	case 0xF78:
	case 0xF79:
	case 0xF7A:
	case 0xF7B:
	case 0xF7C:
	case 0xF7D:
	case 0xF7E:
	case 0xF7F:
		{	/* user defined flags */
			int mask = 1 << (port - 0xF70);
			if (data & 0x01)
				I.flag |= mask;
			else
				I.flag &= ~ mask;
		}
		break;

	case 0x0FED:
			/* MID flag */
		I.MID_flag = data & 0x01;
		break;
	}
			/* External CRU */
	/* Even though all the registers above are implemented internally, accesses
    are passed to the external bus, too, and an external device might respond
    to a write to these CRU address as well (particularly a write to the user
    flag registers). */
	WRITEPORT(port, (data & 0x01));
}
#else
#define write_single_CRU(port, data) WRITEPORT(port, data)
#endif

/*
    performs a normal write to CRU bus (used by SBZ, SBO, LDCR : address range 0 -> 0xFFF)
*/
static cru_error_code writeCRU(int CRUAddr, int Number, UINT16 Value)
{
	int count;

	/*logerror("PC %4.4x Write CRU %x for %x =%x\n",I.PC,CRUAddr,Number,Value);*/

	CRUAddr &= wCRUAddrMask;

	/* Write Number bits from CRUAddr */

	for(count=0; count<Number; count++)
	{
		#if HAS_PRIVILEGE
			if ((I.STATUS & ST_PR) && (CRUAddr >= 0xE00))
				return CRU_PRIVILEGE_VIOLATION;
#endif

		write_single_CRU(CRUAddr, (Value & 0x01));
		Value >>= 1;
		CRUAddr = (CRUAddr + 1) & wCRUAddrMask;
	}

	return CRU_OK;
}

#if EXTERNAL_INSTRUCTION_DECODING
/*
    Some opcodes perform a dummy write to a special CRU address, so that an external function may be
    triggered.

    Only the first 3 MSBs of the address matter : other address bits and the written value itself
    are undefined.

    How should we support this ? With callback functions ? Actually, as long as we do not support
    hardware which makes use of this feature, it does not really matter :-) .
*/
static void external_instruction_notify(int ext_op_ID)
{
#if 1
	/* I guess we can support this like normal CRU operations */
#if (TMS99XX_MODEL == TMS9900_ID)
	WRITEPORT(ext_op_ID << 12, 0); /* or is it 1 ??? */
#elif (TMS99XX_MODEL == TMS9980_ID)
		WRITEPORT((ext_op_ID & 3) << 11, (ext_op_ID & 4) ? 1 : 0);
#elif (TMS99XX_MODEL == TMS9995_ID)
		WRITEPORT(ext_op_ID << 15, 0); /* or is it 1 ??? */
#else
		#warning "I don't know how your processor handles external opcodes (maybe you don't need them, though)."
#endif

#else
	switch (ext_op_ID)
	{
		case 2: /* IDLE */

			break;
		case 3: /* RSET */

			break;
		case 5: /* CKON */

			break;
		case 6: /* CKOF */

			break;
		case 7: /* LREX */

			break;
		case 0:
			/* normal CRU write !!! */
			logerror("PC %4.4x : external_instruction_notify : wrong ext_op_ID",I.PC);
			break;
		default:
			/* unknown address */
			logerror("PC %4.4x : external_instruction_notify : unknown ext_op_ID",I.PC);
			break;
	}
#endif
}
#endif

/*
    performs a normal read to CRU bus (used by TB, STCR : address range 0->0xFFF)

    Note that on some hardware, e.g. TI99/4(a), all normal memory operations cause unwanted CRU
    read at the same address.  This seems to be impossible to emulate efficiently, so, if you need
    to emulate this, you're in trouble.
*/
#define READPORT(port) io_read_byte_8be(port)


#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
/* on tms9940, we have to handle internal CRU ports */
static int read_single_CRU(int port)
{
	int reply;
	int shift;

	if (! (port & 0x20))
	{
		/*if (I.config & CB0)*/
			/* External CRU */
			reply = READPORT(port, (data & 0x01));
	}
	else
	{
		/* internal CRU */
		switch (port)
		{
		case 0x10:
			/* read interrupt state */
			reply = I.irq_state;
			break;

		case 0x12:
			/* read decrementer LSB */
			/* ... */
			break;
		case 0x13:
			/* read decrementer MSB */
			/* ... */
			break;

		case 0x14:
			/* read multiprocessor system interface LSB */
			/* ... */
			break;
		case 0x15:
			/* read multiprocessor system interface MSB */
			/* ... */
			break;

		case 0x16:
			/* read flags LSB */
			/* ... */
			break;
		case 0x17:
			/* read flags MSB */
			/* ... */
			break;

		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
			/* direction for P0-P31 */
			shift = (port - 0x18) << 3;
			/* ... */
			break;

		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
			/* data for P0-P31 */
			shift = (port - 0x1C) << 3;
			/* ... */
			break;

		default:
			reply = 0;
			break;
		}
	}

	return reply;
}
#elif (TMS99XX_MODEL == TMS9995_ID)
/* on tms9995, we have to handle internal CRU ports */
static int read_single_CRU(int port)
{
	switch (port)
{
	case 0x1EE:
		/* flag, bits 0-7 */
		return I.flag & 0xFF;
	case 0x1EF:
		/* flag, bits 8-15 */
		return (I.flag >> 8) & 0xFF;
	case 0x1FD:
		/* MID flag, and external devices */
		if (I.MID_flag)
			return READPORT(port) | 0x10;
		else
			return READPORT(port) & ~ 0x10;
	default:
		/* external devices */
		return READPORT(port);
	}
}
#else
#define read_single_CRU(port) READPORT(port)
#endif

static int readCRU(int CRUAddr, int Number)
{
	static const int BitMask[] =
	{
		0, /* filler - saves a subtract to find mask */
		0x0001,0x0003,0x0007,0x000F,0x001F,0x003F,0x007F,0x00FF,
		0x01FF,0x03FF,0x07FF,0x0FFF,0x1FFF,0x3FFF,0x7FFF,0xFFFF
	};

	int Offset,Location,Value;

	/*logerror("Read CRU %x for %x\n",CRUAddr,Number);*/

	Location = (CRUAddr >> 3)  & rCRUAddrMask;
	Offset   = CRUAddr & 07;

	/* Read 8 bits */
	#if HAS_PRIVILEGE
		if ((I.STATUS & ST_PR) && (Location >= (0xE00 >> 3)))
			return CRU_PRIVILEGE_VIOLATION;
	#endif
	Value = read_single_CRU(Location);

	if ((Offset+Number) > 8)
	{
		/* Read next 8 bits */
		Location = (Location + 1) & rCRUAddrMask;
		#if HAS_PRIVILEGE
			if ((I.STATUS & ST_PR) && (Location >= (0xE00 >> 3)))
				return CRU_PRIVILEGE_VIOLATION;
		#endif
		Value |= read_single_CRU(Location) << 8;

		if ((Offset+Number) > 16)
	{
			/* Read next 8 bits */
			Location = (Location + 1) & rCRUAddrMask;
			#if HAS_PRIVILEGE
				if ((I.STATUS & ST_PR) && (Location >= (0xE00 >> 3)))
					return CRU_PRIVILEGE_VIOLATION;
			#endif
			Value |= read_single_CRU(Location) << 16;
		}
	}

		/* Allow for Offset */
		Value >>= Offset;

		/* Mask out what we want */
	Value &= BitMask[Number];

		/* And update */
	return Value;
	}

/*****************************************************************************/

#if HAS_MAPPING
/* load a map file from memory */
static void load_map_file(UINT16 src_addr, int src_map_file, int dst_file)
{
	int i;


	/* load mapped address into the memory address register */
	if ((src_map_file == 0) && (src_addr >= 0xf800))
	{	/* intercept TPCS and CPU ROM */
		if (src_addr < 0xfc00)
			/* TPCS */
			I.mapper_address_latch = 0x1f0000+src_addr;
		else
			/* CPU ROM */
			I.mapper_address_latch = 0x1f0000+src_addr;	/* hack... */
	}
	else if (! I.mapping_on)
	{
		I.mapper_address_latch = src_addr;
}
	else
	{
		int map_index;

		if (src_addr <= I.map_files[src_map_file].limit[0])
			map_index = 0;
		else if (src_addr <= I.map_files[src_map_file].limit[1])
			map_index = 1;
		else if (src_addr <= I.map_files[src_map_file].limit[2])
			map_index = 2;
		else
		{
			if ((! I.reset_maperr) && ! (I.error_interrupt_register & EIR_MAPERR))
			{
				I.error_interrupt_register |= EIR_MAPERR;
				write_inhibit = 1;
			}
			I.mapper_address_latch = src_addr;
			map_index = -1;
		}
		if (map_index != -1)
			I.mapper_address_latch = I.map_files[src_map_file].bias[map_index]+src_addr;
	}


	for (i=0; i<3; i++)
{
		I.map_files[dst_file].L[i] = program_read_word_16be(I.mapper_address_latch) & 0xffe0;
		I.map_files[dst_file].limit[i] = (I.map_files[dst_file].L[i] ^ 0xffe0) | 0x001f;
		I.mapper_address_latch = (I.mapper_address_latch+2) & 0x1fffff;
		I.map_files[dst_file].B[i] = program_read_word_16be(I.mapper_address_latch);
		I.map_files[dst_file].bias[i] = ((unsigned int) I.map_files[dst_file].B[i]) << 5;
		I.mapper_address_latch = (I.mapper_address_latch+2) & 0x1fffff;
	}
}
#endif

/* contextswitch : performs a BLWP, i.e. load PC, WP, and save old PC, old WP and ST... */
static void contextswitch(UINT16 addr)
{
	UINT16 oldWP, oldpc;

	/* save old state */
	oldWP = I.WP;
	oldpc = I.PC;

	/* load vector */
	I.WP = readword(addr) & ~1;
	I.PC = readword(addr+2) & ~1;

	/* write old state to regs */
	WRITEREG(R13, oldWP);
	WRITEREG(R14, oldpc);
	setstat();
	WRITEREG(R15, I.STATUS);
}

#if HAS_MAPPING || HAS_PRIVILEGE

/* priviledged context switch, that occurs after a reset, interrupt or XOP:
we enter priviledged mode and select map file 0 before doing the context switch */
/* For CPU that have no priviledge support, contextswitchX would behave
identically to contextswitch, so we can call contextswitch in all cases. */
static void contextswitchX(UINT16 addr)
{
	UINT16 oldWP, oldpc, oldST;

	/* save old state */
	oldWP = I.WP;
	oldpc = I.PC;
	setstat();
	oldST = I.STATUS;

	/* enter priviledged mode and select map file 0 */
	#if HAS_PRIVILEGE
		I.STATUS &= ~ ST_PR;
	#endif
	#if HAS_MAPPING
		I.STATUS &= ~ ST_MF;
	#endif
	getstat();

	/* load vector */
	I.WP = readword(addr) & ~1;
	I.PC = readword(addr+2) & ~1;

	/* write old state to regs */
	WRITEREG(R13, oldWP);
	WRITEREG(R14, oldpc);
	WRITEREG(R15, oldST);
}

#endif

/*
 * decipheraddr : compute and return the effective adress in word instructions.
 *
 * NOTA : the LSBit is always ignored in word adresses,
 * but we do not set it to 0 because of XOP...
 */
static UINT16 decipheraddr(UINT16 opcode)
{
	register UINT16 ts = opcode & 0x30;
	register UINT16 reg = opcode & 0xF;

	reg += reg;

	if (ts == 0)
		/* Rx */
		return(reg + I.WP);
	else if (ts == 0x10)
	{	/* *Rx */
		CYCLES(0, 4, 1);
		return(readword(reg + I.WP));
	}
	else if (ts == 0x20)
	{
		register UINT16 imm;

		imm = fetch();

		if (reg)
		{	/* @>xxxx(Rx) */
			CYCLES(1, 8, 3);
			return(readword(reg + I.WP) + imm);
		}
		else
		{	/* @>xxxx */
			CYCLES(3, 8, 1);
			return(imm);
		}
	}
	else /*if (ts == 0x30)*/
	{	/* *Rx+ */
		register UINT16 response;

		reg += I.WP;    /* reg now contains effective address */

		CYCLES(1, 8, 3);

		response = readword(reg);
		writeword(reg, response+2); /* we increment register content */
		return(response);
	}
}

/* decipheraddrbyte : compute and return the effective adress in byte instructions. */
static UINT16 decipheraddrbyte(UINT16 opcode)
{
	register UINT16 ts = opcode & 0x30;
	register UINT16 reg = opcode & 0xF;

	reg += reg;

	if (ts == 0)
		/* Rx */
		return(reg + I.WP);
	else if (ts == 0x10)
	{	/* *Rx */
		CYCLES(0, 4, 1);
		return(readword(reg + I.WP));
	}
	else if (ts == 0x20)
	{
		register UINT16 imm;

		imm = fetch();

		if (reg)
		{	/* @>xxxx(Rx) */
			CYCLES(1, 8, 3);
			return(readword(reg + I.WP) + imm);
		}
		else
		{	/* @>xxxx */
			CYCLES(3, 8, 1);
			return(imm);
		}
	}
	else /*if (ts == 0x30)*/
	{	/* *Rx+ */
		register UINT16 response;

		reg += I.WP;    /* reg now contains effective address */

		CYCLES(1, 6, 3);

		response = readword(reg);
		writeword(reg, response+1); /* we increment register content */
		return(response);
	}
}


/*************************************************************************/

#if (TMS99XX_MODEL == TI990_10_ID)
	/* TI990/10 generates an error interrupt */
	/* timings are unknown */
	#define HANDLE_ILLEGAL													\
	{																		\
		I.error_interrupt_register |= EIR_ILLOP;							\
		if (I.error_interrupt_callback)										\
			(*I.error_interrupt_callback)(1);								\
	}
#elif TMS99XX_MODEL <= TMS9989_ID
	/* TMS9900/TMS9980 merely ignore the instruction */
	#define HANDLE_ILLEGAL TMS99XX_ICOUNT -= 6
#elif TMS99XX_MODEL == TMS9995_ID
	/* TMS9995 generates a MID interrupt */
	#define HANDLE_ILLEGAL \
	{ \
		I.MID_flag = 1; \
		contextswitchX(0x0008); \
		I.STATUS = (I.STATUS & 0xFE00) | 0x1; \
		I.disable_interrupt_recognition = 1; \
	}
#else
#define HANDLE_ILLEGAL
#warning "don't know"
#endif

#if HAS_PRIVILEGE
	#if (TMS99XX_MODEL == TI990_10_ID)
		/* TI990/10 generates an error interrupt */
		/* timings are unknown */
		#define HANDLE_PRIVILEGE_VIOLATION									\
		{																	\
			I.error_interrupt_register |= EIR_PRIVOP;						\
			if (I.error_interrupt_callback)									\
				(*I.error_interrupt_callback)(1);							\
		}
	#else
		#define HANDLE_PRIVILEGE_VIOLATION
		#warning "don't know"
	#endif
#endif

/*==========================================================================
   Illegal instructions                                        >0000->01FF (not for 9989 and later)
                                                               >0C00->0FFF (not for 990/12 and 99110)
============================================================================*/

static void illegal(UINT16 opcode)
{
	HANDLE_ILLEGAL;
}


#if 0		/* ti990/12 only */
/*==========================================================================
   Additionnal instructions,                                   >0000->002F
   Additionnal single-register instruction,                    >0030->003F
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |     o p c o d e               |
    |     o p c o d e       | reg # |
    ---------------------------------

============================================================================*/
static void h0000(UINT16 opcode)
{
	if (opcode >= 0x30)
	{	/* STPC STore Program Counter */

	}
	else
	{
	switch (opcode /*& 0x3F*/)
	{
	case 0x1C:  /* SRAM */
			/* SRAM -- Shift Right Arithmetic Multiple precision */
		case 0x1D:  /* SLAM */
			/* SLAM -- Shift Left Arithmetic Multiple precision */
	case 0x1E:
			/* RTO --- Right Test for Ones */
	case 0x1F:
			/* LTO --- Left Test for Ones */
	case 0x20:
		/* CNTO -- CouNT Ones */
	case 0x21:
		/* SLSL -- Search LiSt Logical address */
	case 0x22:
		/* SLSP -- Search LiSt Physical address */
	case 0x23:
		/* BDC --- Binary to Decimal ascii Conversion */
	case 0x24:
			/* DBC --- Decimal to Binary ascii Conversion */
	case 0x25:
		/* SWPM -- SWaP Multiple precision */
	case 0x26:
		/* XORM -- eXclusive OR Multiple precision */
	case 0x27:
		/* ORM --- OR Multiple precision */
	case 0x28:
		/* ANDM -- AND Multiple precision */
		case 0x29:  /* SM */
			/* SM ---- Subtract Multiple precision integer */
		case 0x2A:  /* AM */
			/* AM ---- Add Multiple precision integer */
	case 0x2B:
		/* MOVA -- MOVe Address */
	case 0x2D:
		/* EMD --- Execute Micro-Diagnostics */
	case 0x2E:
		/* EINT -- Enable INTerrupts */
	case 0x2F:
		/* DINT -- Disable INTerrupts */

		break;

	default:
		HANDLE_ILLEGAL;
		break;
	}
}
}
#endif


#if HAS_9995_OPCODES
/*==========================================================================
   Additionnal single-register instructions,                   >0040->00FF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |     o p c o d e       | reg # |
    ---------------------------------

tms9989 and later : LST, LWP
============================================================================*/
static void h0040(UINT16 opcode)
{
	register UINT16 addr;

	addr = opcode & 0xF;
	addr = ((addr + addr) + I.WP) & ~1;

	switch ((opcode & 0xF0) >> 4)
	{
	case 8:   /* LST */
		/* LST --- Load STatus register */
		/* ST = *Reg */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
				I.STATUS = (I.STATUS & 0x01DF) | (readword(addr) & 0xFE20);
			else
				I.STATUS = readword(addr);
		#else
		I.STATUS = readword(addr);
		#endif
		getstat();  /* set last_parity */
		break;

	case 9:   /* LWP */
		/* LWP --- Load Workspace Pointer */
		/* WP = *Reg */
		I.WP = readword(addr) & ~1;
		break;

#if 0	/* 990/12 opcodes */
	case 4:
		/* CS ---- Compare Strings */
	case 5:
		/* SEQB -- Search string for EQual Byte */
	case 6:
		/* MOVS -- MOVe String */
	case 7:
		/* LIM --- Load Interrupt Mask  */

	case 10:
		/* LCS --- Load writable Control Store */

	case 11:  /* BLSK */
		/* BLSK -- Branch immediate and push Link to StacK */

	case 12:
		/* MVSR -- MoVe String Reverse */
	case 13:
		/* MVSK -- MoVe string from StacK */
	case 14:
		/* POPS -- POP String from stack    */
	case 15:
		/* PSHS -- PuSH String to stack */

		break;
#endif

	default:
		HANDLE_ILLEGAL;
		break;
	}
}


/*==========================================================================
   Additionnal single-operand instructions,                    >0100->01FF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |    o p c o d e    |TS |   S   |
    ---------------------------------

tms9989 and later : DIVS, MPYS
tms99xxx : BIND
============================================================================*/
static void h0100(UINT16 opcode)
{
	register UINT16 src;
#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
#endif

	src = decipheraddr(opcode) & ~1;

  switch ((opcode & 0xC0) >> 6)
  {
#if HAS_BIND_OPCODE
	case 1:   /* BIND */
		/* BIND -- Branch INDirect */
		I.PC = readwordX(src, src_map) & ~1;
		CYCLES(Mooof!, Mooof!, 4 /*don't know*/);
		break;
#endif

  case 2:   /* DIVS */
		/* DIVS -- DIVide Signed */
		/* R0 = (R0:R1)/S   R1 = (R0:R1)%S */
		{
			INT16 d = readwordX(src, src_map);
			INT32 divq = (READREG(R0) << 16) | READREG(R1);
			INT32 q = divq/d;

			if ((q < -32768L) || (q > 32767L))
			{
				I.STATUS |= ST_OV;
				CYCLES(Mooof!, Mooof!, 10);
			}
			else
			{
				I.STATUS &= ~ST_OV;
				setst_lae(q);
				WRITEREG(R0, q);
				WRITEREG(R1, divq%d);
				/* tms9995 : 33 is the worst case */
				CYCLES(Mooof!, Mooof!, 33);
			}
		}
		break;

  case 3:   /* MPYS */
		/* MPYS -- MultiPlY Signed */
		/* Results:  R0:R1 = R0*S */
		{
			INT32 prod = ((INT32) (INT16) readwordX(src, src_map));
			prod = prod*((INT32) (INT16) READREG(R0));

			I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ);
			if (prod > 0)
				I.STATUS |= (ST_LGT | ST_AGT);
			else if (prod < 0)
				I.STATUS |= ST_LGT;
			else
				I.STATUS |= ST_EQ;

			WRITEREG(R0, prod >> 16);
			WRITEREG(R1, prod);
		}
		CYCLES(Mooof!, Mooof!, 25);
		break;

#if 0	/* Origin unknown */
	case 0:   /* EVAD */
		/* EVAD -- EValuate ADdress instruction */

		break;
#endif

	default:
		HANDLE_ILLEGAL;
		break;
  }
}
#endif


/*==========================================================================
   Immediate, Control instructions,                            >0200->03FF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |     o p c o d e     |0| reg # |
    ---------------------------------

  LI, AI, ANDI, ORI, CI, STWP, STST, LIMI, LWPI, IDLE, RSET, RTWP, CKON, CKOF, LREX
systems with memory mapper: LMF
============================================================================*/
static void h0200(UINT16 opcode)
{
	register UINT16 addr;
	register UINT16 value;	/* used for anything */

	addr = opcode & 0xF;
	addr = ((addr + addr) + I.WP) & ~1;

	#if HAS_MAPPING
	if ((opcode >= 0x0320) && (opcode < 0x0340))
	{	/* LMF */
		/* LMF --- Load memory Map File */
			/* Used by the memory mapper on ti990/10 with mapping option, ti990/12, and the TIM99610
            mapper chip to be associated with tms99000.
            Syntax: "LMF Rn,m" loads map file m (0 or 1) with six words of memory, starting at address
            specified in workspace register Rn (0 thru 15). */
			#if HAS_PRIVILEGE
				if (I.STATUS & ST_PR)
				{
					HANDLE_PRIVILEGE_VIOLATION
					return;
				}
			#endif

			/* read address pointer */
			addr = readword(addr);

			load_map_file(addr, I.cur_map, (opcode & 0x10) ? 1 : 0);

			CYCLES(3, Mooof!, Mooof!);
		return;
	}
#endif

	#if BETTER_0200_DECODING
		/* better instruction decoding on ti990/10 */
		if (opcode & 0x10)
		{
			HANDLE_ILLEGAL;
			return;
		}
	#elif COMPLETE_0200_DECODING
		/* full instruction decoding on tms9995 */
	if (((opcode < 0x2E0) && (opcode & 0x10)) || ((opcode >= 0x2E0) && (opcode & 0x1F)))
	{
#if 0
			/* tms99110 opcode (not supported by 990/12) */
		if (opcode == 0x0301)
		{	/* CR ---- Compare Reals */
		}
			else
			/* tms99105+tms99110 opcode (not supported by 990/12) */
			if (opcode == 0x0302)
		{	/* MM ---- Multiply Multiple */
		}
			else
		#endif
		#if 0	/* ti990/12 only */
			if (opcode >= 0x03F0)
			{	/* EP ---- Extended Precision */
		}
		else
#endif
		HANDLE_ILLEGAL;
		return;
	}
#endif

	switch ((opcode & 0x1e0) >> 5)
	{
	case 0:   /* LI */
		/* LI ---- Load Immediate */
		/* *Reg = *PC+ */
		value = fetch();
		writeword(addr, value);
		setst_lae(value);
		CYCLES(3, 12, 3);
		break;

	case 1:   /* AI */
		/* AI ---- Add Immediate */
		/* *Reg += *PC+ */
		value = fetch();
		value = setst_add_laeco(readword(addr), value);
		writeword(addr, value);
		CYCLES(4, 14, 4);
		break;

	case 2:   /* ANDI */
		/* ANDI -- AND Immediate */
		/* *Reg &= *PC+ */
		value = fetch();
		value = readword(addr) & value;
		writeword(addr, value);
		setst_lae(value);
		CYCLES(4, 14, 4);
		break;

	case 3:   /* ORI */
		/* ORI --- OR Immediate */
		/* *Reg |= *PC+ */
		value = fetch();
		value = readword(addr) | value;
		writeword(addr, value);
		setst_lae(value);
		CYCLES(4, 14, 4);
		break;

	case 4:   /* CI */
		/* CI ---- Compare Immediate */
		/* status = (*Reg-*PC+) */
		value = fetch();
		setst_c_lae(value, readword(addr));
		CYCLES(6, 14, 4);
		break;

	case 5:   /* STWP */
		/* STWP -- STore Workspace Pointer */
		/* *Reg = WP */
		writeword(addr, I.WP);
		CYCLES(2, 8, 3);
		break;

	case 6:   /* STST */
		/* STST -- STore STatus register */
		/* *Reg = ST */
		setstat();
		writeword(addr, I.STATUS);
		CYCLES(2, 8, 3);
		break;

	case 7:   /* LWPI */
		/* LWPI -- Load Workspace Pointer Immediate */
		/* WP = *PC+ */
		I.WP = fetch() & ~1;
		CYCLES(3, 10, 4);
		break;

	case 8:   /* LIMI */
		/* LIMI -- Load Interrupt Mask Immediate */
		/* ST&15 |= (*PC+)&15 */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
#endif

		value = fetch();
		I.STATUS = (I.STATUS & ~ST_IM) | (value & ST_IM);
		field_interrupt();  /*IM has been modified.*/
		CYCLES(3, 16, 5);
		break;

	case 9:   /* LMF is implemented elsewhere - when it is implemented */
		HANDLE_ILLEGAL;
		break;

	case 10:  /* IDLE */
		/* IDLE -- IDLE until a reset, interrupt, load */
		/* The TMS99000 locks until an interrupt happen (like with 68k STOP instruction),
           and continuously performs a special CRU write (code 2). */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif
		I.IDLE = 1;
		#if EXTERNAL_INSTRUCTION_DECODING
		external_instruction_notify(2);
		#endif
		if (I.idle_callback)
			(*I.idle_callback)(1);
		CYCLES(4, 12, 7);
		/* we take care of further external_instruction_notify(2); in execute() */
		break;

	case 12:  /* RTWP */
		/* RTWP -- Return with Workspace Pointer */
		/* WP = R13, PC = R14, ST = R15 */
		addr = (I.WP + R13) & ~1;
		I.WP = readword(addr) & ~1;
		addr += 2;
		I.PC = readword(addr) & ~1;
		addr += 2;
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
				I.STATUS = (I.STATUS & 0x01DF) | (readword(addr) & 0xFE20);
			else
				I.STATUS = readword(addr);
		#else
			I.STATUS = readword(addr);
		#endif
		getstat();  /* set last_parity */

		field_interrupt();  /*IM has been modified.*/
		CYCLES(3, 14, 6);
		break;

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)

	case 11:  /* RSET */
	case 13:  /* CKON */
	case 14:  /* CKOF */
	case 15:  /* LREX */
		HANDLE_ILLEGAL;	/* These instruction "have been deleted" on the TMS9940 */
		break;

#else

	case 11:  /* RSET */
		/* RSET -- ReSET */
		/* Reset the Interrupt Mask, and perform a special CRU write (code 3). */
		/* The CRU write is supposed to reset external devices. */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif

		I.STATUS &= 0xFFF0; /*clear IM.*/
		field_interrupt();  /*IM has been modified.*/

		#if (TMS99XX_MODEL == TMS9995_ID)
			/*I.MID_flag = 0;*/		/* not sure about this */
		#endif
		#if (TMS99XX_MODEL == TI990_10_ID)
			I.error_interrupt_register = 0;
			I.mapping_on = 0;

			I.reset_maperr = 0;
			I.mapper_cru_read_register = 0;
			I.latch_control[0] = I.latch_control[1] = I.latch_control[2] = 0;

			I.diaglat = 0;
		#endif

		#if EXTERNAL_INSTRUCTION_DECODING
		external_instruction_notify(3);
		#endif

		#if EXTERNAL_INSTRUCTION_CALLBACK
			if (I.rset_callback)
				(*I.rset_callback)();
		#endif

		CYCLES(5, 12, 7);
		break;

#if EXTERNAL_INSTRUCTION_DECODING

	case 13:  /* CKON */
	case 14:  /* CKOF */
	case 15:  /* LREX */
		/* CKON -- ClocK ON */
		/* Perform a special CRU write (code 5). */
		/* An external circuitery could, for instance, enable the line clock interrupt (100Hz or 120Hz, depending on the electrical alimentation frequency). */
		/* CKOF -- ClocK OFf */
		/* Perform a special CRU write (code 6). */
		/* An external circuitery could, for instance, disable the line clock interrupt. */
		/* LREX -- Load or REstart eXecution */
		/* Perform a special CRU write (code 7). */
		/* An external circuitery could, for instance, activate the LOAD* line,
           causing a non-maskable LOAD interrupt (vector -1). */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif

		external_instruction_notify((opcode & 0x00e0) >> 5);

		#if EXTERNAL_INSTRUCTION_CALLBACK
			#warning "todo..."
		#endif

		CYCLES(Mooof!, 12, 7);
		break;

#elif EXTERNAL_INSTRUCTION_CALLBACK

	case 13:  /* CKON */
	case 14:  /* CKOF */
		/* CKON -- ClocK ON */
		/* Enable the line clock interrupt (100Hz or 120Hz, depending on the electrical alimentation frequency). */
		/* CKOF -- ClocK OFf */
		/* Disable the line clock interrupt. */
		/* We use a callback because the line clock is implemented in machine
           code, not in the CPU core. */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif

		if (I.ckon_ckof_callback)
			(*I.ckon_ckof_callback)((opcode & 0x0020) ? 1 : 0);


		CYCLES(5, Mooof!, Mooof!);
		break;

	case 15:  /* LREX */
		/* LREX -- Load or REstart eXecution */
		/* Trigger a LOAD interrupt (vector -1).  (We use a callback, and I
           have forgotten the reason why.) */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif

		if (I.lrex_callback)
			(*I.lrex_callback)();

		CYCLES(6, Mooof!, Mooof!);
		break;

#else

	#warning "Should not happen..."

#endif

#endif
	}
}


/*==========================================================================
   Single-operand instructions,                                >0400->07FF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |    o p c o d e    |TS |   S   |
    ---------------------------------

  BLWP, B, X, CLR, NEG, INV, INC, INCT, DEC, DECT, BL, SWPB, SETO, ABS
systems with memory mapper: LDD, LDS
============================================================================*/
static void h0400(UINT16 opcode)
{
	register UINT16 addr = decipheraddr(opcode) & ~1;
	register UINT16 value;  /* used for anything */
#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
#endif

	switch ((opcode & 0x3C0) >> 6)
	{
	case 0:   /* BLWP */
		/* BLWP -- Branch and Link with Workspace Pointer */
		/* Result: WP = *S+, PC = *S */
		/*         New R13=old WP, New R14=Old PC, New R15=Old ST */
		contextswitch(addr);
		CYCLES(3, 26, 11);
		I.disable_interrupt_recognition = 1;
		break;
	case 1:   /* B */
		/* B ----- Branch */
		/* PC = S */
#if ((TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID))
		(void) readwordX(addr, src_map);
#endif
		I.PC = addr;
		CYCLES(2, 8, 3);
		break;
	case 2:   /* X */
		/* X ----- eXecute */
		/* Executes instruction *S */
		execute(readwordX(addr, src_map));
		/* On tms9900, the X instruction actually takes 8 cycles, but we gain 2 cycles on the next
        instruction, as we don't need to fetch it. */
		CYCLES(1, 6, 2);
		break;
	case 3:   /* CLR */
		/* CLR --- CLeaR */
		/* *S = 0 */
#if ((TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID))
		(void) readwordX(addr, src_map);
#endif
		writewordX(addr, 0, src_map);
		CYCLES(2, 10, 3);
		break;
	case 4:   /* NEG */
		/* NEG --- NEGate */
		/* *S = -*S */
		value = - (INT16) readwordX(addr, src_map);
		if (value)
			I.STATUS &= ~ ST_C;
		else
			I.STATUS |= ST_C;

		#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
		if (value & 0x0FFF)
			I.STATUS &= ~ ST_DC;
		else
			I.STATUS |= ST_DC;
#endif

		setst_laeo(value);
		writewordX(addr, value, src_map);
		CYCLES(3, 12, 3);
		break;
	case 5:   /* INV */
		/* INV --- INVert */
		/* *S = ~*S */
		value = ~ readwordX(addr, src_map);
		writewordX(addr, value, src_map);
		setst_lae(value);
		CYCLES(2, 10, 3);
		break;
	case 6:   /* INC */
		/* INC --- INCrement */
		/* (*S)++ */
		value = setst_add_laeco(readwordX(addr, src_map), 1);
		writewordX(addr, value, src_map);
		CYCLES(2, 10, 3);
		break;
	case 7:   /* INCT */
		/* INCT -- INCrement by Two */
		/* (*S) +=2 */
		value = setst_add_laeco(readwordX(addr, src_map), 2);
		writewordX(addr, value, src_map);
		CYCLES(2, 10, 3);
		break;
	case 8:   /* DEC */
		/* DEC --- DECrement */
		/* (*S)-- */
		value = setst_sub_laeco(readwordX(addr, src_map), 1);
		writewordX(addr, value, src_map);
		CYCLES(2, 10, 3);
		break;
	case 9:   /* DECT */
		/* DECT -- DECrement by Two */
		/* (*S) -= 2 */
		value = setst_sub_laeco(readwordX(addr, src_map), 2);
		writewordX(addr, value, src_map);
		CYCLES(2, 10, 3);
		break;
	case 10:  /* BL */
		/* BL ---- Branch and Link */
		/* IP=S, R11=old IP */
#if ((TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID))
		(void) readwordX(addr, src_map);
#endif
		WRITEREG(R11, I.PC);
		I.PC = addr;
		CYCLES(3, 12, 5);
		break;
	case 11:  /* SWPB */
		/* SWPB -- SWaP Bytes */
		/* *S = swab(*S) */
		value = readwordX(addr, src_map);
		value = logical_right_shift(value, 8) | (value << 8);
		writewordX(addr, value, src_map);
		CYCLES(2, 10, 13);
		break;
	case 12:  /* SETO */
		/* SETO -- SET to Ones */
		/* *S = #$FFFF */
#if ((TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9980_ID))
		(void) readwordX(addr, src_map);
#endif
		writewordX(addr, 0xFFFF, src_map);
		CYCLES(2, 10, 3);
		break;
	case 13:  /* ABS */
		/* ABS --- ABSolute value */
		/* *S = |*S| */
		/* clearing ST_C seems to be necessary, although ABS will never set it. */
#if (TMS99XX_MODEL <= TMS9985_ID)

		/* ti990/10 and tms9900/tms9980 only write the result if it has changed */
		I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);

		#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
		/* I guess ST_DC is cleared here, too*/
		I.STATUS &= ~ ST_DC;
#endif

		value = readwordX(addr, src_map);

		CYCLES(5, 12, Mooof!);

		if (((INT16) value) > 0)
			I.STATUS |= ST_LGT | ST_AGT;
		else if (((INT16) value) < 0)
		{
			I.STATUS |= ST_LGT;
			if (value == 0x8000)
				I.STATUS |= ST_OV;

			#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
			if (! (value & 0x0FFF))
				I.STATUS |= ST_DC;
#endif

			writewordX(addr, - ((INT16) value), src_map);
			CYCLES(0, 2, Mooof!);
		}
		else
			I.STATUS |= ST_EQ;

		break;

#else

		/* tms9995 always write the result */
		I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);
		value = readwordX(addr, src_map);

		CYCLES(Mooof!, Mooof!, 3);
		if (((INT16) value) > 0)
			I.STATUS |= ST_LGT | ST_AGT;
		else if (((INT16) value) < 0)
		{
			I.STATUS |= ST_LGT;
			if (value == 0x8000)
				I.STATUS |= ST_OV;
			value = - ((INT16) value);
		}
		else
			I.STATUS |= ST_EQ;

		writewordX(addr, value, src_map);

		break;

#endif


#if HAS_MAPPING
	/* Used by the memory mapper on ti990/10 with mapping option, ti990/12, and the TIM99610
    mapper chip to be associated with tms99000. */
	/* These opcode allow access to another page without the need of switching a page someplace. */
	/* Note that, if I read the 990/10 schematics correctly, two consecutive LDS or LDD would
    cause some trouble.  */
	case 14:  /* LDS */
		/* LDS --- Long Distance Source */

		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif

		load_map_file(addr, src_map, 2);
		lds_flag = 1;
		I.disable_interrupt_recognition = 1;
		break;
	case 15:  /* LDD */
		/* LDD --- Long Distance Destination */
		#if HAS_PRIVILEGE
			if (I.STATUS & ST_PR)
			{
				HANDLE_PRIVILEGE_VIOLATION
				break;
			}
		#endif

		load_map_file(addr, src_map, 2);
		ldd_flag = 1;
		I.disable_interrupt_recognition = 1;
		break;

#else

	default:
		/* illegal instructions */
		HANDLE_ILLEGAL;
		break;

#endif
	}
}


/*==========================================================================
   Shift instructions,                                         >0800->0BFF
  --------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    | o p c o d e   |   C   |   W   |
    ---------------------------------

  SRA, SRL, SLA, SRC
============================================================================*/
static void h0800(UINT16 opcode)
{
	register UINT16 addr;
	register UINT16 cnt = (opcode & 0xF0) >> 4;
	register UINT16 value;

	addr = (opcode & 0xF);
	addr = ((addr+addr) + I.WP) & ~1;

	CYCLES(3, 12, 5);

	if (cnt == 0)
	{
		CYCLES(2, 8, 2);

		cnt = READREG(R0) & 0xF;

		if (cnt == 0)
			cnt = 16;
	}

	CYCLES(cnt, cnt+cnt, cnt);

	switch ((opcode & 0x300) >> 8)
	{
	case 0:   /* SRA */
		/* SRA --- Shift Right Arithmetic */
		/* *W >>= C   (*W is filled on the left with a copy of the sign bit) */
		value = setst_sra_laec(readword(addr), cnt);
		writeword(addr, value);
		break;
	case 1:   /* SRL */
		/* SRL --- Shift Right Logical */
		/* *W >>= C   (*W is filled on the left with 0) */
		value = setst_srl_laec(readword(addr), cnt);
		writeword(addr, value);
		break;
	case 2:   /* SLA */
		/* SLA --- Shift Left Arithmetic */
		/* *W <<= C */
		value = setst_sla_laeco(readword(addr), cnt);
		writeword(addr, value);
		break;
	case 3:   /* SRC */
		/* SRC --- Shift Right Circular */
		/* *W = rightcircularshift(*W, C) */
		value = setst_src_laec(readword(addr), cnt);
		writeword(addr, value);
		break;
	}
}


#if 0	/* 990/12 opcodes */
/*==========================================================================
   Additionnal instructions,                                   >0C00->0C0F
   Additionnal single-register instructions,                   >0C10->0C3F
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |     o p c o d e               |
    |     o p c o d e       | reg # |
    ---------------------------------

============================================================================*/
static void h0c00(UINT16 opcode)
{
	if (opcode & 0x30)
	{
		switch ((opcode & 0x30) >> 4)
		{
		case 1:
			/* INSF -- INSert Field */
			break;
		case 2:
			/* XV ---- eXtract Value */
			break;
		case 3:
			/* XF ---- eXtract Field */
			break;
		}
	}
	else
	{
		switch (opcode & 0x0F)
		{
		/* floating point instructions */
		case 0:
			/* CRI --- Convert Real to Integer */
			break;
		case 2:
			/* NEGR -- NEGate Real */
			break;
		case 4:
			/* CRE --- Convert Real to Extended integer */
			break;
		case 6:
			/* CER --- Convert Extended integer to Real */
			break;

		case 1:
			/* CDI --- Convert Double precision real to Integer */
			break;
		case 3:
			/* NEGD -- NEGate Double precision real */
			break;
		case 5:
			/* CDE --- Convert Double precision real to Extended integer */
			break;
		case 7:
			/* CED --- Convert Extended integer to Double precision real */
			break;

		case 8:
			/* NRM --- NoRMalize */
			break;

		case 9:
			/* TMB --- Test Memory Bit */
			break;
		case 10:
			/* TCMB -- Test and Clear Memory Bit */
			break;
		case 11:
			/* TSMB -- Test and Set Memory Bit */
			break;

	case 12:
			/* SRJ --- Subtract from Register and Jump */
		case 13:
			/* ARJ --- Add to Register and Jump */

		case 14:
		case 15:
			/* XIT --- eXIT from floating point interpreter */
			/* Generated by some compilers, but equivalent to NOP on TI990/12.  May have been used
            by some software floating-point emulators. */
			break;

		default:
			HANDLE_ILLEGAL;
			break;
		}
	}
}


/*==========================================================================
   Additionnal single-operand instructions,                    >0C40->0DFF
                                                               >0E40->0FFF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |    o p c o d e    |TS |   S   |
    ---------------------------------

============================================================================*/
static void h0c40(UINT16 opcode)
{
	register UINT16 src;

#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
#endif

	src = decipheraddr(opcode) & ~1;

	switch ((opcode & 0x03C0) >> 6)
	{
	case 1:
		/* AR ---- Add Real */
		break;
	case 2:
		/* CIR --- Convert Integer to Real */
		break;
	case 3:
		/* SR ---- Subtract Real */
		break;
	case 4:
		/* MR ---- Multiply Real */
		break;
	case 5:
		/* DR ---- Divide Real */
		break;
	case 6:
		/* LR ---- Load Real */
		break;
	case 7:
		/* STR --- STore Real */
		break;
	case 9:
		/* AD ---- Add Double */
		break;
	case 10:
		/* CID --- Convert Integer to Double */
		break;
	case 11:
		/* SD ---- Subtract Double */
		break;
	case 12:
		/* MD ---- Multiply Double */
		break;
	case 13:
		/* DD ---- Divide Double  */
		break;
	case 14:
		/* LD ---- Load Double */
		break;
	case 15:
		/* SD ---- Store Double */
		break;
	default:
		HANDLE_ILLEGAL;
		break;
	}
}

/*==========================================================================
   Additionnal single-register instructions,                   >0E00->0E3F
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |     o p c o d e       | reg # |
    ---------------------------------

============================================================================*/
static void h0e00(UINT16 opcode)
{
	switch ((opcode & 0x30) >> 4)
	{
	case 1:
		/* IOF --- Invert Order of Field */
		break;
	case 1:
		/* SNEB -- Search string for Not Equal Byte */
		break;
	case 2:
		/* CRC --- Cyclic Redundancy Code calculation */
		break;
	case 3:
		/* TS ---- Translate String */
		break;
	}
}
#endif


/*==========================================================================
   Jump, CRU bit instructions,                                 >1000->1FFF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |  o p c o d e  | signed offset |
    ---------------------------------

  JMP, JLT, JLE, JEQ, JHE, JGT, JNE, JNC, JOC, JNO, JL, JH, JOP
  SBO, SBZ, TB
============================================================================*/
static void h1000(UINT16 opcode)
{
	/* we convert 8 bit signed word offset to a 16 bit effective word offset. */
	register INT16 offset = ((INT8) opcode);


	switch ((opcode & 0xF00) >> 8)
	{
	case 0:   /* JMP */
		/* JMP --- unconditional JuMP */
		/* PC += offset */
		I.PC += (offset + offset);
		CYCLES(3, 10, 3);
		break;
	case 1:   /* JLT */
		/* JLT --- Jump if Less Than (arithmetic) */
		/* if (A==0 && EQ==0), PC += offset */
		if (! (I.STATUS & (ST_AGT | ST_EQ)))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 2:   /* JLE */
		/* JLE --- Jump if Lower or Equal (logical) */
		/* if (L==0 || EQ==1), PC += offset */
		if ((! (I.STATUS & ST_LGT)) || (I.STATUS & ST_EQ))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 3:   /* JEQ */
		/* JEQ --- Jump if EQual */
		/* if (EQ==1), PC += offset */
		if (I.STATUS & ST_EQ)
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 4:   /* JHE */
		/* JHE --- Jump if Higher or Equal (logical) */
		/* if (L==1 || EQ==1), PC += offset */
		if (I.STATUS & (ST_LGT | ST_EQ))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 5:   /* JGT */
		/* JGT --- Jump if Greater Than (arithmetic) */
		/* if (A==1), PC += offset */
		if (I.STATUS & ST_AGT)
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 6:   /* JNE */
		/* JNE --- Jump if Not Equal */
		/* if (EQ==0), PC += offset */
		if (! (I.STATUS & ST_EQ))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 7:   /* JNC */
		/* JNC --- Jump if No Carry */
		/* if (C==0), PC += offset */
		if (! (I.STATUS & ST_C))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 8:   /* JOC */
		/* JOC --- Jump On Carry */
		/* if (C==1), PC += offset */
		if (I.STATUS & ST_C)
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 9:   /* JNO */
		/* JNO --- Jump if No Overflow */
		/* if (OV==0), PC += offset */
		if (! (I.STATUS & ST_OV))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 10:  /* JL */
		/* JL ---- Jump if Lower (logical) */
		/* if (L==0 && EQ==0), PC += offset */
		if (! (I.STATUS & (ST_LGT | ST_EQ)))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 11:  /* JH */
		/* JH ---- Jump if Higher (logical) */
		/* if (L==1 && EQ==0), PC += offset */
		if ((I.STATUS & ST_LGT) && ! (I.STATUS & ST_EQ))
		{
			I.PC += (offset + offset);
			CYCLES(3, 10, 3);
		}
		else
			CYCLES(2, 8, 3);
		break;
	case 12:  /* JOP */
		/* JOP --- Jump On (odd) Parity */
		/* if (P==1), PC += offset */
		{
			/* Let's set ST_OP. */
			int i;
			UINT8 a;
				a = lastparity;
			i = 0;

			while (a != 0)
			{
				if (a & 1)  /* If current bit is set, */
					i++;      /* increment bit count. */
				a >>= 1U;   /* Next bit. */
			}

			/* Set ST_OP bit. */
			/*if (i & 1)
                I.STATUS |= ST_OP;
            else
                I.STATUS &= ~ ST_OP;*/

			/* Jump accordingly. */
			if (i & 1)  /*(I.STATUS & ST_OP)*/
			{
				I.PC += (offset + offset);
				CYCLES(3, 10, 3);
			}
			else
				CYCLES(2, 8, 3);
		}

		break;
	case 13:  /* SBO */
		/* SBO --- Set Bit to One */
		/* CRU Bit = 1 */
		#if HAS_PRIVILEGE
			if (writeCRU((READREG(R12) >> 1) + offset, 1, 1) == CRU_PRIVILEGE_VIOLATION)
				HANDLE_PRIVILEGE_VIOLATION
		#else
		writeCRU((READREG(R12) >> 1) + offset, 1, 1);
		#endif

		CYCLES(4, 12, 8);
		break;

	case 14:  /* SBZ */
		/* SBZ --- Set Bit to Zero */
		/* CRU Bit = 0 */
		#if HAS_PRIVILEGE
			if (writeCRU((READREG(R12) >> 1) + offset, 1, 0) == CRU_PRIVILEGE_VIOLATION)
				HANDLE_PRIVILEGE_VIOLATION
		#else
		writeCRU((READREG(R12) >> 1) + offset, 1, 0);
		#endif

		CYCLES(4, 12, 8);
		break;

	case 15:  /* TB */
		/* TB ---- Test Bit */
		/* EQ = (CRU Bit == 1) */
		#if HAS_PRIVILEGE
			{
				int value;

				value = readCRU((READREG(R12)>> 1) + offset, 1);
				if (value == CRU_PRIVILEGE_VIOLATION)
					HANDLE_PRIVILEGE_VIOLATION
				else
					setst_e(value & 1, 1);
			}
		#else
		setst_e(readCRU((READREG(R12)>> 1) + offset, 1) & 1, 1);
		#endif
		CYCLES(5, 12, 8);
		break;
	}
}


/*==========================================================================
   General and One-Register instructions                       >2000->3FFF
 ---------------------------------------------------------------------------

     0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ---------------------------------
    |  opcode   |   D   |TS |   S   |
    ---------------------------------

  COC, CZC, XOR, LDCR, STCR, XOP, MPY, DIV
tms9940 : DCA, DCS, LIIM
==========================================================================*/

/* xop, ldcr and stcr are handled elsewhere */
static void h2000(UINT16 opcode)
{
	register UINT16 dest = (opcode & 0x3C0) >> 6;
	register UINT16 src;
	register UINT16 value;

#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
#endif

	src = decipheraddr(opcode) & ~1;
	dest = ((dest+dest) + I.WP) & ~1;

	switch ((opcode & 0x1C00) >> 10)
	{
	case 0:   /* COC */
		/* COC --- Compare Ones Corresponding */
		/* status E bit = (S&D == S) */
		value = readwordX(src, src_map);
		setst_e(value & readword(dest), value);
		CYCLES(5, 14, 4);
		break;
	case 1:   /* CZC */
		/* CZC --- Compare Zeroes Corresponding */
		/* status E bit = (S&~D == S) */
		value = readwordX(src, src_map);
		setst_e(value & (~ readword(dest)), value);
		CYCLES(5, 14, 4);
		break;
	case 2:   /* XOR */
		/* XOR --- eXclusive OR */
		/* D ^= S */
		value = readwordX(src, src_map);
		value ^= readword(dest);
		setst_lae(value);
		writeword(dest,value);
		CYCLES(3, 14, 4);
		break;
	/*case 3:*/   /* XOP is implemented elsewhere */
	/*case 4:*/   /* LDCR is implemented elsewhere */
	/*case 5:*/   /* STCR is implemented elsewhere */
	case 6:   /* MPY */
		/* MPY --- MultiPlY  (unsigned) */
		/* Results:  D:D+1 = D*S */
		/* Note that early TMS9995 reportedly performs an extra dummy read in PC space */
		{
			unsigned long prod = ((unsigned long) readwordX(src, src_map));
			prod = prod * ((unsigned long) readword(dest));
			writeword(dest, prod >> 16);
			writeword((dest+2)&0xffff, prod);
		}
		/* ti990/10 : from 19 to 35, possibly 19 + (number of bits to 1 in one operand) */
		CYCLES(35, 52, 23);
		break;
	case 7:   /* DIV */
		/* DIV --- DIVide    (unsigned) */
		/* D = D/S    D+1 = D%S */
		{
			UINT16 d = readwordX(src, src_map);
			UINT16 hi = readword(dest);
			unsigned long divq = (((unsigned long) hi) << 16) | readword((dest+2)&0xffff);

			if (d <= hi)
			{
				I.STATUS |= ST_OV;
				CYCLES(4, 16, 6);
			}
			else
			{
				I.STATUS &= ~ST_OV;
				writeword(dest, divq/d);
				writeword((dest+2)&0xffff, divq%d);
				/* tms9900 : from 92 to 124, possibly 92 + 2*(number of bits to 1 (or 0?) in quotient) */
				/* tms9995 : 28 is the worst case */
				/* ti990/10 : from 41 to 58, possibly 41 + (number of bits to 1 (or 0?) in quotient) */
				CYCLES(41, 92, 28);
			}
		}
		break;
	}
}

static void xop(UINT16 opcode)
{	/* XOP */
	/* XOP --- eXtended OPeration */
	/* WP = *(40h+D), PC = *(42h+D) */
	/* New R13=old WP, New R14=Old IP, New R15=Old ST */
	/* New R11=S */
	/* Xop bit set */

	register UINT16 immediate = (opcode & 0x3C0) >> 6;
	register UINT16 operand;


#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
		switch (immediate)
		{
		case 0: /* DCA */
			/* DCA --- Decimal Correct Addition */
			operand = decipheraddrbyte(opcode);
			{
			int value = readbyte(operand);
			int X = (value >> 4) & 0xf;
			int Y = value & 0xf;

			if (Y >= 10)
			{
				Y -= 10;
				I.STATUS |= ST_DC;
				X++;
			}
			else if (I.STATUS & ST_DC)
			{
				Y += 6;
			}

			if (X >= 10)
			{
				X -= 10;
				I.STATUS |= ST_C;
			}
			else if (I.STATUS & ST_C)
			{
				X += 6;
			}

			writebyte(operand, (X << 4) | Y);
			}
			break;
		case 1:	/* DCS */
			/* DCS --- Decimal Correct Substraction */
			operand = decipheraddrbyte(opcode);
			{
			int value = readbyte(operand);

			if (! (I.STATUS & ST_DC))
			{
				value += 10;
			}

			if (! (I.STATUS & ST_C))
			{
				value += 10 << 4;
			}

			I.STATUS ^= ST_DC;

			writebyte(operand, value);
			}
			break;
		case 2: /* LIIM */
		case 3: /* LIIM */
			/* LIIM - Load Immediate Interrupt Mask */
			/* Does the same job as LIMI, with a different opcode format. */
			/* Note that, unlike TMS9900, the interrupt mask is only 2-bit long. */
		(void)decipheraddr(opcode);	/* dummy decode (personnal guess) */

		I.STATUS = (I.STATUS & ~ST_IM) | (opcode & ST_IM);
			break;
		default:  /* normal XOP */
#endif

	/* TODO : emulate 990/10 hardware XOP */
	operand = decipheraddr(opcode);

	#if ((TMS99XX_MODEL <= TMS9989_ID) && (TMS99XX_MODEL != TI990_10_ID))
		(void)readword(operand & ~1); /*dummy read (personnal guess)*/
#endif

	contextswitchX(0x40 + (immediate << 2));

	#if ! ((TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID))
		/* The bit is not set on tms9940 */
		I.STATUS |= ST_X;
#endif

		WRITEREG(R11, operand);
	CYCLES(7, 36, 15);
	I.disable_interrupt_recognition = 1;

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
			break;
		}
#endif
}

/* LDCR and STCR */
static void ldcr_stcr(UINT16 opcode)
{
	register UINT16 cnt = (opcode & 0x3C0) >> 6;
	register UINT16 addr;
	int value;

#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
#endif

	if (cnt == 0)
		cnt = 16;

	if (cnt <= 8)
		addr = decipheraddrbyte(opcode);
	else
		addr = decipheraddr(opcode) & ~1;

	if (opcode < 0x3400)
	{	/* LDCR */
		/* LDCR -- LoaD into CRu */
		/* CRU R12--CRU R12+D-1 set to S */
		if (cnt <= 8)
		{
#if (TMS99XX_MODEL != TMS9995_ID)
				value = readbyteX(addr, src_map);
#else
			/* just for once, tms9995 behaves like earlier 8-bit tms99xx chips */
			/* this must be because instruction decoding is too complex */
				value = readwordX(addr & ~1, src_map);
			if (addr & 1)
				value &= 0xFF;
			else
				value = (value >> 8) & 0xFF;
#endif

			(void)READREG(cnt+cnt); /*dummy read (reasonnable guess for TMS9995 & TMS9900, ti990/10)*/

			setst_byte_laep(value);
		}
		else
		{
			value = readwordX(addr, src_map);

			(void)READREG(cnt+cnt); /*dummy read (reasonnable guess for TMS9995 & TMS9900, ti990/10)*/

			setst_lae(value);
		}

		#if HAS_PRIVILEGE
			if (writeCRU((READREG(R12) >> 1), cnt, value) == CRU_PRIVILEGE_VIOLATION)
				HANDLE_PRIVILEGE_VIOLATION
		#else
			writeCRU((READREG(R12) >> 1), cnt, value);
		#endif

		CYCLES(4+cnt, 20 + cnt+cnt, 9 + cnt+cnt);
	}
	else
	{	/* STCR */
		/* STCR -- STore from CRu */
		/* S = CRU R12--CRU R12+D-1 */
		if (cnt <= 8)
		{
#if (TMS99XX_MODEL != TMS9995_ID)
				(void)readbyteX(addr, src_map);	/*dummy read*/

				(void)READREG(cnt+cnt); /*dummy read (reasonnable guess for TMS9995 & TMS9900, ti990/10)*/

				#if HAS_PRIVILEGE
					value = readCRU((READREG(R12) >> 1), cnt);
					if (value == CRU_PRIVILEGE_VIOLATION)
						HANDLE_PRIVILEGE_VIOLATION
					else
					{
						setst_byte_laep(value);
						writebyteX(addr, value, src_map);
					}
				#else
			value = readCRU((READREG(R12) >> 1), cnt);
			setst_byte_laep(value);
					writebyteX(addr, value, src_map);
				#endif
				CYCLES(18+cnt, (cnt != 8) ? 42 : 44, 19 + cnt);
#else
			/* just for once, tms9995 behaves like earlier 8-bit tms99xx chips */
			/* this must be because instruction decoding is too complex */
				int value2 = readwordX(addr & ~1, src_map);

				(void)READREG(cnt+cnt); /*dummy read (reasonnable guess for TMS9995 & TMS9900, ti990/10)*/

			value = readCRU((READREG(R12) >> 1), cnt);
			setst_byte_laep(value);

			if (addr & 1)
					writewordX(addr & ~1, (value & 0x00FF) | (value2 & 0xFF00), src_map);
			else
					writewordX(addr & ~1, (value2 & 0x00FF) | ((value << 8) & 0xFF00), src_map);

				CYCLES(Mooof!, Mooof!, 19 + cnt);
#endif
		}
		else
		{
			(void)readwordX(addr, src_map);	/*dummy read*/

			(void)READREG(cnt+cnt); /*dummy read (reasonnable guess for TMS9995 & TMS9900, ti990/10)*/

			#if HAS_PRIVILEGE
				value = readCRU((READREG(R12) >> 1), cnt);
				if (value == CRU_PRIVILEGE_VIOLATION)
					HANDLE_PRIVILEGE_VIOLATION
				else
				{
					setst_lae(value);
					writewordX(addr, value, src_map);
				}
			#else
			value = readCRU((READREG(R12) >> 1), cnt);
			setst_lae(value);
				writewordX(addr, value, src_map);
			#endif
			CYCLES(24+cnt, (cnt != 16) ? 58 : 60, 27 + cnt);
		}
	}
}


/*==========================================================================
   Two-Operand instructions                                    >4000->FFFF
 ---------------------------------------------------------------------------

      0 1 2 3-4 5 6 7+8 9 A B-C D E F
    ----------------------------------
    |opcode|B|TD |   D   |TS |   S   |
    ----------------------------------

  SZC, SZCB, S, SB, C, CB, A, AB, MOV, MOVB, SOC, SOCB
============================================================================*/

/* word instructions */
static void h4000w(UINT16 opcode)
{
	register UINT16 src;
	register UINT16 dest;
	register UINT16 value;

#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
	int dst_map = (opcode & 0x0c00) ? I.cur_dst_map : I.cur_map;
#endif

	src = decipheraddr(opcode) & ~1;
	dest = decipheraddr(opcode >> 6) & ~1;

	value = readwordX(src, src_map);

	switch ((opcode >> 13) & 0x0007)    /* ((opcode & 0xE000) >> 13) */
	{
	case 2:   /* SZC */
		/* SZC --- Set Zeros Corresponding */
		/* D &= ~S */
		value = readwordX(dest, dst_map) & (~ value);
		setst_lae(value);
		writewordX(dest, value, dst_map);
		CYCLES(2, 14, 4);
		break;
	case 3:   /* S */
		/* S ----- Subtract */
		/* D -= S */
		value = setst_sub_laeco(readwordX(dest, dst_map), value);
		writewordX(dest, value, dst_map);
		CYCLES(2, 14, 4);
		break;
	case 4:   /* C */
		/* C ----- Compare */
		/* ST = (D - S) */
		setst_c_lae(readwordX(dest, dst_map), value);
		CYCLES(5, 14, 4);
		break;
	case 5:   /* A */
		/* A ----- Add */
		/* D += S */
		value = setst_add_laeco(readwordX(dest, dst_map), value);
		writewordX(dest, value, dst_map);
		CYCLES(2, 14, 4);
		break;
	case 6:   /* MOV */
		/* MOV --- MOVe */
		/* D = S */
		setst_lae(value);
		#if ((TMS99XX_MODEL >= TMS9900_ID) && (TMS99XX_MODEL <= TMS9985_ID))
			/* MOV performs a dummy read with tms9900/9980 (but neither ti990/10 nor tms9995) */
			(void)readwordX(dest, dst_map);
#endif
		writewordX(dest, value, dst_map);
		CYCLES(1, 14, 3);
		break;
	case 7:   /* SOC */
		/* SOC --- Set Ones Corresponding */
		/* D |= S */
		value = value | readwordX(dest, dst_map);
		setst_lae(value);
		writewordX(dest, value, dst_map);
		CYCLES(2, 14, 4);
		break;
	}
}

/* byte instruction */
static void h4000b(UINT16 opcode)
{
	register UINT16 src;
	register UINT16 dest;
	register UINT16 value;

#if HAS_MAPPING
	int src_map = (opcode & 0x0030) ? I.cur_src_map : I.cur_map;
	int dst_map = (opcode & 0x0c00) ? I.cur_dst_map : I.cur_map;
#endif

	src = decipheraddrbyte(opcode);
	dest = decipheraddrbyte(opcode >> 6);

	value = readbyteX(src, src_map);

	switch ((opcode >> 13) & 0x0007)    /* ((opcode & 0xE000) >> 13) */
	{
	case 2:   /* SZCB */
		/* SZCB -- Set Zeros Corresponding, Byte */
		/* D &= ~S */
		value = readbyteX(dest, dst_map) & (~ value);
		setst_byte_laep(value);
		writebyteX(dest, value, dst_map);
		CYCLES(3, 14, 4);
		break;
	case 3:   /* SB */
		/* SB ---- Subtract, Byte */
		/* D -= S */
		value = setst_subbyte_laecop(readbyteX(dest, dst_map), value);
		writebyteX(dest, value, dst_map);
		CYCLES(3, 14, 4);
		break;
	case 4:   /* CB */
		/* CB ---- Compare Bytes */
		/* ST = (D - S) */
		setst_c_lae(readbyteX(dest, dst_map)<<8, value<<8);
		lastparity = value;
		CYCLES(5, 14, 4);
		break;
	case 5:   /* AB */
		/* AB ---- Add, Byte */
		/* D += S */
		value = setst_addbyte_laecop(readbyteX(dest, dst_map), value);
		writebyteX(dest, value, dst_map);
		CYCLES(3, 14, 4);
		break;
	case 6:   /* MOVB */
		/* MOVB -- MOVe Bytes */
		/* D = S */
		setst_byte_laep(value);
		#if (TMS99XX_MODEL <= TMS9985_ID)
			/* On ti990/10 and tms9900, MOVB needs to read destination, because it cannot actually
              read one single byte.  It reads a word, replaces the revelant byte, then write
              the result.  A tms9980 should not need to do so, but still does, because it is just
              a tms9900 with a 16 to 8 bit multiplexer (instead of a new chip design, like tms9995). */
			(void)readbyteX(dest, dst_map);
#endif
		writebyteX(dest, value, dst_map);
		CYCLES(3, 14, 3);
		break;
	case 7:   /* SOCB */
		/* SOCB -- Set Ones Corresponding, Byte */
		/* D |= S */
		value = value | readbyteX(dest, dst_map);
		setst_byte_laep(value);
		writebyteX(dest, value, dst_map);
		CYCLES(3, 14, 4);
		break;
	}
}


INLINE void execute(UINT16 opcode)
{
#if (! HAS_9995_OPCODES)

	/* tms9900-like instruction set*/

	static void (*const jumptable[128])(UINT16) =
	{
		&illegal,&h0200,&h0400,&h0400,&h0800,&h0800,&illegal,&illegal,
		&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,
		&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,&xop,&xop,
		&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&h2000,&h2000,&h2000,&h2000,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b
	};

	(* jumptable[opcode >> 9])(opcode);

#else

	/* tms9989 and tms9995 include 4 extra instructions, and one additionnal instruction type */
	/* tms99000 includes yet another additional instruction */

	static void (*const jumptable[256])(UINT16) =
	{
		&h0040,&h0100,&h0200,&h0200,&h0400,&h0400,&h0400,&h0400,
		&h0800,&h0800,&h0800,&h0800,&illegal,&illegal,&illegal,&illegal,
		&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,
		&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,&h1000,
		&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,
		&h2000,&h2000,&h2000,&h2000,&xop,&xop,&xop,&xop,
		&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,&ldcr_stcr,
		&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,&h2000,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,&h4000w,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,
		&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b,&h4000b
	};

	(* jumptable[opcode >> 8])(opcode);

#endif
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void tms99xx_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		tms99xx_set_irq_line(INPUT_LINE_NMI, info->i); break;
		case CPUINFO_INT_INPUT_STATE + 0:					tms99xx_set_irq_line(0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + 1:					tms99xx_set_irq_line(1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + 2:					tms99xx_set_irq_line(2, info->i);		break;

#if (TMS99XX_MODEL == TI990_10_ID)
		case CPUINFO_INT_PC:
			{
				const unsigned top = (I.cur_map == 0) ? 0xf800 : 0x10000;

				if ((I.cur_map == 0) && (info->i >= 0x1ff800))
					/* intercept TPCS and CPU ROM */
					I.PC = info->i - 0x1f0000;
				else if (! I.mapping_on)
					I.PC = (info->i < top) ? info->i : 0;
				else
				{
					if ((info->i >= I.map_files[I.cur_map].bias[0])
							&& (info->i <= (I.map_files[I.cur_map].bias[0]+I.map_files[I.cur_map].limit[0])))
						I.PC = info->i - I.map_files[I.cur_map].bias[0];
					else if ((info->i > (I.map_files[I.cur_map].bias[1]+I.map_files[I.cur_map].limit[0]))
							&& (info->i <= (I.map_files[I.cur_map].bias[1]+I.map_files[I.cur_map].limit[1])))
						I.PC = info->i - I.map_files[I.cur_map].bias[1];
					else if ((info->i > (I.map_files[I.cur_map].bias[2]+I.map_files[I.cur_map].limit[0]))
							&& (info->i > (I.map_files[I.cur_map].bias[2]+I.map_files[I.cur_map].limit[1]))
							&& (info->i <= (I.map_files[I.cur_map].bias[2]+I.map_files[I.cur_map].limit[2])))
						I.PC = info->i - I.map_files[I.cur_map].bias[2];
					else
					{
						/*if ((info->i < top)
                                && (info->i > I.map_files[I.cur_map].limit[0])
                                && (info->i > I.map_files[I.cur_map].limit[1])
                                && (info->i > I.map_files[I.cur_map].limit[2]))
                            I.PC = info->i;
                        else*/
							I.PC = 0;
					}
					/*if (info->i >= top)
                        I.PC = 0;*/
				}
				I.PC &= 0xfffe;
			}
			break;
#else
		case CPUINFO_INT_PC:
#endif
		case CPUINFO_INT_REGISTER + TMS9900_PC:			I.PC = info->i & 0xfffe;				break;
		case CPUINFO_INT_REGISTER + TMS9900_IR:			I.IR = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS9900_WP:			I.WP = info->i & 0xfffe;				break;
		case CPUINFO_INT_REGISTER + TMS9900_STATUS:		I.STATUS = info->i; getstat();			break;
		case CPUINFO_INT_REGISTER + TMS9900_R0:			WRITEREG_DEBUG(R0, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R1:			WRITEREG_DEBUG(R1, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R2:			WRITEREG_DEBUG(R2, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R3:			WRITEREG_DEBUG(R3, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R4:			WRITEREG_DEBUG(R4, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R5:			WRITEREG_DEBUG(R5, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R6:			WRITEREG_DEBUG(R6, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R7:			WRITEREG_DEBUG(R7, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R8:			WRITEREG_DEBUG(R8, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R9:			WRITEREG_DEBUG(R9, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R10:		WRITEREG_DEBUG(R10, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R11:		WRITEREG_DEBUG(R11, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R12:		WRITEREG_DEBUG(R12, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R13:		WRITEREG_DEBUG(R13, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R14:		WRITEREG_DEBUG(R14, info->i);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R15:		WRITEREG_DEBUG(R15, info->i);			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void TMS99XX_GET_INFO(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(I);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;/*8 with 990/12, 99105, 99110*/break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;/*TODO: compute this value*/break;

#if (USE_16_BIT_ACCESSORS)
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
#else
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
#endif
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:
#if (TMS99XX_MODEL == TI990_10_ID)
			/* this CPU has a mapper to expand the address space */
			info->i = 21;
#elif (TMS99XX_MODEL == TMS9900_ID) || (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID) || (TMS99XX_MODEL == TMS9995_ID)
			/* basic 16-bit address bus */
			info->i = 16;
#elif (TMS99XX_MODEL == TMS9980_ID)
			/* 2 address MSBits are unconnected */
			info->i = 14;
#else
			#warning "Please check how many address bits your CPU has."
			info->i = 16;
#endif
			break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:
#if (TMS99XX_MODEL == TI990_10_ID)
			/* 3 MSBs do exist, although they are not connected (don't ask...) */
			info->i = 15;
#elif (TMS99XX_MODEL == TMS9900_ID)
			/* 3 MSBs are external instruction code */
			info->i = 12;
#elif (TMS99XX_MODEL == TMS9980_ID)
			/* 2 bits unused, and 2 MSBs are external instruction code */
			info->i = 13;
#elif (TMS99XX_MODEL == TMS9940_ID)
			/* 9 internal address lines (8 external) */
			info->i = 9;
#elif (TMS99XX_MODEL == TMS9995_ID)
			/* 15-bit address and data bus lines D0-D2 provide the external instruction code */
			info->i = 15+3;
#else
			#warning "I don't know how your processor handles CRU."
			info->i = 15;
#endif
			break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

/* not implemented */
/*      case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:  info->i = get_irq_line(INPUT_LINE_NMI); break;
        case CPUINFO_INT_INPUT_STATE + 0:               info->i = get_irq_line(0);              break;
        case CPUINFO_INT_INPUT_STATE + 1:               info->i = get_irq_line(1);              break;
        case CPUINFO_INT_INPUT_STATE + 2:               info->i = get_irq_line(2);              break;*/

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

#if (TMS99XX_MODEL == TI990_10_ID)
		case CPUINFO_INT_PC:
			if ((I.cur_map == 0) && (I.PC >= 0xf800))
				/* intercept TPCS and CPU ROM */
				info->i = 0x1f0000+I.PC;
			else if (! I.mapping_on)
				info->i = I.PC;
			else
			{
				int map_index;

				if (I.PC <= I.map_files[I.cur_map].limit[0])
					map_index = 0;
				else if (I.PC <= I.map_files[I.cur_map].limit[1])
					map_index = 1;
				else if (I.PC <= I.map_files[I.cur_map].limit[2])
					map_index = 2;
				else
				{
					info->i = I.PC;
					break;
				}

				info->i = I.map_files[I.cur_map].bias[map_index]+I.PC;
			}
			break;
#else
		case CPUINFO_INT_PC:
#endif
		case CPUINFO_INT_REGISTER + TMS9900_PC:			info->i = I.PC;							break;
		case CPUINFO_INT_REGISTER + TMS9900_IR:			info->i = I.IR;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS9900_WP:			info->i = I.WP;							break;
		case CPUINFO_INT_REGISTER + TMS9900_STATUS:		setstat(); info->i = I.STATUS;			break;
		case CPUINFO_INT_REGISTER + TMS9900_R0:			info->i = READREG_DEBUG(R0);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R1:			info->i = READREG_DEBUG(R1);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R2:			info->i = READREG_DEBUG(R2);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R3:			info->i = READREG_DEBUG(R3);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R4:			info->i = READREG_DEBUG(R4);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R5:			info->i = READREG_DEBUG(R5);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R6:			info->i = READREG_DEBUG(R6);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R7:			info->i = READREG_DEBUG(R7);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R8:			info->i = READREG_DEBUG(R8);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R9:			info->i = READREG_DEBUG(R9);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R10:		info->i = READREG_DEBUG(R10);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R11:		info->i = READREG_DEBUG(R11);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R12:		info->i = READREG_DEBUG(R12);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R13:		info->i = READREG_DEBUG(R13);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R14:		info->i = READREG_DEBUG(R14);			break;
		case CPUINFO_INT_REGISTER + TMS9900_R15:		info->i = READREG_DEBUG(R15);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = tms99xx_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = tms99xx_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = tms99xx_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = tms99xx_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = tms99xx_reset;			break;
		case CPUINFO_PTR_EXIT:							info->exit = tms99xx_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = tms99xx_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = tms99xx_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &TMS99XX_ICOUNT;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, TMS99XX_CPU_NAME);		break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Texas Instruments 9900"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "2.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "C TMS9900 emulator by Edward Swartz, initially converted for Mame by M.Coates, updated by R. Nabet"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				I.WP & 0x8000 ? 'L':'.',
				I.WP & 0x4000 ? 'A':'.',
				I.WP & 0x2000 ? 'E':'.',
				I.WP & 0x1000 ? 'C':'.',
				I.WP & 0x0800 ? 'V':'.',
				I.WP & 0x0400 ? 'P':'.',
				I.WP & 0x0200 ? 'X':'.',
				I.WP & 0x0100 ? '?':'.',
				I.WP & 0x0080 ? '?':'.',
				I.WP & 0x0040 ? '?':'.',
				I.WP & 0x0020 ? '?':'.',
				I.WP & 0x0010 ? '?':'.',
				I.WP & 0x0008 ? 'I':'.',
				I.WP & 0x0004 ? 'I':'.',
				I.WP & 0x0002 ? 'I':'.',
				I.WP & 0x0001 ? 'I':'.');
			break;

		case CPUINFO_STR_REGISTER + TMS9900_PC:			sprintf(info->s, "PC :%04X",  I.PC); break;
		case CPUINFO_STR_REGISTER + TMS9900_IR:			sprintf(info->s, "IR :%04X",  I.IR); break;
		case CPUINFO_STR_REGISTER + TMS9900_WP:			sprintf(info->s, "WP :%04X",  I.WP); break;
		case CPUINFO_STR_REGISTER + TMS9900_STATUS:		sprintf(info->s, "ST :%04X",  I.STATUS); break;

		case CPUINFO_STR_REGISTER + TMS9900_R0:			sprintf(info->s, "R0 :%04X",  READREG_DEBUG(R0)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R1:			sprintf(info->s, "R1 :%04X",  READREG_DEBUG(R1)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R2:			sprintf(info->s, "R2 :%04X",  READREG_DEBUG(R2)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R3:			sprintf(info->s, "R3 :%04X",  READREG_DEBUG(R3)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R4:			sprintf(info->s, "R4 :%04X",  READREG_DEBUG(R4)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R5:			sprintf(info->s, "R5 :%04X",  READREG_DEBUG(R5)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R6:			sprintf(info->s, "R6 :%04X",  READREG_DEBUG(R6)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R7:			sprintf(info->s, "R7 :%04X",  READREG_DEBUG(R7)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R8:			sprintf(info->s, "R8 :%04X",  READREG_DEBUG(R8)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R9:			sprintf(info->s, "R9 :%04X",  READREG_DEBUG(R9)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R10:		sprintf(info->s, "R10:%04X",  READREG_DEBUG(R10)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R11:		sprintf(info->s, "R11:%04X",  READREG_DEBUG(R11)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R12:		sprintf(info->s, "R12:%04X",  READREG_DEBUG(R12)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R13:		sprintf(info->s, "R13:%04X",  READREG_DEBUG(R13)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R14:		sprintf(info->s, "R14:%04X",  READREG_DEBUG(R14)); break;
		case CPUINFO_STR_REGISTER + TMS9900_R15:		sprintf(info->s, "R15:%04X",  READREG_DEBUG(R15)); break;
	}
}
