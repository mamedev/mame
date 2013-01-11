/***************************************************************************

    cubeqcpu.c

    Implementation of the Cube Quest AM2901-based CPUs

    TODO:

    * Tidy up diassembly (split into different files?)

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cubeqcpu.h"


CPU_DISASSEMBLE( cquestsnd );
CPU_DISASSEMBLE( cquestrot );
CPU_DISASSEMBLE( cquestlin );


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Am2901 Instruction Fields */
enum alu_src
{
	AQ = 0,
	AB = 1,
	ZQ = 2,
	ZB = 3,
	ZA = 4,
	DA = 5,
	DQ = 6,
	DZ = 7,
};

enum alu_ins
{
	ADD   = 0,
	SUBR  = 1,
	SUBS  = 2,
	OR    = 3,
	AND   = 4,
	NOTRS = 5,
	EXOR  = 6,
	EXNOR = 7,
};

enum alu_dst
{
	QREG  = 0,
	NOP   = 1,
	RAMA  = 2,
	RAMF  = 3,
	RAMQD = 4,
	RAMD  = 5,
	RAMQU = 6,
	RAMU  = 7,
};

/***************************************************************************
    MACROS
***************************************************************************/

#define _BIT(x, n)          ((x) & (1 << (n)))

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

struct cquestsnd_state
{
	/* AM2901 internals */
	UINT16  ram[16];
	UINT16  q;
	UINT16  f;
	UINT16  y;
	UINT32  cflag;
	UINT32  vflag;

	UINT8   pc;         /* 2 x LS161 @ 6E, 6F */
	UINT16  platch;
	UINT8   rtnlatch;   /* LS374 @ 5F */
	UINT8   adrcntr;    /* 2 x LS161 */
	UINT16  adrlatch;
	UINT16  dinlatch;
	UINT16  ramwlatch;

	UINT16 *sram;

	int prev_ipram;
	int prev_ipwrt;

	cubeqst_dac_w_func dac_w;
	UINT16 *sound_data;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int icount;
};


struct cquestrot_state
{
	/* AM2901 internals */
	UINT16  ram[16];
	UINT16  q;
	UINT16  f;
	UINT16  y;
	UINT32  cflag;
	UINT32  vflag;

	UINT16  pc;         /* 12-bit, but only 9 used */
	UINT8   seqcnt;     /* 4-bit counter */

	UINT8   dsrclatch;
	UINT8   rsrclatch;
	UINT16  dynaddr;    /* LS374 at 2D, 8D  */
	UINT16  dyndata;    /* LS374 at 10B, 9B */
	UINT16  yrlatch;    /* LS374 at 9D, 10D */
	UINT16  ydlatch;    /* LS374 at 9C, 10C */
	UINT16  dinlatch;
	UINT8   divreg;     /* LS74 at ? */

	UINT16  linedata;
	UINT16  lineaddr;

	UINT16 *dram;
	UINT16 *sram;

	UINT8 prev_dred;
	UINT8 prev_dwrt;
	UINT8 wc;
	UINT8 rc;
	UINT8 clkcnt;

	legacy_cpu_device *device;
	legacy_cpu_device *lindevice;
	address_space *program;
	direct_read_data *direct;
	int icount;
};


struct cquestlin_state
{
	/* 12-bit AM2901 internals */
	UINT16  ram[16];
	UINT16  q;
	UINT16  f;
	UINT16  y;
	UINT32  cflag;
	UINT32  vflag;

	UINT8   pc[2];      /* Two program counters; one for FG, other for BG */

	UINT16  seqcnt;     /* 12-bit */
	UINT16  clatch;     /* LS374 at 9E and 1-bit FF */
	UINT8   zlatch;     /* LS374 at 4H */

	UINT16  xcnt;
	UINT16  ycnt;
	UINT8   sreg;

	UINT16  fadlatch;
	UINT16  badlatch;

	UINT16  sramdlatch;

	UINT8   fglatch;
	UINT8   bglatch;
	UINT8   gt0reg;
	UINT8   fdxreg;
	UINT32  field;

	UINT32  clkcnt;

	/* RAM */
	UINT16  *sram;
	UINT8   *ptr_ram;
	UINT32  *e_stack;
	UINT32  *o_stack;

	legacy_cpu_device *device;
	legacy_cpu_device *rotdevice;
	address_space *program;
	direct_read_data *direct;
	int icount;
};

/***************************************************************************
    STATE ACCESSORS
***************************************************************************/

INLINE cquestsnd_state *get_safe_token_snd(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CQUESTSND);
	return (cquestsnd_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE cquestrot_state *get_safe_token_rot(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CQUESTROT);
	return (cquestrot_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE cquestlin_state *get_safe_token_lin(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CQUESTLIN);
	return (cquestlin_state *)downcast<legacy_cpu_device *>(device)->token();
}

/***************************************************************************
    MEMORY ACCESSORS FOR 68000
***************************************************************************/

WRITE16_DEVICE_HANDLER( cubeqcpu_sndram_w )
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	COMBINE_DATA(&cpustate->sram[offset]);
}

READ16_DEVICE_HANDLER( cubeqcpu_sndram_r )
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	return cpustate->sram[offset];
}


WRITE16_DEVICE_HANDLER( cubeqcpu_rotram_w )
{
	cquestrot_state *cpustate = get_safe_token_rot(device);
	COMBINE_DATA(&cpustate->dram[offset]);
}

READ16_DEVICE_HANDLER( cubeqcpu_rotram_r )
{
	cquestrot_state *cpustate = get_safe_token_rot(device);
	return cpustate->dram[offset];
}

/***************************************************************************
    SOUND INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void cquestsnd_state_register(device_t *device)
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	device->save_item(NAME(cpustate->ram));
	device->save_item(NAME(cpustate->q));
	device->save_item(NAME(cpustate->f));
	device->save_item(NAME(cpustate->y));
	device->save_item(NAME(cpustate->cflag));
	device->save_item(NAME(cpustate->vflag));

	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->platch));
	device->save_item(NAME(cpustate->rtnlatch));
	device->save_item(NAME(cpustate->adrcntr));
	device->save_item(NAME(cpustate->adrlatch));
	device->save_item(NAME(cpustate->dinlatch));
	device->save_item(NAME(cpustate->ramwlatch));
	device->save_item(NAME(cpustate->prev_ipram));
	device->save_item(NAME(cpustate->prev_ipwrt));
}

static CPU_INIT( cquestsnd )
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	cubeqst_snd_config* _config = (cubeqst_snd_config*)device->static_config();

	memset(cpustate, 0, sizeof(*cpustate));

	cpustate->dac_w = _config->dac_w;
	cpustate->sound_data = (UINT16*)device->machine().root_device().memregion(_config->sound_data_region)->base();

	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	/* Allocate RAM shared with 68000 */
	cpustate->sram = auto_alloc_array(device->machine(), UINT16, 4096/2);

	cquestsnd_state_register(device);
}


static CPU_RESET( cquestsnd )
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	cpustate->pc = 0;
}


static CPU_EXIT( cquestsnd )
{
}


