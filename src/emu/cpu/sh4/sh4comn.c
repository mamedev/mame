/*****************************************************************************
 *
 *   sh4comn.c
 *
 *   SH-4 non-specific components
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"

static const int tcnt_div[8] = { 4, 16, 64, 256, 1024, 1, 1, 1 };
static const int rtcnt_div[8] = { 0, 4, 16, 64, 256, 1024, 2048, 4096 };
static const int daysmonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const int dmasize[8] = { 8, 1, 2, 4, 32, 0, 0, 0 };
static const UINT32 exception_priority_default[] = { EXPPRI(1,1,0,0), EXPPRI(1,2,0,1), EXPPRI(1,1,0,2), EXPPRI(1,3,0,3), EXPPRI(1,4,0,4),
	EXPPRI(2,0,0,5), EXPPRI(2,1,0,6), EXPPRI(2,2,0,7), EXPPRI(2,3,0,8), EXPPRI(2,4,0,9), EXPPRI(2,4,0,10), EXPPRI(2,4,0,11), EXPPRI(2,4,0,12),
	EXPPRI(2,5,0,13), EXPPRI(2,5,0,14), EXPPRI(2,6,0,15), EXPPRI(2,6,0,16), EXPPRI(2,7,0,17), EXPPRI(2,7,0,18), EXPPRI(2,8,0,19),
	EXPPRI(2,9,0,20), EXPPRI(2,4,0,21), EXPPRI(2,10,0,22), EXPPRI(3,0,16,SH4_INTC_NMI) };
static const int exception_codes[] = { 0x000, 0x020, 0x000, 0x140, 0x140, 0x1E0, 0x0E0, 0x040, 0x0A0, 0x180, 0x1A0, 0x800, 0x820, 0x0E0,
	0x100, 0x040, 0x060, 0x0A0, 0x0C0, 0x120, 0x080, 0x160, 0x1E0, 0x1C0, 0x200, 0x220, 0x240, 0x260, 0x280, 0x2A0, 0x2C0, 0x2E0, 0x300,
	0x320, 0x340, 0x360, 0x380, 0x3A0, 0x3C0, 0x240, 0x2A0, 0x300, 0x360, 0x600, 0x620, 0x640, 0x660, 0x680, 0x6A0, 0x780, 0x7A0, 0x7C0,
	0x7E0, 0x6C0, 0xB00, 0xB80, 0x400, 0x420, 0x440, 0x460, 0x480, 0x4A0, 0x4C0, 0x4E0, 0x500, 0x520, 0x540, 0x700, 0x720, 0x740, 0x760,
	0x560, 0x580, 0x5A0 };

static const UINT16 tcnt[] = { TCNT0, TCNT1, TCNT2 };
static const UINT16 tcor[] = { TCOR0, TCOR1, TCOR2 };
static const UINT16 tcr[] = { TCR0, TCR1, TCR2 };

INLINE SH4 *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == CPU);
	assert(cpu_get_type(device) == CPU_SH4);
	return (SH4 *)downcast<legacy_cpu_device *>(device)->token();
}

void sh4_change_register_bank(SH4 *sh4, int to)
{
int s;

	if (to) // 0 -> 1
	{
		for (s = 0;s < 8;s++)
		{
			sh4->rbnk[0][s] = sh4->r[s];
			sh4->r[s] = sh4->rbnk[1][s];
		}
	}
	else // 1 -> 0
	{
		for (s = 0;s < 8;s++)
		{
			sh4->rbnk[1][s] = sh4->r[s];
			sh4->r[s] = sh4->rbnk[0][s];
		}
	}
}

void sh4_swap_fp_registers(SH4 *sh4)
{
int s;
UINT32 z;

	for (s = 0;s <= 15;s++)
	{
		z = sh4->fr[s];
		sh4->fr[s] = sh4->xf[s];
		sh4->xf[s] = z;
	}
}

#ifdef LSB_FIRST
void sh4_swap_fp_couples(SH4 *sh4)
{
int s;
UINT32 z;

	for (s = 0;s <= 15;s = s+2)
	{
		z = sh4->fr[s];
		sh4->fr[s] = sh4->fr[s + 1];
		sh4->fr[s + 1] = z;
		z = sh4->xf[s];
		sh4->xf[s] = sh4->xf[s + 1];
		sh4->xf[s + 1] = z;
	}
}
#endif

void sh4_syncronize_register_bank(SH4 *sh4, int to)
{
int s;

	for (s = 0;s < 8;s++)
	{
		sh4->rbnk[to][s] = sh4->r[s];
	}
}

void sh4_default_exception_priorities(SH4 *sh4) // setup default priorities for exceptions
{
int a;

	for (a=0;a <= SH4_INTC_NMI;a++)
		sh4->exception_priority[a] = exception_priority_default[a];
	for (a=SH4_INTC_IRLn0;a <= SH4_INTC_IRLnE;a++)
		sh4->exception_priority[a] = INTPRI(15-(a-SH4_INTC_IRLn0), a);
	sh4->exception_priority[SH4_INTC_IRL0] = INTPRI(13, SH4_INTC_IRL0);
	sh4->exception_priority[SH4_INTC_IRL1] = INTPRI(10, SH4_INTC_IRL1);
	sh4->exception_priority[SH4_INTC_IRL2] = INTPRI(7, SH4_INTC_IRL2);
	sh4->exception_priority[SH4_INTC_IRL3] = INTPRI(4, SH4_INTC_IRL3);
	for (a=SH4_INTC_HUDI;a <= SH4_INTC_ROVI;a++)
		sh4->exception_priority[a] = INTPRI(0, a);
}

void sh4_exception_recompute(SH4 *sh4) // checks if there is any interrupt with high enough priority
{
	int a,z;

	sh4->test_irq = 0;
	if ((!sh4->pending_irq) || ((sh4->sr & BL) && (sh4->exception_requesting[SH4_INTC_NMI] == 0)))
		return;
	z = (sh4->sr >> 4) & 15;
	for (a=0;a <= SH4_INTC_ROVI;a++)
	{
		if (sh4->exception_requesting[a])
			if ((((int)sh4->exception_priority[a] >> 8) & 255) > z)
			{
				sh4->test_irq = 1; // will check for exception at end of instructions
				break;
			}
	}
}

void sh4_exception_request(SH4 *sh4, int exception) // start requesting an exception
{
	if (!sh4->exception_requesting[exception])
	{
		sh4->exception_requesting[exception] = 1;
		sh4->pending_irq++;
		sh4_exception_recompute(sh4);
	}
}

void sh4_exception_unrequest(SH4 *sh4, int exception) // stop requesting an exception
{
	if (sh4->exception_requesting[exception])
	{
		sh4->exception_requesting[exception] = 0;
		sh4->pending_irq--;
		sh4_exception_recompute(sh4);
	}
}

void sh4_exception_checkunrequest(SH4 *sh4, int exception)
{
	if (exception == SH4_INTC_NMI)
		sh4_exception_unrequest(sh4, exception);
	if ((exception == SH4_INTC_DMTE0) || (exception == SH4_INTC_DMTE1) ||
		(exception == SH4_INTC_DMTE2) || (exception == SH4_INTC_DMTE3))
		sh4_exception_unrequest(sh4, exception);
}

void sh4_exception(SH4 *sh4, const char *message, int exception) // handle exception
{
	UINT32 vector;

	if (exception < SH4_INTC_NMI)
		return; // Not yet supported
	if (exception == SH4_INTC_NMI) {
		if ((sh4->sr & BL) && (!(sh4->m[ICR] & 0x200)))
			return;
		sh4->m[ICR] &= ~0x200;
		sh4->m[INTEVT] = 0x1c0;
		vector = 0x600;
		sh4->irq_callback(sh4->device, INPUT_LINE_NMI);
		LOG(("SH-4 '%s' nmi exception after [%s]\n", sh4->device->tag(), message));
	} else {
//      if ((sh4->m[ICR] & 0x4000) && (sh4->nmi_line_state == ASSERT_LINE))
//          return;
		if (sh4->sr & BL)
			return;
		if (((sh4->exception_priority[exception] >> 8) & 255) <= ((sh4->sr >> 4) & 15))
			return;
		sh4->m[INTEVT] = exception_codes[exception];
		vector = 0x600;
		if ((exception >= SH4_INTC_IRL0) && (exception <= SH4_INTC_IRL3))
			sh4->irq_callback(sh4->device, SH4_INTC_IRL0-exception+SH4_IRL0);
		else
			sh4->irq_callback(sh4->device, SH4_IRL3+1);
		LOG(("SH-4 '%s' interrupt exception #%d after [%s]\n", sh4->device->tag(), exception, message));
	}
	sh4_exception_checkunrequest(sh4, exception);

	sh4->spc = sh4->pc;
	sh4->ssr = sh4->sr;
	sh4->sgr = sh4->r[15];

	sh4->sr |= MD;
	if ((sh4->device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank(sh4, (sh4->sr & sRB) >> 29);
	if (!(sh4->sr & sRB))
		sh4_change_register_bank(sh4, 1);
	sh4->sr |= sRB;
	sh4->sr |= BL;
	sh4_exception_recompute(sh4);

	/* fetch PC */
	sh4->pc = sh4->vbr + vector;
	/* wake up if a sleep opcode is triggered */
	if(sh4->sleep_mode == 1) { sh4->sleep_mode = 2; }
}

