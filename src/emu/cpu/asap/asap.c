/***************************************************************************

    asap.c
    Core implementation for the portable ASAP emulator.
    ASAP = Atari Simplified Architecture Processor

    Written by Aaron Giles
    Special thanks to Mike Albaugh for clarification on a couple of fine points.

***************************************************************************/

#include "debugger.h"
#include "asap.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PS_CFLAG			0x00000001
#define PS_VFLAG			0x00000002
#define PS_ZFLAG			0x00000004
#define PS_NFLAG			0x00000008
#define PS_IFLAG			0x00000010
#define PS_PFLAG			0x00000020

#define EXCEPTION_RESET		0
#define EXCEPTION_TRAP0		1
#define EXCEPTION_TRAPF		2
#define EXCEPTION_INTERRUPT	3


#define REGBASE				0xffe0


#define SET_C_ADD(a,b)		(asap.cflag = (UINT32)(b) > (UINT32)(~(a)))
#define SET_C_SUB(a,b)		(asap.cflag = (UINT32)(b) <= (UINT32)(a))
#define SET_V_ADD(r,a,b)	(asap.vflag = ~((a) ^ (b)) & ((a) ^ (r)))
#define SET_V_SUB(r,a,b)	(asap.vflag =  ((a) ^ (b)) & ((a) ^ (r)))
#define SET_ZN(r)			(asap.znflag = (r))
#define SET_ZNCV_ADD(r,a,b)	SET_ZN(r); SET_C_ADD(a,b); SET_V_ADD(r,a,b)
#define SET_ZNCV_SUB(r,a,b)	SET_ZN(r); SET_C_SUB(a,b); SET_V_SUB(r,a,b)

#define SET_VFLAG(val)		(asap.vflag = (val) << 31)
#define SET_CFLAG(val)		(asap.cflag = (val))

#define GET_FLAGS(r)		((r)->cflag | \
							 (((r)->vflag >> 30) & PS_VFLAG) | \
							 (((r)->znflag == 0) << 2) | \
							 (((r)->znflag >> 28) & PS_NFLAG) | \
							 ((r)->iflag << 4) | \
							 ((r)->pflag << 5))

#define SET_FLAGS(r,v)		do { \
								(r)->cflag = (v) & PS_CFLAG; \
								(r)->vflag = ((v) & PS_VFLAG) << 30; \
								(r)->znflag = ((v) & PS_ZFLAG) ? 0 : ((v) & PS_NFLAG) ? -1 : 1; \
								(r)->iflag = ((v) & PS_IFLAG) >> 4; \
								(r)->pflag = ((v) & PS_PFLAG) >> 5; \
							} while (0);


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* ASAP Registers */
typedef struct
{
	/* core registers */
	UINT32		r[32];
	UINT32		pc;

	/* expanded flags */
	UINT32		pflag;
	UINT32		iflag;
	UINT32		cflag;
	UINT32		vflag;
	UINT32		znflag;

	/* internal stuff */
	PAIR		op;
	UINT32		ppc;
	UINT32		nextpc;
	UINT8		irq_state;
	int			interrupt_cycles;
	int 		(*irq_callback)(int irqline);
} asap_regs;



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static asap_regs asap;

static void (**opcode)(void);
static UINT32 *src2val;

static int asap_icount;



/***************************************************************************
    OPCODE TABLE
***************************************************************************/

static void noop(void);
static void trap0(void);
static void bsp(void);
static void bmz(void);
static void bgt(void);
static void ble(void);
static void bge(void);
static void blt(void);
static void bhi(void);
static void bls(void);
static void bcc(void);
static void bcs(void);
static void bpl(void);
static void bmi(void);
static void bne(void);
static void beq(void);
static void bvc(void);
static void bvs(void);
static void bsr(void);
static void bsr_0(void);
static void lea(void);
static void lea_c(void);
static void lea_c0(void);
static void leah(void);
static void leah_c(void);
static void leah_c0(void);
static void subr(void);
static void subr_c(void);
static void subr_c0(void);
static void xor(void);
static void xor_c(void);
static void xor_c0(void);
static void xorn(void);
static void xorn_c(void);
static void xorn_c0(void);
static void add(void);
static void add_c(void);
static void add_c0(void);
static void sub(void);
static void sub_c(void);
static void sub_c0(void);
static void addc(void);
static void addc_c(void);
static void addc_c0(void);
static void subc(void);
static void subc_c(void);
static void subc_c0(void);
static void and(void);
static void and_c(void);
static void and_c0(void);
static void andn(void);
static void andn_c(void);
static void andn_c0(void);
static void or(void);
static void or_c(void);
static void or_c0(void);
static void orn(void);
static void orn_c(void);
static void orn_c0(void);
static void ld(void);
static void ld_0(void);
static void ld_c(void);
static void ld_c0(void);
static void ldh(void);
static void ldh_0(void);
static void ldh_c(void);
static void ldh_c0(void);
static void lduh(void);
static void lduh_0(void);
static void lduh_c(void);
static void lduh_c0(void);
static void sth(void);
static void sth_0(void);
static void sth_c(void);
static void sth_c0(void);
static void st(void);
static void st_0(void);
static void st_c(void);
static void st_c0(void);
static void ldb(void);
static void ldb_0(void);
static void ldb_c(void);
static void ldb_c0(void);
static void ldub(void);
static void ldub_0(void);
static void ldub_c(void);
static void ldub_c0(void);
static void stb(void);
static void stb_0(void);
static void stb_c(void);
static void stb_c0(void);
static void ashr(void);
static void ashr_c(void);
static void ashr_c0(void);
static void lshr(void);
static void lshr_c(void);
static void lshr_c0(void);
static void ashl(void);
static void ashl_c(void);
static void ashl_c0(void);
static void rotl(void);
static void rotl_c(void);
static void rotl_c0(void);
static void getps(void);
static void putps(void);
static void jsr(void);
static void jsr_0(void);
static void jsr_c(void);
static void jsr_c0(void);
static void trapf(void);

