// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles, Jean-François DEL NERO

***************************************************************************

    Notes:
        * Connected mode working

**************************************************************************/

#include "emu.h"
#include "smc91c9x.h"
// Needed for netdev_count???
#include "osdnet.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

//#define VERBOSE 1
#include "logmacro.h"

#define DISPLAY_STATS       (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Ethernet registers - bank 0 */
#define EREG_TCR            (0*8 + 0)
#define EREG_EPH_STATUS     (0*8 + 1)
#define EREG_RCR            (0*8 + 2)
#define EREG_COUNTER        (0*8 + 3)
#define EREG_MIR            (0*8 + 4)
#define EREG_MCR            (0*8 + 5)
#define EREG_BANK           (0*8 + 7)

/* Ethernet registers - bank 1 */
#define EREG_CONFIG         (1*8 + 0)
#define EREG_BASE           (1*8 + 1)
#define EREG_IA0_1          (1*8 + 2)
#define EREG_IA2_3          (1*8 + 3)
#define EREG_IA4_5          (1*8 + 4)
#define EREG_GENERAL_PURP   (1*8 + 5)
#define EREG_CONTROL        (1*8 + 6)

/* Ethernet registers - bank 2 */
#define EREG_MMU_COMMAND    (2*8 + 0)
#define EREG_PNR_ARR        (2*8 + 1)
#define EREG_FIFO_PORTS     (2*8 + 2)
#define EREG_POINTER        (2*8 + 3)
#define EREG_DATA_0         (2*8 + 4)
#define EREG_DATA_1         (2*8 + 5)
#define EREG_INTERRUPT      (2*8 + 6)

/* Ethernet registers - bank 3 */
#define EREG_MT0_1          (3*8 + 0)
#define EREG_MT2_3          (3*8 + 1)
#define EREG_MT4_5          (3*8 + 2)
#define EREG_MT6_7          (3*8 + 3)
#define EREG_MGMT           (3*8 + 4)
#define EREG_REVISION       (3*8 + 5)
#define EREG_ERCV           (3*8 + 6)

/* Ethernet MMU commands */
#define ECMD_NOP                        0
#define ECMD_ALLOCATE                   2
#define ECMD_RESET_MMU                  4
#define ECMD_REMOVE_TOPFRAME_RX         6
#define ECMD_REMOVE_TOPFRAME_TX         7
#define ECMD_REMOVE_RELEASE_TOPFRAME_RX 8
#define ECMD_RELEASE_PACKET             10
#define ECMD_ENQUEUE_PACKET             12
#define ECMD_RESET_FIFOS                14

/* Ethernet interrupt bits */
#define EINT_RCV            0x01
#define EINT_TX             0x02
#define EINT_TX_EMPTY       0x04
#define EINT_ALLOC          0x08
#define EINT_RX_OVRN        0x10
#define EINT_EPH            0x20
#define EINT_ERCV           0x40

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

smc91c9x_device::smc91c9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10.0f)
	, m_irq_handler(*this)
	, m_link_unconnected(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smc91c9x_device::device_start()
{
	// TX timer
	m_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(smc91c9x_device::send_frame), this));

	m_irq_handler.resolve_safe();

	/* register ide states */
	save_item(NAME(m_reg));
	save_item(NAME(m_regmask));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_buffer));
	save_item(NAME(m_sent));
	save_item(NAME(m_recd));
	save_item(NAME(m_alloc_rx));
	save_item(NAME(m_alloc_tx));
	// Save vector data for proper save state restoration
	save_item(NAME(m_comp_rx_data));
	save_item(NAME(m_comp_tx_data));
	save_item(NAME(m_trans_tx_data));
	// Save vector sizes for proper save state restoration
	save_item(NAME(m_comp_tx_size));
	save_item(NAME(m_comp_rx_size));
	save_item(NAME(m_trans_tx_size));
}

