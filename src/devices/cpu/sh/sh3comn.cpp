// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* Handlers for SH3 internals */

#include "emu.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
#include "sh4dmac.h"

// CCN
uint32_t sh3_base_device::pteh_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (PTEH) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_pteh);
	return m_pteh;
}

void sh3_base_device::pteh_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pteh);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (PTEH)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::ptel_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (PTEL) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_ptel);
	return m_ptel;
}

void sh3_base_device::ptel_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ptel);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (PTEL)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::ttb_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (TTB) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_ttb);
	return m_ttb;
}

void sh3_base_device::ttb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ttb);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (TTB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::tea_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (TEA) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_tea);
	return m_tea;
}

void sh3_base_device::tea_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tea);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (TEA)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::mmucr_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (MMUCR) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_mmucr);
	return m_mmucr;
}

void sh3_base_device::mmucr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_mmucr);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (MMUCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::basra_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %02x (BASRA) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_basra);
	return m_basra;
}

void sh3_base_device::basra_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_basra);
	logerror("'%s' (%08x): CCN unmapped internal write %02x & %02x (BASRA)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::basrb_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %02x (BASRB) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_basrb);
	return m_basrb;
}

void sh3_base_device::basrb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_basrb);
	logerror("'%s' (%08x): CCN unmapped internal write %02x & %02x (BASRB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::ccr_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (CCR) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_ccr);
	return m_ccr;
}

void sh3_base_device::ccr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ccr);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (CCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::tra_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (TRA) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_tra);
	return m_tra;
}

void sh3_base_device::tra_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tra);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (TRA)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::expevt_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (EXPEVT) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_expevt);
	return m_expevt;
}

void sh3_base_device::expevt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_expevt);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (EXPEVT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::intevt_r(offs_t offset, uint32_t mem_mask)
{
	//logerror("'%s' (%08x): CCN unmapped internal read mask %08x (INTEVT) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_intevt);
	return m_intevt;
}

void sh3_base_device::intevt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intevt);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (INTEVT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// CCN 7709S
uint32_t sh3_base_device::ccr2_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (CCR2) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_ccr2);
	return m_ccr2;
}

void sh3_base_device::ccr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ccr2);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (CCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// UBC
uint32_t sh3_base_device::bara_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %08x (BARA) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_bara);
	return m_bara;
}

void sh3_base_device::bara_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bara);
	logerror("'%s' (%08x): UBC unmapped internal write %08x & %08x (BARA)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::bamra_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %02x (BAMRA) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_bamra);
	return m_bamra;
}

void sh3_base_device::bamra_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_bamra);
	logerror("'%s' (%08x): UBC unmapped internal write %02x & %02x (BAMRA)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::bbra_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %04x (BBRA) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_bbra);
	return m_bbra;
}

void sh3_base_device::bbra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bbra);
	logerror("'%s' (%08x): UBC unmapped internal write %04x & %04x (BBRA)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::barb_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %08x (BARB) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_barb);
	return m_barb;
}

void sh3_base_device::barb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_barb);
	logerror("'%s' (%08x): UBC unmapped internal write %08x & %08x (BARB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::bamrb_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %02x (BAMRB) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_bamrb);
	return m_bamrb;
}

void sh3_base_device::bamrb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_bamrb);
	logerror("'%s' (%08x): UBC unmapped internal write %02x & %02x (BAMRB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::bbrb_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %04x (BBRB) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_bbrb);
	return m_bbrb;
}

void sh3_base_device::bbrb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bbrb);
	logerror("'%s' (%08x): UBC unmapped internal write %04x & %04x (BBRB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::bdrb_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %08x (BDRB) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_bdrb);
	return m_bdrb;
}

void sh3_base_device::bdrb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bdrb);
	logerror("'%s' (%08x): UBC unmapped internal write %08x & %08x (BDRB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::bdmrb_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %08x (BDMRB) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_bdmrb);
	return m_bdmrb;
}

void sh3_base_device::bdmrb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bdmrb);
	logerror("'%s' (%08x): UBC unmapped internal write %08x & %08x (BDMRB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::brcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %04x (BRCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_brcr);
	return m_brcr;
}

