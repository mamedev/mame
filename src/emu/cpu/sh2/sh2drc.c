/***************************************************************************

    sh2drc.c
    Universal machine language-based SH-2 emulator.
    By R. Belmont

    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

    ST-V status:
    colmns97 & stress crash due to SCSP stream->machine() getting corrupted.

    cottonbm w/US bios: run to 60323B4 on master, then MOV insn @ 602f5aa crashes?
    actually crash on slave @ 6032b38 after above.  reading wrong addr for jump vector.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "sh2.h"
#include "sh2comn.h"

CPU_DISASSEMBLE( sh2 );
extern unsigned DasmSH2(char *buffer, unsigned pc, UINT16 opcode);

#ifdef USE_SH2DRC

using namespace uml;

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define FORCE_C_BACKEND					(0)	// use the C backend even when a native one is available
#define LOG_UML						(0)	// log UML assembly
#define LOG_NATIVE					(0)	// log native assembly

#define SET_EA						(0)	// makes slower but "shows work" in the EA fake register like the interpreter

#define DISABLE_FAST_REGISTERS				(0)	// set to 1 to turn off usage of register caching
#define SINGLE_INSTRUCTION_MODE				(0)

#define ADDSUBV_DIRECT				(0)

#define VERBOSE 0
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#if SET_EA
#define SETEA(x) UML_MOV(block, mem(&sh2->ea), ireg(x))
#else
#define SETEA(x)
#endif

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC					M0
#define MAPVAR_CYCLES					M1

/* size of the execution code cache */
#define CACHE_SIZE					(32 * 1024 * 1024)

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES			64
#define COMPILE_FORWARDS_BYTES			256
#define COMPILE_MAX_INSTRUCTIONS		((COMPILE_BACKWARDS_BYTES/2) + (COMPILE_FORWARDS_BYTES/2))
#define COMPILE_MAX_SEQUENCE			64

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES			0
#define EXECUTE_MISSING_CODE			1
#define EXECUTE_UNMAPPED_CODE			2
#define EXECUTE_RESET_CACHE			3

#define PROBE_ADDRESS					~0

extern int sh2_describe(void *param, opcode_desc *desc, const opcode_desc *prev);

/***************************************************************************
    MACROS
***************************************************************************/

#define R32(reg)		sh2->regmap[reg]

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* internal compiler state */
typedef struct _compiler_state compiler_state;
struct _compiler_state
{
	UINT32			cycles;						/* accumulated cycles */
	UINT8			checkints;					/* need to check interrupts before next instruction */
	code_label	labelnum;					/* index for local labels */
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void static_generate_entry_point(sh2_state *sh2);
static void static_generate_nocode_handler(sh2_state *sh2);
static void static_generate_out_of_cycles(sh2_state *sh2);
static void static_generate_memory_accessor(sh2_state *sh2, int size, int iswrite, const char *name, code_handle **handleptr);

static void generate_update_cycles(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, parameter param, int allow_exception);
static void generate_checksum_block(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
static void generate_sequence_instruction(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc);
static void generate_delay_slot(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc);

static int generate_opcode(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc);
static int generate_group_0(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
static int generate_group_2(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
static int generate_group_3(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, UINT32 ovrpc);
static int generate_group_4(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
static int generate_group_6(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
static int generate_group_8(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
static int generate_group_12(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);

static void code_compile_block(sh2_state *sh2, UINT8 mode, offs_t pc);

static void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);
static void log_register_list(drcuml_state *drcuml, const char *string, const UINT32 *reglist, const UINT32 *regnostarlist);
static void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op);
static const char *log_desc_flags_to_string(UINT32 flags);

static void cfunc_printf_probe(void *param);
static void cfunc_unimplemented(void *param);
static void cfunc_checkirqs(void *param);
static void cfunc_fastirq(void *param);
static void cfunc_MAC_W(void *param);
static void cfunc_MAC_L(void *param);
static void cfunc_DIV1(void *param);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE sh2_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SH1 ||
		   device->type() == SH2);
	return *(sh2_state **)downcast<legacy_cpu_device *>(device)->token();
}

INLINE UINT16 RW(sh2_state *sh2, offs_t A)
{
	if (A >= 0xe0000000)
		return sh2_internal_r(sh2->internal, (A & 0x1fc)>>2, 0xffff << (((~A) & 2)*8)) >> (((~A) & 2)*8);

	if (A >= 0xc0000000)
		return sh2->program->read_word(A);

	return sh2->program->read_word(A & AM);
}

INLINE UINT32 RL(sh2_state *sh2, offs_t A)
{
	if (A >= 0xe0000000)
		return sh2_internal_r(sh2->internal, (A & 0x1fc)>>2, 0xffffffff);

	if (A >= 0xc0000000)
		return sh2->program->read_dword(A);

	return sh2->program->read_dword(A & AM);
}

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

INLINE UINT32 epc(const opcode_desc *desc)
{
	return (desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 1) : desc->pc;
}

/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

INLINE void alloc_handle(drcuml_state *drcuml, code_handle **handleptr, const char *name)
{
	if (*handleptr == NULL)
		*handleptr = drcuml->handle_alloc(name);
}

/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

INLINE void load_fast_iregs(sh2_state *sh2, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(sh2->regmap); regnum++)
	{
		if (sh2->regmap[regnum].is_int_register())
		{
			UML_MOV(block, parameter::make_ireg(sh2->regmap[regnum].ireg()), mem(&sh2->r[regnum]));
		}
	}
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

INLINE void save_fast_iregs(sh2_state *sh2, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(sh2->regmap); regnum++)
	{
		if (sh2->regmap[regnum].is_int_register())
		{
			UML_MOV(block, mem(&sh2->r[regnum]), parameter::make_ireg(sh2->regmap[regnum].ireg()));
		}
	}
}

/*-------------------------------------------------
    cfunc_printf_probe - print the current CPU
    state and return
-------------------------------------------------*/

static void cfunc_printf_probe(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	UINT32 pc = sh2->pc;

	printf(" PC=%08X          r0=%08X  r1=%08X  r2=%08X\n",
		pc,
		(UINT32)sh2->r[0],
		(UINT32)sh2->r[1],
		(UINT32)sh2->r[2]);
	printf(" r3=%08X  r4=%08X  r5=%08X  r6=%08X\n",
		(UINT32)sh2->r[3],
		(UINT32)sh2->r[4],
		(UINT32)sh2->r[5],
		(UINT32)sh2->r[6]);
	printf(" r7=%08X  r8=%08X  r9=%08X  r10=%08X\n",
		(UINT32)sh2->r[7],
		(UINT32)sh2->r[8],
		(UINT32)sh2->r[9],
		(UINT32)sh2->r[10]);
	printf(" r11=%08X  r12=%08X  r13=%08X  r14=%08X\n",
		(UINT32)sh2->r[11],
		(UINT32)sh2->r[12],
		(UINT32)sh2->r[13],
		(UINT32)sh2->r[14]);
	printf(" r15=%08X  macl=%08X  mach=%08X  gbr=%08X\n",
		(UINT32)sh2->r[15],
		(UINT32)sh2->macl,
		(UINT32)sh2->mach,
		(UINT32)sh2->gbr);
	printf(" evec %x irqsr %x pc=%08x\n",
		(UINT32)sh2->evec,
		(UINT32)sh2->irqsr, (UINT32)sh2->pc);
}

/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

static void cfunc_unimplemented(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	UINT16 opcode = sh2->arg0;
	fatalerror("PC=%08X: Unimplemented op %04X", sh2->pc, opcode);
}

/*-------------------------------------------------
    cfunc_checkirqs - checks for pending IRQs
-------------------------------------------------*/
static void cfunc_checkirqs(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	// if NMI is pending, evec etc are already set up
	if (sh2->pending_nmi)
	{
		sh2->pending_nmi = 0;
	}
	else
	{
		sh2->evec = 0xffffffff;
		CHECK_PENDING_IRQ("cfunc_checkirqs");
	}
}

/*-------------------------------------------------
    cfunc_fastirq - checks for pending IRQs
-------------------------------------------------*/
static void cfunc_fastirq(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	sh2_exception(sh2, "fastirq",sh2->irqline);
}

/*-------------------------------------------------
    cfunc_MAC_W - implementation of MAC_W Rm,Rn
-------------------------------------------------*/
static void cfunc_MAC_W(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;
	UINT16 opcode;
	int n, m;

	// recover the opcode
	opcode = sh2->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	tempn = (INT32) RW( sh2, sh2->r[n] );
	sh2->r[n] += 2;
	tempm = (INT32) RW( sh2, sh2->r[m] );
	sh2->r[m] += 2;
	templ = sh2->macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) sh2->macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) tempm >= 0)
	{
		src = 0;
		tempn = 0;
	}
	else
	{
		src = 1;
		tempn = 0xffffffff;
	}
	src += dest;
	sh2->macl += tempm;
	if ((INT32) sh2->macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (sh2->sr & S)
	{
		if (ans == 1)
			{
				if ((sh2->cpu_type == CPU_TYPE_SH1) && ((src == 0) || (src == 2)))
				{
					sh2->mach |= 0x00000001;
				}

				if (src == 0)
					sh2->macl = 0x7fffffff;
				if (src == 2)
					sh2->macl = 0x80000000;
			}
	}
	else
	{
		sh2->mach += tempn;
		if (templ > sh2->macl)
			sh2->mach += 1;

		// SH-1 has limited precision
		if (sh2->cpu_type == CPU_TYPE_SH1)
		{
			if ((sh2->mach & 0x200) == 0)
			{
				sh2->mach &= 0x3ff;
			}
			else
			{
				sh2->mach |= 0xfffffc00;
			}
		}


	}
}

