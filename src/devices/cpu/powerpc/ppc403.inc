// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* PowerPC 403 specific functions */

static void ppc403_dma_exec(int ch);

#define DMA_CE      0x80000000
#define DMA_CIE     0x40000000
#define DMA_TD      0x20000000
#define DMA_PL      0x10000000
#define DMA_DAI     0x02000000
#define DMA_SAI     0x01000000
#define DMA_CP      0x00800000
#define DMA_ETD     0x00000200
#define DMA_TCE     0x00000100
#define DMA_CH      0x00000080
#define DMA_BME     0x00000040
#define DMA_ECE     0x00000020
#define DMA_TCD     0x00000010
#define DMA_PCE     0x00000008

static SPU_RX_HANDLER spu_rx_handler;
static SPU_TX_HANDLER spu_tx_handler;
static TIMER_CALLBACK( ppc403_spu_rx_callback );
static TIMER_CALLBACK( ppc403_spu_tx_callback );

static PPC_DMA_HANDLER spu_rx_dma_handler;
static PPC_DMA_HANDLER spu_tx_dma_handler;
static UINT8 *spu_rx_dma_ptr;
static UINT8 *spu_tx_dma_ptr;

static PPC_DMA_HANDLER dma_read_handler[4];
static PPC_DMA_HANDLER dma_write_handler[4];
static UINT8 *dma_read_ptr[4];
static UINT8 *dma_write_ptr[4];

INLINE void ppc_set_dcr(int dcr, UINT32 value)
{
	switch(dcr)
	{
		case DCR_BEAR:      ppc.bear = value; break;
		case DCR_BESR:      ppc.besr = value; break;
		case DCR_BR0:       ppc.br[0] = value; break;
		case DCR_BR1:       ppc.br[1] = value; break;
		case DCR_BR2:       ppc.br[2] = value; break;
		case DCR_BR3:       ppc.br[3] = value; break;
		case DCR_BR4:       ppc.br[4] = value; break;
		case DCR_BR5:       ppc.br[5] = value; break;
		case DCR_BR6:       ppc.br[6] = value; break;
		case DCR_BR7:       ppc.br[7] = value; break;

		case DCR_EXISR:     ppc.exisr &= ~value; break;
		case DCR_EXIER:     EXIER = value; ppc.exisr &= EXIER; break;
		case DCR_IOCR:      ppc.iocr = value; break;
		case DCR_DMASR:     break;      /* TODO */
		case DCR_DMADA0:    ppc.dma[0].da = value; break;
		case DCR_DMADA1:    ppc.dma[1].da = value; break;
		case DCR_DMADA2:    ppc.dma[2].da = value; break;
		case DCR_DMADA3:    ppc.dma[3].da = value; break;
		case DCR_DMASA0:    ppc.dma[0].sa = value; break;
		case DCR_DMASA1:    ppc.dma[1].sa = value; break;
		case DCR_DMASA2:    ppc.dma[2].sa = value; break;
		case DCR_DMASA3:    ppc.dma[3].sa = value; break;
		case DCR_DMACT0:    ppc.dma[0].ct = value; break;
		case DCR_DMACT1:    ppc.dma[1].ct = value; break;
		case DCR_DMACT2:    ppc.dma[2].ct = value; break;
		case DCR_DMACT3:    ppc.dma[3].ct = value; break;
		case DCR_DMACR0:    ppc.dma[0].cr = value; ppc403_dma_exec(0); break;
		case DCR_DMACR1:    ppc.dma[1].cr = value; ppc403_dma_exec(1); break;
		case DCR_DMACR2:    ppc.dma[2].cr = value; ppc403_dma_exec(2); break;
		case DCR_DMACR3:    ppc.dma[3].cr = value; ppc403_dma_exec(3); break;

		default:
			fatalerror("ppc: set_dcr: Unimplemented DCR %X\n", dcr);
			break;
	}
}

