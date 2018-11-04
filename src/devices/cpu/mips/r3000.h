// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    r3000.h
    Interface file for the portable MIPS R3000 emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_MIPS_R3000_H
#define MAME_CPU_MIPS_R3000_H

#pragma once


/***************************************************************************
    INTERFACE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_R3000_ENDIANNESS(_endianness) \
	downcast<r3000_device_base &>(*device).set_endianness(_endianness);

#define MCFG_R3000_BRCOND0_INPUT(_devcb) \
	downcast<r3000_device_base &>(*device).set_brcond0_input(DEVCB_##_devcb);

#define MCFG_R3000_BRCOND1_INPUT(_devcb) \
	downcast<r3000_device_base &>(*device).set_brcond1_input(DEVCB_##_devcb);

#define MCFG_R3000_BRCOND2_INPUT(_devcb) \
	downcast<r3000_device_base &>(*device).set_brcond2_input(DEVCB_##_devcb);

#define MCFG_R3000_BRCOND3_INPUT(_devcb) \
	downcast<r3000_device_base &>(*device).set_brcond3_input(DEVCB_##_devcb);


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	R3000_PC=1,R3000_SR,
	R3000_R0,R3000_R1,R3000_R2,R3000_R3,R3000_R4,R3000_R5,R3000_R6,R3000_R7,
	R3000_R8,R3000_R9,R3000_R10,R3000_R11,R3000_R12,R3000_R13,R3000_R14,R3000_R15,
	R3000_R16,R3000_R17,R3000_R18,R3000_R19,R3000_R20,R3000_R21,R3000_R22,R3000_R23,
	R3000_R24,R3000_R25,R3000_R26,R3000_R27,R3000_R28,R3000_R29,R3000_R30,R3000_R31
};


/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define R3000_IRQ0      0       /* IRQ0 */
#define R3000_IRQ1      1       /* IRQ1 */
#define R3000_IRQ2      2       /* IRQ2 */
#define R3000_IRQ3      3       /* IRQ3 */
#define R3000_IRQ4      4       /* IRQ4 */
#define R3000_IRQ5      5       /* IRQ5 */


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> r3000_device_base

class r3000_device_base : public cpu_device
{
public:
	// construction/destruction
	virtual ~r3000_device_base();

	// inline configuration helpers
	void set_endianness(endianness_t endianness) { m_endianness = endianness; }
	void set_fpurev(uint32_t revision) { m_hasfpu = true; m_fpurev = revision; }

