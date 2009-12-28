/*===========================================================================
    ASTAT -- ALU/MAC status register
===========================================================================*/

/* flag definitions */
#define SSFLAG			0x80
#define MVFLAG			0x40
#define QFLAG			0x20
#define SFLAG			0x10
#define CFLAG			0x08
#define VFLAG			0x04
#define NFLAG			0x02
#define ZFLAG			0x01

/* extracts flags */
#define GET_SS			(adsp->astat & SSFLAG)
#define GET_MV			(adsp->astat & MVFLAG)
#define GET_Q			(adsp->astat &  QFLAG)
#define GET_S			(adsp->astat &  SFLAG)
#define GET_C			(adsp->astat &  CFLAG)
#define GET_V			(adsp->astat &  VFLAG)
#define GET_N			(adsp->astat &  NFLAG)
#define GET_Z			(adsp->astat &  ZFLAG)

/* clears flags */
#define CLR_SS			(adsp->astat &= ~SSFLAG)
#define CLR_MV			(adsp->astat &= ~MVFLAG)
#define CLR_Q			(adsp->astat &=  ~QFLAG)
#define CLR_S			(adsp->astat &=  ~SFLAG)
#define CLR_C			(adsp->astat &=  ~CFLAG)
#define CLR_V			(adsp->astat &=  ~VFLAG)
#define CLR_N			(adsp->astat &=  ~NFLAG)
#define CLR_Z			(adsp->astat &=  ~ZFLAG)

/* sets flags */
#define SET_SS			(adsp->astat |= SSFLAG)
#define SET_MV			(adsp->astat |= MVFLAG)
#define SET_Q			(adsp->astat |=  QFLAG)
#define SET_S			(adsp->astat |=  SFLAG)
#define SET_C			(adsp->astat |=  CFLAG)
#define SET_V			(adsp->astat |=  VFLAG)
#define SET_Z			(adsp->astat |=  ZFLAG)
#define SET_N			(adsp->astat |=  NFLAG)

/* flag clearing; must be done before setting */
#define CLR_FLAGS		(adsp->astat &= adsp->astat_clear)

/* compute flags */
#define CALC_Z(r)		(adsp->astat |= ((r & 0xffff) == 0))
#define CALC_N(r)		(adsp->astat |= (r >> 14) & 0x02)
#define CALC_V(s,d,r)	(adsp->astat |= ((s ^ d ^ r ^ (r >> 1)) >> 13) & 0x04)
#define CALC_C(r)		(adsp->astat |= (r >> 13) & 0x08)
#define CALC_C_SUB(r)	(adsp->astat |= (~r >> 13) & 0x08)
#define CALC_NZ(r)		CLR_FLAGS; CALC_N(r); CALC_Z(r)
#define CALC_NZV(s,d,r) CLR_FLAGS; CALC_N(r); CALC_Z(r); CALC_V(s,d,r)
#define CALC_NZVC(s,d,r) CLR_FLAGS; CALC_N(r); CALC_Z(r); CALC_V(s,d,r); CALC_C(r)
#define CALC_NZVC_SUB(s,d,r) CLR_FLAGS; CALC_N(r); CALC_Z(r); CALC_V(s,d,r); CALC_C_SUB(r)

/* ADSP-218x constants */
static const INT32 constants[] =
{
	0x0001, 0xfffe, 0x0002, 0xfffd, 0x0004, 0xfffb, 0x0008, 0xfff7,
	0x0010, 0xffef, 0x0020, 0xffdf, 0x0040, 0xffbf, 0x0080, 0xff7f,
	0x0100, 0xfeff, 0x0200, 0xfdff, 0x0400, 0xfbff, 0x0800, 0xf7ff,
	0x1000, 0xefff, 0x2000, 0xdfff, 0x4000, 0xbfff, 0x8000, 0x7fff
};



/*===========================================================================
    MSTAT -- ALU/MAC control register
===========================================================================*/

/* flag definitions */
#define MSTAT_BANK		0x01			/* register bank select */
#define MSTAT_REVERSE	0x02			/* bit-reverse addressing enable (DAG1) */
#define MSTAT_STICKYV	0x04			/* sticky ALU overflow enable */
#define MSTAT_SATURATE	0x08			/* AR saturation mode enable */
#define MSTAT_INTEGER	0x10			/* MAC result placement; 0=fractional, 1=integer */
#define MSTAT_TIMER		0x20			/* timer enable */
#define MSTAT_GOMODE	0x40			/* go mode enable */

/* you must call this in order to change MSTAT */
INLINE void update_mstat(adsp2100_state *adsp)
{
	if ((adsp->mstat ^ adsp->mstat_prev) & MSTAT_BANK)
	{
		ADSPCORE temp = adsp->core;
		adsp->core = adsp->alt;
		adsp->alt = temp;
	}
	if ((adsp->mstat ^ adsp->mstat_prev) & MSTAT_TIMER)
		if (adsp->timer_fired != NULL)
			(*adsp->timer_fired)(adsp->device, (adsp->mstat & MSTAT_TIMER) != 0);
	if (adsp->mstat & MSTAT_STICKYV)
		adsp->astat_clear = ~(CFLAG | NFLAG | ZFLAG);
	else
		adsp->astat_clear = ~(CFLAG | VFLAG | NFLAG | ZFLAG);
	adsp->mstat_prev = adsp->mstat;
}


/*===========================================================================
    SSTAT -- stack status register
===========================================================================*/

/* flag definitions */
#define PC_EMPTY		0x01			/* PC stack empty */
#define PC_OVER			0x02			/* PC stack overflow */
#define COUNT_EMPTY		0x04			/* count stack empty */
#define COUNT_OVER		0x08			/* count stack overflow */
#define STATUS_EMPTY	0x10			/* status stack empty */
#define STATUS_OVER		0x20			/* status stack overflow */
#define LOOP_EMPTY		0x40			/* loop stack empty */
#define LOOP_OVER		0x80			/* loop stack overflow */



/*===========================================================================
    PC stack handlers
===========================================================================*/

INLINE UINT32 pc_stack_top(adsp2100_state *adsp)
{
	if (adsp->pc_sp > 0)
		return adsp->pc_stack[adsp->pc_sp - 1];
	else
		return adsp->pc_stack[0];
}

INLINE void set_pc_stack_top(adsp2100_state *adsp, UINT32 top)
{
	if (adsp->pc_sp > 0)
		adsp->pc_stack[adsp->pc_sp - 1] = top;
	else
		adsp->pc_stack[0] = top;
}

