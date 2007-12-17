#define SIGN_EXTEND6(x)		(((x) & 0x20) ? (0xffffffc0 | (x)) : (x))
#define SIGN_EXTEND24(x)	(((x) & 0x800000) ? (0xff000000 | (x)) : (x))

#define PM_REG_I(x)			(sharc.dag2.i[x])
#define PM_REG_M(x)			(sharc.dag2.m[x])
#define PM_REG_B(x)			(sharc.dag2.b[x])
#define PM_REG_L(x)			(sharc.dag2.l[x])
#define DM_REG_I(x)			(sharc.dag1.i[x])
#define DM_REG_M(x)			(sharc.dag1.m[x])
#define DM_REG_B(x)			(sharc.dag1.b[x])
#define DM_REG_L(x)			(sharc.dag1.l[x])

// ASTAT flags
#define AZ		0x1			/* ALU result zero */
#define AV		0x2			/* ALU overflow */
#define AN		0x4			/* ALU result negative */
#define AC		0x8			/* ALU fixed-point carry */
#define AS		0x10		/* ALU X input sign */
#define AI		0x20		/* ALU floating-point invalid operation */
#define MN		0x40		/* Multiplier result negative */
#define MV		0x80		/* Multiplier overflow */
#define MU		0x100		/* Multiplier underflow */
#define MI		0x200		/* Multiplier floating-point invalid operation */
#define AF		0x400
#define SV		0x800		/* Shifter overflow */
#define SZ		0x1000		/* Shifter result zero */
#define SS		0x2000		/* Shifter input sign */
#define BTF		0x40000		/* Bit Test Flag */
#define FLG0	0x80000		/* FLAG0 */
#define FLG1	0x100000	/* FLAG1 */
#define FLG2	0x200000	/* FLAG2 */
#define FLG3	0x400000	/* FLAG3 */

// STKY flags
#define AUS		0x1			/* ALU floating-point underflow */
#define AVS		0x2			/* ALU floating-point overflow */
#define AOS		0x4			/* ALU fixed-point overflow */
#define AIS		0x20		/* ALU floating-point invalid operation */

// MODE1 flags
#define MODE1_BR8			0x1			/* Bit-reverse for I8 */
#define MODE1_BR0			0x2			/* Bit-reverse for I0 */
#define MODE1_SRCU			0x4			/* Alternate register select for computational units */
#define MODE1_SRD1H			0x8			/* DAG alternate register select (7-4) */
#define MODE1_SRD1L			0x10		/* DAG alternate register select (3-0) */
#define MODE1_SRD2H			0x20		/* DAG alternate register select (15-12) */
#define MODE1_SRD2L			0x40		/* DAG alternate register select (11-8) */
#define MODE1_SRRFH			0x80		/* Register file alternate select for R(15-8) */
#define MODE1_SRRFL			0x400		/* Register file alternate select for R(7-0) */
#define MODE1_NESTM			0x800		/* Interrupt nesting enable */
#define MODE1_IRPTEN		0x1000		/* Global interrupt enable */
#define MODE1_ALUSAT		0x2000		/* Enable ALU fixed-point saturation */
#define MODE1_SSE			0x4000		/* Enable short word sign extension */
#define MODE1_TRUNCATE		0x8000		/* (1) Floating-point truncation / (0) round to nearest */
#define MODE1_RND32			0x10000		/* (1) 32-bit floating-point rounding / (0) 40-bit rounding */
#define MODE1_CSEL			0x60000		/* CSelect */

// MODE2 flags
#define MODE2_IRQ0E			0x1			/* IRQ0 (1) Edge sens. / (0) Level sens. */
#define MODE2_IRQ1E			0x2			/* IRQ1 (1) Edge sens. / (0) Level sens. */
#define MODE2_IRQ2E			0x4			/* IRQ2 (1) Edge sens. / (0) Level sens. */
#define MODE2_CADIS			0x10		/* Cache disable */
#define MODE2_TIMEN			0x20		/* Timer enable */
#define MODE2_BUSLK			0x40		/* External bus lock */
#define MODE2_FLG0O			0x8000		/* FLAG0 (1) Output / (0) Input */
#define MODE2_FLG1O			0x10000		/* FLAG1 (1) Output / (0) Input */
#define MODE2_FLG2O			0x20000		/* FLAG2 (1) Output / (0) Input */
#define MODE2_FLG3O			0x40000		/* FLAG3 (1) Output / (0) Input */
#define MODE2_CAFRZ			0x80000		/* Cache freeze */


#define REG_PC				0x63
#define REG_PCSTK			0x64
#define REG_PCSTKP			0x65
#define REG_LADDR			0x66
#define REG_CURLCNTR		0x67
#define REG_LCNTR			0x68
#define REG_USTAT1			0x70
#define REG_USTAT2			0x71
#define REG_IRPTL			0x79
#define REG_MODE2			0x7a
#define REG_MODE1			0x7b
#define REG_ASTAT			0x7c
#define REG_IMASK			0x7d
#define REG_STKY			0x7e
#define REG_IMASKP			0x7f



#define REG(x)		(sharc.r[x].r)
#define FREG(x)		(sharc.r[x].f)

#define UPDATE_CIRCULAR_BUFFER_PM(x)						\
	{														\
		if (PM_REG_L(x) != 0)								\
		{													\
			if (PM_REG_I(x) > PM_REG_B(x)+PM_REG_L(x))		\
			{												\
				PM_REG_I(x) -= PM_REG_L(x);					\
			}												\
			else if (PM_REG_I(x) < PM_REG_B(x))				\
			{												\
				PM_REG_I(x) += PM_REG_L(x);					\
			}												\
		}													\
	}

#define UPDATE_CIRCULAR_BUFFER_DM(x)						\
	{														\
		if (DM_REG_L(x) != 0)								\
		{													\
			if (DM_REG_I(x) > DM_REG_B(x)+DM_REG_L(x))		\
			{												\
				DM_REG_I(x) -= DM_REG_L(x);					\
			}												\
			else if (DM_REG_I(x) < DM_REG_B(x))				\
			{												\
				DM_REG_I(x) += DM_REG_L(x);					\
			}												\
		}													\
	}


/*****************************************************************************/

static void systemreg_write_latency_effect(void);

static void add_systemreg_write_latency_effect(int sysreg, UINT32 data, UINT32 prev_data)
{
	if (sharc.systemreg_latency_cycles > 0)
	{
		//fatalerror("SHARC: add_systemreg_write_latency_effect: already scheduled! (reg: %02X, data: %08X, PC: %08X)\n", systemreg_latency_reg, systemreg_latency_data, sharc.pc);
		systemreg_write_latency_effect();
	}

	sharc.systemreg_latency_cycles = 2;
	sharc.systemreg_latency_reg = sysreg;
	sharc.systemreg_latency_data = data;
	sharc.systemreg_previous_data = prev_data;
}

INLINE void swap_register(UINT32 *a, UINT32 *b)
{
	UINT32 temp = *a;
	*a = *b;
	*b = temp;
}