/***************************************************************************
    ROTATE INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void cquestrot_state_register(device_t *device)
{
	cquestrot_state *cpustate = get_safe_token_rot(device);
	device->save_item(NAME(cpustate->ram));
	device->save_item(NAME(cpustate->q));
	device->save_item(NAME(cpustate->f));
	device->save_item(NAME(cpustate->y));
	device->save_item(NAME(cpustate->cflag));
	device->save_item(NAME(cpustate->vflag));

	device->save_item(NAME(cpustate->pc));
	device->save_item(NAME(cpustate->seqcnt));
	device->save_item(NAME(cpustate->dsrclatch));
	device->save_item(NAME(cpustate->rsrclatch));
	device->save_item(NAME(cpustate->dynaddr));
	device->save_item(NAME(cpustate->dyndata));
	device->save_item(NAME(cpustate->yrlatch));
	device->save_item(NAME(cpustate->ydlatch));
	device->save_item(NAME(cpustate->dinlatch));
	device->save_item(NAME(cpustate->divreg));
	device->save_item(NAME(cpustate->linedata));
	device->save_item(NAME(cpustate->lineaddr));
	device->save_item(NAME(cpustate->prev_dred));
	device->save_item(NAME(cpustate->prev_dwrt));
	device->save_item(NAME(cpustate->wc));

	device->save_pointer(NAME(cpustate->dram), 16384);
	device->save_pointer(NAME(cpustate->sram), 2048);
}

static CPU_INIT( cquestrot )
{
	const cubeqst_rot_config *rotconfig = (const cubeqst_rot_config *)device->static_config();
	cquestrot_state *cpustate = get_safe_token_rot(device);
	memset(cpustate, 0, sizeof(*cpustate));

	/* Allocate RAM */
	cpustate->dram = auto_alloc_array(device->machine(), UINT16, 16384);  /* Shared with 68000 */
	cpustate->sram = auto_alloc_array(device->machine(), UINT16, 2048);   /* Private */

	cpustate->device = device;
	cpustate->lindevice = device->machine().device<legacy_cpu_device>(rotconfig->lin_cpu_tag);
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	cquestrot_state_register(device);
}


static CPU_RESET( cquestrot )
{
	cquestrot_state *cpustate = get_safe_token_rot(device);
	cpustate->pc = 0;
	cpustate->wc = 0;
	cpustate->prev_dred = 1;
	cpustate->prev_dwrt = 1;
}


static CPU_EXIT( cquestrot )
{
}


/***************************************************************************
    LINE DRAWER INITIALIZATION AND SHUTDOWN
***************************************************************************/
#define FOREGROUND      0
#define BACKGROUND      1
#define ODD_FIELD       0
#define EVEN_FIELD      1

static void cquestlin_state_register(device_t *device)
{
	cquestlin_state *cpustate = get_safe_token_lin(device);

	device->save_item(NAME(cpustate->ram));
	device->save_item(NAME(cpustate->q));
	device->save_item(NAME(cpustate->f));
	device->save_item(NAME(cpustate->y));
	device->save_item(NAME(cpustate->cflag));
	device->save_item(NAME(cpustate->vflag));

	device->save_item(NAME(cpustate->pc[0]));
	device->save_item(NAME(cpustate->pc[1]));
	device->save_item(NAME(cpustate->seqcnt));
	device->save_item(NAME(cpustate->clatch));
	device->save_item(NAME(cpustate->zlatch));
	device->save_item(NAME(cpustate->xcnt));
	device->save_item(NAME(cpustate->ycnt));
	device->save_item(NAME(cpustate->sreg));
	device->save_item(NAME(cpustate->fadlatch));
	device->save_item(NAME(cpustate->badlatch));
	device->save_item(NAME(cpustate->sramdlatch));
	device->save_item(NAME(cpustate->fglatch));
	device->save_item(NAME(cpustate->bglatch));
	device->save_item(NAME(cpustate->gt0reg));
	device->save_item(NAME(cpustate->fdxreg));
	device->save_item(NAME(cpustate->field));
	device->save_item(NAME(cpustate->clkcnt));

	device->save_pointer(NAME(cpustate->sram), 4096);
	device->save_pointer(NAME(cpustate->ptr_ram), 1024);
	device->save_pointer(NAME(cpustate->e_stack), 32768);
	device->save_pointer(NAME(cpustate->o_stack), 32768);
}

static CPU_INIT( cquestlin )
{
	const cubeqst_lin_config *linconfig = (const cubeqst_lin_config *)device->static_config();
	cquestlin_state *cpustate = get_safe_token_lin(device);
	memset(cpustate, 0, sizeof(*cpustate));

	/* Allocate RAM */
	cpustate->sram = auto_alloc_array(device->machine(), UINT16, 4096);      /* Shared with rotate CPU */
	cpustate->ptr_ram = auto_alloc_array(device->machine(), UINT8, 1024);                    /* Pointer RAM */
	cpustate->e_stack = auto_alloc_array(device->machine(), UINT32, 32768);  /* Stack DRAM: 32kx20 */
	cpustate->o_stack = auto_alloc_array(device->machine(), UINT32, 32768);  /* Stack DRAM: 32kx20 */

	cpustate->device = device;
	cpustate->rotdevice = device->machine().device<legacy_cpu_device>(linconfig->rot_cpu_tag);
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	cquestlin_state_register(device);
}


static CPU_RESET( cquestlin )
{
	cquestlin_state *cpustate = get_safe_token_lin(device);
	cpustate->clkcnt = 0;
	cpustate->pc[FOREGROUND] = 0;
	cpustate->pc[BACKGROUND] = 0x80;
}


static CPU_EXIT( cquestlin )
{
}


/***************************************************************************
    SOUND CORE EXECUTION LOOP
***************************************************************************/

#define SND_PC          (cpustate->pc)
#define SND_DATA_IN     (_ramen ? cpustate->sound_data[cpustate->platch] : cpustate->dinlatch)

enum snd_latch_type
{
	PLTCH = 0,
	DAC = 1,
	ADLATCH = 2,
};

static int do_sndjmp(cquestsnd_state *cpustate, int jmp)
{
	switch (jmp)
	{
		/* JUMP */ case 0: return 1;
		/* MSB  */ case 2: return cpustate->f & 0x8000 ? 0 : 1;
		/* !MSB */ case 3: return cpustate->f & 0x8000 ? 1 : 0;
		/* ZERO */ case 5: return cpustate->f == 0 ? 0 : 1;
		/* OVR  */ case 6: return cpustate->vflag ? 0 : 1;
		/* LOOP */ case 7: return cpustate->adrcntr & 0x80 ? 0: 1;
	}

	return 0;
}

