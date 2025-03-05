// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
/**
    MB89372
    Fujitsu
    Multi-Protocol Controller
 **/

/*
    registers are based on guesswork!
    port 00-0f - SIU A
    port 10-1f - SIU B
    port 20-2f - DMA A-D
        20,21,22 chan a address (siu-a rx)
        23       chan a command/status
        24,25,26 chan b address (siu-a tx)
        27       chan b command/status
        28,29,2a chan c address (siu-b rx)
        2b       chan c command/status
        2c,2d,2e chan d address (siu-b tx)
        2f       chan d command/status
    port 30-3f - INT?
*/

#include "emu.h"
#include "mb89372.h"
#include "emuopts.h"

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB89372, mb89372_device, "mb89372", "Fujitsu MB89372 Multi-Protocol Controller")

//-------------------------------------------------
//  mb89372_device - constructor
//-------------------------------------------------

mb89372_device::mb89372_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock ) :
	device_t(mconfig, MB89372, tag, owner, clock),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_hack(0),
	m_out_hreq_cb(*this),
	m_out_irq_cb(*this),
	m_in_memr_cb(*this, 0),
	m_out_memw_cb(*this)
{
	// prepare "filenames"
	m_localhost = util::string_format("socket.%s:%s", mconfig.options().comm_localhost(), mconfig.options().comm_localport());
	m_remotehost = util::string_format("socket.%s:%s", mconfig.options().comm_remotehost(), mconfig.options().comm_remoteport());
}


//-------------------------------------------------
//  device_start - device-specific startup
//------------------------------------------------

void mb89372_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	for(auto &elem : m_channel)
	{
		elem.m_address = 0;
		elem.m_count = 0;
		elem.m_base_address = 0;
		elem.m_base_flags = 0;
		elem.m_state = 0;
	}

	// state saving
	save_item(NAME(m_hreq));
	save_item(NAME(m_hack));
	save_item(NAME(m_irq));
	save_item(NAME(m_reg));

	save_item(STRUCT_MEMBER(m_channel, m_address));
	save_item(STRUCT_MEMBER(m_channel, m_count));
	save_item(STRUCT_MEMBER(m_channel, m_base_address));
	save_item(STRUCT_MEMBER(m_channel, m_base_flags));
	save_item(STRUCT_MEMBER(m_channel, m_state));

	save_item(NAME(m_current_channel));
	save_item(NAME(m_last_channel));

	save_item(NAME(m_intr_delay));
	save_item(NAME(m_sock_delay));

	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rx_offset));
	save_item(NAME(m_rx_length));

	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_offset));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb89372_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	set_hreq(0);
	m_hack = 0;
	set_irq(0);

	m_current_channel = -1;
	m_last_channel = 3;

	m_dma_delay = 0;
	m_intr_delay = 0;
	m_sock_delay = 0x20;

	m_rx_length = 0x0000;
	m_rx_offset = 0x0000;

	m_tx_offset = 0x0000;
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mb89372_device::execute_run()
{
	while (m_icount > 0)
	{
		// TODO waste some cycles before triggering ints
		if (m_intr_delay > 0)
		{
			m_intr_delay--;
		}
		else
		{
			//m_intr_delay = 0x04;
			check_ints();
		}

		if (m_sock_delay > 0)
		{
			m_sock_delay--;
		}
		else
		{
			m_sock_delay = 0x20;
			check_sockets();
		}

		if (m_dma_delay > 0)
		{
			m_dma_delay--;
		}
		else
		{
			check_dma();
		}

		m_icount--;
	}
}


//-------------------------------------------------
//  read - handler for register reading
//-------------------------------------------------

uint8_t mb89372_device::read(offs_t offset)
{
	uint8_t data = 0xff;
	switch (offset & 0x3f)
	{
		case 0x08:
			data = (m_rx_length > 0) ? 1 : 0;
			break;

		case 0x0f:
			data = rx_read();
			break;

		default:
			data = m_reg[offset & 0x3f];
			if (!machine().side_effects_disabled())
				logerror("MB89372 unimplemented register read @%02X\n", offset);
	}
	return data;
}


//-------------------------------------------------
//  write - handler for register writing
//-------------------------------------------------

void mb89372_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x3f)
	{
		case 0x20:
		case 0x21:
		case 0x22:
			// dma A desc pointer
		case 0x23:
			// dma A command register?
			m_reg[offset & 0x3f] = data;
			break;

		case 0x24:
		case 0x25:
		case 0x26:
			// dma B desc pointer
		case 0x27:
			// dma B command register?
			m_reg[offset & 0x3f] = data;
			break;

		default:
			m_reg[offset & 0x3f] = data;
			logerror("MB89372 unimplemented register write @%02X = %02X\n", offset, data);
	}
	trigger(1);
}


