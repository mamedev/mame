// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_CPU_SCMP_SCMP_H
#define MAME_CPU_SCMP_SCMP_H

#pragma once


class scmp_device : public cpu_device
{
public:
	// construction/destruction
	scmp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto flag_out() { return m_flag_out_func.bind(); }
	auto s_out() { return m_sout_func.bind(); }
	auto s_in() { return m_sin_func.bind(); }
	auto sense_a() { return m_sensea_func.bind(); }
	auto sense_b() { return m_senseb_func.bind(); }
	auto halt() { return m_halt_func.bind(); }

protected:
	enum
	{
		SCMP_PC, SCMP_P1, SCMP_P2, SCMP_P3, SCMP_AC, SCMP_ER, SCMP_SR
	};

	scmp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 23+8; } // max opcode + interrupt
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	PAIR    m_PC;
	PAIR    m_P1;
	PAIR    m_P2;
	PAIR    m_P3;
	uint8_t   m_AC;
	uint8_t   m_ER;
	uint8_t   m_SR;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;
	int                 m_icount;

	devcb_write8       m_flag_out_func;
	devcb_write_line   m_sout_func;
	devcb_read_line    m_sin_func;
	devcb_read_line    m_sensea_func;
	devcb_read_line    m_senseb_func;
	devcb_write_line   m_halt_func;

	inline uint16_t ADD12(uint16_t addr, int8_t val);
	inline uint8_t ROP();
	inline uint8_t ARG();
	inline uint8_t RM(uint32_t a);
	inline void WM(uint32_t a, uint8_t v);
	inline void illegal(uint8_t opcode);
	inline PAIR *GET_PTR_REG(int num);
	inline void BIN_ADD(uint8_t val);
	inline void DEC_ADD(uint8_t val);
	inline uint16_t GET_ADDR(uint8_t code);
	void execute_one(int opcode);
	void take_interrupt();
};


class ins8060_device : public scmp_device
{
public:
	// construction/destruction
	ins8060_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
};


DECLARE_DEVICE_TYPE(SCMP, scmp_device)
DECLARE_DEVICE_TYPE(INS8060, ins8060_device)

#endif // MAME_CPU_SCMP_SCMP_H
