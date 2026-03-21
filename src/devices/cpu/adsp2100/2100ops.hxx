// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*===========================================================================
    ASTAT -- ALU/MAC status register
===========================================================================*/

// extracts flags
#define GET_SS          (m_astat & SSFLAG)
#define GET_MV          (m_astat & MVFLAG)
#define GET_Q           (m_astat &  QFLAG)
#define GET_S           (m_astat &  SFLAG)
#define GET_C           (m_astat &  CFLAG)
#define GET_V           (m_astat &  VFLAG)
#define GET_N           (m_astat &  NFLAG)
#define GET_Z           (m_astat &  ZFLAG)

// clears flags
#define CLR_SS          (m_astat &= ~SSFLAG)
#define CLR_MV          (m_astat &= ~MVFLAG)
#define CLR_Q           (m_astat &=  ~QFLAG)
#define CLR_S           (m_astat &=  ~SFLAG)
#define CLR_C           (m_astat &=  ~CFLAG)
#define CLR_V           (m_astat &=  ~VFLAG)
#define CLR_N           (m_astat &=  ~NFLAG)
#define CLR_Z           (m_astat &=  ~ZFLAG)

// sets flags
#define SET_SS          (m_astat |= SSFLAG)
#define SET_MV          (m_astat |= MVFLAG)
#define SET_Q           (m_astat |=  QFLAG)
#define SET_S           (m_astat |=  SFLAG)
#define SET_C           (m_astat |=  CFLAG)
#define SET_V           (m_astat |=  VFLAG)
#define SET_Z           (m_astat |=  ZFLAG)
#define SET_N           (m_astat |=  NFLAG)

// flag clearing; must be done before setting
#define CLR_FLAGS       (m_astat &= m_astat_clear)

// compute flags
#define CALC_Z(r)       (m_astat |= ((r & 0xffff) == 0))
#define CALC_N(r)       (m_astat |= (r >> 14) & 0x02)
#define CALC_V(s,d,r)   (m_astat |= ((s ^ d ^ r ^ (r >> 1)) >> 13) & 0x04)
#define CALC_C(r)       (m_astat |= (r >> 13) & 0x08)
#define CALC_C_SUB(r)   (m_astat |= (~r >> 13) & 0x08)
#define CALC_NZ(r)      CLR_FLAGS; CALC_N(r); CALC_Z(r)
#define CALC_NZV(s,d,r) CLR_FLAGS; CALC_N(r); CALC_Z(r); CALC_V(s,d,r)
#define CALC_NZVC(s,d,r) CLR_FLAGS; CALC_N(r); CALC_Z(r); CALC_V(s,d,r); CALC_C(r)
#define CALC_NZVC_SUB(s,d,r) CLR_FLAGS; CALC_N(r); CALC_Z(r); CALC_V(s,d,r); CALC_C_SUB(r)

// ADSP-218x constants
static const int32_t constants[] =
{
	0x0001, 0xfffe, 0x0002, 0xfffd, 0x0004, 0xfffb, 0x0008, 0xfff7,
	0x0010, 0xffef, 0x0020, 0xffdf, 0x0040, 0xffbf, 0x0080, 0xff7f,
	0x0100, 0xfeff, 0x0200, 0xfdff, 0x0400, 0xfbff, 0x0800, 0xf7ff,
	0x1000, 0xefff, 0x2000, 0xdfff, 0x4000, 0xbfff, 0x8000, 0x7fff
};



/*===========================================================================
    MSTAT -- ALU/MAC control register
===========================================================================*/

// flag definitions
enum : uint32_t
{
	MSTAT_BANK      = 0x01,             // register bank select
	MSTAT_REVERSE   = 0x02,             // bit-reverse addressing enable (DAG1)
	MSTAT_STICKYV   = 0x04,             // sticky ALU overflow enable
	MSTAT_SATURATE  = 0x08,             // AR saturation mode enable
	MSTAT_INTEGER   = 0x10,             // MAC result placement; 0=fractional, 1=integer
	MSTAT_TIMER     = 0x20,             // timer enable
	MSTAT_GOMODE    = 0x40              // go mode enable
};

// you must call this in order to change MSTAT
inline void adsp21xx_device::update_mstat()
{
	if ((m_mstat ^ m_mstat_prev) & MSTAT_BANK)
	{
		adsp_core temp = m_core;
		m_core = m_alt;
		m_alt = temp;
	}
	if ((m_mstat ^ m_mstat_prev) & MSTAT_TIMER)
		m_timer_fired_cb((m_mstat & MSTAT_TIMER) != 0);
	if (m_mstat & MSTAT_STICKYV)
		m_astat_clear = ~(CFLAG | NFLAG | ZFLAG);
	else
		m_astat_clear = ~(CFLAG | VFLAG | NFLAG | ZFLAG);
	m_mstat_prev = m_mstat;
}


/*===========================================================================
    SSTAT -- stack status register
===========================================================================*/