//-------------------------------------------------
//  hack_w - hold acknowledge
//-------------------------------------------------

void mb89372_device::hack_w(int state)
{
	m_hack = state;
	trigger(1);
}


//**************************************************************************
//  int logic
//**************************************************************************
void mb89372_device::check_ints()
{
	int active = (m_reg[0x23] & 0x01) | (m_reg[0x27] & 0x01);
	set_irq(active);
}


//**************************************************************************
//  dma logic
//**************************************************************************
void mb89372_device::check_dma()
{
	if (m_current_channel != -1)
	{
		int reg = 0x20 + (m_current_channel * 4);
		int addr = (m_reg[reg + 2] << 16) | (m_reg[reg + 1] << 8) | m_reg[reg];
		switch (m_channel[m_current_channel].m_state)
		{
			case 0:
				// inactive
				m_current_channel = -1;
				break;

			case 1:
				// waiting for hack
				set_hreq(1);
				if (m_hack)
				{
					m_channel[m_current_channel].m_state = 2;
				}
				break;

			case 2:
				// read command
				m_channel[m_current_channel].m_base_address = addr;
				m_channel[m_current_channel].m_count = (m_in_memr_cb(addr + 1) << 8) | m_in_memr_cb(addr);
				m_channel[m_current_channel].m_address = (m_in_memr_cb(addr + 4) << 16) | (m_in_memr_cb(addr + 3) << 8) | m_in_memr_cb(addr + 2);
				m_channel[m_current_channel].m_base_flags = m_in_memr_cb(addr + 5);

				m_dma_delay = 6*4;
				m_channel[m_current_channel].m_state = 3;
				logerror("read %01x\n", m_current_channel);

				break;

			case 3:
				// check command
				if (m_current_channel == 0)
				{
					// rx channel
					if (m_channel[m_current_channel].m_count > m_rx_length)
					{
						logerror("not enough data for %01x\n", m_current_channel);
						m_current_channel = -1;
						set_hreq(0);
					}
					else
					{
						m_channel[m_current_channel].m_state = 4;
					}
				}
				else
				{
					// tx channel
					m_channel[m_current_channel].m_state = 5;
				}
				break;

			case 4:
				// execute rx
				set_hreq(1);
				if (m_hack)
				{
					if (m_channel[m_current_channel].m_count > 0)
					{
						m_out_memw_cb(m_channel[m_current_channel].m_address, rx_read());
						m_channel[m_current_channel].m_address++;
						m_channel[m_current_channel].m_count--;
						m_dma_delay = 4;
						logerror("rx %01x\n", m_current_channel);
					}
					else
					{
						m_channel[m_current_channel].m_state = 6;
						logerror("rc %01x\n", m_current_channel);
					}
				}
				break;

			case 5:
				// execute tx
				set_hreq(1);
				if (m_hack)
				{
					if (m_channel[m_current_channel].m_count > 0)
					{
						tx_write(m_in_memr_cb(m_channel[m_current_channel].m_address));
						m_channel[m_current_channel].m_address++;
						m_channel[m_current_channel].m_count--;
						m_dma_delay = 4;
						logerror("tx %01x\n", m_current_channel);
					}
					else
					{
						tx_complete();
						m_channel[m_current_channel].m_state = 6;
						logerror("tc %01x\n", m_current_channel);
					}
				}
				break;

			case 6:
				m_out_memw_cb(m_channel[m_current_channel].m_base_address + 5, m_channel[m_current_channel].m_base_flags | 0x60);
				if (m_channel[m_current_channel].m_base_flags & 0x10)
				{
					// continue chain
					m_reg[reg] = m_in_memr_cb(m_channel[m_current_channel].m_base_address + 9);
					m_reg[reg + 1] = m_in_memr_cb(m_channel[m_current_channel].m_base_address + 10);
					m_reg[reg + 2] = m_in_memr_cb(m_channel[m_current_channel].m_base_address + 11);
					m_channel[m_current_channel].m_state = 2;
					logerror("continue %01x\n", m_current_channel);
				}
				else
				{
					// chain complete
					m_reg[reg + 3] |= 0x01; // enable int flag?
					m_reg[reg + 3] &= 0x7f; // disable active flag?
					m_channel[m_current_channel].m_state = 0;
					logerror("completed %01x\n", m_current_channel);
					m_current_channel = -1;
					set_hreq(0);
				}
				break;

			default:
					logerror("unknown state?! %01x = %02x\n", m_current_channel, m_channel[m_current_channel].m_state);
				break;
		}
	}
	else
	{
		for (int channel = 3; channel >= 0; channel--)
		{
			if (m_channel[channel].m_state > 0)
			{
				m_current_channel = channel;
				return;
			}
			int reg = 0x20 + (channel * 4);
			if ((m_channel[channel].m_state == 0) && (m_reg[reg + 3] & 0x80))
			{
				m_channel[channel].m_state = 1;
				m_current_channel = channel;
				logerror("meep... %01x\n", channel);
				return;
			}
		}
	}
}


