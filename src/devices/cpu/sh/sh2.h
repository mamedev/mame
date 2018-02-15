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

#define MCFG_SH2_IS_SLAVE(_slave) \
	sh2_device::set_is_slave(*device, _slave);

#define MCFG_SH2_DMA_KLUDGE_CB(_class, _method) \
	sh2_device::set_dma_kludge_callback(*device, sh2_device::dma_kludge_delegate(&_class::_method, #_class "::" #_method, this));

#define MCFG_SH2_FIFO_DATA_AVAIL_CB(_class, _method) \
	sh2_device::set_dma_fifo_data_available_callback(*device, sh2_device::dma_fifo_data_available_delegate(&_class::_method, #_class "::" #_method, this));

#define MCFG_SH2_FTCSR_READ_CB(_class, _method) \
	sh2_device::set_ftcsr_read_callback(*device, sh2_device::ftcsr_read_delegate(&_class::_method, #_class "::" #_method, this));


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

	static void set_is_slave(device_t &device, int slave) { downcast<sh2_device &>(device).m_is_slave = slave; }
	static void set_dma_kludge_callback(device_t &device, dma_kludge_delegate callback) { downcast<sh2_device &>(device).m_dma_kludge_cb = callback; }
	static void set_dma_fifo_data_available_callback(device_t &device, dma_fifo_data_available_delegate callback) { downcast<sh2_device &>(device).m_dma_fifo_data_available_cb = callback; }
	static void set_ftcsr_read_callback(device_t &device, ftcsr_read_delegate callback) { downcast<sh2_device &>(device).m_ftcsr_read_cb = callback; }

	DECLARE_WRITE32_MEMBER( sh7604_w );
	DECLARE_READ32_MEMBER( sh7604_r );
	DECLARE_READ32_MEMBER(sh2_internal_a5);

	void sh2_set_frt_input(int state);
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
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 4; }
	virtual uint32_t execute_input_lines() const override { return 16; }
	virtual uint32_t execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual util::disasm_interface *create_disassembler() override;

	address_space *m_decrypted_program;

private:
	address_space_config m_program_config, m_decrypted_program_config;

	uint32_t  m_cpu_off;
	uint32_t  m_test_irq;

	int8_t    m_irq_line_state[17];

	address_space *m_internal;
	uint32_t m_m[0x200/4];
	int8_t  m_nmi_line_state;

	uint16_t  m_frc;
	uint16_t  m_ocra, m_ocrb, m_icr;
	uint64_t  m_frc_base;

	int     m_frt_input;
	int     m_internal_irq_vector;

	emu_timer *m_timer;
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
	uint16_t m_wtcnt;
	uint8_t m_wtcsr;

	int     m_is_slave;
	dma_kludge_delegate              m_dma_kludge_cb;
	dma_fifo_data_available_delegate m_dma_fifo_data_available_cb;
	ftcsr_read_delegate              m_ftcsr_read_cb;

	std::unique_ptr<sh2_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */

	uint32_t m_debugger_temp;

	inline uint8_t RB(offs_t A) override;
	inline uint16_t RW(offs_t A) override;
	inline uint32_t RL(offs_t A) override;
	inline void WB(offs_t A, uint8_t V) override;
	inline void WW(offs_t A, uint16_t V) override;
	inline void WL(offs_t A, uint32_t V) override;

	virtual void LDCMSR(const uint16_t opcode) override;
	virtual void LDCSR(const uint16_t opcode) override;
	virtual void TRAPA(uint32_t i) override;
	virtual void RTE() override;
	virtual void ILLEGAL() override;

	virtual void execute_one_f000(uint16_t opcode) override;

	TIMER_CALLBACK_MEMBER( sh2_timer_callback );
	TIMER_CALLBACK_MEMBER( sh2_dma_current_active_callback );
	void sh2_timer_resync();
	void sh2_timer_activate();
	void sh2_do_dma(int dma);
	virtual void sh2_exception(const char *message, int irqline) override;
	void sh2_dmac_check(int dma);
	void sh2_recalc_irq();

	virtual void init_drc_frontend() override;
	virtual const opcode_desc* get_desclist(offs_t pc) override;

	virtual void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, bool allow_exception) override;
	virtual void static_generate_entry_point() override;
	virtual void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle **handleptr) override;

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