INLINE void pc_stack_push(adsp2100_state *adsp)
{
	if (adsp->pc_sp < PC_STACK_DEPTH)
	{
		adsp->pc_stack[adsp->pc_sp] = adsp->pc;
		adsp->pc_sp++;
		adsp->sstat &= ~PC_EMPTY;
	}
	else
		adsp->sstat |= PC_OVER;
}

INLINE void pc_stack_push_val(adsp2100_state *adsp, UINT32 val)
{
	if (adsp->pc_sp < PC_STACK_DEPTH)
	{
		adsp->pc_stack[adsp->pc_sp] = val;
		adsp->pc_sp++;
		adsp->sstat &= ~PC_EMPTY;
	}
	else
		adsp->sstat |= PC_OVER;
}

INLINE void pc_stack_pop(adsp2100_state *adsp)
{
	if (adsp->pc_sp > 0)
	{
		adsp->pc_sp--;
		if (adsp->pc_sp == 0)
			adsp->sstat |= PC_EMPTY;
	}
	adsp->pc = adsp->pc_stack[adsp->pc_sp];
}

INLINE UINT32 pc_stack_pop_val(adsp2100_state *adsp)
{
	if (adsp->pc_sp > 0)
	{
		adsp->pc_sp--;
		if (adsp->pc_sp == 0)
			adsp->sstat |= PC_EMPTY;
	}
	return adsp->pc_stack[adsp->pc_sp];
}


/*===========================================================================
    CNTR stack handlers
===========================================================================*/

INLINE UINT32 cntr_stack_top(adsp2100_state *adsp)
{
	if (adsp->cntr_sp > 0)
		return adsp->cntr_stack[adsp->cntr_sp - 1];
	else
		return adsp->cntr_stack[0];
}

INLINE void cntr_stack_push(adsp2100_state *adsp)
{
	if (adsp->cntr_sp < CNTR_STACK_DEPTH)
	{
		adsp->cntr_stack[adsp->cntr_sp] = adsp->cntr;
		adsp->cntr_sp++;
		adsp->sstat &= ~COUNT_EMPTY;
	}
	else
		adsp->sstat |= COUNT_OVER;
}

INLINE void cntr_stack_pop(adsp2100_state *adsp)
{
	if (adsp->cntr_sp > 0)
	{
		adsp->cntr_sp--;
		if (adsp->cntr_sp == 0)
			adsp->sstat |= COUNT_EMPTY;
	}
	adsp->cntr = adsp->cntr_stack[adsp->cntr_sp];
}


/*===========================================================================
    LOOP stack handlers
===========================================================================*/

INLINE UINT32 loop_stack_top(adsp2100_state *adsp)
{
	if (adsp->loop_sp > 0)
		return adsp->loop_stack[adsp->loop_sp - 1];
	else
		return adsp->loop_stack[0];
}

INLINE void loop_stack_push(adsp2100_state *adsp, UINT32 value)
{
	if (adsp->loop_sp < LOOP_STACK_DEPTH)
	{
		adsp->loop_stack[adsp->loop_sp] = value;
		adsp->loop_sp++;
		adsp->loop = value >> 4;
		adsp->loop_condition = value & 15;
		adsp->sstat &= ~LOOP_EMPTY;
	}
	else
		adsp->sstat |= LOOP_OVER;
}

INLINE void loop_stack_pop(adsp2100_state *adsp)
{
	if (adsp->loop_sp > 0)
	{
		adsp->loop_sp--;
		if (adsp->loop_sp == 0)
		{
			adsp->loop = 0xffff;
			adsp->loop_condition = 0;
			adsp->sstat |= LOOP_EMPTY;
		}
		else
		{
			adsp->loop = adsp->loop_stack[adsp->loop_sp -1] >> 4;
			adsp->loop_condition = adsp->loop_stack[adsp->loop_sp - 1] & 15;
		}
	}
}


/*===========================================================================
    STAT stack handlers
===========================================================================*/

INLINE void stat_stack_push(adsp2100_state *adsp)
{
	if (adsp->stat_sp < STAT_STACK_DEPTH)
	{
		adsp->stat_stack[adsp->stat_sp][0] = adsp->mstat;
		adsp->stat_stack[adsp->stat_sp][1] = adsp->imask;
		adsp->stat_stack[adsp->stat_sp][2] = adsp->astat;
		adsp->stat_sp++;
		adsp->sstat &= ~STATUS_EMPTY;
	}
	else
		adsp->sstat |= STATUS_OVER;
}

INLINE void stat_stack_pop(adsp2100_state *adsp)
{
	if (adsp->stat_sp > 0)
	{
		adsp->stat_sp--;
		if (adsp->stat_sp == 0)
			adsp->sstat |= STATUS_EMPTY;
	}
	adsp->mstat = adsp->stat_stack[adsp->stat_sp][0];
	update_mstat(adsp);
	adsp->imask = adsp->stat_stack[adsp->stat_sp][1];
	adsp->astat = adsp->stat_stack[adsp->stat_sp][2];
	check_irqs(adsp);
}



/*===========================================================================
    condition code checking
===========================================================================*/

INLINE int CONDITION(adsp2100_state *adsp, int c)
{
	if (c != 14)
		return condition_table[((c) << 8) | adsp->astat];
	else if ((INT32)--adsp->cntr > 0)
		return 1;
	else
	{
		cntr_stack_pop(adsp);
		return 0;
	}
}



/*===========================================================================
    register writing
===========================================================================*/

INLINE void update_i(adsp2100_state *adsp, int which)
{
	adsp->base[which] = adsp->i[which] & adsp->lmask[which];
}

INLINE void update_l(adsp2100_state *adsp, int which)
{
	adsp->lmask[which] = mask_table[adsp->l[which] & 0x3fff];
	adsp->base[which] = adsp->i[which] & adsp->lmask[which];
}

