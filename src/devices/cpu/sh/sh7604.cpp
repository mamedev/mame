// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, R. Belmont
/*****************************************************************************
 *
 *   sh7604.cpp
 *   Portable Hitachi SH-2 (SH7600 family) emulator
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was adapted to the MAME CPU core requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/
/*
TODO: Test and use sh7604_wdt_device, sh7604_sci_device, and sh7604_bus_device as appropriate
*/


#include "emu.h"

#include "sh7604.h"

//#define VERBOSE 1
#include "logmacro.h"


static constexpr int div_tab[4] = { 3, 5, 7, 0 };
static constexpr int wdtclk_tab[8] = { 1, 6, 7, 8, 9, 10, 12, 13 };


DEFINE_DEVICE_TYPE(SH7604,  sh7604_device,  "sh2_7604",  "Hitachi SH-2 (SH7604)")


sh7604_device::sh7604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH7604, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7604_device::sh7604_map), this), 32, 0xc7ffffff)
	, m_test_irq(0), m_internal_irq_vector(0)
	, m_smr(0), m_brr(0), m_scr(0), m_tdr(0), m_ssr(0)
	, m_tier(0), m_ftcsr(0), m_frc_tcr(0), m_tocr(0), m_frc(0), m_ocra(0), m_ocrb(0), m_frc_icr(0)
	, m_ipra(0), m_iprb(0), m_vcra(0), m_vcrb(0), m_vcrc(0), m_vcrd(0), m_vcrwdt(0), m_vcrdiv(0), m_intc_icr(0), m_vecmd(false), m_nmie(false)
	, m_divu_ovf(false), m_divu_ovfie(false), m_dvsr(0), m_dvdntl(0), m_dvdnth(0)
	, m_wtcnt(0), m_wtcsr(0), m_rstcsr(0)
	, m_dmaor(0)
	, m_sbycr(0), m_ccr(0)
	, m_bcr1(0), m_bcr2(0), m_wcr(0), m_mcr(0), m_rtcsr(0), m_rtcor(0), m_rtcnt(0)
	, m_frc_base(0), m_frt_input(0)
	, m_timer(nullptr), m_wdtimer(nullptr)
	, m_is_slave(0)
	, m_dma_kludge_cb(*this)
	, m_dma_fifo_data_available_cb(*this)
	, m_ftcsr_read_cb(*this)
{
	std::fill(std::begin(m_vcrdma), std::end(m_vcrdma), 0);
	std::fill(std::begin(m_dma_timer_active), std::end(m_dma_timer_active), 0);
	std::fill(std::begin(m_dma_irq), std::end(m_dma_irq), 0);
	std::fill(std::begin(m_active_dma_incs), std::end(m_active_dma_incs), 0);
	std::fill(std::begin(m_active_dma_incd), std::end(m_active_dma_incd), 0);
	std::fill(std::begin(m_active_dma_size), std::end(m_active_dma_size), 0);
	std::fill(std::begin(m_active_dma_steal), std::end(m_active_dma_steal), 0);
	std::fill(std::begin(m_active_dma_src), std::end(m_active_dma_src), 0);
	std::fill(std::begin(m_active_dma_dst), std::end(m_active_dma_dst), 0);
	std::fill(std::begin(m_active_dma_count), std::end(m_active_dma_count), 0);
	std::fill(std::begin(m_wtcw), std::end(m_wtcw), 0);
	std::fill(std::begin(m_dma_current_active_timer), std::end(m_dma_current_active_timer), nullptr);

	m_irq_vector.fic = m_irq_vector.foc = m_irq_vector.fov = m_irq_vector.divu = 0;
	std::fill(std::begin(m_irq_vector.dmac), std::end(m_irq_vector.dmac), 0);

	m_irq_level.frc = m_irq_level.sci = m_irq_level.divu = m_irq_level.dmac = m_irq_level.wdt = 0;

	for (int i = 0; i < 2; i++)
		m_dmac[i].drcr = m_dmac[i].sar = m_dmac[i].dar = m_dmac[i].tcr = m_dmac[i].chcr = 0;
}

