// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
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
#include "includes/vertigo.h"


/*************************************
 *
 *  Macros and enums
 *
 *************************************/

#define V_ADDPOINT(h,v,c,i) \
	m_vector->add_point (((h) & 0x7ff) << 14, (0x6ff - ((v) & 0x7ff)) << 14, VECTOR_COLOR444(c), (i))

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

void vertigo_state::vertigo_vproc_init()
{
	save_item(m_vs.sram, "vector_proc/m_vs.sram");
	save_item(m_vs.ramlatch, "vector_proc/m_vs.ramlatch");
	save_item(m_vs.rom_adr, "vector_proc/m_vs.rom_adr");
	save_item(m_vs.pc, "vector_proc/m_vs.pc");
	save_item(m_vs.ret, "vector_proc/m_vs.ret");

	save_item(m_bsp.ram, "vector_proc/m_bsp.ram");
	save_item(m_bsp.d, "vector_proc/m_bsp.d");
	save_item(m_bsp.q, "vector_proc/m_bsp.q");
	save_item(m_bsp.f, "vector_proc/m_bsp.f");
	save_item(m_bsp.y, "vector_proc/m_bsp.y");

	save_item(m_vgen.sreg, "vector_proc/m_vgen.sreg");
	save_item(m_vgen.l1, "vector_proc/m_vgen.l1");
	save_item(m_vgen.l2, "vector_proc/m_vgen.l2");
	save_item(m_vgen.c_v, "vector_proc/m_vgen.c_v");
	save_item(m_vgen.c_h, "vector_proc/m_vgen.c_h");
	save_item(m_vgen.c_l, "vector_proc/m_vgen.c_l");
	save_item(m_vgen.adder_s, "vector_proc/m_vgen.adder_s");
	save_item(m_vgen.adder_a, "vector_proc/m_vgen.adder_a");
	save_item(m_vgen.color, "vector_proc/m_vgen.color");
	save_item(m_vgen.intensity, "vector_proc/m_vgen.intensity");
	save_item(m_vgen.brez, "vector_proc/m_vgen.brez");
	save_item(m_vgen.vfin, "vector_proc/m_vgen.vfin");
	save_item(m_vgen.hud1, "vector_proc/m_vgen.hud1");
	save_item(m_vgen.hud2, "vector_proc/m_vgen.hud2");
	save_item(m_vgen.vud1, "vector_proc/m_vgen.vud1");
	save_item(m_vgen.vud2, "vector_proc/m_vgen.vud2");
	save_item(m_vgen.hc1, "vector_proc/m_vgen.hc1");
	save_item(m_vgen.ven, "vector_proc/m_vgen.ven");
}


void vertigo_state::vertigo_vproc_reset()
{
	int i;
	UINT64 *mcode;

	m_vectorrom = (UINT16 *)memregion("user1")->base();
	mcode = (UINT64 *)memregion("proms")->base();

	/* Decode microcode */
	for (i = 0; i < MC_LENGTH; i++)
	{
		m_mc[i].x = (mcode[i] >> 44) & 0x3f;
		m_mc[i].a = (mcode[i] >> 40) & 0xf;
		m_mc[i].b = (mcode[i] >> 36) & 0xf;
		m_mc[i].inst = (mcode[i] >> 27) & 077;
		m_mc[i].dest = (mcode[i] >> 33) & 07;
		m_mc[i].cn = (mcode[i] >> 26) & 0x1;
		m_mc[i].mreq = (mcode[i] >> 25) & 0x1;
		m_mc[i].rwrite = (mcode[i] >> 23) & 0x1;
		m_mc[i].rsel = m_mc[i].rwrite & ((mcode[i] >> 24) & 0x1);
		m_mc[i].of =  (mcode[i] >> 20) & 0x7;
		m_mc[i].iif = (mcode[i] >> 18) & 0x3;
		m_mc[i].oa = (mcode[i] >> 16) & 0x3;
		m_mc[i].jpos = (mcode[i] >> 14) & 0x1;
		m_mc[i].jmp = (mcode[i] >> 12) & 0x3;
		m_mc[i].jcon = (mcode[i] >> 9) & 0x7;
		m_mc[i].ma = mcode[i] & 0x1ff;
	}

	memset(&m_vs, 0, sizeof(m_vs));
	memset(&m_bsp, 0, sizeof(m_bsp));
	memset(&m_vgen, 0, sizeof(m_vgen));
}


/********************************************
 *
 *  4 x AM2901 bit slice processors
 *  Q3 and IN3 are hardwired
 *
 ********************************************/

void vertigo_state::am2901x4 (am2901 *bsp, microcode *mc)
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

void vertigo_state::vertigo_vgen (vector_generator *vg)
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

