// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    tmc208k.h
    TRW TMC208K/TMC28KU 8x8-bit Parallel Multiplier

    Known Equivalents:
    - Logic Devices Inc. LMU08/8U

***************************************************************************/

#ifndef MAME_MACHINE_TMC208K_TMC208K_H
#define MAME_MACHINE_TMC208K_TMC208K_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tmc208_base_device

template <typename RegType, typename OutType>
class tmc208_base_device : public device_t
{
public:
	// construction/destruction
	tmc208_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	void a_w(uint8_t data);
	void b_w(uint8_t data);
	void trim_w(int state);
	void tril_w(int state);
	void clk_a_w(int state);
	void clk_b_w(int state);
	void clk_r_w(int state);
	void rnd_w(int state);

	// Outputs by group
	auto msp() { return m_msp.bind(); }
	auto lsp() { return m_lsp.bind(); }

	// Full output
	auto r() { return m_r.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void clock_a();
	virtual void clock_b();
	void multiply();

	RegType m_a_in;
	RegType m_a;
	RegType m_b_in;
	RegType m_b;
	OutType m_r_out;

	bool m_trim;
	bool m_tril;
	bool m_clk_a;
	bool m_clk_b;
	bool m_clk_r;
	bool m_rnd_in;
	bool m_rnd;

	devcb_write8 m_msp;
	devcb_write8 m_lsp;
	devcb_write16 m_r;
};


// ======================> tmc208k_device

class tmc208k_device : public tmc208_base_device<int8_t, int16_t>
{
public:
	// construction/destruction
	tmc208k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> tmc28ku_device

class tmc28ku_device : public tmc208_base_device<uint8_t, uint16_t>
{
public:
	// construction/destruction
	tmc28ku_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void clock_b() override;
};

// device type definition
DECLARE_DEVICE_TYPE(TMC208K, tmc208k_device)
DECLARE_DEVICE_TYPE(TMC28KU, tmc28ku_device)

#endif // MAME_MACHINE_TMC208K_TMC208K_H
