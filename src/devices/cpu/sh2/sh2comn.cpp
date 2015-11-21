// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh2common.c
 *
 *   SH-2 non-specific components
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "sh2.h"
#include "sh2comn.h"

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

static const int div_tab[4] = { 3, 5, 7, 0 };


void sh2_device::sh2_timer_resync()
{
	int divider = div_tab[(m_m[5] >> 8) & 3];
	UINT64 cur_time = total_cycles();
	UINT64 add = (cur_time - m_frc_base) >> divider;

	if (add > 0)
	{
		if(divider)
			m_frc += add;

		m_frc_base = cur_time;
	}
}

void sh2_device::sh2_timer_activate()
{
	int max_delta = 0xfffff;
	UINT16 frc;

	m_timer->adjust(attotime::never);

	frc = m_frc;
	if(!(m_m[4] & OCFA)) {
		UINT16 delta = m_ocra - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(m_m[4] & OCFB) && (m_ocra <= m_ocrb || !(m_m[4] & 0x010000))) {
		UINT16 delta = m_ocrb - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(m_m[4] & OVF) && !(m_m[4] & 0x010000)) {
		int delta = 0x10000 - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(max_delta != 0xfffff) {
		int divider = div_tab[(m_m[5] >> 8) & 3];
		if(divider) {
			max_delta <<= divider;
			m_frc_base = total_cycles();
			m_timer->adjust(cycles_to_attotime(max_delta));
		} else {
			logerror("SH2.%s: Timer event in %d cycles of external clock", tag(), max_delta);
		}
	}
}

TIMER_CALLBACK_MEMBER( sh2_device::sh2_timer_callback )
{
	UINT16 frc;

	sh2_timer_resync();

	frc = m_frc;

	if(frc == m_ocrb)
		m_m[4] |= OCFB;

	if(frc == 0x0000)
		m_m[4] |= OVF;

	if(frc == m_ocra)
	{
		m_m[4] |= OCFA;

		if(m_m[4] & 0x010000)
			m_frc = 0;
	}

	sh2_recalc_irq();
	sh2_timer_activate();
}


/*
  We have to do DMA on a timer (or at least, in chunks) due to the way some systems use it.
  The 32x is a difficult case, they set the SOURCE of the DMA to a FIFO buffer, which at most
  can have 8 words in it.  Attempting to do an 'instant DMA' in this scenario is impossible
  because the game is expecting the 68k of the system to feed data into the FIFO at the same
  time as the SH2 is transfering it out via DMA

  There are two ways we can do this

  a) with a high frequency timer (more accurate, but a large performance hit)

  or

  b) in the CPU_EXECUTE loop


  we're currently doing a)

  b) causes problems with ST-V games

*/



void sh2_device::sh2_notify_dma_data_available()
{
	//printf("call notify\n");

	for (int dma=0;dma<2;dma++)
	{
		//printf("m_dma_timer_active[dma] %04x\n",m_dma_timer_active[dma]);

		if (m_dma_timer_active[dma]==2) // 2 = stalled
		{
		//  printf("resuming stalled dma\n");
			m_dma_timer_active[dma]=1;
			m_dma_current_active_timer[dma]->adjust(attotime::zero, dma);
		}
	}

}

void sh2_device::sh2_do_dma(int dma)
{
	UINT32 dmadata;

	UINT32 tempsrc, tempdst;

	if (m_active_dma_count[dma] > 0)
	{
		// process current DMA
		switch(m_active_dma_size[dma])
		{
		case 0:
			{
				// we need to know the src / dest ahead of time without changing them
				// to allow for the callback to check if we can process the DMA at this
				// time (we need to know where we're reading / writing to/from)

				if(m_active_dma_incs[dma] == 2)
					tempsrc = m_active_dma_src[dma] - 1;
				else
					tempsrc = m_active_dma_src[dma];

				if(m_active_dma_incd[dma] == 2)
					tempdst = m_active_dma_dst[dma] - 1;
				else
					tempdst = m_active_dma_dst[dma];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dma]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dma]=2;// mark as stalled
						return;
					}
				}

				#ifdef USE_TIMER_FOR_DMA
					//schedule next DMA callback
				m_dma_current_active_timer[dma]->adjust(cycles_to_attotime(2), dma);
				#endif


				dmadata = m_program->read_byte(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_byte(tempdst, dmadata);

				if(m_active_dma_incs[dma] == 2)
					m_active_dma_src[dma] --;
				if(m_active_dma_incd[dma] == 2)
					m_active_dma_dst[dma] --;


				if(m_active_dma_incs[dma] == 1)
					m_active_dma_src[dma] ++;
				if(m_active_dma_incd[dma] == 1)
					m_active_dma_dst[dma] ++;

				m_active_dma_count[dma] --;
			}
			break;
		case 1:
			{
				if(m_active_dma_incs[dma] == 2)
					tempsrc = m_active_dma_src[dma] - 2;
				else
					tempsrc = m_active_dma_src[dma];

				if(m_active_dma_incd[dma] == 2)
					tempdst = m_active_dma_dst[dma] - 2;
				else
					tempdst = m_active_dma_dst[dma];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dma]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dma]=2;// mark as stalled
						return;
					}
				}

				#ifdef USE_TIMER_FOR_DMA
					//schedule next DMA callback
				m_dma_current_active_timer[dma]->adjust(cycles_to_attotime(2), dma);
				#endif

				// check: should this really be using read_word_32 / write_word_32?
				dmadata = m_program->read_word(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_word(tempdst, dmadata);

				if(m_active_dma_incs[dma] == 2)
					m_active_dma_src[dma] -= 2;
				if(m_active_dma_incd[dma] == 2)
					m_active_dma_dst[dma] -= 2;

				if(m_active_dma_incs[dma] == 1)
					m_active_dma_src[dma] += 2;
				if(m_active_dma_incd[dma] == 1)
					m_active_dma_dst[dma] += 2;

				m_active_dma_count[dma] --;
			}
			break;
		case 2:
			{
				if(m_active_dma_incs[dma] == 2)
					tempsrc = m_active_dma_src[dma] - 4;
				else
					tempsrc = m_active_dma_src[dma];

				if(m_active_dma_incd[dma] == 2)
					tempdst = m_active_dma_dst[dma] - 4;
				else
					tempdst = m_active_dma_dst[dma];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dma]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dma]=2;// mark as stalled
						return;
					}
				}

				#ifdef USE_TIMER_FOR_DMA
					//schedule next DMA callback
				m_dma_current_active_timer[dma]->adjust(cycles_to_attotime(2), dma);
				#endif

				dmadata = m_program->read_dword(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_dword(tempdst, dmadata);

				if(m_active_dma_incs[dma] == 2)
					m_active_dma_src[dma] -= 4;
				if(m_active_dma_incd[dma] == 2)
					m_active_dma_dst[dma] -= 4;

				if(m_active_dma_incs[dma] == 1)
					m_active_dma_src[dma] += 4;
				if(m_active_dma_incd[dma] == 1)
					m_active_dma_dst[dma] += 4;

				m_active_dma_count[dma] --;
			}
			break;
		case 3:
			{
				// shouldn't this really be 4 calls here instead?

				tempsrc = m_active_dma_src[dma];

				if(m_active_dma_incd[dma] == 2)
					tempdst = m_active_dma_dst[dma] - 16;
				else
					tempdst = m_active_dma_dst[dma];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dma]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dma]=2;// mark as stalled
						fatalerror("SH2 dma_callback_fifo_data_available == 0 in unsupported mode\n");
					}
				}

				#ifdef USE_TIMER_FOR_DMA
					//schedule next DMA callback
				m_dma_current_active_timer[dma]->adjust(cycles_to_attotime(2), dma);
				#endif

				dmadata = m_program->read_dword(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_dword(tempdst, dmadata);

				dmadata = m_program->read_dword(tempsrc+4);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_dword(tempdst+4, dmadata);

				dmadata = m_program->read_dword(tempsrc+8);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_dword(tempdst+8, dmadata);

				dmadata = m_program->read_dword(tempsrc+12);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dma]);
				m_program->write_dword(tempdst+12, dmadata);

				if(m_active_dma_incd[dma] == 2)
					m_active_dma_dst[dma] -= 16;

				m_active_dma_src[dma] += 16;
				if(m_active_dma_incd[dma] == 1)
					m_active_dma_dst[dma] += 16;

				m_active_dma_count[dma]-=4;
			}
			break;
		}
	}
	else // the dma is complete
	{
	//  int dma = param & 1;

		// fever soccer uses cycle-stealing mode, resume the CPU now DMA has finished
		if (m_active_dma_steal[dma])
		{
			resume(SUSPEND_REASON_HALT );
		}


		LOG(("SH2.%s: DMA %d complete\n", tag(), dma));
		m_m[0x62+4*dma] = 0;
		m_m[0x63+4*dma] |= 2;
		m_dma_timer_active[dma] = 0;
		m_dma_irq[dma] |= 1;
		sh2_recalc_irq();

	}
}