void sh7604_device::device_start()
{
	sh2_device::device_start();

	m_timer = timer_alloc(FUNC(sh7604_device::sh2_timer_callback), this);
	m_timer->adjust(attotime::never);
	m_wdtimer = timer_alloc(FUNC(sh7604_device::sh2_wdtimer_callback), this);
	m_wdtimer->adjust(attotime::never);

	m_dma_current_active_timer[0] = timer_alloc(FUNC(sh7604_device::sh2_dma_current_active_callback), this);
	m_dma_current_active_timer[0]->adjust(attotime::never);

	m_dma_current_active_timer[1] = timer_alloc(FUNC(sh7604_device::sh2_dma_current_active_callback), this);
	m_dma_current_active_timer[1]->adjust(attotime::never);

	/* resolve callbacks */
	m_dma_kludge_cb.resolve();
	m_dma_fifo_data_available_cb.resolve();
	m_ftcsr_read_cb.resolve();

	// SCI
	save_item(NAME(m_smr));
	save_item(NAME(m_brr));
	save_item(NAME(m_scr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_ssr));

	// FRT / FRC
	save_item(NAME(m_tier));
	save_item(NAME(m_ftcsr));
	save_item(NAME(m_frc_tcr));
	save_item(NAME(m_tocr));
	save_item(NAME(m_frc));
	save_item(NAME(m_ocra));
	save_item(NAME(m_ocrb));
	save_item(NAME(m_frc_icr));
	save_item(NAME(m_frc_base));
	save_item(NAME(m_frt_input));

	// INTC
	save_item(NAME(m_irq_level.frc));
	save_item(NAME(m_irq_level.sci));
	save_item(NAME(m_irq_level.divu));
	save_item(NAME(m_irq_level.dmac));
	save_item(NAME(m_irq_level.wdt));
	save_item(NAME(m_irq_vector.fic));
	save_item(NAME(m_irq_vector.foc));
	save_item(NAME(m_irq_vector.fov));
	save_item(NAME(m_irq_vector.divu));
	save_item(NAME(m_irq_vector.dmac));

	save_item(NAME(m_ipra));
	save_item(NAME(m_iprb));
	save_item(NAME(m_vcra));
	save_item(NAME(m_vcrb));
	save_item(NAME(m_vcrc));
	save_item(NAME(m_vcrd));
	save_item(NAME(m_vcrwdt));
	save_item(NAME(m_vcrdiv));
	save_item(NAME(m_intc_icr));
	save_item(NAME(m_vcrdma));

	save_item(NAME(m_vecmd));
	save_item(NAME(m_nmie));

	// DIVU
	save_item(NAME(m_divu_ovf));
	save_item(NAME(m_divu_ovfie));
	save_item(NAME(m_dvsr));
	save_item(NAME(m_dvdntl));
	save_item(NAME(m_dvdnth));

	// WTC
	save_item(NAME(m_wtcnt));
	save_item(NAME(m_wtcsr));
	save_item(NAME(m_rstcsr));
	save_item(NAME(m_wtcw));

	// UBC
	save_item(NAME(m_barah));
	save_item(NAME(m_baral));
	save_item(NAME(m_barbh));
	save_item(NAME(m_barbl));

	// DMAC
	save_item(NAME(m_dmaor));
	save_item(STRUCT_MEMBER(m_dmac, drcr));
	save_item(STRUCT_MEMBER(m_dmac, sar));
	save_item(STRUCT_MEMBER(m_dmac, dar));
	save_item(STRUCT_MEMBER(m_dmac, tcr));
	save_item(STRUCT_MEMBER(m_dmac, chcr));

	// misc
	save_item(NAME(m_sbycr));
	save_item(NAME(m_ccr));

	// BSC
	save_item(NAME(m_bcr1));
	save_item(NAME(m_bcr2));
	save_item(NAME(m_wcr));
	save_item(NAME(m_mcr));
	save_item(NAME(m_rtcsr));
	save_item(NAME(m_rtcor));
	save_item(NAME(m_rtcnt));
}

void sh7604_device::device_reset()
{
	sh2_device::device_reset();

	m_frc = 0;
	m_ocra = 0;
	m_ocrb = 0;
	m_frc_icr = 0;
	m_frc_base = 0;
	m_frt_input = 0;

	for (int i = 0; i < 2; i++)
	{
		m_dma_timer_active[i] = 0;
		m_dma_irq[i] = 0;
		m_active_dma_incs[i] = 0;
		m_active_dma_incd[i] = 0;
		m_active_dma_size[i] = 0;
		m_active_dma_steal[i] = 0;
		m_active_dma_src[i] = 0;
		m_active_dma_dst[i] = 0;
		m_active_dma_count[i] = 0;
	}

	m_wtcnt = 0;
	m_wtcsr = 0;

	m_barah = 0;
	m_baral = 0;
	m_barbh = 0;
	m_barbl = 0;
}

