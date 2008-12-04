/***************************************************************************

    cubeqcpu.c

    Implementation of the Cube Quest AM2901-based CPUs

    TODO:

    * Tidy up diassembly (split into different files?)

***************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "cubeqcpu.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Am2901 Instruction Fields */
static const char *const ins[] =
{
	"ADD  ",
	"SUBR ",
	"SUBS ",
	"OR   ",
	"AND  ",
	"NOTRS",
	"EXOR ",
	"EXNOR",
};

static const char *const src[] =
{
	"A,Q",
	"A,B",
	"0,Q",
	"0,B",
	"0,A",
	"D,A",
	"D,Q",
	"D,0",
};

static const char *const dst[] =
{
	"QREG ",
	"NOP  ",
	"RAMA ",
	"RAMF ",
	"RAMQD",
	"RAMD ",
	"RAMQU",
	"RAMU ",
};

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

#define _BIT(x, n)			((x) & (1 << (n)))

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

typedef struct
{
	/* AM2901 internals */
	UINT16	ram[16];
	UINT16  q;
	UINT16	f;
	UINT16	y;
	UINT32	cflag;
	UINT32	vflag;

	UINT8	pc;         /* 2 x LS161 @ 6E, 6F */
	UINT16	platch;
	UINT8	rtnlatch;   /* LS374 @ 5F */
	UINT8	adrcntr;    /* 2 x LS161 */
	UINT16	adrlatch;
	UINT16	dinlatch;
	UINT16	ramwlatch;

	UINT16 *sram;

	int prev_ipram;
	int prev_ipwrt;

	void (*dac_w)(UINT16 data);
	UINT16 *sound_data;

	const device_config *device;
	const address_space *program;
} cquestsnd_state;


typedef struct
{
	/* AM2901 internals */
	UINT16	ram[16];
	UINT16  q;
	UINT16	f;
	UINT16	y;
	UINT32	cflag;
	UINT32	vflag;

	UINT16	pc;			/* 12-bit, but only 9 used */
	UINT8	seqcnt;		/* 4-bit counter */

	UINT8	dsrclatch;
	UINT8	rsrclatch;
	UINT16	dynaddr;	/* LS374 at 2D, 8D  */
	UINT16	dyndata;	/* LS374 at 10B, 9B */
	UINT16	yrlatch;	/* LS374 at 9D, 10D */
	UINT16	ydlatch;	/* LS374 at 9C, 10C */
	UINT16	dinlatch;
	UINT8	divreg;		/* LS74 at ? */

	UINT16	linedata;
	UINT16	lineaddr;

	UINT16 *dram;
	UINT16 *sram;

	UINT8 prev_dred;
	UINT8 prev_dwrt;
	UINT8 wc;
	UINT8 rc;
	UINT8 clkcnt;

	const device_config *device;
	const address_space *program;
} cquestrot_state;


typedef struct
{
	/* 12-bit AM2901 internals */
	UINT16	ram[16];
	UINT16  q;
	UINT16	f;
	UINT16	y;
	UINT32	cflag;
	UINT32	vflag;

	UINT8	pc[2];		/* Two program counters; one for FG, other for BG */

	UINT16	seqcnt;		/* 12-bit */
	UINT16	clatch;		/* LS374 at 9E and 1-bit FF */
	UINT8	zlatch;		/* LS374 at 4H */

	UINT16	xcnt;
	UINT16	ycnt;
	UINT8	sreg;

	UINT16	fadlatch;
	UINT16	badlatch;

	UINT16	sramdlatch;

	UINT8	fglatch;
	UINT8	bglatch;
	UINT8	gt0reg;
	UINT8	fdxreg;
	UINT32	field;

	UINT32	clkcnt;

	/* RAM */
	UINT16	*sram;
	UINT8	*ptr_ram;
	UINT32	*e_stack;
	UINT32	*o_stack;

	const device_config *device;
	const address_space *program;

} cquestlin_state;

/***************************************************************************
    PUBLIC GLOBAL VARIABLES
***************************************************************************/

static int cquestsnd_icount;
static int cquestrot_icount;
static int cquestlin_icount;

/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static cquestsnd_state cquestsnd;
static cquestrot_state cquestrot;
static cquestlin_state cquestlin;

/***************************************************************************
    MEMORY ACCESSORS FOR 68000
***************************************************************************/

WRITE16_HANDLER( write_sndram )
{
	COMBINE_DATA(&cquestsnd.sram[offset]);
}

READ16_HANDLER( read_sndram )
{
	return cquestsnd.sram[offset];
}


WRITE16_HANDLER( write_rotram )
{
	COMBINE_DATA(&cquestrot.dram[offset]);
}

