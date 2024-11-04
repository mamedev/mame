// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega controller adapter emulation

    Simple wired adapter:

            PC Pin  Sega pin
             Up  1  1  Up
           Down  2  2  Down
           Left  3  3  Left
          Right  4  4  Right
            +5V  5  5  +5V
      Trigger 1  6  6  TL
      Trigger 2  7  9  TR
         Strobe  8  7  TH
            GND  9  8  GND

    An adapter with this wiring was included with the game Chelnov
    for Sharp X68000, to be used with a 3-button Sega Mega Drive
    Control Pad.  The same adapters were supported by Super Street
    Figher II for Sharp X68000 for use with 6-button Sega Mega
    Drive controllers.

**********************************************************************/

#include "emu.h"
#include "sgadapt.h"

#include "bus/sms_ctrl/controllers.h"
#include "bus/sms_ctrl/smsctrl.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

class sega_adapter_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	sega_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_6_w(int state) override;
	virtual void pin_7_w(int state) override;
	virtual void pin_8_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<sms_control_port_device> m_port;

	u8 m_out;
	u8 m_th_in;
};


sega_adapter_device::sega_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SEGACTRL, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_port(*this, "ctrl")
	, m_out(0x7f)
	, m_th_in(0x40)
{
}

u8 sega_adapter_device::read()
{
	return (m_port->in_r() & 0x3f) | m_th_in;
}

void sega_adapter_device::pin_6_w(int state)
{
	if (u8(state ? 1 : 0) != BIT(m_out, 4))
	{
		if (state)
			m_out |= 0x10;
		else
			m_out &= ~0x10;
		LOG("TL %u -> %u (out 0x%02X)\n", BIT(~m_out, 4), BIT(m_out, 4), m_out);
		m_port->out_w(m_out, m_out ^ 0x7f);
	}
}

void sega_adapter_device::pin_7_w(int state)
{
	if (u8(state ? 1 : 0) != BIT(m_out, 5))
	{
		if (state)
			m_out |= 0x20;
		else
			m_out &= ~0x20;
		LOG("TR %u -> %u (out 0x%02X)\n", BIT(~m_out, 5), BIT(m_out, 5), m_out);
		m_port->out_w(m_out, m_out ^ 0x7f);
	}
}

void sega_adapter_device::pin_8_w(int state)
{
	if (u8(state ? 1 : 0) != BIT(m_out, 6))
	{
		if (state)
			m_out |= 0x40;
		else
			m_out &= ~0x40;
		LOG("TH %u -> %u (out 0x%02X)\n", BIT(~m_out, 6), BIT(m_out, 6), m_out);
		m_port->out_w(m_out, m_out ^ 0x7f);
	}
}

void sega_adapter_device::device_start()
{
	save_item(NAME(m_out));
	save_item(NAME(m_th_in));
}

void sega_adapter_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_port, sms_control_port_devices, SMS_CTRL_OPTION_MD_6BUTTON);
	m_port->th_handler().set([this] (int state) { m_th_in = state ? 0x40 : 0x00; });
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_SEGACTRL, device_msx_general_purpose_port_interface, sega_adapter_device, "msx_segactrl", "X68000 Sega Controller Adapter")