/*-------------------------------------------------
    cfunc_MAC_L - implementation of MAC_L Rm,Rn
-------------------------------------------------*/
static void cfunc_MAC_L(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;
	UINT16 opcode;
	int n, m;

	// recover the opcode
	opcode = sh2->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	tempn = (INT32) RL( sh2, sh2->r[n] );
	sh2->r[n] += 4;
	tempm = (INT32) RL( sh2, sh2->r[m] );
	sh2->r[m] += 4;
	if ((INT32) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	if (sh2->sr & S)
	{
		Res0 = sh2->macl + Res0;
		if (sh2->macl > Res0)
			Res2++;
		Res2 += (sh2->mach & 0x0000ffff);
		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		sh2->mach = Res2;
		sh2->macl = Res0;
	}
	else
	{
		Res0 = sh2->macl + Res0;
		if (sh2->macl > Res0)
			Res2++;
		Res2 += sh2->mach;
		sh2->mach = Res2;
		sh2->macl = Res0;
	}
}

/*-------------------------------------------------
    cfunc_DIV1 - implementation of DIV1 Rm,Rn
-------------------------------------------------*/
static void cfunc_DIV1(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	UINT32 tmp0;
	UINT32 old_q;
	UINT16 opcode;
	int n, m;

	// recover the opcode
	opcode = sh2->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	old_q = sh2->sr & Q;
	if (0x80000000 & sh2->r[n])
		sh2->sr |= Q;
	else
		sh2->sr &= ~Q;

	sh2->r[n] = (sh2->r[n] << 1) | (sh2->sr & T);

	if (!old_q)
	{
		if (!(sh2->sr & M))
		{
			tmp0 = sh2->r[n];
			sh2->r[n] -= sh2->r[m];
			if(!(sh2->sr & Q))
				if(sh2->r[n] > tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
			else
				if(sh2->r[n] > tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
		}
		else
		{
			tmp0 = sh2->r[n];
			sh2->r[n] += sh2->r[m];
			if(!(sh2->sr & Q))
			{
				if(sh2->r[n] < tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
			}
			else
			{
				if(sh2->r[n] < tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(sh2->sr & M))
		{
			tmp0 = sh2->r[n];
			sh2->r[n] += sh2->r[m];
			if(!(sh2->sr & Q))
				if(sh2->r[n] < tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
			else
				if(sh2->r[n] < tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
		}
		else
		{
			tmp0 = sh2->r[n];
			sh2->r[n] -= sh2->r[m];
			if(!(sh2->sr & Q))
				if(sh2->r[n] > tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
			else
				if(sh2->r[n] > tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
		}
	}

	tmp0 = (sh2->sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

#if (!ADDSUBV_DIRECT)
/*-------------------------------------------------
    cfunc_ADDV - implementation of ADDV Rm,Rn
-------------------------------------------------*/
static void cfunc_ADDV(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	INT32 dest, src, ans;
	UINT16 opcode;
	int n, m;

	// recover the opcode
	opcode = sh2->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	if ((INT32) sh2->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh2->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh2->r[n] += sh2->r[m];
	if ((INT32) sh2->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			sh2->sr |= T;
		else
			sh2->sr &= ~T;
	}
	else
		sh2->sr &= ~T;
}

/*-------------------------------------------------
    cfunc_SUBV - implementation of SUBV Rm,Rn
-------------------------------------------------*/
static void cfunc_SUBV(void *param)
{
	sh2_state *sh2 = (sh2_state *)param;
	INT32 dest, src, ans;
	UINT16 opcode;
	int n, m;

	// recover the opcode
	opcode = sh2->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	if ((INT32) sh2->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh2->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh2->r[n] -= sh2->r[m];
	if ((INT32) sh2->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			sh2->sr |= T;
		else
			sh2->sr &= ~T;
	}
	else
		sh2->sr &= ~T;
}
#endif

/*-------------------------------------------------
    sh2_init - initialize the processor
-------------------------------------------------*/

static CPU_INIT( sh2 )
{
	sh2_state *sh2 = get_safe_token(device);
	drc_cache *cache;
	drcbe_info beinfo;
	UINT32 flags = 0;
	int regnum;

	/* allocate enough space for the cache and the core */
	cache = auto_alloc(device->machine(), drc_cache(CACHE_SIZE + sizeof(sh2_state)));

	/* allocate the core memory */
	*(sh2_state **)device->token() = sh2 = (sh2_state *)cache->alloc_near(sizeof(sh2_state));
	memset(sh2, 0, sizeof(sh2_state));

	/* initialize the common core parts */
	sh2_common_init(sh2, device, irqcallback);

	/* allocate the implementation-specific state from the full cache */
	sh2->cache = cache;

	/* reset per-driver pcflushes */
	sh2->pcfsel = 0;

	/* initialize the UML generator */
	if (FORCE_C_BACKEND)
		flags |= DRCUML_OPTION_USE_C;
	if (LOG_UML)
		flags |= DRCUML_OPTION_LOG_UML;
	if (LOG_NATIVE)
		flags |= DRCUML_OPTION_LOG_NATIVE;
	sh2->drcuml = auto_alloc(device->machine(), drcuml_state(*device, *cache, flags, 1, 32, 1));

	/* add symbols for our stuff */
	sh2->drcuml->symbol_add(&sh2->pc, sizeof(sh2->pc), "pc");
	sh2->drcuml->symbol_add(&sh2->icount, sizeof(sh2->icount), "icount");
	for (regnum = 0; regnum < 16; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		sh2->drcuml->symbol_add(&sh2->r[regnum], sizeof(sh2->r[regnum]), buf);
	}
	sh2->drcuml->symbol_add(&sh2->pr, sizeof(sh2->pr), "pr");
	sh2->drcuml->symbol_add(&sh2->sr, sizeof(sh2->sr), "sr");
	sh2->drcuml->symbol_add(&sh2->gbr, sizeof(sh2->gbr), "gbr");
	sh2->drcuml->symbol_add(&sh2->vbr, sizeof(sh2->vbr), "vbr");
	sh2->drcuml->symbol_add(&sh2->macl, sizeof(sh2->macl), "macl");
	sh2->drcuml->symbol_add(&sh2->mach, sizeof(sh2->macl), "mach");

	/* initialize the front-end helper */
	sh2->drcfe = auto_alloc(device->machine(), sh2_frontend(*sh2, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE));

	/* compute the register parameters */
	for (regnum = 0; regnum < 16; regnum++)
	{
		sh2->regmap[regnum] = mem(&sh2->r[regnum]);
	}

	/* if we have registers to spare, assign r0, r1, r2 to leftovers */
	/* WARNING: do not use synthetic registers that are mapped here! */
	if (!DISABLE_FAST_REGISTERS)
	{
		sh2->drcuml->get_backend_info(beinfo);
		if (beinfo.direct_iregs > 4)
		{
			sh2->regmap[0] = I4;
		}
		if (beinfo.direct_iregs > 5)
		{
			sh2->regmap[1] = I5;
		}
		if (beinfo.direct_iregs > 6)
		{
			sh2->regmap[2] = I6;
		}
	}

	/* mark the cache dirty so it is updated on next execute */
	sh2->cache_dirty = TRUE;
}

/*-------------------------------------------------
    sh2_exit - cleanup from execution
-------------------------------------------------*/

static CPU_EXIT( sh2 )
{
	sh2_state *sh2 = get_safe_token(device);

	/* clean up the DRC */
	auto_free(device->machine(), sh2->drcfe);
	auto_free(device->machine(), sh2->drcuml);
	auto_free(device->machine(), sh2->cache);
}


/*-------------------------------------------------
    sh2_reset - reset the processor
-------------------------------------------------*/

static CPU_RESET( sh2 )
{
	sh2_state *sh2 = get_safe_token(device);
	emu_timer *tsave, *tsaved0, *tsaved1;
	UINT32 *m;

	void (*f)(UINT32 data);
	device_irq_acknowledge_callback save_irqcallback;

	m = sh2->m;
	tsave = sh2->timer;
	tsaved0 = sh2->dma_current_active_timer[0];
	tsaved1 = sh2->dma_current_active_timer[1];

	f = sh2->ftcsr_read_callback;
	save_irqcallback = sh2->irq_callback;

	sh2->ppc = sh2->pc = sh2->pr = sh2->sr = sh2->gbr = sh2->vbr = sh2->mach = sh2->macl = 0;
	sh2->evec = sh2->irqsr = 0;
	memset(&sh2->r[0], 0, sizeof(sh2->r[0])*16);
	sh2->ea = sh2->delay = sh2->cpu_off = sh2->dvsr = sh2->dvdnth = sh2->dvdntl = sh2->dvcr = 0;
	sh2->pending_irq = sh2->test_irq = 0;
	memset(&sh2->irq_queue[0], 0, sizeof(sh2->irq_queue[0])*16);
	memset(&sh2->irq_line_state[0], 0, sizeof(sh2->irq_line_state[0])*17);
	sh2->frc = sh2->ocra = sh2->ocrb = sh2->icr = 0;
	sh2->frc_base = 0;
	sh2->frt_input = sh2->internal_irq_level = sh2->internal_irq_vector = 0;
	sh2->dma_timer_active[0] = sh2->dma_timer_active[1] = 0;

	sh2->ftcsr_read_callback = f;
	sh2->irq_callback = save_irqcallback;
	sh2->device = device;

	sh2->timer = tsave;
	sh2->dma_current_active_timer[0] = tsaved0;
	sh2->dma_current_active_timer[1] = tsaved1;
	sh2->m = m;
	memset(sh2->m, 0, 0x200);

	sh2->pc = sh2->program->read_dword(0);
	sh2->r[15] = sh2->program->read_dword(4);
	sh2->sr = I;

	sh2->internal_irq_level = -1;

	sh2->cache_dirty = TRUE;

	sh2->cpu_type = CPU_TYPE_SH2;
}

/*-------------------------------------------------
    sh1_reset - reset the processor
-------------------------------------------------*/

static CPU_RESET( sh1 )
{
	sh2_state *sh2 = get_safe_token(device);
	CPU_RESET_CALL(sh2);
	sh2->cpu_type = CPU_TYPE_SH1;
}

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

static void code_flush_cache(sh2_state *sh2)
{
	drcuml_state *drcuml = sh2->drcuml;

	/* empty the transient cache contents */
	drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_nocode_handler(sh2);
		static_generate_out_of_cycles(sh2);
		static_generate_entry_point(sh2);

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(sh2, 1, FALSE, "read8", &sh2->read8);
		static_generate_memory_accessor(sh2, 1, TRUE,  "write8", &sh2->write8);
		static_generate_memory_accessor(sh2, 2, FALSE, "read16", &sh2->read16);
		static_generate_memory_accessor(sh2, 2, TRUE,  "write16", &sh2->write16);
		static_generate_memory_accessor(sh2, 4, FALSE, "read32", &sh2->read32);
		static_generate_memory_accessor(sh2, 4, TRUE,  "write32", &sh2->write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate SH2 static code");
	}

	sh2->cache_dirty = FALSE;
}

/* Execute cycles - returns number of cycles actually run */
static CPU_EXECUTE( sh2 )
{
	sh2_state *sh2 = get_safe_token(device);
	drcuml_state *drcuml = sh2->drcuml;
	int execute_result;

	// run any active DMAs now
#ifndef USE_TIMER_FOR_DMA
	for ( int i = 0; i < sh2->icount ; i++)
	{
		for( int dma=0;dma<1;dma++)
		{
			if (sh2->dma_timer_active[dma])
				sh2_do_dma(sh2, dma);
		}
	}
#endif

	/* reset the cache if dirty */
	if (sh2->cache_dirty)
		code_flush_cache(sh2);

	/* execute */
	do
	{
		/* run as much as we can */
		execute_result = drcuml->execute(*sh2->entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(sh2, 0, sh2->pc);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", sh2->pc);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			code_flush_cache(sh2);
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}

/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

static void code_compile_block(sh2_state *sh2, UINT8 mode, offs_t pc)
{
	drcuml_state *drcuml = sh2->drcuml;
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = FALSE;
	drcuml_block *block;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = sh2->drcfe->describe_code(pc);
	if (LOG_UML || LOG_NATIVE)
		log_opcode_desc(drcuml, desclist, 0);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != NULL; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				UINT32 nextpc;

				/* add a code log entry */
				if (LOG_UML)
					block->append_comment("-------------------------");					// comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != NULL);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !drcuml->hash_exists(mode, seqhead->pc))
					UML_HASH(block, mode, seqhead->pc);										// hash    mode,pc

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = TRUE;
					UML_HASH(block, mode, seqhead->pc);										// hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);								// label   seqhead->pc | 0x80000000
					UML_HASHJMP(block, 0, seqhead->pc, *sh2->nocode);
																							// hashjmp <mode>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (sh2->program->get_write_ptr(seqhead->physpc) != NULL)
					generate_checksum_block(sh2, block, &compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);								// label   seqhead->pc | 0x80000000
				}

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
				{
					generate_sequence_instruction(sh2, block, &compiler, curdesc, 0xffffffff);
				}

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
				{
					nextpc = pc;
				}
				/* otherwise we just go to the next instruction */
				else
				{
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 2;
				}

				/* count off cycles and go there */
				generate_update_cycles(sh2, block, &compiler, nextpc, TRUE);				// <subtract cycles>

				/* SH2 has no modes */
				if (seqlast->next() == NULL || seqlast->next()->pc != nextpc)
				{
					UML_HASHJMP(block, 0, nextpc, *sh2->nocode);
				}
																							// hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block->end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache(sh2);
		}
	}
}

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

static void static_generate_entry_point(sh2_state *sh2)
{
	drcuml_state *drcuml = sh2->drcuml;
	code_label skip = 1;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(200);

	/* forward references */
	alloc_handle(drcuml, &sh2->nocode, "nocode");
	alloc_handle(drcuml, &sh2->write32, "write32");		// necessary?
	alloc_handle(drcuml, &sh2->entry, "entry");
	UML_HANDLE(block, *sh2->entry);							// handle  entry

	/* load fast integer registers */
	load_fast_iregs(sh2, block);

	/* check for interrupts */
	UML_MOV(block, mem(&sh2->irqline), 0xffffffff);		// mov irqline, #-1
	UML_CMP(block, mem(&sh2->pending_nmi), 0);			// cmp pending_nmi, #0
	UML_JMPc(block, COND_Z, skip+2);					// jz skip+2

	UML_MOV(block, mem(&sh2->pending_nmi), 0);			// zap pending_nmi
	UML_JMP(block, skip+1);						// and then go take it (evec is already set)

	UML_LABEL(block, skip+2);					// skip+2:
	UML_MOV(block, mem(&sh2->evec), 0xffffffff);		// mov evec, -1
	UML_MOV(block, I0, 0xffffffff);			// mov r0, -1 (r0 = irq)
	UML_AND(block, I1,  I0, 0xffff);	    		// and r1, 0xffff

	UML_LZCNT(block, I1, mem(&sh2->pending_irq));		// lzcnt r1, r1
	UML_CMP(block, I1, 32);				// cmp r1, #32
	UML_JMPc(block, COND_Z, skip+4);					// jz skip+4

	UML_SUB(block, mem(&sh2->irqline), 31, I1);		// sub irqline, #31, r1

	UML_LABEL(block, skip+4);					// skip+4:
	UML_CMP(block, mem(&sh2->internal_irq_level), 0xffffffff);	// cmp internal_irq_level, #-1
	UML_JMPc(block, COND_Z, skip+3);					// jz skip+3
	UML_CMP(block, mem(&sh2->internal_irq_level), mem(&sh2->irqline));		// cmp internal_irq_level, irqline
	UML_JMPc(block, COND_LE, skip+3);					// jle skip+3

	UML_MOV(block, mem(&sh2->irqline), mem(&sh2->internal_irq_level));		// mov r0, internal_irq_level

	UML_LABEL(block, skip+3);					// skip+3:
	UML_CMP(block, mem(&sh2->irqline), 0xffffffff);		// cmp irqline, #-1
	UML_JMPc(block, COND_Z, skip+1);					// jz skip+1
	UML_CALLC(block, cfunc_fastirq, sh2);				// callc fastirq

	UML_LABEL(block, skip+1);					// skip+1:

	UML_CMP(block, mem(&sh2->evec), 0xffffffff);		// cmp evec, 0xffffffff
	UML_JMPc(block, COND_Z, skip);  				// jz skip

	UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
	UML_MOV(block, I0, R32(15));				// mov r0, R15
	UML_MOV(block, I1, mem(&sh2->irqsr));			// mov r1, irqsr
	UML_CALLH(block, *sh2->write32);					// call write32

	UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
	UML_MOV(block, I0, R32(15));				// mov r0, R15
	UML_MOV(block, I1, mem(&sh2->pc));				// mov r1, pc
	UML_CALLH(block, *sh2->write32);					// call write32

	UML_MOV(block, mem(&sh2->pc), mem(&sh2->evec));	    		// mov pc, evec

	UML_LABEL(block, skip);	    					// skip:

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&sh2->pc), *sh2->nocode);		// hashjmp <mode>,<pc>,nocode

	block->end();
}

/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

static void static_generate_nocode_handler(sh2_state *sh2)
{
	drcuml_state *drcuml = sh2->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &sh2->nocode, "nocode");
	UML_HANDLE(block, *sh2->nocode);									// handle  nocode
	UML_GETEXP(block, I0);									// getexp  i0
	UML_MOV(block, mem(&sh2->pc), I0);								// mov     [pc],i0
	save_fast_iregs(sh2, block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);							// exit    EXECUTE_MISSING_CODE

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

static void static_generate_out_of_cycles(sh2_state *sh2)
{
	drcuml_state *drcuml = sh2->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &sh2->out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *sh2->out_of_cycles);								// handle  out_of_cycles
	UML_GETEXP(block, I0);									// getexp  i0
	UML_MOV(block, mem(&sh2->pc), I0);								// mov     <pc>,i0
	save_fast_iregs(sh2,block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);							// exit    EXECUTE_OUT_OF_CYCLES

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

static void static_generate_memory_accessor(sh2_state *sh2, int size, int iswrite, const char *name, code_handle **handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0 */
	drcuml_state *drcuml = sh2->drcuml;
	drcuml_block *block;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, handleptr, name);
	UML_HANDLE(block, **handleptr);							// handle  *handleptr

	// with internal handlers this becomes easier.
	// if addr < 0x40000000 AND it with AM and do the read/write, else just do the read/write
	UML_TEST(block, I0, 0x80000000);		// test r0, #0x80000000
	UML_JMPc(block, COND_NZ, label);				// if high bit is set, don't mask

	UML_CMP(block, I0, 0x40000000);		// cmp #0x40000000, r0
	UML_JMPc(block, COND_AE, label);			// bae label

	UML_AND(block, I0, I0, AM);		// and r0, r0, #AM (0xc7ffffff)

	UML_LABEL(block, label++);				// label:

	if (iswrite)
	{
		switch (size)
		{
			case 1:
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM);	// write r0, r1, program_byte
				break;

			case 2:
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);	// write r0, r1, program_word
				break;

			case 4:
				UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);	// write r0, r1, program_dword
				break;
		}
	}
	else
	{
		switch (size)
		{
			case 1:
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);	// read r0, program_byte
				break;

			case 2:
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);	// read r0, program_word
				break;

			case 4:
				UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM);	// read r0, program_dword
				break;
		}
	}

	UML_RET(block);							// ret

	block->end();
}

/*-------------------------------------------------
    log_desc_flags_to_string - generate a string
    representing the instruction description
    flags
-------------------------------------------------*/

static const char *log_desc_flags_to_string(UINT32 flags)
{
	static char tempbuf[30];
	char *dest = tempbuf;

	/* branches */
	if (flags & OPFLAG_IS_UNCONDITIONAL_BRANCH)
		*dest++ = 'U';
	else if (flags & OPFLAG_IS_CONDITIONAL_BRANCH)
		*dest++ = 'C';
	else
		*dest++ = '.';

	/* intrablock branches */
	*dest++ = (flags & OPFLAG_INTRABLOCK_BRANCH) ? 'i' : '.';

	/* branch targets */
	*dest++ = (flags & OPFLAG_IS_BRANCH_TARGET) ? 'B' : '.';

	/* delay slots */
	*dest++ = (flags & OPFLAG_IN_DELAY_SLOT) ? 'D' : '.';

	/* exceptions */
	if (flags & OPFLAG_WILL_CAUSE_EXCEPTION)
		*dest++ = 'E';
	else if (flags & OPFLAG_CAN_CAUSE_EXCEPTION)
		*dest++ = 'e';
	else
		*dest++ = '.';

	/* read/write */
	if (flags & OPFLAG_READS_MEMORY)
		*dest++ = 'R';
	else if (flags & OPFLAG_WRITES_MEMORY)
		*dest++ = 'W';
	else
		*dest++ = '.';

	/* TLB validation */
	*dest++ = (flags & OPFLAG_VALIDATE_TLB) ? 'V' : '.';

	/* TLB modification */
	*dest++ = (flags & OPFLAG_MODIFIES_TRANSLATION) ? 'T' : '.';

	/* redispatch */
	*dest++ = (flags & OPFLAG_REDISPATCH) ? 'R' : '.';
	return tempbuf;
}


/*-------------------------------------------------
    log_register_list - log a list of GPR registers
-------------------------------------------------*/

static void log_register_list(drcuml_state *drcuml, const char *string, const UINT32 *reglist, const UINT32 *regnostarlist)
{
	int count = 0;
	int regnum;

	/* skip if nothing */
	if (reglist[0] == 0 && reglist[1] == 0 && reglist[2] == 0)
		return;

	drcuml->log_printf("[%s:", string);

	for (regnum = 0; regnum < 16; regnum++)
	{
		if (reglist[0] & REGFLAG_R(regnum))
		{
			drcuml->log_printf("%sr%d", (count++ == 0) ? "" : ",", regnum);
			if (regnostarlist != NULL && !(regnostarlist[0] & REGFLAG_R(regnum)))
				drcuml->log_printf("*");
		}
	}

	if (reglist[1] & REGFLAG_PR)
	{
		drcuml->log_printf("%spr", (count++ == 0) ? "" : ",");
		if (regnostarlist != NULL && !(regnostarlist[1] & REGFLAG_PR))
			drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_SR)
	{
		drcuml->log_printf("%ssr", (count++ == 0) ? "" : ",");
		if (regnostarlist != NULL && !(regnostarlist[1] & REGFLAG_SR))
			drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_MACL)
	{
		drcuml->log_printf("%smacl", (count++ == 0) ? "" : ",");
		if (regnostarlist != NULL && !(regnostarlist[1] & REGFLAG_MACL))
			drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_MACH)
	{
		drcuml->log_printf("%smach", (count++ == 0) ? "" : ",");
		if (regnostarlist != NULL && !(regnostarlist[1] & REGFLAG_MACH))
			drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_GBR)
	{
		drcuml->log_printf("%sgbr", (count++ == 0) ? "" : ",");
		if (regnostarlist != NULL && !(regnostarlist[1] & REGFLAG_GBR))
			drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_VBR)
	{
		drcuml->log_printf("%svbr", (count++ == 0) ? "" : ",");
		if (regnostarlist != NULL && !(regnostarlist[1] & REGFLAG_VBR))
			drcuml->log_printf("*");
	}

	drcuml->log_printf("] ");
}

/*-------------------------------------------------
    log_opcode_desc - log a list of descriptions
-------------------------------------------------*/

static void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent)
{
	/* open the file, creating it if necessary */
	if (indent == 0)
		drcuml->log_printf("\nDescriptor list @ %08X\n", desclist->pc);

	/* output each descriptor */
	for ( ; desclist != NULL; desclist = desclist->next())
	{
		char buffer[100];

		/* disassemle the current instruction and output it to the log */
#if (LOG_UML || LOG_NATIVE)
		if (desclist->flags & OPFLAG_VIRTUAL_NOOP)
			strcpy(buffer, "<virtual nop>");
		else
			DasmSH2(buffer, desclist->pc, desclist->opptr.w[0]);
#else
		strcpy(buffer, "???");
#endif
		drcuml->log_printf("%08X [%08X] t:%08X f:%s: %-30s", desclist->pc, desclist->physpc, desclist->targetpc, log_desc_flags_to_string(desclist->flags), buffer);

		/* output register states */
		log_register_list(drcuml, "use", desclist->regin, NULL);
		log_register_list(drcuml, "mod", desclist->regout, desclist->regreq);
		drcuml->log_printf("\n");

		/* if we have a delay slot, output it recursively */
		if (desclist->delay.first() != NULL)
			log_opcode_desc(drcuml, desclist->delay.first(), indent + 1);

		/* at the end of a sequence add a dividing line */
		if (desclist->flags & OPFLAG_END_SEQUENCE)
			drcuml->log_printf("-----\n");
	}
}

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a MIPS instruction
-------------------------------------------------*/

static void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op)
{
#if (LOG_UML)
	char buffer[100];
	DasmSH2(buffer, pc, op);
	block->append_comment("%08X: %s", pc, buffer);					// comment
#endif
}

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/
static void generate_update_cycles(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, parameter param, int allow_exception)
{
	/* check full interrupts if pending */
	if (compiler->checkints)
	{
		code_label skip = compiler->labelnum++;

		compiler->checkints = FALSE;
		compiler->labelnum += 4;

		/* check for interrupts */
		UML_MOV(block, mem(&sh2->irqline), 0xffffffff);		// mov irqline, #-1
		UML_CMP(block, mem(&sh2->pending_nmi), 0);			// cmp pending_nmi, #0
		UML_JMPc(block, COND_Z, skip+2);					// jz skip+2

		UML_MOV(block, mem(&sh2->pending_nmi), 0);			// zap pending_nmi
		UML_JMP(block, skip+1);						// and then go take it (evec is already set)

		UML_LABEL(block, skip+2);					// skip+2:
		UML_MOV(block, mem(&sh2->evec), 0xffffffff);		// mov evec, -1
		UML_MOV(block, I0, 0xffffffff);			// mov r0, -1 (r0 = irq)
		UML_AND(block, I1,  I0, 0xffff);	    		// and r1, r0, 0xffff

		UML_LZCNT(block, I1, mem(&sh2->pending_irq));		// lzcnt r1, pending_irq
		UML_CMP(block, I1, 32);				// cmp r1, #32
		UML_JMPc(block, COND_Z, skip+4);					// jz skip+4

		UML_SUB(block, mem(&sh2->irqline), 31, I1);		// sub irqline, #31, r1

		UML_LABEL(block, skip+4);					// skip+4:
		UML_CMP(block, mem(&sh2->internal_irq_level), 0xffffffff);	// cmp internal_irq_level, #-1
		UML_JMPc(block, COND_Z, skip+3);					// jz skip+3
		UML_CMP(block, mem(&sh2->internal_irq_level), mem(&sh2->irqline));		// cmp internal_irq_level, irqline
		UML_JMPc(block, COND_LE, skip+3);					// jle skip+3

		UML_MOV(block, mem(&sh2->irqline), mem(&sh2->internal_irq_level));		// mov r0, internal_irq_level

		UML_LABEL(block, skip+3);					// skip+3:
		UML_CMP(block, mem(&sh2->irqline), 0xffffffff);		// cmp irqline, #-1
		UML_JMPc(block, COND_Z, skip+1);					// jz skip+1
		UML_CALLC(block, cfunc_fastirq, sh2);				// callc fastirq

		UML_LABEL(block, skip+1);					// skip+1:
		UML_CMP(block, mem(&sh2->evec), 0xffffffff);		// cmp evec, 0xffffffff
		UML_JMPc(block, COND_Z, skip);  				// jz skip

		UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
		UML_MOV(block, I0, R32(15));				// mov r0, R15
		UML_MOV(block, I1, mem(&sh2->irqsr));			// mov r1, irqsr
		UML_CALLH(block, *sh2->write32);					// call write32

		UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
		UML_MOV(block, I0, R32(15));				// mov r0, R15
		UML_MOV(block, I1, param);	    		// mov r1, nextpc
		UML_CALLH(block, *sh2->write32);					// call write32

		UML_HASHJMP(block, 0, mem(&sh2->evec), *sh2->nocode);   	// hashjmp sh2->evec

		UML_LABEL(block, skip);	    					// skip:
	}

	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&sh2->icount), mem(&sh2->icount), MAPVAR_CYCLES);	// sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);										// mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *sh2->out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler->cycles = 0;
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

static void generate_checksum_block(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (LOG_UML)
		block->append_comment("[Validation for %08X]", seqhead->pc);				// comment

	/* loose verify or single instruction: just compare and fail */
	if (!(sh2->drcoptions & SH2DRC_STRICT_VERIFY) || seqhead->next() == NULL)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			void *base = sh2->direct->read_decrypted_ptr(seqhead->physpc, SH2_CODE_XOR(0));
			UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x2);							// load    i0,base,word
			UML_CMP(block, I0, seqhead->opptr.w[0]);						// cmp     i0,*opptr
			UML_EXHc(block, COND_NE, *sh2->nocode, epc(seqhead));		// exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
#if 0
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = sh2->direct->read_decrypted_ptr(curdesc->physpc, SH2_CODE_XOR(0));
				UML_LOAD(block, I0, curdesc->opptr.w, 0, SIZE_WORD, SCALE_x2);			// load    i0,*opptr,0,word
				UML_CMP(block, I0, curdesc->opptr.w[0]);					// cmp     i0,*opptr
				UML_EXHc(block, COND_NE, *sh2->nocode, epc(seqhead));	// exne    nocode,seqhead->pc
			}
