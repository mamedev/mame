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
	m_irq_handler.resolve_safe();

	/* register ide states */
	save_item(NAME(m_reg));
	save_item(NAME(m_regmask));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_alloc_count));
	save_item(NAME(m_rx));
	save_item(NAME(m_tx));
	save_item(NAME(m_sent));
	save_item(NAME(m_recd));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smc91c9x_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	std::fill(std::begin(m_rx), std::end(m_rx), 0);
	std::fill(std::begin(m_tx), std::end(m_tx), 0);

	std::fill(std::begin(m_regmask), std::end(m_regmask), 0);

	m_irq_state = 0;
	m_alloc_count = 0;
	rx_fifo_out = 0;
	rx_fifo_in = 0;

	tx_fifo_out = 0;
	tx_fifo_in = 0;

	m_sent = 0;
	m_recd = 0;

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

	// Interface MAC
	m_reg[EREG_IA0_1]        = mac[0] | (mac[1]<<8);
	m_reg[EREG_IA2_3]        = mac[2] | (mac[3]<<8);
	m_reg[EREG_IA4_5]        = mac[4] | (mac[5]<<8);

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
	m_reg[EREG_REVISION]     = 0x3340;   m_regmask[EREG_REVISION]     = 0x0000;
	m_reg[EREG_ERCV]         = 0x331f;   m_regmask[EREG_ERCV]         = 0x009f;

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

void smc91c9x_device::clear_tx_fifo()
{
	tx_fifo_in = 0;
	tx_fifo_out = 0;
	std::fill(std::begin(m_tx), std::end(m_tx), 0);
}

void smc91c9x_device::clear_rx_fifo()
{
	rx_fifo_in = 0;
	rx_fifo_out = 0;
	std::fill(std::begin(m_rx), std::end(m_rx), 0);
}

int smc91c9x_device::is_broadcast(uint8_t mac_address[])
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


