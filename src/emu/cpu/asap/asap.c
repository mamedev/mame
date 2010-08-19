/***************************************************************************

    asap.c
    Core implementation for the portable ASAP emulator.
    ASAP = Atari Simplified Architecture Processor

    Written by Aaron Giles
    Special thanks to Mike Albaugh for clarification on a couple of fine points.

***************************************************************************/

#include "emu.h"
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


#define SET_C_ADD(A,a,b)		((A)->cflag = (UINT32)(b) > (UINT32)(~(a)))
#define SET_C_SUB(A,a,b)		((A)->cflag = (UINT32)(b) <= (UINT32)(a))
#define SET_V_ADD(A,r,a,b)		((A)->vflag = ~((a) ^ (b)) & ((a) ^ (r)))
#define SET_V_SUB(A,r,a,b)		((A)->vflag =  ((a) ^ (b)) & ((a) ^ (r)))
#define SET_ZN(A,r)				((A)->znflag = (r))
#define SET_ZNCV_ADD(A,r,a,b)	SET_ZN(A,r); SET_C_ADD(A,a,b); SET_V_ADD(A,r,a,b)
#define SET_ZNCV_SUB(A,r,a,b)	SET_ZN(A,r); SET_C_SUB(A,a,b); SET_V_SUB(A,r,a,b)

#define SET_VFLAG(A,val)		((A)->vflag = (val) << 31)
#define SET_CFLAG(A,val)		((A)->cflag = (val))

#define GET_FLAGS(A)			((A)->cflag | \
								 (((A)->vflag >> 30) & PS_VFLAG) | \
								 (((A)->znflag == 0) << 2) | \
								 (((A)->znflag >> 28) & PS_NFLAG) | \
								 ((A)->iflag << 4) | \
								 ((A)->pflag << 5))

#define SET_FLAGS(A,v)			do { \
									(A)->cflag = (v) & PS_CFLAG; \
									(A)->vflag = ((v) & PS_VFLAG) << 30; \
									(A)->znflag = ((v) & PS_ZFLAG) ? 0 : ((v) & PS_NFLAG) ? -1 : 1; \
									(A)->iflag = ((v) & PS_IFLAG) >> 4; \
									(A)->pflag = ((v) & PS_PFLAG) >> 5; \
								} while (0);


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* ASAP Registers */
typedef struct _asap_state asap_state;
struct _asap_state
{
	/* core registers */
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
	int			icount;
	device_irq_callback irq_callback;
	address_space *program;
	legacy_cpu_device *device;

	/* src2val table, registers are at the end */
	UINT32		src2val[65536];
};



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static void (**opcode)(asap_state *);



/***************************************************************************
    OPCODE TABLE
***************************************************************************/

static void noop(asap_state *);
static void trap0(asap_state *);
static void bsp(asap_state *);
static void bmz(asap_state *);
static void bgt(asap_state *);
static void ble(asap_state *);
static void bge(asap_state *);
static void blt(asap_state *);
static void bhi(asap_state *);
static void bls(asap_state *);
static void bcc(asap_state *);
static void bcs(asap_state *);
static void bpl(asap_state *);
static void bmi(asap_state *);
static void bne(asap_state *);
static void beq(asap_state *);
static void bvc(asap_state *);
static void bvs(asap_state *);
static void bsr(asap_state *);
static void bsr_0(asap_state *);
static void lea(asap_state *);
static void lea_c(asap_state *);
static void lea_c0(asap_state *);
static void leah(asap_state *);
static void leah_c(asap_state *);
static void leah_c0(asap_state *);
static void subr(asap_state *);
static void subr_c(asap_state *);
static void subr_c0(asap_state *);
static void xor_(asap_state *);
static void xor_c(asap_state *);
static void xor_c0(asap_state *);
static void xorn(asap_state *);
static void xorn_c(asap_state *);
static void xorn_c0(asap_state *);
static void add(asap_state *);
static void add_c(asap_state *);
static void add_c0(asap_state *);
static void sub(asap_state *);
static void sub_c(asap_state *);
static void sub_c0(asap_state *);
static void addc(asap_state *);
static void addc_c(asap_state *);
static void addc_c0(asap_state *);
static void subc(asap_state *);
static void subc_c(asap_state *);
static void subc_c0(asap_state *);
static void and_(asap_state *);
static void and_c(asap_state *);
static void and_c0(asap_state *);
static void andn(asap_state *);
static void andn_c(asap_state *);
static void andn_c0(asap_state *);
static void or_(asap_state *);
static void or_c(asap_state *);
static void or_c0(asap_state *);
static void orn(asap_state *);
static void orn_c(asap_state *);
static void orn_c0(asap_state *);
static void ld(asap_state *);
static void ld_0(asap_state *);
static void ld_c(asap_state *);
static void ld_c0(asap_state *);
static void ldh(asap_state *);
static void ldh_0(asap_state *);
static void ldh_c(asap_state *);
static void ldh_c0(asap_state *);
static void lduh(asap_state *);
static void lduh_0(asap_state *);
static void lduh_c(asap_state *);
static void lduh_c0(asap_state *);
static void sth(asap_state *);
static void sth_0(asap_state *);
static void sth_c(asap_state *);
static void sth_c0(asap_state *);
static void st(asap_state *);
static void st_0(asap_state *);
static void st_c(asap_state *);
static void st_c0(asap_state *);
static void ldb(asap_state *);
static void ldb_0(asap_state *);
static void ldb_c(asap_state *);
static void ldb_c0(asap_state *);
static void ldub(asap_state *);
static void ldub_0(asap_state *);
static void ldub_c(asap_state *);
static void ldub_c0(asap_state *);
static void stb(asap_state *);
static void stb_0(asap_state *);
static void stb_c(asap_state *);
static void stb_c0(asap_state *);
static void ashr(asap_state *);
static void ashr_c(asap_state *);
static void ashr_c0(asap_state *);
static void lshr(asap_state *);
static void lshr_c(asap_state *);
static void lshr_c0(asap_state *);
static void ashl(asap_state *);
static void ashl_c(asap_state *);
static void ashl_c0(asap_state *);
static void rotl(asap_state *);
static void rotl_c(asap_state *);
static void rotl_c0(asap_state *);
static void getps(asap_state *);
static void putps(asap_state *);
static void jsr(asap_state *);
static void jsr_0(asap_state *);
static void jsr_c(asap_state *);
static void jsr_c0(asap_state *);
static void trapf(asap_state *);