INLINE UINT32 ppc_get_dcr(int dcr)
{
	switch(dcr)
	{
		case DCR_BEAR:      return ppc.bear;
		case DCR_BESR:      return ppc.besr;
		case DCR_BR0:       return ppc.br[0];
		case DCR_BR1:       return ppc.br[1];
		case DCR_BR2:       return ppc.br[2];
		case DCR_BR3:       return ppc.br[3];
		case DCR_BR4:       return ppc.br[4];
		case DCR_BR5:       return ppc.br[5];
		case DCR_BR6:       return ppc.br[6];
		case DCR_BR7:       return ppc.br[7];
		case DCR_EXISR:     return EXISR;
		case DCR_EXIER:     return EXIER;
		case DCR_IOCR:      return ppc.iocr;
		case DCR_DMASR:     return ppc.dmasr;
		case DCR_DMADA0:    return ppc.dma[0].da;
		case DCR_DMADA1:    return ppc.dma[1].da;
		case DCR_DMADA2:    return ppc.dma[2].da;
		case DCR_DMADA3:    return ppc.dma[3].da;
		case DCR_DMASA0:    return ppc.dma[0].sa;
		case DCR_DMASA1:    return ppc.dma[1].sa;
		case DCR_DMASA2:    return ppc.dma[2].sa;
		case DCR_DMASA3:    return ppc.dma[3].sa;
		case DCR_DMACT0:    return ppc.dma[0].ct;
		case DCR_DMACT1:    return ppc.dma[1].ct;
		case DCR_DMACT2:    return ppc.dma[2].ct;
		case DCR_DMACT3:    return ppc.dma[3].ct;
		case DCR_DMACR0:    return ppc.dma[0].cr;
		case DCR_DMACR1:    return ppc.dma[1].cr;
		case DCR_DMACR2:    return ppc.dma[2].cr;
		case DCR_DMACR3:    return ppc.dma[3].cr;

		default:
			fatalerror("ppc: get_dcr: Unimplemented DCR %X\n", dcr);
			break;
	}
}



#ifndef PPC_DRC
INLINE void ppc403_check_interrupts(void)
{
	if (MSR & MSR_EE)
	{
		if (ppc.interrupt_pending != 0)
		{
			if (ppc.interrupt_pending & 0x1)
			{
				ppc403_exception(EXCEPTION_IRQ);
			}
			else if (ppc.interrupt_pending & 0x2)
			{
				ppc403_exception(EXCEPTION_PROGRAMMABLE_INTERVAL_TIMER);
			}
			else if (ppc.interrupt_pending & 0x4)
			{
				ppc403_exception(EXCEPTION_FIXED_INTERVAL_TIMER);
			}
		}
	}
}

static CPU_RESET( ppc403 )
{
	ppc.pc = ppc.npc = 0xfffffffc;

	ppc_set_msr(0);
}

