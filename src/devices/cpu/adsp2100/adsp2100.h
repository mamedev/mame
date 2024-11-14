// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    adsp2100.h

    ADSP-21xx series emulator.

***************************************************************************/

#ifndef MAME_CPU_ADSP2100_ADSP2100_H
#define MAME_CPU_ADSP2100_ADSP2100_H

#pragma once


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define ADSP_TRACK_HOTSPOTS     0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// ADSP-2100 IRQs
const int ADSP2100_IRQ0         = 0;        // IRQ0
const int ADSP2100_SPORT1_RX    = 0;        // SPORT1 receive IRQ
const int ADSP2100_IRQ1         = 1;        // IRQ1
const int ADSP2100_SPORT1_TX    = 1;        // SPORT1 transmit IRQ
const int ADSP2100_IRQ2         = 2;        // IRQ2
const int ADSP2100_IRQ3         = 3;        // IRQ3

// ADSP-2101 IRQs
const int ADSP2101_IRQ0         = 0;        // IRQ0
const int ADSP2101_SPORT1_RX    = 0;        // SPORT1 receive IRQ
const int ADSP2101_IRQ1         = 1;        // IRQ1
const int ADSP2101_SPORT1_TX    = 1;        // SPORT1 transmit IRQ
const int ADSP2101_IRQ2         = 2;        // IRQ2
const int ADSP2101_SPORT0_RX    = 3;        // SPORT0 receive IRQ
const int ADSP2101_SPORT0_TX    = 4;        // SPORT0 transmit IRQ
const int ADSP2101_TIMER        = 5;        // internal timer IRQ

// ADSP-2104 IRQs
const int ADSP2104_IRQ0         = 0;        // IRQ0
const int ADSP2104_SPORT1_RX    = 0;        // SPORT1 receive IRQ
const int ADSP2104_IRQ1         = 1;        // IRQ1
const int ADSP2104_SPORT1_TX    = 1;        // SPORT1 transmit IRQ
const int ADSP2104_IRQ2         = 2;        // IRQ2
const int ADSP2104_SPORT0_RX    = 3;        // SPORT0 receive IRQ
const int ADSP2104_SPORT0_TX    = 4;        // SPORT0 transmit IRQ
const int ADSP2104_TIMER        = 5;        // internal timer IRQ

// ADSP-2105 IRQs
const int ADSP2105_IRQ0         = 0;        // IRQ0
const int ADSP2105_SPORT1_RX    = 0;        // SPORT1 receive IRQ
const int ADSP2105_IRQ1         = 1;        // IRQ1
const int ADSP2105_SPORT1_TX    = 1;        // SPORT1 transmit IRQ
const int ADSP2105_IRQ2         = 2;        // IRQ2
const int ADSP2105_TIMER        = 5;        // internal timer IRQ

// ADSP-2115 IRQs
const int ADSP2115_IRQ0         = 0;        // IRQ0
const int ADSP2115_SPORT1_RX    = 0;        // SPORT1 receive IRQ
const int ADSP2115_IRQ1         = 1;        // IRQ1
const int ADSP2115_SPORT1_TX    = 1;        // SPORT1 transmit IRQ
const int ADSP2115_IRQ2         = 2;        // IRQ2
const int ADSP2115_SPORT0_RX    = 3;        // SPORT0 receive IRQ
const int ADSP2115_SPORT0_TX    = 4;        // SPORT0 transmit IRQ
const int ADSP2115_TIMER        = 5;        // internal timer IRQ

// ADSP-2181 IRQs
const int ADSP2181_IRQ0         = 0;        // IRQ0
const int ADSP2181_SPORT1_RX    = 0;        // SPORT1 receive IRQ
const int ADSP2181_IRQ1         = 1;        // IRQ1
const int ADSP2181_SPORT1_TX    = 1;        // SPORT1 transmit IRQ
const int ADSP2181_IRQ2         = 2;        // IRQ2
const int ADSP2181_SPORT0_RX    = 3;        // SPORT0 receive IRQ
const int ADSP2181_SPORT0_TX    = 4;        // SPORT0 transmit IRQ
const int ADSP2181_TIMER        = 5;        // internal timer IRQ
const int ADSP2181_IRQE         = 6;        // IRQE
const int ADSP2181_BDMA         = 7;        // BDMA
const int ADSP2181_IRQL1        = 8;        // IRQL1
const int ADSP2181_IRQL0        = 9;        // IRQL0

