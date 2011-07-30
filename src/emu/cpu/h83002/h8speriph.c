/*

	Hitachi H8S/2xxx MCU peripherals

	(c) 2001-2007 Tim Schuerewegen

	H8S/2241
	H8S/2246
	H8S/2323

*/

#include "emu.h"
#include "debugger.h"
#include "h8.h"
#include "h8priv.h"

#define LOG_LEVEL  1
#define _logerror(level,...)  if (LOG_LEVEL > level) logerror(__VA_ARGS__)

//#define ENABLE_SCI_TIMER

////////////
// DTVECR //
////////////

#define DTVECR_ADDR(x) (0x400 + (x << 1))

///////////////
// MEMCONV.H //
///////////////

static TIMER_CALLBACK( h8s_tmr_callback );
static TIMER_CALLBACK( h8s_tpu_callback );
#ifdef ENABLE_SCI_TIMER
static TIMER_CALLBACK( h8s_sci_callback );
#endif

/////////////
// TIMER.C //
/////////////

#ifdef ENABLE_SCI_TIMER
static void timer_adjust_pulse(emu_timer *timer, UINT32 hz, int num)
{
	timer->adjust(ATTOTIME_IN_HZ( hz), num, ATTOTIME_IN_HZ( hz));
}
#endif

void h8s2xxx_interrupt_request(h83xx_state *h8, UINT32 vecnum)
{
	UINT8 idx, bit;
	idx = vecnum >> 5;
	bit = vecnum & 0x1F;
	h8->irq_req[idx] |= (1 << bit);
//	logerror( "irq_req = %08X %08X %08X\n", h8->irq_req[0], h8->irq_req[1], h8->irq_req[2]);
}

void h8s_dtce_execute(h83xx_state *h8, UINT32 addr_dtce, UINT8 mask_dtce, UINT32 addr_dtvecr)
{
	UINT32 data[3], dtc_vect, dtc_sar, dtc_dar, cnt, i;
	UINT8 dtc_mra, dtc_mrb, sz;
	UINT16 dtc_cra, dtc_crb;
	printf("dtce exec!\n");
	// get dtc info
	dtc_vect  = 0xFF0000 | h8->program->read_word(addr_dtvecr);
	data[0]   = h8->program->read_dword( dtc_vect + 0);
	data[1]   = h8->program->read_dword( dtc_vect + 4);
	data[2]   = h8->program->read_dword( dtc_vect + 8);
	dtc_mra   = (data[0] >> 24) & 0xFF;
	dtc_sar   = (data[0] >>  0) & 0xFFFFFF;
	dtc_mrb   = (data[1] >> 24) & 0xFF;
	dtc_dar   = (data[1] >>  0) & 0xFFFFFF;
	dtc_cra   = (data[2] >> 16) & 0xFFFF;
	dtc_crb   = (data[2] >>  0) & 0xFFFF;
	_logerror( 3, "dtc : vect %08X mra %02X sar %08X mrb %02X dar %08X cra %04X crb %04X\n", dtc_vect, dtc_mra, dtc_sar, dtc_mrb, dtc_dar, dtc_cra, dtc_crb);
	// execute
	if ((dtc_mra & 0x0E) != 0x00) fatalerror("H8S: dtc unsupported MRA %x\n", dtc_mra&0x0e);
	sz = 1 << (dtc_mra & 0x01);
	cnt = dtc_cra;
	for (i=0;i<cnt;i++)
	{
		if (dtc_sar == H8S_IO_ADDR( H8S_IO_RDR1)) h8->program->write_byte( H8S_IO_ADDR( H8S_IO_SSR1), h8->program->read_byte( H8S_IO_ADDR( H8S_IO_SSR1)) & (~H8S_SSR_TDRE));
		if (dtc_mra & 0x01) h8->program->write_word( dtc_dar, h8->program->read_word( dtc_sar)); else h8->program->write_byte( dtc_dar, h8->program->read_byte( dtc_sar));
		if (dtc_dar == H8S_IO_ADDR( H8S_IO_TDR0)) h8->program->write_byte( H8S_IO_ADDR( H8S_IO_SSR0), h8->program->read_byte( H8S_IO_ADDR( H8S_IO_SSR0)) & (~H8S_SSR_TDRE));
		if (dtc_mra & 0x80) { if (dtc_mra & 0x40) dtc_sar -= sz; else dtc_sar += sz; }
		if (dtc_mra & 0x20) { if (dtc_mra & 0x10) dtc_dar -= sz; else dtc_dar += sz; }
	}
	h8->program->write_byte( addr_dtce, h8->program->read_byte( addr_dtce) & (~mask_dtce));
}

