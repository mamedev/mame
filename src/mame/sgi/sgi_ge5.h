// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_SGI_GE5_H
#define MAME_SGI_SGI_GE5_H

#pragma once

#include "machine/wtl3132.h"

class sgi_ge5_device : public cpu_device
{
public:
	sgi_ge5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// host interface
	auto out_int() { return m_int_cb.bind(); }
	auto fifo_empty() { return m_fifo_empty.bind(); }
	auto fifo_read() { return m_fifo_read.bind(); }

	// raster engine interface
	void re_rdy_w(int state) { m_re_rdy = bool(state); }
	void re_drq_w(int state) { m_re_drq = bool(state); }
	auto re_r() { return m_re_r.bind(); }
	auto re_w() { return m_re_w.bind(); }

	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override {}

	u32 buffer_r(offs_t offset) { return m_bus; }

	template <bool High> u32 code_r(offs_t offset);
	u32 data_r(offs_t offset);
	template <bool High> void code_w(offs_t offset, u32 data, u32 mem_mask);
	void data_w(offs_t offset, u32 data, u32 mem_mask);

	void command_w(offs_t offset, u16 data, u16 mem_mask);
	u16 pc_r() { return m_pc; }
	void mar_w(offs_t offset, u32 data, u32 mem_mask) { m_mar = offset & 0x7f; }
	void cwen_w(int state) { m_cwen = bool(state); }

	u32 finish_r(offs_t offset) { return m_finish[offset]; }
	void finish_w(offs_t offset, u32 data, u32 mem_mask) { m_finish[offset] = data; }

protected:
	void code_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	void decode();
	void secondary();

	void set_int(bool state)
	{
		if (state != m_int_state)
		{
			m_int_state = state;
			m_int_cb(m_int_state);
		}
	}

private:
	// device configuration state
	address_space_config m_code_config;
	address_space_config m_data_config;

	required_device<wtl3132_device> m_fpu;

	devcb_write_line m_int_cb;
	devcb_read_line m_fifo_empty;
	devcb_read64 m_fifo_read;
	devcb_read32 m_re_r;
	devcb_write32 m_re_w;

	// runtime state
	int m_icount;
	enum ge5_state : unsigned
	{
		STALL,
		DECODE,   // fetch and decode instruction, update pointers
		READ,     // read source
		CONTROL,  // flow control
		WRITE,    // write destination
		COMPLETE, // cycle fpu and complete
	}
	m_state;

	// line state
	bool m_int_state;
	bool m_cwen;
	bool m_fpu_c;
	bool m_re_rdy;
	bool m_re_drq;

	// hq1 registers
	u8 m_mar;
	u16 m_pc;
	unsigned m_sp;
	u16 m_stack[8];
	u8 m_reptr;
	u16 m_memptr;
	u16 m_memptr_temp;
	u16 m_dma_count;
	u32 m_finish[2];

	// decoded instruction
	struct ge5_decode
	{
		// primary
		unsigned source;
		bool inc_reptr;
		bool secondary;
		bool inc_memptr;
		unsigned destination;
		unsigned control;
		u64 fpu;

		// secondary
		u8 operation;
		u16 immediate;
	}
	m_decode;

	// dynamic state
	u64 m_bus;
	u32 m_fpu_data;
	bool m_fpu_c_latch;
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

#endif // MAME_SGI_SGI_GE5_H