// register enumeration
enum
{
	ADSP2100_PC = STATE_GENPC,
	ADSP2100_AX0 = 0,
	ADSP2100_AX1,
	ADSP2100_AY0,
	ADSP2100_AY1,
	ADSP2100_AR,
	ADSP2100_AF,
	ADSP2100_MX0,
	ADSP2100_MX1,
	ADSP2100_MY0,
	ADSP2100_MY1,
	ADSP2100_MR0,
	ADSP2100_MR1,
	ADSP2100_MR2,
	ADSP2100_MF,
	ADSP2100_SI,
	ADSP2100_SE,
	ADSP2100_SB,
	ADSP2100_SR0,
	ADSP2100_SR1,
	ADSP2100_I0,
	ADSP2100_I1,
	ADSP2100_I2,
	ADSP2100_I3,
	ADSP2100_I4,
	ADSP2100_I5,
	ADSP2100_I6,
	ADSP2100_I7,
	ADSP2100_L0,
	ADSP2100_L1,
	ADSP2100_L2,
	ADSP2100_L3,
	ADSP2100_L4,
	ADSP2100_L5,
	ADSP2100_L6,
	ADSP2100_L7,
	ADSP2100_M0,
	ADSP2100_M1,
	ADSP2100_M2,
	ADSP2100_M3,
	ADSP2100_M4,
	ADSP2100_M5,
	ADSP2100_M6,
	ADSP2100_M7,
	ADSP2100_PX,
	ADSP2100_CNTR,
	ADSP2100_ASTAT,
	ADSP2100_SSTAT,
	ADSP2100_MSTAT,
	ADSP2100_PCSP,
	ADSP2100_CNTRSP,
	ADSP2100_STATSP,
	ADSP2100_LOOPSP,
	ADSP2100_IMASK,
	ADSP2100_ICNTL,
	ADSP2100_IRQSTATE0,
	ADSP2100_IRQSTATE1,
	ADSP2100_IRQSTATE2,
	ADSP2100_IRQSTATE3,
	ADSP2100_FLAGIN,
	ADSP2100_FLAGOUT,
	ADSP2100_FL0,
	ADSP2100_FL1,
	ADSP2100_FL2,
	ADSP2100_PMOVLAY,
	ADSP2100_DMOVLAY,
	ADSP2100_AX0_SEC,
	ADSP2100_AX1_SEC,
	ADSP2100_AY0_SEC,
	ADSP2100_AY1_SEC,
	ADSP2100_AR_SEC,
	ADSP2100_AF_SEC,
	ADSP2100_MX0_SEC,
	ADSP2100_MX1_SEC,
	ADSP2100_MY0_SEC,
	ADSP2100_MY1_SEC,
	ADSP2100_MR0_SEC,
	ADSP2100_MR1_SEC,
	ADSP2100_MR2_SEC,
	ADSP2100_MF_SEC,
	ADSP2100_SI_SEC,
	ADSP2100_SE_SEC,
	ADSP2100_SB_SEC,
	ADSP2100_SR0_SEC,
	ADSP2100_SR1_SEC
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adsp21xx_device

class adsp21xx_device : public cpu_device
{
public:
	virtual ~adsp21xx_device();

	// inline configuration helpers
	auto sport_rx() { return m_sport_rx_cb.bind(); }
	auto sport_tx() { return m_sport_tx_cb.bind(); }
	auto timer_fired() { return m_timer_fired_cb.bind(); }
	auto dmovlay() { return m_dmovlay_cb.bind(); }

	// public interfaces
	void load_boot_data(uint8_t *srcdata, uint32_t *dstdata);
	// Returns base address for circular dag
	uint32_t get_ibase(int index) { return m_base[index]; }

protected:
	enum
	{
		CHIP_TYPE_ADSP2100,
		CHIP_TYPE_ADSP2101,
		CHIP_TYPE_ADSP2104,
		CHIP_TYPE_ADSP2105,
		CHIP_TYPE_ADSP2115,
		CHIP_TYPE_ADSP2181
	};