int smc91c9x_device::ethernet_packet_is_for_me(const uint8_t mac_address[])
{
	// tcpdump -i eth0 -q ether host 08:00:1e:01:ae:a5 or ether broadcast or ether dst 09:00:1e:00:00:00 or ether dst 09:00:1e:00:00:01
	// wireshark filter: eth.addr eq 08:00:1e:01:ae:a5 or eth.dst eq ff:ff:ff:ff:ff:ff or eth.dst eq 09:00:1e:00:00:00 or eth.dst eq 09:00:1e:00:00:01

	int i;
	uint8_t local_address[ETHERNET_ADDR_SIZE];

	LOG("\n");

	local_address[0] = (m_reg[EREG_IA0_1]>>0) & 0xFF;
	local_address[1] = (m_reg[EREG_IA0_1]>>8) & 0xFF;
	local_address[2] = (m_reg[EREG_IA2_3]>>0) & 0xFF;
	local_address[3] = (m_reg[EREG_IA2_3]>>8) & 0xFF;
	local_address[4] = (m_reg[EREG_IA4_5]>>0) & 0xFF;
	local_address[5] = (m_reg[EREG_IA4_5]>>8) & 0xFF;

	if (VERBOSE & LOG_GENERAL)
	{
		for ( i = 0 ; i < ETHERNET_ADDR_SIZE ; i++ )
		{
			logerror("%.2X",local_address[i]);
		}
		logerror("=");
		for ( i = 0 ; i < ETHERNET_ADDR_SIZE ; i++ )
		{
			logerror("%.2X",mac_address[i]);
		}
		logerror("?");
	}

	// skip Ethernet broadcast packets if RECV_BROAD is not set
	if (is_broadcast((uint8_t *)mac_address))
	{
		LOG(" -- Broadcast rx\n");
		return 2;
	}

	if (memcmp(mac_address, local_address, ETHERNET_ADDR_SIZE) == 0)
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

	if ( (length < ETHERNET_ADDR_SIZE || !isforme) && !(m_reg[EREG_RCR] & 0x0100) )
	{
		LOG("\n");

		// skip packet
		return;
	}

	/* signal a receive */

	/* compute the packet length */

	if ( ( length < ( ETHER_BUFFER_SIZE - ( 2+2+2 ) ) ) )
	{
		uint8_t *const packet = &m_rx[ ( rx_fifo_in & ( ETHER_RX_BUFFERS - 1 ) ) * ETHER_BUFFER_SIZE];

		std::fill_n(packet, ETHER_BUFFER_SIZE, 0);

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

		dst += 2;

		dst &= 0x7FF;

		packet[2] = (dst&0xFF);
		packet[3] = (dst) >> 8;

		m_reg[EREG_INTERRUPT] |= EINT_RCV;
		m_reg[EREG_FIFO_PORTS] &= ~0x8000;

		rx_fifo_in = (rx_fifo_in + 1) & ( ETHER_RX_BUFFERS - 1 );
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
	uint8_t const mask = m_reg[EREG_INTERRUPT] >> 8;
	uint8_t const state = m_reg[EREG_INTERRUPT] & 0xff;

	/* update the IRQ state */
	m_irq_state = ((mask & state) != 0);
	m_irq_handler(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
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

int smc91c9x_device::send_frame()
{
	bool const is_broadcast = (m_tx[4] == 0xff && m_tx[5] == 0xff && m_tx[6] == 0xff &&
						m_tx[7] == 0xff && m_tx[8] == 0xff && m_tx[9] == 0xff);

	tx_fifo_in = ( tx_fifo_in + 1 )  & ( ETHER_TX_BUFFERS - 1 );

	uint8_t *const tx_buffer = &m_tx[(tx_fifo_out & (ETHER_TX_BUFFERS-1))* ETHER_BUFFER_SIZE];
	tx_fifo_out = ((tx_fifo_out + 1)& (ETHER_TX_BUFFERS-1));

	/* update the EPH register and stuff it in the first transmit word */
	m_reg[EREG_EPH_STATUS] = 0x0001;

	if (is_broadcast)
		m_reg[EREG_EPH_STATUS] |= 0x0040;

	tx_buffer[0] = m_reg[EREG_EPH_STATUS];
	tx_buffer[1] = m_reg[EREG_EPH_STATUS] >> 8;

	/* signal a transmit interrupt and mark the transmit buffer empty */
	m_reg[EREG_INTERRUPT] |= EINT_TX;
	m_reg[EREG_INTERRUPT] |= EINT_TX_EMPTY;
	m_reg[EREG_FIFO_PORTS] |= 0x0080;
	m_sent++;

	update_stats();

	int buffer_len = ((tx_buffer[3] << 8) | tx_buffer[2]) & 0x7ff;

	if (VERBOSE & LOG_GENERAL)
	{
		logerror("TX: ");
		for (int i = 4; i < (4 + ETHERNET_ADDR_SIZE); i++)
			logerror("%.2X", tx_buffer[i]);

		logerror(" ");

		for (int i = 0; i < (buffer_len - (ETHERNET_ADDR_SIZE + 4)); i++)
			logerror("%.2X", tx_buffer[4 + ETHERNET_ADDR_SIZE + i]);

		logerror("--- %d/0x%x bytes\n", buffer_len, buffer_len);
	}

	if ( buffer_len > 4 )
	{
		// odd or even sized frame ?
		if (tx_buffer[buffer_len-1] & 0x20)
			buffer_len--;
		else
			buffer_len -= 2;

		if (!(m_reg[EREG_TCR] & 0x2002))
		{
			// No loopback... Send the frame
			if ( !send(&tx_buffer[4], buffer_len-4) )
			{
				// FIXME: failed to send the Ethernet packet
				//logerror("failed to send Ethernet packet\n");
				//LOG(this,("read_command_port(): !!! failed to send Ethernet packet"));
			}
		}
		else
		{
			// TODO loopback mode : Push the frame to the RX FIFO.
		}
	}

	return 0;
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
			LOG("   ALLOCATE MEMORY FOR TX (%d)\n", (data & 7));
			m_reg[EREG_PNR_ARR] &= ~0xff00;
			m_reg[EREG_PNR_ARR] |= (m_alloc_count++ & 0x7F) << 8;
			m_reg[EREG_INTERRUPT] |= 0x0008;
			update_ethernet_irq();
			break;

		case ECMD_RESET_MMU:
			/*
			0100
			- RESET MMU TO INITIAL STATE -
			Frees   all   memory   allocations,   clears   relevant
			interrupts, resets packet FIFO pointers.
			*/

			LOG("   RESET MMU\n");
			// Flush fifos.
			clear_tx_fifo();
			clear_rx_fifo();
			break;

		case ECMD_REMOVE_TOPFRAME_TX:
			LOG("   REMOVE FRAME FROM TX FIFO\n");
			break;

		case ECMD_REMOVE_TOPFRAME_RX:
			LOG("   REMOVE FRAME FROM RX FIFO\n");

		case ECMD_REMOVE_RELEASE_TOPFRAME_RX:
			LOG("   REMOVE AND RELEASE FRAME FROM RX FIFO (RXI=%d RXO=%d)\n", rx_fifo_in & (ETHER_RX_BUFFERS - 1), rx_fifo_out & (ETHER_RX_BUFFERS - 1));

			m_reg[EREG_INTERRUPT] &= ~EINT_RCV;

			if ( (rx_fifo_in & ( ETHER_RX_BUFFERS - 1 ) ) != (rx_fifo_out & ( ETHER_RX_BUFFERS - 1 ) ) )
				rx_fifo_out = ( (rx_fifo_out + 1) & ( ETHER_RX_BUFFERS - 1 ) );

			if ( (rx_fifo_in & ( ETHER_RX_BUFFERS - 1 ) ) != (rx_fifo_out & ( ETHER_RX_BUFFERS - 1 ) ) )
			{
				m_reg[EREG_INTERRUPT] |= EINT_RCV;
				m_reg[EREG_FIFO_PORTS] &= ~0x8000;
			}
			else
				m_reg[EREG_FIFO_PORTS] |= 0x8000;

			update_ethernet_irq();
			m_recd++;
			update_stats();
			break;

		case ECMD_RELEASE_PACKET:
			LOG("   RELEASE SPECIFIC PACKET\n");
			break;

		case ECMD_ENQUEUE_PACKET:
			LOG("   ENQUEUE TX PACKET\n");

			if ( m_link_unconnected )
			{
				// Set lost carrier
				if ( m_reg[EREG_TCR] & 0x0400 )
				{
					m_reg[EREG_EPH_STATUS] |= 0x400;
					// Clear Tx Enable on error
					m_reg[EREG_TCR] &= ~0x1;
				}

				// Set signal quality error
				if ( m_reg[EREG_TCR] & 0x1000 )
				{
					m_reg[EREG_EPH_STATUS] |= 0x20;
					// Clear Tx Enable on error
					m_reg[EREG_TCR] &= ~0x1;
				}

				// signal a no transmit
				m_reg[EREG_INTERRUPT] &= ~EINT_TX;
				// Set a ethernet phy status interrupt
				m_reg[EREG_INTERRUPT] |= EINT_EPH;

				// Flush fifos.
				clear_tx_fifo();
				clear_rx_fifo();
			}
			else
			{
				if ( m_reg[EREG_TCR] & 0x0001 ) // TX EN ?
				{
					send_frame();
				}
			}

			update_ethernet_irq();

			break;

		case ECMD_RESET_FIFOS:
			LOG("   RESET TX FIFOS\n");
			// Flush fifos.
			clear_tx_fifo();
			clear_rx_fifo();

			break;
	}
	// Set Busy (clear on next read)
	m_reg[EREG_MMU_COMMAND] |= 0x0001;
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
				m_reg[EREG_INTERRUPT] &= ~0x0008;
				update_ethernet_irq();
			}
			break;

		case EREG_DATA_0:   /* data register */
		case EREG_DATA_1:   /* data register */
		{
			uint8_t *buffer;
			int addr = m_reg[EREG_POINTER] & 0x7ff;

			if ( m_reg[EREG_POINTER] & 0x8000 )
			{
				buffer = &m_rx[(rx_fifo_out & ( ETHER_RX_BUFFERS - 1 )) * ETHER_BUFFER_SIZE];
			}
			else
			{
				buffer = (uint8_t *)&m_tx[(tx_fifo_in & (ETHER_TX_BUFFERS-1))* ETHER_BUFFER_SIZE];;
			}

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

	if (offset != 7 && offset < sizeof(m_reg))
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

			if ( m_reg[EREG_POINTER] & 0x8000 )
			{
				buffer = &m_rx[(rx_fifo_out & ( ETHER_RX_BUFFERS - 1 )) * ETHER_BUFFER_SIZE];
			}
			else
			{
				buffer = (uint8_t *)&m_tx[(tx_fifo_in & (ETHER_TX_BUFFERS-1))* ETHER_BUFFER_SIZE];;
			}

			buffer[addr++] = data;
			if ( ACCESSING_BITS_8_15 )
				buffer[addr++] = data >> 8;
			if ( m_reg[EREG_POINTER] & 0x4000 )
				m_reg[EREG_POINTER] = (m_reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}

		case EREG_INTERRUPT:
			m_reg[EREG_INTERRUPT] &= ~(data & 0x56);
			// Need to clear tx int here for vegas cartfury
			if ( m_reg[EREG_FIFO_PORTS] & 0x0080 )
				m_reg[EREG_INTERRUPT] &= ~EINT_TX;
			update_ethernet_irq();
			break;
	}
}
