// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    PLA (Programmable Logic Array) emulation

**********************************************************************/

#pragma once

#ifndef __PLA__
#define __PLA__

#include "emu.h"



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
	pla_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void set_num_inputs(device_t &device, UINT32 i) { downcast<pla_device &>(device).m_inputs = i; }
	static void set_num_outputs(device_t &device, UINT32 o) { downcast<pla_device &>(device).m_outputs = o; }
	static void set_num_terms(device_t &device, UINT32 t) { downcast<pla_device &>(device).m_terms = t; }
	static void set_inputmask(device_t &device, UINT32 mask) { downcast<pla_device &>(device).m_input_mask = mask; } // UINT32!
	static void set_format(device_t &device, int format) { downcast<pla_device &>(device).m_format = format; }

	UINT32 inputs() { return m_inputs; }
	UINT32 outputs() { return m_outputs; }

	UINT32 read(UINT32 input);

protected:
	// device-level overrides
	virtual void device_start();

private:
	void parse_fusemap();

	int m_format;

	UINT32 m_inputs;
	UINT32 m_outputs;
	UINT32 m_terms;
	UINT64 m_input_mask;
	UINT64 m_xor;

	int m_cache_size;
	std::vector<UINT32> m_cache;
	UINT64 m_cache2[CACHE2_SIZE];
	UINT8 m_cache2_ptr;

	struct term
	{
		UINT64 and_mask;
		UINT64 or_mask;
	} m_term[MAX_TERMS];
};


// device type definition
extern const device_type PLA;


#endif
