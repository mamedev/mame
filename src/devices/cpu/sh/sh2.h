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
	sh2_device::set_dma_kludge_callback(*device, sh2_device::dma_kludge_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SH2_FIFO_DATA_AVAIL_CB(_class, _method) \
	sh2_device::set_dma_fifo_data_available_callback(*device, sh2_device::dma_fifo_data_available_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SH2_FTCSR_READ_CB(_class, _method) \
	sh2_device::set_ftcsr_read_callback(*device, sh2_device::ftcsr_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

#define SH2DRC_STRICT_VERIFY        0x0001          /* verify all instructions */
#define SH2DRC_FLUSH_PC         0x0002          /* flush the PC value before each memory access */
#define SH2DRC_STRICT_PCREL     0x0004          /* do actual loads on MOVLI/MOVWI instead of collapsing to immediates */

#define SH2DRC_COMPATIBLE_OPTIONS   (SH2DRC_STRICT_VERIFY | SH2DRC_FLUSH_PC | SH2DRC_STRICT_PCREL)
#define SH2DRC_FASTEST_OPTIONS  (0)


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
	void sh2drc_set_options(uint32_t options);
	void sh2drc_add_pcflush(offs_t address);

	void sh2_notify_dma_data_available();

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
	virtual uint32_t disasm_min_opcode_bytes() const override { return 2; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;
	address_space *m_program, *m_decrypted_program;

private:
	address_space_config m_program_config, m_decrypted_program_config;

	uint32_t  m_cpu_off;
	//uint32_t  m_dvsr, m_dvdnth, m_dvdntl, m_dvcr;
	uint32_t  m_test_irq;
	
	/*
	struct
	{
		int irq_vector;
		int irq_priority;
	} m_irq_queue[16];
	*/



	int8_t    m_irq_line_state[17];

private:
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
	virtual	void ILLEGAL() override;

	virtual void execute_one_0000(uint16_t opcode) override;
	virtual void execute_one_4000(uint16_t opcode) override;
	virtual void execute_one_f000(uint16_t opcode) override;

	TIMER_CALLBACK_MEMBER( sh2_timer_callback );
	TIMER_CALLBACK_MEMBER( sh2_dma_current_active_callback );
	void sh2_timer_resync();
	void sh2_timer_activate();
	void sh2_do_dma(int dma);
	virtual void sh2_exception(const char *message, int irqline) override;
	void sh2_dmac_check(int dma);
	void sh2_recalc_irq();



	inline uint32_t epc(const opcode_desc *desc);
	inline void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	
	virtual void load_fast_iregs(drcuml_block *block) override;
	virtual void save_fast_iregs(drcuml_block *block) override;

	void code_flush_cache();
	void execute_run_drc();

	virtual void init_drc_frontend() override;
	const opcode_desc* get_desclist(offs_t pc);
	void code_compile_block(uint8_t mode, offs_t pc);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle **handleptr);
	const char *log_desc_flags_to_string(uint32_t flags);
	void log_register_list(drcuml_state *drcuml, const char *string, const uint32_t *reglist, const uint32_t *regnostarlist);
	void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);
	void log_add_disasm_comment(drcuml_block *block, uint32_t pc, uint32_t op);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t ovrpc);
	virtual void generate_delay_slot(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t ovrpc) override;
	bool generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t ovrpc);
	bool generate_group_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

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
private:
	uint16_t m_sh7032_regs[0x200];
};


class sh2_frontend : public sh_frontend
{
public:
	sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:

private:
	virtual bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	virtual bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
};

DECLARE_DEVICE_TYPE(SH1,  sh1_device)
DECLARE_DEVICE_TYPE(SH2,  sh2_device)
DECLARE_DEVICE_TYPE(SH2A, sh2a_device)


#endif // MAME_CPU_SH2_SH2_H