static CPU_EXECUTE( cquestsnd )
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	int calldebugger = ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	/* Core execution loop */
	do
	{
		/* Decode the instruction */
		UINT64 inst = cpustate->direct->read_decrypted_qword(SND_PC << 3);
		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 24) & 0xff;
		int b       = (inshig >> 20) & 0xf;
		int a       = (inshig >> 16) & 0xf;
		int ci      = (inshig >> 15) & 1;
		int i5_3    = (inshig >> 12) & 7;
		int _ramen  = (inshig >> 11) & 1;
		int i2_0    = (inshig >> 8) & 7;
		int rtnltch = (inshig >> 7) & 1;
		int jmp     = (inshig >> 4) & 7;
		int inca    = (inshig >> 3) & 1;
		int i8_6    = (inshig >> 0) & 7;
		int _ipram  = (inslow >> 31) & 1;
		int _ipwrt  = (inslow >> 30) & 1;
		int latch   = (inslow >> 28) & 3;
		int rtn     = (inslow >> 27) & 1;
		int _rin    = (inslow >> 26) & 1;

		if (calldebugger)
			debugger_instruction_hook(device, cpustate->pc);

		/* Don't think this matters, but just in case */
		if (rtn)
			t = cpustate->rtnlatch;

		/* Handle the AM2901 ALU instruction */
		{
			UINT16 r = 0;
			UINT16 s = 0;

			UINT32 res = 0;
			UINT32 cflag = 0;
			UINT32 vflag = 0;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case AQ: r = cpustate->ram[a]; s = cpustate->q;      break;
				case AB: r = cpustate->ram[a]; s = cpustate->ram[b]; break;
				case ZQ: r = 0;                s = cpustate->q;      break;
				case ZB: r = 0;                s = cpustate->ram[b]; break;
				case ZA: r = 0;                s = cpustate->ram[a]; break;
				case DA: r = SND_DATA_IN;      s = cpustate->ram[a]; break;
				case DQ: r = SND_DATA_IN;      s = cpustate->q;      break;
				case DZ: r = SND_DATA_IN;      s = 0;                break;
			}

			/* Perform the ALU operation */
			switch (i5_3)
			{
				case ADD:
					res = r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBR:
					res = ~r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((s & 0x7fff) + (~r & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBS:
					res = r + ~s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (~s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case OR:
					res = r | s;
					break;
				case AND:
					res = r & s;
					break;
				case NOTRS:
					res = ~r & s;
					break;
				case EXOR:
					res = r ^ s;
					break;
				case EXNOR:
					res = ~(r ^ s);
					break;
			}

			cpustate->f = res;
			cpustate->cflag = cflag;
			cpustate->vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					cpustate->q = cpustate->f;
					cpustate->y = cpustate->f;
					break;
				case NOP:
					cpustate->y = cpustate->f;
					break;
				case RAMA:
					cpustate->y = cpustate->ram[a];
					cpustate->ram[b] = cpustate->f;
					break;
				case RAMF:
					cpustate->ram[b] = cpustate->f;
					cpustate->y = cpustate->f;
					break;
				case RAMQD:
				{
					UINT16 qin;

					cpustate->ram[b] = (_rin ? 0 : 0x8000) | (cpustate->f >> 1);
					cpustate->q >>= 1;
					cpustate->y = cpustate->f;

					/* When right shifting Q, we need to OR in a value */
					qin = (((cpustate->y >> 15) ^ (cpustate->y >> 1)) & 1) ? 0 : 0x8000;

					cpustate->q |= qin;
					break;
				}
				case RAMD:
					cpustate->ram[b] = (_rin ? 0 : 0x8000) | (cpustate->f >> 1);
					cpustate->y = cpustate->f;
					break;
				case RAMQU:
					cpustate->ram[b] = (cpustate->f << 1) | (_rin ? 0 : 0x0001);
					cpustate->q <<= 1;
					cpustate->y = cpustate->f;
					break;
				case RAMU:
					cpustate->ram[b] = (cpustate->f << 1) | (_rin ? 0 : 0x0001);
					cpustate->y = cpustate->f;
					break;
			}
		}

		/* Now handle any SRAM accesses from the previous cycle */
		if (!cpustate->prev_ipram)
		{
			UINT16 addr = cpustate->adrlatch | (cpustate->adrcntr & 0x7f);

			if (!cpustate->prev_ipwrt)
			cpustate->sram[addr] = cpustate->ramwlatch;
			else
			cpustate->dinlatch = cpustate->sram[addr];
		}

		/* Handle latches */
		if (latch == PLTCH)
		{
			cpustate->platch = ((t & 3) << 9) | ((cpustate->y >> 6) & 0x1ff);
		}
		else if (latch == DAC)
		{
			cpustate->dac_w(cpustate->device, (cpustate->y & 0xfff0) | ((cpustate->adrcntr >> 3) & 0xf));
		}
		else if (latch == ADLATCH)
		{
			/* Load the SRAM address counter - this value is instantly loaded */
			cpustate->adrcntr = cpustate->y & 0x7f;

			/* Also load the SRAM address latch */
			cpustate->adrlatch = cpustate->y & 0x780;
		}

		/* Check for jump/return */
		if ( do_sndjmp(cpustate, jmp) )
			cpustate->pc = rtn ? cpustate->rtnlatch : t;
		else
			cpustate->pc++;

		/* Load the return latch? (Obviously a load and a ret in the same cycle are invalid) */
		if (rtnltch)
			cpustate->rtnlatch = t;

		/* Only increment the sound counter if not loading */
		if (inca && latch != ADLATCH)
			cpustate->adrcntr++;

		/* Latch data for a RAM write (do actual write on the next cycle) */
		if (!_ipwrt)
			cpustate->ramwlatch = cpustate->y;

		/* Save level sensitive bits */
		cpustate->prev_ipram = _ipram;
		cpustate->prev_ipwrt = _ipwrt;

		cpustate->icount--;
	} while (cpustate->icount > 0);
}


/***************************************************************************
    ROTATE CORE EXECUTION LOOP
***************************************************************************/

#define ROT_PC          (cpustate->pc & 0x1ff)

enum rot_spf
{
	SPF_UNUSED0 = 0,
	SPF_UNUSED1 = 1,
	SPF_OP      = 2,
	SPF_RET     = 3,
	SPF_SQLTCH  = 4,
	SPF_SWRT    = 5,
	SPF_DIV     = 6,
	SPF_MULT    = 7,
	SPF_DRED    = 8,
	SPF_DWRT    = 9,
};

enum rot_yout
{
	YOUT_UNUSED0 = 0,
	YOUT_UNUSED1 = 1,
	YOUT_Y2LDA   = 2,
	YOUT_Y2LDD   = 3,
	YOUT_Y2DAD   = 4,
	YOUT_Y2DYN   = 5,
	YOUT_Y2R     = 6,
	YOUT_Y2D     = 7,
};

/* Sync is asserted for the duration of every fourth cycle */
/* The Dynamic RAM latch clocks in a value at the end of this cycle */
/* So CPU waits for sync before reading from DRAM */

INLINE int do_rotjmp(cquestrot_state *cpustate, int jmp)
{
	int ret = 0;

	switch (jmp & 7)
	{
		/*        */ case 0: ret = 0;                         break;
		/* SEQ    */ case 1: ret = (cpustate->seqcnt == 0xf); break;
		/* CAROUT */ case 2: ret = cpustate->cflag;           break;
		/* SYNC   */ case 3: ret = !(cpustate->clkcnt & 0x3); break;
		/* LDWAIT */ case 4: ret = 0;                         break;
		/* MSB    */ case 5: ret = BIT(cpustate->f, 15);      break;
		/* >=1    */ case 6: ret = (!_BIT(cpustate->f, 15) && !(cpustate->f == 0)); break;
		/* ZERO   */ case 7: ret = (cpustate->f == 0);        break;
	}

	return !(!ret ^ BIT(jmp, 3));
}


#define ROT_SRAM_ADDRESS    ((cpustate->dsrclatch & 2) ? cpustate->yrlatch : (cpustate->rsrclatch | 0x700))