//**************************************************************************
//  buffer logic
//**************************************************************************

void mb89372_device::rx_reset()
{
	m_rx_length = 0;
	m_rx_offset = 0;
}

uint8_t mb89372_device::rx_read()
{
	uint8_t data = m_rx_buffer[m_rx_offset];
	m_rx_offset++;

	/*
	if (m_rx_offset == m_rx_length)
	    m_rxsr0 |= 0x40; // EOF
	*/

	if (m_rx_offset >= m_rx_length)
		rx_reset();
	return data;
}

void mb89372_device::tx_reset()
{
	m_tx_offset = 0;
	//m_txsr |= 0x05;
}

void mb89372_device::tx_write(uint8_t data)
{
	m_tx_buffer[m_tx_offset] = data;
	m_tx_offset++;
	//m_txsr = 0x6b;

	// prevent overflow
	if (m_tx_offset >= 0x0f00)
		m_tx_offset = 0x0eff;
}

void mb89372_device::tx_complete()
{
	if (m_tx_offset > 0)
	{
		if (m_line_rx && m_line_tx)
		{
			m_socket_buffer[0x00] = m_tx_offset & 0xff;
			m_socket_buffer[0x01] = (m_tx_offset >> 8) & 0xff;
			for (int i = 0x00 ; i < m_tx_offset ; i++)
			{
				m_socket_buffer[i + 2] = m_tx_buffer[i];
			}

			uint32_t dataSize = m_tx_offset + 2;
			uint32_t written;

			m_line_tx->write(&m_socket_buffer, 0, dataSize, written);
		}
	}

	//m_txsr = 0x6f;

	tx_reset();
}

void mb89372_device::check_sockets()
{
	// check rx socket
	if (!m_line_rx)
	{
		osd_printf_verbose("MB89372: rx listen on %s\n", m_localhost);
		uint64_t filesize; // unused
		std::error_condition filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
		if (filerr.value() != 0)
		{
			osd_printf_verbose("MB89372: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
			m_line_rx.reset();
		}
	}

	// check tx socket
	if (!m_line_tx)
	{
		osd_printf_verbose("MB89372: tx connect to %s\n", m_remotehost);
		uint64_t filesize; // unused
		std::error_condition filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
		if (filerr.value() != 0)
		{
			osd_printf_verbose("MB89372: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
			m_line_tx.reset();
		}
	}

	if (m_line_rx && m_line_tx)
	{
		// RXCR_RXE
		if (true)
		{
			if (m_rx_length == 0)
			{
				uint32_t recv = 0;
				m_line_rx->read(m_socket_buffer, 0, 2, recv);
				if (recv > 0)
				{
					if (recv == 2)
						m_rx_length = m_socket_buffer[0x01] << 8 | m_socket_buffer[0x00];
					else
					{
						m_rx_length = m_socket_buffer[0x00];
						m_line_rx->read(m_socket_buffer, 0, 1, recv);
						while (recv == 0) {}
						m_rx_length |= m_socket_buffer[0x00] << 8;
					}

					int offset = 0;
					int togo = m_rx_length;
					while (togo > 0)
					{
						m_line_rx->read(m_socket_buffer, 0, togo, recv);
						for (int i = 0x00 ; i < recv ; i++)
						{
							m_rx_buffer[offset] = m_socket_buffer[i];
							offset++;
						}
						togo -= recv;
					}

					m_rx_offset = 0;
					//m_rxsr0 = 0x01; // RXRDY

					/*
					if (m_rx_offset + 1 == m_rx_length)
					    m_rxsr0 |= 0x40; // EOF
					*/

					//m_rxsr1 = 0xc8;

					//set_po3(!MASKR_MRXDRQ && RXIER_RXDI ? 1 : 0);
				}
			}
		}
	}
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void mb89372_device::set_hreq(int state)
{
	if (m_hreq != state)
	{
		m_out_hreq_cb(state);
		m_hreq = state;
	}
}

inline void mb89372_device::set_irq(int state)
{
	if (m_irq != state)
	{
		m_out_irq_cb(state);
		m_irq = state;
	}
}