static UINT32 compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor)
{
	// elapsed:total = x : ticks
	// x=elapsed*tics/total -> x=elapsed*(double)100000000/rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]
	// ticks/total=ticks / ((rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] * ticks) / 100000000)=1/((rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] / 100000000)=100000000/rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]
	return base + (UINT32)((attotime_to_double(timer_timeelapsed(timer)) * (double)hertz) / (double)divisor);
}

static void sh4_refresh_timer_recompute(SH4 *sh4)
{
UINT32 ticks;

	//if rtcnt < rtcor then rtcor-rtcnt
	//if rtcnt >= rtcor then 256-rtcnt+rtcor=256+rtcor-rtcnt
	ticks = sh4->m[RTCOR]-sh4->m[RTCNT];
	if (ticks <= 0)
		ticks = 256 + ticks;
	timer_adjust_oneshot(sh4->refresh_timer, attotime_mul(attotime_mul(ATTOTIME_IN_HZ(sh4->bus_clock), rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]), ticks), 0);
	sh4->refresh_timer_base = sh4->m[RTCNT];
}

/*-------------------------------------------------
    sh4_scale_up_mame_time - multiply a attotime by
    a (constant+1) where 0 <= constant < 2^32
-------------------------------------------------*/

INLINE attotime sh4_scale_up_mame_time(attotime _time1, UINT32 factor1)
{
	return attotime_add(attotime_mul(_time1, factor1), _time1);
}

static UINT32 compute_ticks_timer(emu_timer *timer, int hertz, int divisor)
{
	double ret;

	ret=((attotime_to_double(timer_timeleft(timer)) * (double)hertz) / (double)divisor) - 1;
	return (UINT32)ret;
}

static void sh4_timer_recompute(SH4 *sh4, int which)
{
	double ticks;

	ticks = sh4->m[tcnt[which]];
	timer_adjust_oneshot(sh4->timer[which], sh4_scale_up_mame_time(attotime_mul(ATTOTIME_IN_HZ(sh4->pm_clock), tcnt_div[sh4->m[tcr[which]] & 7]), ticks), which);
}

static TIMER_CALLBACK( sh4_refresh_timer_callback )
{
	SH4 *sh4 = (SH4 *)ptr;

	sh4->m[RTCNT] = 0;
	sh4_refresh_timer_recompute(sh4);
	sh4->m[RTCSR] |= 128;
	if ((sh4->m[MCR] & 4) && !(sh4->m[MCR] & 2))
	{
		sh4->m[RFCR] = (sh4->m[RFCR] + 1) & 1023;
		if (((sh4->m[RTCSR] & 1) && (sh4->m[RFCR] == 512)) || (sh4->m[RFCR] == 0))
		{
			sh4->m[RFCR] = 0;
			sh4->m[RTCSR] |= 4;
		}
	}
}

