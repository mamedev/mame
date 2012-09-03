/***************************************************************************

    jaguar.c
    Core implementation for the portable Jaguar DSP emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "jaguar.h"

CPU_DISASSEMBLE( jaguargpu );
CPU_DISASSEMBLE( jaguardsp );

#define LOG_GPU_IO		0
#define LOG_DSP_IO		0


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ZFLAG					0x00001
#define CFLAG					0x00002
#define NFLAG					0x00004
#define IFLAG					0x00008
#define EINT0FLAG				0x00010
#define EINT1FLAG				0x00020
#define EINT2FLAG				0x00040
#define EINT3FLAG				0x00080
#define EINT4FLAG				0x00100
#define EINT04FLAGS				(EINT0FLAG | EINT1FLAG | EINT2FLAG | EINT3FLAG | EINT4FLAG)
#define CINT0FLAG				0x00200
#define CINT1FLAG				0x00400
#define CINT2FLAG				0x00800
#define CINT3FLAG				0x01000
#define CINT4FLAG				0x02000
#define CINT04FLAGS				(CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG)
#define RPAGEFLAG				0x04000
#define DMAFLAG					0x08000
#define EINT5FLAG				0x10000		/* DSP only */
#define CINT5FLAG				0x20000		/* DSP only */

#define CLR_Z(J)				((J)->FLAGS &= ~ZFLAG)
#define CLR_ZN(J)				((J)->FLAGS &= ~(ZFLAG | NFLAG))
#define CLR_ZNC(J)				((J)->FLAGS &= ~(CFLAG | ZFLAG | NFLAG))
#define SET_Z(J,r)				((J)->FLAGS |= ((r) == 0))
#define SET_C_ADD(J,a,b)		((J)->FLAGS |= ((UINT32)(b) > (UINT32)(~(a))) << 1)
#define SET_C_SUB(J,a,b)		((J)->FLAGS |= ((UINT32)(b) > (UINT32)(a)) << 1)
#define SET_N(J,r)				((J)->FLAGS |= (((UINT32)(r) >> 29) & 4))
#define SET_ZN(J,r)				SET_N(J,r); SET_Z(J,r)
#define SET_ZNC_ADD(J,a,b,r)	SET_N(J,r); SET_Z(J,r); SET_C_ADD(J,a,b)
#define SET_ZNC_SUB(J,a,b,r)	SET_N(J,r); SET_Z(J,r); SET_C_SUB(J,a,b)



/***************************************************************************
    MACROS
***************************************************************************/

#define PC					ctrl[G_PC]
#define FLAGS				ctrl[G_FLAGS]

#define CONDITION(x)		condition_table[(x) + ((jaguar->FLAGS & 7) << 5)]

#define READBYTE(J,a)		(J)->program->read_byte(a)
#define READWORD(J,a)		(J)->program->read_word(a)
#define READLONG(J,a)		(J)->program->read_dword(a)

#define WRITEBYTE(J,a,v)	(J)->program->write_byte(a, v)
#define WRITEWORD(J,a,v)	(J)->program->write_word(a, v)
#define WRITELONG(J,a,v)	(J)->program->write_dword(a, v)



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* Jaguar Registers */
typedef struct _jaguar_state jaguar_state;
struct _jaguar_state
{
	/* core registers */
	UINT32		r[32];
	UINT32		a[32];
	UINT32 *	b0;
	UINT32 *	b1;

	/* control registers */
	UINT32		ctrl[G_CTRLMAX];
	UINT32		ppc;
	UINT64		accum;

	/* internal stuff */
	int			isdsp;
	int			icount;
	int			bankswitch_icount;
	void		(*const *table)(jaguar_state *jaguar, UINT16 op);
	device_irq_acknowledge_callback irq_callback;
	jaguar_int_func cpu_interrupt;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
};



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static UINT32		table_refcount;
static UINT16 *		mirror_table;
static UINT8 *		condition_table;

static const UINT32 convert_zero[32] =
{ 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };



/***************************************************************************
    FUNCTION TABLES
***************************************************************************/

