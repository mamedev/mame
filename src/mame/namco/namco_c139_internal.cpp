// license:BSD-3-Clause
// copyright-holders:Angelo Salese, John Bennett, Ariane Fugmann
/***************************************************************************

	Namco C139 - Serial I/F Controller

	(from assault schematics, page 5-18 and 5-19)
	connected to M5M5179P RAM with a 13-bit address bus, and 9 bit data bus
	connected to host cpu with a 14*-bit address bus, and 13 bit data bus
	2 clock inputs - 16M and 12M
	currently there are 4 known modes of operation:

	mode 0x08:
	- ridgera2
	- raverace

	mode 0x09:
	- fourtrax
	- suzuka8h
	- suzuk8h2
	- winrungp
	- winrun91
	- driveyes (center)
	- cybsled
	- cybrcomm
	- acedrive
	- victlap
	- cybrcycc
	- adillor

	mode 0x0c:
	- ridgeracf

	mode 0x0d:
	- finallap
	- finallap2
	- finallap3
	- driveyes (sides)
	- tokyowar
	- aircomb
	- dirtdash
	- alpiner2b (uses 0xfd)

	mode 0x0f (configuration mode)
	- 0x02 used to setup byte/word adressing

	NOTES:
	  apparently mode 0x09 and 0x0d modify the received data.
	  mode 0x09 does not update *anything* after data got changed. might be automatic.
	  mode 0x0d updates the tx offset pointing to the rx buffer (which is not supported right now)

	TODO:
	- hook a real chip and test in detail
	- mode 0x0d shows 1 machine in service mode and attract mode, however most games seem to work "okay" in multiplayer.
	- C422 seems to be a pin compatible upgrade to C139, probably supporting higher clock speeds? hook up c139 in system23
	- mode 0x0b is used by s23 games to test interrupts.

***************************************************************************/

#include "emu.h"
#include "namco_c139_internal.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

//DO NOT MERGE ME - need to do it properly
#define GALAXIAN3HACK

#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(NAMCO_C139_LOCAL, namco_c139_local_device, "namco_c139_local", "Namco C139 Serial (internal)")

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void namco_c139_local_device::data_map(address_map& map)
{
	map(0x0000, 0x3fff).rw(FUNC(namco_c139_local_device::ram_r), FUNC(namco_c139_local_device::ram_w));
}

void namco_c139_local_device::regs_map(address_map& map)
{
	map(0x00, 0x0f).rw(FUNC(namco_c139_local_device::reg_r), FUNC(namco_c139_local_device::reg_w));
}


//-------------------------------------------------
//  namco_c139_local_device - constructor
//-------------------------------------------------

namco_c139_local_device::namco_c139_local_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C139_LOCAL, tag, owner, clock),
	m_irq_cb(*this)
{
//	auto const& opts = mconfig.options();

	
	// come up with some magic number for identification
	m_linkid = 0;
	
	LOG("C139: ID byte = %02d\n", m_linkid);

	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c139_local_device::device_start()
{
	m_tick_timer = timer_alloc(FUNC(namco_c139_local_device::tick_timer_callback), this);
	m_tick_timer->adjust(attotime::never);


	// state saving
	save_item(NAME(m_ram));
	save_item(NAME(m_reg));

	save_item(NAME(m_linktimer));
	save_item(NAME(m_linkid));

	save_item(NAME(m_txsize));
	save_item(NAME(m_txblock));
	save_item(NAME(m_reg_f3));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c139_local_device::device_reset()
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	//m_context->reset(m_localhost, m_localport, m_remotehost, m_remoteport);

	m_linktimer = 0x0200;

	m_txsize = 0x00;
	m_txblock = 0x00;
	m_reg_f3 = 0x00;

	m_rx_bytes = 0;
	m_empty = 0;
	m_comms_counter = 0;


//	m_tick_timer->adjust(attotime::from_hz(60*4), 0, attotime::from_hz(60*4));
	m_tick_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));


}

void namco_c139_local_device::device_stop()
{
	m_tick_timer->adjust(attotime::never);

}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t namco_c139_local_device::ram_r(offs_t offset)
{
	return m_ram[offset & 8191];
}

void namco_c139_local_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[offset & 8191]);
	m_txsize = offset;
}

uint16_t namco_c139_local_device::reg_r(offs_t offset)
{
	uint16_t result = m_reg[offset];

	return result;
}