// flag definitions
enum : uint32_t
{
	PC_EMPTY        = 0x01,            // PC stack empty
	PC_OVER         = 0x02,            // PC stack overflow
	COUNT_EMPTY     = 0x04,            // count stack empty
	COUNT_OVER      = 0x08,            // count stack overflow
	STATUS_EMPTY    = 0x10,            // status stack empty
	STATUS_OVER     = 0x20,            // status stack overflow
	LOOP_EMPTY      = 0x40,            // loop stack empty
	LOOP_OVER       = 0x80             // loop stack overflow
};


/*===========================================================================
    PC stack handlers
===========================================================================*/

inline uint32_t adsp21xx_device::pc_stack_top()
{
	if (m_pc_sp > 0)
		return m_pc_stack[m_pc_sp - 1];
	else
		return m_pc_stack[0];
}

inline void adsp21xx_device::set_pc_stack_top(uint32_t top)
{
	if (m_pc_sp > 0)
		m_pc_stack[m_pc_sp - 1] = top;
	else
		m_pc_stack[0] = top;
}

inline void adsp21xx_device::pc_stack_push()
{
	if (m_pc_sp < PC_STACK_DEPTH)
	{
		m_pc_stack[m_pc_sp] = m_pc;
		m_pc_sp++;
		m_sstat &= ~PC_EMPTY;
	}
	else
		m_sstat |= PC_OVER;
}

inline void adsp21xx_device::pc_stack_push_val(uint32_t val)
{
	if (m_pc_sp < PC_STACK_DEPTH)
	{
		m_pc_stack[m_pc_sp] = val;
		m_pc_sp++;
		m_sstat &= ~PC_EMPTY;
	}
	else
		m_sstat |= PC_OVER;
}

inline void adsp21xx_device::pc_stack_pop()
{
	if (m_pc_sp > 0)
	{
		m_pc_sp--;
		if (m_pc_sp == 0)
			m_sstat |= PC_EMPTY;
	}
	m_pc = m_pc_stack[m_pc_sp];
}

inline uint32_t adsp21xx_device::pc_stack_pop_val()
{
	if (m_pc_sp > 0)
	{
		m_pc_sp--;
		if (m_pc_sp == 0)
			m_sstat |= PC_EMPTY;
	}
	return m_pc_stack[m_pc_sp];
}


/*===========================================================================
    CNTR stack handlers
===========================================================================*/

inline uint32_t adsp21xx_device::cntr_stack_top()
{
	if (m_cntr_sp > 0)
		return m_cntr_stack[m_cntr_sp - 1];
	else
		return m_cntr_stack[0];
}

inline void adsp21xx_device::cntr_stack_push()
{
	if (m_cntr_sp < CNTR_STACK_DEPTH)
	{
		m_cntr_stack[m_cntr_sp] = m_cntr;
		m_cntr_sp++;
		m_sstat &= ~COUNT_EMPTY;
	}
	else
		m_sstat |= COUNT_OVER;
}

inline void adsp21xx_device::cntr_stack_pop()
{
	if (m_cntr_sp > 0)
	{
		m_cntr_sp--;
		if (m_cntr_sp == 0)
			m_sstat |= COUNT_EMPTY;
	}
	m_cntr = m_cntr_stack[m_cntr_sp];
}


/*===========================================================================
    LOOP stack handlers
===========================================================================*/

inline uint32_t adsp21xx_device::loop_stack_top()
{
	if (m_loop_sp > 0)
		return m_loop_stack[m_loop_sp - 1];
	else
		return m_loop_stack[0];
}

inline void adsp21xx_device::loop_stack_push(uint32_t value)
{
	if (m_loop_sp < LOOP_STACK_DEPTH)
	{
		m_loop_stack[m_loop_sp] = value;
		m_loop_sp++;
		m_loop = value >> 4;
		m_loop_condition = value & 15;
		m_sstat &= ~LOOP_EMPTY;
	}
	else
		m_sstat |= LOOP_OVER;
}

inline void adsp21xx_device::loop_stack_pop()
{
	if (m_loop_sp > 0)
	{
		m_loop_sp--;
		if (m_loop_sp == 0)
		{
			m_loop = 0xffff;
			m_loop_condition = 0;
			m_sstat |= LOOP_EMPTY;
		}
		else
		{
			m_loop = m_loop_stack[m_loop_sp - 1] >> 4;
			m_loop_condition = m_loop_stack[m_loop_sp - 1] & 15;
		}
	}
}


/*===========================================================================
    STAT stack handlers
===========================================================================*/

inline void adsp21xx_device::stat_stack_push()
{
	if (m_stat_sp < STAT_STACK_DEPTH)
	{
		m_stat_stack[m_stat_sp][0] = m_mstat;
		m_stat_stack[m_stat_sp][1] = m_imask;
		m_stat_stack[m_stat_sp][2] = m_astat;
		m_stat_sp++;
		m_sstat &= ~STATUS_EMPTY;
	}
	else
		m_sstat |= STATUS_OVER;
}

inline void adsp21xx_device::stat_stack_pop()
{
	if (m_stat_sp > 0)
	{
		m_stat_sp--;
		if (m_stat_sp == 0)
			m_sstat |= STATUS_EMPTY;
	}
	m_mstat = m_stat_stack[m_stat_sp][0];
	update_mstat();
	m_imask = m_stat_stack[m_stat_sp][1];
	m_astat = m_stat_stack[m_stat_sp][2];
	check_irqs();
}