static void abs_rn(jaguar_state *jaguar, UINT16 op);
static void add_rn_rn(jaguar_state *jaguar, UINT16 op);
static void addc_rn_rn(jaguar_state *jaguar, UINT16 op);
static void addq_n_rn(jaguar_state *jaguar, UINT16 op);
static void addqmod_n_rn(jaguar_state *jaguar, UINT16 op);	/* DSP only */
static void addqt_n_rn(jaguar_state *jaguar, UINT16 op);
static void and_rn_rn(jaguar_state *jaguar, UINT16 op);
static void bclr_n_rn(jaguar_state *jaguar, UINT16 op);
static void bset_n_rn(jaguar_state *jaguar, UINT16 op);
static void btst_n_rn(jaguar_state *jaguar, UINT16 op);
static void cmp_rn_rn(jaguar_state *jaguar, UINT16 op);
static void cmpq_n_rn(jaguar_state *jaguar, UINT16 op);
static void div_rn_rn(jaguar_state *jaguar, UINT16 op);
static void illegal(jaguar_state *jaguar, UINT16 op);
static void imacn_rn_rn(jaguar_state *jaguar, UINT16 op);
static void imult_rn_rn(jaguar_state *jaguar, UINT16 op);
static void imultn_rn_rn(jaguar_state *jaguar, UINT16 op);
static void jr_cc_n(jaguar_state *jaguar, UINT16 op);
static void jump_cc_rn(jaguar_state *jaguar, UINT16 op);
static void load_rn_rn(jaguar_state *jaguar, UINT16 op);
static void load_r14n_rn(jaguar_state *jaguar, UINT16 op);
static void load_r15n_rn(jaguar_state *jaguar, UINT16 op);
static void load_r14rn_rn(jaguar_state *jaguar, UINT16 op);
static void load_r15rn_rn(jaguar_state *jaguar, UINT16 op);
static void loadb_rn_rn(jaguar_state *jaguar, UINT16 op);
static void loadw_rn_rn(jaguar_state *jaguar, UINT16 op);
static void loadp_rn_rn(jaguar_state *jaguar, UINT16 op);	/* GPU only */
static void mirror_rn(jaguar_state *jaguar, UINT16 op);	/* DSP only */
static void mmult_rn_rn(jaguar_state *jaguar, UINT16 op);
static void move_rn_rn(jaguar_state *jaguar, UINT16 op);
static void move_pc_rn(jaguar_state *jaguar, UINT16 op);
static void movefa_rn_rn(jaguar_state *jaguar, UINT16 op);
static void movei_n_rn(jaguar_state *jaguar, UINT16 op);
static void moveq_n_rn(jaguar_state *jaguar, UINT16 op);
static void moveta_rn_rn(jaguar_state *jaguar, UINT16 op);
static void mtoi_rn_rn(jaguar_state *jaguar, UINT16 op);
static void mult_rn_rn(jaguar_state *jaguar, UINT16 op);
static void neg_rn(jaguar_state *jaguar, UINT16 op);
static void nop(jaguar_state *jaguar, UINT16 op);
static void normi_rn_rn(jaguar_state *jaguar, UINT16 op);
static void not_rn(jaguar_state *jaguar, UINT16 op);
static void or_rn_rn(jaguar_state *jaguar, UINT16 op);
static void pack_rn(jaguar_state *jaguar, UINT16 op);		/* GPU only */
static void resmac_rn(jaguar_state *jaguar, UINT16 op);
static void ror_rn_rn(jaguar_state *jaguar, UINT16 op);
static void rorq_n_rn(jaguar_state *jaguar, UINT16 op);
static void sat8_rn(jaguar_state *jaguar, UINT16 op);		/* GPU only */
static void sat16_rn(jaguar_state *jaguar, UINT16 op);		/* GPU only */
static void sat16s_rn(jaguar_state *jaguar, UINT16 op);		/* DSP only */
static void sat24_rn(jaguar_state *jaguar, UINT16 op);			/* GPU only */
static void sat32s_rn(jaguar_state *jaguar, UINT16 op);		/* DSP only */
static void sh_rn_rn(jaguar_state *jaguar, UINT16 op);
static void sha_rn_rn(jaguar_state *jaguar, UINT16 op);
static void sharq_n_rn(jaguar_state *jaguar, UINT16 op);
static void shlq_n_rn(jaguar_state *jaguar, UINT16 op);
static void shrq_n_rn(jaguar_state *jaguar, UINT16 op);
static void store_rn_rn(jaguar_state *jaguar, UINT16 op);
static void store_rn_r14n(jaguar_state *jaguar, UINT16 op);
static void store_rn_r15n(jaguar_state *jaguar, UINT16 op);
static void store_rn_r14rn(jaguar_state *jaguar, UINT16 op);
static void store_rn_r15rn(jaguar_state *jaguar, UINT16 op);
static void storeb_rn_rn(jaguar_state *jaguar, UINT16 op);
static void storew_rn_rn(jaguar_state *jaguar, UINT16 op);
static void storep_rn_rn(jaguar_state *jaguar, UINT16 op);	/* GPU only */
static void sub_rn_rn(jaguar_state *jaguar, UINT16 op);
static void subc_rn_rn(jaguar_state *jaguar, UINT16 op);
static void subq_n_rn(jaguar_state *jaguar, UINT16 op);
static void subqmod_n_rn(jaguar_state *jaguar, UINT16 op);	/* DSP only */
static void subqt_n_rn(jaguar_state *jaguar, UINT16 op);
static void xor_rn_rn(jaguar_state *jaguar, UINT16 op);

static void (*const gpu_op_table[64])(jaguar_state *jaguar, UINT16 op) =
{
	/* 00-03 */	add_rn_rn,		addc_rn_rn,		addq_n_rn,		addqt_n_rn,
	/* 04-07 */	sub_rn_rn,		subc_rn_rn,		subq_n_rn,		subqt_n_rn,
	/* 08-11 */	neg_rn,			and_rn_rn,		or_rn_rn,		xor_rn_rn,
	/* 12-15 */	not_rn,			btst_n_rn,		bset_n_rn,		bclr_n_rn,
	/* 16-19 */	mult_rn_rn,		imult_rn_rn,	imultn_rn_rn,	resmac_rn,
	/* 20-23 */	imacn_rn_rn,	div_rn_rn,		abs_rn,			sh_rn_rn,
	/* 24-27 */	shlq_n_rn,		shrq_n_rn,		sha_rn_rn,		sharq_n_rn,
	/* 28-31 */	ror_rn_rn,		rorq_n_rn,		cmp_rn_rn,		cmpq_n_rn,
	/* 32-35 */	sat8_rn,		sat16_rn,		move_rn_rn,		moveq_n_rn,
	/* 36-39 */	moveta_rn_rn,	movefa_rn_rn,	movei_n_rn,		loadb_rn_rn,
	/* 40-43 */	loadw_rn_rn,	load_rn_rn,		loadp_rn_rn,	load_r14n_rn,
	/* 44-47 */	load_r15n_rn,	storeb_rn_rn,	storew_rn_rn,	store_rn_rn,
	/* 48-51 */	storep_rn_rn,	store_rn_r14n,	store_rn_r15n,	move_pc_rn,
	/* 52-55 */	jump_cc_rn,		jr_cc_n,		mmult_rn_rn,	mtoi_rn_rn,
	/* 56-59 */	normi_rn_rn,	nop,			load_r14rn_rn,	load_r15rn_rn,
	/* 60-63 */	store_rn_r14rn,	store_rn_r15rn,	sat24_rn,		pack_rn
};