static CPU_EXECUTE( ppc403 )
{
	UINT32 fit_trigger_cycle;
	ppc_tb_base_icount = cycles;

	fit_trigger_cycle = 0x7fffffff;

	if (ppc.fit_int_enable)
	{
		UINT32 tb = (UINT32)ppc.tb;
		UINT32 fit_cycles = 0;

		if (ppc.tb & ppc.fit_bit)
		{
			fit_cycles += ppc.fit_bit;
			tb += fit_cycles;
		}

		fit_cycles += ppc.fit_bit - (tb & (ppc.fit_bit-1));

		fit_trigger_cycle = ppc_icount - fit_cycles;
	}

	while( ppc_icount > 0 )
	{
		UINT32 opcode;

		debugger_instruction_hook(device, ppc.pc);
		ppc.pc = ppc.npc;
		ppc.npc += 4;
		opcode = ROPCODE(ppc.pc);

		switch(opcode >> 26)
		{
			case 19:    ppc.optable19[(opcode >> 1) & 0x3ff](opcode); break;
			case 31:    ppc.optable31[(opcode >> 1) & 0x3ff](opcode); break;
			case 59:    ppc.optable59[(opcode >> 1) & 0x3ff](opcode); break;
			case 63:    ppc.optable63[(opcode >> 1) & 0x3ff](opcode); break;
			default:    ppc.optable[opcode >> 26](opcode); break;
		}

		ppc_icount--;

		/* Programmable Interval Timer (PIT) */
		if (ppc.pit_counter > 0)
		{
			ppc.pit_counter--;
			if (ppc.pit_counter == 0)
			{
				if (ppc.pit_int_enable) {
					ppc.interrupt_pending |= 0x2;
				}
				if (ppc.tcr & 0x00400000)   // Automatic reload
				{
					ppc.pit_counter = ppc.pit;
				}
			}
		}

		/* Fixed Interval Timer */
		if (fit_trigger_cycle != 0x7fffffff)
		{
			if (ppc_icount == fit_trigger_cycle)
			{
				if (ppc.fit_int_enable)
				{
					fit_trigger_cycle -= ppc.fit_bit;
					ppc.interrupt_pending |= 0x4;
				}
			}
		}

#if 0
		/* Watchdog Timer */
		if (((UINT32)(ppc.tb) & ppc.wdt_bit) && (tblo & ppc.wdt_bit) == 0) {
			switch((ppc.tsr >> 28) & 0x3)
			{
				case 0: ppc.tsr |= TSR_ENW; break;
				case 1: ppc.tsr |= TSR_ENW; break;
				case 2:
					if (ppc.wdt_int_enable && (ppc.msr & MSR_CE)) {
						ppc403_exception(EXCEPTION_WATCHDOG_TIMER);
					}
					break;
				case 3:
					fatalerror("PPC: Watchdog Timer caused reset\n");
					break;
			}
		}
#endif

		ppc403_check_interrupts();
	}

	// update timebase
	ppc.tb += (ppc_tb_base_icount - ppc_icount);
}

void ppc403_exception(int exception)
{
	switch( exception )
	{
		case EXCEPTION_IRQ:     /* External Interrupt */
		{
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr;

				msr &= ~(MSR_WE | MSR_PR | MSR_EE | MSR_PE);    // Clear WE, PR, EE, PR
				if( msr & MSR_LE )
					msr |= MSR_ILE;
				else
					msr &= ~MSR_ILE;
				ppc_set_msr(msr);

				ppc.npc = EVPR | 0x0500;

				ppc.interrupt_pending &= ~0x1;
			}
			break;
		}

		case EXCEPTION_TRAP:            /* Program exception / Trap */
		{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.pc;
				SRR1 = msr;

				msr &= ~(MSR_WE | MSR_PR | MSR_EE | MSR_PE);    // Clear WE, PR, EE, PR
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0700;
				else
					ppc.npc = EVPR | 0x0700;
			break;
		}

		case EXCEPTION_SYSTEM_CALL:     /* System call */
		{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr;

				msr &= ~(MSR_WE | MSR_PR | MSR_EE | MSR_PE);    // Clear WE, PR, EE, PR
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0c00;
				else
					ppc.npc = EVPR | 0x0c00;
			break;
		}

		case EXCEPTION_PROGRAMMABLE_INTERVAL_TIMER:
		{
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr;

				msr &= ~(MSR_WE | MSR_PR | MSR_EE | MSR_PE);    // Clear WE, PR, EE, PR
				if( msr & MSR_LE )
					msr |= MSR_ILE;
				else
					msr &= ~MSR_ILE;
				ppc_set_msr(msr);

				ppc.npc = EVPR | 0x1000;

				ppc.tsr |= 0x08000000;      // PIT interrupt
				ppc.interrupt_pending &= ~0x2;
			}
			break;
		}

		case EXCEPTION_FIXED_INTERVAL_TIMER:        /* Fixed Interval Timer */
		{
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr;

				msr &= ~(MSR_WE | MSR_PR | MSR_EE | MSR_PE);    // Clear WE, PR, EE, PR
				if( msr & MSR_LE )
					msr |= MSR_ILE;
				else
					msr &= ~MSR_ILE;
				ppc_set_msr(msr);

				ppc.npc = EVPR | 0x1010;
			ppc.interrupt_pending &= ~0x4;
			}
			break;
		}

		case EXCEPTION_WATCHDOG_TIMER:              /* Watchdog Timer */
		{
			UINT32 msr = ppc_get_msr();

			SRR2 = ppc.npc;
			SRR3 = msr;

			msr &= ~(MSR_WE | MSR_PR | MSR_CE | MSR_EE | MSR_DE | MSR_PE | MSR_DR | MSR_IR);
			if (msr & MSR_LE)
				msr |= MSR_ILE;
			else
				msr &= ~MSR_ILE;
			ppc_set_msr(msr);

			ppc.npc = EVPR | 0x1020;
			break;
		}

		case EXCEPTION_CRITICAL_INTERRUPT:
		{
			UINT32 msr = ppc_get_msr();

			SRR2 = ppc.npc;
			SRR3 = msr;

			msr &= ~(MSR_WE | MSR_PR | MSR_CE | MSR_EE | MSR_DE | MSR_PE | MSR_DR | MSR_IR);
			if (msr & MSR_LE)
				msr |= MSR_ILE;
			else
				msr &= ~MSR_ILE;
			ppc_set_msr(msr);

			EXISR |= 0x80000000;
			ppc.npc = EVPR | 0x100;
			break;
		}

		default:
			fatalerror("ppc: Unhandled exception %d\n", exception);
			break;
	}
}

