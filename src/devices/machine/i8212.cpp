// license:BSD-3-Clause
// copyright-holders:Curt Coder,AJR
/**********************************************************************

    Intel 8212/3212 8-Bit Input/Output Port (Multi-Mode Latch Buffer)

    The Intel 8212 was one of the first in a line of bipolar Schottky
    peripherals released early on for the 8080. Many of these were
    assigned alternate part numbers in the 3200 series to identify
    them with Intel's 3001/3002 bipolar bit-slice processing elements.

    The 8212's MD pin is typically tied to either GND or Vcc to fix
    the chip in one of its two operating modes. In the input mode
    (MD = L), data is latched on the falling edge of STB, and the
    three-state outputs are enabled by a combination of two chip
    select inputs of opposite polarities. In the output mode (MD = H),
    data is latched on the falling edge of chip selection, and outputs
    are always enabled. The service request flip-flop is clocked on
    the falling edge of STB to produce the INT output, and is reset by
    either chip selection or the active-low CLR input, the latter
    also resetting the latched data to zero.

    The 8212 in output mode was often used with the 8080 to latch the
    status word and with the 8085 to latch the lower address bits.

    RCA's CDP1852 is an almost pin-for-pin CMOS counterpart to the
    8212. The control lines of the CDP1852, however, work slightly
    differently, especially in output mode.

    When TI second-sourced the 8080A, they cloned the 8212 as the
    SN74S412 (and numbered their versions of the 8224, 8228 and 8338
    similarly). While simpler octal latches from the 7400 series such
    as 74LS273, 74LS373 and 74LS374 became far more common and widely
    used, even when a separate service request flip-flop needed to be
    coupled, the Fairchild Advanced Schottky TTL (FAST) evolution of
    the 7400 series had both inverting (74F432) and non-inverting
    (74F412) versions of this device.

**********************************************************************/

#include "emu.h"
#include "i8212.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I8212, i8212_device, "i8212", "Intel 8212 I/O Port")

//-------------------------------------------------
//  i8212_device - constructor
//-------------------------------------------------

i8212_device::i8212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, I8212, tag, owner, clock),
	m_write_int(*this),
	m_read_di(*this),
	m_write_do(*this),
	m_read_md(*this),
	m_stb(1), m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8212_device::device_start()
{
	// resolve callbacks
	m_write_int.resolve_safe();
	m_read_di.resolve_safe(0);
	m_write_do.resolve_safe();
	m_read_md.resolve_safe(0);

	// register for state saving
	save_item(NAME(m_stb));
	save_item(NAME(m_data));
}


//-------------------------------------------------
//  get_mode - resolve device mode
//-------------------------------------------------

i8212_device::mode i8212_device::get_mode()
{
	return m_read_md() ? mode::OUTPUT : mode::INPUT;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8212_device::device_reset()
{
	// clear interrupt line
	m_write_int(CLEAR_LINE);

	// clear latched data
	m_data = 0;

	if (get_mode() == mode::OUTPUT)
	{
		// output data
		m_write_do((offs_t)0, m_data);
	}
}


//-------------------------------------------------
//  read - data latch read
//-------------------------------------------------

uint8_t i8212_device::read()
{
	if (!machine().side_effects_disabled())
	{
		// clear interrupt line
		m_write_int(CLEAR_LINE);
	}

	return m_data;
}


//-------------------------------------------------
//  inta_cb - data latch read (INTA triggered)
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(i8212_device::inta_cb)
{
	// clear interrupt line
	m_write_int(CLEAR_LINE);

	// read latched data as interrupt vector
	return m_data;
}


//-------------------------------------------------
//  write - data latch write
//-------------------------------------------------

void i8212_device::write(uint8_t data)
{
	// clear interrupt line
	m_write_int(CLEAR_LINE);

	if (get_mode() == mode::OUTPUT)
	{
		// latch data
		m_data = data;
		LOG("I8212: Writing %02X into latch (output mode)\n", data);

		// output data
		m_write_do((offs_t)0, m_data);
	}
}


//-------------------------------------------------
//  strobe - data input strobe
//-------------------------------------------------

void i8212_device::strobe(uint8_t data)
{
	if (get_mode() == mode::INPUT)
	{
		m_data = data;
		LOG("I8212: Writing %02X into latch (input mode)\n", data);
	}

	// assert interrupt line
	m_write_int(ASSERT_LINE);
}


//-------------------------------------------------
//  stb_w - data strobe write
//-------------------------------------------------

WRITE_LINE_MEMBER(i8212_device::stb_w)
{
	// active on falling edge
	if (m_stb && !state)
	{
		if (get_mode() == mode::INPUT)
		{
			// input data
			m_data = m_read_di(0);
			LOG("I8212: Reading %02X into latch (input mode)\n", m_data);
		}

		// assert interrupt line
		m_write_int(ASSERT_LINE);
	}

	m_stb = state;
}
