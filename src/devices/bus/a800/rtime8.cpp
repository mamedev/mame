// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************

    ICD R-Time 8 (Atari 800 cartridge)

    The circuitry in this clock cartridge is very simple, containing the RTC itself, oscillator and
    battery, and a 74HCT138 to decode the $D5B8-$D5BF address range from the /CCTL and A7-A3 pins. No ROM
    is included; however, the cartridge can be placed in the (currently unemulated) passthrough slot of the
    SpartaDOS X cartridge.

***********************************************************************************************************/

#include "emu.h"
#include "rtime8.h"

// device type definition
DEFINE_DEVICE_TYPE(A800_RTIME8, a800_rtime8_device, "a800_rtime8", "ICD R-Time 8")


//-------------------------------------------------
//  a800_rtime8_device - constructor
//-------------------------------------------------

a800_rtime8_device::a800_rtime8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A800_RTIME8, tag, owner, clock)
	, device_a800_cart_interface(mconfig, *this) 
	, m_rtc(*this, "rtc")
{
}

//-------------------------------------------------
//  device_add_mconfig - configure subdevices
//-------------------------------------------------

void a800_rtime8_device::device_add_mconfig(machine_config &config)
{
	M3002(config, m_rtc, 32.768_kHz_XTAL);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a800_rtime8_device::device_start()
{
}


//-------------------------------------------------
//  read_d5xx - handle reads from $D500-$D5FF
//-------------------------------------------------

u8 a800_rtime8_device::read_d5xx(offs_t offset)
{
	if ((offset & 0xf8) == 0xb8)
		return m_rtc->read(); // TODO: D7-D4 is open bus, in case this matters
	else
		return 0xff;
}

//-------------------------------------------------
//  write_d5xx - handle writes to $D500-$D5FF
//-------------------------------------------------

void a800_rtime8_device::write_d5xx(offs_t offset, u8 data)
{
	if ((offset & 0xf8) == 0xb8)
		m_rtc->write(data & 0x0f);
}