static void (*opcodetable[32][4])(void) =
{
	{	trap0,		trap0,		trap0,		trap0		},
	{	NULL,		NULL,		NULL,		NULL		},
	{	bsr,		bsr_0,		bsr,		bsr_0		},
	{	lea,		noop,		lea_c,		lea_c0		},
	{	leah,		noop,		leah_c,		leah_c0		},
	{	subr,		noop,		subr_c,		subr_c0		},
	{	xor,		noop,		xor_c,		xor_c0		},
	{	xorn,		noop,		xorn_c,		xorn_c0		},
	{	add,		noop,		add_c,		add_c0		},
	{	sub,		noop,		sub_c,		sub_c0		},
	{	addc,		noop,		addc_c,		addc_c0		},
	{	subc,		noop,		subc_c,		subc_c0		},
	{	and,		noop,		and_c,		and_c0		},
	{	andn,		noop,		andn_c,		andn_c0		},
	{	or,			noop,		or_c,		or_c0		},
	{	orn,		noop,		orn_c,		orn_c0		},
	{	ld,			ld_0,		ld_c,		ld_c0		},
	{	ldh,		ldh_0,		ldh_c,		ldh_c0		},
	{	lduh,		lduh_0,		lduh_c,		lduh_c0		},
	{	sth,		sth_0,		sth_c,		sth_c0		},
	{	st,			st_0,		st_c,		st_c0		},
	{	ldb,		ldb_0,		ldb_c,		ldb_c0		},
	{	ldub,		ldub_0,		ldub_c,		ldub_c0		},
	{	stb,		stb_0,		stb_c,		stb_c0		},
	{	ashr,		noop,		ashr_c,		ashr_c0		},
	{	lshr,		noop,		lshr_c,		lshr_c0		},
	{	ashl,		noop,		ashl_c,		ashl_c0		},
	{	rotl,		noop,		rotl_c,		rotl_c0		},
	{	getps,		noop,		getps,		noop		},
	{	putps,		putps,		putps,		putps		},
	{	jsr,		jsr_0,		jsr_c,		jsr_c0		},
	{	trapf,		trapf,		trapf,		trapf		}
};

static void (*conditiontable[16])(void) =
{
	bsp, bmz, bgt, ble, bge, blt, bhi, bls,
	bcc, bcs, bpl, bmi, bne, beq, bvc, bvs
};



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)		cpu_readop32(pc)
#define UPDATEPC()		change_pc(asap.pc)


INLINE UINT8 READBYTE(offs_t address)
{
	/* no alignment issues with bytes */
	return program_read_byte_32le(address);
}

INLINE UINT16 READWORD(offs_t address)
{
	/* aligned reads are easy */
	if (!(address & 1))
		return program_read_word_32le(address);

	/* misaligned reads are tricky */
	return program_read_dword_32le(address & ~3) >> (address & 3);
}

INLINE UINT32 READLONG(offs_t address)
{
	/* aligned reads are easy */
	if (!(address & 3))
		return program_read_dword_32le(address);

	/* misaligned reads are tricky */
	return program_read_dword_32le(address & ~3) >> (address & 3);
}

INLINE void WRITEBYTE(offs_t address, UINT8 data)
{
	/* no alignment issues with bytes */
	program_write_byte_32le(address, data);
}

INLINE void WRITEWORD(offs_t address, UINT16 data)
{
	/* aligned writes are easy */
	if (!(address & 1))
	{
		program_write_word_32le(address, data);
		return;
	}

	/* misaligned writes are tricky */
	if (!(address & 2))
	{
		program_write_byte_32le(address + 1, data);
		program_write_byte_32le(address + 2, data >> 8);
	}
	else
		program_write_byte_32le(address + 1, data);
}

INLINE void WRITELONG(offs_t address, UINT32 data)
{
	/* aligned writes are easy */
	if (!(address & 3))
	{
		program_write_dword_32le(address, data);
		return;
	}

	/* misaligned writes are tricky */
	switch (address & 3)
	{
		case 1:
			program_write_byte_32le(address, data);
			program_write_word_32le(address + 1, data >> 8);
			break;
		case 2:
			program_write_word_32le(address, data);
			break;
		case 3:
			program_write_byte_32le(address, data);
			break;
	}
}



/***************************************************************************
    EXCEPTION HANDLING
***************************************************************************/

