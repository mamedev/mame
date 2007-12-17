/***************************************************************************

    r3000.c
    Core implementation for the portable MIPS R3000 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "debugger.h"
#include "r3000.h"


#define ENABLE_OVERFLOWS	0


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define COP0_Index			0
#define COP0_Random			1
#define COP0_EntryLo		2
#define COP0_Context		4
#define COP0_BadVAddr		8
#define COP0_Status			12
#define COP0_Cause			13
#define COP0_EPC			14
#define COP0_PRId			15

#define SR_IEc				0x00000001
#define SR_KUc				0x00000002
#define SR_IEp				0x00000004
#define SR_KUp				0x00000008
#define SR_IEo				0x00000010
#define SR_KUo				0x00000020
#define SR_IMSW0			0x00000100
#define SR_IMSW1			0x00000200
#define SR_IMEX0			0x00000400
#define SR_IMEX1			0x00000800
#define SR_IMEX2			0x00001000
#define SR_IMEX3			0x00002000
#define SR_IMEX4			0x00004000
#define SR_IMEX5			0x00008000
#define SR_IsC				0x00010000
#define SR_SwC				0x00020000
#define SR_PZ				0x00040000
#define SR_CM				0x00080000
#define SR_PE				0x00100000
#define SR_TS				0x00200000
#define SR_BEV				0x00400000
#define SR_RE				0x02000000
#define SR_COP0				0x10000000
#define SR_COP1				0x20000000
#define SR_COP2				0x40000000
#define SR_COP3				0x80000000

#define EXCEPTION_INTERRUPT	0
#define EXCEPTION_TLBMOD	1
#define EXCEPTION_TLBLOAD	2
#define EXCEPTION_TLBSTORE	3
#define EXCEPTION_ADDRLOAD	4
#define EXCEPTION_ADDRSTORE	5
#define EXCEPTION_BUSINST	6
#define EXCEPTION_BUSDATA	7
#define EXCEPTION_SYSCALL	8
#define EXCEPTION_BREAK		9
#define EXCEPTION_INVALIDOP	10
#define EXCEPTION_BADCOP	11
#define EXCEPTION_OVERFLOW	12
#define EXCEPTION_TRAP		13



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define RSREG		((op >> 21) & 31)
#define RTREG		((op >> 16) & 31)
#define RDREG		((op >> 11) & 31)
#define SHIFT		((op >> 6) & 31)

#define RSVAL		(r3000.r[RSREG])
#define RTVAL		(r3000.r[RTREG])
#define RDVAL		(r3000.r[RDREG])

#define SIMMVAL		((INT16)op)
#define UIMMVAL		((UINT16)op)
#define LIMMVAL		(op & 0x03ffffff)

#define ADDPC(x)	r3000.nextpc = r3000.pc + ((x) << 2)
#define ADDPCL(x,l)	{ r3000.nextpc = r3000.pc + ((x) << 2); r3000.r[l] = r3000.pc + 4; }
#define ABSPC(x)	r3000.nextpc = (r3000.pc & 0xf0000000) | ((x) << 2)
#define ABSPCL(x,l)	{ r3000.nextpc = (r3000.pc & 0xf0000000) | ((x) << 2); r3000.r[l] = r3000.pc + 4; }
#define SETPC(x)	r3000.nextpc = (x)
#define SETPCL(x,l)	{ r3000.nextpc = (x); r3000.r[l] = r3000.pc + 4; }

#define RBYTE(x)	(*r3000.cur.readbyte)(x)
#define RWORD(x)	(*r3000.cur.readword)(x)
#define RLONG(x)	(*r3000.cur.readlong)(x)

#define WBYTE(x,v)	(*r3000.cur.writebyte)(x,v)
#define WWORD(x,v)	(*r3000.cur.writeword)(x,v)
#define WLONG(x,v)	(*r3000.cur.writelong)(x,v)

#define HIVAL		r3000.hi
#define LOVAL		r3000.lo
#define SR			r3000.cpr[0][COP0_Status]
#define CAUSE		r3000.cpr[0][COP0_Cause]



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* memory access function table */
typedef struct
{
	UINT8		(*readbyte)(offs_t);
	UINT16	(*readword)(offs_t);
	UINT32	(*readlong)(offs_t);
	void		(*writebyte)(offs_t, UINT8);
	void		(*writeword)(offs_t, UINT16);
	void		(*writelong)(offs_t, UINT32);
} memory_handlers;

/* R3000 Registers */
typedef struct
{
	/* core registers */
	UINT32		pc;
	UINT32		hi;
	UINT32		lo;
	UINT32		r[32];

	/* COP registers */
	UINT32		cpr[4][32];
	UINT32		ccr[4][32];
	UINT8		cf[4];

	/* internal stuff */
	UINT32		ppc;
	UINT32		nextpc;
	int			op;
	int			interrupt_cycles;
	int			hasfpu;
	int 		(*irq_callback)(int irqline);

	/* endian-dependent load/store */
	void		(*lwl)(UINT32 op);
	void		(*lwr)(UINT32 op);
	void		(*swl)(UINT32 op);
	void		(*swr)(UINT32 op);

	/* memory accesses */
	UINT8		bigendian;
	memory_handlers cur;
	const memory_handlers *memory_hand;
	const memory_handlers *cache_hand;

	/* cache memory */
	UINT32 *	cache;
	UINT32 *	icache;
	UINT32 *	dcache;
	size_t		cache_size;
	size_t		icache_size;
	size_t		dcache_size;
} r3000_regs;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void lwl_be(UINT32 op);
static void lwr_be(UINT32 op);
static void swl_be(UINT32 op);
static void swr_be(UINT32 op);

static void lwl_le(UINT32 op);
static void lwr_le(UINT32 op);
static void swl_le(UINT32 op);
static void swr_le(UINT32 op);

