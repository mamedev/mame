// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor DP8344 Biphase Communications Processor

**********************************************************************/

#ifndef MAME_CPU_BCP_DP8344_H
#define MAME_CPU_BCP_DP8344_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dp8344_device

class dp8344_device : public cpu_device
{
public:
	// input line enumeration
	enum {
		BIRQ_LINE = INPUT_LINE_IRQ0,
		NMI_LINE = INPUT_LINE_NMI,
		DATA_IN_LINE = BIRQ_LINE + 1,
		X_TCLK_LINE
	};

	// register enumeration
	enum {
		BCP_PC,
		BCP_BA, BCP_BB,
		BCP_CCR, BCP_NCF, BCP_ICR, BCP_ACR,
		BCP_DCR, BCP_IBR, BCP_ATR, BCP_FBR,
		BCP_GP0, BCP_GP1, BCP_GP2, BCP_GP3, BCP_GP4, BCP_GP5, BCP_GP6, BCP_GP7,
		BCP_GP8, BCP_GP9, BCP_GP10, BCP_GP11, BCP_GP12, BCP_GP13, BCP_GP14, BCP_GP15,
		BCP_ECR, BCP_TSR, BCP_TCR, BCP_TMR,
		BCP_GP4_ALT, BCP_GP5_ALT, BCP_GP6_ALT, BCP_GP7_ALT,
		BCP_IW, BCP_IX, BCP_IY, BCP_IZ,
		BCP_IWLO, BCP_IXLO, BCP_IYLO, BCP_IZLO,
		BCP_IWHI, BCP_IXHI, BCP_IYHI, BCP_IZHI,
		BCP_TR, BCP_COUNT,
		BCP_ASP, BCP_DSP
	};

	// callback configuration
	auto birq_out_cb() { return m_birq_out_cb.bind(); }
	auto data_out_cb() { return m_data_out_cb.bind(); }
	auto data_dly_cb() { return m_data_dly_cb.bind(); }
	auto tx_act_cb() { return m_tx_act_cb.bind(); }

	// misc. configuration
	void set_auto_start(bool auto_start) { m_auto_start = auto_start; }

	// remote interface
	u8 cmd_r();
	void cmd_w(u8 data);
	u8 remote_read(offs_t offset);
	void remote_write(offs_t offset, u8 data);

protected:
	// construction/destruction
	dp8344_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int irqline, int state) override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	static const char *const s_protocol_names[8];

	enum inst_state : u8 {
		T1_DECODE, T1_START, T1_SKIP, T1_LJMP, T1_LCALL,
		TX_READ, TX_WRITE,
		TX1_JRMK, TX1_JMP, TX2_JMP, TX_CALL, TX_RETF,
		T2_NEXT, T2_STORE, T2_READ, T2_WRITE, T2_ABSOLUTE
	};

	// internal functions
	void set_receiver_interrupt(bool state);
	void set_transmitter_interrupt(bool state);
	void set_line_turn_around_interrupt(bool state);
	void set_birq_interrupt(bool state);
	void set_timer_interrupt(bool state);
	void set_gie(bool state);
	void set_condition_code(u8 data);
	void set_interrupt_control(u8 data);
	void set_auxiliary_control(u8 data);
	void set_device_control(u8 data);
	bool interrupt_active() const;
	u8 get_interrupt_vector() const;
	bool get_flag(unsigned f) const;
	void set_nz(u8 result);
	void set_carry(bool state);
	static u8 rotate_right(u8 data, u8 b);
	u8 add_nzcv(u8 s1, u8 s2, bool carry_in);
	u8 sub_nzcv(u8 s1, u8 s2, bool carry_in);
	void timer_count();
	void address_stack_push();
	void address_stack_pop(u8 g, bool rf);
	void set_stack_pointer(u8 data);
	void data_stack_push(u8 data);
	u8 data_stack_pop();
	void transceiver_reset();
	void transmit_fifo_push(u8 data);
	u16 transmit_fifo_pop();
	void transmitter_idle();
	void receiver_active();
	void receive_fifo_push(u16 data);
	u8 receive_fifo_pop(bool test);
	void set_receiver_error(u8 code);
	u8 get_error_code(bool test);
	void set_transceiver_command(u8 data);
	void set_transceiver_mode(u8 data);
	void clear_network_command_flag(u8 data);
	u8 read_register(unsigned reg, bool test);
	u8 read_accumulator() const;
	void write_register(unsigned reg, u8 data);
	void prefetch_instruction();
	void latch_address(bool rw);
	void instruction_wait();
	inst_state decode_instruction();
	void store_result();
	void data_write();
	void data_read();

	// address spaces
	const address_space_config m_inst_config;
	const address_space_config m_data_config;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache m_inst_cache;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_inst_space;
	memory_access<16, 0,  0, ENDIANNESS_LITTLE>::specific m_data_space;

	// output callbacks
	devcb_write_line m_birq_out_cb;
	devcb_write_line m_data_out_cb;
	devcb_write_line m_data_dly_cb;
	devcb_write_line m_tx_act_cb;

	// execution state
	u16 m_pc;
	u16 m_ppc;
	s32 m_icount;
	bool m_nmi_pending;
	inst_state m_inst_state;
	u8 m_wait_states;
	u8 m_source_data;
	u16 m_data_address;

	// control registers
	u8 m_ccr;
	u8 m_ncf;
	u8 m_icr;
	u8 m_acr;
	u8 m_dcr;
	u8 m_ibr;
	u8 m_atr;
	u8 m_fbr;
	u8 m_ecr;
	u8 m_tsr;
	u8 m_tcr;
	u8 m_tmr;

	// general purpose registers
	u8 m_gp_main[16];
	u8 m_gp_alt[4];

	// index registers
	u16 m_ir[4];

	// timer registers
	u16 m_tr;
	u8 m_tclk;
	u16 m_tcount;

	// internal stacks
	u32 m_as[12];
	u8 m_ds[16];
	u8 m_asp;
	u8 m_dsp;

	// bank selects
	bool m_ba;
	bool m_bb;

	// remote interface registers
	u8 m_ric;
	bool m_hib;
	u16 m_latched_instr;
	bool m_auto_start;

	// input lines
	bool m_nmi_state;

	// transceiver FIFOs
	u16 m_rfifo[3];
	u16 m_tfifo[3];
	u8 m_rfifo_head;
	u8 m_tfifo_head;

	// misc. state
	bool m_receiver_interrupt;
};

// ======================> dp8344a_device

class dp8344a_device : public dp8344_device
{
public:
	// device type constructor
	dp8344a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> dp8344b_device

class dp8344b_device : public dp8344_device
{
public:
	// device type constructor
	dp8344b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(DP8344A, dp8344a_device)
DECLARE_DEVICE_TYPE(DP8344B, dp8344b_device)

#endif // MAME_CPU_BCP_DP8344_H
