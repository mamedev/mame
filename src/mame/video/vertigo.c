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

#include "emu.h"
#include "video/vector.h"
#include "includes/vertigo.h"


/*************************************
 *
 *  Macros and enums
 *
 *************************************/

#define V_ADDPOINT(m,h,v,c,i) \
	vector_add_point (m, ((h) & 0x7ff) << 14, (0x6ff - ((v) & 0x7ff)) << 14, VECTOR_COLOR444(c), (i))

#define ADD(r,s,c)  (((r)  + (s) + (c)) & 0xffff)
#define SUBR(r,s,c) ((~(r) + (s) + (c)) & 0xffff)
#define SUBS(r,s,c) (((r) + ~(s) + (c)) & 0xffff)
#define OR(r,s)     ((r) | (s))
#define AND(r,s)    ((r) & (s))
#define NOTRS(r,s)  (~(r) & (s))
#define EXOR(r,s)   ((r) ^ (s))
#define EXNOR(r,s)  (~((r) ^ (s)))

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
 *  Vector processor initialization
 *
 *************************************/

void vertigo_vproc_init(running_machine &machine)
{
	vertigo_state *state = machine.driver_data<vertigo_state>();
	state_save_register_item_array(machine, "vector_proc", NULL, 0, state->m_vs.sram);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vs.ramlatch);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vs.rom_adr);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vs.pc);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vs.ret);

	state_save_register_item_array(machine, "vector_proc", NULL, 0, state->m_bsp.ram);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_bsp.d);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_bsp.q);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_bsp.f);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_bsp.y);

	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.sreg);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.l1);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.l2);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.c_v);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.c_h);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.c_l);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.adder_s);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.adder_a);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.color);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.intensity);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.brez);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.vfin);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.hud1);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.hud2);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.vud1);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.vud2);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.hc1);
	state_save_register_item(machine, "vector_proc", NULL, 0, state->m_vgen.ven);
}


void vertigo_vproc_reset(running_machine &machine)
{
	vertigo_state *state = machine.driver_data<vertigo_state>();
	int i;
	UINT64 *mcode;

	state->m_vectorrom = (UINT16 *)state->memregion("user1")->base();
	mcode = (UINT64 *)state->memregion("proms")->base();

	/* Decode microcode */
	for (i = 0; i < MC_LENGTH; i++)
	{
		state->m_mc[i].x = (mcode[i] >> 44) & 0x3f;
		state->m_mc[i].a = (mcode[i] >> 40) & 0xf;
		state->m_mc[i].b = (mcode[i] >> 36) & 0xf;
		state->m_mc[i].inst = (mcode[i] >> 27) & 077;
		state->m_mc[i].dest = (mcode[i] >> 33) & 07;
		state->m_mc[i].cn = (mcode[i] >> 26) & 0x1;
		state->m_mc[i].mreq = (mcode[i] >> 25) & 0x1;
		state->m_mc[i].rwrite = (mcode[i] >> 23) & 0x1;
		state->m_mc[i].rsel = state->m_mc[i].rwrite & ((mcode[i] >> 24) & 0x1);
		state->m_mc[i].of =  (mcode[i] >> 20) & 0x7;
		state->m_mc[i].iif = (mcode[i] >> 18) & 0x3;
		state->m_mc[i].oa = (mcode[i] >> 16) & 0x3;
		state->m_mc[i].jpos = (mcode[i] >> 14) & 0x1;
		state->m_mc[i].jmp = (mcode[i] >> 12) & 0x3;
		state->m_mc[i].jcon = (mcode[i] >> 9) & 0x7;
		state->m_mc[i].ma = mcode[i] & 0x1ff;
	}

	memset(&state->m_vs, 0, sizeof(state->m_vs));
	memset(&state->m_bsp, 0, sizeof(state->m_bsp));
	memset(&state->m_vgen, 0, sizeof(state->m_vgen));
	state->m_vgen.set_machine(machine);
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
		bsp->q = (bsp->q >> 1) & 0x7fff;          /* Q3 is low */
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
		V_ADDPOINT (vg->machine(), vg->c_h, vg->c_v, 0, 0);
		else
			V_ADDPOINT (vg->machine(), vg->c_h, vg->c_v, vg->color, vg->intensity);
		vg->ven = vg->brez;
	}
}

/*************************************
 *
 *  Vector processor
 *
 *************************************/

