// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hughes HLCD 0515/0569 LCD Driver

  TODO:
  - x

*/

#include "video/hlcd0515.h"


const device_type HLCD0515 = &device_creator<hlcd0515_device>;
const device_type HLCD0569 = &device_creator<hlcd0569_device>;

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hlcd0515_device::hlcd0515_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HLCD0515, "HLCD 0515 LCD Driver", tag, owner, clock, "hlcd0515", __FILE__)
{
}

hlcd0515_device::hlcd0515_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

hlcd0569_device::hlcd0569_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: hlcd0515_device(mconfig, HLCD0569, "HLCD 0569 LCD Driver", tag, owner, clock, "hlcd0569", __FILE__)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0515_device::device_start()
{
	// resolve callbacks

	// zerofill
	m_cs = 0;

	// register for savestates
	save_item(NAME(m_cs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hlcd0515_device::device_reset()
{
}



//-------------------------------------------------
//  handlers
//-------------------------------------------------
