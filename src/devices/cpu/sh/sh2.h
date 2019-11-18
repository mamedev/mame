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

	DECLARE_READ32_MEMBER(sh2_internal_a5);

	// SCI
	DECLARE_READ8_MEMBER( smr_r );
	DECLARE_WRITE8_MEMBER( smr_w );
	DECLARE_READ8_MEMBER( brr_r );
	DECLARE_WRITE8_MEMBER( brr_w );
	DECLARE_READ8_MEMBER( scr_r );
	DECLARE_WRITE8_MEMBER( scr_w );
	DECLARE_READ8_MEMBER( tdr_r );
	DECLARE_WRITE8_MEMBER( tdr_w );
	DECLARE_READ8_MEMBER( ssr_r );
	DECLARE_WRITE8_MEMBER( ssr_w );
	DECLARE_READ8_MEMBER( rdr_r );

	// FRT / FRC
	DECLARE_READ8_MEMBER( tier_r );
	DECLARE_WRITE8_MEMBER( tier_w );
	DECLARE_READ16_MEMBER( frc_r );
	DECLARE_WRITE16_MEMBER( frc_w );
	DECLARE_READ8_MEMBER( ftcsr_r );
	DECLARE_WRITE8_MEMBER( ftcsr_w );
	DECLARE_READ16_MEMBER( ocra_b_r );
	DECLARE_WRITE16_MEMBER( ocra_b_w );
	DECLARE_READ8_MEMBER( frc_tcr_r );
	DECLARE_WRITE8_MEMBER( frc_tcr_w );
	DECLARE_READ8_MEMBER( tocr_r );
	DECLARE_WRITE8_MEMBER( tocr_w );
	DECLARE_READ16_MEMBER( frc_icr_r );

	// INTC
	DECLARE_READ16_MEMBER( ipra_r );
	DECLARE_WRITE16_MEMBER( ipra_w );
	DECLARE_READ16_MEMBER( iprb_r );
	DECLARE_WRITE16_MEMBER( iprb_w );
	DECLARE_READ16_MEMBER( vcra_r );
	DECLARE_WRITE16_MEMBER( vcra_w );
	DECLARE_READ16_MEMBER( vcrb_r );
	DECLARE_WRITE16_MEMBER( vcrb_w );
	DECLARE_READ16_MEMBER( vcrc_r );
	DECLARE_WRITE16_MEMBER( vcrc_w );
	DECLARE_READ16_MEMBER( vcrd_r );
	DECLARE_WRITE16_MEMBER( vcrd_w );
	DECLARE_READ16_MEMBER( vcrwdt_r );
	DECLARE_WRITE16_MEMBER( vcrwdt_w );
	DECLARE_READ32_MEMBER( vcrdiv_r );
	DECLARE_WRITE32_MEMBER( vcrdiv_w );
	DECLARE_READ16_MEMBER( intc_icr_r );
	DECLARE_WRITE16_MEMBER( intc_icr_w );

	// DIVU
	DECLARE_READ32_MEMBER( dvsr_r );
	DECLARE_WRITE32_MEMBER( dvsr_w );
	DECLARE_READ32_MEMBER( dvdnt_r );
	DECLARE_WRITE32_MEMBER( dvdnt_w );
	DECLARE_READ32_MEMBER( dvdnth_r );
	DECLARE_WRITE32_MEMBER( dvdnth_w );
	DECLARE_READ32_MEMBER( dvdntl_r );
	DECLARE_WRITE32_MEMBER( dvdntl_w );

	DECLARE_READ32_MEMBER( dvcr_r );
	DECLARE_WRITE32_MEMBER( dvcr_w );

	// DMAC
	template <int Channel> READ32_MEMBER(vcrdma_r)
	{
		return m_vcrdma[Channel] & 0x7f;
	}

	template <int Channel> WRITE32_MEMBER(vcrdma_w)
	{
		COMBINE_DATA(&m_vcrdma[Channel]);
		m_irq_vector.dmac[Channel] = m_vcrdma[Channel] & 0x7f;
		sh2_recalc_irq();
	}

	template <int Channel> READ8_MEMBER(drcr_r) { return m_dmac[Channel].drcr & 3; }
	template <int Channel> WRITE8_MEMBER(drcr_w) { m_dmac[Channel].drcr = data & 3; sh2_recalc_irq(); }
	template <int Channel> READ32_MEMBER(sar_r) { return m_dmac[Channel].sar; }
	template <int Channel> WRITE32_MEMBER(sar_w) { COMBINE_DATA(&m_dmac[Channel].sar); }
	template <int Channel> READ32_MEMBER(dar_r) { return m_dmac[Channel].dar; }
	template <int Channel> WRITE32_MEMBER(dar_w) { COMBINE_DATA(&m_dmac[Channel].dar); }
	template <int Channel> READ32_MEMBER(dmac_tcr_r) { return m_dmac[Channel].tcr; }
	template <int Channel> WRITE32_MEMBER(dmac_tcr_w) { COMBINE_DATA(&m_dmac[Channel].tcr); m_dmac[Channel].tcr &= 0xffffff; }
	template <int Channel> READ32_MEMBER(chcr_r) { return m_dmac[Channel].chcr; }
	template <int Channel> WRITE32_MEMBER(chcr_w)
	{
		uint32_t old;
		old = m_dmac[Channel].chcr;
		COMBINE_DATA(&m_dmac[Channel].chcr);
		m_dmac[Channel].chcr = (data & ~2) | (old & m_dmac[Channel].chcr & 2);
		sh2_dmac_check(Channel);
	}
	READ32_MEMBER( dmaor_r ) { return m_dmaor & 0xf; }
	WRITE32_MEMBER( dmaor_w )
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
	DECLARE_READ16_MEMBER( wtcnt_r );
	DECLARE_WRITE16_MEMBER( wtcnt_w );
	DECLARE_READ16_MEMBER( rstcsr_r );
	DECLARE_WRITE16_MEMBER( rstcsr_w );

	// misc
	DECLARE_READ16_MEMBER( fmr_sbycr_r );
	DECLARE_WRITE16_MEMBER( fmr_sbycr_w );
	DECLARE_READ8_MEMBER( ccr_r );
	DECLARE_WRITE8_MEMBER( ccr_w );

	// BSC
	DECLARE_READ32_MEMBER( bcr1_r );
	DECLARE_WRITE32_MEMBER( bcr1_w );
	DECLARE_READ32_MEMBER( bcr2_r );
	DECLARE_WRITE32_MEMBER( bcr2_w );
	DECLARE_READ32_MEMBER( wcr_r );
	DECLARE_WRITE32_MEMBER( wcr_w );
	DECLARE_READ32_MEMBER( mcr_r );
	DECLARE_WRITE32_MEMBER( mcr_w );
	DECLARE_READ32_MEMBER( rtcsr_r );
	DECLARE_WRITE32_MEMBER( rtcsr_w );
	DECLARE_READ32_MEMBER( rtcor_r );
	DECLARE_WRITE32_MEMBER( rtcor_w );
	DECLARE_READ32_MEMBER( rtcnt_r );
	DECLARE_WRITE32_MEMBER( rtcnt_w );


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

	uint32_t  m_cpu_off;
	uint32_t  m_test_irq;

	int8_t    m_irq_line_state[17];

	address_space *m_internal;
	// SCI
	uint8_t m_smr, m_brr, m_scr, m_tdr, m_ssr;
	// FRT / FRC
	uint8_t m_tier, m_ftcsr, m_frc_tcr, m_tocr;
	uint16_t m_frc;
	uint16_t m_ocra, m_ocrb, m_frc_icr;
	// INTC
	struct {
		uint8_t frc;
		uint8_t sci;
		uint8_t divu;
		uint8_t dmac;
		uint8_t wdt;
	} m_irq_level;
	struct {
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
	struct {
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

	int8_t  m_nmi_line_state;

	uint64_t  m_frc_base;

	int     m_frt_input;
	int     m_internal_irq_vector;

	emu_timer *m_timer;
	emu_timer *m_wdtimer;
	emu_timer *m_dma_current_active_timer[2];
	int     m_dma_timer_active[2];
	uint8_t  m_dma_irq[2];

	int m_active_dma_incs[2];
	int m_active_dma_incd[2];
	int m_active_dma_size[2];
	int m_active_dma_steal[2];
	uint32_t m_active_dma_src[2];
	uint32_t m_active_dma_dst[2];
	uint32_t m_active_dma_count[2];

	int     m_is_slave;
	dma_kludge_delegate              m_dma_kludge_cb;
	dma_fifo_data_available_delegate m_dma_fifo_data_available_cb;
	ftcsr_read_delegate              m_ftcsr_read_cb;

	std::unique_ptr<sh2_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */

	uint32_t m_debugger_temp;

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

	DECLARE_READ32_MEMBER(dma_sar0_r);
	DECLARE_WRITE32_MEMBER(dma_sar0_w);
	DECLARE_READ32_MEMBER(dma_dar0_r);
	DECLARE_WRITE32_MEMBER(dma_dar0_w);
	DECLARE_READ16_MEMBER(dmaor_r);
	DECLARE_WRITE16_MEMBER(dmaor_w);
	DECLARE_READ16_MEMBER(dma_tcr0_r);
	DECLARE_WRITE16_MEMBER(dma_tcr0_w);
	DECLARE_READ16_MEMBER(dma_chcr0_r);
	DECLARE_WRITE16_MEMBER(dma_chcr0_w);
	DECLARE_READ16_MEMBER(sh7021_r);
	DECLARE_WRITE16_MEMBER(sh7021_w);
	void sh7032_dma_exec(int ch);

	void sh7021_map(address_map &map);
private:
	uint16_t m_sh7021_regs[0x200];
	struct
	{
		uint32_t              sar;    /**< Source Address Register */
		uint32_t              dar;    /**< Destination Address Register */
		uint16_t              tcr;    /**< Transfer Count Register */
		uint16_t              chcr;   /**< Channel Control Register */
	} m_dma[4];
	uint16_t m_dmaor;                 /**< DMA Operation Register (status flags) */

};

class sh1_device : public sh2_device
{
public:
	// construction/destruction
	sh1_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	DECLARE_READ16_MEMBER(sh7032_r);
	DECLARE_WRITE16_MEMBER(sh7032_w);
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