typedef void (*asap_ophandler)(asap_state *);

static const asap_ophandler opcodetable[32][4] =
{
	{	trap0,		trap0,		trap0,		trap0		},
	{	NULL,		NULL,		NULL,		NULL		},
	{	bsr,		bsr_0,		bsr,		bsr_0		},
	{	lea,		noop,		lea_c,		lea_c0		},
	{	leah,		noop,		leah_c,		leah_c0		},
	{	subr,		noop,		subr_c,		subr_c0		},
	{	xor_,		noop,		xor_c,		xor_c0		},
	{	xorn,		noop,		xorn_c,		xorn_c0		},
	{	add,		noop,		add_c,		add_c0		},
	{	sub,		noop,		sub_c,		sub_c0		},
	{	addc,		noop,		addc_c,		addc_c0		},
	{	subc,		noop,		subc_c,		subc_c0		},
	{	and_,		noop,		and_c,		and_c0		},
	{	andn,		noop,		andn_c,		andn_c0		},
	{	or_,		noop,		or_c,		or_c0		},
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

static void (*const conditiontable[16])(asap_state *) =
{
	bsp, bmz, bgt, ble, bge, blt, bhi, bls,
	bcc, bcs, bpl, bmi, bne, beq, bvc, bvs
};



/***************************************************************************
    STATE ACCESSORS
***************************************************************************/

INLINE asap_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == ASAP);
	return (asap_state *)downcast<legacy_cpu_device *>(device)->token();
}



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(A,pc)	memory_decrypted_read_dword((A)->program, pc)


INLINE UINT8 READBYTE(asap_state *asap, offs_t address)
{
	/* no alignment issues with bytes */
	return asap->program->read_byte(address);
}

INLINE UINT16 READWORD(asap_state *asap, offs_t address)
{
	/* aligned reads are easy */
	if (!(address & 1))
		return asap->program->read_word(address);

	/* misaligned reads are tricky */
	return asap->program->read_dword(address & ~3) >> (address & 3);
}

INLINE UINT32 READLONG(asap_state *asap, offs_t address)
{
	/* aligned reads are easy */
	if (!(address & 3))
		return asap->program->read_dword(address);

	/* misaligned reads are tricky */
	return asap->program->read_dword(address & ~3) >> (address & 3);
}

INLINE void WRITEBYTE(asap_state *asap, offs_t address, UINT8 data)
{
	/* no alignment issues with bytes */
	asap->program->write_byte(address, data);
}

INLINE void WRITEWORD(asap_state *asap, offs_t address, UINT16 data)
{
	/* aligned writes are easy */
	if (!(address & 1))
	{
		asap->program->write_word(address, data);
		return;
	}

	/* misaligned writes are tricky */
	if (!(address & 2))
	{
		asap->program->write_byte(address + 1, data);
		asap->program->write_byte(address + 2, data >> 8);
	}
	else
		asap->program->write_byte(address + 1, data);
}

INLINE void WRITELONG(asap_state *asap, offs_t address, UINT32 data)
{
	/* aligned writes are easy */
	if (!(address & 3))
	{
		asap->program->write_dword(address, data);
		return;
	}

	/* misaligned writes are tricky */
	switch (address & 3)
	{
		case 1:
			asap->program->write_byte(address, data);
			asap->program->write_word(address + 1, data >> 8);
			break;
		case 2:
			asap->program->write_word(address, data);
			break;
		case 3:
			asap->program->write_byte(address, data);
			break;
	}
}



/***************************************************************************
    EXCEPTION HANDLING
***************************************************************************/