static void ppc403_set_irq_line(int irqline, int state)
{
	if (irqline >= INPUT_LINE_IRQ0 && irqline <= INPUT_LINE_IRQ4)
	{
		UINT32 mask = (1 << (4 - irqline));
		if( state == ASSERT_LINE) {
			if( EXIER & mask ) {
				ppc.exisr |= mask;
				ppc.interrupt_pending |= 0x1;

				if (ppc.irq_callback)
				{
					ppc.irq_callback(ppc.device, irqline);
				}
			}
		}
		// clear line is used to clear the interrupt when the interrupts are level-sensitive
		else if (state == CLEAR_LINE)
		{
			ppc.exisr &= ~mask;
		}
	}
	else if (irqline == PPC_IRQ_SPU_RX)
	{
		UINT32 mask = 0x08000000;
		if (state) {
			if( EXIER & mask ) {
				ppc.exisr |= mask;
				ppc.interrupt_pending |= 0x1;
			}
		}
	}
	else if (irqline == PPC_IRQ_SPU_TX)
	{
		UINT32 mask = 0x04000000;
		if (state) {
			if( EXIER & mask ) {
				ppc.exisr |= mask;
				ppc.interrupt_pending |= 0x1;
			}
		}
	}
	else if (irqline == PPC_IRQ_CRITICAL)
	{
		if (state) {
			if (EXIER & 0x80000000) {
				ppc403_exception(EXCEPTION_CRITICAL_INTERRUPT);
			}
		}
	}
	else
	{
		fatalerror("PPC: Unknown IRQ line %d\n", irqline);
	}
}

static void ppc403_dma_set_irq_line(int dma, int state)
{
	UINT32 mask = (1 << (3 - dma)) << 20;
	if( state ) {
		if( EXIER & mask ) {
			ppc.exisr |= mask;
			ppc.interrupt_pending |= 0x1;
		}
	}
}
#endif

#ifdef PPC_DRC
static void ppc403_dma_set_irq_line(int dma, int state)
{
	UINT32 mask = (1 << (3 - dma)) << 20;
	if( state ) {
		if( EXIER & mask ) {
			ppc.exisr |= mask;
			ppc.exception_pending |= 0x1;
		}
	}
}
#endif






