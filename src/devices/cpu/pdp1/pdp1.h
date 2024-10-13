// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#ifndef MAME_CPU_PDP1_PDP1_H
#define MAME_CPU_PDP1_PDP1_H

#pragma once



/* register ids for pdp1_get_reg/pdp1_set_reg */
enum
{
	PDP1_PC=1, PDP1_IR, PDP1_MB, PDP1_MA, PDP1_AC, PDP1_IO,
	PDP1_PF, PDP1_PF1, PDP1_PF2, PDP1_PF3, PDP1_PF4, PDP1_PF5, PDP1_PF6,
	PDP1_TA, PDP1_TW,
	PDP1_SS, PDP1_SS1, PDP1_SS2, PDP1_SS3, PDP1_SS4, PDP1_SS5, PDP1_SS6,
	PDP1_SNGL_STEP, PDP1_SNGL_INST, PDP1_EXTEND_SW,
	PDP1_RUN, PDP1_CYC, PDP1_DEFER, PDP1_BRK_CTR, PDP1_OV, PDP1_RIM, PDP1_SBM, PDP1_EXD,
	PDP1_IOC, PDP1_IOH, PDP1_IOS
};


struct pdp1_reset_param_t
{
	/* 0: no extend support, 1: extend with 15-bit address, 2: extend with 16-bit address */
	int extend_support;
	/* 1 to use hardware multiply/divide (MUL, DIV) instead of MUS, DIS */
	int hw_mul_div;
	/* 0: standard sequence break system 1: type 20 sequence break system */
	int type_20_sbs;
};

#define IOT_NO_COMPLETION_PULSE -1


class pdp1_device : public cpu_device, public pdp1_reset_param_t
{
public:
	typedef device_delegate<void (int op2, int nac, int mb, int &io, int ac)> iot_delegate;
	typedef device_delegate<void ()> io_sc_delegate;

	enum opcode
	{
		AND = 001,
		IOR = 002,
		XOR = 003,
		XCT = 004,
		CALJDA = 007,
		LAC = 010,
		LIO = 011,
		DAC = 012,
		DAP = 013,
		DIP = 014,
		DIO = 015,
		DZM = 016,
		ADD = 020,
		SUB = 021,
		IDX = 022,
		ISP = 023,
		SAD = 024,
		SAS = 025,
		MUS_MUL = 026,
		DIS_DIV = 027,
		JMP = 030,
		JSP = 031,
		SKP = 032,
		SFT = 033,
		LAW = 034,
		IOT = 035,
		OPR = 037
	};

	// construction/destruction
	pdp1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <int I, typename... T> void set_iot_callback(T &&... args) { m_extern_iot[I].set(std::forward<T>(args)...); }
	template <typename... T> void set_io_sc_callback(T &&... args) { m_io_sc_callback.set(std::forward<T>(args)...); }
	void set_reset_param(const pdp1_reset_param_t *param) { m_reset_param = param; }

	void pulse_start_clear();
	void io_complete() { m_ios = 1; }
	void null_iot(int op2, int nac, int mb, int &io, int ac);
	void lem_eem_iot(int op2, int nac, int mb, int &io, int ac);
	void sbs_iot(int op2, int nac, int mb, int &io, int ac);
	void type_20_sbs_iot(int op2, int nac, int mb, int &io, int ac);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 31; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	/* processor registers */
	uint32_t m_pc;      /* program counter (12, 15 or 16 bits) */
	int m_ir;         /* basic operation code of current instruction (5 bits) */
	int m_mb;         /* memory buffer (used for holding the current instruction only) (18 bits) */
	int m_ma;         /* memory address (12, 15 or 16 bits) */
	int m_ac;         /* accumulator (18 bits) */
	int m_io;         /* i/o register (18 bits) */
	int m_pf;         /* program flag register (6 bits) */

	/* operator panel switches */
	int m_ta;         /* current state of the 12 or 16 address switches */
	int m_tw;         /* current state of the 18 test word switches */
	int m_ss;         /* current state of the 6 sense switches on the operator panel (6 bits) */
	unsigned int m_sngl_step; /* stop every memory cycle */
	unsigned int m_sngl_inst; /* stop every instruction */
	unsigned int m_extend_sw; /* extend switch (loaded into the extend flip-flop on start/read-in) */

	/* processor state flip-flops */
	unsigned int m_run;       /* processor is running */
	unsigned int m_cycle;     /* processor is in the midst of an instruction */
	unsigned int m_defer;     /* processor is handling deferred (i.e. indirect) addressing */
	unsigned int m_brk_ctr;   /* break counter */
	unsigned int m_ov;            /* overflow flip-flop */
	unsigned int m_rim;       /* processor is in read-in mode */

	unsigned int m_sbm;       /* processor is in sequence break mode (i.e. interrupts are enabled) */

	unsigned int m_exd;       /* extend mode: processor is in extend mode */
	unsigned int m_exc : 1;       /* extend-mode cycle: current instruction cycle is done in extend mode */
	unsigned int m_ioc;       /* i-o commands: seems to be equivalent to (! ioh) */
	unsigned int m_ioh;       /* i-o halt: processor is executing an Input-Output Transfer wait */
	unsigned int m_ios;       /* i-o synchronizer: set on i-o operation completion */

	/* sequence break system */
	uint16_t m_irq_state;    /* mirrors the state of the interrupt pins */
	uint16_t m_b1;           /* interrupt enable */
	uint16_t m_b2;           /* interrupt pulse request pending - asynchronous with computer operation (set by pulses on irq_state, cleared when interrupt is taken) */
	/*uint16_t m_b3;*/           /* interrupt request pending - synchronous with computer operation (logical or of irq_state and b2???) */
	uint16_t m_b4;           /* interrupt in progress */

	/* additional emulator state variables */
	int m_rim_step;           /* current step in rim execution */
	int m_sbs_request;        /* interrupt request (i.e. (b3 & (~ b4)) && (! sbm)) */
	int m_sbs_level;          /* interrupt request level (first bit in (b3 & (~ b4)) */
	int m_sbs_restore;        /* set when a jump instruction is an interrupt return */
	int m_no_sequence_break;  /* disable sequence break recognition for one cycle */

	/* callbacks for iot instructions (required for any I/O) */
	iot_delegate::array<64> m_extern_iot;
	/* callback called when sc is pulsed: IO devices should reset */
	io_sc_delegate m_io_sc_callback;

	/* 0: no extend support, 1: extend with 15-bit address, 2: extend with 16-bit address */
	int m_extend_support;

	int m_extended_address_mask;  /* 07777 with no extend support, 077777 or 0177777 with extend support */
	int m_address_extension_mask; /* 00000 with no extend support, 070000 or 0170000 with extend support */

	/* 1 to use hardware multiply/divide (MUL, DIV) instead of MUS, DIS */
	int m_hw_mul_div;

	/* 1 for 16-line sequence break system, 0 for default break system */
	int m_type_20_sbs;

	address_space *m_program;
	int m_icount;
	uint32_t m_debugger_temp;

	void field_interrupt();
	void execute_instruction();

	const pdp1_reset_param_t *m_reset_param;
};


DECLARE_DEVICE_TYPE(PDP1, pdp1_device)

#endif // MAME_CPU_PDP1_PDP1_H