static void (*const dsp_op_table[64])(jaguar_state *jaguar, UINT16 op) =
{
	/* 00-03 */	add_rn_rn,		addc_rn_rn,		addq_n_rn,		addqt_n_rn,
	/* 04-07 */	sub_rn_rn,		subc_rn_rn,		subq_n_rn,		subqt_n_rn,
	/* 08-11 */	neg_rn,			and_rn_rn,		or_rn_rn,		xor_rn_rn,
	/* 12-15 */	not_rn,			btst_n_rn,		bset_n_rn,		bclr_n_rn,
	/* 16-19 */	mult_rn_rn,		imult_rn_rn,	imultn_rn_rn,	resmac_rn,
	/* 20-23 */	imacn_rn_rn,	div_rn_rn,		abs_rn,			sh_rn_rn,
	/* 24-27 */	shlq_n_rn,		shrq_n_rn,		sha_rn_rn,		sharq_n_rn,
	/* 28-31 */	ror_rn_rn,		rorq_n_rn,		cmp_rn_rn,		cmpq_n_rn,
	/* 32-35 */	subqmod_n_rn,	sat16s_rn,		move_rn_rn,		moveq_n_rn,
	/* 36-39 */	moveta_rn_rn,	movefa_rn_rn,	movei_n_rn,		loadb_rn_rn,
	/* 40-43 */	loadw_rn_rn,	load_rn_rn,		sat32s_rn,		load_r14n_rn,
	/* 44-47 */	load_r15n_rn,	storeb_rn_rn,	storew_rn_rn,	store_rn_rn,
	/* 48-51 */	mirror_rn,		store_rn_r14n,	store_rn_r15n,	move_pc_rn,
	/* 52-55 */	jump_cc_rn,		jr_cc_n,		mmult_rn_rn,	mtoi_rn_rn,
	/* 56-59 */	normi_rn_rn,	nop,			load_r14rn_rn,	load_r15rn_rn,
	/* 60-63 */	store_rn_r14rn,	store_rn_r15rn,	illegal,		addqmod_n_rn
};



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(J,pc)		((J)->direct->read_decrypted_word(pc, WORD_XOR_BE(0)))



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE jaguar_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == JAGUARGPU ||
		   device->type() == JAGUARDSP);
	return (jaguar_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void update_register_banks(jaguar_state *jaguar)
{
	UINT32 temp;
	int i, bank;

	/* pick the bank */
	bank = jaguar->FLAGS & RPAGEFLAG;
	if (jaguar->FLAGS & IFLAG) bank = 0;

	/* do we need to swap? */
	if ((bank == 0 && jaguar->b0 != jaguar->r) || (bank != 0 && jaguar->b1 != jaguar->r))
	{
		/* remember the icount of the instruction after we swap */
		jaguar->bankswitch_icount = jaguar->icount - 1;

		/* exchange the contents */
		for (i = 0; i < 32; i++)
			temp = jaguar->r[i], jaguar->r[i] = jaguar->a[i], jaguar->a[i] = temp;

		/* swap the bank pointers */
		if (bank == 0)
		{
			jaguar->b0 = jaguar->r;
			jaguar->b1 = jaguar->a;
		}
		else
		{
			jaguar->b0 = jaguar->a;
			jaguar->b1 = jaguar->r;
		}
	}
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(jaguar_state *jaguar)
{
	int bits, mask, which = 0;

	/* if the IMASK is set, bail */
	if (jaguar->FLAGS & IFLAG)
		return;

	/* get the active interrupt bits */
	bits = (jaguar->ctrl[G_CTRL] >> 6) & 0x1f;
	bits |= (jaguar->ctrl[G_CTRL] >> 10) & 0x20;

	/* get the interrupt mask */
	mask = (jaguar->FLAGS >> 4) & 0x1f;
	mask |= (jaguar->FLAGS >> 11) & 0x20;

	/* bail if nothing is available */
	bits &= mask;
	if (bits == 0)
		return;

	/* determine which interrupt */
	if (bits & 0x01) which = 0;
	if (bits & 0x02) which = 1;
	if (bits & 0x04) which = 2;
	if (bits & 0x08) which = 3;
	if (bits & 0x10) which = 4;
	if (bits & 0x20) which = 5;

	/* set the interrupt flag */
	jaguar->FLAGS |= IFLAG;
	update_register_banks(jaguar);

	/* push the PC-2 on the stack */
	jaguar->r[31] -= 4;
	WRITELONG(jaguar, jaguar->r[31], jaguar->PC - 2);

	/* dispatch */
	jaguar->PC = (jaguar->isdsp) ? 0xf1b000 : 0xf03000;
	jaguar->PC += which * 0x10;
}


static void set_irq_line(jaguar_state *jaguar, int irqline, int state)
{
	int mask = (irqline < 5) ? (0x40 << irqline) : 0x10000;
	jaguar->ctrl[G_CTRL] &= ~mask;
	if (state != CLEAR_LINE)
	{
		jaguar->ctrl[G_CTRL] |= mask;
		check_irqs(jaguar);
	}
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void init_tables(void)
{
	int i, j;

	/* if we're not the first, skip */
	if (table_refcount++ != 0)
	{
		assert(mirror_table != NULL);
		assert(condition_table != NULL);
		return;
	}

	/* fill in the mirror table */
	mirror_table = global_alloc_array(UINT16, 65536);
	for (i = 0; i < 65536; i++)
		mirror_table[i] = ((i >> 15) & 0x0001) | ((i >> 13) & 0x0002) |
		                  ((i >> 11) & 0x0004) | ((i >> 9)  & 0x0008) |
		                  ((i >> 7)  & 0x0010) | ((i >> 5)  & 0x0020) |
		                  ((i >> 3)  & 0x0040) | ((i >> 1)  & 0x0080) |
		                  ((i << 1)  & 0x0100) | ((i << 3)  & 0x0200) |
		                  ((i << 5)  & 0x0400) | ((i << 7)  & 0x0800) |
		                  ((i << 9)  & 0x1000) | ((i << 11) & 0x2000) |
		                  ((i << 13) & 0x4000) | ((i << 15) & 0x8000);

	/* fill in the condition table */
	condition_table = global_alloc_array(UINT8, 32 * 8);
	for (i = 0; i < 8; i++)
		for (j = 0; j < 32; j++)
		{
			int result = 1;
			if (j & 1)
				if (i & ZFLAG) result = 0;
			if (j & 2)
				if (!(i & ZFLAG)) result = 0;
			if (j & 4)
				if (i & (CFLAG << (j >> 4))) result = 0;
			if (j & 8)
				if (!(i & (CFLAG << (j >> 4)))) result = 0;
			condition_table[i * 32 + j] = result;
		}
}


static void jaguar_postload(jaguar_state *jaguar)
{
	update_register_banks(jaguar);
	check_irqs(jaguar);
}


static void init_common(int isdsp, legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback)
{
	const jaguar_cpu_config *configdata = (const jaguar_cpu_config *)device->static_config();
	jaguar_state *jaguar = get_safe_token(device);

	init_tables();

	jaguar->table = isdsp ? dsp_op_table : gpu_op_table;
	jaguar->isdsp = isdsp;

	jaguar->irq_callback = irqcallback;
	jaguar->device = device;
	jaguar->program = device->space(AS_PROGRAM);
	jaguar->direct = &jaguar->program->direct();
	if (configdata != NULL)
		jaguar->cpu_interrupt = configdata->cpu_int_callback;

	device->save_item(NAME(jaguar->r));
	device->save_item(NAME(jaguar->a));
	device->save_item(NAME(jaguar->ctrl));
	device->save_item(NAME(jaguar->ppc));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(jaguar_postload), jaguar));
}


static CPU_INIT( jaguargpu )
{
	init_common(FALSE, device, irqcallback);
}


static CPU_INIT( jaguardsp )
{
	init_common(TRUE, device, irqcallback);
}


static CPU_RESET( jaguar )
{
	jaguar_state *jaguar = get_safe_token(device);

	jaguar->b0 = jaguar->r;
	jaguar->b1 = jaguar->a;
}


static CPU_EXIT( jaguar )
{
	if (--table_refcount != 0)
		return;

	if (mirror_table != NULL)
		global_free(mirror_table);
	mirror_table = NULL;

	if (condition_table != NULL)
		global_free(condition_table);
	condition_table = NULL;
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static CPU_EXECUTE( jaguargpu )
{
	jaguar_state *jaguar = get_safe_token(device);

	/* if we're halted, we shouldn't be here */
	if (!(jaguar->ctrl[G_CTRL] & 1))
	{
		//device_set_input_line(device, INPUT_LINE_HALT, ASSERT_LINE);
		jaguar->icount = 0;
		return;
	}

	/* check for IRQs */
	check_irqs(jaguar);

	/* count cycles and interrupt cycles */
	jaguar->bankswitch_icount = -1000;

	/* core execution loop */
	do
	{
		UINT32 op;

		/* debugging */
		//if (jaguar->PC < 0xf03000 || jaguar->PC > 0xf04000) { fatalerror("GPU: jaguar->PC = %06X (ppc = %06X)", jaguar->PC, jaguar->ppc); }
		jaguar->ppc = jaguar->PC;
		debugger_instruction_hook(device, jaguar->PC);

		/* instruction fetch */
		op = ROPCODE(jaguar, jaguar->PC);
		jaguar->PC += 2;

		/* parse the instruction */
		(*gpu_op_table[op >> 10])(jaguar, op);
		jaguar->icount--;

	} while (jaguar->icount > 0 || jaguar->icount == jaguar->bankswitch_icount);
}

static CPU_EXECUTE( jaguardsp )
{
	jaguar_state *jaguar = get_safe_token(device);

	/* if we're halted, we shouldn't be here */
	if (!(jaguar->ctrl[G_CTRL] & 1))
	{
		//device_set_input_line(device, INPUT_LINE_HALT, ASSERT_LINE);
		jaguar->icount = 0;
		return;
	}

	/* check for IRQs */
	check_irqs(jaguar);

	/* count cycles and interrupt cycles */
	jaguar->bankswitch_icount = -1000;

	/* core execution loop */
	do
	{
		UINT32 op;

		/* debugging */
		//if (jaguar->PC < 0xf1b000 || jaguar->PC > 0xf1d000) { fatalerror(stderr, "DSP: jaguar->PC = %06X", jaguar->PC); }
		jaguar->ppc = jaguar->PC;
		debugger_instruction_hook(device, jaguar->PC);

		/* instruction fetch */
		op = ROPCODE(jaguar, jaguar->PC);
		jaguar->PC += 2;

		/* parse the instruction */
		(*dsp_op_table[op >> 10])(jaguar, op);
		jaguar->icount--;

	} while (jaguar->icount > 0 || jaguar->icount == jaguar->bankswitch_icount);
}



/***************************************************************************
    OPCODES
***************************************************************************/

void abs_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 res = jaguar->r[dreg];
	CLR_ZNC(jaguar);
	if (res & 0x80000000)
	{
		jaguar->r[dreg] = res = -res;
		jaguar->FLAGS |= CFLAG;
	}
	SET_Z(jaguar, res);
}

void add_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 + r1;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_ADD(jaguar, r2, r1, res);
}