static void systemreg_write_latency_effect(void)
{
	int i;
	UINT32 data = sharc.systemreg_latency_data;
	UINT32 old_data = sharc.systemreg_previous_data;

	switch(sharc.systemreg_latency_reg)
	{
		case 0xb:	/* MODE1 */
		{
			UINT32 oldreg = old_data;
			sharc.mode1 = data;

			if ((data & 0x1) != (oldreg & 0x1))
			{
				fatalerror("SHARC: systemreg_latency_op: enable I8 bit-reversing");
			}
			if ((data & 0x2) != (oldreg & 0x2))
			{
				fatalerror("SHARC: systemreg_latency_op: enable I0 bit-reversing");
			}
			if ((data & 0x4) != (oldreg & 0x4))
			{
				fatalerror("SHARC: systemreg_latency_op: enable MR alternate");
			}

			if ((data & 0x8) != (oldreg & 0x8))			/* Switch DAG1 7-4 */
			{
				swap_register(&sharc.dag1.i[4], &sharc.dag1_alt.i[4]);
				swap_register(&sharc.dag1.i[5], &sharc.dag1_alt.i[5]);
				swap_register(&sharc.dag1.i[6], &sharc.dag1_alt.i[6]);
				swap_register(&sharc.dag1.i[7], &sharc.dag1_alt.i[7]);
				swap_register(&sharc.dag1.m[4], &sharc.dag1_alt.m[4]);
				swap_register(&sharc.dag1.m[5], &sharc.dag1_alt.m[5]);
				swap_register(&sharc.dag1.m[6], &sharc.dag1_alt.m[6]);
				swap_register(&sharc.dag1.m[7], &sharc.dag1_alt.m[7]);
				swap_register(&sharc.dag1.l[4], &sharc.dag1_alt.l[4]);
				swap_register(&sharc.dag1.l[5], &sharc.dag1_alt.l[5]);
				swap_register(&sharc.dag1.l[6], &sharc.dag1_alt.l[6]);
				swap_register(&sharc.dag1.l[7], &sharc.dag1_alt.l[7]);
				swap_register(&sharc.dag1.b[4], &sharc.dag1_alt.b[4]);
				swap_register(&sharc.dag1.b[5], &sharc.dag1_alt.b[5]);
				swap_register(&sharc.dag1.b[6], &sharc.dag1_alt.b[6]);
				swap_register(&sharc.dag1.b[7], &sharc.dag1_alt.b[7]);
			}
			if ((data & 0x10) != (oldreg & 0x10))		/* Switch DAG1 3-0 */
			{
				swap_register(&sharc.dag1.i[0], &sharc.dag1_alt.i[0]);
				swap_register(&sharc.dag1.i[1], &sharc.dag1_alt.i[1]);
				swap_register(&sharc.dag1.i[2], &sharc.dag1_alt.i[2]);
				swap_register(&sharc.dag1.i[3], &sharc.dag1_alt.i[3]);
				swap_register(&sharc.dag1.m[0], &sharc.dag1_alt.m[0]);
				swap_register(&sharc.dag1.m[1], &sharc.dag1_alt.m[1]);
				swap_register(&sharc.dag1.m[2], &sharc.dag1_alt.m[2]);
				swap_register(&sharc.dag1.m[3], &sharc.dag1_alt.m[3]);
				swap_register(&sharc.dag1.l[0], &sharc.dag1_alt.l[0]);
				swap_register(&sharc.dag1.l[1], &sharc.dag1_alt.l[1]);
				swap_register(&sharc.dag1.l[2], &sharc.dag1_alt.l[2]);
				swap_register(&sharc.dag1.l[3], &sharc.dag1_alt.l[3]);
				swap_register(&sharc.dag1.b[0], &sharc.dag1_alt.b[0]);
				swap_register(&sharc.dag1.b[1], &sharc.dag1_alt.b[1]);
				swap_register(&sharc.dag1.b[2], &sharc.dag1_alt.b[2]);
				swap_register(&sharc.dag1.b[3], &sharc.dag1_alt.b[3]);
			}
			if ((data & 0x20) != (oldreg & 0x20))		/* Switch DAG2 15-12 */
			{
				swap_register(&sharc.dag2.i[4], &sharc.dag2_alt.i[4]);
				swap_register(&sharc.dag2.i[5], &sharc.dag2_alt.i[5]);
				swap_register(&sharc.dag2.i[6], &sharc.dag2_alt.i[6]);
				swap_register(&sharc.dag2.i[7], &sharc.dag2_alt.i[7]);
				swap_register(&sharc.dag2.m[4], &sharc.dag2_alt.m[4]);
				swap_register(&sharc.dag2.m[5], &sharc.dag2_alt.m[5]);
				swap_register(&sharc.dag2.m[6], &sharc.dag2_alt.m[6]);
				swap_register(&sharc.dag2.m[7], &sharc.dag2_alt.m[7]);
				swap_register(&sharc.dag2.l[4], &sharc.dag2_alt.l[4]);
				swap_register(&sharc.dag2.l[5], &sharc.dag2_alt.l[5]);
				swap_register(&sharc.dag2.l[6], &sharc.dag2_alt.l[6]);
				swap_register(&sharc.dag2.l[7], &sharc.dag2_alt.l[7]);
				swap_register(&sharc.dag2.b[4], &sharc.dag2_alt.b[4]);
				swap_register(&sharc.dag2.b[5], &sharc.dag2_alt.b[5]);
				swap_register(&sharc.dag2.b[6], &sharc.dag2_alt.b[6]);
				swap_register(&sharc.dag2.b[7], &sharc.dag2_alt.b[7]);
			}
			if ((data & 0x40) != (oldreg & 0x40))		/* Switch DAG2 11-8 */
			{
				swap_register(&sharc.dag2.i[0], &sharc.dag2_alt.i[0]);
				swap_register(&sharc.dag2.i[1], &sharc.dag2_alt.i[1]);
				swap_register(&sharc.dag2.i[2], &sharc.dag2_alt.i[2]);
				swap_register(&sharc.dag2.i[3], &sharc.dag2_alt.i[3]);
				swap_register(&sharc.dag2.m[0], &sharc.dag2_alt.m[0]);
				swap_register(&sharc.dag2.m[1], &sharc.dag2_alt.m[1]);
				swap_register(&sharc.dag2.m[2], &sharc.dag2_alt.m[2]);
				swap_register(&sharc.dag2.m[3], &sharc.dag2_alt.m[3]);
				swap_register(&sharc.dag2.l[0], &sharc.dag2_alt.l[0]);
				swap_register(&sharc.dag2.l[1], &sharc.dag2_alt.l[1]);
				swap_register(&sharc.dag2.l[2], &sharc.dag2_alt.l[2]);
				swap_register(&sharc.dag2.l[3], &sharc.dag2_alt.l[3]);
				swap_register(&sharc.dag2.b[0], &sharc.dag2_alt.b[0]);
				swap_register(&sharc.dag2.b[1], &sharc.dag2_alt.b[1]);
				swap_register(&sharc.dag2.b[2], &sharc.dag2_alt.b[2]);
				swap_register(&sharc.dag2.b[3], &sharc.dag2_alt.b[3]);
			}
			if ((data & 0x80) != (oldreg & 0x80))
			{
				for (i=8; i<16; i++)
					swap_register((UINT32*)&sharc.r[i].r, (UINT32*)&sharc.reg_alt[i].r);
			}
			if ((data & 0x400) != (oldreg & 0x400))
			{
				for (i=0; i<8; i++)
					swap_register((UINT32*)&sharc.r[i].r, (UINT32*)&sharc.reg_alt[i].r);
			}
			break;
		}
		default:	fatalerror("SHARC: systemreg_latency_op: unknown register %02X at %08X", sharc.systemreg_latency_reg, sharc.pc);
	}

	sharc.systemreg_latency_reg = -1;
}