TIMER_CALLBACK_MEMBER( sh2_device::sh2_dma_current_active_callback )
{
	int dma = param & 1;

	sh2_do_dma(dma);
}


void sh2_device::sh2_dmac_check(int dma)
{
	if(m_m[0x63+4*dma] & m_m[0x6c] & 1)
	{
		if(!m_dma_timer_active[dma] && !(m_m[0x63+4*dma] & 2))
		{
			m_active_dma_incd[dma] = (m_m[0x63+4*dma] >> 14) & 3;
			m_active_dma_incs[dma] = (m_m[0x63+4*dma] >> 12) & 3;
			m_active_dma_size[dma] = (m_m[0x63+4*dma] >> 10) & 3;
			m_active_dma_steal[dma] = (m_m[0x63+4*dma] &0x10);

			if(m_active_dma_incd[dma] == 3 || m_active_dma_incs[dma] == 3)
			{
				logerror("SH2: DMA: bad increment values (%d, %d, %d, %04x)\n", m_active_dma_incd[dma], m_active_dma_incs[dma], m_active_dma_size[dma], m_m[0x63+4*dma]);
				return;
			}
			m_active_dma_src[dma]   = m_m[0x60+4*dma];
			m_active_dma_dst[dma]   = m_m[0x61+4*dma];
			m_active_dma_count[dma] = m_m[0x62+4*dma];
			if(!m_active_dma_count[dma])
				m_active_dma_count[dma] = 0x1000000;

			LOG(("SH2: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", dma, m_active_dma_src[dma], m_active_dma_dst[dma], m_active_dma_count[dma], m_m[0x63+4*dma], m_active_dma_incs[dma], m_active_dma_incd[dma], m_active_dma_size[dma]));

			m_dma_timer_active[dma] = 1;

			m_active_dma_src[dma] &= AM;
			m_active_dma_dst[dma] &= AM;

			switch(m_active_dma_size[dma])
			{
			case 0:
				break;
			case 1:
				m_active_dma_src[dma] &= ~1;
				m_active_dma_dst[dma] &= ~1;
				break;
			case 2:
				m_active_dma_src[dma] &= ~3;
				m_active_dma_dst[dma] &= ~3;
				break;
			case 3:
				m_active_dma_src[dma] &= ~3;
				m_active_dma_dst[dma] &= ~3;
				m_active_dma_count[dma] &= ~3;
				break;
			}




#ifdef USE_TIMER_FOR_DMA
			// start DMA timer

			// fever soccer uses cycle-stealing mode, requiring the CPU to be halted
			if (m_active_dma_steal[dma])
			{
				//printf("cycle stealing DMA\n");
				suspend(SUSPEND_REASON_HALT, 1 );
			}

			m_dma_current_active_timer[dma]->adjust(cycles_to_attotime(2), dma);
#endif

		}
	}
	else
	{
		if(m_dma_timer_active[dma])
		{
			logerror("SH2: DMA %d cancelled in-flight\n", dma);
			//m_dma_complete_timer[dma]->adjust(attotime::never);
			m_dma_current_active_timer[dma]->adjust(attotime::never);

			m_dma_timer_active[dma] = 0;
		}
	}
}


