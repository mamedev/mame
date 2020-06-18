// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   sh2.h
 *   Portable Hitachi SH-2 (SH7600 family) emulator interface
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was heavily changed to the MAME CPU requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH2_SH2_H
#define MAME_CPU_SH2_SH2_H

#pragma once


#include "sh.h"

#define SH2_INT_NONE    -1
#define SH2_INT_VBLIN   0
#define SH2_INT_VBLOUT  1
#define SH2_INT_HBLIN   2
#define SH2_INT_TIMER0  3
#define SH2_INT_TIMER1  4
#define SH2_INT_DSP     5
#define SH2_INT_SOUND   6
#define SH2_INT_SMPC    7
#define SH2_INT_PAD     8
#define SH2_INT_DMA2    9
#define SH2_INT_DMA1    10
#define SH2_INT_DMA0    11
#define SH2_INT_DMAILL  12
#define SH2_INT_SPRITE  13
#define SH2_INT_14      14
#define SH2_INT_15      15
#define SH2_INT_ABUS    16

#define SH2_DMA_KLUDGE_CB(name)  int name(uint32_t src, uint32_t dst, uint32_t data, int size)
#define SH2_DMA_FIFO_DATA_AVAILABLE_CB(name)  int name(uint32_t src, uint32_t dst, uint32_t data, int size)
#define SH2_FTCSR_READ_CB(name)  void name(uint32_t data)

class sh2_frontend;

class sh2_device : public sh_common_execution
{
	friend class sh2_frontend;

public:
	typedef device_delegate<int (uint32_t src, uint32_t dst, uint32_t data, int size)> dma_kludge_delegate;
	typedef device_delegate<int (uint32_t src, uint32_t dst, uint32_t data, int size)> dma_fifo_data_available_delegate;
	typedef device_delegate<void (uint32_t data)> ftcsr_read_delegate;

	// construction/destruction
	sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~sh2_device() override;

	void set_is_slave(int slave) { m_is_slave = slave; }

	template <typename... T> void set_dma_kludge_callback(T &&... args) { m_dma_kludge_cb.set(std::forward<T>(args)...); }

	template <typename... T> void set_dma_fifo_data_available_callback(T &&... args) { m_dma_fifo_data_available_cb.set(std::forward<T>(args)...); }

	template <typename... T> void set_ftcsr_read_callback(T &&... args) { m_ftcsr_read_cb.set(std::forward<T>(args)...); }

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
	template <int Channel> uint32_t vcrdma_r()
	{
		return m_vcrdma[Channel] & 0x7f;
	}

	template <int Channel> void vcrdma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0)
	{
		COMBINE_DATA(&m_vcrdma[Channel]);
		m_irq_vector.dmac[Channel] = m_vcrdma[Channel] & 0x7f;
		sh2_recalc_irq();
	}

	template <int Channel> uint8_t drcr_r() { return m_dmac[Channel].drcr & 3; }
	template <int Channel> void drcr_w(uint8_t data) { m_dmac[Channel].drcr = data & 3; sh2_recalc_irq(); }
	template <int Channel> uint32_t sar_r() { return m_dmac[Channel].sar; }
	template <int Channel> void sar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_dmac[Channel].sar); }
	template <int Channel> uint32_t dar_r() { return m_dmac[Channel].dar; }
	template <int Channel> void dar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_dmac[Channel].dar); }
	template <int Channel> uint32_t dmac_tcr_r() { return m_dmac[Channel].tcr; }
	template <int Channel> void dmac_tcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_dmac[Channel].tcr); m_dmac[Channel].tcr &= 0xffffff; }
	template <int Channel> uint32_t chcr_r() { return m_dmac[Channel].chcr; }
	template <int Channel> void chcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0)
	{
		uint32_t old;
		old = m_dmac[Channel].chcr;
		COMBINE_DATA(&m_dmac[Channel].chcr);
		m_dmac[Channel].chcr = (data & ~2) | (old & m_dmac[Channel].chcr & 2);
		sh2_dmac_check(Channel);
	}
	uint32_t dmaor_r() { return m_dmaor & 0xf; }
	void dmaor_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0)
	{
		if(ACCESSING_BITS_0_7)
		{
			uint8_t old;
			old = m_dmaor & 0xf;
			m_dmaor = (data & ~6) | (old & m_dmaor & 6);
			sh2_dmac_check(0);
			sh2_dmac_check(1);
		}
	}

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
	void sh2_notify_dma_data_available();
	void func_fastirq();

	void sh7604_map(address_map &map);
