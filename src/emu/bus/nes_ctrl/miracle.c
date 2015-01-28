/**********************************************************************

    Nintendo Entertainment System - Miracle Piano Keyboard

    TODO: basically everything, this is just a skeleton with no
    real MIDI handling at the moment.

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "miracle.h"

#define MIRACLE_MIDI_WAITING 0
#define MIRACLE_MIDI_RECEIVE 1      // receive byte from piano
#define MIRACLE_MIDI_SEND 2         // send byte to piano

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_MIRACLE = &device_creator<nes_miracle_device>;


MACHINE_CONFIG_FRAGMENT( nes_miracle )
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
					device_t(mconfig, NES_MIRACLE, "Miracle Piano Controller", tag, owner, clock, "nes_miracle", __FILE__),
					device_serial_interface(mconfig, *this),
					device_nes_control_port_interface(mconfig, *this),
					m_midiin(*this, "mdin"),
					m_midiout(*this, "mdout")
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

	// set standard MIDI parameters
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(31250);
	set_tra_rate(31250);

	m_xmit_read = m_xmit_write = 0;
	m_tx_busy = false;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

// TODO: here, reads from serial midi in bit0, when in MIDI_SEND mode

UINT8 nes_miracle_device::read_bit0()
{
	UINT8 ret = 0;

	if (m_midi_mode == MIRACLE_MIDI_RECEIVE)
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
// c4fc = start of recv routine
// c53a = start of send routine

void nes_miracle_device::write(UINT8 data)
{
//  printf("write: %d (%d %02x %d)\n", data & 1, m_sent_bits, m_data_sent, m_midi_mode);

	if (m_midi_mode == MIRACLE_MIDI_SEND)
	{
		//NES writes (data & 1) to Miracle Piano!
		// 1st write is data present flag (1=data present)
		// next 8 writes are actual data bits (with ^1)
		m_sent_bits++;
		m_data_sent <<= 1;
		m_data_sent |= (data & 1);
		// then we go back to waiting
		if (m_sent_bits == 8)
		{
//          printf("xmit MIDI byte %02x\n", m_data_sent);
			xmit_char(m_data_sent);
			m_midi_mode = MIRACLE_MIDI_WAITING;
			m_sent_bits = 0;
		}

		return;
	}

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
//          printf("got strobe at %d clocks\n", m_strobe_clock);

			if (m_strobe_clock < 66 && data == 0)
			{
				// short delay is recieve mode
				m_midi_mode = MIRACLE_MIDI_RECEIVE;
				strobe_timer->reset();
				m_strobe_on = 0;
				m_strobe_clock = 0;
				return;
			}
			else if (m_strobe_clock >= 66)
			{
				// more than 66 clocks since strobe on write means send mode
				m_midi_mode = MIRACLE_MIDI_SEND;
				strobe_timer->reset();
				m_strobe_on = 0;
				m_strobe_clock = 0;
				m_sent_bits = 1;
				m_data_sent <<= 1;
				m_data_sent |= (data & 1);
				return;
			}
		}

		if (m_midi_mode == MIRACLE_MIDI_SEND &&  data == 0)
		{
			// strobe off after the end of a byte
			m_midi_mode = MIRACLE_MIDI_WAITING;
		}
	}
}

void nes_miracle_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
//  UINT8 rcv = get_received_char();
}

void nes_miracle_device::tra_complete()    // Tx completed sending byte
{
//  printf("Tx complete\n");
	// is there more waiting to send?
	if (m_xmit_read != m_xmit_write)
	{
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= XMIT_RING_SIZE)
		{
			m_xmit_read = 0;
		}
	}
	else
	{
		m_tx_busy = false;
	}
}

void nes_miracle_device::tra_callback()    // Tx send bit
{
	// send this to midi out
	m_midiout->write_txd(transmit_register_get_data_bit());
}

void nes_miracle_device::xmit_char(UINT8 data)
{
//  printf("xmit %02x\n", data);

	// if tx is busy it'll pick this up automatically when it completes
	// if not, send now!
	if (!m_tx_busy)
	{
		m_tx_busy = true;
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= XMIT_RING_SIZE)
		{
			m_xmit_write = 0;
		}
	}
}