static CPU_EXECUTE( cquestrot )
{
	cquestrot_state *cpustate = get_safe_token_rot(device);
	cquestlin_state *lincpustate = get_safe_token_lin(cpustate->lindevice);
	int calldebugger = ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	/* Core execution loop */
	do
	{
		/* Decode the instruction */
		UINT64 inst = cpustate->direct->read_decrypted_qword(ROT_PC << 3);

		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 20) & 0xfff;
		int jmp     = (inshig >> 16) & 0xf;
		int spf     = (inshig >> 12) & 0xf;
		int rsrc    = (inshig >> 11) & 0x1;
		int yout    = (inshig >> 8) & 0x7;
		int sel     = (inshig >> 6) & 0x3;
		int dsrc    = (inshig >> 4) & 0x3;
		int b       = (inshig >> 0) & 0xf;
		int a       = (inslow >> 28) & 0xf;
		int i8_6    = (inslow >> 24) & 0x7;
		int ci      = (inslow >> 23) & 0x1;
		int i5_3    = (inslow >> 20) & 0x7;
		int _sex    = (inslow >> 19) & 0x1;
		int i2_0    = (inslow >> 16) & 0x7;

		int dsrclatch;
		UINT16 data_in = 0xffff;

		if (calldebugger)
			debugger_instruction_hook(device, ROT_PC);

		/* Handle DRAM accesses - I ought to check this... */
		if (!(cpustate->clkcnt & 3))
		{
			if (cpustate->wc)
			{
				cpustate->wc = 0;
				cpustate->dram[cpustate->dynaddr & 0x3fff] = cpustate->dyndata;
			}
			if (cpustate->rc)
			{
				cpustate->rc = 0;
				cpustate->dinlatch = cpustate->dram[cpustate->dynaddr & 0x3fff];
			}
		}

		/* Flag pending DRAM accesses */
		if (!cpustate->prev_dwrt)
			cpustate->wc = 1;
		else if (!cpustate->prev_dred)
			cpustate->rc = 1;

		/* What's on the D-Bus? */
		if (~cpustate->dsrclatch & 0x10)
			data_in = cpustate->dinlatch;
		else if (~cpustate->dsrclatch & 0x20)
			data_in = cpustate->sram[ROT_SRAM_ADDRESS];
		else if (~cpustate->dsrclatch & 0x40)
			data_in = cpustate->ydlatch;
		else if (~cpustate->dsrclatch & 0x80)
			data_in = t & 0xfff;

		/* What's on the T-Bus? */
		if ((spf == SPF_RET) && (cpustate->dsrclatch & 0x80))
			t = data_in;
		else if (spf == SPF_OP)
			t = (t & ~0xf) | (data_in >> 12);


		if (~cpustate->dsrclatch & 1)
			cpustate->sram[ROT_SRAM_ADDRESS] = data_in;


		/* Sign extend ALU input? */
		if (!_sex)
			data_in = (data_in & ~0xf000) | ((data_in & 0x800) ? 0xf000 : 0);

		/* No do the ALU operation */
		{
			UINT16 r = 0;
			UINT16 s = 0;

			UINT32 res = 0;
			UINT32 cflag = 0;
			UINT32 vflag = 0;

			/* First, determine correct I1 bit */
			if ((spf == SPF_MULT) && !_BIT(cpustate->q, 0))
				i2_0 |= 2;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case 0: r = cpustate->ram[a]; s = cpustate->q;      break;
				case 1: r = cpustate->ram[a]; s = cpustate->ram[b]; break;
				case 2: r = 0;                s = cpustate->q;      break;
				case 3: r = 0;                s = cpustate->ram[b]; break;
				case 4: r = 0;                s = cpustate->ram[a]; break;
				case 5: r = data_in;          s = cpustate->ram[a]; break;
				case 6: r = data_in;          s = cpustate->q;      break;
				case 7: r = data_in;          s = 0;                break;
			}

			/* Next, determine the I3 and carry bits */
			if ((spf == SPF_DIV) && cpustate->divreg)
			{
				i5_3 |= 1;
				ci = 1;
			}

			/* Perform the ALU operation */
			switch (i5_3)
			{
				case ADD:
					res = r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBR:
					res = ~r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((s & 0x7fff) + (~r & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBS:
					res = r + ~s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (~s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case OR:
					res = r | s;
					break;
				case AND:
					res = r & s;
					break;
				case NOTRS:
					res = ~r & s;
					break;
				case EXOR:
					res = r ^ s;
					break;
				case EXNOR:
					res = ~(r ^ s);
					break;
			}

			cpustate->f = res;
			cpustate->cflag = cflag;
			cpustate->vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					cpustate->q = cpustate->f;
					cpustate->y = cpustate->f;
					break;
				case NOP:
					cpustate->y = cpustate->f;
					break;
				case RAMA:
					cpustate->y = cpustate->ram[a];
					cpustate->ram[b] = cpustate->f;
					break;
				case RAMF:
					cpustate->ram[b] = cpustate->f;
					cpustate->y = cpustate->f;
					break;
				case RAMQD:
				{
					UINT16 q0 = cpustate->q & 1;
					UINT16 r0 = cpustate->f & 1;
					UINT16 q15 = 0;
					UINT16 r15 = 0;

					/* Determine Q15 and RAM15 */
					switch (sel)
					{
						case 0: q15 = r15 = 0;
								break;
						case 1: q15 = r15 = 0x8000;
								break;
						case 2: q15 = q0 << 15;
								r15 = r0 << 15;
								break;
						case 3: q15 = r0 << 15;
								r15 = (cpustate->vflag ^ BIT(cpustate->f, 15)) << 15;
								break;
					}

					cpustate->ram[b] = r15 | (cpustate->f >> 1);
					cpustate->q = q15 | (cpustate->q >> 1);
					cpustate->y = cpustate->f;
					break;
				}
				case RAMD:
				{
					UINT16 r0 = cpustate->f & 1;
					UINT16 r15 = 0;

					switch (sel)
					{
						case 0: r15 = 0;        break;
						case 1: r15 = 0x8000;   break;
						case 2: r15 = r0 << 15; break;
						case 3:
							r15 = (cpustate->vflag ^ BIT(cpustate->f, 15)) << 15;
							break;
					}

					cpustate->ram[b] = r15 | (cpustate->f >> 1);
					cpustate->y = cpustate->f;
					break;
				}
				case RAMQU:
				{
					UINT16 q15 = BIT(cpustate->q, 15);
					UINT16 r15 = BIT(cpustate->f, 15);
					UINT16 q0 = 0;
					UINT16 r0 = 0;

					switch (sel)
					{
						case 0: q0 = 0; r0 = 0;     break;
						case 1: q0 = 1; r0 = 1;     break;
						case 2: q0 = q15; r0 = r15; break;
						case 3:
						{
							q0 = (spf == SPF_DIV) && !BIT(cpustate->f, 15);
							r0 = q15;
							break;
						}
					}

					cpustate->ram[b] = (cpustate->f << 1) | r0;
					cpustate->q = (cpustate->q << 1) | q0;
					cpustate->y = cpustate->f;
					break;
				}
				case RAMU:
				{
					UINT16 q15 = BIT(cpustate->q, 15);
					UINT16 r15 = BIT(cpustate->f, 15);
					UINT16 r0 = 0;

					switch (sel)
					{
						case 0: r0 = 0;     break;
						case 1: r0 = 1;     break;
						case 2: r0 = r15;   break;
						case 3: r0 = q15;   break;
					}

					cpustate->ram[b] = (cpustate->f << 1) | r0;
					cpustate->y = cpustate->f;
					break;
				}
			}
		}

		/* Check for jump */
		if ( do_rotjmp(cpustate, jmp) )
			cpustate->pc = t;
		else
			cpustate->pc = (cpustate->pc + 1) & 0xfff;

		/* Rising edge; update the sequence counter */
		if (spf == SPF_SQLTCH)
			cpustate->seqcnt = t & 0xf;
		else if ( (spf == SPF_MULT) || (spf == SPF_DIV) )
			cpustate->seqcnt = (cpustate->seqcnt + 1) & 0xf;

		/* Rising edge; write data source reg */
		dsrclatch =
				(~(0x10 << dsrc) & 0xf0)
				| (rsrc ? 0x04 : 0x02)
				| !(spf == SPF_SWRT);

		/* R-latch is written on rising edge of dsrclatch bit 2 */
		if (!_BIT(cpustate->dsrclatch, 2) && _BIT(dsrclatch, 2))
			cpustate->rsrclatch = t & 0xff;

		cpustate->dsrclatch = dsrclatch;

		/* Handle latching on rising edge */
		switch (yout)
		{
			case YOUT_Y2LDA:
			{
				cpustate->lineaddr = cpustate->y & 0xfff;
				break;
			}
			case YOUT_Y2LDD:
			{
				cpustate->linedata = ((t & 0xf) << 12) | (cpustate->y & 0xfff);
				lincpustate->sram[cpustate->lineaddr] = cpustate->linedata;
				break;
			}
			case YOUT_Y2DAD: cpustate->dynaddr = cpustate->y & 0x3fff;  break;
			case YOUT_Y2DYN: cpustate->dyndata = cpustate->y & 0xffff;  break;
			case YOUT_Y2R:   cpustate->yrlatch = cpustate->y & 0x7ff;   break;
			case YOUT_Y2D:   cpustate->ydlatch = cpustate->y;           break;
		}

		/* Clock in the divide register */
		cpustate->divreg = (spf == SPF_DIV) && !_BIT(cpustate->f, 15);

		/* DRAM accessing */
		cpustate->prev_dred = !(spf == SPF_DRED);
		cpustate->prev_dwrt = !(spf == SPF_DWRT);

		cpustate->clkcnt++;
		cpustate->icount--;
	} while (cpustate->icount > 0);
}


