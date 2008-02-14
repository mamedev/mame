/*************************************************************************

 Exidy Vertigo hardware

 The Vertigo vector CPU consists of four AMD 2901 bit slice
 processors, logic to control microcode program flow and a digital
 vector generator. The microcode for the bit slice CPUs is stored in 13
 bipolar proms for a total of 512 52bit wide micro instructions. The
 microcode not only crontrols the 2901s but also loading and storing
 of operands and results, program flow control and vector generation.

 +----+----+----+----+----+----+-------+-----+-----+------+-----+----+---+
 |VUC |VUC |VUC |VUC |VUC |VUC |  VUC  | VUC | VUC | VUC  | VUC |VUC |VUC| labels
 | 10 | 13 | 9  | 8  | 7  | 6  |   5   |  12 |  11 |  2   |  1  | 4  | 3 |
 +----+----+----+----+----+----+-------+-----+-----+------+-----+----+---+
 |    |    |    |    |    |    |       |     |     |      |     |PR5/|R5/| schematics
 |J5/4|G5/1|K5/5|L5/6|M5/7|N5/8| P5/9  |H5/2 |HJ5/3|S5/12 |T5/13| 10 |11 |
 +----+----+----+----+----+----+-------+-----+-----+------+-----+----+---+
 55 44|4444|4444|3333|3333|3322|2 2 2 2|2 222|11 11|1 1 11|110 0|0000|0000
 21 98|7654|3210|9876|5432|1098|7 6 5 4|3 210|98 76|5 4 32|109 8|7654|3210

    xx|xxxx|aaaa|bbbb|iiii|iiii|i c m r|r ooo|ii oo|  j jj|jjj m|mmmm|mmmm
    54|3210|3210|3210|8765|4321|0 n r s|w fff|ff aa|  p 43|210 a|aaaa|aaaa
                                    e e|r 210|10 10|  o   |    8|7654|3210
                                    q l i             s
                                        t
                                        e
 x:    address for 64 words of 16 bit wide SRAM
 a:    A register index
 b:    B register index
 i:    2901 instruction
 cn:   carry bit
 mreq, rsel, rwrite: signals for memory access
 of:   vector generator
 if:   vector RAM/ROM data select
 oa:   vector RAM/ROM address select
 jpos: jump condition inverted
 j:    jump condition and type
 m:    jump address

 Variables, enums and defines are named as in the schematics (pp. 6, 7)
 where possible.

*************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "video/vector.h"
#include "vertigo.h"


/*************************************
 *
 *  Macros and enums
 *
 *************************************/

#define MC_LENGTH 512

#define V_ADDPOINT(h,v,c,i) \
	vector_add_point (((h) & 0x7ff) << 14, (0x6ff - ((v) & 0x7ff)) << 14, VECTOR_COLOR444(c), (i))

#define ADD(r,s,c)	(((r)  + (s) + (c)) & 0xffff)
#define SUBR(r,s,c) ((~(r) + (s) + (c)) & 0xffff)
#define SUBS(r,s,c) (((r) + ~(s) + (c)) & 0xffff)
#define OR(r,s)		((r) | (s))
#define AND(r,s)	((r) & (s))
#define NOTRS(r,s)	(~(r) & (s))
#define EXOR(r,s)	((r) ^ (s))
#define EXNOR(r,s)	(~((r) ^ (s)))

/* values for MC_DST */
enum {
	QREG = 0,
	NOP,
	RAMA,
	RAMF,
	RAMQD,
	RAMD,
	RAMQU,
	RAMU
};

/* values for MC_IF */
enum {
	S_ROMDE = 0,
	S_RAMDE
};

/* values for MC_OA */
enum {
	S_SREG = 0,
	S_ROMA,
	S_RAMD
};

/* values for MC_JMP */
enum {
	S_JBK = 0,
	S_CALL,
	S_OPT,
	S_RETURN
};

/* values for MC_JCON */
enum {
	S_ALWAYS = 0,
	S_MSB,
	S_FEQ0,
	S_Y10,
	S_VFIN,
	S_FPOS,
	S_INTL4
};


