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
#include "sh2.h"
#include "sh2comn.h"

#include "debugger.h"

//#define VERBOSE 1
#include "logmacro.h"

static const int div_tab[4] = { 3, 5, 7, 0 };


void sh2_device::sh2_timer_resync()
{
	// TODO: setting 3 is "External clock: count on rising edge"
	int divider = div_tab[m_frc_tcr & 3];
	uint64_t cur_time = total_cycles();
	uint64_t add = (cur_time - m_frc_base) >> divider;

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
	uint16_t frc;

	m_timer->adjust(attotime::never);

	frc = m_frc;
	if(!(m_ftcsr & OCFA)) {
		uint16_t delta = m_ocra - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(m_ftcsr & OCFB) && (m_ocra <= m_ocrb || !(m_ftcsr & CCLRA))) {
		uint16_t delta = m_ocrb - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(m_ftcsr & OVF) && !(m_ftcsr & CCLRA)) {
		int delta = 0x10000 - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(max_delta != 0xfffff) {
		int divider = div_tab[m_frc_tcr & 3];
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
	uint16_t frc;

	sh2_timer_resync();

	frc = m_frc;

	if(frc == m_ocrb)
		m_ftcsr |= OCFB;

	if(frc == 0x0000)
		m_ftcsr |= OVF;

	if(frc == m_ocra)
	{
		m_ftcsr |= OCFA;

		if(m_ftcsr & CCLRA)
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

	for (int dmach=0;dmach<2;dmach++)
	{
		//printf("m_dma_timer_active[dmach] %04x\n",m_dma_timer_active[dmach]);

		if (m_dma_timer_active[dmach]==2) // 2 = stalled
		{
		//  printf("resuming stalled dma\n");
			m_dma_timer_active[dmach]=1;
			m_dma_current_active_timer[dmach]->adjust(attotime::zero, dmach);
		}
	}

}

void sh2_device::sh2_do_dma(int dmach)
{
	uint32_t dmadata;

	uint32_t tempsrc, tempdst;

	if (m_active_dma_count[dmach] > 0)
	{
		// process current DMA
		switch(m_active_dma_size[dmach])
		{
		case 0:
			{
				// we need to know the src / dest ahead of time without changing them
				// to allow for the callback to check if we can process the DMA at this
				// time (we need to know where we're reading / writing to/from)

				if(m_active_dma_incs[dmach] == 2)
					tempsrc = m_active_dma_src[dmach] - 1;
				else
					tempsrc = m_active_dma_src[dmach];

				if(m_active_dma_incd[dmach] == 2)
					tempdst = m_active_dma_dst[dmach] - 1;
				else
					tempdst = m_active_dma_dst[dmach];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dmach]=2;// mark as stalled
						return;
					}
				}

				//schedule next DMA callback
				m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

				dmadata = m_program->read_byte(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_byte(tempdst, dmadata);

				if(m_active_dma_incs[dmach] == 2)
					m_active_dma_src[dmach] --;
				if(m_active_dma_incd[dmach] == 2)
					m_active_dma_dst[dmach] --;


				if(m_active_dma_incs[dmach] == 1)
					m_active_dma_src[dmach] ++;
				if(m_active_dma_incd[dmach] == 1)
					m_active_dma_dst[dmach] ++;

				m_active_dma_count[dmach] --;
			}
			break;
		case 1:
			{
				if(m_active_dma_incs[dmach] == 2)
					tempsrc = m_active_dma_src[dmach] - 2;
				else
					tempsrc = m_active_dma_src[dmach];

				if(m_active_dma_incd[dmach] == 2)
					tempdst = m_active_dma_dst[dmach] - 2;
				else
					tempdst = m_active_dma_dst[dmach];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dmach]=2;// mark as stalled
						return;
					}
				}

				//schedule next DMA callback
				m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

				// check: should this really be using read_word_32 / write_word_32?
				dmadata = m_program->read_word(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_word(tempdst, dmadata);

				if(m_active_dma_incs[dmach] == 2)
					m_active_dma_src[dmach] -= 2;
				if(m_active_dma_incd[dmach] == 2)
					m_active_dma_dst[dmach] -= 2;

				if(m_active_dma_incs[dmach] == 1)
					m_active_dma_src[dmach] += 2;
				if(m_active_dma_incd[dmach] == 1)
					m_active_dma_dst[dmach] += 2;

				m_active_dma_count[dmach] --;
			}
			break;
		case 2:
			{
				if(m_active_dma_incs[dmach] == 2)
					tempsrc = m_active_dma_src[dmach] - 4;
				else
					tempsrc = m_active_dma_src[dmach];

				if(m_active_dma_incd[dmach] == 2)
					tempdst = m_active_dma_dst[dmach] - 4;
				else
					tempdst = m_active_dma_dst[dmach];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dmach]=2;// mark as stalled
						return;
					}
				}

				//schedule next DMA callback
				m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

				dmadata = m_program->read_dword(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_dword(tempdst, dmadata);

				if(m_active_dma_incs[dmach] == 2)
					m_active_dma_src[dmach] -= 4;
				if(m_active_dma_incd[dmach] == 2)
					m_active_dma_dst[dmach] -= 4;

				if(m_active_dma_incs[dmach] == 1)
					m_active_dma_src[dmach] += 4;
				if(m_active_dma_incd[dmach] == 1)
					m_active_dma_dst[dmach] += 4;

				m_active_dma_count[dmach] --;
			}
			break;
		case 3:
			{
				// shouldn't this really be 4 calls here instead?

				tempsrc = m_active_dma_src[dmach];

				if(m_active_dma_incd[dmach] == 2)
					tempdst = m_active_dma_dst[dmach] - 16;
				else
					tempdst = m_active_dma_dst[dmach];

				if (!m_dma_fifo_data_available_cb.isnull())
				{
					int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

					if (!available)
					{
						//printf("dma stalled\n");
						m_dma_timer_active[dmach]=2;// mark as stalled
						fatalerror("SH2 dma_callback_fifo_data_available == 0 in unsupported mode\n");
					}
				}

				//schedule next DMA callback
				m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

				dmadata = m_program->read_dword(tempsrc);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_dword(tempdst, dmadata);

				dmadata = m_program->read_dword(tempsrc+4);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_dword(tempdst+4, dmadata);

				dmadata = m_program->read_dword(tempsrc+8);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_dword(tempdst+8, dmadata);

				dmadata = m_program->read_dword(tempsrc+12);
				if (!m_dma_kludge_cb.isnull()) dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
				m_program->write_dword(tempdst+12, dmadata);

				if(m_active_dma_incd[dmach] == 2)
					m_active_dma_dst[dmach] -= 16;

				m_active_dma_src[dmach] += 16;
				if(m_active_dma_incd[dmach] == 1)
					m_active_dma_dst[dmach] += 16;

				m_active_dma_count[dmach]-=4;
			}
			break;
		}
	}
	else // the dma is complete
	{
	//  int dma = param & 1;

		// fever soccer uses cycle-stealing mode, resume the CPU now DMA has finished
		if (m_active_dma_steal[dmach])
		{
			resume(SUSPEND_REASON_HALT );
		}


		LOG("SH2: DMA %d complete\n", dmach);
		m_dmac[dmach].tcr = 0;
		m_dmac[dmach].chcr |= 2;
		m_dma_timer_active[dmach] = 0;
		m_dma_irq[dmach] |= 1;
		sh2_recalc_irq();

	}
}

