// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Epoch Barcode Battler

    TODO: this should be actually emulated as a standalone system with
    a few 7segments LEDs, once we get a dump of its BIOS
    At the moment we only emulated the connection with a Famicom

**********************************************************************/

#include "bcbattle.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_BARCODE_BATTLER = &device_creator<nes_bcbattle_device>;


MACHINE_CONFIG_FRAGMENT( nes_battler )
	MCFG_BARCODE_READER_ADD("battler")
MACHINE_CONFIG_END

machine_config_constructor nes_bcbattle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( nes_battler );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

// This part is the hacky replacement for the real Barcode unit [shared with SNES implementation]:
// code periodically checks whether a new code has been scanned and it moves it to the
// m_current_barcode array
void nes_bcbattle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_BATTLER)
	{
		int old = m_new_code;
		// has something new been scanned?
		if (old < m_reader->get_pending_code())
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
		m_new_code = m_reader->get_pending_code();
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_bcbattle_device - constructor
//-------------------------------------------------

nes_bcbattle_device::nes_bcbattle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_BARCODE_BATTLER, "Epoch Barcode Battler (FC)", tag, owner, clock, "nes_bcbattle", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_reader(*this, "battler"), m_pending_code(0), m_new_code(0), m_transmitting(0), m_cur_bit(0), m_cur_byte(0), battler_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_bcbattle_device::device_start()
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

void nes_bcbattle_device::device_reset()
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

int nes_bcbattle_device::read_current_bit()
{
	if (m_pending_code && !m_transmitting)
	{
		// we start with 1
		m_transmitting = 1;
		m_cur_byte = 0;
		m_cur_bit = 0;
		return 1;
	}

	if (m_transmitting)
	{
		if (m_cur_bit == 0)
		{
			m_cur_bit++;
			return 1;
		}
		if (m_cur_bit < 9)
		{
			int bit = (BIT(m_current_barcode[m_cur_byte], m_cur_bit - 1)) ^ 1;
			m_cur_bit++;
			return bit;
		}
		if (m_cur_bit == 9)
		{
			m_cur_bit = 0;
			//printf("%X ", m_current_barcode[m_cur_byte]);
			m_cur_byte++;
			if (m_cur_byte == 20)
			{
				m_cur_byte = 0;
				m_transmitting = 0;
				m_pending_code = 0;
			}
			return 0;
		}
	}

	return 0;
}

UINT8 nes_bcbattle_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 1)    //$4017
	{
		ret |= read_current_bit() << 2;
	}

	return ret;
}