static UINT32 GET_UREG(int ureg)
{
	int reg = ureg & 0xf;
	switch((ureg >> 4) & 0xf)
	{
		case 0x0:		/* R0 - R15 */
		{
			return sharc.r[reg].r;
		}

		case 0x1:
		{
			if (reg & 0x8)		/* I8 - I15 */
			{
				return sharc.dag2.i[reg & 0x7];
			}
			else 				/* I0 - I7 */
			{
				return sharc.dag1.i[reg & 0x7];
			}
		}

		case 0x2:
		{
			if (reg & 0x8)		/* M8 - M15 */
			{
				INT32 r = sharc.dag2.m[reg & 0x7];
				if (r & 0x800000)	r |= 0xff000000;

				return r;
			}
			else				/* M0 - M7 */
			{
				return sharc.dag1.m[reg & 0x7];
			}
		}

		case 0x3:
		{
			if (reg & 0x8)		/* L8 - L15 */
			{
				return sharc.dag2.l[reg & 0x7];
			}
			else				/* L0 - L7 */
			{
				return sharc.dag1.l[reg & 0x7];
			}
		}

		case 0x4:
		{
			if (reg & 0x8)		/* B8 - B15 */
			{
				return sharc.dag2.b[reg & 0x7];
			}
			else				/* B0 - B7 */
			{
				return sharc.dag1.b[reg & 0x7];
			}
		}

		case 0x6:
		{
			switch(reg)
			{
				case 0x4:	return sharc.pcstack[sharc.pcstkp];		/* PCSTK */
				default:	fatalerror("SHARC: GET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
			}
			break;
		}

		case 0x7:
		{
			switch(reg)
			{
				case 0x0:	return sharc.ustat1;		/* USTAT1 */
				case 0x1:	return sharc.ustat2;		/* USTAT2 */
				case 0x9:	return sharc.irptl;			/* IRPTL */
				case 0xa:	return sharc.mode2;			/* MODE2 */
				case 0xb:	return sharc.mode1;			/* MODE1 */
				case 0xc:								/* ASTAT */
				{
					UINT32 r = sharc.astat;
					r &= ~0x00780000;
					r |= (sharc.flag[0] << 19);
					r |= (sharc.flag[1] << 20);
					r |= (sharc.flag[2] << 21);
					r |= (sharc.flag[3] << 22);
					return r;
				}
				case 0xd:	return sharc.imask;			/* IMASK */
				case 0xe:	return sharc.stky;			/* STKY */
				default:	fatalerror("SHARC: GET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
			}
			break;
		}

		case 0xd:
		{
			switch(reg)
			{
				/* PX needs to be handled separately if the whole 48 bits are needed */
				case 0xb:	return (UINT32)(sharc.px);			/* PX */
				case 0xc:	return (UINT16)(sharc.px);			/* PX1 */
				case 0xd:	return (UINT32)(sharc.px >> 16);	/* PX2 */
				default:	fatalerror("SHARC: GET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
			}
			break;
		}

		default:			fatalerror("SHARC: GET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
	}
}

static void SET_UREG(int ureg, UINT32 data)
{
	int reg = ureg & 0xf;
	switch((ureg >> 4) & 0xf)
	{
		case 0x0:		/* R0 - R15 */
			sharc.r[reg].r = data;
			break;

		case 0x1:
			if (reg & 0x8)		/* I8 - I15 */
			{
				sharc.dag2.i[reg & 0x7] = data;
			}
			else				/* I0 - I7 */
			{
				sharc.dag1.i[reg & 0x7] = data;
			}
			break;

		case 0x2:
			if (reg & 0x8)		/* M8 - M15 */
			{
				sharc.dag2.m[reg & 0x7] = data;
			}
			else				/* M0 - M7 */
			{
				sharc.dag1.m[reg & 0x7] = data;
			}
			break;

		case 0x3:
			if (reg & 0x8)		/* L8 - L15 */
			{
				sharc.dag2.l[reg & 0x7] = data;
			}
			else				/* L0 - L7 */
			{
				sharc.dag1.l[reg & 0x7] = data;
			}
			break;

		case 0x4:
			// Note: loading B also loads the same value in I
			if (reg & 0x8)		/* B8 - B15 */
			{
				sharc.dag2.b[reg & 0x7] = data;
				sharc.dag2.i[reg & 0x7] = data;
			}
			else				/* B0 - B7 */
			{
				sharc.dag1.b[reg & 0x7] = data;
				sharc.dag1.i[reg & 0x7] = data;
			}
			break;

		case 0x6:
			switch (reg)
			{
				case 0x5:	sharc.pcstkp = data; break;		/* PCSTKP */
				case 0x8:	sharc.lcntr = data; break;		/* LCNTR */
				default:	fatalerror("SHARC: SET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
			}
			break;

		case 0x7:		/* system regs */
			switch(reg)
			{
				case 0x0:	sharc.ustat1 = data; break;		/* USTAT1 */
				case 0x1:	sharc.ustat2 = data; break;		/* USTAT2 */

				case 0x9:	sharc.irptl = data; break;		/* IRPTL */
				case 0xa:	sharc.mode2 = data; break;		/* MODE2 */

				case 0xb:									/* MODE1 */
				{
					add_systemreg_write_latency_effect(reg, data, sharc.mode1);
					sharc.mode1 = data;
					break;
				}

				case 0xc:	sharc.astat = data; break;		/* ASTAT */

				case 0xd:									/* IMASK */
				{
					check_interrupts();
					sharc.imask = data;
					break;
				}

				case 0xe:	sharc.stky = data; break;		/* STKY */
				default:	fatalerror("SHARC: SET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
			}
			break;

		case 0xd:
			switch(reg)
			{
				case 0xc:	sharc.px &= U64(0xffffffffffff0000); sharc.px |= (data & 0xffff); break;		/* PX1 */
				case 0xd:	sharc.px &= U64(0x000000000000ffff); sharc.px |= (UINT64)data << 16; break;		/* PX2 */
				default:	fatalerror("SHARC: SET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
			}
			break;

		default:			fatalerror("SHARC: SET_UREG: unknown register %08X at %08X", ureg, sharc.pc);
	}
}

/*****************************************************************************/
#define SET_FLAG_SV_LSHIFT(x, shift)	if((x) & ((UINT32)0xffffffff << shift)) sharc.astat |= SV
#define SET_FLAG_SV_RSHIFT(x, shift)	if((x) & ((UINT32)0xffffffff >> shift)) sharc.astat |= SV

#define SET_FLAG_SZ(x)					if((x) == 0) sharc.astat |= SZ

#define MAKE_EXTRACT_MASK(start_bit, length)	((0xffffffff << start_bit) & (((UINT32)0xffffffff) >> (32 - (start_bit + length))))

static void SHIFT_OPERATION_IMM(int shiftop, int data, int rn, int rx)
{
	INT8 shift = data & 0xff;
	int bit = data & 0x3f;
	int len = (data >> 6) & 0x3f;

	sharc.astat &= ~(SZ|SV|SS);

	switch(shiftop)
	{
		case 0x00:		/* LSHIFT Rx BY <data8>*/
		{
			if(shift < 0) {
				REG(rn) = (shift > -32 ) ? (REG(rx) >> -shift) : 0;
			} else {
				REG(rn) = (shift < 32) ? (REG(rx) << shift) : 0;
				if (shift > 0)
				{
					sharc.astat |= SV;
				}
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x01:		/* ASHIFT Rx BY <data8> */
		{
			if (shift < 0)
			{
				REG(rn) = (shift > -32) ? ((INT32)REG(rx) >> -shift) : ((REG(rx) & 0x80000000) ? 0xffffffff : 0);
			}
			else
			{
				REG(rn) = (shift < 32) ? ((INT32)REG(rx) << shift) : 0;
				if (shift > 0)
				{
					sharc.astat |= SV;
				}
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x02:		/* ROT Rx BY <data8> */
		{
			if (shift < 0)
			{
				int s = (-shift) & 0x1f;
				REG(rn) = (((UINT32)REG(rx) >> s) & ((UINT32)(0xffffffff) >> s)) |
							  (((UINT32)REG(rx) << (32-s)) & ((UINT32)(0xffffffff) << (32-s)));
			}
			else
			{
				int s = shift & 0x1f;
				REG(rn) = (((UINT32)REG(rx) << s) & ((UINT32)(0xffffffff) << s)) |
							  (((UINT32)REG(rx) >> (32-s)) & ((UINT32)(0xffffffff) >> (32-s)));
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x08:		/* Rn = Rn OR LSHIFT Rx BY <data8> */
		{
			UINT32 r = 0;
			if(shift < 0) {
				r = (shift > -32 ) ? (REG(rx) >> -shift) : 0;
			} else {
				r = (shift < 32) ? (REG(rx) << shift) : 0;
				if (shift > 0)
				{
					sharc.astat |= SV;
				}
			}
			SET_FLAG_SZ(r);

			REG(rn) = REG(rn) | r;
			break;
		}

		case 0x10:		/* FEXT Rx BY <bit6>:<len6> */
		{
			UINT32 ext = REG(rx) & MAKE_EXTRACT_MASK(bit, len);
			REG(rn) = ext >> bit;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				sharc.astat |= SV;
			}
			break;
		}

		case 0x12:		/* FEXT Rx BY <bit6>:<len6> (Sign Extended) */
		{
			UINT32 ext = (REG(rx) & MAKE_EXTRACT_MASK(bit, len)) >> bit;
			if (ext & (1 << (len-1))) {
				ext |= (UINT32)0xffffffff << (len-1);
			}
			REG(rn) = ext;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				sharc.astat |= SV;
			}
			break;
		}

		case 0x13:		/* FDEP Rx BY Ry <bit6>:<len6> (Sign Extended) */
		{
			UINT32 ext = REG(rx) & MAKE_EXTRACT_MASK(0, len);
			if (ext & (1 << (len-1))) {
				ext |= (UINT32)0xffffffff << (len-1);
			}
			REG(rn) = ext << bit;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				sharc.astat |= SV;
			}
			break;
		}

		case 0x19:		/* Rn = Rn OR FDEP Rx BY <bit6>:<len6> */
		{
			UINT32 ext = REG(rx) & MAKE_EXTRACT_MASK(0, len);

			REG(rn) |= ext << bit;

			SET_FLAG_SZ(REG(rn));
			if (bit+len > 32)
			{
				sharc.astat |= SV;
			}
			break;
		}

		case 0x30:		/* BSET Rx BY <data8> */
		{
			REG(rn) = REG(rx);
			if (data >= 0 && data < 32)
			{
				REG(rn) |= (1 << data);
			}
			else
			{
				sharc.astat |= SV;
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x31:		/* BCLR Rx BY <data8> */
		{
			REG(rn) = REG(rx);
			if (data >= 0 && data < 32)
			{
				REG(rn) &= ~(1 << data);
			}
			else
			{
				sharc.astat |= SV;
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x32:		/* BTGL Rx BY <data8> */
		{
			REG(rn) = REG(rx);
			if (data >= 0 && data < 32)
			{
				REG(rn) ^= (1 << data);
			}
			else
			{
				sharc.astat |= SV;
			}
			SET_FLAG_SZ(REG(rn));
			break;
		}

		case 0x33:		/* BTST Rx BY <data8> */
		{
			if (data < 32)
			{
				UINT32 r = REG(rx) & (1 << data);

				SET_FLAG_SZ(r);
			}
			else
			{
				sharc.astat |= SZ | SV;
			}
			break;
		}

		default:	fatalerror("SHARC: unimplemented shift operation %02X at %08X", shiftop, sharc.pc);
	}
}

#include "compute.c"

static void COMPUTE(UINT32 opcode)
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

	if(opcode & 0x400000)		/* Multi-function opcode */
	{
		int fm = (opcode >> 12) & 0xf;
		int fa = (opcode >> 8) & 0xf;
		int fxm = (opcode >> 6) & 0x3;			// registers 0 - 3
		int fym = ((opcode >> 4) & 0x3) + 4;	// registers 4 - 7
		int fxa = ((opcode >> 2) & 0x3) + 8;	// registers 8 - 11
		int fya = (opcode & 0x3) + 12;			// registers 12 - 15

		multiop = (opcode >> 16) & 0x3f;
		switch(multiop)
		{
			case 0x00:		compute_multi_mr_to_reg(op & 0xf, rn); break;
			case 0x01:		compute_multi_reg_to_mr(op & 0xf, rn); break;

			case 0x04:		/* Rm = Rxm * Rym (SSFR),   Ra = Rxa + Rya */
			{
				compute_mul_ssfr_add(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x05:		/* Rm = Rxm * Rym (SSFR),   Ra = Rxa - Rya */
			{
				compute_mul_ssfr_sub(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x18:		/* Fm = Fxm * Fym,   Fa = Fxa + Fya */
			{
				compute_fmul_fadd(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x19:		/* Fm = Fxm * Fym,   Fa = Fxa - Fya */
			{
				compute_fmul_fsub(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1a:		/* Fm = Fxm * Fym,   Fa = FLOAT Fxa BY Fya */
			{
				compute_fmul_float_scaled(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1b:		/* Fm = Fxm * Fym,   Fa = FIX Fxa BY Fya */
			{
				compute_fmul_fix_scaled(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1e:		/* Fm = Fxm * Fym,   Fa = MAX(Fxa, Fya) */
			{
				compute_fmul_fmax(fm, fxm, fym, fa, fxa, fya);
				break;
			}

			case 0x1f:		/* Fm = Fxm * Fym,   Fa = MIN(Fxa, Fya) */
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
				fatalerror("SHARC: compute: multi-function opcode %02X not implemented ! (%08X, %08X)", multiop, sharc.pc, opcode);
				break;
		}
	}
	else						/* Single-function opcode */
	{
		switch(cu)
		{
			/* ALU operations */
			case 0:
			{
				switch(op)
				{
					case 0x01:		compute_add(rn, rx, ry); break;
					case 0x02:		compute_sub(rn, rx, ry); break;
					case 0x05:		compute_add_ci(rn, rx, ry); break;
					case 0x06:		compute_sub_ci(rn, rx, ry); break;
					case 0x0a:		compute_comp(rx, ry); break;
					case 0x21:		compute_pass(rn, rx); break;
					case 0x22:		compute_neg(rn, rx); break;
					case 0x29:		compute_inc(rn, rx); break;
					case 0x2a:		compute_dec(rn, rx); break;
					case 0x40:		compute_and(rn, rx, ry); break;
					case 0x41:		compute_or(rn, rx, ry); break;
					case 0x42:		compute_xor(rn, rx, ry); break;
					case 0x43:		compute_not(rn, rx); break;
					case 0x61:		compute_min(rn, rx, ry); break;
					case 0x62:		compute_max(rn, rx, ry); break;
					case 0x81:		compute_fadd(rn, rx, ry); break;
					case 0x82:		compute_fsub(rn, rx, ry); break;
					case 0x8a:		compute_fcomp(rx, ry); break;
					case 0x91:		compute_fabs_plus(rn, rx, ry); break;
					case 0xa1:		compute_fpass(rn, rx); break;
					case 0xa2:		compute_fneg(rn, rx); break;
					case 0xb0:		compute_fabs(rn, rx); break;
					case 0xbd:		compute_scalb(rn, rx, ry); break;
					case 0xc1:		compute_logb(rn, rx); break;
					case 0xc4:		compute_recips(rn, rx); break;
					case 0xc5:		compute_rsqrts(rn, rx); break;
					case 0xc9:		compute_fix(rn, rx); break;
					case 0xca:		compute_float(rn, rx); break;
					case 0xd9:		compute_fix_scaled(rn, rx, ry); break;
					case 0xda:		compute_float_scaled(rn, rx, ry); break;
					case 0xe1:		compute_fmin(rn, rx, ry); break;
					case 0xe2:		compute_fmax(rn, rx, ry); break;
					case 0xe3:		compute_fclip(rn, rx, ry); break;

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

					default:		fatalerror("SHARC: compute: unimplemented ALU operation %02X (%08X, %08X)", op, sharc.pc, opcode);
				}
				break;
			}


			/* Multiplier operations */
			case 1:
			{
				switch(op)
				{
					case 0x14:		sharc.mrf = 0; break;
					case 0x16:		sharc.mrb = 0; break;

					case 0x30:		compute_fmul(rn, rx, ry); break;
					case 0x40:		compute_mul_uuin(rn, rx, ry); break;
					case 0x70:		compute_mul_ssin(rn, rx, ry); break;

					case 0xb0:		REG(rn) = compute_mrf_plus_mul_ssin(rx, ry); break;
					case 0xb2:		REG(rn) = compute_mrb_plus_mul_ssin(rx, ry); break;

					default:
						fatalerror("SHARC: compute: multiplier operation %02X not implemented ! (%08X, %08X)", op, sharc.pc, opcode);
						break;
				}
				break;
			}


			/* Shifter operations */
			case 2:
			{
				sharc.astat &= ~(SZ|SV|SS);

				op >>= 2;
				switch(op)
				{
					case 0x00:		/* LSHIFT Rx BY Ry*/
					{
						int shift = REG(ry);
						if(shift < 0)
						{
							REG(rn) = (shift > -32 ) ? (REG(rx) >> -shift) : 0;
						}
						else
						{
							REG(rn) = (shift < 32) ? (REG(rx) << shift) : 0;
							if (shift > 0)
							{
								sharc.astat |= SV;
							}
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x02:		/* ROT Rx BY Ry */
					{
						int shift = REG(ry);
						if (shift < 0)
						{
							int s = (-shift) & 0x1f;
							REG(rn) = (((UINT32)REG(rx) >> s) & ((UINT32)(0xffffffff) >> s)) |
									  (((UINT32)REG(rx) << (32-s)) & ((UINT32)(0xffffffff) << (32-s)));
						}
						else
						{
							int s = shift & 0x1f;
							REG(rn) = (((UINT32)REG(rx) << s) & ((UINT32)(0xffffffff) << s)) |
									  (((UINT32)REG(rx) >> (32-s)) & ((UINT32)(0xffffffff) >> (32-s)));
							if (shift > 0)
							{
								sharc.astat |= SV;
							}
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x08:		/* Rn = Rn OR LSHIFT Rx BY Ry*/
					{
						INT8 shift = REG(ry);
						if(shift < 0) {
							REG(rn) = REG(rn) | ((shift > -32 ) ? (REG(rx) >> -shift) : 0);
						} else {
							REG(rn) = REG(rn) | ((shift < 32) ? (REG(rx) << shift) : 0);
							if (shift > 0)
							{
								sharc.astat |= SV;
							}
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x10:		/* FEXT Rx BY Ry */
					{
						int bit = REG(ry) & 0x3f;
						int len = (REG(ry) >> 6) & 0x3f;
						UINT32 ext = REG(rx) & MAKE_EXTRACT_MASK(bit, len);
						REG(rn) = ext >> bit;

						SET_FLAG_SZ(REG(rn));
						if (bit+len > 32)
						{
							sharc.astat |= SV;
						}
						break;
					}

					case 0x12:		/* FEXT Rx BY Ry (Sign Extended) */
					{
						int bit = REG(ry) & 0x3f;
						int len = (REG(ry) >> 6) & 0x3f;
						UINT32 ext = (REG(rx) & MAKE_EXTRACT_MASK(bit, len)) >> bit;
						if (ext & (1 << (len-1))) {
							ext |= (UINT32)0xffffffff << (len-1);
						}
						REG(rn) = ext;

						SET_FLAG_SZ(REG(rn));
						if (bit+len > 32)
						{
							sharc.astat |= SV;
						}
						break;
					}

					case 0x19:		/* Rn = Rn OR FDEP Rx BY Ry */
					{
						int bit = REG(ry) & 0x3f;
						int len = (REG(ry) >> 6) & 0x3f;
						UINT32 ext = REG(rx) & MAKE_EXTRACT_MASK(0, len);

						REG(rn) |= ext << bit;

						SET_FLAG_SZ(REG(rn));
						if (bit+len > 32)
						{
							sharc.astat |= SV;
						}
						break;
					}

					case 0x30:		/* BSET Rx BY Ry */
					{
						UINT32 shift = REG(ry);
						REG(rn) = REG(rx);
						if (shift >= 0 && shift < 32)
						{
							REG(rn) |= (1 << shift);
						}
						else
						{
							sharc.astat |= SV;
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x31:		/* BCLR Rx BY Ry */
					{
						UINT32 shift = REG(ry);
						REG(rn) = REG(rx);
						if (shift >= 0 && shift < 32)
						{
							REG(rn) &= ~(1 << shift);
						}
						else
						{
							sharc.astat |= SV;
						}
						SET_FLAG_SZ(REG(rn));
						break;
					}

					case 0x33:		/* BTST Rx BY Ry */
					{
						UINT32 shift = REG(ry);
						if (shift < 32)
						{
							UINT32 r = REG(rx) & (1 << shift);

							SET_FLAG_SZ(r);
						}
						else
						{
							sharc.astat |= SZ | SV;
						}
						break;
					}

					default:
						fatalerror("SHARC: compute: shift operation %02X not implemented ! (%08X, %08X)", op, sharc.pc, opcode);
				}
				break;
			}

			default:
				fatalerror("SHARC: compute: invalid single-function operation %02X", cu);
		}
	}
}

INLINE void PUSH_PC(UINT32 pc)
{
	sharc.pcstkp++;
	if(sharc.pcstkp >= 32)
	{
		fatalerror("SHARC: PC Stack overflow !");
	}

	if (sharc.pcstkp == 0)
	{
		sharc.stky |= 0x400000;
	}
	else
	{
		sharc.stky &= ~0x400000;
	}

	sharc.pcstk = pc;
	sharc.pcstack[sharc.pcstkp] = pc;
}

INLINE UINT32 POP_PC(void)
{
	sharc.pcstk = sharc.pcstack[sharc.pcstkp];

	if(sharc.pcstkp == 0)
	{
		fatalerror("SHARC: PC Stack underflow !");
	}

	sharc.pcstkp--;

	if (sharc.pcstkp == 0)
	{
		sharc.stky |= 0x400000;
	}
	else
	{
		sharc.stky &= ~0x400000;
	}

	return sharc.pcstk;
}

INLINE UINT32 TOP_PC(void)
{
	return sharc.pcstack[sharc.pcstkp];
}

INLINE void PUSH_LOOP(UINT32 pc, UINT32 count)
{
	sharc.lstkp++;
	if(sharc.lstkp >= 6)
	{
		fatalerror("SHARC: Loop Stack overflow !");
	}

	if (sharc.lstkp == 0)
	{
		sharc.stky |= 0x4000000;
	}
	else
	{
		sharc.stky &= ~0x4000000;
	}

	sharc.lcstack[sharc.lstkp] = count;
	sharc.lastack[sharc.lstkp] = pc;
	sharc.curlcntr = count;
	sharc.laddr = pc;
}

INLINE void POP_LOOP(void)
{
	if(sharc.lstkp == 0)
	{
		fatalerror("SHARC: Loop Stack underflow !");
	}

	sharc.lstkp--;

	if (sharc.lstkp == 0)
	{
		sharc.stky |= 0x4000000;
	}
	else
	{
		sharc.stky &= ~0x4000000;
	}

	sharc.curlcntr = sharc.lcstack[sharc.lstkp];
	sharc.laddr = sharc.lastack[sharc.lstkp];
}

INLINE void PUSH_STATUS_STACK(void)
{
	sharc.status_stkp++;
	if (sharc.status_stkp >= 5)
	{
		fatalerror("SHARC: Status stack overflow !");
	}

	if (sharc.status_stkp == 0)
	{
		sharc.stky |= 0x1000000;
	}
	else
	{
		sharc.stky &= ~0x1000000;
	}

	sharc.status_stack[sharc.status_stkp].mode1 = GET_UREG(REG_MODE1);
	sharc.status_stack[sharc.status_stkp].astat = GET_UREG(REG_ASTAT);
}

INLINE void POP_STATUS_STACK(void)
{
	SET_UREG(REG_MODE1, sharc.status_stack[sharc.status_stkp].mode1);
	SET_UREG(REG_ASTAT, sharc.status_stack[sharc.status_stkp].astat);

	sharc.status_stkp--;
	if (sharc.status_stkp < 0)
	{
		fatalerror("SHARC: Status stack underflow !");
	}

	if (sharc.status_stkp == 0)
	{
		sharc.stky |= 0x1000000;
	}
	else
	{
		sharc.stky &= ~0x1000000;
	}
}

INLINE int IF_CONDITION_CODE(int cond)
{
	switch(cond)
	{
		case 0x00:	return sharc.astat & AZ;		/* EQ */
		case 0x01:	return !(sharc.astat & AZ) && (sharc.astat & AN);	/* LT */
		case 0x02:	return (sharc.astat & AZ) || (sharc.astat & AN);	/* LE */
		case 0x03:	return (sharc.astat & AC);		/* AC */
		case 0x04:	return (sharc.astat & AV);		/* AV */
		case 0x05:	return (sharc.astat & MV);		/* MV */
		case 0x06:	return (sharc.astat & MN);		/* MS */
		case 0x07:	return (sharc.astat & SV);		/* SV */
		case 0x08:	return (sharc.astat & SZ);		/* SZ */
		case 0x09:	return (sharc.flag[0] != 0);	/* FLAG0 */
		case 0x0a:	return (sharc.flag[1] != 0);	/* FLAG1 */
		case 0x0b:	return (sharc.flag[2] != 0);	/* FLAG2 */
		case 0x0c:	return (sharc.flag[3] != 0);	/* FLAG3 */
		case 0x0d:	return (sharc.astat & BTF);		/* TF */
		case 0x0e:	return 0;						/* BM */
		case 0x0f:	return (sharc.curlcntr!=1);		/* NOT LCE */
		case 0x10:	return !(sharc.astat & AZ);		/* NOT EQUAL */
		case 0x11:	return (sharc.astat & AZ) || !(sharc.astat & AN);	/* GE */
		case 0x12:	return !(sharc.astat & AZ) && !(sharc.astat & AN);	/* GT */
		case 0x13:	return !(sharc.astat & AC);		/* NOT AC */
		case 0x14:	return !(sharc.astat & AV);		/* NOT AV */
		case 0x15:	return !(sharc.astat & MV);		/* NOT MV */
		case 0x16:	return !(sharc.astat & MN);		/* NOT MS */
		case 0x17:	return !(sharc.astat & SV);		/* NOT SV */
		case 0x18:	return !(sharc.astat & SZ);		/* NOT SZ */
		case 0x19:	return (sharc.flag[0] == 0);	/* NOT FLAG0 */
		case 0x1a:	return (sharc.flag[1] == 0);	/* NOT FLAG1 */
		case 0x1b:	return (sharc.flag[2] == 0);	/* NOT FLAG2 */
		case 0x1c:	return (sharc.flag[3] == 0);	/* NOT FLAG3 */
		case 0x1d:	return !(sharc.astat & BTF);	/* NOT TF */
		case 0x1e:	return 1;						/* NOT BM */
		case 0x1f:	return 1;						/* TRUE */
	}
	return 1;
}

INLINE int DO_CONDITION_CODE(int cond)
{
	switch(cond)
	{
		case 0x00:	return sharc.astat & AZ;		/* EQ */
		case 0x01:	return !(sharc.astat & AZ) && (sharc.astat & AN);	/* LT */
		case 0x02:	return (sharc.astat & AZ) || (sharc.astat & AN);	/* LE */
		case 0x03:	return (sharc.astat & AC);		/* AC */
		case 0x04:	return (sharc.astat & AV);		/* AV */
		case 0x05:	return (sharc.astat & MV);		/* MV */
		case 0x06:	return (sharc.astat & MN);		/* MS */
		case 0x07:	return (sharc.astat & SV);		/* SV */
		case 0x08:	return (sharc.astat & SZ);		/* SZ */
		case 0x09:	return (sharc.flag[0] != 0);	/* FLAG0 */
		case 0x0a:	return (sharc.flag[1] != 0);	/* FLAG1 */
		case 0x0b:	return (sharc.flag[2] != 0);	/* FLAG2 */
		case 0x0c:	return (sharc.flag[3] != 0);	/* FLAG3 */
		case 0x0d:	return (sharc.astat & BTF);		/* TF */
		case 0x0e:	return 0;						/* BM */
		case 0x0f:	return (sharc.curlcntr==1);		/* LCE */
		case 0x10:	return !(sharc.astat & AZ);		/* NOT EQUAL */
		case 0x11:	return (sharc.astat & AZ) || !(sharc.astat & AN);	/* GE */
		case 0x12:	return !(sharc.astat & AZ) && !(sharc.astat & AN);	/* GT */
		case 0x13:	return !(sharc.astat & AC);		/* NOT AC */
		case 0x14:	return !(sharc.astat & AV);		/* NOT AV */
		case 0x15:	return !(sharc.astat & MV);		/* NOT MV */
		case 0x16:	return !(sharc.astat & MN);		/* NOT MS */
		case 0x17:	return !(sharc.astat & SV);		/* NOT SV */
		case 0x18:	return !(sharc.astat & SZ);		/* NOT SZ */
		case 0x19:	return (sharc.flag[0] == 0);	/* NOT FLAG0 */
		case 0x1a:	return (sharc.flag[1] == 0);	/* NOT FLAG1 */
		case 0x1b:	return (sharc.flag[2] == 0);	/* NOT FLAG2 */
		case 0x1c:	return (sharc.flag[3] == 0);	/* NOT FLAG3 */
		case 0x1d:	return !(sharc.astat & BTF);	/* NOT TF */
		case 0x1e:	return 1;						/* NOT BM */
		case 0x1f:	return 0;						/* FALSE (FOREVER) */
	}
	return 1;
}

/*****************************************************************************/
/* | 001xxxxxx | */

/* compute / dreg <-> DM / dreg <-> PM */
static void sharcop_compute_dreg_dm_dreg_pm(void)
{
	int pm_dreg = (sharc.opcode >> 23) & 0xf;
	int pmm = (sharc.opcode >> 27) & 0x7;
	int pmi = (sharc.opcode >> 30) & 0x7;
	int dm_dreg = (sharc.opcode >> 33) & 0xf;
	int dmm = (sharc.opcode >> 38) & 0x7;
	int dmi = (sharc.opcode >> 41) & 0x7;
	int pmd = (sharc.opcode >> 37) & 0x1;
	int dmd = (sharc.opcode >> 44) & 0x1;
	int compute = sharc.opcode & 0x7fffff;

	/* due to parallelity issues, source DREGs must be saved */
	/* because the compute operation may change them */
	UINT32 parallel_pm_dreg = REG(pm_dreg);
	UINT32 parallel_dm_dreg = REG(dm_dreg);

	if (compute)
	{
		COMPUTE(compute);
	}

	if (pmd)		// dreg -> PM
	{
		pm_write32(PM_REG_I(pmi), parallel_pm_dreg);
		PM_REG_I(pmi) += PM_REG_M(pmm);
		UPDATE_CIRCULAR_BUFFER_PM(pmi);
	}
	else			// PM -> dreg
	{
		REG(pm_dreg) = pm_read32(PM_REG_I(pmi));
		PM_REG_I(pmi) += PM_REG_M(pmm);
		UPDATE_CIRCULAR_BUFFER_PM(pmi);
	}

	if (dmd)		// dreg -> DM
	{
		dm_write32(DM_REG_I(dmi), parallel_dm_dreg);
		DM_REG_I(dmi) += DM_REG_M(dmm);
		UPDATE_CIRCULAR_BUFFER_DM(dmi);
	}
	else			// DM -> dreg
	{
		REG(dm_dreg) = dm_read32(DM_REG_I(dmi));
		DM_REG_I(dmi) += DM_REG_M(dmm);
		UPDATE_CIRCULAR_BUFFER_DM(dmi);
	}
}

/*****************************************************************************/
/* | 00000001x | */

/* compute */
static void sharcop_compute(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond) && compute != 0)
	{
		COMPUTE(compute);
	}
}

/*****************************************************************************/
/* | 010xxxxxx | */

/* compute / ureg <-> DM|PM, pre-modify */
static void sharcop_compute_ureg_dmpm_premod(void)
{
	int i = (sharc.opcode >> 41) & 0x7;
	int m = (sharc.opcode >> 38) & 0x7;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int g = (sharc.opcode >> 32) & 0x1;
	int d = (sharc.opcode >> 31) & 0x1;
	int ureg = (sharc.opcode >> 23) & 0xff;
	int compute = sharc.opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source UREG must be saved */
		/* because the compute operation may change it */
		UINT32 parallel_ureg = GET_UREG(ureg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (g)		/* PM */
		{
			if (d)		/* ureg -> PM */
			{
				if (ureg == 0xdb)		/* PX register access is always 48-bit */
				{
					pm_write48(PM_REG_I(i)+PM_REG_M(m), sharc.px);
				}
				else
				{
					pm_write32(PM_REG_I(i)+PM_REG_M(m), parallel_ureg);
				}
			}
			else		/* PM <- ureg */
			{
				if (ureg == 0xdb)		/* PX register access is always 48-bit */
				{
					sharc.px = pm_read48(PM_REG_I(i)+PM_REG_M(m));
				}
				else
				{
					SET_UREG(ureg, pm_read32(PM_REG_I(i)+PM_REG_M(m)));
				}
			}
		}
		else	/* DM */
		{
			if (d)		/* ureg -> DM */
			{
				dm_write32(DM_REG_I(i)+DM_REG_M(m), parallel_ureg);
			}
			else		/* DM <- ureg */
			{
				SET_UREG(ureg, dm_read32(DM_REG_I(i)+DM_REG_M(m)));
			}
		}
	}
}

/* compute / ureg <-> DM|PM, post-modify */
static void sharcop_compute_ureg_dmpm_postmod(void)
{
	int i = (sharc.opcode >> 41) & 0x7;
	int m = (sharc.opcode >> 38) & 0x7;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int g = (sharc.opcode >> 32) & 0x1;
	int d = (sharc.opcode >> 31) & 0x1;
	int ureg = (sharc.opcode >> 23) & 0xff;
	int compute = sharc.opcode & 0x7fffff;

	if(IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source UREG must be saved */
		/* because the compute operation may change it */
		UINT32 parallel_ureg = GET_UREG(ureg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (g)		/* PM */
		{
			if (d)		/* ureg -> PM */
			{
				if (ureg == 0xdb)		/* PX register access is always 48-bit */
				{
					pm_write48(PM_REG_I(i), sharc.px);
				}
				else
				{
					pm_write32(PM_REG_I(i), parallel_ureg);
				}
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
			else		/* PM <- ureg */
			{
				if (ureg == 0xdb)		/* PX register access is always 48-bit */
				{
					sharc.px = pm_read48(PM_REG_I(i));
				}
				else
				{
					SET_UREG(ureg, pm_read32(PM_REG_I(i)));
				}
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
		}
		else	/* DM */
		{
			if (d)		/* ureg -> DM */
			{
				dm_write32(DM_REG_I(i), parallel_ureg);
				DM_REG_I(i) += DM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_DM(i);
			}
			else		/* DM <- ureg */
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
static void sharcop_compute_dm_to_dreg_immmod(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int u = (sharc.opcode >> 38) & 0x1;
	int dreg = (sharc.opcode >> 23) & 0xf;
	int i = (sharc.opcode >> 41) & 0x7;
	int mod = SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f);
	int compute = sharc.opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)		/* post-modify with update */
		{
			REG(dreg) = dm_read32(DM_REG_I(i));
			DM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_DM(i);
		}
		else		/* pre-modify, no update */
		{
			REG(dreg) = dm_read32(DM_REG_I(i) + mod);
		}
	}
}

/* compute / dreg -> DM, immediate modify */
static void sharcop_compute_dreg_to_dm_immmod(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int u = (sharc.opcode >> 38) & 0x1;
	int dreg = (sharc.opcode >> 23) & 0xf;
	int i = (sharc.opcode >> 41) & 0x7;
	int mod = SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f);
	int compute = sharc.opcode & 0x7fffff;

	/* due to parallelity issues, source REG must be saved */
	/* because the shift operation may change it */
	UINT32 parallel_dreg = REG(dreg);

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)		/* post-modify with update */
		{
			dm_write32(DM_REG_I(i), parallel_dreg);
			DM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_DM(i);
		}
		else		/* pre-modify, no update */
		{
			dm_write32(DM_REG_I(i) + mod, parallel_dreg);
		}
	}
}

/* compute / dreg <- PM, immediate modify */
static void sharcop_compute_pm_to_dreg_immmod(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int u = (sharc.opcode >> 38) & 0x1;
	int dreg = (sharc.opcode >> 23) & 0xf;
	int i = (sharc.opcode >> 41) & 0x7;
	int mod = SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f);
	int compute = sharc.opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)		/* post-modify with update */
		{
			REG(dreg) = pm_read32(PM_REG_I(i));
			PM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_PM(i);
		}
		else		/* pre-modify, no update */
		{
			REG(dreg) = pm_read32(PM_REG_I(i) + mod);
		}
	}
}

/* compute / dreg -> PM, immediate modify */
static void sharcop_compute_dreg_to_pm_immmod(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int u = (sharc.opcode >> 38) & 0x1;
	int dreg = (sharc.opcode >> 23) & 0xf;
	int i = (sharc.opcode >> 41) & 0x7;
	int mod = SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f);
	int compute = sharc.opcode & 0x7fffff;

	/* due to parallelity issues, source REG must be saved */
	/* because the compute operation may change it */
	UINT32 parallel_dreg = REG(dreg);

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (u)		/* post-modify with update */
		{
			pm_write32(PM_REG_I(i), parallel_dreg);
			PM_REG_I(i) += mod;
			UPDATE_CIRCULAR_BUFFER_PM(i);
		}
		else		/* pre-modify, no update */
		{
			pm_write32(PM_REG_I(i) + mod, parallel_dreg);
		}
	}
}

/*****************************************************************************/
/* | 0111xxxxx | */

/* compute / ureg <-> ureg */
static void sharcop_compute_ureg_to_ureg(void)
{
	int src_ureg = (sharc.opcode >> 36) & 0xff;
	int dst_ureg = (sharc.opcode >> 23) & 0xff;
	int cond = (sharc.opcode >> 31) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;

	if (IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source UREG must be saved */
		/* because the compute operation may change it */
		UINT32 parallel_ureg = GET_UREG(src_ureg);

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
static void sharcop_imm_shift_dreg_dmpm(void)
{
	int i = (sharc.opcode >> 41) & 0x7;
	int m = (sharc.opcode >> 38) & 0x7;
	int g = (sharc.opcode >> 32) & 0x1;
	int d = (sharc.opcode >> 31) & 0x1;
	int dreg = (sharc.opcode >> 23) & 0xf;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int data = ((sharc.opcode >> 8) & 0xff) | ((sharc.opcode >> 19) & 0xf00);
	int shiftop = (sharc.opcode >> 16) & 0x3f;
	int rn = (sharc.opcode >> 4) & 0xf;
	int rx = (sharc.opcode & 0xf);

	if (IF_CONDITION_CODE(cond))
	{
		/* due to parallelity issues, source REG must be saved */
		/* because the shift operation may change it */
		UINT32 parallel_dreg = REG(dreg);

		SHIFT_OPERATION_IMM(shiftop, data, rn, rx);

		if (g)		/* PM */
		{
			if (d)		/* dreg -> PM */
			{
				pm_write32(PM_REG_I(i), parallel_dreg);
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
			else		/* PM <- dreg */
			{
				REG(dreg) = pm_read32(PM_REG_I(i));
				PM_REG_I(i) += PM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_PM(i);
			}
		}
		else		/* DM */
		{
			if (d)		/* dreg -> DM */
			{
				dm_write32(DM_REG_I(i), parallel_dreg);
				DM_REG_I(i) += DM_REG_M(m);
				UPDATE_CIRCULAR_BUFFER_DM(i);
			}
			else	/* DM <- dreg */
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
static void sharcop_imm_shift(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int data = ((sharc.opcode >> 8) & 0xff) | ((sharc.opcode >> 19) & 0xf00);
	int shiftop = (sharc.opcode >> 16) & 0x3f;
	int rn = (sharc.opcode >> 4) & 0xf;
	int rx = (sharc.opcode & 0xf);

	if (IF_CONDITION_CODE(cond))
	{
		SHIFT_OPERATION_IMM(shiftop, data, rn, rx);
	}
}

/*****************************************************************************/
/* | 00000100x | */

/* compute / modify */
static void sharcop_compute_modify(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;
	int g = (sharc.opcode >> 38) & 0x1;
	int m = (sharc.opcode >> 27) & 0x7;
	int i = (sharc.opcode >> 30) & 0x7;

	if (IF_CONDITION_CODE(cond))
	{
		if (compute != 0)
		{
			COMPUTE(compute);
		}

		if (g)		/* Modify PM */
		{
			PM_REG_I(i) += PM_REG_M(m);
			UPDATE_CIRCULAR_BUFFER_PM(i);
		}
		else		/* Modify DM */
		{
			DM_REG_I(i) += DM_REG_M(m);
			UPDATE_CIRCULAR_BUFFER_DM(i);
		}
	}
}

/*****************************************************************************/
/* | 00000110x | */

/* direct call to absolute address */
static void sharcop_direct_call(void)
{
	int j = (sharc.opcode >> 26) & 0x1;
	int cond = (sharc.opcode >> 33) & 0x1f;
	UINT32 address = sharc.opcode & 0xffffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (j)
		{
			//PUSH_PC(sharc.pc+3);  /* 1 instruction + 2 delayed instructions */
			PUSH_PC(sharc.nfaddr);	/* 1 instruction + 2 delayed instructions */
			CHANGE_PC_DELAYED(address);
		}
		else
		{
			//PUSH_PC(sharc.pc+1);
			PUSH_PC(sharc.daddr);
			CHANGE_PC(address);
		}
	}
}

/* direct jump to absolute address */
static void sharcop_direct_jump(void)
{
	int la = (sharc.opcode >> 38) & 0x1;
	int ci = (sharc.opcode >> 24) & 0x1;
	int j = (sharc.opcode >> 26) & 0x1;
	int cond = (sharc.opcode >> 33) & 0x1f;
	UINT32 address = sharc.opcode & 0xffffff;

	if(IF_CONDITION_CODE(cond))
	{
		// Clear Interrupt
		if (ci)
		{
			// TODO: anything else?
			if (sharc.status_stkp > 0)
			{
				POP_STATUS_STACK();
			}

			sharc.interrupt_active = 0;
			sharc.irptl &= ~(1 << sharc.active_irq_num);
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
static void sharcop_relative_call(void)
{
	int j = (sharc.opcode >> 26) & 0x1;
	int cond = (sharc.opcode >> 33) & 0x1f;
	UINT32 address = sharc.opcode & 0xffffff;

	if (IF_CONDITION_CODE(cond))
	{
		if (j)
		{
			PUSH_PC(sharc.pc+3);	/* 1 instruction + 2 delayed instructions */
			CHANGE_PC_DELAYED(sharc.pc + SIGN_EXTEND24(address));
		}
		else
		{
			PUSH_PC(sharc.pc+1);
			CHANGE_PC(sharc.pc + SIGN_EXTEND24(address));
		}
	}
}

/* direct jump to relative address */
static void sharcop_relative_jump(void)
{
	int la = (sharc.opcode >> 38) & 0x1;
	int ci = (sharc.opcode >> 24) & 0x1;
	int j = (sharc.opcode >> 26) & 0x1;
	int cond = (sharc.opcode >> 33) & 0x1f;
	UINT32 address = sharc.opcode & 0xffffff;

	if (IF_CONDITION_CODE(cond))
	{
		// Clear Interrupt
		if (ci)
		{
			// TODO: anything else?
			if (sharc.status_stkp > 0)
			{
				POP_STATUS_STACK();
			}

			sharc.interrupt_active = 0;
			sharc.irptl &= ~(1 << sharc.active_irq_num);
		}

		if (la)
		{
			POP_PC();
			POP_LOOP();
		}

		if (j)
		{
			CHANGE_PC_DELAYED(sharc.pc + SIGN_EXTEND24(address));
		}
		else
		{
			CHANGE_PC(sharc.pc + SIGN_EXTEND24(address));
		}
	}
}

/*****************************************************************************/
/* | 00001000x | */

/* indirect jump */
static void sharcop_indirect_jump(void)
{
	int la = (sharc.opcode >> 38) & 0x1;
	int ci = (sharc.opcode >> 24) & 0x1;
	int j = (sharc.opcode >> 26) & 0x1;
	int e = (sharc.opcode >> 25) & 0x1;
	int pmi = (sharc.opcode >> 30) & 0x7;
	int pmm = (sharc.opcode >> 27) & 0x7;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;

	// Clear Interrupt
	if (ci)
	{
		// TODO: anything else?
		if (sharc.status_stkp > 0)
		{
			POP_STATUS_STACK();
		}

		sharc.interrupt_active = 0;
		sharc.irptl &= ~(1 << sharc.active_irq_num);
	}

	if (e)		/* IF...ELSE */
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
	else		/* IF */
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
static void sharcop_indirect_call(void)
{
	int j = (sharc.opcode >> 26) & 0x1;
	int e = (sharc.opcode >> 25) & 0x1;
	int pmi = (sharc.opcode >> 30) & 0x7;
	int pmm = (sharc.opcode >> 27) & 0x7;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;

	if (e)		/* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (j)
			{
				//PUSH_PC(sharc.pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(sharc.nfaddr);	/* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
			else
			{
				//PUSH_PC(sharc.pc+1);
				PUSH_PC(sharc.daddr);
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
	else		/* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (j)
			{
				//PUSH_PC(sharc.pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(sharc.nfaddr);	/* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
			else
			{
				//PUSH_PC(sharc.pc+1);
				PUSH_PC(sharc.daddr);
				CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
			}
		}
	}
}

/*****************************************************************************/
/* | 00001001x | */

/* indirect jump to relative address */
static void sharcop_relative_jump_compute(void)
{
	int la = (sharc.opcode >> 38) & 0x1;
	int ci = (sharc.opcode >> 24) & 0x1;
	int j = (sharc.opcode >> 26) & 0x1;
	int e = (sharc.opcode >> 25) & 0x1;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;

	// Clear Interrupt
	if (ci)
	{
		// TODO: anything else?
		if (sharc.status_stkp > 0)
		{
			POP_STATUS_STACK();
		}

		sharc.interrupt_active = 0;
		sharc.irptl &= ~(1 << sharc.active_irq_num);
	}

	if (e)		/* IF...ELSE */
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
				CHANGE_PC_DELAYED(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
			}
			else
			{
				CHANGE_PC(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
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
	else		/* IF */
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
				CHANGE_PC_DELAYED(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
			}
			else
			{
				CHANGE_PC(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
			}
		}
	}
}

/* indirect call to relative address */
static void sharcop_relative_call_compute(void)
{
	int j = (sharc.opcode >> 26) & 0x1;
	int e = (sharc.opcode >> 25) & 0x1;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int compute = sharc.opcode & 0x7fffff;

	if (e)		/* IF...ELSE */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (j)
			{
				//PUSH_PC(sharc.pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(sharc.nfaddr);	/* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
			}
			else
			{
				//PUSH_PC(sharc.pc+1);
				PUSH_PC(sharc.daddr);
				CHANGE_PC(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
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
	else		/* IF */
	{
		if (IF_CONDITION_CODE(cond))
		{
			if (compute)
			{
				COMPUTE(compute);
			}

			if (j)
			{
				//PUSH_PC(sharc.pc+3);  /* 1 instruction + 2 delayed instructions */
				PUSH_PC(sharc.nfaddr);	/* 1 instruction + 2 delayed instructions */
				CHANGE_PC_DELAYED(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
			}
			else
			{
				//PUSH_PC(sharc.pc+1);
				PUSH_PC(sharc.daddr);
				CHANGE_PC(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
			}
		}
	}
}

/*****************************************************************************/
/* | 110xxxxxx | */

/* indirect jump / compute / dreg <-> DM */
static void sharcop_indirect_jump_compute_dreg_dm(void)
{
	int d = (sharc.opcode >> 44) & 0x1;
	int dmi = (sharc.opcode >> 41) & 0x7;
	int dmm = (sharc.opcode >> 38) & 0x7;
	int pmi = (sharc.opcode >> 30) & 0x7;
	int pmm = (sharc.opcode >> 27) & 0x7;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int dreg = (sharc.opcode >> 23) & 0xf;

	if (IF_CONDITION_CODE(cond))
	{
		CHANGE_PC(PM_REG_I(pmi) + PM_REG_M(pmm));
	}
	else
	{
		UINT32 compute = sharc.opcode & 0x7fffff;
		/* due to parallelity issues, source REG must be saved */
		/* because the compute operation may change it */
		UINT32 parallel_dreg = REG(dreg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (d)		/* dreg -> DM */
		{
			dm_write32(DM_REG_I(dmi), parallel_dreg);
			DM_REG_I(dmi) += DM_REG_M(dmm);
			UPDATE_CIRCULAR_BUFFER_DM(dmi);
		}
		else		/* DM <- dreg */
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
static void sharcop_relative_jump_compute_dreg_dm(void)
{
	int d = (sharc.opcode >> 44) & 0x1;
	int dmi = (sharc.opcode >> 41) & 0x7;
	int dmm = (sharc.opcode >> 38) & 0x7;
	int cond = (sharc.opcode >> 33) & 0x1f;
	int dreg = (sharc.opcode >> 23) & 0xf;

	if (IF_CONDITION_CODE(cond))
	{
		CHANGE_PC(sharc.pc + SIGN_EXTEND6((sharc.opcode >> 27) & 0x3f));
	}
	else
	{
		UINT32 compute = sharc.opcode & 0x7fffff;
		/* due to parallelity issues, source REG must be saved */
		/* because the compute operation may change it */
		UINT32 parallel_dreg = REG(dreg);

		if (compute)
		{
			COMPUTE(compute);
		}

		if (d)		/* dreg -> DM */
		{
			dm_write32(DM_REG_I(dmi), parallel_dreg);
			DM_REG_I(dmi) += DM_REG_M(dmm);
			UPDATE_CIRCULAR_BUFFER_DM(dmi);
		}
		else		/* DM <- dreg */
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
static void sharcop_rts(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int j = (sharc.opcode >> 26) & 0x1;
	int e = (sharc.opcode >> 25) & 0x1;
	//int lr = (sharc.opcode >> 24) & 0x1;
	int compute = sharc.opcode & 0x7fffff;

	//if(lr)
	//  fatalerror("SHARC: rts: loop reentry not implemented !");

	if (e)		/* IF...ELSE */
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
	else		/* IF */
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
static void sharcop_rti(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int j = (sharc.opcode >> 26) & 0x1;
	int e = (sharc.opcode >> 25) & 0x1;
	int compute = sharc.opcode & 0x7fffff;

	sharc.irptl &= ~(1 << sharc.active_irq_num);

	if(e)		/* IF...ELSE */
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
	else		/* IF */
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

	if (sharc.status_stkp > 0)
	{
		POP_STATUS_STACK();
	}

	sharc.interrupt_active = 0;
	check_interrupts();
}

/*****************************************************************************/
/* | 00001100x | */

/* do until counter expired, LCNTR immediate */
static void sharcop_do_until_counter_imm(void)
{
	UINT16 data = (UINT16)(sharc.opcode >> 24);
	int offset = SIGN_EXTEND24(sharc.opcode & 0xffffff);
	UINT32 address = sharc.pc + offset;
	int type;
	int cond = 0xf;		/* until LCE (loop counter expired */
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

	sharc.lcntr = data;
	if (sharc.lcntr > 0)
	{
		PUSH_PC(sharc.pc+1);
		PUSH_LOOP(address | (type << 30) | (cond << 24), sharc.lcntr);
	}
}

/*****************************************************************************/
/* | 00001101x | */

/* do until counter expired, LCNTR from UREG */
static void sharcop_do_until_counter_ureg(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	int offset = SIGN_EXTEND24(sharc.opcode & 0xffffff);
	UINT32 address = sharc.pc + offset;
	int type;
	int cond = 0xf;		/* until LCE (loop counter expired */
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

	sharc.lcntr = GET_UREG(ureg);
	if (sharc.lcntr > 0)
	{
		PUSH_PC(sharc.pc+1);
		PUSH_LOOP(address | (type << 30) | (cond << 24), sharc.lcntr);
	}
}

/*****************************************************************************/
/* | 00001110x | */

/* do until */
static void sharcop_do_until(void)
{
	int cond = (sharc.opcode >> 33) & 0x1f;
	int offset = SIGN_EXTEND24(sharc.opcode & 0xffffff);
	UINT32 address = (sharc.pc + offset);

	PUSH_PC(sharc.pc+1);
	PUSH_LOOP(address | (cond << 24), 0);
}

/*****************************************************************************/
/* | 000100 | G | D | */

/* ureg <- DM (direct addressing) */
static void sharcop_dm_to_ureg_direct(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 address = (UINT32)(sharc.opcode);

	SET_UREG(ureg, dm_read32(address));
}

/* ureg -> DM (direct addressing) */
static void sharcop_ureg_to_dm_direct(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 address = (UINT32)(sharc.opcode);

	dm_write32(address, GET_UREG(ureg));
}

/* ureg <- PM (direct addressing) */
static void sharcop_pm_to_ureg_direct(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 address = (UINT32)(sharc.opcode);

	if (ureg == 0xdb)		// PX is 48-bit
	{
		sharc.px = pm_read48(address);
	}
	else
	{
		SET_UREG(ureg, pm_read32(address));
	}
}

/* ureg -> PM (direct addressing) */
static void sharcop_ureg_to_pm_direct(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 address = (UINT32)(sharc.opcode);

	if (ureg == 0xdb)		// PX is 48-bit
	{
		pm_write48(address, sharc.px);
	}
	else
	{
		pm_write32(address, GET_UREG(ureg));
	}
}

/*****************************************************************************/
/* | 101 | G | III | D | */

/* ureg <- DM (indirect addressing) */
static void sharcop_dm_to_ureg_indirect(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 offset = (UINT32)sharc.opcode;
	int i = (sharc.opcode >> 41) & 0x7;

	SET_UREG(ureg, dm_read32(DM_REG_I(i) + offset));
}

/* ureg -> DM (indirect addressing) */
static void sharcop_ureg_to_dm_indirect(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 offset = (UINT32)sharc.opcode;
	int i = (sharc.opcode >> 41) & 0x7;

	dm_write32(DM_REG_I(i) + offset, GET_UREG(ureg));
}

/* ureg <- PM (indirect addressing) */
static void sharcop_pm_to_ureg_indirect(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 offset = sharc.opcode & 0xffffff;
	int i = (sharc.opcode >> 41) & 0x7;

	if (ureg == 0xdb)		/* PX is 48-bit */
	{
		sharc.px = pm_read48(PM_REG_I(i) + offset);
	}
	else
	{
		SET_UREG(ureg, pm_read32(PM_REG_I(i) + offset));
	}
}

/* ureg -> PM (indirect addressing) */
static void sharcop_ureg_to_pm_indirect(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 offset = (UINT32)sharc.opcode;
	int i = (sharc.opcode >> 41) & 0x7;

	if (ureg == 0xdb)		/* PX is 48-bit */
	{
		pm_write48(PM_REG_I(i) + offset, sharc.px);
	}
	else
	{
		pm_write32(PM_REG_I(i) + offset, GET_UREG(ureg));
	}
}

/*****************************************************************************/
/* | 1001xxxxx | */

/* immediate data -> DM|PM */
static void sharcop_imm_to_dmpm(void)
{
	int i = (sharc.opcode >> 41) & 0x7;
	int m = (sharc.opcode >> 38) & 0x7;
	int g = (sharc.opcode >> 37) & 0x1;
	UINT32 data = (UINT32)sharc.opcode;

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
static void sharcop_imm_to_ureg(void)
{
	int ureg = (sharc.opcode >> 32) & 0xff;
	UINT32 data = (UINT32)sharc.opcode;

	SET_UREG(ureg, data);
}

/*****************************************************************************/
/* | 00010100x | */

/* system register bit manipulation */
static void sharcop_sysreg_bitop(void)
{
	int bop = (sharc.opcode >> 37) & 0x7;
	int sreg = (sharc.opcode >> 32) & 0xf;
	UINT32 data = (UINT32)sharc.opcode;

	UINT32 src = GET_UREG(0x70 | sreg);

	switch(bop)
	{
		case 0:		/* SET */
		{
			src |= data;
			break;
		}
		case 1:		/* CLEAR */
		{
			src &= ~data;
			break;
		}
		case 2:		/* TOGGLE */
		{
			src ^= data;
			break;
		}
		case 4:		/* TEST */
		{
			if ((src & data) == data)
			{
				sharc.astat |= BTF;
			}
			else
			{
				sharc.astat &= ~BTF;
			}
			break;
		}
		case 5:		/* XOR */
		{
			if (src == data)
			{
				sharc.astat |= BTF;
			}
			else
			{
				sharc.astat &= ~BTF;
			}
			break;
		}
		default:
			fatalerror("SHARC: sysreg_bitop: invalid bitop %d", bop);
			break;
	}

	SET_UREG(0x70 | sreg, src);
}

/*****************************************************************************/
/* | 000101100 | */

/* I register modify */
static void sharcop_modify(void)
{
	int g = (sharc.opcode >> 38) & 0x1;
	int i = (sharc.opcode >> 32) & 0x7;
	INT32 data = (sharc.opcode);

	if (g)		// PM
	{
		PM_REG_I(i) += data;
		UPDATE_CIRCULAR_BUFFER_PM(i);
	}
	else		// DM
	{
		DM_REG_I(i) += data;
		UPDATE_CIRCULAR_BUFFER_DM(i);
	}
}

/*****************************************************************************/
/* | 000101101 | */

/* I register bit-reverse */
static void sharcop_bit_reverse(void)
{
	fatalerror("SHARC: sharcop_bit_reverse unimplemented");
}

/*****************************************************************************/
/* | 00010111x | */

/* push/pop stacks / flush cache */
static void sharcop_push_pop_stacks(void)
{
	if (sharc.opcode & U64(0x008000000000))
	{
		fatalerror("sharcop_push_pop_stacks: push loop not implemented");
	}
	if (sharc.opcode & U64(0x004000000000))
	{
		fatalerror("sharcop_push_pop_stacks: pop loop not implemented");
	}
	if (sharc.opcode & U64(0x002000000000))
	{
		//fatalerror("sharcop_push_pop_stacks: push sts not implemented");
		PUSH_STATUS_STACK();
	}
	if (sharc.opcode & U64(0x001000000000))
	{
		//fatalerror("sharcop_push_pop_stacks: pop sts not implemented");
		POP_STATUS_STACK();
	}
	if (sharc.opcode & U64(0x000800000000))
	{
		PUSH_PC(sharc.pcstk);
	}
	if (sharc.opcode & U64(0x000400000000))
	{
		POP_PC();
	}
}

/*****************************************************************************/
/* | 000000000 | */

static void sharcop_nop(void)
{

}

/*****************************************************************************/
/* | 000000001 | */

static void sharcop_idle(void)
{
	//CHANGE_PC(sharc.pc);

	sharc.daddr = sharc.pc;
	sharc.faddr = sharc.pc+1;
	sharc.nfaddr = sharc.pc+2;

	sharc.decode_opcode = ROPCODE(sharc.daddr);
	sharc.fetch_opcode = ROPCODE(sharc.faddr);

	sharc.idle = 1;
}

/*****************************************************************************/

static void sharcop_unimplemented(void)
{
#ifdef MAME_DEBUG
	char dasm[1000];
	sharc_dasm(dasm, sharc.pc, NULL, NULL);
	mame_printf_debug("SHARC: %08X: %s\n", sharc.pc, dasm);
	fatalerror("SHARC: Unimplemented opcode %04X%08X at %08X", (UINT16)(sharc.opcode >> 32), (UINT32)(sharc.opcode), sharc.pc);
#else
	fatalerror("SHARC: Unimplemented opcode %04X%08X at %08X", (UINT16)(sharc.opcode >> 32), (UINT32)(sharc.opcode), sharc.pc);
#endif
}