TIMER_CALLBACK_MEMBER( sh2_device::sh2_dma_current_active_callback )
{
	int dma = param & 1;

	sh2_do_dma(dma);
}


void sh2_device::sh2_dmac_check(int dmach)
{
	if(m_dmac[dmach].chcr & m_dmaor & 1)
	{
		if(!m_dma_timer_active[dmach] && !(m_dmac[dmach].chcr & 2))
		{
			m_active_dma_incd[dmach] = (m_dmac[dmach].chcr >> 14) & 3;
			m_active_dma_incs[dmach] = (m_dmac[dmach].chcr >> 12) & 3;
			m_active_dma_size[dmach] = (m_dmac[dmach].chcr >> 10) & 3;
			m_active_dma_steal[dmach] = (m_dmac[dmach].chcr & 0x10);

			if(m_active_dma_incd[dmach] == 3 || m_active_dma_incs[dmach] == 3)
			{
				logerror("SH2: DMA: bad increment values (%d, %d, %d, %04x)\n", m_active_dma_incd[dmach], m_active_dma_incs[dmach], m_active_dma_size[dmach], m_dmac[dmach].chcr);
				return;
			}
			m_active_dma_src[dmach]   = m_dmac[dmach].sar;
			m_active_dma_dst[dmach]   = m_dmac[dmach].dar;
			m_active_dma_count[dmach] = m_dmac[dmach].tcr;
			if(!m_active_dma_count[dmach])
				m_active_dma_count[dmach] = 0x1000000;

			LOG("SH2: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", dmach, m_active_dma_src[dmach], m_active_dma_dst[dmach], m_active_dma_count[dmach], m_dmac[dmach].chcr, m_active_dma_incs[dmach], m_active_dma_incd[dmach], m_active_dma_size[dmach]);

			m_dma_timer_active[dmach] = 1;

			m_active_dma_src[dmach] &= SH12_AM;
			m_active_dma_dst[dmach] &= SH12_AM;

			switch(m_active_dma_size[dmach])
			{
			case 0:
				break;
			case 1:
				m_active_dma_src[dmach] &= ~1;
				m_active_dma_dst[dmach] &= ~1;
				break;
			case 2:
				m_active_dma_src[dmach] &= ~3;
				m_active_dma_dst[dmach] &= ~3;
				break;
			case 3:
				m_active_dma_src[dmach] &= ~3;
				m_active_dma_dst[dmach] &= ~3;
				m_active_dma_count[dmach] &= ~3;
				break;
			}

			// start DMA timer

			// fever soccer uses cycle-stealing mode, requiring the CPU to be halted
			if (m_active_dma_steal[dmach])
			{
				//printf("cycle stealing DMA\n");
				suspend(SUSPEND_REASON_HALT, 1 );
			}

			m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);
		}
	}
	else
	{
		if(m_dma_timer_active[dmach])
		{
			logerror("SH2: DMA %d cancelled in-flight\n", dmach);
			//m_dma_complete_timer[dmach]->adjust(attotime::never);
			m_dma_current_active_timer[dmach]->adjust(attotime::never, dmach);

			m_dma_timer_active[dmach] = 0;
		}
	}
}

