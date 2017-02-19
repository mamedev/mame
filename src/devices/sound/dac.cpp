// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    dac.cpp

    Four quadrant multiplying DAC.

    Binary Weighted Resistor Network, R-2R Ladder & PWM

    Binary, Ones Complement, Twos Complement or Sign Magnitude coding

***************************************************************************/

#define DAC_GENERATOR_EPILOG(_dac_type, _dac_class) \
const device_type _dac_type = &device_creator<_dac_class>;

#include "emu.h"
#include "dac.h"