/***************************************************************************
    LINE DRAWER CORE EXECUTION LOOP
***************************************************************************/

#define VISIBLE_FIELD   !cpustate->field

enum line_spf
{
	LSPF_UNUSED  = 0,
	LSPF_FSTOP   = 1,
	LSPF_SREG    = 2,
	LSPF_FSTRT   = 3,
	LSPF_PWRT    = 4,
	LSPF_MULT    = 5,
	LSPF_LSTOP   = 6,
	LSPF_BRES    = 7,
};

enum line_latch
{
	LLATCH_UNUSED   = 0,
	LLATCH_SEQLATCH = 1,
	LLATCH_XLATCH   = 2,
	LLATCH_YLATCH   = 3,
	LLATCH_BADLATCH = 4,
	LLATCH_FADLATCH = 5,
	LLATCH_CLATCH   = 6,
	LLATCH_ZLATCH   = 7,
};

enum sreg_bits
{
	SREG_E0     = 0,
	SREG_DX_DY  = 1,
	SREG_DY     = 2,
	SREG_DX     = 3,
	SREG_LE0    = 4,
	SREG_LDX_DY = 5,
	SREG_LDY    = 6,
	SREG_LDX    = 7,
};

INLINE int do_linjmp(cquestlin_state *cpustate, int jmp)
{
	int ret = 0;

	switch (jmp & 7)
	{
		/*        */ case 0: ret = 0; break;
		/* MSB    */ case 1: ret = BIT(cpustate->f, 11); break;
		/* SEQ    */ case 2: ret = (cpustate->seqcnt == 0xfff); break;
		/* >0     */ case 3: ret = !(cpustate->f == 0) && !_BIT(cpustate->f, 11); break;
		/* CAROUT */ case 4: ret = (cpustate->cflag); break;
		/* ZERO   */ case 5: ret = (cpustate->f == 0); break;
	}

	return !(!ret ^ BIT(jmp, 3));
}



void cubeqcpu_swap_line_banks(device_t *device)
{
	cquestlin_state *cpustate = get_safe_token_lin(device);
	cpustate->field = cpustate->field ^ 1;
}


void cubeqcpu_clear_stack(device_t *device)
{
	cquestlin_state *cpustate = get_safe_token_lin(device);
	memset(&cpustate->ptr_ram[cpustate->field * 256], 0, 256);
}

UINT8 cubeqcpu_get_ptr_ram_val(device_t *device, int i)
{
	cquestlin_state *cpustate = get_safe_token_lin(device);
	return cpustate->ptr_ram[(VISIBLE_FIELD * 256) + i];
}

UINT32* cubeqcpu_get_stack_ram(device_t *device)
{
	cquestlin_state *cpustate = get_safe_token_lin(device);
	if (VISIBLE_FIELD == ODD_FIELD)
		return cpustate->o_stack;
	else
		return cpustate->e_stack;
}