void sh7604_device::sh7604_map(address_map &map)
{
	map(0x40000000, 0xbfffffff).r(FUNC(sh7604_device::sh2_internal_a5));

//  TODO: cps3boot breaks with this enabled. Needs callback
//  map(0xc0000000, 0xc0000fff).ram(); // cache data array

//  map(0xe0000000, 0xe00001ff).mirror(0x1ffffe00).rw(FUNC(sh7604_device::sh7604_r), FUNC(sh7604_device::sh7604_w));
	// TODO: internal map takes way too much resources if mirrored with 0x1ffffe00
	//       we eventually internalize again via trampoline & sh7604_device
	//       Also area 0xffff8000-0xffffbfff is for synchronous DRAM mode,
	//       so this isn't actually a full mirror
	// SCI
	map(0xfffffe00, 0xfffffe00).rw(FUNC(sh7604_device::smr_r), FUNC(sh7604_device::smr_w));
	map(0xfffffe01, 0xfffffe01).rw(FUNC(sh7604_device::brr_r), FUNC(sh7604_device::brr_w));
	map(0xfffffe02, 0xfffffe02).rw(FUNC(sh7604_device::scr_r), FUNC(sh7604_device::scr_w));
	map(0xfffffe03, 0xfffffe03).rw(FUNC(sh7604_device::tdr_r), FUNC(sh7604_device::tdr_w));
	map(0xfffffe04, 0xfffffe04).rw(FUNC(sh7604_device::ssr_r), FUNC(sh7604_device::ssr_w));
	map(0xfffffe05, 0xfffffe05).r(FUNC(sh7604_device::rdr_r));

	// FRC
	map(0xfffffe10, 0xfffffe10).rw(FUNC(sh7604_device::tier_r), FUNC(sh7604_device::tier_w));
	map(0xfffffe11, 0xfffffe11).rw(FUNC(sh7604_device::ftcsr_r), FUNC(sh7604_device::ftcsr_w));
	map(0xfffffe12, 0xfffffe13).rw(FUNC(sh7604_device::frc_r), FUNC(sh7604_device::frc_w));
	map(0xfffffe14, 0xfffffe15).rw(FUNC(sh7604_device::ocra_b_r), FUNC(sh7604_device::ocra_b_w));
	map(0xfffffe16, 0xfffffe16).rw(FUNC(sh7604_device::frc_tcr_r), FUNC(sh7604_device::frc_tcr_w));
	map(0xfffffe17, 0xfffffe17).rw(FUNC(sh7604_device::tocr_r), FUNC(sh7604_device::tocr_w));
	map(0xfffffe18, 0xfffffe19).r(FUNC(sh7604_device::frc_icr_r));

	// INTC
	map(0xfffffe60, 0xfffffe61).rw(FUNC(sh7604_device::iprb_r), FUNC(sh7604_device::iprb_w));
	map(0xfffffe62, 0xfffffe63).rw(FUNC(sh7604_device::vcra_r), FUNC(sh7604_device::vcra_w));
	map(0xfffffe64, 0xfffffe65).rw(FUNC(sh7604_device::vcrb_r), FUNC(sh7604_device::vcrb_w));
	map(0xfffffe66, 0xfffffe67).rw(FUNC(sh7604_device::vcrc_r), FUNC(sh7604_device::vcrc_w));
	map(0xfffffe68, 0xfffffe69).rw(FUNC(sh7604_device::vcrd_r), FUNC(sh7604_device::vcrd_w));

	map(0xfffffe71, 0xfffffe71).rw(FUNC(sh7604_device::drcr_r<0>), FUNC(sh7604_device::drcr_w<0>));
	map(0xfffffe72, 0xfffffe72).rw(FUNC(sh7604_device::drcr_r<1>), FUNC(sh7604_device::drcr_w<1>));

	// WTC
	map(0xfffffe80, 0xfffffe81).rw(FUNC(sh7604_device::wtcnt_r), FUNC(sh7604_device::wtcnt_w));
	map(0xfffffe82, 0xfffffe83).rw(FUNC(sh7604_device::rstcsr_r), FUNC(sh7604_device::rstcsr_w));

	// standby and cache control
	map(0xfffffe90, 0xfffffe91).rw(FUNC(sh7604_device::fmr_sbycr_r), FUNC(sh7604_device::fmr_sbycr_w));
	map(0xfffffe92, 0xfffffe92).rw(FUNC(sh7604_device::ccr_r), FUNC(sh7604_device::ccr_w));

	// INTC second section
	map(0xfffffee0, 0xfffffee1).rw(FUNC(sh7604_device::intc_icr_r), FUNC(sh7604_device::intc_icr_w));
	map(0xfffffee2, 0xfffffee3).rw(FUNC(sh7604_device::ipra_r), FUNC(sh7604_device::ipra_w));
	map(0xfffffee4, 0xfffffee5).rw(FUNC(sh7604_device::vcrwdt_r), FUNC(sh7604_device::vcrwdt_w));

	// DIVU
	map(0xffffff00, 0xffffff03).rw(FUNC(sh7604_device::dvsr_r), FUNC(sh7604_device::dvsr_w));
	map(0xffffff04, 0xffffff07).rw(FUNC(sh7604_device::dvdnt_r), FUNC(sh7604_device::dvdnt_w));
	map(0xffffff08, 0xffffff0b).rw(FUNC(sh7604_device::dvcr_r), FUNC(sh7604_device::dvcr_w));
	// INTC third section
	map(0xffffff0c, 0xffffff0f).rw(FUNC(sh7604_device::vcrdiv_r), FUNC(sh7604_device::vcrdiv_w));
	// DIVU continued (64-bit plus mirrors)
	map(0xffffff10, 0xffffff13).rw(FUNC(sh7604_device::dvdnth_r), FUNC(sh7604_device::dvdnth_w));
	map(0xffffff14, 0xffffff17).rw(FUNC(sh7604_device::dvdntl_r), FUNC(sh7604_device::dvdntl_w));
	map(0xffffff18, 0xffffff1b).r(FUNC(sh7604_device::dvdnth_r));
	map(0xffffff1c, 0xffffff1f).r(FUNC(sh7604_device::dvdntl_r));

	// UBC
	map(0xffffff40, 0xffffff41).rw(FUNC(sh7604_device::barah_r), FUNC(sh7604_device::barah_w));
	map(0xffffff42, 0xffffff43).rw(FUNC(sh7604_device::baral_r), FUNC(sh7604_device::baral_w));
//  map(0xffffff44, 0xffffff45).rw(FUNC(sh7604_device::bamrah_r), FUNC(sh7604_device::bamrah_w));
//  map(0xffffff46, 0xffffff47).rw(FUNC(sh7604_device::bamral_r), FUNC(sh7604_device::bamral_w));
//  map(0xffffff48, ).rw(FUNC(sh7604_device::bbra_r), FUNC(sh7604_device::bbra_w));

	map(0xffffff60, 0xffffff61).rw(FUNC(sh7604_device::barbh_r), FUNC(sh7604_device::barbh_w));
	map(0xffffff62, 0xffffff63).rw(FUNC(sh7604_device::barbl_r), FUNC(sh7604_device::barbl_w));
//  map(0xffffff64, 0xffffff65).rw(FUNC(sh7604_device::bamrbh_r), FUNC(sh7604_device::bamrbh_w));
//  map(0xffffff66, 0xffffff67).rw(FUNC(sh7604_device::bamrbl_r), FUNC(sh7604_device::bamrbl_w));
//  map(0xffffff68, ).rw(FUNC(sh7604_device::bbrb_r), FUNC(sh7604_device::bbrb_w));
//  map(0xffffff70, 0xffffff71).rw(FUNC(sh7604_device::bdrbh_r), FUNC(sh7604_device::bdrbh_w));
//  map(0xffffff72, 0xffffff73).rw(FUNC(sh7604_device::bdrbl_r), FUNC(sh7604_device::bdrbl_w));
//  map(0xffffff74, 0xffffff75).rw(FUNC(sh7604_device::bdmrbh_r), FUNC(sh7604_device::bdmrbh_w));
//  map(0xffffff76, 0xffffff77).rw(FUNC(sh7604_device::bdmrbl_r), FUNC(sh7604_device::bdmrbl_w));
//  map(0xffffff78, 0xffffff79).rw(FUNC(sh7604_device::brcr_r), FUNC(sh7604_device::brcr_w));

	// DMAC
	map(0xffffff80, 0xffffff83).rw(FUNC(sh7604_device::sar_r<0>), FUNC(sh7604_device::sar_w<0>));
	map(0xffffff84, 0xffffff87).rw(FUNC(sh7604_device::dar_r<0>), FUNC(sh7604_device::dar_w<0>));
	map(0xffffff88, 0xffffff8b).rw(FUNC(sh7604_device::dmac_tcr_r<0>), FUNC(sh7604_device::dmac_tcr_w<0>));
	map(0xffffff8c, 0xffffff8f).rw(FUNC(sh7604_device::chcr_r<0>), FUNC(sh7604_device::chcr_w<0>));

	map(0xffffff90, 0xffffff93).rw(FUNC(sh7604_device::sar_r<1>), FUNC(sh7604_device::sar_w<1>));
	map(0xffffff94, 0xffffff97).rw(FUNC(sh7604_device::dar_r<1>), FUNC(sh7604_device::dar_w<1>));
	map(0xffffff98, 0xffffff9b).rw(FUNC(sh7604_device::dmac_tcr_r<1>), FUNC(sh7604_device::dmac_tcr_w<1>));
	map(0xffffff9c, 0xffffff9f).rw(FUNC(sh7604_device::chcr_r<1>), FUNC(sh7604_device::chcr_w<1>));

	map(0xffffffa0, 0xffffffa3).rw(FUNC(sh7604_device::vcrdma_r<0>), FUNC(sh7604_device::vcrdma_w<0>));
	map(0xffffffa8, 0xffffffab).rw(FUNC(sh7604_device::vcrdma_r<1>), FUNC(sh7604_device::vcrdma_w<1>));
	map(0xffffffb0, 0xffffffb3).rw(FUNC(sh7604_device::dmaor_r), FUNC(sh7604_device::dmaor_w));

	// BSC
	map(0xffffffe0, 0xffffffe3).rw(FUNC(sh7604_device::bcr1_r), FUNC(sh7604_device::bcr1_w));
	map(0xffffffe4, 0xffffffe7).rw(FUNC(sh7604_device::bcr2_r), FUNC(sh7604_device::bcr2_w));
	map(0xffffffe8, 0xffffffeb).rw(FUNC(sh7604_device::wcr_r), FUNC(sh7604_device::wcr_w));
	map(0xffffffec, 0xffffffef).rw(FUNC(sh7604_device::mcr_r), FUNC(sh7604_device::mcr_w));
	map(0xfffffff0, 0xfffffff3).rw(FUNC(sh7604_device::rtcsr_r), FUNC(sh7604_device::rtcsr_w));
	map(0xfffffff4, 0xfffffff7).rw(FUNC(sh7604_device::rtcnt_r), FUNC(sh7604_device::rtcnt_w));
	map(0xfffffff8, 0xfffffffb).rw(FUNC(sh7604_device::rtcor_r), FUNC(sh7604_device::rtcor_w));
}