static void increment_rtc_time(SH4 *sh4, int mode)
{
	int carry, year, leap, days;

	if (mode == 0)
	{
		carry = 0;
		sh4->m[RSECCNT] = sh4->m[RSECCNT] + 1;
		if ((sh4->m[RSECCNT] & 0xf) == 0xa)
			sh4->m[RSECCNT] = sh4->m[RSECCNT] + 6;
		if (sh4->m[RSECCNT] == 0x60)
		{
			sh4->m[RSECCNT] = 0;
			carry=1;
		}
		else
			return;
	}
	else
		carry = 1;

	sh4->m[RMINCNT] = sh4->m[RMINCNT] + carry;
	if ((sh4->m[RMINCNT] & 0xf) == 0xa)
		sh4->m[RMINCNT] = sh4->m[RMINCNT] + 6;
	carry=0;
	if (sh4->m[RMINCNT] == 0x60)
	{
		sh4->m[RMINCNT] = 0;
		carry = 1;
	}

	sh4->m[RHRCNT] = sh4->m[RHRCNT] + carry;
	if ((sh4->m[RHRCNT] & 0xf) == 0xa)
		sh4->m[RHRCNT] = sh4->m[RHRCNT] + 6;
	carry = 0;
	if (sh4->m[RHRCNT] == 0x24)
	{
		sh4->m[RHRCNT] = 0;
		carry = 1;
	}

	sh4->m[RWKCNT] = sh4->m[RWKCNT] + carry;
	if (sh4->m[RWKCNT] == 0x7)
	{
		sh4->m[RWKCNT] = 0;
	}

	days = 0;
	year = (sh4->m[RYRCNT] & 0xf) + ((sh4->m[RYRCNT] & 0xf0) >> 4)*10 + ((sh4->m[RYRCNT] & 0xf00) >> 8)*100 + ((sh4->m[RYRCNT] & 0xf000) >> 12)*1000;
	leap = 0;
	if (!(year%100))
	{
		if (!(year%400))
			leap = 1;
	}
	else if (!(year%4))
		leap = 1;
	if (sh4->m[RMONCNT] != 2)
		leap = 0;
	if (sh4->m[RMONCNT])
		days = daysmonth[(sh4->m[RMONCNT] & 0xf) + ((sh4->m[RMONCNT] & 0xf0) >> 4)*10 - 1];

	sh4->m[RDAYCNT] = sh4->m[RDAYCNT] + carry;
	if ((sh4->m[RDAYCNT] & 0xf) == 0xa)
		sh4->m[RDAYCNT] = sh4->m[RDAYCNT] + 6;
	carry = 0;
	if (sh4->m[RDAYCNT] > (days+leap))
	{
		sh4->m[RDAYCNT] = 1;
		carry = 1;
	}

	sh4->m[RMONCNT] = sh4->m[RMONCNT] + carry;
	if ((sh4->m[RMONCNT] & 0xf) == 0xa)
		sh4->m[RMONCNT] = sh4->m[RMONCNT] + 6;
	carry=0;
	if (sh4->m[RMONCNT] == 0x13)
	{
		sh4->m[RMONCNT] = 1;
		carry = 1;
	}

	sh4->m[RYRCNT] = sh4->m[RYRCNT] + carry;
	if ((sh4->m[RYRCNT] & 0xf) >= 0xa)
		sh4->m[RYRCNT] = sh4->m[RYRCNT] + 6;
	if ((sh4->m[RYRCNT] & 0xf0) >= 0xa0)
		sh4->m[RYRCNT] = sh4->m[RYRCNT] + 0x60;
	if ((sh4->m[RYRCNT] & 0xf00) >= 0xa00)
		sh4->m[RYRCNT] = sh4->m[RYRCNT] + 0x600;
	if ((sh4->m[RYRCNT] & 0xf000) >= 0xa000)
		sh4->m[RYRCNT] = 0;
}

static TIMER_CALLBACK( sh4_rtc_timer_callback )
{
	SH4 *sh4 = (SH4 *)ptr;

	timer_adjust_oneshot(sh4->rtc_timer, ATTOTIME_IN_HZ(128), 0);
	sh4->m[R64CNT] = (sh4->m[R64CNT]+1) & 0x7f;
	if (sh4->m[R64CNT] == 64)
	{
		sh4->m[RCR1] |= 0x80;
		increment_rtc_time(sh4, 0);
		//sh4_exception_request(sh4, SH4_INTC_NMI); // TEST
	}
}

static TIMER_CALLBACK( sh4_timer_callback )
{
	static const UINT16 tuni[] = { SH4_INTC_TUNI0, SH4_INTC_TUNI1, SH4_INTC_TUNI2 };
	SH4 *sh4 = (SH4 *)ptr;
	int which = param;
	int idx = tcr[which];

	sh4->m[tcnt[which]] = sh4->m[tcor[which]];
	sh4_timer_recompute(sh4, which);
	sh4->m[idx] = sh4->m[idx] | 0x100;
	if (sh4->m[idx] & 0x20)
		sh4_exception_request(sh4, tuni[which]);
}

static TIMER_CALLBACK( sh4_dmac_callback )
{
	SH4 *sh4 = (SH4 *)ptr;
	int channel = param;

	LOG(("SH4 '%s': DMA %d complete\n", sh4->device->tag(), channel));
	sh4->dma_timer_active[channel] = 0;
	switch (channel)
	{
	case 0:
		sh4->m[DMATCR0] = 0;
		sh4->m[CHCR0] |= 2;
		if (sh4->m[CHCR0] & 4)
			sh4_exception_request(sh4, SH4_INTC_DMTE0);
		break;
	case 1:
		sh4->m[DMATCR1] = 0;
		sh4->m[CHCR1] |= 2;
		if (sh4->m[CHCR1] & 4)
			sh4_exception_request(sh4, SH4_INTC_DMTE1);
		break;
	case 2:
		sh4->m[DMATCR2] = 0;
		sh4->m[CHCR2] |= 2;
		if (sh4->m[CHCR2] & 4)
			sh4_exception_request(sh4, SH4_INTC_DMTE2);
		break;
	case 3:
		sh4->m[DMATCR3] = 0;
		sh4->m[CHCR3] |= 2;
		if (sh4->m[CHCR3] & 4)
			sh4_exception_request(sh4, SH4_INTC_DMTE3);
		break;
	}
}