void sh3_base_device::brcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_brcr);
	logerror("'%s' (%08x): UBC unmapped internal write %04x & %04x (BRCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// UBC 7709S
uint16_t sh3_base_device::betr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %04x (BETR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_betr);
	return m_betr;
}

void sh3_base_device::betr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_betr);
	logerror("'%s' (%08x): UBC unmapped internal write %04x & %04x (BETR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::brsr_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %08x (BRSR) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_brsr);
	return m_brsr;
}

void sh3_base_device::brsr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_brsr);
	logerror("'%s' (%08x): UBC unmapped internal write %08x & %08x (BRSR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint32_t sh3_base_device::brdr_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): UBC unmapped internal read mask %08x (BRDR) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_brdr);
	return m_brdr;
}

void sh3_base_device::brdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_brdr);
	logerror("'%s' (%08x): UBC unmapped internal write %08x & %08x (BRDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// CPG
uint16_t sh3_base_device::frqcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): CPG unmapped internal read mask %04x (FRQCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_frqcr);
	return m_frqcr;
}

void sh3_base_device::frqcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_frqcr);
	logerror("'%s' (%08x): CPG unmapped internal write %04x & %04x (FRQCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::stbcr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): CPG unmapped internal read mask %02x (STBCR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_stbcr);
	return m_stbcr;
}

void sh3_base_device::stbcr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_stbcr);
	logerror("'%s' (%08x): CPG unmapped internal write %02x & %02x (STBCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::wtcnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): CPG unmapped internal read mask %02x (WTCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_stbcr);
	return m_wtcnt;
}

void sh3_base_device::wtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((data & 0xff00) == 0x5a00)
		COMBINE_DATA(&m_wtcnt);

	logerror("'%s' (%08x): CPG unmapped internal write %04x & %04x (WTCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::wtcsr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): CPG unmapped internal read mask %02x (WTCSR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_wtcsr);
	return m_wtcsr;
}

void sh3_base_device::wtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((data & 0xff00) == 0xa500)
		COMBINE_DATA(&m_wtcsr);
	logerror("'%s' (%08x): CPG unmapped internal write %04x & %04x (WTCSR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// CPG 7709
uint8_t sh3_base_device::stbcr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): CPG unmapped internal read mask %02x (STBCR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_stbcr2);
	return m_stbcr2;
}

void sh3_base_device::stbcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_stbcr2);
	logerror("'%s' (%08x): CPG unmapped internal write %02x & %02x (STBCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// BSC
uint16_t sh3_base_device::bcr1_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (BCR1) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_bcr1);
	return m_bcr1;
}

void sh3_base_device::bcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr1);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (BCR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::bcr2_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (BCR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_bcr2);
	return m_bcr2;
}

void sh3_base_device::bcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr2);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (BCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::wcr1_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (WCR1) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_wcr1);
	return m_wcr1;
}

void sh3_base_device::wcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wcr1);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (WCR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::wcr2_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (WCR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_wcr2);
	return m_wcr2;
}

void sh3_base_device::wcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wcr2);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (WCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcr);
	return m_mcr;
}

void sh3_base_device::mcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (PCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pcr);
	return m_pcr;
}

void sh3_base_device::pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pcr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (PCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::rtcsr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (RTCSR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_rtcsr);
	return m_rtcsr;
}

void sh3_base_device::rtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcsr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (RTCSR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::rtcnt_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (RTCNT) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_rtcnt);
	return m_rtcnt;
}

void sh3_base_device::rtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcnt);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (RTCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::rtcor_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (RTCOR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_rtcor);
	return m_rtcor;
}

void sh3_base_device::rtcor_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcor);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (RTCOR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::rfcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (RFCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_rfcr);
	return m_rfcr;
}

void sh3_base_device::rfcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rfcr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (RFCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::sdmr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %02x @ %05x (SDMR)\n", tag(), m_sh2_state->pc, mem_mask, offset);
	return 0;
}

void sh3_base_device::sdmr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal write %02x & %02x @ %05x (SDMR)\n", tag(), m_sh2_state->pc, data, mem_mask, offset);
}

// BSC 7708
uint16_t sh3_base_device::dcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (DCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_dcr);
	return m_dcr;
}

void sh3_base_device::dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dcr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (DCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pctr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (PCTR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pctr);
	return m_pctr;
}