static CPU_EXECUTE( cquestlin )
{
#define LINE_PC ((cpustate->pc[prog] & 0x7f) | ((prog == BACKGROUND) ? 0x80 : 0))

	cquestlin_state *cpustate = get_safe_token_lin(device);
	cquestrot_state *rotcpustate = get_safe_token_rot(cpustate->rotdevice);
	int calldebugger = ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);
	UINT32  *stack_ram;
	UINT8   *ptr_ram;

	/* Check the field and set the stack/pointer RAM pointers appropriately */
	if (cpustate->field == ODD_FIELD)
	{
		stack_ram = cpustate->o_stack;
		ptr_ram = &cpustate->ptr_ram[0];
	}
	else
	{
		stack_ram = cpustate->e_stack;
		ptr_ram = &cpustate->ptr_ram[0x100];
	}

	/* Core execution loop */
	do
	{
		/* Are we executing the foreground or backgroud program? */
		int prog = (cpustate->clkcnt & 3) ? BACKGROUND : FOREGROUND;

		UINT64 inst = cpustate->direct->read_decrypted_qword(LINE_PC << 3);

		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 24) & 0xff;
		int jmp     = (inshig >> 20) & 0xf;
		int latch   = (inshig >> 16) & 0x7;
		int op      = (inshig >> 15) & 0x1;
		int spf     = (inshig >> 12) & 0x7;
		int b       = (inshig >> 8) & 0xf;
		int a       = (inshig >> 4) & 0xf;
		int i8_6    = (inshig >> 0) & 0x7;
		int ci      = (inslow >> 31) & 0x1;
		int i5_3    = (inslow >> 28) & 0x7;
		int _pbcs   = (inslow >> 27) & 0x1;
		int i2_0    = (inslow >> 24) & 0x7;

		UINT16  data_in = 0;

		if (calldebugger)
			debugger_instruction_hook(device, cpustate->pc[prog]);

		/* Handle accesses to and from shared SRAM */
		if (prog == FOREGROUND)
		{
			if (!_BIT(cpustate->fglatch, 5))
				data_in = cpustate->sram[cpustate->fadlatch];
			else
				data_in = rotcpustate->linedata;
		}
		else
		{
			if (!_BIT(cpustate->bglatch, 4))
				cpustate->sram[cpustate->badlatch] = cpustate->sramdlatch;
			else if (_BIT(cpustate->bglatch, 2))
				data_in = cpustate->sram[cpustate->badlatch];
			else
				data_in = rotcpustate->linedata;
		}

		/* Handle a write to stack RAM (/DOWRT) */
		if ((cpustate->clkcnt & 3) == 1)
		{
			if (_BIT(cpustate->fglatch, 4) && (cpustate->ycnt < 256))
			{
				/* 20-bit words */
				UINT32 data;
				UINT16 h = cpustate->xcnt;
				UINT8 v = cpustate->ycnt & 0xff;

				/* Clamp H between 0 and 319 */
				if (h >= 320)
					h = (h & 0x800) ? 0 : 319;

				/* Stack word type depends on STOP/#START bit */
				if ( _BIT(cpustate->fglatch, 3) )
					data = (0 << 19) | (h << 8) | cpustate->zlatch;
				else
					data = (1 << 19) | ((cpustate->clatch & 0x100) << 9) | (h << 8) | (cpustate->clatch & 0xff);

				stack_ram[(v << 7) | (ptr_ram[v] & 0x7f)] = data;

				/* Also increment the pointer RAM entry. Note that it cannot exceed 128 */
				ptr_ram[v] = (ptr_ram[v] + 1) & 0x7f;
			}
		}

		/* Override T3-0? */
		if (op)
			t = (t & ~0xf) | (data_in >> 12);

		/* Determine the correct I1 bit  */
		if ((spf == LSPF_MULT) && !_BIT(cpustate->q, 0))
			i2_0 |= 2;

		/* Determine A0 (BRESA0) */
		if ((prog == FOREGROUND) && !_BIT(cpustate->fglatch, 2))
			a |= cpustate->gt0reg;

		/* Now do the ALU operation */
		{
			UINT16 r = 0;
			UINT16 s = 0;

			UINT16 res = 0;
			UINT32 cflag = 0;
			UINT32 vflag = 0;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case 0: r = cpustate->ram[a];   s = cpustate->q;      break;
				case 1: r = cpustate->ram[a];   s = cpustate->ram[b]; break;
				case 2: r = 0;                  s = cpustate->q;      break;
				case 3: r = 0;                  s = cpustate->ram[b]; break;
				case 4: r = 0;                  s = cpustate->ram[a]; break;
				case 5: r = data_in;            s = cpustate->ram[a]; break;
				case 6: r = data_in;            s = cpustate->q;      break;
				case 7: r = data_in;            s = 0;                break;
			}

			/* 12-bits */
			r &= 0xfff;
			s &= 0xfff;

			/* Perform the 12-bit ALU operation */
			switch (i5_3)
			{
				case ADD:
					res = r + s + ci;
					cflag = (res >> 12) & 1;
					vflag = (((r & 0x7ff) + (s & 0x7ff) + ci) >> 11) ^ cflag;
					break;
				case SUBR:
					res = (r ^ 0x0FFF) + s + ci;
					cflag = (res >> 12) & 1;
					vflag = (((s & 0x7ff) + (~r & 0x7ff) + ci) >> 11) ^ cflag;
					break;
				case SUBS:
					res = r + (s ^ 0x0FFF) + ci;
					cflag = (res >> 12) & 1;
					vflag = (((r & 0x7ff) + (~s & 0x7ff) + ci) >> 11) ^ cflag;
					break;
				case OR:
					res = r | s;
					break;
				case AND:
					res = r & s;
					break;
				case NOTRS:
					res = ~r & s;
					break;
				case EXOR:
					res = r ^ s;
					break;
				case EXNOR:
					res = ~(r ^ s);
					break;
			}

			cpustate->f = res & 0xfff;
			cpustate->cflag = cflag;
			cpustate->vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					cpustate->q = cpustate->f;
					cpustate->y = cpustate->f;
					break;
				case NOP:
					cpustate->y = cpustate->f;
					break;
				case RAMA:
					cpustate->y = cpustate->ram[a];
					cpustate->ram[b] = cpustate->f;
					break;
				case RAMF:
					cpustate->ram[b] = cpustate->f;
					cpustate->y = cpustate->f;
					break;
				case RAMQD:
				{
					UINT16 r11 = (BIT(cpustate->f, 11) ^ cpustate->vflag) ? 0x800 : 0;
					UINT16 q11 = (prog == BACKGROUND) ? 0x800 : 0;

					cpustate->ram[b] = r11 | (cpustate->f >> 1);
					cpustate->q = q11 | (cpustate->q >> 1);
					cpustate->y = cpustate->f;
					break;
				}
				case RAMD:
				{
					UINT16 r11 = (BIT(cpustate->f, 11) ^ cpustate->vflag) ? 0x800 : 0;

					cpustate->ram[b] = r11 | (cpustate->f >> 1);
					cpustate->y = cpustate->f;
					break;
				}
				case RAMQU:
				{
					/* Determine shift inputs */
					UINT16 r0 = (prog == BACKGROUND);

					/* This should never happen - Q0 will be invalid */
					cpustate->ram[b] = (cpustate->f << 1) | r0;
					cpustate->q = (cpustate->q << 1) | 0;
					cpustate->y = cpustate->f;
					break;
				}
				case RAMU:
				{
					UINT16 r0 = (prog == BACKGROUND);

					cpustate->ram[b] = (cpustate->f << 1) | r0;
					cpustate->y = cpustate->f;
					break;
				}
			}
		}

		/* Adjust program counter */
		if ( do_linjmp(cpustate, jmp) )
			cpustate->pc[prog] = t & 0x7f;
		else
			cpustate->pc[prog] = (cpustate->pc[prog] + 1) & 0x7f;

		if (prog == BACKGROUND)
			cpustate->pc[prog] |= 0x80;
		else
		{
			/* Handle events that happen during FG execution */
			if (latch == LLATCH_XLATCH)
				cpustate->xcnt = cpustate->y & 0xfff;
			else
			{
				int _xcet;
				int mux_sel = (BIT(cpustate->sreg, SREG_DX_DY) << 1) | (BIT(cpustate->sreg, SREG_DX) ^ BIT(cpustate->sreg, SREG_DY));

				if (mux_sel == 0)
					_xcet = !(spf == LSPF_BRES);
				else if (mux_sel == 1)
					_xcet = _BIT(cpustate->fglatch, 1);
				else if (mux_sel == 2)
					_xcet = !(cpustate->gt0reg && (spf == LSPF_BRES));
				else
					_xcet = _BIT(cpustate->fglatch, 0);

				if (!_xcet)
					cpustate->xcnt = (cpustate->xcnt + (_BIT(cpustate->sreg, SREG_DX) ? 1 : -1)) & 0xfff;
			}

			if (latch == LLATCH_YLATCH)
				cpustate->ycnt = cpustate->y & 0xfff;
			else
			{
				int _ycet;
				int mux_sel = (BIT(cpustate->sreg, SREG_DX_DY) << 1) | (BIT(cpustate->sreg, SREG_DX) ^ BIT(cpustate->sreg, SREG_DY));

				if (mux_sel == 0)
					_ycet = !(cpustate->gt0reg && (spf == LSPF_BRES));
				else if (mux_sel == 1)
					_ycet = _BIT(cpustate->fglatch, 0);
				else if (mux_sel == 2)
					_ycet = !(spf == LSPF_BRES);
				else
					_ycet = _BIT(cpustate->fglatch, 1);

				if (!_ycet)
					cpustate->ycnt = (cpustate->ycnt + (_BIT(cpustate->sreg, SREG_DY) ? 1 : -1)) & 0xfff;
			}
		}

		if (latch == LLATCH_CLATCH)
			cpustate->clatch = cpustate->y & 0x1ff;
		else if (latch == LLATCH_ZLATCH)
			cpustate->zlatch = cpustate->y & 0xff;
		else if (latch == LLATCH_FADLATCH)
			cpustate->fadlatch = cpustate->y & 0xfff;
		else if (latch == LLATCH_BADLATCH)
			cpustate->badlatch = cpustate->y & 0xfff;

		/* What about the SRAM dlatch? */
		if ( !_BIT(cpustate->bglatch, 5) )
			cpustate->sramdlatch = ((t & 0xf) << 12) | (cpustate->y & 0x0fff);

		/* BG and FG latches */
		if (prog == FOREGROUND)
		{
			int mux_sel = (!(spf == LSPF_FSTOP) << 1) | !(spf == LSPF_LSTOP);
			int dowrt;
			int start_stop;

			/* Handle the stack write and start/stop mux */
			if (mux_sel == 0)
			{
				dowrt = 0;
				start_stop = 0;
			}
			else if (mux_sel == 1)
			{
				dowrt = cpustate->fdxreg ^ BIT(cpustate->sreg, SREG_DX);
				start_stop = cpustate->fdxreg;
			}
			else if (mux_sel == 2)
			{
				dowrt = BIT(cpustate->sreg, SREG_LDX) ^ BIT(cpustate->sreg, SREG_DX);
				start_stop = BIT(cpustate->sreg, SREG_DX);
			}
			else
			{
				dowrt = (spf == LSPF_BRES) && (_BIT(cpustate->sreg, SREG_DX_DY) || cpustate->gt0reg);
				start_stop = BIT(cpustate->sreg, SREG_DY);
			}

			cpustate->fglatch =
					(!(latch == LLATCH_FADLATCH) << 5)
					| (dowrt << 4)
					| (start_stop << 3)
					| (_pbcs << 2)
					| (!(spf == LSPF_BRES) << 1)
					| !(cpustate->gt0reg && (spf == LSPF_BRES));
		}
		else
		{
			int _lpwrt = BIT(cpustate->bglatch, 5);

			cpustate->bglatch =
					(!(spf == LSPF_PWRT) << 5)
					| (_lpwrt << 4)
					| ((!_lpwrt || (!(spf == LSPF_PWRT) && (latch == LLATCH_BADLATCH))) << 2);
		}

		/* Clock-in another bit into the sign bit shifter? */
		if (spf == LSPF_SREG)
		{
			/* The sign bit is inverted */
			cpustate->sreg = (cpustate->sreg << 1) | !BIT(cpustate->f, 11);

			/* Also latch the >0 reg */
			cpustate->gt0reg = !(cpustate->f == 0) && !_BIT(cpustate->f, 11);
		}
		else if (spf == LSPF_FSTRT)
		{
			cpustate->fdxreg = BIT(cpustate->sreg, 3);
		}

		/* Load or increment sequence counter? */
		if (latch == LLATCH_SEQLATCH)
		{
			cpustate->seqcnt = cpustate->y & 0xfff;
		}
		else if (spf == LSPF_BRES)
		{
			cpustate->seqcnt = (cpustate->seqcnt + 1) & 0xfff;

			/* Also latch the >0 reg */
			cpustate->gt0reg = !(cpustate->f == 0) && !_BIT(cpustate->f, 11);
		}

		cpustate->icount--;
		cpustate->clkcnt++;
	} while (cpustate->icount > 0);
}


