// license:BSD-3-Clause
// copyright-holders:Aaron Giles
static void ppc_dsa(UINT32 op)
{
	UINT32 msr = ppc_get_msr();

	msr &= ~(MSR_SA | MSR_EE | MSR_PR | MSR_AP);
	if (ppc.esasrr & 0x8)   msr |= MSR_PR;
	if (ppc.esasrr & 0x4)   msr |= MSR_AP;
	if (ppc.esasrr & 0x2)   msr |= MSR_SA;
	if (ppc.esasrr & 0x1)   msr |= MSR_EE;

	ppc_set_msr(msr);
}

static void ppc_esa(UINT32 op)
{
	int sa, ee, pr, ap;
	UINT32 msr = ppc_get_msr();

	sa = (msr & MSR_SA) ? 1 : 0;
	ee = (msr & MSR_EE) ? 1 : 0;
	pr = (msr & MSR_PR) ? 1 : 0;
	ap = (msr & MSR_AP) ? 1 : 0;

	ppc.esasrr = (pr << 3) | (ap << 2) | (sa << 1) | (ee);

	msr &= ~(MSR_EE | MSR_PR | MSR_AP);
	msr |= MSR_SA;

	ppc_set_msr(msr);
}

#ifndef PPC_DRC
static void ppc_tlbli(UINT32 op)
{
}

static void ppc_tlbld(UINT32 op)
{
}
#endif

#ifndef PPC_DRC
void ppc602_exception(int exception)
{
	switch( exception )
	{
		case EXCEPTION_IRQ:     /* External Interrupt */
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0500;
				else
					ppc.npc = ppc.ibr | 0x0500;

				ppc.interrupt_pending &= ~0x1;
			}
			break;

		case EXCEPTION_DECREMENTER:     /* Decrementer overflow exception */
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0900;
				else
					ppc.npc = ppc.ibr | 0x0900;

				ppc.interrupt_pending &= ~0x2;
			}
			break;

		case EXCEPTION_TRAP:            /* Program exception / Trap */
			{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.pc;
				SRR1 = (msr & 0xff73) | 0x20000;    /* 0x20000 = TRAP bit */

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0700;
				else
					ppc.npc = ppc.ibr | 0x0700;
			}
			break;

		case EXCEPTION_SYSTEM_CALL:     /* System call */
			{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = (msr & 0xff73);

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0c00;
				else
					ppc.npc = ppc.ibr | 0x0c00;
			}
			break;

		case EXCEPTION_SMI:
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x1400;
				else
					ppc.npc = ppc.ibr | 0x1400;

				ppc.interrupt_pending &= ~0x4;
			}
			break;


		default:
			fatalerror("ppc: Unhandled exception %d\n", exception);
			break;
	}
}

static void ppc602_set_irq_line(int irqline, int state)
{
	if( state ) {
		ppc.interrupt_pending |= 0x1;
		if (ppc.irq_callback)
		{
			ppc.irq_callback(ppc.device, irqline);
		}
	}
}

static void ppc602_set_smi_line(int state)
{
	if( state ) {
		ppc.interrupt_pending |= 0x4;
	}
}

INLINE void ppc602_check_interrupts(void)
{
	if (MSR & MSR_EE)
	{
		if (ppc.interrupt_pending != 0)
		{
			if (ppc.interrupt_pending & 0x1)
			{
				ppc602_exception(EXCEPTION_IRQ);
			}
			else if (ppc.interrupt_pending & 0x2)
			{
				ppc602_exception(EXCEPTION_DECREMENTER);
			}
			else if (ppc.interrupt_pending & 0x4)
			{
				ppc602_exception(EXCEPTION_SMI);
			}
		}
	}
}

static CPU_RESET( ppc602 )
{
	ppc.pc = ppc.npc = 0xfff00100;

	ppc_set_msr(0x40);

	ppc.hid0 = 1;

	ppc.interrupt_pending = 0;
}

static CPU_EXECUTE( ppc602 )
{
	int exception_type;
	UINT32 opcode;
	ppc_tb_base_icount = ppc_icount;
	ppc_dec_base_icount = ppc_icount;

	// check if decrementer exception occurs during execution
	if ((UINT32)(DEC - ppc_icount) > (UINT32)(DEC))
	{
		ppc_dec_trigger_cycle = ppc_icount - DEC;
	}
	else
	{
		ppc_dec_trigger_cycle = 0x7fffffff;
	}

	// MinGW's optimizer kills setjmp()/longjmp()
	SETJMP_GNUC_PROTECT();

	exception_type = setjmp(ppc.exception_jmpbuf);
	if (exception_type)
	{
		ppc.npc = ppc.pc;
		ppc602_exception(exception_type);
	}

	while( ppc_icount > 0 )
	{
		ppc.pc = ppc.npc;
		debugger_instruction_hook(device, ppc.pc);

		if (MSR & MSR_IR)
			opcode = ppc_readop_translated(ppc.program, ppc.pc);
		else
		opcode = ROPCODE64(ppc.pc);

		ppc.npc = ppc.pc + 4;
		switch(opcode >> 26)
		{
			case 19:    ppc.optable19[(opcode >> 1) & 0x3ff](opcode); break;
			case 31:    ppc.optable31[(opcode >> 1) & 0x3ff](opcode); break;
			case 59:    ppc.optable59[(opcode >> 1) & 0x3ff](opcode); break;
			case 63:    ppc.optable63[(opcode >> 1) & 0x3ff](opcode); break;
			default:    ppc.optable[opcode >> 26](opcode); break;
		}

		ppc_icount--;

		if(ppc_icount == ppc_dec_trigger_cycle) {
			ppc.interrupt_pending |= 0x2;
		}

		ppc602_check_interrupts();
	}

	// update timebase
	// timebase is incremented once every four core clock cycles, so adjust the cycles accordingly
	ppc.tb += ((ppc_tb_base_icount - ppc_icount) / 4);

	// update decrementer
	DEC -= ((ppc_dec_base_icount - ppc_icount) / (bus_freq_multiplier * 2));
}
#endif  // PPC_DRC