void sh7604_device::sh2_exception(const char *message, int irqline)
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
			LOG("SH-2 exception #%d (internal vector: $%x) after [%s]\n", irqline, vector, message);
		}
		else
		{
			if (m_vecmd)
			{
				vector = standard_irq_callback(irqline, m_sh2_state->pc);
				LOG("SH-2 exception #%d (external vector: $%x) after [%s]\n", irqline, vector, message);
			}
			else
			{
				standard_irq_callback(irqline, m_sh2_state->pc);
				vector = 64 + irqline/2;
				LOG("SH-2 exception #%d (autovector: $%x) after [%s]\n", irqline, vector, message);
			}
		}
	}
	else
	{
		vector = 11;
		LOG("SH-2 nmi exception (autovector: $%x) after [%s]\n", vector, message);
	}

	sh2_exception_internal(message, irqline, vector);
}

uint32_t sh7604_device::sh2_internal_a5()
{
	return 0xa5a5a5a5;
}

void sh7604_device::sh2_timer_resync()
{
	// TODO: setting 3 is "External clock: count on rising edge"
	int divider = div_tab[m_frc_tcr & 3];
	uint64_t cur_time = total_cycles();
	uint64_t add = (cur_time - m_frc_base) >> divider;

	if (add > 0)
	{
		if (divider)
			m_frc += add;

		m_frc_base = cur_time;
	}
}

void sh7604_device::sh2_timer_activate()
{
	int max_delta = 0xfffff;

	m_timer->adjust(attotime::never);

	uint16_t frc = m_frc;
	if (!(m_ftcsr & OCFA))
	{
		uint16_t delta = m_ocra - frc;
		if (delta < max_delta)
			max_delta = delta;
	}

	if (!(m_ftcsr & OCFB) && (m_ocra <= m_ocrb || !(m_ftcsr & CCLRA)))
	{
		uint16_t delta = m_ocrb - frc;
		if (delta < max_delta)
			max_delta = delta;
	}

	if (!(m_ftcsr & OVF) && !(m_ftcsr & CCLRA))
	{
		int delta = 0x10000 - frc;
		if (delta < max_delta)
			max_delta = delta;
	}

	if (max_delta != 0xfffff)
	{
		int divider = div_tab[m_frc_tcr & 3];
		if (divider)
		{
			max_delta <<= divider;
			m_frc_base = total_cycles();
			m_timer->adjust(cycles_to_attotime(max_delta));
		}
		else
		{
			logerror("SH2.%s: Timer event in %d cycles of external clock", tag(), max_delta);
		}
	}
}

TIMER_CALLBACK_MEMBER(sh7604_device::sh2_timer_callback)
{
	sh2_timer_resync();
	uint16_t frc = m_frc;

	if (frc == m_ocrb)
		m_ftcsr |= OCFB;

	if (frc == 0x0000)
		m_ftcsr |= OVF;

	if (frc == m_ocra)
	{
		m_ftcsr |= OCFA;

		if (m_ftcsr & CCLRA)
			m_frc = 0;
	}

	sh2_recalc_irq();
	sh2_timer_activate();
}

void sh7604_device::sh2_wtcnt_recalc()
{
	if (m_wdtimer->expire() != attotime::never)
		m_wtcnt = 0x100 - (attotime_to_cycles(m_wdtimer->remaining()) >> wdtclk_tab[m_wtcsr & 7]);
}

void sh7604_device::sh2_wdt_activate()
{
	m_wdtimer->adjust(cycles_to_attotime((0x100 - m_wtcnt) << wdtclk_tab[m_wtcsr & 7]));
}

