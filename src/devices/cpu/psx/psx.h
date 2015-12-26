// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation CPU emulator
 *
 * Copyright 2003-2013 smf
 *
 */

#pragma once

#ifndef __PSXCPU_H__
#define __PSXCPU_H__

#include "emu.h"
#include "machine/ram.h"
#include "dma.h"
#include "gte.h"
#include "irq.h"
#include "sio.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// cache

#define ICACHE_ENTRIES ( 0x400 )
#define DCACHE_ENTRIES ( 0x100 )

// interrupts

#define PSXCPU_IRQ0 ( 0 )
#define PSXCPU_IRQ1 ( 1 )
#define PSXCPU_IRQ2 ( 2 )
#define PSXCPU_IRQ3 ( 3 )
#define PSXCPU_IRQ4 ( 4 )
#define PSXCPU_IRQ5 ( 5 )

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


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PSX_DMA_CHANNEL_READ( cputag, channel, handler ) \
	psxcpu_device::getcpu( *owner, cputag )->subdevice<psxdma_device>("dma")->install_read_handler( channel, handler );

#define MCFG_PSX_DMA_CHANNEL_WRITE( cputag, channel, handler ) \
	psxcpu_device::getcpu( *owner, cputag )->subdevice<psxdma_device>("dma")->install_write_handler( channel, handler );