void addc_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 + r1 + ((jaguar->FLAGS >> 1) & 1);
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_ADD(jaguar, r2, r1, res);
}

void addq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 + r1;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_ADD(jaguar, r2, r1, res);
}

void addqmod_n_rn(jaguar_state *jaguar, UINT16 op)	/* DSP only */
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 + r1;
	res = (res & ~jaguar->ctrl[D_MOD]) | (r2 & ~jaguar->ctrl[D_MOD]);
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_ADD(jaguar, r2, r1, res);
}

void addqt_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 + r1;
	jaguar->r[dreg] = res;
}

void and_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 & r1;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void bclr_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = (op >> 5) & 31;
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 & ~(1 << r1);
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void bset_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = (op >> 5) & 31;
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 | (1 << r1);
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void btst_n_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = (op >> 5) & 31;
	UINT32 r2 = jaguar->r[op & 31];
	CLR_Z(jaguar); jaguar->FLAGS |= (~r2 >> r1) & 1;
}

void cmp_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[op & 31];
	UINT32 res = r2 - r1;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, r2, r1, res);
}

void cmpq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = (INT8)(op >> 2) >> 3;
	UINT32 r2 = jaguar->r[op & 31];
	UINT32 res = r2 - r1;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, r2, r1, res);
}

void div_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	if (r1)
	{
		if (jaguar->ctrl[D_DIVCTRL] & 1)
		{
			jaguar->r[dreg] = ((UINT64)r2 << 16) / r1;
			jaguar->ctrl[D_REMAINDER] = ((UINT64)r2 << 16) % r1;
		}
		else
		{
			jaguar->r[dreg] = r2 / r1;
			jaguar->ctrl[D_REMAINDER] = r2 % r1;
		}
	}
	else
		jaguar->r[dreg] = 0xffffffff;
}

void illegal(jaguar_state *jaguar, UINT16 op)
{
}

void imacn_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[op & 31];
	jaguar->accum += (INT64)((INT16)r1 * (INT16)r2);
	logerror("Unexpected IMACN instruction!\n");
}

void imult_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = (INT16)r1 * (INT16)r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void imultn_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = (INT16)r1 * (INT16)r2;
	jaguar->accum = (INT32)res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);

	op = ROPCODE(jaguar, jaguar->PC);
	while ((op >> 10) == 20)
	{
		r1 = jaguar->r[(op >> 5) & 31];
		r2 = jaguar->r[op & 31];
		jaguar->accum += (INT64)((INT16)r1 * (INT16)r2);
		jaguar->PC += 2;
		op = ROPCODE(jaguar, jaguar->PC);
	}
	if ((op >> 10) == 19)
	{
		jaguar->PC += 2;
		jaguar->r[op & 31] = (UINT32)jaguar->accum;
	}
}

