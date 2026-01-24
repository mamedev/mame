// license:BSD-3-Clause
// copyright-holders:Ville Linde
#define PM_REG_I(x)         (m_core->dag2.i[x])
#define PM_REG_M(x)         (m_core->dag2.m[x])
#define PM_REG_B(x)         (m_core->dag2.b[x])
#define PM_REG_L(x)         (m_core->dag2.l[x])
#define DM_REG_I(x)         (m_core->dag1.i[x])
#define DM_REG_M(x)         (m_core->dag1.m[x])
#define DM_REG_B(x)         (m_core->dag1.b[x])
#define DM_REG_L(x)         (m_core->dag1.l[x])

// ASTAT flags
#define AZ      0x1         /* ALU result zero */
#define AV      0x2         /* ALU overflow */
#define AN      0x4         /* ALU result negative */
#define AC      0x8         /* ALU fixed-point carry */
#define AS      0x10        /* ALU X input sign */
#define AI      0x20        /* ALU floating-point invalid operation */
#define MN      0x40        /* Multiplier result negative */
#define MV      0x80        /* Multiplier overflow */
#define MU      0x100       /* Multiplier underflow */
#define MI      0x200       /* Multiplier floating-point invalid operation */
#define AF      0x400
#define SV      0x800       /* Shifter overflow */
#define SZ      0x1000      /* Shifter result zero */
#define SS      0x2000      /* Shifter input sign */
#define BTF     0x40000     /* Bit Test Flag */
#define FLG0    0x80000     /* FLAG0 */
#define FLG1    0x100000    /* FLAG1 */
#define FLG2    0x200000    /* FLAG2 */
#define FLG3    0x400000    /* FLAG3 */

#define REG_PC              0x63
#define REG_PCSTK           0x64
#define REG_PCSTKP          0x65
#define REG_LADDR           0x66
#define REG_CURLCNTR        0x67
#define REG_LCNTR           0x68
#define REG_USTAT1          0x70
#define REG_USTAT2          0x71
#define REG_IRPTL           0x79
#define REG_MODE2           0x7a
#define REG_MODE1           0x7b
#define REG_ASTAT           0x7c
#define REG_IMASK           0x7d
#define REG_STKY            0x7e
#define REG_IMASKP          0x7f



#define REG(x)      (m_core->r[x].r)
#define FREG(x)     (m_core->r[x].f)

#define UPDATE_CIRCULAR_BUFFER_PM(x)                        \
	{                                                       \
		if (PM_REG_L(x) != 0)                               \
		{                                                   \
			if (PM_REG_I(x) > PM_REG_B(x)+PM_REG_L(x))      \
			{                                               \
				PM_REG_I(x) -= PM_REG_L(x);                 \
			}                                               \
			else if (PM_REG_I(x) < PM_REG_B(x))             \
			{                                               \
				PM_REG_I(x) += PM_REG_L(x);                 \
			}                                               \
		}                                                   \
	}

#define UPDATE_CIRCULAR_BUFFER_DM(x)                        \
	{                                                       \
		if (DM_REG_L(x) != 0)                               \
		{                                                   \
			if (DM_REG_I(x) > DM_REG_B(x)+DM_REG_L(x))      \
			{                                               \
				DM_REG_I(x) -= DM_REG_L(x);                 \
			}                                               \
			else if (DM_REG_I(x) < DM_REG_B(x))             \
			{                                               \
				DM_REG_I(x) += DM_REG_L(x);                 \
			}                                               \
		}                                                   \
	}


/*****************************************************************************/

void adsp21062_device::add_systemreg_write_latency_effect(int sysreg, uint32_t data, uint32_t prev_data)
{
	if (m_core->systemreg_latency_cycles > 0)
	{
		//fatalerror("SHARC: add_systemreg_write_latency_effect: already scheduled! (reg: %02X, data: %08X, PC: %08X)\n", systemreg_latency_reg, systemreg_latency_data, m_core->pc);
		systemreg_write_latency_effect();
	}

	m_core->systemreg_latency_cycles = 2;
	m_core->systemreg_latency_reg = sysreg;
	m_core->systemreg_latency_data = data;
	m_core->systemreg_previous_data = prev_data;
}

void adsp21062_device::swap_register(uint32_t *a, uint32_t *b)
{
	uint32_t temp = *a;
	*a = *b;
	*b = temp;
}

void adsp21062_device::systemreg_write_latency_effect()
{
	int i;
	uint32_t data = m_core->systemreg_latency_data;
	uint32_t old_data = m_core->systemreg_previous_data;

	switch(m_core->systemreg_latency_reg)
	{
		case 0xb:   /* MODE1 */
		{
			uint32_t oldreg = old_data;
			m_core->mode1 = data;

			if ((data & 0x1) != (oldreg & 0x1))
			{
				fatalerror("SHARC: systemreg_latency_op: enable I8 bit-reversing\n");
			}
			if ((data & 0x2) != (oldreg & 0x2))
			{
				fatalerror("SHARC: systemreg_latency_op: enable I0 bit-reversing\n");
			}
			if ((data & 0x4) != (oldreg & 0x4))
			{
				fatalerror("SHARC: systemreg_latency_op: enable MR alternate\n");
			}

			if ((data & 0x8) != (oldreg & 0x8))         /* Switch DAG1 7-4 */
			{
				swap_register(&m_core->dag1.i[4], &m_core->dag1_alt.i[4]);
				swap_register(&m_core->dag1.i[5], &m_core->dag1_alt.i[5]);
				swap_register(&m_core->dag1.i[6], &m_core->dag1_alt.i[6]);
				swap_register(&m_core->dag1.i[7], &m_core->dag1_alt.i[7]);
				swap_register(&m_core->dag1.m[4], &m_core->dag1_alt.m[4]);
				swap_register(&m_core->dag1.m[5], &m_core->dag1_alt.m[5]);
				swap_register(&m_core->dag1.m[6], &m_core->dag1_alt.m[6]);
				swap_register(&m_core->dag1.m[7], &m_core->dag1_alt.m[7]);
				swap_register(&m_core->dag1.l[4], &m_core->dag1_alt.l[4]);
				swap_register(&m_core->dag1.l[5], &m_core->dag1_alt.l[5]);
				swap_register(&m_core->dag1.l[6], &m_core->dag1_alt.l[6]);
				swap_register(&m_core->dag1.l[7], &m_core->dag1_alt.l[7]);
				swap_register(&m_core->dag1.b[4], &m_core->dag1_alt.b[4]);
				swap_register(&m_core->dag1.b[5], &m_core->dag1_alt.b[5]);
				swap_register(&m_core->dag1.b[6], &m_core->dag1_alt.b[6]);
				swap_register(&m_core->dag1.b[7], &m_core->dag1_alt.b[7]);
			}
			if ((data & 0x10) != (oldreg & 0x10))       /* Switch DAG1 3-0 */
			{
				swap_register(&m_core->dag1.i[0], &m_core->dag1_alt.i[0]);
				swap_register(&m_core->dag1.i[1], &m_core->dag1_alt.i[1]);
				swap_register(&m_core->dag1.i[2], &m_core->dag1_alt.i[2]);
				swap_register(&m_core->dag1.i[3], &m_core->dag1_alt.i[3]);
				swap_register(&m_core->dag1.m[0], &m_core->dag1_alt.m[0]);
				swap_register(&m_core->dag1.m[1], &m_core->dag1_alt.m[1]);
				swap_register(&m_core->dag1.m[2], &m_core->dag1_alt.m[2]);
				swap_register(&m_core->dag1.m[3], &m_core->dag1_alt.m[3]);
				swap_register(&m_core->dag1.l[0], &m_core->dag1_alt.l[0]);
				swap_register(&m_core->dag1.l[1], &m_core->dag1_alt.l[1]);
				swap_register(&m_core->dag1.l[2], &m_core->dag1_alt.l[2]);
				swap_register(&m_core->dag1.l[3], &m_core->dag1_alt.l[3]);
				swap_register(&m_core->dag1.b[0], &m_core->dag1_alt.b[0]);
				swap_register(&m_core->dag1.b[1], &m_core->dag1_alt.b[1]);
				swap_register(&m_core->dag1.b[2], &m_core->dag1_alt.b[2]);
				swap_register(&m_core->dag1.b[3], &m_core->dag1_alt.b[3]);
			}
			if ((data & 0x20) != (oldreg & 0x20))       /* Switch DAG2 15-12 */
			{
				swap_register(&m_core->dag2.i[4], &m_core->dag2_alt.i[4]);
				swap_register(&m_core->dag2.i[5], &m_core->dag2_alt.i[5]);
				swap_register(&m_core->dag2.i[6], &m_core->dag2_alt.i[6]);
				swap_register(&m_core->dag2.i[7], &m_core->dag2_alt.i[7]);
				swap_register(&m_core->dag2.m[4], &m_core->dag2_alt.m[4]);
				swap_register(&m_core->dag2.m[5], &m_core->dag2_alt.m[5]);
				swap_register(&m_core->dag2.m[6], &m_core->dag2_alt.m[6]);
				swap_register(&m_core->dag2.m[7], &m_core->dag2_alt.m[7]);
				swap_register(&m_core->dag2.l[4], &m_core->dag2_alt.l[4]);
				swap_register(&m_core->dag2.l[5], &m_core->dag2_alt.l[5]);
				swap_register(&m_core->dag2.l[6], &m_core->dag2_alt.l[6]);
				swap_register(&m_core->dag2.l[7], &m_core->dag2_alt.l[7]);
				swap_register(&m_core->dag2.b[4], &m_core->dag2_alt.b[4]);
				swap_register(&m_core->dag2.b[5], &m_core->dag2_alt.b[5]);
				swap_register(&m_core->dag2.b[6], &m_core->dag2_alt.b[6]);
				swap_register(&m_core->dag2.b[7], &m_core->dag2_alt.b[7]);
			}
			if ((data & 0x40) != (oldreg & 0x40))       /* Switch DAG2 11-8 */
			{
				swap_register(&m_core->dag2.i[0], &m_core->dag2_alt.i[0]);
				swap_register(&m_core->dag2.i[1], &m_core->dag2_alt.i[1]);
				swap_register(&m_core->dag2.i[2], &m_core->dag2_alt.i[2]);
				swap_register(&m_core->dag2.i[3], &m_core->dag2_alt.i[3]);
				swap_register(&m_core->dag2.m[0], &m_core->dag2_alt.m[0]);
				swap_register(&m_core->dag2.m[1], &m_core->dag2_alt.m[1]);
				swap_register(&m_core->dag2.m[2], &m_core->dag2_alt.m[2]);
				swap_register(&m_core->dag2.m[3], &m_core->dag2_alt.m[3]);
				swap_register(&m_core->dag2.l[0], &m_core->dag2_alt.l[0]);
				swap_register(&m_core->dag2.l[1], &m_core->dag2_alt.l[1]);
				swap_register(&m_core->dag2.l[2], &m_core->dag2_alt.l[2]);
				swap_register(&m_core->dag2.l[3], &m_core->dag2_alt.l[3]);
				swap_register(&m_core->dag2.b[0], &m_core->dag2_alt.b[0]);
				swap_register(&m_core->dag2.b[1], &m_core->dag2_alt.b[1]);
				swap_register(&m_core->dag2.b[2], &m_core->dag2_alt.b[2]);
				swap_register(&m_core->dag2.b[3], &m_core->dag2_alt.b[3]);
			}
			if ((data & 0x80) != (oldreg & 0x80))
			{
				for (i=8; i<16; i++)
					swap_register((uint32_t*)&m_core->r[i].r, (uint32_t*)&m_core->reg_alt[i].r);
			}
			if ((data & 0x400) != (oldreg & 0x400))
			{
				for (i=0; i<8; i++)
					swap_register((uint32_t*)&m_core->r[i].r, (uint32_t*)&m_core->reg_alt[i].r);
			}
			break;
		}
		default:    fatalerror("SHARC: systemreg_latency_op: unknown register %02X at %08X\n", m_core->systemreg_latency_reg, m_core->pc);
	}

	m_core->systemreg_latency_reg = -1;
}