TIMER_CALLBACK_MEMBER(sh7604_device::sh2_wdtimer_callback)
{
	m_wtcnt = 0;
	if (!(m_wtcsr & 0x40))  // timer mode
	{
		m_wtcsr |= 0x80;
		sh2_recalc_irq();
		sh2_wdt_activate();
	}
	else // watchdog mode
	{
		m_rstcsr |= 0x80;
		// TODO reset and /WDTOVF out
	}
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



void sh7604_device::sh2_notify_dma_data_available()
{
	//printf("call notify\n");

	for (int dmach=0;dmach<2;dmach++)
	{
		//printf("m_dma_timer_active[dmach] %04x\n", m_dma_timer_active[dmach]);

		if (m_dma_timer_active[dmach]==2) // 2 = stalled
		{
			//printf("resuming stalled dma\n");
			m_dma_timer_active[dmach]=1;
			m_dma_current_active_timer[dmach]->adjust(attotime::zero, dmach);
		}
	}

}

void sh7604_device::sh2_do_dma(int dmach)
{
	if (m_active_dma_count[dmach] > 0)
	{
		// process current DMA
		switch (m_active_dma_size[dmach])
		{
		case 0:
		{
			// we need to know the src / dest ahead of time without changing them
			// to allow for the callback to check if we can process the DMA at this
			// time (we need to know where we're reading / writing to/from)

			uint32_t tempsrc = m_active_dma_src[dmach];
			if (m_active_dma_incs[dmach] == 2)
				tempsrc--;

			uint32_t tempdst = m_active_dma_dst[dmach];
			if (m_active_dma_incd[dmach] == 2)
				tempdst--;

			if (!m_dma_fifo_data_available_cb.isnull())
			{
				int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

				if (!available)
				{
					//printf("dma stalled\n");
					m_dma_timer_active[dmach] = 2; // mark as stalled
					return;
				}
			}

			//schedule next DMA callback
			m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

			uint32_t dmadata = m_program->read_byte(tempsrc);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_byte(tempdst, dmadata);

			if (m_active_dma_incs[dmach] == 2)
				m_active_dma_src[dmach]--;
			if (m_active_dma_incd[dmach] == 2)
				m_active_dma_dst[dmach]--;

			if (m_active_dma_incs[dmach] == 1)
				m_active_dma_src[dmach]++;
			if (m_active_dma_incd[dmach] == 1)
				m_active_dma_dst[dmach]++;

			m_active_dma_count[dmach]--;
			break;
		}

		case 1:
		{
			uint32_t tempsrc = m_active_dma_src[dmach];
			if (m_active_dma_incs[dmach] == 2)
				tempsrc -= 2;

			uint32_t tempdst = m_active_dma_dst[dmach];
			if (m_active_dma_incd[dmach] == 2)
				tempdst -= 2;

			if (!m_dma_fifo_data_available_cb.isnull())
			{
				int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

				if (!available)
				{
					//printf("dma stalled\n");
					m_dma_timer_active[dmach] = 2; // mark as stalled
					return;
				}
			}

			//schedule next DMA callback
			m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

			// check: should this really be using read_word_32 / write_word_32?
			uint32_t dmadata = m_program->read_word(tempsrc);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_word(tempdst, dmadata);

			if (m_active_dma_incs[dmach] == 2)
				m_active_dma_src[dmach] -= 2;
			if (m_active_dma_incd[dmach] == 2)
				m_active_dma_dst[dmach] -= 2;

			if (m_active_dma_incs[dmach] == 1)
				m_active_dma_src[dmach] += 2;
			if (m_active_dma_incd[dmach] == 1)
				m_active_dma_dst[dmach] += 2;

			m_active_dma_count[dmach]--;
			break;
		}

		case 2:
		{
			uint32_t tempsrc = m_active_dma_src[dmach];
			if (m_active_dma_incs[dmach] == 2)
				tempsrc -= 4;

			uint32_t tempdst = m_active_dma_dst[dmach];
			if (m_active_dma_incd[dmach] == 2)
				tempdst -= 4;

			if (!m_dma_fifo_data_available_cb.isnull())
			{
				int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

				if (!available)
				{
					//printf("dma stalled\n");
					m_dma_timer_active[dmach] = 2; // mark as stalled
					return;
				}
			}

			//schedule next DMA callback
			m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

			uint32_t dmadata = m_program->read_dword(tempsrc);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_dword(tempdst, dmadata);

			if (m_active_dma_incs[dmach] == 2)
				m_active_dma_src[dmach] -= 4;
			if (m_active_dma_incd[dmach] == 2)
				m_active_dma_dst[dmach] -= 4;

			if (m_active_dma_incs[dmach] == 1)
				m_active_dma_src[dmach] += 4;
			if (m_active_dma_incd[dmach] == 1)
				m_active_dma_dst[dmach] += 4;

			m_active_dma_count[dmach]--;
			break;
		}

		case 3:
		{
			// shouldn't this really be 4 calls here instead?

			uint32_t tempsrc = m_active_dma_src[dmach];

			uint32_t tempdst = m_active_dma_dst[dmach];
			if (m_active_dma_incd[dmach] == 2)
				tempdst -= 16;

			if (!m_dma_fifo_data_available_cb.isnull())
			{
				int available = m_dma_fifo_data_available_cb(tempsrc, tempdst, 0, m_active_dma_size[dmach]);

				if (!available)
				{
					//printf("dma stalled\n");
					m_dma_timer_active[dmach] = 2; // mark as stalled
					fatalerror("SH2 dma_callback_fifo_data_available == 0 in unsupported mode\n");
				}
			}

			//schedule next DMA callback
			m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);

			uint32_t dmadata = m_program->read_dword(tempsrc);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_dword(tempdst, dmadata);

			dmadata = m_program->read_dword(tempsrc + 4);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_dword(tempdst + 4, dmadata);

			dmadata = m_program->read_dword(tempsrc + 8);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_dword(tempdst + 8, dmadata);

			dmadata = m_program->read_dword(tempsrc + 12);
			if (!m_dma_kludge_cb.isnull())
				dmadata = m_dma_kludge_cb(tempsrc, tempdst, dmadata, m_active_dma_size[dmach]);
			m_program->write_dword(tempdst + 12, dmadata);

			if (m_active_dma_incd[dmach] == 2)
				m_active_dma_dst[dmach] -= 16;

			m_active_dma_src[dmach] += 16;
			if (m_active_dma_incd[dmach] == 1)
				m_active_dma_dst[dmach] += 16;

			m_active_dma_count[dmach] -= 4;
			break;
		}
		}
	}
	else // the dma is complete
	{
		// int dma = param & 1;

		// fever soccer uses cycle-stealing mode, resume the CPU now DMA has finished
		if (m_active_dma_steal[dmach])
		{
			resume(SUSPEND_REASON_HALT);
		}

		LOG("SH2: DMA %d complete\n", dmach);
		m_dmac[dmach].tcr = 0;
		m_dmac[dmach].chcr |= 2;
		m_dma_timer_active[dmach] = 0;
		m_dma_irq[dmach] |= 1;
		sh2_recalc_irq();

	}
}

