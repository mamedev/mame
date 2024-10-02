// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_CPU_MCS40_MCS40_H
#define MAME_CPU_MCS40_MCS40_H

#pragma once

#include <memory>
#include <utility>


/***********************************************************************
    CONSTANTS
***********************************************************************/

enum
{
	I4004_TEST_LINE = 0,

	I4040_TEST_LINE = I4004_TEST_LINE,
	I4040_INT_LINE = 1,
	I4040_STP_LINE = 2
};



/***********************************************************************
    TYPE DEFINITIONS
***********************************************************************/

class mcs40_cpu_device_base : public cpu_device
{
public:
	enum {
		AS_ROM              = AS_PROGRAM,
		AS_RAM_MEMORY       = AS_DATA,
		AS_ROM_PORTS        = AS_IO,
		AS_RAM_STATUS       = AS_OPCODES + 1,
		AS_RAM_PORTS,
		AS_PROGRAM_MEMORY
	};
	enum class phase : u8 { A1, A2, A3, M1, M2, X1, X2, X3 };

	// step isn't a real signal, but realistically anything watching the bus will have a counter to track it
	typedef device_delegate<void (phase step, u8 sync, u8 data)> bus_cycle_delegate;

	// configuration helpers
	template <typename... T> void set_rom_map(T &&... args) { set_addrmap(AS_ROM, std::forward<T>(args)...); }
	template <typename... T> void set_ram_memory_map(T &&... args) { set_addrmap(AS_RAM_MEMORY, std::forward<T>(args)...); }
	template <typename... T> void set_rom_ports_map(T &&... args) { set_addrmap(AS_ROM_PORTS, std::forward<T>(args)...); }
	template <typename... T> void set_ram_status_map(T &&... args) { set_addrmap(AS_RAM_STATUS, std::forward<T>(args)...); }
	template <typename... T> void set_ram_ports_map(T &&... args) { set_addrmap(AS_RAM_PORTS, std::forward<T>(args)...); }
	template <typename... T> void set_program_memory_map(T &&... args) { set_addrmap(AS_PROGRAM_MEMORY, std::forward<T>(args)...); }
	template <typename... T> void set_bus_cycle_cb(T &&... args) { m_bus_cycle_cb.set(std::forward<T>(args)...); }
	auto i4289_pm_cb() { return m_4289_pm_cb.bind(); }
	auto i4289_f_l_cb() { return m_4289_f_l_cb.bind(); }

	// chip select outputs
	u8 get_cm_rom() const { return m_cm_rom; }
	u8 get_cm_ram() const { return m_cm_ram; }

	// 4008/4009 or 4289 outputs
	u8 get_4289_a() const { return m_4289_a; } // 8-bit address
	u8 get_4289_c() const { return m_4289_c; } // 4-bit chip select
	int get_4289_pm() const { return BIT(m_4289_pm, 0); } // 0 = active
	int get_4289_f_l() const { return BIT(m_4289_f_l, 0); } // 1 = odd, 0 = even

protected:
	enum class cycle : u8 { OP, IM, IN };
	enum class pmem : u8 { NONE, READ, WRITE };

	mcs40_cpu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock,
			bool extended_cm,
			unsigned rom_width,
			unsigned stack_ptr_mask,
			unsigned index_reg_cnt,
			unsigned cr_mask);

	// device_t implementation
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_import(device_state_entry const &entry) override;
	virtual void state_export(device_state_entry const &entry) override;
	virtual void state_string_export(device_state_entry const &entry, std::string &str) const override;

	// instruction execution
	virtual bool is_io_op(u8 opr) = 0;
	virtual cycle do_cycle1(u8 opr, u8 opa, pmem &program_op) = 0;
	virtual void do_cycle2(u8 opr, u8 opa, u8 arg) = 0;
	virtual u8 do_io(u8 opr, u8 opa) = 0;

	// register access
	u8 get_a() const;
	u8 get_c() const;
	void set_a(u8 val);
	void set_c(u8 val);
	void set_a_c(u8 val);
	void set_pc(u16 addr, u16 mask);
	void push_pc();
	void pop_pc();
	u8 &index_reg_pair(unsigned n);
	u8 get_index_reg(unsigned n);
	void set_index_reg(unsigned n, u8 val);
	void set_index_reg_bank(u8 val);

	// I/O control
	void halt_decoded();
	void set_rom_addr(u16 addr, u16 mask);
	u8 get_cr();
	void set_cr(u8 val, u8 mask);
	void set_pending_rom_bank(u8 val);
	void set_rc(u8 val);
	u8 read_memory();
	void write_memory(u8 val);
	u8 read_status();
	void write_status(u8 val);
	u8 read_rom_port();
	void write_rom_port(u8 val);
	void write_memory_port(u8 val);

	// input lines
	bool get_test();
	void set_test(int state);
	void set_stp(int state);

	// configuration helpers
	auto sync_cb() { return m_sync_cb.bind(); }
	template <unsigned N> auto cm_rom_cb() { return m_cm_rom_cb[N].bind(); }
	template <unsigned N> auto cm_ram_cb() { return m_cm_ram_cb[N].bind(); }
	auto cy_cb() { return m_cy_cb.bind(); }
	auto stp_ack_cb() { return m_stp_ack_cb.bind(); }

