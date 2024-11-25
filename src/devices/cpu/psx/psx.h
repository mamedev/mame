// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation CPU emulator
 *
 * Copyright 2003-2017 smf
 *
 */

#ifndef MAME_CPU_PSX_PSX_H
#define MAME_CPU_PSX_PSX_H

#pragma once

#include "machine/ram.h"
#include "dma.h"
#include "gte.h"
#include "irq.h"
#include "sio.h"
#include "psxdasm.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// interrupts

enum
{
	PSXCPU_IRQ0 = 0,
	PSXCPU_IRQ1,
	PSXCPU_IRQ2,
	PSXCPU_IRQ3,
	PSXCPU_IRQ4,
	PSXCPU_IRQ5,
};

// register enumeration

enum
{
	PSXCPU_PC = 1,
	PSXCPU_DELAYV, PSXCPU_DELAYR,
	PSXCPU_HI, PSXCPU_LO,
	PSXCPU_BIU,
	PSXCPU_R0, PSXCPU_R1,
	PSXCPU_R2, PSXCPU_R3,
	PSXCPU_R4, PSXCPU_R5,
	PSXCPU_R6, PSXCPU_R7,
	PSXCPU_R8, PSXCPU_R9,
	PSXCPU_R10, PSXCPU_R11,
	PSXCPU_R12, PSXCPU_R13,
	PSXCPU_R14, PSXCPU_R15,
	PSXCPU_R16, PSXCPU_R17,
	PSXCPU_R18, PSXCPU_R19,
	PSXCPU_R20, PSXCPU_R21,
	PSXCPU_R22, PSXCPU_R23,
	PSXCPU_R24, PSXCPU_R25,
	PSXCPU_R26, PSXCPU_R27,
	PSXCPU_R28, PSXCPU_R29,
	PSXCPU_R30, PSXCPU_R31,
	PSXCPU_CP0R0, PSXCPU_CP0R1,
	PSXCPU_CP0R2, PSXCPU_CP0R3,
	PSXCPU_CP0R4, PSXCPU_CP0R5,
	PSXCPU_CP0R6, PSXCPU_CP0R7,
	PSXCPU_CP0R8, PSXCPU_CP0R9,
	PSXCPU_CP0R10, PSXCPU_CP0R11,
	PSXCPU_CP0R12, PSXCPU_CP0R13,
	PSXCPU_CP0R14, PSXCPU_CP0R15,
	PSXCPU_CP2DR0, PSXCPU_CP2DR1,
	PSXCPU_CP2DR2, PSXCPU_CP2DR3,
	PSXCPU_CP2DR4, PSXCPU_CP2DR5,
	PSXCPU_CP2DR6, PSXCPU_CP2DR7,
	PSXCPU_CP2DR8, PSXCPU_CP2DR9,
	PSXCPU_CP2DR10, PSXCPU_CP2DR11,
	PSXCPU_CP2DR12, PSXCPU_CP2DR13,
	PSXCPU_CP2DR14, PSXCPU_CP2DR15,
	PSXCPU_CP2DR16, PSXCPU_CP2DR17,
	PSXCPU_CP2DR18, PSXCPU_CP2DR19,
	PSXCPU_CP2DR20, PSXCPU_CP2DR21,
	PSXCPU_CP2DR22, PSXCPU_CP2DR23,
	PSXCPU_CP2DR24, PSXCPU_CP2DR25,
	PSXCPU_CP2DR26, PSXCPU_CP2DR27,
	PSXCPU_CP2DR28, PSXCPU_CP2DR29,
	PSXCPU_CP2DR30, PSXCPU_CP2DR31,
	PSXCPU_CP2CR0, PSXCPU_CP2CR1,
	PSXCPU_CP2CR2, PSXCPU_CP2CR3,
	PSXCPU_CP2CR4, PSXCPU_CP2CR5,
	PSXCPU_CP2CR6, PSXCPU_CP2CR7,
	PSXCPU_CP2CR8, PSXCPU_CP2CR9,
	PSXCPU_CP2CR10, PSXCPU_CP2CR11,
	PSXCPU_CP2CR12, PSXCPU_CP2CR13,
	PSXCPU_CP2CR14, PSXCPU_CP2CR15,
	PSXCPU_CP2CR16, PSXCPU_CP2CR17,
	PSXCPU_CP2CR18, PSXCPU_CP2CR19,
	PSXCPU_CP2CR20, PSXCPU_CP2CR21,
	PSXCPU_CP2CR22, PSXCPU_CP2CR23,
	PSXCPU_CP2CR24, PSXCPU_CP2CR25,
	PSXCPU_CP2CR26, PSXCPU_CP2CR27,
	PSXCPU_CP2CR28, PSXCPU_CP2CR29,
	PSXCPU_CP2CR30, PSXCPU_CP2CR31
};

