// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atariscom.cpp

    Atari sound communications device.

***************************************************************************/

#include "emu.h"
#include "atariscom.h"
#include "cpu/m6502/m6502.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SOUND_TIMER_RATE            attotime::from_usec(5)
#define SOUND_TIMER_BOOST           attotime::from_usec(1000)



//**************************************************************************
//  SOUND COMMUNICATIONS DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ATARI_SOUND_COMM, atari_sound_comm_device, "atariscom", "Atari Sound Communications")

//-------------------------------------------------
//  atari_sound_comm_device - constructor
//-------------------------------------------------

atari_sound_comm_device::atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ATARI_SOUND_COMM, tag, owner, clock)
	, m_main_int_cb(*this)
	, m_sound_cpu(*this, finder_base::DUMMY_TAG)
	, m_main_to_sound_ready(false)
	, m_sound_to_main_ready(false)
	, m_main_to_sound_data(0)
	, m_sound_to_main_data(0)
{
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_sound_comm_device::device_start()
{
	// resolve callbacks
	m_main_int_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_main_to_sound_ready));
	save_item(NAME(m_sound_to_main_ready));
	save_item(NAME(m_main_to_sound_data));
	save_item(NAME(m_sound_to_main_data));
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_sound_comm_device::device_reset()
{
	// reset the sound I/O states
	m_main_to_sound_data = m_sound_to_main_data = 0;
	m_main_to_sound_ready = m_sound_to_main_ready = false;
}


//-------------------------------------------------
//  device_timer: Handle device-specific timer
//  calbacks
//-------------------------------------------------

void atari_sound_comm_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TID_SOUND_RESET:
			delayed_sound_reset(param);
			break;

		case TID_SOUND_WRITE:
			delayed_sound_write(param);
			break;

		case TID_6502_WRITE:
			delayed_6502_write(param);
			break;
	}
}


//-------------------------------------------------
//  sound_reset_w: Write handler which resets the
//  sound CPU in response.
//-------------------------------------------------

void atari_sound_comm_device::sound_reset_w(u16 data)
{
	synchronize(TID_SOUND_RESET);
}


//-------------------------------------------------
//  main_command_w: Handles communication from the main CPU
//  to the sound CPU. Two versions are provided, one with the
//  data byte in the low 8 bits, and one with the data byte in
//  the upper 8 bits.
//-------------------------------------------------

void atari_sound_comm_device::main_command_w(u8 data)
{
	synchronize(TID_SOUND_WRITE, data);
}


//-------------------------------------------------
//  main_response_r: Handles reading data communicated from the
//  sound CPU to the main CPU. Two versions are provided, one
//  with the data byte in the low 8 bits, and one with the data
//  byte in the upper 8 bits.
//-------------------------------------------------

u8 atari_sound_comm_device::main_response_r()
{
	if (!machine().side_effects_disabled())
	{
		m_sound_to_main_ready = false;
		m_main_int_cb(CLEAR_LINE);
	}
	return m_sound_to_main_data;
}


//-------------------------------------------------
//  sound_response_w: Handles communication from the
//  sound CPU to the main CPU.
//-------------------------------------------------

void atari_sound_comm_device::sound_response_w(u8 data)
{
	synchronize(TID_6502_WRITE, data);
}


//-------------------------------------------------
//  sound_command_r: Handles reading data
//  communicated from the main CPU to the sound
//  CPU.
//-------------------------------------------------

u8 atari_sound_comm_device::sound_command_r()
{
	if (!machine().side_effects_disabled())
	{
		m_main_to_sound_ready = false;
		m_sound_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	return m_main_to_sound_data;
}


//-------------------------------------------------
//  delayed_sound_reset: Synchronizes the sound
//  reset command between the two CPUs.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_reset(int param)
{
	// unhalt and reset the sound CPU
	if (param == 0)
	{
		m_sound_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_sound_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}

	// reset the sound write state
	m_sound_to_main_ready = false;
	m_main_int_cb(CLEAR_LINE);

	// allocate a high frequency timer until a response is generated
	// the main CPU is *very* sensistive to the timing of the response
	machine().scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


//-------------------------------------------------
//  delayed_sound_write: Synchronizes a data write
//  from the main CPU to the sound CPU.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_write(int data)
{
	// warn if we missed something
	if (m_main_to_sound_ready)
		logerror("Missed command from 680x0\n");

	// set up the states and signal an NMI to the sound CPU
	m_main_to_sound_data = data;
	m_main_to_sound_ready = true;
	m_sound_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// allocate a high frequency timer until a response is generated
	// the main CPU is *very* sensistive to the timing of the response
	machine().scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


//-------------------------------------------------
//  delayed_6502_write: Synchronizes a data write
//  from the sound CPU to the main CPU.
//-------------------------------------------------

void atari_sound_comm_device::delayed_6502_write(int data)
{
	// warn if we missed something
	if (m_sound_to_main_ready)
		logerror("Missed result from 6502\n");

	// set up the states and signal the sound interrupt to the main CPU
	m_sound_to_main_data = data;
	m_sound_to_main_ready = true;
	m_main_int_cb(ASSERT_LINE);
}