/*===========================================================================
    condition code checking
===========================================================================*/

// gcc doesn't want to inline this, so we use a macro
#define condition(c) (((c) != 14) ? (m_condition_table[((c) << 8) | m_astat]) : slow_condition())

/*
inline int adsp21xx_device::condition(int c)
{
    if (c != 14)
        return m_condition_table[((c) << 8) | m_astat];
    else
        return slow_condition();
}
*/

int adsp21xx_device::slow_condition()
{
	if ((int32_t)--m_cntr > 0)
		return 1;
	else
	{
		cntr_stack_pop();
		return 0;
	}
}



/*===========================================================================
    register writing
===========================================================================*/

inline void adsp21xx_device::update_i(int which)
{
	m_base[which] = m_i[which] & m_lmask[which];
}

inline void adsp21xx_device::update_l(int which)
{
	m_lmask[which] = m_mask_table[m_l[which] & 0x3fff];
	m_base[which] = m_i[which] & m_lmask[which];
}

inline void adsp21xx_device::update_dmovlay()
{
	m_dmovlay_cb(m_dmovlay);
}

void adsp21xx_device::write_reg0(int regnum, int32_t val)
{
	switch (regnum)
	{
		case 0x00:  m_core.ax0.s = val;                 break;
		case 0x01:  m_core.ax1.s = val;                 break;
		case 0x02:  m_core.mx0.s = val;                 break;
		case 0x03:  m_core.mx1.s = val;                 break;
		case 0x04:  m_core.ay0.s = val;                 break;
		case 0x05:  m_core.ay1.s = val;                 break;
		case 0x06:  m_core.my0.s = val;                 break;
		case 0x07:  m_core.my1.s = val;                 break;
		case 0x08:  m_core.si.s = val;                  break;
		case 0x09:  m_core.se.s = (int8_t)val;          break;
		case 0x0a:  m_core.ar.s = val;                  break;
		case 0x0b:  m_core.mr.mrx.mr0.s = val;          break;
		case 0x0c:  m_core.mr.mrx.mr1.s = val; m_core.mr.mrx.mr2.s = (int16_t)val >> 15;  break;
		case 0x0d:  m_core.mr.mrx.mr2.s = (int8_t)val;  break;
		case 0x0e:  m_core.sr.srx.sr0.s = val;          break;
		case 0x0f:  m_core.sr.srx.sr1.s = val;          break;
	}
}

void adsp21xx_device::write_reg1(int regnum, int32_t val)
{
	int index = regnum & 3;
	switch (regnum >> 2)
	{
		case 0:
			m_i[index] = val & 0x3fff;
			update_i(index);
			break;

		case 1:
			m_m[index] = util::sext(val, 14);
			break;

		case 2:
			m_l[index] = val & 0x3fff;
			update_l(index);
			break;

		case 3:
			if (m_chip_type == CHIP_TYPE_ADSP2181) // ADSP2181 Overlay register
			{
				switch (index)
				{
					case 2:
						m_pmovlay = val & 0x3fff;
						//update_pmovlay();
						break;
					case 3:
						m_dmovlay = val & 0x3fff;
						update_dmovlay();
						break;
					default:
						logerror("ADSP %04x: Writing to an invalid register! RGP=01 RegCode=%1X Val=%04X\n", m_ppc, regnum, val);
						break;
				}
			}
			else
				logerror("ADSP %04x: Writing to an invalid register! RGP=01 RegCode=%1X Val=%04X\n", m_ppc, regnum, val);
			break;
	}
}

void adsp21xx_device::write_reg2(int regnum, int32_t val)
{
	int index = 4 + (regnum & 3);
	switch (regnum >> 2)
	{
		case 0:
			m_i[index] = val & 0x3fff;
			update_i(index);
			break;

		case 1:
			m_m[index] = util::sext(val, 14);
			break;

		case 2:
			m_l[index] = val & 0x3fff;
			update_l(index);
			break;

		case 3:
			logerror("ADSP %04x: Writing to an invalid register!\n", m_ppc);
			break;
	}
}

