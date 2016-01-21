// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Super Nintendo Entertainment System - Miracle Piano Keyboard

    recv at PC = 008a4a
    xmit at PC = 008adb

**********************************************************************/

#include "miracle.h"

#define MIRACLE_MIDI_WAITING 0
#define MIRACLE_MIDI_RECEIVE 1      // receive byte from piano
#define MIRACLE_MIDI_SEND 2         // send byte to piano

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SNES_MIRACLE = &device_creator<snes_miracle_device>;


MACHINE_CONFIG_FRAGMENT( snes_miracle )
	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(WRITELINE(snes_miracle_device, rx_w))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
MACHINE_CONFIG_END

machine_config_constructor snes_miracle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_miracle );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void snes_miracle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_STROBE_ON)
	{
		m_strobe_clock++;
	}
	else
	{
		device_serial_interface::device_timer(timer, id, param, ptr);
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_miracle_device - constructor
//-------------------------------------------------

snes_miracle_device::snes_miracle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SNES_MIRACLE, "Miracle Piano SNES Cable", tag, owner, clock, "snes_miracle", __FILE__),
					device_serial_interface(mconfig, *this),
					device_snes_control_port_interface(mconfig, *this),
					m_midiin(*this, "mdin"),
					m_midiout(*this, "mdout"), strobe_timer(nullptr), m_strobe_on(0), m_midi_mode(0), m_sent_bits(0), m_strobe_clock(0),
	m_data_sent(0), m_xmit_read(0), m_xmit_write(0), m_recv_read(0), m_recv_write(0), m_tx_busy(false), m_read_status(false), m_status_bit(false)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void snes_miracle_device::device_start()
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

void snes_miracle_device::device_reset()
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
	m_recv_read = m_recv_write = 0;
	m_read_status = m_status_bit = false;
	m_tx_busy = false;
}

UINT8 snes_miracle_device::read_pin4()
{
	UINT8 ret = 0;

	if (m_midi_mode == MIRACLE_MIDI_RECEIVE)
	{
		if (m_status_bit)
		{
			m_status_bit = false;
			ret = (m_read_status) ? 1 : 0;
		}
		else
		{
			ret = (m_data_sent & 0x80) ? 0 : 1;
			m_data_sent <<= 1;
		}
	}

	return ret;
}

void snes_miracle_device::write_pin6(UINT8 data)
{
//  printf("%02x to pin6\n", data);
}

void snes_miracle_device::write_strobe(UINT8 data)
{
//  printf("%02x to strobe\n", data);

	if (m_midi_mode == MIRACLE_MIDI_SEND)
	{
		// console writes (data & 1) to Miracle Piano.
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

			if (m_strobe_clock < 500 && data == 0)
			{
				// short delay is receive mode
				m_midi_mode = MIRACLE_MIDI_RECEIVE;
				strobe_timer->reset();
				m_strobe_on = 0;
				m_strobe_clock = 0;

				m_status_bit = true;
				if (m_recv_read != m_recv_write)
				{
//                  printf("Getting %02x from Miracle[%d]\n", m_recvring[m_recv_read], m_recv_read);
					m_data_sent = m_recvring[m_recv_read++];
					if (m_recv_read >= RECV_RING_SIZE)
					{
						m_recv_read = 0;
					}
					m_read_status = true;
				}
				else
				{
					m_read_status = false;
//                  printf("Miracle has no data\n");
				}
				return;
			}
			else if (m_strobe_clock >= 500)
			{
				// more than ~520 clocks since strobe on write means send mode
				// (ranges from 522-528 have been seen)
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

void snes_miracle_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	UINT8 rcv = get_received_char();

//  printf("Got %02x -> [%d]\n", rcv, m_recv_write);
	m_recvring[m_recv_write++] = rcv;
	if (m_recv_write >= RECV_RING_SIZE)
	{
		m_recv_write = 0;
	}
}

void snes_miracle_device::tra_complete()    // Tx completed sending byte
{
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

void snes_miracle_device::tra_callback()    // Tx send bit
{
	UINT8 bit = transmit_register_get_data_bit();

	// send this to midi out
	m_midiout->write_txd(bit);
}

void snes_miracle_device::xmit_char(UINT8 data)
{
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