READ16_HANDLER( read_rotram )
{
	return cquestrot.dram[offset];
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/


/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static CPU_GET_CONTEXT( cquestsnd )
{
	/* Copy the context */
	if (dst)
		*(cquestsnd_state *)dst = cquestsnd;
}


static CPU_SET_CONTEXT( cquestsnd )
{
	/* Copy the context */
	if (src)
		cquestsnd = *(cquestsnd_state *)src;
}


static CPU_GET_CONTEXT( cquestrot )
{
	/* Copy the context */
	if (dst)
		*(cquestrot_state *)dst = cquestrot;
}


static CPU_SET_CONTEXT( cquestrot )
{
	/* Copy the context */
	if (src)
		cquestrot = *(cquestrot_state *)src;
}


static CPU_GET_CONTEXT( cquestlin )
{
	/* Copy the context */
	if (dst)
		*(cquestlin_state *)dst = cquestlin;
}


static CPU_SET_CONTEXT( cquestlin )
{
	/* Copy the context */
	if (src)
		cquestlin = *(cquestlin_state *)src;
}

/***************************************************************************
    SOUND INITIALIZATION AND SHUTDOWN
***************************************************************************/

static STATE_POSTLOAD( cquestsnd_postload )
{

}

static void cquestsnd_state_register(const device_config *device, const char *type)
{
	state_save_register_item_array(type, device->tag, 0, cquestsnd.ram);
	state_save_register_item(type, device->tag, 0, cquestsnd.q);
	state_save_register_item(type, device->tag, 0, cquestsnd.f);
	state_save_register_item(type, device->tag, 0, cquestsnd.y);
	state_save_register_item(type, device->tag, 0, cquestsnd.cflag);
	state_save_register_item(type, device->tag, 0, cquestsnd.vflag);

	state_save_register_item(type, device->tag, 0, cquestsnd.pc);
	state_save_register_item(type, device->tag, 0, cquestsnd.platch);
	state_save_register_item(type, device->tag, 0, cquestsnd.rtnlatch);
	state_save_register_item(type, device->tag, 0, cquestsnd.adrcntr);
	state_save_register_item(type, device->tag, 0, cquestsnd.adrlatch);
	state_save_register_item(type, device->tag, 0, cquestsnd.dinlatch);
	state_save_register_item(type, device->tag, 0, cquestsnd.ramwlatch);
	state_save_register_item(type, device->tag, 0, cquestsnd.prev_ipram);
	state_save_register_item(type, device->tag, 0, cquestsnd.prev_ipwrt);

	state_save_register_postload(device->machine, cquestsnd_postload, (void *)device);
}

static CPU_INIT( cquestsnd )
{
	cubeqst_snd_config* _config = (cubeqst_snd_config*)device->static_config;

	memset(&cquestsnd, 0, sizeof(cquestsnd));

	cquestsnd.dac_w = _config->dac_w;
	cquestsnd.sound_data = (UINT16*)memory_region(device->machine, _config->sound_data_region);

	cquestsnd.device = device;
	cquestsnd.program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

	/* Allocate RAM shared with 68000 */
	cquestsnd.sram = malloc(4096);

	cquestsnd_state_register(device, "cquestsnd");
}


static CPU_RESET( cquestsnd )
{
	cquestsnd.pc = 0;
}


static CPU_EXIT( cquestsnd )
{
	free(cquestsnd.sram);
}


/***************************************************************************
    ROTATE INITIALIZATION AND SHUTDOWN
***************************************************************************/

static STATE_POSTLOAD( cquestrot_postload )
{

}

static void cquestrot_state_register(const device_config *device, const char *type)
{
	state_save_register_item_array(type, device->tag, 0, cquestrot.ram);
	state_save_register_item(type, device->tag, 0, cquestrot.q);
	state_save_register_item(type, device->tag, 0, cquestrot.f);
	state_save_register_item(type, device->tag, 0, cquestrot.y);
	state_save_register_item(type, device->tag, 0, cquestrot.cflag);
	state_save_register_item(type, device->tag, 0, cquestrot.vflag);

	state_save_register_item(type, device->tag, 0, cquestrot.pc);
	state_save_register_item(type, device->tag, 0, cquestrot.seqcnt);
	state_save_register_item(type, device->tag, 0, cquestrot.dsrclatch);
	state_save_register_item(type, device->tag, 0, cquestrot.rsrclatch);
	state_save_register_item(type, device->tag, 0, cquestrot.dynaddr);
	state_save_register_item(type, device->tag, 0, cquestrot.dyndata);
	state_save_register_item(type, device->tag, 0, cquestrot.yrlatch);
	state_save_register_item(type, device->tag, 0, cquestrot.ydlatch);
	state_save_register_item(type, device->tag, 0, cquestrot.dinlatch);
	state_save_register_item(type, device->tag, 0, cquestrot.divreg);
	state_save_register_item(type, device->tag, 0, cquestrot.linedata);
	state_save_register_item(type, device->tag, 0, cquestrot.lineaddr);
	state_save_register_item(type, device->tag, 0, cquestrot.prev_dred);
	state_save_register_item(type, device->tag, 0, cquestrot.prev_dwrt);
	state_save_register_item(type, device->tag, 0, cquestrot.wc);

	state_save_register_item_pointer(type, device->tag, 0, cquestrot.dram, 16384);
	state_save_register_item_pointer(type, device->tag, 0, cquestrot.sram, 2048);

	state_save_register_postload(device->machine, cquestrot_postload, (void *)device);
}

static CPU_INIT( cquestrot )
{
	memset(&cquestrot, 0, sizeof(cquestrot));

	/* Allocate RAM */
	cquestrot.dram = malloc(16384 * sizeof(UINT16));  /* Shared with 68000 */
	cquestrot.sram = malloc(2048 * sizeof(UINT16));   /* Private */

	cquestrot.device = device;
	cquestrot.program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

	cquestrot_state_register(device, "cquestrot");
}


static CPU_RESET( cquestrot )
{
	cquestrot.pc = 0;
	cquestrot.wc = 0;
	cquestrot.prev_dred = 1;
	cquestrot.prev_dwrt = 1;
}


static CPU_EXIT( cquestrot )
{
	free(cquestrot.dram);
	free(cquestrot.sram);
}


/***************************************************************************
    LINE DRAWER INITIALIZATION AND SHUTDOWN
***************************************************************************/
#define FOREGROUND		0
#define BACKGROUND		1
#define ODD_FIELD		0
#define EVEN_FIELD		1

static STATE_POSTLOAD( cquestlin_postload )
{

}

static void cquestlin_state_register(const device_config *device, const char *type)
{
	state_save_register_item_array(type, device->tag, 0, cquestlin.ram);
	state_save_register_item(type, device->tag, 0, cquestlin.q);
	state_save_register_item(type, device->tag, 0, cquestlin.f);
	state_save_register_item(type, device->tag, 0, cquestlin.y);
	state_save_register_item(type, device->tag, 0, cquestlin.cflag);
	state_save_register_item(type, device->tag, 0, cquestlin.vflag);

	state_save_register_item(type, device->tag, 0, cquestlin.pc[0]);
	state_save_register_item(type, device->tag, 0, cquestlin.pc[1]);
	state_save_register_item(type, device->tag, 0, cquestlin.seqcnt);
	state_save_register_item(type, device->tag, 0, cquestlin.clatch);
	state_save_register_item(type, device->tag, 0, cquestlin.zlatch);
	state_save_register_item(type, device->tag, 0, cquestlin.xcnt);
	state_save_register_item(type, device->tag, 0, cquestlin.ycnt);
	state_save_register_item(type, device->tag, 0, cquestlin.sreg);
	state_save_register_item(type, device->tag, 0, cquestlin.fadlatch);
	state_save_register_item(type, device->tag, 0, cquestlin.badlatch);
	state_save_register_item(type, device->tag, 0, cquestlin.sramdlatch);
	state_save_register_item(type, device->tag, 0, cquestlin.fglatch);
	state_save_register_item(type, device->tag, 0, cquestlin.bglatch);
	state_save_register_item(type, device->tag, 0, cquestlin.gt0reg);
	state_save_register_item(type, device->tag, 0, cquestlin.fdxreg);
	state_save_register_item(type, device->tag, 0, cquestlin.field);
	state_save_register_item(type, device->tag, 0, cquestlin.clkcnt);

	state_save_register_item_pointer(type, device->tag, 0, cquestlin.sram, 4096);
	state_save_register_item_pointer(type, device->tag, 0, cquestlin.ptr_ram, 1024);
	state_save_register_item_pointer(type, device->tag, 0, cquestlin.e_stack, 32768);
	state_save_register_item_pointer(type, device->tag, 0, cquestlin.o_stack, 32768);

	state_save_register_postload(device->machine, cquestlin_postload, (void *)device);
}

static CPU_INIT( cquestlin )
{
	memset(&cquestlin, 0, sizeof(cquestlin));

	/* Allocate RAM */
	cquestlin.sram = malloc(4096 * sizeof(UINT16));      /* Shared with rotate CPU */
	cquestlin.ptr_ram = malloc(1024);                    /* Pointer RAM */
	cquestlin.e_stack = malloc(32768 * sizeof(UINT32));  /* Stack DRAM: 32kx20 */
	cquestlin.o_stack = malloc(32768 * sizeof(UINT32));  /* Stack DRAM: 32kx20 */

	cquestlin.device = device;
	cquestlin.program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

	cquestlin_state_register(device, "cquestlin");
}


static CPU_RESET( cquestlin )
{
	cquestlin.clkcnt = 0;
	cquestlin.pc[FOREGROUND] = 0;
	cquestlin.pc[BACKGROUND] = 0x80;
}


static CPU_EXIT( cquestlin )
{
	free(cquestlin.sram);
	free(cquestlin.e_stack);
	free(cquestlin.o_stack);
	free(cquestlin.ptr_ram);
}


/***************************************************************************
    SOUND CORE EXECUTION LOOP
***************************************************************************/

#define SND_PC			(cquestsnd.pc)
#define SND_DATA_IN		(_ramen ? cquestsnd.sound_data[cquestsnd.platch] : cquestsnd.dinlatch)

enum snd_latch_type
{
	PLTCH = 0,
	DAC = 1,
	ADLATCH = 2,
};

static int do_sndjmp(int jmp)
{
	switch (jmp)
	{
		/* JUMP */ case 0: return 1;
		/* MSB  */ case 2: return cquestsnd.f & 0x8000 ? 0 : 1;
		/* !MSB */ case 3: return cquestsnd.f & 0x8000 ? 1 : 0;
		/* ZERO */ case 5: return cquestsnd.f == 0 ? 0 : 1;
		/* OVR  */ case 6: return cquestsnd.vflag ? 0 : 1;
		/* LOOP */ case 7: return cquestsnd.adrcntr & 0x80 ? 0: 1;
	}

	return 0;
}

static CPU_EXECUTE( cquestsnd )
{
	int calldebugger = ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);

	cquestsnd_icount = cycles;

	/* Core execution loop */
	do
	{
		/* Decode the instruction */
		UINT64 inst = memory_decrypted_read_qword(cquestsnd.program, SND_PC << 3);
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
			debugger_instruction_hook(device, cquestsnd.pc);

		/* Don't think this matters, but just in case */
		if (rtn)
			t = cquestsnd.rtnlatch;

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
				case AQ: r = cquestsnd.ram[a]; s = cquestsnd.q;      break;
				case AB: r = cquestsnd.ram[a]; s = cquestsnd.ram[b]; break;
				case ZQ: r = 0;                s = cquestsnd.q;      break;
				case ZB: r = 0;                s = cquestsnd.ram[b]; break;
				case ZA: r = 0;                s = cquestsnd.ram[a]; break;
				case DA: r = SND_DATA_IN;      s = cquestsnd.ram[a]; break;
				case DQ: r = SND_DATA_IN;      s = cquestsnd.q;      break;
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

			cquestsnd.f = res;
			cquestsnd.cflag = cflag;
			cquestsnd.vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					cquestsnd.q = cquestsnd.f;
					cquestsnd.y = cquestsnd.f;
					break;
				case NOP:
					cquestsnd.y = cquestsnd.f;
					break;
				case RAMA:
					cquestsnd.y = cquestsnd.ram[a];
					cquestsnd.ram[b] = cquestsnd.f;
					break;
				case RAMF:
					cquestsnd.ram[b] = cquestsnd.f;
					cquestsnd.y = cquestsnd.f;
					break;
				case RAMQD:
				{
					UINT16 qin;

					cquestsnd.ram[b] = (_rin ? 0 : 0x8000) | (cquestsnd.f >> 1);
					cquestsnd.q >>= 1;
					cquestsnd.y = cquestsnd.f;

					/* When right shifting Q, we need to OR in a value */
					qin = (((cquestsnd.y >> 15) ^ (cquestsnd.y >> 1)) & 1) ? 0 : 0x8000;

					cquestsnd.q |= qin;
					break;
				}
				case RAMD:
					cquestsnd.ram[b] = (_rin ? 0 : 0x8000) | (cquestsnd.f >> 1);
					cquestsnd.y = cquestsnd.f;
					break;
				case RAMQU:
					cquestsnd.ram[b] = (cquestsnd.f << 1) | (_rin ? 0 : 0x0001);
					cquestsnd.q <<= 1;
					cquestsnd.y = cquestsnd.f;
					break;
				case RAMU:
					cquestsnd.ram[b] = (cquestsnd.f << 1) | (_rin ? 0 : 0x0001);
					cquestsnd.y = cquestsnd.f;
					break;
			 }
		}

		/* Now handle any SRAM accesses from the previous cycle */
		if (!cquestsnd.prev_ipram)
		{
		  UINT16 addr = cquestsnd.adrlatch | (cquestsnd.adrcntr & 0x7f);

		  if (!cquestsnd.prev_ipwrt)
			cquestsnd.sram[addr] = cquestsnd.ramwlatch;
		  else
			cquestsnd.dinlatch = cquestsnd.sram[addr];
		}

		/* Handle latches */
		if (latch == PLTCH)
		{
			cquestsnd.platch = ((t & 3) << 9) | ((cquestsnd.y >> 6) & 0x1ff);
		}
		else if (latch == DAC)
		{
			cquestsnd.dac_w((cquestsnd.y & 0xfff0) | ((cquestsnd.adrcntr >> 3) & 0xf));
		}
		else if (latch == ADLATCH)
		{
			/* Load the SRAM address counter - this value is instantly loaded */
			cquestsnd.adrcntr = cquestsnd.y & 0x7f;

			/* Also load the SRAM address latch */
			cquestsnd.adrlatch = cquestsnd.y & 0x780;
		}

		/* Check for jump/return */
		if ( do_sndjmp(jmp) )
			cquestsnd.pc = rtn ? cquestsnd.rtnlatch : t;
		else
			cquestsnd.pc++;

		/* Load the return latch? (Obviously a load and a ret in the same cycle are invalid) */
		if (rtnltch)
		  cquestsnd.rtnlatch = t;

		/* Only increment the sound counter if not loading */
		if (inca && latch != ADLATCH)
		  cquestsnd.adrcntr++;

		/* Latch data for a RAM write (do actual write on the next cycle) */
		if (!_ipwrt)
		  cquestsnd.ramwlatch = cquestsnd.y;

		/* Save level sensitive bits */
		cquestsnd.prev_ipram = _ipram;
		cquestsnd.prev_ipwrt = _ipwrt;

		cquestsnd_icount--;
	} while (cquestsnd_icount > 0);

	return cycles - cquestsnd_icount;
}