/*
 * SCI
 */
// TODO: identical to H8 counterpart

READ8_MEMBER( sh2_device::smr_r )
{
	return m_smr;
}

WRITE8_MEMBER( sh2_device::smr_w )
{
	m_smr = data;
}

READ8_MEMBER( sh2_device::brr_r )
{
	return m_brr;
}

WRITE8_MEMBER( sh2_device::brr_w )
{
	m_brr = data;
}

READ8_MEMBER( sh2_device::scr_r )
{
	return m_scr;
}

WRITE8_MEMBER( sh2_device::scr_w )
{
	m_scr = data;
}

READ8_MEMBER( sh2_device::tdr_r )
{
	return m_tdr;
}

WRITE8_MEMBER( sh2_device::tdr_w )
{
	m_tdr = data;
	// printf("%c",data & 0xff);
}

READ8_MEMBER( sh2_device::ssr_r )
{
	// 0x84 is needed by EGWord on Saturn to make it to boot for some reason.
	return m_ssr | 0x84;
}

WRITE8_MEMBER( sh2_device::ssr_w )
{
	m_ssr = data;
}

READ8_MEMBER( sh2_device::rdr_r )
{
	return 0;
}

/*
 * FRC
 */

READ8_MEMBER( sh2_device::tier_r )
{
	return m_tier;
}

WRITE8_MEMBER( sh2_device::tier_w )
{
	sh2_timer_resync();
	m_tier = data;
	sh2_timer_activate();
	sh2_recalc_irq();
}

