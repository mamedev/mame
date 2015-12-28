// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
#pragma once

#ifndef __TX0_H__
#define __TX0_H__



/* register ids for tx0_get_reg/tx0_set_reg */
enum
{
	TX0_MBR=1, TX0_AC, TX0_MAR, TX0_PC, TX0_IR, TX0_LR, TX0_XR, TX0_PF,
	TX0_TBR, TX0_TAC,
	TX0_TSS00, TX0_TSS01, TX0_TSS02, TX0_TSS03, TX0_TSS04, TX0_TSS05, TX0_TSS06, TX0_TSS07,
	TX0_TSS10, TX0_TSS11, TX0_TSS12, TX0_TSS13, TX0_TSS14, TX0_TSS15, TX0_TSS16, TX0_TSS17,
	TX0_CM_SEL, TX0_LR_SEL, TX0_GBL_CM_SEL,
	TX0_STOP_CYC0, TX0_STOP_CYC1,
	TX0_RUN, TX0_RIM,
	TX0_CYCLE, TX0_IOH, TX0_IOS
};


#define MCFG_TX0_CONFIG(_cpy_devcb, _r1l_devcb, _dis_devcb, _r3l_devcb, _prt_devcb, _rsv_devcb, _p6h_devcb, _p7h_devcb, _sel_devcb, _res_devcb) \
	tx0_device::set_cpy_cb(*device, DEVCB_##_cpy_devcb); \
	tx0_device::set_r1l_cb(*device, DEVCB_##_r1l_devcb); \
	tx0_device::set_dis_cb(*device, DEVCB_##_dis_devcb); \
	tx0_device::set_r3l_cb(*device, DEVCB_##_r3l_devcb); \
	tx0_device::set_prt_cb(*device, DEVCB_##_prt_devcb); \
	tx0_device::set_rsv_cb(*device, DEVCB_##_rsv_devcb); \
	tx0_device::set_p6h_cb(*device, DEVCB_##_p6h_devcb); \
	tx0_device::set_p7h_cb(*device, DEVCB_##_p7h_devcb); \
	tx0_device::set_sel_cb(*device, DEVCB_##_sel_devcb); \
	tx0_device::set_res_cb(*device, DEVCB_##_res_devcb);


class tx0_device : public cpu_device
{
public:
	// construction/destruction
	tx0_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int addr_bits, int address_mask, int ir_mask);

	// static configuration helpers
	template<class _Object> static devcb_base &set_cpy_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_cpy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_r1l_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_r1l_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dis_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_dis_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_r3l_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_r3l_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_prt_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_prt_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rsv_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_rsv_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p6h_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_p6h_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p7h_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_p7h_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_sel_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_sel_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_res_cb(device_t &device, _Object object) { return downcast<tx0_device &>(device).m_io_reset_callback.set_callback(object); }

	void pulse_reset();
	void io_complete();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 3; }

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }

protected:
	address_space_config m_program_config;

	/* processor registers */
	int m_mbr;        /* memory buffer register (18 bits) */
	int m_ac;         /* accumulator (18 bits) */
	int m_mar;        /* memory address register (16 (64kW) or 13 (8kW) bits) */
	int m_pc;         /* program counter (16 (64kW) or 13 (8kW) bits) */
	int m_ir;         /* instruction register (2 (64kW) or 5 (8kW) bits) */
	int m_lr;         /* live register (18 bits) */
	int m_xr;         /* index register (14 bits) (8kW only) */
	int m_pf;         /* program flags (6 bits expandable to 10) (8kW only) */

	/* operator panel switches */
	int m_tbr;        /* toggle switch buffer register (18 bits) */
	int m_tac;        /* toggle switch accumulator (18 bits) */
	int m_tss[16];    /* toggle switch storage (18 bits * 16) */
	UINT16 m_cm_sel;   /* individual cm select (1 bit * 16) */
	UINT16 m_lr_sel;   /* individual lr select (1 bit * 16) */
	unsigned int m_gbl_cm_sel;/* global cm select (1 bit) */
	unsigned int m_stop_cyc0; /* stop on cycle 0 */
	unsigned int m_stop_cyc1; /* stop on cycle 1 */

	/* processor state flip-flops */
	unsigned int m_run;       /* processor is running */
	unsigned int m_rim;       /* processor is in read-in mode */
	unsigned int m_cycle;     /* 0 -> fetch */
								/* 1 -> execute (except for taken branches) */
								/* 2 -> extra execute cycle for SXA and ADO */

	unsigned int m_ioh;       /* i-o halt: processor is executing an Input-Output Transfer wait */
	unsigned int m_ios;       /* i-o synchronizer: set on i-o operation completion */

	/* additional emulator state variables */
	int m_rim_step;           /* current step in rim execution */

	int m_address_mask;       /* address mask */
	int m_ir_mask;            /* IR mask */

	int m_icount;

	address_space *m_program;

	/* 8 standard I/O handlers:
	    0: cpy (8kW only)
	    1: r1l
	    2: dis
	    3: r3l
	    4: prt
	    5: reserved (for unimplemented typ instruction?)
	    6: p6h
	    7: p7h */
	devcb_write_line m_cpy_handler;
	devcb_write_line m_r1l_handler;
	devcb_write_line m_dis_handler;
	devcb_write_line m_r3l_handler;
	devcb_write_line m_prt_handler;
	devcb_write_line m_rsv_handler;
	devcb_write_line m_p6h_handler;
	devcb_write_line m_p7h_handler;
	/* select instruction handler */
	devcb_write_line m_sel_handler;
	/* callback called when reset line is pulsed: IO devices should reset */
	devcb_write_line m_io_reset_callback;

	int tx0_read(offs_t address);
	void tx0_write(offs_t address, int data);
	void call_io_handler(int io_handler);
	void indexed_address_eval();
};


class tx0_8kw_device : public tx0_device
{
public:
	// construction/destruction
	tx0_8kw_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

protected:
	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	void execute_instruction_8kw();
};


class tx0_64kw_device : public tx0_device
{
public:
	// construction/destruction
	tx0_64kw_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

protected:
	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	void execute_instruction_64kw();
};


extern const device_type TX0_64KW;
extern const device_type TX0_8KW;

#endif /* __TX0_H__ */
