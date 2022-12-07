// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega Mark III "Paddle Control" emulation


    Release data from the Sega Retro project:

      Year: 1987    Country/region: JP    Model code: HPD-200

    Notes:

      The main chip contained in the device is labeled 315-5243.

      On Japanese systems (Mark III and Master System), the games
      read the port and expect the value on the low four bits
      (usually used joystick switches) to alternate between the low
      and high nybbles of the paddle position, with TR indicating
      the current state.

      On export consoles, the games toggle the TH output before
      reading a nybble, expecting to be able to select the high or
      low nybble of the paddle position.  It spins for longer between
      pulling TH low and reading the low nybble than it does between
      pulling TH high and reading the high nybble, suggesting that
      pulling TH low triggers acquisition.

      Only a single paddle controller was released that works with
      both Japanese and export consoles, and the Mark III has the pin
      that became TH tied low while it's left high when reading the
      paddle on Japanese Master Systems.  This suggests that if the
      TH line doesn't change state for some amount of time, the
      paddle will periodically switch between high and low nybbles.

**********************************************************************/

#include "emu.h"
#include "paddle.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace  {

INPUT_PORTS_START( sms_paddle )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0, 255) PORT_SENSITIVITY(40) PORT_KEYDELTA(20) PORT_CENTERDELTA(0)
INPUT_PORTS_END



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_paddle_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_paddle_device(const machine_config &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_sms_control_interface implementation
	virtual u8 in_r() override;
	virtual void out_w(u8 data, u8 mem_mask) override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_paddle); }
	virtual void device_start() override;

private:
	TIMER_CALLBACK_MEMBER(timeout);

	// time interval guessed
	// Player 2 of Galactic Protector is the most sensitive to this timing
	static attotime interval() { return attotime::from_hz(XTAL(10'738'635) / 3 / 100); }

	// delay guessed
	static attotime delay() { return attotime::from_msec(5); }

	required_ioport m_button;
	required_ioport m_axis;

	emu_timer *m_idle_timer;

	u8 m_th;
	u8 m_nybble;
	u8 m_data;
};


sms_paddle_device::sms_paddle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_PADDLE, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_button(*this, "BUTTON"),
	m_axis(*this, "PADDLE"),
	m_idle_timer(nullptr),
	m_th(1),
	m_nybble(1),
	m_data(0)
{
}


u8 sms_paddle_device::in_r()
{
	LOG("%s: read %s nybble = 0x%x\n", machine().describe_context(), m_nybble ? "high" : "low", m_data & 0x0f);
	return (m_nybble << 5) | (m_button->read() << 4) | (m_data & 0x0f);
}


void sms_paddle_device::out_w(u8 data, u8 mem_mask)
{
	u8 const th = BIT(data, 6);
	if (th != m_th)
	{
		m_idle_timer->reset(delay());
		if (!th)
		{
			m_nybble = 0;
			m_data = m_axis->read();
			LOG("%s: TH %u -> %u\n, acquire data 0x%02X", machine().describe_context(), m_th, th, m_data);
		}
		else if (!m_nybble)
		{
			m_nybble = 1;
			m_data >>= 4;
			LOG("%s: TH %u -> %u\n, shift data", machine().describe_context(), m_th, th);
		}
		else
		{
			LOG("%s: TH %u -> %u\n", machine().describe_context(), m_th, th);
		}
		m_th = th;
	}
}


void sms_paddle_device::device_start()
{
	m_idle_timer = timer_alloc(FUNC(sms_paddle_device::timeout), this);

	m_nybble = 1;
	m_data = 0;

	save_item(NAME(m_th));
	save_item(NAME(m_nybble));
	save_item(NAME(m_data));

	m_idle_timer->adjust(delay(), 0, interval());
}


TIMER_CALLBACK_MEMBER(sms_paddle_device::timeout)
{
	if (m_nybble)
	{
		m_nybble = 0;
		m_data = m_axis->read();
		LOG("timeout: acquire data 0x%02X\n", m_data);
	}
	else
	{
		m_nybble = 1;
		m_data >>= 4;
		LOG("timeout: shift data\n");
	}
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_PADDLE, device_sms_control_interface, sms_paddle_device, "sms_paddle", "Sega Mark III Paddle (Japan)")