READ8_MEMBER( sh2_device::ftcsr_r )
{
	// TODO: to be tested
	if (!m_ftcsr_read_cb.isnull())
		m_ftcsr_read_cb((((m_tier<<24) | (m_ftcsr<<16)) & 0xffff0000) | m_frc);

	return m_ftcsr;
}

WRITE8_MEMBER( sh2_device::ftcsr_w )
{
	uint8_t old;
	old = m_ftcsr;

	m_ftcsr = data;
	sh2_timer_resync();
	m_ftcsr = (m_ftcsr & ~(ICF|OCFA|OCFB|OVF)) | (old & m_ftcsr & (ICF|OCFA|OCFB|OVF));
	sh2_timer_activate();
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::frc_r )
{
	sh2_timer_resync();
	return m_frc;
}

WRITE16_MEMBER( sh2_device::frc_w )
{
	sh2_timer_resync();
	COMBINE_DATA(&m_frc);
	sh2_timer_activate();
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::ocra_b_r )
{
	return (m_tocr & 0x10) ? m_ocrb : m_ocra;
}

WRITE16_MEMBER( sh2_device::ocra_b_w )
{
	sh2_timer_resync();
	if(m_tocr & 0x10)
		m_ocrb = (m_ocrb & (~mem_mask)) | (data & mem_mask);
	else
		m_ocra = (m_ocra & (~mem_mask)) | (data & mem_mask);
	sh2_timer_activate();
	sh2_recalc_irq();
}

READ8_MEMBER( sh2_device::frc_tcr_r )
{
	return m_frc_tcr & 0x83;
}

WRITE8_MEMBER( sh2_device::frc_tcr_w )
{
	sh2_timer_resync();
	m_frc_tcr = data & 0x83;
	sh2_timer_activate();
	sh2_recalc_irq();
}

READ8_MEMBER( sh2_device::tocr_r )
{
	return (m_tocr & 0x13) | 0xe0;
}

WRITE8_MEMBER( sh2_device::tocr_w )
{
	sh2_timer_resync();
	// TODO: output levels A/B (bits 1-0)
	m_tocr = data & 0x13;
	sh2_timer_activate();
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::frc_icr_r )
{
	return m_frc_icr;
}

/*
 * INTC
 */

READ16_MEMBER( sh2_device::intc_icr_r )
{
	// TODO: flip meaning based off NMI edge select bit (NMIE)
	uint16_t nmilv = m_nmi_line_state == ASSERT_LINE ? 0 : 0x8000;
	return (nmilv) | (m_intc_icr & 0x0101);
}

WRITE16_MEMBER( sh2_device::intc_icr_w )
{
	COMBINE_DATA(&m_intc_icr);
	m_nmie = bool(BIT(m_intc_icr, 8));
	m_vecmd = bool(BIT(m_intc_icr, 0));
}

READ16_MEMBER( sh2_device::ipra_r )
{
	return m_ipra & 0xfff0;
}

WRITE16_MEMBER( sh2_device::ipra_w )
{
	COMBINE_DATA(&m_ipra);
	m_irq_level.divu = (m_ipra >> 12) & 0xf;
	m_irq_level.dmac = (m_ipra >> 8) & 0xf;
	m_irq_level.wdt = (m_ipra >> 4) & 0xf;
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::iprb_r )
{
	return m_iprb & 0xff00;
}

WRITE16_MEMBER( sh2_device::iprb_w )
{
	COMBINE_DATA(&m_iprb);
	m_irq_level.sci = (m_iprb >> 12) & 0xf;
	m_irq_level.frc = (m_iprb >> 8) & 0xf;
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::vcra_r )
{
	return m_vcra & 0x7f7f;
}

WRITE16_MEMBER( sh2_device::vcra_w )
{
	COMBINE_DATA(&m_vcra);
	// ...
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::vcrb_r )
{
	return m_vcrb;
}

WRITE16_MEMBER( sh2_device::vcrb_w )
{
	COMBINE_DATA(&m_vcrb);
	// ...
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::vcrc_r )
{
	return m_vcrc & 0x7f7f;
}