#ifndef PPC_DRC
static void ppc_dccci(UINT32 op)
{
}

static void ppc_dcread(UINT32 op)
{
}

static void ppc_icbt(UINT32 op)
{
}

static void ppc_iccci(UINT32 op)
{
}

static void ppc_icread(UINT32 op)
{
}

static void ppc_rfci(UINT32 op)
{
	UINT32 msr;
	ppc.npc = ppc.srr2;
	msr = ppc.srr3;
	ppc_set_msr( msr );

}
#endif

static void ppc_mfdcr(UINT32 op)
{
	REG(RT) = ppc_get_dcr(SPR);
}

static void ppc_mtdcr(UINT32 op)
{
	ppc_set_dcr(SPR, REG(RS));
}

static void ppc_wrtee(UINT32 op)
{
	if( REG(RS) & 0x8000 )
		ppc_set_msr( ppc_get_msr() | MSR_EE);
	else
		ppc_set_msr( ppc_get_msr() & ~MSR_EE);
}

static void ppc_wrteei(UINT32 op)
{
	if( op & 0x8000 )
		ppc_set_msr( ppc_get_msr() | MSR_EE);
	else
		ppc_set_msr( ppc_get_msr() & ~MSR_EE);
}



/**************************************************************************/
/* PPC403 Serial Port */

static UINT8 ppc403_spu_r(UINT32 a)
{
	switch(a & 0xf)
	{
		case 0x0:       return ppc.spu.spls | 0x6;      /* transmit buffer is always empty */
		case 0x2:       return ppc.spu.sphs;
		case 0x4:       return (ppc.spu.brd >> 8) & 0xf;
		case 0x5:       return (ppc.spu.brd & 0xff);
		case 0x6:       return ppc.spu.spctl;
		case 0x7:       return ppc.spu.sprc;
		case 0x8:       return ppc.spu.sptc;
		case 0x9:       return ppc.spu.sprb;
		default:        fatalerror("ppc: spu_r: %02X\n", a & 0xf);
	}
}

