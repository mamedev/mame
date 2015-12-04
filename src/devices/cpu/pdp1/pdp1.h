// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#pragma once

#ifndef __PDP1_H__
#define __PDP1_H__



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


typedef void (*pdp1_extern_iot_func)(device_t *device, int op2, int nac, int mb, int *io, int ac);
typedef void (*pdp1_read_binary_word_func)(device_t *device);
typedef void (*pdp1_io_sc_func)(device_t *device);


struct pdp1_reset_param_t
{
	/* callbacks for iot instructions (required for any I/O) */
	pdp1_extern_iot_func extern_iot[64];
	/* read a word from the perforated tape reader (required for read-in mode) */
	pdp1_read_binary_word_func read_binary_word;
	/* callback called when sc is pulsed: IO devices should reset */
	pdp1_io_sc_func io_sc_callback;

	/* 0: no extend support, 1: extend with 15-bit address, 2: extend with 16-bit address */
	int extend_support;
	/* 1 to use hardware multiply/divide (MUL, DIV) instead of MUS, DIS */
	int hw_mul_div;
	/* 0: standard sequence break system 1: type 20 sequence break system */
	int type_20_sbs;
};

#define IOT_NO_COMPLETION_PULSE -1


#define AND 001
#define IOR 002
#define XOR 003
#define XCT 004
#define CALJDA 007
#define LAC 010
#define LIO 011
#define DAC 012
#define DAP 013
#define DIP 014
#define DIO 015
#define DZM 016
#define ADD 020
#define SUB 021
#define IDX 022
#define ISP 023
#define SAD 024
#define SAS 025
#define MUS_MUL 026
#define DIS_DIV 027
#define JMP 030
#define JSP 031
#define SKP 032
#define SFT 033
#define LAW 034
#define IOT 035
#define OPR 037


class pdp1_device : public cpu_device
					, public pdp1_reset_param_t
{
public:
	// construction/destruction
	pdp1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void pulse_start_clear();
	void io_complete() { m_ios = 1; }
	void pdp1_null_iot(int op2, int nac, int mb, int *io, int ac);
	void pdp1_lem_eem_iot(int op2, int nac, int mb, int *io, int ac);
	void pdp1_sbs_iot(int op2, int nac, int mb, int *io, int ac);
	void pdp1_type_20_sbs_iot(int op2, int nac, int mb, int *io, int ac);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 31; }
	virtual UINT32 execute_input_lines() const { return 16; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;

	/* processor registers */
	UINT32 m_pc;      /* program counter (12, 15 or 16 bits) */
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
	UINT16 m_irq_state;    /* mirrors the state of the interrupt pins */
	UINT16 m_b1;           /* interrupt enable */
	UINT16 m_b2;           /* interrupt pulse request pending - asynchronous with computer operation (set by pulses on irq_state, cleared when interrupt is taken) */
	/*UINT16 m_b3;*/           /* interrupt request pending - synchronous with computer operation (logical or of irq_state and b2???) */
	UINT16 m_b4;           /* interrupt in progress */

	/* additional emulator state variables */
	int m_rim_step;           /* current step in rim execution */
	int m_sbs_request;        /* interrupt request (i.e. (b3 & (~ b4)) && (! sbm)) */
	int m_sbs_level;          /* interrupt request level (first bit in (b3 & (~ b4)) */
	int m_sbs_restore;        /* set when a jump instruction is an interrupt return */
	int m_no_sequence_break;  /* disable sequence break recognition for one cycle */

	/* callbacks for iot instructions (required for any I/O) */
	pdp1_extern_iot_func m_extern_iot[64];
	/* read a word from the perforated tape reader (required for read-in mode) */
	pdp1_read_binary_word_func m_read_binary_word;
	/* callback called when sc is pulsed: IO devices should reset */
	pdp1_io_sc_func m_io_sc_callback;

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
	UINT32 m_debugger_temp;

	void field_interrupt();
	void execute_instruction();

};


extern const device_type PDP1;


#endif /* __PDP1_H__ */
