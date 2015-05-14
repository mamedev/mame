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


const device_type TTL74145 = &device_creator<ttl74145_device>;

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/
//-------------------------------------------------
//  ttl74145_device - constructor
//-------------------------------------------------

ttl74145_device::ttl74145_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TTL74145, "TTL74145", tag, owner, clock, "ttl74145", __FILE__)
	, m_output_line_0_cb(*this)
	, m_output_line_1_cb(*this)
	, m_output_line_2_cb(*this)
	, m_output_line_3_cb(*this)
	, m_output_line_4_cb(*this)
	, m_output_line_5_cb(*this)
	, m_output_line_6_cb(*this)
	, m_output_line_7_cb(*this)
	, m_output_line_8_cb(*this)
	, m_output_line_9_cb(*this)
	, m_number(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74145_device::device_start()
{
	/* resolve callbacks */
	m_output_line_0_cb.resolve_safe();
	m_output_line_1_cb.resolve_safe();
	m_output_line_2_cb.resolve_safe();
	m_output_line_3_cb.resolve_safe();
	m_output_line_4_cb.resolve_safe();
	m_output_line_5_cb.resolve_safe();
	m_output_line_6_cb.resolve_safe();
	m_output_line_7_cb.resolve_safe();
	m_output_line_8_cb.resolve_safe();
	m_output_line_9_cb.resolve_safe();

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

void ttl74145_device::write(UINT8 data)
{
	/* decode number */
	UINT16 new_number = bcd_2_dec(data & 0x0f);

	/* call output callbacks if the number changed */
	if (new_number != m_number)
	{
		m_output_line_0_cb(new_number == 0);
		m_output_line_1_cb(new_number == 1);
		m_output_line_2_cb(new_number == 2);
		m_output_line_3_cb(new_number == 3);
		m_output_line_4_cb(new_number == 4);
		m_output_line_5_cb(new_number == 5);
		m_output_line_6_cb(new_number == 6);
		m_output_line_7_cb(new_number == 7);
		m_output_line_8_cb(new_number == 8);
		m_output_line_9_cb(new_number == 9);
	}

	/* update state */
	m_number = new_number;
}


UINT16 ttl74145_device::read()
{
	return (1 << m_number) & 0x3ff;
}
