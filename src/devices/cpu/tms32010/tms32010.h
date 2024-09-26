// license:BSD-3-Clause
// copyright-holders:Tony La Porta
/**************************************************************************

    Texas Instruments TMS32010 DSP Emulator

    Copyright Tony La Porta

**************************************************************************/

#ifndef MAME_CPU_TMS32010_TMS32010_H
#define MAME_CPU_TMS32010_TMS32010_H

#pragma once

#define TMS32010_INT_PENDING    0x80000000
#define TMS32010_INT_NONE       0

enum
{
	TMS32010_PC=1, TMS32010_SP,   TMS32010_STR,  TMS32010_ACC,
	TMS32010_PREG, TMS32010_TREG, TMS32010_AR0,  TMS32010_AR1,
	TMS32010_STK0, TMS32010_STK1, TMS32010_STK2, TMS32010_STK3
};


/****************************************************************************
 *  Public Functions
 */

template <int HighBits>
class tms3201x_base_device : public cpu_device
{
public:
	// configuration helpers
	auto bio() { return m_bio_in.bind(); }

protected:
	tms3201x_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor data_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 3; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void tms32010_ram(address_map &map) ATTR_COLD;
	void tms32015_ram(address_map &map) ATTR_COLD;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	devcb_read_line m_bio_in;

	using opcode_func = void (tms3201x_base_device::*)();
	struct tms32010_opcode
	{
		uint8_t       cycles;
		opcode_func function;
	};
	static const tms32010_opcode s_opcode_main[256];
	static const tms32010_opcode s_opcode_7F[32];

	/******************** CPU Internal Registers *******************/
	uint16_t  m_PC;
	uint16_t  m_PREVPC;     /* previous program counter */
	uint16_t  m_STR;
	PAIR      m_ACC;
	PAIR      m_ALU;
	PAIR      m_Preg;
	uint16_t  m_Treg;
	uint16_t  m_AR[2];
	uint16_t  m_STACK[4];

	PAIR      m_opcode;
	int       m_INTF;       /* Pending Interrupt flag */
	int       m_icount;
	PAIR      m_oldacc;
	uint16_t  m_memaccess;
	int       m_addr_mask;

	typename memory_access<HighBits, 1, -1, ENDIANNESS_BIG>::cache m_cache;
	typename memory_access<HighBits, 1, -1, ENDIANNESS_BIG>::specific m_program;
	memory_access<8, 1, -1, ENDIANNESS_BIG>::specific m_data;
	memory_access<4, 1, -1, ENDIANNESS_BIG>::specific m_io;

	inline void CLR(uint16_t flag);
	inline void SET_FLAG(uint16_t flag);
	inline void CALCULATE_ADD_OVERFLOW(int32_t addval);
	inline void CALCULATE_SUB_OVERFLOW(int32_t subval);
	inline uint16_t POP_STACK();
	inline void PUSH_STACK(uint16_t data);
	inline void UPDATE_AR();
	inline void UPDATE_ARP();
	inline void getdata(uint8_t shift,uint8_t signext);
	inline void putdata(uint16_t data);
	inline void putdata_sar(uint8_t data);
	inline void putdata_sst(uint16_t data);
	void opcodes_7F();
	void illegal();
	void abst();
	void add_sh();
	void addh();
	void adds();
	void and_();
	void apac();
	void br();
	void banz();
	void bgez();
	void bgz();
	void bioz();
	void blez();
	void blz();
	void bnz();
	void bv();
	void bz();
	void cala();
	void call();
	void dint();
	void dmov();
	void eint();
	void in_p();
	void lac_sh();
	void lack();
	void lar_ar0();
	void lar_ar1();
	void lark_ar0();
	void lark_ar1();
	void larp_mar();
	void ldp();
	void ldpk();
	void lst();
	void lt();
	void lta();
	void ltd();
	void mpy();
	void mpyk();
	void nop();
	void or_();
	void out_p();
	void pac();
	void pop();
	void push();
	void ret();
	void rovm();
	void sach_sh();
	void sacl();
	void sar_ar0();
	void sar_ar1();
	void sovm();
	void spac();
	void sst();
	void sub_sh();
	void subc();
	void subh();
	void subs();
	void tblr();
	void tblw();
	void xor_();
	void zac();
	void zalh();
	void zals();
	inline int add_branch_cycle();
	int Ext_IRQ();
};


class tms32010_device : public tms3201x_base_device<12>
{
public:
	// construction/destruction
	tms32010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms32015_device : public tms3201x_base_device<12>
{
public:
	// construction/destruction
	tms32015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms32016_device : public tms3201x_base_device<16>
{
public:
	// construction/destruction
	tms32016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMS32010, tms32010_device)
DECLARE_DEVICE_TYPE(TMS32015, tms32015_device)
DECLARE_DEVICE_TYPE(TMS32016, tms32016_device)

#endif // MAME_CPU_TMS32010_TMS32010_H
