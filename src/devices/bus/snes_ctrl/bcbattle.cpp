// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom - Epoch Barcode Battler

    TODO: this should be actually emulated as a standalone system with
    a few 7segments LEDs, once we get a dump of its BIOS
    At the moment we only emulated the connection with a Super Famicom

**********************************************************************/

#include "bcbattle.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SNES_BARCODE_BATTLER = &device_creator<snes_bcbattle_device>;


MACHINE_CONFIG_FRAGMENT( snes_battler )
	MCFG_BARCODE_READER_ADD("battler")
MACHINE_CONFIG_END

machine_config_constructor snes_bcbattle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_battler );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

// This part is the hacky replacement for the real Barcode unit [shared with NES implementation]:
// code periodically checks whether a new code has been scanned and it moves it to the
// m_current_barcode array
void snes_bcbattle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_BATTLER)
	{
		int old = m_new_code;
		m_new_code = m_reader->get_pending_code();
		// has something new been scanned?
		if (old < m_new_code)
		{
			if (m_reader->get_byte_length() == 13)
			{
				for (int i = 0; i < 13; i++)
					m_current_barcode[i] = m_reader->read_code() + '0';
			}
			else if (m_reader->get_byte_length() == 8)
			{
				for (int i = 0; i < 5; i++)
					m_current_barcode[i] = 0x20;
				for (int i = 5; i < 13; i++)
					m_current_barcode[i] = m_reader->read_code() + '0';
			}
			// read one more, to reset the internal byte counter
			m_reader->read_code();

			// the string "SUNSOFT" is accepted as well by Barcode World
			m_current_barcode[13] = 'E';
			m_current_barcode[14] = 'P';
			m_current_barcode[15] = 'O';
			m_current_barcode[16] = 'C';
			m_current_barcode[17] = 'H';
			m_current_barcode[18] = 0x0d;
			m_current_barcode[19] = 0x0a;
			m_pending_code = 1;
		}
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_bcbattle_device - constructor
//-------------------------------------------------

snes_bcbattle_device::snes_bcbattle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SNES_BARCODE_BATTLER, "Epoch Barcode Battler (SFC)", tag, owner, clock, "snes_bcbattle", __FILE__),
					device_snes_control_port_interface(mconfig, *this),
					m_reader(*this, "battler"), m_pending_code(0), m_new_code(0), m_transmitting(0), m_cur_bit(0), m_cur_byte(0), battler_timer(nullptr), m_strobe(0), m_idx(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void snes_bcbattle_device::device_start()
{
	// lacking emulation of the standalone Barcode Battler, we refresh periodically the input from the reader
	// proper emulation would have the standalone unit acknowledging that a new barcode has been scanned
	// and sending the proper serial bits, instead of our read_current_bit() function!
	battler_timer = timer_alloc(TIMER_BATTLER);
	battler_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1000));

	save_item(NAME(m_current_barcode));
	save_item(NAME(m_new_code));
	save_item(NAME(m_pending_code));
	save_item(NAME(m_transmitting));
	save_item(NAME(m_cur_bit));
	save_item(NAME(m_cur_byte));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void snes_bcbattle_device::device_reset()
{
	m_pending_code = 0;
	m_new_code = 0;
	m_transmitting = 0;
	m_cur_bit = 0;
	m_cur_byte = 0;
	memset(m_current_barcode, 0, ARRAY_LENGTH(m_current_barcode));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

int snes_bcbattle_device::read_current_bit()
{
	if (m_pending_code)
	{
		if (m_cur_bit < 4)
		{
			int bit = BIT(m_current_barcode[m_cur_byte], m_cur_bit - 1);
			m_cur_bit++;
			return bit;
		}
		if (m_cur_bit == 4) // only the low nibble is transmitted (this is the main action of the BBII interface for SNES)
		{
			m_cur_bit = 0;
			//printf("%X ", m_current_barcode[m_cur_byte]);
			m_cur_byte++;
			if (m_cur_byte == 13)
			{
				m_cur_byte = 0;
				m_pending_code = 0;
			}
			return 0;
		}
	}

	return 0;
}


//-------------------------------------------------
//  poll
//-------------------------------------------------

void snes_bcbattle_device::port_poll()
{
	m_idx = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 snes_bcbattle_device::read_pin4()
{
	UINT8 ret = 0;

	if (m_idx >= 80)
		ret |= 0x00;
	else if (m_idx >= 28)   // scan actual barcode
	{
		ret |= read_current_bit();  // if no code is pending transmission, the function returns 0
		m_idx++;
	}
	else if (m_idx >= 25)   // unknown flags?
		m_idx++;
	else if (m_idx == 24)   // barcode present
	{
		ret |= m_pending_code;
		m_idx++;
	}
	else if (m_idx >= 12)   // controller ID
		ret |= BIT(0x7000, m_idx++);
	else    // first 12 bytes are unknown and probably always 0
		m_idx++;

	return ret;
}


//-------------------------------------------------
//  write
//-------------------------------------------------

void snes_bcbattle_device::write_strobe(UINT8 data)
{
	int old = m_strobe;
	m_strobe = data & 0x01;

	if (m_strobe < old) // 1 -> 0 transition
		port_poll();
}