void sh3_base_device::pctr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pctr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (PCTR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pdtr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (PDTR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pdtr);
	return m_pdtr;
}

void sh3_base_device::pdtr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pdtr);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (PDTR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// BSC 7709
uint16_t sh3_base_device::bcr3_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (BCR3) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_bcr3);
	return m_bcr3;
}

void sh3_base_device::bcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr3);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (BCR3)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// BSC 7709S
uint16_t sh3_base_device::mcscr0_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR0) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr0);
	return m_mcscr0;
}

void sh3_base_device::mcscr0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr0);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR0)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr1_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR1) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr1);
	return m_mcscr1;
}

void sh3_base_device::mcscr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr1);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr2_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr2);
	return m_mcscr2;
}

void sh3_base_device::mcscr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr2);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr3_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR3) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr3);
	return m_mcscr3;
}

void sh3_base_device::mcscr3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr3);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR3)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr4_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR4) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr4);
	return m_mcscr4;
}

void sh3_base_device::mcscr4_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr4);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR4)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr5_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR5) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr5);
	return m_mcscr5;
}

void sh3_base_device::mcscr5_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr5);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR5)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr6_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR6) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr6);
	return m_mcscr6;
}

void sh3_base_device::mcscr6_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr6);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR6)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::mcscr7_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): BSC unmapped internal read mask %04x (MCSCR7) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_mcscr7);
	return m_mcscr7;
}

void sh3_base_device::mcscr7_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mcscr7);
	logerror("'%s' (%08x): BSC unmapped internal write %04x & %04x (MCSCR7)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// RTC
uint8_t sh3_base_device::r64cnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (R64CNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_r64cnt);
	return m_r64cnt;
}

uint8_t sh3_base_device::rseccnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RSECCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rseccnt);
	return m_rseccnt;
}

void sh3_base_device::rseccnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rseccnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RSECCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rmincnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RMINCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rseccnt);
	return m_rmincnt;
}

void sh3_base_device::rmincnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rmincnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RMINCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rhrcnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RHRCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rhrcnt);
	return m_rhrcnt;
}

void sh3_base_device::rhrcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rhrcnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RHRCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rwkcnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RWKCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rhrcnt);
	return m_rwkcnt;
}

void sh3_base_device::rwkcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rwkcnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RWKCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rdaycnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RDAYCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rdaycnt);
	return m_rdaycnt;
}

void sh3_base_device::rdaycnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rdaycnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RDAYCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rmoncnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RMONCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rmoncnt);
	return m_rmoncnt;
}

void sh3_base_device::rmoncnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rmoncnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RMONCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::ryrcnt_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RYRCNT) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_ryrcnt);
	return m_ryrcnt;
}

void sh3_base_device::ryrcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_ryrcnt);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RYRCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rsecar_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RSECAR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rsecar);
	return m_rsecar;
}

void sh3_base_device::rsecar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rsecar);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RSECAR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rminar_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RMINAR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rminar);
	return m_rminar;
}

void sh3_base_device::rminar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rminar);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RMINAR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rhrar_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RHRAR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rhrar);
	return m_rhrar;
}

void sh3_base_device::rhrar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rhrar);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RHRAR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rwkar_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RWKAR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rhrar);
	return m_rwkar;
}

void sh3_base_device::rwkar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rwkar);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RWKAR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rdayar_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RDAYAR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rdayar);
	return m_rdayar;
}

void sh3_base_device::rdayar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rdayar);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RDAYAR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rmonar_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RMONAR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rmonar);
	return m_rmonar;
}

void sh3_base_device::rmonar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rmonar);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RMONAR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rcr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %02x (RCR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_rcr1);
	return m_rcr1;
}

void sh3_base_device::rcr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rcr1);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RCR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::rcr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): RTC unmapped internal read mask %04x (RCR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_rcr2);
	return m_rcr2;
}

void sh3_base_device::rcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rcr2);
	logerror("'%s' (%08x): RTC unmapped internal write %02x & %02x (RCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// INTC
uint16_t sh3_base_device::icr0_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (ICR0) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_icr0);
	return m_icr0;
}

void sh3_base_device::icr0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_icr0);
	logerror("'%s' (%08x): INTC unmapped internal write %04x & %04x (ICR0)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::iprb_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (IPRB) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_iprb);
	return m_iprb;
}

