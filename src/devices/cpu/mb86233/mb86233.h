// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __MB86233_H__
#define __MB86233_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB86233_PC=1,
	MB86233_A,
	MB86233_B,
	MB86233_D,
	MB86233_P,
	MB86233_REP,
	MB86233_SP,
	MB86233_EB,
	MB86233_SHIFT,
	MB86233_FLAGS,
	MB86233_R0,
	MB86233_R1,
	MB86233_R2,
	MB86233_R3,
	MB86233_R4,
	MB86233_R5,
	MB86233_R6,
	MB86233_R7,
	MB86233_R8,
	MB86233_R9,
	MB86233_R10,
	MB86233_R11,
	MB86233_R12,
	MB86233_R13,
	MB86233_R14,
	MB86233_R15
};


#define MCFG_MB86233_FIFO_READ_CB(_devcb) mb86233_cpu_device::set_fifo_read_cb(*device, DEVCB_##_devcb);
#define MCFG_MB86233_FIFO_READ_OK_CB(_devcb) mb86233_cpu_device::set_fifo_read_ok_cb(*device, DEVCB_##_devcb);
#define MCFG_MB86233_FIFO_WRITE_CB(_devcb) mb86233_cpu_device::set_fifo_write_cb(*device, DEVCB_##_devcb);
#define MCFG_MB86233_TABLE_REGION(_region) mb86233_cpu_device::set_tablergn(*device, _region);


class mb86233_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mb86233_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_fifo_read_cb(device_t &device, _Object object) { return downcast<mb86233_cpu_device &>(device).m_fifo_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_fifo_read_ok_cb(device_t &device, _Object object) { return downcast<mb86233_cpu_device &>(device).m_fifo_read_ok_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_fifo_write_cb(device_t &device, _Object object) { return downcast<mb86233_cpu_device &>(device).m_fifo_write_cb.set_callback(object); }
	static void set_tablergn(device_t &device, const char *tablergn) { downcast<mb86233_cpu_device &>(device).m_tablergn = tablergn; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 2; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config :  nullptr ); }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	union MB86233_REG
	{
		INT32   i;
		UINT32  u;
		float   f;
	};

	UINT16          m_pc;
	MB86233_REG     m_a;
	MB86233_REG     m_b;
	MB86233_REG     m_d;
	MB86233_REG     m_p;

	UINT16          m_reps;
	UINT16          m_pcs[4];
	UINT8           m_pcsp;
	UINT32          m_eb;
	UINT32          m_shift;
	UINT32          m_repcnt;
	UINT16          m_sr;
	UINT8           m_fpucontrol;

	UINT32          m_gpr[16];
	UINT32          m_extport[0x30];

	address_space *m_program;
	direct_read_data *m_direct;
	int m_icount;

	/* FIFO */
	int              m_fifo_wait;
	devcb_read32    m_fifo_read_cb;
	devcb_read_line m_fifo_read_ok_cb;
	devcb_write32   m_fifo_write_cb;
	const char       *m_tablergn;

	/* internal RAM */
	UINT32          m_RAM[2 * 0x200];
	UINT32          *m_ARAM, *m_BRAM;
	UINT32          *m_Tables;

	void FLAGSF( float v );
	void FLAGSI( UINT32 v );
	int COND( UINT32 cond );
	void ALU( UINT32 alu);
	UINT32 ScaleExp(unsigned int v,int scale);
	UINT32 GETEXTERNAL( UINT32 EB, UINT32 offset );
	void SETEXTERNAL( UINT32 EB, UINT32 offset, UINT32 value );
	UINT32 GETREGS( UINT32 reg, int source );
	void SETREGS( UINT32 reg, UINT32 val );
	UINT32 INDIRECT( UINT32 reg, int source );

};


extern const device_type MB86233;

#endif /* __MB86233_H__ */