WRITE32_MEMBER( sh2_device::sh7604_w )
{
	UINT32 old;

	old = m_m[offset];
	COMBINE_DATA(m_m+offset);

	//  if(offset != 0x20)
	//      logerror("sh2_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfffffe00+offset*4, offset, data, mem_mask);

//    if(offset != 0x20)
//        printf("sh2_internal_w:  Write %08x (%x), %08x @ %08x (PC %x)\n", 0xfffffe00+offset*4, offset, data, mem_mask, space.device().safe_pc());

	switch( offset )
	{
	case 0x00:
		//if(mem_mask == 0xff)
		//  printf("%c",data & 0xff);
		break;
	case 0x01:
		//printf("%08x %02x %02x\n",mem_mask,offset,data);
		break;
		// Timers
	case 0x04: // TIER, FTCSR, FRC
		if((mem_mask & 0x00ffffff) != 0)
		{
			sh2_timer_resync();
		}
//      printf("SH2.%s: TIER write %04x @ %04x\n", m_device->tag(), data >> 16, mem_mask>>16);
		m_m[4] = (m_m[4] & ~(ICF|OCFA|OCFB|OVF)) | (old & m_m[4] & (ICF|OCFA|OCFB|OVF));
		COMBINE_DATA(&m_frc);
		if((mem_mask & 0x00ffffff) != 0)
			sh2_timer_activate();
		sh2_recalc_irq();
		break;
	case 0x05: // OCRx, TCR, TOCR
//      printf("SH2.%s: TCR write %08x @ %08x\n", m_device->tag(), data, mem_mask);
		sh2_timer_resync();
		if(m_m[5] & 0x10)
			m_ocrb = (m_ocrb & (~mem_mask >> 16)) | ((data & mem_mask) >> 16);
		else
			m_ocra = (m_ocra & (~mem_mask >> 16)) | ((data & mem_mask) >> 16);
		sh2_timer_activate();
		break;

	case 0x06: // ICR
		break;

		// Interrupt vectors
	case 0x18: // IPRB, VCRA
	case 0x19: // VCRB, VCRC
	case 0x1a: // VCRD
		sh2_recalc_irq();
		break;

		// DMA
	case 0x1c: // DRCR0, DRCR1
		break;

		// Watchdog
	case 0x20: // WTCNT, RSTCSR
		if((m_m[0x20] & 0xff000000) == 0x5a000000)
			m_wtcnt = (m_m[0x20] >> 16) & 0xff;

		if((m_m[0x20] & 0xff000000) == 0xa5000000)
		{
			/*
			WTCSR
			x--- ---- Overflow in IT mode
			-x-- ---- Timer mode (0: IT 1: watchdog)
			--x- ---- Timer enable
			---1 1---
			---- -xxx Clock select
			*/

			m_wtcsr = (m_m[0x20] >> 16) & 0xff;
		}

		if((m_m[0x20] & 0x0000ff00) == 0x00005a00)
		{
			// -x-- ---- RSTE (1: resets wtcnt when overflows 0: no reset)
			// --x- ---- RSTS (0: power-on reset 1: Manual reset)
			// ...
		}

		if((m_m[0x20] & 0x0000ff00) == 0x0000a500)
		{
			// clear WOVF
			// ...
		}



		break;

		// Standby and cache
	case 0x24: // SBYCR, CCR
		/*
		    CCR
		    xx-- ---- ---- ---- Way 0/1
		    ---x ---- ---- ---- Cache Purge (CP)
		    ---- x--- ---- ---- Two-Way Mode (TW)
		    ---- -x-- ---- ---- Data Replacement Disable (OD)
		    ---- --x- ---- ---- Instruction Replacement Disable (ID)
		    ---- ---x ---- ---- Cache Enable (CE)
		*/
		break;

		// Interrupt vectors cont.
	case 0x38: // ICR, IRPA
		break;
	case 0x39: // VCRWDT
		break;

		// Division box
	case 0x40: // DVSR
		break;
	case 0x41: // DVDNT
		{
			INT32 a = m_m[0x41];
			INT32 b = m_m[0x40];
			LOG(("SH2 '%s' div+mod %d/%d\n", tag(), a, b));
			if (b)
			{
				m_m[0x45] = a / b;
				m_m[0x44] = a % b;
			}
			else
			{
				m_m[0x42] |= 0x00010000;
				m_m[0x45] = 0x7fffffff;
				m_m[0x44] = 0x7fffffff;
				sh2_recalc_irq();
			}
			break;
		}
	case 0x42: // DVCR
		m_m[0x42] = (m_m[0x42] & ~0x00001000) | (old & m_m[0x42] & 0x00010000);
		sh2_recalc_irq();
		break;
	case 0x43: // VCRDIV
		sh2_recalc_irq();
		break;
	case 0x44: // DVDNTH
		break;
	case 0x45: // DVDNTL
		{
			INT64 a = m_m[0x45] | ((UINT64)(m_m[0x44]) << 32);
			INT64 b = (INT32)m_m[0x40];
			LOG(("SH2 '%s' div+mod %" I64FMT "d/%" I64FMT "d\n", tag(), a, b));
			if (b)
			{
				INT64 q = a / b;
				if (q != (INT32)q)
				{
					m_m[0x42] |= 0x00010000;
					m_m[0x45] = 0x7fffffff;
					m_m[0x44] = 0x7fffffff;
					sh2_recalc_irq();
				}
				else
				{
					m_m[0x45] = q;
					m_m[0x44] = a % b;
				}
			}
			else
			{
				m_m[0x42] |= 0x00010000;
				m_m[0x45] = 0x7fffffff;
				m_m[0x44] = 0x7fffffff;
				sh2_recalc_irq();
			}
			break;
		}

		// DMA controller
	case 0x60: // SAR0
	case 0x61: // DAR0
		break;
	case 0x62: // DTCR0
		m_m[0x62] &= 0xffffff;
		break;
	case 0x63: // CHCR0
		m_m[0x63] = (m_m[0x63] & ~2) | (old & m_m[0x63] & 2);
		sh2_dmac_check(0);
		break;
	case 0x64: // SAR1
	case 0x65: // DAR1
		break;
	case 0x66: // DTCR1
		m_m[0x66] &= 0xffffff;
		break;
	case 0x67: // CHCR1
		m_m[0x67] = (m_m[0x67] & ~2) | (old & m_m[0x67] & 2);
		sh2_dmac_check(1);
		break;
	case 0x68: // VCRDMA0
	case 0x6a: // VCRDMA1
		sh2_recalc_irq();
		break;
	case 0x6c: // DMAOR
		m_m[0x6c] = (m_m[0x6c] & ~6) | (old & m_m[0x6c] & 6);
		sh2_dmac_check(0);
		sh2_dmac_check(1);
		break;

		// Bus controller
	case 0x78: // BCR1
	case 0x79: // BCR2
	case 0x7a: // WCR
	case 0x7b: // MCR
	case 0x7c: // RTCSR
	case 0x7d: // RTCNT
	case 0x7e: // RTCOR
		break;

	default:
		logerror("sh2_internal_w:  Unmapped write %08x, %08x @ %08x\n", 0xfffffe00+offset*4, data, mem_mask);
		break;
	}
}