/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT16 *vertigo_vectorram;


/*************************************
 *
 *  Typedefs
 *
 *************************************/

typedef struct _am2901
{
	UINT32 ram[16];	  /* internal ram */
	UINT32 d;		  /* direct data D input */
	UINT32 q;		  /* Q register */
	UINT32 f;		  /* F ALU result */
	UINT32 y;		  /* Y output */
} am2901;

typedef struct _vector_generator
{
	UINT32 sreg;	  /* shift register */
	UINT32 l1;		  /* latch 1 adder operand only */
	UINT32 l2;		  /* latch 2 adder operand only */
	UINT32 c_v;		  /* vertical position counter */
	UINT32 c_h;		  /* horizontal position counter */
	UINT32 c_l;		  /* length counter */
	UINT32 adder_s;	  /* slope generator result and B input */
	UINT32 adder_a;	  /* slope generator A input */
	UINT32 color;	  /* color */
	UINT32 intensity; /* intensity */
	UINT32 brez;	  /* h/v-counters enable */
	UINT32 vfin;	  /* drawing yes/no */
	UINT32 hud1;	  /* h-counter up or down (stored in L1) */
	UINT32 hud2;	  /* h-counter up or down (stored in L2) */
	UINT32 vud1;	  /* v-counter up or down (stored in L1) */
	UINT32 vud2;	  /* v-counter up or down (stored in L2) */
	UINT32 hc1;		  /* use h- or v-counter in L1 mode */
	UINT32 ven;       /* vector intensity enable */
} vector_generator;

typedef struct _microcode
{
	UINT32 x;
	UINT32 a;
	UINT32 b;
	UINT32 inst;
	UINT32 dest;
	UINT32 cn;
	UINT32 mreq;
	UINT32 rsel;
	UINT32 rwrite;
	UINT32 of;
	UINT32 iif;
	UINT32 oa;
	UINT32 jpos;
	UINT32 jmp;
	UINT32 jcon;
	UINT32 ma;
} microcode;

typedef struct _vproc
{
	UINT16 sram[64]; /* external sram */
	UINT16 ramlatch; /* latch between 2901 and sram */
	UINT16 rom_adr;	 /* vector ROM/RAM address latch */
	UINT32 pc;		 /* program counter */
	UINT32 ret;		 /* return address */

} vproc;


/*************************************
 *
 *  Statics
 *
 *************************************/

static vproc vs;
static am2901 bsp;
static vector_generator vgen;
static UINT16 *vertigo_vectorrom;
static microcode mc[MC_LENGTH];


/*************************************
 *
 *  Vector processor initialization
 *
 *************************************/

