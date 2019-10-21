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
		BCP_TR,
		BCP_ASP, BCP_DSP
	};

	// construction/destruction
	dp8344_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 2; }
	virtual u32 execute_max_cycles() const override { return 4; }
	virtual u32 execute_input_lines() const override { return 3; }
	virtual void execute_run() override;
	virtual void execute_set_input(int irqline, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// internal functions
	bool get_flag(unsigned f) const;
	void set_receiver_interrupt(bool state);
	void set_transmitter_interrupt(bool state);
	void set_line_turn_around_interrupt(bool state);
	void set_birq_interrupt(bool state);
	void set_timer_interrupt(bool state);
	void set_gie(bool state);
	void set_condition_code(u8 data);
	void set_interrupt_control(u8 data);
	void set_auxiliary_control(u8 data);
	u16 get_timer_count();
	void address_stack_push();
	void address_stack_pop(u8 grf);
	void set_stack_pointer(u8 data);
	void data_stack_push(u8 data);
	u8 data_stack_pop();
	void transceiver_reset();
	void transmit_fifo_push(u8 data);
	u16 transmit_fifo_pop();
	void transmitter_idle();
	void receiver_active();
	void receive_fifo_push(u8 data);
	u8 receive_fifo_pop();
	void set_receiver_error(u8 code);
	u8 get_error_code();
	void set_transceiver_mode(u8 data);
	void clear_network_command_flag(u8 data);
	u8 read_register(unsigned reg);
	u8 read_accumulator() const;
	void write_register(unsigned reg, u8 data);

	// address spaces
	const address_space_config m_inst_config;
	const address_space_config m_data_config;
	address_space *m_inst_space;
	address_space *m_data_space;
	memory_access_cache<1, -1, ENDIANNESS_LITTLE> *m_inst_cache;

	// output callbacks
	devcb_write_line m_birq_out_cb;
	devcb_write_line m_data_out_cb;
	devcb_write_line m_data_dly_cb;
	devcb_write_line m_tx_act_cb;

	// execution state
	u16 m_pc;
	s32 m_icount;
	bool m_nmi_pending;

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
	PAIR16 m_ir[4];

	// timer registers
	u16 m_tr;

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
};

// device type declaration
DECLARE_DEVICE_TYPE(DP8344, dp8344_device)

#endif // MAME_CPU_BCP_DP8344_H