READ32_MEMBER( sh2_device::sh7604_r )
{
//  logerror("sh2_internal_r:  Read %08x (%x) @ %08x\n", 0xfffffe00+offset*4, offset, mem_mask);
	switch( offset )
	{
	case 0x00:
		break;
	case 0x01:
//      return m_m[1] | 0; // bit31 is TDRE: Trasmit Data Register Empty. Forcing it to be '1' breaks Saturn ...
		return m_m[1] | (0x84 << 24); // ... but this is actually needed to make EGWord on SS to boot?

	case 0x04: // TIER, FTCSR, FRC
		if ( mem_mask == 0x00ff0000 )
		{
			if (!m_ftcsr_read_cb.isnull())
			{
				m_ftcsr_read_cb((m_m[4] & 0xffff0000) | m_frc);
			}
		}
		sh2_timer_resync();
		return (m_m[4] & 0xffff0000) | m_frc;
	case 0x05: // OCRx, TCR, TOCR
		if(m_m[5] & 0x10)
			return (m_ocrb << 16) | (m_m[5] & 0xffff);
		else
			return (m_ocra << 16) | (m_m[5] & 0xffff);
	case 0x06: // ICR
		return m_icr << 16;

	case 0x20:
		return (((m_wtcsr | 0x18) & 0xff) << 24)  | ((m_wtcnt & 0xff) << 16);

	case 0x24: // SBYCR, CCR
		return m_m[0x24] & ~0x3000; /* bit 4-5 of CCR are always zero */

	case 0x38: // ICR, IPRA
		return (m_m[0x38] & 0x7fffffff) | (m_nmi_line_state == ASSERT_LINE ? 0 : 0x80000000);

	case 0x78: // BCR1
		return m_is_slave ? 0x00008000 : 0;

	case 0x41: // dvdntl mirrors
	case 0x47:
		return m_m[0x45];

	case 0x46: // dvdnth mirror
		return m_m[0x44];
	}
	return m_m[offset];
}