// delay slot sentinels

enum
{
	PSXCPU_DELAYR_PC = 32,
	PSXCPU_DELAYR_NOTPC = 33
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psxcpu_device

class psxcpu_device : public cpu_device, psxcpu_disassembler::config
{
public:
	// configuration helpers
	auto gpu_read() { return m_gpu_read_handler.bind(); }
	auto gpu_write() { return m_gpu_write_handler.bind(); }
	auto spu_read() { return m_spu_read_handler.bind(); }
	auto spu_write() { return m_spu_write_handler.bind(); }
	auto cd_read() { return m_cd_read_handler.bind(); }
	auto cd_write() { return m_cd_write_handler.bind(); }

	// public interfaces
	void berr_w(uint32_t data);
	uint32_t berr_r();

	uint32_t exp_base();

	void exp_base_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t exp_base_r();

	void exp_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t exp_config_r();

	void ram_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ram_config_r();

	void rom_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rom_config_r();

	void biu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t biu_r();

	void gpu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpu_r(offs_t offset, uint32_t mem_mask = ~0);

	void spu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spu_r(offs_t offset, uint16_t mem_mask = ~0);

	void cd_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t cd_r(offs_t offset, uint8_t mem_mask = ~0);

	void com_delay_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t com_delay_r(offs_t offset, uint32_t mem_mask = ~0);

	void set_disable_rom_berr(bool mode);

	void psxcpu_internal_map(address_map &map) ATTR_COLD;
protected:
	static constexpr unsigned ICACHE_ENTRIES = 0x400;
	static constexpr unsigned DCACHE_ENTRIES = 0x100;

	psxcpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 40; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return ( clocks + 3 ) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles * 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// CPU registers
	uint32_t m_pc;
	uint32_t m_r[ 32 ];
	uint32_t m_cp0r[ 16 ];
	uint32_t m_hi;
	uint32_t m_lo;

	// internal stuff
	uint32_t m_op;

	// address spaces
	const address_space_config m_program_config;
	address_space *m_program;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_instruction;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_data;

	// other internal states
	int m_icount;
	uint32_t m_com_delay;
	uint32_t m_delayv;
	uint32_t m_delayr;
	uint32_t m_berr;
	uint32_t m_biu;
	uint32_t m_icacheTag[ ICACHE_ENTRIES / 4 ];
	uint32_t m_icache[ ICACHE_ENTRIES ];
	uint32_t m_dcache[ DCACHE_ENTRIES ];
	int m_multiplier_operation;
	uint32_t m_multiplier_operand1;
	uint32_t m_multiplier_operand2;
	int m_bus_attached;
	uint32_t m_bad_byte_address_mask;
	uint32_t m_bad_half_address_mask;
	uint32_t m_bad_word_address_mask;
	uint32_t m_exp_base;
	uint32_t m_exp_config;
	uint32_t m_ram_config;
	uint32_t m_rom_config;