WRITE16_MEMBER( sh2_device::vcrc_w )
{
	COMBINE_DATA(&m_vcrc);
	m_irq_vector.fic = (m_vcrc >> 8) & 0x7f;
	m_irq_vector.foc = (m_vcrc >> 0) & 0x7f;
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::vcrd_r )
{
	return m_vcrd & 0x7f00;
}

WRITE16_MEMBER( sh2_device::vcrd_w )
{
	COMBINE_DATA(&m_vcrd);
	m_irq_vector.fov = (m_vcrc >> 8) & 0x7f;
	sh2_recalc_irq();
}

READ16_MEMBER( sh2_device::vcrwdt_r )
{
	return m_vcrwdt & 0x7f7f;
}

WRITE16_MEMBER( sh2_device::vcrwdt_w )
{
	COMBINE_DATA(&m_vcrwdt);
	// ...
	sh2_recalc_irq();
}

READ32_MEMBER( sh2_device::vcrdiv_r )
{
	return m_vcrdiv & 0x7f;
}

WRITE32_MEMBER( sh2_device::vcrdiv_w )
{
	COMBINE_DATA(&m_vcrdiv);
	// TODO: unemulated, level is seemingly not documented/settable?
	m_irq_vector.divu = data & 0x7f;
	sh2_recalc_irq();
}

/*
 * DIVU
 */

READ32_MEMBER( sh2_device::dvcr_r )
{
	return (m_divu_ovfie == true ? 2 : 0) | (m_divu_ovf == true ? 1 : 0);
}

WRITE32_MEMBER( sh2_device::dvcr_w )
{
	if(ACCESSING_BITS_0_7)
	{
		if (data & 1)
			m_divu_ovf = false;
		if (data & 2)
		{
			m_divu_ovfie = bool(BIT(data, 1));
			if (m_divu_ovfie == true)
				logerror("SH2: unemulated DIVU OVF interrupt enable\n");
		}
		sh2_recalc_irq();
	}
}

READ32_MEMBER( sh2_device::dvsr_r )
{
	return m_dvsr;
}

WRITE32_MEMBER( sh2_device::dvsr_w )
{
	COMBINE_DATA(&m_dvsr);
}

READ32_MEMBER( sh2_device::dvdnt_r )
{
	return m_dvdntl;
}

WRITE32_MEMBER( sh2_device::dvdnt_w )
{
	COMBINE_DATA(&m_dvdntl);
	int32_t a = m_dvdntl;
	int32_t b = m_dvsr;
	LOG("SH2 div32+mod %d/%d\n", a, b);
	if (b)
	{
		m_dvdntl = a / b;
		m_dvdnth = a % b;
	}
	else
	{
		m_divu_ovf = true;
		m_dvdntl = 0x7fffffff;
		m_dvdnth = 0x7fffffff;
		sh2_recalc_irq();
	}
}

READ32_MEMBER( sh2_device::dvdnth_r )
{
	return m_dvdnth;
}

READ32_MEMBER( sh2_device::dvdntl_r )
{
	return m_dvdntl;
}

WRITE32_MEMBER( sh2_device::dvdnth_w )
{
	COMBINE_DATA(&m_dvdnth);
}

WRITE32_MEMBER( sh2_device::dvdntl_w )
{
	COMBINE_DATA(&m_dvdntl);
	int64_t a = m_dvdntl | ((uint64_t)(m_dvdnth) << 32);
	int64_t b = (int32_t)m_dvsr;
	LOG("SH2 div64+mod %d/%d\n", a, b);
	if (b)
	{
		int64_t q = a / b;
		if (q != (int32_t)q)
		{
			m_divu_ovf = true;
			m_dvdntl = 0x7fffffff;
			m_dvdnth = 0x7fffffff;
			sh2_recalc_irq();
		}
		else
		{
			m_dvdntl = q;
			m_dvdnth = a % b;
		}
	}
	else
	{
		m_divu_ovf = true;
		m_dvdntl = 0x7fffffff;
		m_dvdnth = 0x7fffffff;
		sh2_recalc_irq();
	}
}