static void ppc403_spu_w(UINT32 a, UINT8 d)
{
	switch(a & 0xf)
	{
		case 0x0:
			if( d & 0x80 )  ppc.spu.spls &= ~0x80;
			if( d & 0x40 )  ppc.spu.spls &= ~0x40;
			if( d & 0x20 )  ppc.spu.spls &= ~0x20;
			if( d & 0x10 )  ppc.spu.spls &= ~0x10;
			if( d & 0x08 )  ppc.spu.spls &= ~0x08;
			break;

		case 0x2:
			ppc.spu.sphs = d;
			break;

		case 0x4:
			ppc.spu.brd &= 0xff;
			ppc.spu.brd |= (d << 8);
			break;

		case 0x5:
			ppc.spu.brd &= 0xff00;
			ppc.spu.brd |= d;
			if (ppc.iocr & 0x2) {
				osd_printf_debug("ppc: SPU Baud rate: %d\n", (3686400 / (ppc.spu.brd + 1)) / 16);
			} else {
				osd_printf_debug("ppc: SPU Baud rate: %d\n", (33333333 / (ppc.spu.brd + 1)) / 16);
			}
			break;

		case 0x6:
			ppc.spu.spctl = d;
			break;

		case 0x7:
			ppc.spu.sprc = d;
			if (ppc.spu.sprc & 0x80)    /* enable RX */
			{
				/*
				int baud_rate;
				if (ppc.iocr & 0x2) {
				    baud_rate = (3686400 / (ppc.spu.brd + 1)) / 16;
				} else {
				    baud_rate = (33333333 / (ppc.spu.brd + 1)) / 16;
				}
				*/

				/* check if serial port is hooked to a DMA channel */
				/* if so, do a DMA operation */
				if( ((((ppc.spu.sprc >> 5) & 0x3) == 2) && (ppc.dma[2].cr & DMA_CE)) ||
					((((ppc.spu.sprc >> 5) & 0x3) == 3) && (ppc.dma[3].cr & DMA_CE)) )
				{
					int i;
					int ch = (ppc.spu.sprc >> 5) & 0x3;
				//  osd_printf_debug("ppc: DMA from serial port on channel %d (DA: %08X)\n", ch, ppc.dma[ch].da);

					if (spu_rx_dma_handler)
					{
						int length = ppc.dma[ch].ct;

						spu_rx_dma_handler(length);

						for (i=0; i < length; i++)
						{
							ppc.program->write_byte(ppc.dma[ch].da++, spu_rx_dma_ptr[i]);
						}
					}

					ppc.dmasr |= (1 << (27 - ch));

					/* generate interrupts */
					if( ppc.dma[ch].cr & DMA_CIE )
					{
						ppc403_dma_set_irq_line( ch, PULSE_LINE );
					}

					/* set receive buffer full */
					ppc.spu.spls = 0x80;

#ifndef PPC_DRC
					ppc403_set_irq_line(PPC_IRQ_SPU_RX, ASSERT_LINE);
#else
					ppcdrc403_set_irq_line(PPC_IRQ_SPU_RX, ASSERT_LINE);
#endif
				}
			}
			else                        /* disable RX */
			{
			}
			break;

		case 0x8:
			ppc.spu.sptc = d;
			break;

		case 0x9:
			ppc.spu.sptb = d;
			ppc403_spu_tx_callback(NULL/* Machine */, NULL, cpunum_get_active());
			break;

		default:
			fatalerror("ppc: spu_w: %02X, %02X\n", a & 0xf, d);
			break;
	}
	//osd_printf_debug("spu_w: %02X, %02X at %08X\n", a & 0xf, d, ppc.pc);
}

void ppc403_spu_rx(UINT8 data)
{
	ppc.spu.sprb = data;

	/* set receive buffer full */
	ppc.spu.spls = 0x80;

	/* generate interrupt if DMA is disabled and RBR interrupt is enabled */
	if (((ppc.spu.sprc >> 5) & 0x3) == 0x01) {
#ifndef PPC_DRC
		ppc403_set_irq_line(PPC_IRQ_SPU_RX, ASSERT_LINE);
#else
		ppcdrc403_set_irq_line(PPC_IRQ_SPU_RX, ASSERT_LINE);
#endif
	}
}

static TIMER_CALLBACK( ppc403_spu_rx_callback )
{
	if (spu_rx_handler != NULL)
	{
		ppc403_spu_rx(spu_rx_handler());
	}
}

static TIMER_CALLBACK( ppc403_spu_tx_callback )
{
	if (spu_tx_handler != NULL)
	{
		spu_tx_handler(ppc.spu.sptb);

		/* generate interrupt if DMA is disabled and TBR interrupt is enabled */
		if (((ppc.spu.sptc >> 5) & 0x3) == 0x01) {
#ifndef PPC_DRC
			ppc403_set_irq_line(PPC_IRQ_SPU_TX, ASSERT_LINE);
#else
			ppcdrc403_set_irq_line(PPC_IRQ_SPU_TX, ASSERT_LINE);
#endif
		}
	}
}

void ppc403_install_spu_rx_handler(SPU_RX_HANDLER rx_handler)
{
	spu_rx_handler = rx_handler;
}

void ppc403_install_spu_tx_handler(SPU_TX_HANDLER tx_handler)
{
	spu_tx_handler = tx_handler;
}


void ppc403_spu_rx_dma(UINT8 *data, int length)
{
}

void ppc403_install_spu_rx_dma_handler(PPC_DMA_HANDLER rx_dma_handler, UINT8 *buffer)
{
	spu_rx_dma_handler = rx_dma_handler;
	spu_rx_dma_ptr = buffer;
}