void sh3_base_device::iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprb);
	logerror("'%s' (%08x): INTC unmapped internal write %04x & %04x (IPRB)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// INTC 7709
uint32_t sh3_base_device::intevt2_r(offs_t offset, uint32_t mem_mask)
{
	//logerror("'%s' (%08x): INTC unmapped internal read mask %08x (INTEVT2) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_intevt2);
	return m_intevt2;
}

void sh3_base_device::intevt2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intevt2);
	logerror("'%s' (%08x): INTC unmapped internal write %08x & %08x (INTEVT2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::irr0_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %02x (IRR0) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_irr0);
	return m_irr0;
}

void sh3_base_device::irr0_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_irr0);
	logerror("'%s' (%08x): INTC unmapped internal write %02x & %02x (IRR0)\n", tag(), m_sh2_state->pc, data, mem_mask);
	//  not sure if this is how we should clear lines in this core...
	if (!(data & 0x01)) execute_set_input(0, CLEAR_LINE);
	if (!(data & 0x02)) execute_set_input(1, CLEAR_LINE);
	if (!(data & 0x04)) execute_set_input(2, CLEAR_LINE);
	if (!(data & 0x08)) execute_set_input(3, CLEAR_LINE);
}

uint8_t sh3_base_device::irr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %02x (IRR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_irr1);
	return m_irr1;
}

void sh3_base_device::irr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_irr1);
	logerror("'%s' (%08x): INTC unmapped internal write %02x & %02x (IRR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::irr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %02x (IRR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_irr2);
	return m_irr2;
}

void sh3_base_device::irr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_irr2);
	logerror("'%s' (%08x): INTC unmapped internal write %02x & %02x (IRR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::icr1_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (ICR1) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_icr1);
	return m_icr1;
}

void sh3_base_device::icr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_icr1);
	logerror("'%s' (%08x): INTC unmapped internal write %04x & %04x (ICR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::icr2_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (ICR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_icr2);
	return m_icr2;
}

void sh3_base_device::icr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_icr2);
	logerror("'%s' (%08x): INTC unmapped internal write %04x & %04x (ICR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pinter_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (PINTER) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pinter);
	return m_pinter;
}

void sh3_base_device::pinter_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pinter);
	logerror("'%s' (%08x): INTC unmapped internal write %04x & %04x (PINTER)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::iprc_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (IPRC) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_iprc);
	return m_iprc;
}

void sh3_base_device::iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprc);
	logerror("'%s' (%08x): INTC internal write %04x & %04x (IPRC)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_exception_priority[SH4_INTC_IRL0] = INTPRI((m_iprc & 0x000f) >> 0,  SH4_INTC_IRL0);
	m_exception_priority[SH4_INTC_IRL1] = INTPRI((m_iprc & 0x00f0) >> 4,  SH4_INTC_IRL1);
	m_exception_priority[SH4_INTC_IRL2] = INTPRI((m_iprc & 0x0f00) >> 8,  SH4_INTC_IRL2);
	m_exception_priority[SH4_INTC_IRL3] = INTPRI((m_iprc & 0xf000) >> 12, SH4_INTC_IRL3);
	sh4_exception_recompute();
}

uint16_t sh3_base_device::iprd_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (IPRD) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_iprd);
	return m_iprd;
}

void sh3_base_device::iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprd);
	logerror("'%s' (%08x): INTC internal write %04x & %04x (IPRD)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::ipre_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): INTC unmapped internal read mask %04x (IPRE) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_ipre);
	return m_ipre;
}

void sh3_base_device::ipre_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ipre);
	logerror("'%s' (%08x): INTC internal write %04x & %04x (IPRE)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// SCI
uint8_t sh3_base_device::scsmr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCSMR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scsmr);
	return m_scsmr;
}

void sh3_base_device::scsmr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scsmr);
	logerror("'%s' (%08x): SCI unmapped internal write %02x & %02x (SCSMR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scbrr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCBRR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scbrr);
	return m_scbrr;
}

void sh3_base_device::scbrr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scbrr);
	logerror("'%s' (%08x): SCSI unmapped internal write %02x & %02x (SCBRR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scscr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCSCR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scscr);
	return m_scscr;
}