	void stop();
	uint32_t cache_readword( uint32_t offset );
	void cache_writeword( uint32_t offset, uint32_t data );
	uint8_t readbyte( uint32_t address );
	uint16_t readhalf( uint32_t address );
	uint32_t readword( uint32_t address );
	uint32_t readword_masked( uint32_t address, uint32_t mask );
	void writeword( uint32_t address, uint32_t data );
	void writeword_masked( uint32_t address, uint32_t data, uint32_t mask );
	uint32_t log_bioscall_parameter( int parm );
	const char *log_bioscall_string( int parm );
	const char *log_bioscall_hex( int parm );
	const char *log_bioscall_char( int parm );
	void log_bioscall();
	void log_syscall();
	void update_memory_handlers();
	void funct_mthi();
	void funct_mtlo();
	void funct_mult();
	void funct_multu();
	void funct_div();
	void funct_divu();
	void multiplier_update();
	uint32_t get_hi();
	uint32_t get_lo();
	int execute_unstoppable_instructions( int executeCop2 );
	void update_address_masks();
	void update_scratchpad();
	void update_ram_config();
	void update_rom_config();
	void update_cop0( int reg );
	void commit_delayed_load();
	void set_pc( unsigned pc );
	void fetch_next_op();
	void advance_pc();
	void load( uint32_t reg, uint32_t value );
	void delayed_load( uint32_t reg, uint32_t value );
	void branch( uint32_t address );
	void conditional_branch( int takeBranch );
	void unconditional_branch();
	void common_exception( int exception, uint32_t romOffset, uint32_t ramOffset );
	void exception( int exception );
	void breakpoint_exception();
	void fetch_bus_error_exception();
	void load_bus_error_exception();
	void store_bus_error_exception();
	void load_bad_address( uint32_t address );
	void store_bad_address( uint32_t address );
	int program_counter_breakpoint();
	int data_address_breakpoint( int dcic_rw, int dcic_status, uint32_t address );
	int load_data_address_breakpoint( uint32_t address );
	int store_data_address_breakpoint( uint32_t address );

	uint32_t get_register_from_pipeline( int reg );
	int cop0_usable();
	void lwc( int cop, int sr_cu );
	void swc( int cop, int sr_cu );
	void bc( int cop, int sr_cu, int condition );

	uint32_t getcp1dr( int reg );
	void setcp1dr( int reg, uint32_t value );
	uint32_t getcp1cr( int reg );
	void setcp1cr( int reg, uint32_t value );
	uint32_t getcp3dr( int reg );
	void setcp3dr( int reg, uint32_t value );
	uint32_t getcp3cr( int reg );
	void setcp3cr( int reg, uint32_t value );

	gte m_gte;

	devcb_read32 m_gpu_read_handler;
	devcb_write32 m_gpu_write_handler;
	devcb_read16 m_spu_read_handler;
	devcb_write16 m_spu_write_handler;
	devcb_read8 m_cd_read_handler;
	devcb_write8 m_cd_write_handler;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	bool m_disable_rom_berr;

private:
	// disassembler interface
	virtual uint32_t pc() override { return m_pc; }
	virtual uint32_t delayr() override { return m_delayr; }
	virtual uint32_t delayv() override { return m_delayv; }
	virtual uint32_t r(int i) override { return m_r[ i ]; }
};

class cxd8530aq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8530aq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8530bq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8530bq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8530cq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8530cq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8661r_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8661r_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8606bq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8606bq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8606cq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8606cq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(CXD8530AQ, cxd8530aq_device)
DECLARE_DEVICE_TYPE(CXD8530BQ, cxd8530bq_device)
DECLARE_DEVICE_TYPE(CXD8530CQ, cxd8530cq_device)
DECLARE_DEVICE_TYPE(CXD8661R,  cxd8661r_device)
DECLARE_DEVICE_TYPE(CXD8606BQ, cxd8606bq_device)
DECLARE_DEVICE_TYPE(CXD8606CQ, cxd8606cq_device)


#endif // MAME_CPU_PSX_PSX_H