INLINE void generate_exception(int exception)
{
	asap.pflag = asap.iflag;
	asap.iflag = 0;

	src2val[REGBASE + 30] = asap.pc;
	src2val[REGBASE + 31] = (asap.nextpc == ~0) ? asap.pc + 4 : asap.nextpc;

	asap.pc = 0x40 * exception;
	asap.nextpc = ~0;
	UPDATEPC();

	asap.interrupt_cycles++;
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

INLINE void check_irqs(void)
{
	if (asap.irq_state && asap.iflag)
	{
		generate_exception(EXCEPTION_INTERRUPT);
		if (asap.irq_callback)
			(*asap.irq_callback)(ASAP_IRQ0);
	}
}


static void set_irq_line(int irqline, int state)
{
	asap.irq_state = (state != CLEAR_LINE);
	check_irqs();
}



/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static void asap_get_context(void *dst)
{
	/* copy the context */
	if (dst)
	{
		if (src2val)
			memcpy(&asap.r[0], &src2val[REGBASE], 32 * sizeof(UINT32));
		*(asap_regs *)dst = asap;
	}
}


static void asap_set_context(void *src)
{
	/* copy the context */
	if (src)
	{
		asap = *(asap_regs *)src;
		if (src2val)
			memcpy(&src2val[REGBASE], &asap.r[0], 32 * sizeof(UINT32));
		UPDATEPC();

		/* check for IRQs */
		check_irqs();
	}
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void init_tables(void)
{
	/* allocate opcode table */
	if (!opcode)
		opcode = auto_malloc(32 * 32 * 2 * sizeof(void *));

	/* fill opcode table */
	if (opcode)
	{
		int op, dst, cond;

		for (op = 0; op < 32; op++)
			for (dst = 0; dst < 32; dst++)
				for (cond = 0; cond < 2; cond++)
					if (op == 1)
						opcode[(op << 6) + (dst << 1) + cond] = conditiontable[dst & 15];
					else if (cond && dst == 0)
						opcode[(op << 6) + (dst << 1) + cond] = opcodetable[op][3];
					else if (cond)
						opcode[(op << 6) + (dst << 1) + cond] = opcodetable[op][2];
					else if (dst == 0)
						opcode[(op << 6) + (dst << 1) + cond] = opcodetable[op][1];
					else
						opcode[(op << 6) + (dst << 1) + cond] = opcodetable[op][0];
	}

	/* allocate src2 table */
	if (!src2val)
		src2val = auto_malloc(65536 * sizeof(UINT32));

	/* fill scr2 table */
	if (src2val)
	{
		int i;

		for (i = 0; i < REGBASE; i++)
			src2val[i] = i;
		memcpy(&src2val[REGBASE], &asap.r[0], 32 * sizeof(UINT32));
	}
}

static void asap_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	init_tables();
	asap.irq_callback = irqcallback;
}

static void asap_reset(void)
{
	/* initialize the state */
	src2val[REGBASE + 0] = 0;
	asap.pc = 0;
	asap.iflag = 0;

	asap.ppc = 0;
	asap.nextpc = ~0;
	asap.irq_state = 0;
	asap.interrupt_cycles = 0;
	asap.irq_callback = NULL;

	UPDATEPC();
}


static void asap_exit(void)
{
	opcode = NULL;
	src2val = NULL;
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

INLINE void fetch_instruction(void)
{
	/* debugging */
	asap.ppc = asap.pc;
	CALL_MAME_DEBUG;

	/* instruction fetch */
	asap.op.d = ROPCODE(asap.pc);
	asap.pc += 4;
}

INLINE void execute_instruction(void)
{
	/* parse the instruction */
	(*opcode[asap.op.d >> 21])();
}

static int asap_execute(int cycles)
{
	/* count cycles and interrupt cycles */
	asap_icount = cycles;
	asap_icount -= asap.interrupt_cycles;
	asap.interrupt_cycles = 0;
	UPDATEPC();

	/* core execution loop */
	do
	{
		/* fetch and execute the next instruction */
		fetch_instruction();
		execute_instruction();

		/* fetch and execute the next instruction */
		fetch_instruction();
		execute_instruction();

		/* fetch and execute the next instruction */
		fetch_instruction();
		execute_instruction();

		/* fetch and execute the next instruction */
		fetch_instruction();
		execute_instruction();

		asap_icount -= 4;

	} while (asap_icount > 0);

	/* eat any new interrupt cycles */
	asap_icount -= asap.interrupt_cycles;
	asap.interrupt_cycles = 0;
	return cycles - asap_icount;
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

#ifdef MAME_DEBUG
extern offs_t asap_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif /* MAME_DEBUG */



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define OPCODE		(asap.op.d >> 27)
#define DSTREG		((asap.op.d >> 22) & 31)
#define DSTVAL		src2val[REGBASE + DSTREG]
#define SRC1REG		((asap.op.d >> 16) & 31)
#define SRC1VAL		src2val[REGBASE + SRC1REG]
#define SRC2VAL		src2val[asap.op.w.l]



/***************************************************************************
    OPCODES
***************************************************************************/

static void noop(void)
{
}

/**************************** TRAP 0 ******************************/

static void trap0(void)
{
	generate_exception(EXCEPTION_TRAP0);
}

/**************************** Bcc ******************************/

static void bsp(void)
{
	if ((INT32)asap.znflag > 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bmz(void)
{
	if ((INT32)asap.znflag <= 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bgt(void)
{
	if (asap.znflag != 0 && (INT32)(asap.znflag ^ asap.vflag) >= 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void ble(void)
{
	if (asap.znflag == 0 || (INT32)(asap.znflag ^ asap.vflag) < 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bge(void)
{
	if ((INT32)(asap.znflag ^ asap.vflag) >= 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void blt(void)
{
	if ((INT32)(asap.znflag ^ asap.vflag) < 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bhi(void)
{
	if (asap.znflag != 0 && asap.cflag)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bls(void)
{
	if (asap.znflag == 0 || !asap.cflag)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bcc(void)
{
	if (!asap.cflag)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bcs(void)
{
	if (asap.cflag)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bpl(void)
{
	if ((INT32)asap.znflag >= 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bmi(void)
{
	if ((INT32)asap.znflag < 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bne(void)
{
	if (asap.znflag != 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void beq(void)
{
	if (asap.znflag == 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bvc(void)
{
	if ((INT32)asap.vflag >= 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

static void bvs(void)
{
	if ((INT32)asap.vflag < 0)
	{
		asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

		fetch_instruction();
		asap.pc = asap.nextpc;
		asap.nextpc = ~0;
		/*UPDATEPC();*/

		execute_instruction();
		asap_icount--;
	}
}

/**************************** BSR ******************************/

static void bsr(void)
{
	DSTVAL = asap.pc + 4;
	asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

	fetch_instruction();
	asap.pc = asap.nextpc;
	asap.nextpc = ~0;
	/*UPDATEPC();*/

	execute_instruction();
	asap_icount--;
}

static void bsr_0(void)
{
	asap.nextpc = asap.ppc + ((INT32)(asap.op.d << 10) >> 8);

	fetch_instruction();
	asap.pc = asap.nextpc;
	asap.nextpc = ~0;
	/*UPDATEPC();*/

	execute_instruction();
	asap_icount--;
}

/**************************** LEA ******************************/

static void lea(void)
{
	DSTVAL = SRC1VAL + (SRC2VAL << 2);
}

static void lea_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 2);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0xc0000000)
		SET_CFLAG(1);
	if (((src1 ^ (src1 >> 1)) & 0x20000000) || (src1 ^ (src1 >> 2)) & 0x20000000)
		SET_VFLAG(1);
	DSTVAL = dst;
}

static void lea_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 2);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0xc0000000)
		SET_CFLAG(1);
	if (((src1 ^ (src1 >> 1)) & 0x20000000) || (src1 ^ (src1 >> 2)) & 0x20000000)
		SET_VFLAG(1);
}

/**************************** LEAH ******************************/

static void leah(void)
{
	DSTVAL = SRC1VAL + (SRC2VAL << 1);
}

static void leah_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 1);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0x80000000)
		SET_CFLAG(1);
	if ((src1 ^ (src1 >> 1)) & 0x40000000)
		SET_VFLAG(1);
	DSTVAL = dst;
}

static void leah_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + (src2 << 1);

	SET_ZNCV_ADD(dst, src1, src2);
	if (src1 & 0x80000000)
		SET_CFLAG(1);
	if ((src1 ^ (src1 >> 1)) & 0x40000000)
		SET_VFLAG(1);
}

/**************************** SUBR ******************************/

static void subr(void)
{
	DSTVAL = SRC2VAL - SRC1VAL;
}

static void subr_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src2 - src1;

	SET_ZNCV_SUB(dst, src2, src1);
	DSTVAL = dst;
}

static void subr_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src2 - src1;

	SET_ZNCV_SUB(dst, src2, src1);
}

/**************************** XOR ******************************/

static void xor(void)
{
	DSTVAL = SRC1VAL ^ SRC2VAL;
}

static void xor_c(void)
{
	UINT32 dst = SRC1VAL ^ SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

static void xor_c0(void)
{
	UINT32 dst = SRC1VAL ^ SRC2VAL;
	SET_ZN(dst);
}

/**************************** XOR ******************************/

static void xorn(void)
{
	DSTVAL = SRC1VAL ^ ~SRC2VAL;
}

static void xorn_c(void)
{
	UINT32 dst = SRC1VAL ^ ~SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

static void xorn_c0(void)
{
	UINT32 dst = SRC1VAL ^ ~SRC2VAL;
	SET_ZN(dst);
}

/**************************** ADD ******************************/

static void add(void)
{
	DSTVAL = SRC1VAL + SRC2VAL;
}

static void add_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2;

	SET_ZNCV_ADD(dst, src1, src2);
	DSTVAL = dst;
}

static void add_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2;

	SET_ZNCV_ADD(dst, src1, src2);
}

/**************************** ADD ******************************/

static void sub(void)
{
	DSTVAL = SRC1VAL - SRC2VAL;
}

static void sub_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2;

	SET_ZNCV_SUB(dst, src1, src2);
	DSTVAL = dst;
}

static void sub_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2;

	SET_ZNCV_SUB(dst, src1, src2);
}

/**************************** ADDC ******************************/

static void addc(void)
{
	DSTVAL = SRC1VAL + SRC2VAL + asap.cflag;
}

static void addc_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2 + asap.cflag;

	SET_ZNCV_ADD(dst, src1, src2);
	DSTVAL = dst;
}

static void addc_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 + src2 + asap.cflag;

	SET_ZNCV_ADD(dst, src1, src2);
}

/**************************** SUBC ******************************/

static void subc(void)
{
	DSTVAL = SRC1VAL - SRC2VAL - 1 + asap.cflag;
}

static void subc_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2 - 1 + asap.cflag;

	SET_ZNCV_SUB(dst, src1, src2);
	DSTVAL = dst;
}

static void subc_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL;
	UINT32 dst = src1 - src2 - 1 + asap.cflag;

	SET_ZNCV_SUB(dst, src1, src2);
}

/**************************** AND ******************************/

static void and(void)
{
	DSTVAL = SRC1VAL & SRC2VAL;
}

static void and_c(void)
{
	UINT32 dst = SRC1VAL & SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

static void and_c0(void)
{
	UINT32 dst = SRC1VAL & SRC2VAL;
	SET_ZN(dst);
}

/**************************** ANDN ******************************/

static void andn(void)
{
	DSTVAL = SRC1VAL & ~SRC2VAL;
}

static void andn_c(void)
{
	UINT32 dst = SRC1VAL & ~SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

static void andn_c0(void)
{
	UINT32 dst = SRC1VAL & ~SRC2VAL;
	SET_ZN(dst);
}

/**************************** OR ******************************/

static void or(void)
{
	DSTVAL = SRC1VAL | SRC2VAL;
}

static void or_c(void)
{
	UINT32 dst = SRC1VAL | SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

static void or_c0(void)
{
	UINT32 dst = SRC1VAL | SRC2VAL;
	SET_ZN(dst);
}

/**************************** ORN ******************************/

static void orn(void)
{
	DSTVAL = SRC1VAL | ~SRC2VAL;
}

static void orn_c(void)
{
	UINT32 dst = SRC1VAL | ~SRC2VAL;
	SET_ZN(dst);
	DSTVAL = dst;
}

static void orn_c0(void)
{
	UINT32 dst = SRC1VAL | ~SRC2VAL;
	SET_ZN(dst);
}

/**************************** LD ******************************/

static void ld(void)
{
	DSTVAL = READLONG(SRC1VAL + (SRC2VAL << 2));
}

static void ld_0(void)
{
	READLONG(SRC1VAL + (SRC2VAL << 2));
}

static void ld_c(void)
{
	UINT32 dst = READLONG(SRC1VAL + (SRC2VAL << 2));
	SET_ZN(dst);
	DSTVAL = dst;
}

static void ld_c0(void)
{
	UINT32 dst = READLONG(SRC1VAL + (SRC2VAL << 2));
	SET_ZN(dst);
}

/**************************** LDH ******************************/

static void ldh(void)
{
	DSTVAL = (INT16)READWORD(SRC1VAL + (SRC2VAL << 1));
}

static void ldh_0(void)
{
	READWORD(SRC1VAL + (SRC2VAL << 1));
}

static void ldh_c(void)
{
	UINT32 dst = (INT16)READWORD(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
	DSTVAL = dst;
}

static void ldh_c0(void)
{
	UINT32 dst = (INT16)READWORD(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
}

/**************************** LDUH ******************************/

static void lduh(void)
{
	DSTVAL = READWORD(SRC1VAL + (SRC2VAL << 1));
}

static void lduh_0(void)
{
	READWORD(SRC1VAL + (SRC2VAL << 1));
}

static void lduh_c(void)
{
	UINT32 dst = READWORD(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
	DSTVAL = dst;
}

static void lduh_c0(void)
{
	UINT32 dst = READWORD(SRC1VAL + (SRC2VAL << 1));
	SET_ZN(dst);
}

/**************************** STH ******************************/

static void sth(void)
{
	WRITEWORD(SRC1VAL + (SRC2VAL << 1), DSTVAL);
}

static void sth_0(void)
{
	WRITEWORD(SRC1VAL + (SRC2VAL << 1), 0);
}

static void sth_c(void)
{
	UINT32 dst = (UINT16)DSTVAL;
	SET_ZN(dst);
	WRITEWORD(SRC1VAL + (SRC2VAL << 1), dst);
}

static void sth_c0(void)
{
	SET_ZN(0);
	WRITEWORD(SRC1VAL + (SRC2VAL << 1), 0);
}

/**************************** ST ******************************/

static void st(void)
{
	WRITELONG(SRC1VAL + (SRC2VAL << 2), DSTVAL);
}

static void st_0(void)
{
	WRITELONG(SRC1VAL + (SRC2VAL << 2), 0);
}

static void st_c(void)
{
	UINT32 dst = DSTVAL;
	SET_ZN(dst);
	WRITELONG(SRC1VAL + (SRC2VAL << 2), dst);
}

static void st_c0(void)
{
	SET_ZN(0);
	WRITELONG(SRC1VAL + (SRC2VAL << 2), 0);
}

/**************************** LDB ******************************/

static void ldb(void)
{
	DSTVAL = (INT8)READBYTE(SRC1VAL + SRC2VAL);
}

static void ldb_0(void)
{
	READBYTE(SRC1VAL + SRC2VAL);
}

static void ldb_c(void)
{
	UINT32 dst = (INT8)READBYTE(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
	DSTVAL = dst;
}

static void ldb_c0(void)
{
	UINT32 dst = (INT8)READBYTE(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
}

/**************************** LDUB ******************************/

static void ldub(void)
{
	DSTVAL = READBYTE(SRC1VAL + SRC2VAL);
}

static void ldub_0(void)
{
	READBYTE(SRC1VAL + SRC2VAL);
}

static void ldub_c(void)
{
	UINT32 dst = READBYTE(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
	DSTVAL = dst;
}

static void ldub_c0(void)
{
	UINT32 dst = READBYTE(SRC1VAL + SRC2VAL);
	SET_ZN(dst);
}

/**************************** STB ******************************/

static void stb(void)
{
	WRITEBYTE(SRC1VAL + SRC2VAL, DSTVAL);
}

static void stb_0(void)
{
	WRITEBYTE(SRC1VAL + SRC2VAL, 0);
}

static void stb_c(void)
{
	UINT32 dst = (UINT8)DSTVAL;
	SET_ZN(dst);
	WRITEBYTE(SRC1VAL + SRC2VAL, dst);
}

static void stb_c0(void)
{
	SET_ZN(0);
	WRITEBYTE(SRC1VAL + SRC2VAL, 0);
}

/**************************** ASHR ******************************/

static void ashr(void)
{
	UINT32 src2 = SRC2VAL;
	DSTVAL = (src2 < 32) ? ((INT32)SRC1VAL >> src2) : ((INT32)SRC1VAL >> 31);
}

static void ashr_c(void)
{
	UINT32 src2 = SRC2VAL;
	asap.cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = (INT32)src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap.cflag = src1 >> 31;
		}
		DSTVAL = dst;
	}
	else
	{
		UINT32 dst = (INT32)SRC1VAL >> 31;
		SET_ZN(dst);
		DSTVAL = dst;
	}
}

static void ashr_c0(void)
{
	UINT32 src2 = SRC2VAL;
	asap.cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = (INT32)src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap.cflag = src1 >> 31;
		}
	}
	else
	{
		UINT32 dst = (INT32)SRC1VAL >> 31;
		SET_ZN(dst);
	}
}

/**************************** LSHR ******************************/

static void lshr(void)
{
	UINT32 src2 = SRC2VAL;
	DSTVAL = (src2 < 32) ? (SRC1VAL >> src2) : (SRC1VAL >> 31);
}

static void lshr_c(void)
{
	UINT32 src2 = SRC2VAL;
	asap.cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap.cflag = src1 >> 31;
		}
		DSTVAL = dst;
	}
	else
	{
		UINT32 dst = SRC1VAL >> 31;
		SET_ZN(dst);
		DSTVAL = dst;
	}
}

static void lshr_c0(void)
{
	UINT32 src2 = SRC2VAL;
	asap.cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 >> src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap.cflag = src1 >> 31;
		}
	}
	else
	{
		SET_ZN(0);
		DSTVAL = 0;
	}
}

/**************************** ASHL ******************************/

static void ashl(void)
{
	UINT32 src2 = SRC2VAL;
	DSTVAL = (src2 < 32) ? (SRC1VAL << src2) : 0;
}

static void ashl_c(void)
{
	UINT32 src2 = SRC2VAL;
	asap.cflag = asap.vflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 << src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = (INT32)src1 >> (32 - src2);
			asap.cflag = src1 & PS_CFLAG;
			asap.vflag = (src1 != ((INT32)dst >> 31)) << 31;
		}
		DSTVAL = dst;
	}
	else
	{
		SET_ZN(0);
		DSTVAL = 0;
	}
}

static void ashl_c0(void)
{
	UINT32 src2 = SRC2VAL;
	asap.cflag = asap.vflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL;
		UINT32 dst = src1 << src2;
		SET_ZN(dst);
		if (src2 != 0)
		{
			src1 = (INT32)src1 >> (32 - src2);
			asap.cflag = src1 & PS_CFLAG;
			asap.vflag = (src1 != ((INT32)dst >> 31)) << 31;
		}
	}
	else
		SET_ZN(0);
}

/**************************** ROTL ******************************/

static void rotl(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL & 31;
	DSTVAL = (src1 << src2) | (src1 >> (32 - src2));
}

static void rotl_c(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL & 31;
	UINT32 dst = (src1 << src2) | (src1 >> (32 - src2));
	SET_ZN(dst);
	DSTVAL = dst;
}

static void rotl_c0(void)
{
	UINT32 src1 = SRC1VAL;
	UINT32 src2 = SRC2VAL & 31;
	UINT32 dst = (src1 << src2) | (src1 >> (32 - src2));
	SET_ZN(dst);
}

/**************************** GETPS ******************************/

static void getps(void)
{
	DSTVAL = GET_FLAGS(&asap);
}

/**************************** PUTPS ******************************/

static void putps(void)
{
	UINT32 src2 = SRC2VAL & 0x3f;
	SET_FLAGS(&asap, src2);
	check_irqs();
}

/**************************** JSR ******************************/

static void jsr(void)
{
	DSTVAL = asap.pc + 4;
	asap.nextpc = SRC1VAL + (SRC2VAL << 2);

	fetch_instruction();
	asap.pc = asap.nextpc;
	asap.nextpc = ~0;
	UPDATEPC();

	execute_instruction();
	asap_icount--;
}

static void jsr_0(void)
{
	asap.nextpc = SRC1VAL + (SRC2VAL << 2);

	fetch_instruction();
	asap.pc = asap.nextpc;
	asap.nextpc = ~0;
	UPDATEPC();

	execute_instruction();
	asap_icount--;
}

static void jsr_c(void)
{
	DSTVAL = asap.pc + 4;
	asap.nextpc = SRC1VAL + (SRC2VAL << 2);
	asap.iflag = asap.pflag;

	fetch_instruction();
	asap.pc = asap.nextpc;
	asap.nextpc = ~0;
	UPDATEPC();

	execute_instruction();
	asap_icount--;
	check_irqs();
}

static void jsr_c0(void)
{
	asap.nextpc = SRC1VAL + (SRC2VAL << 2);
	asap.iflag = asap.pflag;

	fetch_instruction();
	asap.pc = asap.nextpc;
	asap.nextpc = ~0;
	UPDATEPC();

	execute_instruction();
	asap_icount--;
	check_irqs();
}

/**************************** TRAP F ******************************/

static void trapf(void)
{
	generate_exception(EXCEPTION_TRAPF);
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void asap_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + ASAP_IRQ0:	set_irq_line(ASAP_IRQ0, info->i);			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ASAP_PC:	asap.pc = info->i;								break;
		case CPUINFO_INT_REGISTER + ASAP_PS:	SET_FLAGS(&asap, info->i); 						break;

		case CPUINFO_INT_REGISTER + ASAP_R0:	src2val[REGBASE + 0] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R1:	src2val[REGBASE + 1] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R2:	src2val[REGBASE + 2] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R3:	src2val[REGBASE + 3] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R4:	src2val[REGBASE + 4] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R5:	src2val[REGBASE + 5] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R6:	src2val[REGBASE + 6] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R7:	src2val[REGBASE + 7] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R8:	src2val[REGBASE + 8] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R9:	src2val[REGBASE + 9] = info->i;					break;
		case CPUINFO_INT_REGISTER + ASAP_R10:	src2val[REGBASE + 10] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R11:	src2val[REGBASE + 11] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R12:	src2val[REGBASE + 12] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R13:	src2val[REGBASE + 13] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R14:	src2val[REGBASE + 14] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R15:	src2val[REGBASE + 15] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R16:	src2val[REGBASE + 16] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R17:	src2val[REGBASE + 17] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R18:	src2val[REGBASE + 18] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R19:	src2val[REGBASE + 19] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R20:	src2val[REGBASE + 20] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R21:	src2val[REGBASE + 21] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R22:	src2val[REGBASE + 22] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R23:	src2val[REGBASE + 23] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R24:	src2val[REGBASE + 24] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R25:	src2val[REGBASE + 25] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R26:	src2val[REGBASE + 26] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R27:	src2val[REGBASE + 27] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R28:	src2val[REGBASE + 28] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R29:	src2val[REGBASE + 29] = info->i;				break;
		case CPUINFO_INT_REGISTER + ASAP_R30:	src2val[REGBASE + 30] = info->i;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + ASAP_R31:	src2val[REGBASE + 31] = info->i;				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void asap_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(asap);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 12;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + ASAP_IRQ0:		info->i = asap.irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = asap.ppc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ASAP_PC:			info->i = asap.pc;						break;
		case CPUINFO_INT_REGISTER + ASAP_PS:			info->i = GET_FLAGS(&asap);				break;

		case CPUINFO_INT_REGISTER + ASAP_R0:			info->i = src2val[REGBASE + 0];			break;
		case CPUINFO_INT_REGISTER + ASAP_R1:			info->i = src2val[REGBASE + 1];			break;
		case CPUINFO_INT_REGISTER + ASAP_R2:			info->i = src2val[REGBASE + 2];			break;
		case CPUINFO_INT_REGISTER + ASAP_R3:			info->i = src2val[REGBASE + 3];			break;
		case CPUINFO_INT_REGISTER + ASAP_R4:			info->i = src2val[REGBASE + 4];			break;
		case CPUINFO_INT_REGISTER + ASAP_R5:			info->i = src2val[REGBASE + 5];			break;
		case CPUINFO_INT_REGISTER + ASAP_R6:			info->i = src2val[REGBASE + 6];			break;
		case CPUINFO_INT_REGISTER + ASAP_R7:			info->i = src2val[REGBASE + 7];			break;
		case CPUINFO_INT_REGISTER + ASAP_R8:			info->i = src2val[REGBASE + 8];			break;
		case CPUINFO_INT_REGISTER + ASAP_R9:			info->i = src2val[REGBASE + 9];			break;
		case CPUINFO_INT_REGISTER + ASAP_R10:			info->i = src2val[REGBASE + 10];		break;
		case CPUINFO_INT_REGISTER + ASAP_R11:			info->i = src2val[REGBASE + 11];		break;
		case CPUINFO_INT_REGISTER + ASAP_R12:			info->i = src2val[REGBASE + 12];		break;
		case CPUINFO_INT_REGISTER + ASAP_R13:			info->i = src2val[REGBASE + 13];		break;
		case CPUINFO_INT_REGISTER + ASAP_R14:			info->i = src2val[REGBASE + 14];		break;
		case CPUINFO_INT_REGISTER + ASAP_R15:			info->i = src2val[REGBASE + 15];		break;
		case CPUINFO_INT_REGISTER + ASAP_R16:			info->i = src2val[REGBASE + 16];		break;
		case CPUINFO_INT_REGISTER + ASAP_R17:			info->i = src2val[REGBASE + 17];		break;
		case CPUINFO_INT_REGISTER + ASAP_R18:			info->i = src2val[REGBASE + 18];		break;
		case CPUINFO_INT_REGISTER + ASAP_R19:			info->i = src2val[REGBASE + 19];		break;
		case CPUINFO_INT_REGISTER + ASAP_R20:			info->i = src2val[REGBASE + 20];		break;
		case CPUINFO_INT_REGISTER + ASAP_R21:			info->i = src2val[REGBASE + 21];		break;
		case CPUINFO_INT_REGISTER + ASAP_R22:			info->i = src2val[REGBASE + 22];		break;
		case CPUINFO_INT_REGISTER + ASAP_R23:			info->i = src2val[REGBASE + 23];		break;
		case CPUINFO_INT_REGISTER + ASAP_R24:			info->i = src2val[REGBASE + 24];		break;
		case CPUINFO_INT_REGISTER + ASAP_R25:			info->i = src2val[REGBASE + 25];		break;
		case CPUINFO_INT_REGISTER + ASAP_R26:			info->i = src2val[REGBASE + 26];		break;
		case CPUINFO_INT_REGISTER + ASAP_R27:			info->i = src2val[REGBASE + 27];		break;
		case CPUINFO_INT_REGISTER + ASAP_R28:			info->i = src2val[REGBASE + 28];		break;
		case CPUINFO_INT_REGISTER + ASAP_R29:			info->i = src2val[REGBASE + 29];		break;
		case CPUINFO_INT_REGISTER + ASAP_R30:			info->i = src2val[REGBASE + 30];		break;
		case CPUINFO_INT_REGISTER + ASAP_R31:			info->i = src2val[REGBASE + 31];		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = asap_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = asap_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = asap_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = asap_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = asap_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = asap_exit;					break;
		case CPUINFO_PTR_EXECUTE:						info->execute = asap_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = asap_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &asap_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ASAP");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Atari ASAP");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C) Aaron Giles 2000-2004"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + ASAP_PC:  			sprintf(info->s, "PC: %08X", asap.pc);	break;
		case CPUINFO_STR_REGISTER + ASAP_PS:  			sprintf(info->s, "PS: %08X", GET_FLAGS(&asap)); break;

		case CPUINFO_STR_REGISTER + ASAP_R0:			sprintf(info->s, "R0: %08X", asap.r[0]); break;
		case CPUINFO_STR_REGISTER + ASAP_R1:			sprintf(info->s, "R1: %08X", asap.r[1]); break;
		case CPUINFO_STR_REGISTER + ASAP_R2:			sprintf(info->s, "R2: %08X", asap.r[2]); break;
		case CPUINFO_STR_REGISTER + ASAP_R3:			sprintf(info->s, "R3: %08X", asap.r[3]); break;
		case CPUINFO_STR_REGISTER + ASAP_R4:			sprintf(info->s, "R4: %08X", asap.r[4]); break;
		case CPUINFO_STR_REGISTER + ASAP_R5:			sprintf(info->s, "R5: %08X", asap.r[5]); break;
		case CPUINFO_STR_REGISTER + ASAP_R6:			sprintf(info->s, "R6: %08X", asap.r[6]); break;
		case CPUINFO_STR_REGISTER + ASAP_R7:			sprintf(info->s, "R7: %08X", asap.r[7]); break;
		case CPUINFO_STR_REGISTER + ASAP_R8:			sprintf(info->s, "R8: %08X", asap.r[8]); break;
		case CPUINFO_STR_REGISTER + ASAP_R9:			sprintf(info->s, "R9: %08X", asap.r[9]); break;
		case CPUINFO_STR_REGISTER + ASAP_R10:			sprintf(info->s, "R10:%08X", asap.r[10]); break;
		case CPUINFO_STR_REGISTER + ASAP_R11:			sprintf(info->s, "R11:%08X", asap.r[11]); break;
		case CPUINFO_STR_REGISTER + ASAP_R12:			sprintf(info->s, "R12:%08X", asap.r[12]); break;
		case CPUINFO_STR_REGISTER + ASAP_R13:			sprintf(info->s, "R13:%08X", asap.r[13]); break;
		case CPUINFO_STR_REGISTER + ASAP_R14:			sprintf(info->s, "R14:%08X", asap.r[14]); break;
		case CPUINFO_STR_REGISTER + ASAP_R15:			sprintf(info->s, "R15:%08X", asap.r[15]); break;
		case CPUINFO_STR_REGISTER + ASAP_R16:			sprintf(info->s, "R16:%08X", asap.r[16]); break;
		case CPUINFO_STR_REGISTER + ASAP_R17:			sprintf(info->s, "R17:%08X", asap.r[17]); break;
		case CPUINFO_STR_REGISTER + ASAP_R18:			sprintf(info->s, "R18:%08X", asap.r[18]); break;
		case CPUINFO_STR_REGISTER + ASAP_R19:			sprintf(info->s, "R19:%08X", asap.r[19]); break;
		case CPUINFO_STR_REGISTER + ASAP_R20:			sprintf(info->s, "R20:%08X", asap.r[20]); break;
		case CPUINFO_STR_REGISTER + ASAP_R21:			sprintf(info->s, "R21:%08X", asap.r[21]); break;
		case CPUINFO_STR_REGISTER + ASAP_R22:			sprintf(info->s, "R22:%08X", asap.r[22]); break;
		case CPUINFO_STR_REGISTER + ASAP_R23:			sprintf(info->s, "R23:%08X", asap.r[23]); break;
		case CPUINFO_STR_REGISTER + ASAP_R24:			sprintf(info->s, "R24:%08X", asap.r[24]); break;
		case CPUINFO_STR_REGISTER + ASAP_R25:			sprintf(info->s, "R25:%08X", asap.r[25]); break;
		case CPUINFO_STR_REGISTER + ASAP_R26:			sprintf(info->s, "R26:%08X", asap.r[26]); break;
		case CPUINFO_STR_REGISTER + ASAP_R27:			sprintf(info->s, "R27:%08X", asap.r[27]); break;
		case CPUINFO_STR_REGISTER + ASAP_R28:			sprintf(info->s, "R28:%08X", asap.r[28]); break;
		case CPUINFO_STR_REGISTER + ASAP_R29:			sprintf(info->s, "R29:%08X", asap.r[29]); break;
		case CPUINFO_STR_REGISTER + ASAP_R30:			sprintf(info->s, "R30:%08X", asap.r[30]); break;
		case CPUINFO_STR_REGISTER + ASAP_R31:			sprintf(info->s, "R31:%08X", asap.r[31]); break;
	}
}