void h8s_dtce_check(h83xx_state *h8,  int vecnum)
{
	UINT32 dtce = 0;
	int bit = 0;
	// get dtce info
	switch (vecnum)
	{
		// DTCEA
		case H8S_INT_IRQ0  : dtce = H8S_IO_DTCEA; bit = 7; break;
		case H8S_INT_IRQ1  : dtce = H8S_IO_DTCEA; bit = 6; break;
		case H8S_INT_IRQ2  : dtce = H8S_IO_DTCEA; bit = 5; break;
		case H8S_INT_IRQ3  : dtce = H8S_IO_DTCEA; bit = 4; break;
		case H8S_INT_IRQ4  : dtce = H8S_IO_DTCEA; bit = 3; break;
		case H8S_INT_IRQ5  : dtce = H8S_IO_DTCEA; bit = 2; break;
		case H8S_INT_IRQ6  : dtce = H8S_IO_DTCEA; bit = 1; break;
		case H8S_INT_IRQ7  : dtce = H8S_IO_DTCEA; bit = 0; break;
		// DTCEB
		case H8S_INT_ADI   : dtce = H8S_IO_DTCEB; bit = 6; break;
		case H8S_INT_TGI0A : dtce = H8S_IO_DTCEB; bit = 5; break;
		case H8S_INT_TGI0B : dtce = H8S_IO_DTCEB; bit = 4; break;
		case H8S_INT_TGI0C : dtce = H8S_IO_DTCEB; bit = 3; break;
		case H8S_INT_TGI0D : dtce = H8S_IO_DTCEB; bit = 2; break;
		case H8S_INT_TGI1A : dtce = H8S_IO_DTCEB; bit = 1; break;
		case H8S_INT_TGI1B : dtce = H8S_IO_DTCEB; bit = 0; break;
		// DTCEC
		case H8S_INT_TGI2A : dtce = H8S_IO_DTCEC; bit = 7; break;
		case H8S_INT_TGI2B : dtce = H8S_IO_DTCEC; bit = 6; break;
		// DTCED
		case H8S_INT_CMIA0 : dtce = H8S_IO_DTCED; bit = 3; break;
		case H8S_INT_CMIB0 : dtce = H8S_IO_DTCED; bit = 2; break;
		case H8S_INT_CMIA1 : dtce = H8S_IO_DTCED; bit = 1; break;
		case H8S_INT_CMIB1 : dtce = H8S_IO_DTCED; bit = 0; break;
		// DTCEE
		case H8S_INT_RXI0  : dtce = H8S_IO_DTCEE; bit = 3; break;
		case H8S_INT_TXI0  : dtce = H8S_IO_DTCEE; bit = 2; break;
		case H8S_INT_RXI1  : dtce = H8S_IO_DTCEE; bit = 1; break;
		case H8S_INT_TXI1  : dtce = H8S_IO_DTCEE; bit = 0; break;
		// DTCEF
		case H8S_INT_RXI2  : dtce = H8S_IO_DTCEF; bit = 7; break;
		case H8S_INT_TXI2  : dtce = H8S_IO_DTCEF; bit = 6; break;
	}
	// execute
	if ((dtce != 0) && (h8->per_regs[dtce] & (1 << bit))) h8s_dtce_execute(h8, H8S_IO_ADDR( dtce), (1 << bit), DTVECR_ADDR( vecnum));
}

