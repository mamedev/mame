// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Dempa Micom Soft Analog/Digital Controller XE-1AP emulation

    PC pin  Name    MD pin  Name    Dir     Signal
    1       Up      1       Up      In      D0
    2       Down    2       Down    In      D1
    3       Left    3       Left    In      D2
    4       Right   4       Right   In      D3
    6       TRIG1   6       TL      In      L/H
    7       TRIG2   9       TR      In      Ack
    8       STROBE  7       TH      Out     Req

    In analog mode, data is shifted out as eleven nybbles:

            _           ____________________________________________
    Req      \_________/
            ____    __    __    __    __    __    __    __    __
    Ack         \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/
                    _____       _____       _____       _____
    L/H     XX\____/     \_____/     \_____/     \_____/     \_____/
               ____ _____ _____ _____ _____ _____ _____ _____ _____
    D       XXX____X_____X_____X_____X_____X_____X_____X_____X_____X

    The falling edge on Req causes data output to start.  The host
    can't control the speed, it just polls the L/H and Ack lines to
    know when the data is ready to read.

    Step    D0      D1      D2      D3
     1      Select  Start   B'      A'
     2      D       C       B       A
     2      X4      X5      X6      X7
     3      Y4      Y5      Y6      Y7
     5      RZ4     RZ5     RZ6     RZ7
     6      Z4      Z5      Z6      Z7
     7      X0      X1      X2      X3
     8      Y0      Y1      Y2      Y3
     9      RZ0     RZ1     RZ2     RZ3
    10      Z0      Z1      Z2      Z3
    11      E2      E1      -       -

    In digital mode, Req is a simple multiplexer input:

    Req     0       1
    D0      Up      Throttle Up
    D1      Down    Throttle Down
    D2      Left    C
    D3      Right   D
    L/H     A       E1
    Ack     B       E2

    Start appears as simultaneous Left/Right
    Select appears as simultaneous Up/Down

    This mode is almost compatible with a 6-button Towns Pad (on a
    real 6-button Towns Pad, buttons A and B can be read in either
    state, they bypass the multiplexer).

    TODO:
    * Measure timings.
    * Confirm reported value for unused analog channel 4.
    * Confirm A', B', E1 and E2 bit mappings in analog mode.
    * Confirm buttons mapping in digital mode.
     - Is the mapping different in PC vs MD mode?
    * Implement trigger A/B rapid fire switches.
    * Implement channel shift switch (Y->X, X->Z, Z->X).
    * Implement special modes (holding buttons on power-on):
     - Double displacement modes:
      + X/Y (hold SELECT + A')
      + Z (hold SELECT + B')
      + X/Y/Z (hold SELECT + A' + B')
     - Up/down reverse mode (hold C)

**********************************************************************/

#include "emu.h"
#include "xe1ap.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

class sms_xe1ap_device : public device_t, public device_sms_control_interface
{
public:
	sms_xe1ap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override;
	virtual void out_w(u8 data, u8 mem_mask) override;

	DECLARE_INPUT_CHANGED_MEMBER(mode_changed);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	TIMER_CALLBACK_MEMBER(step_output);

	required_ioport m_buttons;
	required_ioport_array<4> m_axes;
	required_ioport m_mode;

	emu_timer *m_output_timer;

	u8 m_req;
	u8 m_out;
};



INPUT_PORTS_START( sms_xe1ap )
	PORT_START("BUTTONS")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_SELECT)                     // left face bottom
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_START)                      // right face bottom
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("%p B'") // right face top inner
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("%p A'") // right face top outer
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("%p D")  // left shoulder lower
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("%p C")  // left shoulder upper
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("%p B")  // right shoulder lower
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("%p A")  // right shoulder upper
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("%p E2") // left face top inner
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("%p E1") // left face top outer
	PORT_BIT(0x0c00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CH0")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y)              PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("CH1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X)              PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("CH2")
	PORT_BIT(0xff, 0x80, IPT_PADDLE_V)   PORT_REVERSE PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("CH3")
	PORT_BIT(0xff, 0x80, IPT_UNUSED)

	PORT_START("MODE")
	PORT_CONFNAME(0x01, 0x01, "Mode") PORT_CHANGED_MEMBER(DEVICE_SELF, sms_xe1ap_device, mode_changed, 0)
	PORT_CONFSETTING(   0x00, "Digital")
	PORT_CONFSETTING(   0x01, "Analog")
INPUT_PORTS_END