/**************************************************************************
 * Sound set_info
 **************************************************************************/

static CPU_SET_INFO( cquestsnd )
{
	cquestsnd_state *cpustate = get_safe_token_snd(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTSND_PC:       cpustate->pc = info->i;             break;
		case CPUINFO_INT_REGISTER + CQUESTSND_Q:        cpustate->q = info->i;              break;
		case CPUINFO_INT_REGISTER + CQUESTSND_RTNLATCH: cpustate->rtnlatch = info->i;       break;
		case CPUINFO_INT_REGISTER + CQUESTSND_ADRCNTR:  cpustate->adrcntr = info->i;        break;
		case CPUINFO_STR_REGISTER + CQUESTSND_DINLATCH: cpustate->dinlatch = info->i;       break;

		case CPUINFO_STR_REGISTER + CQUESTSND_RAM0:  cpustate->ram[0x0] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM1:  cpustate->ram[0x1] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM2:  cpustate->ram[0x2] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM3:  cpustate->ram[0x3] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM4:  cpustate->ram[0x4] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM5:  cpustate->ram[0x5] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM6:  cpustate->ram[0x6] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM7:  cpustate->ram[0x7] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM8:  cpustate->ram[0x8] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM9:  cpustate->ram[0x9] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMA:  cpustate->ram[0xa] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMB:  cpustate->ram[0xb] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMC:  cpustate->ram[0xc] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMD:  cpustate->ram[0xd] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAME:  cpustate->ram[0xe] = info->i;          break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMF:  cpustate->ram[0xf] = info->i;          break;
	}
}

/**************************************************************************
 * Sound get_info
 **************************************************************************/

CPU_GET_INFO( cquestsnd )
{
	cquestsnd_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token_snd(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(cquestsnd_state);      break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;               break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 8;                            break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 8;                            break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                            break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 1;                            break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 8;                   break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -3;                  break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                    break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTSND_PC:           info->i = cpustate->pc;             break;
		case CPUINFO_INT_REGISTER + CQUESTSND_RTNLATCH:     info->i = cpustate->rtnlatch;       break;
		case CPUINFO_INT_REGISTER + CQUESTSND_ADRCNTR:      info->i = cpustate->adrcntr;        break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(cquestsnd);       break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(cquestsnd);          break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(cquestsnd);            break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(cquestsnd);          break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(cquestsnd);        break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                      break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(cquestsnd);        break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &cpustate->icount;       break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "Sound CPU");break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Cube Quest");          break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "1.0");                 break;
		case CPUINFO_STR_SOURCE_FILE:                       strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:                         sprintf(info->s, ".......");             break;
		case CPUINFO_STR_REGISTER + CQUESTSND_PC:       sprintf(info->s, "PC:  %02X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_Q:        sprintf(info->s, "Q:   %04X", cpustate->q); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RTNLATCH: sprintf(info->s, "RTN: %02X", cpustate->rtnlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_ADRCNTR:  sprintf(info->s, "CNT: %02X", cpustate->adrcntr); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_DINLATCH: sprintf(info->s, "DIN: %04X", cpustate->dinlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM0:     sprintf(info->s, "RAM[0]: %04X", cpustate->ram[0x0]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM1:     sprintf(info->s, "RAM[1]: %04X", cpustate->ram[0x1]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM2:     sprintf(info->s, "RAM[2]: %04X", cpustate->ram[0x2]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM3:     sprintf(info->s, "RAM[3]: %04X", cpustate->ram[0x3]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM4:     sprintf(info->s, "RAM[4]: %04X", cpustate->ram[0x4]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM5:     sprintf(info->s, "RAM[5]: %04X", cpustate->ram[0x5]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM6:     sprintf(info->s, "RAM[6]: %04X", cpustate->ram[0x6]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM7:     sprintf(info->s, "RAM[7]: %04X", cpustate->ram[0x7]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM8:     sprintf(info->s, "RAM[8]: %04X", cpustate->ram[0x8]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM9:     sprintf(info->s, "RAM[9]: %04X", cpustate->ram[0x9]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMA:     sprintf(info->s, "RAM[A]: %04X", cpustate->ram[0xa]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMB:     sprintf(info->s, "RAM[B]: %04X", cpustate->ram[0xb]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMC:     sprintf(info->s, "RAM[C]: %04X", cpustate->ram[0xc]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMD:     sprintf(info->s, "RAM[D]: %04X", cpustate->ram[0xd]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAME:     sprintf(info->s, "RAM[E]: %04X", cpustate->ram[0xe]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMF:     sprintf(info->s, "RAM[F]: %04X", cpustate->ram[0xf]); break;
	}
}


/**************************************************************************
 * Rotate set_info
 **************************************************************************/

static CPU_SET_INFO( cquestrot )
{
	cquestrot_state *cpustate = get_safe_token_rot(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTROT_PC:   cpustate->pc = info->i;                 break;
		case CPUINFO_INT_REGISTER + CQUESTROT_Q:    cpustate->q = info->i;                  break;

		case CPUINFO_STR_REGISTER + CQUESTROT_RAM0:     cpustate->ram[0x0] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM1:     cpustate->ram[0x1] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM2:     cpustate->ram[0x2] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM3:     cpustate->ram[0x3] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM4:     cpustate->ram[0x4] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM5:     cpustate->ram[0x5] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM6:     cpustate->ram[0x6] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM7:     cpustate->ram[0x7] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM8:     cpustate->ram[0x8] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM9:     cpustate->ram[0x9] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMA:     cpustate->ram[0xa] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMB:     cpustate->ram[0xb] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMC:     cpustate->ram[0xc] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMD:     cpustate->ram[0xd] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAME:     cpustate->ram[0xe] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMF:     cpustate->ram[0xf] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTROT_SEQCNT:   cpustate->seqcnt    = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNADDR:  cpustate->dynaddr   = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNDATA:  cpustate->dyndata   = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YRLATCH:  cpustate->yrlatch   = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YDLATCH:  cpustate->ydlatch   = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DINLATCH: cpustate->dinlatch  = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DSRCLATCH:cpustate->dsrclatch = info->i;      break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RSRCLATCH:cpustate->rsrclatch = info->i;      break;
	}
}

/**************************************************************************
 * Rotate get_info
 **************************************************************************/