void sh3_base_device::scscr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scscr);
	logerror("'%s' (%08x): SCI unmapped internal write %02x & %02x (SCSCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::sctdr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCTDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_sctdr);
	return m_sctdr;
}

void sh3_base_device::sctdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_sctdr);
	logerror("'%s' (%08x): SCI unmapped internal write %02x & %02x (SCTDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scssr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCSSR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scssr);
	return m_scssr;
}

void sh3_base_device::scssr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scssr = (m_scssr | (data & 1)) & (data | 6);
	if (!(m_scssr & 0x80))
	{
		//printf("%c", m_sctdr);
		m_scssr |= 0x80;
	}
	logerror("'%s' (%08x): SCI unmapped internal write %02x & %02x (SCSSR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scrdr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCRDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scrdr);
	return m_scrdr;
}

uint8_t sh3_base_device::scscmr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCSCMR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scscmr);
	return m_scscmr;
}

void sh3_base_device::scscmr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scscmr);
	logerror("'%s' (%08x): SCI unmapped internal write %02x & %02x (SCSCMR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// SCI 7708
uint8_t sh3_base_device::scsptr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCI unmapped internal read mask %02x (SCSPTR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scsptr);
	return m_scsptr;
}

void sh3_base_device::scsptr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scsptr);
	logerror("'%s' (%08x): SCI unmapped internal write %02x & %02x (SCSPTR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// CMT 7709
uint16_t sh3_base_device::cmstr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): CMT unmapped internal read mask %04x (CMSTR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_cmstr);
	return m_cmstr;
}

void sh3_base_device::cmstr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cmstr);
	logerror("'%s' (%08x): CMT unmapped internal write %04x & %04x (CMSTR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::cmscr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): CMT unmapped internal read mask %04x (CMSCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_cmscr);
	return m_cmscr;
}

void sh3_base_device::cmscr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cmscr);
	logerror("'%s' (%08x): CMT unmapped internal write %04x & %04x (CMSCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::cmcnt_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): CMT unmapped internal read mask %04x (CMCNT) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_cmcnt);
	return m_cmcnt;
}

void sh3_base_device::cmcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cmcnt);
	logerror("'%s' (%08x): CMT unmapped internal write %04x & %04x (CMCNT)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::cmcor_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): CMT unmapped internal read mask %04x (CMCOR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_cmcor);
	return m_cmcor;
}

void sh3_base_device::cmcor_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cmcor);
	logerror("'%s' (%08x): CMT unmapped internal write %04x & %04x (CMCOR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// AD 7709
uint8_t sh3_base_device::addrah_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRAH) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrah);
	return m_addrah;
}

void sh3_base_device::addrah_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrah);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRAH)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addral_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRAL) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addral);
	return m_addral;
}

void sh3_base_device::addral_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addral);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRAL)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addrbh_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRBH) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrbh);
	return m_addrbh;
}

void sh3_base_device::addrbh_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrbh);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRBH)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addrbl_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRBL) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrbl);
	return m_addrbl;
}

void sh3_base_device::addrbl_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrbl);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRBL)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addrch_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRCH) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrch);
	return m_addrch;
}

void sh3_base_device::addrch_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrch);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRCH)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addrcl_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRCL) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrcl);
	return m_addrcl;
}

void sh3_base_device::addrcl_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrcl);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRCL)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addrdh_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRDH) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrdh);
	return m_addrdh;
}

void sh3_base_device::addrdh_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrdh);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRDH)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::addrdl_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADDRDL) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_addrdl);
	return m_addrdl;
}

void sh3_base_device::addrdl_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_addrdl);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADDRDL)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::adcsr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADCSR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_adcsr);
	return m_adcsr;
}

void sh3_base_device::adcsr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_adcsr);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADCSR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::adcr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): AD unmapped internal read mask %02x (ADCR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_adcr);
	return m_adcr;
}

void sh3_base_device::adcr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_adcr);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (ADCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// DA 7709
uint8_t sh3_base_device::dadr0_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): DA unmapped internal read mask %02x (DADR0) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_dadr0);
	return m_dadr0;
}