static UINT8 readcache_be(offs_t offset);
static UINT16 readcache_be_word(offs_t offset);
static UINT32 readcache_be_dword(offs_t offset);
static void writecache_be(offs_t offset, UINT8 data);
static void writecache_be_word(offs_t offset, UINT16 data);
static void writecache_be_dword(offs_t offset, UINT32 data);

static UINT8 readcache_le(offs_t offset);
static UINT16 readcache_le_word(offs_t offset);
static UINT32 readcache_le_dword(offs_t offset);
static void writecache_le(offs_t offset, UINT8 data);
static void writecache_le_word(offs_t offset, UINT16 data);
static void writecache_le_dword(offs_t offset, UINT32 data);


/***************************************************************************
    PUBLIC GLOBAL VARIABLES
***************************************************************************/

static int	r3000_icount;



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static r3000_regs r3000;


static const memory_handlers be_memory =
{
	program_read_byte_32be,  program_read_word_32be,  program_read_dword_32be,
	program_write_byte_32be,  program_write_word_32be,  program_write_dword_32be
};

static const memory_handlers le_memory =
{
	program_read_byte_32le,  program_read_word_32le,  program_read_dword_32le,
	program_write_byte_32le,  program_write_word_32le,  program_write_dword_32le
};

static const memory_handlers be_cache =
{
	readcache_be,  readcache_be_word,  readcache_be_dword,
	writecache_be,  writecache_be_word,  writecache_be_dword
};

static const memory_handlers le_cache =
{
	readcache_le,  readcache_le_word,  readcache_le_dword,
	writecache_le,  writecache_le_word,  writecache_le_dword
};



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)		cpu_readop32(pc)



/***************************************************************************
    EXECEPTION HANDLING
***************************************************************************/

INLINE void generate_exception(int exception)
{
	/* set the exception PC */
	r3000.cpr[0][COP0_EPC] = r3000.pc;

	/* put the cause in the low 8 bits and clear the branch delay flag */
	CAUSE = (CAUSE & ~0x800000ff) | (exception << 2);

	/* if we were in a branch delay slot, adjust */
	if (r3000.nextpc != ~0)
	{
		r3000.nextpc = ~0;
		r3000.cpr[0][COP0_EPC] -= 4;
		CAUSE |= 0x80000000;
	}

	/* shift the exception bits */
	SR = (SR & 0xffffffc0) | ((SR << 2) & 0x3c);

	/* based on the BEV bit, we either go to ROM or RAM */
	r3000.pc = (SR & SR_BEV) ? 0xbfc00000 : 0x80000000;

	/* most exceptions go to offset 0x180, except for TLB stuff */
	if (exception >= EXCEPTION_TLBMOD && exception <= EXCEPTION_TLBSTORE)
		r3000.pc += 0x80;
	else
		r3000.pc += 0x180;

	/* swap to the new space */
	change_pc(r3000.pc);
}