TIMER_CALLBACK_MEMBER(sh7604_device::sh2_dma_current_active_callback)
{
	sh2_do_dma(param & 1);
}


void sh7604_device::sh2_dmac_check(int dmach)
{
	if (m_dmac[dmach].chcr & m_dmaor & 1)
	{
		if (!m_dma_timer_active[dmach] && !(m_dmac[dmach].chcr & 2))
		{
			m_active_dma_incd[dmach] = (m_dmac[dmach].chcr >> 14) & 3;
			m_active_dma_incs[dmach] = (m_dmac[dmach].chcr >> 12) & 3;
			m_active_dma_size[dmach] = (m_dmac[dmach].chcr >> 10) & 3;
			m_active_dma_steal[dmach] = (m_dmac[dmach].chcr & 0x10);

			if (m_active_dma_incd[dmach] == 3 || m_active_dma_incs[dmach] == 3)
			{
				LOG("SH2: DMA: bad increment values (%d, %d, %d, %04x)\n", m_active_dma_incd[dmach], m_active_dma_incs[dmach], m_active_dma_size[dmach], m_dmac[dmach].chcr);
				return;
			}
			m_active_dma_src[dmach]   = m_dmac[dmach].sar;
			m_active_dma_dst[dmach]   = m_dmac[dmach].dar;
			m_active_dma_count[dmach] = m_dmac[dmach].tcr;
			if (!m_active_dma_count[dmach])
				m_active_dma_count[dmach] = 0x1000000;

			LOG("SH2: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", dmach, m_active_dma_src[dmach], m_active_dma_dst[dmach], m_active_dma_count[dmach], m_dmac[dmach].chcr, m_active_dma_incs[dmach], m_active_dma_incd[dmach], m_active_dma_size[dmach]);

			m_dma_timer_active[dmach] = 1;

			m_active_dma_src[dmach] &= m_am;
			m_active_dma_dst[dmach] &= m_am;

			switch (m_active_dma_size[dmach])
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
				suspend(SUSPEND_REASON_HALT, 1);
			}

			m_dma_current_active_timer[dmach]->adjust(cycles_to_attotime(2), dmach);
		}
	}
	else
	{
		if (m_dma_timer_active[dmach])
		{
			LOG("SH2: DMA %d cancelled in-flight\n", dmach);
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

uint8_t sh7604_device::smr_r()
{
	return m_smr;
}

void sh7604_device::smr_w(uint8_t data)
{
	m_smr = data;
}

uint8_t sh7604_device::brr_r()
{
	return m_brr;
}

void sh7604_device::brr_w(uint8_t data)
{
	m_brr = data;
}

uint8_t sh7604_device::scr_r()
{
	return m_scr;
}

void sh7604_device::scr_w(uint8_t data)
{
	m_scr = data;
}

uint8_t sh7604_device::tdr_r()
{
	return m_tdr;
}

void sh7604_device::tdr_w(uint8_t data)
{
	m_tdr = data;
	//printf("%c", data & 0xff);
}

uint8_t sh7604_device::ssr_r()
{
	// 0x84 is needed by EGWord on Saturn to make it to boot for some reason.
	return m_ssr | 0x84;
}

void sh7604_device::ssr_w(uint8_t data)
{
	m_ssr = data;
}

uint8_t sh7604_device::rdr_r()
{
	return 0;
}

/*
 * FRC
 */

uint8_t sh7604_device::tier_r()
{
	return m_tier;
}

void sh7604_device::tier_w(uint8_t data)
{
	sh2_timer_resync();
	m_tier = data;
	sh2_timer_activate();
	sh2_recalc_irq();
}

uint8_t sh7604_device::ftcsr_r()
{
	// TODO: to be tested
	if (!m_ftcsr_read_cb.isnull())
		m_ftcsr_read_cb((((m_tier << 24) | (m_ftcsr << 16)) & 0xffff0000) | m_frc);

	return m_ftcsr;
}

void sh7604_device::ftcsr_w(uint8_t data)
{
	uint8_t old = m_ftcsr;

	m_ftcsr = data;
	sh2_timer_resync();
	m_ftcsr = (m_ftcsr & ~(ICF | OCFA | OCFB | OVF)) | (old & m_ftcsr & (ICF | OCFA | OCFB | OVF));
	sh2_timer_activate();
	sh2_recalc_irq();
}

uint16_t sh7604_device::frc_r()
{
	sh2_timer_resync();
	return m_frc;
}

void sh7604_device::frc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	sh2_timer_resync();
	COMBINE_DATA(&m_frc);
	sh2_timer_activate();
	sh2_recalc_irq();
}

uint16_t sh7604_device::ocra_b_r()
{
	return (m_tocr & 0x10) ? m_ocrb : m_ocra;
}

void sh7604_device::ocra_b_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	sh2_timer_resync();
	if (m_tocr & 0x10)
		m_ocrb = (m_ocrb & ~mem_mask) | (data & mem_mask);
	else
		m_ocra = (m_ocra & ~mem_mask) | (data & mem_mask);
	sh2_timer_activate();
	sh2_recalc_irq();
}

