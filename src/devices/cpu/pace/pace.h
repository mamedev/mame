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
		PACE_SP,
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

	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int irqline, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// internal helpers
	void fr_w(u16 data);

	// address space and cache
	address_space_config m_space_config;
	address_space *m_space;
	memory_access_cache<1, -1, ENDIANNESS_LITTLE> *m_inst_cache;

	// callback objects
	devcb_read_line m_jc_callback[3];
	devcb_write_line m_flag_callback[4];

	// execution state
	s32 m_icount;
	u16 m_pc;
	u16 m_fr;
	u16 m_ac[4];
	u8 m_sp;
	u16 m_stk[10];
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