INLINE void invalid_instruction(UINT32 op)
{
	generate_exception(EXCEPTION_INVALIDOP);
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(void)
{
	if ((CAUSE & SR & 0xff00) && (SR & SR_IEc))
		generate_exception(EXCEPTION_INTERRUPT);
}


static void set_irq_line(int irqline, int state)
{
	if (state != CLEAR_LINE)
		CAUSE |= 0x400 << irqline;
	else
		CAUSE &= ~(0x400 << irqline);
	check_irqs();
}



/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static void r3000_get_context(void *dst)
{
	/* copy the context */
	if (dst)
		*(r3000_regs *)dst = r3000;
}


static void r3000_set_context(void *src)
{
	/* copy the context */
	if (src)
		r3000 = *(r3000_regs *)src;

	change_pc(r3000.pc);

	/* check for IRQs */
	check_irqs();
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void r3000_init(int index, int clock, const void *_config, int (*irqcallback)(int))
{
	const struct r3000_config *config = _config;

	/* allocate memory */
	r3000.icache = auto_malloc(config->icache);
	r3000.dcache = auto_malloc(config->dcache);

	r3000.icache_size = config->icache;
	r3000.dcache_size = config->dcache;
	r3000.hasfpu = config->hasfpu;

	r3000.irq_callback = irqcallback;
}


static void r3000_reset(int bigendian)
{
	/* set up the endianness */
	r3000.bigendian = bigendian;
	if (r3000.bigendian)
	{
		r3000.memory_hand = &be_memory;
		r3000.cache_hand = &be_cache;
		r3000.lwl = lwl_be;
		r3000.lwr = lwr_be;
		r3000.swl = swl_be;
		r3000.swr = swr_be;
	}
	else
	{
		r3000.memory_hand = &le_memory;
		r3000.cache_hand = &le_cache;
		r3000.lwl = lwl_le;
		r3000.lwr = lwr_le;
		r3000.swl = swl_le;
		r3000.swr = swr_le;
	}

	/* initialize the rest of the config */
	r3000.cur = *r3000.memory_hand;
	r3000.cache = r3000.dcache;
	r3000.cache_size = r3000.dcache_size;

	/* initialize the state */
	r3000.pc = 0xbfc00000;
	r3000.nextpc = ~0;
	r3000.cpr[0][COP0_PRId] = 0x0200;
	r3000.cpr[0][COP0_Status] = 0x0000;
	change_pc(r3000.pc);
}

static void r3000be_reset(void)
{
	r3000_reset(1);
}

static void r3000le_reset(void)
{
	r3000_reset(0);
}


static void r3000_exit(void)
{
}



/***************************************************************************
    COP0 (SYSTEM) EXECUTION HANDLING
***************************************************************************/

INLINE UINT32 get_cop0_reg(int idx)
{
	return r3000.cpr[0][idx];
}

INLINE void set_cop0_reg(int idx, UINT32 val)
{
	if (idx == COP0_Cause)
	{
		CAUSE = (CAUSE & 0xfc00) | (val & ~0xfc00);

		/* update interrupts -- software ints can occur this way */
		check_irqs();
	}
	else if (idx == COP0_Status)
	{
		UINT32 oldsr = r3000.cpr[0][idx];
		UINT32 diff = oldsr ^ val;

		/* handle cache isolation */
		if (diff & SR_IsC)
		{
			if (val & SR_IsC)
				r3000.cur = *r3000.cache_hand;
			else
				r3000.cur = *r3000.memory_hand;
		}

		/* handle cache switching */
		if (diff & SR_SwC)
		{
			if (val & SR_SwC)
				r3000.cache = r3000.icache, r3000.cache_size = r3000.icache_size;
			else
				r3000.cache = r3000.dcache, r3000.cache_size = r3000.dcache_size;
		}
		r3000.cpr[0][idx] = val;

		/* update interrupts */
		check_irqs();
	}
	else
		r3000.cpr[0][idx] = val;
}

INLINE UINT32 get_cop0_creg(int idx)
{
	return r3000.ccr[0][idx];
}

INLINE void set_cop0_creg(int idx, UINT32 val)
{
	r3000.ccr[0][idx] = val;
}

INLINE void handle_cop0(UINT32 op)
{
	if (!(SR & SR_COP0) && (SR & SR_KUc))
		generate_exception(EXCEPTION_BADCOP);

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL = get_cop0_reg(RDREG);					break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL = get_cop0_creg(RDREG);				break;
		case 0x04:	/* MTCz */		set_cop0_reg(RDREG, RTVAL);								break;
		case 0x06:	/* CTCz */		set_cop0_creg(RDREG, RTVAL);							break;
		case 0x08:	/* BC */
			switch (RTREG)
			{
				case 0x00:	/* BCzF */	if (!r3000.cf[0]) ADDPC(SIMMVAL);					break;
				case 0x01:	/* BCzF */	if (r3000.cf[0]) ADDPC(SIMMVAL);					break;
				case 0x02:	/* BCzFL */	invalid_instruction(op);							break;
				case 0x03:	/* BCzTL */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */
			switch (op & 0x01ffffff)
			{
				case 0x01:	/* TLBR */														break;
				case 0x02:	/* TLBWI */														break;
				case 0x06:	/* TLBWR */														break;
				case 0x08:	/* TLBP */														break;
				case 0x10:	/* RFE */	SR = (SR & 0xfffffff0) | ((SR >> 2) & 0x0f);		break;
				case 0x18:	/* ERET */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		default:	invalid_instruction(op);												break;
	}
}



/***************************************************************************
    COP1 (FPU) EXECUTION HANDLING
***************************************************************************/

INLINE UINT32 get_cop1_reg(int idx)
{
	return r3000.cpr[1][idx];
}

INLINE void set_cop1_reg(int idx, UINT32 val)
{
	r3000.cpr[1][idx] = val;
}

INLINE UINT32 get_cop1_creg(int idx)
{
	return r3000.ccr[1][idx];
}

INLINE void set_cop1_creg(int idx, UINT32 val)
{
	r3000.ccr[1][idx] = val;
}

INLINE void handle_cop1(UINT32 op)
{
	if (!(SR & SR_COP1))
		generate_exception(EXCEPTION_BADCOP);
	if (!r3000.hasfpu)
		return;

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL = get_cop1_reg(RDREG);					break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL = get_cop1_creg(RDREG);				break;
		case 0x04:	/* MTCz */		set_cop1_reg(RDREG, RTVAL);								break;
		case 0x06:	/* CTCz */		set_cop1_creg(RDREG, RTVAL);							break;
		case 0x08:	/* BC */
			switch (RTREG)
			{
				case 0x00:	/* BCzF */	if (!r3000.cf[1]) ADDPC(SIMMVAL);					break;
				case 0x01:	/* BCzF */	if (r3000.cf[1]) ADDPC(SIMMVAL);					break;
				case 0x02:	/* BCzFL */	invalid_instruction(op);							break;
				case 0x03:	/* BCzTL */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */		invalid_instruction(op);								break;
		default:	invalid_instruction(op);												break;
	}
}



/***************************************************************************
    COP2 (CUSTOM) EXECUTION HANDLING
***************************************************************************/

INLINE UINT32 get_cop2_reg(int idx)
{
	return r3000.cpr[2][idx];
}

INLINE void set_cop2_reg(int idx, UINT32 val)
{
	r3000.cpr[2][idx] = val;
}

INLINE UINT32 get_cop2_creg(int idx)
{
	return r3000.ccr[2][idx];
}

INLINE void set_cop2_creg(int idx, UINT32 val)
{
	r3000.ccr[2][idx] = val;
}

INLINE void handle_cop2(UINT32 op)
{
	if (!(SR & SR_COP2))
		generate_exception(EXCEPTION_BADCOP);

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL = get_cop2_reg(RDREG);					break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL = get_cop2_creg(RDREG);				break;
		case 0x04:	/* MTCz */		set_cop2_reg(RDREG, RTVAL);								break;
		case 0x06:	/* CTCz */		set_cop2_creg(RDREG, RTVAL);							break;
		case 0x08:	/* BC */
			switch (RTREG)
			{
				case 0x00:	/* BCzF */	if (!r3000.cf[2]) ADDPC(SIMMVAL);					break;
				case 0x01:	/* BCzF */	if (r3000.cf[2]) ADDPC(SIMMVAL);					break;
				case 0x02:	/* BCzFL */	invalid_instruction(op);							break;
				case 0x03:	/* BCzTL */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */		invalid_instruction(op);								break;
		default:	invalid_instruction(op);												break;
	}
}



/***************************************************************************
    COP3 (CUSTOM) EXECUTION HANDLING
***************************************************************************/

INLINE UINT32 get_cop3_reg(int idx)
{
	return r3000.cpr[3][idx];
}

INLINE void set_cop3_reg(int idx, UINT32 val)
{
	r3000.cpr[3][idx] = val;
}

INLINE UINT32 get_cop3_creg(int idx)
{
	return r3000.ccr[3][idx];
}

INLINE void set_cop3_creg(int idx, UINT32 val)
{
	r3000.ccr[3][idx] = val;
}

INLINE void handle_cop3(UINT32 op)
{
	if (!(SR & SR_COP3))
		generate_exception(EXCEPTION_BADCOP);

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL = get_cop3_reg(RDREG);					break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL = get_cop3_creg(RDREG);				break;
		case 0x04:	/* MTCz */		set_cop3_reg(RDREG, RTVAL);								break;
		case 0x06:	/* CTCz */		set_cop3_creg(RDREG, RTVAL);							break;
		case 0x08:	/* BC */
			switch (RTREG)
			{
				case 0x00:	/* BCzF */	if (!r3000.cf[3]) ADDPC(SIMMVAL);					break;
				case 0x01:	/* BCzF */	if (r3000.cf[3]) ADDPC(SIMMVAL);					break;
				case 0x02:	/* BCzFL */	invalid_instruction(op);							break;
				case 0x03:	/* BCzTL */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */		invalid_instruction(op);								break;
		default:	invalid_instruction(op);												break;
	}
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static int r3000_execute(int cycles)
{
	/* count cycles and interrupt cycles */
	r3000_icount = cycles;
	r3000_icount -= r3000.interrupt_cycles;
	r3000.interrupt_cycles = 0;
	change_pc(r3000.pc);

	/* core execution loop */
	do
	{
		UINT32 op;
		UINT64 temp64;
		int temp;

		/* debugging */
		r3000.ppc = r3000.pc;
		CALL_MAME_DEBUG;

		/* instruction fetch */
		op = ROPCODE(r3000.pc);

		/* adjust for next PC */
		if (r3000.nextpc != ~0)
		{
			r3000.pc = r3000.nextpc;
			r3000.nextpc = ~0;
			change_pc(r3000.pc);
		}
		else
			r3000.pc += 4;

		/* parse the instruction */
		switch (op >> 26)
		{
			case 0x00:	/* SPECIAL */
				switch (op & 63)
				{
					case 0x00:	/* SLL */		if (RDREG) RDVAL = RTVAL << SHIFT;						break;
					case 0x02:	/* SRL */		if (RDREG) RDVAL = RTVAL >> SHIFT;						break;
					case 0x03:	/* SRA */		if (RDREG) RDVAL = (INT32)RTVAL >> SHIFT;				break;
					case 0x04:	/* SLLV */		if (RDREG) RDVAL = RTVAL << (RSVAL & 31);				break;
					case 0x06:	/* SRLV */		if (RDREG) RDVAL = RTVAL >> (RSVAL & 31);				break;
					case 0x07:	/* SRAV */		if (RDREG) RDVAL = (INT32)RTVAL >> (RSVAL & 31);		break;
					case 0x08:	/* JR */		SETPC(RSVAL);											break;
					case 0x09:	/* JALR */		SETPCL(RSVAL,RDREG);									break;
					case 0x0c:	/* SYSCALL */	generate_exception(EXCEPTION_SYSCALL);					break;
					case 0x0d:	/* BREAK */		generate_exception(EXCEPTION_BREAK);					break;
					case 0x0f:	/* SYNC */		invalid_instruction(op);								break;
					case 0x10:	/* MFHI */		if (RDREG) RDVAL = HIVAL;								break;
					case 0x11:	/* MTHI */		HIVAL = RSVAL;											break;
					case 0x12:	/* MFLO */		if (RDREG) RDVAL = LOVAL;								break;
					case 0x13:	/* MTLO */		LOVAL = RSVAL;											break;
					case 0x18:	/* MULT */
						temp64 = (INT64)(INT32)RSVAL * (INT64)(INT32)RTVAL;
						LOVAL = (UINT32)temp64;
						HIVAL = (UINT32)(temp64 >> 32);
						r3000_icount -= 11;
						break;
					case 0x19:	/* MULTU */
						temp64 = (UINT64)RSVAL * (UINT64)RTVAL;
						LOVAL = (UINT32)temp64;
						HIVAL = (UINT32)(temp64 >> 32);
						r3000_icount -= 11;
						break;
					case 0x1a:	/* DIV */
						if (RTVAL)
						{
							LOVAL = (INT32)RSVAL / (INT32)RTVAL;
							HIVAL = (INT32)RSVAL % (INT32)RTVAL;
						}
						r3000_icount -= 34;
						break;
					case 0x1b:	/* DIVU */
						if (RTVAL)
						{
							LOVAL = RSVAL / RTVAL;
							HIVAL = RSVAL % RTVAL;
						}
						r3000_icount -= 34;
						break;
					case 0x20:	/* ADD */
						if (ENABLE_OVERFLOWS && RSVAL > ~RTVAL) generate_exception(EXCEPTION_OVERFLOW);
						else RDVAL = RSVAL + RTVAL;
						break;
					case 0x21:	/* ADDU */		if (RDREG) RDVAL = RSVAL + RTVAL;						break;
					case 0x22:	/* SUB */
						if (ENABLE_OVERFLOWS && RSVAL < RTVAL) generate_exception(EXCEPTION_OVERFLOW);
						else RDVAL = RSVAL - RTVAL;
						break;
					case 0x23:	/* SUBU */		if (RDREG) RDVAL = RSVAL - RTVAL;						break;
					case 0x24:	/* AND */		if (RDREG) RDVAL = RSVAL & RTVAL;						break;
					case 0x25:	/* OR */		if (RDREG) RDVAL = RSVAL | RTVAL;						break;
					case 0x26:	/* XOR */		if (RDREG) RDVAL = RSVAL ^ RTVAL;						break;
					case 0x27:	/* NOR */		if (RDREG) RDVAL = ~(RSVAL | RTVAL);					break;
					case 0x2a:	/* SLT */		if (RDREG) RDVAL = (INT32)RSVAL < (INT32)RTVAL;			break;
					case 0x2b:	/* SLTU */		if (RDREG) RDVAL = (UINT32)RSVAL < (UINT32)RTVAL;		break;
					case 0x30:	/* TEQ */		invalid_instruction(op);								break;
					case 0x31:	/* TGEU */		invalid_instruction(op);								break;
					case 0x32:	/* TLT */		invalid_instruction(op);								break;
					case 0x33:	/* TLTU */		invalid_instruction(op);								break;
					case 0x34:	/* TGE */		invalid_instruction(op);								break;
					case 0x36:	/* TNE */		invalid_instruction(op);								break;
					default:	/* ??? */		invalid_instruction(op);								break;
				}
				break;

			case 0x01:	/* REGIMM */
				switch (RTREG)
				{
					case 0x00:	/* BLTZ */		if ((INT32)RSVAL < 0) ADDPC(SIMMVAL);					break;
					case 0x01:	/* BGEZ */		if ((INT32)RSVAL >= 0) ADDPC(SIMMVAL);					break;
					case 0x02:	/* BLTZL */		invalid_instruction(op);								break;
					case 0x03:	/* BGEZL */		invalid_instruction(op);								break;
					case 0x08:	/* TGEI */		invalid_instruction(op);								break;
					case 0x09:	/* TGEIU */		invalid_instruction(op);								break;
					case 0x0a:	/* TLTI */		invalid_instruction(op);								break;
					case 0x0b:	/* TLTIU */		invalid_instruction(op);								break;
					case 0x0c:	/* TEQI */		invalid_instruction(op);								break;
					case 0x0e:	/* TNEI */		invalid_instruction(op);								break;
					case 0x10:	/* BLTZAL */	if ((INT32)RSVAL < 0) ADDPCL(SIMMVAL,31);				break;
					case 0x11:	/* BGEZAL */	if ((INT32)RSVAL >= 0) ADDPCL(SIMMVAL,31);				break;
					case 0x12:	/* BLTZALL */	invalid_instruction(op);								break;
					case 0x13:	/* BGEZALL */	invalid_instruction(op);								break;
					default:	/* ??? */		invalid_instruction(op);								break;
				}
				break;

			case 0x02:	/* J */			ABSPC(LIMMVAL);													break;
			case 0x03:	/* JAL */		ABSPCL(LIMMVAL,31);												break;
			case 0x04:	/* BEQ */		if (RSVAL == RTVAL) ADDPC(SIMMVAL);								break;
			case 0x05:	/* BNE */		if (RSVAL != RTVAL) ADDPC(SIMMVAL);								break;
			case 0x06:	/* BLEZ */		if ((INT32)RSVAL <= 0) ADDPC(SIMMVAL);							break;
			case 0x07:	/* BGTZ */		if ((INT32)RSVAL > 0) ADDPC(SIMMVAL);							break;
			case 0x08:	/* ADDI */
				if (ENABLE_OVERFLOWS && RSVAL > ~SIMMVAL) generate_exception(EXCEPTION_OVERFLOW);
				else if (RTREG) RTVAL = RSVAL + SIMMVAL;
				break;
			case 0x09:	/* ADDIU */		if (RTREG) RTVAL = RSVAL + SIMMVAL;								break;
			case 0x0a:	/* SLTI */		if (RTREG) RTVAL = (INT32)RSVAL < (INT32)SIMMVAL;				break;
			case 0x0b:	/* SLTIU */		if (RTREG) RTVAL = (UINT32)RSVAL < (UINT32)SIMMVAL;				break;
			case 0x0c:	/* ANDI */		if (RTREG) RTVAL = RSVAL & UIMMVAL;								break;
			case 0x0d:	/* ORI */		if (RTREG) RTVAL = RSVAL | UIMMVAL;								break;
			case 0x0e:	/* XORI */		if (RTREG) RTVAL = RSVAL ^ UIMMVAL;								break;
			case 0x0f:	/* LUI */		if (RTREG) RTVAL = UIMMVAL << 16;								break;
			case 0x10:	/* COP0 */		handle_cop0(op);												break;
			case 0x11:	/* COP1 */		handle_cop1(op);												break;
			case 0x12:	/* COP2 */		handle_cop2(op);												break;
			case 0x13:	/* COP3 */		handle_cop3(op);												break;
			case 0x14:	/* BEQL */		invalid_instruction(op);										break;
			case 0x15:	/* BNEL */		invalid_instruction(op);										break;
			case 0x16:	/* BLEZL */		invalid_instruction(op);										break;
			case 0x17:	/* BGTZL */		invalid_instruction(op);										break;
			case 0x20:	/* LB */		temp = RBYTE(SIMMVAL+RSVAL); if (RTREG) RTVAL = (INT8)temp;		break;
			case 0x21:	/* LH */		temp = RWORD(SIMMVAL+RSVAL); if (RTREG) RTVAL = (INT16)temp;	break;
			case 0x22:	/* LWL */		(*r3000.lwl)(op);												break;
			case 0x23:	/* LW */		temp = RLONG(SIMMVAL+RSVAL); if (RTREG) RTVAL = temp;			break;
			case 0x24:	/* LBU */		temp = RBYTE(SIMMVAL+RSVAL); if (RTREG) RTVAL = (UINT8)temp;	break;
			case 0x25:	/* LHU */		temp = RWORD(SIMMVAL+RSVAL); if (RTREG) RTVAL = (UINT16)temp;	break;
			case 0x26:	/* LWR */		(*r3000.lwr)(op);												break;
			case 0x28:	/* SB */		WBYTE(SIMMVAL+RSVAL, RTVAL);									break;
			case 0x29:	/* SH */		WWORD(SIMMVAL+RSVAL, RTVAL); 									break;
			case 0x2a:	/* SWL */		(*r3000.swl)(op);												break;
			case 0x2b:	/* SW */		WLONG(SIMMVAL+RSVAL, RTVAL);									break;
			case 0x2e:	/* SWR */		(*r3000.swr)(op);												break;
			case 0x2f:	/* CACHE */		invalid_instruction(op);										break;
			case 0x30:	/* LL */		invalid_instruction(op);										break;
			case 0x31:	/* LWC1 */		set_cop1_reg(RTREG, RLONG(SIMMVAL+RSVAL));						break;
			case 0x32:	/* LWC2 */		set_cop2_reg(RTREG, RLONG(SIMMVAL+RSVAL));						break;
			case 0x33:	/* LWC3 */		set_cop3_reg(RTREG, RLONG(SIMMVAL+RSVAL));						break;
			case 0x34:	/* LDC0 */		invalid_instruction(op);										break;
			case 0x35:	/* LDC1 */		invalid_instruction(op);										break;
			case 0x36:	/* LDC2 */		invalid_instruction(op);										break;
			case 0x37:	/* LDC3 */		invalid_instruction(op);										break;
			case 0x38:	/* SC */		invalid_instruction(op);										break;
			case 0x39:	/* LWC1 */		WLONG(SIMMVAL+RSVAL, get_cop1_reg(RTREG));						break;
			case 0x3a:	/* LWC2 */		WLONG(SIMMVAL+RSVAL, get_cop2_reg(RTREG));						break;
			case 0x3b:	/* LWC3 */		WLONG(SIMMVAL+RSVAL, get_cop3_reg(RTREG));						break;
			case 0x3c:	/* SDC0 */		invalid_instruction(op);										break;
			case 0x3d:	/* SDC1 */		invalid_instruction(op);										break;
			case 0x3e:	/* SDC2 */		invalid_instruction(op);										break;
			case 0x3f:	/* SDC3 */		invalid_instruction(op);										break;
			default:	/* ??? */		invalid_instruction(op);										break;
		}
		r3000_icount--;

	} while (r3000_icount > 0 || r3000.nextpc != ~0);

	r3000_icount -= r3000.interrupt_cycles;
	r3000.interrupt_cycles = 0;
	return cycles - r3000_icount;
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

#ifdef MAME_DEBUG
static offs_t r3000_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	extern unsigned dasmr3k(char *, unsigned, UINT32);
	UINT32 op = *(UINT32 *)oprom;
	if (r3000.bigendian)
		op = BIG_ENDIANIZE_INT32(op);
	else
		op = LITTLE_ENDIANIZE_INT32(op);

	return dasmr3k(buffer, pc, op);
}
#endif /* MAME_DEBUG */



/***************************************************************************
    CACHE I/O
***************************************************************************/

static UINT8 readcache_be(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < r3000.cache_size) ? r3000.cache[BYTE4_XOR_BE(offset)] : 0xff;
}

static UINT16 readcache_be_word(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < r3000.cache_size) ? *(UINT16 *)&r3000.cache[WORD_XOR_BE(offset)] : 0xffff;
}

static UINT32 readcache_be_dword(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < r3000.cache_size) ? *(UINT32 *)&r3000.cache[offset] : 0xffffffff;
}

static void writecache_be(offs_t offset, UINT8 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < r3000.cache_size) r3000.cache[BYTE4_XOR_BE(offset)] = data;
}