/***************************************************************************
    SOUND DISASSEMBLY HOOK
***************************************************************************/

static CPU_DISASSEMBLE( cquestsnd )
{
	static const char *const jmps[] =
	{
		"JUMP ",
		"     ",
		"JMSB ",
		"JNMSB",
		"     ",
		"JZERO",
		"JOVR ",
		"JLOOP",
	};


	static const char *const latches[] =
	{
		"PLTCH  ",
		"DAC    ",
		"ADLATCH",
		"       ",
	};

	UINT64 inst = BIG_ENDIANIZE_INT64(*(UINT64 *)oprom);
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


	sprintf(buffer, "%s %s %s %x,%x,%c %.2x %s %s %.2x %s %s %s %c %c %c\n",
			ins[i5_3],
			src[i2_0],
			dst[i8_6],
			a,
			b,
			ci ? 'C' : ' ',
			_rin,
			jmps[jmp],
			rtn ? "RET" : "   ",
			t,
			latches[latch],
			rtnltch ? "RTLATCH" : "       ",
			_ramen ? "PROM" : "RAM ",
			_ipram ? ' ' : 'R',
			_ipwrt ? ' ' : 'W',
			inca ? 'I' : ' ');

	return 1 | DASMFLAG_SUPPORTED;
}


/***************************************************************************
    ROTATE CORE EXECUTION LOOP
***************************************************************************/

#define ROT_PC			(cquestrot.pc & 0x1ff)

enum rot_spf
{
	SPF_UNUSED0 = 0,
	SPF_UNUSED1 = 1,
	SPF_OP		= 2,
	SPF_RET		= 3,
	SPF_SQLTCH	= 4,
	SPF_SWRT	= 5,
	SPF_DIV		= 6,
	SPF_MULT	= 7,
	SPF_DRED	= 8,
	SPF_DWRT	= 9,
};

enum rot_yout
{
	YOUT_UNUSED0 = 0,
	YOUT_UNUSED1 = 1,
	YOUT_Y2LDA   = 2,
	YOUT_Y2LDD	 = 3,
	YOUT_Y2DAD   = 4,
	YOUT_Y2DYN   = 5,
	YOUT_Y2R	 = 6,
	YOUT_Y2D	 = 7,
};

/* Sync is asserted for the duration of every fourth cycle */
/* The Dynamic RAM latch clocks in a value at the end of this cycle */
/* So CPU waits for sync before reading from DRAM */

INLINE int do_rotjmp(int jmp)
{
	int ret = 0;

	switch (jmp & 7)
	{
		/*        */ case 0: ret = 0;                         break;
		/* SEQ    */ case 1: ret = (cquestrot.seqcnt == 0xf); break;
		/* CAROUT */ case 2: ret = cquestrot.cflag;           break;
		/* SYNC   */ case 3: ret = !(cquestrot.clkcnt & 0x3); break;
		/* LDWAIT */ case 4: ret = 0;                         break;
		/* MSB    */ case 5: ret = BIT(cquestrot.f, 15);      break;
		/* >=1    */ case 6: ret = (!_BIT(cquestrot.f, 15) && !(cquestrot.f == 0)); break;
		/* ZERO   */ case 7: ret = (cquestrot.f == 0);        break;
	}

	return !(!ret ^ BIT(jmp, 3));
}


#define ROT_SRAM_ADDRESS	((cquestrot.dsrclatch & 2) ? cquestrot.yrlatch : (cquestrot.rsrclatch | 0x700))