INLINE void generate_exception(asap_state *asap, int exception)
{
	asap->pflag = asap->iflag;
	asap->iflag = 0;

	asap->src2val[REGBASE + 30] = asap->pc;
	asap->src2val[REGBASE + 31] = (asap->nextpc == ~0) ? asap->pc + 4 : asap->nextpc;

	asap->pc = 0x40 * exception;
	asap->nextpc = ~0;

	asap->icount--;
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

INLINE void check_irqs(asap_state *asap)
{
	if (asap->irq_state && asap->iflag)
	{
		generate_exception(asap, EXCEPTION_INTERRUPT);
		if (asap->irq_callback != NULL)
			(*asap->irq_callback)(asap->device, ASAP_IRQ0);
	}
}


static void set_irq_line(asap_state *asap, int irqline, int state)
{
	asap->irq_state = (state != CLEAR_LINE);
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void init_tables(running_machine *machine)
{
	/* allocate opcode table */
	if (!opcode)
		opcode = auto_alloc_array(machine, asap_ophandler, 32 * 32 * 2);

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
}

static CPU_INIT( asap )
{
	asap_state *asap = get_safe_token(device);
	int i;

	init_tables(device->machine);
	for (i = 0; i < REGBASE; i++)
		asap->src2val[i] = i;
	asap->irq_callback = irqcallback;
	asap->device = device;
	asap->program = device->space(AS_PROGRAM);


	state_save_register_device_item(device, 0, asap->pc);
	state_save_register_device_item(device, 0, asap->pflag);
	state_save_register_device_item(device, 0, asap->iflag);
	state_save_register_device_item(device, 0, asap->cflag);
	state_save_register_device_item(device, 0, asap->vflag);
	state_save_register_device_item(device, 0, asap->znflag);
	state_save_register_device_item(device, 0, asap->op);
	state_save_register_device_item(device, 0, asap->ppc);
	state_save_register_device_item(device, 0, asap->nextpc);
	state_save_register_device_item(device, 0, asap->irq_state);
}

static CPU_RESET( asap )
{
	asap_state *asap = get_safe_token(device);

	/* initialize the state */
	asap->src2val[REGBASE + 0] = 0;
	asap->pc = 0;
	asap->iflag = 0;

	asap->ppc = 0;
	asap->nextpc = ~0;
	asap->irq_state = 0;
	asap->irq_callback = NULL;
}


static CPU_EXIT( asap )
{
	opcode = NULL;
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

INLINE void fetch_instruction(asap_state *asap)
{
	/* debugging */
	asap->ppc = asap->pc;

	/* instruction fetch */
	asap->op.d = ROPCODE(asap, asap->pc);
	asap->pc += 4;
}

INLINE void fetch_instruction_debug(asap_state *asap)
{
	/* debugging */
	asap->ppc = asap->pc;
	debugger_instruction_hook(asap->device, asap->pc);

	/* instruction fetch */
	asap->op.d = ROPCODE(asap, asap->pc);
	asap->pc += 4;
}

INLINE void execute_instruction(asap_state *asap)
{
	/* parse the instruction */
	(*opcode[asap->op.d >> 21])(asap);
}

static CPU_EXECUTE( asap )
{
	asap_state *asap = get_safe_token(device);

	/* check for IRQs */
	check_irqs(asap);

	/* core execution loop */
	if ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		do
		{
			/* fetch and execute the next instruction */
			fetch_instruction(asap);
			execute_instruction(asap);

			/* fetch and execute the next instruction */
			fetch_instruction(asap);
			execute_instruction(asap);

			/* fetch and execute the next instruction */
			fetch_instruction(asap);
			execute_instruction(asap);

			/* fetch and execute the next instruction */
			fetch_instruction(asap);
			execute_instruction(asap);

			asap->icount -= 4;

		} while (asap->icount > 0);
	}
	else
	{
		do
		{
			/* fetch and execute the next instruction */
			fetch_instruction_debug(asap);
			execute_instruction(asap);

			/* fetch and execute the next instruction */
			fetch_instruction_debug(asap);
			execute_instruction(asap);

			/* fetch and execute the next instruction */
			fetch_instruction_debug(asap);
			execute_instruction(asap);

			/* fetch and execute the next instruction */
			fetch_instruction_debug(asap);
			execute_instruction(asap);

			asap->icount -= 4;

		} while (asap->icount > 0);
	}
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

extern CPU_DISASSEMBLE( asap );



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define OPCODE(A)	((A)->op.d >> 27)
#define DSTREG(A)	(((A)->op.d >> 22) & 31)
#define DSTVAL(A)	(A)->src2val[REGBASE + DSTREG(A)]
#define SRC1REG(A)	(((A)->op.d >> 16) & 31)
#define SRC1VAL(A)	(A)->src2val[REGBASE + SRC1REG(A)]
#define SRC2VAL(A)	(A)->src2val[(A)->op.w.l]



/***************************************************************************
    OPCODES
***************************************************************************/

static void noop(asap_state *asap)
{
}

/**************************** TRAP 0 ******************************/

static void trap0(asap_state *asap)
{
	generate_exception(asap, EXCEPTION_TRAP0);
}

/**************************** Bcc ******************************/

static void bsp(asap_state *asap)
{
	if ((INT32)asap->znflag > 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bmz(asap_state *asap)
{
	if ((INT32)asap->znflag <= 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bgt(asap_state *asap)
{
	if (asap->znflag != 0 && (INT32)(asap->znflag ^ asap->vflag) >= 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void ble(asap_state *asap)
{
	if (asap->znflag == 0 || (INT32)(asap->znflag ^ asap->vflag) < 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bge(asap_state *asap)
{
	if ((INT32)(asap->znflag ^ asap->vflag) >= 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void blt(asap_state *asap)
{
	if ((INT32)(asap->znflag ^ asap->vflag) < 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bhi(asap_state *asap)
{
	if (asap->znflag != 0 && asap->cflag)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bls(asap_state *asap)
{
	if (asap->znflag == 0 || !asap->cflag)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bcc(asap_state *asap)
{
	if (!asap->cflag)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bcs(asap_state *asap)
{
	if (asap->cflag)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bpl(asap_state *asap)
{
	if ((INT32)asap->znflag >= 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bmi(asap_state *asap)
{
	if ((INT32)asap->znflag < 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bne(asap_state *asap)
{
	if (asap->znflag != 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void beq(asap_state *asap)
{
	if (asap->znflag == 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bvc(asap_state *asap)
{
	if ((INT32)asap->vflag >= 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

static void bvs(asap_state *asap)
{
	if ((INT32)asap->vflag < 0)
	{
		asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

		fetch_instruction(asap);
		asap->pc = asap->nextpc;
		asap->nextpc = ~0;

		execute_instruction(asap);
		asap->icount--;
	}
}

/**************************** BSR ******************************/

static void bsr(asap_state *asap)
{
	DSTVAL(asap) = asap->pc + 4;
	asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

	fetch_instruction(asap);
	asap->pc = asap->nextpc;
	asap->nextpc = ~0;

	execute_instruction(asap);
	asap->icount--;
}

static void bsr_0(asap_state *asap)
{
	asap->nextpc = asap->ppc + ((INT32)(asap->op.d << 10) >> 8);

	fetch_instruction(asap);
	asap->pc = asap->nextpc;
	asap->nextpc = ~0;

	execute_instruction(asap);
	asap->icount--;
}

/**************************** LEA ******************************/

static void lea(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) + (SRC2VAL(asap) << 2);
}

static void lea_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + (src2 << 2);

	SET_ZNCV_ADD(asap, dst, src1, src2);
	if (src1 & 0xc0000000)
		SET_CFLAG(asap, 1);
	if (((src1 ^ (src1 >> 1)) & 0x20000000) || (src1 ^ (src1 >> 2)) & 0x20000000)
		SET_VFLAG(asap, 1);
	DSTVAL(asap) = dst;
}

static void lea_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + (src2 << 2);

	SET_ZNCV_ADD(asap, dst, src1, src2);
	if (src1 & 0xc0000000)
		SET_CFLAG(asap, 1);
	if (((src1 ^ (src1 >> 1)) & 0x20000000) || (src1 ^ (src1 >> 2)) & 0x20000000)
		SET_VFLAG(asap, 1);
}

/**************************** LEAH ******************************/

static void leah(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) + (SRC2VAL(asap) << 1);
}

static void leah_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + (src2 << 1);

	SET_ZNCV_ADD(asap, dst, src1, src2);
	if (src1 & 0x80000000)
		SET_CFLAG(asap, 1);
	if ((src1 ^ (src1 >> 1)) & 0x40000000)
		SET_VFLAG(asap, 1);
	DSTVAL(asap) = dst;
}

static void leah_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + (src2 << 1);

	SET_ZNCV_ADD(asap, dst, src1, src2);
	if (src1 & 0x80000000)
		SET_CFLAG(asap, 1);
	if ((src1 ^ (src1 >> 1)) & 0x40000000)
		SET_VFLAG(asap, 1);
}

/**************************** SUBR ******************************/

static void subr(asap_state *asap)
{
	DSTVAL(asap) = SRC2VAL(asap) - SRC1VAL(asap);
}

static void subr_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src2 - src1;

	SET_ZNCV_SUB(asap, dst, src2, src1);
	DSTVAL(asap) = dst;
}

static void subr_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src2 - src1;

	SET_ZNCV_SUB(asap, dst, src2, src1);
}

/**************************** XOR ******************************/

static void xor_(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) ^ SRC2VAL(asap);
}

static void xor_c(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) ^ SRC2VAL(asap);
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void xor_c0(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) ^ SRC2VAL(asap);
	SET_ZN(asap, dst);
}

/**************************** XOR ******************************/

static void xorn(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) ^ ~SRC2VAL(asap);
}

static void xorn_c(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) ^ ~SRC2VAL(asap);
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void xorn_c0(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) ^ ~SRC2VAL(asap);
	SET_ZN(asap, dst);
}

/**************************** ADD ******************************/

static void add(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) + SRC2VAL(asap);
}

static void add_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + src2;

	SET_ZNCV_ADD(asap, dst, src1, src2);
	DSTVAL(asap) = dst;
}

static void add_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + src2;

	SET_ZNCV_ADD(asap, dst, src1, src2);
}

/**************************** ADD ******************************/

static void sub(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) - SRC2VAL(asap);
}

static void sub_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 - src2;

	SET_ZNCV_SUB(asap, dst, src1, src2);
	DSTVAL(asap) = dst;
}

static void sub_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 - src2;

	SET_ZNCV_SUB(asap, dst, src1, src2);
}

/**************************** ADDC ******************************/

static void addc(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) + SRC2VAL(asap) + asap->cflag;
}

static void addc_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + src2 + asap->cflag;

	SET_ZNCV_ADD(asap, dst, src1, src2);
	DSTVAL(asap) = dst;
}

static void addc_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 + src2 + asap->cflag;

	SET_ZNCV_ADD(asap, dst, src1, src2);
}

/**************************** SUBC ******************************/

static void subc(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) - SRC2VAL(asap) - 1 + asap->cflag;
}

static void subc_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 - src2 - 1 + asap->cflag;

	SET_ZNCV_SUB(asap, dst, src1, src2);
	DSTVAL(asap) = dst;
}

static void subc_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap);
	UINT32 dst = src1 - src2 - 1 + asap->cflag;

	SET_ZNCV_SUB(asap, dst, src1, src2);
}

/**************************** AND ******************************/

static void and_(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) & SRC2VAL(asap);
}

static void and_c(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) & SRC2VAL(asap);
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void and_c0(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) & SRC2VAL(asap);
	SET_ZN(asap, dst);
}

/**************************** ANDN ******************************/

static void andn(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) & ~SRC2VAL(asap);
}

static void andn_c(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) & ~SRC2VAL(asap);
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void andn_c0(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) & ~SRC2VAL(asap);
	SET_ZN(asap, dst);
}