#define MCFG_PSX_GPU_READ_HANDLER(_devcb) \
	devcb = &psxcpu_device::set_gpu_read_handler(*device, DEVCB_##_devcb);
#define MCFG_PSX_GPU_WRITE_HANDLER(_devcb) \
	devcb = &psxcpu_device::set_gpu_write_handler(*device, DEVCB_##_devcb);

#define MCFG_PSX_SPU_READ_HANDLER(_devcb) \
	devcb = &psxcpu_device::set_spu_read_handler(*device, DEVCB_##_devcb);
#define MCFG_PSX_SPU_WRITE_HANDLER(_devcb) \
	devcb = &psxcpu_device::set_spu_write_handler(*device, DEVCB_##_devcb);

#define MCFG_PSX_CD_READ_HANDLER(_devcb) \
	devcb = &psxcpu_device::set_cd_read_handler(*device, DEVCB_##_devcb);
#define MCFG_PSX_CD_WRITE_HANDLER(_devcb) \
	devcb = &psxcpu_device::set_cd_write_handler(*device, DEVCB_##_devcb);
#define MCFG_PSX_DISABLE_ROM_BERR \
	downcast<psxcpu_device *>(device)->set_disable_rom_berr(true);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class psxcpu_state
{
public:
	virtual ~psxcpu_state() {};

	virtual UINT32 pc() = 0;
	virtual UINT32 delayr() = 0;
	virtual UINT32 delayv() = 0;
	virtual UINT32 r(int i) = 0;
};

// ======================> psxcpu_device

class psxcpu_device : public cpu_device,
	psxcpu_state
{
public:
	// construction/destruction
	virtual ~psxcpu_device() {};

	// static configuration helpers
	template<class _Object> static devcb_base &set_gpu_read_handler(device_t &device, _Object object) { return downcast<psxcpu_device &>(device).m_gpu_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_gpu_write_handler(device_t &device, _Object object) { return downcast<psxcpu_device &>(device).m_gpu_write_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_spu_read_handler(device_t &device, _Object object) { return downcast<psxcpu_device &>(device).m_spu_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_spu_write_handler(device_t &device, _Object object) { return downcast<psxcpu_device &>(device).m_spu_write_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_read_handler(device_t &device, _Object object) { return downcast<psxcpu_device &>(device).m_cd_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_write_handler(device_t &device, _Object object) { return downcast<psxcpu_device &>(device).m_cd_write_handler.set_callback(object); }

	// public interfaces
	DECLARE_WRITE32_MEMBER( berr_w );
	DECLARE_READ32_MEMBER( berr_r );

	UINT32 exp_base();

	DECLARE_WRITE32_MEMBER( exp_base_w );
	DECLARE_READ32_MEMBER( exp_base_r );

	DECLARE_WRITE32_MEMBER( exp_config_w );
	DECLARE_READ32_MEMBER( exp_config_r );

	DECLARE_WRITE32_MEMBER( ram_config_w );
	DECLARE_READ32_MEMBER( ram_config_r );

	DECLARE_WRITE32_MEMBER( rom_config_w );
	DECLARE_READ32_MEMBER( rom_config_r );

	DECLARE_WRITE32_MEMBER( biu_w );
	DECLARE_READ32_MEMBER( biu_r );

	DECLARE_WRITE32_MEMBER( gpu_w );
	DECLARE_READ32_MEMBER( gpu_r );

	DECLARE_WRITE16_MEMBER( spu_w );
	DECLARE_READ16_MEMBER( spu_r );

	DECLARE_WRITE8_MEMBER( cd_w );
	DECLARE_READ8_MEMBER( cd_r );

	DECLARE_WRITE32_MEMBER( com_delay_w );
	DECLARE_READ32_MEMBER( com_delay_r );

	static psxcpu_device *getcpu( device_t &device, const char *cputag );
	void set_disable_rom_berr(bool mode);

protected:
	psxcpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 40; }
	virtual UINT32 execute_input_lines() const override { return 6; }
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return ( clocks + 3 ) / 4; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return cycles * 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// CPU registers
	UINT32 m_pc;
	UINT32 m_r[ 32 ];
	UINT32 m_cp0r[ 16 ];
	UINT32 m_hi;
	UINT32 m_lo;

	// internal stuff
	UINT32 m_op;

	// address spaces
	const address_space_config m_program_config;
	address_space *m_program;
	direct_read_data *m_direct;

	// other internal states
	int m_icount;
	UINT32 m_com_delay;
	UINT32 m_delayv;
	UINT32 m_delayr;
	UINT32 m_berr;
	UINT32 m_biu;
	UINT32 m_icacheTag[ ICACHE_ENTRIES / 4 ];
	UINT32 m_icache[ ICACHE_ENTRIES ];
	UINT32 m_dcache[ DCACHE_ENTRIES ];
	int m_multiplier_operation;
	UINT32 m_multiplier_operand1;
	UINT32 m_multiplier_operand2;
	int m_bus_attached;
	UINT32 m_bad_byte_address_mask;
	UINT32 m_bad_half_address_mask;
	UINT32 m_bad_word_address_mask;
	UINT32 m_exp_base;
	UINT32 m_exp_config;
	UINT32 m_ram_config;
	UINT32 m_rom_config;

	void stop();
	UINT32 cache_readword( UINT32 offset );
	void cache_writeword( UINT32 offset, UINT32 data );
	UINT8 readbyte( UINT32 address );
	UINT16 readhalf( UINT32 address );
	UINT32 readword( UINT32 address );
	UINT32 readword_masked( UINT32 address, UINT32 mask );
	void writeword( UINT32 address, UINT32 data );
	void writeword_masked( UINT32 address, UINT32 data, UINT32 mask );
	UINT32 log_bioscall_parameter( int parm );
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
	UINT32 get_hi();
	UINT32 get_lo();
	int execute_unstoppable_instructions( int executeCop2 );
	void update_address_masks();
	void update_scratchpad();
	void update_ram_config();
	void update_rom_config();
	void update_cop0( int reg );
	void commit_delayed_load();
	void set_pc( unsigned pc );
	void fetch_next_op();
	int advance_pc();
	void load( UINT32 reg, UINT32 value );
	void delayed_load( UINT32 reg, UINT32 value );
	void branch( UINT32 address );
	void conditional_branch( int takeBranch );
	void unconditional_branch();
	void common_exception( int exception, UINT32 romOffset, UINT32 ramOffset );
	void exception( int exception );
	void breakpoint_exception();
	void fetch_bus_error_exception();
	void load_bus_error_exception();
	void store_bus_error_exception();
	void load_bad_address( UINT32 address );
	void store_bad_address( UINT32 address );
	int data_address_breakpoint( int dcic_rw, int dcic_status, UINT32 address );
	int load_data_address_breakpoint( UINT32 address );
	int store_data_address_breakpoint( UINT32 address );

	UINT32 get_register_from_pipeline( int reg );
	int cop0_usable();
	void lwc( int cop, int sr_cu );
	void swc( int cop, int sr_cu );
	void bc( int cop, int sr_cu, int condition );

	UINT32 getcp1dr( int reg );
	void setcp1dr( int reg, UINT32 value );
	UINT32 getcp1cr( int reg );
	void setcp1cr( int reg, UINT32 value );
	UINT32 getcp3dr( int reg );
	void setcp3dr( int reg, UINT32 value );
	UINT32 getcp3cr( int reg );
	void setcp3cr( int reg, UINT32 value );

	gte m_gte;

	devcb_read32 m_gpu_read_handler;
	devcb_write32 m_gpu_write_handler;
	devcb_read16 m_spu_read_handler;
	devcb_write16 m_spu_write_handler;
	devcb_read8 m_cd_read_handler;
	devcb_write8 m_cd_write_handler;
	required_device<ram_device> m_ram;
	memory_region *m_rom;
	bool m_disable_rom_berr;

private:
	// disassembler interface
	virtual UINT32 pc() override { return m_pc; }
	virtual UINT32 delayr() override { return m_delayr; }
	virtual UINT32 delayv() override { return m_delayv; }
	virtual UINT32 r(int i) override { return m_r[ i ]; }
};

class cxd8530aq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8530aq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class cxd8530bq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8530bq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class cxd8530cq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8530cq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class cxd8661r_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8661r_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class cxd8606bq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8606bq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class cxd8606cq_device : public psxcpu_device
{
public:
	// construction/destruction
	cxd8606cq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// device type definition
extern const device_type CXD8530AQ;
extern const device_type CXD8530BQ;
extern const device_type CXD8530CQ;
extern const device_type CXD8661R;
extern const device_type CXD8606BQ;
extern const device_type CXD8606CQ;



#define PSXCPU_DELAYR_PC ( 32 )
#define PSXCPU_DELAYR_NOTPC ( 33 )

#define PSXCPU_BYTE_EXTEND( a ) ( (INT32)(INT8)a )
#define PSXCPU_WORD_EXTEND( a ) ( (INT32)(INT16)a )

#define INS_OP( op ) ( ( op >> 26 ) & 63 )
#define INS_RS( op ) ( ( op >> 21 ) & 31 )
#define INS_RT( op ) ( ( op >> 16 ) & 31 )
#define INS_IMMEDIATE( op ) ( op & 0xffff )
#define INS_TARGET( op ) ( op & 0x3ffffff )
#define INS_RD( op ) ( ( op >> 11 ) & 31 )
#define INS_SHAMT( op ) ( ( op >> 6 ) & 31 )
#define INS_FUNCT( op ) ( op & 63 )
#define INS_CODE( op ) ( ( op >> 6 ) & 0xfffff )
#define INS_CO( op ) ( ( op >> 25 ) & 1 )
#define INS_COFUN( op ) ( op & 0x1ffffff )
#define INS_CF( op ) ( op & 31 )
#define INS_BC( op ) ( ( op >> 16 ) & 1 )
#define INS_RT_REGIMM( op ) ( ( op >> 16 ) & 1 )

#define OP_SPECIAL ( 0 )
#define OP_REGIMM ( 1 )
#define OP_J ( 2 )
#define OP_JAL ( 3 )
#define OP_BEQ ( 4 )
#define OP_BNE ( 5 )
#define OP_BLEZ ( 6 )
#define OP_BGTZ ( 7 )
#define OP_ADDI ( 8 )
#define OP_ADDIU ( 9 )
#define OP_SLTI ( 10 )
#define OP_SLTIU ( 11 )
#define OP_ANDI ( 12 )
#define OP_ORI ( 13 )
#define OP_XORI ( 14 )
#define OP_LUI ( 15 )
#define OP_COP0 ( 16 )
#define OP_COP1 ( 17 )
#define OP_COP2 ( 18 )
#define OP_COP3 ( 19 )
#define OP_LB ( 32 )
#define OP_LH ( 33 )
#define OP_LWL ( 34 )
#define OP_LW ( 35 )
#define OP_LBU ( 36 )
#define OP_LHU ( 37 )
#define OP_LWR ( 38 )
#define OP_SB ( 40 )
#define OP_SH ( 41 )
#define OP_SWL ( 42 )
#define OP_SW ( 43 )
#define OP_SWR ( 46 )
#define OP_LWC0 ( 48 )
#define OP_LWC1 ( 49 )
#define OP_LWC2 ( 50 )
#define OP_LWC3 ( 51 )
#define OP_SWC0 ( 56 )
#define OP_SWC1 ( 57 )
#define OP_SWC2 ( 58 )
#define OP_SWC3 ( 59 )

/* OP_SPECIAL */
#define FUNCT_SLL ( 0 )
#define FUNCT_SRL ( 2 )
#define FUNCT_SRA ( 3 )
#define FUNCT_SLLV ( 4 )
#define FUNCT_SRLV ( 6 )
#define FUNCT_SRAV ( 7 )
#define FUNCT_JR ( 8 )
#define FUNCT_JALR ( 9 )
#define FUNCT_SYSCALL ( 12 )
#define FUNCT_BREAK ( 13 )
#define FUNCT_MFHI ( 16 )
#define FUNCT_MTHI ( 17 )
#define FUNCT_MFLO ( 18 )
#define FUNCT_MTLO ( 19 )
#define FUNCT_MULT ( 24 )
#define FUNCT_MULTU ( 25 )
#define FUNCT_DIV ( 26 )
#define FUNCT_DIVU ( 27 )
#define FUNCT_ADD ( 32 )
#define FUNCT_ADDU ( 33 )
#define FUNCT_SUB ( 34 )
#define FUNCT_SUBU ( 35 )
#define FUNCT_AND ( 36 )
#define FUNCT_OR ( 37 )
#define FUNCT_XOR ( 38 )
#define FUNCT_NOR ( 39 )
#define FUNCT_SLT ( 42 )
#define FUNCT_SLTU ( 43 )

/* OP_REGIMM */
#define RT_BLTZ ( 0 )
#define RT_BGEZ ( 1 )
#define RT_BLTZAL ( 16 )
#define RT_BGEZAL ( 17 )

/* OP_COP0/OP_COP1/OP_COP2 */
#define RS_MFC ( 0 )
#define RS_CFC ( 2 )
#define RS_MTC ( 4 )
#define RS_CTC ( 6 )
#define RS_BC ( 8 )
#define RS_BC_ALT ( 12 )

/* BC_BC */
#define BC_BCF ( 0 )
#define BC_BCT ( 1 )

/* OP_COP0 */
#define CF_TLBR ( 1 )
#define CF_TLBWI ( 2 )
#define CF_TLBWR ( 6 )
#define CF_TLBP ( 8 )
#define CF_RFE ( 16 )

extern unsigned DasmPSXCPU( psxcpu_state *state, char *buffer, UINT32 pc, const UINT8 *opram );

#endif /* __PSXCPU_H__ */