void h8s_periph_reset(h83xx_state *h8)
{
	h8->per_regs[H8S_IO_SSR0] = 0x84;
	h8->per_regs[H8S_IO_SSR1] = 0x84;
	h8->per_regs[H8S_IO_SSR2] = 0x84;
	h8->per_regs[H8S_IO_TDR0] = 0xFF;
	h8->per_regs[H8S_IO_TDR1] = 0xFF;
	h8->per_regs[H8S_IO_TDR2] = 0xFF;
	h8->per_regs[H8S_IO_BRR0] = 0xFF;
	h8->per_regs[H8S_IO_BRR1] = 0xFF;
	h8->per_regs[H8S_IO_BRR2] = 0xFF;
}

///////////
// TIMER //
///////////

typedef struct
{
	UINT32 reg_tcsr, reg_tcr, reg_tcnt;
	UINT8 int_ovi;
} H8S_TMR_ENTRY;

const H8S_TMR_ENTRY H8S_TMR_TABLE[] =
{
	// TMR 0
	{
		H8S_IO_TCSR0, H8S_IO_TCR0, H8S_IO_TCNT0,
		H8S_INT_OVI0
	},
	// TMR 1
	{
		H8S_IO_TCSR1, H8S_IO_TCR1, H8S_IO_TCNT1,
		H8S_INT_OVI1
	}
};

const UINT32 TCR_CKS[] = { 0, 8, 64, 8192 };

const H8S_TMR_ENTRY *h8s_tmr_entry( int num)
{
	return &H8S_TMR_TABLE[num];
}

static TIMER_CALLBACK( h8s_tmr_callback)
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	const H8S_TMR_ENTRY *info = h8s_tmr_entry( param);
	_logerror( 2, "h8s_tmr_callback (%d)\n", param);
	// disable timer
	h8->tmr[param].timer->adjust(attotime::never, param);
	// set timer overflow flag
	h8->per_regs[info->reg_tcsr] |= 0x20;
	// overflow interrupt
	if (h8->per_regs[info->reg_tcr] & 0x20) h8s2xxx_interrupt_request(h8, info->int_ovi);
}

void h8s_tmr_init(h83xx_state *h8)
{
	int i;
	for (i=0;i<2;i++)
	{
		h8->tmr[i].timer = h8->device->machine().scheduler().timer_alloc(FUNC(h8s_tmr_callback), h8);
		h8->tmr[i].timer->adjust(attotime::never, i);
	}
}

void h8s_tmr_resync(h83xx_state *h8, int num)
{
	const H8S_TMR_ENTRY *info = h8s_tmr_entry(num);
	UINT8 cks, tcr, tcnt;
	// get timer data
	tcr = h8->per_regs[info->reg_tcr];
	tcnt = h8->per_regs[info->reg_tcnt];
	//
	cks = tcr & 3;
	if (cks != 0)
	{
		UINT32 cycles, hz;
		cycles = TCR_CKS[cks] * (0xFF - tcnt);
		if (cycles == 0) cycles = 1; // cybiko game "lost in labyrinth" => divide by zero
		hz = h8->device->unscaled_clock() / cycles;
		_logerror( 2, "cycles %d hz %d\n", cycles, hz);
		h8->tmr[num].timer->adjust(attotime::from_hz(hz), num);
	}
	else
	{
		_logerror( 2, "time never\n");
		h8->tmr[num].timer->adjust(attotime::never, num);
	}
}

/////////
// TPU //
/////////

const UINT32 TPSC[] = { 1, 4, 16, 64, 0, 0, 256, 0 };

void set_p1dr_tiocb1(h83xx_state *h8, int data)
{
	UINT8 p1dr;
	p1dr = h8->per_regs[H8S_IO_P1DR];
	if (data) p1dr |= H8S_P1_TIOCB1; else p1dr &= ~H8S_P1_TIOCB1;
	h8->io->write_byte(H8S_IO_ADDR(H8S_IO_P1DR), p1dr);
	h8->per_regs[H8S_IO_P1DR] = p1dr;
}

