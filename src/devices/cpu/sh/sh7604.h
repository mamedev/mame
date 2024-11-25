// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, R. Belmont

// sh7604, sh2 variant

#ifndef MAME_CPU_SH_SH7604_H
#define MAME_CPU_SH_SH7604_H

#pragma once

#include "sh2.h"

#define SH2_DMA_KLUDGE_CB(name)  int name(uint32_t src, uint32_t dst, uint32_t data, int size)
#define SH2_DMA_FIFO_DATA_AVAILABLE_CB(name)  int name(uint32_t src, uint32_t dst, uint32_t data, int size)
#define SH2_FTCSR_READ_CB(name)  void name(uint32_t data)


class sh7604_device : public sh2_device
{
public:
	typedef device_delegate<int (uint32_t src, uint32_t dst, uint32_t data, int size)> dma_kludge_delegate;
	typedef device_delegate<int (uint32_t src, uint32_t dst, uint32_t data, int size)> dma_fifo_data_available_delegate;
	typedef device_delegate<void (uint32_t data)> ftcsr_read_delegate;

	sh7604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_is_slave(int slave) { m_is_slave = slave; }

	template <typename... T> void set_dma_kludge_callback(T &&... args) { m_dma_kludge_cb.set(std::forward<T>(args)...); }

	template <typename... T> void set_dma_fifo_data_available_callback(T &&... args) { m_dma_fifo_data_available_cb.set(std::forward<T>(args)...); }

	template <typename... T> void set_ftcsr_read_callback(T &&... args) { m_ftcsr_read_cb.set(std::forward<T>(args)...); }

	void sh2_notify_dma_data_available();

protected:
	sh7604_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type, address_map_constructor internal_map, int addrlines);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sh2_exception(const char *message, int irqline) override;

	uint32_t m_test_irq;
	int m_internal_irq_vector;