void namco_c139_local_device::reg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_reg[offset] = data;


	if (offset == REG_2_CONTROL && data == 1)
	{
		m_txsize = 0; // autocount mode
	}

	if (offset == REG_3_START && data == 1)
	{
		m_txblock = 0;
		
	
		if (m_linkid <= 4 )

		{
			send_data_autotransmit();  //Disabling auto transferring of SCI 25_2_26
		
		}
		else
	
		{
			comm_tick();
		}
	}

	// mode 08 & 0c tx trigger
	if (offset == REG_2_CONTROL && data == 0x03)
		m_txblock = 0x00;


}


/*
   Set up a point to point internal link
   For games consisting of multiple PCB sets that don't require actual networking
   Specify the device this PCB will receive from
   This can allow multiple PCBs to receive from a single transmitter

*/
void namco_c139_local_device::set_rx_ptr(uint8_t* buffer_ptr, uint16_t* count_ptr)
{
	m_rx_buffer_ptr = buffer_ptr;

	m_rx_bytes_ptr = count_ptr;

	*m_rx_bytes_ptr = 0;

}

void namco_c139_local_device::get_rx_buffer(uint8_t** buffer_ptr, uint16_t** count_ptr)
{
	*buffer_ptr = &m_buffer[0];

	*count_ptr = &m_rx_bytes;
}



TIMER_CALLBACK_MEMBER(namco_c139_local_device::tick_timer_callback)
{
	comm_tick();
}

void namco_c139_local_device::comm_tick()
{


	unsigned data_size = 0x100;


	switch (m_reg[REG_1_MODE])
	{

	case 0x08:
		// ridgera2, raverace
		// 0b1000
		// reg2 - 1 > write mem > 3 > 1 > write mem > 3 etc.
		// txcount NOT cleared on send
		read_data(data_size);
		if (m_reg[REG_2_CONTROL] == 0x03 && m_reg[REG_5_TXSIZE] > 0x00)
		{
			send_data(data_size);
		}
		break;

	case 0x04: //JB HACK
		read_data(data_size);
		send_data(data_size);
		m_reg[REG_1_MODE] = 0;
		break;
	case 0x09:
		// suzuka8h, acedrive, winrungp, cybrcycc, driveyes (center)
		// 0b1001 - auto-send via sync bit (and auto offset)
		read_data(data_size);
		send_data(data_size);
		break;
	case 0x0c:
		// ridgeracf
		// 0b1100 - send by register / txwords
		// txcount IS cleared on send
		read_data(data_size);
		if (m_reg[REG_2_CONTROL] == 0x03 && m_reg[REG_3_START] == 0x00)
			send_data(data_size);
		break;

	case 0x0d:
		// final lap, driveyes (left & right)
		// 0b1101 - auto-send via register?
		read_data(data_size);
		if (m_reg[REG_3_START] == 0x00 && m_reg[REG_5_TXSIZE] > 0x00)
		{
			send_data(data_size);
		}
		break;

	case 0x0f:
		// init / reset
		break;

	default:
		// unknown mode
		read_data(data_size); // Should this be here?

		break;
	}

}


void namco_c139_local_device::read_data(unsigned data_size)
{
	if (m_reg[REG_0_STATUS] != 0x06)
	{

		// try to read a message
		unsigned recv = read_frame(data_size);


		if (recv > 0)
		{
			// save message to "rx buffer"
			unsigned rx_size = (m_buffer[2] << 8) | m_buffer[1];
			unsigned rx_offset = m_reg[REG_6_RXOFFSET]; // rx offset in words

			if (m_linkid < 4)
			{
				rx_offset = m_reg[REG_6_RXOFFSET] & 0xFF00; // Hack so we always put at 0x2000
			}

			unsigned buf_offset = 3;
			for (unsigned j = 0x00; j < rx_size; j++)
			{
				uint16_t data = (m_buffer[buf_offset + 1] << 8) | m_buffer[buf_offset];
				m_ram[0x1000 + (rx_offset & 0x0fff)] = data;
				rx_offset++;
				buf_offset += 2;
				if (buf_offset > 510)
				{
					buf_offset = 510;

					rx_size = 0;
					return;

				}
			}

			// relay messages
			if (m_reg[REG_1_MODE] == 0x09)
			{
				m_reg[REG_5_TXSIZE] = 0x00;
			}
			// update regs
			m_reg[REG_0_STATUS] = 0x06;

			
			if (m_linkid == 0x40) // Sound PCB
			{
				m_reg[REG_4_RXSIZE] = -rx_size-1;
			}
			else if (m_linkid > 4)
			{
				if (m_reg[REG_1_MODE] != 0x0d)
					m_reg[REG_4_RXSIZE] += rx_size;
				else
					m_reg[REG_4_RXSIZE] -= rx_size;

				m_reg[REG_6_RXOFFSET] += rx_size;
			}
	
			else
			{
				//RSO to PSN
			//	WRONG!!!
				m_reg[REG_4_RXSIZE] = ( - rx_size - 1) & 0xff;
				m_reg[REG_6_RXOFFSET] = rx_size;


			}
			// prevent overflow
			m_reg[REG_4_RXSIZE] &= 0x0fff;
			m_reg[REG_6_RXOFFSET] &= 0x0fff;



#ifdef GALAXIAN3HACK

			//Only do this strange hack when RSO transmitting
			m_reg[REG_6_RXOFFSET] |= 0x1000;

#endif
			
			// fire interrupt
			if (rx_size)
			{
				m_irq_cb(ASSERT_LINE);

			}
		}
	}
	else
	{

		m_irq_cb(ASSERT_LINE);
	}
}

