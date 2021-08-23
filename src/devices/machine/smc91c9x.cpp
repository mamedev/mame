// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles, Ted Green

***************************************************************************/

#include "emu.h"
#include "smc91c9x.h"
#include <sstream>
#include <iomanip>

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_GENERAL (1U << 0)
#define LOG_PACKETS (1U << 1)
#define LOG_TX      (1U << 2)
#define LOG_RX      (1U << 3)
#define LOG_FILTER  (1U << 4)
//#define VERBOSE (LOG_GENERAL | LOG_PACKETS | LOG_TX | LOG_RX | LOG_FILTER)
#include "logmacro.h"

#define DISPLAY_STATS       (0)

/* Ethernet register names */
static const char *const ethernet_regname[64] =
{
	"TCR", "EPH STATUS", "RCR", "COUNTER", "MIR", "MCR", "(0.6)", "BANK",
	"CONFIG", "BASE", "IA0-1", "IA2-3", "IA4-5", "GENERAL PURPOSE", "CONTROL", "BANK",
	"MMU COMMAND", "PNR ARR", "FIFO PORTS", "POINTER", "DATA", "DATA", "INTERRUPT", "BANK",
	"MT0-1", "MT2-3", "MT4-5", "MT6-7", "MGMT", "REVISION", "ERCV", "BANK",
	"(4.0)", "(4.1)", "(4.2)", "(4.3)", "(4.4)", "(4.5)", "(4.6)", "BANK",
	"(5.0)", "(5.1)", "(5.2)", "(5.3)", "(5.4)", "(5.5)", "(5.6)", "BANK",
	"(6.0)", "(6.1)", "(6.2)", "(6.3)", "(6.4)", "(6.5)", "(6.6)", "BANK",
	"(7.0)", "(7.1)", "(7.2)", "(7.3)", "(7.4)", "(7.5)", "(7.6)", "BANK"
};

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

DEFINE_DEVICE_TYPE(SMC91C94, smc91c94_device, "smc91c94", "SMC91C94 Ethernet Controller")

smc91c94_device::smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: smc91c9x_device(mconfig, SMC91C94, tag, owner, clock, dev_type::SMC91C94)
{
	m_num_ebuf = 18;
}

DEFINE_DEVICE_TYPE(SMC91C96, smc91c96_device, "smc91c96", "SMC91C96 Ethernet Controller")

smc91c96_device::smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: smc91c9x_device(mconfig, SMC91C96, tag, owner, clock, dev_type::SMC91C96)
{
	m_num_ebuf = 24;
}

smc91c9x_device::smc91c9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, dev_type device_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10.0f)
	, m_device_type(device_type)
	, m_num_ebuf(16)
	, m_irq_handler(*this)
	, m_link_unconnected(false)
{
}