static void wr_inval(adsp2100_state *adsp, INT32 val) { logerror( "ADSP %04x: Writing to an invalid register!\n", adsp->ppc ); }
static void wr_ax0(adsp2100_state *adsp, INT32 val)   { adsp->core.ax0.s = val; }
static void wr_ax1(adsp2100_state *adsp, INT32 val)   { adsp->core.ax1.s = val; }
static void wr_mx0(adsp2100_state *adsp, INT32 val)   { adsp->core.mx0.s = val; }
static void wr_mx1(adsp2100_state *adsp, INT32 val)   { adsp->core.mx1.s = val; }
static void wr_ay0(adsp2100_state *adsp, INT32 val)   { adsp->core.ay0.s = val; }
static void wr_ay1(adsp2100_state *adsp, INT32 val)   { adsp->core.ay1.s = val; }
static void wr_my0(adsp2100_state *adsp, INT32 val)   { adsp->core.my0.s = val; }
static void wr_my1(adsp2100_state *adsp, INT32 val)   { adsp->core.my1.s = val; }
static void wr_si(adsp2100_state *adsp, INT32 val)    { adsp->core.si.s = val; }
static void wr_se(adsp2100_state *adsp, INT32 val)    { adsp->core.se.s = (INT8)val; }
static void wr_ar(adsp2100_state *adsp, INT32 val)    { adsp->core.ar.s = val; }
static void wr_mr0(adsp2100_state *adsp, INT32 val)   { adsp->core.mr.mrx.mr0.s = val; }
static void wr_mr1(adsp2100_state *adsp, INT32 val)   { adsp->core.mr.mrx.mr1.s = val; adsp->core.mr.mrx.mr2.s = (INT16)val >> 15; }
static void wr_mr2(adsp2100_state *adsp, INT32 val)   { adsp->core.mr.mrx.mr2.s = (INT8)val; }
static void wr_sr0(adsp2100_state *adsp, INT32 val)   { adsp->core.sr.srx.sr0.s = val; }
static void wr_sr1(adsp2100_state *adsp, INT32 val)   { adsp->core.sr.srx.sr1.s = val; }
static void wr_i0(adsp2100_state *adsp, INT32 val)    { adsp->i[0] = val & 0x3fff; update_i(adsp, 0); }
static void wr_i1(adsp2100_state *adsp, INT32 val)    { adsp->i[1] = val & 0x3fff; update_i(adsp, 1); }
static void wr_i2(adsp2100_state *adsp, INT32 val)    { adsp->i[2] = val & 0x3fff; update_i(adsp, 2); }
static void wr_i3(adsp2100_state *adsp, INT32 val)    { adsp->i[3] = val & 0x3fff; update_i(adsp, 3); }
static void wr_i4(adsp2100_state *adsp, INT32 val)    { adsp->i[4] = val & 0x3fff; update_i(adsp, 4); }
static void wr_i5(adsp2100_state *adsp, INT32 val)    { adsp->i[5] = val & 0x3fff; update_i(adsp, 5); }
static void wr_i6(adsp2100_state *adsp, INT32 val)    { adsp->i[6] = val & 0x3fff; update_i(adsp, 6); }
static void wr_i7(adsp2100_state *adsp, INT32 val)    { adsp->i[7] = val & 0x3fff; update_i(adsp, 7); }
static void wr_m0(adsp2100_state *adsp, INT32 val)    { adsp->m[0] = (INT32)(val << 18) >> 18; }
static void wr_m1(adsp2100_state *adsp, INT32 val)    { adsp->m[1] = (INT32)(val << 18) >> 18; }
static void wr_m2(adsp2100_state *adsp, INT32 val)    { adsp->m[2] = (INT32)(val << 18) >> 18; }
static void wr_m3(adsp2100_state *adsp, INT32 val)    { adsp->m[3] = (INT32)(val << 18) >> 18; }
static void wr_m4(adsp2100_state *adsp, INT32 val)    { adsp->m[4] = (INT32)(val << 18) >> 18; }
static void wr_m5(adsp2100_state *adsp, INT32 val)    { adsp->m[5] = (INT32)(val << 18) >> 18; }
static void wr_m6(adsp2100_state *adsp, INT32 val)    { adsp->m[6] = (INT32)(val << 18) >> 18; }
static void wr_m7(adsp2100_state *adsp, INT32 val)    { adsp->m[7] = (INT32)(val << 18) >> 18; }
static void wr_l0(adsp2100_state *adsp, INT32 val)    { adsp->l[0] = val & 0x3fff; update_l(adsp, 0); }
static void wr_l1(adsp2100_state *adsp, INT32 val)    { adsp->l[1] = val & 0x3fff; update_l(adsp, 1); }
static void wr_l2(adsp2100_state *adsp, INT32 val)    { adsp->l[2] = val & 0x3fff; update_l(adsp, 2); }
static void wr_l3(adsp2100_state *adsp, INT32 val)    { adsp->l[3] = val & 0x3fff; update_l(adsp, 3); }
static void wr_l4(adsp2100_state *adsp, INT32 val)    { adsp->l[4] = val & 0x3fff; update_l(adsp, 4); }
static void wr_l5(adsp2100_state *adsp, INT32 val)    { adsp->l[5] = val & 0x3fff; update_l(adsp, 5); }
static void wr_l6(adsp2100_state *adsp, INT32 val)    { adsp->l[6] = val & 0x3fff; update_l(adsp, 6); }
static void wr_l7(adsp2100_state *adsp, INT32 val)    { adsp->l[7] = val & 0x3fff; update_l(adsp, 7); }
static void wr_astat(adsp2100_state *adsp, INT32 val) { adsp->astat = val & 0x00ff; }
static void wr_mstat(adsp2100_state *adsp, INT32 val) { adsp->mstat = val & adsp->mstat_mask; update_mstat(adsp); }
static void wr_imask(adsp2100_state *adsp, INT32 val) { adsp->imask = val & adsp->imask_mask; check_irqs(adsp); }
static void wr_icntl(adsp2100_state *adsp, INT32 val) { adsp->icntl = val & 0x001f; check_irqs(adsp); }
static void wr_cntr(adsp2100_state *adsp, INT32 val)  { cntr_stack_push(adsp); adsp->cntr = val & 0x3fff; }
static void wr_sb(adsp2100_state *adsp, INT32 val)    { adsp->core.sb.s = (INT32)(val << 27) >> 27; }
static void wr_px(adsp2100_state *adsp, INT32 val)    { adsp->px = val; }
static void wr_ifc(adsp2100_state *adsp, INT32 val)
{
	adsp->ifc = val;
	if (adsp->chip_type >= CHIP_TYPE_ADSP2181)
	{
		/* clear timer */
		if (val & 0x0002) adsp->irq_latch[ADSP2181_IRQ0] = 0;
		if (val & 0x0004) adsp->irq_latch[ADSP2181_IRQ1] = 0;
		/* clear BDMA */
		if (val & 0x0010) adsp->irq_latch[ADSP2181_IRQE] = 0;
		if (val & 0x0020) adsp->irq_latch[ADSP2181_SPORT0_RX] = 0;
		if (val & 0x0040) adsp->irq_latch[ADSP2181_SPORT0_TX] = 0;
		if (val & 0x0080) adsp->irq_latch[ADSP2181_IRQ2] = 0;
		/* force timer */
		if (val & 0x0200) adsp->irq_latch[ADSP2181_IRQ0] = 1;
		if (val & 0x0400) adsp->irq_latch[ADSP2181_IRQ1] = 1;
		/* force BDMA */
		if (val & 0x1000) adsp->irq_latch[ADSP2181_IRQE] = 1;
		if (val & 0x2000) adsp->irq_latch[ADSP2181_SPORT0_RX] = 1;
		if (val & 0x4000) adsp->irq_latch[ADSP2181_SPORT0_TX] = 1;
		if (val & 0x8000) adsp->irq_latch[ADSP2181_IRQ2] = 1;
	}
	else
	{
		/* clear timer */
		if (val & 0x002) adsp->irq_latch[ADSP2101_IRQ0] = 0;
		if (val & 0x004) adsp->irq_latch[ADSP2101_IRQ1] = 0;
		if (val & 0x008) adsp->irq_latch[ADSP2101_SPORT0_RX] = 0;
		if (val & 0x010) adsp->irq_latch[ADSP2101_SPORT0_TX] = 0;
		if (val & 0x020) adsp->irq_latch[ADSP2101_IRQ2] = 0;
		/* set timer */
		if (val & 0x080) adsp->irq_latch[ADSP2101_IRQ0] = 1;
		if (val & 0x100) adsp->irq_latch[ADSP2101_IRQ1] = 1;
		if (val & 0x200) adsp->irq_latch[ADSP2101_SPORT0_RX] = 1;
		if (val & 0x400) adsp->irq_latch[ADSP2101_SPORT0_TX] = 1;
		if (val & 0x800) adsp->irq_latch[ADSP2101_IRQ2] = 1;
	}
	check_irqs(adsp);
}
static void wr_tx0(adsp2100_state *adsp, INT32 val)	{ if (adsp->sport_tx_callback) (*adsp->sport_tx_callback)(adsp->device, 0, val); }
static void wr_tx1(adsp2100_state *adsp, INT32 val)	{ if (adsp->sport_tx_callback) (*adsp->sport_tx_callback)(adsp->device, 1, val); }
static void wr_owrctr(adsp2100_state *adsp, INT32 val) { adsp->cntr = val & 0x3fff; }
static void wr_topstack(adsp2100_state *adsp, INT32 val) { pc_stack_push_val(adsp, val & 0x3fff); }

