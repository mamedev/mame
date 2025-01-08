// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    PLA (Programmable Logic Array) emulation

**********************************************************************/

#ifndef MAME_MACHINE_PLA_H
#define MAME_MACHINE_PLA_H

#pragma once


// 82S100, 82S101, PLS100, PLS101
// 16x48x8 PLA, 28-pin:
/*           _____   _____
     FE   1 |*    \_/     | 28  Vcc
     I7   2 |             | 27  I8
     I6   3 |             | 26  I9
     I5   4 |             | 25  I10
     I4   5 |             | 24  I11
     I3   6 |    82S100   | 23  I12
     I2   7 |    82S101   | 22  I13
     I1   8 |    PLS100   | 21  I14
     I0   9 |    PLS101   | 20  I15
     F7  10 |             | 19  _CE
     F6  11 |             | 18  F0
     F5  12 |             | 17  F1
     F4  13 |             | 16  F2
    GND  14 |_____________| 15  F3
*/


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> pla_device

class pla_device : public device_t
{
public:
	enum class FMT
	{
		JEDBIN = 0,
		BERKELEY
	};

	// construction/destruction
	pla_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	pla_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	pla_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t inputs, uint32_t outputs, uint32_t terms)
		: pla_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_num_inputs(inputs);
		set_num_outputs(outputs);
		set_num_terms(terms);
	}

	// configuration helpers
	void set_num_inputs(uint32_t i) { m_inputs = i; }
	void set_num_outputs(uint32_t o) { m_outputs = o; }
	void set_num_terms(uint32_t t) { m_terms = t; }
	void set_inputmask(uint32_t mask) { m_input_mask = mask; } // uint32_t!
	void set_format(FMT format) { m_format = format; }

	uint32_t inputs() { return m_inputs; }
	uint32_t outputs() { return m_outputs; }

	uint32_t read(uint32_t input);
	bool reinit();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr unsigned MAX_TERMS       = 512;
	static constexpr unsigned MAX_CACHE_BITS  = 20;
	static constexpr unsigned CACHE2_SIZE     = 8;

	int parse_fusemap();

	required_memory_region m_region;

	FMT m_format;

	uint32_t m_inputs;
	uint32_t m_outputs;
	uint32_t m_terms;
	uint64_t m_input_mask;
	uint64_t m_xor;

	int m_cache_size;
	std::vector<uint32_t> m_cache;
	uint64_t m_cache2[CACHE2_SIZE];
	uint8_t m_cache2_ptr;

	struct term
	{
		uint64_t and_mask;
		uint64_t or_mask;
	} m_term[MAX_TERMS];
};

class pls100_device : public pla_device
{
public:
	pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class mos8721_device : public pla_device
{
public:
	mos8721_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(PLA, pla_device)
DECLARE_DEVICE_TYPE(PLS100, pls100_device)
DECLARE_DEVICE_TYPE(MOS8721, mos8721_device)

#endif // MAME_MACHINE_PLA_H