static void writecache_be_word(offs_t offset, UINT16 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < r3000.cache_size) *(UINT16 *)&r3000.cache[WORD_XOR_BE(offset)] = data;
}

static void writecache_be_dword(offs_t offset, UINT32 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < r3000.cache_size) *(UINT32 *)&r3000.cache[offset] = data;
}

static UINT8 readcache_le(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < r3000.cache_size) ? r3000.cache[BYTE4_XOR_LE(offset)] : 0xff;
}

static UINT16 readcache_le_word(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < r3000.cache_size) ? *(UINT16 *)&r3000.cache[WORD_XOR_LE(offset)] : 0xffff;
}

static UINT32 readcache_le_dword(offs_t offset)
{
	offset &= 0x1fffffff;
	return (offset * 4 < r3000.cache_size) ? *(UINT32 *)&r3000.cache[offset] : 0xffffffff;
}

static void writecache_le(offs_t offset, UINT8 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < r3000.cache_size) r3000.cache[BYTE4_XOR_LE(offset)] = data;
}

static void writecache_le_word(offs_t offset, UINT16 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < r3000.cache_size) *(UINT16 *)&r3000.cache[WORD_XOR_LE(offset)] = data;
}

static void writecache_le_dword(offs_t offset, UINT32 data)
{
	offset &= 0x1fffffff;
	if (offset * 4 < r3000.cache_size) *(UINT32 *)&r3000.cache[offset] = data;
}