// Save state presave to save vector sizes
void smc91c9x_device::device_pre_save()
{
	m_comp_tx_size = m_comp_tx.size();
	m_comp_rx_size = m_comp_rx.size();
	m_trans_tx_size = m_trans_tx.size();
	memcpy(m_comp_rx_data, m_comp_rx.data(), m_comp_rx_size * sizeof(u32));
	memcpy(m_comp_tx_data, m_comp_tx.data(), m_comp_tx_size * sizeof(u32));
	memcpy(m_trans_tx_data, m_trans_tx.data(), m_trans_tx_size * sizeof(u32));

	//osd_printf_info("Save: comp_tx: %d comp_rx: %d trans_tx: %d\n", m_comp_tx_size, m_comp_rx_size, m_trans_tx_size);
	//if (m_comp_tx_size)
	//  osd_printf_info("comp_tx packet: %d\n", m_comp_tx.front());
}

// Save state preload to restore vector sizes
void smc91c9x_device::device_post_load()
{
	m_comp_tx.resize(m_comp_tx_size);
	m_comp_rx.resize(m_comp_rx_size);
	m_trans_tx.resize(m_trans_tx_size);
	memcpy(m_comp_rx.data(), m_comp_rx_data, m_comp_rx_size * sizeof(u32));
	memcpy(m_comp_tx.data(), m_comp_tx_data, m_comp_tx_size * sizeof(u32));
	memcpy(m_trans_tx.data(), m_trans_tx_data, m_trans_tx_size * sizeof(u32));

	//osd_printf_info("Restore: comp_tx: %d comp_rx: %d trans_tx: %d\n", m_comp_tx_size, m_comp_rx_size, m_trans_tx_size);
	//if (m_comp_tx_size)
	//  osd_printf_info("comp_tx size: %lu comp_tx packet: %d array_data: %d\n", m_comp_tx.size(), m_comp_tx.front(), m_comp_tx_data[0]);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smc91c9x_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	std::fill(std::begin(m_regmask), std::end(m_regmask), 0);

	m_irq_state = 0;

	m_sent = 0;
	m_recd = 0;

	m_reg[EREG_TCR]          = 0x0000;   m_regmask[EREG_TCR]          = 0x3d87;
	m_reg[EREG_EPH_STATUS]   = 0x0000;   m_regmask[EREG_EPH_STATUS]   = 0x0000;
	m_reg[EREG_RCR]          = 0x0000;   m_regmask[EREG_RCR]          = 0xc307;
	m_reg[EREG_COUNTER]      = 0x0000;   m_regmask[EREG_COUNTER]      = 0x0000;
	m_reg[EREG_MIR]          = 0x1212;   m_regmask[EREG_MIR]          = 0x0000;
	m_reg[EREG_MCR]          = 0x3300;   m_regmask[EREG_MCR]          = 0x00ff;
	m_reg[EREG_BANK]         = 0x3300;   m_regmask[EREG_BANK]         = 0x0007;

	m_reg[EREG_CONFIG]       = 0x0030;   m_regmask[EREG_CONFIG]       = 0x17c6;
	m_reg[EREG_BASE]         = 0x1866;   m_regmask[EREG_BASE]         = 0xfffe;

	// Default MAC
	m_reg[EREG_IA0_1]        = 0x1300;   m_regmask[EREG_IA0_1]        = 0xffff;
	m_reg[EREG_IA2_3]        = 0x12F7;   m_regmask[EREG_IA2_3]        = 0xffff;
	m_reg[EREG_IA4_5]        = 0x5634;   m_regmask[EREG_IA4_5]        = 0xffff;

	m_reg[EREG_GENERAL_PURP] = 0x0000;   m_regmask[EREG_GENERAL_PURP] = 0xffff;
	m_reg[EREG_CONTROL]      = 0x0100;   m_regmask[EREG_CONTROL]      = 0x68e7;

	m_reg[EREG_MMU_COMMAND]  = 0x0000;   m_regmask[EREG_MMU_COMMAND]  = 0x00e7;
	m_reg[EREG_PNR_ARR]      = 0x8000;   m_regmask[EREG_PNR_ARR]      = 0x00ff;
	m_reg[EREG_FIFO_PORTS]   = 0x8080;   m_regmask[EREG_FIFO_PORTS]   = 0x0000;
	m_reg[EREG_POINTER]      = 0x0000;   m_regmask[EREG_POINTER]      = 0xf7ff;
	m_reg[EREG_DATA_0]       = 0x0000;   m_regmask[EREG_DATA_0]       = 0xffff;
	m_reg[EREG_DATA_1]       = 0x0000;   m_regmask[EREG_DATA_1]       = 0xffff;
	m_reg[EREG_INTERRUPT]    = 0x0004;   m_regmask[EREG_INTERRUPT]    = 0x7f00;

	m_reg[EREG_MT0_1]        = 0x0000;   m_regmask[EREG_MT0_1]        = 0xffff;
	m_reg[EREG_MT2_3]        = 0x0000;   m_regmask[EREG_MT2_3]        = 0xffff;
	m_reg[EREG_MT4_5]        = 0x0000;   m_regmask[EREG_MT4_5]        = 0xffff;
	m_reg[EREG_MT6_7]        = 0x0000;   m_regmask[EREG_MT6_7]        = 0xffff;
	m_reg[EREG_MGMT]         = 0x3030;   m_regmask[EREG_MGMT]         = 0x0f0f;
	// TODO: Revision should be set based on chip type
	m_reg[EREG_REVISION]     = 0x3345;   m_regmask[EREG_REVISION]     = 0x0000;
	m_reg[EREG_ERCV]         = 0x331f;   m_regmask[EREG_ERCV]         = 0x009f;

	update_ethernet_irq();
	m_tx_timer->reset();

	// Setup real network if enabled
	m_network_available = false;
	if (netdev_count()) {
		m_network_available = true;
		osd_list_network_adapters();
		unsigned char const *const mac = (const unsigned char *)get_mac();
		if (VERBOSE & LOG_GENERAL)
		{
			logerror("MAC : ");
			for (int i = 0; i < ETHERNET_ADDR_SIZE; i++)
				logerror("%.2X", mac[i]);

			logerror("\n");
		}

		set_promisc(true);
		// Interface MAC
		m_reg[EREG_IA0_1] = mac[0] | (mac[1] << 8);
		m_reg[EREG_IA2_3] = mac[2] | (mac[3] << 8);
		m_reg[EREG_IA4_5] = mac[4] | (mac[5] << 8);
	}

	// Reset MMU
	mmu_reset();
}