void ppc403_install_spu_tx_dma_handler(PPC_DMA_HANDLER tx_dma_handler, UINT8 *buffer)
{
	spu_tx_dma_handler = tx_dma_handler;
	spu_tx_dma_ptr = buffer;
}

/*********************************************************************************/

/* PPC 403 DMA */

static const int dma_transfer_width[4] = { 1, 2, 4, 16 };

void ppc403_install_dma_read_handler(int ch, PPC_DMA_HANDLER dma_handler, UINT8 *buffer)
{
	dma_read_handler[ch] = dma_handler;
	dma_read_ptr[ch] = buffer;
}

void ppc403_install_dma_write_handler(int ch, PPC_DMA_HANDLER dma_handler, UINT8 *buffer)
{
	dma_write_handler[ch] = dma_handler;
	dma_write_ptr[ch] = buffer;
}

static void ppc403_dma_exec(int ch)
{
	int i;
	int dai, sai, width;

	/* Is the DMA channel enabled ? */
	if( ppc.dma[ch].cr & DMA_CE )
	{
		/* transfer width */
		width = dma_transfer_width[(ppc.dma[ch].cr >> 26) & 0x3];

		if( ppc.dma[ch].cr & DMA_DAI )
			dai = width;
		else
			dai = 0;        /* DA not incremented */

		if( ppc.dma[ch].cr & DMA_SAI )
			sai = width;
		else
			sai = 0;        /* SA not incremented */


		/* transfer mode */
		switch( (ppc.dma[ch].cr >> 21) & 0x3 )
		{
			case 0:     /* buffered DMA */
				if( ppc.dma[ch].cr & DMA_TD )   /* peripheral to mem */
				{
					// nothing to do for now */
				}
				else                            /* mem to peripheral */
				{
					/* check if the serial port is hooked to channel 2 or 3 */
					if( (ch == 2 && ((ppc.spu.sptc >> 5) & 0x3) == 2) ||
						(ch == 3 && ((ppc.spu.sptc >> 5) & 0x3) == 3) )
					{
						osd_printf_debug("ppc: dma_exec: DMA to serial port on channel %d (DA: %08X)\n", ch, ppc.dma[ch].da);

						if (spu_tx_dma_handler)
						{
							int length = ppc.dma[ch].ct;

							for( i=0; i < length; i++ ) {
								spu_tx_dma_ptr[i] = ppc.program->read_byte(ppc.dma[ch].da++);
							}
							spu_tx_dma_handler(length);
						}

#ifndef PPC_DRC
						ppc403_set_irq_line(PPC_IRQ_SPU_TX, ASSERT_LINE);
#else
						ppcdrc403_set_irq_line(PPC_IRQ_SPU_TX, ASSERT_LINE);
#endif
					}
					else {
						fatalerror("ppc: dma_exec: buffered DMA to unknown peripheral ! (channel %d)\n", ch);
					}

				}
				break;

			case 1:     /* fly-by DMA */
				fatalerror("ppc: dma_exec: fly-by DMA not implemented\n");
				break;

			case 2:     /* software initiated mem-to-mem DMA */
				//osd_printf_debug("ppc: DMA (%d, SW mem-to-mem): SA = %08X, DA = %08X, CT = %08X\n", ch, ppc.dma[ch].sa, ppc.dma[ch].da, ppc.dma[ch].ct);

				switch(width)
				{
					case 1:     /* Byte transfer */
						for (i=0; i < ppc.dma[ch].ct; i++)
						{
							UINT8 b = READ8(ppc.dma[ch].sa);
							WRITE8(ppc.dma[ch].da, b);
							ppc.dma[ch].sa += sai;
							ppc.dma[ch].da += dai;
						}
						break;
					case 2:     /* Word transfer */
						for (i=0; i < ppc.dma[ch].ct; i++)
						{
							UINT16 w = READ16(ppc.dma[ch].sa);
							WRITE16(ppc.dma[ch].da, w);
							ppc.dma[ch].sa += sai;
							ppc.dma[ch].da += dai;
						}
						break;
					case 4:     /* Double word transfer */
						for (i=0; i < ppc.dma[ch].ct; i++)
						{
							UINT32 d = READ32(ppc.dma[ch].sa);
							WRITE32(ppc.dma[ch].da, d);
							ppc.dma[ch].sa += sai;
							ppc.dma[ch].da += dai;
						}
						break;
					case 16:    /* 16-byte transfer */
						for (i=0; i < ppc.dma[ch].ct; i++)
						{
							UINT32 d1 = READ32(ppc.dma[ch].sa+0);
							UINT32 d2 = READ32(ppc.dma[ch].sa+4);
							UINT32 d3 = READ32(ppc.dma[ch].sa+8);
							UINT32 d4 = READ32(ppc.dma[ch].sa+12);
							WRITE32(ppc.dma[ch].da+0, d1);
							WRITE32(ppc.dma[ch].da+4, d2);
							WRITE32(ppc.dma[ch].da+8, d3);
							WRITE32(ppc.dma[ch].da+12, d4);
							ppc.dma[ch].sa += 16;
							ppc.dma[ch].da += 16;
						}
						break;
					default:
						fatalerror("dma: dma_exec: SW mem-to-mem DMA, width = %d\n", width);
				}
				break;

			case 3:     /* hardware initiated mem-to-mem DMA */
				fatalerror("ppc: dma_exec: HW mem-to-mem DMA not implemented\n");
				break;
		}

		ppc.dmasr |= (1 << (27 - ch));

		/* DEBUG: check for not yet supported features */
		if( (ppc.dma[ch].cr & DMA_TCE) == 0 )
			fatalerror("ppc: dma_exec: DMA_TCE == 0\n");

		if( ppc.dma[ch].cr & DMA_CH )
			fatalerror("ppc: dma_exec: DMA chaining not implemented\n");

		/* generate interrupts */
		if( ppc.dma[ch].cr & DMA_CIE )
			ppc403_dma_set_irq_line( ch, PULSE_LINE );

	}
}