/**************************** OR ******************************/

static void or_(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) | SRC2VAL(asap);
}

static void or_c(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) | SRC2VAL(asap);
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void or_c0(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) | SRC2VAL(asap);
	SET_ZN(asap, dst);
}

/**************************** ORN ******************************/

static void orn(asap_state *asap)
{
	DSTVAL(asap) = SRC1VAL(asap) | ~SRC2VAL(asap);
}

static void orn_c(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) | ~SRC2VAL(asap);
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void orn_c0(asap_state *asap)
{
	UINT32 dst = SRC1VAL(asap) | ~SRC2VAL(asap);
	SET_ZN(asap, dst);
}

/**************************** LD ******************************/

static void ld(asap_state *asap)
{
	DSTVAL(asap) = READLONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2));
}

static void ld_0(asap_state *asap)
{
	READLONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2));
}

static void ld_c(asap_state *asap)
{
	UINT32 dst = READLONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2));
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void ld_c0(asap_state *asap)
{
	UINT32 dst = READLONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2));
	SET_ZN(asap, dst);
}

/**************************** LDH ******************************/

static void ldh(asap_state *asap)
{
	DSTVAL(asap) = (INT16)READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
}

static void ldh_0(asap_state *asap)
{
	READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
}

static void ldh_c(asap_state *asap)
{
	UINT32 dst = (INT16)READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void ldh_c0(asap_state *asap)
{
	UINT32 dst = (INT16)READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
	SET_ZN(asap, dst);
}

/**************************** LDUH ******************************/

static void lduh(asap_state *asap)
{
	DSTVAL(asap) = READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
}

static void lduh_0(asap_state *asap)
{
	READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
}

static void lduh_c(asap_state *asap)
{
	UINT32 dst = READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void lduh_c0(asap_state *asap)
{
	UINT32 dst = READWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1));
	SET_ZN(asap, dst);
}