static TIMER_CALLBACK( h8s_tpu_callback)
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	H8S2XXX_TPU *tpu = &h8->tpu[param];
	_logerror( 2, "h8s_tpu_callback (%d)\n", param);
	switch (param)
	{
		case 1 :
		{
			UINT32 hz, cycles;
			int tiocb1, vecnum;
			// get info
			vecnum = tpu->item[tpu->tgrcur].irq;
			tiocb1 = tpu->item[tpu->tgrcur].out;
			if (tpu->tgrcur >= tpu->tgrmax) tpu->tgrcur = 0;
			cycles = tpu->item[tpu->tgrcur+1].tgr - tpu->item[tpu->tgrcur+0].tgr;
			tpu->tgrcur++;
			// adjust timer
			hz = h8->device->unscaled_clock() / (tpu->prescaler * cycles);
			tpu->timer->adjust(attotime::from_hz(hz), 1);
			// output
			set_p1dr_tiocb1(h8, tiocb1);
			// interrupt
			if (vecnum != 0) h8s2xxx_interrupt_request(h8, vecnum);
		}
		break;
	}
}

void h8s_tpu_init(h83xx_state *h8)
{
	int i;
	for (i=0;i<3;i++)
	{
		h8->tpu[i].timer = h8->device->machine().scheduler().timer_alloc(FUNC(h8s_tpu_callback), h8);
		h8->tpu[i].timer->adjust(attotime::never, i);
	}
}

void h8s_tpu_start(h83xx_state *h8, int num)
{
	UINT8 ttcr, tior, tier, tsr;
	UINT16 tgra, tgrb; //, tcnt;
	UINT32 hz, i, cycles;
	H8S2XXX_TPU *tpu = &h8->tpu[num];
	switch (num)
	{
		case 1 :
		{
			ttcr = h8->per_regs[H8S_IO_TTCR1];
			tior = h8->per_regs[H8S_IO_TIOR1];
			tier = h8->per_regs[H8S_IO_TIER1];
			tsr  = h8->per_regs[H8S_IO_TSR1];
			tgra = (h8->per_regs[H8S_IO_TGR1A+0] << 8) | (h8->per_regs[H8S_IO_TGR1A+1] << 0);
			tgrb = (h8->per_regs[H8S_IO_TGR1B+0] << 8) | (h8->per_regs[H8S_IO_TGR1B+1] << 0);
//			tcnt = (h8->per_regs[H8S_IO_TTCNT1+0] << 8) | (h8->per_regs[H8S_IO_TTCNT1+1] << 0);

			//printf( "TTCR %02X TIOR %02X TIER %02X TSR %02X TGRA %04X TGRB %04X TCNT %04X\n", ttcr, tior, tier, tsr, tgra, tgrb, tcnt);

			if ((ttcr != 0x21) || (tior != 0x56) || ((tier != 0x00) && (tier != 0x01)) || (tsr != 0x00))
			{
				fatalerror( "TPU: not supported\n");
			}

			// start
			i = 0;
			tpu->item[i].tgr = 0;
			tpu->item[i].out = (i + 1) & 1;
			tpu->item[i].irq = 0;
			// TGRB
			if (tgrb < tgra) i = 1; else i = 2;
			tpu->item[i].tgr = tgrb;
			tpu->item[i].out = (i + 1) & 1;
			if (tier & 2) tpu->item[i].irq = H8S_INT_TGI1B; else tpu->item[i].irq = 0;
			// TGRA
			if (tgrb < tgra) i = 2; else i = 1;
			tpu->item[i].tgr = tgra;
			tpu->item[i].out = (i + 1) & 1;
			if (tier & 1) tpu->item[i].irq = H8S_INT_TGI1A; else tpu->item[i].irq = 0;
			// ...
			tpu->tgrcur = 0;
			tpu->tgrmax = 2;
			tpu->prescaler = TPSC[ttcr & 7];
			cycles = tpu->item[1].tgr - tpu->item[0].tgr;
			// ajust timer
			hz = h8->device->unscaled_clock() / (tpu->prescaler * cycles);
			tpu->timer->adjust(attotime::from_hz(hz), num);
			// output
			set_p1dr_tiocb1(h8, tpu->item[0].out);
			//
			tpu->tgrcur++;
		}
	}
}

