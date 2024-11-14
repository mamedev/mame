// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************
 *
 *   sh7021.h
 *   Portable Hitachi SH-1 (model SH7021) emulator
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH_SH7021_H
#define MAME_CPU_SH_SH7021_H

#pragma once

#include "sh2.h"

class sh7021_device : public sh2_device
{
public:
	// construction/destruction
	sh7021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_padr(uint16_t data);
	void write_pbdr(uint16_t data);
	template <int Line> void write_padr_bit(int state);
	template <int Line> void write_pbdr_bit(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_run() override;

	void execute_dma(int ch);
	void execute_peripherals(int peripheral_cycles);

	// Interrupt Controller (INTC)
	void intc_ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t intc_ipra_r();
	void intc_iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t intc_iprb_r();
	void intc_iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t intc_iprc_r();
	void intc_iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t intc_iprd_r();
	void intc_ipre_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t intc_ipre_r();
	void intc_icr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t intc_icr_r();

	// User Break Controller (UBC)
	uint16_t ubc_barh_r();
	void ubc_barh_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ubc_barl_r();
	void ubc_barl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ubc_bamrh_r();
	void ubc_bamrh_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ubc_bamrl_r();
	void ubc_bamrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ubc_bbr_r();
	void ubc_bbr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// Bus State Controller (BSC)
	uint16_t bsc_bcr_r();
	void bsc_bcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bsc_wcr1_r();
	void bsc_wcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bsc_wcr2_r();
	void bsc_wcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bsc_wcr3_r();
	void bsc_wcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bsc_dcr_r();
	void bsc_dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bsc_pcr_r();
	void bsc_pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bsc_rcr_r();
	void bsc_rcr_w(uint16_t data);
	uint16_t bsc_rtcsr_r();
	void bsc_rtcsr_w(uint16_t data);
	uint16_t bsc_rtcnt_r();
	void bsc_rtcnt_w(uint16_t data);
	uint16_t bsc_rtcor_r();
	void bsc_rtcor_w(uint16_t data);

	// DMA Controller (DMAC)
	template <int Channel> uint32_t dma_sar_r();
	template <int Channel> void dma_sar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0U);
	template <int Channel> uint32_t dma_dar_r();
	template <int Channel> void dma_dar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0U);
	template <int Channel> uint16_t dma_tcr_r();
	template <int Channel> void dma_tcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Channel> uint16_t dma_chcr_r();
	template <int Channel> void dma_chcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dmaor_r();
	void dmaor_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// 16-Bit Integrated-Timer Pulse Unit (ITU)
	uint8_t itu_tstr_r();
	void itu_tstr_w(uint8_t data);
	uint8_t itu_tsnc_r();
	void itu_tsnc_w(uint8_t data);
	uint8_t itu_tmdr_r();
	void itu_tmdr_w(uint8_t data);
	uint8_t itu_tfcr_r();
	void itu_tfcr_w(uint8_t data);
	uint8_t itu_tocr_r();
	void itu_tocr_w(uint8_t data);

	template <int Channel> uint8_t itu_tcr_r();
	template <int Channel> void itu_tcr_w(uint8_t data);
	template <int Channel> uint8_t itu_tior_r();
	template <int Channel> void itu_tior_w(uint8_t data);
	template <int Channel> uint8_t itu_tier_r();
	template <int Channel> void itu_tier_w(uint8_t data);
	template <int Channel> uint8_t itu_tsr_r();
	template <int Channel> void itu_tsr_w(uint8_t data);
	template <int Channel> uint16_t itu_tcnt_r();
	template <int Channel> void itu_tcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Channel> uint16_t itu_gra_r();
	template <int Channel> void itu_gra_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Channel> uint16_t itu_grb_r();
	template <int Channel> void itu_grb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Channel> uint16_t itu_bra_r();
	template <int Channel> void itu_bra_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Channel> uint16_t itu_brb_r();
	template <int Channel> void itu_brb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// Programmable Timing Pattern Controller (TPC)
	uint8_t tpc_tpmr_r();
	void tpc_tpmr_w(uint8_t data);
	uint8_t tpc_tpcr_r();
	void tpc_tpcr_w(uint8_t data);
	uint8_t tpc_ndera_r();
	void tpc_ndera_w(uint8_t data);
	uint8_t tpc_nderb_r();
	void tpc_nderb_w(uint8_t data);
	uint8_t tpc_ndra_r();
	void tpc_ndra_w(uint8_t data);
	uint8_t tpc_ndra_alt_r();
	void tpc_ndra_alt_w(uint8_t data);
	uint8_t tpc_ndrb_r();
	void tpc_ndrb_w(uint8_t data);
	uint8_t tpc_ndrb_alt_r();
	void tpc_ndrb_alt_w(uint8_t data);

	// Watchdog Timer (WDT)
	uint8_t wdt_tcsr_r(); // TODO: Readable only in 8-bit mode
	void wdt_tcsr_w(uint8_t data); // TODO: Writable only in 16-bit mode
	uint8_t wdt_tcnt_r(); // TODO: Readable only in 8-bit mode
	void wdt_tcnt_w(uint8_t data); // TODO: Writable only in 16-bit mode
	uint8_t wdt_rstcsr_r();
	void wdt_rstcsr_w(uint8_t data); // TODO: Writable only in 16-bit mode

	// Serial Communication Interface (SCI)
	template <int Channel> uint8_t sci_smr_r();
	template <int Channel> void sci_smr_w(uint8_t data);
	template <int Channel> uint8_t sci_brr_r();
	template <int Channel> void sci_brr_w(uint8_t data);
	template <int Channel> uint8_t sci_scr_r();
	template <int Channel> void sci_scr_w(uint8_t data);
	template <int Channel> uint8_t sci_tdr_r();
	template <int Channel> void sci_tdr_w(uint8_t data);
	template <int Channel> uint8_t sci_ssr_r();
	template <int Channel> void sci_ssr_w(uint8_t data);
	template <int Channel> uint8_t sci_rdr_r();

	// Pin Function Controller (PFC)
	uint16_t pfc_paior_r();
	void pfc_paior_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_pacr1_r();
	void pfc_pacr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_pacr2_r();
	void pfc_pacr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_pbior_r();
	void pfc_pbior_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_pbcr1_r();
	void pfc_pbcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_pbcr2_r();
	void pfc_pbcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_padr_r();
	void pfc_padr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_pbdr_r();
	void pfc_pbdr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pfc_cascr_r();
	void pfc_cascr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void internal_map(address_map &map) ATTR_COLD;

	void recalc_irq();

	virtual uint8_t read_byte(offs_t offset) override;
	virtual uint16_t read_word(offs_t offset) override;
	virtual uint32_t read_long(offs_t offset) override;
	virtual uint16_t decrypted_read_word(offs_t offset) override;
	virtual void write_byte(offs_t offset, uint8_t data) override;
	virtual void write_word(offs_t offset, uint16_t data) override;
	virtual void write_long(offs_t offset, uint32_t data) override;

	// Interrupt Controller (INTC)
	uint16_t m_ipra = 0;
	uint16_t m_iprb = 0;
	uint16_t m_iprc = 0;
	uint16_t m_iprd = 0;
	uint16_t m_ipre = 0;
	uint16_t m_icr = 0;

	// User Break Controller (UBC)
	struct
	{
		uint16_t barh = 0;
		uint16_t barl = 0;
		uint16_t bamrh = 0;
		uint16_t bamrl = 0;
		uint16_t bbr = 0;
	} m_ubc;

	// Bus State Controller (BSC)
	struct
	{
		uint16_t bcr = 0;
		uint16_t wcr1 = 0;
		uint16_t wcr2 = 0;
		uint16_t wcr3 = 0;
		uint16_t dcr = 0;
		uint16_t pcr = 0;
		uint8_t rcr = 0;
		uint8_t rtcsr = 0;
		bool rtcsr_read = false;
		uint8_t rtcnt = 0;
		uint8_t rtcor = 0;
	} m_bsc;

	// DMA Controller (DMAC)
	struct
	{
		uint32_t sar = 0;   // Source Address Register
		uint32_t dar = 0;   // Destination Address Register
		uint16_t tcr = 0;   // Transfer Count Register
		uint16_t chcr = 0;  // Channel Control Register
	} m_dma[4];
	uint16_t m_dmaor = 0;   // DMA Operation Register (status flags)
	int m_dma_cycles;

	// 16-Bit Integrated-Timer Pulse Unit (ITU)
	struct
	{
		uint8_t tstr = 0;
		uint8_t tsnc = 0;
		uint8_t tmdr = 0;
		uint8_t tfcr = 0;
		uint8_t tocr = 0;

		struct
		{
			uint8_t tcr = 0;
			uint8_t tior = 0;
			uint8_t tier = 0;
			uint8_t tsr = 0;
			uint16_t tcnt = 0;
			uint16_t gra = 0;
			uint16_t grb = 0;
			uint16_t bra = 0;
			uint16_t brb = 0;
			emu_timer *et = nullptr;
		} timer[5];
	} m_itu;

	void start_timer(int i);

	template<int which> TIMER_CALLBACK_MEMBER(sh7021_timer_callback);
	template<int which> TIMER_CALLBACK_MEMBER(sh7021_sci_callback);

	// Programmable Timing Pattern Controller (TPC)
	struct
	{
		uint8_t tpmr = 0;
		uint8_t tpcr = 0;
		uint8_t ndera = 0;
		uint8_t nderb = 0;
		uint8_t ndra = 0;
		uint8_t ndrb = 0;
	} m_tpc;

	// Watchdog Timer (WDT)
	struct
	{
		uint8_t tcsr = 0;
		uint8_t tcnt = 0;
		uint8_t rstcsr = 0;
	} m_wdt;

	// Serial Communication Interface (SCI)
	struct
	{
		uint8_t smr = 0;
		uint8_t brr = 0;
		uint8_t scr = 0;
		uint8_t tsr = 0;
		uint8_t tdr = 0;
		uint8_t ssr = 0;
		uint8_t ssr_read = 0;
		uint8_t rsr = 0;
		uint8_t rdr = 0;
		emu_timer *et = nullptr;
	} m_sci[2];

	// Pin Function Controller (PFC)
	struct
	{
		uint16_t paior = 0;
		uint16_t pacr1 = 0;
		uint16_t pacr2 = 0;
		uint16_t pbior = 0;
		uint16_t pbcr1 = 0;
		uint16_t pbcr2 = 0;
		uint16_t padr = 0;
		uint16_t pbdr = 0;
		uint16_t padr_in = 0;
		uint16_t pbdr_in = 0;
		uint16_t cascr = 0;
		uint8_t pafunc[16];
		uint8_t pbfunc[16];
		uint16_t pa_gpio_mask = 0;
		uint16_t pb_gpio_mask = 0;
	} m_pfc;
	devcb_write16 m_pa_out;
	devcb_write16 m_pb_out;
	devcb_write_line::array<16> m_pa_bit_out;
	devcb_write_line::array<16> m_pb_bit_out;
};

DECLARE_DEVICE_TYPE(SH7021, sh7021_device)

#endif // MAME_CPU_SH_SH7021_H
