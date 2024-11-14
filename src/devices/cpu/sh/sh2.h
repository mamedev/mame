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

#ifndef MAME_CPU_SH_SH2_H
#define MAME_CPU_SH_SH2_H

#pragma once

#include "sh.h"

class sh2_frontend;

class sh2_device : public sh_common_execution
{
	friend class sh2_frontend;

public:
	void set_frt_input(int state) override {} // not every CPU needs this, let the ones that do override it

	void func_fastirq(); // required for DRC, needs to be public to be accessible through non-classed static trampoline function

protected:
	sh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type, address_map_constructor internal_map, int addrlines, uint32_t address_mask);

	void check_pending_irq(const char *message);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
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

	virtual void sh2_exception(const char *message, int irqline) override;
	virtual void sh2_exception_internal(const char *message, int irqline, int vector);

	address_space *m_decrypted_program;
	const uint32_t m_am;

	uint32_t m_test_irq;
	int32_t m_internal_irq_vector;
	int8_t m_nmi_line_state;

private:
	virtual uint8_t read_byte(offs_t A) override;
	virtual uint16_t read_word(offs_t A) override;
	virtual uint32_t read_long(offs_t A) override;
	virtual uint16_t decrypted_read_word(offs_t offset) override;
	virtual void write_byte(offs_t A, uint8_t V) override;
	virtual void write_word(offs_t A, uint16_t V) override;
	virtual void write_long(offs_t A, uint32_t V) override;

	virtual void LDCMSR(const uint16_t opcode) override;
	virtual void LDCSR(const uint16_t opcode) override;
	virtual void TRAPA(uint32_t i) override;
	virtual void RTE() override;
	virtual void ILLEGAL() override;

	virtual void execute_one_f000(uint16_t opcode) override;

	virtual void init_drc_frontend() override;
	virtual const opcode_desc* get_desclist(offs_t pc) override;

	virtual void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception) override;
	virtual void static_generate_entry_point() override;
	virtual void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr) override;

	address_space_config m_program_config, m_decrypted_program_config;

	std::unique_ptr<sh2_frontend> m_drcfe; /* pointer to the DRC front-end state */

	uint32_t m_cpu_off;
	int8_t m_irq_line_state[17];
};


class sh2_frontend : public sh_frontend
{
public:
	sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:

private:
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
};

#endif // MAME_CPU_SH_SH2_H