void jr_cc_n(jaguar_state *jaguar, UINT16 op)
{
	if (CONDITION(op & 31))
	{
		INT32 r1 = (INT8)((op >> 2) & 0xf8) >> 2;
		UINT32 newpc = jaguar->PC + r1;
		debugger_instruction_hook(jaguar->device, jaguar->PC);
		op = ROPCODE(jaguar, jaguar->PC);
		jaguar->PC = newpc;
		(*jaguar->table[op >> 10])(jaguar, op);

		jaguar->icount -= 3;	/* 3 wait states guaranteed */
	}
}

void jump_cc_rn(jaguar_state *jaguar, UINT16 op)
{
	if (CONDITION(op & 31))
	{
		UINT8 reg = (op >> 5) & 31;

		/* special kludge for risky code in the cojag DSP interrupt handlers */
		UINT32 newpc = (jaguar->icount == jaguar->bankswitch_icount) ? jaguar->a[reg] : jaguar->r[reg];
		debugger_instruction_hook(jaguar->device, jaguar->PC);
		op = ROPCODE(jaguar, jaguar->PC);
		jaguar->PC = newpc;
		(*jaguar->table[op >> 10])(jaguar, op);

		jaguar->icount -= 3;	/* 3 wait states guaranteed */
	}
}

void load_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->r[op & 31] = READLONG(jaguar, r1);
}

void load_r14n_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	jaguar->r[op & 31] = READLONG(jaguar, jaguar->r[14] + 4 * r1);
}

void load_r15n_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	jaguar->r[op & 31] = READLONG(jaguar, jaguar->r[15] + 4 * r1);
}

void load_r14rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->r[op & 31] = READLONG(jaguar, jaguar->r[14] + r1);
}

void load_r15rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->r[op & 31] = READLONG(jaguar, jaguar->r[15] + r1);
}

void loadb_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->r[op & 31] = READBYTE(jaguar, r1);
}

void loadw_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->r[op & 31] = READWORD(jaguar, r1);
}

void loadp_rn_rn(jaguar_state *jaguar, UINT16 op)	/* GPU only */
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->ctrl[G_HIDATA] = READWORD(jaguar, r1);
	jaguar->r[op & 31] = READWORD(jaguar, r1+4);
}

void mirror_rn(jaguar_state *jaguar, UINT16 op)	/* DSP only */
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[dreg];
	UINT32 res = (mirror_table[r1 & 0xffff] << 16) | mirror_table[r1 >> 16];
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void mmult_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int count = jaguar->ctrl[G_MTXC] & 15, i;
	int sreg = (op >> 5) & 31;
	int dreg = op & 31;
	UINT32 addr = jaguar->ctrl[G_MTXA];
	INT64 accum = 0;
	UINT32 res;

	if (!(jaguar->ctrl[G_MTXC] & 0x10))
	{
		for (i = 0; i < count; i++)
		{
			accum += (INT16)(jaguar->b1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (INT16)READWORD(jaguar, addr);
			addr += 2;
		}
	}
	else
	{
		for (i = 0; i < count; i++)
		{
			accum += (INT16)(jaguar->b1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (INT16)READWORD(jaguar, addr);
			addr += 2 * count;
		}
	}
	jaguar->r[dreg] = res = (UINT32)accum;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void move_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	jaguar->r[op & 31] = jaguar->r[(op >> 5) & 31];
}

void move_pc_rn(jaguar_state *jaguar, UINT16 op)
{
	jaguar->r[op & 31] = jaguar->ppc;
}

void movefa_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	jaguar->r[op & 31] = jaguar->a[(op >> 5) & 31];
}

void movei_n_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 res = ROPCODE(jaguar, jaguar->PC) | (ROPCODE(jaguar, jaguar->PC + 2) << 16);
	jaguar->PC += 4;
	jaguar->r[op & 31] = res;
}

void moveq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	jaguar->r[op & 31] = (op >> 5) & 31;
}

void moveta_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	jaguar->a[op & 31] = jaguar->r[(op >> 5) & 31];
}

void mtoi_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	jaguar->r[op & 31] = (((INT32)r1 >> 8) & 0xff800000) | (r1 & 0x007fffff);
}

void mult_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = (UINT16)r1 * (UINT16)r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void neg_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = -r2;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, 0, r2, res);
}

void nop(jaguar_state *jaguar, UINT16 op)
{
}

void normi_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 res = 0;
	if (r1 != 0)
	{
		while ((r1 & 0xffc00000) == 0)
		{
			r1 <<= 1;
			res--;
		}
		while ((r1 & 0xff800000) != 0)
		{
			r1 >>= 1;
			res++;
		}
	}
	jaguar->r[op & 31] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void not_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 res = ~jaguar->r[dreg];
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void or_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r1 | r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void pack_rn(jaguar_state *jaguar, UINT16 op)		/* GPU only */
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res;
	if (r1 == 0)	/* PACK */
		res = ((r2 >> 10) & 0xf000) | ((r2 >> 5) & 0x0f00) | (r2 & 0xff);
	else			/* UNPACK */
		res = ((r2 & 0xf000) << 10) | ((r2 & 0x0f00) << 5) | (r2 & 0xff);
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void resmac_rn(jaguar_state *jaguar, UINT16 op)
{
	jaguar->r[op & 31] = (UINT32)jaguar->accum;
}

void ror_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31] & 31;
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZN(jaguar, res); jaguar->FLAGS |= (r2 >> 30) & 2;
}

void rorq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZN(jaguar, res); jaguar->FLAGS |= (r2 >> 30) & 2;
}