#else
		UINT32 sum = 0;
		void *base = sh2->direct->read_decrypted_ptr(seqhead->physpc, SH2_CODE_XOR(0));
		UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x4);								// load    i0,base,word
		sum += seqhead->opptr.w[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = sh2->direct->read_decrypted_ptr(curdesc->physpc, SH2_CODE_XOR(0));
				UML_LOAD(block, I1, base, 0, SIZE_WORD, SCALE_x2);						// load    i1,*opptr,word
				UML_ADD(block, I0, I0, I1);							// add     i0,i0,i1
				sum += curdesc->opptr.w[0];
			}
		UML_CMP(block, I0, sum);											// cmp     i0,sum
		UML_EXHc(block, COND_NE, *sh2->nocode, epc(seqhead));			// exne    nocode,seqhead->pc
#endif
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

static void generate_sequence_instruction(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc)
{
	offs_t expc;

	/* add an entry for the log */
	if (LOG_UML && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.w[0]);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 1 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);												// mapvar  PC,expc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);								// mapvar  CYCLES,compiler->cycles

	/* if we want a probe, add it here */
	if (desc->pc == PROBE_ADDRESS)
	{
		UML_MOV(block, mem(&sh2->pc), desc->pc);								// mov     [pc],desc->pc
		UML_CALLC(block, cfunc_printf_probe, sh2);									// callc   cfunc_printf_probe,sh2
	}

	/* if we are debugging, call the debugger */
	if ((sh2->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&sh2->pc), desc->pc);								// mov     [pc],desc->pc
		save_fast_iregs(sh2, block);
		UML_DEBUG(block, desc->pc);											// debug   desc->pc
	}
	else	// not debug, see what other reasons there are for flushing the PC
	{
		if (sh2->drcoptions & SH2DRC_FLUSH_PC)	// always flush?
		{
			UML_MOV(block, mem(&sh2->pc), desc->pc);		// mov sh2->pc, desc->pc
		}
		else	// check for driver-selected flushes
		{
			int pcflush;

			for (pcflush = 0; pcflush < sh2->pcfsel; pcflush++)
			{
				if (desc->pc == sh2->pcflushes[pcflush])
				{
					UML_MOV(block, mem(&sh2->pc), desc->pc);		// mov sh2->pc, desc->pc
				}
			}
		}
	}


	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&sh2->pc), desc->pc);								// mov     [pc],desc->pc
		save_fast_iregs(sh2, block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);								// exit    EXECUTE_UNMAPPED_CODE
	}

	/* if this is an invalid opcode, die */
	if (desc->flags & OPFLAG_INVALID_OPCODE)
	{
		fatalerror("SH2DRC: invalid opcode!\n");
	}

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	else if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(sh2, block, compiler, desc, ovrpc))
		{
			UML_MOV(block, mem(&sh2->pc), desc->pc);							// mov     [pc],desc->pc
			UML_MOV(block, mem(&sh2->arg0), desc->opptr.w[0]);					// mov     [arg0],opcode
			UML_CALLC(block, cfunc_unimplemented, sh2);								// callc   cfunc_unimplemented
		}
	}
}