void adsp21xx_device::write_reg3(int regnum, int32_t val)
{
	switch (regnum)
	{
		case 0x00:  m_astat = val & 0x00ff;                         break;
		case 0x01:  m_mstat = val & m_mstat_mask; update_mstat();   break;
		case 0x03:  m_imask = val & m_imask_mask; check_irqs();     break;
		case 0x04:  m_icntl = val & 0x001f; check_irqs();           break;
		case 0x05:  cntr_stack_push(); m_cntr = val & 0x3fff;       break;
		case 0x06:  m_core.sb.s = util::sext(val, 5);               break;
		case 0x07:  m_px = val;                                     break;
		case 0x09:  m_sport_tx_cb(0, val, 0xffff);                  break;
		case 0x0b:  m_sport_tx_cb(1, val, 0xffff);                  break;
		case 0x0c:
			m_ifc = val;
			if (m_chip_type >= CHIP_TYPE_ADSP2181)
			{
				// clear timer
				if (BIT(val, 1)) m_irq_latch[ADSP2181_IRQ0] = 0;
				if (BIT(val, 2)) m_irq_latch[ADSP2181_IRQ1] = 0;
				if (BIT(val, 3)) m_irq_latch[ADSP2181_BDMA] = 0;
				if (BIT(val, 4)) m_irq_latch[ADSP2181_IRQE] = 0;
				if (BIT(val, 5)) m_irq_latch[ADSP2181_SPORT0_RX] = 0;
				if (BIT(val, 6)) m_irq_latch[ADSP2181_SPORT0_TX] = 0;
				if (BIT(val, 7)) m_irq_latch[ADSP2181_IRQ2] = 0;
				// force timer
				if (BIT(val, 9)) m_irq_latch[ADSP2181_IRQ0] = 1;
				if (BIT(val, 10)) m_irq_latch[ADSP2181_IRQ1] = 1;
				if (BIT(val, 11)) m_irq_latch[ADSP2181_BDMA] = 1;
				if (BIT(val, 12)) m_irq_latch[ADSP2181_IRQE] = 1;
				if (BIT(val, 13)) m_irq_latch[ADSP2181_SPORT0_RX] = 1;
				if (BIT(val, 14)) m_irq_latch[ADSP2181_SPORT0_TX] = 1;
				if (BIT(val, 15)) m_irq_latch[ADSP2181_IRQ2] = 1;
			}
			else
			{
				// clear timer
				if (BIT(val, 1)) m_irq_latch[ADSP2101_IRQ0] = 0;
				if (BIT(val, 2)) m_irq_latch[ADSP2101_IRQ1] = 0;
				if (BIT(val, 3)) m_irq_latch[ADSP2101_SPORT0_RX] = 0;
				if (BIT(val, 4)) m_irq_latch[ADSP2101_SPORT0_TX] = 0;
				if (BIT(val, 5)) m_irq_latch[ADSP2101_IRQ2] = 0;
				// set timer
				if (BIT(val, 7)) m_irq_latch[ADSP2101_IRQ0] = 1;
				if (BIT(val, 8)) m_irq_latch[ADSP2101_IRQ1] = 1;
				if (BIT(val, 9)) m_irq_latch[ADSP2101_SPORT0_RX] = 1;
				if (BIT(val, 10)) m_irq_latch[ADSP2101_SPORT0_TX] = 1;
				if (BIT(val, 11)) m_irq_latch[ADSP2101_IRQ2] = 1;
			}
			check_irqs();
			break;
		case 0x0d:  m_cntr = val & 0x3fff;              break;
		case 0x0f:  pc_stack_push_val(val & 0x3fff);    break;
		default:    logerror("ADSP %04x: Writing to an invalid register!\n", m_ppc); break;
	}
}

#define WRITE_REG(adsp,grp,reg,val) ((this->*wr_reg[grp][reg])(val))



/*===========================================================================
    register reading
===========================================================================*/

int32_t adsp21xx_device::read_reg0(int regnum)
{
	return *m_read0_ptr[regnum];
}

int32_t adsp21xx_device::read_reg1(int regnum)
{
	int32_t ret = *m_read1_ptr[regnum];
	// ADSP2181 Overlay register
	if ((m_chip_type == CHIP_TYPE_ADSP2181) && (regnum >= 0xe))
	{
		switch (regnum)
		{
			case 0xe: ret = m_pmovlay; break;
			case 0xf: ret = m_dmovlay; break;
			default:                   break;
		}
	}

	return ret;
}

int32_t adsp21xx_device::read_reg2(int regnum)
{
	return *m_read2_ptr[regnum];
}

int32_t adsp21xx_device::read_reg3(int regnum)
{
	switch (regnum)
	{
		case 0x00:  return m_astat;
		case 0x01:  return m_mstat;
		case 0x02:  return m_sstat;
		case 0x03:  return m_imask;
		case 0x04:  return m_icntl;
		case 0x05:  return m_cntr;
		case 0x06:  return m_core.sb.s;
		case 0x07:  return m_px;
		case 0x08:  return m_sport_rx_cb(0);
		case 0x0a:  return m_sport_rx_cb(1);
		case 0x0f:  return pc_stack_pop_val();
		default:    logerror("ADSP %04x: Reading from an invalid register! RGP=b11 RegCode=%1X\n", m_ppc, regnum); return 0;
	}
}



/*===========================================================================
    Modulus addressing logic
===========================================================================*/

inline void adsp21xx_device::modify_address(uint32_t ireg, uint32_t mreg)
{
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	m_i[ireg] = i;
}



/*===========================================================================
    Data memory accessors
===========================================================================*/

inline void adsp21xx_device::data_write_dag1(uint32_t op, int32_t val)
{
	uint32_t ireg = BIT(op, 2, 2);
	uint32_t mreg = BIT(op, 0, 2);
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];

	if ( m_mstat & MSTAT_REVERSE )
	{
		uint32_t ir = m_reverse_table[ i & 0x3fff ];
		data_write(ir, val);
	}
	else
		data_write(i, val);

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;

	m_i[ireg] = i;
}