/***************************************************************************
    COMPLEX OPCODE IMPLEMENTATIONS
***************************************************************************/

static void lwl_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if (!(offs & 3)) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0x00ffffff >> (24 - shift))) | (temp << shift);
		}
	}
}

static void lwr_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if ((offs & 3) == 3) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0xffffff00 << shift)) | (temp >> (24 - shift));
		}
	}
}

static void swl_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	if (!(offs & 3)) WLONG(offs, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0xffffff00 << (24 - shift))) | (RTVAL >> shift));
	}
}


static void swr_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	if ((offs & 3) == 3) WLONG(offs & ~3, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0x00ffffff >> shift)) | (RTVAL << (24 - shift)));
	}
}



static void lwl_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if (!(offs & 3)) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0xffffff00 << (24 - shift))) | (temp >> shift);
		}
	}
}

static void lwr_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	UINT32 temp = RLONG(offs & ~3);
	if (RTREG)
	{
		if ((offs & 3) == 3) RTVAL = temp;
		else
		{
			int shift = 8 * (offs & 3);
			RTVAL = (RTVAL & (0x00ffffff >> shift)) | (temp << (24 - shift));
		}
	}
}

static void swl_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	if (!(offs & 3)) WLONG(offs, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0x00ffffff >> (24 - shift))) | (RTVAL << shift));
	}
}