void h8s_tpu_stop(h83xx_state *h8, int num)
{
	h8->tpu[num].timer->adjust(attotime::never, num);
}

void h8s_tpu_resync(h83xx_state *h8, UINT8 data)
{
	if (data & 1) h8s_tpu_start(h8, 0); else h8s_tpu_stop(h8, 0);
	if (data & 2) h8s_tpu_start(h8, 1); else h8s_tpu_stop(h8, 1);
	if (data & 4) h8s_tpu_start(h8, 2); else h8s_tpu_stop(h8, 2);
}

/////////////////////////////////
// SERIAL CONTROLLER INTERFACE //
/////////////////////////////////

typedef struct
{
	UINT32 reg_smr, reg_brr, reg_scr, reg_tdr, reg_ssr, reg_rdr;
	UINT32 reg_pdr, reg_port;
	UINT8 port_mask_sck, port_mask_txd, port_mask_rxd;
	UINT8 int_tx, int_rx;
} H8S_SCI_ENTRY;

const H8S_SCI_ENTRY H8S_SCI_TABLE[] =
{
	// SCI 0
	{
		H8S_IO_SMR0, H8S_IO_BRR0, H8S_IO_SCR0, H8S_IO_TDR0, H8S_IO_SSR0, H8S_IO_RDR0,
		H8S_IO_P3DR, H8S_IO_PORT3,
		H8S_P3_SCK0, H8S_P3_TXD0, H8S_P3_RXD0,
		H8S_INT_TXI0, H8S_INT_RXI0
	},
	// SCI 1
	{
		H8S_IO_SMR1, H8S_IO_BRR1, H8S_IO_SCR1, H8S_IO_TDR1, H8S_IO_SSR1, H8S_IO_RDR1,
		H8S_IO_P3DR, H8S_IO_PORT3,
		H8S_P3_SCK1, H8S_P3_TXD1, H8S_P3_RXD1,
		H8S_INT_TXI1, H8S_INT_RXI1
	},
	// SCI 2
	{
		H8S_IO_SMR2, H8S_IO_BRR2, H8S_IO_SCR2, H8S_IO_TDR2, H8S_IO_SSR2, H8S_IO_RDR2,
		H8S_IO_P5DR, H8S_IO_PORT5,
		H8S_P5_SCK2, H8S_P5_TXD2, H8S_P5_RXD2,
		H8S_INT_TXI2, H8S_INT_RXI2
	}
};

const H8S_SCI_ENTRY *h8s_sci_entry( int num)
{
	return &H8S_SCI_TABLE[num];
}

void h8s_sci_init(h83xx_state *h8)
{
	int i;
	for (i=0;i<3;i++)
	{
		#ifdef ENABLE_SCI_TIMER
		h8->sci[i].timer = h8->device->machine().scheduler().timer_alloc(FUNC(h8s_sci_callback), h8);
		h8->sci[i].timer->adjust(attotime::never, i);
		#endif
	}
}

#ifdef ENABLE_SCI_TIMER
static TIMER_CALLBACK(h8s_sci_callback)
{
//	h83xx_state *h8 = (h83xx_state *)ptr;
	_logerror( 2, "h8s_sci_callback (%d)\n", param);
}
#endif

void h8s_sci_start(h83xx_state *h8, int num)
{
	#ifdef ENABLE_SCI_TIMER
	h8->sci[num].timer->adjust(h8->sci[num].bitrate, num);
	#endif
}