/**************************** STH ******************************/

static void sth(asap_state *asap)
{
	WRITEWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1), DSTVAL(asap));
}

static void sth_0(asap_state *asap)
{
	WRITEWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1), 0);
}

static void sth_c(asap_state *asap)
{
	UINT32 dst = (UINT16)DSTVAL(asap);
	SET_ZN(asap, dst);
	WRITEWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1), dst);
}

static void sth_c0(asap_state *asap)
{
	SET_ZN(asap, 0);
	WRITEWORD(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 1), 0);
}

/**************************** ST ******************************/

static void st(asap_state *asap)
{
	WRITELONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2), DSTVAL(asap));
}

static void st_0(asap_state *asap)
{
	WRITELONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2), 0);
}

static void st_c(asap_state *asap)
{
	UINT32 dst = DSTVAL(asap);
	SET_ZN(asap, dst);
	WRITELONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2), dst);
}

static void st_c0(asap_state *asap)
{
	SET_ZN(asap, 0);
	WRITELONG(asap, SRC1VAL(asap) + (SRC2VAL(asap) << 2), 0);
}

/**************************** LDB ******************************/

static void ldb(asap_state *asap)
{
	DSTVAL(asap) = (INT8)READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
}

static void ldb_0(asap_state *asap)
{
	READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
}

static void ldb_c(asap_state *asap)
{
	UINT32 dst = (INT8)READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void ldb_c0(asap_state *asap)
{
	UINT32 dst = (INT8)READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
	SET_ZN(asap, dst);
}

/**************************** LDUB ******************************/

static void ldub(asap_state *asap)
{
	DSTVAL(asap) = READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
}

static void ldub_0(asap_state *asap)
{
	READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
}

static void ldub_c(asap_state *asap)
{
	UINT32 dst = READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void ldub_c0(asap_state *asap)
{
	UINT32 dst = READBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap));
	SET_ZN(asap, dst);
}

/**************************** STB ******************************/

static void stb(asap_state *asap)
{
	WRITEBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap), DSTVAL(asap));
}

static void stb_0(asap_state *asap)
{
	WRITEBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap), 0);
}

static void stb_c(asap_state *asap)
{
	UINT32 dst = (UINT8)DSTVAL(asap);
	SET_ZN(asap, dst);
	WRITEBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap), dst);
}

static void stb_c0(asap_state *asap)
{
	SET_ZN(asap, 0);
	WRITEBYTE(asap, SRC1VAL(asap) + SRC2VAL(asap), 0);
}

/**************************** ASHR ******************************/

static void ashr(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	DSTVAL(asap) = (src2 < 32) ? ((INT32)SRC1VAL(asap) >> src2) : ((INT32)SRC1VAL(asap) >> 31);
}

static void ashr_c(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	asap->cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL(asap);
		UINT32 dst = (INT32)src1 >> src2;
		SET_ZN(asap, dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap->cflag = src1 >> 31;
		}
		DSTVAL(asap) = dst;
	}
	else
	{
		UINT32 dst = (INT32)SRC1VAL(asap) >> 31;
		SET_ZN(asap, dst);
		DSTVAL(asap) = dst;
	}
}

static void ashr_c0(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	asap->cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL(asap);
		UINT32 dst = (INT32)src1 >> src2;
		SET_ZN(asap, dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap->cflag = src1 >> 31;
		}
	}
	else
	{
		UINT32 dst = (INT32)SRC1VAL(asap) >> 31;
		SET_ZN(asap, dst);
	}
}

/**************************** LSHR ******************************/

static void lshr(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	DSTVAL(asap) = (src2 < 32) ? (SRC1VAL(asap) >> src2) : (SRC1VAL(asap) >> 31);
}