static int sh4_dma_transfer(SH4 *sh4, int channel, int timermode, UINT32 chcr, UINT32 *sar, UINT32 *dar, UINT32 *dmatcr)
{
	int incs, incd, size;
	UINT32 src, dst, count;

	incd = (chcr >> 14) & 3;
	incs = (chcr >> 12) & 3;
	size = dmasize[(chcr >> 4) & 7];
	if(incd == 3 || incs == 3)
	{
		logerror("SH4: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, chcr);
		return 0;
	}
	src   = *sar;
	dst   = *dar;
	count = *dmatcr;
	if (!count)
		count = 0x1000000;

	LOG(("SH4: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", channel, src, dst, count, chcr, incs, incd, size));

	if (timermode == 1)
	{
		sh4->dma_timer_active[channel] = 1;
		timer_adjust_oneshot(sh4->dma_timer[channel], cpu_clocks_to_attotime(sh4->device, 2*count+1), channel);
	}
	else if (timermode == 2)
	{
		sh4->dma_timer_active[channel] = 1;
		timer_adjust_oneshot(sh4->dma_timer[channel], attotime_zero, channel);
	}

	src &= AM;
	dst &= AM;

	switch(size)
	{
	case 1: // 8 bit
		for(;count > 0; count --)
		{
			if(incs == 2)
				src --;
			if(incd == 2)
				dst --;
			memory_write_byte_64le(sh4->program, dst, memory_read_byte_64le(sh4->program, src));
			if(incs == 1)
				src ++;
			if(incd == 1)
				dst ++;
		}
		break;
	case 2: // 16 bit
		src &= ~1;
		dst &= ~1;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 2;
			if(incd == 2)
				dst -= 2;
			memory_write_word_64le(sh4->program, dst, memory_read_word_64le(sh4->program, src));
			if(incs == 1)
				src += 2;
			if(incd == 1)
				dst += 2;
		}
		break;
	case 8: // 64 bit
		src &= ~7;
		dst &= ~7;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 8;
			if(incd == 2)
				dst -= 8;
			memory_write_qword_64le(sh4->program, dst, memory_read_qword_64le(sh4->program, src));
			if(incs == 1)
				src += 8;
			if(incd == 1)
				dst += 8;

		}
		break;
	case 4: // 32 bit
		src &= ~3;
		dst &= ~3;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 4;
			if(incd == 2)
				dst -= 4;
			memory_write_dword_64le(sh4->program, dst, memory_read_dword_64le(sh4->program, src));
			if(incs == 1)
				src += 4;
			if(incd == 1)
				dst += 4;

		}
		break;
	case 32:
		src &= ~31;
		dst &= ~31;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 32;
			if(incd == 2)
				dst -= 32;
			memory_write_qword_64le(sh4->program, dst, memory_read_qword_64le(sh4->program, src));
			memory_write_qword_64le(sh4->program, dst+8, memory_read_qword_64le(sh4->program, src+8));
			memory_write_qword_64le(sh4->program, dst+16, memory_read_qword_64le(sh4->program, src+16));
			memory_write_qword_64le(sh4->program, dst+24, memory_read_qword_64le(sh4->program, src+24));
			if(incs == 1)
				src += 32;
			if(incd == 1)
				dst += 32;
		}
		break;
	}
	*sar    = (*sar & !AM) | src;
	*dar    = (*dar & !AM) | dst;
	*dmatcr = count;
	return 1;
}

static void sh4_dmac_check(SH4 *sh4, int channel)
{
UINT32 dmatcr,chcr,sar,dar;

	switch (channel)
	{
	case 0:
		sar = sh4->m[SAR0];
		dar = sh4->m[DAR0];
		chcr = sh4->m[CHCR0];
		dmatcr = sh4->m[DMATCR0];
		break;
	case 1:
		sar = sh4->m[SAR1];
		dar = sh4->m[DAR1];
		chcr = sh4->m[CHCR1];
		dmatcr = sh4->m[DMATCR1];
		break;
	case 2:
		sar = sh4->m[SAR2];
		dar = sh4->m[DAR2];
		chcr = sh4->m[CHCR2];
		dmatcr = sh4->m[DMATCR2];
		break;
	case 3:
		sar = sh4->m[SAR3];
		dar = sh4->m[DAR3];
		chcr = sh4->m[CHCR3];
		dmatcr = sh4->m[DMATCR3];
		break;
	default:
		return;
	}
	if (chcr & sh4->m[DMAOR] & 1)
	{
		if ((((chcr >> 8) & 15) < 4) || (((chcr >> 8) & 15) > 6))
			return;
		if (!sh4->dma_timer_active[channel] && !(chcr & 2) && !(sh4->m[DMAOR] & 6))
			sh4_dma_transfer(sh4, channel, 1, chcr, &sar, &dar, &dmatcr);
	}
	else
	{
		if (sh4->dma_timer_active[channel])
		{
			logerror("SH4: DMA %d cancelled in-flight but all data transferred", channel);
			timer_adjust_oneshot(sh4->dma_timer[channel], attotime_never, channel);
			sh4->dma_timer_active[channel] = 0;
		}
	}
}

static void sh4_dmac_nmi(SH4 *sh4) // manage dma when nmi
{
int s;

	sh4->m[DMAOR] |= 2; // nmif = 1
	for (s = 0;s < 4;s++)
	{
		if (sh4->dma_timer_active[s])
		{
			logerror("SH4: DMA %d cancelled due to NMI but all data transferred", s);
			timer_adjust_oneshot(sh4->dma_timer[s], attotime_never, s);
			sh4->dma_timer_active[s] = 0;
		}
	}
}