inline uint32_t adsp21xx_device::data_read_dag1(uint32_t op)
{
	uint32_t ireg = BIT(op, 2, 2);
	uint32_t mreg = BIT(op, 0, 2);
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];
	uint32_t res;

	if (m_mstat & MSTAT_REVERSE)
	{
		uint32_t ir = m_reverse_table[i & 0x3fff];
		res = data_read(ir);
	}
	else
		res = data_read(i);

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	m_i[ireg] = i;

	return res;
}

inline void adsp21xx_device::data_write_dag2(uint32_t op, int32_t val)
{
	uint32_t ireg = 4 + BIT(op, 2, 2);
	uint32_t mreg = 4 + BIT(op, 0, 2);
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];

	data_write(i, val);

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	m_i[ireg] = i;
}


inline uint32_t adsp21xx_device::data_read_dag2(uint32_t op)
{
	uint32_t ireg = 4 + BIT(op, 2, 2);
	uint32_t mreg = 4 + BIT(op, 0, 2);
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];

	uint32_t res = data_read(i);

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	m_i[ireg] = i;

	return res;
}

/*===========================================================================
    Program memory accessors
===========================================================================*/

inline void adsp21xx_device::pgm_write_dag2(uint32_t op, int32_t val)
{
	uint32_t ireg = 4 + BIT(op, 2, 2);
	uint32_t mreg = 4 + BIT(op, 0, 2);
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];

	program_write(i, (val << 8) | m_px);

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	m_i[ireg] = i;
}


inline uint32_t adsp21xx_device::pgm_read_dag2(uint32_t op)
{
	uint32_t ireg = 4 + BIT(op, 2, 2);
	uint32_t mreg = 4 + BIT(op, 0, 2);
	uint32_t base = m_base[ireg];
	uint32_t i = m_i[ireg];
	uint32_t l = m_l[ireg];
	uint32_t res;

	res = program_read(i);
	m_px = res;
	res >>= 8;

	i = (i + m_m[mreg]) & 0x3fff;
	if (i < base) i += l;
	else if (i >= base + l) i -= l;
	m_i[ireg] = i;

	return res;
}



/*===========================================================================
    register reading
===========================================================================*/

#define ALU_GETXREG_UNSIGNED(x) (*(uint16_t *)m_alu_xregs[x])
#define ALU_GETYREG_UNSIGNED(y) (*(uint16_t *)m_alu_yregs[y])

#define MAC_GETXREG_UNSIGNED(x) (*(uint16_t *)m_mac_xregs[x])
#define MAC_GETXREG_SIGNED(x)   (*( int16_t *)m_mac_xregs[x])
#define MAC_GETYREG_UNSIGNED(y) (*(uint16_t *)m_mac_yregs[y])
#define MAC_GETYREG_SIGNED(y)   (*( int16_t *)m_mac_yregs[y])

#define SHIFT_GETXREG_UNSIGNED(x) (*(uint16_t *)m_shift_xregs[x])
#define SHIFT_GETXREG_SIGNED(x)   (*( int16_t *)m_shift_xregs[x])



/*===========================================================================
    ALU operations (result in AR)
===========================================================================*/