const u8 smc91c9x_device::ETH_BROADCAST[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smc91c9x_device::device_start()
{
	// Allocate main buffer
	m_buffer = std::make_unique<u8[]>(ETHER_BUFFER_SIZE * m_num_ebuf);

	// TX timer
	m_tx_poll = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(smc91c9x_device::tx_poll), this));

	m_irq_handler.resolve_safe();

	// These registers don't get cleared on reset
	m_reg[B1_CONFIG] = 0x0030;   m_regmask[B1_CONFIG] = 0x17c6;
	m_reg[B1_BASE]   = 0x1866;   m_regmask[B1_BASE] = 0xfffe;

	m_reg[B1_IA0_1] = 0x0000;   m_regmask[B1_IA0_1] = 0xffff;
	m_reg[B1_IA2_3] = 0x0000;   m_regmask[B1_IA2_3] = 0xffff;
	m_reg[B1_IA4_5] = 0x0000;   m_regmask[B1_IA4_5] = 0xffff;

	// Revision and MIR is based on chip type
	m_regmask[B3_REVISION] = 0x0000;
	m_regmask[B0_MIR] = 0x0000;
	if (m_device_type == dev_type::SMC91C94)
	{
		m_reg[B3_REVISION] = 0x3345;
		m_reg[B0_MIR] = 0x1212;
	}
	else if (m_device_type == dev_type::SMC91C96)
	{
		m_reg[B3_REVISION] = 0x3346;
		m_reg[B0_MIR] = 0x1818;
	}
	else
		fatalerror("device_start: Unknown device type\n");

	/* register ide states */
	save_item(NAME(m_reg));
	save_item(NAME(m_regmask));
	save_item(NAME(m_irq_state));
	save_pointer(NAME(m_buffer), ETHER_BUFFER_SIZE * m_num_ebuf);
	save_item(NAME(m_sent));
	save_item(NAME(m_recd));
	save_item(NAME(m_alloc_rx));
	save_item(NAME(m_alloc_tx));
	save_item(NAME(m_tx_active));
	save_item(NAME(m_rx_active));
	save_item(NAME(m_tx_retry_count));
	save_item(NAME(m_rx_hash));
	save_item(NAME(m_loopback_result));

	// Circular FIFOs
	save_item(NAME(m_queued_tx));
	save_item(NAME(m_queued_tx_h));
	save_item(NAME(m_queued_tx_t));
	save_item(NAME(m_completed_tx));
	save_item(NAME(m_completed_tx_h));
	save_item(NAME(m_completed_tx_t));
	save_item(NAME(m_completed_rx));
	save_item(NAME(m_completed_rx_h));
	save_item(NAME(m_completed_rx_t));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smc91c9x_device::device_reset()
{
	m_irq_state = 0;

	m_sent = 0;
	m_recd = 0;

	m_tx_active = 0;
	m_rx_active = 0;
	m_tx_retry_count = 0;

	m_reg[B0_TCR]          = 0x0000;   m_regmask[B0_TCR]          = 0x3d87;
	m_reg[B0_EPH_STATUS]   = 0x4000;   m_regmask[B0_EPH_STATUS]   = 0x0000;
	m_reg[B0_RCR]          = 0x0000;   m_regmask[B0_RCR]          = 0xc307;
	m_reg[B0_COUNTER]      = 0x0000;   m_regmask[B0_COUNTER]      = 0x0000;
	m_reg[B0_MCR]          = 0x3300;   m_regmask[B0_MCR]          = 0x00ff;
	m_reg[B0_BANK]         = 0x3330;   m_regmask[B0_BANK]         = 0x0007;

	m_reg[B1_GENERAL_PURP] = 0x0000;   m_regmask[B1_GENERAL_PURP] = 0xffff;
	m_reg[B1_CONTROL]      = 0x0100;   m_regmask[B1_CONTROL]      = 0x68e7;

	m_reg[B2_MMU_COMMAND]  = 0x0000;   m_regmask[B2_MMU_COMMAND]  = 0x00e7;
	m_reg[B2_PNR_ARR]      = 0x8000;   m_regmask[B2_PNR_ARR]      = 0x00ff;
	m_reg[B2_FIFO_PORTS]   = 0x8080;   m_regmask[B2_FIFO_PORTS]   = 0x0000;
	m_reg[B2_POINTER]      = 0x0000;   m_regmask[B2_POINTER]      = 0xf7ff;
	m_reg[B2_DATA_0]       = 0x0000;   m_regmask[B2_DATA_0]       = 0xffff;
	m_reg[B2_DATA_1]       = 0x0000;   m_regmask[B2_DATA_1]       = 0xffff;
	m_reg[B2_INTERRUPT]    = 0x0004;   m_regmask[B2_INTERRUPT]    = 0x7f00;

	m_reg[B3_MT0_1]        = 0x0000;   m_regmask[B3_MT0_1]        = 0xffff;
	m_reg[B3_MT2_3]        = 0x0000;   m_regmask[B3_MT2_3]        = 0xffff;
	m_reg[B3_MT4_5]        = 0x0000;   m_regmask[B3_MT4_5]        = 0xffff;
	m_reg[B3_MT6_7]        = 0x0000;   m_regmask[B3_MT6_7]        = 0xffff;
	m_reg[B3_MGMT]         = 0x3030;   m_regmask[B3_MGMT]         = 0x0f0f;

	m_reg[B3_ERCV]         = 0x331f;   m_regmask[B3_ERCV]         = 0x009f;

	set_promisc(false);

	update_ethernet_irq();

	// Reset MMU
	mmu_reset();
}

void smc91c9x_device::mmu_reset()
{
	// Reset MMU allocations
	m_alloc_rx = 0;
	m_alloc_tx = 0;

	// The register defaults to the MEMORY SIZE upon reset or upon the RESET MMU command.
	m_reg[B0_MIR] &= 0xff;
	m_reg[B0_MIR] |= m_reg[B0_MIR] << 8;

	// Reset fifos.
	reset_tx_fifos();
	reset_completed_rx();

	update_ethernet_irq();
}

void smc91c9x_device::reset_tx_fifos()
{
	// Disable transmit timer
	m_tx_poll->enable(false);
	// Reset transmit queue
	reset_queued_tx();
	// Reset completion FIFOs
	reset_completed_tx();
}