private:
	enum
	{
		ICF  = 0x80,
		OCFA = 0x08,
		OCFB = 0x04,
		OVF  = 0x02,
		CCLRA = 0x01
	};

	void sh7604_map(address_map &map) ATTR_COLD;

	uint32_t sh2_internal_a5();

	// SCI
	uint8_t smr_r();
	void smr_w(uint8_t data);
	uint8_t brr_r();
	void brr_w(uint8_t data);
	uint8_t scr_r();
	void scr_w(uint8_t data);
	uint8_t tdr_r();
	void tdr_w(uint8_t data);
	uint8_t ssr_r();
	void ssr_w(uint8_t data);
	uint8_t rdr_r();

	// FRT / FRC
	uint8_t tier_r();
	void tier_w(uint8_t data);
	uint16_t frc_r();
	void frc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t ftcsr_r();
	void ftcsr_w(uint8_t data);
	uint16_t ocra_b_r();
	void ocra_b_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t frc_tcr_r();
	void frc_tcr_w(uint8_t data);
	uint8_t tocr_r();
	void tocr_w(uint8_t data);
	uint16_t frc_icr_r();

	// INTC
	uint16_t ipra_r();
	void ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t iprb_r();
	void iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vcra_r();
	void vcra_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vcrb_r();
	void vcrb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vcrc_r();
	void vcrc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vcrd_r();
	void vcrd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vcrwdt_r();
	void vcrwdt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t vcrdiv_r();
	void vcrdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t intc_icr_r();
	void intc_icr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// DIVU
	uint32_t dvsr_r();
	void dvsr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dvdnt_r();
	void dvdnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dvdnth_r();
	void dvdnth_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dvdntl_r();
	void dvdntl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t dvcr_r();
	void dvcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// DMAC
	template <int Channel> uint32_t vcrdma_r();
	template <int Channel> void vcrdma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Channel> uint8_t drcr_r();
	template <int Channel> void drcr_w(uint8_t data);
	template <int Channel> uint32_t sar_r();
	template <int Channel> void sar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Channel> uint32_t dar_r();
	template <int Channel> void dar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Channel> uint32_t dmac_tcr_r();
	template <int Channel> void dmac_tcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Channel> uint32_t chcr_r();
	template <int Channel> void chcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dmaor_r();
	void dmaor_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// WTC
	uint16_t wtcnt_r();
	void wtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rstcsr_r();
	void rstcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// misc
	uint16_t fmr_sbycr_r();
	void fmr_sbycr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t ccr_r();
	void ccr_w(uint8_t data);

	// BSC
	uint32_t bcr1_r();
	void bcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t bcr2_r();
	void bcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t wcr_r();
	void wcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t mcr_r();
	void mcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtcsr_r();
	void rtcsr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtcor_r();
	void rtcor_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtcnt_r();
	void rtcnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void set_frt_input(int state) override;

	// SCI
	uint8_t m_smr, m_brr, m_scr, m_tdr, m_ssr;

	// FRT / FRC
	uint8_t m_tier, m_ftcsr, m_frc_tcr, m_tocr;
	uint16_t m_frc;
	uint16_t m_ocra, m_ocrb, m_frc_icr;

	// INTC
	struct
	{
		uint8_t frc;
		uint8_t sci;
		uint8_t divu;
		uint8_t dmac;
		uint8_t wdt;
	} m_irq_level;

	struct
	{
		uint8_t fic;
		uint8_t foc;
		uint8_t fov;
		uint8_t divu;
		uint8_t dmac[2];
	} m_irq_vector;

	uint16_t m_ipra, m_iprb;
	uint16_t m_vcra, m_vcrb, m_vcrc, m_vcrd, m_vcrwdt, m_vcrdiv, m_intc_icr, m_vcrdma[2];
	bool m_vecmd, m_nmie;

	// DIVU
	bool m_divu_ovf, m_divu_ovfie;
	uint32_t m_dvsr, m_dvdntl, m_dvdnth;

	// WTC
	uint8_t m_wtcnt, m_wtcsr;
	uint8_t m_rstcsr;
	uint16_t m_wtcw[2];

	// DMAC
	struct
	{
		uint8_t drcr;
		uint32_t sar;
		uint32_t dar;
		uint32_t tcr;
		uint32_t chcr;
	} m_dmac[2];
	uint8_t m_dmaor;

	// misc
	uint8_t m_sbycr, m_ccr;

	// BSC
	uint32_t m_bcr1, m_bcr2, m_wcr, m_mcr, m_rtcsr, m_rtcor, m_rtcnt;

	uint64_t m_frc_base;

	int m_frt_input;

	emu_timer *m_timer;
	emu_timer *m_wdtimer;
	emu_timer *m_dma_current_active_timer[2];
	int m_dma_timer_active[2];
	uint8_t m_dma_irq[2];

	int m_active_dma_incs[2];
	int m_active_dma_incd[2];
	int m_active_dma_size[2];
	int m_active_dma_steal[2];
	uint32_t m_active_dma_src[2];
	uint32_t m_active_dma_dst[2];
	uint32_t m_active_dma_count[2];

	int m_is_slave;

	dma_kludge_delegate              m_dma_kludge_cb;
	dma_fifo_data_available_delegate m_dma_fifo_data_available_cb;
	ftcsr_read_delegate              m_ftcsr_read_cb;

	TIMER_CALLBACK_MEMBER(sh2_timer_callback);
	TIMER_CALLBACK_MEMBER(sh2_wdtimer_callback);
	TIMER_CALLBACK_MEMBER(sh2_dma_current_active_callback);
	void sh2_timer_resync();
	void sh2_timer_activate();
	void sh2_wtcnt_recalc();
	void sh2_wdt_activate();
	void sh2_do_dma(int dmach);
	void sh2_dmac_check(int dma);
	void sh2_recalc_irq();
};

DECLARE_DEVICE_TYPE(SH7604, sh7604_device)

#endif // MAME_CPU_SH_SH7604_H