/*
 * WTC
 */

READ16_MEMBER( sh2_device::wtcnt_r )
{
	return ((m_wtcsr | 0x18) << 8) | (m_wtcnt & 0xff);
}

READ16_MEMBER( sh2_device::rstcsr_r )
{
	return (m_rstcsr & 0xe0) | 0x1f;
}

WRITE16_MEMBER( sh2_device::wtcnt_w )
{
	COMBINE_DATA(&m_wtcw[0]);
	switch (m_wtcw[0] & 0xff00)
	{
		case 0x5a00:
			m_wtcnt = m_wtcw[0] & 0xff;
			break;
		case 0xa500:
			/*
			WTCSR
			x--- ---- Overflow in IT mode
			-x-- ---- Timer mode (0: IT 1: watchdog)
			--x- ---- Timer enable
			---1 1---
			---- -xxx Clock select
			*/
			m_wtcsr = m_wtcw[0] & 0xff;
			break;
	}
}

WRITE16_MEMBER( sh2_device::rstcsr_w )
{
	COMBINE_DATA(&m_wtcw[1]);
	switch (m_wtcw[1] & 0xff00)
	{
		case 0xa500:
			// clear WOVF flag
			if ((m_wtcw[1] & 0x80) == 0)
				m_rstcsr &= 0x7f;
			break;
		case 0x5a00:
			m_rstcsr = (m_rstcsr & 0x80) | (m_wtcw[1] & 0x60);
			break;
	}
}

READ16_MEMBER( sh2_device::fmr_sbycr_r )
{
	return m_sbycr;
}

WRITE16_MEMBER( sh2_device::fmr_sbycr_w )
{
	switch (mem_mask)
	{
	case 0xff00: // FMR 8bit
		logerror("SH2 set clock multiplier x%d\n", 1 << ((data >> 8) & 3));
		break;
	case 0xffff: // FMR 16bit
		// SH7604 docs says FMR register must be set using 8-bit write, however at practice 16-bit works too.
		// has been verified for CPS3 custom SH2, SH7604 and SH7095 (clock multiplier feature is not officially documented for SH7095).
		logerror("SH2 set clock multiplier x%d\n", 1 << (data & 3));
		break;
	case 0x00ff: // SBYCR
		m_sbycr = data;
		if (data & 0x1f)
			logerror("SH2 module stop selected %02x\n", data);
		break;
	}
}

READ8_MEMBER( sh2_device::ccr_r )
{
	return m_ccr & ~0x30;
}

WRITE8_MEMBER( sh2_device::ccr_w )
{
	/*
	    xx-- ---- Way 0/1
	    ---x ---- Cache Purge (CP), write only
	    ---- x--- Two-Way Mode (TW)
	    ---- -x-- Data Replacement Disable (OD)
	    ---- --x- Instruction Replacement Disable (ID)
	    ---- ---x Cache Enable (CE)
	*/
	m_ccr = data;
}

READ32_MEMBER( sh2_device::bcr1_r )
{
	return (m_bcr1 & ~0xe008) | (m_is_slave ? 0x8000 : 0);
}

WRITE32_MEMBER( sh2_device::bcr1_w )
{
	COMBINE_DATA(&m_bcr1);
}

READ32_MEMBER( sh2_device::bcr2_r )
{
	return m_bcr2;
}

WRITE32_MEMBER( sh2_device::bcr2_w )
{
	COMBINE_DATA(&m_bcr2);
}

READ32_MEMBER( sh2_device::wcr_r )
{
	return m_wcr;
}

WRITE32_MEMBER( sh2_device::wcr_w )
{
	COMBINE_DATA(&m_wcr);
}

READ32_MEMBER( sh2_device::mcr_r )
{
	return m_mcr & ~0x103;
}

WRITE32_MEMBER( sh2_device::mcr_w )
{
	COMBINE_DATA(&m_mcr);
}

READ32_MEMBER( sh2_device::rtcsr_r )
{
	return m_rtcsr & 0xf8;
}