void sh3_base_device::dadr0_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_dadr0);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (DADR0)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::dadr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): DA unmapped internal read mask %02x (DADR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_dadr1);
	return m_dadr1;
}

void sh3_base_device::dadr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_dadr1);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (DADR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::dadcr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): DA unmapped internal read mask %02x (DADCR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_dadcr);
	return m_dadcr;
}

void sh3_base_device::dadcr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_dadcr);
	logerror("'%s' (%08x): AD unmapped internal write %02x & %02x (DADCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// PORT 7709
uint16_t sh3_base_device::pacr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PACR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pacr);
	return m_pacr;
}

void sh3_base_device::pacr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pacr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PACR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pbcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PBCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pbcr);
	return m_pbcr;
}

void sh3_base_device::pbcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pbcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PBCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pccr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PCCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pccr);
	return m_pccr;
}

void sh3_base_device::pccr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pccr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PCCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pdcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PDCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pdcr);
	return m_pdcr;
}

void sh3_base_device::pdcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pdcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PDCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pecr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PECR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pecr);
	return m_pecr;
}

void sh3_base_device::pecr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pecr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PECR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pfcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PFCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pfcr);
	return m_pfcr;
}

void sh3_base_device::pfcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pfcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PFCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pgcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PGCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pgcr);
	return m_pgcr;
}

void sh3_base_device::pgcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pgcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PGCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::phcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PHCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_phcr);
	return m_phcr;
}

void sh3_base_device::phcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_phcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PHCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pjcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PJCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pjcr);
	return m_pjcr;
}

void sh3_base_device::pjcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pjcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PJCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::pkcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PKCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_pkcr);
	return m_pkcr;
}

void sh3_base_device::pkcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pkcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PKCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::plcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (PLCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_plcr);
	return m_plcr;
}

void sh3_base_device::plcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_plcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (PLCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::scpcr_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %04x (SCPCR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_scpcr);
	return m_scpcr;
}

void sh3_base_device::scpcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scpcr);
	logerror("'%s' (%08x): PORT unmapped internal write %04x & %04x (SCPCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::padr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PADR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_padr);
	return m_io->read_qword(SH3_PORT_A);
}

void sh3_base_device::padr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_padr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PADR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_A, data);
}

uint8_t sh3_base_device::pbdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PBDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_padr);
	return m_io->read_qword(SH3_PORT_B);
}

void sh3_base_device::pbdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pbdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PBDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_B, data);
}

uint8_t sh3_base_device::pcdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PCDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pcdr);
	return m_io->read_qword(SH3_PORT_C);
}

void sh3_base_device::pcdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pcdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PCDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_C, data);
}

uint8_t sh3_base_device::pddr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PDDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pddr);
	return m_io->read_qword(SH3_PORT_D);
}

void sh3_base_device::pddr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pddr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PDDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_D, data);
}

uint8_t sh3_base_device::pedr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PEDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pedr);
	return m_io->read_qword(SH3_PORT_E);
}

void sh3_base_device::pedr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pedr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PEDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_E, data);
}

uint8_t sh3_base_device::pfdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PFDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pfdr);
	return m_io->read_qword(SH3_PORT_F);
}

void sh3_base_device::pfdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pfdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PFDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_F, data);
}

uint8_t sh3_base_device::pgdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PGDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pgdr);
	return m_io->read_qword(SH3_PORT_G);
}

void sh3_base_device::pgdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pgdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PGDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_G, data);
}

uint8_t sh3_base_device::phdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PHDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_phdr);
	return m_io->read_qword(SH3_PORT_H);
}

void sh3_base_device::phdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_phdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PHDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_H, data);
}

uint8_t sh3_base_device::pjdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PJDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pjdr);
	return m_io->read_qword(SH3_PORT_J);
}

void sh3_base_device::pjdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pjdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PJDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_J, data);
}

uint8_t sh3_base_device::pkdr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PKDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pkdr);
	return m_io->read_qword(SH3_PORT_K);
}

void sh3_base_device::pkdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pkdr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PKDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_K, data);
}

uint8_t sh3_base_device::pldr_r(offs_t offset, uint8_t mem_mask)
{
	//logerror("'%s' (%08x): PORT unmapped internal read mask %02x (PLDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_pldr);
	return m_io->read_qword(SH3_PORT_L);
}