static CPU_EXECUTE( cquestrot )
{
	int calldebugger = ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);

	cquestrot_icount = cycles;

	/* Core execution loop */
	do
	{
		/* Decode the instruction */
		UINT64 inst = memory_decrypted_read_qword(cquestrot.program, ROT_PC << 3);

		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 20) & 0xfff;
		int jmp     = (inshig >> 16) & 0xf;
		int spf		= (inshig >> 12) & 0xf;
		int rsrc	= (inshig >> 11) & 0x1;
		int yout	= (inshig >> 8) & 0x7;
		int sel		= (inshig >> 6) & 0x3;
		int dsrc	= (inshig >> 4) & 0x3;
		int b		= (inshig >> 0) & 0xf;
		int a		= (inslow >> 28) & 0xf;
		int i8_6	= (inslow >> 24) & 0x7;
		int ci		= (inslow >> 23) & 0x1;
		int i5_3	= (inslow >> 20) & 0x7;
		int _sex	= (inslow >> 19) & 0x1;
		int i2_0	= (inslow >> 16) & 0x7;

		int dsrclatch;
		UINT16 data_in = 0xffff;

		if (calldebugger)
			debugger_instruction_hook(device, ROT_PC);

		/* Handle DRAM accesses - I ought to check this... */
		if (!(cquestrot.clkcnt & 3))
		{
			if (cquestrot.wc)
			{
				cquestrot.wc = 0;
				cquestrot.dram[cquestrot.dynaddr & 0x3fff] = cquestrot.dyndata;
			}
			if (cquestrot.rc)
			{
				cquestrot.rc = 0;
				cquestrot.dinlatch = cquestrot.dram[cquestrot.dynaddr & 0x3fff];
			}
		}

		/* Flag pending DRAM accesses */
		if (!cquestrot.prev_dwrt)
			cquestrot.wc = 1;
		else if (!cquestrot.prev_dred)
			cquestrot.rc = 1;

		/* What's on the D-Bus? */
		if (~cquestrot.dsrclatch & 0x10)
			data_in = cquestrot.dinlatch;
		else if (~cquestrot.dsrclatch & 0x20)
			data_in = cquestrot.sram[ROT_SRAM_ADDRESS];
		else if (~cquestrot.dsrclatch & 0x40)
			data_in = cquestrot.ydlatch;
		else if (~cquestrot.dsrclatch & 0x80)
			data_in = t & 0xfff;

		/* What's on the T-Bus? */
		if ((spf == SPF_RET) && (cquestrot.dsrclatch & 0x80))
			t = data_in;
		else if (spf == SPF_OP)
			t = (t & ~0xf) | (data_in >> 12);


		if (~cquestrot.dsrclatch & 1)
			cquestrot.sram[ROT_SRAM_ADDRESS] = data_in;


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
			if ((spf == SPF_MULT) && !_BIT(cquestrot.q, 0))
				i2_0 |= 2;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case 0: r = cquestrot.ram[a]; s = cquestrot.q;      break;
				case 1: r = cquestrot.ram[a]; s = cquestrot.ram[b]; break;
				case 2: r = 0;                s = cquestrot.q;      break;
				case 3: r = 0;                s = cquestrot.ram[b]; break;
				case 4: r = 0;                s = cquestrot.ram[a]; break;
				case 5: r = data_in;		  s = cquestrot.ram[a]; break;
				case 6: r = data_in;		  s = cquestrot.q;      break;
				case 7: r = data_in;		  s = 0;                break;
			}

			/* Next, determine the I3 and carry bits */
			if ((spf == SPF_DIV) && cquestrot.divreg)
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

			cquestrot.f = res;
			cquestrot.cflag = cflag;
			cquestrot.vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					cquestrot.q = cquestrot.f;
					cquestrot.y = cquestrot.f;
					break;
				case NOP:
					cquestrot.y = cquestrot.f;
					break;
				case RAMA:
					cquestrot.y = cquestrot.ram[a];
					cquestrot.ram[b] = cquestrot.f;
					break;
				case RAMF:
					cquestrot.ram[b] = cquestrot.f;
					cquestrot.y = cquestrot.f;
					break;
				case RAMQD:
				{
					UINT16 q0 = cquestrot.q & 1;
					UINT16 r0 = cquestrot.f & 1;
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
								r15 = (cquestrot.vflag ^ BIT(cquestrot.f, 15)) << 15;
								break;
					}

					cquestrot.ram[b] = r15 | (cquestrot.f >> 1);
					cquestrot.q = q15 | (cquestrot.q >> 1);
					cquestrot.y = cquestrot.f;
					break;
				}
				case RAMD:
				{
					UINT16 r0 = cquestrot.f & 1;
					UINT16 r15 = 0;

					switch (sel)
					{
						case 0: r15 = 0;		break;
						case 1: r15 = 0x8000;	break;
						case 2: r15 = r0 << 15;	break;
						case 3:
							r15 = (cquestrot.vflag ^ BIT(cquestrot.f, 15)) << 15;
							break;
					}

					cquestrot.ram[b] = r15 | (cquestrot.f >> 1);
					cquestrot.y = cquestrot.f;
					break;
				}
				case RAMQU:
				{
					UINT16 q15 = BIT(cquestrot.q, 15);
					UINT16 r15 = BIT(cquestrot.f, 15);
					UINT16 q0 = 0;
					UINT16 r0 = 0;

					switch (sel)
					{
						case 0: q0 = 0; r0 = 0;		break;
						case 1: q0 = 1; r0 = 1;		break;
						case 2: q0 = q15; r0 = r15; break;
						case 3:
						{
							q0 = (spf == SPF_DIV) && !BIT(cquestrot.f, 15);
							r0 = q15;
							break;
						}
					}

					cquestrot.ram[b] = (cquestrot.f << 1) | r0;
					cquestrot.q = (cquestrot.q << 1) | q0;
					cquestrot.y = cquestrot.f;
					break;
				}
				case RAMU:
				{

					UINT16 q15 = BIT(cquestrot.q, 15);
					UINT16 r15 = BIT(cquestrot.f, 15);
					UINT16 r0 = 0;

					switch (sel)
					{
						case 0: r0 = 0;		break;
						case 1: r0 = 1;		break;
						case 2: r0 = r15;	break;
						case 3: r0 = q15;	break;
					}

					cquestrot.ram[b] = (cquestrot.f << 1) | r0;
					cquestrot.y = cquestrot.f;
					break;
				}
			 }
		}

		/* Check for jump */
		if ( do_rotjmp(jmp) )
			cquestrot.pc = t;
		else
			cquestrot.pc = (cquestrot.pc + 1) & 0xfff;

		/* Rising edge; update the sequence counter */
		if (spf == SPF_SQLTCH)
			cquestrot.seqcnt = t & 0xf;
		else if ( (spf == SPF_MULT) || (spf == SPF_DIV) )
			cquestrot.seqcnt = (cquestrot.seqcnt + 1) & 0xf;

		/* Rising edge; write data source reg */
		dsrclatch =
				(~(0x10 << dsrc) & 0xf0)
				| (rsrc ? 0x04 : 0x02)
				| !(spf == SPF_SWRT);

		/* R-latch is written on rising edge of dsrclatch bit 2 */
		if (!_BIT(cquestrot.dsrclatch, 2) && _BIT(dsrclatch, 2))
			cquestrot.rsrclatch = t & 0xff;

		cquestrot.dsrclatch = dsrclatch;

		/* Handle latching on rising edge */
		switch (yout)
		{
			case YOUT_Y2LDA:
			{
				cquestrot.lineaddr = cquestrot.y & 0xfff;
				break;
			}
			case YOUT_Y2LDD:
			{
				cquestrot.linedata = ((t & 0xf) << 12) | (cquestrot.y & 0xfff);
				cquestlin.sram[cquestrot.lineaddr] = cquestrot.linedata;
				break;
			}
			case YOUT_Y2DAD: cquestrot.dynaddr = cquestrot.y & 0x3fff;  break;
			case YOUT_Y2DYN: cquestrot.dyndata = cquestrot.y & 0xffff;	break;
			case YOUT_Y2R:   cquestrot.yrlatch = cquestrot.y & 0x7ff;	break;
			case YOUT_Y2D:	 cquestrot.ydlatch = cquestrot.y;			break;
		}

		/* Clock in the divide register */
		cquestrot.divreg = (spf == SPF_DIV) && !_BIT(cquestrot.f, 15);

		/* DRAM accessing */
		cquestrot.prev_dred = !(spf == SPF_DRED);
		cquestrot.prev_dwrt = !(spf == SPF_DWRT);

		cquestrot.clkcnt++;
		cquestrot_icount--;
	} while (cquestrot_icount > 0);

	return cycles - cquestrot_icount;
}


/***************************************************************************
    ROTATE DISASSEMBLY HOOK
***************************************************************************/