private:
	enum
	{
		I4004_A = 1,
		I4004_R01, I4004_R23, I4004_R45, I4004_R67, I4004_R89, I4004_RAB, I4004_RCD, I4004_REF,
		I4040_RGH, I4040_RIJ, I4040_RKL, I4040_RMN,
		I4004_R0, I4004_R1, I4004_R2, I4004_R3, I4004_R4, I4004_R5, I4004_R6, I4004_R7,
		I4004_R8, I4004_R9, I4004_R10, I4004_R11, I4004_R12, I4004_R13, I4004_R14, I4004_R15,
		I4040_R16, I4040_R17, I4040_R18, I4040_R19, I4040_R20, I4040_R21, I4040_R22, I4040_R23,
		I4004_SP,
		I4004_ADDR0, I4004_ADDR1, I4004_ADDR2, I4004_ADDR3,
		I4040_ADDR4, I4040_ADDR5, I4040_ADDR6, I4040_ADDR7,
		I4004_CR, I4004_RC, I4004_RCN,
		I4040_SRC
	};

	// instruction phases
	void do_a1();
	void do_a2();
	void do_a3();
	void do_m1();
	void do_m2();
	void do_x1();
	void do_x2();
	void do_x3();

	// internal helpers
	u16 &pc() { return m_addr_stack[m_stack_ptr]; }
	u16 rom_bank() const { return BIT(m_cr, 3) ? 0x1000U : 0x0000U; }
	u16 program_addr() const { return (u16(BIT(m_cr, 3)) << 9) | (u16(m_4289_a) << 1) | BIT(m_4289_f_l, 0); }
	void update_cm_rom(u8 val);
	void update_cm_ram(u8 val);
	void update_cy(u8 val);
	void update_4289_pm(u8 val);
	void update_4289_f_l(u8 val);

	// address spaces
	address_space_config const  m_space_config[7];
	address_space               *m_spaces[7];
	memory_access<12, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;

	// bus snooping callback
	bus_cycle_delegate          m_bus_cycle_cb;

	// output callbacks
	devcb_write_line            m_sync_cb;
	devcb_write_line::array<2>  m_cm_rom_cb;
	devcb_write_line::array<4>  m_cm_ram_cb;
	devcb_write_line            m_cy_cb, m_stp_ack_cb;

	// 4008/4009 or 4289 output callbacks
	devcb_write_line            m_4289_pm_cb, m_4289_f_l_cb;

	// configuration
	bool const  m_extended_cm;
	u8 const    m_stack_ptr_mask;
	u8 const    m_index_reg_cnt;
	u8 const    m_cr_mask;
	u16 const   m_pc_mask;

	// machine/instruction phase
	int     m_icount;
	phase   m_phase;
	cycle   m_cycle;
	bool    m_io_pending;
	pmem    m_program_op;

	// stop/halt control
	bool    m_stop_latch, m_stop_ff, m_decoded_halt, m_resume;

	// instruction ROM fetch/decode
	u16     m_rom_bank, m_rom_addr;
	u8      m_opr, m_opa, m_arg;
	bool    m_4289_first;

	// ALU registers
	u8      m_a, m_c;

	// address stack
	std::unique_ptr<u16 []> m_addr_stack;
	u8                      m_stack_ptr;

	// index registers
	std::unique_ptr<u8 []>  m_index_regs;
	u8                      m_index_reg_bank;

	// RAM/I/O control
	u8          m_cr, m_pending_cr3, m_latched_rc, m_new_rc, m_src;
	bool        m_rc_pending;

	// input/output lines
	int m_test, m_stp;
	u8  m_cm_rom, m_cm_ram, m_cy;
	u8  m_4289_a, m_4289_c, m_4289_pm, m_4289_f_l;

	// state export/import
	std::unique_ptr<u8 []>  m_index_reg_halves;
	u16                     m_pc, m_pcbase;
	u8                      m_genflags;
};


class i4004_cpu_device : public mcs40_cpu_device_base
{
public:
	// configuration helpers
	using mcs40_cpu_device_base::sync_cb;
	auto cm_rom_cb() { return mcs40_cpu_device_base::cm_rom_cb<0>(); }
	template <unsigned N> auto cm_ram_cb() { return mcs40_cpu_device_base::cm_ram_cb<N>(); }

	i4004_cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~i4004_cpu_device();

protected:
	using mcs40_cpu_device_base::mcs40_cpu_device_base;

	// device_execute_interface implementation
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// mcs40_cpu_device_base implementation
	virtual bool is_io_op(u8 opr) override;
	virtual cycle do_cycle1(u8 opr, u8 opa, pmem &program_op) override;
	virtual void do_cycle2(u8 opr, u8 opa, u8 arg) override;
	virtual u8 do_io(u8 opr, u8 opa) override;
};


class i4040_cpu_device : public i4004_cpu_device
{
public:
	// configuration helpers
	template <unsigned N> auto cm_rom_cb() { return mcs40_cpu_device_base::cm_rom_cb<N>(); }
	template <unsigned N> auto cm_ram_cb() { return mcs40_cpu_device_base::cm_ram_cb<N>(); }
	auto cy_cb() { return i4004_cpu_device::cy_cb(); }
	using mcs40_cpu_device_base::stp_ack_cb;

	i4040_cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~i4040_cpu_device();

protected:
	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface implementation
	virtual void execute_set_input(int inputnum, int state) override;

	// mcs40_cpu_device_base implementation
	virtual cycle do_cycle1(u8 opr, u8 opa, pmem &program_op) override;
};



/***********************************************************************
    DEVICE TYPE DECLARATIONS
***********************************************************************/

DECLARE_DEVICE_TYPE(I4004, i4004_cpu_device)
DECLARE_DEVICE_TYPE(I4040, i4040_cpu_device)

#endif // MAME_CPU_MCS40_MCS40_H