/*------------------------------------------------------------------
    generate_delay_slot
------------------------------------------------------------------*/

static void generate_delay_slot(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc)
{
	compiler_state compiler_temp = *compiler;

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay.first() != NULL);
	generate_sequence_instruction(sh2, block, &compiler_temp, desc->delay.first(), ovrpc);				// <next instruction>

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;
}

/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

static int generate_opcode(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc)
{
	UINT32 scratch, scratch2;
	INT32 disp;
	UINT16 opcode = desc->opptr.w[0];
	UINT8 opswitch = opcode >> 12;
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);

	switch (opswitch)
	{
		case  0:
			return generate_group_0(sh2, block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  1:	// MOVLS4
			scratch = (opcode & 0x0f) * 4;
			UML_ADD(block, I0, R32(Rn), scratch);	// add r0, Rn, scratch
			UML_MOV(block, I1, R32(Rm));		// mov r1, Rm
			SETEA(0);						// set ea for debug
			UML_CALLH(block, *sh2->write32);

			if (!in_delay_slot)
				generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
			return TRUE;

		case  2:
			return generate_group_2(sh2, block, compiler, desc, opcode, in_delay_slot, ovrpc);
		case  3:
			return generate_group_3(sh2, block, compiler, desc, opcode, ovrpc);
		case  4:
			return generate_group_4(sh2, block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  5:	// MOVLL4
			scratch = (opcode & 0x0f) * 4;
			UML_ADD(block, I0, R32(Rm), scratch);		// add r0, Rm, scratch
			SETEA(0);						// set ea for debug
			UML_CALLH(block, *sh2->read32);				// call read32
			UML_MOV(block, R32(Rn), I0);			// mov Rn, r0

			if (!in_delay_slot)
				generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
			return TRUE;

		case  6:
			return generate_group_6(sh2, block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  7:	// ADDI
			scratch = opcode & 0xff;
			scratch2 = (UINT32)(INT32)(INT16)(INT8)scratch;
			UML_ADD(block, R32(Rn), R32(Rn), scratch2);	// add Rn, Rn, scratch2
			return TRUE;

		case  8:
			return generate_group_8(sh2, block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  9:	// MOVWI
			if (ovrpc == 0xffffffff)
			{
				scratch = (desc->pc + 2) + ((opcode & 0xff) * 2) + 2;
			}
			else
			{
				scratch = (ovrpc + 2) + ((opcode & 0xff) * 2) + 2;
			}

			if (sh2->drcoptions & SH2DRC_STRICT_PCREL)
			{
				UML_MOV(block, I0, scratch);			// mov r0, scratch
				SETEA(0);						// set ea for debug
				UML_CALLH(block, *sh2->read16);				// read16(r0, r1)
				UML_SEXT(block, R32(Rn), I0, SIZE_WORD);    		// sext Rn, r0, WORD
			}
			else
			{
				scratch2 = (UINT32)(INT32)(INT16) RW(sh2, scratch);
				UML_MOV(block, R32(Rn), scratch2);			// mov Rn, scratch2
			}

			if (!in_delay_slot)
				generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
			return TRUE;

		case 10:	// BRA
			disp = ((INT32)opcode << 20) >> 20;
			sh2->ea = (desc->pc + 2) + disp * 2 + 2;			// sh2->ea = pc+4 + disp*2 + 2

			generate_delay_slot(sh2, block, compiler, desc, sh2->ea-2);

			generate_update_cycles(sh2, block, compiler, sh2->ea, TRUE);	// <subtract cycles>
			UML_HASHJMP(block, 0, sh2->ea, *sh2->nocode);	// hashjmp sh2->ea
			return TRUE;

		case 11:	// BSR
			// panicstr @ 403da22 relies on the delay slot clobbering the PR set by a BSR, so
			// do this before running the delay slot
			UML_ADD(block, mem(&sh2->pr), desc->pc, 4);	// add sh2->pr, desc->pc, #4 (skip the current insn & delay slot)

			disp = ((INT32)opcode << 20) >> 20;
			sh2->ea = (desc->pc + 2) + disp * 2 + 2;			// sh2->ea = pc+4 + disp*2 + 2

			generate_delay_slot(sh2, block, compiler, desc, sh2->ea-2);

			generate_update_cycles(sh2, block, compiler, sh2->ea, TRUE);	// <subtract cycles>
			UML_HASHJMP(block, 0, sh2->ea, *sh2->nocode);	// hashjmp sh2->ea
			return TRUE;

		case 12:
			return generate_group_12(sh2, block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case 13:	// MOVLI
			if (ovrpc == 0xffffffff)
			{
				scratch = ((desc->pc + 4) & ~3) + ((opcode & 0xff) * 4);
			}
			else
			{
				scratch = ((ovrpc + 4) & ~3) + ((opcode & 0xff) * 4);
			}

			if (sh2->drcoptions & SH2DRC_STRICT_PCREL)
			{
				UML_MOV(block, I0, scratch);			// mov r0, scratch
				UML_CALLH(block, *sh2->read32);				// read32(r0, r1)
				UML_MOV(block, R32(Rn), I0);			// mov Rn, r0
			}
			else
			{
				scratch2 = RL(sh2, scratch);
				UML_MOV(block, R32(Rn), scratch2);			// mov Rn, scratch2
			}

			if (!in_delay_slot)
				generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
			return TRUE;

		case 14:	// MOVI
			scratch = opcode & 0xff;
			scratch2 = (UINT32)(INT32)(INT16)(INT8)scratch;
			UML_MOV(block, R32(Rn), scratch2);
			return TRUE;

		case 15:	// NOP
			return TRUE;
	}

	return FALSE;
}

static int generate_group_0(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc)
{
	switch (opcode & 0x3F)
	{
	case 0x00: // NOP();
	case 0x01: // NOP();
	case 0x09: // NOP();
	case 0x10: // NOP();
	case 0x11: // NOP();
	case 0x13: // NOP();
	case 0x20: // NOP();
	case 0x21: // NOP();
	case 0x30: // NOP();
	case 0x31: // NOP();
	case 0x32: // NOP();
	case 0x33: // NOP();
	case 0x38: // NOP();
	case 0x39: // NOP();
	case 0x3a: // NOP();
	case 0x3b: // NOP();
		return TRUE;

	case 0x02: // STCSR(Rn);
		UML_MOV(block, R32(Rn), mem(&sh2->sr));
		return TRUE;

	case 0x03: // BSRF(Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_ADD(block, mem(&sh2->target), R32(Rn), 4);	// add target, Rm, #4
			UML_ADD(block, mem(&sh2->target), mem(&sh2->target), desc->pc);	// add target, target, pc

			// 32x Cosmic Carnage @ 6002cb0 relies on the delay slot
			// clobbering the calculated PR, so do it first
			UML_ADD(block, mem(&sh2->pr), desc->pc, 4);	// add sh2->pr, desc->pc, #4 (skip the current insn & delay slot)

			generate_delay_slot(sh2, block, compiler, desc, sh2->target);

			generate_update_cycles(sh2, block, compiler, mem(&sh2->target), TRUE);	// <subtract cycles>
			UML_HASHJMP(block, 0, mem(&sh2->target), *sh2->nocode);	// jmp target
			return TRUE;
		}
		break;

	case 0x04: // MOVBS0(Rm, Rn);
	case 0x14: // MOVBS0(Rm, Rn);
	case 0x24: // MOVBS0(Rm, Rn);
	case 0x34: // MOVBS0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rn));		// add r0, R0, Rn
		UML_AND(block, I1, R32(Rm), 0x000000ff);	// and r1, Rm, 0xff
		UML_CALLH(block, *sh2->write8);				// call write8

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x05: // MOVWS0(Rm, Rn);
	case 0x15: // MOVWS0(Rm, Rn);
	case 0x25: // MOVWS0(Rm, Rn);
	case 0x35: // MOVWS0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rn));		// add r0, R0, Rn
		UML_AND(block, I1, R32(Rm), 0x0000ffff);	// and r1, Rm, 0xffff
		UML_CALLH(block, *sh2->write16);				// call write16

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x06: // MOVLS0(Rm, Rn);
	case 0x16: // MOVLS0(Rm, Rn);
	case 0x26: // MOVLS0(Rm, Rn);
	case 0x36: // MOVLS0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rn));		// add r0, R0, Rn
		UML_MOV(block, I1, R32(Rm));			// mov r1, Rm
		UML_CALLH(block, *sh2->write32);				// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x07: // MULL(Rm, Rn);
	case 0x17: // MULL(Rm, Rn);
	case 0x27: // MULL(Rm, Rn);
	case 0x37: // MULL(Rm, Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_MULU(block, mem(&sh2->macl), mem(&sh2->ea), R32(Rn), R32(Rm));	// mulu macl, ea, Rn, Rm
			return TRUE;
		}
		break;

	case 0x08: // CLRT();
		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and r0, sr, ~T (clear the T bit)
		return TRUE;

	case 0x0a: // STSMACH(Rn);
		UML_MOV(block, R32(Rn), mem(&sh2->mach));		// mov Rn, mach
		return TRUE;

	case 0x0b: // RTS();
		UML_MOV(block, mem(&sh2->target), mem(&sh2->pr));	// mov target, pr (in case of d-slot shenanigans)

		generate_delay_slot(sh2, block, compiler, desc, sh2->target);

		generate_update_cycles(sh2, block, compiler, mem(&sh2->target), TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&sh2->target), *sh2->nocode);
		return TRUE;

	case 0x0c: // MOVBL0(Rm, Rn);
	case 0x1c: // MOVBL0(Rm, Rn);
	case 0x2c: // MOVBL0(Rm, Rn);
	case 0x3c: // MOVBL0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rm));		// add r0, R0, Rm
		UML_CALLH(block, *sh2->read8);				// call read8
		UML_SEXT(block, R32(Rn), I0, SIZE_BYTE);		// sext Rn, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x0d: // MOVWL0(Rm, Rn);
	case 0x1d: // MOVWL0(Rm, Rn);
	case 0x2d: // MOVWL0(Rm, Rn);
	case 0x3d: // MOVWL0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rm));		// add r0, R0, Rm
		UML_CALLH(block, *sh2->read16);				// call read16
		UML_SEXT(block, R32(Rn), I0, SIZE_WORD);		// sext Rn, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x0e: // MOVLL0(Rm, Rn);
	case 0x1e: // MOVLL0(Rm, Rn);
	case 0x2e: // MOVLL0(Rm, Rn);
	case 0x3e: // MOVLL0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rm));		// add r0, R0, Rm
		UML_CALLH(block, *sh2->read32);				// call read32
		UML_MOV(block, R32(Rn), I0);			// mov Rn, r0

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x0f: // MAC_L(Rm, Rn);
	case 0x1f: // MAC_L(Rm, Rn);
	case 0x2f: // MAC_L(Rm, Rn);
	case 0x3f: // MAC_L(Rm, Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			save_fast_iregs(sh2, block);
			UML_MOV(block, mem(&sh2->arg0), desc->opptr.w[0]);
			UML_CALLC(block, cfunc_MAC_L, sh2);
			load_fast_iregs(sh2, block);
			return TRUE;
		}
		break;

	case 0x12: // STCGBR(Rn);
		UML_MOV(block, R32(Rn), mem(&sh2->gbr));		// mov Rn, gbr
		return TRUE;

	case 0x18: // SETT();
		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), T);	// or sr, sr, T
		return TRUE;

	case 0x19: // DIV0U();
		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~(M|Q|T));	// and sr, sr, ~(M|Q|T)
		return TRUE;

	case 0x1a: // STSMACL(Rn);
		UML_MOV(block, R32(Rn), mem(&sh2->macl));		// mov Rn, macl
		return TRUE;

	case 0x1b: // SLEEP();
		// inlined special version of generate_update_cycles here
		// if an interrupt is taken, the return address is set to the next instruction
		UML_CALLC(block, cfunc_checkirqs, sh2);

		UML_MOV(block, I0, mem(&sh2->evec));			// mov r0, evec
		UML_CMP(block, I0, 0xffffffff);			// cmp r0, 0xffffffff
		UML_JMPc(block, COND_Z, compiler->labelnum);			// jz skip

		UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
		UML_MOV(block, I0, R32(15));				// mov r0, R15
		UML_MOV(block, I1, mem(&sh2->irqsr));			// mov r1, irqsr
		UML_CALLH(block, *sh2->write32);					// call write32

		UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
		UML_MOV(block, I0, R32(15));				// mov r0, R15
		UML_MOV(block, I1, desc->pc+2);	    		// mov r1, nextpc
		UML_CALLH(block, *sh2->write32);					// call write32

		UML_HASHJMP(block, 0, mem(&sh2->evec), *sh2->nocode);   	// hashjmp sh2->evec

		UML_LABEL(block, compiler->labelnum++);	    			// skip:

		// now go "out of cycles"
		if (compiler->cycles > 0)
		{
			UML_MOV(block, mem(&sh2->icount), 0);			// mov icount, #0
			UML_MAPVAR(block, MAPVAR_CYCLES, 0);				// mapvar  cycles,0
			UML_EXH(block, *sh2->out_of_cycles, desc->pc);		// go out of cycles
		}
		else
		{
			UML_HASHJMP(block, 0, desc->pc, *sh2->nocode);
		}

		return TRUE;

	case 0x22: // STCVBR(Rn);
		UML_MOV(block, R32(Rn), mem(&sh2->vbr));		// mov Rn, vbr
		return TRUE;

	case 0x23: // BRAF(Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_ADD(block, mem(&sh2->target), R32(Rn), desc->pc+4);	// add target, Rn, pc+4

			generate_delay_slot(sh2, block, compiler, desc, sh2->target);

			generate_update_cycles(sh2, block, compiler, mem(&sh2->target), TRUE);	// <subtract cycles>
			UML_HASHJMP(block, 0, mem(&sh2->target), *sh2->nocode);	// jmp target
			return TRUE;
		}
		break;

	case 0x28: // CLRMAC();
		UML_MOV(block, mem(&sh2->macl), 0);		// mov macl, #0
		UML_MOV(block, mem(&sh2->mach), 0);		// mov mach, #0
		return TRUE;

	case 0x29: // MOVT(Rn);
		UML_AND(block, R32(Rn), mem(&sh2->sr), T);		// and Rn, sr, T
		return TRUE;

	case 0x2a: // STSPR(Rn);
		UML_MOV(block, R32(Rn), mem(&sh2->pr));			// mov Rn, pr
		return TRUE;

	case 0x2b: // RTE();
		generate_delay_slot(sh2, block, compiler, desc, 0xffffffff);

		UML_MOV(block, I0, R32(15));			// mov r0, R15
		UML_CALLH(block, *sh2->read32);				// call read32
		UML_MOV(block, mem(&sh2->pc), I0);			// mov pc, r0
		UML_ADD(block, R32(15), R32(15), 4);		// add R15, R15, #4

		UML_MOV(block, I0, R32(15));			// mov r0, R15
		UML_CALLH(block, *sh2->read32);				// call read32
		UML_MOV(block, mem(&sh2->sr), I0);			// mov sr, r0
		UML_ADD(block, R32(15), R32(15), 4);		// add R15, R15, #4

		compiler->checkints = TRUE;
		UML_MOV(block, mem(&sh2->ea), mem(&sh2->pc));		// mov ea, pc
		generate_update_cycles(sh2, block, compiler, mem(&sh2->ea), TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&sh2->pc), *sh2->nocode);	// and jump to the "resume PC"

		return TRUE;
	}

	return FALSE;
}

