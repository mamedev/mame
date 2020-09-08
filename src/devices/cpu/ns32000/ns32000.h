// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_NS32000_NS32000_H
#define MAME_CPU_NS32000_NS32000_H

#pragma once

class ns32000_device : public cpu_device
{
public:
	// construction/destruction
	ns32000_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

protected:
	ns32000_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, int databits, int addrbits);

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 6; }
	virtual u32 execute_input_lines() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	// device_state_interface overrides
	virtual void state_string_export(device_state_entry const &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	enum addr_mode_type : unsigned
	{
		IMM,
		REG,
		MEM,
		IND,
		EXT,
		TOS,
	};
	struct addr_mode
	{
		addr_mode(unsigned gen)
			: gen(gen)
			, base(0)
			, disp(0)
		{};

		void rmw() { if (type == TOS) type = MEM; }
		void addr() { if (type == TOS) type = MEM; }
		void regaddr() { if (type == TOS) type = MEM; }

		unsigned gen;
		addr_mode_type type;
		u32 base;
		u32 disp;
	};

	// instruction decoding helpers
	s32 displacement(unsigned &bytes);
	void decode(addr_mode *mode, unsigned size, unsigned &bytes);

	// operand read/write helpers
	u32 ea(addr_mode const mode);
	u32 gen_read(addr_mode mode, unsigned size);
	s32 gen_read_sx(addr_mode mode, unsigned size);
	void gen_write(addr_mode mode, unsigned size, u64 data);

	// other execution helpers
	bool condition(unsigned const cc);
	void flags(u32 const src1, u32 const src2, u32 const dest, unsigned const size, bool const subtraction);
	void interrupt(unsigned const vector, u32 const return_address, bool const trap = true);

	// configuration
	address_space_config m_program_config;
	address_space_config m_interrupt_config;

	// emulation state
	int m_icount;

	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::cache m_bus[16];

	u32 m_pc;      // program counter
	u32 m_sb;      // static base
	u32 m_fp;      // frame pointer
	u32 m_sp1;     // user stack pointer
	u32 m_sp0;     // interrupt stack pointer
	u32 m_intbase; // interrupt base
	u16 m_psr;     // processor status
	u16 m_mod;     // module
	u8 m_cfg;      // configuration

	u32 m_r[8];
	u32 m_f[8];

	bool m_nmi_line;
	bool m_int_line;
	bool m_wait;
};

class ns32008_device : public ns32000_device
{
public:
	ns32008_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class ns32016_device : public ns32000_device
{
public:
	ns32016_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class ns32032_device : public ns32000_device
{
public:
	ns32032_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(NS32008, ns32008_device)
DECLARE_DEVICE_TYPE(NS32016, ns32016_device)
DECLARE_DEVICE_TYPE(NS32032, ns32032_device)

#endif // MAME_CPU_NS32000_NS32000_H
