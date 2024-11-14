// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Dempa Micom Soft Analog/Digital Controller emulation

    PC pin  Name    MD pin  Name    Dir     Signal
    1       Up      1       Up      In      D0
    2       Down    2       Down    In      D1
    3       Left    3       Left    In      D2
    4       Right   4       Right   In      D3
    6       TRIG1   6       TL      In      L/H
    7       TRIG2   9       TR      In      ACK
    8       STROBE  7       TH      Out     REQ

    In analog mode, data is shifted out as twelve nybbles:

          _           ________________________________________________________________
    REQ    \_________/
          ____    __    __    __    __    __    __    __    __    __    __    __    __
    ACK       \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/
                  _____       _____       _____       _____       _____       _____
    L/H   _______/     \_____/     \_____/     \_____/     \_____/     \_____/     \__
              _____ _____ _____ _____ _____ _____ _____ _____ _____ _____ _____ _____
    D     XXXX_____X_____X_____X_____X_____X_____X_____X_____X_____X_____X_____X_____X

    The falling edge on REQ causes data output to start.  The host
    can't control the speed, it just polls the L/H and ACK lines to
    know when the data is ready to read.

    Nybble  D3      D2      D1      D0
     1      A/A'    B/B'    C       D
     2      E1      E2      Start   Select
     3      Y7      Y6      Y5      Y4
     4      X7      X6      X5      X4
     5      Z7      Z6      Z5      Z4
     6      RZ7     RZ6     RZ5     RZ4
     7      Y3      Y2      Y1      Y0
     8      X3      X2      X1      X0
     9      Z3      Z2      Z1      Z0
    10      RZ3     RZ2     RZ1     RZ0
    11      A       B       A'      B'
    12      -       -       -       -

    In MD mode, each pair of nybbles is transmitted in reverse
    order.

    Sharp released assembly language source code for an X68000
    driver.  It uses the following algorithm:
    1. Generate falling edge on REQ
    2. Wait until L/H is low
    3. Wait until ACK is low
    4. Read a nybble
    5. Wait until L/H is high
    6. Wait until ACK is low
    7. Read a nybble
    8. If eight nybbles have been read, raise REQ.
    9. Loop to step 2 until twelve nybbles have been read

    Mega Drive games use a similar approach, but raise REQ after
    reading two nybbles.  PC Engine games only generate a short low
    pulse on REQ, but use the same algorithm to determine when to
    read data.

    CSK Research Institute games for FM Towns (including After
    Burner III and Galaxy Force II) use a different algorithm:
     1. Generate falling edge on REQ
     2. Wait until L/H is high
     3. Wait until ACK is high
     4. Read a nybble
     5. Wait until L/H is low
     6. Wait until ACK is high
     7. Read a nybble
     8. Wait until L/H is high
     9. Wait until ACK is high
    10. Read a nybble
    11. Loop to step 5 until eleven nybbles have been read
    11. Raise REQ

    From this it can be deduced that:
    * A negative edge on REQ triggers a report.
    * The exact time REQ is held low isn't important.
    * Data is valid while ACK is low and for some time after ACK is
      raised.
    * L/H is low when idle and changes some time before data is
      updated.

    In digital mode, REQ is a simple multiplexer input:

    REQ     0       1
    D0      Up      Throttle Up
    D1      Down    Throttle Down
    D2      Left    C
    D3      Right   D
    L/H     A/A'    E1
    ACK     B/B'    E2

    Start appears as simultaneous Left/Right
    Select appears as simultaneous Up/Down

    This mode is almost compatible with a 6-button Towns Pad (on a
    real 6-button Towns Pad, buttons A and B can be read in either
    state, they bypass the multiplexer).

    Digital MD mode emulates a 3-button Mega Drive pad:

    REQ     0       1
    D0      Up      Up
    D1      Down    Down
    D2      0       Left
    D3      0       Right
    L/H     A       B
    ACK     Start   C

    TODO:
    * Dump MB88513 microcontroller from original controller.
    * Measure timings.
     - Timings currently fudged for CRI games in FM Towns.
    * Latch data at beginning of packet.
    * Confirm button mapping in digital mode.
    * Estimate thresholds in digital modes.
    * Implement trigger A/B rapid fire switches.
    * Implement channel shift switch (Y->X, X->Z, Z->X).
    * Does channel shift affect digital mode?
    * Implement special modes (holding buttons on power-on):
     - Double displacement modes:
      + X/Y (hold SELECT + A')
      + Z (hold SELECT + B')
      + X/Y/Z (hold SELECT + A' + B')
     - Up/down reverse mode (hold C)
    * Implement desktop (XE-1AJ/CZ-8NJ2) version:
     - Four analog channels
     - E1/E2 on a rocker switch (can't press simultaneously)
     - Hold mode for A and B triggers
     - Variable rapid fire rate for A and B triggers
     - Reset button
     - Different special modes
     - No Mega Drive mode
     - Start and Select not reported in digital mode

**********************************************************************/

#include "emu.h"
#include "micomxe1a.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MICOM_XE_1A, micom_xe_1a_device, "micom_xe_1a", "Dempa Micom Soft Analog/Digital Intelligent Controller")



micom_xe_1a_device::micom_xe_1a_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock):
	device_t(mconfig, MICOM_XE_1A, tag, owner, clock),
	m_buttons_callback(*this, 0xffff),
	m_analog_callback(*this, 0x00),
	m_output_timer(nullptr),
	m_req(1),
	m_mode(1),
	m_interface(0),
	m_out(0x2f)
{
}

micom_xe_1a_device::~micom_xe_1a_device()
{
}


u8 micom_xe_1a_device::out_r()
{
	if (m_mode)
	{
		LOG("%s: analog mode read data = %02X\n", machine().describe_context(), m_out);
		return m_out;
	}
	else
	{
		u16 const buttons = m_buttons_callback();
		if (m_interface)
		{
			u8 const y = m_analog_callback(0);
			if (m_req)
			{
				u8 const x = m_analog_callback(1);
				u8 const result =
						((0x40 <= y) ? 0x01 : 0x00) |                      // Up
						((0xc0 > y)  ? 0x02 : 0x00) |                      // Down
						((0x40 <= x) ? 0x04 : 0x00) |                      // Left
						((0xc0 > x)  ? 0x08 : 0x00) |                      // Right
						((BIT(buttons, 2) & BIT(buttons, 8)) << 4) |       // B/B'
						(BIT(buttons, 1) << 5);                            // C
				LOG(
						"%s: MD digital mode basic read = 0x%02X\n",
						machine().describe_context(),
						result);
				return result;
			}
			else
			{
				u8 const result =
						((0x40 <= y) ? 0x01 : 0x00) |                      // Up
						((0xc0 > y)  ? 0x02 : 0x00) |                      // Down
						((BIT(buttons, 3) & BIT(buttons, 9)) << 4) |       // A/A'
						(BIT(buttons, 5) << 5);                            // Start
				LOG(
						"%s: MD digital mode extended read = 0x%02X\n",
						machine().describe_context(),
						result);
				return result;
			}
		}
		else
		{
			if (m_req)
			{
				u8 const z = m_analog_callback(2);
				u8 const result =
						((0x40 <= z) ? 0x01 : 0x00) | // Throttle Up
						((0xc0 > z)  ? 0x02 : 0x00) | // Throttle Down
						(BIT(buttons, 1) << 2) |      // C
						(BIT(buttons, 0) << 3) |      // D
						(BIT(buttons, 7) << 4) |      // E1
						(BIT(buttons, 6) << 5);       // E2
				LOG(
						"%s: digital mode extended read = 0x%02X\n",
						machine().describe_context(),
						result);
				return result;
			}
			else
			{
				u8 const y = m_analog_callback(0);
				u8 const x = m_analog_callback(1);
				u8 const result =
						((BIT(buttons, 4) && (0x40 <= y)) ? 0x01 : 0x00) | // Select/Up
						((BIT(buttons, 4) && (0xc0 > y))  ? 0x02 : 0x00) | // Select/Down
						((BIT(buttons, 5) && (0x40 <= x)) ? 0x04 : 0x00) | // Start/Left
						((BIT(buttons, 5) && (0xc0 > x))  ? 0x08 : 0x00) | // Start/Right
						((BIT(buttons, 3) & BIT(buttons, 9)) << 4) |       // A/A'
						((BIT(buttons, 2) & BIT(buttons, 8)) << 5);        // B/B'
				LOG(
						"%s: digital mode basic read = 0x%02X\n",
						machine().describe_context(),
						result);
				return result;
			}
		}
	}
}


void micom_xe_1a_device::req_w(int state)
{
	u8 const req = state ? 1 : 0;
	if (req != m_req)
	{
		if (m_mode)
		{
			LOG("%s: /REQ = %u\n", machine().describe_context(), req);
			if (!req)
			{
				// acquire data
				u16 const buttons = m_buttons_callback();
				u8 analog[4];
				for (unsigned i = 0; std::size(analog) > i; ++i)
					analog[i] = m_analog_callback(i);

				// pack data
				m_data[0] = BIT(buttons, 0, 8) & ((BIT(buttons, 8, 2) << 2) | 0xf3);
				m_data[1] = BIT(analog[0], 4, 4) | (BIT(analog[1], 4, 4) << 4);
				m_data[2] = BIT(analog[2], 4, 4) | (BIT(analog[3], 4, 4) << 4);
				m_data[3] = BIT(analog[0], 0, 4) | (BIT(analog[1], 0, 4) << 4);
				m_data[4] = BIT(analog[2], 0, 4) | (BIT(analog[3], 0, 4) << 4);
				m_data[5] = BIT(buttons, 8, 8) & ((BIT(buttons, 2, 2) << 2) | 0xf3);

				// takes a while to respond
				m_output_timer->adjust(attotime::from_nsec(50'000), 0);
			}
		}
		else
		{
			LOG("%s: /REQ = %u ignored in digital mode\n", machine().describe_context(), req);
		}
		m_req = req;
	}
}


void micom_xe_1a_device::mode_w(int state)
{
	u8 const mode = state ? 1 : 0;
	if (mode != m_mode)
	{
		if (mode)
		{
			LOG("Analog mode selected\n");
		}
		else
		{
			LOG("Digital mode selected\n");
			m_output_timer->enable(false);
			m_out = 0x2f;
		}
		m_mode = mode;
	}
}


void micom_xe_1a_device::interface_w(int state)
{
	m_interface = state ? 1 : 0;
}


void micom_xe_1a_device::device_start()
{
	m_output_timer = timer_alloc(FUNC(micom_xe_1a_device::step_output), this);

	std::fill(std::begin(m_data), std::end(m_data), 0x00);
	m_out = 0x2f;

	save_item(NAME(m_req));
	save_item(NAME(m_mode));
	save_item(NAME(m_interface));
	save_item(NAME(m_data));
	save_item(NAME(m_out));
}


TIMER_CALLBACK_MEMBER(micom_xe_1a_device::step_output)
{
	auto const step = param >> 1;
	if (!BIT(param, 0))
	{
		m_out = (m_out & 0x0f) | (BIT(step, 0) ? 0x30 : 0x20);
		LOG(
				"Set nybble %u data = 0x%X, L/H = %u, /ACK = %u\n",
				step,
				BIT(m_out, 0, 4),
				BIT(m_out, 4),
				BIT(m_out, 5));
		if ((std::size(m_data) * 2) > step)
		{
			m_output_timer->adjust(attotime::from_nsec(10'000), param + 1);
		}
	}
	else
	{
		if ((std::size(m_data) * 2) > step)
		{
			auto const nybble = step ^ m_interface;
			if ((std::size(m_data) * 2) > step)
				m_out = BIT(m_data[nybble >> 1], BIT(nybble, 0) ? 4 : 0, 4) | (m_out & 0x10);
			else
				m_out = 0x0f | (m_out & 0x10);
			LOG(
					"Set nybble %u data = 0x%X, L/H = %u, /ACK = %u\n",
					step,
					BIT(m_out, 0, 4),
					BIT(m_out, 4),
					BIT(m_out, 5));
			m_output_timer->adjust(attotime::from_nsec(10'000), param + 1);
		}
	}
}