void sh2_device::sh2_set_frt_input(int state)
{
	if(state == PULSE_LINE)
	{
		sh2_set_frt_input(ASSERT_LINE);
		sh2_set_frt_input(CLEAR_LINE);
		return;
	}

	if(m_frt_input == state) {
		return;
	}

	m_frt_input = state;

	if(m_m[5] & 0x8000) {
		if(state == CLEAR_LINE) {
			return;
		}
	} else {
		if(state == ASSERT_LINE) {
			return;
		}
	}

	sh2_timer_resync();
	m_icr = m_frc;
	m_m[4] |= ICF;
	//logerror("SH2.%s: ICF activated (%x)\n", tag(), m_sh2_state->pc & AM);
	sh2_recalc_irq();
}

void sh2_device::sh2_recalc_irq()
{
	int irq = 0, vector = -1;
	int  level;

	// Timer irqs
	if((m_m[4]>>8) & m_m[4] & (ICF|OCFA|OCFB|OVF))
	{
		level = (m_m[0x18] >> 24) & 15;
		if(level > irq)
		{
			int mask = (m_m[4]>>8) & m_m[4];
			irq = level;
			if(mask & ICF)
				vector = (m_m[0x19] >> 8) & 0x7f;
			else if(mask & (OCFA|OCFB))
				vector = m_m[0x19] & 0x7f;
			else
				vector = (m_m[0x1a] >> 24) & 0x7f;
		}
	}

	// DMA irqs
	if((m_m[0x63] & 6) == 6 && m_dma_irq[0]) {
		level = (m_m[0x38] >> 8) & 15;
		if(level > irq) {
			irq = level;
			m_dma_irq[0] &= ~1;
			vector = (m_m[0x68]) & 0x7f;
		}
	}
	else if((m_m[0x67] & 6) == 6 && m_dma_irq[1]) {
		level = (m_m[0x38] >> 8) & 15;
		if(level > irq) {
			irq = level;
			m_dma_irq[1] &= ~1;
			vector = (m_m[0x6a]) & 0x7f;
		}
	}

	m_sh2_state->internal_irq_level = irq;
	m_internal_irq_vector = vector;
	m_test_irq = 1;
}