static int generate_group_2(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc)
{
	switch (opcode & 15)
	{
	case  0: // MOVBS(Rm, Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_AND(block, I1, R32(Rm), 0xff);	// and r1, Rm, 0xff
		UML_CALLH(block, *sh2->write8);

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  1: // MOVWS(Rm, Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_AND(block, I1, R32(Rm), 0xffff);	// and r1, Rm, 0xffff
		UML_CALLH(block, *sh2->write16);

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  2: // MOVLS(Rm, Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_MOV(block, I1, R32(Rm));		// mov r1, Rm
		UML_CALLH(block, *sh2->write32);

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  3: // NOP();
		return TRUE;

	case  4: // MOVBM(Rm, Rn);
		UML_MOV(block, I1, R32(Rm));		// mov r1, Rm
		UML_SUB(block, R32(Rn), R32(Rn), 1);	// sub Rn, Rn, 1
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_CALLH(block, *sh2->write8);			// call write8

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  5: // MOVWM(Rm, Rn);
		UML_MOV(block, I1, R32(Rm));		// mov r1, Rm
		UML_SUB(block, R32(Rn), R32(Rn), 2);	// sub Rn, Rn, 2
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_CALLH(block, *sh2->write16);			// call write16

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  6: // MOVLM(Rm, Rn);
		UML_MOV(block, I1, R32(Rm));		// mov r1, Rm
		UML_SUB(block, R32(Rn), R32(Rn), 4);	// sub Rn, Rn, 4
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_CALLH(block, *sh2->write32);			// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 13: // XTRCT(Rm, Rn);
		UML_SHL(block, I0, R32(Rm), 16);		// shl r0, Rm, #16
		UML_AND(block, I0, I0, 0xffff0000);	// and r0, r0, #0xffff0000

		UML_SHR(block, I1, R32(Rn), 16);		// shr, r1, Rn, #16
		UML_AND(block, I1, I1, 0xffff);		// and r1, r1, #0x0000ffff

		UML_OR(block, R32(Rn), I0, I1);		// or Rn, r0, r1
		return TRUE;

	case  7: // DIV0S(Rm, Rn);
		UML_MOV(block, I0, mem(&sh2->sr));				// move r0, sr
		UML_AND(block, I0, I0, ~(Q|M|T));		// and r0, r0, ~(Q|M|T) (clear the Q,M, and T bits)

		UML_TEST(block, R32(Rn), 0x80000000);			// test Rn, #0x80000000
		UML_JMPc(block, COND_Z, compiler->labelnum);			// jz labelnum

		UML_OR(block, I0, I0, Q);   			// or r0, r0, Q
		UML_LABEL(block, compiler->labelnum++);				// labelnum:

		UML_TEST(block, R32(Rm), 0x80000000);			// test Rm, #0x80000000
		UML_JMPc(block, COND_Z, compiler->labelnum);			// jz labelnum

		UML_OR(block, I0, I0, M);   			// or r0, r0, M
		UML_LABEL(block, compiler->labelnum++);				// labelnum:

		UML_XOR(block, I1, R32(Rn), R32(Rm));			// xor r1, Rn, Rm
		UML_TEST(block, I1, 0x80000000);			// test r1, #0x80000000
		UML_JMPc(block, COND_Z, compiler->labelnum);			// jz labelnum

		UML_OR(block, I0, I0, T);   			// or r0, r0, T
		UML_LABEL(block, compiler->labelnum++);				// labelnum:
		UML_MOV(block, mem(&sh2->sr), I0);				// mov sr, r0
		return TRUE;

	case  8: // TST(Rm, Rn);
		UML_AND(block, I0, mem(&sh2->sr), ~T);	// and r0, sr, ~T (clear the T bit)
		UML_TEST(block, R32(Rm), R32(Rn));		// test Rm, Rn
		UML_JMPc(block, COND_NZ, compiler->labelnum);	// jnz compiler->labelnum

		UML_OR(block, I0, I0, T);	// or r0, r0, T
		UML_LABEL(block, compiler->labelnum++);	    	// desc->pc:

		UML_MOV(block, mem(&sh2->sr), I0);		// mov sh2->sr, r0
		return TRUE;

	case 12: // CMPSTR(Rm, Rn);
		UML_XOR(block, I0, R32(Rn), R32(Rm));	// xor r0, Rn, Rm       (temp)

		UML_SHR(block, I1, I0, 24);	// shr r1, r0, #24  (HH)
		UML_AND(block, I1, I1, 0xff);	// and r1, r1, #0xff

		UML_SHR(block, I2, I0, 16);	// shr r2, r0, #16  (HL)
		UML_AND(block, I2, I2, 0xff);	// and r2, r2, #0xff

		UML_SHR(block, I3, I0, 8);	// shr r3, r0, #8   (LH)
		UML_AND(block, I3, I3, 0xff);	// and r3, r3, #0xff

		UML_AND(block, I7, I0, 0xff);	// and r7, r0, #0xff    (LL)

		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T (clear the T bit)

		UML_CMP(block, I1, 0);		// cmp r1, #0
		UML_JMPc(block, COND_Z, compiler->labelnum);	// jnz labelnum
		UML_CMP(block, I2, 0);		// cmp r2, #0
		UML_JMPc(block, COND_Z, compiler->labelnum);	// jnz labelnum
		UML_CMP(block, I3, 0);		// cmp r3, #0
		UML_JMPc(block, COND_Z, compiler->labelnum);	// jnz labelnum
		UML_CMP(block, I7, 0);		// cmp r7, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum+1);	// jnz labelnum

		UML_LABEL(block, compiler->labelnum++);		// labelnum:
		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), T);	// or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);		// labelnum+1:
		return TRUE;

	case  9: // AND(Rm, Rn);
		UML_AND(block, R32(Rn), R32(Rn), R32(Rm));	// and Rn, Rn, Rm
		return TRUE;

	case 10: // XOR(Rm, Rn);
		UML_XOR(block, R32(Rn), R32(Rn), R32(Rm));	// xor Rn, Rn, Rm
		return TRUE;

	case 11: // OR(Rm, Rn);
		UML_OR(block, R32(Rn), R32(Rn), R32(Rm));	// or Rn, Rn, Rm
		return TRUE;

	case 14: // MULU(Rm, Rn);
		UML_AND(block, I0, R32(Rm), 0xffff);				// and r0, Rm, 0xffff
		UML_AND(block, I1, R32(Rn), 0xffff);				// and r1, Rn, 0xffff
		UML_MULU(block, mem(&sh2->macl), mem(&sh2->ea), I0, I1);	// mulu macl, ea, r0, r1
		return TRUE;

	case 15: // MULS(Rm, Rn);
		UML_SEXT(block, I0, R32(Rm), SIZE_WORD);				// sext r0, Rm
		UML_SEXT(block, I1, R32(Rn), SIZE_WORD);				// sext r1, Rn
		UML_MULS(block, mem(&sh2->macl), mem(&sh2->ea), I0, I1);	// muls macl, ea, r0, r1
		return TRUE;
	}

	return FALSE;
}

static int generate_group_3(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, UINT32 ovrpc)
{
	switch (opcode & 15)
	{
	case  0: // CMPEQ(Rm, Rn); (equality)
		UML_CMP(block, R32(Rn), R32(Rm));		// cmp Rn, Rm
		UML_SETc(block, COND_E, I0);			// set E, r0
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, 1);	// rolins sr, r0, 0, 1
		return TRUE;

	case  2: // CMPHS(Rm, Rn); (unsigned greater than or equal)
		UML_CMP(block, R32(Rn), R32(Rm));		// cmp Rn, Rm
		UML_SETc(block, COND_AE, I0);		// set AE, r0
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, 1);	// rolins sr, r0, 0, 1
		return TRUE;

	case  3: // CMPGE(Rm, Rn); (signed greater than or equal)
		UML_CMP(block, R32(Rn), R32(Rm));		// cmp Rn, Rm
		UML_SETc(block, COND_GE, I0);		// set GE, r0
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, 1);	// rolins sr, r0, 0, 1
		return TRUE;

	case  6: // CMPHI(Rm, Rn); (unsigned greater than)
		UML_CMP(block, R32(Rn), R32(Rm));		// cmp Rn, Rm
		UML_SETc(block, COND_A, I0);			// set A, r0
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, 1);	// rolins sr, r0, 0, 1
		return TRUE;

	case  7: // CMPGT(Rm, Rn); (signed greater than)
		UML_CMP(block, R32(Rn), R32(Rm));		// cmp Rn, Rm
		UML_SETc(block, COND_G, I0);			// set G, r0
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, 1);	// rolins sr, r0, 0, 1
		return TRUE;

	case  1: // NOP();
	case  9: // NOP();
		return TRUE;

	case  4: // DIV1(Rm, Rn);
		save_fast_iregs(sh2, block);
		UML_MOV(block, mem(&sh2->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_DIV1, sh2);
		load_fast_iregs(sh2, block);
		return TRUE;

	case  5: // DMULU(Rm, Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_MULU(block, mem(&sh2->macl), mem(&sh2->mach), R32(Rn), R32(Rm));
			return TRUE;
		}
		break;

	case 13: // DMULS(Rm, Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_MULS(block, mem(&sh2->macl), mem(&sh2->mach), R32(Rn), R32(Rm));
			return TRUE;
		}
		break;

	case  8: // SUB(Rm, Rn);
		UML_SUB(block, R32(Rn), R32(Rn), R32(Rm));	// sub Rn, Rn, Rm
		return TRUE;

	case 12: // ADD(Rm, Rn);
		UML_ADD(block, R32(Rn), R32(Rn), R32(Rm));	// add Rn, Rn, Rm
		return TRUE;

	case 10: // SUBC(Rm, Rn);
		UML_CARRY(block, mem(&sh2->sr), 0);	// carry = T (T is bit 0 of SR)
		UML_SUBB(block, R32(Rn), R32(Rn), R32(Rm));	// addc Rn, Rn, Rm
		UML_SETc(block, COND_C, I0);				// setc    i0, C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins sr,i0,0,T
		return TRUE;

	case 11: // SUBV(Rm, Rn);