static CPU_DISASSEMBLE( cquestrot )
{
	static const char *const jmps[] =
	{
		"       ",
		"JSEQ   ",
		"JC     ",
		"JSYNC  ",
		"JLDWAIT",
		"JMSB   ",
		"JGEONE ",
		"JZERO  ",

		"JUMP   ",
		"JNSEQ  ",
		"JNC    ",
		"JNSYNC ",
		"JNLDWAI",
		"JNMSB  ",
		"JLTONE ",
		"JNZERO ",
	};

	static const char *const youts[] =
	{
		"     ",
		"     ",
		"Y2LDA",
		"Y2LDD",
		"Y2DAD",
		"Y2DIN",
		"Y2R  ",
		"Y2D  ",
	};

	static const char *const spfs[] =
	{
		"      ",
		"      ",
		"OP    ",
		"RET   ",
		"SQLTCH",
		"SWRT  ",
		"DIV   ",
		"MULT  ",

		"DRED  ",
		"DWRT  ",
		"???   ",
		"???   ",
		"???   ",
		"???   ",
		"???   ",
		"???   "
	};

	UINT64 inst = BIG_ENDIANIZE_INT64(*(UINT64 *)oprom);
	UINT32 inslow = inst & 0xffffffff;
	UINT32 inshig = inst >> 32;

	int t       = (inshig >> 20) & 0xfff;
	int jmp     = (inshig >> 16) & 0xf;
	int spf		= (inshig >> 12) & 0xf;
//  int rsrc    = (inshig >> 11) & 0x1;
	int yout	= (inshig >> 8) & 0x7;
	int sel		= (inshig >> 6) & 0x3;
//  int dsrc    = (inshig >> 4) & 0x3;
	int b		= (inshig >> 0) & 0xf;
	int a		= (inslow >> 28) & 0xf;
	int i8_6	= (inslow >> 24) & 0x7;
	int ci		= (inslow >> 23) & 0x1;
	int i5_3	= (inslow >> 20) & 0x7;
//  int _sex    = (inslow >> 19) & 0x1;
	int i2_0	= (inslow >> 16) & 0x7;

	sprintf(buffer, "%s %s,%s %x,%x,%c %d %s %s %s %.2x\n",
			ins[i5_3],
			src[i2_0],
			dst[i8_6],
			a,
			b,
			ci ? 'C' : ' ',
			sel,
			jmps[jmp],
			youts[yout],
			spfs[spf],
			t);

	return 1 | DASMFLAG_SUPPORTED;
}

/***************************************************************************
    LINE DRAWER CORE EXECUTION LOOP
***************************************************************************/

#define VISIBLE_FIELD	!cquestlin.field

enum line_spf
{
	LSPF_UNUSUED = 0,
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

INLINE int do_linjmp(int jmp)
{
	int ret = 0;

	switch (jmp & 7)
	{
		/*        */ case 0: ret = 0; break;
		/* MSB    */ case 1: ret = BIT(cquestlin.f, 11); break;
		/* SEQ    */ case 2: ret = (cquestlin.seqcnt == 0xfff); break;
		/* >0     */ case 3: ret = !(cquestlin.f == 0) && !_BIT(cquestlin.f, 11); break;
		/* CAROUT */ case 4: ret = (cquestlin.cflag); break;
		/* ZERO   */ case 5: ret = (cquestlin.f == 0); break;
	}

	return !(!ret ^ BIT(jmp, 3));
}



void cubeqcpu_swap_line_banks(void)
{
	cquestlin.field = cquestlin.field ^ 1;
}


void clear_stack(void)
{
	memset(&cquestlin.ptr_ram[cquestlin.field * 256], 0, 256);
}

UINT8 get_ptr_ram_val(int i)
{
	return cquestlin.ptr_ram[(VISIBLE_FIELD * 256) + i];
}

UINT32* get_stack_ram(void)
{
	if (VISIBLE_FIELD == ODD_FIELD)
		return cquestlin.o_stack;
	else
		return cquestlin.e_stack;
}


static CPU_EXECUTE( cquestlin )
{
#define LINE_PC ((cquestlin.pc[prog] & 0x7f) | ((prog == BACKGROUND) ? 0x80 : 0))

	int calldebugger = ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0);
	UINT32	*stack_ram;
	UINT8	*ptr_ram;

	/* Check the field and set the stack/pointer RAM pointers appropriately */
	if (cquestlin.field == ODD_FIELD)
	{
		stack_ram = cquestlin.o_stack;
		ptr_ram = &cquestlin.ptr_ram[0];
	}
	else
	{
		stack_ram = cquestlin.e_stack;
		ptr_ram = &cquestlin.ptr_ram[0x100];
	}

	cquestlin_icount = cycles;

	/* Core execution loop */
	do
	{
		/* Are we executing the foreground or backgroud program? */
		int prog = (cquestlin.clkcnt & 3) ? BACKGROUND : FOREGROUND;

		UINT64 inst = memory_decrypted_read_qword(cquestlin.program, LINE_PC << 3);

		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 24) & 0xff;
		int jmp		= (inshig >> 20) & 0xf;
		int latch	= (inshig >> 16) & 0x7;
		int op		= (inshig >> 15) & 0x1;
		int spf		= (inshig >> 12) & 0x7;
		int b		= (inshig >> 8) & 0xf;
		int a		= (inshig >> 4) & 0xf;
		int i8_6	= (inshig >> 0) & 0x7;
		int ci		= (inslow >> 31) & 0x1;
		int i5_3	= (inslow >> 28) & 0x7;
		int _pbcs	= (inslow >> 27) & 0x1;
		int i2_0	= (inslow >> 24) & 0x7;

		UINT16	data_in = 0;

		if (calldebugger)
			debugger_instruction_hook(device, cquestlin.pc[prog]);

		/* Handle accesses to and from shared SRAM */
		if (prog == FOREGROUND)
		{
			if (!_BIT(cquestlin.fglatch, 5))
				data_in = cquestlin.sram[cquestlin.fadlatch];
			else
				data_in = cquestrot.linedata;
		}
		else
		{
			if (!_BIT(cquestlin.bglatch, 4))
				cquestlin.sram[cquestlin.badlatch] = cquestlin.sramdlatch;
			else if (_BIT(cquestlin.bglatch, 2))
				data_in = cquestlin.sram[cquestlin.badlatch];
			else
				data_in = cquestrot.linedata;
		}

		/* Handle a write to stack RAM (/DOWRT) */
		if ((cquestlin.clkcnt & 3) == 1)
		{
			if (_BIT(cquestlin.fglatch, 4) && (cquestlin.ycnt < 256))
			{
				/* 20-bit words */
				UINT32 data;
				UINT16 h = cquestlin.xcnt;
				UINT8 v = cquestlin.ycnt & 0xff;

				/* Clamp H between 0 and 319 */
				if (h >= 320)
					h = (h & 0x800) ? 0 : 319;

				/* Stack word type depends on STOP/#START bit */
				if ( _BIT(cquestlin.fglatch, 3) )
					data = (0 << 19) | (h << 8) | cquestlin.zlatch;
				else
					data = (1 << 19) | ((cquestlin.clatch & 0x100) << 9) | (h << 8) | (cquestlin.clatch & 0xff);

				stack_ram[(v << 7) | (ptr_ram[v] & 0x7f)] = data;

				/* Also increment the pointer RAM entry. Note that it cannot exceed 128 */
				ptr_ram[v] = (ptr_ram[v] + 1) & 0x7f;
			}
		}

		/* Override T3-0? */
		if (op)
			t = (t & ~0xf) | (data_in >> 12);

		/* Determine the correct I1 bit  */
		if ((spf == LSPF_MULT) && !_BIT(cquestlin.q, 0))
			i2_0 |= 2;

		/* Determine A0 (BRESA0) */
		if ((prog == FOREGROUND) && !_BIT(cquestlin.fglatch, 2))
			a |= cquestlin.gt0reg;

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
				case 0: r = cquestlin.ram[a];	s = cquestlin.q;      break;
				case 1: r = cquestlin.ram[a];	s = cquestlin.ram[b]; break;
				case 2: r = 0;					s = cquestlin.q;      break;
				case 3: r = 0;					s = cquestlin.ram[b]; break;
				case 4: r = 0;					s = cquestlin.ram[a]; break;
				case 5: r = data_in;		  	s = cquestlin.ram[a]; break;
				case 6: r = data_in;		  	s = cquestlin.q;      break;
				case 7: r = data_in;		  	s = 0;                break;
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