void sat8_rn(jaguar_state *jaguar, UINT16 op)		/* GPU only */
{
	int dreg = op & 31;
	INT32 r2 = jaguar->r[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 255) ? 255 : r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void sat16_rn(jaguar_state *jaguar, UINT16 op)		/* GPU only */
{
	int dreg = op & 31;
	INT32 r2 = jaguar->r[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 65535) ? 65535 : r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void sat16s_rn(jaguar_state *jaguar, UINT16 op)		/* DSP only */
{
	int dreg = op & 31;
	INT32 r2 = jaguar->r[dreg];
	UINT32 res = (r2 < -32768) ? -32768 : (r2 > 32767) ? 32767 : r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void sat24_rn(jaguar_state *jaguar, UINT16 op)			/* GPU only */
{
	int dreg = op & 31;
	INT32 r2 = jaguar->r[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 16777215) ? 16777215 : r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void sat32s_rn(jaguar_state *jaguar, UINT16 op)		/* DSP only */
{
	int dreg = op & 31;
	INT32 r2 = (UINT32)jaguar->r[dreg];
	INT32 temp = jaguar->accum >> 32;
	UINT32 res = (temp < -1) ? (INT32)0x80000000 : (temp > 0) ? (INT32)0x7fffffff : r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}

void sh_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = (INT32)jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res;

	CLR_ZNC(jaguar);
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		jaguar->FLAGS |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? 0 : (r2 >> r1);
		jaguar->FLAGS |= (r2 << 1) & 2;
	}
	jaguar->r[dreg] = res;
	SET_ZN(jaguar, res);
}

void sha_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = (INT32)jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res;

	CLR_ZNC(jaguar);
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		jaguar->FLAGS |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? ((INT32)r2 >> 31) : ((INT32)r2 >> r1);
		jaguar->FLAGS |= (r2 << 1) & 2;
	}
	jaguar->r[dreg] = res;
	SET_ZN(jaguar, res);
}

void sharq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = (INT32)r2 >> r1;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZN(jaguar, res); jaguar->FLAGS |= (r2 << 1) & 2;
}

void shlq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 << (32 - r1);
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZN(jaguar, res); jaguar->FLAGS |= (r2 >> 30) & 2;
}

void shrq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 >> r1;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZN(jaguar, res); jaguar->FLAGS |= (r2 << 1) & 2;
}

void store_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	WRITELONG(jaguar, r1, jaguar->r[op & 31]);
}

void store_rn_r14n(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	WRITELONG(jaguar, jaguar->r[14] + r1 * 4, jaguar->r[op & 31]);
}

void store_rn_r15n(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	WRITELONG(jaguar, jaguar->r[15] + r1 * 4, jaguar->r[op & 31]);
}

void store_rn_r14rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	WRITELONG(jaguar, jaguar->r[14] + r1, jaguar->r[op & 31]);
}

void store_rn_r15rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	WRITELONG(jaguar, jaguar->r[15] + r1, jaguar->r[op & 31]);
}

void storeb_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	WRITEBYTE(jaguar, r1, jaguar->r[op & 31]);
}

void storew_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	WRITEWORD(jaguar, r1, jaguar->r[op & 31]);
}

void storep_rn_rn(jaguar_state *jaguar, UINT16 op)	/* GPU only */
{
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	WRITELONG(jaguar, r1, jaguar->ctrl[G_HIDATA]);
	WRITELONG(jaguar, r1+4, jaguar->r[op & 31]);
}

void sub_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 - r1;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, r2, r1, res);
}

void subc_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 - r1 - ((jaguar->FLAGS >> 1) & 1);
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, r2, r1, res);
}

void subq_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 - r1;
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, r2, r1, res);
}

void subqmod_n_rn(jaguar_state *jaguar, UINT16 op)	/* DSP only */
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 - r1;
	res = (res & ~jaguar->ctrl[D_MOD]) | (r2 & ~jaguar->ctrl[D_MOD]);
	jaguar->r[dreg] = res;
	CLR_ZNC(jaguar); SET_ZNC_SUB(jaguar, r2, r1, res);
}

void subqt_n_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r2 - r1;
	jaguar->r[dreg] = res;
}

void xor_rn_rn(jaguar_state *jaguar, UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = jaguar->r[(op >> 5) & 31];
	UINT32 r2 = jaguar->r[dreg];
	UINT32 res = r1 ^ r2;
	jaguar->r[dreg] = res;
	CLR_ZN(jaguar); SET_ZN(jaguar, res);
}



/***************************************************************************
    I/O HANDLING
***************************************************************************/

UINT32 jaguargpu_ctrl_r(device_t *device, offs_t offset)
{
	jaguar_state *jaguar = get_safe_token(device);

	if (LOG_GPU_IO) logerror("GPU read register @ F021%02X\n", offset * 4);

	return jaguar->ctrl[offset];
}


void jaguargpu_ctrl_w(device_t *device, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	jaguar_state *jaguar = get_safe_token(device);
	UINT32 oldval, newval;

	if (LOG_GPU_IO && offset != G_HIDATA)
		logerror("GPU write register @ F021%02X = %08X\n", offset * 4, data);

	/* remember the old and set the new */
	oldval = jaguar->ctrl[offset];
	newval = oldval;
	COMBINE_DATA(&newval);

	/* handle the various registers */
	switch (offset)
	{
		case G_FLAGS:

			/* combine the data properly */
			jaguar->ctrl[offset] = newval & (ZFLAG | CFLAG | NFLAG | EINT04FLAGS | RPAGEFLAG);
			if (newval & IFLAG)
				jaguar->ctrl[offset] |= oldval & IFLAG;

			/* clear interrupts */
			jaguar->ctrl[G_CTRL] &= ~((newval & CINT04FLAGS) >> 3);

			/* determine which register bank should be active */
			update_register_banks(jaguar);

			/* update IRQs */
			check_irqs(jaguar);
			break;

		case G_MTXC:
		case G_MTXA:
			jaguar->ctrl[offset] = newval;
			break;

		case G_END:
			jaguar->ctrl[offset] = newval;
			if ((newval & 7) != 7)
				logerror("GPU to set to little-endian!\n");
			break;

		case G_PC:
			jaguar->PC = newval & 0xffffff;
			break;

		case G_CTRL:
			jaguar->ctrl[offset] = newval;
			if ((oldval ^ newval) & 0x01)
			{
				device_set_input_line(device, INPUT_LINE_HALT, (newval & 1) ? CLEAR_LINE : ASSERT_LINE);
				device_yield(device);
			}
			if (newval & 0x02)
			{
				if (jaguar->cpu_interrupt != NULL)
					(*jaguar->cpu_interrupt)(device);
				jaguar->ctrl[offset] &= ~0x02;
			}
			if (newval & 0x04)
			{
				jaguar->ctrl[G_CTRL] |= 1 << 6;
				jaguar->ctrl[offset] &= ~0x04;
				check_irqs(jaguar);
			}
			if (newval & 0x18)
			{
				logerror("GPU single stepping was enabled!\n");
			}
			break;

		case G_HIDATA:
		case G_DIVCTRL:
			jaguar->ctrl[offset] = newval;
			break;
	}
}