uint8_t sh7604_device::frc_tcr_r()
{
	return m_frc_tcr & 0x83;
}

void sh7604_device::frc_tcr_w(uint8_t data)
{
	sh2_timer_resync();
	m_frc_tcr = data & 0x83;
	sh2_timer_activate();
	sh2_recalc_irq();
}

uint8_t sh7604_device::tocr_r()
{
	return (m_tocr & 0x13) | 0xe0;
}

void sh7604_device::tocr_w(uint8_t data)
{
	sh2_timer_resync();
	// TODO: output levels A/B (bits 1-0)
	m_tocr = data & 0x13;
	sh2_timer_activate();
	sh2_recalc_irq();
}

uint16_t sh7604_device::frc_icr_r()
{
	return m_frc_icr;
}

/*
 * INTC
 */

uint16_t sh7604_device::intc_icr_r()
{
	// TODO: flip meaning based off NMI edge select bit (NMIE)
	uint16_t nmilv = m_nmi_line_state == ASSERT_LINE ? 0 : 0x8000;
	return nmilv | (m_intc_icr & 0x0101);
}

void sh7604_device::intc_icr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_intc_icr);
	m_nmie = BIT(m_intc_icr, 8);
	m_vecmd = BIT(m_intc_icr, 0);
}

uint16_t sh7604_device::ipra_r()
{
	return m_ipra & 0xfff0;
}

void sh7604_device::ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ipra);
	m_irq_level.divu = (m_ipra >> 12) & 0xf;
	m_irq_level.dmac = (m_ipra >> 8) & 0xf;
	m_irq_level.wdt = (m_ipra >> 4) & 0xf;
	sh2_recalc_irq();
}

uint16_t sh7604_device::iprb_r()
{
	return m_iprb & 0xff00;
}

void sh7604_device::iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprb);
	m_irq_level.sci = (m_iprb >> 12) & 0xf;
	m_irq_level.frc = (m_iprb >> 8) & 0xf;
	sh2_recalc_irq();
}

uint16_t sh7604_device::vcra_r()
{
	return m_vcra & 0x7f7f;
}

void sh7604_device::vcra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vcra);
	// ...
	sh2_recalc_irq();
}

uint16_t sh7604_device::vcrb_r()
{
	return m_vcrb;
}

void sh7604_device::vcrb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vcrb);
	// ...
	sh2_recalc_irq();
}

uint16_t sh7604_device::vcrc_r()
{
	return m_vcrc & 0x7f7f;
}

void sh7604_device::vcrc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vcrc);
	m_irq_vector.fic = (m_vcrc >> 8) & 0x7f;
	m_irq_vector.foc = (m_vcrc >> 0) & 0x7f;
	sh2_recalc_irq();
}

uint16_t sh7604_device::vcrd_r()
{
	return m_vcrd & 0x7f00;
}

void sh7604_device::vcrd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vcrd);
	m_irq_vector.fov = (m_vcrc >> 8) & 0x7f;
	sh2_recalc_irq();
}

uint16_t sh7604_device::vcrwdt_r()
{
	return m_vcrwdt & 0x7f7f;
}

void sh7604_device::vcrwdt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vcrwdt);
	// ...
	sh2_recalc_irq();
}

uint32_t sh7604_device::vcrdiv_r()
{
	return m_vcrdiv & 0x7f;
}

void sh7604_device::vcrdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vcrdiv);
	// TODO: unemulated, level is seemingly not documented/settable?
	m_irq_vector.divu = data & 0x7f;
	sh2_recalc_irq();
}

/*
 * DIVU
 */

uint32_t sh7604_device::dvcr_r()
{
	return (m_divu_ovfie ? 2 : 0) | (m_divu_ovf ? 1 : 0);
}

void sh7604_device::dvcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 1)
			m_divu_ovf = false;
		if (data & 2)
		{
			m_divu_ovfie = BIT(data, 1);
			if (m_divu_ovfie)
				LOG("SH2: unemulated DIVU OVF interrupt enable\n");
		}
		sh2_recalc_irq();
	}
}

uint32_t sh7604_device::dvsr_r()
{
	return m_dvsr;
}

void sh7604_device::dvsr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dvsr);
}

uint32_t sh7604_device::dvdnt_r()
{
	return m_dvdntl;
}

void sh7604_device::dvdnt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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

uint32_t sh7604_device::dvdnth_r()
{
	return m_dvdnth;
}

uint32_t sh7604_device::dvdntl_r()
{
	return m_dvdntl;
}

void sh7604_device::dvdnth_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dvdnth);
}

void sh7604_device::dvdntl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dvdntl);
	int64_t a = m_dvdntl | ((uint64_t)m_dvdnth << 32);
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
 * UBC
 */

// TODO: bare-bones, used for proper 32x:aburnerju sound (on slave side) as buffer storage

uint16_t sh7604_device::barah_r()
{
	return m_barah;
}

uint16_t sh7604_device::baral_r()
{
	return m_baral;
}

void sh7604_device::barah_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_barah);
}

void sh7604_device::baral_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_baral);
}

uint16_t sh7604_device::barbh_r()
{
	return m_barbh;
}

uint16_t sh7604_device::barbl_r()
{
	return m_barbl;
}

void sh7604_device::barbh_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_barbh);
}

void sh7604_device::barbl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_barbl);
}

/*
 * WTC
 */

uint16_t sh7604_device::wtcnt_r()
{
	sh2_wtcnt_recalc();
	return ((m_wtcsr | 0x18) << 8) | (m_wtcnt & 0xff);
}

uint16_t sh7604_device::rstcsr_r()
{
	return (m_rstcsr & 0xe0) | 0x1f;
}