void vertigo_vproc_init(void)
{
	int i;
	UINT64 *mcode;

	vertigo_vectorrom = (UINT16 *)memory_region(REGION_USER1);
	mcode = (UINT64 *)memory_region(REGION_PROMS);

	/* Decode microcode */
	for (i = 0; i < MC_LENGTH; i++)
	{
		mc[i].x = (mcode[i] >> 44) & 0x3f;
		mc[i].a = (mcode[i] >> 40) & 0xf;
		mc[i].b = (mcode[i] >> 36) & 0xf;
		mc[i].inst = (mcode[i] >> 27) & 077;
		mc[i].dest = (mcode[i] >> 33) & 07;
		mc[i].cn = (mcode[i] >> 26) & 0x1;
		mc[i].mreq = (mcode[i] >> 25) & 0x1;
		mc[i].rwrite = (mcode[i] >> 23) & 0x1;
		mc[i].rsel = mc[i].rwrite & ((mcode[i] >> 24) & 0x1);
		mc[i].of =  (mcode[i] >> 20) & 0x7;
		mc[i].iif = (mcode[i] >> 18) & 0x3;
		mc[i].oa = (mcode[i] >> 16) & 0x3;
		mc[i].jpos = (mcode[i] >> 14) & 0x1;
		mc[i].jmp = (mcode[i] >> 12) & 0x3;
		mc[i].jcon = (mcode[i] >> 9) & 0x7;
		mc[i].ma = mcode[i] & 0x1ff;
	}

	memset(&vs, 0, sizeof(vs));
	memset(&bsp, 0, sizeof(bsp));
	memset(&vgen, 0, sizeof(vgen));

	state_save_register_item_array("vector_proc", 0, vs.sram);
	state_save_register_item("vector_proc", 0, vs.ramlatch);
	state_save_register_item("vector_proc", 0, vs.rom_adr);
	state_save_register_item("vector_proc", 0, vs.pc);
	state_save_register_item("vector_proc", 0, vs.ret);

	state_save_register_item_array("vector_proc", 0, bsp.ram);
	state_save_register_item("vector_proc", 0, bsp.d);
	state_save_register_item("vector_proc", 0, bsp.q);
	state_save_register_item("vector_proc", 0, bsp.f);
	state_save_register_item("vector_proc", 0, bsp.y);

	state_save_register_item("vector_proc", 0, vgen.sreg);
	state_save_register_item("vector_proc", 0, vgen.l1);
	state_save_register_item("vector_proc", 0, vgen.l2);
	state_save_register_item("vector_proc", 0, vgen.c_v);
	state_save_register_item("vector_proc", 0, vgen.c_h);
	state_save_register_item("vector_proc", 0, vgen.c_l);
	state_save_register_item("vector_proc", 0, vgen.adder_s);
	state_save_register_item("vector_proc", 0, vgen.adder_a);
	state_save_register_item("vector_proc", 0, vgen.color);
	state_save_register_item("vector_proc", 0, vgen.intensity);
	state_save_register_item("vector_proc", 0, vgen.brez);
	state_save_register_item("vector_proc", 0, vgen.vfin);
	state_save_register_item("vector_proc", 0, vgen.hud1);
	state_save_register_item("vector_proc", 0, vgen.hud2);
	state_save_register_item("vector_proc", 0, vgen.vud1);
	state_save_register_item("vector_proc", 0, vgen.vud2);
	state_save_register_item("vector_proc", 0, vgen.hc1);
	state_save_register_item("vector_proc", 0, vgen.ven);
}


/********************************************
 *
 *  4 x AM2901 bit slice processors
 *  Q3 and IN3 are hardwired
 *
 ********************************************/