void h8s_sci_stop(h83xx_state *h8, int num)
{
	#ifdef ENABLE_SCI_TIMER
	h8->sci[num].timer->adjust(attotime::never, num);
	#endif
}

void h8s_sci_execute(h83xx_state *h8, int num)
{
	UINT8 scr, tdr, ssr, rdr, tsr, rsr, pdr, port;
	int i;
	const H8S_SCI_ENTRY *info = h8s_sci_entry( num);
//	logerror( "h8s_sci_execute(%d)\n", num);
	// load regs
	scr = h8->per_regs[info->reg_scr];
	tdr = h8->per_regs[info->reg_tdr];
	ssr = h8->per_regs[info->reg_ssr];
	rdr = h8->per_regs[info->reg_rdr];
	tsr = 0;
	rsr = 0;
	pdr = h8->per_regs[info->reg_pdr] & (~info->port_mask_sck);
	// move byte from TDR to TSR
	if (scr & H8S_SCR_TE)
	{
		tsr = tdr;
		ssr |= H8S_SSR_TDRE;
	}
	// generate transmit data empty interrupt
	if ((scr & H8S_SCR_TIE) && (ssr & H8S_SSR_TDRE)) h8s2xxx_interrupt_request(h8, info->int_tx);
	// transmit/receive bits
	for (i=0;i<8;i++)
	{
		// write bit
		if (scr & H8S_SCR_TE)
		{
			if (tsr & (1 << i)) pdr = pdr | info->port_mask_txd; else pdr = pdr & (~info->port_mask_txd);
			h8->io->write_byte( H8S_IO_ADDR( info->reg_pdr), pdr);
		}
		// clock high to low
		h8->io->write_byte( H8S_IO_ADDR( info->reg_pdr), pdr | info->port_mask_sck);
		h8->io->write_byte( H8S_IO_ADDR( info->reg_pdr), pdr);
		// read bit
		if (scr & H8S_SCR_RE)
		{
			port = h8->io->read_byte(H8S_IO_ADDR( info->reg_port));
			if (port & info->port_mask_rxd) rsr = rsr | (1 << i);
		}
	}
	// move byte from RSR to RDR
	if (scr & H8S_SCR_RE)
	{
		rdr = rsr;
		//ssr |= H8S_SSR_RDRF;
	}
	// generate receive data full interrupt
	if ((scr & H8S_SCR_RIE) && (ssr & H8S_SSR_RDRF)) h8s2xxx_interrupt_request(h8, info->int_rx);
	// save regs
	h8->per_regs[info->reg_scr] = scr;
	h8->per_regs[info->reg_tdr] = tdr;
	h8->per_regs[info->reg_ssr] = ssr;
	h8->per_regs[info->reg_rdr] = rdr;
}

const double SCR_CKE[] = { 0.5, 2, 8, 32 }; // = 2 ^ ((2 * cke) - 1)
const int SMR_MODE[] = { 64, 8 };

void h8s_sci_rate(h83xx_state *h8, int num)
{
	UINT8 brr, scr, smr, cke, mode;
	UINT32 bitrate;
	const H8S_SCI_ENTRY *info = h8s_sci_entry( num);
	// read regs
	brr = h8->per_regs[info->reg_brr];
	scr = h8->per_regs[info->reg_scr];
	smr = h8->per_regs[info->reg_smr];
	_logerror( 2, "BRR %02X SCR %02X SMR %02X\n", brr, scr, smr);
	// calculate bitrate
	cke = (scr >> 0) & 3;
	mode = (smr >> 7) & 1;
	bitrate = (UINT32)((h8->device->unscaled_clock() / (brr + 1)) / (SMR_MODE[mode] * SCR_CKE[cke]));
	_logerror( 2, "SCI%d bitrate %d\n", num, bitrate);
	// store bitrate
	#ifdef ENABLE_SCI_TIMER
	h8->sci[num].bitrate = bitrate;
	#endif
}

////////////////////////////
// INTERNAL I/O REGISTERS //
////////////////////////////

