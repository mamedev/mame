// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    asap.h

    Core implementation for the portable ASAP emulator.
    ASAP = Atari Simplified Architecture Processor

***************************************************************************/

#ifndef MAME_CPU_ASAP_ASAP_H
#define MAME_CPU_ASAP_ASAP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> asap_device

class asap_device : public cpu_device
{
public:
	// construction/destruction
	asap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// public interfaces

protected:
	enum
	{
		ASAP_PC = 1,
		ASAP_PS,
		ASAP_R0,
		ASAP_R1,
		ASAP_R2,
		ASAP_R3,
		ASAP_R4,
		ASAP_R5,
		ASAP_R6,
		ASAP_R7,
		ASAP_R8,
		ASAP_R9,
		ASAP_R10,
		ASAP_R11,
		ASAP_R12,
		ASAP_R13,
		ASAP_R14,
		ASAP_R15,
		ASAP_R16,
		ASAP_R17,
		ASAP_R18,
		ASAP_R19,
		ASAP_R20,
		ASAP_R21,
		ASAP_R22,
		ASAP_R23,
		ASAP_R24,
		ASAP_R25,
		ASAP_R26,
		ASAP_R27,
		ASAP_R28,
		ASAP_R29,
		ASAP_R30,
		ASAP_R31
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
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

	// helpers
	inline uint32_t readop(offs_t pc);
	inline uint8_t readbyte(offs_t address);
	inline uint16_t readword(offs_t address);
	inline uint32_t readlong(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);
	inline void writeword(offs_t address, uint16_t data);
	inline void writelong(offs_t address, uint32_t data);
	inline void generate_exception(int exception);
	inline void check_irqs();
	inline void fetch_instruction();
	inline void fetch_instruction_debug();
	inline void execute_instruction();

	// condition handlers
	void bsp();
	void bmz();
	void bgt();
	void ble();
	void bge();
	void blt();
	void bhi();
	void bls();
	void bcc();
	void bcs();
	void bpl();
	void bmi();
	void bne();
	void beq();
	void bvc();
	void bvs();

	// opcode handlers
	void noop();
	void trap0();
	void bsr();
	void bsr_0();
	void lea();
	void lea_c();
	void lea_c0();
	void leah();
	void leah_c();
	void leah_c0();
	void subr();
	void subr_c();
	void subr_c0();
	void xor_();
	void xor_c();
	void xor_c0();
	void xorn();
	void xorn_c();
	void xorn_c0();
	void add();
	void add_c();
	void add_c0();
	void sub();
	void sub_c();
	void sub_c0();
	void addc();
	void addc_c();
	void addc_c0();
	void subc();
	void subc_c();
	void subc_c0();
	void and_();
	void and_c();
	void and_c0();
	void andn();
	void andn_c();
	void andn_c0();
	void or_();
	void or_c();
	void or_c0();
	void orn();
	void orn_c();
	void orn_c0();
	void ld();
	void ld_0();
	void ld_c();
	void ld_c0();
	void ldh();
	void ldh_0();
	void ldh_c();
	void ldh_c0();
	void lduh();
	void lduh_0();
	void lduh_c();
	void lduh_c0();
	void sth();
	void sth_0();
	void sth_c();
	void sth_c0();
	void st();
	void st_0();
	void st_c();
	void st_c0();
	void ldb();
	void ldb_0();
	void ldb_c();
	void ldb_c0();
	void ldub();
	void ldub_0();
	void ldub_c();
	void ldub_c0();
	void stb();
	void stb_0();
	void stb_c();
	void stb_c0();
	void ashr();
	void ashr_c();
	void ashr_c0();
	void lshr();
	void lshr_c();
	void lshr_c0();
	void ashl();
	void ashl_c();
	void ashl_c0();
	void rotl();
	void rotl_c();
	void rotl_c0();
	void getps();
	void putps();
	void jsr();
	void jsr_0();
	void jsr_c();
	void jsr_c0();
	void trapf();

	// internal state
	const address_space_config      m_program_config;
	uint32_t              m_pc;

	// expanded flags
	uint32_t              m_pflag;
	uint32_t              m_iflag;
	uint32_t              m_cflag;
	uint32_t              m_vflag;
	uint32_t              m_znflag;
	uint32_t              m_flagsio;

	// internal stuff
	uint32_t              m_op;
	uint32_t              m_ppc;
	uint32_t              m_nextpc;
	uint8_t               m_irq_state;
	int                 m_icount;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;

	// src2val table, registers are at the end
	uint32_t              m_src2val[65536];

	// opcode/condition tables
	typedef void (asap_device::*ophandler)();

	ophandler           m_opcode[32 * 32 * 2];

	static const ophandler s_opcodetable[32][4];
	static const ophandler s_conditiontable[16];
};



//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// input lines
enum
{
	ASAP_IRQ0
};



// device type definition
DECLARE_DEVICE_TYPE(ASAP, asap_device)

#endif // MAME_CPU_ASAP_ASAP_H