static void lshr_c(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	asap->cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL(asap);
		UINT32 dst = src1 >> src2;
		SET_ZN(asap, dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap->cflag = src1 >> 31;
		}
		DSTVAL(asap) = dst;
	}
	else
	{
		UINT32 dst = SRC1VAL(asap) >> 31;
		SET_ZN(asap, dst);
		DSTVAL(asap) = dst;
	}
}

static void lshr_c0(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	asap->cflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL(asap);
		UINT32 dst = src1 >> src2;
		SET_ZN(asap, dst);
		if (src2 != 0)
		{
			src1 = src1 << (32 - src2);
			asap->cflag = src1 >> 31;
		}
	}
	else
	{
		SET_ZN(asap, 0);
		DSTVAL(asap) = 0;
	}
}

/**************************** ASHL ******************************/

static void ashl(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	DSTVAL(asap) = (src2 < 32) ? (SRC1VAL(asap) << src2) : 0;
}

static void ashl_c(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	asap->cflag = asap->vflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL(asap);
		UINT32 dst = src1 << src2;
		SET_ZN(asap, dst);
		if (src2 != 0)
		{
			src1 = (INT32)src1 >> (32 - src2);
			asap->cflag = src1 & PS_CFLAG;
			asap->vflag = (src1 != ((INT32)dst >> 31)) << 31;
		}
		DSTVAL(asap) = dst;
	}
	else
	{
		SET_ZN(asap, 0);
		DSTVAL(asap) = 0;
	}
}

static void ashl_c0(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap);
	asap->cflag = asap->vflag = 0;
	if (src2 < 32)
	{
		UINT32 src1 = SRC1VAL(asap);
		UINT32 dst = src1 << src2;
		SET_ZN(asap, dst);
		if (src2 != 0)
		{
			src1 = (INT32)src1 >> (32 - src2);
			asap->cflag = src1 & PS_CFLAG;
			asap->vflag = (src1 != ((INT32)dst >> 31)) << 31;
		}
	}
	else
		SET_ZN(asap, 0);
}

/**************************** ROTL ******************************/

static void rotl(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap) & 31;
	DSTVAL(asap) = (src1 << src2) | (src1 >> (32 - src2));
}

static void rotl_c(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap) & 31;
	UINT32 dst = (src1 << src2) | (src1 >> (32 - src2));
	SET_ZN(asap, dst);
	DSTVAL(asap) = dst;
}

static void rotl_c0(asap_state *asap)
{
	UINT32 src1 = SRC1VAL(asap);
	UINT32 src2 = SRC2VAL(asap) & 31;
	UINT32 dst = (src1 << src2) | (src1 >> (32 - src2));
	SET_ZN(asap, dst);
}

/**************************** GETPS ******************************/

static void getps(asap_state *asap)
{
	DSTVAL(asap) = GET_FLAGS(asap);
}

/**************************** PUTPS ******************************/

static void putps(asap_state *asap)
{
	UINT32 src2 = SRC2VAL(asap) & 0x3f;
	SET_FLAGS(asap, src2);
	check_irqs(asap);
}

/**************************** JSR ******************************/

static void jsr(asap_state *asap)
{
	DSTVAL(asap) = asap->pc + 4;
	asap->nextpc = SRC1VAL(asap) + (SRC2VAL(asap) << 2);

	fetch_instruction(asap);
	asap->pc = asap->nextpc;
	asap->nextpc = ~0;

	execute_instruction(asap);
	asap->icount--;
}

static void jsr_0(asap_state *asap)
{
	asap->nextpc = SRC1VAL(asap) + (SRC2VAL(asap) << 2);

	fetch_instruction(asap);
	asap->pc = asap->nextpc;
	asap->nextpc = ~0;

	execute_instruction(asap);
	asap->icount--;
}

static void jsr_c(asap_state *asap)
{
	DSTVAL(asap) = asap->pc + 4;
	asap->nextpc = SRC1VAL(asap) + (SRC2VAL(asap) << 2);
	asap->iflag = asap->pflag;

	fetch_instruction(asap);
	asap->pc = asap->nextpc;
	asap->nextpc = ~0;

	execute_instruction(asap);
	asap->icount--;
	check_irqs(asap);
}

static void jsr_c0(asap_state *asap)
{
	asap->nextpc = SRC1VAL(asap) + (SRC2VAL(asap) << 2);
	asap->iflag = asap->pflag;

	fetch_instruction(asap);
	asap->pc = asap->nextpc;
	asap->nextpc = ~0;

	execute_instruction(asap);
	asap->icount--;
	check_irqs(asap);
}

/**************************** TRAP F ******************************/