protected:
	sh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type,address_map_constructor internal_map, int addrlines);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual uint32_t execute_input_lines() const noexcept override { return 16; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space *m_decrypted_program;

private:
	address_space_config m_program_config, m_decrypted_program_config;

	uint32_t  m_cpu_off = 0;
	uint32_t  m_test_irq = 0;

	int8_t    m_irq_line_state[17];

	address_space *m_internal;


	// SCI
	uint8_t m_smr = 0, m_brr = 0, m_scr = 0, m_tdr = 0, m_ssr = 0;
	// FRT / FRC
	uint8_t m_tier = 0, m_ftcsr = 0, m_frc_tcr = 0, m_tocr = 0;
	uint16_t m_frc = 0;
	uint16_t m_ocra = 0, m_ocrb = 0, m_frc_icr = 0;
	// INTC
	struct {
		uint8_t frc = 0;
		uint8_t sci = 0;
		uint8_t divu = 0;
		uint8_t dmac = 0;
		uint8_t wdt = 0;
	} m_irq_level;
	struct {
		uint8_t fic = 0;
		uint8_t foc = 0;
		uint8_t fov = 0;
		uint8_t divu = 0;
		uint8_t dmac[2] = { 0, 0 };
	} m_irq_vector;
	uint16_t m_ipra = 0, m_iprb = 0;
	uint16_t m_vcra = 0, m_vcrb = 0, m_vcrc = 0, m_vcrd = 0, m_vcrwdt = 0, m_vcrdiv = 0, m_intc_icr = 0, m_vcrdma[2] = { 0, 0, };
	bool m_vecmd = false, m_nmie = false;

	// DIVU
	bool m_divu_ovf = false, m_divu_ovfie = false;
	uint32_t m_dvsr = 0, m_dvdntl = 0, m_dvdnth = 0;

	// WTC
	uint8_t m_wtcnt = 0, m_wtcsr = 0;
	uint8_t m_rstcsr = 0;
	uint16_t m_wtcw[2] = { 0, 0 };

	// DMAC
	struct {
		uint8_t drcr = 0;
		uint32_t sar = 0;
		uint32_t dar = 0;
		uint32_t tcr = 0;
		uint32_t chcr = 0;
	} m_dmac[2];
	uint8_t m_dmaor = 0;

	// misc
	uint8_t m_sbycr = 0, m_ccr = 0;

	// BSC
	uint32_t m_bcr1 = 0, m_bcr2 = 0, m_wcr = 0, m_mcr = 0, m_rtcsr = 0, m_rtcor = 0, m_rtcnt = 0;

	int8_t  m_nmi_line_state = 0;

	uint64_t  m_frc_base = 0;

	int     m_frt_input = 0;
	int     m_internal_irq_vector = 0;

	emu_timer *m_timer = nullptr;
	emu_timer *m_wdtimer = nullptr;
	emu_timer *m_dma_current_active_timer[2] { nullptr, nullptr };
	int     m_dma_timer_active[2] = { 0, 0 };
	uint8_t  m_dma_irq[2] = { 0, 0 };

	int m_active_dma_incs[2] = { 0, 0 };
	int m_active_dma_incd[2] = { 0, 0 };
	int m_active_dma_size[2] = { 0, 0 };
	int m_active_dma_steal[2] = { 0, 0 };
	uint32_t m_active_dma_src[2] = { 0, 0 };
	uint32_t m_active_dma_dst[2] = { 0, 0 };
	uint32_t m_active_dma_count[2] = { 0, 0 };

	int     m_is_slave = 0;
	dma_kludge_delegate              m_dma_kludge_cb;
	dma_fifo_data_available_delegate m_dma_fifo_data_available_cb;
	ftcsr_read_delegate              m_ftcsr_read_cb;

	std::unique_ptr<sh2_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */

	uint32_t m_debugger_temp = 0;

	virtual uint8_t RB(offs_t A) override;
	virtual uint16_t RW(offs_t A) override;
	virtual uint32_t RL(offs_t A) override;
	virtual void WB(offs_t A, uint8_t V) override;
	virtual void WW(offs_t A, uint16_t V) override;
	virtual void WL(offs_t A, uint32_t V) override;

	virtual void LDCMSR(const uint16_t opcode) override;
	virtual void LDCSR(const uint16_t opcode) override;
	virtual void TRAPA(uint32_t i) override;
	virtual void RTE() override;
	virtual void ILLEGAL() override;

	virtual void execute_one_f000(uint16_t opcode) override;

	TIMER_CALLBACK_MEMBER( sh2_timer_callback );
	TIMER_CALLBACK_MEMBER( sh2_wdtimer_callback );
	TIMER_CALLBACK_MEMBER( sh2_dma_current_active_callback );
	void sh2_timer_resync();
	void sh2_timer_activate();
	void sh2_wtcnt_recalc();
	void sh2_wdt_activate();
	void sh2_do_dma(int dmach);
	virtual void sh2_exception(const char *message, int irqline) override;
	void sh2_dmac_check(int dma);
	void sh2_recalc_irq();

	virtual void init_drc_frontend() override;
	virtual const opcode_desc* get_desclist(offs_t pc) override;

	virtual void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception) override;
	virtual void static_generate_entry_point() override;
	virtual void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr) override;

};

class sh2a_device : public sh2_device
{
public:
	// construction/destruction
	sh2a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t dma_sar0_r();
	void dma_sar0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dma_dar0_r();
	void dma_dar0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t dmaor_r();
	void dmaor_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dma_tcr0_r();
	void dma_tcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dma_chcr0_r();
	void dma_chcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sh7021_r(offs_t offset);
	void sh7021_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sh7032_dma_exec(int ch);

	void sh7021_map(address_map &map);
private:
	uint16_t m_sh7021_regs[0x200];
	struct
	{
		uint32_t              sar = 0;    /**< Source Address Register */
		uint32_t              dar = 0;    /**< Destination Address Register */
		uint16_t              tcr = 0;    /**< Transfer Count Register */
		uint16_t              chcr = 0;   /**< Channel Control Register */
	} m_dma[4];
	uint16_t m_dmaor = 0;                 /**< DMA Operation Register (status flags) */

};

class sh1_device : public sh2_device
{
public:
	// construction/destruction
	sh1_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	uint16_t sh7032_r(offs_t offset);
	void sh7032_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sh7032_map(address_map &map);
private:
	uint16_t m_sh7032_regs[0x200];
};


class sh2_frontend : public sh_frontend
{
public:
	sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:

private:
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
};

DECLARE_DEVICE_TYPE(SH1,  sh1_device)
DECLARE_DEVICE_TYPE(SH2,  sh2_device)
DECLARE_DEVICE_TYPE(SH2A, sh2a_device)

#endif // MAME_CPU_SH2_SH2_H