			cquestlin.f = res & 0xfff;
			cquestlin.cflag = cflag;
			cquestlin.vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					cquestlin.q = cquestlin.f;
					cquestlin.y = cquestlin.f;
					break;
				case NOP:
					cquestlin.y = cquestlin.f;
					break;
				case RAMA:
					cquestlin.y = cquestlin.ram[a];
					cquestlin.ram[b] = cquestlin.f;
					break;
				case RAMF:
					cquestlin.ram[b] = cquestlin.f;
					cquestlin.y = cquestlin.f;
					break;
				case RAMQD:
				{
					UINT16 r11 = (BIT(cquestlin.f, 11) ^ cquestlin.vflag) ? 0x800 : 0;
					UINT16 q11 = (prog == BACKGROUND) ? 0x800 : 0;

					cquestlin.ram[b] = r11 | (cquestlin.f >> 1);
					cquestlin.q = q11 | (cquestlin.q >> 1);
					cquestlin.y = cquestlin.f;
					break;
				}
				case RAMD:
				{
					UINT16 r11 = (BIT(cquestlin.f, 11) ^ cquestlin.vflag) ? 0x800 : 0;

					cquestlin.ram[b] = r11 | (cquestlin.f >> 1);
					cquestlin.y = cquestlin.f;
					break;
				}
				case RAMQU:
				{
					/* Determine shift inputs */
					UINT16 r0 = (prog == BACKGROUND);

					/* This should never happen - Q0 will be invalid */
					cquestlin.ram[b] = (cquestlin.f << 1) | r0;
					cquestlin.q = (cquestlin.q << 1) | 0;
					cquestlin.y = cquestlin.f;
					break;
				}
				case RAMU:
				{
					UINT16 r0 = (prog == BACKGROUND);

					cquestlin.ram[b] = (cquestlin.f << 1) | r0;
					cquestlin.y = cquestlin.f;
					break;
				}
			 }
		}

		/* Adjust program counter */
		if ( do_linjmp(jmp) )
			cquestlin.pc[prog] = t & 0x7f;
		else
			cquestlin.pc[prog] = (cquestlin.pc[prog] + 1) & 0x7f;

		if (prog == BACKGROUND)
			cquestlin.pc[prog] |= 0x80;
		else
		{
			/* Handle events that happen during FG execution */
			if (latch == LLATCH_XLATCH)
				cquestlin.xcnt = cquestlin.y & 0xfff;
			else
			{
				int _xcet;
				int mux_sel = (BIT(cquestlin.sreg, SREG_DX_DY) << 1) | (BIT(cquestlin.sreg, SREG_DX) ^ BIT(cquestlin.sreg, SREG_DY));

				if (mux_sel == 0)
					_xcet = !(spf == LSPF_BRES);
				else if (mux_sel == 1)
					_xcet = _BIT(cquestlin.fglatch, 1);
				else if (mux_sel == 2)
					_xcet = !(cquestlin.gt0reg && (spf == LSPF_BRES));
				else
					_xcet = _BIT(cquestlin.fglatch, 0);

				if (!_xcet)
					cquestlin.xcnt = (cquestlin.xcnt + (_BIT(cquestlin.sreg, SREG_DX) ? 1 : -1)) & 0xfff;
			}

			if (latch == LLATCH_YLATCH)
				cquestlin.ycnt = cquestlin.y & 0xfff;
			else
			{
				int _ycet;
				int mux_sel = (BIT(cquestlin.sreg, SREG_DX_DY) << 1) | (BIT(cquestlin.sreg, SREG_DX) ^ BIT(cquestlin.sreg, SREG_DY));

				if (mux_sel == 0)
					_ycet = !(cquestlin.gt0reg && (spf == LSPF_BRES));
				else if (mux_sel == 1)
					_ycet = _BIT(cquestlin.fglatch, 0);
				else if (mux_sel == 2)
					_ycet = !(spf == LSPF_BRES);
				else
					_ycet = _BIT(cquestlin.fglatch, 1);

				if (!_ycet)
					cquestlin.ycnt = (cquestlin.ycnt + (_BIT(cquestlin.sreg, SREG_DY) ? 1 : -1)) & 0xfff;
			}
		}

		if (latch == LLATCH_CLATCH)
			cquestlin.clatch = cquestlin.y & 0x1ff;
		else if (latch == LLATCH_ZLATCH)
			cquestlin.zlatch = cquestlin.y & 0xff;
		else if (latch == LLATCH_FADLATCH)
			cquestlin.fadlatch = cquestlin.y & 0xfff;
		else if (latch == LLATCH_BADLATCH)
			cquestlin.badlatch = cquestlin.y & 0xfff;

		/* What about the SRAM dlatch? */
		if ( !_BIT(cquestlin.bglatch, 5) )
			cquestlin.sramdlatch = ((t & 0xf) << 12) | (cquestlin.y & 0x0fff);

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
				dowrt = cquestlin.fdxreg ^ BIT(cquestlin.sreg, SREG_DX);
				start_stop = cquestlin.fdxreg;
			}
			else if (mux_sel == 2)
			{
				dowrt = BIT(cquestlin.sreg, SREG_LDX) ^ BIT(cquestlin.sreg, SREG_DX);
				start_stop = BIT(cquestlin.sreg, SREG_DX);
			}
			else
			{
				dowrt = (spf == LSPF_BRES) && (_BIT(cquestlin.sreg, SREG_DX_DY) || cquestlin.gt0reg);
				start_stop = BIT(cquestlin.sreg, SREG_DY);
			}

			cquestlin.fglatch =
					(!(latch == LLATCH_FADLATCH) << 5)
					| (dowrt << 4)
					| (start_stop << 3)
					| (_pbcs << 2)
					| (!(spf == LSPF_BRES) << 1)
					| !(cquestlin.gt0reg && (spf == LSPF_BRES));
		}
		else
		{
			int _lpwrt = BIT(cquestlin.bglatch, 5);

			cquestlin.bglatch =
					(!(spf == LSPF_PWRT) << 5)
					| (_lpwrt << 4)
					| ((!_lpwrt || (!(spf == LSPF_PWRT) && (latch == LLATCH_BADLATCH))) << 2);
		}

		/* Clock-in another bit into the sign bit shifter? */
		if (spf == LSPF_SREG)
		{
			/* The sign bit is inverted */
			cquestlin.sreg = (cquestlin.sreg << 1) | !BIT(cquestlin.f, 11);

			/* Also latch the >0 reg */
			cquestlin.gt0reg = !(cquestlin.f == 0) && !_BIT(cquestlin.f, 11);
		}
		else if (spf == LSPF_FSTRT)
		{
			cquestlin.fdxreg = BIT(cquestlin.sreg, 3);
		}

		/* Load or increment sequence counter? */
		if (latch == LLATCH_SEQLATCH)
		{
			cquestlin.seqcnt = cquestlin.y & 0xfff;
		}
		else if (spf == LSPF_BRES)
		{
			cquestlin.seqcnt = (cquestlin.seqcnt + 1) & 0xfff;

			/* Also latch the >0 reg */
			cquestlin.gt0reg = !(cquestlin.f == 0) && !_BIT(cquestlin.f, 11);
		}

		cquestlin_icount--;
		cquestlin.clkcnt++;
	} while (cquestlin_icount > 0);

	return cycles - cquestlin_icount;
}


/***************************************************************************
    LINE DRAWER DISASSEMBLY HOOK
***************************************************************************/

