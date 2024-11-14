// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// *****************************
// Emulator for HP Capricorn CPU
// *****************************
//
// References:
// HP 00085-90444, Nov 81, HP83/85 Assembler ROM manual
// US Patent 4,424,563 describing the architecture of HP85
//
#ifndef MAME_CPU_CAPRICORN_CAPRICORN_H
#define MAME_CPU_CAPRICORN_CAPRICORN_H

#pragma once

class capricorn_cpu_device : public cpu_device
{
public:
	capricorn_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t flatten_burst();

	// This CB reports the start of LMA (Load Memory Address) cycles
	// Its parameter is true when starting a LMARD cycle
	auto lma_cb() { return m_lma_out.bind(); }

	// Tap into fetched opcodes
	auto opcode_cb() { return m_opcode_func.bind(); }

	// Interrupt vector fetch (INTACK: /LMA = /RD = /WR = all 0)
	auto intack_cb() { return m_intack_in.bind(); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 17; }
	virtual void execute_run() override;
	virtual void execute_set_input(int linenum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	int m_icount;

	// State of processor
	uint8_t m_reg[ 64 ];// Registers R00-R77
	uint8_t m_arp;      // ARP register (6 bits)
	uint8_t m_drp;      // DRP register (6 bits)
	uint8_t m_reg_E;    // E register (4 bits)
	uint16_t m_flags;   // Flags
	uint16_t m_genpc;   // PC

	// Burst memory accesses
	bool m_flatten;         // Consecutive accesses to memory are "flattened"
	uint16_t m_start_addr;  // Start address of burst
	uint16_t m_curr_addr;   // Current address in burst

	devcb_write8 m_opcode_func;
	devcb_write_line m_lma_out;
	devcb_read8 m_intack_in;

	// Effective Addresses
	// When b17 = 0, b15..b0 hold 16-bit memory address
	// When b17 = 1, b5..b0 hold register index
	typedef unsigned ea_addr_t;

	void start_mem_burst(ea_addr_t addr , bool lmard = false);
	uint16_t read_u16(ea_addr_t addr , bool lmard = true);
	void write_u16(ea_addr_t addr , uint16_t v);
	uint8_t RM(ea_addr_t& addr);
	void WM(ea_addr_t& addr , uint8_t v);
	uint8_t fetch();
	void offset_pc(uint16_t offset);
	void vector_to_pc(uint8_t vector);
	void do_jump(bool condition);
	uint8_t get_lower_boundary() const;
	uint8_t get_upper_boundary() const;
	void update_flags_right(uint8_t res);
	void update_flags_left(uint8_t res);
	void update_flags_every(uint8_t res);
	ea_addr_t get_ea_reg_imm();
	ea_addr_t get_ea_lit_imm(bool multibyte);
	ea_addr_t get_ea_reg_dir();
	ea_addr_t get_ea_lit_dir();
	ea_addr_t get_ea_reg_indir();
	ea_addr_t get_ea_idx_dir();
	ea_addr_t get_ea_jsbx();
	ea_addr_t get_ea_lit_indir();
	ea_addr_t get_ea_idx_indir();

	static uint8_t add_bcd_digits(uint8_t first , uint8_t second , bool& carry);
	static uint8_t add_bcd_bytes(uint8_t first , uint8_t second , bool& carry);
	static uint8_t sub_bcd_digits(uint8_t first , uint8_t second , bool& carry);
	static uint8_t sub_bcd_bytes(uint8_t first , uint8_t second , bool& carry);

	void do_AN_op(ea_addr_t ea);
	void do_LD_op(ea_addr_t ea , bool multibyte);
	void do_ST_op(ea_addr_t ea , bool multibyte);
	void do_AD_op(ea_addr_t ea , bool multibyte);
	void do_SB_op(ea_addr_t ea , bool multibyte);
	void do_CM_op(ea_addr_t ea , bool multibyte);
	void do_OR_op(bool multibyte);
	void do_XR_op(bool multibyte);
	void do_IC_op(bool multibyte);
	void do_DC_op(bool multibyte);
	void do_TC_op(bool multibyte);
	void do_NC_op(bool multibyte);
	void do_TS_op(bool multibyte);
	void do_CL_op(bool multibyte);
	void do_EL_op(bool multibyte);
	void do_LL_op(bool multibyte);
	void do_ER_op(bool multibyte);
	void do_LR_op(bool multibyte);
	void do_SAD_op();
	void do_PAD_op();
	void do_RTN_op();
	void push_pc();
	void do_JSB_op(ea_addr_t ea);
	void do_PU_op(bool multibyte , bool direct , bool increment);
	void do_PO_op(bool multibyte , bool direct , bool increment);

	void execute_one(uint8_t opcode);
	void take_interrupt();
};

DECLARE_DEVICE_TYPE(HP_CAPRICORN , capricorn_cpu_device);

#endif // MAME_CPU_CAPRICORN_CAPRICORN_H