#if ADDSUBV_DIRECT
		UML_SUB(block, R32(Rn), R32(Rn), R32(Rm));		// sub Rn, Rn, Rm
		UML_SETc(block, COND_V, I0);					// setc    i0, V
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins [sr],i0,0,T
#else
		save_fast_iregs(sh2, block);
		UML_MOV(block, mem(&sh2->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_SUBV, sh2);
		load_fast_iregs(sh2, block);
#endif
		return TRUE;

	case 14: // ADDC(Rm, Rn);
		UML_CARRY(block, mem(&sh2->sr), 0);	// carry = T (T is bit 0 of SR)
		UML_ADDC(block, R32(Rn), R32(Rn), R32(Rm));	// addc Rn, Rn, Rm
		UML_SETc(block, COND_C, I0);				// setc    i0, C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins sr,i0,0,T
		return TRUE;

	case 15: // ADDV(Rm, Rn);
#if ADDSUBV_DIRECT
		UML_ADD(block, R32(Rn), R32(Rn), R32(Rm));		// add Rn, Rn, Rm
		UML_SETc(block, COND_V, I0);					// setc    i0, V
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins [sr],i0,0,T
#else
		save_fast_iregs(sh2, block);
		UML_MOV(block, mem(&sh2->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_ADDV, sh2);
		load_fast_iregs(sh2, block);
#endif
		return TRUE;
	}
	return FALSE;
}

static int generate_group_4(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc)
{
	switch (opcode & 0x3F)
	{
	case 0x00: // SHLL(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 1);		// shl Rn, Rn, 1
		UML_SETc(block, COND_C, I0);					// set i0,C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins [sr],i0,0,T
		return TRUE;

	case 0x01: // SHLR(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 1);		// shr Rn, Rn, 1
		UML_SETc(block, COND_C, I0);					// set i0,C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins [sr],i0,0,T
		return TRUE;

	case 0x04: // ROTL(Rn);
		UML_ROL(block, R32(Rn), R32(Rn), 1);		// rol Rn, Rn, 1
		UML_SETc(block, COND_C, I0);					// set i0,C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins [sr],i0,0,T
		return TRUE;

	case 0x05: // ROTR(Rn);
		UML_ROR(block, R32(Rn), R32(Rn), 1);		// ror Rn, Rn, 1
		UML_SETc(block, COND_C, I0);					// set i0,C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins [sr],i0,0,T
		return TRUE;

	case 0x02: // STSMMACH(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);	// sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_MOV(block, I1, mem(&sh2->mach));	// mov r1, mach
		SETEA(0);					// set ea for debug
		UML_CALLH(block, *sh2->write32);			// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x03: // STCMSR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);	// sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_MOV(block, I1, mem(&sh2->sr));		// mov r1, sr
		SETEA(0);					// set ea for debug
		UML_CALLH(block, *sh2->write32);			// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x06: // LDSMMACH(Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read32);			// call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);	// add Rn, #4
		UML_MOV(block, mem(&sh2->mach), I0);	// mov mach, r0

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x07: // LDCMSR(Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read32);			// call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);	// add Rn, #4
		UML_MOV(block, mem(&sh2->sr), I0);		// mov sr, r0

		compiler->checkints = TRUE;
		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;


	case 0x08: // SHLL2(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 2);
		return TRUE;

	case 0x09: // SHLR2(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 2);
		return TRUE;

	case 0x18: // SHLL8(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 8);
		return TRUE;

	case 0x19: // SHLR8(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 8);
		return TRUE;

	case 0x28: // SHLL16(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 16);
		return TRUE;

	case 0x29: // SHLR16(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 16);
		return TRUE;

	case 0x0a: // LDSMACH(Rn);
		UML_MOV(block, mem(&sh2->mach), R32(Rn));		// mov mach, Rn
		return TRUE;

	case 0x0b: // JSR(Rn);
		UML_MOV(block, mem(&sh2->target), R32(Rn));		// mov target, Rn

		UML_ADD(block, mem(&sh2->pr), desc->pc, 4);	// add sh2->pr, desc->pc, #4 (skip the current insn & delay slot)

		generate_delay_slot(sh2, block, compiler, desc, sh2->target-4);

		generate_update_cycles(sh2, block, compiler, mem(&sh2->target), TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&sh2->target), *sh2->nocode);	// and do the jump
		return TRUE;

	case 0x0e: // LDCSR(Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_AND(block, I0, I0, FLAGS);	// and r0, r0, FLAGS
		UML_MOV(block, mem(&sh2->sr), I0);

		compiler->checkints = TRUE;
		return TRUE;

	case 0x0f: // MAC_W(Rm, Rn);
	case 0x1f: // MAC_W(Rm, Rn);
	case 0x2f: // MAC_W(Rm, Rn);
	case 0x3f: // MAC_W(Rm, Rn);
		save_fast_iregs(sh2, block);
		UML_MOV(block, mem(&sh2->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_MAC_W, sh2);
		load_fast_iregs(sh2, block);
		return TRUE;

	case 0x10: // DT(Rn);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_AND(block, I0, mem(&sh2->sr), ~T);	// and r0, sr, ~T (clear the T bit)
			UML_SUB(block, R32(Rn), R32(Rn), 1);	// sub Rn, Rn, 1
			UML_JMPc(block, COND_NZ, compiler->labelnum);	// jz compiler->labelnum

			UML_OR(block, I0, I0, T);	// or r0, r0, T
			UML_LABEL(block, compiler->labelnum++);	    	// desc->pc:

			UML_MOV(block, mem(&sh2->sr), I0);		// mov sh2->sr, r0
			return TRUE;
		}
		break;

	case 0x11: // CMPPZ(Rn);
		UML_AND(block, I0, mem(&sh2->sr), ~T);	// and r0, sr, ~T (clear the T bit)

		UML_CMP(block, R32(Rn), 0);		// cmp Rn, 0
		UML_JMPc(block, COND_S, compiler->labelnum);	// js compiler->labelnum    (if negative)

		UML_OR(block, I0, I0, T);	// or r0, r0, T
		UML_LABEL(block, compiler->labelnum++);	    	// desc->pc:

		UML_MOV(block, mem(&sh2->sr), I0);		// mov sh2->sr, r0
		return TRUE;

	case 0x15: // CMPPL(Rn);
		UML_AND(block, I0, mem(&sh2->sr), ~T);	// and r0, sr, ~T (clear the T bit)

		UML_CMP(block, R32(Rn), 0);		// cmp Rn, 0

		UML_JMPc(block, COND_S, compiler->labelnum);	// js compiler->labelnum    (if negative)
		UML_JMPc(block, COND_Z, compiler->labelnum);	// jz compiler->labelnum    (if zero)

		UML_OR(block, I0, I0, T);	// or r0, r0, T

		UML_LABEL(block, compiler->labelnum++);	    	// desc->pc:
		UML_MOV(block, mem(&sh2->sr), I0);		// mov sh2->sr, r0
		return TRUE;

	case 0x12: // STSMMACL(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);	// sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_MOV(block, I1, mem(&sh2->macl));	// mov r1, macl
		SETEA(0);					// set ea for debug
		UML_CALLH(block, *sh2->write32);			// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x13: // STCMGBR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);	// sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_MOV(block, I1, mem(&sh2->gbr));	// mov r1, gbr
		SETEA(0);					// set ea for debug
		UML_CALLH(block, *sh2->write32);			// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x16: // LDSMMACL(Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read32);			// call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);	// add Rn, #4
		UML_MOV(block, mem(&sh2->macl), I0);	// mov macl, r0

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x17: // LDCMGBR(Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read32);			// call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);	// add Rn, #4
		UML_MOV(block, mem(&sh2->gbr), I0);	// mov gbr, r0

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x1a: // LDSMACL(Rn);
		UML_MOV(block, mem(&sh2->macl), R32(Rn));		// mov macl, Rn
		return TRUE;

	case 0x1b: // TAS(Rn);
		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read8);			// call read8

		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T

		UML_CMP(block, I0, 0);		// cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum);	// jnz labelnum

		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), T);	// or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);		// labelnum:

		UML_OR(block, I1, I0, 0x80);	// or r1, r0, #0x80

		UML_MOV(block, I0, R32(Rn));		// mov r0, Rn
		UML_CALLH(block, *sh2->write8);			// write the value back

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x1e: // LDCGBR(Rn);
		UML_MOV(block, mem(&sh2->gbr), R32(Rn));	// mov gbr, Rn
		return TRUE;

	case 0x20: // SHAL(Rn);
		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T
		UML_SHR(block, I0, R32(Rn), 31);		// shr r0, Rn, 31
		UML_AND(block, I0, I0, T);		// and r0, r0, T
		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), I0);	// or sr, sr, r0
		UML_SHL(block, R32(Rn), R32(Rn), 1);		// shl Rn, Rn, 1
		return TRUE;

	case 0x21: // SHAR(Rn);
		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T
		UML_AND(block, I0, R32(Rn), T);		// and r0, Rn, T
		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), I0);	// or sr, sr, r0
		UML_SAR(block, R32(Rn), R32(Rn), 1);		// sar Rn, Rn, 1
		return TRUE;

	case 0x22: // STSMPR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);		// sub Rn, Rn, 4
		UML_MOV(block, I0, R32(Rn));			// mov r0, Rn
		SETEA(0);
		UML_MOV(block, I1, mem(&sh2->pr));			// mov r1, pr
		UML_CALLH(block, *sh2->write32);				// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x23: // STCMVBR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);		// sub Rn, Rn, 4
		UML_MOV(block, I0, R32(Rn));			// mov r0, Rn
		SETEA(0);
		UML_MOV(block, I1, mem(&sh2->vbr));		// mov r1, vbr
		UML_CALLH(block, *sh2->write32);				// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x24: // ROTCL(Rn);
		UML_CARRY(block, mem(&sh2->sr), 0);			// carry sr,0
		UML_ROLC(block, R32(Rn), R32(Rn), 1);			// rolc  Rn,Rn,1
		UML_SETc(block, COND_C, I0);						// set   i0,C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins sr,i0,0,T
		return TRUE;

	case 0x25: // ROTCR(Rn);
		UML_CARRY(block, mem(&sh2->sr), 0);			// carry sr,0
		UML_RORC(block, R32(Rn), R32(Rn), 1);			// rorc  Rn,Rn,1
		UML_SETc(block, COND_C, I0);						// set   i0,C
		UML_ROLINS(block, mem(&sh2->sr), I0, 0, T); // rolins sr,i0,0,T
		return TRUE;

	case 0x26: // LDSMPR(Rn);
		UML_MOV(block, I0, R32(Rn));			// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read32);				// call read32
		UML_MOV(block, mem(&sh2->pr), I0);			// mov sh2->pr, r0
		UML_ADD(block, R32(Rn), R32(Rn), 4);		// add Rn, Rn, #4

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x27: // LDCMVBR(Rn);
		UML_MOV(block, I0, R32(Rn));			// mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *sh2->read32);				// call read32
		UML_MOV(block, mem(&sh2->vbr), I0);		// mov sh2->vbr, r0
		UML_ADD(block, R32(Rn), R32(Rn), 4);		// add Rn, Rn, #4

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case 0x2a: // LDSPR(Rn);
		UML_MOV(block, mem(&sh2->pr), R32(Rn));			// mov sh2->pr, Rn
		return TRUE;

	case 0x2b: // JMP(Rn);
		UML_MOV(block, mem(&sh2->target), R32(Rn));		// mov target, Rn

		generate_delay_slot(sh2, block, compiler, desc, sh2->target);

		generate_update_cycles(sh2, block, compiler, mem(&sh2->target), TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&sh2->target), *sh2->nocode);	// jmp (target)
		return TRUE;

	case 0x2e: // LDCVBR(Rn);
		UML_MOV(block, mem(&sh2->vbr), R32(Rn));		//  mov vbr, Rn
		return TRUE;

	case 0x0c: // NOP();
	case 0x0d: // NOP();
	case 0x14: // NOP();
	case 0x1c: // NOP();
	case 0x1d: // NOP();
	case 0x2c: // NOP();
	case 0x2d: // NOP();
	case 0x30: // NOP();
	case 0x31: // NOP();
	case 0x32: // NOP();
	case 0x33: // NOP();
	case 0x34: // NOP();
	case 0x35: // NOP();
	case 0x36: // NOP();
	case 0x37: // NOP();
	case 0x38: // NOP();
	case 0x39: // NOP();
	case 0x3a: // NOP();
	case 0x3b: // NOP();
	case 0x3c: // NOP();
	case 0x3d: // NOP();
	case 0x3e: // NOP();
		return TRUE;
	}

	return FALSE;
}