	template <class Object> devcb_base &set_brcond0_input(Object &&cb) { return m_in_brcond0.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_brcond1_input(Object &&cb) { return m_in_brcond1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_brcond2_input(Object &&cb) { return m_in_brcond2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_brcond3_input(Object &&cb) { return m_in_brcond3.set_callback(std::forward<Object>(cb)); }
	auto in_brcond0() { return m_in_brcond0.bind(); }
	auto in_brcond1() { return m_in_brcond1.bind(); }
	auto in_brcond2() { return m_in_brcond2.bind(); }
	auto in_brcond3() { return m_in_brcond3.bind(); }

protected:
	r3000_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t cpurev, size_t icache_size, size_t dcache_size);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
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

	// memory accessors
	struct r3000_data_accessors
	{
		uint8_t   (r3000_device_base::*m_read_byte)(offs_t byteaddress);
		uint16_t  (r3000_device_base::*m_read_word)(offs_t byteaddress);
		uint32_t  (r3000_device_base::*m_read_dword)(offs_t byteaddress);
		void    (r3000_device_base::*m_write_byte)(offs_t byteaddress, uint8_t data);
		void    (r3000_device_base::*m_write_word)(offs_t byteaddress, uint16_t data);
		void    (r3000_device_base::*m_write_dword)(offs_t byteaddress, uint32_t data);
	};

	uint32_t readop(offs_t pc);
	uint8_t readmem(offs_t offset);
	uint16_t readmem_word(offs_t offset);
	uint32_t readmem_dword(offs_t offset);
	void writemem(offs_t offset, uint8_t data);
	void writemem_word(offs_t offset, uint16_t data);
	void writemem_dword(offs_t offset, uint32_t data);

	uint8_t readcache_be(offs_t offset);
	uint16_t readcache_be_word(offs_t offset);
	uint32_t readcache_be_dword(offs_t offset);
	void writecache_be(offs_t offset, uint8_t data);
	void writecache_be_word(offs_t offset, uint16_t data);
	void writecache_be_dword(offs_t offset, uint32_t data);

	uint8_t readcache_le(offs_t offset);
	uint16_t readcache_le_word(offs_t offset);
	uint32_t readcache_le_dword(offs_t offset);
	void writecache_le(offs_t offset, uint8_t data);
	void writecache_le_word(offs_t offset, uint16_t data);
	void writecache_le_dword(offs_t offset, uint32_t data);

	// interrupts
	void generate_exception(int exception, bool backup);
	void check_irqs();
	void set_irq_line(int irqline, int state);
	void invalid_instruction();

	// instructions
	uint32_t get_cop0_reg(int idx);
	void set_cop0_reg(int idx, uint32_t val);
	uint32_t get_cop0_creg(int idx);
	void set_cop0_creg(int idx, uint32_t val);
	void handle_cop0();

	uint32_t get_cop1_reg(int idx);
	void set_cop1_reg(int idx, uint32_t val);
	uint32_t get_cop1_creg(int idx);
	void set_cop1_creg(int idx, uint32_t val);
	void handle_cop1();

	uint32_t get_cop2_reg(int idx);
	void set_cop2_reg(int idx, uint32_t val);
	uint32_t get_cop2_creg(int idx);
	void set_cop2_creg(int idx, uint32_t val);
	void handle_cop2();

	uint32_t get_cop3_reg(int idx);
	void set_cop3_reg(int idx, uint32_t val);
	uint32_t get_cop3_creg(int idx);
	void set_cop3_creg(int idx, uint32_t val);
	void handle_cop3();

	// complex opcodes
	void lwl_be();
	void lwr_be();
	void swl_be();
	void swr_be();

	void lwl_le();
	void lwr_le();
	void swl_le();
	void swr_le();

	// address spaces
	const address_space_config m_program_config_be;
	const address_space_config m_program_config_le;
	address_space *m_program;
	std::function<u32 (offs_t)> m_pr32;
	std::function<const void * (offs_t)> m_prptr;

	// configuration
	uint32_t        m_cpurev;
	bool            m_hasfpu;
	uint32_t        m_fpurev;
	endianness_t    m_endianness;

	// core registers
	uint32_t      m_pc;
	uint32_t      m_nextpc;
	uint32_t      m_hi;
	uint32_t      m_lo;
	uint32_t      m_r[32];

	// COP registers
	uint32_t      m_cpr[4][32];
	uint32_t      m_ccr[4][32];

	// internal stuff
	uint32_t      m_ppc;
	uint32_t      m_op;
	int         m_icount;
	int         m_interrupt_cycles;

	// endian-dependent load/store
	void        (r3000_device_base::*m_lwl)();
	void        (r3000_device_base::*m_lwr)();
	void        (r3000_device_base::*m_swl)();
	void        (r3000_device_base::*m_swr)();

	// memory accesses
	r3000_data_accessors *m_cur;
	r3000_data_accessors m_memory_hand;
	r3000_data_accessors m_cache_hand;

	// cache memory
	uint32_t *    m_cache;
	std::vector<uint32_t> m_icache;
	std::vector<uint32_t> m_dcache;
	size_t      m_cache_size;
	size_t const m_icache_size;
	size_t const m_dcache_size;

	// I/O
	devcb_read_line    m_in_brcond0;
	devcb_read_line    m_in_brcond1;
	devcb_read_line    m_in_brcond2;
	devcb_read_line    m_in_brcond3;
};

// ======================> r2000_device

class r2000_device : public r3000_device_base
{
public:
	r2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, size_t icache_size = 0, size_t dcache_size = 0);
};


// ======================> r2000a_device

class r2000a_device : public r3000_device_base
{
public:
	r2000a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, size_t icache_size = 0, size_t dcache_size = 0);
};


// ======================> r3000_device

class r3000_device : public r3000_device_base
{
public:
	r3000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, size_t icache_size = 0, size_t dcache_size = 0);
};


// ======================> r3000a_device

class r3000a_device : public r3000_device_base
{
public:
	r3000a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, size_t icache_size = 0, size_t dcache_size = 0);
};


// ======================> r3041_device

class r3041_device : public r3000_device_base
{
public:
	r3041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> r3051_device

class r3051_device : public r3000_device_base
{
public:
	r3051_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> r3052_device

class r3052_device : public r3000_device_base
{
public:
	r3052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> r3071_device

class r3071_device : public r3000_device_base
{
public:
	r3071_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, size_t icache_size = 16384, size_t dcache_size = 4096);
};


// ======================> r3081_device

class r3081_device : public r3000_device_base
{
public:
	r3081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, size_t icache_size = 16384, size_t dcache_size = 4096);
};


// ======================> iop_device

class iop_device : public r3000_device_base
{
public:
	iop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition

DECLARE_DEVICE_TYPE(R2000,       r2000_device)
DECLARE_DEVICE_TYPE(R2000A,      r2000a_device)
DECLARE_DEVICE_TYPE(R3000,       r3000_device)
DECLARE_DEVICE_TYPE(R3000A,      r3000a_device)
DECLARE_DEVICE_TYPE(R3041,       r3041_device)
DECLARE_DEVICE_TYPE(R3051,       r3051_device)
DECLARE_DEVICE_TYPE(R3052,       r3052_device)
DECLARE_DEVICE_TYPE(R3071,       r3071_device)
DECLARE_DEVICE_TYPE(R3081,       r3081_device)
DECLARE_DEVICE_TYPE(SONYPS2_IOP, iop_device)

#endif // MAME_CPU_MIPS_R3000_H