static void am2901x4 (am2901 *bsp, microcode *mc)
{
	switch (mc->inst)
	{
	case 000: bsp->f = ADD(bsp->ram[mc->a], bsp->q, mc->cn); break;
	case 001: bsp->f = ADD(bsp->ram[mc->a], bsp->ram[mc->b], mc->cn); break;
	case 002: bsp->f = ADD(0, bsp->q, mc->cn); break;
	case 003: bsp->f = ADD(0, bsp->ram[mc->b], mc->cn); break;
	case 004: bsp->f = ADD(0, bsp->ram[mc->a], mc->cn); break;
	case 005: bsp->f = ADD(bsp->d, bsp->ram[mc->a], mc->cn); break;
	case 006: bsp->f = ADD(bsp->d, bsp->q, mc->cn); break;
	case 007: bsp->f = ADD(bsp->d, 0, mc->cn); break;

	case 010: bsp->f = SUBR(bsp->ram[mc->a], bsp->q, mc->cn); break;
	case 011: bsp->f = SUBR(bsp->ram[mc->a], bsp->ram[mc->b], mc->cn); break;
	case 012: bsp->f = SUBR(0, bsp->q, mc->cn); break;
	case 013: bsp->f = SUBR(0, bsp->ram[mc->b], mc->cn); break;
	case 014: bsp->f = SUBR(0, bsp->ram[mc->a], mc->cn); break;
	case 015: bsp->f = SUBR(bsp->d, bsp->ram[mc->a], mc->cn); break;
	case 016: bsp->f = SUBR(bsp->d, bsp->q, mc->cn); break;
	case 017: bsp->f = SUBR(bsp->d, 0, mc->cn); break;

	case 020: bsp->f = SUBS(bsp->ram[mc->a], bsp->q, mc->cn); break;
	case 021: bsp->f = SUBS(bsp->ram[mc->a], bsp->ram[mc->b], mc->cn); break;
	case 022: bsp->f = SUBS(0, bsp->q, mc->cn); break;
	case 023: bsp->f = SUBS(0, bsp->ram[mc->b], mc->cn); break;
	case 024: bsp->f = SUBS(0, bsp->ram[mc->a], mc->cn); break;
	case 025: bsp->f = SUBS(bsp->d, bsp->ram[mc->a], mc->cn); break;
	case 026: bsp->f = SUBS(bsp->d, bsp->q, mc->cn); break;
	case 027: bsp->f = SUBS(bsp->d, 0, mc->cn); break;

	case 030: bsp->f = OR(bsp->ram[mc->a], bsp->q); break;
	case 031: bsp->f = OR(bsp->ram[mc->a], bsp->ram[mc->b]); break;
	case 032: bsp->f = OR(0, bsp->q); break;
	case 033: bsp->f = OR(0, bsp->ram[mc->b]); break;
	case 034: bsp->f = OR(0, bsp->ram[mc->a]); break;
	case 035: bsp->f = OR(bsp->d, bsp->ram[mc->a]); break;
	case 036: bsp->f = OR(bsp->d, bsp->q); break;
	case 037: bsp->f = OR(bsp->d, 0); break;

	case 040: bsp->f = AND(bsp->ram[mc->a], bsp->q); break;
	case 041: bsp->f = AND(bsp->ram[mc->a], bsp->ram[mc->b]); break;
	case 042: bsp->f = AND(0, bsp->q); break;
	case 043: bsp->f = AND(0, bsp->ram[mc->b]); break;
	case 044: bsp->f = AND(0, bsp->ram[mc->a]); break;
	case 045: bsp->f = AND(bsp->d, bsp->ram[mc->a]); break;
	case 046: bsp->f = AND(bsp->d, bsp->q); break;
	case 047: bsp->f = AND(bsp->d, 0); break;

	case 050: bsp->f = NOTRS(bsp->ram[mc->a], bsp->q); break;
	case 051: bsp->f = NOTRS(bsp->ram[mc->a], bsp->ram[mc->b]); break;
	case 052: bsp->f = NOTRS(0, bsp->q); break;
	case 053: bsp->f = NOTRS(0, bsp->ram[mc->b]); break;
	case 054: bsp->f = NOTRS(0, bsp->ram[mc->a]); break;
	case 055: bsp->f = NOTRS(bsp->d, bsp->ram[mc->a]); break;
	case 056: bsp->f = NOTRS(bsp->d, bsp->q); break;
	case 057: bsp->f = NOTRS(bsp->d, 0); break;

	case 060: bsp->f = EXOR(bsp->ram[mc->a], bsp->q); break;
	case 061: bsp->f = EXOR(bsp->ram[mc->a], bsp->ram[mc->b]); break;
	case 062: bsp->f = EXOR(0, bsp->q); break;
	case 063: bsp->f = EXOR(0, bsp->ram[mc->b]); break;
	case 064: bsp->f = EXOR(0, bsp->ram[mc->a]); break;
	case 065: bsp->f = EXOR(bsp->d, bsp->ram[mc->a]); break;
	case 066: bsp->f = EXOR(bsp->d, bsp->q); break;
	case 067: bsp->f = EXOR(bsp->d, 0); break;

	case 070: bsp->f = EXNOR(bsp->ram[mc->a], bsp->q); break;
	case 071: bsp->f = EXNOR(bsp->ram[mc->a], bsp->ram[mc->b]); break;
	case 072: bsp->f = EXNOR(0, bsp->q); break;
	case 073: bsp->f = EXNOR(0, bsp->ram[mc->b]); break;
	case 074: bsp->f = EXNOR(0, bsp->ram[mc->a]); break;
	case 075: bsp->f = EXNOR(bsp->d, bsp->ram[mc->a]); break;
	case 076: bsp->f = EXNOR(bsp->d, bsp->q); break;
	case 077: bsp->f = EXNOR(bsp->d, 0); break;
	}

	switch (mc->dest)
	{
	case QREG:
		bsp->q = bsp->f;
		bsp->y = bsp->f;
		break;
	case NOP:
		bsp->y = bsp->f;
		break;
	case RAMA:
		bsp->y = bsp->ram[mc->a];
		bsp->ram[mc->b] = bsp->f;
		break;
	case RAMF:
		bsp->y = bsp->f;
		bsp->ram[mc->b] = bsp->f;
		break;
	case RAMQD:
		bsp->y = bsp->f;
		bsp->q = (bsp->q >> 1) & 0x7fff;		  /* Q3 is low */
		bsp->ram[mc->b] = (bsp->f >> 1) | 0x8000; /* IN3 is high! */
		break;
	case RAMD:
		bsp->y = bsp->f;
		bsp->ram[mc->b] = (bsp->f >> 1) | 0x8000; /* IN3 is high! */
		break;
	case RAMQU:
		bsp->y = bsp->f;
		bsp->ram[mc->b] = (bsp->f << 1) & 0xffff;
		bsp->q = (bsp->q << 1) & 0xffff;
		break;
	case RAMU:
		bsp->y = bsp->f;
		bsp->ram[mc->b] = (bsp->f << 1) & 0xffff;
		break;
	}
}