static int generate_group_6(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc)
{
	switch (opcode & 15)
	{
	case  0: // MOVBL(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));		// mov r0, Rm
		SETEA(0);					// debug: ea = r0
		UML_CALLH(block, *sh2->read8);			// call read8
		UML_SEXT(block, R32(Rn), I0, SIZE_BYTE);	// sext Rn, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  1: // MOVWL(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));		// mov r0, Rm
		SETEA(0);					// debug: ea = r0
		UML_CALLH(block, *sh2->read16);			// call read16
		UML_SEXT(block, R32(Rn), I0, SIZE_WORD);	// sext Rn, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  2: // MOVLL(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));		// mov r0, Rm
		SETEA(0);					// debug: ea = r0
		UML_CALLH(block, *sh2->read32);			// call read32
		UML_MOV(block, R32(Rn), I0);		// mov Rn, r0

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  3: // MOV(Rm, Rn);
		UML_MOV(block, R32(Rn), R32(Rm));		// mov Rn, Rm
		return TRUE;

	case  7: // NOT(Rm, Rn);
		UML_XOR(block, R32(Rn), R32(Rm), 0xffffffff);	// xor Rn, Rm, 0xffffffff
		return TRUE;

	case  9: // SWAPW(Rm, Rn);
		UML_ROL(block, R32(Rn), R32(Rm), 16);	// rol Rn, Rm, 16
		return TRUE;

	case 11: // NEG(Rm, Rn);
		UML_SUB(block, R32(Rn), 0, R32(Rm));	// sub Rn, 0, Rm
		return TRUE;

	case 12: // EXTUB(Rm, Rn);
		UML_AND(block, R32(Rn), R32(Rm), 0x000000ff);	// and Rn, Rm, 0xff
		return TRUE;

	case 13: // EXTUW(Rm, Rn);
		UML_AND(block, R32(Rn), R32(Rm), 0x0000ffff);	// and Rn, Rm, 0xffff
		return TRUE;

	case 14: // EXTSB(Rm, Rn);
		UML_SEXT(block, R32(Rn), R32(Rm), SIZE_BYTE);		// sext Rn, Rm, BYTE
		return TRUE;

	case 15: // EXTSW(Rm, Rn);
		UML_SEXT(block, R32(Rn), R32(Rm), SIZE_WORD);		// sext Rn, Rm, WORD
		return TRUE;

	case  4: // MOVBP(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));		// mov r0, Rm
		UML_CALLH(block, *sh2->read8);			// call read8
		UML_SEXT(block, R32(Rn), I0, SIZE_BYTE);    	// sext Rn, r0, BYTE

		if (Rm != Rn)
			UML_ADD(block, R32(Rm), R32(Rm), 1);	// add Rm, Rm, #1

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  5: // MOVWP(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));		// mov r0, Rm
		UML_CALLH(block, *sh2->read16);			// call read16
		UML_SEXT(block, R32(Rn), I0, SIZE_WORD);    	// sext Rn, r0, WORD

		if (Rm != Rn)
			UML_ADD(block, R32(Rm), R32(Rm), 2);	// add Rm, Rm, #2

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  6: // MOVLP(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));		// mov r0, Rm
		UML_CALLH(block, *sh2->read32);			// call read32
		UML_MOV(block, R32(Rn), I0);		// mov Rn, r0

		if (Rm != Rn)
			UML_ADD(block, R32(Rm), R32(Rm), 4);	// add Rm, Rm, #4

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  8: // SWAPB(Rm, Rn);
		UML_AND(block, I0, R32(Rm), 0xffff0000);	// and r0, Rm, #0xffff0000
		UML_AND(block, I1, R32(Rm), 0x000000ff);	// and r0, Rm, #0x000000ff
		UML_AND(block, I2, R32(Rm), 0x0000ff00);	// and r0, Rm, #0x0000ff00
		UML_SHL(block, I1, I1, 8);		// shl r1, r1, #8
		UML_SHR(block, I2, I2, 8);		// shr r2, r2, #8
		UML_OR(block, I0, I0, I1);		// or r0, r0, r1
		UML_OR(block, R32(Rn), I0, I2);		// or Rn, r0, r2
		return TRUE;

	case 10: // NEGC(Rm, Rn);
		UML_MOV(block, I0, mem(&sh2->sr));		// mov r0, sr (save SR)
		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T (clear the T bit)
		UML_CARRY(block, I0, 0);	// carry = T (T is bit 0 of SR)
		UML_SUBB(block, R32(Rn), 0, R32(Rm));	// subb Rn, #0, Rm

		UML_JMPc(block, COND_NC, compiler->labelnum);	// jnc labelnum

		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), T);	// or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);		// labelnum:

		return TRUE;
	}

	return FALSE;
}