void smc91c9x_device::mmu_reset()
{
	// Reset MMU allocations
	m_alloc_rx = 0;
	m_alloc_tx = 0;

	// Flush fifos.
	clear_tx_fifo();
	clear_rx_fifo();

	update_ethernet_irq();
}

DEFINE_DEVICE_TYPE(SMC91C94, smc91c94_device, "smc91c94", "SMC91C94 Ethernet Controller")

smc91c94_device::smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: smc91c9x_device(mconfig, SMC91C94, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(SMC91C96, smc91c96_device, "smc91c96", "SMC91C96 Ethernet Controller")

smc91c96_device::smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: smc91c9x_device(mconfig, SMC91C96, tag, owner, clock)
{
}

bool smc91c9x_device::alloc_req(const int tx, int &packet_num)
{
	u32 curr_alloc = m_alloc_rx | m_alloc_tx;

	for (int index = 0; index < ETHER_BUFFERS; index++) {
		if (!(curr_alloc & (1 << index))) {
			packet_num = index;
			if (tx) {
				m_alloc_tx |= 1 << index;
			} else {
				m_alloc_rx |= 1 << index;
			}
			return true;
		}
	}

	return false;
}

void smc91c9x_device::alloc_release(const int packet_num)
{
	int clear_mask = ~(1 << packet_num);
	if (!((m_alloc_tx | m_alloc_rx) & (1 << packet_num))) {
		logerror("alloc_release: Trying to release a non-allocated packet. packet_num: %02x alloc_tx: %04x alloc_rx: %04x\n",
			packet_num, m_alloc_tx, m_alloc_rx);
	}
	m_alloc_tx &= clear_mask;
	m_alloc_rx &= clear_mask;
}

void smc91c9x_device::clear_tx_fifo()
{
	// Clear transmit timer
	m_tx_timer->reset();
	// Reset transmit queue
	m_trans_tx.clear();
	// Reset completion FIFOs
	m_comp_tx.clear();
}

void smc91c9x_device::clear_rx_fifo()
{
	// Clear recieve FIFO
	m_comp_rx.clear();
}

int smc91c9x_device::is_broadcast(const uint8_t *mac_address)
{
	int i;

	i = 0;

	while(mac_address[i] == 0xFF)
	{
		i++;
	}

	if ( i == 6 )
		return 1;

	return 0;
}


int smc91c9x_device::ethernet_packet_is_for_me(const uint8_t *mac_address)
{
	// tcpdump -i eth0 -q ether host 08:00:1e:01:ae:a5 or ether broadcast or ether dst 09:00:1e:00:00:00 or ether dst 09:00:1e:00:00:01
	// wireshark filter: eth.addr eq 08:00:1e:01:ae:a5 or eth.dst eq ff:ff:ff:ff:ff:ff or eth.dst eq 09:00:1e:00:00:00 or eth.dst eq 09:00:1e:00:00:01

	int i;

	LOG("\n");

	if (VERBOSE & LOG_GENERAL)
	{
		for ( i = 0 ; i < ETHERNET_ADDR_SIZE ; i++ )
		{
			logerror("%.2X", ((u8 *)&m_reg[EREG_IA0_1])[i]);
		}
		logerror("=");
		for ( i = 0 ; i < ETHERNET_ADDR_SIZE ; i++ )
		{
			logerror("%.2X",mac_address[i]);
		}
		logerror("?");
	}

	// skip Ethernet broadcast packets if RECV_BROAD is not set
	if (is_broadcast(mac_address))
	{
		LOG(" -- Broadcast rx\n");
		return 2;
	}

	if (memcmp(mac_address, &m_reg[EREG_IA0_1], ETHERNET_ADDR_SIZE) == 0)
	{
		LOG(" -- Address Match\n");
		return 1;
	}

	LOG(" -- Not Matching\n");

	return 0;
}

/***************************************************************************
 recv_cb - receive callback - receive and process an ethernet packet
 ***************************************************************************/

void smc91c9x_device::recv_cb(uint8_t *data, int length)
{
	LOG("recv_cb : %d/0x%x\n",length,length);

	int const isforme = ethernet_packet_is_for_me( data );

	if (isforme==1 && (length >= ETHERNET_ADDR_SIZE) && (VERBOSE & LOG_GENERAL))
	{
		logerror("RX: ");
		for (int i = 0; i < ETHERNET_ADDR_SIZE; i++)
			logerror("%.2X", data[i]);

		logerror(" ");

		for (int i = 0; i < length-ETHERNET_ADDR_SIZE; i++)
			logerror("%.2X", data[ETHERNET_ADDR_SIZE + i]);

		logerror(" - IsForMe %d - %d/0x%x bytes\n", isforme, length, length);
	}

	if ( (length < ETHERNET_ADDR_SIZE || !isforme) && !(m_reg[EREG_RCR] & 0x0102) )
	{
		LOG("\n");

		// skip packet
		return;
	}

	/* signal a receive */

	// Try to request a packet number
	int packet_num;
	if (!alloc_req(0, packet_num)) {
		logerror("recv_cb: Couldn't allocate a receive packet\n");
		return;
	}

	/* compute the packet length */

	if ( ( length < ( ETHER_BUFFER_SIZE - ( 2+2+2 ) ) ) )
	{
		uint8_t *const packet = &m_buffer[ packet_num * ETHER_BUFFER_SIZE];

		int dst = 0;

		// build up the packet

		// Status word
		packet[dst++] = 0x00;

		// set the broadcast flag
		if ( isforme == 2 )
			packet[dst++] |= 0x40;
		else
			packet[dst++] = 0x00;

		//bytes count
		packet[dst++] = 0x00;
		packet[dst++] = 0x00;

		memcpy(&packet[dst], data, length );
		dst += length;

		if ( dst & 1 )
		{
			// ODD Frame
			packet[dst++] = 0x40 | 0x20; // Control
		}
		else
		{
			packet[dst++] = 0x00; // Pad
			packet[dst++] = 0x40 | 0x00; // Control
		}

		//dst += 2;

		dst &= 0x7FF;

		packet[2] = (dst&0xFF);
		packet[3] = (dst) >> 8;

		// Push packet number to rx completion fifo
		m_comp_rx.push_back(packet_num);
	}
	else
	{
		LOG("Rejected ! Fifo Full ?");
	}

	update_ethernet_irq();

	LOG("\n");
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
	if (m_comp_tx.empty())
		m_reg[EREG_INTERRUPT] &= ~EINT_TX;
	else
		m_reg[EREG_INTERRUPT] |= EINT_TX;

	// Check rx completion fifo empty
	if (m_comp_rx.empty())
		m_reg[EREG_INTERRUPT] &= ~EINT_RCV;
	else
		m_reg[EREG_INTERRUPT] |= EINT_RCV;

	uint8_t const mask = m_reg[EREG_INTERRUPT] >> 8;
	uint8_t const state = m_reg[EREG_INTERRUPT] & 0xff;


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
    send_frame - push a frame to the interface
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(smc91c9x_device::send_frame)
{
	// Get the packet number from the transmit fifo
	const int packet_num = m_trans_tx.front();
	uint8_t *const tx_buffer = &m_buffer[packet_num * ETHER_BUFFER_SIZE];

	// Pop the transmit fifo
	m_trans_tx.erase(m_trans_tx.begin());

	/* update the EPH register */
	m_reg[EREG_EPH_STATUS] = 0x0001;

	if (is_broadcast(&tx_buffer[4]))
		m_reg[EREG_EPH_STATUS] |= 0x0040;

	// Check tx completion fifo empty
	if (m_trans_tx.empty())
		m_reg[EREG_INTERRUPT] |= EINT_TX_EMPTY;

	m_sent++;

	update_stats();

	int buffer_len = ((tx_buffer[3] << 8) | tx_buffer[2]) & 0x7ff;
	// Remove status, length, [pad], control
	if (tx_buffer[buffer_len - 1] & 0x20)
		buffer_len -= 5;
	else
		buffer_len -= 6;
	// Add padding
	if (buffer_len < 64 && (m_reg[EREG_TCR] & 0x0080)) {
		while (buffer_len < 64)
			tx_buffer[4 + buffer_len++] = 0x00;
	}
	if (VERBOSE & LOG_GENERAL)
	{
		logerror("TX: ");
		for (int i = 0; i < ETHERNET_ADDR_SIZE; i++)
			logerror("%.2X", tx_buffer[4 + i]);

		logerror(" ");

		for (int i = ETHERNET_ADDR_SIZE; i < buffer_len; i++)
			logerror("%.2X", tx_buffer[4 + i]);

		logerror("--- %d/0x%x bytes\n", buffer_len, buffer_len);
	}

	if (buffer_len > 4)
	{
		if (m_link_unconnected)
		{
			// Set lost carrier
			if (m_reg[EREG_TCR] & 0x0400)
			{
				m_reg[EREG_EPH_STATUS] |= 0x400;
				// Clear Tx Enable on error
				m_reg[EREG_TCR] &= ~0x1;
			}

			// Set signal quality error
			if (m_reg[EREG_TCR] & 0x1000)
			{
				m_reg[EREG_EPH_STATUS] |= 0x20;
				// Clear Tx Enable on error
				m_reg[EREG_TCR] &= ~0x1;
			}

			// Set a ethernet phy status interrupt
			m_reg[EREG_INTERRUPT] |= EINT_EPH;

			// TODO: Is it necessary to clear FIFOs on error?
			// Flush fifos.
			//clear_tx_fifo();
			//clear_rx_fifo();
		}
		else
		{
			// Send the frame
			if (!send(&tx_buffer[4], buffer_len))
			{
				// FIXME: failed to send the Ethernet packet
				//logerror("failed to send Ethernet packet\n");
				//LOG(this,("read_command_port(): !!! failed to send Ethernet packet"));
			}

			// Loopback if loopback is set or fduplx is set
			// TODO: Figure out correct size
			// TODO: Check for addtional filter options for FDUPLX mode
			if ((m_reg[EREG_TCR] & 0x2002) || (m_network_available && (m_reg[EREG_TCR] & 0x0800)))
				recv_cb(&tx_buffer[4], buffer_len);
		}
	}
	// Update status in the transmit word
	tx_buffer[0] = m_reg[EREG_EPH_STATUS];
	tx_buffer[1] = m_reg[EREG_EPH_STATUS] >> 8;

	// Push the packet number onto the tx completion fifo
	m_comp_tx.push_back(packet_num);

	update_ethernet_irq();

	// If there is more packets to transmit then set the tx timer
	if ((m_reg[EREG_TCR] & 0x1) && !m_trans_tx.empty()) {
		// Shortest packet (64 bytes @ 10Mbps = 50us)
		m_tx_timer->adjust(attotime::from_usec(50));
	}
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
				if (alloc_req(1, packet_num)) {
					LOG(" packet_num = %02x\n", (packet_num));
					// Set ARR register
					m_reg[EREG_PNR_ARR] &= ~0xff00;
					m_reg[EREG_PNR_ARR] |= packet_num << 8;
					m_reg[EREG_INTERRUPT] |= EINT_ALLOC;

					update_ethernet_irq();
				}
				else {
					logerror("ECMD_ALLOCATE: Couldn't allocate TX memory\n");
				}
			}
			break;

		case ECMD_RESET_MMU:
			/*
			0100
			- RESET MMU TO INITIAL STATE -
			Frees   all   memory   allocations,   clears   relevant
			interrupts, resets packet FIFO pointers.
			*/

			LOG("   RESET MMU\n");
			mmu_reset();
			break;

		case ECMD_REMOVE_TOPFRAME_TX:
			LOG("   REMOVE FRAME FROM TX FIFO\n");
			if (m_comp_tx.empty())
				logerror("process_command: Trying to remove entry from empty tx completion fifo\n");
			else
				m_comp_tx.erase(m_comp_tx.begin());
			break;

		case ECMD_REMOVE_RELEASE_TOPFRAME_RX:
			LOG("   REMOVE AND RELEASE FRAME FROM RX FIFO (PACK_NUM=%d)\n", m_comp_rx.front());
			// Release memory allocation
			alloc_release(m_comp_rx.front());
			// Fall through
		case ECMD_REMOVE_TOPFRAME_RX:
			LOG("   REMOVE FRAME FROM RX FIFO\n");
			// remove entry from rx completion queue
			if (m_comp_rx.empty())
				logerror("process_command: Trying to remove entry from empty rx completion fifo\n");
			else
				m_comp_rx.erase(m_comp_rx.begin());

			update_ethernet_irq();
			m_recd++;
			update_stats();
			break;

		case ECMD_RELEASE_PACKET:
			{
				const int packet_number = m_reg[EREG_PNR_ARR] & 0xff;
				alloc_release(packet_number);
				LOG("   RELEASE SPECIFIC PACKET %d\n", packet_number);
			}
			break;

		case ECMD_ENQUEUE_PACKET:
			LOG("   ENQUEUE TX PACKET ");

			if (m_reg[EREG_TCR] & 0x0001) // TX EN ?
			{
				const int packet_number = m_reg[EREG_PNR_ARR] & 0xff;
				LOG("(PACKET_NUM=%d)\n", packet_number);
				// Push packet number to tx transmit fifo
				m_trans_tx.push_back(packet_number);
				// Calculate transmit time
				//uint8_t *const tx_buffer = &m_buffer[packet_number * ETHER_BUFFER_SIZE];
				//int buffer_len = ((tx_buffer[3] << 8) | tx_buffer[2]) & 0x7ff;
				//buffer_len -= 6;
				//int usec = ((buffer_len * 8) / 10) + 1;
				// Shortest packet (64 bytes @ 10Mbps = 50us)
				m_tx_timer->adjust(attotime::from_usec(50));
			}
			break;

		case ECMD_RESET_FIFOS:
			LOG("   RESET TX FIFOS\n");
			// Flush fifos.
			clear_tx_fifo();

			break;
	}
	// Set Busy (clear on next read)
	m_reg[EREG_MMU_COMMAND] |= 0x0001;
	//LOG("process_command: TxQ: %d TxComp: %d RxComp: %d TxAlloc: %04x RxAlloc: %04x\n",
	//  m_trans_tx.size(), m_comp_tx.size(), m_comp_rx.size(), m_alloc_tx, m_alloc_rx);
}