bool smc91c9x_device::alloc_req(const int tx, int &packet_num)
{
	u32 curr_alloc = m_alloc_rx | m_alloc_tx;

	for (int index = 0; index < m_num_ebuf; index++)
	{
		if (!(curr_alloc & (1 << index)))
		{
			packet_num = index;
			if (tx)
				m_alloc_tx |= 1 << index;
			else
				m_alloc_rx |= 1 << index;
			return true;
		}
	}

	return false;
}

void smc91c9x_device::alloc_release(const int packet_num)
{
	int clear_mask = ~(1 << packet_num);
	if (!((m_alloc_tx | m_alloc_rx) & (1 << packet_num)))
	{
		logerror("alloc_release: Trying to release a non-allocated packet. packet_num: %02x alloc_tx: %04x alloc_rx: %04x\n",
			packet_num, m_alloc_tx, m_alloc_rx);
	}
	m_alloc_tx &= clear_mask;
	m_alloc_rx &= clear_mask;
}

/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    update_ethernet_irq - update the IRQ state
-------------------------------------------------*/

void smc91c9x_device::update_ethernet_irq()
{
	// Check tx completion fifo empty
	if (empty_completed_tx())
		m_reg[B2_INTERRUPT] &= ~EINT_TX;
	else
		m_reg[B2_INTERRUPT] |= EINT_TX;

	// Check rx completion fifo empty
	if (empty_completed_rx())
		m_reg[B2_INTERRUPT] &= ~EINT_RCV;
	else
		m_reg[B2_INTERRUPT] |= EINT_RCV;

	uint8_t const mask = m_reg[B2_INTERRUPT] >> 8;
	uint8_t const state = m_reg[B2_INTERRUPT] & 0xff;


	/* update the IRQ state */
	uint8_t new_state = mask & state;
	if (m_irq_state ^ new_state)
	{
		LOG("update_ethernet_irq: old: %02x new: %02x\n", m_irq_state, new_state);
		m_irq_state = new_state;
		m_irq_handler(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
	}
}


/*-------------------------------------------------
    update_stats - draw statistics
-------------------------------------------------*/

void smc91c9x_device::update_stats()
{
	if ( DISPLAY_STATS )
		popmessage("Sent:%d  Rec'd:%d", m_sent, m_recd);
}

/*-------------------------------------------------
dump_bytes - Print packet bytes
-------------------------------------------------*/

void smc91c9x_device::dump_bytes(u8 *buf, int length)
{
	if (VERBOSE & LOG_PACKETS)
	{
		std::stringstream ss_bytes;
		ss_bytes << std::hex << std::setfill('0');
		for (int i = 0; i < length; i++)
		{
			ss_bytes << std::setw(2) << (int) buf[i];
			// Send newline every 16 bytes and at the end
			if ((i & 0xf) == 0xf || i == length - 1)
			{
				LOGMASKED(LOG_PACKETS, "%s\n", ss_bytes.str());
				ss_bytes.str("");
			}
			else
				ss_bytes << " ";
		}
	}
}

/*-------------------------------------------------
address_filter - Filter the received packet
-------------------------------------------------*/

int smc91c9x_device::address_filter(u8 *buf)
{
	if (m_reg[B0_RCR] & PRMS)
	{
		// TODO: 91C94 doesn't receive it's own transmisson when not in full duplex
		LOGMASKED(LOG_FILTER, "address_filter accepted (promiscuous mode)\n");
		return ADDR_UNICAST;
	}
	else if (buf[0] & 1)
	{
		// broadcast
		if (!memcmp(ETH_BROADCAST, buf, 6))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted (broadcast) %02x-%02x-%02x-%02x-%02x-%02x\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

			return ADDR_BROADCAST;
		}

		// multicast
		/*
		* Multicast address matching is performed by computing the fcs crc of
		* the destination address, and then using the upper 6 bits as an index
		* into the 64-bit logical address filter.
		*/
		// Check for all multicast bit
		if (m_reg[B0_RCR] & ALMUL)
			return ADDR_MULTICAST;

		u32 const crc = util::crc32_creator::simple(buf, 6);
		// The hash is based on the top 6 MSBs of the CRC
		// The CRC needs to be inverted and reflected
		m_rx_hash = 0x0;
		for (int i = 0; i < 6; i++)
			m_rx_hash |= (((~crc) >> i) & 1) << (5 - i);
		u64 multicast_addr = *(u64*)&m_reg[B3_MT0_1];
		if (BIT(multicast_addr, m_rx_hash))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted (multicast address match) %02x-%02x-%02x-%02x-%02x-%02x\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

			return ADDR_MULTICAST;
		}
		LOGMASKED(LOG_FILTER, "address_filter rejected multicast  %02x-%02x-%02x-%02x-%02x-%02x crc: %08x hash: %02x multi: %16ullx\n",
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], crc, m_rx_hash, *(u64*)&m_reg[B3_MT0_1]);
	}
	else
	{
		// unicast
		if (!memcmp(&m_reg[B1_IA0_1], buf, 6))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted (physical address match)\n");

			return ADDR_UNICAST;
		}
	}
	return ADDR_NOMATCH;
}

