/**********************************************************************

    Nintendo Entertainment System - Miracle Piano Keyboard
 
    TODO: basically everything, this is just a skeleton with no 
    real MIDI handling at the moment.

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "miracle.h"
#include "bus/midi/midi.h"

#define MIRACLE_MIDI_WAITING 0
#define MIRACLE_MIDI_RECEIVE 1
#define MIRACLE_MIDI_SEND 2

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_MIRACLE = &device_creator<nes_miracle_device>;


MACHINE_CONFIG_FRAGMENT( nes_miracle )
//	MCFG_CPU_ADD("piano_cpu", I8051, XTAL_11_0592MHz)	// xtal to be verified

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
MACHINE_CONFIG_END

machine_config_constructor nes_miracle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( nes_miracle );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nes_miracle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_STROBE_ON)
	{
		m_strobe_clock++;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_miracle_device - constructor
//-------------------------------------------------

nes_miracle_device::nes_miracle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_MIRACLE, "Miracle Piano Controller", tag, owner, clock, "nes_miracle", __FILE__)
					, device_nes_control_port_interface(mconfig, *this)
//					, m_cpu(*this, "piano_cpu")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_miracle_device::device_start()
{
	strobe_timer = timer_alloc(TIMER_STROBE_ON);
	strobe_timer->adjust(attotime::never);
	save_item(NAME(m_strobe_on));
	save_item(NAME(m_sent_bits));
	save_item(NAME(m_strobe_clock));
	save_item(NAME(m_midi_mode));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_miracle_device::device_reset()
{
	m_strobe_on = 0;
	m_sent_bits = 0;
	m_strobe_clock = 0;
	m_midi_mode = MIRACLE_MIDI_WAITING;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

// TODO: here, reads from serial midi in bit0, when in MIDI_SEND mode

UINT8 nes_miracle_device::read_bit0()
{
	UINT8 ret = 0;
	if (m_strobe_clock >= 66)
	{
		// more than 66 clocks since strobe on write means send mode
		m_midi_mode = MIRACLE_MIDI_SEND;
		strobe_timer->reset();
		m_strobe_on = 0;
		m_strobe_clock = 0;
//		printf("send start\n");
	}

	if (m_midi_mode == MIRACLE_MIDI_SEND)
	{
		//NES reads from Miracle Piano!
		// ret |= ...
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

// TODO: here, writes to serial midi in bit0, when in MIDI_RECEIVE mode

void nes_miracle_device::write(UINT8 data)
{
	if (data == 1 && !m_strobe_on)
	{
		strobe_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));
		m_strobe_on = 1;
		return;
	}
	
	if (m_strobe_on)
	{
		// was timer running?
		if (m_strobe_clock > 0)
		{
			if (m_strobe_clock < 66 && data == 0)
			{
				// less than 66 clocks before new write means receive mode
				m_midi_mode = MIRACLE_MIDI_RECEIVE;
//				printf("receive start\n");
				strobe_timer->reset();
				m_strobe_on = 0;
				m_strobe_clock = 0;
				return;
			}
		}

		if (m_midi_mode == MIRACLE_MIDI_SEND &&  data == 0)
		{
			// strobe off after the end of a byte
			m_midi_mode = MIRACLE_MIDI_WAITING;
//			printf("send end\n");
		}
	}

	if (m_midi_mode == MIRACLE_MIDI_RECEIVE)
	{
		//NES writes (data & 1) to Miracle Piano!
		// 1st write is data present flag (1=data present)
		// next 8 writes are actual data bits (with ^1)
		m_sent_bits++;
		// then we go back to waiting
		if (m_sent_bits == 9)
		{
//			printf("receive end\n");
			m_midi_mode = MIRACLE_MIDI_WAITING;
			m_sent_bits = 0;
		}
	}
}
