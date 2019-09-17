// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_VIDEO_SGI_GE5_H
#define MAME_VIDEO_SGI_GE5_H

#pragma once

#include "machine/wtl3132.h"

class sgi_ge5_device : public cpu_device
{
public:
	sgi_ge5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto out_int() { return m_int_cb.bind(); }
	auto fifo_empty() { return m_fifo_empty.bind(); }
	auto fifo_read() { return m_fifo_read.bind(); }

	auto re_r() { return m_re_r.bind(); }
	auto re_w() { return m_re_w.bind(); }

	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 1; }
	virtual u32 execute_max_cycles() const override { return 1; }
	virtual u32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override {}

	u32 code_r(offs_t offset);
	u32 data_r(offs_t offset);
	void code_w(offs_t offset, u32 data, u32 mem_mask);
	void data_w(offs_t offset, u32 data, u32 mem_mask);

	void command_w(offs_t offset, u16 data, u16 mem_mask);
	u16 pc_r() { return m_pc; }
	void mar_w(offs_t offset, u32 data, u32 mem_mask) { m_mar = offset & 0x7f; }
	void mar_msb_w(offs_t offset, u32 data, u32 mem_mask) { m_mar_msb = bool(offset); }
	void cwen_w(int state) { m_cwen = bool(state); }

protected:
	void code_map(address_map &map);
	void data_map(address_map &map);

private:
	// device configuration state
	address_space_config m_code_config;
	address_space_config m_data_config;

	required_device<wtl3132_device> m_fpu;

	devcb_write_line m_int_cb;
	int m_int_state;

	devcb_read_line m_fifo_empty;
	devcb_read64 m_fifo_read;
	devcb_read32 m_re_r;
	devcb_write32 m_re_w;

	// runtime state
	int m_icount;

	// cpu state
	bool m_fetch;
	u16 m_pc;
	unsigned m_sp;
	u16 m_stack[8];

	u16 m_memptr;
	u16 m_memptr_temp;
	u8 m_reptr;
	u16 m_dma_count;

	u64 m_bus;

	u8 m_mar;
	bool m_mar_msb;

	int m_fpu_c;
	int m_fpu_z;

	bool m_cwen;
};

class sgi_ge5_disassembler : public util::disasm_interface
{
public:
	sgi_ge5_disassembler() = default;
	virtual ~sgi_ge5_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;

private:
};

DECLARE_DEVICE_TYPE(SGI_GE5, sgi_ge5_device)

#endif // MAME_VIDEO_SGI_GE5_H
