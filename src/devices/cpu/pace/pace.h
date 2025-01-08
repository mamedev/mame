// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor PACE/INS8900

****************************************************************************
                            _____________
                   D04   1 |             | 40  D05
                   D03   2 |             | 39  D06
                   D02   3 |             | 38  D07
                   D01   4 |             | 37  D08
                   D00   5 |             | 36  D09
                   IDS   6 |             | 35  D10
                   ODS   7 |             | 34  D11
                  NADS   8 |             | 33  D12
                 NHALT   9 |             | 32  D13
                CONTIN  10 |   IPC-16A   | 31  D14
                  JC14  11 |   INS8900   | 30  D15
                  JC15  12 |             | 29  Vgg/Vdd*
                  JC13  13 |             | 28  BPS
                  NIR5  14 |             | 27  EXTEND
                  NIR4  15 |             | 26  NINIT
                  NIR3  16 |             | 25  CLK/Vcc*
                  NIR2  17 |             | 24  NCLK/CLKX
                   F11  18 |             | 23  Vbb*
                   F12  19 |             | 22  F14
                  Vss*  20 |_____________| 21  F13

    * For PACE (IPC-16A): Vss = +5V, Vbb = +8V, Vgg = -12V
    * For INS8900: Vss = GND, Vbb = -8V, Vcc = +5V, Vdd = +12V

***************************************************************************/

#ifndef MAME_CPU_PACE_PACE_H
#define MAME_CPU_PACE_PACE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pace_device

class pace_device : public cpu_device
{
public:
	// register enumeration
	enum {
		PACE_PC,
		PACE_FR,
		PACE_AC0, PACE_AC1, PACE_AC2, PACE_AC3,
		PACE_STKD,
		PACE_STK0, PACE_STK1, PACE_STK2, PACE_STK3, PACE_STK4,
		PACE_STK5, PACE_STK6, PACE_STK7, PACE_STK8, PACE_STK9
	};

	// callback configuration
	auto jc13_callback() { return m_jc_callback[0].bind(); }
	auto jc14_callback() { return m_jc_callback[1].bind(); }
	auto jc15_callback() { return m_jc_callback[2].bind(); }
	auto f11_callback() { return m_flag_callback[0].bind(); }
	auto f12_callback() { return m_flag_callback[1].bind(); }
	auto f13_callback() { return m_flag_callback[2].bind(); }
	auto f14_callback() { return m_flag_callback[3].bind(); }

protected:
	// construction/destruction
	pace_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual void execute_run() override;
	virtual void execute_set_input(int irqline, int state) override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	enum class cycle : u8 {
		IFETCH_M1, LEA_M2, RDEA_M3,

		BOC_M4, BOC_M5,
		JMP_M4,
		JMP_IND_M4,
		JSR_M4, JSR_M5,
		JSR_IND_M4, JSR_IND_M5,
		RTS_M4, RTS_M5,
		RTI_M4, RTI_M5, RTI_M6,
		BRANCH,

		SKNE_M4, SKNE_M5,
		SKG_M4, SKG_M5, SKG_M6, SKG_M7,
		SKAZ_M4, SKAZ_M5,
		ISZ_M4, ISZ_M5, ISZ_M6, ISZ_M7,
		DSZ_M4, DSZ_M5, DSZ_M6, DSZ_M7,
		AISZ_M4, AISZ_M5,
		SKIP,

		LD_M4,
		LD_IND_M4, LD_IND_M5,
		ST_M4,
		ST_IND_M4,
		LSEX_M4,

		AND_M4,
		OR_M4,
		ADD_M4,
		SUBB_M4,
		DECA_M4, DECA_M5, DECA_M6, DECA_M7,

		LI_M4,
		RCPY_M4,
		RXCH_M4, RXCH_M5, RXCH_M6,
		XCHRS_M4, XCHRS_M5, XCHRS_M6,
		CFR_M4,
		CRF_M4,
		PUSH_M4,
		PULL_M4,
		PUSHF_M4,
		PULLF_M4,

		RADD_M4,
		RADC_M4,
		RAND_M4,
		RXOR_M4,
		CAI_M4, CAI_M5,

		SHL_M4, SHL_M5, SHL_M6, SHL_M7, SHL_M8,
		SHR_M4, SHR_M5, SHR_M6, SHR_M7, SHR_M8,
		ROL_M4, ROL_M5, ROL_M6, ROL_M7, ROL_M8,
		ROR_M4, ROR_M5, ROR_M6, ROR_M7, ROR_M8,

		HALT_M4,
		PFLG_M4, PFLG_M5, PFLG_M6,

		UNKNOWN
	};

	static const cycle s_decode[64];

	// flag and interrupt helpers
	void set_control_flag(u8 fc);
	void reset_control_flag(u8 fc);
	void set_fr(u16 r);

	// conditions and ALU helpers
	bool sign_bit(u16 r) const noexcept;
	bool equals_0(u16 r) const noexcept;
	bool poll_condition(u8 cc);
	static void sign_extend(u16 &r);
	void add(u16 &dr, u16 sr, bool c);
	void decimal_add(u16 &dr, u16 sr, unsigned stage);
	void prepare_shift();
	void shift_left(u16 &r, bool rotate);
	void shift_right(u16 &r, bool rotate);

	// stack helpers
	void stack_push(u16 r);
	u16 stack_pull();

	// execution helpers
	void read_instruction();
	u16 get_effective_address();
	void read_effective_address();
	void write_effective_address(u16 r);
	cycle execute_one();

	// address space and cache
	address_space_config m_space_config;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_space;

	// callback objects
	devcb_read_line m_bps_callback;
	devcb_read_line::array<3> m_jc_callback;
	devcb_write_line::array<4> m_flag_callback;

	// core registers
	u16 m_fr;
	u16 m_pc;
	u16 m_mdr;
	u16 m_mar;
	u16 m_ac[4];

	// stack
	u8 m_stkp;
	u8 m_stack_depth;
	u16 m_stack[10];

	// execution state
	u16 m_ppc;
	u16 m_cir;
	bool m_shift_link;
	cycle m_cycle;
	s32 m_icount;
};

// ======================> ins8900_device

class ins8900_device : public pace_device
{
public:
	// device type constructor
	ins8900_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(INS8900, ins8900_device)

#endif // MAME_CPU_PACE_PACE_H