#define WRITE_REG(adsp,grp,reg,val) ((*wr_reg[grp][reg])(adsp,val))

static void (*const wr_reg[4][16])(adsp2100_state*, INT32) =
{
	{
		wr_ax0, wr_ax1, wr_mx0, wr_mx1, wr_ay0, wr_ay1, wr_my0, wr_my1,
		wr_si, wr_se, wr_ar, wr_mr0, wr_mr1, wr_mr2, wr_sr0, wr_sr1
	},
	{
		wr_i0, wr_i1, wr_i2, wr_i3, wr_m0, wr_m1, wr_m2, wr_m3,
		wr_l0, wr_l1, wr_l2, wr_l3, wr_inval, wr_inval, wr_inval, wr_inval
	},
	{
		wr_i4, wr_i5, wr_i6, wr_i7, wr_m4, wr_m5, wr_m6, wr_m7,
		wr_l4, wr_l5, wr_l6, wr_l7, wr_inval, wr_inval, wr_inval, wr_inval
	},
	{
		wr_astat, wr_mstat, wr_inval, wr_imask, wr_icntl, wr_cntr, wr_sb, wr_px,
		wr_inval, wr_tx0, wr_inval, wr_tx1, wr_ifc, wr_owrctr, wr_inval, wr_topstack
	}
};



/*===========================================================================
    register reading
===========================================================================*/