/*-------------------------------------------------
recv_start_cb - Start receiving packet
    A return value of 0 will stop rx processing in dinetwork device
    Any other value will be sent to the recv_complete_cb
-------------------------------------------------*/

int smc91c9x_device::recv_start_cb(u8 *buf, int length)
{
	// check internal loopback
	if (m_reg[B0_TCR] & (EPH_LOOP | LOOP))
	{
		LOGMASKED(LOG_RX, "receive internal loopback mode, external packet discarded\n");

		return 0;
	}

	// discard bad length packets
	if (length < 64 || length > 256*6 - 6)
	{
		LOGMASKED(LOG_RX, "received bad length packet length %d discarded\n", length);

		return 0;
	}

	// Check for active transmission
	if (m_tx_active)
	{
		// TODO: Update collision counters
		LOGMASKED(LOG_RX, "transmit active COLLISION, rx packet length %d discarded\n", length);

		return 0;

	}

	return receive(buf, length);
}

/*-------------------------------------------------
receive - Receive data into buffer
    Returns the buffer packet number + 1 if successful
-------------------------------------------------*/

int smc91c9x_device::receive(u8 *buf, int length)
{
	// check receiver enabled
	if (!(m_reg[B0_RCR] & RXEN))
	{
		LOGMASKED(LOG_RX, "receive disabled, external packet discarded\n");

		return -1;
	}

	// address filter
	int filter = address_filter(buf);
	if (filter == ADDR_NOMATCH)
		return -1;

	LOGMASKED(LOG_RX, "receive packet length %d\n", length);
	dump_bytes(buf, length);

	// Try to request a packet number
	int packet_num;
	if (!alloc_req(0, packet_num))
	{
		logerror("recv_cb: Couldn't allocate memory for receive packet\n");
		return -2;
	}

	m_rx_active = 1;

	// build up the packet
	uint8_t *const packet = &m_buffer[packet_num * ETHER_BUFFER_SIZE];

	// Strip CRC
	if (m_reg[B0_RCR] & STRIP_CRC)
		length -= 4;

	// Copy received payload
	memcpy(&packet[4], buf, length);

	// Status word
	u16 *rx_status = (u16*)&packet[0];
	*rx_status = 0x0000;

	// set the broadcast flag
	if (filter == ADDR_BROADCAST)
		*rx_status |= BRODCAST;

	// set the multicast flag and hash
	if (filter == ADDR_MULTICAST)
	{
		*rx_status |= (m_rx_hash << 1) | MULTCAST;
	}

	// Calculate buffer length and set control byte
	u16 buf_length;

	if (length & 1)
	{
		// ODD Frame
		*rx_status |= ODDFRM;
		packet[length + 4] = EBUF_RX_ALWAYS | EBUF_ODD; // Control
		buf_length = length + 5;
	}
	else
	{
		packet[length + 4] = 0x00; // Pad
		packet[length + 5] = EBUF_RX_ALWAYS; // Control
		buf_length = length + 6;
	}

	// Set buffer length word
	*(u16*)&packet[2] = buf_length;

	return packet_num + 1;
}

/*-------------------------------------------------
recv_complete_cb - End of receive
-------------------------------------------------*/

void smc91c9x_device::recv_complete_cb(int result)
{
	if (result > 0)
	{
		// Push packet number to rx completion fifo
		push_completed_rx(result - 1);
	}
	// Couldn't allocate memory
	else if (result == -2)
	{
		m_reg[B2_INTERRUPT] |= EINT_ALLOC;
	}

	update_ethernet_irq();

	m_rx_active = 0;
}