WRITE32_MEMBER( sh2_device::rtcsr_w )
{
	COMBINE_DATA(&m_rtcsr);
}

READ32_MEMBER( sh2_device::rtcnt_r )
{
	return m_rtcnt & 0xff;
}

WRITE32_MEMBER( sh2_device::rtcnt_w )
{
	COMBINE_DATA(&m_rtcnt);
	m_rtcnt &= 0xff;
}

READ32_MEMBER( sh2_device::rtcor_r )
{
	return m_rtcor & 0xff;
}

WRITE32_MEMBER( sh2_device::rtcor_w )
{
	COMBINE_DATA(&m_rtcor);
	m_rtcor &= 0xff;
}

void sh2_device::set_frt_input(int state)
{
	if(m_frt_input == state) {
		return;
	}

	m_frt_input = state;

	if(m_frc_tcr & 0x80) {
		if(state == CLEAR_LINE) {
			return;
		}
	} else {
		if(state == ASSERT_LINE) {
			return;
		}
	}

	sh2_timer_resync();
	m_frc_icr = m_frc;
	m_ftcsr |= ICF;
	//logerror("SH2.%s: ICF activated (%x)\n", tag(), m_sh2_state->pc & AM);
	sh2_recalc_irq();
}

void sh2_device::sh2_recalc_irq()
{
	int irq = 0, vector = -1;
	int  level;

	// Timer irqs
	if (m_tier & m_ftcsr & (ICF|OCFA|OCFB|OVF))
	{
		level = (m_irq_level.frc & 15);
		if (level > irq)
		{
			int mask = m_tier & m_ftcsr;
			irq = level;
			if(mask & ICF)
				vector = m_irq_vector.fic & 0x7f;
			else if(mask & (OCFA|OCFB))
				vector = m_irq_vector.foc & 0x7f;
			else
				vector = m_irq_vector.fov & 0x7f;
		}
	}

	// DMA irqs
	if((m_dmac[0].chcr & 6) == 6 && m_dma_irq[0]) {
		level = m_irq_level.dmac & 15;
		if(level > irq) {
			irq = level;
			m_dma_irq[0] &= ~1;
			vector = m_irq_vector.dmac[0] & 0x7f;
		}
	}
	else if((m_dmac[1].chcr & 6) == 6 && m_dma_irq[1]) {
		level = m_irq_level.dmac & 15;
		if(level > irq) {
			irq = level;
			m_dma_irq[1] &= ~1;
			vector = m_irq_vector.dmac[1] & 0x7f;
		}
	}

	m_sh2_state->internal_irq_level = irq;
	m_internal_irq_vector = vector;
	m_test_irq = 1;
}

/*
 SH-7021 on-chip device
 */

void sh2a_device::sh7032_dma_exec(int ch)
{
	const short dma_word_size[4] = { 0, +1, -1, 0 };
	uint8_t rs = (m_dma[ch].chcr >> 8) & 0xf; /**< Resource Select bits */
	if(rs != 0xc) // Auto-Request
	{
		logerror("Warning: SH7032 DMA enables non auto-request transfer\n");
		return;
	}

	// channel enable & master enable
	if((m_dma[ch].chcr & 1) == 0 || (m_dmaor & 1) == 0)
		return;

	printf("%08x %08x %04x\n",m_dma[ch].sar,m_dma[ch].dar,m_dma[ch].chcr);
	uint8_t dm = (m_dma[ch].chcr >> 14) & 3;  /**< Destination Address Mode bits */
	uint8_t sm = (m_dma[ch].chcr >> 12) & 3;  /**< Source Address Mode bits */
	bool ts = (m_dma[ch].chcr & 8);         /**< Transfer Size bit */
	int src_word_size = dma_word_size[sm] * ((ts == true) ? 2 : 1);
	int dst_word_size = dma_word_size[dm] * ((ts == true) ? 2 : 1);
	uint32_t src_addr = m_dma[ch].sar;
	uint32_t dst_addr = m_dma[ch].dar;
	uint32_t size_index = m_dma[ch].tcr;
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
