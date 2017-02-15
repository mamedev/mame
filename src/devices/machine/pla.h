// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    PLA (Programmable Logic Array) emulation

**********************************************************************/

#pragma once

#ifndef __PLA__
#define __PLA__




//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MAX_TERMS       512
#define MAX_CACHE_BITS  20
#define CACHE2_SIZE     8

enum
{
	PLA_FMT_JEDBIN = 0,
	PLA_FMT_BERKELEY
};



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_PLA_ADD(_tag, _inputs, _outputs, _terms) \
	MCFG_DEVICE_ADD(_tag, PLA, 0) \
	pla_device::set_num_inputs(*device, _inputs); \
	pla_device::set_num_outputs(*device, _outputs); \
	pla_device::set_num_terms(*device, _terms);

#define MCFG_PLA_INPUTMASK(_mask) \
	pla_device::set_inputmask(*device, _mask);

#define MCFG_PLA_FILEFORMAT(_format) \
	pla_device::set_format(*device, _format);


// macros for known (and used) devices

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
#define MCFG_PLS100_ADD(_tag) \
	MCFG_PLA_ADD(_tag, 16, 8, 48)

// MOS 8721 PLA
// TODO: actual number of terms is unknown
#define MCFG_MOS8721_ADD(_tag) \
	MCFG_PLA_ADD(_tag, 27, 18, 379)



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> pla_device

class pla_device : public device_t
{
public:
	// construction/destruction
	pla_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_num_inputs(device_t &device, uint32_t i) { downcast<pla_device &>(device).m_inputs = i; }
	static void set_num_outputs(device_t &device, uint32_t o) { downcast<pla_device &>(device).m_outputs = o; }
	static void set_num_terms(device_t &device, uint32_t t) { downcast<pla_device &>(device).m_terms = t; }
	static void set_inputmask(device_t &device, uint32_t mask) { downcast<pla_device &>(device).m_input_mask = mask; } // uint32_t!
	static void set_format(device_t &device, int format) { downcast<pla_device &>(device).m_format = format; }

	uint32_t inputs() { return m_inputs; }
	uint32_t outputs() { return m_outputs; }

	uint32_t read(uint32_t input);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void parse_fusemap();

	required_memory_region m_region;

	int m_format;

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


// device type definition
extern const device_type PLA;


#endif