static INT32 rd_inval(adsp2100_state *adsp) { logerror( "ADSP %04x: Writing to an invalid register!\n", adsp->ppc ); return 0; }
static INT32 rd_ax0(adsp2100_state *adsp)   { return adsp->core.ax0.s; }
static INT32 rd_ax1(adsp2100_state *adsp)   { return adsp->core.ax1.s; }
static INT32 rd_mx0(adsp2100_state *adsp)   { return adsp->core.mx0.s; }
static INT32 rd_mx1(adsp2100_state *adsp)   { return adsp->core.mx1.s; }
static INT32 rd_ay0(adsp2100_state *adsp)   { return adsp->core.ay0.s; }
static INT32 rd_ay1(adsp2100_state *adsp)   { return adsp->core.ay1.s; }
static INT32 rd_my0(adsp2100_state *adsp)   { return adsp->core.my0.s; }
static INT32 rd_my1(adsp2100_state *adsp)   { return adsp->core.my1.s; }
static INT32 rd_si(adsp2100_state *adsp)    { return adsp->core.si.s; }
static INT32 rd_se(adsp2100_state *adsp)    { return adsp->core.se.s; }
static INT32 rd_ar(adsp2100_state *adsp)    { return adsp->core.ar.s; }
static INT32 rd_mr0(adsp2100_state *adsp)   { return adsp->core.mr.mrx.mr0.s; }
static INT32 rd_mr1(adsp2100_state *adsp)   { return adsp->core.mr.mrx.mr1.s; }
static INT32 rd_mr2(adsp2100_state *adsp)   { return adsp->core.mr.mrx.mr2.s; }
static INT32 rd_sr0(adsp2100_state *adsp)   { return adsp->core.sr.srx.sr0.s; }
static INT32 rd_sr1(adsp2100_state *adsp)   { return adsp->core.sr.srx.sr1.s; }
static INT32 rd_i0(adsp2100_state *adsp)    { return adsp->i[0]; }
static INT32 rd_i1(adsp2100_state *adsp)    { return adsp->i[1]; }
static INT32 rd_i2(adsp2100_state *adsp)    { return adsp->i[2]; }
static INT32 rd_i3(adsp2100_state *adsp)    { return adsp->i[3]; }
static INT32 rd_i4(adsp2100_state *adsp)    { return adsp->i[4]; }
static INT32 rd_i5(adsp2100_state *adsp)    { return adsp->i[5]; }
static INT32 rd_i6(adsp2100_state *adsp)    { return adsp->i[6]; }
static INT32 rd_i7(adsp2100_state *adsp)    { return adsp->i[7]; }
static INT32 rd_m0(adsp2100_state *adsp)    { return adsp->m[0]; }
static INT32 rd_m1(adsp2100_state *adsp)    { return adsp->m[1]; }
static INT32 rd_m2(adsp2100_state *adsp)    { return adsp->m[2]; }
static INT32 rd_m3(adsp2100_state *adsp)    { return adsp->m[3]; }
static INT32 rd_m4(adsp2100_state *adsp)    { return adsp->m[4]; }
static INT32 rd_m5(adsp2100_state *adsp)    { return adsp->m[5]; }
static INT32 rd_m6(adsp2100_state *adsp)    { return adsp->m[6]; }
static INT32 rd_m7(adsp2100_state *adsp)    { return adsp->m[7]; }
static INT32 rd_l0(adsp2100_state *adsp)    { return adsp->l[0]; }
static INT32 rd_l1(adsp2100_state *adsp)    { return adsp->l[1]; }
static INT32 rd_l2(adsp2100_state *adsp)    { return adsp->l[2]; }
static INT32 rd_l3(adsp2100_state *adsp)    { return adsp->l[3]; }
static INT32 rd_l4(adsp2100_state *adsp)    { return adsp->l[4]; }
static INT32 rd_l5(adsp2100_state *adsp)    { return adsp->l[5]; }
static INT32 rd_l6(adsp2100_state *adsp)    { return adsp->l[6]; }
static INT32 rd_l7(adsp2100_state *adsp)    { return adsp->l[7]; }
static INT32 rd_astat(adsp2100_state *adsp) { return adsp->astat; }
static INT32 rd_mstat(adsp2100_state *adsp) { return adsp->mstat; }
static INT32 rd_sstat(adsp2100_state *adsp) { return adsp->sstat; }
static INT32 rd_imask(adsp2100_state *adsp) { return adsp->imask; }
static INT32 rd_icntl(adsp2100_state *adsp) { return adsp->icntl; }
static INT32 rd_cntr(adsp2100_state *adsp)  { return adsp->cntr; }
static INT32 rd_sb(adsp2100_state *adsp)    { return adsp->core.sb.s; }
static INT32 rd_px(adsp2100_state *adsp)    { return adsp->px; }
static INT32 rd_rx0(adsp2100_state *adsp)	{ if (adsp->sport_rx_callback) return (*adsp->sport_rx_callback)(adsp->device, 0); else return 0; }
static INT32 rd_rx1(adsp2100_state *adsp)	{ if (adsp->sport_rx_callback) return (*adsp->sport_rx_callback)(adsp->device, 1); else return 0; }
static INT32 rd_stacktop(adsp2100_state *adsp)	{ return pc_stack_pop_val(adsp); }

#define READ_REG(adsp,grp,reg) ((*rd_reg[grp][reg])(adsp))

static INT32 (*const rd_reg[4][16])(adsp2100_state *adsp) =
{
	{
		rd_ax0, rd_ax1, rd_mx0, rd_mx1, rd_ay0, rd_ay1, rd_my0, rd_my1,
		rd_si, rd_se, rd_ar, rd_mr0, rd_mr1, rd_mr2, rd_sr0, rd_sr1
	},
	{
		rd_i0, rd_i1, rd_i2, rd_i3, rd_m0, rd_m1, rd_m2, rd_m3,
		rd_l0, rd_l1, rd_l2, rd_l3, rd_inval, rd_inval, rd_inval, rd_inval
	},
	{
		rd_i4, rd_i5, rd_i6, rd_i7, rd_m4, rd_m5, rd_m6, rd_m7,
		rd_l4, rd_l5, rd_l6, rd_l7, rd_inval, rd_inval, rd_inval, rd_inval
	},
	{
		rd_astat, rd_mstat, rd_sstat, rd_imask, rd_icntl, rd_cntr, rd_sb, rd_px,
		rd_rx0, rd_inval, rd_rx1, rd_inval, rd_inval, rd_inval, rd_inval, rd_stacktop
	}
};



/*===========================================================================
    Modulus addressing logic
===========================================================================*/

INLINE void modify_address(adsp2100_state *adsp, UINT32 ireg, UINT32 mreg)
{
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;
}



/*===========================================================================
    Data memory accessors
===========================================================================*/

INLINE void data_write_dag1(adsp2100_state *adsp, UINT32 op, INT32 val)
{
	UINT32 ireg = (op >> 2) & 3;
	UINT32 mreg = op & 3;
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];

	if ( adsp->mstat & MSTAT_REVERSE )
	{
		UINT32 ir = reverse_table[ i & 0x3fff ];
		WWORD_DATA(adsp, ir, val);
	}
	else
		WWORD_DATA(adsp, i, val);

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;
}


INLINE UINT32 data_read_dag1(adsp2100_state *adsp, UINT32 op)
{
	UINT32 ireg = (op >> 2) & 3;
	UINT32 mreg = op & 3;
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];
	UINT32 res;

	if (adsp->mstat & MSTAT_REVERSE)
	{
		UINT32 ir = reverse_table[i & 0x3fff];
		res = RWORD_DATA(adsp, ir);
	}
	else
		res = RWORD_DATA(adsp, i);

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;

	return res;
}

INLINE void data_write_dag2(adsp2100_state *adsp, UINT32 op, INT32 val)
{
	UINT32 ireg = 4 + ((op >> 2) & 3);
	UINT32 mreg = 4 + (op & 3);
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];

	WWORD_DATA(adsp, i, val);

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;
}


INLINE UINT32 data_read_dag2(adsp2100_state *adsp, UINT32 op)
{
	UINT32 ireg = 4 + ((op >> 2) & 3);
	UINT32 mreg = 4 + (op & 3);
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];

	UINT32 res = RWORD_DATA(adsp, i);

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;

	return res;
}

/*===========================================================================
    Program memory accessors
===========================================================================*/

INLINE void pgm_write_dag2(adsp2100_state *adsp, UINT32 op, INT32 val)
{
	UINT32 ireg = 4 + ((op >> 2) & 3);
	UINT32 mreg = 4 + (op & 3);
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];

	WWORD_PGM(adsp, i, (val << 8) | adsp->px);

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;
}


INLINE UINT32 pgm_read_dag2(adsp2100_state *adsp, UINT32 op)
{
	UINT32 ireg = 4 + ((op >> 2) & 3);
	UINT32 mreg = 4 + (op & 3);
	UINT32 base = adsp->base[ireg];
	UINT32 i = adsp->i[ireg];
	UINT32 l = adsp->l[ireg];
	UINT32 res;

	res = RWORD_PGM(adsp, i);
	adsp->px = res;
	res >>= 8;

	i += adsp->m[mreg];
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	adsp->i[ireg] = i;

	return res;
}



