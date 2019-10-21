// license:GPL-2.0+
// copyright-holders:Dirk Best
/*****************************************************************************
 *
 *  TTL74145
 *
 *
 *  BCD-to-Decimal decoder
 *
 *        __ __
 *     0-|  v  |-VCC
 *     1-|     |-A
 *     2-|     |-B
 *     3-|     |-C
 *     4-|     |-D
 *     5-|     |-9
 *     6-|     |-8
 *   GND-|_____|-7
 *
 *
 * Truth table
 *  _______________________________
 * | Inputs  | Outputs             |
 * | D C B A | 0 1 2 3 4 5 6 7 8 9 |
 * |-------------------------------|
 * | L L L L | L H H H H H H H H H |
 * | L L L H | H L H H H H H H H H |
 * | L L H L | H H L H H H H H H H |
 * | L L H H | H H H L H H H H H H |
 * | L H L L | H H H H L H H H H H |
 * |-------------------------------|
 * | L H L H | H H H H H L H H H H |
 * | L H H L | H H H H H H L H H H |
 * | L H H H | H H H H H H H L H H |
 * | H L L L | H H H H H H H H L H |
 * | H L L H | H H H H H H H H H L |
 * |-------------------------------|
 * | H L H L | H H H H H H H H H H |
 * | H L H H | H H H H H H H H H H |
 * | H H L L | H H H H H H H H H H |
 * | H H L H | H H H H H H H H H H |
 * | H H H L | H H H H H H H H H H |
 * | H H H H | H H H H H H H H H H |
 *  -------------------------------
 *
 ****************************************************************************/

#include "emu.h"
#include "74145.h"
#include "coreutil.h"


DEFINE_DEVICE_TYPE(TTL74145, ttl74145_device, "ttl74145", "TTL74145")

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/
//-------------------------------------------------
//  ttl74145_device - constructor
//-------------------------------------------------

ttl74145_device::ttl74145_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TTL74145, tag, owner, clock)
	, m_output_line_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_number(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74145_device::device_start()
{
	/* resolve callbacks */
	for (std::size_t bit = 0; bit < 10; bit++)
		m_output_line_cb[bit].resolve_safe();

	// register for state saving
	save_item(NAME(m_number));
}

//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void ttl74145_device::device_reset()
{
	m_number = 0;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void ttl74145_device::write(uint8_t data)
{
	/* decode number */
	uint16_t new_number = bcd_2_dec(data & 0x0f);

	/* call output callbacks if the number changed */
	if (new_number != m_number)
	{
		for (std::size_t bit = 0; bit < 10; bit++)
			m_output_line_cb[bit](new_number == bit);
	}

	/* update state */
	m_number = new_number;
}


uint16_t ttl74145_device::read()
{
	return (1 << m_number) & 0x3ff;
}