WRITE32_HANDLER( sh4_internal_w )
{
	SH4 *sh4 = get_safe_token(space->cpu);
	int a;
	UINT32 old = sh4->m[offset];
	COMBINE_DATA(sh4->m+offset);

	//  logerror("sh4_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, data, mem_mask);

	switch( offset )
	{
	case MMUCR: // MMU Control
		if (data & 1)
		{
			printf("SH4 MMU Enabled\n");
			printf("If you're seeing this, but running something other than a Naomi GD-ROM game then chances are it won't work\n");
			printf("The MMU emulation is a hack specific to that system\n");
			sh4->sh4_mmu_enabled = 1;

			// should be a different bit!
			{
				int i;
				for (i=0;i<64;i++)
				{
					sh4->sh4_tlb_address[i] = 0;
					sh4->sh4_tlb_data[i] = 0;
				}

			}
		}
		else
		{
			sh4->sh4_mmu_enabled = 0;
		}

		break;

		// Memory refresh
	case RTCSR:
		sh4->m[RTCSR] &= 255;
		if ((old >> 3) & 7)
			sh4->m[RTCNT] = compute_ticks_refresh_timer(sh4->refresh_timer, sh4->bus_clock, sh4->refresh_timer_base, rtcnt_div[(old >> 3) & 7]) & 0xff;
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // activated
			sh4_refresh_timer_recompute(sh4);
		}
		else
		{
			timer_adjust_oneshot(sh4->refresh_timer, attotime_never, 0);
		}
		break;

	case RTCNT:
		sh4->m[RTCNT] &= 255;
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // active
			sh4_refresh_timer_recompute(sh4);
		}
		break;

	case RTCOR:
		sh4->m[RTCOR] &= 255;
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // active
			sh4->m[RTCNT] = compute_ticks_refresh_timer(sh4->refresh_timer, sh4->bus_clock, sh4->refresh_timer_base, rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]) & 0xff;
			sh4_refresh_timer_recompute(sh4);
		}
		break;

	case RFCR:
		sh4->m[RFCR] &= 1023;
		break;

		// RTC
	case RCR1:
		if ((sh4->m[RCR1] & 8) && (~old & 8)) // 0 -> 1
			sh4->m[RCR1] ^= 1;
		break;

	case RCR2:
		if (sh4->m[RCR2] & 2)
		{
			sh4->m[R64CNT] = 0;
			sh4->m[RCR2] ^= 2;
		}
		if (sh4->m[RCR2] & 4)
		{
			sh4->m[R64CNT] = 0;
			if (sh4->m[RSECCNT] >= 30)
				increment_rtc_time(sh4, 1);
			sh4->m[RSECCNT] = 0;
		}
		if ((sh4->m[RCR2] & 8) && (~old & 8))
		{ // 0 -> 1
			timer_adjust_oneshot(sh4->rtc_timer, ATTOTIME_IN_HZ(128), 0);
		}
		else if (~(sh4->m[RCR2]) & 8)
		{ // 0
			timer_adjust_oneshot(sh4->rtc_timer, attotime_never, 0);
		}
		break;

		// TMU
	case TSTR:
		if (old & 1)
			sh4->m[TCNT0] = compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[sh4->m[TCR0] & 7]);
		if ((sh4->m[TSTR] & 1) == 0) {
			timer_adjust_oneshot(sh4->timer[0], attotime_never, 0);
		} else
			sh4_timer_recompute(sh4, 0);

		if (old & 2)
			sh4->m[TCNT1] = compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[sh4->m[TCR1] & 7]);
		if ((sh4->m[TSTR] & 2) == 0) {
			timer_adjust_oneshot(sh4->timer[1], attotime_never, 0);
		} else
			sh4_timer_recompute(sh4, 1);

		if (old & 4)
			sh4->m[TCNT2] = compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[sh4->m[TCR2] & 7]);
		if ((sh4->m[TSTR] & 4) == 0) {
			timer_adjust_oneshot(sh4->timer[2], attotime_never, 0);
		} else
			sh4_timer_recompute(sh4, 2);
		break;

	case TCR0:
		if (sh4->m[TSTR] & 1)
		{
			sh4->m[TCNT0] = compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[old & 7]);
			sh4_timer_recompute(sh4, 0);
		}
		if (!(sh4->m[TCR0] & 0x20) || !(sh4->m[TCR0] & 0x100))
			sh4_exception_unrequest(sh4, SH4_INTC_TUNI0);
		break;
	case TCR1:
		if (sh4->m[TSTR] & 2)
		{
			sh4->m[TCNT1] = compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[old & 7]);
			sh4_timer_recompute(sh4, 1);
		}
		if (!(sh4->m[TCR1] & 0x20) || !(sh4->m[TCR1] & 0x100))
			sh4_exception_unrequest(sh4, SH4_INTC_TUNI1);
		break;
	case TCR2:
		if (sh4->m[TSTR] & 4)
		{
			sh4->m[TCNT2] = compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[old & 7]);
			sh4_timer_recompute(sh4, 2);
		}
		if (!(sh4->m[TCR2] & 0x20) || !(sh4->m[TCR2] & 0x100))
			sh4_exception_unrequest(sh4, SH4_INTC_TUNI2);
		break;

	case TCOR0:
		if (sh4->m[TSTR] & 1)
		{
			sh4->m[TCNT0] = compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[sh4->m[TCR0] & 7]);
			sh4_timer_recompute(sh4, 0);
		}
		break;
	case TCNT0:
		if (sh4->m[TSTR] & 1)
			sh4_timer_recompute(sh4, 0);
		break;
	case TCOR1:
		if (sh4->m[TSTR] & 2)
		{
			sh4->m[TCNT1] = compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[sh4->m[TCR1] & 7]);
			sh4_timer_recompute(sh4, 1);
		}
		break;
	case TCNT1:
		if (sh4->m[TSTR] & 2)
			sh4_timer_recompute(sh4, 1);
		break;
	case TCOR2:
		if (sh4->m[TSTR] & 4)
		{
			sh4->m[TCNT2] = compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[sh4->m[TCR2] & 7]);
			sh4_timer_recompute(sh4, 2);
		}
		break;
	case TCNT2:
		if (sh4->m[TSTR] & 4)
			sh4_timer_recompute(sh4, 2);
		break;

		// INTC
	case ICR:
		sh4->m[ICR] = (sh4->m[ICR] & 0x7fff) | (old & 0x8000);
		break;
	case IPRA:
		sh4->exception_priority[SH4_INTC_ATI] = INTPRI(sh4->m[IPRA] & 0x000f, SH4_INTC_ATI);
		sh4->exception_priority[SH4_INTC_PRI] = INTPRI(sh4->m[IPRA] & 0x000f, SH4_INTC_PRI);
		sh4->exception_priority[SH4_INTC_CUI] = INTPRI(sh4->m[IPRA] & 0x000f, SH4_INTC_CUI);
		sh4->exception_priority[SH4_INTC_TUNI2] = INTPRI((sh4->m[IPRA] & 0x00f0) >> 4, SH4_INTC_TUNI2);
		sh4->exception_priority[SH4_INTC_TICPI2] = INTPRI((sh4->m[IPRA] & 0x00f0) >> 4, SH4_INTC_TICPI2);
		sh4->exception_priority[SH4_INTC_TUNI1] = INTPRI((sh4->m[IPRA] & 0x0f00) >> 8, SH4_INTC_TUNI1);
		sh4->exception_priority[SH4_INTC_TUNI0] = INTPRI((sh4->m[IPRA] & 0xf000) >> 12, SH4_INTC_TUNI0);
		sh4_exception_recompute(sh4);
		break;
	case IPRB:
		sh4->exception_priority[SH4_INTC_SCI1ERI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1ERI);
		sh4->exception_priority[SH4_INTC_SCI1RXI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1RXI);
		sh4->exception_priority[SH4_INTC_SCI1TXI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TXI);
		sh4->exception_priority[SH4_INTC_SCI1TEI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TEI);
		sh4->exception_priority[SH4_INTC_RCMI] = INTPRI((sh4->m[IPRB] & 0x0f00) >> 8, SH4_INTC_RCMI);
		sh4->exception_priority[SH4_INTC_ROVI] = INTPRI((sh4->m[IPRB] & 0x0f00) >> 8, SH4_INTC_ROVI);
		sh4->exception_priority[SH4_INTC_ITI] = INTPRI((sh4->m[IPRB] & 0xf000) >> 12, SH4_INTC_ITI);
		sh4_exception_recompute(sh4);
		break;
	case IPRC:
		sh4->exception_priority[SH4_INTC_HUDI] = INTPRI(sh4->m[IPRC] & 0x000f, SH4_INTC_HUDI);
		sh4->exception_priority[SH4_INTC_SCIFERI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFERI);
		sh4->exception_priority[SH4_INTC_SCIFRXI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFRXI);
		sh4->exception_priority[SH4_INTC_SCIFBRI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFBRI);
		sh4->exception_priority[SH4_INTC_SCIFTXI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFTXI);
		sh4->exception_priority[SH4_INTC_DMTE0] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE0);
		sh4->exception_priority[SH4_INTC_DMTE1] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE1);
		sh4->exception_priority[SH4_INTC_DMTE2] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE2);
		sh4->exception_priority[SH4_INTC_DMTE3] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE3);
		sh4->exception_priority[SH4_INTC_DMAE] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMAE);
		sh4->exception_priority[SH4_INTC_GPOI] = INTPRI((sh4->m[IPRC] & 0xf000) >> 12, SH4_INTC_GPOI);
		sh4_exception_recompute(sh4);
		break;

		// DMA
	case SAR0:
	case SAR1:
	case SAR2:
	case SAR3:
	case DAR0:
	case DAR1:
	case DAR2:
	case DAR3:
	case DMATCR0:
	case DMATCR1:
	case DMATCR2:
	case DMATCR3:
		break;
	case CHCR0:
		sh4_dmac_check(sh4, 0);
		break;
	case CHCR1:
		sh4_dmac_check(sh4, 1);
		break;
	case CHCR2:
		sh4_dmac_check(sh4, 2);
		break;
	case CHCR3:
		sh4_dmac_check(sh4, 3);
		break;
	case DMAOR:
		if ((sh4->m[DMAOR] & 4) && (~old & 4))
			sh4->m[DMAOR] &= ~4;
		if ((sh4->m[DMAOR] & 2) && (~old & 2))
			sh4->m[DMAOR] &= ~2;
		sh4_dmac_check(sh4, 0);
		sh4_dmac_check(sh4, 1);
		sh4_dmac_check(sh4, 2);
		sh4_dmac_check(sh4, 3);
		break;

		// Store Queues
	case QACR0:
	case QACR1:
		break;

		// I/O ports
	case PCTRA:
		sh4->ioport16_pullup = 0;
		sh4->ioport16_direction = 0;
		for (a=0;a < 16;a++) {
			sh4->ioport16_direction |= (sh4->m[PCTRA] & (1 << (a*2))) >> a;
			sh4->ioport16_pullup |= (sh4->m[PCTRA] & (1 << (a*2+1))) >> (a+1);
		}
		sh4->ioport16_direction &= 0xffff;
		sh4->ioport16_pullup = (sh4->ioport16_pullup | sh4->ioport16_direction) ^ 0xffff;
		if (sh4->m[BCR2] & 1)
			memory_write_dword_64le(sh4->io, SH4_IOPORT_16, (UINT64)(sh4->m[PDTRA] & sh4->ioport16_direction) | ((UINT64)sh4->m[PCTRA] << 16));
		break;
	case PDTRA:
		if (sh4->m[BCR2] & 1)
			memory_write_dword_64le(sh4->io, SH4_IOPORT_16, (UINT64)(sh4->m[PDTRA] & sh4->ioport16_direction) | ((UINT64)sh4->m[PCTRA] << 16));
		break;
	case PCTRB:
		sh4->ioport4_pullup = 0;
		sh4->ioport4_direction = 0;
		for (a=0;a < 4;a++) {
			sh4->ioport4_direction |= (sh4->m[PCTRB] & (1 << (a*2))) >> a;
			sh4->ioport4_pullup |= (sh4->m[PCTRB] & (1 << (a*2+1))) >> (a+1);
		}
		sh4->ioport4_direction &= 0xf;
		sh4->ioport4_pullup = (sh4->ioport4_pullup | sh4->ioport4_direction) ^ 0xf;
		if (sh4->m[BCR2] & 1)
			memory_write_dword_64le(sh4->io, SH4_IOPORT_4, (sh4->m[PDTRB] & sh4->ioport4_direction) | (sh4->m[PCTRB] << 16));
		break;
	case PDTRB:
		if (sh4->m[BCR2] & 1)
			memory_write_dword_64le(sh4->io, SH4_IOPORT_4, (sh4->m[PDTRB] & sh4->ioport4_direction) | (sh4->m[PCTRB] << 16));
		break;

	case SCBRR2:
		break;

	default:
		logerror("sh4_internal_w:  Unmapped write %08x, %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), data, mem_mask);
		break;
	}
}

