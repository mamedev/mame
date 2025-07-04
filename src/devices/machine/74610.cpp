// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*****************************************************************************

    74610: Memory mappers

    Variant   Output       Type
    ----------------------------------------
    74610     latched      tristate
    74611     latched      open collector
    74612     direct       tristate
    74613     direct       open collector


    Connection Diagram:
              _________
     RS2  1 |   | |   | 40  Vcc
     MA3  2 |   ---   | 39  MA2
     RS3  3 |         | 38  RS1
     CS*  4 |         | 37  MA1
 STROBE*  5 |         | 36  RS0
    R/W*  6 |         | 35  MA0
     D0   7 |         | 34  D11
     D1   8 |         | 33  D10
     D2   9 |         | 33  D9
     D3  10 |         | 31  D8
     D4  11 |         | 30  D7
     D5  12 |         | 29  D6
    MM*  13 |         | 28  C (610/611), nc (612/613)
    MO0  14 |         | 27  MO11
    MO1  15 |         | 26  MO10
    MO2  16 |         | 25  MO9
    MO3  17 |         | 24  MO8
    MO4  18 |         | 23  MO7
    MO5  19 |         | 22  MO6
    GND  20 |_________| 21  ME*


    Data bus connection: D0-D11
    Map output: MO0-MO11

    The internal memory is a 16 words (selected by RS3...RS0) by 12 bit RAM.

    The mapper is intended to expand e.g. a 16-bit address by 8 additional bits,
    making it a 24-bit address.

    =====================++
                         ||  D11 ... D0
    MA3...MA0      _____________
    ==============|             |---------------
                  |             |
    --------------|             |  MO11 ... MO0
    RS3 ... RS0   |             |  (A23 ... A12)
    (A15...A12)   |             |
    --------------|_____________|---------------
    A11 ... A0                     A11 ... A0
    --------------------------------------------





    The mapping value can be changed by loading the new value into the chip at
    the selected address (via the data bus). If the computer's data bus is
    smaller (e.g. 8 bit), the remaining four bits may be set by mapping the
    chip at different memory locations:

    4000: D11=0, D10=0, D9=0, D8=0
    4001: D11=0, D10=0, D9=0, D8=1
    4002: D11=0, D10=0, D9=1, D8=0
    ...
    400F: D11=1, D10=1, D9=1, D8=1


*****************************************************************************/

#include "emu.h"
#include "74610.h"

DEFINE_DEVICE_TYPE(TTL74610, ttl74610_device, "ttl74610", "SN74LS610 Memory mapper (latched, tristate)")
DEFINE_DEVICE_TYPE(TTL74611, ttl74611_device, "ttl74611", "SN74LS611 Memory mapper (latched, openc)")
DEFINE_DEVICE_TYPE(TTL74612, ttl74612_device, "ttl74612", "SN74LS612 Memory mapper (tristate)")
DEFINE_DEVICE_TYPE(TTL74613, ttl74613_device, "ttl74613", "SN74LS613 Memory mapper (openc)")

ttl7461x_device::ttl7461x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_has_tristate(true),
	m_enabled(false),
	m_map_output(*this),
	m_map_mode(false)
{
}

ttl7461x_latched_device::ttl7461x_latched_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ttl7461x_device(mconfig, type, tag, owner, clock),
	m_latch_enabled(false),
	m_latched_output(0)
{
}

ttl74610_device::ttl74610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7461x_latched_device(mconfig, TTL74610, tag, owner, clock)
{
}

ttl74611_device::ttl74611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7461x_latched_device(mconfig, TTL74611, tag, owner, clock)
{
	m_has_tristate = false;
}

ttl74612_device::ttl74612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7461x_device(mconfig, TTL74612, tag, owner, clock)
{
}

ttl74613_device::ttl74613_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7461x_device(mconfig, TTL74613, tag, owner, clock)
{
	m_has_tristate = false;
}

void ttl7461x_device::device_start()
{
	for (auto & elem : m_map) elem = 0;

	save_item(NAME(m_map));
	save_item(NAME(m_map_mode));
	save_item(NAME(m_enabled));
}

void ttl7461x_device::device_reset()
{
}

void ttl7461x_device::set_register(int num, uint16_t value)
{
	m_map[num & 0x0f] = value & 0x03ff;
}

uint16_t ttl7461x_device::get_register(int num)
{
	return m_map[num & 0x0f];
}

/*
    To make full use of the tristate/open collector feature, this method
    should be used.
*/
void ttl7461x_device::mapper_output_rz(uint8_t num, uint16_t& value)
{
	if (m_enabled)
	{
		if (m_has_tristate)
			value = get_mapper_output(num);
		else
			// open collectors can only pull down
			value &= get_mapper_output(num);
	}
}

uint16_t ttl7461x_device::get_mapper_output(uint8_t num)
{
	if (m_map_mode)
		return m_map[num & 0x0f];
	else
		return num;
}

void ttl7461x_device::map_mode_w(int mapping)
{
	m_map_mode = (((line_state)mapping) == ASSERT_LINE);
}

void ttl7461x_device::map_enable_w(int enable)
{
	m_enabled = (((line_state)enable) == ASSERT_LINE);
}

// Latched versions

void ttl7461x_latched_device::device_start()
{
	ttl7461x_device::device_start();
	save_item(NAME(m_latch_enabled));
	save_item(NAME(m_latched_output));
}

uint16_t ttl7461x_latched_device::get_mapper_output()
{
	return m_latched_output;
}

void ttl7461x_latched_device::mapper_output_rz(uint16_t& value)
{
	if (m_enabled) value = get_mapper_output();
}

void ttl7461x_latched_device::latch_enable_w(int enable)
{
	m_latch_enabled = (((line_state)enable) == ASSERT_LINE);
}

void ttl7461x_latched_device::set_map_address(uint8_t num)
{
	uint16_t mapvalue = ttl7461x_device::get_mapper_output(num);
	if (m_latch_enabled)
		m_latched_output = mapvalue;

	if (m_enabled)
		m_map_output(mapvalue);
}
