// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    r3000.h
    Interface file for the portable MIPS R3000 emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef __R3000_H__
#define __R3000_H__


/***************************************************************************
    INTERFACE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_R3000_ENDIANNESS(_endianness) \
	r3000_device::static_set_endianness(*device, _endianness);

#define MCFG_R3000_BRCOND0_INPUT(_devcb) \
	devcb = &r3000_device::static_set_brcond0_input(*device, DEVCB_##_devcb);

#define MCFG_R3000_BRCOND1_INPUT(_devcb) \
	devcb = &r3000_device::static_set_brcond1_input(*device, DEVCB_##_devcb);

#define MCFG_R3000_BRCOND2_INPUT(_devcb) \
	devcb = &r3000_device::static_set_brcond2_input(*device, DEVCB_##_devcb);

#define MCFG_R3000_BRCOND3_INPUT(_devcb) \
	devcb = &r3000_device::static_set_brcond3_input(*device, DEVCB_##_devcb);


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

// ======================> r3000_device

class r3000_device : public cpu_device
{
protected:
	enum chip_type
	{
		CHIP_TYPE_R3041,
		CHIP_TYPE_R3051,
		CHIP_TYPE_R3052,
		CHIP_TYPE_R3071,
		CHIP_TYPE_R3081
	};

	// construction/destruction
	r3000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, chip_type chiptype, const char *shortname, const char *source);
	virtual ~r3000_device();

public:
	// inline configuration helpers
	static void static_set_endianness(device_t &device, endianness_t endianness)
	{
		downcast<r3000_device &>(device).m_endianness = endianness;
	}

	template<class _Object> static devcb_base &static_set_brcond0_input(device_t &device, _Object object)
	{
		return downcast<r3000_device &>(device).m_in_brcond0.set_callback(object);
	}

	template<class _Object> static devcb_base &static_set_brcond1_input(device_t &device, _Object object)
	{
		return downcast<r3000_device &>(device).m_in_brcond1.set_callback(object);
	}

	template<class _Object> static devcb_base &static_set_brcond2_input(device_t &device, _Object object)
	{
		return downcast<r3000_device &>(device).m_in_brcond2.set_callback(object);
	}

	template<class _Object> static devcb_base &static_set_brcond3_input(device_t &device, _Object object)
	{
		return downcast<r3000_device &>(device).m_in_brcond3.set_callback(object);
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// memory accessors
	struct r3000_data_accessors
	{
		UINT8   (r3000_device::*m_read_byte)(offs_t byteaddress);
		UINT16  (r3000_device::*m_read_word)(offs_t byteaddress);
		UINT32  (r3000_device::*m_read_dword)(offs_t byteaddress);
		void    (r3000_device::*m_write_byte)(offs_t byteaddress, UINT8 data);
		void    (r3000_device::*m_write_word)(offs_t byteaddress, UINT16 data);
		void    (r3000_device::*m_write_dword)(offs_t byteaddress, UINT32 data);
	};

	UINT32 readop(offs_t pc);
	UINT8 readmem(offs_t offset);
	UINT16 readmem_word(offs_t offset);
	UINT32 readmem_dword(offs_t offset);
	void writemem(offs_t offset, UINT8 data);
	void writemem_word(offs_t offset, UINT16 data);
	void writemem_dword(offs_t offset, UINT32 data);

	UINT8 readcache_be(offs_t offset);
	UINT16 readcache_be_word(offs_t offset);
	UINT32 readcache_be_dword(offs_t offset);
	void writecache_be(offs_t offset, UINT8 data);
	void writecache_be_word(offs_t offset, UINT16 data);
	void writecache_be_dword(offs_t offset, UINT32 data);

	UINT8 readcache_le(offs_t offset);
	UINT16 readcache_le_word(offs_t offset);
	UINT32 readcache_le_dword(offs_t offset);
	void writecache_le(offs_t offset, UINT8 data);
	void writecache_le_word(offs_t offset, UINT16 data);
	void writecache_le_dword(offs_t offset, UINT32 data);

	// interrupts
	void generate_exception(int exception);
	void check_irqs();
	void set_irq_line(int irqline, int state);
	void invalid_instruction();

	// instructions
	UINT32 get_cop0_reg(int idx);
	void set_cop0_reg(int idx, UINT32 val);
	UINT32 get_cop0_creg(int idx);
	void set_cop0_creg(int idx, UINT32 val);
	void handle_cop0();

	UINT32 get_cop1_reg(int idx);
	void set_cop1_reg(int idx, UINT32 val);
	UINT32 get_cop1_creg(int idx);
	void set_cop1_creg(int idx, UINT32 val);
	void handle_cop1();

	UINT32 get_cop2_reg(int idx);
	void set_cop2_reg(int idx, UINT32 val);
	UINT32 get_cop2_creg(int idx);
	void set_cop2_creg(int idx, UINT32 val);
	void handle_cop2();

	UINT32 get_cop3_reg(int idx);
	void set_cop3_reg(int idx, UINT32 val);
	UINT32 get_cop3_creg(int idx);
	void set_cop3_creg(int idx, UINT32 val);
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
	direct_read_data *m_direct;

	// configuration
	chip_type       m_chip_type;
	bool            m_hasfpu;
	endianness_t    m_endianness;

	// core registers
	UINT32      m_pc;
	UINT32      m_nextpc;
	UINT32      m_hi;
	UINT32      m_lo;
	UINT32      m_r[32];

	// COP registers
	UINT32      m_cpr[4][32];
	UINT32      m_ccr[4][32];

	// internal stuff
	UINT32      m_ppc;
	UINT32      m_op;
	int         m_icount;
	int         m_interrupt_cycles;

	// endian-dependent load/store
	void        (r3000_device::*m_lwl)();
	void        (r3000_device::*m_lwr)();
	void        (r3000_device::*m_swl)();
	void        (r3000_device::*m_swr)();

	// memory accesses
	r3000_data_accessors *m_cur;
	r3000_data_accessors m_memory_hand;
	r3000_data_accessors m_cache_hand;

	// cache memory
	UINT32 *    m_cache;
	std::vector<UINT32> m_icache;
	std::vector<UINT32> m_dcache;
	size_t      m_cache_size;
	size_t      m_icache_size;
	size_t      m_dcache_size;

	// I/O
	devcb_read_line    m_in_brcond0;
	devcb_read_line    m_in_brcond1;
	devcb_read_line    m_in_brcond2;
	devcb_read_line    m_in_brcond3;
};


// ======================> r3041_device

class r3041_device : public r3000_device
{
public:
	r3041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> r3051_device

class r3051_device : public r3000_device
{
public:
	r3051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> r3052_device

class r3052_device : public r3000_device
{
public:
	r3052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> r3071_device

class r3071_device : public r3000_device
{
public:
	r3071_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> r3081_device

class r3081_device : public r3000_device
{
public:
	r3081_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition

extern const device_type R3041;
extern const device_type R3051;
extern const device_type R3052;
extern const device_type R3071;
extern const device_type R3081;

#endif /* __R3000_H__ */
