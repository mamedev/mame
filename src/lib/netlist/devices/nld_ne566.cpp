// license:GPL-2.0+
// copyright-holders:Couriersud

//- Identifier:  NE566_DIP
//- Title: NE566 VOLTAGE-CONTROLED OSCILLATOR
//- Description:
//-
//- Pinalias: GND,NC,SQUARE,TRIANGLE,MODULATION,R1,C1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Phillips datasheet
//- Limitations:
//- Example:
//- FunctionTable:
//-
//-


/*
 * "Description: The Swiss army knife for timing purposes\n"
 * "    which has a ton of applications.\n"
 * "DipAlias: GND,NC,SQUARE,TRIANGLE,MODULATION,R1,C1,VCC\n"
 * "Package: DIP\n"
 * "NamingConvention: Naming conventions follow Philips datasheet\n"
 * "Limitations: \n"
 * "Function Table:\n"
 *
 *  Function table created from truthtable if missing.
 *
 *  For package, refer to:
 *
 *  https://en.wikipedia.org/wiki/List_of_integrated_circuit_packaging_types
 *
 *  Special case: GATE -> use symbolic names
 *
 */

#include "nld_ne566.h"
#include "netlist/analog/nlid_twoterm.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(NE566)
	{
		NETLIB_CONSTRUCTOR(NE566)
		, m_MODULATION(*this, "MODULATION")	// Pin 5
		, m_R1(*this, "R1")    				// Pin 6
		, m_C1(*this, "C1")    				// Pin 7
		, m_SQUARE(*this, "SQUARE")	        // Pin 3
		, m_TRIANGLE(*this, "TRIANGLE")	    // Pin 4
		, m_power_pins(*this)
		{
		}

		NETLIB_UPDATEI();
		NETLIB_RESETI();

	private:
		analog_input_t m_MODULATION;
		analog_input_t m_R1;
		analog_input_t m_C1;
		analog_output_t m_SQUARE;
		analog_output_t m_TRIANGLE;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(NE566_dip, NE566)
	{
		NETLIB_CONSTRUCTOR_DERIVED(NE566_dip, NE566)
		{
			register_subalias("1", "GND");        // Pin 1
			register_subalias("2", "NC");         // Pin 2
			register_subalias("3", "SQUARE");     // Pin 3
			register_subalias("4", "TRIANGLE");   // Pin 4
			register_subalias("5", "MODULATION"); // Pin 5
			register_subalias("6", "R1");         // Pin 6
			register_subalias("7", "C1");         // Pin 7
			register_subalias("8", "VCC");        // Pin 8
		}
	};

	NETLIB_RESET(NE566)
	{
	}

	NETLIB_UPDATE(NE566)
	{
		// frequency = f0 = (2 * (VCC - MODULATION)) / (R1 * C1 * VCC)
	}

	NETLIB_DEVICE_IMPL(NE566,     "NE566", "")
	NETLIB_DEVICE_IMPL(NE566_dip, "NE566_DIP", "")
	} //namespace devices
} // namespace netlist