/*-------------------------------------------------
    tx_poll - Starts transmit
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(smc91c9x_device::tx_poll)
{
	// Check for active RX and delay if necessary
	if (m_rx_active)
	{
		// TODO: Implement correct CSMA/CD algorithm
		m_tx_poll->adjust(attotime::from_usec(40));
		m_tx_retry_count++;
		LOGMASKED(LOG_TX, "tx_poll: Delaying TX due to active RX retry_count = %d\n", m_tx_retry_count);
	}
	// Check if TX is enabled and packet is queued
	else if ((m_reg[B0_TCR] & TXENA) && !empty_queued_tx())
	{
		// Reset retry count
		m_tx_retry_count = 0;

		// Get the packet number from the transmit fifo
		const int packet_num = curr_queued_tx();
		uint8_t *const tx_buffer = &m_buffer[packet_num * ETHER_BUFFER_SIZE];

		// Get the length and control fields from buffer
		u16 length = (*(u16*)&tx_buffer[2]) & 0x7ff;
		const u8 control = tx_buffer[length - 1];

		// Remove [pad], control
		if (control & EBUF_ODD)
			length -= 1;
		else
			length -= 2;

		// Add padding up to CRC area
		// take into account status & length removal (-4) and crc addtion (+4)
		while (length < 64 + 4 - 4 && (m_reg[B0_TCR] & PAD_EN))
			tx_buffer[length++] = 0x00;

		// Add CRC
		if (((control & EBUF_CRC) || !(m_reg[B0_TCR] & NOCRC)))
		{
			u32 crc = util::crc32_creator::simple(tx_buffer + 4, length - 4);

			tx_buffer[length++] = (crc >> 0) & 0xff;
			tx_buffer[length++] = (crc >> 8) & 0xff;
			tx_buffer[length++] = (crc >> 16) & 0xff;
			tx_buffer[length++] = (crc >> 24) & 0xff;
		}

		// Remove status, length
		length -= 4;

		// Reset the EPH register */
		m_reg[B0_EPH_STATUS] &= LINK_OK;

		// Send the frame
		m_tx_active = 1;
		m_tx_poll->enable(false);

		LOGMASKED(LOG_TX, "Start sending packet %d length = %d time: %s\n", packet_num, length, machine().scheduler().time().as_string());
		dump_bytes(&tx_buffer[4], length);

		// Write loopback data and save result
		if (m_reg[B0_TCR] & (EPH_LOOP | LOOP | FDUPLX))
			m_loopback_result = receive(&tx_buffer[4], length);
		else
			m_loopback_result = 0;

		// Local loopback isn't sent to cable
		//if ((m_reg[B0_TCR] & (EPH_LOOP | LOOP) || (get_interface() < 0 && (m_reg[B0_TCR] & FDUPLX))))
		if (m_reg[B0_TCR] & (EPH_LOOP | LOOP))
			send_complete_cb(length);
		else
			send(&tx_buffer[4], length, 4);

	}
}

/*-------------------------------------------------
send_complete_cb - Called after transmit complete
-------------------------------------------------*/

void smc91c9x_device::send_complete_cb(int result)
{
	m_sent++;
	update_stats();

	// Pop the packet number from the transmit fifo
	const int packet_num = pop_queued_tx();
	uint8_t *const tx_buffer = &m_buffer[packet_num * ETHER_BUFFER_SIZE];

	LOGMASKED(LOG_TX, "End sending packet %d result = %d time: %s\n", packet_num, result, machine().scheduler().time().as_string());

	/* update the EPH register */
	m_reg[B0_EPH_STATUS] |= TX_SUC;

	// Set LINK_OK in status
	if (0 && !(m_reg[B0_EPH_STATUS] & LINK_OK))
	{
		m_reg[B0_EPH_STATUS] |= LINK_OK;
		// Set a ethernet phy status interrupt
		m_reg[B2_INTERRUPT] |= EINT_EPH;
	}

	// Set Tx broadcast flag
	if (!memcmp(ETH_BROADCAST, &tx_buffer[4], 6))
		m_reg[B0_EPH_STATUS] |= LTX_BRD;

	// Check tx queued fifo empty
	if (empty_queued_tx())
		m_reg[B2_INTERRUPT] |= EINT_TX_EMPTY;

	// Set no-transmission flags
	if (m_link_unconnected)
	{
		//m_reg[B0_EPH_STATUS] &= ~LINK_OK;
		//m_reg[B0_EPH_STATUS] &= ~TX_SUC;

		// Set lost carrier
		if (m_reg[B0_TCR] & MON_CSN)
		{
			m_reg[B0_EPH_STATUS] |= LOST_CARR;
			// Clear Tx Enable on error
			m_reg[B0_TCR] &= ~TXENA;
		}

		// Set signal quality error
		if (m_reg[B0_TCR] & STP_SQET)
		{
			m_reg[B0_EPH_STATUS] |= SQET;
			// Clear Tx Enable on error
			m_reg[B0_TCR] &= ~TXENA;
		}

		// Set a ethernet phy status interrupt
		m_reg[B2_INTERRUPT] |= EINT_EPH;
	}

	// Update status in the transmit word
	*(u16*)&tx_buffer[0] = m_reg[B0_EPH_STATUS];

	// Push the packet number onto the tx completion fifo
	push_completed_tx(packet_num);

	update_ethernet_irq();

	// Loopback if loopback is set or fduplx is set
	if (m_loopback_result)
	{
		//int rx_result = receive(&tx_buffer[4], result);
		recv_complete_cb(m_loopback_result);
	}

	// If there is more packets to transmit then start the tx polling
	if ((m_reg[B0_TCR] & TXENA) && !empty_queued_tx())
	{
		m_tx_poll->adjust(attotime::from_usec(10));
	}

	m_tx_active = 0;
}

