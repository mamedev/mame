// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiout.c

    MIDI Out image device and serial receiver

*********************************************************************/

#include "emu.h"
#include "midiout.h"

#include "osdepend.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(MIDIOUT, midiout_device, "midiout", "MIDI Out image device")

/*-------------------------------------------------
    ctor
-------------------------------------------------*/

midiout_device::midiout_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDIOUT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, device_serial_interface(mconfig, *this)
	, m_midi()
{
}

midiout_device::~midiout_device()
{
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void midiout_device::device_start()
{
	m_midi.reset();
}

void midiout_device::device_reset()
{
	// we don't Tx, we Rx at 31250 8-N-1
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(31250);
	set_tra_rate(0);
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/

std::pair<std::error_condition, std::string> midiout_device::call_load()
{
	m_midi = machine().osd().create_midi_output(filename());
	if (!m_midi)
	{
		return std::make_pair(image_error::UNSPECIFIED, std::string());
	}

	return std::make_pair(std::error_condition(), std::string());
}

/*-------------------------------------------------
    call_unload
-------------------------------------------------*/

void midiout_device::call_unload()
{
	m_midi.reset();
}

void midiout_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

	if (m_midi)
	{
		m_midi->write(data);
	}
}