void sh2_device::sh2_exception(const char *message, int irqline)
{
	int vector;

	if (irqline != 16)
	{
		if (irqline <= ((m_sh2_state->sr >> 4) & 15)) /* If the cpu forbids this interrupt */
			return;

		// if this is an sh2 internal irq, use its vector
		if (m_sh2_state->internal_irq_level == irqline)
		{
			vector = m_internal_irq_vector;
			/* avoid spurious irqs with this (TODO: needs a better fix) */
			m_sh2_state->internal_irq_level = -1;
			LOG(("SH-2 '%s' exception #%d (internal vector: $%x) after [%s]\n", tag(), irqline, vector, message));
		}
		else
		{
			if(m_m[0x38] & 0x00010000)
			{
				vector = standard_irq_callback(irqline);
				LOG(("SH-2 '%s' exception #%d (external vector: $%x) after [%s]\n", tag(), irqline, vector, message));
			}
			else
			{
				standard_irq_callback(irqline);
				vector = 64 + irqline/2;
				LOG(("SH-2 '%s' exception #%d (autovector: $%x) after [%s]\n", tag(), irqline, vector, message));
			}
		}
	}
	else
	{
		vector = 11;
		LOG(("SH-2 '%s' nmi exception (autovector: $%x) after [%s]\n", tag(), vector, message));
	}

	if (m_isdrc)
	{
		m_sh2_state->evec = RL( m_sh2_state->vbr + vector * 4 );
		m_sh2_state->evec &= AM;
		m_sh2_state->irqsr = m_sh2_state->sr;

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~I) | (irqline << 4);

//  printf("sh2_exception [%s] irqline %x evec %x save SR %x new SR %x\n", message, irqline, m_sh2_state->evec, m_sh2_state->irqsr, m_sh2_state->sr);
	} else {
		m_sh2_state->r[15] -= 4;
		WL( m_sh2_state->r[15], m_sh2_state->sr );     /* push SR onto stack */
		m_sh2_state->r[15] -= 4;
		WL( m_sh2_state->r[15], m_sh2_state->pc );     /* push PC onto stack */

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~I) | (irqline << 4);

		/* fetch PC */
		m_sh2_state->pc = RL( m_sh2_state->vbr + vector * 4 );
	}

	if(m_sh2_state->sleep_mode == 1) { m_sh2_state->sleep_mode = 2; }
}