static void swr_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL;
	if ((offs & 3) == 3) WLONG(offs & ~3, RTVAL);
	else
	{
		UINT32 temp = RLONG(offs & ~3);
		int shift = 8 * (offs & 3);
		WLONG(offs & ~3, (temp & (0xffffff00 << shift)) | (RTVAL >> (24 - shift)));
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void r3000_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ0:		set_irq_line(R3000_IRQ0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ1:		set_irq_line(R3000_IRQ1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ2:		set_irq_line(R3000_IRQ2, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ3:		set_irq_line(R3000_IRQ3, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ4:		set_irq_line(R3000_IRQ4, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ5:		set_irq_line(R3000_IRQ5, info->i);		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + R3000_PC:			r3000.pc = info->i;						break;
		case CPUINFO_INT_REGISTER + R3000_SR:			SR = info->i;							break;

		case CPUINFO_INT_REGISTER + R3000_R0:			r3000.r[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R1:			r3000.r[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R2:			r3000.r[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R3:			r3000.r[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R4:			r3000.r[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R5:			r3000.r[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R6:			r3000.r[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R7:			r3000.r[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R8:			r3000.r[8] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R9:			r3000.r[9] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R10:			r3000.r[10] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R11:			r3000.r[11] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R12:			r3000.r[12] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R13:			r3000.r[13] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R14:			r3000.r[14] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R15:			r3000.r[15] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R16:			r3000.r[16] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R17:			r3000.r[17] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R18:			r3000.r[18] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R19:			r3000.r[19] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R20:			r3000.r[20] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R21:			r3000.r[21] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R22:			r3000.r[22] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R23:			r3000.r[23] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R24:			r3000.r[24] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R25:			r3000.r[25] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R26:			r3000.r[26] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R27:			r3000.r[27] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R28:			r3000.r[28] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R29:			r3000.r[29] = info->i;					break;
		case CPUINFO_INT_REGISTER + R3000_R30:			r3000.r[30] = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + R3000_R31:			r3000.r[31] = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void r3000_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(r3000);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 29;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + R3000_IRQ0:		info->i = (r3000.cpr[0][COP0_Cause] & 0x400) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ1:		info->i = (r3000.cpr[0][COP0_Cause] & 0x800) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ2:		info->i = (r3000.cpr[0][COP0_Cause] & 0x1000) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ3:		info->i = (r3000.cpr[0][COP0_Cause] & 0x2000) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ4:		info->i = (r3000.cpr[0][COP0_Cause] & 0x4000) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + R3000_IRQ5:		info->i = (r3000.cpr[0][COP0_Cause] & 0x8000) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = r3000.ppc;					break;

		case CPUINFO_INT_PC:							info->i = r3000.pc & 0x1fffffff;		break;
		case CPUINFO_INT_REGISTER + R3000_PC:			info->i = r3000.pc;						break;
		case CPUINFO_INT_REGISTER + R3000_SR:			info->i = SR;							break;

		case CPUINFO_INT_REGISTER + R3000_R0:			info->i = r3000.r[0];					break;
		case CPUINFO_INT_REGISTER + R3000_R1:			info->i = r3000.r[1];					break;
		case CPUINFO_INT_REGISTER + R3000_R2:			info->i = r3000.r[2];					break;
		case CPUINFO_INT_REGISTER + R3000_R3:			info->i = r3000.r[3];					break;
		case CPUINFO_INT_REGISTER + R3000_R4:			info->i = r3000.r[4];					break;
		case CPUINFO_INT_REGISTER + R3000_R5:			info->i = r3000.r[5];					break;
		case CPUINFO_INT_REGISTER + R3000_R6:			info->i = r3000.r[6];					break;
		case CPUINFO_INT_REGISTER + R3000_R7:			info->i = r3000.r[7];					break;
		case CPUINFO_INT_REGISTER + R3000_R8:			info->i = r3000.r[8];					break;
		case CPUINFO_INT_REGISTER + R3000_R9:			info->i = r3000.r[9];					break;
		case CPUINFO_INT_REGISTER + R3000_R10:			info->i = r3000.r[10];					break;
		case CPUINFO_INT_REGISTER + R3000_R11:			info->i = r3000.r[11];					break;
		case CPUINFO_INT_REGISTER + R3000_R12:			info->i = r3000.r[12];					break;
		case CPUINFO_INT_REGISTER + R3000_R13:			info->i = r3000.r[13];					break;
		case CPUINFO_INT_REGISTER + R3000_R14:			info->i = r3000.r[14];					break;
		case CPUINFO_INT_REGISTER + R3000_R15:			info->i = r3000.r[15];					break;
		case CPUINFO_INT_REGISTER + R3000_R16:			info->i = r3000.r[16];					break;
		case CPUINFO_INT_REGISTER + R3000_R17:			info->i = r3000.r[17];					break;
		case CPUINFO_INT_REGISTER + R3000_R18:			info->i = r3000.r[18];					break;
		case CPUINFO_INT_REGISTER + R3000_R19:			info->i = r3000.r[19];					break;
		case CPUINFO_INT_REGISTER + R3000_R20:			info->i = r3000.r[20];					break;
		case CPUINFO_INT_REGISTER + R3000_R21:			info->i = r3000.r[21];					break;
		case CPUINFO_INT_REGISTER + R3000_R22:			info->i = r3000.r[22];					break;
		case CPUINFO_INT_REGISTER + R3000_R23:			info->i = r3000.r[23];					break;
		case CPUINFO_INT_REGISTER + R3000_R24:			info->i = r3000.r[24];					break;
		case CPUINFO_INT_REGISTER + R3000_R25:			info->i = r3000.r[25];					break;
		case CPUINFO_INT_REGISTER + R3000_R26:			info->i = r3000.r[26];					break;
		case CPUINFO_INT_REGISTER + R3000_R27:			info->i = r3000.r[27];					break;
		case CPUINFO_INT_REGISTER + R3000_R28:			info->i = r3000.r[28];					break;
		case CPUINFO_INT_REGISTER + R3000_R29:			info->i = r3000.r[29];					break;
		case CPUINFO_INT_REGISTER + R3000_R30:			info->i = r3000.r[30];					break;
		case CPUINFO_INT_SP:							info->i = r3000.r[31] & 0x1fffffff;		break;
		case CPUINFO_INT_REGISTER + R3000_R31:			info->i = r3000.r[31];					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = r3000_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = r3000_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = r3000_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = r3000_init;				break;
		case CPUINFO_PTR_RESET:							/* provided per-CPU */					break;
		case CPUINFO_PTR_EXIT:							info->exit = r3000_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = r3000_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = r3000_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &r3000_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R3000");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "MIPS II");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C) Aaron Giles 2000-2002"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + R3000_PC:  			sprintf(info->s, "PC: %08X", r3000.pc); break;
		case CPUINFO_STR_REGISTER + R3000_SR:  			sprintf(info->s, "SR: %08X", r3000.cpr[0][COP0_Status]); break;

		case CPUINFO_STR_REGISTER + R3000_R0:			sprintf(info->s, "R0: %08X", r3000.r[0]); break;
		case CPUINFO_STR_REGISTER + R3000_R1:			sprintf(info->s, "R1: %08X", r3000.r[1]); break;
		case CPUINFO_STR_REGISTER + R3000_R2:			sprintf(info->s, "R2: %08X", r3000.r[2]); break;
		case CPUINFO_STR_REGISTER + R3000_R3:			sprintf(info->s, "R3: %08X", r3000.r[3]); break;
		case CPUINFO_STR_REGISTER + R3000_R4:			sprintf(info->s, "R4: %08X", r3000.r[4]); break;
		case CPUINFO_STR_REGISTER + R3000_R5:			sprintf(info->s, "R5: %08X", r3000.r[5]); break;
		case CPUINFO_STR_REGISTER + R3000_R6:			sprintf(info->s, "R6: %08X", r3000.r[6]); break;
		case CPUINFO_STR_REGISTER + R3000_R7:			sprintf(info->s, "R7: %08X", r3000.r[7]); break;
		case CPUINFO_STR_REGISTER + R3000_R8:			sprintf(info->s, "R8: %08X", r3000.r[8]); break;
		case CPUINFO_STR_REGISTER + R3000_R9:			sprintf(info->s, "R9: %08X", r3000.r[9]); break;
		case CPUINFO_STR_REGISTER + R3000_R10:			sprintf(info->s, "R10:%08X", r3000.r[10]); break;
		case CPUINFO_STR_REGISTER + R3000_R11:			sprintf(info->s, "R11:%08X", r3000.r[11]); break;
		case CPUINFO_STR_REGISTER + R3000_R12:			sprintf(info->s, "R12:%08X", r3000.r[12]); break;
		case CPUINFO_STR_REGISTER + R3000_R13:			sprintf(info->s, "R13:%08X", r3000.r[13]); break;
		case CPUINFO_STR_REGISTER + R3000_R14:			sprintf(info->s, "R14:%08X", r3000.r[14]); break;
		case CPUINFO_STR_REGISTER + R3000_R15:			sprintf(info->s, "R15:%08X", r3000.r[15]); break;
		case CPUINFO_STR_REGISTER + R3000_R16:			sprintf(info->s, "R16:%08X", r3000.r[16]); break;
		case CPUINFO_STR_REGISTER + R3000_R17:			sprintf(info->s, "R17:%08X", r3000.r[17]); break;
		case CPUINFO_STR_REGISTER + R3000_R18:			sprintf(info->s, "R18:%08X", r3000.r[18]); break;
		case CPUINFO_STR_REGISTER + R3000_R19:			sprintf(info->s, "R19:%08X", r3000.r[19]); break;
		case CPUINFO_STR_REGISTER + R3000_R20:			sprintf(info->s, "R20:%08X", r3000.r[20]); break;
		case CPUINFO_STR_REGISTER + R3000_R21:			sprintf(info->s, "R21:%08X", r3000.r[21]); break;
		case CPUINFO_STR_REGISTER + R3000_R22:			sprintf(info->s, "R22:%08X", r3000.r[22]); break;
		case CPUINFO_STR_REGISTER + R3000_R23:			sprintf(info->s, "R23:%08X", r3000.r[23]); break;
		case CPUINFO_STR_REGISTER + R3000_R24:			sprintf(info->s, "R24:%08X", r3000.r[24]); break;
		case CPUINFO_STR_REGISTER + R3000_R25:			sprintf(info->s, "R25:%08X", r3000.r[25]); break;
		case CPUINFO_STR_REGISTER + R3000_R26:			sprintf(info->s, "R26:%08X", r3000.r[26]); break;
		case CPUINFO_STR_REGISTER + R3000_R27:			sprintf(info->s, "R27:%08X", r3000.r[27]); break;
		case CPUINFO_STR_REGISTER + R3000_R28:			sprintf(info->s, "R28:%08X", r3000.r[28]); break;
		case CPUINFO_STR_REGISTER + R3000_R29:			sprintf(info->s, "R29:%08X", r3000.r[29]); break;
		case CPUINFO_STR_REGISTER + R3000_R30:			sprintf(info->s, "R30:%08X", r3000.r[30]); break;
		case CPUINFO_STR_REGISTER + R3000_R31:			sprintf(info->s, "R31:%08X", r3000.r[31]); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void r3000be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = r3000be_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R3000 (big)");			break;

		default:										r3000_get_info(state, info);			break;
	}
}


void r3000le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = r3000le_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R3000 (little)");		break;

		default:										r3000_get_info(state, info);			break;
	}
}