void vertigo_vproc(running_machine &machine, int cycles, int irq4)
{
	vertigo_state *state = machine.driver_data<vertigo_state>();
	int jcond;
	microcode *cmc;

	if (irq4) vector_clear_list();

	g_profiler.start(PROFILER_USER1);

	while (cycles--)
	{
		/* Microcode at current PC */
		cmc = &state->m_mc[state->m_vs.pc];

		/* Load data */
		if (cmc->iif == S_RAMDE)
		{
			state->m_bsp.d = state->m_vs.ramlatch;
		}
		else if (cmc->iif == S_ROMDE)
		{
			if (state->m_vs.rom_adr < 0x2000)
			{
				state->m_bsp.d = state->m_vectorram[state->m_vs.rom_adr & 0xfff];
			}
			else
			{
				state->m_bsp.d = state->m_vectorrom[state->m_vs.rom_adr & 0x7fff];
			}
		}

		/* SRAM selected ? */
		if (cmc->rsel == 0)
		{
			if (cmc->rwrite)
			{
				state->m_bsp.d = state->m_vs.sram[cmc->x];
			}
			else
			{
				/* Data can be transferred between vector ROM/RAM
				   and SRAM without going through the 2901 */
				state->m_vs.sram[cmc->x] = state->m_bsp.d;
			}
		}

		am2901x4 (&state->m_bsp, cmc);

		/* Store data */
		switch (cmc->oa)
		{
		case S_RAMD:
			state->m_vs.ramlatch = state->m_bsp.y;
			if (cmc->iif==S_RAMDE && (cmc->rsel == 0) && (cmc->rwrite == 0))
				state->m_vs.sram[cmc->x] = state->m_vs.ramlatch;
			break;
		case S_ROMA:
			state->m_vs.rom_adr = state->m_bsp.y;
			break;
		case S_SREG:
			/* FPOS is shifted into sreg */
			state->m_vgen.sreg = (state->m_vgen.sreg >> 1) | ((state->m_bsp.f >> 9) & 4);
			break;
		default:
			break;
		}

		/* Vector generator setup */
		switch (cmc->of)
		{
		case 0:
			state->m_vgen.color = state->m_bsp.y & 0xfff;
			break;
		case 1:
			state->m_vgen.intensity = state->m_bsp.y & 0xff;
			break;
		case 2:
			state->m_vgen.l1 = state->m_bsp.y & 0xfff;
			state->m_vgen.adder_s = 0;
			state->m_vgen.adder_a = state->m_vgen.l2;
			state->m_vgen.hud1 = state->m_vgen.sreg & 1;
			state->m_vgen.vud1 = state->m_vgen.sreg & 2;
			state->m_vgen.hc1  = state->m_vgen.sreg & 4;
			state->m_vgen.brez = 1;
			break;
		case 3:
			state->m_vgen.l2 = state->m_bsp.y & 0xfff;
			state->m_vgen.adder_s = (state->m_vgen.adder_s + state->m_vgen.adder_a) & 0xfff;
			if (state->m_vgen.adder_s & 0x800)
				state->m_vgen.adder_a = state->m_vgen.l1;
			else
				state->m_vgen.adder_a = state->m_vgen.l2;
			state->m_vgen.hud2 = state->m_vgen.sreg & 1;
			state->m_vgen.vud2 = state->m_vgen.sreg & 2;
			break;
		case 4:
			state->m_vgen.c_v = state->m_bsp.y & 0xfff;
			break;
		case 5:
			state->m_vgen.c_h = state->m_bsp.y & 0xfff;
			break;
		case 6:
			/* Loading the c_l counter starts
			 * the vector counters if MSB is set
			 */
			state->m_vgen.c_l = state->m_bsp.y & 0xfff;
			break;
		}

		vertigo_vgen (&state->m_vgen);

		/* Microcode program flow */
		switch (cmc->jcon)
		{
		case S_MSB:
			/* ALU most significant bit */
			jcond = (state->m_bsp.f >> 15) & 1;
			break;
		case S_FEQ0:
			/* ALU is 0 */
			jcond = (state->m_bsp.f == 0)? 1 : 0;
			break;
		case S_Y10:
			jcond = (state->m_bsp.y >> 10) & 1;
			break;
		case S_VFIN:
			jcond = state->m_vgen.vfin;
			break;
		case S_FPOS:
			/* FPOS is bit 11 */
			jcond = (state->m_bsp.f >> 11) & 1;
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
				state->m_vs.pc = cmc->ma;
				break;
			case S_CALL:
				/* call and store return address */
				state->m_vs.ret = (state->m_vs.pc + 1) & 0xff;
				state->m_vs.pc = (state->m_vs.pc & 0x100) | (cmc->ma & 0xff);
				break;
			case S_OPT:
				/* OPT is used for microcode jump tables. The first
				   four address bits are defined by bits 12-15
				   of 2901 input (D) */
				state->m_vs.pc = (state->m_vs.pc & 0x100) | (cmc->ma & 0xf0) | ((state->m_bsp.d >> 12) & 0xf);
				break;
			case S_RETURN:
				/* return from call */
				state->m_vs.pc = (state->m_vs.pc & 0x100) | state->m_vs.ret;
				break;
			}
		}
		else
		{
			state->m_vs.pc = (state->m_vs.pc & 0x100) | ((state->m_vs.pc + 1) & 0xff);
		}
	}

	g_profiler.stop();
}