READ32_HANDLER( sh4_internal_r )
{
	SH4 *sh4 = get_safe_token(space->cpu);
	//  logerror("sh4_internal_r:  Read %08x (%x) @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, mem_mask);
	switch( offset )
	{
	case RTCNT:
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // activated
			//((double)rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] / (double)100000000)
			//return (refresh_timer_base + (timer_timeelapsed(sh4->refresh_timer) * (double)100000000) / (double)rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]) & 0xff;
			return compute_ticks_refresh_timer(sh4->refresh_timer, sh4->bus_clock, sh4->refresh_timer_base, rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]) & 0xff;
		}
		else
			return sh4->m[RTCNT];
		break;

	case TCNT0:
		if (sh4->m[TSTR] & 1)
			return compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[sh4->m[TCR0] & 7]);
		else
			return sh4->m[TCNT0];
		break;
	case TCNT1:
		if (sh4->m[TSTR] & 2)
			return compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[sh4->m[TCR1] & 7]);
		else
			return sh4->m[TCNT1];
		break;
	case TCNT2:
		if (sh4->m[TSTR] & 4)
			return compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[sh4->m[TCR2] & 7]);
		else
			return sh4->m[TCNT2];
		break;

		// I/O ports
	case PDTRA:
		if (sh4->m[BCR2] & 1)
			return (memory_read_dword_64le(sh4->io, SH4_IOPORT_16) & ~sh4->ioport16_direction) | (sh4->m[PDTRA] & sh4->ioport16_direction);
		break;
	case PDTRB:
		if (sh4->m[BCR2] & 1)
			return (memory_read_dword_64le(sh4->io, SH4_IOPORT_4) & ~sh4->ioport4_direction) | (sh4->m[PDTRB] & sh4->ioport4_direction);
		break;

		// SCIF (UART with FIFO)
	case SCFSR2:
		return 0x60; //read-only status register
	}
	return sh4->m[offset];
}