	// construction/destruction
	adsp21xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t chiptype);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// helpers
	void create_tables();
	inline void update_mstat();
	inline uint32_t pc_stack_top();
	inline void set_pc_stack_top(uint32_t top);
	inline void pc_stack_push();
	inline void pc_stack_push_val(uint32_t val);
	inline void pc_stack_pop();
	inline uint32_t pc_stack_pop_val();
	inline uint32_t cntr_stack_top();
	inline void cntr_stack_push();
	inline void cntr_stack_pop();
	inline uint32_t loop_stack_top();
	inline void loop_stack_push(uint32_t value);
	inline void loop_stack_pop();
	inline void stat_stack_push();
	inline void stat_stack_pop();
//  inline int condition(int c);
	int slow_condition();
	inline void modify_address(uint32_t ireg, uint32_t mreg);
	inline void data_write_dag1(uint32_t op, int32_t val);
	inline uint32_t data_read_dag1(uint32_t op);
	inline void data_write_dag2(uint32_t op, int32_t val);
	inline uint32_t data_read_dag2(uint32_t op);
	inline void pgm_write_dag2(uint32_t op, int32_t val);
	inline uint32_t pgm_read_dag2(uint32_t op);
	void alu_op_ar(uint32_t op);
	void alu_op_ar_const(uint32_t op);
	void alu_op_af(uint32_t op);
	void alu_op_af_const(uint32_t op);
	void alu_op_none(uint32_t op);
	void mac_op_mr(uint32_t op);
	void mac_op_mr_xop(uint32_t op);
	void mac_op_mf(uint32_t op);
	void mac_op_mf_xop(uint32_t op);
	void shift_op(uint32_t op);
	void shift_op_imm(uint32_t op);

	// memory access
	inline uint16_t data_read(uint32_t addr);
	inline void data_write(uint32_t addr, uint16_t data);
	inline uint16_t io_read(uint32_t addr);
	inline void io_write(uint32_t addr, uint16_t data);
	inline uint32_t program_read(uint32_t addr);
	inline void program_write(uint32_t addr, uint32_t data);
	inline uint32_t opcode_read();

	// register read/write
	inline void update_i(int which);
	inline void update_l(int which);
	inline void update_dmovlay();
	inline void write_reg0(int regnum, int32_t val);
	inline void write_reg1(int regnum, int32_t val);
	inline void write_reg2(int regnum, int32_t val);
	inline void write_reg3(int regnum, int32_t val);
	inline int32_t read_reg0(int regnum);
	inline int32_t read_reg1(int regnum);
	inline int32_t read_reg2(int regnum);
	inline int32_t read_reg3(int regnum);

	// interrupts
	virtual bool generate_irq(int which, int indx = 0) = 0;
	virtual void check_irqs() = 0;

	// internal state
	static const int PC_STACK_DEPTH     = 16;
	static const int CNTR_STACK_DEPTH   = 4;
	static const int STAT_STACK_DEPTH   = 4;
	static const int LOOP_STACK_DEPTH   = 4;

	// 16-bit registers that can be loaded signed or unsigned
	union adsp_reg16
	{
		uint16_t  u;
		int16_t   s;
	};

	// the SHIFT result register is 32 bits
	union adsp_shift
	{
#ifdef LSB_FIRST
		struct { adsp_reg16 sr0, sr1; } srx;
#else
		struct { adsp_reg16 sr1, sr0; } srx;
#endif
		uint32_t sr;
	};

	// the MAC result register is 40 bits
	union adsp_mac
	{
#ifdef LSB_FIRST
		struct { adsp_reg16 mr0, mr1, mr2, mrzero; } mrx;
		struct { uint32_t mr0, mr1; } mry;
#else
		struct { adsp_reg16 mrzero, mr2, mr1, mr0; } mrx;
		struct { uint32_t mr1, mr0; } mry;
#endif
		uint64_t mr;
	};

	// core registers which are replicated
	struct adsp_core
	{
		// ALU registers
		adsp_reg16  ax0, ax1;
		adsp_reg16  ay0, ay1;
		adsp_reg16  ar;
		adsp_reg16  af;

		// MAC registers
		adsp_reg16  mx0, mx1;
		adsp_reg16  my0, my1;
		adsp_mac    mr;
		adsp_reg16  mf;

		// SHIFT registers
		adsp_reg16  si;
		adsp_reg16  se;
		adsp_reg16  sb;
		adsp_shift  sr;

		// dummy registers
		adsp_reg16  zero;
	};

	// configuration
	const address_space_config      m_program_config;
	const address_space_config      m_data_config;
	uint32_t                          m_chip_type;

	// other CPU registers
	uint32_t              m_pc;
	uint32_t              m_ppc;
	uint32_t              m_loop;
	uint32_t              m_loop_condition;
	uint32_t              m_cntr;

	// status registers
	uint32_t              m_astat;
	uint32_t              m_sstat;
	uint32_t              m_mstat;
	uint32_t              m_mstat_prev;
	uint32_t              m_astat_clear;
	uint32_t              m_idle;

	// live set of core registers
	adsp_core           m_core;

	// memory addressing registers
	uint32_t              m_i[8];
	int32_t               m_m[8];
	uint32_t              m_l[8];
	uint32_t              m_lmask[8];
	uint32_t              m_base[8];
	uint8_t               m_px;
	uint32_t              m_pmovlay; // External Program Space overlay
	uint32_t              m_dmovlay; // External Data Space overlay