void adsp21xx_device::alu_op_ar(uint32_t op)
{
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = BIT(op, 11, 2);
	int32_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// Y                Clear when y = 0
			res = ALU_GETYREG_UNSIGNED(yop);
			CALC_NZ(res);
			break;
		case 0x01:
			// Y + 1            PASS 1 when y = 0
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02:
			// X + Y + C
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03:
			// X + Y            X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04:
			// NOT Y
			res = ALU_GETYREG_UNSIGNED(yop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05:
			// -Y
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06:
			// X - Y + C - 1    X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07:
			// X - Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08:
			// Y - 1            PASS -1 when y = 0
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09:
			// Y - X            -X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a:
			// Y - X + C - 1    -X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b:
			// NOT X
			res = ALU_GETXREG_UNSIGNED(xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c:
			// X AND Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d:
			// X OR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e:
			// X XOR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f:
			// ABS X
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = (xop & 0x8000) ? -xop : xop;
			CLR_FLAGS;
			CLR_S;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// saturate
	if ((m_mstat & MSTAT_SATURATE) && GET_V) res = GET_C ? -32768 : 32767;

	// set the final value
	m_core.ar.u = res;
}



/*===========================================================================
    ALU operations (result in AR, constant yop)
===========================================================================*/

void adsp21xx_device::alu_op_ar_const(uint32_t op)
{
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = constants[BIT(op, 5, 3) | BIT(op, 11, 2) << 3];
	int32_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// Y                Clear when y = 0
			res = yop;
			CALC_NZ(res);
			break;
		case 0x01:
			// Y + 1            PASS 1 when y = 0
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02:
			// X + Y + C
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03:
			// X + Y            X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04:
			// NOT Y
			res = yop ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05:
			// -Y
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06:
			// X - Y + C - 1    X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07:
			// X - Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08:
			// Y - 1            PASS -1 when y = 0
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09:
			// Y - X            -X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a:
			// Y - X + C - 1    -X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b:
			// NOT X
			res = ALU_GETXREG_UNSIGNED(xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c:
			// X AND Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d:
			// X OR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e:
			// X XOR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f:
			// ABS X
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = (xop & 0x8000) ? -xop : xop;
			CLR_FLAGS;
			CLR_S;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// saturate
	if ((m_mstat & MSTAT_SATURATE) && GET_V) res = GET_C ? -32768 : 32767;

	// set the final value
	m_core.ar.u = res;
}



/*===========================================================================
    ALU operations (result in AF)
===========================================================================*/

void adsp21xx_device::alu_op_af(uint32_t op)
{
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = BIT(op, 11, 2);
	int32_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// Y                Clear when y = 0
			res = ALU_GETYREG_UNSIGNED(yop);
			CALC_NZ(res);
			break;
		case 0x01:
			// Y + 1            PASS 1 when y = 0
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02:
			// X + Y + C
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03:
			// X + Y            X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04:
			// NOT Y
			res = ALU_GETYREG_UNSIGNED(yop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05:
			// -Y
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06:
			// X - Y + C - 1    X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07:
			// X - Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08:
			// Y - 1            PASS -1 when y = 0
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09:
			// Y - X            -X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a:
			// Y - X + C - 1    -X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b:
			// NOT X
			res = ALU_GETXREG_UNSIGNED(xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c:
			// X AND Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d:
			// X OR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e:
			// X XOR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f:
			// ABS X
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = (xop & 0x8000) ? -xop : xop;
			CLR_FLAGS;
			CLR_S;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// set the final value
	m_core.af.u = res;
}



/*===========================================================================
    ALU operations (result in AF, constant yop)
===========================================================================*/

void adsp21xx_device::alu_op_af_const(uint32_t op)
{
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = constants[BIT(op, 5, 3) | BIT(op, 11, 2) << 3];
	int32_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// Y                Clear when y = 0
			res = yop;
			CALC_NZ(res);
			break;
		case 0x01:
			// Y + 1            PASS 1 when y = 0
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02:
			// X + Y + C
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03:
			// X + Y            X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04:
			// NOT Y
			res = yop ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05:
			// -Y
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06:
			// X - Y + C - 1    X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07:
			// X - Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08:
			// Y - 1            PASS -1 when y = 0
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09:
			// Y - X            -X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a:
			// Y - X + C - 1    -X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b:
			// NOT X
			res = ALU_GETXREG_UNSIGNED(xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c:
			// X AND Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d:
			// X OR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e:
			// X XOR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f:
			// ABS X
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = (xop & 0x8000) ? -xop : xop;
			CLR_FLAGS;
			CLR_S;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			if (xop & 0x8000) SET_S;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// set the final value
	m_core.af.u = res;
}



/*===========================================================================
    ALU operations (no result)
===========================================================================*/

void adsp21xx_device::alu_op_none(uint32_t op)
{
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = BIT(op, 11, 2);
	int32_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// Y                Clear when y = 0
			res = ALU_GETYREG_UNSIGNED(yop);
			CALC_NZ(res);
			break;
		case 0x01:
			// Y + 1            PASS 1 when y = 0
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop + 1;
			CALC_NZ(res);
			if (yop == 0x7fff) SET_V;
			else if (yop == 0xffff) SET_C;
			break;
		case 0x02:
			// X + Y + C
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			yop += GET_C >> 3;
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x03:
			// X + Y            X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop + yop;
			CALC_NZVC(xop, yop, res);
			break;
		case 0x04:
			// NOT Y
			res = ALU_GETYREG_UNSIGNED(yop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x05:
			// -Y
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = -yop;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			if (yop == 0x0000) SET_C;
			break;
		case 0x06:
			// X - Y + C - 1    X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop - yop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x07:
			// X - Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop - yop;
			CALC_NZVC_SUB(xop, yop, res);
			break;
		case 0x08:
			// Y - 1            PASS -1 when y = 0
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - 1;
			CALC_NZ(res);
			if (yop == 0x8000) SET_V;
			else if (yop == 0x0000) SET_C;
			break;
		case 0x09:
			// Y - X            -X when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - xop;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0a:
			// Y - X + C - 1    -X + C - 1 when y = 0
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = yop - xop + (GET_C >> 3) - 1;
			CALC_NZVC_SUB(yop, xop, res);
			break;
		case 0x0b:
			// NOT X
			res = ALU_GETXREG_UNSIGNED(xop) ^ 0xffff;
			CALC_NZ(res);
			break;
		case 0x0c:
			// X AND Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop & yop;
			CALC_NZ(res);
			break;
		case 0x0d:
			// X OR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop | yop;
			CALC_NZ(res);
			break;
		case 0x0e:
			// X XOR Y
			xop = ALU_GETXREG_UNSIGNED(xop);
			yop = ALU_GETYREG_UNSIGNED(yop);
			res = xop ^ yop;
			CALC_NZ(res);
			break;
		case 0x0f:
			// ABS X
			xop = ALU_GETXREG_UNSIGNED(xop);
			res = (xop & 0x8000) ? -xop : xop;
			CLR_FLAGS;
			CLR_S;
			if (xop == 0) SET_Z;
			if (xop == 0x8000) SET_N, SET_V;
			if (xop & 0x8000) SET_S;
			break;
	}
}



/*===========================================================================
    MAC operations (result in MR)
===========================================================================*/

void adsp21xx_device::mac_op_mr(uint32_t op)
{
	int8_t shift = (m_mstat & MSTAT_INTEGER) ? 0 : 1;
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = BIT(op, 11, 2);
	int32_t temp;
	int64_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// no-op
			return;
		case 0x01:
			// X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x02:
			// MR + X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x03:
			// MR - X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x04:
			// X * Y (SS)       Clear when y = 0
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x05:
			// X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x06:
			// X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x07:
			// X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x08:
			// MR + X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x09:
			// MR + X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0a:
			// MR + X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0b:
			// MR + X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0c:
			// MR - X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0d:
			// MR - X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0e:
			// MR - X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0f:
			// MR - X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// set the final value
	temp = BIT(res, 31, 9);
	CLR_MV;
	if (temp != 0x000 && temp != 0x1ff) SET_MV;
	m_core.mr.mr = res;
}



/*===========================================================================
    MAC operations (result in MR, yop == xop)
===========================================================================*/

void adsp21xx_device::mac_op_mr_xop(uint32_t op)
{
	int8_t shift = (m_mstat & MSTAT_INTEGER) ? 0 : 1;
	int32_t xop = BIT(op, 8, 3);
	int32_t temp;
	int64_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// no-op
			return;
		case 0x01:
			// X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x02:
			// MR + X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x03:
			// MR - X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x04:
			// X * Y (SS)       Clear when y = 0
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x05:
			// X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x06:
			// X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x07:
			// X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x08:
			// MR + X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x09:
			// MR + X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0a:
			// MR + X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0b:
			// MR + X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0c:
			// MR - X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0d:
			// MR - X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0e:
			// MR - X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0f:
			// MR - X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// set the final value
	temp = BIT(res, 31, 9);
	CLR_MV;
	if (temp != 0x000 && temp != 0x1ff) SET_MV;
	m_core.mr.mr = res;
}



/*===========================================================================
    MAC operations (result in MF)
===========================================================================*/

void adsp21xx_device::mac_op_mf(uint32_t op)
{
	int8_t shift = (m_mstat & MSTAT_INTEGER) ? 0 : 1;
	int32_t xop = BIT(op, 8, 3);
	int32_t yop = BIT(op, 11, 2);
	int32_t temp;
	int64_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// no-op
			return;
		case 0x01:
			// X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x02:
			// MR + X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x03:
			// MR - X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x04:
			// X * Y (SS)       Clear when y = 0
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x05:
			// X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x06:
			// X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x07:
			// X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = (int64_t)temp;
			break;
		case 0x08:
			// MR + X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x09:
			// MR + X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0a:
			// MR + X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0b:
			// MR + X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0c:
			// MR - X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0d:
			// MR - X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0e:
			// MR - X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_SIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0f:
			// MR - X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			yop = MAC_GETYREG_UNSIGNED(yop);
			temp = (xop * yop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// set the final value
	m_core.mf.u = (uint32_t)res >> 16;
}



/*===========================================================================
    MAC operations (result in MF, yop == xop)
===========================================================================*/

void adsp21xx_device::mac_op_mf_xop(uint32_t op)
{
	int8_t shift = (m_mstat & MSTAT_INTEGER) ? 0 : 1;
	int32_t xop = BIT(op, 8, 3);
	int32_t temp;
	int64_t res;

	switch (BIT(op, 13, 4))  //JB
	{
		case 0x00:
			// no-op
			return;
		case 0x01:
			// X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x02:
			// MR + X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x03:
			// MR - X * Y (RND)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
#if 0
			if ((res & 0xffff) == 0x8000) res &= ~((uint64_t)0x10000);
			else res += (res & 0x8000) << 1;
#else
			temp &= 0xffff;
			res += 0x8000;
			if ( temp == 0x8000 )
				res &= ~((uint64_t)0x10000);
#endif
			break;
		case 0x04:
			// X * Y (SS)       Clear when y = 0
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x05:
			// X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x06:
			// X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x07:
			// X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = (int64_t)temp;
			break;
		case 0x08:
			// MR + X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x09:
			// MR + X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0a:
			// MR + X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0b:
			// MR + X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr + (int64_t)temp;
			break;
		case 0x0c:
			// MR - X * Y (SS)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0d:
			// MR - X * Y (SU)
			xop = MAC_GETXREG_SIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0e:
			// MR - X * Y (US)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		case 0x0f:
			// MR - X * Y (UU)
			xop = MAC_GETXREG_UNSIGNED(xop);
			temp = (xop * xop) << shift;
			res = m_core.mr.mr - (int64_t)temp;
			break;
		default:
			res = 0;    // just to keep the compiler happy
			break;
	}

	// set the final value
	m_core.mf.u = (uint32_t)res >> 16;
}



/*===========================================================================
    SHIFT operations (result in SR/SE/SB)
===========================================================================*/

void adsp21xx_device::shift_op(uint32_t op)
{
	int8_t sc = m_core.se.s;
	int32_t xop = BIT(op, 8, 3);
	uint32_t res;

	switch (BIT(op, 11, 4))  //JB
	{
		case 0x00:
			// LSHIFT (HI)
			xop = SHIFT_GETXREG_UNSIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((uint32_t)xop >> -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x01:
			// LSHIFT (HI, OR)
			xop = SHIFT_GETXREG_UNSIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((uint32_t)xop >> -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x02:
			// LSHIFT (LO)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x03:
			// LSHIFT (LO, OR)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x04:
			// ASHIFT (HI)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr = res;
			break;
		case 0x05:
			// ASHIFT (HI, OR)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr |= res;
			break;
		case 0x06:
			// ASHIFT (LO)
			xop = SHIFT_GETXREG_SIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr = res;
			break;
		case 0x07:
			// ASHIFT (LO, OR)
			xop = SHIFT_GETXREG_SIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr |= res;
			break;
		case 0x08:
			// NORM (HI)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0)
			{
				xop = ((uint32_t)xop >> 1) | ((m_astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x09:
			// NORM (HI, OR)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0)
			{
				xop = ((uint32_t)xop >> 1) | ((m_astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x0a:
			// NORM (LO)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x0b:
			// NORM (LO, OR)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x0c:
			// EXP (HI)
			xop = SHIFT_GETXREG_SIGNED(xop);
			if (xop < 0)
			{
				SET_SS;
				res = count_leading_ones_32(xop) - 16 - 1;
			}
			else
			{
				CLR_SS;
				res = count_leading_zeros_32(xop) - 16 - 1;
			}
			m_core.se.s = -res;
			break;
		case 0x0d:
			// EXP (HIX)
			xop = SHIFT_GETXREG_SIGNED(xop);
			if (GET_V)
			{
				m_core.se.s = 1;
				if (xop < 0) CLR_SS;
				else SET_SS;
			}
			else
			{
				if (xop < 0)
				{
					SET_SS;
					res = count_leading_ones_32(xop) - 16 - 1;
				}
				else
				{
					CLR_SS;
					res = count_leading_zeros_32(xop) - 16 - 1;
				}
				m_core.se.s = -res;
			}
			break;
		case 0x0e:
			// EXP (LO)
			if (m_core.se.s == -15)
			{
				xop = SHIFT_GETXREG_SIGNED(xop);
				res = count_leading_zeros_32((GET_SS ? ~xop : xop) & 0xffff) - 1;
				m_core.se.s = -res;
			}
			break;
		case 0x0f:
			// EXPADJ
			xop = SHIFT_GETXREG_SIGNED(xop);
			res = count_leading_zeros_32((xop < 0) ? ~xop : xop) - 16 - 1;
			if (res < -m_core.sb.s)
				m_core.sb.s = -res;
			break;
	}
}



/*===========================================================================
    Immediate SHIFT operations (result in SR/SE/SB)
===========================================================================*/

void adsp21xx_device::shift_op_imm(uint32_t op)
{
	int8_t sc = (int8_t)op;
	int32_t xop = BIT(op, 8, 3);
	uint32_t res;

	switch (BIT(op, 11, 4))  //JB
	{
		case 0x00:
			// LSHIFT (HI)
			xop = SHIFT_GETXREG_UNSIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((uint32_t)xop >> -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x01:
			// LSHIFT (HI, OR)
			xop = SHIFT_GETXREG_UNSIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? ((uint32_t)xop >> -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x02:
			// LSHIFT (LO)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x03:
			// LSHIFT (LO, OR)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x04:
			// ASHIFT (HI)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr = res;
			break;
		case 0x05:
			// ASHIFT (HI, OR)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr |= res;
			break;
		case 0x06:
			// ASHIFT (LO)
			xop = SHIFT_GETXREG_SIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr = res;
			break;
		case 0x07:
			// ASHIFT (LO, OR)
			xop = SHIFT_GETXREG_SIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop << sc) : 0;
			else res = (sc > -32) ? (xop >> -sc) : (xop >> 31);
			m_core.sr.sr |= res;
			break;
		case 0x08:
			// NORM (HI)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0)
			{
				xop = ((uint32_t)xop >> 1) | ((m_astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x09:
			// NORM (HI, OR)
			xop = SHIFT_GETXREG_SIGNED(xop) << 16;
			if (sc > 0)
			{
				xop = ((uint32_t)xop >> 1) | ((m_astat & CFLAG) << 28);
				res = xop >> (sc - 1);
			}
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr |= res;
			break;
		case 0x0a:
			// NORM (LO)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr = res;
			break;
		case 0x0b:
			// NORM (LO, OR)
			xop = SHIFT_GETXREG_UNSIGNED(xop);
			if (sc > 0) res = (sc < 32) ? (xop >> sc) : 0;
			else res = (sc > -32) ? (xop << -sc) : 0;
			m_core.sr.sr |= res;
			break;
	}
}
