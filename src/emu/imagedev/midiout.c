/*********************************************************************

    midiout.c

    MIDI Out image device and serial receiver

*********************************************************************/

#include "emu.h"
#include "midiout.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

const device_type MIDIOUT = &device_creator<midiout_device>;

/*-------------------------------------------------
    ctor
-------------------------------------------------*/

midiout_device::midiout_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MIDIOUT, "MIDI Out image device", tag, owner, clock),
	device_image_interface(mconfig, *this),
		device_serial_interface(mconfig, *this)
{
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void midiout_device::device_start()
{
}

void midiout_device::device_reset()
{
	// we don't Tx, we Rx at 31250 8-N-1
	set_rcv_rate(31250);
	set_tra_rate(0);
	set_data_frame(8, 1, SERIAL_PARITY_NONE);
}

/*-------------------------------------------------
    device_config_complete  										   
-------------------------------------------------*/

void midiout_device::device_config_complete(void)
{
	update_names();
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/

bool midiout_device::call_load(void)
{
	m_midi = osd_open_midi_output(filename());

	if (m_midi == NULL)
	{
		return IMAGE_INIT_FAIL;
	}

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    call_unload
-------------------------------------------------*/

void midiout_device::call_unload(void)
{
	osd_close_midi_channel(m_midi);
}

void midiout_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	UINT8 data = get_received_char();

	osd_write_midi_channel(m_midi, data);
}

void midiout_device::input_callback(UINT8 state)
{
}

