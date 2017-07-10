// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_CPU_MB86233_MB86233_H
#define MAME_CPU_MB86233_MB86233_H

#pragma once


#define MCFG_MB86233_FIFO_READ_CB(_devcb) \
	devcb = &mb86233_cpu_device::set_fifo_read_cb(*device, DEVCB_##_devcb);
#define MCFG_MB86233_FIFO_READ_OK_CB(_devcb) \
	devcb = &mb86233_cpu_device::set_fifo_read_ok_cb(*device, DEVCB_##_devcb);
#define MCFG_MB86233_FIFO_WRITE_CB(_devcb) \
	devcb = &mb86233_cpu_device::set_fifo_write_cb(*device, DEVCB_##_devcb);
#define MCFG_MB86233_TABLE_REGION(_region) \
	mb86233_cpu_device::set_tablergn(*device, _region);


class mb86233_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mb86233_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> static devcb_base &set_fifo_read_cb(device_t &device, Object &&cb) { return downcast<mb86233_cpu_device &>(device).m_fifo_read_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_fifo_read_ok_cb(device_t &device, Object &&cb) { return downcast<mb86233_cpu_device &>(device).m_fifo_read_ok_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_fifo_write_cb(device_t &device, Object &&cb) { return downcast<mb86233_cpu_device &>(device).m_fifo_write_cb.set_callback(std::forward<Object>(cb)); }
	static void set_tablergn(device_t &device, const char *tablergn) { downcast<mb86233_cpu_device &>(device).m_tablergn = tablergn; }

protected:
	// register enumeration
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

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 2; }
	virtual uint32_t execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 4; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	union MB86233_REG
	{
		int32_t   i;
		uint32_t  u;
		float   f;
	};

	uint16_t          m_pc;
	MB86233_REG     m_a;
	MB86233_REG     m_b;
	MB86233_REG     m_d;
	MB86233_REG     m_p;

	uint16_t          m_reps;
	uint16_t          m_pcs[4];
	uint8_t           m_pcsp;
	uint32_t          m_eb;
	uint32_t          m_shift;
	uint32_t          m_repcnt;
	uint16_t          m_sr;
	uint8_t           m_fpucontrol;

	uint32_t          m_gpr[16];
	uint32_t          m_extport[0x30];

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
	uint32_t          m_RAM[2 * 0x200];
	uint32_t          *m_ARAM, *m_BRAM;
	uint32_t          *m_Tables;

	void FLAGSF( float v );
	void FLAGSI( uint32_t v );
	int COND( uint32_t cond );
	void ALU( uint32_t alu);
	uint32_t ScaleExp(unsigned int v,int scale);
	uint32_t GETEXTERNAL( uint32_t EB, uint32_t offset );
	void SETEXTERNAL( uint32_t EB, uint32_t offset, uint32_t value );
	uint32_t GETREGS( uint32_t reg, int source );
	void SETREGS( uint32_t reg, uint32_t val );
	uint32_t INDIRECT( uint32_t reg, int source );

};


DECLARE_DEVICE_TYPE(MB86233, mb86233_cpu_device)

#endif // MAME_CPU_MB86233_MB86233_H