static int generate_group_8(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc)
{
	INT32 disp;
	UINT32 udisp;
	code_label templabel;

	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: // MOVBS4(opcode & 0x0f, Rm);
		udisp = (opcode & 0x0f);
		UML_ADD(block, I0, R32(Rm), udisp);		// add r0, Rm, udisp
		UML_MOV(block, I1, R32(0));			// mov r1, R0
		UML_CALLH(block, *sh2->write8);				// call write8

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  1 << 8: // MOVWS4(opcode & 0x0f, Rm);
		udisp = (opcode & 0x0f) * 2;
		UML_ADD(block, I0, R32(Rm), udisp);		// add r0, Rm, udisp
		UML_MOV(block, I1, R32(0));			// mov r1, R0
		UML_CALLH(block, *sh2->write16);				// call write16

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  2<< 8: // NOP();
	case  3<< 8: // NOP();
	case  6<< 8: // NOP();
	case  7<< 8: // NOP();
	case 10<< 8: // NOP();
	case 12<< 8: // NOP();
	case 14<< 8: // NOP();
		return TRUE;

	case  4<< 8: // MOVBL4(Rm, opcode & 0x0f);
		udisp = opcode & 0x0f;
		UML_ADD(block, I0, R32(Rm), udisp);		// add r0, Rm, udisp
		SETEA(0);
		UML_CALLH(block, *sh2->read8);				// call read8
		UML_SEXT(block, R32(0), I0, SIZE_BYTE);			// sext R0, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  5<< 8: // MOVWL4(Rm, opcode & 0x0f);
		udisp = (opcode & 0x0f)*2;
		UML_ADD(block, I0, R32(Rm), udisp);		// add r0, Rm, udisp
		SETEA(0);
		UML_CALLH(block, *sh2->read16);				// call read16
		UML_SEXT(block, R32(0), I0, SIZE_WORD);			// sext R0, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  8<< 8: // CMPIM(opcode & 0xff);
		UML_AND(block, I0, mem(&sh2->sr), ~T);	// and r0, sr, ~T (clear the T bit)

		UML_SEXT(block, I1, opcode&0xff, SIZE_BYTE);	// sext r1, opcode&0xff, BYTE
		UML_CMP(block, I1, R32(0));			// cmp r1, R0
		UML_JMPc(block, COND_NZ, compiler->labelnum);	// jnz compiler->labelnum   (if negative)

		UML_OR(block, I0, I0, T);	// or r0, r0, T

		UML_LABEL(block, compiler->labelnum++);	    	// labelnum:
		UML_MOV(block, mem(&sh2->sr), I0);		// mov sh2->sr, r0
		return TRUE;

	case  9<< 8: // BT(opcode & 0xff);
		UML_TEST(block, mem(&sh2->sr), T);		// test sh2->sr, T
		UML_JMPc(block, COND_Z, compiler->labelnum);	// jz compiler->labelnum

		disp = ((INT32)opcode << 24) >> 24;
		sh2->ea = (desc->pc + 2) + disp * 2 + 2;	// sh2->ea = destination

		generate_update_cycles(sh2, block, compiler, sh2->ea, TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, sh2->ea, *sh2->nocode);	// jmp sh2->ea

		UML_LABEL(block, compiler->labelnum++);	    	// labelnum:
		return TRUE;

	case 11<< 8: // BF(opcode & 0xff);
		UML_TEST(block, mem(&sh2->sr), T);		// test sh2->sr, T
		UML_JMPc(block, COND_NZ, compiler->labelnum);	// jnz compiler->labelnum

		disp = ((INT32)opcode << 24) >> 24;
		sh2->ea = (desc->pc + 2) + disp * 2 + 2;    	// sh2->ea = destination

		generate_update_cycles(sh2, block, compiler, sh2->ea, TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, sh2->ea, *sh2->nocode);	// jmp sh2->ea

		UML_LABEL(block, compiler->labelnum++);	    	// labelnum:
		return TRUE;

	case 13<< 8: // BTS(opcode & 0xff);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_TEST(block, mem(&sh2->sr), T);		// test sh2->sr, T
			UML_JMPc(block, COND_Z, compiler->labelnum);	// jz compiler->labelnum

			disp = ((INT32)opcode << 24) >> 24;
			sh2->ea = (desc->pc + 2) + disp * 2 + 2;    	// sh2->ea = destination

			templabel = compiler->labelnum;			// save our label
			compiler->labelnum++;				// make sure the delay slot doesn't use it
			generate_delay_slot(sh2, block, compiler, desc, sh2->ea-2);

			generate_update_cycles(sh2, block, compiler, sh2->ea, TRUE);	// <subtract cycles>
			UML_HASHJMP(block, 0, sh2->ea, *sh2->nocode);	// jmp sh2->ea

			UML_LABEL(block, templabel);	    	// labelnum:
			return TRUE;
		}
		break;

	case 15<< 8: // BFS(opcode & 0xff);
		if (sh2->cpu_type > CPU_TYPE_SH1)
		{
			UML_TEST(block, mem(&sh2->sr), T);		// test sh2->sr, T
			UML_JMPc(block, COND_NZ, compiler->labelnum);	// jnz compiler->labelnum

			disp = ((INT32)opcode << 24) >> 24;
			sh2->ea = (desc->pc + 2) + disp * 2 + 2;    	// sh2->ea = destination

			templabel = compiler->labelnum;			// save our label
			compiler->labelnum++;				// make sure the delay slot doesn't use it
			generate_delay_slot(sh2, block, compiler, desc, sh2->ea-2);	// delay slot only if the branch is taken

			generate_update_cycles(sh2, block, compiler, sh2->ea, TRUE);	// <subtract cycles>
			UML_HASHJMP(block, 0, sh2->ea, *sh2->nocode);	// jmp sh2->ea

			UML_LABEL(block, templabel);	    	// labelnum:
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static int generate_group_12(sh2_state *sh2, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc)
{
	UINT32 scratch;

	switch (opcode & (15<<8))
	{
	case  0<<8: // MOVBSG(opcode & 0xff);
		scratch = (opcode & 0xff);
		UML_ADD(block, I0, mem(&sh2->gbr), scratch);	// add r0, gbr, scratch
		UML_AND(block, I1, R32(0), 0xff);		// and r1, R0, 0xff
		UML_CALLH(block, *sh2->write8);				// call write8

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  1<<8: // MOVWSG(opcode & 0xff);
		scratch = (opcode & 0xff) * 2;
		UML_ADD(block, I0, mem(&sh2->gbr), scratch);	// add r0, gbr, scratch
		UML_AND(block, I1, R32(0), 0xffff);		// and r1, R0, 0xffff
		UML_CALLH(block, *sh2->write16);				// call write16

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  2<<8: // MOVLSG(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		UML_ADD(block, I0, mem(&sh2->gbr), scratch);	// add r0, gbr, scratch
		UML_MOV(block, I1, R32(0));			// mov r1, R0
		UML_CALLH(block, *sh2->write32);				// call write32

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  3<<8: // TRAPA(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		UML_ADD(block, mem(&sh2->ea), mem(&sh2->vbr), scratch);	// add ea, vbr, scratch

		UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
		UML_MOV(block, I0, R32(15));				// mov r0, R15
		UML_MOV(block, I1, mem(&sh2->sr));				// mov r1, sr
		UML_CALLH(block, *sh2->write32);					// write32

		UML_SUB(block, R32(15), R32(15), 4);			// sub R15, R15, #4
		UML_MOV(block, I0, R32(15));				// mov r0, R15
		UML_MOV(block, I1, desc->pc+2); 			// mov r1, pc+2
		UML_CALLH(block, *sh2->write32);					// write32

		UML_MOV(block, I0, mem(&sh2->ea));				// mov r0, ea
		UML_CALLH(block, *sh2->read32);					// read32
		UML_HASHJMP(block, 0, I0, *sh2->nocode);		// jmp (r0)

		return TRUE;

	case  4<<8: // MOVBLG(opcode & 0xff);
		scratch = (opcode & 0xff);
		UML_ADD(block, I0, mem(&sh2->gbr), scratch);	// add r0, gbr, scratch
		UML_CALLH(block, *sh2->read8);				// call read16
		UML_SEXT(block, R32(0), I0, SIZE_BYTE);			// sext R0, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  5<<8: // MOVWLG(opcode & 0xff);
		scratch = (opcode & 0xff) * 2;
		UML_ADD(block, I0, mem(&sh2->gbr), scratch);	// add r0, gbr, scratch
		UML_CALLH(block, *sh2->read16);				// call read16
		UML_SEXT(block, R32(0), I0, SIZE_WORD);			// sext R0, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  6<<8: // MOVLLG(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		UML_ADD(block, I0, mem(&sh2->gbr), scratch);	// add r0, gbr, scratch
		UML_CALLH(block, *sh2->read32);				// call read32
		UML_MOV(block, R32(0), I0);			// mov R0, r0

		if (!in_delay_slot)
			generate_update_cycles(sh2, block, compiler, desc->pc + 2, TRUE);
		return TRUE;

	case  7<<8: // MOVA(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		scratch += ((desc->pc + 4) & ~3);

		UML_MOV(block, R32(0), scratch);			// mov R0, scratch
		return TRUE;

	case  8<<8: // TSTI(opcode & 0xff);
		scratch = opcode & 0xff;

		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T (clear the T bit)
		UML_AND(block, I0, R32(0), scratch);		// and r0, R0, scratch
		UML_CMP(block, I0, 0);			// cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum);		// jnz labelnum

		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), T);	// or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);			// labelnum:
		return TRUE;

	case  9<<8: // ANDI(opcode & 0xff);
		UML_AND(block, R32(0), R32(0), opcode & 0xff);	// and r0, r0, opcode & 0xff
		return TRUE;

	case 10<<8: // XORI(opcode & 0xff);
		UML_XOR(block, R32(0), R32(0), opcode & 0xff);	// xor r0, r0, opcode & 0xff
		return TRUE;

	case 11<<8: // ORI(opcode & 0xff);
		UML_OR(block, R32(0), R32(0), opcode & 0xff);	// or r0, r0, opcode & 0xff
		return TRUE;

	case 12<<8: // TSTM(opcode & 0xff);
		UML_AND(block, mem(&sh2->sr), mem(&sh2->sr), ~T);	// and sr, sr, ~T (clear the T bit)
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		UML_CALLH(block, *sh2->read8);				// read8

		UML_AND(block, I0, I0, opcode & 0xff);
		UML_CMP(block, I0, 0);			// cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum);		// jnz labelnum

		UML_OR(block, mem(&sh2->sr), mem(&sh2->sr), T);	// or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);			// labelnum:
		return TRUE;

	case 13<<8: // ANDM(opcode & 0xff);
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		UML_CALLH(block, *sh2->read8);				// read8

		UML_AND(block, I1, I0, opcode&0xff);	// and r1, r0, #opcode&0xff
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		SETEA(0);
		UML_CALLH(block, *sh2->write8);				// write8
		return TRUE;

	case 14<<8: // XORM(opcode & 0xff);
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		UML_CALLH(block, *sh2->read8);				// read8

		UML_XOR(block, I1, I0, opcode&0xff);	// xor r1, r0, #opcode&0xff
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		SETEA(0);
		UML_CALLH(block, *sh2->write8);				// write8
		return TRUE;

	case 15<<8: // ORM(opcode & 0xff);
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		UML_CALLH(block, *sh2->read8);				// read8

		UML_OR(block, I1, I0, opcode&0xff);	// or r1, r0, #opcode&0xff
		UML_ADD(block, I0, R32(0), mem(&sh2->gbr));	// add r0, R0, gbr
		SETEA(0);
		UML_CALLH(block, *sh2->write8);				// write8
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    sh2drc_set_options - configure DRC options
-------------------------------------------------*/

void sh2drc_set_options(device_t *device, UINT32 options)
{
	sh2_state *sh2 = get_safe_token(device);
	sh2->drcoptions = options;
}


/*-------------------------------------------------
    sh2drc_add_pcflush - add a new address where
    the PC must be flushed for speedups to work
-------------------------------------------------*/

void sh2drc_add_pcflush(device_t *device, offs_t address)
{
	sh2_state *sh2 = get_safe_token(device);

	if (sh2->pcfsel < ARRAY_LENGTH(sh2->pcflushes))
		sh2->pcflushes[sh2->pcfsel++] = address;
}


/*-------------------------------------------------
    sh2_internal_a5 - read handler for
    SH2 internal map
-------------------------------------------------*/

static READ32_HANDLER(sh2_internal_a5)
{
	return 0xa5a5a5a5;
}


/*-------------------------------------------------
    sh2_internal_map - maps SH2 built-ins
-------------------------------------------------*/

static ADDRESS_MAP_START( sh2_internal_map, AS_PROGRAM, 32, legacy_cpu_device )
	AM_RANGE(0x40000000, 0xbfffffff) AM_READ_LEGACY(sh2_internal_a5)
	AM_RANGE(0xe0000000, 0xffffffff) AM_READWRITE_LEGACY(sh2_internal_r, sh2_internal_w)
ADDRESS_MAP_END

/*-------------------------------------------------
    sh2_set_info - set information about a given
    CPU instance
-------------------------------------------------*/

static CPU_SET_INFO( sh2 )
{
	sh2_state *sh2 = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLIN:	sh2_set_irq_line(sh2, SH2_INT_VBLIN, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLOUT:	sh2_set_irq_line(sh2, SH2_INT_VBLOUT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_HBLIN:	sh2_set_irq_line(sh2, SH2_INT_HBLIN, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER0:	sh2_set_irq_line(sh2, SH2_INT_TIMER0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER1:	sh2_set_irq_line(sh2, SH2_INT_TIMER1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DSP:		sh2_set_irq_line(sh2, SH2_INT_DSP, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SOUND:	sh2_set_irq_line(sh2, SH2_INT_SOUND, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SMPC:	sh2_set_irq_line(sh2, SH2_INT_SMPC, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_PAD:		sh2_set_irq_line(sh2, SH2_INT_PAD, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA2:	sh2_set_irq_line(sh2, SH2_INT_DMA2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA1:	sh2_set_irq_line(sh2, SH2_INT_DMA1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA0:	sh2_set_irq_line(sh2, SH2_INT_DMA0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMAILL:	sh2_set_irq_line(sh2, SH2_INT_DMAILL, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SPRITE:	sh2_set_irq_line(sh2, SH2_INT_SPRITE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_14:		sh2_set_irq_line(sh2, SH2_INT_14, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_15:		sh2_set_irq_line(sh2, SH2_INT_15, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_ABUS:	sh2_set_irq_line(sh2, SH2_INT_ABUS, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	sh2_set_irq_line(sh2, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_REGISTER + SH2_PC:
		case CPUINFO_INT_PC:							sh2->pc = info->i; sh2->delay = 0;		break;
		case CPUINFO_INT_SP:							sh2->r[15] = info->i;   				break;
		case CPUINFO_INT_REGISTER + SH2_PR: 			sh2->pr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_SR:				sh2->sr = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_GBR:			sh2->gbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_VBR:			sh2->vbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_MACH:			sh2->mach = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_MACL:			sh2->macl = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_R0:				sh2->r[ 0] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R1:				sh2->r[ 1] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R2:				sh2->r[ 2] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R3:				sh2->r[ 3] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R4:				sh2->r[ 4] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R5:				sh2->r[ 5] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R6:				sh2->r[ 6] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R7:				sh2->r[ 7] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R8:				sh2->r[ 8] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R9:				sh2->r[ 9] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R10:			sh2->r[10] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R11:			sh2->r[11] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R12:			sh2->r[12] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R13:			sh2->r[13] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R14:			sh2->r[14] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R15:			sh2->r[15] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_EA:				sh2->ea = info->i;						break;
	}
}

/*-------------------------------------------------
    sh2_get_info - return information about a
    given CPU instance
-------------------------------------------------*/

CPU_GET_INFO( sh2 )
{
	sh2_state *sh2 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(sh2_state *);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 16;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		// Internal maps
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(sh2_internal_map); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map32 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map32 = NULL;	break;

		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLIN:	info->i = sh2->irq_line_state[SH2_INT_VBLIN]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLOUT:	info->i = sh2->irq_line_state[SH2_INT_VBLOUT]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_HBLIN:	info->i = sh2->irq_line_state[SH2_INT_HBLIN]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER0:	info->i = sh2->irq_line_state[SH2_INT_TIMER0]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER1:	info->i = sh2->irq_line_state[SH2_INT_TIMER1]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DSP:		info->i = sh2->irq_line_state[SH2_INT_DSP]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SOUND:	info->i = sh2->irq_line_state[SH2_INT_SOUND]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SMPC:	info->i = sh2->irq_line_state[SH2_INT_SMPC];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_PAD:		info->i = sh2->irq_line_state[SH2_INT_PAD]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA2:	info->i = sh2->irq_line_state[SH2_INT_DMA2];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA1:	info->i = sh2->irq_line_state[SH2_INT_DMA1];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA0:	info->i = sh2->irq_line_state[SH2_INT_DMA0];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMAILL:	info->i = sh2->irq_line_state[SH2_INT_DMAILL]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SPRITE:	info->i = sh2->irq_line_state[SH2_INT_SPRITE]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_14:		info->i = sh2->irq_line_state[SH2_INT_14]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_15:		info->i = sh2->irq_line_state[SH2_INT_15]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_ABUS:	info->i = sh2->irq_line_state[SH2_INT_ABUS];	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = sh2->nmi_line_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = sh2->ppc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SH2_PC:				info->i = (sh2->delay) ? (sh2->delay & AM) : (sh2->pc & AM); break;
		case CPUINFO_INT_SP:							info->i = sh2->r[15];					break;
		case CPUINFO_INT_REGISTER + SH2_PR:				info->i = sh2->pr;						break;
		case CPUINFO_INT_REGISTER + SH2_SR:				info->i = sh2->sr;						break;
		case CPUINFO_INT_REGISTER + SH2_GBR:			info->i = sh2->gbr;						break;
		case CPUINFO_INT_REGISTER + SH2_VBR:			info->i = sh2->vbr;						break;
		case CPUINFO_INT_REGISTER + SH2_MACH:			info->i = sh2->mach;						break;
		case CPUINFO_INT_REGISTER + SH2_MACL:			info->i = sh2->macl;						break;
		case CPUINFO_INT_REGISTER + SH2_R0:				info->i = sh2->r[ 0];					break;
		case CPUINFO_INT_REGISTER + SH2_R1:				info->i = sh2->r[ 1];					break;
		case CPUINFO_INT_REGISTER + SH2_R2:				info->i = sh2->r[ 2];					break;
		case CPUINFO_INT_REGISTER + SH2_R3:				info->i = sh2->r[ 3];					break;
		case CPUINFO_INT_REGISTER + SH2_R4:				info->i = sh2->r[ 4];					break;
		case CPUINFO_INT_REGISTER + SH2_R5:				info->i = sh2->r[ 5];					break;
		case CPUINFO_INT_REGISTER + SH2_R6:				info->i = sh2->r[ 6];					break;
		case CPUINFO_INT_REGISTER + SH2_R7:				info->i = sh2->r[ 7];					break;
		case CPUINFO_INT_REGISTER + SH2_R8:				info->i = sh2->r[ 8];					break;
		case CPUINFO_INT_REGISTER + SH2_R9:				info->i = sh2->r[ 9];					break;
		case CPUINFO_INT_REGISTER + SH2_R10:			info->i = sh2->r[10];					break;
		case CPUINFO_INT_REGISTER + SH2_R11:			info->i = sh2->r[11];					break;
		case CPUINFO_INT_REGISTER + SH2_R12:			info->i = sh2->r[12];					break;
		case CPUINFO_INT_REGISTER + SH2_R13:			info->i = sh2->r[13];					break;
		case CPUINFO_INT_REGISTER + SH2_R14:			info->i = sh2->r[14];					break;
		case CPUINFO_INT_REGISTER + SH2_R15:			info->i = sh2->r[15];					break;
		case CPUINFO_INT_REGISTER + SH2_EA:				info->i = sh2->ea;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(sh2);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(sh2);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(sh2);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(sh2);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(sh2);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(sh2);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &sh2->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "SH-2");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Hitachi SuperH RISC");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "2.0");				break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME team, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%d%c%c",
					sh2->sr & M ? 'M':'.',
					sh2->sr & Q ? 'Q':'.',
					(sh2->sr & I) >> 4,
					sh2->sr & S ? 'S':'.',
					sh2->sr & T ? 'T':'.');
			break;

		case CPUINFO_STR_REGISTER + SH2_PC:				sprintf(info->s, "PC  :%08X", sh2->pc); break;
		case CPUINFO_STR_REGISTER + SH2_SR:				sprintf(info->s, "SR  :%08X", sh2->sr); break;
		case CPUINFO_STR_REGISTER + SH2_PR:				sprintf(info->s, "PR  :%08X", sh2->pr); break;
		case CPUINFO_STR_REGISTER + SH2_GBR:			sprintf(info->s, "GBR :%08X", sh2->gbr); break;
		case CPUINFO_STR_REGISTER + SH2_VBR:			sprintf(info->s, "VBR :%08X", sh2->vbr); break;
		case CPUINFO_STR_REGISTER + SH2_MACH:			sprintf(info->s, "MACH:%08X", sh2->mach); break;
		case CPUINFO_STR_REGISTER + SH2_MACL:			sprintf(info->s, "MACL:%08X", sh2->macl); break;
		case CPUINFO_STR_REGISTER + SH2_R0:				sprintf(info->s, "R0  :%08X", sh2->r[ 0]); break;
		case CPUINFO_STR_REGISTER + SH2_R1:				sprintf(info->s, "R1  :%08X", sh2->r[ 1]); break;
		case CPUINFO_STR_REGISTER + SH2_R2:				sprintf(info->s, "R2  :%08X", sh2->r[ 2]); break;
		case CPUINFO_STR_REGISTER + SH2_R3:				sprintf(info->s, "R3  :%08X", sh2->r[ 3]); break;
		case CPUINFO_STR_REGISTER + SH2_R4:				sprintf(info->s, "R4  :%08X", sh2->r[ 4]); break;
		case CPUINFO_STR_REGISTER + SH2_R5:				sprintf(info->s, "R5  :%08X", sh2->r[ 5]); break;
		case CPUINFO_STR_REGISTER + SH2_R6:				sprintf(info->s, "R6  :%08X", sh2->r[ 6]); break;
		case CPUINFO_STR_REGISTER + SH2_R7:				sprintf(info->s, "R7  :%08X", sh2->r[ 7]); break;
		case CPUINFO_STR_REGISTER + SH2_R8:				sprintf(info->s, "R8  :%08X", sh2->r[ 8]); break;
		case CPUINFO_STR_REGISTER + SH2_R9:				sprintf(info->s, "R9  :%08X", sh2->r[ 9]); break;
		case CPUINFO_STR_REGISTER + SH2_R10:			sprintf(info->s, "R10 :%08X", sh2->r[10]); break;
		case CPUINFO_STR_REGISTER + SH2_R11:			sprintf(info->s, "R11 :%08X", sh2->r[11]); break;
		case CPUINFO_STR_REGISTER + SH2_R12:			sprintf(info->s, "R12 :%08X", sh2->r[12]); break;
		case CPUINFO_STR_REGISTER + SH2_R13:			sprintf(info->s, "R13 :%08X", sh2->r[13]); break;
		case CPUINFO_STR_REGISTER + SH2_R14:			sprintf(info->s, "R14 :%08X", sh2->r[14]); break;
		case CPUINFO_STR_REGISTER + SH2_R15:			sprintf(info->s, "R15 :%08X", sh2->r[15]); break;
		case CPUINFO_STR_REGISTER + SH2_EA:				sprintf(info->s, "EA  :%08X", sh2->ea);    break;
	}
}

/*-------------------------------------------------
    sh1_get_info - return information about a
    given CPU instance
-------------------------------------------------*/

CPU_GET_INFO( sh1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:						info->reset = CPU_RESET_NAME(sh1);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:	    					strcpy(info->s, "SH-1");				break;

		default:							CPU_GET_INFO_CALL(sh2);			break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(SH1, sh1);
DEFINE_LEGACY_CPU_DEVICE(SH2, sh2);

#endif	// USE_SH2DRC