/***************************************************************************
    I/O HANDLING
***************************************************************************/

UINT32 jaguardsp_ctrl_r(device_t *device, offs_t offset)
{
	jaguar_state *jaguar = get_safe_token(device);

	if (LOG_DSP_IO && offset != D_FLAGS)
		logerror("DSP read register @ F1A1%02X\n", offset * 4);

	/* switch to the target context */
	return jaguar->ctrl[offset];
}


void jaguardsp_ctrl_w(device_t *device, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	jaguar_state *jaguar = get_safe_token(device);
	UINT32 oldval, newval;

	if (LOG_DSP_IO && offset != D_FLAGS)
		logerror("DSP write register @ F1A1%02X = %08X\n", offset * 4, data);

	/* remember the old and set the new */
	oldval = jaguar->ctrl[offset];
	newval = oldval;
	COMBINE_DATA(&newval);

	/* handle the various registers */
	switch (offset)
	{
		case D_FLAGS:

			/* combine the data properly */
			jaguar->ctrl[offset] = newval & (ZFLAG | CFLAG | NFLAG | EINT04FLAGS | EINT5FLAG | RPAGEFLAG);
			if (newval & IFLAG)
				jaguar->ctrl[offset] |= oldval & IFLAG;

			/* clear interrupts */
			jaguar->ctrl[D_CTRL] &= ~((newval & CINT04FLAGS) >> 3);
			jaguar->ctrl[D_CTRL] &= ~((newval & CINT5FLAG) >> 1);

			/* determine which register bank should be active */
			update_register_banks(jaguar);

			/* update IRQs */
			check_irqs(jaguar);
			break;

		case D_MTXC:
		case D_MTXA:
			jaguar->ctrl[offset] = newval;
			break;

		case D_END:
			jaguar->ctrl[offset] = newval;
			if ((newval & 7) != 7)
				logerror("DSP to set to little-endian!\n");
			break;

		case D_PC:
			jaguar->PC = newval & 0xffffff;
			break;

		case D_CTRL:
			jaguar->ctrl[offset] = newval;
			if ((oldval ^ newval) & 0x01)
			{
				device_set_input_line(device, INPUT_LINE_HALT, (newval & 1) ? CLEAR_LINE : ASSERT_LINE);
				device_yield(device);
			}
			if (newval & 0x02)
			{
				if (jaguar->cpu_interrupt != NULL)
					(*jaguar->cpu_interrupt)(device);
				jaguar->ctrl[offset] &= ~0x02;
			}
			if (newval & 0x04)
			{
				jaguar->ctrl[D_CTRL] |= 1 << 6;
				jaguar->ctrl[offset] &= ~0x04;
				check_irqs(jaguar);
			}
			if (newval & 0x18)
			{
				logerror("DSP single stepping was enabled!\n");
			}
			break;

		case D_MOD:
		case D_DIVCTRL:
			jaguar->ctrl[offset] = newval;
			break;
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( jaguargpu )
{
	jaguar_state *jaguar = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ0:		set_irq_line(jaguar, JAGUAR_IRQ0, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ1:		set_irq_line(jaguar, JAGUAR_IRQ1, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ2:		set_irq_line(jaguar, JAGUAR_IRQ2, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ3:		set_irq_line(jaguar, JAGUAR_IRQ3, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ4:		set_irq_line(jaguar, JAGUAR_IRQ4, info->i);			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + JAGUAR_PC:			jaguar->PC = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_FLAGS:		jaguar->FLAGS = info->i;							break;

		case CPUINFO_INT_REGISTER + JAGUAR_R0:			jaguar->r[0] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R1:			jaguar->r[1] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R2:			jaguar->r[2] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R3:			jaguar->r[3] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R4:			jaguar->r[4] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R5:			jaguar->r[5] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R6:			jaguar->r[6] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R7:			jaguar->r[7] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R8:			jaguar->r[8] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R9:			jaguar->r[9] = info->i;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R10:			jaguar->r[10] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R11:			jaguar->r[11] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R12:			jaguar->r[12] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R13:			jaguar->r[13] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R14:			jaguar->r[14] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R15:			jaguar->r[15] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R16:			jaguar->r[16] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R17:			jaguar->r[17] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R18:			jaguar->r[18] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R19:			jaguar->r[19] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R20:			jaguar->r[20] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R21:			jaguar->r[21] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R22:			jaguar->r[22] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R23:			jaguar->r[23] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R24:			jaguar->r[24] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R25:			jaguar->r[25] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R26:			jaguar->r[26] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R27:			jaguar->r[27] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R28:			jaguar->r[28] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R29:			jaguar->r[29] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R30:			jaguar->r[30] = info->i;							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R31:			jaguar->r[31] = info->i;							break;
		case CPUINFO_INT_SP:							jaguar->b0[31] = info->i;							break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( jaguargpu )
{
	jaguar_state *jaguar = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(jaguar_state);						break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 5;										break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;										break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;								break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;										break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;										break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;										break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;										break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;										break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;										break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;								break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 24;								break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;								break;

		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ0:		info->i = (jaguar->ctrl[G_CTRL] & 0x40) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ1:		info->i = (jaguar->ctrl[G_CTRL] & 0x80) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ2:		info->i = (jaguar->ctrl[G_CTRL] & 0x100) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ3:		info->i = (jaguar->ctrl[G_CTRL] & 0x200) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ4:		info->i = (jaguar->ctrl[G_CTRL] & 0x400) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = jaguar->ppc;								break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + JAGUAR_PC:			info->i = jaguar->PC;								break;
		case CPUINFO_INT_REGISTER + JAGUAR_FLAGS:		info->i = jaguar->FLAGS;							break;

		case CPUINFO_INT_REGISTER + JAGUAR_R0:			info->i = jaguar->r[0];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R1:			info->i = jaguar->r[1];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R2:			info->i = jaguar->r[2];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R3:			info->i = jaguar->r[3];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R4:			info->i = jaguar->r[4];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R5:			info->i = jaguar->r[5];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R6:			info->i = jaguar->r[6];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R7:			info->i = jaguar->r[7];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R8:			info->i = jaguar->r[8];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R9:			info->i = jaguar->r[9];								break;
		case CPUINFO_INT_REGISTER + JAGUAR_R10:			info->i = jaguar->r[10];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R11:			info->i = jaguar->r[11];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R12:			info->i = jaguar->r[12];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R13:			info->i = jaguar->r[13];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R14:			info->i = jaguar->r[14];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R15:			info->i = jaguar->r[15];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R16:			info->i = jaguar->r[16];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R17:			info->i = jaguar->r[17];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R18:			info->i = jaguar->r[18];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R19:			info->i = jaguar->r[19];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R20:			info->i = jaguar->r[20];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R21:			info->i = jaguar->r[21];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R22:			info->i = jaguar->r[22];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R23:			info->i = jaguar->r[23];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R24:			info->i = jaguar->r[24];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R25:			info->i = jaguar->r[25];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R26:			info->i = jaguar->r[26];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R27:			info->i = jaguar->r[27];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R28:			info->i = jaguar->r[28];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R29:			info->i = jaguar->r[29];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R30:			info->i = jaguar->r[30];							break;
		case CPUINFO_INT_REGISTER + JAGUAR_R31:			info->i = jaguar->r[31];							break;
		case CPUINFO_INT_SP:							info->i = jaguar->b0[31];							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(jaguargpu);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(jaguargpu);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(jaguar);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(jaguar);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(jaguargpu);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;									break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(jaguargpu);break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &jaguar->icount;						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Jaguar GPU");						break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Atari Jaguar");					break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles");			break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c",
															jaguar->FLAGS & 0x8000 ? 'D':'.',
															jaguar->FLAGS & 0x4000 ? 'A':'.',
															jaguar->FLAGS & 0x0100 ? '4':'.',
															jaguar->FLAGS & 0x0080 ? '3':'.',
															jaguar->FLAGS & 0x0040 ? '2':'.',
															jaguar->FLAGS & 0x0020 ? '1':'.',
															jaguar->FLAGS & 0x0010 ? '0':'.',
															jaguar->FLAGS & 0x0008 ? 'I':'.',
															jaguar->FLAGS & 0x0004 ? 'N':'.',
															jaguar->FLAGS & 0x0002 ? 'C':'.',
															jaguar->FLAGS & 0x0001 ? 'Z':'.');				break;

		case CPUINFO_STR_REGISTER + JAGUAR_PC:  		sprintf(info->s, "PC: %08X", jaguar->PC);			break;
		case CPUINFO_STR_REGISTER + JAGUAR_FLAGS:		sprintf(info->s, "FLAGS: %08X", jaguar->FLAGS); 	break;
		case CPUINFO_STR_REGISTER + JAGUAR_R0:			sprintf(info->s, "R0: %08X", jaguar->r[0]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R1:			sprintf(info->s, "R1: %08X", jaguar->r[1]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R2:			sprintf(info->s, "R2: %08X", jaguar->r[2]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R3:			sprintf(info->s, "R3: %08X", jaguar->r[3]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R4:			sprintf(info->s, "R4: %08X", jaguar->r[4]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R5:			sprintf(info->s, "R5: %08X", jaguar->r[5]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R6:			sprintf(info->s, "R6: %08X", jaguar->r[6]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R7:			sprintf(info->s, "R7: %08X", jaguar->r[7]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R8:			sprintf(info->s, "R8: %08X", jaguar->r[8]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R9:			sprintf(info->s, "R9: %08X", jaguar->r[9]); 		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R10:			sprintf(info->s, "R10:%08X", jaguar->r[10]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R11:			sprintf(info->s, "R11:%08X", jaguar->r[11]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R12:			sprintf(info->s, "R12:%08X", jaguar->r[12]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R13:			sprintf(info->s, "R13:%08X", jaguar->r[13]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R14:			sprintf(info->s, "R14:%08X", jaguar->r[14]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R15:			sprintf(info->s, "R15:%08X", jaguar->r[15]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R16:			sprintf(info->s, "R16:%08X", jaguar->r[16]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R17:			sprintf(info->s, "R17:%08X", jaguar->r[17]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R18:			sprintf(info->s, "R18:%08X", jaguar->r[18]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R19:			sprintf(info->s, "R19:%08X", jaguar->r[19]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R20:			sprintf(info->s, "R20:%08X", jaguar->r[20]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R21:			sprintf(info->s, "R21:%08X", jaguar->r[21]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R22:			sprintf(info->s, "R22:%08X", jaguar->r[22]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R23:			sprintf(info->s, "R23:%08X", jaguar->r[23]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R24:			sprintf(info->s, "R24:%08X", jaguar->r[24]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R25:			sprintf(info->s, "R25:%08X", jaguar->r[25]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R26:			sprintf(info->s, "R26:%08X", jaguar->r[26]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R27:			sprintf(info->s, "R27:%08X", jaguar->r[27]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R28:			sprintf(info->s, "R28:%08X", jaguar->r[28]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R29:			sprintf(info->s, "R29:%08X", jaguar->r[29]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R30:			sprintf(info->s, "R30:%08X", jaguar->r[30]);		break;
		case CPUINFO_STR_REGISTER + JAGUAR_R31:			sprintf(info->s, "R31:%08X", jaguar->r[31]);		break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static CPU_SET_INFO( jaguardsp )
{
	jaguar_state *jaguar = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ5:		set_irq_line(jaguar, JAGUAR_IRQ5, info->i);			break;

		/* --- the following bits of info are set as pointers to data or functions --- */

		default:										CPU_SET_INFO_CALL(jaguargpu);						break;
	}
}

CPU_GET_INFO( jaguardsp )
{
	jaguar_state *jaguar = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;										break;
		case CPUINFO_INT_INPUT_STATE + JAGUAR_IRQ5:		info->i = (jaguar->ctrl[G_CTRL] & 0x10000) ? ASSERT_LINE : CLEAR_LINE; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(jaguardsp);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(jaguardsp);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(jaguardsp);		break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(jaguardsp);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Jaguar DSP");						break;

		default:										CPU_GET_INFO_CALL(jaguargpu);						break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(JAGUARGPU, jaguargpu);
DEFINE_LEGACY_CPU_DEVICE(JAGUARDSP, jaguardsp);