void sh3_base_device::pldr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_pldr);
	//logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (PLDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
	m_io->write_qword(SH3_PORT_L, data);
}

uint8_t sh3_base_device::scpdr_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): PORT unmapped internal read mask %02x (SCPDR) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scpdr);
	return m_scpdr;
}

void sh3_base_device::scpdr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scpdr);
	logerror("'%s' (%08x): PORT unmapped internal write %02x & %02x (SCPDR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// IRDA 7709
uint8_t sh3_base_device::scsmr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %02x (SCSMR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scsmr1);
	return m_scsmr1;
}

void sh3_base_device::scsmr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scsmr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %02x & %02x (SCSMR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scbrr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %02x (SCBRR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scbrr1);
	return m_scbrr1;
}

void sh3_base_device::scbrr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scbrr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %02x & %02x (SCBRR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scscr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %02x (SCSCR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scscr1);
	return m_scscr1;
}

void sh3_base_device::scscr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scscr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %02x & %02x (SCSCR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scftdr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %02x (SCFTDR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scftdr1);
	return m_scftdr1;
}

void sh3_base_device::scftdr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scftdr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %02x & %02x (SCFTDR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::scssr1_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %04x (SCSSR1) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_scssr1);
	return m_scssr1;
}

void sh3_base_device::scssr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scssr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %04x & %04x (SCSSR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scfrdr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %02x (SCFRDR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scfrdr1);
	return m_scfrdr1;
}

void sh3_base_device::scfrdr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scfrdr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %02x & %02x (SCFRDR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scfcr1_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %02x (SCFCR1) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scfcr1);
	return m_scfcr1;
}

void sh3_base_device::scfcr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scfcr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %02x & %02x (SCFCR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::scfdr1_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): IRDA unmapped internal read mask %04x (SCFDR1) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_scfdr1);
	return m_scfdr1;
}

void sh3_base_device::scfdr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scfdr1);
	logerror("'%s' (%08x): IRDA unmapped internal write %04x & %04x (SCFDR1)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// SCIF 7709
uint8_t sh3_base_device::scsmr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %02x (SCSMR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scsmr2);
	return m_scsmr2;
}

void sh3_base_device::scsmr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scsmr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %02x & %02x (SCSMR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scbrr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %02x (SCBRR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scbrr2);
	return m_scbrr2;
}

void sh3_base_device::scbrr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scbrr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %02x & %02x (SCBRR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scscr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %02x (SCSCR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scscr2);
	return m_scscr2;
}

void sh3_base_device::scscr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scscr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %02x & %02x (SCSCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scftdr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %02x (SCFTDR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scftdr2);
	return m_scftdr2;
}

void sh3_base_device::scftdr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scftdr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %02x & %02x (SCFTDR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::scssr2_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %04x (SCSSR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_scssr2);
	return m_scssr2;
}

void sh3_base_device::scssr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scssr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %04x & %04x (SCSSR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scfrdr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %02x (SCFRDR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scfrdr2);
	return m_scfrdr2;
}

void sh3_base_device::scfrdr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scfrdr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %02x & %02x (SCFRDR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint8_t sh3_base_device::scfcr2_r(offs_t offset, uint8_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %02x (SCFCR2) %02x\n", tag(), m_sh2_state->pc, mem_mask, m_scfcr2);
	return m_scfcr2;
}

void sh3_base_device::scfcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scfcr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %02x & %02x (SCFCR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

uint16_t sh3_base_device::scfdr2_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): SCIF unmapped internal read mask %04x (SCFDR2) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_scfdr2);
	return m_scfdr2;
}

void sh3_base_device::scfdr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scfdr2);
	logerror("'%s' (%08x): SCIF unmapped internal write %04x & %04x (SCFDR2)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

// UDI 7709S
uint16_t sh3_base_device::sdir_r(offs_t offset, uint16_t mem_mask)
{
	logerror("'%s' (%08x): UDI unmapped internal read mask %04x (SDIR) %04x\n", tag(), m_sh2_state->pc, mem_mask, m_sdir);
	return m_sdir;
}

void sh3_base_device::sdir_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sdir);
	logerror("'%s' (%08x): UDI unmapped internal write %04x & %04x (SDIR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}