/*-------------------------------------------------
    process_command - handle MMU commands
-------------------------------------------------*/

void smc91c9x_device::process_command(uint16_t data)
{
	switch ((data >> 4) & 0xF)
	{
		case ECMD_NOP:
			LOG("   NOP\n");
			break;

		case ECMD_ALLOCATE:
			LOG("   ALLOCATE MEMORY FOR TX (%d)", (data & 7));
			{
				int packet_num;
				if (alloc_req(1, packet_num))
				{
					LOG(" packet_num = %02x\n", (packet_num));
					// Set ARR register
					m_reg[B2_PNR_ARR] &= ~0xff00;
					m_reg[B2_PNR_ARR] |= packet_num << 8;
					m_reg[B2_INTERRUPT] |= EINT_ALLOC;

					update_ethernet_irq();
				}
				else
				{
					logerror("ECMD_ALLOCATE: Couldn't allocate TX memory\n");
				}
			}
			break;

		case ECMD_RESET_MMU:
			LOG("   RESET MMU\n");
			mmu_reset();
			break;

		case ECMD_REMOVE_TOPFRAME_TX:
			LOG("   REMOVE FRAME FROM TX FIFO\n");
			if (empty_completed_tx())
				logerror("process_command: Trying to remove entry from empty tx completion fifo\n");
			else
				pop_completed_tx();
			break;

		case ECMD_REMOVE_RELEASE_TOPFRAME_RX:
			LOG("   REMOVE AND RELEASE FRAME FROM RX FIFO (PACK_NUM=%d)\n", curr_completed_rx());
			// Release memory allocation
			alloc_release(curr_completed_rx());
			[[fallthrough]];
		case ECMD_REMOVE_TOPFRAME_RX:
			LOG("   REMOVE FRAME FROM RX FIFO\n");
			// remove entry from rx completion queue
			if (empty_completed_rx())
				logerror("process_command: Trying to remove entry from empty rx completion fifo\n");
			else
				pop_completed_rx();

			update_ethernet_irq();
			m_recd++;
			update_stats();
			break;

		case ECMD_RELEASE_PACKET:
			{
				const int packet_number = m_reg[B2_PNR_ARR] & 0xff;
				alloc_release(packet_number);
				LOG("   RELEASE SPECIFIC PACKET %d\n", packet_number);
			}
			break;

		case ECMD_ENQUEUE_PACKET:
			LOG("   ENQUEUE TX PACKET ");

			if (m_reg[B0_TCR] & TXENA)
			{
				const int packet_number = m_reg[B2_PNR_ARR] & 0xff;
				LOG("(PACKET_NUM=%d)\n", packet_number);
				// Push packet number to tx transmit fifo
				push_queued_tx(packet_number);
				// Start timer to send frame if not already transmitting
				if (!m_tx_active && !m_tx_poll->enabled())
				{
					m_tx_poll->adjust(attotime::from_usec(10));
					LOG("Start polling time: %s\n", machine().scheduler().time().as_string());
				}
			}
			break;

		case ECMD_RESET_FIFOS:
			LOG("   RESET TX FIFOS\n");
			// Flush fifos.
			reset_tx_fifos();

			break;
	}
	// Set Busy (clear on next read)
	m_reg[B2_MMU_COMMAND] |= 0x0001;

}



/***************************************************************************
    CORE READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    smc91c9x_r - handle a read from the device
-------------------------------------------------*/