/*********************************************************************************/

static UINT8 ppc403_read8(address_space &space, UINT32 a)
{
	if(a >= 0x40000000 && a <= 0x4000000f)      /* Serial Port */
		return ppc403_spu_r(a);
	return space.read_byte(a);
}

#define ppc403_read16   memory_read_word_32be
#define ppc403_read32   memory_read_dword_32be

static void ppc403_write8(address_space &space, UINT32 a, UINT8 d)
{
	if( a >= 0x40000000 && a <= 0x4000000f )        /* Serial Port */
	{
		ppc403_spu_w(a, d);
		return;
	}
	space.write_byte(a, d);
}

#define ppc403_write16  memory_write_word_32be
#define ppc403_write32  memory_write_dword_32be

static UINT16 ppc403_read16_unaligned(address_space &space, UINT32 a)
{
	fatalerror("ppc: Unaligned read16 %08X at %08X\n", a, ppc.pc);
	return 0;
}

static UINT32 ppc403_read32_unaligned(address_space &space, UINT32 a)
{
	fatalerror("ppc: Unaligned read32 %08X at %08X\n", a, ppc.pc);
	return 0;
}

static void ppc403_write16_unaligned(address_space &space, UINT32 a, UINT16 d)
{
	fatalerror("ppc: Unaligned write16 %08X, %04X at %08X\n", a, d, ppc.pc);
}

static void ppc403_write32_unaligned(address_space &space, UINT32 a, UINT32 d)
{
	fatalerror("ppc: Unaligned write32 %08X, %08X at %08X\n", a, d, ppc.pc);
}