void sh4_set_frt_input(running_device *device, int state)
{
	SH4 *sh4 = get_safe_token(device);

	if(state == PULSE_LINE)
	{
		sh4_set_frt_input(device, ASSERT_LINE);
		sh4_set_frt_input(device, CLEAR_LINE);
		return;
	}

	if(sh4->frt_input == state) {
		return;
	}

	sh4->frt_input = state;

	if(sh4->m[5] & 0x8000) {
		if(state == CLEAR_LINE) {
			return;
		}
	} else {
		if(state == ASSERT_LINE) {
			return;
		}
	}

#if 0
	sh4_timer_resync();
	sh4->icr = sh4->frc;
	sh4->m[4] |= ICF;
	logerror("SH4 '%s': ICF activated (%x)\n", sh4->device->tag(), sh4->pc & AM);
	sh4_recalc_irq();
#endif
}

void sh4_set_irln_input(running_device *device, int value)
{
	SH4 *sh4 = get_safe_token(device);

	if (sh4->irln == value)
		return;
	sh4->irln = value;
	cpu_set_input_line(device, SH4_IRLn, ASSERT_LINE);
	cpu_set_input_line(device, SH4_IRLn, CLEAR_LINE);
}

void sh4_set_irq_line(SH4 *sh4, int irqline, int state) // set state of external interrupt line
{
	int s;

	if (irqline == INPUT_LINE_NMI)
    {
		if (sh4->nmi_line_state == state)
			return;
		if (sh4->m[ICR] & 0x100)
		{
			if ((state == CLEAR_LINE) && (sh4->nmi_line_state == ASSERT_LINE))  // rising
			{
				LOG(("SH-4 '%s' assert nmi\n", sh4->device->tag()));
				sh4_exception_request(sh4, SH4_INTC_NMI);
				sh4_dmac_nmi(sh4);
			}
		}
		else
		{
			if ((state == ASSERT_LINE) && (sh4->nmi_line_state == CLEAR_LINE)) // falling
			{
				LOG(("SH-4 '%s' assert nmi\n", sh4->device->tag()));
				sh4_exception_request(sh4, SH4_INTC_NMI);
				sh4_dmac_nmi(sh4);
			}
		}
		if (state == CLEAR_LINE)
			sh4->m[ICR] ^= 0x8000;
		else
			sh4->m[ICR] |= 0x8000;
		sh4->nmi_line_state = state;
	}
	else
	{
		if (sh4->m[ICR] & 0x80) // four independent external interrupt sources
		{
			if (irqline > SH4_IRL3)
				return;
			if (sh4->irq_line_state[irqline] == state)
				return;
			sh4->irq_line_state[irqline] = state;

			if( state == CLEAR_LINE )
			{
				LOG(("SH-4 '%s' cleared external irq IRL%d\n", sh4->device->tag(), irqline));
				sh4_exception_unrequest(sh4, SH4_INTC_IRL0+irqline-SH4_IRL0);
			}
			else
			{
				LOG(("SH-4 '%s' assert external irq IRL%d\n", sh4->device->tag(), irqline));
				sh4_exception_request(sh4, SH4_INTC_IRL0+irqline-SH4_IRL0);
			}
		}
		else // level-encoded interrupt
		{
			if (irqline != SH4_IRLn)
				return;
			if ((sh4->irln > 15) || (sh4->irln < 0))
				return;
			for (s = 0; s < 15; s++)
				sh4_exception_unrequest(sh4, SH4_INTC_IRLn0+s);
			if (sh4->irln < 15)
				sh4_exception_request(sh4, SH4_INTC_IRLn0+sh4->irln);
			LOG(("SH-4 '%s' IRLn0-IRLn3 level #%d\n", sh4->device->tag(), sh4->irln));
		}
	}
	if (sh4->test_irq && (!sh4->delay))
		sh4_check_pending_irq(sh4, "sh4_set_irq_line");
}