/*
 SH-7021 on-chip device
 */

void sh2a_device::sh7032_dma_exec(int ch)
{
	const short dma_word_size[4] = { 0, +1, -1, 0 };
	UINT8 rs = (m_dma[ch].chcr >> 8) & 0xf; /**< Resource Select bits */
	if(rs != 0xc) // Auto-Request
	{
		logerror("Warning: SH7032 DMA enables non auto-request transfer\n");
		return;
	}

	// channel enable & master enable
	if((m_dma[ch].chcr & 1) == 0 || (m_dmaor & 1) == 0)
		return;

	printf("%08x %08x %04x\n",m_dma[ch].sar,m_dma[ch].dar,m_dma[ch].chcr);
	UINT8 dm = (m_dma[ch].chcr >> 14) & 3;  /**< Destination Address Mode bits */
	UINT8 sm = (m_dma[ch].chcr >> 12) & 3;  /**< Source Address Mode bits */
	bool ts = (m_dma[ch].chcr & 8);         /**< Transfer Size bit */
	int src_word_size = dma_word_size[sm] * ((ts == true) ? 2 : 1);
	int dst_word_size = dma_word_size[dm] * ((ts == true) ? 2 : 1);
	UINT32 src_addr = m_dma[ch].sar;
	UINT32 dst_addr = m_dma[ch].dar;
	UINT32 size_index = m_dma[ch].tcr;
	if(size_index == 0)
		size_index = 0x10000;

	if(ts == false)
		logerror("SH7032: DMA byte mode check\n");

	for(int index = size_index;index>-1;index--)
	{
		if(ts == true)
			m_program->write_word(dst_addr,m_program->read_word(src_addr));
		else
			m_program->write_byte(dst_addr,m_program->read_byte(src_addr));

		src_addr += src_word_size;
		dst_addr += dst_word_size;
	}

	m_dma[ch].chcr &= ~1; /**< @todo non-instant DMA */
	printf("%02x %02x %02x %1d\n",sm,dm,rs,ts);
}

READ32_MEMBER(sh2a_device::dma_sar0_r)
{
	return m_dma[0].sar;
}

WRITE32_MEMBER(sh2a_device::dma_sar0_w)
{
	COMBINE_DATA(&m_dma[0].sar);
}

READ32_MEMBER(sh2a_device::dma_dar0_r)
{
	return m_dma[0].dar;
}

WRITE32_MEMBER(sh2a_device::dma_dar0_w)
{
	COMBINE_DATA(&m_dma[0].dar);
}

READ16_MEMBER(sh2a_device::dma_tcr0_r)
{
	return m_dma[0].tcr;
}

WRITE16_MEMBER(sh2a_device::dma_tcr0_w)
{
	//printf("%04x\n",data);
	COMBINE_DATA(&m_dma[0].tcr);
}

READ16_MEMBER(sh2a_device::dma_chcr0_r)
{
	return m_dma[0].chcr;
}

WRITE16_MEMBER(sh2a_device::dma_chcr0_w)
{
	//printf("%04x CHCR0\n",data);
	COMBINE_DATA(&m_dma[0].chcr);
	sh7032_dma_exec(0);
}

READ16_MEMBER(sh2a_device::dmaor_r)
{
	return m_dmaor;
}

WRITE16_MEMBER(sh2a_device::dmaor_w)
{
	COMBINE_DATA(&m_dmaor);
	sh7032_dma_exec(0);
}

/*!
  @brief Dummy debug interface
  */
READ16_MEMBER(sh1_device::sh7032_r)
{
	return m_sh7032_regs[offset];
}

/*!
  @brief Dummy debug interface
 */
WRITE16_MEMBER(sh1_device::sh7032_w)
{
	COMBINE_DATA(&m_sh7032_regs[offset]);
}

READ16_MEMBER(sh2a_device::sh7021_r)
{
	return m_sh7021_regs[offset];
}

/*!
  @brief Dummy debug interface
 */
WRITE16_MEMBER(sh2a_device::sh7021_w)
{
	COMBINE_DATA(&m_sh7021_regs[offset]);
}