static void trapf(asap_state *asap)
{
	generate_exception(asap, EXCEPTION_TRAPF);
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( asap )
{
	asap_state *asap = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + ASAP_IRQ0:	set_irq_line(asap, ASAP_IRQ0, info->i);		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ASAP_PC:	asap->pc = info->i;								break;
		case CPUINFO_INT_REGISTER + ASAP_PS:	SET_FLAGS(asap, info->i);						break;

		case CPUINFO_INT_REGISTER + ASAP_R0:	asap->src2val[REGBASE + 0] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R1:	asap->src2val[REGBASE + 1] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R2:	asap->src2val[REGBASE + 2] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R3:	asap->src2val[REGBASE + 3] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R4:	asap->src2val[REGBASE + 4] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R5:	asap->src2val[REGBASE + 5] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R6:	asap->src2val[REGBASE + 6] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R7:	asap->src2val[REGBASE + 7] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R8:	asap->src2val[REGBASE + 8] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R9:	asap->src2val[REGBASE + 9] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R10:	asap->src2val[REGBASE + 10] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R11:	asap->src2val[REGBASE + 11] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R12:	asap->src2val[REGBASE + 12] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R13:	asap->src2val[REGBASE + 13] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R14:	asap->src2val[REGBASE + 14] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R15:	asap->src2val[REGBASE + 15] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R16:	asap->src2val[REGBASE + 16] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R17:	asap->src2val[REGBASE + 17] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R18:	asap->src2val[REGBASE + 18] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R19:	asap->src2val[REGBASE + 19] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R20:	asap->src2val[REGBASE + 20] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R21:	asap->src2val[REGBASE + 21] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R22:	asap->src2val[REGBASE + 22] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R23:	asap->src2val[REGBASE + 23] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R24:	asap->src2val[REGBASE + 24] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R25:	asap->src2val[REGBASE + 25] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R26:	asap->src2val[REGBASE + 26] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R27:	asap->src2val[REGBASE + 27] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R28:	asap->src2val[REGBASE + 28] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R29:	asap->src2val[REGBASE + 29] = info->i;			break;
		case CPUINFO_INT_REGISTER + ASAP_R30:	asap->src2val[REGBASE + 30] = info->i;			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + ASAP_R31:	asap->src2val[REGBASE + 31] = info->i;			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( asap )
{
	asap_state *asap = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(asap_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 12;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + ASAP_IRQ0:		info->i = asap->irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = asap->ppc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ASAP_PC:			info->i = asap->pc;						break;
		case CPUINFO_INT_REGISTER + ASAP_PS:			info->i = GET_FLAGS(asap);				break;

		case CPUINFO_INT_REGISTER + ASAP_R0:			info->i = asap->src2val[REGBASE + 0];	break;
		case CPUINFO_INT_REGISTER + ASAP_R1:			info->i = asap->src2val[REGBASE + 1];	break;
		case CPUINFO_INT_REGISTER + ASAP_R2:			info->i = asap->src2val[REGBASE + 2];	break;
		case CPUINFO_INT_REGISTER + ASAP_R3:			info->i = asap->src2val[REGBASE + 3];	break;
		case CPUINFO_INT_REGISTER + ASAP_R4:			info->i = asap->src2val[REGBASE + 4];	break;
		case CPUINFO_INT_REGISTER + ASAP_R5:			info->i = asap->src2val[REGBASE + 5];	break;
		case CPUINFO_INT_REGISTER + ASAP_R6:			info->i = asap->src2val[REGBASE + 6];	break;
		case CPUINFO_INT_REGISTER + ASAP_R7:			info->i = asap->src2val[REGBASE + 7];	break;
		case CPUINFO_INT_REGISTER + ASAP_R8:			info->i = asap->src2val[REGBASE + 8];	break;
		case CPUINFO_INT_REGISTER + ASAP_R9:			info->i = asap->src2val[REGBASE + 9];	break;
		case CPUINFO_INT_REGISTER + ASAP_R10:			info->i = asap->src2val[REGBASE + 10];	break;
		case CPUINFO_INT_REGISTER + ASAP_R11:			info->i = asap->src2val[REGBASE + 11];	break;
		case CPUINFO_INT_REGISTER + ASAP_R12:			info->i = asap->src2val[REGBASE + 12];	break;
		case CPUINFO_INT_REGISTER + ASAP_R13:			info->i = asap->src2val[REGBASE + 13];	break;
		case CPUINFO_INT_REGISTER + ASAP_R14:			info->i = asap->src2val[REGBASE + 14];	break;
		case CPUINFO_INT_REGISTER + ASAP_R15:			info->i = asap->src2val[REGBASE + 15];	break;
		case CPUINFO_INT_REGISTER + ASAP_R16:			info->i = asap->src2val[REGBASE + 16];	break;
		case CPUINFO_INT_REGISTER + ASAP_R17:			info->i = asap->src2val[REGBASE + 17];	break;
		case CPUINFO_INT_REGISTER + ASAP_R18:			info->i = asap->src2val[REGBASE + 18];	break;
		case CPUINFO_INT_REGISTER + ASAP_R19:			info->i = asap->src2val[REGBASE + 19];	break;
		case CPUINFO_INT_REGISTER + ASAP_R20:			info->i = asap->src2val[REGBASE + 20];	break;
		case CPUINFO_INT_REGISTER + ASAP_R21:			info->i = asap->src2val[REGBASE + 21];	break;
		case CPUINFO_INT_REGISTER + ASAP_R22:			info->i = asap->src2val[REGBASE + 22];	break;
		case CPUINFO_INT_REGISTER + ASAP_R23:			info->i = asap->src2val[REGBASE + 23];	break;
		case CPUINFO_INT_REGISTER + ASAP_R24:			info->i = asap->src2val[REGBASE + 24];	break;
		case CPUINFO_INT_REGISTER + ASAP_R25:			info->i = asap->src2val[REGBASE + 25];	break;
		case CPUINFO_INT_REGISTER + ASAP_R26:			info->i = asap->src2val[REGBASE + 26];	break;
		case CPUINFO_INT_REGISTER + ASAP_R27:			info->i = asap->src2val[REGBASE + 27];	break;
		case CPUINFO_INT_REGISTER + ASAP_R28:			info->i = asap->src2val[REGBASE + 28];	break;
		case CPUINFO_INT_REGISTER + ASAP_R29:			info->i = asap->src2val[REGBASE + 29];	break;
		case CPUINFO_INT_REGISTER + ASAP_R30:			info->i = asap->src2val[REGBASE + 30];	break;
		case CPUINFO_INT_REGISTER + ASAP_R31:			info->i = asap->src2val[REGBASE + 31];	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(asap);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(asap);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(asap);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(asap);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(asap);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(asap);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &asap->icount;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ASAP");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Atari ASAP");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + ASAP_PC:			sprintf(info->s, "PC: %08X", asap->pc);	break;
		case CPUINFO_STR_REGISTER + ASAP_PS:			sprintf(info->s, "PS: %08X", GET_FLAGS(asap)); break;

		case CPUINFO_STR_REGISTER + ASAP_R0:			sprintf(info->s, "R0: %08X", asap->src2val[REGBASE + 0]); break;
		case CPUINFO_STR_REGISTER + ASAP_R1:			sprintf(info->s, "R1: %08X", asap->src2val[REGBASE + 1]); break;
		case CPUINFO_STR_REGISTER + ASAP_R2:			sprintf(info->s, "R2: %08X", asap->src2val[REGBASE + 2]); break;
		case CPUINFO_STR_REGISTER + ASAP_R3:			sprintf(info->s, "R3: %08X", asap->src2val[REGBASE + 3]); break;
		case CPUINFO_STR_REGISTER + ASAP_R4:			sprintf(info->s, "R4: %08X", asap->src2val[REGBASE + 4]); break;
		case CPUINFO_STR_REGISTER + ASAP_R5:			sprintf(info->s, "R5: %08X", asap->src2val[REGBASE + 5]); break;
		case CPUINFO_STR_REGISTER + ASAP_R6:			sprintf(info->s, "R6: %08X", asap->src2val[REGBASE + 6]); break;
		case CPUINFO_STR_REGISTER + ASAP_R7:			sprintf(info->s, "R7: %08X", asap->src2val[REGBASE + 7]); break;
		case CPUINFO_STR_REGISTER + ASAP_R8:			sprintf(info->s, "R8: %08X", asap->src2val[REGBASE + 8]); break;
		case CPUINFO_STR_REGISTER + ASAP_R9:			sprintf(info->s, "R9: %08X", asap->src2val[REGBASE + 9]); break;
		case CPUINFO_STR_REGISTER + ASAP_R10:			sprintf(info->s, "R10:%08X", asap->src2val[REGBASE + 10]); break;
		case CPUINFO_STR_REGISTER + ASAP_R11:			sprintf(info->s, "R11:%08X", asap->src2val[REGBASE + 11]); break;
		case CPUINFO_STR_REGISTER + ASAP_R12:			sprintf(info->s, "R12:%08X", asap->src2val[REGBASE + 12]); break;
		case CPUINFO_STR_REGISTER + ASAP_R13:			sprintf(info->s, "R13:%08X", asap->src2val[REGBASE + 13]); break;
		case CPUINFO_STR_REGISTER + ASAP_R14:			sprintf(info->s, "R14:%08X", asap->src2val[REGBASE + 14]); break;
		case CPUINFO_STR_REGISTER + ASAP_R15:			sprintf(info->s, "R15:%08X", asap->src2val[REGBASE + 15]); break;
		case CPUINFO_STR_REGISTER + ASAP_R16:			sprintf(info->s, "R16:%08X", asap->src2val[REGBASE + 16]); break;
		case CPUINFO_STR_REGISTER + ASAP_R17:			sprintf(info->s, "R17:%08X", asap->src2val[REGBASE + 17]); break;
		case CPUINFO_STR_REGISTER + ASAP_R18:			sprintf(info->s, "R18:%08X", asap->src2val[REGBASE + 18]); break;
		case CPUINFO_STR_REGISTER + ASAP_R19:			sprintf(info->s, "R19:%08X", asap->src2val[REGBASE + 19]); break;
		case CPUINFO_STR_REGISTER + ASAP_R20:			sprintf(info->s, "R20:%08X", asap->src2val[REGBASE + 20]); break;
		case CPUINFO_STR_REGISTER + ASAP_R21:			sprintf(info->s, "R21:%08X", asap->src2val[REGBASE + 21]); break;
		case CPUINFO_STR_REGISTER + ASAP_R22:			sprintf(info->s, "R22:%08X", asap->src2val[REGBASE + 22]); break;
		case CPUINFO_STR_REGISTER + ASAP_R23:			sprintf(info->s, "R23:%08X", asap->src2val[REGBASE + 23]); break;
		case CPUINFO_STR_REGISTER + ASAP_R24:			sprintf(info->s, "R24:%08X", asap->src2val[REGBASE + 24]); break;
		case CPUINFO_STR_REGISTER + ASAP_R25:			sprintf(info->s, "R25:%08X", asap->src2val[REGBASE + 25]); break;
		case CPUINFO_STR_REGISTER + ASAP_R26:			sprintf(info->s, "R26:%08X", asap->src2val[REGBASE + 26]); break;
		case CPUINFO_STR_REGISTER + ASAP_R27:			sprintf(info->s, "R27:%08X", asap->src2val[REGBASE + 27]); break;
		case CPUINFO_STR_REGISTER + ASAP_R28:			sprintf(info->s, "R28:%08X", asap->src2val[REGBASE + 28]); break;
		case CPUINFO_STR_REGISTER + ASAP_R29:			sprintf(info->s, "R29:%08X", asap->src2val[REGBASE + 29]); break;
		case CPUINFO_STR_REGISTER + ASAP_R30:			sprintf(info->s, "R30:%08X", asap->src2val[REGBASE + 30]); break;
		case CPUINFO_STR_REGISTER + ASAP_R31:			sprintf(info->s, "R31:%08X", asap->src2val[REGBASE + 31]); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(ASAP, asap);