uint32_t adsp21062_device::GET_UREG(int ureg)
{
	int reg = ureg & 0xf;
	switch((ureg >> 4) & 0xf)
	{
		case 0x0:       /* R0 - R15 */
		{
			return m_core->r[reg].r;
		}

		case 0x1:
		{
			if (reg & 0x8)      /* I8 - I15 */
			{
				return m_core->dag2.i[reg & 0x7];
			}
			else                /* I0 - I7 */
			{
				return m_core->dag1.i[reg & 0x7];
			}
		}

		case 0x2:
		{
			if (reg & 0x8)      /* M8 - M15 */
			{
				int32_t r = m_core->dag2.m[reg & 0x7];
				if (r & 0x800000)   r |= 0xff000000;

				return r;
			}
			else                /* M0 - M7 */
			{
				return m_core->dag1.m[reg & 0x7];
			}
		}

		case 0x3:
		{
			if (reg & 0x8)      /* L8 - L15 */
			{
				return m_core->dag2.l[reg & 0x7];
			}
			else                /* L0 - L7 */
			{
				return m_core->dag1.l[reg & 0x7];
			}
		}

		case 0x4:
		{
			if (reg & 0x8)      /* B8 - B15 */
			{
				return m_core->dag2.b[reg & 0x7];
			}
			else                /* B0 - B7 */
			{
				return m_core->dag1.b[reg & 0x7];
			}
		}

		case 0x6:
		{
			switch(reg)
			{
				case 0x4:   return m_core->pcstack[m_core->pcstkp];     /* PCSTK */
				default:    fatalerror("SHARC: GET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
			}
			break;
		}

		case 0x7:
		{
			switch(reg)
			{
				case 0x0:   return m_core->ustat1;        /* USTAT1 */
				case 0x1:   return m_core->ustat2;        /* USTAT2 */
				case 0x9:   return m_core->irptl;         /* IRPTL */
				case 0xa:   return m_core->mode2;         /* MODE2 */
				case 0xb:   return m_core->mode1;         /* MODE1 */
				case 0xc:                               /* ASTAT */
				{
					uint32_t r = m_core->astat;
					r &= ~0x00780000;
					r |= (m_core->flag[0] << 19);
					r |= (m_core->flag[1] << 20);
					r |= (m_core->flag[2] << 21);
					r |= (m_core->flag[3] << 22);
					return r;
				}
				case 0xd:   return m_core->imask;         /* IMASK */
				case 0xe:   return m_core->stky;          /* STKY */
				default:    fatalerror("SHARC: GET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
			}
			break;
		}

		case 0xd:
		{
			switch(reg)
			{
				/* PX needs to be handled separately if the whole 48 bits are needed */
				case 0xb:   return (uint32_t)(m_core->px);          /* PX */
				case 0xc:   return (uint16_t)(m_core->px);          /* PX1 */
				case 0xd:   return (uint32_t)(m_core->px >> 16);    /* PX2 */
				default:    fatalerror("SHARC: GET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
			}
			break;
		}

		default:            fatalerror("SHARC: GET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
	}
}

void adsp21062_device::SET_UREG(int ureg, uint32_t data)
{
	int reg = ureg & 0xf;
	switch((ureg >> 4) & 0xf)
	{
		case 0x0:       /* R0 - R15 */
			m_core->r[reg].r = data;
			break;

		case 0x1:
			if (reg & 0x8)      /* I8 - I15 */
			{
				m_core->dag2.i[reg & 0x7] = data;
			}
			else                /* I0 - I7 */
			{
				m_core->dag1.i[reg & 0x7] = data;
			}
			break;

		case 0x2:
			if (reg & 0x8)      /* M8 - M15 */
			{
				m_core->dag2.m[reg & 0x7] = data;
			}
			else                /* M0 - M7 */
			{
				m_core->dag1.m[reg & 0x7] = data;
			}
			break;

		case 0x3:
			if (reg & 0x8)      /* L8 - L15 */
			{
				m_core->dag2.l[reg & 0x7] = data;
			}
			else                /* L0 - L7 */
			{
				m_core->dag1.l[reg & 0x7] = data;
			}
			break;

		case 0x4:
			// Note: loading B also loads the same value in I
			if (reg & 0x8)      /* B8 - B15 */
			{
				m_core->dag2.b[reg & 0x7] = data;
				m_core->dag2.i[reg & 0x7] = data;
			}
			else                /* B0 - B7 */
			{
				m_core->dag1.b[reg & 0x7] = data;
				m_core->dag1.i[reg & 0x7] = data;
			}
			break;

		case 0x6:
			switch (reg)
			{
				case 0x5:   m_core->pcstkp = data; break;     /* PCSTKP */
				case 0x7:   m_core->curlcntr = data; break;   /* CURLCNTR (Zero Gunner 2B) */
				case 0x8:   m_core->lcntr = data; break;      /* LCNTR */
				default:    fatalerror("SHARC: SET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
			}
			break;

		case 0x7:       /* system regs */
			switch(reg)
			{
				case 0x0:   m_core->ustat1 = data; break;     /* USTAT1 */
				case 0x1:   m_core->ustat2 = data; break;     /* USTAT2 */

				case 0x9:   m_core->irptl = data; break;      /* IRPTL */
				case 0xa:   m_core->mode2 = data; break;      /* MODE2 */

				case 0xb:                                   /* MODE1 */
				{
					add_systemreg_write_latency_effect(reg, data, m_core->mode1);
					m_core->mode1 = data;
					break;
				}

				case 0xc:   m_core->astat = data; break;      /* ASTAT */

				case 0xd:                                   /* IMASK */
				{
					check_interrupts();
					m_core->imask = data;
					break;
				}

				case 0xe:   m_core->stky = data; break;       /* STKY */
				default:    fatalerror("SHARC: SET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
			}
			break;

		case 0xd:
			switch(reg)
			{
				case 0xc:   m_core->px &= 0xffffffffffff0000U; m_core->px |= (data & 0xffff); break;        /* PX1 */
				case 0xd:   m_core->px &= 0x000000000000ffffU; m_core->px |= (uint64_t)data << 16; break;     /* PX2 */
				default:    fatalerror("SHARC: SET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
			}
			break;

		default:            fatalerror("SHARC: SET_UREG: unknown register %08X at %08X\n", ureg, m_core->pc);
	}
}

/*****************************************************************************/
#define SET_FLAG_SV_LSHIFT(x, shift)    if((x) & ((uint32_t)0xffffffff << shift)) m_core->astat |= SV
#define SET_FLAG_SV_RSHIFT(x, shift)    if((x) & ((uint32_t)0xffffffff >> shift)) m_core->astat |= SV

#define SET_FLAG_SZ(x)                  if((x) == 0) m_core->astat |= SZ

void adsp21062_device::SHIFT_OPERATION_IMM(int shiftop, int data, int rn, int rx)
{
	int8_t shift = data & 0xff;
	int bit = data & 0x3f;
	int len = (data >> 6) & 0x3f;

	m_core->astat &= ~(SZ|SV|SS);

	switch(shiftop)
	{
		case 0x00:      /* LSHIFT Rx BY <data8>*/
		{
			if(shift < 0) {
				REG(rn) = (shift > -32 ) ? ((uint32_t)REG(rx) >> -shift) : 0;
			} else {
				REG(rn) = (shift < 32) ? ((uint32_t)REG(rx) << shift) : 0;
				if (shift > 0)
				{
					m_core->astat |= SV;
				}
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x01:      /* ASHIFT Rx BY <data8> */
		{
			if (shift < 0)
			{
				REG(rn) = (int32_t)REG(rx) >> ((shift > -32) ? -shift : 31);
			}
			else
			{
				REG(rn) = (shift < 32) ? ((int32_t)REG(rx) << shift) : 0;
				if (shift > 0)
				{
					m_core->astat |= SV;
				}
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x02:      /* ROT Rx BY <data8> */
		{
			REG(rn) = rotl_32(REG(rx), shift);
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x08:      /* Rn = Rn OR LSHIFT Rx BY <data8> */
		{
			uint32_t r = 0;
			if(shift < 0) {
				r = (shift > -32 ) ? ((uint32_t)REG(rx) >> -shift) : 0;
			} else {
				r = (shift < 32) ? ((uint32_t)REG(rx) << shift) : 0;
				if (shift > 0)
				{
					m_core->astat |= SV;
				}
			}

			SET_FLAG_SZ(r);

			REG(rn) = REG(rn) | r;
			break;
		}

		case 0x10:      /* FEXT Rx BY <bit6>:<len6> */
		{
			if (len == 0 || bit >= 32)
				REG(rn) = 0;
			else
				REG(rn) = BIT(REG(rx), bit, std::min(len, 32));

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				m_core->astat |= SV;
			}
			break;
		}

		case 0x11:      /* Rn = Rn FDEP Rx BY <bit6>:<len6> */
		{
			if (len == 0 || bit >= 32)
				REG(rn) = 0;
			else
				REG(rn) = (REG(rx) & util::make_bitmask<uint32_t>(std::min(len, 32))) << bit;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				m_core->astat |= SV;
			}
			break;
		}

		case 0x12:      /* FEXT Rx BY <bit6>:<len6> (Sign Extended) */
		{
			if (len == 0 || bit >= 32)
				REG(rn) = 0;
			else if (bit+len > 32)
				REG(rn) = (uint32_t)REG(rx) >> bit;
			else
				REG(rn) = util::sext(REG(rx) >> bit, std::min(len, 32));

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				m_core->astat |= SV;
			}
			break;
		}

		case 0x13:      /* FDEP Rx BY Ry <bit6>:<len6> (Sign Extended) */
		{
			if (len == 0 || bit >= 32)
				REG(rn) = 0;
			else
				REG(rn) = util::sext(REG(rx), std::min(len, 32)) << bit;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				m_core->astat |= SV;
			}
			break;
		}

		case 0x19:      /* Rn = Rn OR FDEP Rx BY <bit6>:<len6> */
		{
			if (len != 0 && bit < 32)
				REG(rn) |= (REG(rx) & util::make_bitmask<uint32_t>(std::min(len, 32))) << bit;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				m_core->astat |= SV;
			}
			break;
		}

		case 0x30:      /* BSET Rx BY <data8> */
		{
			REG(rn) = REG(rx);
			if (data >= 0 && data < 32)
			{
				REG(rn) |= (1 << data);
			}
			else
			{
				m_core->astat |= SV;
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x31:      /* BCLR Rx BY <data8> */
		{
			REG(rn) = REG(rx);
			if (data >= 0 && data < 32)
			{
				REG(rn) &= ~(1 << data);
			}
			else
			{
				m_core->astat |= SV;
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x32:      /* BTGL Rx BY <data8> */
		{
			REG(rn) = REG(rx);
			if (data >= 0 && data < 32)
			{
				REG(rn) ^= (1 << data);
			}
			else
			{
				m_core->astat |= SV;
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x33:      /* BTST Rx BY <data8> */
		{
			if (data < 32)
			{
				uint32_t r = REG(rx) & (1 << data);

				SET_FLAG_SZ(r);
			}
			else
			{
				m_core->astat |= SZ | SV;
			}
			break;
		}

		default:    fatalerror("SHARC: unimplemented shift operation %02X at %08X\n", shiftop, m_core->pc);
	}
}

#include "compute.hxx"

void adsp21062_device::COMPUTE(uint32_t opcode)
{
	int multiop;
	int op = (opcode >> 12) & 0xff;
	int cu = (opcode >> 20) & 0x3;
	int rn = (opcode >> 8) & 0xf;
	int rx = (opcode >> 4) & 0xf;
	int ry = (opcode >> 0) & 0xf;
	//int rs = (opcode >> 12) & 0xf;
	//int ra = rn;
	//int rm = rs;

	if(opcode & 0x400000)       /* Multi-function opcode */
	{
		int fm = (opcode >> 12) & 0xf;
		int fa = (opcode >> 8) & 0xf;
		int fxm = (opcode >> 6) & 0x3;          // registers 0 - 3
		int fym = ((opcode >> 4) & 0x3) + 4;    // registers 4 - 7
		int fxa = ((opcode >> 2) & 0x3) + 8;    // registers 8 - 11
		int fya = (opcode & 0x3) + 12;          // registers 12 - 15

		multiop = (opcode >> 16) & 0x3f;
		switch(multiop)
		{
			case 0x00:      compute_multi_mr_to_reg(op & 0xf, rn); break;
			case 0x01:      compute_multi_reg_to_mr(op & 0xf, rn); break;

			case 0x04:      /* Rm = Rxm * Rym (SSFR),   Ra = Rxa + Rya */
			{
				compute_mul_ssfr_add(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x05:      /* Rm = Rxm * Rym (SSFR),   Ra = Rxa - Rya */
			{
				compute_mul_ssfr_sub(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x18:      /* Fm = Fxm * Fym,   Fa = Fxa + Fya */
			{
				compute_fmul_fadd(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x19:      /* Fm = Fxm * Fym,   Fa = Fxa - Fya */
			{
				compute_fmul_fsub(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1a:      /* Fm = Fxm * Fym,   Fa = FLOAT Fxa BY Fya */
			{
				compute_fmul_float_scaled(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1b:      /* Fm = Fxm * Fym,   Fa = FIX Fxa BY Fya */
			{
				compute_fmul_fix_scaled(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			// TODO: verify this (last bronx)
			case 0x1c:
			{
				compute_fmul_avg(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			// TODO: verify this (Gunblade NY Score Attack Remix mode)
			case 0x1d:
			{
				compute_fmul_abs(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1e:      /* Fm = Fxm * Fym,   Fa = MAX(Fxa, Fya) */
			{
				compute_fmul_fmax(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1f:      /* Fm = Fxm * Fym,   Fa = MIN(Fxa, Fya) */
			{
				compute_fmul_fmin(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			{
				/* Parallel Multiplier & Dual Add/Subtract */
				/* Floating-point */
				int fs = (opcode >> 16) & 0xf;
				compute_fmul_dual_fadd_fsub(fm, fxm, fym, fa, fs, fxa, fya);
				break;
			}

			default:
				fatalerror("SHARC: compute: multi-function opcode %02X not implemented ! (%08X, %08X)\n", multiop, m_core->pc, opcode);
				break;
		}
	}
	else                        /* Single-function opcode */
	{
		switch(cu)
		{
			/* ALU operations */
			case 0:
			{
				switch(op)
				{
					case 0x01:      compute_add(rn, rx, ry); break;
					case 0x02:      compute_sub(rn, rx, ry); break;
					case 0x05:      compute_add_ci(rn, rx, ry); break;
					case 0x06:      compute_sub_ci(rn, rx, ry); break;
					case 0x0a:      compute_comp(rx, ry); break;
					case 0x21:      compute_pass(rn, rx); break;
					case 0x22:      compute_neg(rn, rx); break;
					case 0x29:      compute_inc(rn, rx); break;
					case 0x2a:      compute_dec(rn, rx); break;
					case 0x40:      compute_and(rn, rx, ry); break;
					case 0x41:      compute_or(rn, rx, ry); break;
					case 0x42:      compute_xor(rn, rx, ry); break;
					case 0x43:      compute_not(rn, rx); break;
					case 0x61:      compute_min(rn, rx, ry); break;
					case 0x62:      compute_max(rn, rx, ry); break;
					case 0x81:      compute_fadd(rn, rx, ry); break;
					case 0x82:      compute_fsub(rn, rx, ry); break;
					case 0x89:      compute_favg(rn, rx, ry); break;
					case 0x8a:      compute_fcomp(rx, ry); break;
					case 0x91:      compute_fabs_plus(rn, rx, ry); break;
					case 0xa1:      compute_fpass(rn, rx); break;
					case 0xa2:      compute_fneg(rn, rx); break;
					case 0xb0:      compute_fabs(rn, rx); break;
					case 0xbd:      compute_scalb(rn, rx, ry); break;
					case 0xc1:      compute_logb(rn, rx); break;
					case 0xc4:      compute_recips(rn, rx); break;
					case 0xc5:      compute_rsqrts(rn, rx); break;
					case 0xc9:      compute_fix(rn, rx); break;
					case 0xca:      compute_float(rn, rx); break;
					case 0xd9:      compute_fix_scaled(rn, rx, ry); break;
					case 0xda:      compute_float_scaled(rn, rx, ry); break;
					case 0xe0:      compute_fcopysign(rn, rx, ry); break;
					case 0xe1:      compute_fmin(rn, rx, ry); break;
					case 0xe2:      compute_fmax(rn, rx, ry); break;
					case 0xe3:      compute_fclip(rn, rx, ry); break;

					case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
					case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					{
						/* Fixed-point Dual Add/Subtract */
						int rs = (opcode >> 12) & 0xf;
						int ra = (opcode >> 8) & 0xf;
						compute_dual_add_sub(ra, rs, rx, ry);
						break;
					}

					case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
					case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
					{
						/* Floating-point Dual Add/Subtract */
						int rs = (opcode >> 12) & 0xf;
						int ra = (opcode >> 8) & 0xf;
						compute_dual_fadd_fsub(ra, rs, rx, ry);
						break;
					}

					default:        fatalerror("SHARC: compute: unimplemented ALU operation %02X (%08X, %08X)\n", op, m_core->pc, opcode);
				}
				break;
			}


			/* Multiplier operations */
			case 1:
			{
				switch(op)
				{
					case 0x14:      m_core->mrf = 0; break;
					case 0x16:      m_core->mrb = 0; break;

					case 0x30:      compute_fmul(rn, rx, ry); break;
					case 0x40:      compute_mul_uuin(rn, rx, ry); break;
					case 0x70:      compute_mul_ssin(rn, rx, ry); break;

					case 0xb0:      REG(rn) = compute_mrf_plus_mul_ssin(rx, ry); break;
					case 0xb2:      REG(rn) = compute_mrb_plus_mul_ssin(rx, ry); break;

					default:
						fatalerror("SHARC: compute: multiplier operation %02X not implemented ! (%08X, %08X)\n", op, m_core->pc, opcode);
						break;
				}
				break;
			}


			/* Shifter operations */
			case 2:
			{
				m_core->astat &= ~(SZ|SV|SS);

				op >>= 2;
				switch(op)
				{
					case 0x00:      /* LSHIFT Rx BY Ry*/
					{
						int shift = REG(ry);
						if(shift < 0)
						{
							REG(rn) = (shift > -32 ) ? ((uint32_t)REG(rx) >> -shift) : 0;
						}
						else
						{
							REG(rn) = (shift < 32) ? ((uint32_t)REG(rx) << shift) : 0;
							if (shift > 0)
							{
								m_core->astat |= SV;
							}
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x02:      /* ROT Rx BY Ry */
					{
						int shift = REG(ry);
						if (shift < 0)
						{
							REG(rn) = rotr_32(REG(rx), -shift);
						}
						else
						{
							REG(rn) = rotl_32(REG(rx), shift);
							if (shift > 0)
							{
								m_core->astat |= SV;
							}
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x08:      /* Rn = Rn OR LSHIFT Rx BY Ry*/
					{
						int8_t shift = REG(ry);
						if(shift < 0) {
							REG(rn) = REG(rn) | ((shift > -32 ) ? (REG(rx) >> -shift) : 0);
						} else {
							REG(rn) = REG(rn) | ((shift < 32) ? (REG(rx) << shift) : 0);
							if (shift > 0)
							{
								m_core->astat |= SV;
							}
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x10:      /* FEXT Rx BY Ry */
					{
						int bit = REG(ry) & 0x3f;
						int len = (REG(ry) >> 6) & 0x3f;
						if (len == 0 || bit >= 32)
							REG(rn) = 0;
						else
							REG(rn) = BIT(REG(rx), bit, std::min(len, 32));

						SET_FLAG_SZ(REG(rn));
						if (bit+len > 32)
						{
							m_core->astat |= SV;
						}
						break;
					}

					case 0x12:      /* FEXT Rx BY Ry (Sign Extended) */
					{
						int bit = REG(ry) & 0x3f;
						int len = (REG(ry) >> 6) & 0x3f;
						if (len == 0 || bit >= 32)
							REG(rn) = 0;
						else if (bit+len > 32)
							REG(rn) = (uint32_t)REG(rx) >> bit;
						else
							REG(rn) = util::sext(REG(rx) >> bit, std::min(len, 32));

						SET_FLAG_SZ(REG(rn));
						if (bit+len > 32)
						{
							m_core->astat |= SV;
						}
						break;
					}

					case 0x19:      /* Rn = Rn OR FDEP Rx BY Ry */
					{
						int bit = REG(ry) & 0x3f;
						int len = (REG(ry) >> 6) & 0x3f;

						if (len != 0 && bit < 32)
							REG(rn) |= (REG(rx) & util::make_bitmask<uint32_t>(std::min(len, 32))) << bit;

						SET_FLAG_SZ(REG(rn));
						if (bit+len > 32)
						{
							m_core->astat |= SV;
						}
						break;
					}

					case 0x30:      /* BSET Rx BY Ry */
					{
						uint32_t shift = REG(ry);
						REG(rn) = REG(rx);
						if (shift < 32)
						{
							REG(rn) |= (1 << shift);
						}
						else
						{
							m_core->astat |= SV;
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x31:      /* BCLR Rx BY Ry */
					{
						uint32_t shift = REG(ry);
						REG(rn) = REG(rx);
						if (shift < 32)
						{
							REG(rn) &= ~(1 << shift);
						}
						else
						{
							m_core->astat |= SV;
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x33:      /* BTST Rx BY Ry */
					{
						uint32_t shift = REG(ry);
						if (shift < 32)
						{
							uint32_t r = REG(rx) & (1 << shift);

							SET_FLAG_SZ(r);
						}
						else
						{
							m_core->astat |= SZ | SV;
						}
						break;
					}

					default:
						fatalerror("SHARC: compute: shift operation %02X not implemented ! (%08X, %08X)\n", op, m_core->pc, opcode);
				}
				break;
			}

			default:
				fatalerror("SHARC: compute: invalid single-function operation %02X\n", cu);
		}
	}
}

void adsp21062_device::PUSH_PC(uint32_t pc)
{
	m_core->pcstkp++;
	if(m_core->pcstkp >= 32)
	{
		fatalerror("SHARC: PC Stack overflow!\n");
	}

	if (m_core->pcstkp == 0)
	{
		m_core->stky |= 0x400000;
	}
	else
	{
		m_core->stky &= ~0x400000;
	}

	m_core->pcstk = pc;
	m_core->pcstack[m_core->pcstkp] = pc;
}

uint32_t adsp21062_device::POP_PC()
{
	m_core->pcstk = m_core->pcstack[m_core->pcstkp];

	if(m_core->pcstkp == 0)
	{
		fatalerror("SHARC: PC Stack underflow!\n");
	}

	m_core->pcstkp--;

	if (m_core->pcstkp == 0)
	{
		m_core->stky |= 0x400000;
	}
	else
	{
		m_core->stky &= ~0x400000;
	}

	return m_core->pcstk;
}

uint32_t adsp21062_device::TOP_PC()
{
	return m_core->pcstack[m_core->pcstkp];
}

void adsp21062_device::PUSH_LOOP(uint32_t addr, uint32_t code, uint32_t type, uint32_t count)
{
	m_core->lstkp++;
	if(m_core->lstkp >= 6)
	{
		fatalerror("SHARC: Loop Stack overflow!\n");
	}

	if (m_core->lstkp == 0)
	{
		m_core->stky |= 0x4000000;
	}
	else
	{
		m_core->stky &= ~0x4000000;
	}

	m_core->lcstack[m_core->lstkp] = count;
	m_core->lastack[m_core->lstkp] = (type << 30) | (code << 24) | addr;
	m_core->curlcntr = count;

	m_core->laddr.addr = addr;
	m_core->laddr.code = code;
	m_core->laddr.loop_type = type;
}

void adsp21062_device::POP_LOOP()
{
	if(m_core->lstkp == 0)
	{
		fatalerror("SHARC: Loop Stack underflow!\n");
	}

	m_core->lstkp--;

	if (m_core->lstkp == 0)
	{
		m_core->stky |= 0x4000000;
	}
	else
	{
		m_core->stky &= ~0x4000000;
	}

	m_core->curlcntr = m_core->lcstack[m_core->lstkp];

	m_core->laddr.addr = m_core->lastack[m_core->lstkp] & 0xffffff;
	m_core->laddr.code = (m_core->lastack[m_core->lstkp] >> 24) & 0x1f;
	m_core->laddr.loop_type = (m_core->lastack[m_core->lstkp] >> 30) & 0x3;
}

void adsp21062_device::PUSH_STATUS_STACK()
{
	m_core->status_stkp++;
	if (m_core->status_stkp >= 5)
	{
		fatalerror("SHARC: Status stack overflow!\n");
	}

	if (m_core->status_stkp == 0)
	{
		m_core->stky |= 0x1000000;
	}
	else
	{
		m_core->stky &= ~0x1000000;
	}

	m_core->status_stack[m_core->status_stkp].mode1 = GET_UREG(REG_MODE1);
	m_core->status_stack[m_core->status_stkp].astat = GET_UREG(REG_ASTAT);
}

void adsp21062_device::POP_STATUS_STACK()
{
	SET_UREG(REG_MODE1, m_core->status_stack[m_core->status_stkp].mode1);
	SET_UREG(REG_ASTAT, m_core->status_stack[m_core->status_stkp].astat);

	m_core->status_stkp--;
	if (m_core->status_stkp < 0)
	{
		fatalerror("SHARC: Status stack underflow!\n");
	}

	if (m_core->status_stkp == 0)
	{
		m_core->stky |= 0x1000000;
	}
	else
	{
		m_core->stky &= ~0x1000000;
	}
}

int adsp21062_device::IF_CONDITION_CODE(int cond)
{
	switch(cond)
	{
		case 0x00:  return m_core->astat & AZ;        /* EQ */
		case 0x01:  return !(m_core->astat & AZ) && (m_core->astat & AN);   /* LT */
		case 0x02:  return (m_core->astat & AZ) || (m_core->astat & AN);    /* LE */
		case 0x03:  return (m_core->astat & AC);      /* AC */
		case 0x04:  return (m_core->astat & AV);      /* AV */
		case 0x05:  return (m_core->astat & MV);      /* MV */
		case 0x06:  return (m_core->astat & MN);      /* MS */
		case 0x07:  return (m_core->astat & SV);      /* SV */
		case 0x08:  return (m_core->astat & SZ);      /* SZ */
		case 0x09:  return (m_core->flag[0] != 0);    /* FLAG0 */
		case 0x0a:  return (m_core->flag[1] != 0);    /* FLAG1 */
		case 0x0b:  return (m_core->flag[2] != 0);    /* FLAG2 */
		case 0x0c:  return (m_core->flag[3] != 0);    /* FLAG3 */
		case 0x0d:  return (m_core->astat & BTF);     /* TF */
		case 0x0e:  return 0;                       /* BM */
		case 0x0f:  return (m_core->curlcntr!=1);     /* NOT LCE */
		case 0x10:  return !(m_core->astat & AZ);     /* NOT EQUAL */
		case 0x11:  return (m_core->astat & AZ) || !(m_core->astat & AN);   /* GE */
		case 0x12:  return !(m_core->astat & AZ) && !(m_core->astat & AN);  /* GT */
		case 0x13:  return !(m_core->astat & AC);     /* NOT AC */
		case 0x14:  return !(m_core->astat & AV);     /* NOT AV */
		case 0x15:  return !(m_core->astat & MV);     /* NOT MV */
		case 0x16:  return !(m_core->astat & MN);     /* NOT MS */
		case 0x17:  return !(m_core->astat & SV);     /* NOT SV */
		case 0x18:  return !(m_core->astat & SZ);     /* NOT SZ */
		case 0x19:  return (m_core->flag[0] == 0);    /* NOT FLAG0 */
		case 0x1a:  return (m_core->flag[1] == 0);    /* NOT FLAG1 */
		case 0x1b:  return (m_core->flag[2] == 0);    /* NOT FLAG2 */
		case 0x1c:  return (m_core->flag[3] == 0);    /* NOT FLAG3 */
		case 0x1d:  return !(m_core->astat & BTF);    /* NOT TF */
		case 0x1e:  return 1;                       /* NOT BM */
		case 0x1f:  return 1;                       /* TRUE */
	}
	return 1;
}

int adsp21062_device::DO_CONDITION_CODE(int cond)
{
	switch(cond)
	{
		case 0x00:  return m_core->astat & AZ;        /* EQ */
		case 0x01:  return !(m_core->astat & AZ) && (m_core->astat & AN);   /* LT */
		case 0x02:  return (m_core->astat & AZ) || (m_core->astat & AN);    /* LE */
		case 0x03:  return (m_core->astat & AC);      /* AC */
		case 0x04:  return (m_core->astat & AV);      /* AV */
		case 0x05:  return (m_core->astat & MV);      /* MV */
		case 0x06:  return (m_core->astat & MN);      /* MS */
		case 0x07:  return (m_core->astat & SV);      /* SV */
		case 0x08:  return (m_core->astat & SZ);      /* SZ */
		case 0x09:  return (m_core->flag[0] != 0);    /* FLAG0 */
		case 0x0a:  return (m_core->flag[1] != 0);    /* FLAG1 */
		case 0x0b:  return (m_core->flag[2] != 0);    /* FLAG2 */
		case 0x0c:  return (m_core->flag[3] != 0);    /* FLAG3 */
		case 0x0d:  return (m_core->astat & BTF);     /* TF */
		case 0x0e:  return 0;                       /* BM */
		case 0x0f:  return (m_core->curlcntr==1);     /* LCE */
		case 0x10:  return !(m_core->astat & AZ);     /* NOT EQUAL */
		case 0x11:  return (m_core->astat & AZ) || !(m_core->astat & AN);   /* GE */
		case 0x12:  return !(m_core->astat & AZ) && !(m_core->astat & AN);  /* GT */
		case 0x13:  return !(m_core->astat & AC);     /* NOT AC */
		case 0x14:  return !(m_core->astat & AV);     /* NOT AV */
		case 0x15:  return !(m_core->astat & MV);     /* NOT MV */
		case 0x16:  return !(m_core->astat & MN);     /* NOT MS */
		case 0x17:  return !(m_core->astat & SV);     /* NOT SV */
		case 0x18:  return !(m_core->astat & SZ);     /* NOT SZ */
		case 0x19:  return (m_core->flag[0] == 0);    /* NOT FLAG0 */
		case 0x1a:  return (m_core->flag[1] == 0);    /* NOT FLAG1 */
		case 0x1b:  return (m_core->flag[2] == 0);    /* NOT FLAG2 */
		case 0x1c:  return (m_core->flag[3] == 0);    /* NOT FLAG3 */
		case 0x1d:  return !(m_core->astat & BTF);    /* NOT TF */
		case 0x1e:  return 1;                       /* NOT BM */
		case 0x1f:  return 0;                       /* FALSE (FOREVER) */
	}
	return 1;
}

/*****************************************************************************/
/* | 001xxxxxx | */

/* compute / dreg <-> DM / dreg <-> PM */
void adsp21062_device::sharcop_compute_dreg_dm_dreg_pm()
{
	int pm_dreg = (m_core->opcode >> 23) & 0xf;
	int pmm = (m_core->opcode >> 27) & 0x7;
	int pmi = (m_core->opcode >> 30) & 0x7;
	int dm_dreg = (m_core->opcode >> 33) & 0xf;
	int dmm = (m_core->opcode >> 38) & 0x7;
	int dmi = (m_core->opcode >> 41) & 0x7;
	int pmd = (m_core->opcode >> 37) & 0x1;
	int dmd = (m_core->opcode >> 44) & 0x1;
	int compute = m_core->opcode & 0x7fffff;

	/* due to parallelity issues, source DREGs must be saved */
	/* because the compute operation may change them */
	uint32_t parallel_pm_dreg = REG(pm_dreg);
	uint32_t parallel_dm_dreg = REG(dm_dreg);

	if (compute)
	{
		COMPUTE(compute);
	}

	if (pmd)        // dreg -> PM
	{
		pm_write32(PM_REG_I(pmi), parallel_pm_dreg);
		PM_REG_I(pmi) += PM_REG_M(pmm);
		UPDATE_CIRCULAR_BUFFER_PM(pmi);
	}
	else            // PM -> dreg
	{
		REG(pm_dreg) = pm_read32(PM_REG_I(pmi));
		PM_REG_I(pmi) += PM_REG_M(pmm);
		UPDATE_CIRCULAR_BUFFER_PM(pmi);
	}

	if (dmd)        // dreg -> DM
	{
		dm_write32(DM_REG_I(dmi), parallel_dm_dreg);
		DM_REG_I(dmi) += DM_REG_M(dmm);
		UPDATE_CIRCULAR_BUFFER_DM(dmi);
	}
	else            // DM -> dreg
	{
		REG(dm_dreg) = dm_read32(DM_REG_I(dmi));
		DM_REG_I(dmi) += DM_REG_M(dmm);
		UPDATE_CIRCULAR_BUFFER_DM(dmi);
	}
}

/*****************************************************************************/
/* | 00000001x | */

/* compute */
void adsp21062_device::sharcop_compute()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond) && compute != 0)
	{
		COMPUTE(compute);
	}
}

/*****************************************************************************/
/* | 010xxxxxx | */

/* compute / ureg <-> DM|PM, pre-modify */
void adsp21062_device::sharcop_compute_ureg_dmpm_premod()
{
	int i = (m_core->opcode >> 41) & 0x7;
	int m = (m_core->opcode >> 38) & 0x7;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int g = (m_core->opcode >> 32) & 0x1;
	int d = (m_core->opcode >> 31) & 0x1;
	int ureg = (m_core->opcode >> 23) & 0xff;
	int compute = m_core->opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source UREG must be saved */
		/* because the compute operation may change it */
		uint32_t parallel_ureg = GET_UREG(ureg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (g)      /* PM */
		{
			if (d)      /* ureg -> PM */
			{
				if (ureg == 0xdb)       /* PX register access is always 48-bit */
				{
					pm_write48(PM_REG_I(i)+PM_REG_M(m), m_core->px);
				}
				else
				{
					pm_write32(PM_REG_I(i)+PM_REG_M(m), parallel_ureg);
				}
			}
			else        /* PM <- ureg */
			{
				if (ureg == 0xdb)       /* PX register access is always 48-bit */
				{
					m_core->px = pm_read48(PM_REG_I(i)+PM_REG_M(m));
				}
				else
				{
					SET_UREG(ureg, pm_read32(PM_REG_I(i)+PM_REG_M(m)));
				}
			}
		}
		else    /* DM */
		{
			if (d)      /* ureg -> DM */
			{
				dm_write32(DM_REG_I(i)+DM_REG_M(m), parallel_ureg);
			}
			else        /* DM <- ureg */
			{
				SET_UREG(ureg, dm_read32(DM_REG_I(i)+DM_REG_M(m)));
			}
		}
	}
}

/* compute / ureg <-> DM|PM, post-modify */
void adsp21062_device::sharcop_compute_ureg_dmpm_postmod()
{
	int i = (m_core->opcode >> 41) & 0x7;
	int m = (m_core->opcode >> 38) & 0x7;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int g = (m_core->opcode >> 32) & 0x1;
	int d = (m_core->opcode >> 31) & 0x1;
	int ureg = (m_core->opcode >> 23) & 0xff;
	int compute = m_core->opcode & 0x7fffff;

	if(IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source UREG must be saved */
		/* because the compute operation may change it */
		uint32_t parallel_ureg = GET_UREG(ureg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (g)      /* PM */
		{
			if (d)      /* ureg -> PM */
			{
				if (ureg == 0xdb)       /* PX register access is always 48-bit */
				{
					pm_write48(PM_REG_I(i), m_core->px);
				}
				else
				{
					pm_write32(PM_REG_I(i), parallel_ureg);
				}
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
			else        /* PM <- ureg */
			{
				if (ureg == 0xdb)       /* PX register access is always 48-bit */
				{
					m_core->px = pm_read48(PM_REG_I(i));
				}
				else
				{
					SET_UREG(ureg, pm_read32(PM_REG_I(i)));
				}
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
		}
		else    /* DM */
		{
			if (d)      /* ureg -> DM */
			{
				dm_write32(DM_REG_I(i), parallel_ureg);
				DM_REG_I(i) += DM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_DM(i);
			}
			else        /* DM <- ureg */
			{
				SET_UREG(ureg, dm_read32(DM_REG_I(i)));
				DM_REG_I(i) += DM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_DM(i);
			}
		}
	}
}

/*****************************************************************************/
/* | 0110xxxxx | */

/* compute / dreg <- DM, immediate modify */
void adsp21062_device::sharcop_compute_dm_to_dreg_immmod()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int u = (m_core->opcode >> 38) & 0x1;
	int dreg = (m_core->opcode >> 23) & 0xf;
	int i = (m_core->opcode >> 41) & 0x7;
	int mod = util::sext((m_core->opcode >> 27) & 0x3f, 6);
	int compute = m_core->opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)      /* post-modify with update */
		{
			REG(dreg) = dm_read32(DM_REG_I(i));
			DM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_DM(i);
		}
		else        /* pre-modify, no update */
		{
			REG(dreg) = dm_read32(DM_REG_I(i) + mod);
		}
	}
}

/* compute / dreg -> DM, immediate modify */
void adsp21062_device::sharcop_compute_dreg_to_dm_immmod()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int u = (m_core->opcode >> 38) & 0x1;
	int dreg = (m_core->opcode >> 23) & 0xf;
	int i = (m_core->opcode >> 41) & 0x7;
	int mod = util::sext((m_core->opcode >> 27) & 0x3f, 6);
	int compute = m_core->opcode & 0x7fffff;

	/* due to parallelity issues, source REG must be saved */
	/* because the shift operation may change it */
	uint32_t parallel_dreg = REG(dreg);

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)      /* post-modify with update */
		{
			dm_write32(DM_REG_I(i), parallel_dreg);
			DM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_DM(i);
		}
		else        /* pre-modify, no update */
		{
			dm_write32(DM_REG_I(i) + mod, parallel_dreg);
		}
	}
}

/* compute / dreg <- PM, immediate modify */
void adsp21062_device::sharcop_compute_pm_to_dreg_immmod()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int u = (m_core->opcode >> 38) & 0x1;
	int dreg = (m_core->opcode >> 23) & 0xf;
	int i = (m_core->opcode >> 41) & 0x7;
	int mod = util::sext((m_core->opcode >> 27) & 0x3f, 6);
	int compute = m_core->opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)      /* post-modify with update */
		{
			REG(dreg) = pm_read32(PM_REG_I(i));
			PM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_PM(i);
		}
		else        /* pre-modify, no update */
		{
			REG(dreg) = pm_read32(PM_REG_I(i) + mod);
		}
	}
}

/* compute / dreg -> PM, immediate modify */
void adsp21062_device::sharcop_compute_dreg_to_pm_immmod()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int u = (m_core->opcode >> 38) & 0x1;
	int dreg = (m_core->opcode >> 23) & 0xf;
	int i = (m_core->opcode >> 41) & 0x7;
	int mod = util::sext((m_core->opcode >> 27) & 0x3f, 6);
	int compute = m_core->opcode & 0x7fffff;

	/* due to parallelity issues, source REG must be saved */
	/* because the compute operation may change it */
	uint32_t parallel_dreg = REG(dreg);

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)      /* post-modify with update */
		{
			pm_write32(PM_REG_I(i), parallel_dreg);
			PM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_PM(i);
		}
		else        /* pre-modify, no update */
		{
			pm_write32(PM_REG_I(i) + mod, parallel_dreg);
		}
	}
}

/*****************************************************************************/
/* | 0111xxxxx | */

/* compute / ureg <-> ureg */
void adsp21062_device::sharcop_compute_ureg_to_ureg()
{
	int src_ureg = (m_core->opcode >> 36) & 0xff;
	int dst_ureg = (m_core->opcode >> 23) & 0xff;
	int cond = (m_core->opcode >> 31) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source UREG must be saved */
		/* because the compute operation may change it */
		uint32_t parallel_ureg = GET_UREG(src_ureg);

		if (compute != 0)
		{
			COMPUTE(compute);
		}

		SET_UREG(dst_ureg, parallel_ureg);
	}
}

/*****************************************************************************/
/* | 1000xxxxx | */

/* immediate shift / dreg <-> DM|PM */
void adsp21062_device::sharcop_imm_shift_dreg_dmpm()
{
	int i = (m_core->opcode >> 41) & 0x7;
	int m = (m_core->opcode >> 38) & 0x7;
	int g = (m_core->opcode >> 32) & 0x1;
	int d = (m_core->opcode >> 31) & 0x1;
	int dreg = (m_core->opcode >> 23) & 0xf;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int data = ((m_core->opcode >> 8) & 0xff) | ((m_core->opcode >> 19) & 0xf00);
	int shiftop = (m_core->opcode >> 16) & 0x3f;
	int rn = (m_core->opcode >> 4) & 0xf;
	int rx = (m_core->opcode & 0xf);

	if (IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source REG must be saved */
		/* because the shift operation may change it */
		uint32_t parallel_dreg = REG(dreg);

		SHIFT_OPERATION_IMM(shiftop, data, rn, rx);

		if (g)      /* PM */
		{
			if (d)      /* dreg -> PM */
			{
				pm_write32(PM_REG_I(i), parallel_dreg);
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
			else        /* PM <- dreg */
			{
				REG(dreg) = pm_read32(PM_REG_I(i));
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
		}
		else        /* DM */
		{
			if (d)      /* dreg -> DM */
			{
				dm_write32(DM_REG_I(i), parallel_dreg);
				DM_REG_I(i) += DM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_DM(i);
			}
			else    /* DM <- dreg */
			{
				REG(dreg) = dm_read32(DM_REG_I(i));
				DM_REG_I(i) += DM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_DM(i);
			}
		}
	}
}

/*****************************************************************************/
/* | 00000010x | */

/* immediate shift */
void adsp21062_device::sharcop_imm_shift()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int data = ((m_core->opcode >> 8) & 0xff) | ((m_core->opcode >> 19) & 0xf00);
	int shiftop = (m_core->opcode >> 16) & 0x3f;
	int rn = (m_core->opcode >> 4) & 0xf;
	int rx = (m_core->opcode & 0xf);

	if (IF_CONDITION_CODE(cond))
	{
		SHIFT_OPERATION_IMM(shiftop, data, rn, rx);
	}
}

/*****************************************************************************/
/* | 00000100x | */

/* compute / modify */
void adsp21062_device::sharcop_compute_modify()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;
	int g = (m_core->opcode >> 38) & 0x1;
	int m = (m_core->opcode >> 27) & 0x7;
	int i = (m_core->opcode >> 30) & 0x7;

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (g)      /* Modify PM */
		{
			PM_REG_I(i) += PM_REG_M(m);
			UPDATE_CIRCULAR_BUFFER_PM(i);
		}
		else        /* Modify DM */
		{
			DM_REG_I(i) += DM_REG_M(m);
			UPDATE_CIRCULAR_BUFFER_DM(i);
		}
	}
}

/*****************************************************************************/
/* | 00000110x | */

/* direct call to absolute address */
void adsp21062_device::sharcop_direct_call()
{
	int j = (m_core->opcode >> 26) & 0x1;
	int cond = (m_core->opcode >> 33) & 0x1f;
	uint32_t address = m_core->opcode & 0xffffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (j)
		{
			//PUSH_PC(m_core->pc+3);  /* 1 instruction + 2 delayed instructions */
			PUSH_PC(m_core->nfaddr);    /* 1 instruction + 2 delayed instructions */
			CHANGE_PC_DELAYED(address);
		}
		else
		{
			//PUSH_PC(m_core->pc+1);
			PUSH_PC(m_core->daddr);
			CHANGE_PC(address);
		}
	}
}

/* direct jump to absolute address */
void adsp21062_device::sharcop_direct_jump()
{
	int la = (m_core->opcode >> 38) & 0x1;
	int ci = (m_core->opcode >> 24) & 0x1;
	int j = (m_core->opcode >> 26) & 0x1;
	int cond = (m_core->opcode >> 33) & 0x1f;
	uint32_t address = m_core->opcode & 0xffffff;

	if(IF_CONDITION_CODE(cond))
	{
		// Clear Interrupt
		if (ci)
		{
			// TODO: anything else?
			if (m_core->status_stkp > 0)
			{
				POP_STATUS_STACK();
			}

			m_core->interrupt_active = 0;
			m_core->irptl &= ~(1 << m_core->active_irq_num);
		}

		if (la)
		{
			POP_PC();
			POP_LOOP();
		}

		if (j)
		{
			CHANGE_PC_DELAYED(address);
		}
		else
		{
			CHANGE_PC(address);
		}
	}
}

/*****************************************************************************/
/* | 00000111x | */

/* direct call to relative address */
void adsp21062_device::sharcop_relative_call()
{
	int j = (m_core->opcode >> 26) & 0x1;
	int cond = (m_core->opcode >> 33) & 0x1f;
	uint32_t address = m_core->opcode & 0xffffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (j)
		{
			PUSH_PC(m_core->pc+3);  /* 1 instruction + 2 delayed instructions */
			CHANGE_PC_DELAYED(m_core->pc + util::sext(address, 24));
		}
		else
		{
			PUSH_PC(m_core->pc+1);
			CHANGE_PC(m_core->pc + util::sext(address, 24));
		}
	}
}

/* direct jump to relative address */
void adsp21062_device::sharcop_relative_jump()
{
	int la = (m_core->opcode >> 38) & 0x1;
	int ci = (m_core->opcode >> 24) & 0x1;
	int j = (m_core->opcode >> 26) & 0x1;
	int cond = (m_core->opcode >> 33) & 0x1f;
	uint32_t address = m_core->opcode & 0xffffff;

	if (IF_CONDITION_CODE(cond))
	{
		// Clear Interrupt
		if (ci)
		{
			// TODO: anything else?
			if (m_core->status_stkp > 0)
			{
				POP_STATUS_STACK();
			}

			m_core->interrupt_active = 0;
			m_core->irptl &= ~(1 << m_core->active_irq_num);
		}

		if (la)
		{
			POP_PC();
			POP_LOOP();
		}

		if (j)
		{
			CHANGE_PC_DELAYED(m_core->pc + util::sext(address, 24));
		}
		else
		{
			CHANGE_PC(m_core->pc + util::sext(address, 24));
		}
	}
}

/*****************************************************************************/
/* | 00001000x | */

/* indirect jump */
void adsp21062_device::sharcop_indirect_jump()
{
	int la = (m_core->opcode >> 38) & 0x1;
	int ci = (m_core->opcode >> 24) & 0x1;
	int j = (m_core->opcode >> 26) & 0x1;
	int e = (m_core->opcode >> 25) & 0x1;
	int pmi = (m_core->opcode >> 30) & 0x7;
	int pmm = (m_core->opcode >> 27) & 0x7;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;

	// Clear Interrupt
	if (ci)
	{
		// TODO: anything else?
		if (m_core->status_stkp > 0)
		{
			POP_STATUS_STACK();
		}

		m_core->interrupt_active = 0;
		m_core->irptl &= ~(1 << m_core->active_irq_num);
	}

	if (e)      /* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (la)
			{
				POP_PC();
				POP_LOOP();
			}

			if(j)
			{
				CHANGE_PC_DELAYED(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
			else
			{
				CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
		}
		else
		{
			if (compute)
			{
				COMPUTE(compute);
			}
		}
	}
	else        /* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (la)
			{
				POP_PC();
				POP_LOOP();
			}

			if(j)
			{
				CHANGE_PC_DELAYED(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
			else
			{
				CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
		}
	}
}

/* indirect call */
void adsp21062_device::sharcop_indirect_call()
{
	int j = (m_core->opcode >> 26) & 0x1;
	int e = (m_core->opcode >> 25) & 0x1;
	int pmi = (m_core->opcode >> 30) & 0x7;
	int pmm = (m_core->opcode >> 27) & 0x7;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;

	if (e)      /* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (j)
			{
				//PUSH_PC(m_core->pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(m_core->nfaddr);    /* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
			else
			{
				//PUSH_PC(m_core->pc+1);
				PUSH_PC(m_core->daddr);
				CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
		}
		else
		{
			if (compute)
			{
				COMPUTE(compute);
			}
		}
	}
	else        /* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (j)
			{
				//PUSH_PC(m_core->pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(m_core->nfaddr);    /* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
			else
			{
				//PUSH_PC(m_core->pc+1);
				PUSH_PC(m_core->daddr);
				CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
		}
	}
}

/*****************************************************************************/
/* | 00001001x | */

/* indirect jump to relative address */
void adsp21062_device::sharcop_relative_jump_compute()
{
	int la = (m_core->opcode >> 38) & 0x1;
	int ci = (m_core->opcode >> 24) & 0x1;
	int j = (m_core->opcode >> 26) & 0x1;
	int e = (m_core->opcode >> 25) & 0x1;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;

	// Clear Interrupt
	if (ci)
	{
		// TODO: anything else?
		if (m_core->status_stkp > 0)
		{
			POP_STATUS_STACK();
		}

		m_core->interrupt_active = 0;
		m_core->irptl &= ~(1 << m_core->active_irq_num);
	}

	if (e)      /* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (la)
			{
				POP_PC();
				POP_LOOP();
			}

			if (j)
			{
				CHANGE_PC_DELAYED(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
			else
			{
				CHANGE_PC(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
		}
		else
		{
			if (compute)
			{
				COMPUTE(compute);
			}
		}
	}
	else        /* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (la)
			{
				POP_PC();
				POP_LOOP();
			}

			if (j)
			{
				CHANGE_PC_DELAYED(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
			else
			{
				CHANGE_PC(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
		}
	}
}

/* indirect call to relative address */
void adsp21062_device::sharcop_relative_call_compute()
{
	int j = (m_core->opcode >> 26) & 0x1;
	int e = (m_core->opcode >> 25) & 0x1;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int compute = m_core->opcode & 0x7fffff;

	if (e)      /* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (j)
			{
				//PUSH_PC(m_core->pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(m_core->nfaddr);    /* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
			else
			{
				//PUSH_PC(m_core->pc+1);
				PUSH_PC(m_core->daddr);
				CHANGE_PC(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
		}
		else
		{
			if (compute)
			{
				COMPUTE(compute);
			}
		}
	}
	else        /* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (j)
			{
				//PUSH_PC(m_core->pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(m_core->nfaddr);    /* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
			else
			{
				//PUSH_PC(m_core->pc+1);
				PUSH_PC(m_core->daddr);
				CHANGE_PC(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
			}
		}
	}
}

/*****************************************************************************/
/* | 110xxxxxx | */

/* indirect jump / compute / dreg <-> DM */
void adsp21062_device::sharcop_indirect_jump_compute_dreg_dm()
{
	int d = (m_core->opcode >> 44) & 0x1;
	int dmi = (m_core->opcode >> 41) & 0x7;
	int dmm = (m_core->opcode >> 38) & 0x7;
	int pmi = (m_core->opcode >> 30) & 0x7;
	int pmm = (m_core->opcode >> 27) & 0x7;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int dreg = (m_core->opcode >> 23) & 0xf;

	if (IF_CONDITION_CODE(cond))
	{
		CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
	}
	else
	{
		uint32_t compute = m_core->opcode & 0x7fffff;
		/* due to parallelity issues, source REG must be saved */
		/* because the compute operation may change it */
		uint32_t parallel_dreg = REG(dreg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (d)      /* dreg -> DM */
		{
			dm_write32(DM_REG_I(dmi), parallel_dreg);
			DM_REG_I(dmi) += DM_REG_M(dmm);
			UPDATE_CIRCULAR_BUFFER_DM(dmi);
		}
		else        /* DM <- dreg */
		{
			REG(dreg) = dm_read32(DM_REG_I(dmi));
			DM_REG_I(dmi) += DM_REG_M(dmm);
			UPDATE_CIRCULAR_BUFFER_DM(dmi);
		}
	}
}

/*****************************************************************************/
/* | 111xxxxxx | */

/* relative jump / compute / dreg <-> DM */
void adsp21062_device::sharcop_relative_jump_compute_dreg_dm()
{
	int d = (m_core->opcode >> 44) & 0x1;
	int dmi = (m_core->opcode >> 41) & 0x7;
	int dmm = (m_core->opcode >> 38) & 0x7;
	int cond = (m_core->opcode >> 33) & 0x1f;
	int dreg = (m_core->opcode >> 23) & 0xf;

	if (IF_CONDITION_CODE(cond))
	{
		CHANGE_PC(m_core->pc + util::sext((m_core->opcode >> 27) & 0x3f, 6));
	}
	else
	{
		uint32_t compute = m_core->opcode & 0x7fffff;
		/* due to parallelity issues, source REG must be saved */
		/* because the compute operation may change it */
		uint32_t parallel_dreg = REG(dreg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (d)      /* dreg -> DM */
		{
			dm_write32(DM_REG_I(dmi), parallel_dreg);
			DM_REG_I(dmi) += DM_REG_M(dmm);
			UPDATE_CIRCULAR_BUFFER_DM(dmi);
		}
		else        /* DM <- dreg */
		{
			REG(dreg) = dm_read32(DM_REG_I(dmi));
			DM_REG_I(dmi) += DM_REG_M(dmm);
			UPDATE_CIRCULAR_BUFFER_DM(dmi);
		}
	}
}

/*****************************************************************************/
/* | 00001010x | */

/* return from subroutine / compute */
void adsp21062_device::sharcop_rts()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int j = (m_core->opcode >> 26) & 0x1;
	int e = (m_core->opcode >> 25) & 0x1;
	//int lr = (m_core->opcode >> 24) & 0x1;
	int compute = m_core->opcode & 0x7fffff;

	//if(lr)
	//  fatalerror("SHARC: rts: loop reentry not implemented!\n");

	if (e)      /* IF...ELSE */
	{
		if(IF_CONDITION_CODE(cond))
		{
			if (j)
			{
				CHANGE_PC_DELAYED(POP_PC());
			}
			else
			{
				CHANGE_PC(POP_PC());
			}
		}
		else
		{
			if (compute)
			{
				COMPUTE(compute);
			}
		}
	}
	else        /* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (j)
			{
				CHANGE_PC_DELAYED(POP_PC());
			}
			else
			{
				CHANGE_PC(POP_PC());
			}
		}
	}
}

/*****************************************************************************/
/* | 00001011x | */

/* return from interrupt / compute */
void adsp21062_device::sharcop_rti()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int j = (m_core->opcode >> 26) & 0x1;
	int e = (m_core->opcode >> 25) & 0x1;
	int compute = m_core->opcode & 0x7fffff;

	m_core->irptl &= ~(1 << m_core->active_irq_num);

	if(e)       /* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (j)
			{
				CHANGE_PC_DELAYED(POP_PC());
			}
			else
			{
				CHANGE_PC(POP_PC());
			}
		}
		else
		{
			if (compute)
			{
				COMPUTE(compute);
			}
		}
	}
	else        /* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (j)
			{
				CHANGE_PC_DELAYED(POP_PC());
			}
			else
			{
				CHANGE_PC(POP_PC());
			}
		}
	}

	if (m_core->status_stkp > 0)
	{
		POP_STATUS_STACK();
	}

	m_core->interrupt_active = 0;
	check_interrupts();
}

/*****************************************************************************/
/* | 00001100x | */

/* do until counter expired, LCNTR immediate */
void adsp21062_device::sharcop_do_until_counter_imm()
{
	uint16_t data = (uint16_t)(m_core->opcode >> 24);
	int offset = util::sext(m_core->opcode & 0xffffff, 24);
	uint32_t address = m_core->pc + offset;
	int type;
	int cond = 0xf;     /* until LCE (loop counter expired */
	int distance = abs(offset);

	if (distance == 1)
	{
		type = 1;
	}
	else if (distance == 2)
	{
		type = 2;
	}
	else
	{
		type = 3;
	}

	m_core->lcntr = data;
	if (m_core->lcntr > 0)
	{
		PUSH_PC(m_core->pc+1);
		PUSH_LOOP(address, cond, type, m_core->lcntr);
	}
}

/*****************************************************************************/
/* | 00001101x | */

/* do until counter expired, LCNTR from UREG */
void adsp21062_device::sharcop_do_until_counter_ureg()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	int offset = util::sext(m_core->opcode & 0xffffff, 24);
	uint32_t address = m_core->pc + offset;
	int type;
	int cond = 0xf;     /* until LCE (loop counter expired */
	int distance = abs(offset);

	if (distance == 1)
	{
		type = 1;
	}
	else if (distance == 2)
	{
		type = 2;
	}
	else
	{
		type = 3;
	}

	m_core->lcntr = GET_UREG(ureg);
	if (m_core->lcntr > 0)
	{
		PUSH_PC(m_core->pc+1);
		PUSH_LOOP(address, cond, type, m_core->lcntr);
	}
}

/*****************************************************************************/
/* | 00001110x | */

/* do until */
void adsp21062_device::sharcop_do_until()
{
	int cond = (m_core->opcode >> 33) & 0x1f;
	int offset = util::sext(m_core->opcode & 0xffffff, 24);
	uint32_t address = (m_core->pc + offset);

	PUSH_PC(m_core->pc+1);
	PUSH_LOOP(address, cond, 0, 0);
}

/*****************************************************************************/
/* | 000100 | G | D | */

/* ureg <- DM (direct addressing) */
void adsp21062_device::sharcop_dm_to_ureg_direct()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t address = (uint32_t)(m_core->opcode);

	SET_UREG(ureg, dm_read32(address));
}

/* ureg -> DM (direct addressing) */
void adsp21062_device::sharcop_ureg_to_dm_direct()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t address = (uint32_t)(m_core->opcode);

	dm_write32(address, GET_UREG(ureg));
}

/* ureg <- PM (direct addressing) */
void adsp21062_device::sharcop_pm_to_ureg_direct()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t address = (uint32_t)(m_core->opcode);

	if (ureg == 0xdb)       // PX is 48-bit
	{
		m_core->px = pm_read48(address);
	}
	else
	{
		SET_UREG(ureg, pm_read32(address));
	}
}

/* ureg -> PM (direct addressing) */
void adsp21062_device::sharcop_ureg_to_pm_direct()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t address = (uint32_t)(m_core->opcode);

	if (ureg == 0xdb)       // PX is 48-bit
	{
		pm_write48(address, m_core->px);
	}
	else
	{
		pm_write32(address, GET_UREG(ureg));
	}
}

/*****************************************************************************/
/* | 101 | G | III | D | */

/* ureg <- DM (indirect addressing) */
void adsp21062_device::sharcop_dm_to_ureg_indirect()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t offset = (uint32_t)m_core->opcode;
	int i = (m_core->opcode >> 41) & 0x7;

	SET_UREG(ureg, dm_read32(DM_REG_I(i) + offset));
}

/* ureg -> DM (indirect addressing) */
void adsp21062_device::sharcop_ureg_to_dm_indirect()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t offset = (uint32_t)m_core->opcode;
	int i = (m_core->opcode >> 41) & 0x7;

	dm_write32(DM_REG_I(i) + offset, GET_UREG(ureg));
}

/* ureg <- PM (indirect addressing) */
void adsp21062_device::sharcop_pm_to_ureg_indirect()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t offset = m_core->opcode & 0xffffff;
	int i = (m_core->opcode >> 41) & 0x7;

	if (ureg == 0xdb)       /* PX is 48-bit */
	{
		m_core->px = pm_read48(PM_REG_I(i) + offset);
	}
	else
	{
		SET_UREG(ureg, pm_read32(PM_REG_I(i) + offset));
	}
}

/* ureg -> PM (indirect addressing) */
void adsp21062_device::sharcop_ureg_to_pm_indirect()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t offset = (uint32_t)m_core->opcode;
	int i = (m_core->opcode >> 41) & 0x7;

	if (ureg == 0xdb)       /* PX is 48-bit */
	{
		pm_write48(PM_REG_I(i) + offset, m_core->px);
	}
	else
	{
		pm_write32(PM_REG_I(i) + offset, GET_UREG(ureg));
	}
}

/*****************************************************************************/
/* | 1001xxxxx | */

/* immediate data -> DM|PM */
void adsp21062_device::sharcop_imm_to_dmpm()
{
	int i = (m_core->opcode >> 41) & 0x7;
	int m = (m_core->opcode >> 38) & 0x7;
	int g = (m_core->opcode >> 37) & 0x1;
	uint32_t data = (uint32_t)m_core->opcode;

	if (g)
	{
		/* program memory (PM) */
		pm_write32(PM_REG_I(i), data);
		PM_REG_I(i) += PM_REG_M(m);
		UPDATE_CIRCULAR_BUFFER_PM(i);
	}
	else
	{
		/* data memory (DM) */
		dm_write32(DM_REG_I(i), data);
		DM_REG_I(i) += DM_REG_M(m);
		UPDATE_CIRCULAR_BUFFER_DM(i);
	}
}

/*****************************************************************************/
/* | 00001111x | */

/* immediate data -> ureg */
void adsp21062_device::sharcop_imm_to_ureg()
{
	int ureg = (m_core->opcode >> 32) & 0xff;
	uint32_t data = (uint32_t)m_core->opcode;

	SET_UREG(ureg, data);
}

/*****************************************************************************/
/* | 00010100x | */

/* system register bit manipulation */
void adsp21062_device::sharcop_sysreg_bitop()
{
	int bop = (m_core->opcode >> 37) & 0x7;
	int sreg = (m_core->opcode >> 32) & 0xf;
	uint32_t data = (uint32_t)m_core->opcode;

	uint32_t src = GET_UREG(0x70 | sreg);

	switch(bop)
	{
		case 0:     /* SET */
		{
			src |= data;
			break;
		}
		case 1:     /* CLEAR */
		{
			src &= ~data;
			break;
		}
		case 2:     /* TOGGLE */
		{
			src ^= data;
			break;
		}
		case 4:     /* TEST */
		{
			if ((src & data) == data)
			{
				m_core->astat |= BTF;
			}
			else
			{
				m_core->astat &= ~BTF;
			}
			break;
		}
		case 5:     /* XOR */
		{
			if (src == data)
			{
				m_core->astat |= BTF;
			}
			else
			{
				m_core->astat &= ~BTF;
			}
			break;
		}
		default:
			fatalerror("SHARC: sysreg_bitop: invalid bitop %d\n", bop);
			break;
	}

	SET_UREG(0x70 | sreg, src);
}

/*****************************************************************************/
/* | 000101100 | */

/* I register modify */
void adsp21062_device::sharcop_modify()
{
	int g = (m_core->opcode >> 38) & 0x1;
	int i = (m_core->opcode >> 32) & 0x7;
	int32_t data = (m_core->opcode);

	if (g)      // PM
	{
		PM_REG_I(i) += data;
		UPDATE_CIRCULAR_BUFFER_PM(i);
	}
	else        // DM
	{
		DM_REG_I(i) += data;
		UPDATE_CIRCULAR_BUFFER_DM(i);
	}
}

/*****************************************************************************/
/* | 000101101 | */

/* I register bit-reverse */
void adsp21062_device::sharcop_bit_reverse()
{
	fatalerror("SHARC: sharcop_bit_reverse unimplemented\n");
}

/*****************************************************************************/
/* | 00010111x | */

/* push/pop stacks / flush cache */
void adsp21062_device::sharcop_push_pop_stacks()
{
	if (m_core->opcode & 0x008000000000U)
	{
		fatalerror("sharcop_push_pop_stacks: push loop not implemented\n");
	}
	if (m_core->opcode & 0x004000000000U)
	{
		fatalerror("sharcop_push_pop_stacks: pop loop not implemented\n");
	}
	if (m_core->opcode & 0x002000000000U)
	{
		//fatalerror("sharcop_push_pop_stacks: push sts not implemented\n");
		PUSH_STATUS_STACK();
	}
	if (m_core->opcode & 0x001000000000U)
	{
		//fatalerror("sharcop_push_pop_stacks: pop sts not implemented\n");
		POP_STATUS_STACK();
	}
	if (m_core->opcode & 0x000800000000U)
	{
		PUSH_PC(m_core->pcstk);
	}
	if (m_core->opcode & 0x000400000000U)
	{
		POP_PC();
	}
}

/*****************************************************************************/
/* | 000000000 | */

void adsp21062_device::sharcop_nop()
{
}

/*****************************************************************************/
/* | 000000001 | */

void adsp21062_device::sharcop_idle()
{
	//CHANGE_PC(m_core->pc);

	m_core->daddr = m_core->pc;
	m_core->faddr = m_core->pc+1;
	m_core->nfaddr = m_core->pc+2;

	m_core->idle = 1;
}

/*****************************************************************************/

void adsp21062_device::sharcop_unimplemented()
{
	fatalerror("SHARC: Unimplemented opcode %012X at %08X\n", m_core->opcode, m_core->pc);
}