void sh4_parse_configuration(SH4 *sh4, const struct sh4_config *conf)
{
	if(conf)
	{
		switch((conf->md2 << 2) | (conf->md1 << 1) | (conf->md0))
		{
		case 0:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 4;
			sh4->pm_clock = conf->clock / 4;
			break;
		case 1:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 6;
			sh4->pm_clock = conf->clock / 6;
			break;
		case 2:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 3;
			sh4->pm_clock = conf->clock / 6;
			break;
		case 3:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 3;
			sh4->pm_clock = conf->clock / 6;
			break;
		case 4:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 2;
			sh4->pm_clock = conf->clock / 4;
			break;
		case 5:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 2;
			sh4->pm_clock = conf->clock / 4;
			break;
		}
		sh4->is_slave = (~(conf->md7)) & 1;
	}
	else
	{
		sh4->cpu_clock = 200000000;
		sh4->bus_clock = 100000000;
		sh4->pm_clock = 50000000;
		sh4->is_slave = 0;
	}
}

void sh4_common_init(running_device *device)
{
	SH4 *sh4 = get_safe_token(device);
	int i;

	for (i=0; i<3; i++)
	{
		sh4->timer[i] = timer_alloc(device->machine, sh4_timer_callback, sh4);
		timer_adjust_oneshot(sh4->timer[i], attotime_never, i);
	}

	for (i=0; i<4; i++)
	{
		sh4->dma_timer[i] = timer_alloc(device->machine, sh4_dmac_callback, sh4);
		timer_adjust_oneshot(sh4->dma_timer[i], attotime_never, i);
	}

	sh4->refresh_timer = timer_alloc(device->machine, sh4_refresh_timer_callback, sh4);
	timer_adjust_oneshot(sh4->refresh_timer, attotime_never, 0);
	sh4->refresh_timer_base = 0;

	sh4->rtc_timer = timer_alloc(device->machine, sh4_rtc_timer_callback, sh4);
	timer_adjust_oneshot(sh4->rtc_timer, attotime_never, 0);

	sh4->m = auto_alloc_array(device->machine, UINT32, 16384);
}

void sh4_dma_ddt(running_device *device, struct sh4_ddt_dma *s)
{
	SH4 *sh4 = get_safe_token(device);
	UINT32 chcr;
	UINT32 *p32bits;
	UINT64 *p32bytes;
	UINT32 pos,len,siz;

	if (sh4->dma_timer_active[s->channel])
		return;
	if (s->mode >= 0) {
		switch (s->channel)
		{
		case 0:
			if (s->mode & 1)
				s->source = sh4->m[SAR0];
			if (s->mode & 2)
				sh4->m[SAR0] = s->source;
			if (s->mode & 4)
				s->destination = sh4->m[DAR0];
			if (s->mode & 8)
				sh4->m[DAR0] = s->destination;
			break;
		case 1:
			if (s->mode & 1)
				s->source = sh4->m[SAR1];
			if (s->mode & 2)
				sh4->m[SAR1] = s->source;
			if (s->mode & 4)
				s->destination = sh4->m[DAR1];
			if (s->mode & 8)
				sh4->m[DAR1] = s->destination;
			break;
		case 2:
			if (s->mode & 1)
				s->source = sh4->m[SAR2];
			if (s->mode & 2)
				sh4->m[SAR2] = s->source;
			if (s->mode & 4)
				s->destination = sh4->m[DAR2];
			if (s->mode & 8)
				sh4->m[DAR2] = s->destination;
			break;
		case 3:
		default:
			if (s->mode & 1)
				s->source = sh4->m[SAR3];
			if (s->mode & 2)
				sh4->m[SAR3] = s->source;
			if (s->mode & 4)
				s->destination = sh4->m[DAR3];
			if (s->mode & 8)
				sh4->m[DAR3] = s->destination;
			break;
		}
		switch (s->channel)
		{
		case 0:
			chcr = sh4->m[CHCR0];
			len = sh4->m[DMATCR0];
			break;
		case 1:
			chcr = sh4->m[CHCR1];
			len = sh4->m[DMATCR1];
			break;
		case 2:
			chcr = sh4->m[CHCR2];
			len = sh4->m[DMATCR2];
			break;
		case 3:
		default:
			chcr = sh4->m[CHCR3];
			len = sh4->m[DMATCR3];
			break;
		}
		if ((s->direction) == 0) {
			chcr = (chcr & 0xffff3fff) | ((s->mode & 0x30) << 10);
		} else {
			chcr = (chcr & 0xffffcfff) | ((s->mode & 0x30) << 8);
		}
		siz = dmasize[(chcr >> 4) & 7];
		if (siz && (s->size))
			if ((len * siz) != (s->length * s->size))
				return;
		sh4_dma_transfer(sh4, s->channel, 0, chcr, &s->source, &s->destination, &len);
	} else {
		if (s->size == 4) {
			if ((s->direction) == 0) {
				len = s->length;
				p32bits = (UINT32 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					*p32bits = memory_read_dword_64le(sh4->program, s->source);
					p32bits++;
					s->source = s->source + 4;
				}
			} else {
				len = s->length;
				p32bits = (UINT32 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					memory_write_dword_64le(sh4->program, s->destination, *p32bits);
					p32bits++;
					s->destination = s->destination + 4;
				}
			}
		}
		if (s->size == 32) {
			if ((s->direction) == 0) {
				len = s->length * 4;
				p32bytes = (UINT64 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					*p32bytes = memory_read_qword_64le(sh4->program, s->source);
					p32bytes++;
					s->destination = s->destination + 8;
				}
			} else {
				len = s->length * 4;
				p32bytes = (UINT64 *)(s->buffer);
				for (pos = 0;pos < len;pos++) {
					memory_write_qword_64le(sh4->program, s->destination, *p32bytes);
					p32bytes++;
					s->destination = s->destination + 8;
				}
			}
		}
	}
}