void h8s2xxx_per_regs_w_byte(h83xx_state *h8, int offset, UINT8 data)
{
	h8->per_regs[offset] = data;
	switch (offset)
	{
		// SCI 0
		case H8S_IO_SSR0  : if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 0); break;
		case H8S_IO_SCR0  : if (data & H8S_SCR_TIE) h8s2xxx_interrupt_request(h8, h8s_sci_entry(0)->int_tx); break;
		case H8S_IO_BRR0  : h8s_sci_rate(h8, 0); h8s_sci_start(h8, 0); break;
		// SCI 1
		case H8S_IO_SSR1  : if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 1); break;
		case H8S_IO_SCR1  :	if (data & H8S_SCR_RIE) h8s2xxx_interrupt_request(h8,  h8s_sci_entry(1)->int_rx); break;
		case H8S_IO_BRR1  : h8s_sci_rate(h8, 1); h8s_sci_start(h8, 1); break;
		// SCI 2
		case H8S_IO_SSR2  : if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 2); break;
		case H8S_IO_SCR2  : if (data & H8S_SCR_TIE) h8s2xxx_interrupt_request(h8,  h8s_sci_entry(2)->int_tx); break;
		case H8S_IO_BRR2  : h8s_sci_rate(h8, 2); h8s_sci_start(h8, 2); break;
		// TMR 0 (8-bit)
		case H8S_IO_TCR0  : h8s_tmr_resync(h8, 0); break;
		case H8S_IO_TCNT0 : h8s_tmr_resync(h8, 0); break;
		// TMR 1
		case H8S_IO_TCR1  : h8s_tmr_resync(h8, 1); break;
		case H8S_IO_TCNT1 : h8s_tmr_resync(h8, 1); break;
		// ports
		case H8S_IO_P3DR  : h8->io->write_byte( H8S_IO_ADDR( offset), data); break;
		case H8S_IO_PFDR  : h8->io->write_byte( H8S_IO_ADDR( offset), data); break;
		case H8S_IO_PFDDR : h8->io->write_byte( H8S_IO_ADDR( offset), data); break;
		// TPU
		case H8S_IO_TSTR  : h8s_tpu_resync(h8, data); break;
	}
}

UINT8 h8s2xxx_per_regs_r_byte(h83xx_state *h8, int offset)
{
	UINT8 data;
	switch (offset)
	{
		// SCI 0
		case H8S_IO_SSR0  : data = H8S_SSR_TDRE | H8S_SSR_TEND; break;
		case H8S_IO_RDR0  : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		// SCI 1
		case H8S_IO_SSR1  : data = H8S_SSR_TDRE | H8S_SSR_TEND; break;
		// SCI 2
		case H8S_IO_SSR2 :
		{
			data = h8->per_regs[offset];
			if (!(h8->per_regs[H8S_IO_SCR2] & H8S_SCR_TE)) data |= H8S_SSR_TDRE;
		}
		break;
		// ports
		case H8S_IO_PORT1 : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		case H8S_IO_PORTF : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		case H8S_IO_P3DR  : data = 0; break; // todo: without this cybiko hangs
		// default
		default : data = h8->per_regs[offset]; break;
	}
	return data;
}

WRITE8_HANDLER( h8s2241_per_regs_w_byte )
{
	h83xx_state *h8 = get_safe_token(&space->device());
	_logerror( 3, "[%08X] h8s2241_per_regs_w_byte (%08X/%02X)\n", h8->pc, H8S_IO_ADDR( offset), data);
	h8s2xxx_per_regs_w_byte(h8, offset, data);
}

READ8_HANDLER( h8s2241_per_regs_r_byte )
{
	h83xx_state *h8 = get_safe_token(&space->device());
	UINT8 data = 0;
	_logerror( 3, "[%08X] h8s2241_per_regs_r_byte (%08X)\n", h8->pc, H8S_IO_ADDR( offset));
	data = h8s2xxx_per_regs_r_byte(h8, offset);
	_logerror( 3, "%02X\n", data);
	return data;
}