/*===========================================================================
    register reading
===========================================================================*/

#define ALU_GETXREG_UNSIGNED(a,x) (*(UINT16 *)(a)->alu_xregs[x])
#define ALU_GETYREG_UNSIGNED(a,y) (*(UINT16 *)(a)->alu_yregs[y])

#define MAC_GETXREG_UNSIGNED(a,x) (*(UINT16 *)(a)->mac_xregs[x])
#define MAC_GETXREG_SIGNED(a,x)   (*( INT16 *)(a)->mac_xregs[x])
#define MAC_GETYREG_UNSIGNED(a,y) (*(UINT16 *)(a)->mac_yregs[y])
#define MAC_GETYREG_SIGNED(a,y)   (*( INT16 *)(a)->mac_yregs[y])

#define SHIFT_GETXREG_UNSIGNED(a,x) (*(UINT16 *)(a)->shift_xregs[x])
#define SHIFT_GETXREG_SIGNED(a,x)   (*( INT16 *)(a)->shift_xregs[x])



/*===========================================================================
    ALU operations (result in AR)
===========================================================================*/

static void alu_op_ar(adsp2100_state *adsp, int op)
{
	INT32 xop = (op >> 8) & 7;
	INT32 yop = (op >> 11) & 3;
	INT32 res;

	switch (op & (15<<13))  /*JB*/
	{
		case 0x00<<13:
			/* Y                Clear when y = 0 */
			res = ALU_GETYREG_UNSIGNED(adsp, yop);
			CALC_NZ(res);
			break;
		case 0x01<<13:
			/* Y + 1            PASS 1 when y = 0 */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02<<13:
			/* X + Y + C */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03<<13:
			/* X + Y            X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04<<13:
			/* NOT Y */
			res = ALU_GETYREG_UNSIGNED(adsp, yop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05<<13:
			/* -Y */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06<<13:
			/* X - Y + C - 1    X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07<<13:
			/* X - Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08<<13:
			/* Y - 1            PASS -1 when y = 0 */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09<<13:
			/* Y - X            -X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a<<13:
			/* Y - X + C - 1    -X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b<<13:
			/* NOT X */
			res = ALU_GETXREG_UNSIGNED(adsp, xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c<<13:
			/* X AND Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d<<13:
			/* X OR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e<<13:
			/* X XOR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f<<13:
			/* ABS X */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = (xop & 0x8000) ? -xop : xop;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			CLR_S;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* saturate */
	if ((adsp->mstat & MSTAT_SATURATE) && GET_V) res = GET_C ? -32768 : 32767;

	/* set the final value */
	adsp->core.ar.u = res;
}



/*===========================================================================
    ALU operations (result in AR, constant yop)
===========================================================================*/

static void alu_op_ar_const(adsp2100_state *adsp, int op)
{
	INT32 xop = (op >> 8) & 7;
	INT32 yop = constants[((op >> 5) & 0x07) | ((op >> 8) & 0x18)];
	INT32 res;

	switch (op & (15<<13))  /*JB*/
	{
		case 0x00<<13:
			/* Y                Clear when y = 0 */
			res = yop;
			CALC_NZ(res);
			break;
		case 0x01<<13:
			/* Y + 1            PASS 1 when y = 0 */
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02<<13:
			/* X + Y + C */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03<<13:
			/* X + Y            X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04<<13:
			/* NOT Y */
			res = yop ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05<<13:
			/* -Y */
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06<<13:
			/* X - Y + C - 1    X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07<<13:
			/* X - Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08<<13:
			/* Y - 1            PASS -1 when y = 0 */
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09<<13:
			/* Y - X            -X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a<<13:
			/* Y - X + C - 1    -X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b<<13:
			/* NOT X */
			res = ALU_GETXREG_UNSIGNED(adsp, xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c<<13:
			/* X AND Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d<<13:
			/* X OR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e<<13:
			/* X XOR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f<<13:
			/* ABS X */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = (xop & 0x8000) ? -xop : xop;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			CLR_S;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* saturate */
	if ((adsp->mstat & MSTAT_SATURATE) && GET_V) res = GET_C ? -32768 : 32767;

	/* set the final value */
	adsp->core.ar.u = res;
}



/*===========================================================================
    ALU operations (result in AF)
===========================================================================*/

static void alu_op_af(adsp2100_state *adsp, int op)
{
	INT32 xop = (op >> 8) & 7;
	INT32 yop = (op >> 11) & 3;
	INT32 res;

	switch (op & (15<<13))  /*JB*/
	{
		case 0x00<<13:
			/* Y                Clear when y = 0 */
			res = ALU_GETYREG_UNSIGNED(adsp, yop);
			CALC_NZ(res);
			break;
		case 0x01<<13:
			/* Y + 1            PASS 1 when y = 0 */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02<<13:
			/* X + Y + C */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03<<13:
			/* X + Y            X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04<<13:
			/* NOT Y */
			res = ALU_GETYREG_UNSIGNED(adsp, yop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05<<13:
			/* -Y */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06<<13:
			/* X - Y + C - 1    X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07<<13:
			/* X - Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08<<13:
			/* Y - 1            PASS -1 when y = 0 */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09<<13:
			/* Y - X            -X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a<<13:
			/* Y - X + C - 1    -X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b<<13:
			/* NOT X */
			res = ALU_GETXREG_UNSIGNED(adsp, xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c<<13:
			/* X AND Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d<<13:
			/* X OR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e<<13:
			/* X XOR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f<<13:
			/* ABS X */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = (xop & 0x8000) ? -xop : xop;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			CLR_S;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* set the final value */
	adsp->core.af.u = res;
}



/*===========================================================================
    ALU operations (result in AF, constant yop)
===========================================================================*/

static void alu_op_af_const(adsp2100_state *adsp, int op)
{
	INT32 xop = (op >> 8) & 7;
	INT32 yop = constants[((op >> 5) & 0x07) | ((op >> 8) & 0x18)];
	INT32 res;

	switch (op & (15<<13))  /*JB*/
	{
		case 0x00<<13:
			/* Y                Clear when y = 0 */
			res = yop;
			CALC_NZ(res);
			break;
		case 0x01<<13:
			/* Y + 1            PASS 1 when y = 0 */
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02<<13:
			/* X + Y + C */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03<<13:
			/* X + Y            X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04<<13:
			/* NOT Y */
			res = yop ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05<<13:
			/* -Y */
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06<<13:
			/* X - Y + C - 1    X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07<<13:
			/* X - Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08<<13:
			/* Y - 1            PASS -1 when y = 0 */
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09<<13:
			/* Y - X            -X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a<<13:
			/* Y - X + C - 1    -X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b<<13:
			/* NOT X */
			res = ALU_GETXREG_UNSIGNED(adsp, xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c<<13:
			/* X AND Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d<<13:
			/* X OR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e<<13:
			/* X XOR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f<<13:
			/* ABS X */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = (xop & 0x8000) ? -xop : xop;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			CLR_S;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* set the final value */
	adsp->core.af.u = res;
}



/*===========================================================================
    ALU operations (no result)
===========================================================================*/

static void alu_op_none(adsp2100_state *adsp, int op)
{
	INT32 xop = (op >> 8) & 7;
	INT32 yop = (op >> 11) & 3;
	INT32 res;

	switch (op & (15<<13))  /*JB*/
	{
		case 0x00<<13:
			/* Y                Clear when y = 0 */
			res = ALU_GETYREG_UNSIGNED(adsp, yop);
			CALC_NZ(res);
			break;
		case 0x01<<13:
			/* Y + 1            PASS 1 when y = 0 */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02<<13:
			/* X + Y + C */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03<<13:
			/* X + Y            X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04<<13:
			/* NOT Y */
			res = ALU_GETYREG_UNSIGNED(adsp, yop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05<<13:
			/* -Y */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06<<13:
			/* X - Y + C - 1    X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07<<13:
			/* X - Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08<<13:
			/* Y - 1            PASS -1 when y = 0 */
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09<<13:
			/* Y - X            -X when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a<<13:
			/* Y - X + C - 1    -X + C - 1 when y = 0 */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b<<13:
			/* NOT X */
			res = ALU_GETXREG_UNSIGNED(adsp, xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c<<13:
			/* X AND Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d<<13:
			/* X OR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e<<13:
			/* X XOR Y */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			yop = ALU_GETYREG_UNSIGNED(adsp, yop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f<<13:
			/* ABS X */
			xop = ALU_GETXREG_UNSIGNED(adsp, xop);
			res = (xop & 0x8000) ? -xop : xop;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			CLR_S;
			if (xop & 0x8000) SET_S;
			break;
	}
}



/*===========================================================================
    MAC operations (result in MR)
===========================================================================*/

static void mac_op_mr(adsp2100_state *adsp, int op)
{
	INT8 shift = ((adsp->mstat & MSTAT_INTEGER) >> 4) ^ 1;
	INT32 xop = (op >> 8) & 7;
	INT32 yop = (op >> 11) & 3;
	INT32 temp;
	INT64 res;

	switch (op & (15<<13))	/*JB*/
	{
		case 0x00<<13:
			/* no-op */
			return;
		case 0x01<<13:
			/* X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x02<<13:
			/* MR + X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x03<<13:
			/* MR - X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x04<<13:
			/* X * Y (SS)       Clear when y = 0 */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x05<<13:
			/* X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x06<<13:
			/* X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x07<<13:
			/* X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x08<<13:
			/* MR + X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x09<<13:
			/* MR + X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0a<<13:
			/* MR + X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0b<<13:
			/* MR + X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0c<<13:
			/* MR - X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0d<<13:
			/* MR - X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0e<<13:
			/* MR - X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0f<<13:
			/* MR - X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* set the final value */
	temp = (res >> 31) & 0x1ff;
	CLR_MV;
	if (temp != 0x000 && temp != 0x1ff) SET_MV;
	adsp->core.mr.mr = res;
}



/*===========================================================================
    MAC operations (result in MR, yop == xop)
===========================================================================*/

static void mac_op_mr_xop(adsp2100_state *adsp, int op)
{
	INT8 shift = ((adsp->mstat & MSTAT_INTEGER) >> 4) ^ 1;
	INT32 xop = (op >> 8) & 7;
	INT32 temp;
	INT64 res;

	switch (op & (15<<13))	/*JB*/
	{
		case 0x00<<13:
			/* no-op */
			return;
		case 0x01<<13:
			/* X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x02<<13:
			/* MR + X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x03<<13:
			/* MR - X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x04<<13:
			/* X * Y (SS)       Clear when y = 0 */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x05<<13:
			/* X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x06<<13:
			/* X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x07<<13:
			/* X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x08<<13:
			/* MR + X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x09<<13:
			/* MR + X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0a<<13:
			/* MR + X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0b<<13:
			/* MR + X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0c<<13:
			/* MR - X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0d<<13:
			/* MR - X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0e<<13:
			/* MR - X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0f<<13:
			/* MR - X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* set the final value */
	temp = (res >> 31) & 0x1ff;
	CLR_MV;
	if (temp != 0x000 && temp != 0x1ff) SET_MV;
	adsp->core.mr.mr = res;
}



/*===========================================================================
    MAC operations (result in MF)
===========================================================================*/

static void mac_op_mf(adsp2100_state *adsp, int op)
{
	INT8 shift = ((adsp->mstat & MSTAT_INTEGER) >> 4) ^ 1;
	INT32 xop = (op >> 8) & 7;
	INT32 yop = (op >> 11) & 3;
	INT32 temp;
	INT64 res;

	switch (op & (15<<13))	/*JB*/
	{
		case 0x00<<13:
			/* no-op */
			return;
		case 0x01<<13:
			/* X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x02<<13:
			/* MR + X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x03<<13:
			/* MR - X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x04<<13:
			/* X * Y (SS)       Clear when y = 0 */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x05<<13:
			/* X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x06<<13:
			/* X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x07<<13:
			/* X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = (INT64)temp;
			break;
		case 0x08<<13:
			/* MR + X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x09<<13:
			/* MR + X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0a<<13:
			/* MR + X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0b<<13:
			/* MR + X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0c<<13:
			/* MR - X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0d<<13:
			/* MR - X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0e<<13:
			/* MR - X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_SIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0f<<13:
			/* MR - X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			yop = MAC_GETYREG_UNSIGNED(adsp, yop);
			temp = (xop * yop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* set the final value */
	adsp->core.mf.u = (UINT32)res >> 16;
}



/*===========================================================================
    MAC operations (result in MF, yop == xop)
===========================================================================*/

static void mac_op_mf_xop(adsp2100_state *adsp, int op)
{
	INT8 shift = ((adsp->mstat & MSTAT_INTEGER) >> 4) ^ 1;
	INT32 xop = (op >> 8) & 7;
	INT32 temp;
	INT64 res;

	switch (op & (15<<13))	/*JB*/
	{
		case 0x00<<13:
			/* no-op */
			return;
		case 0x01<<13:
			/* X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x02<<13:
			/* MR + X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x03<<13:
			/* MR - X * Y (RND) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((UINT64)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((UINT64)0x10000);
#endif
			break;
		case 0x04<<13:
			/* X * Y (SS)       Clear when y = 0 */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x05<<13:
			/* X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x06<<13:
			/* X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x07<<13:
			/* X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = (INT64)temp;
			break;
		case 0x08<<13:
			/* MR + X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x09<<13:
			/* MR + X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0a<<13:
			/* MR + X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0b<<13:
			/* MR + X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr + (INT64)temp;
			break;
		case 0x0c<<13:
			/* MR - X * Y (SS) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0d<<13:
			/* MR - X * Y (SU) */
			xop = MAC_GETXREG_SIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0e<<13:
			/* MR - X * Y (US) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		case 0x0f<<13:
			/* MR - X * Y (UU) */
			xop = MAC_GETXREG_UNSIGNED(adsp, xop);
			temp = (xop * xop) << shift;
			res = adsp->core.mr.mr - (INT64)temp;
			break;
		default:
			res = 0;	/* just to keep the compiler happy */
			break;
	}

	/* set the final value */
	adsp->core.mf.u = (UINT32)res >> 16;
}



/*===========================================================================
    SHIFT operations (result in SR/SE/SB)
===========================================================================*/

static void shift_op(adsp2100_state *adsp, int op)
{
	INT8 sc = adsp->core.se.s;
	INT32 xop = (op >> 8) & 7;
	UINT32 res;

	switch (op & (15<<11))	/*JB*/
	{
		case 0x00<<11:
			/* LSHIFT (HI) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((UINT32)xop >> -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x01<<11:
			/* LSHIFT (HI, OR) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((UINT32)xop >> -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x02<<11:
			/* LSHIFT (LO) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x03<<11:
			/* LSHIFT (LO, OR) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x04<<11:
			/* ASHIFT (HI) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr = res;
			break;
		case 0x05<<11:
			/* ASHIFT (HI, OR) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr |= res;
			break;
		case 0x06<<11:
			/* ASHIFT (LO) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr = res;
			break;
		case 0x07<<11:
			/* ASHIFT (LO, OR) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr |= res;
			break;
		case 0x08<<11:
			/* NORM (HI) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0)
			{
				xop = ((UINT32)xop >> 1) | ((adsp->astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x09<<11:
			/* NORM (HI, OR) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0)
			{
				xop = ((UINT32)xop >> 1) | ((adsp->astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x0a<<11:
			/* NORM (LO) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x0b<<11:
			/* NORM (LO, OR) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x0c<<11:
			/* EXP (HI) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			res = 0;
			if (xop < 0)
			{
				SET_SS;
				while ((xop & 0x40000000) != 0) res++, xop <<= 1;
			}
			else
			{
				CLR_SS;
				xop |= 0x8000;
				while ((xop & 0x40000000) == 0) res++, xop <<= 1;
			}
			adsp->core.se.s = -res;
			break;
		case 0x0d<<11:
			/* EXP (HIX) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (GET_V)
			{
				adsp->core.se.s = 1;
				if (xop < 0) CLR_SS;
				else SET_SS;
			}
			else
			{
				res = 0;
				if (xop < 0)
				{
					SET_SS;
					while ((xop & 0x40000000) != 0) res++, xop <<= 1;
				}
				else
				{
					CLR_SS;
					xop |= 0x8000;
					while ((xop & 0x40000000) == 0) res++, xop <<= 1;
				}
				adsp->core.se.s = -res;
			}
			break;
		case 0x0e<<11:
			/* EXP (LO) */
			if (adsp->core.se.s == -15)
			{
				xop = SHIFT_GETXREG_SIGNED(adsp, xop);
				res = 15;
				if (GET_SS)
					while ((xop & 0x8000) != 0) res++, xop <<= 1;
				else
				{
					xop = (xop << 1) | 1;
					while ((xop & 0x10000) == 0) res++, xop <<= 1;
				}
				adsp->core.se.s = -res;
			}
			break;
		case 0x0f<<11:
			/* EXPADJ */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			res = 0;
			if (xop < 0)
				while ((xop & 0x40000000) != 0) res++, xop <<= 1;
			else
			{
				xop |= 0x8000;
				while ((xop & 0x40000000) == 0) res++, xop <<= 1;
			}
			if (res < -adsp->core.sb.s)
				adsp->core.sb.s = -res;
			break;
	}
}



/*===========================================================================
    Immediate SHIFT operations (result in SR/SE/SB)
===========================================================================*/

static void shift_op_imm(adsp2100_state *adsp, int op)
{
	INT8 sc = (INT8)op;
	INT32 xop = (op >> 8) & 7;
	UINT32 res;

	switch (op & (15<<11))	/*JB*/
	{
		case 0x00<<11:
			/* LSHIFT (HI) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((UINT32)xop >> -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x01<<11:
			/* LSHIFT (HI, OR) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((UINT32)xop >> -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x02<<11:
			/* LSHIFT (LO) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x03<<11:
			/* LSHIFT (LO, OR) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x04<<11:
			/* ASHIFT (HI) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr = res;
			break;
		case 0x05<<11:
			/* ASHIFT (HI, OR) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr |= res;
			break;
		case 0x06<<11:
			/* ASHIFT (LO) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr = res;
			break;
		case 0x07<<11:
			/* ASHIFT (LO, OR) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			adsp->core.sr.sr |= res;
			break;
		case 0x08<<11:
			/* NORM (HI) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0)
			{
				xop = ((UINT32)xop >> 1) | ((adsp->astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x09<<11:
			/* NORM (HI, OR) */
			xop = SHIFT_GETXREG_SIGNED(adsp, xop) << 16;
			if (sc > 0)
			{
				xop = ((UINT32)xop >> 1) | ((adsp->astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
		case 0x0a<<11:
			/* NORM (LO) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr = res;
			break;
		case 0x0b<<11:
			/* NORM (LO, OR) */
			xop = SHIFT_GETXREG_UNSIGNED(adsp, xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			adsp->core.sr.sr |= res;
			break;
	}
}