	// stacks
	uint32_t              m_loop_stack[LOOP_STACK_DEPTH];
	uint32_t              m_cntr_stack[CNTR_STACK_DEPTH];
	uint32_t              m_pc_stack[PC_STACK_DEPTH];
	uint16_t              m_stat_stack[STAT_STACK_DEPTH][3];
	int32_t               m_pc_sp;
	int32_t               m_cntr_sp;
	int32_t               m_stat_sp;
	int32_t               m_loop_sp;

	// external I/O
	uint8_t               m_flagout;
	uint8_t               m_flagin;
	uint8_t               m_fl0;
	uint8_t               m_fl1;
	uint8_t               m_fl2;
	uint16_t              m_idma_addr;
	uint16_t              m_idma_cache;
	uint8_t               m_idma_offs;

	// interrupt handling
	uint16_t              m_imask;
	uint8_t               m_icntl;
	uint16_t              m_ifc;
	uint8_t               m_irq_state[10];
	uint8_t               m_irq_latch[10];

	// other internal states
	int                 m_icount;
	int                 m_mstat_mask;
	int                 m_imask_mask;

	// register maps
	int16_t *             m_read0_ptr[16];
	uint32_t *            m_read1_ptr[16];
	uint32_t *            m_read2_ptr[16];
	void *              m_alu_xregs[8];
	void *              m_alu_yregs[4];
	void *              m_mac_xregs[8];
	void *              m_mac_yregs[4];
	void *              m_shift_xregs[8];

	// alternate core registers (at end for performance)
	adsp_core           m_alt;

	// address spaces
	memory_access<14, 2, -2, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<14, 2, -2, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<14, 1, -1, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<11, 1, -1, ENDIANNESS_LITTLE>::specific m_io;

	// tables
	uint8_t               m_condition_table[0x1000];
	uint16_t              m_mask_table[0x4000];
	uint16_t              m_reverse_table[0x4000];

	devcb_read32            m_sport_rx_cb;    // callback for serial receive
	devcb_write32           m_sport_tx_cb;    // callback for serial transmit
	devcb_write_line        m_timer_fired_cb; // callback for timer fired
	devcb_write32           m_dmovlay_cb;     // callback for DMOVLAY instruction

	// debugging
#if ADSP_TRACK_HOTSPOTS
	uint32_t              m_pcbucket[0x4000];
#endif

	// flag definitions
	static const int SSFLAG     = 0x80;
	static const int MVFLAG     = 0x40;
	static const int QFLAG      = 0x20;
	static const int SFLAG      = 0x10;
	static const int CFLAG      = 0x08;
	static const int VFLAG      = 0x04;
	static const int NFLAG      = 0x02;
	static const int ZFLAG      = 0x01;
};


// ======================> adsp2100_device

class adsp2100_device : public adsp21xx_device
{
public:
	// construction/destruction
	adsp2100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// interrupts
	virtual bool generate_irq(int which, int indx) override;
	virtual void check_irqs() override;
};


// ======================> adsp2101_device

class adsp2101_device : public adsp21xx_device
{
public:
	// construction/destruction
	adsp2101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	adsp2101_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t chiptype);

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// interrupts
	virtual bool generate_irq(int which, int indx) override;
	virtual void check_irqs() override;
};


// ======================> adsp2181_device

class adsp2181_device : public adsp21xx_device
{
public:
	// construction/destruction
	adsp2181_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// interrupts
	virtual bool generate_irq(int which, int indx) override;
	virtual void check_irqs() override;

	// address spaces
	const address_space_config      m_io_config;

public:
	// public interfaces
	void idma_addr_w(uint16_t data);
	uint16_t idma_addr_r();
	void idma_data_w(uint16_t data);
	uint16_t idma_data_r();
};


// ======================> trivial variants

class adsp2104_device : public adsp2101_device
{
public:
	adsp2104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class adsp2105_device : public adsp2101_device
{
public:
	adsp2105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class adsp2115_device : public adsp2101_device
{
public:
	adsp2115_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
DECLARE_DEVICE_TYPE(ADSP2100, adsp2100_device)
DECLARE_DEVICE_TYPE(ADSP2101, adsp2101_device)
DECLARE_DEVICE_TYPE(ADSP2104, adsp2104_device)
DECLARE_DEVICE_TYPE(ADSP2105, adsp2105_device)
DECLARE_DEVICE_TYPE(ADSP2115, adsp2115_device)
DECLARE_DEVICE_TYPE(ADSP2181, adsp2181_device)


#endif // MAME_CPU_ADSP2100_ADSP2100_H