/***************************************************************************
    CORE READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    smc91c9x_r - handle a read from the device
-------------------------------------------------*/

READ16_MEMBER( smc91c9x_device::read )
{
	uint32_t result;

	/* determine the effective register */
	offset %= 8;
	if ( offset != EREG_BANK )
		offset += 8 * (m_reg[EREG_BANK] & 7);

	result = m_reg[offset];

	switch (offset)
	{
		case EREG_MMU_COMMAND:
			// Clear busy
			m_reg[EREG_MMU_COMMAND] &= ~0x0001;
			break;

		case EREG_PNR_ARR:
			if ( ACCESSING_BITS_8_15 )
			{
				m_reg[EREG_INTERRUPT] &= ~EINT_ALLOC;
				update_ethernet_irq();
			}
			break;

		case EREG_FIFO_PORTS:
			result = 0;
			if (!m_comp_tx.empty())
				result |= m_comp_tx.front();
			else
				result |= 0x80;
			if (!m_comp_rx.empty())
				result |= m_comp_rx.front() << 8;
			else
				result |= 0x80 << 8;
			break;


		case EREG_DATA_0:   /* data register */
		case EREG_DATA_1:   /* data register */
		{
			uint8_t *buffer;
			int addr = m_reg[EREG_POINTER] & 0x7ff;

			if ( m_reg[EREG_POINTER] & 0x8000 )
				buffer = &m_buffer[m_comp_rx.front() * ETHER_BUFFER_SIZE];
			else
				buffer = &m_buffer[(m_reg[EREG_PNR_ARR] & 0x1f) * ETHER_BUFFER_SIZE];;

			result = buffer[addr++];
			if ( ACCESSING_BITS_8_15 )
				result |= buffer[addr++] << 8;
			if ( m_reg[EREG_POINTER] & 0x4000 )
				m_reg[EREG_POINTER] = (m_reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}
	}

	if (offset != EREG_BANK)
		LOG("%s:smc91c9x_r(%s) = %04X & %04X\n", machine().describe_context(), ethernet_regname[offset], result, mem_mask);
	return result;
}


/*-------------------------------------------------
    smc91c9x_w - handle a write to the device
-------------------------------------------------*/

WRITE16_MEMBER( smc91c9x_device::write )
{
	/* determine the effective register */
	offset %= 8;
	if (offset != EREG_BANK)
		offset += 8 * (m_reg[EREG_BANK] & 7);

	/* update the data generically */

	if (offset != EREG_BANK && offset < sizeof(m_reg))
		LOG("%s:smc91c9x_w(%s) = [%04X]<-%04X & (%04X & %04X)\n", machine().describe_context(), ethernet_regname[offset], offset, data, mem_mask , m_regmask[offset]);

	mem_mask &= m_regmask[offset];
	COMBINE_DATA(&m_reg[offset]);

	/* handle it */
	switch (offset)
	{
		case EREG_TCR:      /* transmit control register */
			// Setting Tx Enable clears some status and interrupts
			if ( data & 0x1 ) {
				m_reg[EREG_EPH_STATUS] &= ~0x420;
				m_reg[EREG_INTERRUPT] &= ~EINT_EPH;
				update_ethernet_irq();
			}

			if (data & 0x2000) LOG("   EPH LOOP\n");
			if (data & 0x1000) LOG("   STP SQET\n");
			if (data & 0x0800) LOG("   FDUPLX\n");
			if (data & 0x0400) LOG("   MON_CSN\n");
			if (data & 0x0100) LOG("   NOCRC\n");
			if (data & 0x0080) LOG("   PAD_EN\n");
			if (data & 0x0004) LOG("   FORCOL\n");
			if (data & 0x0002) LOG("   LOOP\n");
			if (data & 0x0001) LOG("   TXENA\n");
			break;

		case EREG_RCR:      /* receive control register */

			if ( data & 0x8000 )
			{
				clear_rx_fifo();
				clear_tx_fifo();
			}

			if ( !(data & 0x0100) )
			{
				clear_rx_fifo();
			}

			if (data & 0x8000) reset();
			if (data & 0x8000) LOG("   SOFT RST\n");
			if (data & 0x4000) LOG("   FILT_CAR\n");
			if (data & 0x0200) LOG("   STRIP CRC\n");
			if (data & 0x0100) LOG("   RXEN\n");
			if (data & 0x0004) LOG("   ALMUL\n");
			if (data & 0x0002) LOG("   PRMS\n");
			if (data & 0x0001) LOG("   RX_ABORT\n");
			break;

		case EREG_CONFIG:       /* configuration register */
			if (data & 0x1000) LOG("   NO WAIT\n");
			if (data & 0x0400) LOG("   FULL STEP\n");
			if (data & 0x0200) LOG("   SET SQLCH\n");
			if (data & 0x0100) LOG("   AUI SELECT\n");
			if (data & 0x0080) LOG("   16 BIT\n");
			if (data & 0x0040) LOG("   DIS LINK\n");
			if (data & 0x0004) LOG("   INT SEL1\n");
			if (data & 0x0002) LOG("   INT SEL0\n");
			break;

		case EREG_BASE:     /* base address register */
			LOG("   base = $%04X\n", (data & 0xe000) | ((data & 0x1f00) >> 3));
			LOG("   romsize = %d\n", ((data & 0xc0) >> 6));
			LOG("   romaddr = $%05X\n", ((data & 0x3e) << 13));
			break;

		case EREG_CONTROL:      /* control register */
			if (data & 0x4000) LOG("   RCV_BAD\n");
			if (data & 0x2000) LOG("   PWRDN\n");
			if (data & 0x0800) LOG("   AUTO RELEASE\n");
			if (data & 0x0080) LOG("   LE ENABLE\n");
			if (data & 0x0040) LOG("   CR ENABLE\n");
			if (data & 0x0020) LOG("   TE ENABLE\n");
			if (data & 0x0004) LOG("   EEPROM SELECT\n");
			if (data & 0x0002) LOG("   RELOAD\n");
			if (data & 0x0001) LOG("   STORE\n");
			break;

		case EREG_MMU_COMMAND:  /* command register */
			process_command(data);
			break;

		case EREG_DATA_0:   /* data register */
		case EREG_DATA_1:   /* data register */
		{
			uint8_t *buffer;
			int addr = m_reg[EREG_POINTER] & 0x7ff;

			if (m_reg[EREG_POINTER] & 0x8000)
				buffer = &m_buffer[m_comp_rx.front() * ETHER_BUFFER_SIZE];
			else
				buffer = &m_buffer[(m_reg[EREG_PNR_ARR] & 0x1f) * ETHER_BUFFER_SIZE];;

			// TODO: Should be checking if incr is set
			buffer[addr++] = data;
			if ( ACCESSING_BITS_8_15 )
				buffer[addr++] = data >> 8;
			if ( m_reg[EREG_POINTER] & 0x4000 )
				m_reg[EREG_POINTER] = (m_reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}

		case EREG_INTERRUPT:
			// Pop tx fifo packet from completion fifo if clear tx int is set
			if (m_reg[EREG_INTERRUPT] & data & EINT_TX) {
				if (m_comp_tx.empty()) {
					logerror("write: Trying to remove an entry from empty tx completion fifo\n");
				}
				else {
					LOG("Removing tx completion packet_num = %d\n", m_comp_tx.front());
					m_comp_tx.erase(m_comp_tx.begin());
				}
			}
			// Clear TX_EMPTY interrupt if clear tx empty bit is set
			if (m_reg[EREG_INTERRUPT] & data & EINT_TX_EMPTY) {
				m_reg[EREG_INTERRUPT] &= ~EINT_TX_EMPTY;
			}
			m_reg[EREG_INTERRUPT] &= ~(data & 0x56);
			update_ethernet_irq();
			break;
	}
}