u16 smc91c9x_device::read(offs_t offset, u16 mem_mask)
{
	uint32_t result;

	/* determine the effective register */
	offset %= 8;
	if ( offset != B0_BANK )
		offset += 8 * (m_reg[B0_BANK] & 7);

	result = m_reg[offset];

	switch (offset)
	{
		case B2_MMU_COMMAND:
			// Clear busy
			m_reg[B2_MMU_COMMAND] &= ~0x0001;
			break;

		case B2_PNR_ARR:
			if ( ACCESSING_BITS_8_15 )
			{
				m_reg[B2_INTERRUPT] &= ~EINT_ALLOC;
				update_ethernet_irq();
			}
			break;

		case B2_FIFO_PORTS:
			result = 0;
			if (!empty_completed_tx())
				result |= curr_completed_tx();
			else
				result |= 0x80;
			if (!empty_completed_rx())
				result |= curr_completed_rx() << 8;
			else
				result |= 0x80 << 8;
			break;


		case B2_DATA_0:   /* data register */
		case B2_DATA_1:   /* data register */
		{
			uint8_t *buffer;
			int addr = m_reg[B2_POINTER] & 0x7ff;

			if ( m_reg[B2_POINTER] & 0x8000 )
				buffer = &m_buffer[curr_completed_rx() * ETHER_BUFFER_SIZE];
			else
				buffer = &m_buffer[(m_reg[B2_PNR_ARR] & 0x1f) * ETHER_BUFFER_SIZE];

			result = 0;
			if ( ACCESSING_BITS_0_7 )
				result = buffer[addr++];
			if ( ACCESSING_BITS_8_15 )
				result |= buffer[addr++] << 8;
			if ( m_reg[B2_POINTER] & 0x4000 )
				m_reg[B2_POINTER] = (m_reg[B2_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}
	}

	if (offset != B0_BANK)
		LOG("%s:smc91c9x_r(%s) = %04X & %04X\n", machine().describe_context(), ethernet_regname[offset], result, mem_mask);
	return result;
}


/*-------------------------------------------------
    smc91c9x_w - handle a write to the device
-------------------------------------------------*/

void smc91c9x_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	/* determine the effective register */
	offset %= 8;
	if (offset != B0_BANK)
		offset += 8 * (m_reg[B0_BANK] & 7);

	/* update the data generically */

	if (offset != B0_BANK && offset < sizeof(m_reg))
		LOG("%s:smc91c9x_w(%s) = [%04X]<-%04X & (%04X & %04X)\n", machine().describe_context(), ethernet_regname[offset], offset, data, mem_mask , m_regmask[offset]);

	const uint16_t old_reg = m_reg[offset];
	mem_mask &= m_regmask[offset];
	COMBINE_DATA(&m_reg[offset]);
	const uint16_t new_reg = m_reg[offset];

	/* handle it */
	switch (offset)
	{
		case B0_TCR:      /* transmit control register */
			// Setting Tx Enable clears some status and interrupts
			if ( data & TXENA )
			{
				if (m_reg[B0_EPH_STATUS] & (LOST_CARR | SQET | LATCOL | E16COL))
				{
					m_reg[B0_EPH_STATUS] &= ~(LOST_CARR | SQET | LATCOL | E16COL);
					m_reg[B2_INTERRUPT] &= ~EINT_EPH;
					update_ethernet_irq();
				}
			}
			if (VERBOSE & LOG_GENERAL)
			{
				if (data & FDSE)        LOG("   FDSE\n");
				if (data & EPH_LOOP)    LOG("   EPH LOOP\n");
				if (data & STP_SQET)    LOG("   STP SQET\n");
				if (data & FDUPLX)      LOG("   FDUPLX\n");
				if (data & MON_CSN)     LOG("   MON_CSN\n");
				if (data & NOCRC)       LOG("   NOCRC\n");
				if (data & PAD_EN)      LOG("   PAD_EN\n");
				if (data & FORCOL)      LOG("   FORCOL\n");
				if (data & LOOP)        LOG("   LOOP\n");
				if (data & TXENA)       LOG("   TXENA\n");
			}
			break;

		case B0_RCR:      /* receive control register */

			if ( data & SOFT_RST)
			{
				reset();
			}

			if ( (old_reg & RXEN) && !(new_reg & RXEN))
			{
				reset_completed_rx();
			}
			if (data & RXEN)
			{
				// Set LINK_OK in status
				m_reg[B0_EPH_STATUS] |= LINK_OK;
			}

			if ((old_reg ^ new_reg) & PRMS)
			{
				set_promisc(new_reg & PRMS);
			}

			if (VERBOSE & LOG_GENERAL)
			{
				if (data & SOFT_RST)    LOG("   SOFT RST\n");
				if (data & FILT_CAR)    LOG("   FILT_CAR\n");
				if (data & STRIP_CRC)   LOG("   STRIP CRC\n");
				if (data & RXEN)        LOG("   RXEN\n");
				if (data & ALMUL)       LOG("   ALMUL\n");
				if (data & PRMS)        LOG("   PRMS\n");
				if (data & RX_ABORT)    LOG("   RX_ABORT\n");
			}
			break;

		case B1_CONFIG:       /* configuration register */
			if (data & 0x1000) LOG("   NO WAIT\n");
			if (data & 0x0400) LOG("   FULL STEP\n");
			if (data & 0x0200) LOG("   SET SQLCH\n");
			if (data & 0x0100) LOG("   AUI SELECT\n");
			if (data & 0x0080) LOG("   16 BIT\n");
			if (data & 0x0040) LOG("   DIS LINK\n");
			if (data & 0x0004) LOG("   INT SEL1\n");
			if (data & 0x0002) LOG("   INT SEL0\n");
			break;

		case B1_BASE:     /* base address register */
			LOG("   base = $%04X\n", (data & 0xe000) | ((data & 0x1f00) >> 3));
			LOG("   romsize = %d\n", ((data & 0xc0) >> 6));
			LOG("   romaddr = $%05X\n", ((data & 0x3e) << 13));
			break;

		case B1_IA4_5:
			if ( ACCESSING_BITS_8_15 )
			{
				set_promisc(m_reg[B0_RCR] & PRMS);
				set_mac((char *)&m_reg[B1_IA0_1]);
			}
			break;

		case B1_CONTROL:      /* control register */
			// Clearing LE_EN clears interrupt from LINK_OK status change
			if ( (old_reg & LE_ENABLE) && !(new_reg & LE_ENABLE))
			{
				m_reg[B2_INTERRUPT] &= ~EINT_EPH;
				update_ethernet_irq();
			}
			if (0 && (data & LE_ENABLE))
			{
				if (m_reg[B0_EPH_STATUS] & LINK_OK)
				{
					m_reg[B0_EPH_STATUS] &= ~(LINK_OK);
					m_reg[B2_INTERRUPT] &= ~EINT_EPH;
					update_ethernet_irq();
				}
			}
			if (VERBOSE & LOG_GENERAL)
			{
				if (data & RCV_BAD)         LOG("   RCV_BAD\n");
				if (data & PWRDN)           LOG("   PWRDN\n");
				if (data & WAKEUP_EN)       LOG("   WAKEUP ENABLE\n");
				if (data & AUTO_RELEASE)    LOG("   AUTO RELEASE\n");
				if (data & LE_ENABLE)       LOG("   LE ENABLE\n");
				if (data & CR_ENABLE)       LOG("   CR ENABLE\n");
				if (data & TE_ENABLE)       LOG("   TE ENABLE\n");
				if (data & EEPROM_SEL)      LOG("   EEPROM SELECT\n");
				if (data & RELOAD)          LOG("   RELOAD\n");
				if (data & STORE)           LOG("   STORE\n");
			}
			break;

		case B2_MMU_COMMAND:  /* command register */
			process_command(data);
			break;

		case B2_DATA_0:   /* data register */
		case B2_DATA_1:   /* data register */
		{
			uint8_t *buffer;
			int addr = m_reg[B2_POINTER] & PTR;

			if (m_reg[B2_POINTER] & RCV)
				buffer = &m_buffer[curr_completed_rx() * ETHER_BUFFER_SIZE];
			else
				buffer = &m_buffer[(m_reg[B2_PNR_ARR] & 0x1f) * ETHER_BUFFER_SIZE];

			if ( ACCESSING_BITS_0_7 )
				buffer[addr++] = data;
			if ( ACCESSING_BITS_8_15 )
				buffer[addr++] = data >> 8;
			if ( m_reg[B2_POINTER] & AUTO_INCR)
				m_reg[B2_POINTER] = (m_reg[B2_POINTER] & ~PTR) | (addr & PTR);
			break;
		}

		case B2_INTERRUPT:
			// Pop tx fifo packet from completion fifo if clear tx int is set
			if (m_reg[B2_INTERRUPT] & data & EINT_TX)
			{
				if (empty_completed_tx())
					logerror("write: Trying to remove an entry from empty tx completion fifo\n");
				else
				{
					LOG("Removing tx completion packet_num = %d\n", curr_completed_tx());
					pop_completed_tx();
				}
			}
			// Clear interrupts
			m_reg[B2_INTERRUPT] &= ~(data & (EINT_ERCV | EINT_RX_OVRN | EINT_TX_EMPTY | EINT_TX));
			update_ethernet_irq();
			break;
	}
}
