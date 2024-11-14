// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (US model) emulation


Release data from the Sega Retro project:

  Year: 1987    Country/region: US    Model code: 3040

TODO:

- For low-level emulation, a device for the TMP42C66P, a Toshiba 4-bit
  microcontroller, is needed along with a dump of its internal ROM.
- Auto-repeat and Control/Sports mode switches are not emulated.

Notes:

  Games designed for the US model of the Sports Pad controller use the
  TH line of the controller port to select which nibble of the two axis
  to read. The Japanese cartridge Sports Pad Soccer uses a different mode
  when it does not detect use by a Japanese SMS console, because the Sega
  Mark III lacks the TH line. There was a different Sports Pad model released
  in Japan and no information was found about it supporting both modes, so
  that model is currently emulated as a different device (see sportsjp.cpp).

  It was discovered that games designed for the Paddle Controller, released
  in Japan, will switch to a mode incompatible with the original Paddle when
  the system region is detected as Export. Similar to how the US model of the
  Sports Pad works, that mode uses the TH line as an output to select which
  nibble of the X axis will be read. So, on an Export console version,
  paddle games are somewhat playable with the US Sport Pad model, though it
  needs to be used inverted and the trackball needs to be moved slowly,
  otherwise the software for the paddle think it's moving backward.
  See http://mametesters.org/view.php?id=5872 for discussion.

**********************************************************************/

#include "emu.h"
#include "sports.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

INPUT_PORTS_START( sms_sports_pad )
	PORT_START("SPORTS_BT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // TR

	PORT_START("SPORTS_X")    // X axis
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_REVERSE

	PORT_START("SPORTS_Y")    // Y axis
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_REVERSE
INPUT_PORTS_END



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_sports_pad_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_sports_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sms_control_interface implementation
	virtual uint8_t in_r() override;
	virtual void out_w(uint8_t data, uint8_t mem_mask) override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_sports_pad); }
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(timeout);

	required_ioport m_buttons;
	required_ioport_array<2> m_axes;

	emu_timer *m_timeout_timer;

	uint8_t m_th_state;
	uint8_t m_phase;
	uint8_t m_output;
	uint8_t m_base[2];
	uint8_t m_data[2];
};


//-------------------------------------------------
//  sms_sports_pad_device - constructor
//-------------------------------------------------

sms_sports_pad_device::sms_sports_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_SPORTS_PAD, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_buttons(*this, "SPORTS_BT"),
	m_axes(*this, "SPORTS_%c", 'X'),
	m_timeout_timer(nullptr),
	m_th_state(1),
	m_phase(0),
	m_output(0x0f)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_sports_pad_device::device_start()
{
	m_timeout_timer = timer_alloc(FUNC(sms_sports_pad_device::timeout), this);

	m_phase = 0;
	m_output = 0x0f;
	m_base[0] = 0x80; // value 0x80 helps when starting paddle games.
	m_base[1] = 0x80;
	m_data[0] = 0x00;
	m_data[1] = 0x00;

	save_item(NAME(m_th_state));
	save_item(NAME(m_phase));
	save_item(NAME(m_output));
	save_item(NAME(m_base));
	save_item(NAME(m_data));
}


uint8_t sms_sports_pad_device::in_r()
{
	return m_buttons->read() | m_output;
}


void sms_sports_pad_device::out_w(uint8_t data, uint8_t mem_mask)
{
	// Sequence used to read Sports Pad:
	// * TH = 0, spin for 80 microseconds, read X high nybble
	// * TH = 1, spin for 40 microseconds, read X low nybble
	// * TH = 0, spin for 40 microseconds, read Y high nybble
	// * TH = 1, spin for 40 microseconds, read Y low nybble
	uint8_t const th_state = BIT(data, 6);
	if (th_state != m_th_state)
	{
		if (!th_state)
		{
			LOG(
					"%s: TH falling, output %c high nybble\n",
					machine().describe_context(),
					m_phase ? 'Y' : 'X');

			m_timeout_timer->reset();
			m_data[m_phase] = m_axes[m_phase]->read();
			m_output = BIT(m_data[m_phase] - m_base[m_phase], 4, 4);
		}
		else
		{
			LOG(
					"%s: TH rising, output %c low nybble%s\n",
					machine().describe_context(),
					m_phase ? 'Y' : 'X',
					m_phase ? ", zero counts" : "");

			m_timeout_timer->adjust(attotime::from_hz(XTAL(10'738'635) / 3 / 512)); // timeout not verified
			m_output = BIT(m_data[m_phase] - m_base[m_phase], 0, 4);
			if (m_phase)
			{
				// assume the rising edge when Y is output zeroes the axes
				m_base[0] = m_data[0];
				m_base[1] = m_data[1];
			}
			m_phase ^= 1;
		}
	}

	m_th_state = th_state;
}


TIMER_CALLBACK_MEMBER(sms_sports_pad_device::timeout)
{
	// assume it just flips back to X, allowing paddle games to "work"
	LOG("timeout, select X axis\n");
	m_phase = 0;
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_SPORTS_PAD, device_sms_control_interface, sms_sports_pad_device, "sms_sports_pad", "Sega SMS Sports Pad (US)")