WRITE8_HANDLER( h8s2246_per_regs_w_byte )
{
	h83xx_state *h8 = get_safe_token(&space->device());
	_logerror( 3, "[%08X] h8s2246_per_regs_w_byte (%08X/%02X)\n", h8->pc, H8S_IO_ADDR( offset), data);
	h8s2xxx_per_regs_w_byte(h8, offset, data);
}

READ8_HANDLER( h8s2246_per_regs_r_byte )
{
	h83xx_state *h8 = get_safe_token(&space->device());
	UINT8 data=0;
	_logerror( 3, "[%08X] h8s2246_per_regs_r_byte (%08X)\n", h8->pc, H8S_IO_ADDR( offset));
	data = h8s2xxx_per_regs_r_byte(h8, offset);
	_logerror( 3, "%02X\n", data);
	return data;
}

WRITE8_HANDLER( h8s2323_per_regs_w_byte )
{
	h83xx_state *h8 = get_safe_token(&space->device());
	_logerror( 3, "[%08X] h8s2323_per_regs_w_byte (%08X/%02X)\n", h8->pc, H8S_IO_ADDR( offset), data);
	h8s2xxx_per_regs_w_byte(h8, offset, data);
	switch (offset)
	{
		case H8S_IO_DMABCRL :
		{
			if ((data & 0x40) && (data & 0x80))
			{
				UINT32 i, dma_src, dma_dst;
				UINT16 dma_cnt, dma_con;
				int sz;
				dma_src = h8->program->read_dword( H8S_IO_ADDR(H8S_IO_MAR1AH));
				dma_dst = h8->program->read_dword( H8S_IO_ADDR(H8S_IO_MAR1BH));
				dma_cnt = h8->program->read_word( H8S_IO_ADDR(H8S_IO_ETCR1A));
				dma_con = h8->program->read_word( H8S_IO_ADDR(H8S_IO_DMACR1A));
				sz = (dma_con & 0x8000) ? 2 : 1;
				for (i=0;i<dma_cnt;i++)
				{
					if (dma_con & 0x8000) h8->program->write_word( dma_dst, h8->program->read_word( dma_src)); else h8->program->write_byte( dma_dst, h8->program->read_byte( dma_src));
					if (dma_con & 0x2000) { if (dma_con & 0x4000) dma_src -= sz; else dma_src += sz; }
					if (dma_con & 0x0020) { if (dma_con & 0x0040) dma_dst -= sz; else dma_dst += sz; }
				}
				h8->per_regs[H8S_IO_DMABCRL] &= ~0x40;
			}
		}
		break;
	}
}

READ8_HANDLER( h8s2323_per_regs_r_byte )
{
	h83xx_state *h8 = get_safe_token(&space->device());
	UINT8 data=0;
	_logerror( 3, "[%08X] h8s2323_per_regs_r_byte (%08X)\n", h8->pc, H8S_IO_ADDR( offset));
	switch (offset)
	{
		// timer hack for cybiko xtreme (increase timer value after low byte has been read)
		case H8S_IO_TTCNT5 + 1 :
		{
			UINT16 tcnt = (h8->per_regs[H8S_IO_TTCNT5+0] << 8) + (h8->per_regs[H8S_IO_TTCNT5+1] << 0);
			tcnt = tcnt + 1000;
			data = h8->per_regs[offset];
			h8->per_regs[H8S_IO_TTCNT5+0] = (tcnt >> 8) & 0xFF;
			h8->per_regs[H8S_IO_TTCNT5+1] = (tcnt >> 0) & 0xFF;
			break;
		}
		// skip "recharge the batteries"
		case H8S_IO_PORTA : data = h8->per_regs[offset] | (1 << 6); break;
		// default
		default : data = h8s2xxx_per_regs_r_byte(h8, offset); break;
	}
	_logerror( 3, "%02X\n", data);
	return data;
}