/********************************************
 *
 *  Vector Generator
 *
 *  This part of the hardware draws vectors
 *  under control of the bit slice processors.
 *  It is just a bunch of counters, latches
 *  and DACs.
 *
 ********************************************/

static void vertigo_vgen (vector_generator *vg)
{
	if (vg->c_l & 0x800)
	{
		vg->vfin = 1;
		vg->c_l = (vg->c_l+1) & 0xfff;

		if ((vg->c_l & 0x800) == 0)
		{
			vg->brez = 0;
			vg->vfin = 0;
		}

		if (vg->brez) /* H/V counter enabled */
		{
			/* Depending on MSB of adder only one or both
               counters are de-/incremented. This is all
               defined by the shift register which is
               latched in bits 12-15 of L1/L2.
            */
			if (vg->adder_s & 0x800)
			{
				if (vg->hc1)
					vg->c_h += vg->hud1? -1: 1;
				else
					vg->c_v += vg->vud1? -1: 1;
				vg->adder_a = vg->l1;
			}
			else
			{
				vg->c_h += vg->hud2? -1: 1;
				vg->c_v += vg->vud2? -1: 1;
				vg->adder_a = vg->l2;
			}

			/* H/V counters are 12 bit */
			vg->c_v &= 0xfff;
			vg->c_h &= 0xfff;
		}

		vg->adder_s = (vg->adder_s + vg->adder_a) & 0xfff;
	}

	if (vg->brez ^ vg->ven)
	{
		if (vg->brez)
		V_ADDPOINT (vg->c_h, vg->c_v, 0, 0);
		else
			V_ADDPOINT (vg->c_h, vg->c_v, vg->color, vg->intensity);
		vg->ven = vg->brez;
	}
}

/*************************************
 *
 *  Vector processor
 *
 *************************************/