void sh7604_device::wtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wtcw[0]);
	switch (m_wtcw[0] & 0xff00)
	{
		case 0x5a00:
			m_wtcnt = m_wtcw[0] & 0xff;
			if (m_wtcsr & 0x20)
				sh2_wdt_activate();
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
			sh2_wtcnt_recalc();
			m_wtcsr &= m_wtcw[0] & 0x80;
			m_wtcsr |= m_wtcw[0] & 0x7f;
			if (m_wtcsr & 0x20)
				sh2_wdt_activate();
			else
			{
				m_wtcnt = 0;
				m_wdtimer->adjust(attotime::never);
			}
			sh2_recalc_irq();
			break;
	}
}

void sh7604_device::rstcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

uint16_t sh7604_device::fmr_sbycr_r()
{
	return m_sbycr;
}

void sh7604_device::fmr_sbycr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

uint8_t sh7604_device::ccr_r()
{
	return m_ccr & ~0x30;
}

void sh7604_device::ccr_w(uint8_t data)
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

uint32_t sh7604_device::bcr1_r()
{
	return (m_bcr1 & ~0xe008) | (m_is_slave ? 0x8000 : 0);
}

void sh7604_device::bcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bcr1);
}

uint32_t sh7604_device::bcr2_r()
{
	return m_bcr2;
}

void sh7604_device::bcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bcr2);
}

uint32_t sh7604_device::wcr_r()
{
	return m_wcr;
}

void sh7604_device::wcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_wcr);
}

uint32_t sh7604_device::mcr_r()
{
	return m_mcr & ~0x103;
}

void sh7604_device::mcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_mcr);
}

uint32_t sh7604_device::rtcsr_r()
{
	return m_rtcsr & 0xf8;
}

void sh7604_device::rtcsr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_rtcsr);
}

uint32_t sh7604_device::rtcnt_r()
{
	return m_rtcnt & 0xff;
}

void sh7604_device::rtcnt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_rtcnt);
	m_rtcnt &= 0xff;
}

uint32_t sh7604_device::rtcor_r()
{
	return m_rtcor & 0xff;
}

void sh7604_device::rtcor_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_rtcor);
	m_rtcor &= 0xff;
}

void sh7604_device::set_frt_input(int state)
{
	if (m_frt_input == state)
		return;

	m_frt_input = state;

	if (m_frc_tcr & 0x80)
	{
		if (state == CLEAR_LINE)
			return;
	}
	else
	{
		if (state == ASSERT_LINE)
			return;
	}

	sh2_timer_resync();
	m_frc_icr = m_frc;
	m_ftcsr |= ICF;
	//logerror("SH2.%s: ICF activated (%x)\n", tag(), m_sh2_state->pc & AM);
	sh2_recalc_irq();
}

void sh7604_device::sh2_recalc_irq()
{
	int irq = 0;
	int vector = -1;
	int level;

	// Timer irqs
	if (m_tier & m_ftcsr & (ICF | OCFA | OCFB | OVF))
	{
		level = (m_irq_level.frc & 15);
		if (level > irq)
		{
			int mask = m_tier & m_ftcsr;
			irq = level;
			if (mask & ICF)
				vector = m_irq_vector.fic & 0x7f;
			else if (mask & (OCFA | OCFB))
				vector = m_irq_vector.foc & 0x7f;
			else
				vector = m_irq_vector.fov & 0x7f;
		}
	}

	// WDT irqs
	if (m_wtcsr & 0x80)
	{
		level = m_irq_level.wdt & 15;
		if (level > irq)
		{
			irq = level;
			vector = (m_vcrwdt >> 8) & 0x7f;
		}
	}

	// DMA irqs
	if ((m_dmac[0].chcr & 6) == 6 && m_dma_irq[0])
	{
		level = m_irq_level.dmac & 15;
		if (level > irq)
		{
			irq = level;
			m_dma_irq[0] &= ~1;
			vector = m_irq_vector.dmac[0] & 0x7f;
		}
	}
	else if ((m_dmac[1].chcr & 6) == 6 && m_dma_irq[1])
	{
		level = m_irq_level.dmac & 15;
		if (level > irq)
		{
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
 * DMAC
 */

template <int Channel>
uint32_t sh7604_device::vcrdma_r()
{
	return m_vcrdma[Channel] & 0x7f;
}

template <int Channel>
void sh7604_device::vcrdma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vcrdma[Channel]);
	m_irq_vector.dmac[Channel] = m_vcrdma[Channel] & 0x7f;
	sh2_recalc_irq();
}

template <int Channel>
uint8_t sh7604_device::drcr_r()
{
	return m_dmac[Channel].drcr & 3;
}

template <int Channel>
void sh7604_device::drcr_w(uint8_t data)
{
	m_dmac[Channel].drcr = data & 3;
	sh2_recalc_irq();
}

template <int Channel>
uint32_t sh7604_device::sar_r()
{
	return m_dmac[Channel].sar;
}

template <int Channel>
void sh7604_device::sar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmac[Channel].sar);
}

template <int Channel>
uint32_t sh7604_device::dar_r()
{
	return m_dmac[Channel].dar;
}

template <int Channel>
void sh7604_device::dar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmac[Channel].dar);
}

template <int Channel>
uint32_t sh7604_device::dmac_tcr_r()
{
	return m_dmac[Channel].tcr;
}

template <int Channel>
void sh7604_device::dmac_tcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmac[Channel].tcr);
	m_dmac[Channel].tcr &= 0xffffff;
}

template <int Channel>
uint32_t sh7604_device::chcr_r()
{
	return m_dmac[Channel].chcr;
}

template <int Channel>
void sh7604_device::chcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old;
	old = m_dmac[Channel].chcr;
	COMBINE_DATA(&m_dmac[Channel].chcr);
	m_dmac[Channel].chcr = (data & ~2) | (old & m_dmac[Channel].chcr & 2);
	sh2_dmac_check(Channel);
}

uint32_t sh7604_device::dmaor_r()
{
	return m_dmaor & 0xf;
}

void sh7604_device::dmaor_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		uint8_t old = m_dmaor & 0xf;
		m_dmaor = (data & ~6) | (old & m_dmaor & 6); // TODO: should this be old & data & 6? bug?
		sh2_dmac_check(0);
		sh2_dmac_check(1);
	}
}