void vertigo_state::vertigo_vproc(int cycles, int irq4)
{
	int jcond;
	microcode *cmc;

	if (irq4) m_vector->clear_list();

	g_profiler.start(PROFILER_USER1);

	while (cycles--)
	{
		/* Microcode at current PC */
		cmc = &m_mc[m_vs.pc];

		/* Load data */
		if (cmc->iif == S_RAMDE)
		{
			m_bsp.d = m_vs.ramlatch;
		}
		else if (cmc->iif == S_ROMDE)
		{
			if (m_vs.rom_adr < 0x2000)
			{
				m_bsp.d = m_vectorram[m_vs.rom_adr & 0xfff];
			}
			else
			{
				m_bsp.d = m_vectorrom[m_vs.rom_adr & 0x7fff];
			}
		}

		/* SRAM selected ? */
		if (cmc->rsel == 0)
		{
			if (cmc->rwrite)
			{
				m_bsp.d = m_vs.sram[cmc->x];
			}
			else
			{
				/* Data can be transferred between vector ROM/RAM
				   and SRAM without going through the 2901 */
				m_vs.sram[cmc->x] = m_bsp.d;
			}
		}

		am2901x4 (&m_bsp, cmc);

		/* Store data */
		switch (cmc->oa)
		{
		case S_RAMD:
			m_vs.ramlatch = m_bsp.y;
			if (cmc->iif==S_RAMDE && (cmc->rsel == 0) && (cmc->rwrite == 0))
				m_vs.sram[cmc->x] = m_vs.ramlatch;
			break;
		case S_ROMA:
			m_vs.rom_adr = m_bsp.y;
			break;
		case S_SREG:
			/* FPOS is shifted into sreg */
			m_vgen.sreg = (m_vgen.sreg >> 1) | ((m_bsp.f >> 9) & 4);
			break;
		default:
			break;
		}

		/* Vector generator setup */
		switch (cmc->of)
		{
		case 0:
			m_vgen.color = m_bsp.y & 0xfff;
			break;
		case 1:
			m_vgen.intensity = m_bsp.y & 0xff;
			break;
		case 2:
			m_vgen.l1 = m_bsp.y & 0xfff;
			m_vgen.adder_s = 0;
			m_vgen.adder_a = m_vgen.l2;
			m_vgen.hud1 = m_vgen.sreg & 1;
			m_vgen.vud1 = m_vgen.sreg & 2;
			m_vgen.hc1  = m_vgen.sreg & 4;
			m_vgen.brez = 1;
			break;
		case 3:
			m_vgen.l2 = m_bsp.y & 0xfff;
			m_vgen.adder_s = (m_vgen.adder_s + m_vgen.adder_a) & 0xfff;
			if (m_vgen.adder_s & 0x800)
				m_vgen.adder_a = m_vgen.l1;
			else
				m_vgen.adder_a = m_vgen.l2;
			m_vgen.hud2 = m_vgen.sreg & 1;
			m_vgen.vud2 = m_vgen.sreg & 2;
			break;
		case 4:
			m_vgen.c_v = m_bsp.y & 0xfff;
			break;
		case 5:
			m_vgen.c_h = m_bsp.y & 0xfff;
			break;
		case 6:
			/* Loading the c_l counter starts
			 * the vector counters if MSB is set
			 */
			m_vgen.c_l = m_bsp.y & 0xfff;
			break;
		}

		vertigo_vgen (&m_vgen);

		/* Microcode program flow */
		switch (cmc->jcon)
		{
		case S_MSB:
			/* ALU most significant bit */
			jcond = (m_bsp.f >> 15) & 1;
			break;
		case S_FEQ0:
			/* ALU is 0 */
			jcond = (m_bsp.f == 0)? 1 : 0;
			break;
		case S_Y10:
			jcond = (m_bsp.y >> 10) & 1;
			break;
		case S_VFIN:
			jcond = m_vgen.vfin;
			break;
		case S_FPOS:
			/* FPOS is bit 11 */
			jcond = (m_bsp.f >> 11) & 1;
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
				m_vs.pc = cmc->ma;
				break;
			case S_CALL:
				/* call and store return address */
				m_vs.ret = (m_vs.pc + 1) & 0xff;
				m_vs.pc = (m_vs.pc & 0x100) | (cmc->ma & 0xff);
				break;
			case S_OPT:
				/* OPT is used for microcode jump tables. The first
				   four address bits are defined by bits 12-15
				   of 2901 input (D) */
				m_vs.pc = (m_vs.pc & 0x100) | (cmc->ma & 0xf0) | ((m_bsp.d >> 12) & 0xf);
				break;
			case S_RETURN:
				/* return from call */
				m_vs.pc = (m_vs.pc & 0x100) | m_vs.ret;
				break;
			}
		}
		else
		{
			m_vs.pc = (m_vs.pc & 0x100) | ((m_vs.pc + 1) & 0xff);
		}
	}

	g_profiler.stop();
}