void vertigo_vproc(int cycles, int irq4)
{
	int jcond;
	microcode *cmc;

	if (irq4) vector_clear_list();

	profiler_mark(PROFILER_USER1);

	while (cycles--)
	{
		/* Microcode at current PC */
		cmc = &mc[vs.pc];

		/* Load data */
		if (cmc->iif == S_RAMDE)
		{
			bsp.d = vs.ramlatch;
		}
		else if (cmc->iif == S_ROMDE)
		{
			if (vs.rom_adr < 0x2000)
			{
				bsp.d = vertigo_vectorram[vs.rom_adr & 0xfff];
			}
			else
			{
				bsp.d = vertigo_vectorrom[vs.rom_adr & 0x7fff];
			}
		}

		/* SRAM selected ? */
		if (cmc->rsel == 0)
		{
			if (cmc->rwrite)
			{
				bsp.d = vs.sram[cmc->x];
			}
			else
			{
				/* Data can be transferred between vector ROM/RAM
                   and SRAM without going through the 2901 */
				vs.sram[cmc->x] = bsp.d;
			}
		}

		am2901x4 (&bsp, cmc);

		/* Store data */
		switch (cmc->oa)
		{
		case S_RAMD:
			vs.ramlatch = bsp.y;
			if (cmc->iif==S_RAMDE && (cmc->rsel == 0) && (cmc->rwrite == 0))
				vs.sram[cmc->x] = vs.ramlatch;
			break;
		case S_ROMA:
			vs.rom_adr = bsp.y;
			break;
		case S_SREG:
			/* FPOS is shifted into sreg */
			vgen.sreg = (vgen.sreg >> 1) | ((bsp.f >> 9) & 4);
			break;
		default:
			break;
		}

		/* Vector generator setup */
		switch (cmc->of)
		{
		case 0:
			vgen.color = bsp.y & 0xfff;
			break;
		case 1:
			vgen.intensity = bsp.y & 0xff;
			break;
		case 2:
			vgen.l1 = bsp.y & 0xfff;
			vgen.adder_s = 0;
			vgen.adder_a = vgen.l2;
			vgen.hud1 = vgen.sreg & 1;
			vgen.vud1 = vgen.sreg & 2;
			vgen.hc1  = vgen.sreg & 4;
			vgen.brez = 1;
			break;
		case 3:
			vgen.l2 = bsp.y & 0xfff;
			vgen.adder_s = (vgen.adder_s + vgen.adder_a) & 0xfff;
			if (vgen.adder_s & 0x800)
				vgen.adder_a = vgen.l1;
			else
				vgen.adder_a = vgen.l2;
			vgen.hud2 = vgen.sreg & 1;
			vgen.vud2 = vgen.sreg & 2;
			break;
		case 4:
			vgen.c_v = bsp.y & 0xfff;
			break;
		case 5:
			vgen.c_h = bsp.y & 0xfff;
			break;
		case 6:
			/* Loading the c_l counter starts
             * the vector counters if MSB is set
             */
			vgen.c_l = bsp.y & 0xfff;
			break;
		}

		vertigo_vgen (&vgen);

		/* Microcode program flow */
		switch (cmc->jcon)
		{
		case S_MSB:
			/* ALU most significant bit */
			jcond = (bsp.f >> 15) & 1;
			break;
		case S_FEQ0:
			/* ALU is 0 */
			jcond = (bsp.f == 0)? 1 : 0;
			break;
		case S_Y10:
			jcond = (bsp.y >> 10) & 1;
			break;
		case S_VFIN:
			jcond = vgen.vfin;
			break;
		case S_FPOS:
			/* FPOS is bit 11 */
			jcond = (bsp.f >> 11) & 1;
			break;
		case S_INTL4:
			jcond = irq4;
			/* Detect idle loop. If the code takes a jump
             on irq4 or !irq4 the destination is a idle loop
             waiting for irq4 state change. We then take a short
             cut and run for just 100 cycles to make sure the
             loop is actually entered.
            */
			if ((cmc->jpos != irq4) && cycles > 100)
			{
				cycles=100;
			}
			break;
		default:
			jcond = 1;
			break;
		}

		if (jcond ^ cmc->jpos)
		{
			/* Except for JBK, address bit 8 isn't changed
               in program flow. */
			switch (cmc->jmp)
			{
			case S_JBK:
				/* JBK is the only jump where MA8 is used */
				vs.pc = cmc->ma;
				break;
			case S_CALL:
				/* call and store return address */
				vs.ret = (vs.pc + 1) & 0xff;
				vs.pc = (vs.pc & 0x100) | (cmc->ma & 0xff);
				break;
			case S_OPT:
				/* OPT is used for microcode jump tables. The first
                   four address bits are defined by bits 12-15
                   of 2901 input (D) */
				vs.pc = (vs.pc & 0x100) | (cmc->ma & 0xf0) | ((bsp.d >> 12) & 0xf);
				break;
			case S_RETURN:
				/* return from call */
				vs.pc = (vs.pc & 0x100) | vs.ret;
				break;
			}
		}
		else
		{
			vs.pc = (vs.pc & 0x100) | ((vs.pc + 1) & 0xff);
		}
	}

	profiler_mark(PROFILER_END);
}