static CPU_DISASSEMBLE( cquestlin )
{
	static const char *const jmps[] =
	{
		"     ",
		"JMSB ",
		"JSEQ ",
		"JGTZ ",
		"JC   ",
		"JZ   ",
		"?????",
		"?????",

		"JUMP ",
		"JNMSB",
		"JNSEQ",
		"JLEZ ",
		"JNC  ",
		"JNZ  ",
		"?????",
		"?????",
	};

	static const char *const latches[] =
	{
		"       ",
		"SEQLTCH",
		"XLTCH  ",
		"YLTCH  ",
		"BGLTCH ",
		"FGLTCH ",
		"CLTCH  ",
		"ZLTCH  ",
	};

	static const char *const spfs[] =
	{
		"      ",
		"FSTOP ",
		"FREG  ",
		"FSTART",
		"PWRT  ",
		"MULT  ",
		"LSTOP ",
		"BRES  ",
	};

	UINT64 inst = BIG_ENDIANIZE_INT64(*(UINT64 *)oprom);
	UINT32 inslow = inst & 0xffffffff;
	UINT32 inshig = inst >> 32;

	int t       = (inshig >> 24) & 0xff;
	int jmp		= (inshig >> 20) & 0xf;
	int latch	= (inshig >> 16) & 0x7;
	int op		= (inshig >> 15) & 0x1;
	int spf		= (inshig >> 12) & 0x7;
	int b		= (inshig >> 8) & 0xf;
	int a		= (inshig >> 4) & 0xf;
	int i8_6	= (inshig >> 0) & 0x7;
	int ci		= (inslow >> 31) & 0x1;
	int i5_3	= (inslow >> 28) & 0x7;
	int _pbcs	= (inslow >> 27) & 0x1;
	int i2_0	= (inslow >> 24) & 0x7;

	sprintf(buffer, "%s %s,%s %x,%x %c %s %.2x %s %s %s %s\n",
			ins[i5_3],
			src[i2_0],
			dst[i8_6],
			a,
			b,
			ci ? 'C' : ' ',
			jmps[jmp],
			t,
			latches[latch],
			op ? "OP" : "  ",
			_pbcs ? "  " : "PB",
			spfs[spf]);

	return 1 | DASMFLAG_SUPPORTED;
}

/**************************************************************************
 * Sound set_info
 **************************************************************************/

static CPU_SET_INFO( cquestsnd )
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTSND_PC:		cquestsnd.pc = info->i;				break;
		case CPUINFO_INT_REGISTER + CQUESTSND_Q:		cquestsnd.q = info->i;				break;
		case CPUINFO_INT_REGISTER + CQUESTSND_RTNLATCH:	cquestsnd.rtnlatch = info->i;		break;
		case CPUINFO_INT_REGISTER + CQUESTSND_ADRCNTR:	cquestsnd.adrcntr = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTSND_DINLATCH:	cquestsnd.dinlatch = info->i;		break;

		case CPUINFO_STR_REGISTER + CQUESTSND_RAM0:  cquestsnd.ram[0x0] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM1:  cquestsnd.ram[0x1] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM2:  cquestsnd.ram[0x2] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM3:  cquestsnd.ram[0x3] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM4:  cquestsnd.ram[0x4] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM5:  cquestsnd.ram[0x5] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM6:  cquestsnd.ram[0x6] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM7:  cquestsnd.ram[0x7] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM8:  cquestsnd.ram[0x8] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM9:  cquestsnd.ram[0x9] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMA:  cquestsnd.ram[0xa] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMB:  cquestsnd.ram[0xb] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMC:  cquestsnd.ram[0xc] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMD:  cquestsnd.ram[0xd] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAME:  cquestsnd.ram[0xe] = info->i; 			break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMF:  cquestsnd.ram[0xf] = info->i; 			break;
	}
}

/**************************************************************************
 * Sound get_info
 **************************************************************************/

