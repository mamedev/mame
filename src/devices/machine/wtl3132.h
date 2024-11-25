// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_WTL3132_H
#define MAME_MACHINE_WTL3132_H

#pragma once

#include "softfloat3/source/include/softfloat.h"

class wtl3132_device : public device_t
{
public:
	wtl3132_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// output lines
	auto out_fpcn() { return m_fpcn_cb.bind(); }
	auto out_fpex() { return m_fpex_cb.bind(); }
	auto out_zero() { return m_zero_cb.bind(); }
	auto out_port_x() { return m_port_x_cb.bind(); }

	// code and data ports
	void c_port_w(u64 data) { m_c_port = data; }
	u32 x_port_r() { return m_x_port.v; }
	void x_port_w(u32 data) { m_x_port.v = data; }

	// input lines
	void abort_w(int state) { m_abort = state; }
	void clk_w(int state);
	void neut_w(int state) { m_neut = state; }
	void stall_w(int state) { m_stall = state; }

	// disassembly helpers
	static std::string disassemble(u64 const code);
	static std::string reg(unsigned const reg);
	static std::string mbin(u64 const code);
	static std::string abin(u64 const code);
	static std::string adst(u64 const code);

	void state_add(device_state_interface &parent, unsigned base = 0)
	{
		parent.state_add(base + 0, "MODE", m_mode).formatstr("%04X");
		parent.state_add(base + 1, "T1", m_t1.v).formatstr("%08X");
		parent.state_add(base + 2, "T2", m_t2.v).formatstr("%08X");
		parent.state_add(base + 3, "T3", m_t3.v).formatstr("%08X");

		for (unsigned i = 0; i < 32; i++)
			parent.state_add(base + i + 4, util::string_format("F%d", i).c_str(), m_f[i].v).formatstr("%08X");
	}

	enum code_mask : u64
	{
		// opcode is 34 bits wide
		M_ENCN = 0x0'00000003, // condition output select
		M_MBIN = 0x0'00000004, // mbin input select
		M_ADST = 0x0'00000018, // alu destination select
		M_ABIN = 0x0'000000e0, // abin input select
		M_DADD = 0x0'00001f00, // d port register address
		M_IOCT = 0x0'00006000, // i/o control
		M_CWEN = 0x0'00008000, // c port write enable (active low)
		M_CADD = 0x0'001f0000, // c port register address
		M_BADD = 0x0'03e00000, // b port register address
		M_AADD = 0x0'7c000000, // a port register address
		M_F    = 0x3'80000000, // function code

		// internal pipeline control flag
		M_CANCEL = 0x80000000'00000000, // cancelled
	};

	enum code_shift : unsigned
	{
		S_ENCN =  0, // condition output select
		S_MBIN =  2, // mbin input select
		S_ADST =  3, // alu destination select
		S_ABIN =  5, // abin input select
		S_DADD =  8, // d port register address
		S_IOCT = 13, // i/o control
		S_CWEN = 15, // c port write enable (active low)
		S_CADD = 16, // c port register address
		S_BADD = 21, // b port register address
		S_AADD = 26, // a port register address
		S_F    = 31, // function code
	};

	enum function_code : unsigned
	{
		F_MISC  = 0, // miscellaneous
		F_FSUBR = 1, // negate and add
		F_FSUB  = 2, // subtract
		F_FADD  = 3, // add
		F_FMNA  = 5, // multiply, negate and add
		F_FMNS  = 6, // multiply, negate and subtract
		F_FMAC  = 7, // multiply and accumulate
	};

	enum misc_code : unsigned
	{
		MF_FCLSR = 0, // clear status register
		MF_FSTSR = 1, // read status register
		MF_FMODE = 3, // load mode register
		MF_FABS  = 4, // absolute value
		MF_FLOAT = 5, // fixed-to-float
		MF_FIX   = 6, // float-to-fixed
		MF_FLUT  = 7, // lookup operation
	};

	enum mode_mask : u16
	{
		MODE_IBA = 0x0001, // internal bypass A enable
		MODE_RTN = 0x0002, // round to nearest
		MODE_IBP = 0x0008, // input bypass enable
		MODE_OBP = 0x0010, // output bypass enable
		MODE_FXO = 0x0020, // fpex overflow enable
		MODE_CPL = 0x0040, // coprocessor load enable
		MODE_FXH = 0x0100, // fpex active high
		MODE_DBP = 0x0200, // double pump enable (3332 only)
		MODE_IBB = 0x0800, // internal bypass B enable
		MODE_YLI = 0x1000, // Y late input enable (3332 only)
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void stage1(unsigned const index);
	void stage2(unsigned const index);
	void stage3(unsigned const index);
	void stage4(unsigned const index);

private:
	devcb_write_line m_fpcn_cb;
	devcb_write_line m_fpex_cb;
	devcb_write_line m_zero_cb;
	devcb_write32 m_port_x_cb;

	// output line state
	bool m_fpcn_state;
	bool m_fpex_state;
	bool m_zero_state;

	// port state
	u64 m_c_port;
	float32_t m_x_port;
	float32_t m_x_in;
	float32_t m_x_out;

	// input line state
	int m_abort;
	int m_neut;
	int m_stall;

	// pipeline state
	u64 m_slot[4];
	unsigned m_head;

	// registers
	u16 m_mode;
	float32_t m_f[32];
	float32_t m_t1;
	float32_t m_t2;
	float32_t m_t3;
	bool m_cr;
	bool m_sr;
	bool m_zr;

	// C bus
	float32_t m_c_bus_data;
	unsigned m_c_bus_addr;
	bool m_c_bus_cwen;

	// multiplier inputs and output
	float32_t m_ma_in;
	float32_t m_mb_in;
	float32_t m_m_out;

	// accumulator buffers, inputs and output
	float32_t m_aa_in[2];
	float32_t m_ab_in[2];
	float32_t m_a_out;
};

DECLARE_DEVICE_TYPE(WTL3132, wtl3132_device)

#endif // MAME_MACHINE_WTL3132_H