void namco_c139_local_device::send_data(unsigned data_size)
{
	int tx_size = 0;
	unsigned tx_mask = 0x0fff;
	
	if (m_txblock == 0x01)
		return;


	unsigned tx_offset = m_reg[REG_7_TXOFFSET];
	if (m_reg[REG_1_MODE] == 0xd)
	{
		tx_size = m_reg[REG_5_TXSIZE];
	}
	else
	{
		tx_size = m_txsize + 1; // autocount
	}



	if (tx_size == 0)
		return;

	m_buffer[0] = m_linkid;
	m_buffer[1] = tx_size & 0xff;
	m_buffer[2] = (tx_size & 0xff00) >> 8;
	m_buffer[0x1ff] = 1;

	unsigned buf_offset = 3;
	for (unsigned j = 0x00; j < tx_size; j++)
	{
		m_buffer[buf_offset] = m_ram[tx_offset & tx_mask] & 0xff;
		m_buffer[buf_offset + 1] = 0;

		tx_offset++;
		buf_offset += 2;
	}

	// set bit-8 on last byte
	m_buffer[buf_offset - 1] |= 0x01;

	// based on mode, reset tx counter
	switch (m_reg[REG_1_MODE])
	{

	case 0x08:
	case 0x09:
		// do nothing
		m_txblock = 0x01;
		break;
	case 0x0c:
	case 0x0d:
		m_reg[REG_5_TXSIZE] = 0;
		m_txblock = 0x01;
		break;
	default:
		// do nothing
		m_txblock = 0x01; //JB
		break;
	}

	m_txsize = 0;
	send_frame(data_size);

}


void namco_c139_local_device::send_data_autotransmit(void )
{
	for (int y = 0; y < 40; y++)
	{
		m_buffer[y] = y;
	}
	unsigned tx_mask = 0x0fff;
	
	unsigned tx_offset = 0;

	unsigned buf_offset = 3;

	m_txsize++;//Didn't we add one earler?

	m_buffer[0] = m_linkid;
	m_buffer[1] = m_txsize & 0xff;
	m_buffer[2] = (m_txsize & 0xff00) >> 8;
	m_buffer[0x1ff] = 1;

	for (unsigned j = 0x00; j < m_txsize; j++)
	{
		m_buffer[buf_offset] = m_ram[tx_offset & tx_mask] & 0xff;
		m_buffer[buf_offset + 1] = 0;//77;

		tx_offset++;
		buf_offset += 2;
	}


	

	//TODO = silly big - send what you need instead
	send_frame(0x100);

}


void namco_c139_local_device::send_frame(unsigned data_size)
{

		m_comms_counter++;
		m_rx_bytes = (data_size << 4) | m_comms_counter;

}


unsigned namco_c139_local_device::read_frame(unsigned data_size)
{
	
		unsigned bytes_read = 0;
		uint16_t rx_bytes = *m_rx_bytes_ptr;
		uint16_t comms_counter = rx_bytes & 0xF;

		rx_bytes = rx_bytes >> 4;

		if ((rx_bytes) && (comms_counter != m_rx_comms_counter))
		{
			bytes_read = rx_bytes;
			if (bytes_read > 511)
			{
				bytes_read = 511;
				osd_printf_verbose("Overflow. bytes_read = %04x \n", bytes_read );

			//	return (0);
			}
			for (unsigned x = 0; x < bytes_read; x++)
			{
				//Copy data across
				m_buffer[x] = m_rx_buffer_ptr[x];

			}

			m_rx_comms_counter = comms_counter;

		}
		return bytes_read;	

}