sms_xe1ap_device::sms_xe1ap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_XE1AP, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_buttons(*this, "BUTTONS"),
	m_axes(*this, "CH%u", 0U),
	m_mode(*this, "MODE"),
	m_output_timer(nullptr),
	m_req(1),
	m_out(0x3f)
{
}


u8 sms_xe1ap_device::in_r()
{
	if (BIT(m_mode->read(), 0))
	{
		LOG(
				"%s: analog mode read data = %02X\n",
				machine().describe_context(),
				m_out);
		return m_out;
	}
	else
	{
		u16 const buttons = m_buttons->read();
		if (m_req)
		{
			u8 const x = m_axes[0]->read();
			u8 const y = m_axes[1]->read();
			u8 const result =
					((BIT(buttons, 0) && (0x40 <= y)) ? 0x01 : 0x00) | // Select/Up
					((BIT(buttons, 0) && (0xc0 > y))  ? 0x02 : 0x00) | // Select/Down
					((BIT(buttons, 1) && (0x40 <= x)) ? 0x04 : 0x00) | // Start/Left
					((BIT(buttons, 1) && (0xc0 > x))  ? 0x08 : 0x00) | // Start/Right
					(BIT(buttons, 7) << 4) |                           // A
					(BIT(buttons, 6) << 5);                            // B
			LOG(
					"%s: digital mode basic read = 0x%02X\n",
					machine().describe_context(),
					result);
			return result;
		}
		else
		{
			u8 const z = m_axes[3]->read();
			u8 const result =
					((0xc0 > z)  ? 0x01 : 0x00) | // Throttle Up
					((0x40 <= z) ? 0x02 : 0x00) | // Throttle Down
					(BIT(buttons, 5) << 2) |      // C
					(BIT(buttons, 4) << 3) |      // D
					(BIT(buttons, 9) << 4) |      // E1
					(BIT(buttons, 8) << 5);       // E2
			LOG(
					"%s: digital mode extended read = 0x%02X\n",
					machine().describe_context(),
					result);
			return result;
		}
	}
}


void sms_xe1ap_device::out_w(u8 data, u8 mem_mask)
{
	u8 const req = BIT(data, 6);
	if (req != m_req)
	{
		if (BIT(m_mode->read(), 0))
		{
			LOG("%s: /Req = %u\n", machine().describe_context(), req);
			if (!req)
				m_output_timer->adjust(attotime::from_usec(4), 0);
		}
		else
		{
			LOG("%s: /Req = %u ignored in digital mode\n", machine().describe_context(), req);
		}
		m_req = req;
	}
}


INPUT_CHANGED_MEMBER(sms_xe1ap_device::mode_changed)
{
	if (newval)
	{
		LOG("Analog mode selected\n");
	}
	else
	{
		LOG("Digital mode selected\n");
		m_output_timer->enable(false);
		m_out = 0x3f;
	}
}


ioport_constructor sms_xe1ap_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sms_xe1ap);
}


void sms_xe1ap_device::device_start()
{
	m_output_timer = timer_alloc(FUNC(sms_xe1ap_device::step_output), this);

	save_item(NAME(m_req));
	save_item(NAME(m_out));
}


TIMER_CALLBACK_MEMBER(sms_xe1ap_device::step_output)
{
	if (!BIT(param, 0))
	{
		auto const step = param >> 1;
		if (12 > step)
		{
			switch (step)
			{
			case 0:
			case 1:
				m_out = BIT(m_buttons->read(), step ? 4 : 0, 4);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				m_out = BIT(m_axes[((step - 2) & 0x03) ^ 0x01]->read(), (step < 6) ? 4 : 0, 4);
				break;
			case 10:
				m_out = BIT(m_buttons->read(), 8, 4);
				break;
			default:
				m_out = 0x0f;
			}
			m_out |= BIT(param, 1) ? 0x30 : 0x20;
			LOG(
					"Set nybble %u data = 0x%X, L/H = %d, /Ack = 1\n",
					step,
					BIT(m_out, 0, 4),
					BIT(m_out, 4));
			m_output_timer->adjust(attotime::from_usec(4), param + 1);
		}
		else
		{
			m_out = 0x3f;
		}
	}
	else
	{
		m_out &= 0x1f;
		LOG("Set nybble %u /Ack = 0\n", param >> 1);
		m_output_timer->adjust(attotime::from_usec(20), param + 1);
	}
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_XE1AP, device_sms_control_interface, sms_xe1ap_device, "sms_xe1ap", "Dempa Micom Soft Analog Controller (XE-1AP)")
