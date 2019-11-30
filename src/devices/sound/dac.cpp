// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    dac.cpp

    Four quadrant multiplying DAC.

    Binary Weighted Resistor Network, R-2R Ladder & PWM

    Binary, Ones Complement, Twos Complement or Sign Magnitude coding

***************************************************************************/

#include "emu.h"

#define DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname) \
DEFINE_DEVICE_TYPE(_dac_type, _dac_class, _dac_shortname, _dac_description)

#include "dac.h"
