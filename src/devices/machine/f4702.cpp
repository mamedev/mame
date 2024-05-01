// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Fairchild 4702B Programmable Bit Rate Generator

    Originally numbered 34702 in Fairchild's isoplanar CMOS series (whose
    lower-numbered products were logical equivalents of RCA CD4000 and
    Motorola MC14500 series devices), this BRG incorporates some unusually
    sophisticated features into its 16-pin package, which may be why
    Fairchild classified it as a LSI IC.

    The standard 2.4576 MHz master clock, either generated from a crystal
    oscillator (Ix, Ox) or provided as a TTL input (CP), is first prescaled
    by dividing by 8 and then divided further down through a network of
    counters with differing periods. These counters are tapped at 13 points,
    each frequency being 16 times a common baud rate. The rate output (Z),
    whose changes are synchronized with the master clock, is multiplexed
    from any one of these internal sources or an external rate input (Im) by
    a 4-bit select code (S0, S1, S2, S3).

    The buffered clock output (CO) and the three prescaler divisions (Q0,
    Q1, Q2) are not only conveniently available externally for clocking
    additional 4702s or other devices, but can also be exploited to
    generate 8 baud rates simultaneously, with the aid of a Fairchild 93L34
    (or compatible) addressable latch for demultiplexing the channels. In
    this configuration, the CO, Q and Z outputs are respectively tied to the
    latch's enable, address and data inputs, and the Q outputs are directly
    or indirectly fed back into the S inputs. (Fairchild even suggested
    scanning 9LS170 register files with the Q outputs so as to make each
    rate individually programmable.)

    The rate select codes are arranged so that connecting a simple five-
    point switch to the S inputs can obtain 110, 150, 300, 1200 or 2400
    baud, and only three binary switches are needed to select between all
    five of those rates plus 1800, 4800 and 9600 baud. 19200 baud can also
    be obtained by tapping the Q2 output, but no more than 4 internally
    multiplexed rates can be used at once if Im is generated from Q2.

    All inputs and outputs except Ix and Ox are TTL-compatible, with the
    inputs also having internal pull-ups so switches can be connected
    directly.

    To enable the Ix/Ox oscillator circuit, Ecp must be high and CP must
    remain low, since Ecp and CP both being high places the chip in a
    continuous reset mode (except for the CO output). If Ecp is brought
    low, the initialization circuit produces an internal master reset
    pulse the first time CP goes high. Neither of these two reset methods
    was used much in practice.

    Intersil, one of not many companies to second-source the original 4702,
    later produced IM4712, a minor variant which requires fewer external
    discrete components to drive the crystal oscillator but is otherwise
    logically identical.

****************************************************************************/

#include "emu.h"
#include "f4702.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(F4702, f4702_device, "f4702", "Fairchild 4702B Bit Rate Generator")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  f4702_device - constructor
//-------------------------------------------------

f4702_device::f4702_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, F4702, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_s_callback(*this, 15)
	, m_z_callback(*this)
	, m_main_counter(0)
	, m_div_200_50(0)
	, m_div_134_5(0)
	, m_div_110(0)
	, m_div_1800(0)
	, m_im(true)
	, m_s(0)
	, m_icount(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void f4702_device::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_main_counter));
	save_item(NAME(m_div_200_50));
	save_item(NAME(m_div_134_5));
	save_item(NAME(m_div_110));
	save_item(NAME(m_div_1800));
	save_item(NAME(m_im));
	save_item(NAME(m_s));
}


//-------------------------------------------------
//  reset_counters - optional master reset
//-------------------------------------------------

void f4702_device::reset_counters()
{
	// Reset counter network
	m_main_counter = 0;
	m_div_200_50 = 0;
	m_div_134_5 = 0;
	m_div_1800 = 0;

	// Reset Q and Z outputs
	m_z_callback(0, 0);
}


//**************************************************************************
//  RATE GENERATION
//**************************************************************************

//-------------------------------------------------
//  im_w - set external rate input
//-------------------------------------------------

void f4702_device::im_w(int state)
{
	m_im = state;
}


//-------------------------------------------------
//  z_output - recalculate output state
//-------------------------------------------------

bool f4702_device::z_output() const
{
	// Select Z output from one of the 13 counter taps or the external input
	switch (m_s)
	{
	// S3–S0 = LLLL or LLLH: multiplexed input
	case 0:
	case 1:
	default:
		return m_im;

	// S3–S0 = LLHL: 50 baud
	case 2:
		return m_div_200_50 >= 12;

	// S3–S0 = LLHH: 75 baud
	case 3:
		return BIT(m_main_counter, 10);

	// S3–S0 = LHLL: 134.5 baud (-0.87% error)
	case 4:
		return m_div_134_5 >= 9;

	// S3–S0 = LHLH: 200 baud
	case 5:
		return (m_div_200_50 % 6) >= 3;

	// S3–S0 = LHHL: 600 baud
	case 6:
		return BIT(m_main_counter, 7);

	// S3–S0 = LHHH or HHLL: 2400 baud
	case 7:
	case 12:
		return BIT(m_main_counter, 5);

	// S3–S0 = HLLL: 9600 baud
	case 8:
		return BIT(m_main_counter, 3);

	// S3–S0 = HLLH: 4800 baud
	case 9:
		return BIT(m_main_counter, 4);

	// S3–S0 = HLHL: 1800 baud
	case 10:
		return (m_div_1800 % 5) >= 2;

	// S3–S0 = HLHH: 1200 baud
	case 11:
		return BIT(m_main_counter, 6);

	// S3–S0 = HHLH: 300 baud
	case 13:
		return BIT(m_main_counter, 8);

	// S3–S0 = HHHL: 150 baud
	case 14:
		return BIT(m_main_counter, 9);

	// S3–S0 = HHHH: 110 baud (-0.83% error)
	case 15:
		return m_div_110 >= 11;
	}
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  counting
//-------------------------------------------------

void f4702_device::execute_run()
{
	do {
		// Drive the main scan and frequency counter
		m_main_counter++;
		if ((m_main_counter & 0x00f) == 0)
		{
			// Divide 9600 baud by 16/3 (16 = 5 + 5 + 6)
			m_div_1800++;
			if (m_div_1800 >= 16)
				m_div_1800 = 0;

			if ((m_main_counter & 0x03f) == 0)
			{
				// Divide 2400 baud by 18
				m_div_134_5++;
				if (m_div_134_5 >= 18)
					m_div_134_5 = 0;

				// Divide 2400 baud by 22 as well
				m_div_110++;
				if (m_div_110 >= 22)
					m_div_110 = 0;

				if ((m_main_counter & 0x07f) == 0)
				{
					// Divide 1200 baud by 6 and then again by 4
					m_div_200_50++;
					if (m_div_200_50 >= 24)
						m_div_200_50 = 0;
				}
			}
		}

		// Update Q and Z outputs
		m_z_callback(m_main_counter & 0x007, z_output());

		// S3–S0 inputs are valid on the rising edge of CO
		m_s = m_s_callback(m_main_counter & 0x007) & 15;
	} while (--m_icount > 0);
}