CPU_GET_INFO( cquestsnd )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cquestsnd);			break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 64;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -3;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTSND_PC:			info->i = cquestsnd.pc;				break;
		case CPUINFO_INT_REGISTER + CQUESTSND_RTNLATCH:		info->i = cquestsnd.rtnlatch;		break;
		case CPUINFO_INT_REGISTER + CQUESTSND_ADRCNTR:		info->i = cquestsnd.adrcntr;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cquestsnd);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(cquestsnd);break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(cquestsnd);break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(cquestsnd);			break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(cquestsnd);			break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(cquestsnd);			break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cquestsnd);		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cquestsnd);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cquestsnd_icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Sound CPU");break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Cube Quest");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, ".......");			 break;
		case CPUINFO_STR_REGISTER + CQUESTSND_PC:  		sprintf(info->s, "PC:  %02X", cquestsnd.pc); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_Q:  		sprintf(info->s, "Q:   %04X", cquestsnd.q); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RTNLATCH: sprintf(info->s, "RTN: %02X", cquestsnd.rtnlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_ADRCNTR:  sprintf(info->s, "CNT: %02X", cquestsnd.adrcntr); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_DINLATCH:	sprintf(info->s, "DIN: %04X", cquestsnd.dinlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM0:		sprintf(info->s, "RAM[0]: %04X", cquestsnd.ram[0x0]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM1:		sprintf(info->s, "RAM[1]: %04X", cquestsnd.ram[0x1]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM2:		sprintf(info->s, "RAM[2]: %04X", cquestsnd.ram[0x2]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM3:		sprintf(info->s, "RAM[3]: %04X", cquestsnd.ram[0x3]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM4:		sprintf(info->s, "RAM[4]: %04X", cquestsnd.ram[0x4]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM5:		sprintf(info->s, "RAM[5]: %04X", cquestsnd.ram[0x5]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM6:		sprintf(info->s, "RAM[6]: %04X", cquestsnd.ram[0x6]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM7:		sprintf(info->s, "RAM[7]: %04X", cquestsnd.ram[0x7]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM8:		sprintf(info->s, "RAM[8]: %04X", cquestsnd.ram[0x8]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAM9:		sprintf(info->s, "RAM[9]: %04X", cquestsnd.ram[0x9]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMA:		sprintf(info->s, "RAM[A]: %04X", cquestsnd.ram[0xa]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMB:		sprintf(info->s, "RAM[B]: %04X", cquestsnd.ram[0xb]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMC:		sprintf(info->s, "RAM[C]: %04X", cquestsnd.ram[0xc]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMD:		sprintf(info->s, "RAM[D]: %04X", cquestsnd.ram[0xd]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAME:		sprintf(info->s, "RAM[E]: %04X", cquestsnd.ram[0xe]); break;
		case CPUINFO_STR_REGISTER + CQUESTSND_RAMF:		sprintf(info->s, "RAM[F]: %04X", cquestsnd.ram[0xf]); break;
	}
}


/**************************************************************************
 * Rotate set_info
 **************************************************************************/

static CPU_SET_INFO( cquestrot )
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTROT_PC:	cquestrot.pc = info->i;					break;
		case CPUINFO_INT_REGISTER + CQUESTROT_Q:	cquestrot.q = info->i;					break;

		case CPUINFO_STR_REGISTER + CQUESTROT_RAM0:		cquestrot.ram[0x0] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM1:		cquestrot.ram[0x1] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM2:		cquestrot.ram[0x2] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM3:		cquestrot.ram[0x3] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM4:		cquestrot.ram[0x4] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM5:		cquestrot.ram[0x5] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM6:		cquestrot.ram[0x6] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM7:		cquestrot.ram[0x7] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM8:		cquestrot.ram[0x8] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM9:		cquestrot.ram[0x9] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMA:		cquestrot.ram[0xa] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMB:		cquestrot.ram[0xb] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMC:		cquestrot.ram[0xc] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMD:		cquestrot.ram[0xd] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAME:		cquestrot.ram[0xe] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMF:		cquestrot.ram[0xf] = info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_SEQCNT:	cquestrot.seqcnt	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNADDR:	cquestrot.dynaddr	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNDATA:	cquestrot.dyndata	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YRLATCH:	cquestrot.yrlatch	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YDLATCH:	cquestrot.ydlatch	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DINLATCH:	cquestrot.dinlatch	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DSRCLATCH:cquestrot.dsrclatch	= info->i; 		break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RSRCLATCH:cquestrot.rsrclatch	= info->i; 		break;
	}
}

/**************************************************************************
 * Rotate get_info
 **************************************************************************/

CPU_GET_INFO( cquestrot )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cquestrot);			break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 64;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -3;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTROT_PC:		info->i = cquestrot.pc;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cquestrot);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(cquestrot);break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(cquestrot);break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(cquestrot);			break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(cquestrot);			break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(cquestrot);			break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cquestrot);		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cquestrot);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cquestrot_icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Rotate CPU");break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Cube Quest");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c%c", cquestrot.cflag ? 'C' : '.',
																				   cquestrot.vflag ? 'V' : '.',
																				   cquestrot.f ? '.' : 'Z');	break;
		case CPUINFO_STR_REGISTER + CQUESTROT_PC:  		sprintf(info->s, "PC:  %02X", cquestrot.pc); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_Q:		sprintf(info->s, "Q:   %04X", cquestrot.q); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM0:		sprintf(info->s, "RAM[0]: %04X", cquestrot.ram[0x0]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM1:		sprintf(info->s, "RAM[1]: %04X", cquestrot.ram[0x1]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM2:		sprintf(info->s, "RAM[2]: %04X", cquestrot.ram[0x2]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM3:		sprintf(info->s, "RAM[3]: %04X", cquestrot.ram[0x3]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM4:		sprintf(info->s, "RAM[4]: %04X", cquestrot.ram[0x4]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM5:		sprintf(info->s, "RAM[5]: %04X", cquestrot.ram[0x5]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM6:		sprintf(info->s, "RAM[6]: %04X", cquestrot.ram[0x6]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM7:		sprintf(info->s, "RAM[7]: %04X", cquestrot.ram[0x7]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM8:		sprintf(info->s, "RAM[8]: %04X", cquestrot.ram[0x8]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAM9:		sprintf(info->s, "RAM[9]: %04X", cquestrot.ram[0x9]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMA:		sprintf(info->s, "RAM[A]: %04X", cquestrot.ram[0xa]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMB:		sprintf(info->s, "RAM[B]: %04X", cquestrot.ram[0xb]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMC:		sprintf(info->s, "RAM[C]: %04X", cquestrot.ram[0xc]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMD:		sprintf(info->s, "RAM[D]: %04X", cquestrot.ram[0xd]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAME:		sprintf(info->s, "RAM[E]: %04X", cquestrot.ram[0xe]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RAMF:		sprintf(info->s, "RAM[F]: %04X", cquestrot.ram[0xf]); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_SEQCNT:	sprintf(info->s, "SEQCNT: %01X", cquestrot.seqcnt); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNADDR:	sprintf(info->s, "DYNADDR: %04X", cquestrot.dynaddr); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DYNDATA:	sprintf(info->s, "DYNDATA: %04X", cquestrot.dyndata); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YRLATCH:	sprintf(info->s, "YRLATCH: %04X", cquestrot.yrlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_YDLATCH:	sprintf(info->s, "YDLATCH: %04X", cquestrot.ydlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DINLATCH:	sprintf(info->s, "DINLATCH: %04X", cquestrot.dinlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_DSRCLATCH:sprintf(info->s, "DSRCLATCH: %04X", cquestrot.dsrclatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_RSRCLATCH:sprintf(info->s, "RSRCLATCH: %04X", cquestrot.rsrclatch); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_LDADDR:	sprintf(info->s, "LDADDR : %04X", cquestrot.lineaddr); break;
		case CPUINFO_STR_REGISTER + CQUESTROT_LDDATA:	sprintf(info->s, "LDDATA : %04X", cquestrot.linedata); break;
	}
}


/**************************************************************************
 * Line drawer set_info
 **************************************************************************/

static CPU_SET_INFO( cquestlin )
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTLIN_FGPC:	cquestlin.pc[FOREGROUND] = info->i;		break;
		case CPUINFO_INT_REGISTER + CQUESTLIN_BGPC:	cquestlin.pc[BACKGROUND] = info->i;		break;
		case CPUINFO_INT_REGISTER + CQUESTLIN_Q:	cquestlin.q = info->i;					break;

		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM0:		cquestlin.ram[0x0] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM1:		cquestlin.ram[0x1] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM2:		cquestlin.ram[0x2] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM3:		cquestlin.ram[0x3] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM4:		cquestlin.ram[0x4] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM5:		cquestlin.ram[0x5] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM6:		cquestlin.ram[0x6] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM7:		cquestlin.ram[0x7] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM8:		cquestlin.ram[0x8] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM9:		cquestlin.ram[0x9] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMA:		cquestlin.ram[0xa] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMB:		cquestlin.ram[0xb] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMC:		cquestlin.ram[0xc] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMD:		cquestlin.ram[0xd] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAME:		cquestlin.ram[0xe] = info->i;		break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMF:		cquestlin.ram[0xf] = info->i;		break;
	}
}

/**************************************************************************
 * Line drawer get_info
 **************************************************************************/

CPU_GET_INFO( cquestlin )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cquestlin);			break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 64;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -3;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + CQUESTLIN_FGPC:		info->i = cquestlin.pc[cquestlin.clkcnt & 3 ? BACKGROUND : FOREGROUND];	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cquestlin);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(cquestlin);break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(cquestlin);break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(cquestlin);			break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(cquestlin);			break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(cquestlin);			break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cquestlin);		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cquestlin);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cquestlin_icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Line CPU");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Cube Quest");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Philip J Bennett"); break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c%c|%cG",	cquestlin.cflag ? 'C' : '.',
																						cquestlin.vflag ? 'V' : '.',
																						cquestlin.f ? '.' : 'Z',
																						cquestlin.clkcnt & 3 ? 'B' : 'F'); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_FGPC:  	sprintf(info->s, "FPC:  %02X", cquestlin.pc[FOREGROUND]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_BGPC:  	sprintf(info->s, "BPC:  %02X", cquestlin.pc[BACKGROUND]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_Q:		sprintf(info->s, "Q:   %04X", cquestlin.q); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM0:		sprintf(info->s, "RAM[0]: %04X", cquestlin.ram[0x0]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM1:		sprintf(info->s, "RAM[1]: %04X", cquestlin.ram[0x1]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM2:		sprintf(info->s, "RAM[2]: %04X", cquestlin.ram[0x2]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM3:		sprintf(info->s, "RAM[3]: %04X", cquestlin.ram[0x3]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM4:		sprintf(info->s, "RAM[4]: %04X", cquestlin.ram[0x4]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM5:		sprintf(info->s, "RAM[5]: %04X", cquestlin.ram[0x5]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM6:		sprintf(info->s, "RAM[6]: %04X", cquestlin.ram[0x6]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM7:		sprintf(info->s, "RAM[7]: %04X", cquestlin.ram[0x7]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM8:		sprintf(info->s, "RAM[8]: %04X", cquestlin.ram[0x8]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAM9:		sprintf(info->s, "RAM[9]: %04X", cquestlin.ram[0x9]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMA:		sprintf(info->s, "RAM[A]: %04X", cquestlin.ram[0xa]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMB:		sprintf(info->s, "RAM[B]: %04X", cquestlin.ram[0xb]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMC:		sprintf(info->s, "RAM[C]: %04X", cquestlin.ram[0xc]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMD:		sprintf(info->s, "RAM[D]: %04X", cquestlin.ram[0xd]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAME:		sprintf(info->s, "RAM[E]: %04X", cquestlin.ram[0xe]); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_RAMF:		sprintf(info->s, "RAM[F]: %04X", cquestlin.ram[0xf]); break;

		case CPUINFO_STR_REGISTER + CQUESTLIN_FADLATCH:	sprintf(info->s, "FADDR:  %04X", cquestlin.fadlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_BADLATCH:	sprintf(info->s, "BADDR:  %04X", cquestlin.badlatch); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_SREG:		sprintf(info->s, "SREG:   %04X", cquestlin.sreg);	break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_XCNT:		sprintf(info->s, "XCNT:   %03X", cquestlin.xcnt);	break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_YCNT:		sprintf(info->s, "YCNT:   %03X", cquestlin.ycnt);	break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_CLATCH:	sprintf(info->s, "CLATCH: %04X", cquestlin.clatch); break;
		case CPUINFO_STR_REGISTER + CQUESTLIN_ZLATCH:	sprintf(info->s, "ZLATCH: %04X", cquestlin.zlatch); break;
	}
}