CPU_GET_INFO( cquestrot )
{
	cquestrot_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token_rot(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(cquestrot_state);      break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;               break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 8;                            break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 8;                            break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                            break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 1;                            break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 9;                   break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -3;                  break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                    break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTROT_PC:       info->i = cpustate->pc;                 break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(cquestrot);       break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(cquestrot);          break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(cquestrot);            break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(cquestrot);          break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(cquestrot);        break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                      break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(cquestrot);        break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &cpustate->icount;       break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "Rotate CPU");break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Cube Quest");          break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "1.0");                 break;
		case CPUINFO_STR_SOURCE_FILE:                       strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:                         sprintf(info->s, "%c%c%c", cpustate->cflag ? 'C' : '.',
																					cpustate->vflag ? 'V' : '.',
																					cpustate->f ? '.' : 'Z');   break;
		case CPUINFO_STR_REGISTER + CQUESTROT_PC:       sprintf(info->s, "PC:  %02X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_Q:        sprintf(info->s, "Q:   %04X", cpustate->q); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM0:     sprintf(info->s, "RAM[0]: %04X", cpustate->ram[0x0]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM1:     sprintf(info->s, "RAM[1]: %04X", cpustate->ram[0x1]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM2:     sprintf(info->s, "RAM[2]: %04X", cpustate->ram[0x2]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM3:     sprintf(info->s, "RAM[3]: %04X", cpustate->ram[0x3]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM4:     sprintf(info->s, "RAM[4]: %04X", cpustate->ram[0x4]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM5:     sprintf(info->s, "RAM[5]: %04X", cpustate->ram[0x5]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM6:     sprintf(info->s, "RAM[6]: %04X", cpustate->ram[0x6]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM7:     sprintf(info->s, "RAM[7]: %04X", cpustate->ram[0x7]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM8:     sprintf(info->s, "RAM[8]: %04X", cpustate->ram[0x8]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM9:     sprintf(info->s, "RAM[9]: %04X", cpustate->ram[0x9]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMA:     sprintf(info->s, "RAM[A]: %04X", cpustate->ram[0xa]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMB:     sprintf(info->s, "RAM[B]: %04X", cpustate->ram[0xb]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMC:     sprintf(info->s, "RAM[C]: %04X", cpustate->ram[0xc]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMD:     sprintf(info->s, "RAM[D]: %04X", cpustate->ram[0xd]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAME:     sprintf(info->s, "RAM[E]: %04X", cpustate->ram[0xe]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMF:     sprintf(info->s, "RAM[F]: %04X", cpustate->ram[0xf]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_SEQCNT:   sprintf(info->s, "SEQCNT: %01X", cpustate->seqcnt); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNADDR:  sprintf(info->s, "DYNADDR: %04X", cpustate->dynaddr); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNDATA:  sprintf(info->s, "DYNDATA: %04X", cpustate->dyndata); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YRLATCH:  sprintf(info->s, "YRLATCH: %04X", cpustate->yrlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YDLATCH:  sprintf(info->s, "YDLATCH: %04X", cpustate->ydlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DINLATCH: sprintf(info->s, "DINLATCH: %04X", cpustate->dinlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DSRCLATCH:sprintf(info->s, "DSRCLATCH: %04X", cpustate->dsrclatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RSRCLATCH:sprintf(info->s, "RSRCLATCH: %04X", cpustate->rsrclatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_LDADDR:   sprintf(info->s, "LDADDR : %04X", cpustate->lineaddr); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_LDDATA:   sprintf(info->s, "LDDATA : %04X", cpustate->linedata); break;
	}
}


/**************************************************************************
 * Line drawer set_info
 **************************************************************************/

static CPU_SET_INFO( cquestlin )
{
	cquestlin_state *cpustate = get_safe_token_lin(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTLIN_FGPC: cpustate->pc[FOREGROUND] = info->i;     break;
		case CPUINFO_INT_REGISTER + CQUESTLIN_BGPC: cpustate->pc[BACKGROUND] = info->i;     break;
		case CPUINFO_INT_REGISTER + CQUESTLIN_Q:    cpustate->q = info->i;                  break;

		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM0:     cpustate->ram[0x0] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM1:     cpustate->ram[0x1] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM2:     cpustate->ram[0x2] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM3:     cpustate->ram[0x3] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM4:     cpustate->ram[0x4] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM5:     cpustate->ram[0x5] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM6:     cpustate->ram[0x6] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM7:     cpustate->ram[0x7] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM8:     cpustate->ram[0x8] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM9:     cpustate->ram[0x9] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMA:     cpustate->ram[0xa] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMB:     cpustate->ram[0xb] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMC:     cpustate->ram[0xc] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMD:     cpustate->ram[0xd] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAME:     cpustate->ram[0xe] = info->i;       break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMF:     cpustate->ram[0xf] = info->i;       break;
	}
}

/**************************************************************************
 * Line drawer get_info
 **************************************************************************/

CPU_GET_INFO( cquestlin )
{
	cquestlin_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token_lin(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(cquestlin_state);      break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;               break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 8;                            break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 8;                            break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                            break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 1;                            break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 8;                   break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -3;                  break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                    break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTLIN_FGPC:     info->i = cpustate->pc[cpustate->clkcnt & 3 ? BACKGROUND : FOREGROUND]; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(cquestlin);       break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(cquestlin);          break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(cquestlin);            break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(cquestlin);          break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(cquestlin);        break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                      break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(cquestlin);        break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &cpustate->icount;       break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "Line CPU");            break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Cube Quest");          break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "1.0");                 break;
		case CPUINFO_STR_SOURCE_FILE:                       strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:                         sprintf(info->s, "%c%c%c|%cG",  cpustate->cflag ? 'C' : '.',
																						cpustate->vflag ? 'V' : '.',
																						cpustate->f ? '.' : 'Z',
																						cpustate->clkcnt & 3 ? 'B' : 'F'); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_FGPC:     sprintf(info->s, "FPC:  %02X", cpustate->pc[FOREGROUND]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_BGPC:     sprintf(info->s, "BPC:  %02X", cpustate->pc[BACKGROUND]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_Q:        sprintf(info->s, "Q:   %04X", cpustate->q); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM0:     sprintf(info->s, "RAM[0]: %04X", cpustate->ram[0x0]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM1:     sprintf(info->s, "RAM[1]: %04X", cpustate->ram[0x1]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM2:     sprintf(info->s, "RAM[2]: %04X", cpustate->ram[0x2]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM3:     sprintf(info->s, "RAM[3]: %04X", cpustate->ram[0x3]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM4:     sprintf(info->s, "RAM[4]: %04X", cpustate->ram[0x4]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM5:     sprintf(info->s, "RAM[5]: %04X", cpustate->ram[0x5]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM6:     sprintf(info->s, "RAM[6]: %04X", cpustate->ram[0x6]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM7:     sprintf(info->s, "RAM[7]: %04X", cpustate->ram[0x7]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM8:     sprintf(info->s, "RAM[8]: %04X", cpustate->ram[0x8]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM9:     sprintf(info->s, "RAM[9]: %04X", cpustate->ram[0x9]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMA:     sprintf(info->s, "RAM[A]: %04X", cpustate->ram[0xa]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMB:     sprintf(info->s, "RAM[B]: %04X", cpustate->ram[0xb]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMC:     sprintf(info->s, "RAM[C]: %04X", cpustate->ram[0xc]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMD:     sprintf(info->s, "RAM[D]: %04X", cpustate->ram[0xd]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAME:     sprintf(info->s, "RAM[E]: %04X", cpustate->ram[0xe]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMF:     sprintf(info->s, "RAM[F]: %04X", cpustate->ram[0xf]); break;

		case CPUINFO_STR_REGISTER + CQUESTLIN_FADLATCH: sprintf(info->s, "FADDR:  %04X", cpustate->fadlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_BADLATCH: sprintf(info->s, "BADDR:  %04X", cpustate->badlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_SREG:     sprintf(info->s, "SREG:   %04X", cpustate->sreg);   break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_XCNT:     sprintf(info->s, "XCNT:   %03X", cpustate->xcnt);   break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_YCNT:     sprintf(info->s, "YCNT:   %03X", cpustate->ycnt);   break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_CLATCH:   sprintf(info->s, "CLATCH: %04X", cpustate->clatch); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_ZLATCH:   sprintf(info->s, "ZLATCH: %04X", cpustate->zlatch); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(CQUESTSND, cquestsnd);
DEFINE_LEGACY_CPU_DEVICE(CQUESTROT, cquestrot);
DEFINE_LEGACY_CPU_DEVICE(CQUESTLIN, cquestlin);
