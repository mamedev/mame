/*
 * 3c505.c - 3COM 3C505 ethernet controller (for Apollo DN3x00)
 *
 *  Created on: August 27, 2010
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *  see also:
 *  - http://lxr.free-electrons.com/source/drivers/net/3c505.h
 *  - http://lxr.free-electrons.com/source/drivers/net/3c505.c
 *  - http://stason.org/TULARC/pc/network-cards/O/OLIVETTI-Ethernet-NPU-9144-3C505.html
 *  - http://www.bitsavers.org/pdf/3Com/3C500_Mar83.pdf
 */

#include "machine/3c505.h"

#define VERBOSE 0

static int verbose = VERBOSE;

#define LOG(x)	{ logerror ("%s: ", m_device->cpu_context()); logerror x; logerror ("\n"); }
#define LOG1(x)	{ if (verbose > 0) LOG(x)}
#define LOG2(x)	{ if (verbose > 1) LOG(x)}

#define  MAINCPU "maincpu"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

//const device_type THREECOM3C505 = threecom3c505_device_config::static_alloc_device_config;

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Apollo microcode version
#define APOLLO_MC_VERSION_SR10_2 0x0302
#define APOLLO_MC_VERSION_SR10_4 0x0303

#define PORT_DATA_FIFO_SIZE 20

/*
 * I/O register offsets
 */
#define PORT_COMMAND    0x00    /* read/write, 8-bit */
#define PORT_STATUS     0x02    /* read only, 8-bit */
#define PORT_AUXDMA     0x02    /* write only, 8-bit */
#define PORT_DATA       0x04    /* read/write, 16-bit */
#define PORT_CONTROL    0x06    /* read/write, 8-bit */

#define ELP_IO_EXTENT   0x10    /* size of used IO registers */

/*
 * host control registers bits
 */
#define ATTN    0x80    /* attention */
#define FLSH    0x40    /* flush data register */
#define DMAE    0x20    /* DMA enable */
#define DIR_    0x10    /* direction */
#define TCEN    0x08    /* terminal count interrupt enable */
#define CMDE    0x04    /* command register interrupt enable */
#define HSF2    0x02    /* host status flag 2 */
#define HSF1    0x01    /* host status flag 1 */

/*
 * combinations of HSF flags used for PCB transmission
 */
#define HSF_PCB_ACK     HSF1
#define HSF_PCB_NAK     HSF2
#define HSF_PCB_END     (HSF2|HSF1)
#define HSF_PCB_MASK    (HSF2|HSF1)

/*
 * host status register bits
 */
#define HRDY    0x80    /* data register ready */
#define HCRE    0x40    /* command register empty */
#define ACRF    0x20    /* adapter command register full */
/* #define DIR_  0x10    direction - same as in control register */
#define DONE    0x08    /* DMA done */
#define ASF3    0x04    /* adapter status flag 3 */
#define ASF2    0x02    /* adapter status flag 2 */
#define ASF1    0x01    /* adapter status flag 1 */

/*
 * combinations of ASF flags used for PCB reception
 */
#define ASF_PCB_ACK     ASF1
#define ASF_PCB_NAK     ASF2
#define ASF_PCB_END     (ASF2|ASF1)
#define ASF_PCB_MASK    (ASF3|ASF2|ASF1)

/*
 * host aux DMA register bits
 */
#define DMA_BRST        0x01    /* DMA burst */

/*
 * maximum amount of data allowed in a PCB
 */
#define MAX_PCB_DATA    62

#define CMD_RESPONSE_OFFSET 0x30
enum
{
	/* host PCB commands */
	CMD_RESET = 0x00,
	CMD_CONFIGURE_ADAPTER_MEMORY = 0x01,
	CMD_CONFIGURE_82586 = 0x02,
	CMD_STATION_ADDRESS = 0x03,
	CMD_DMA_DOWNLOAD = 0x04,
	CMD_DMA_UPLOAD = 0x05,
	CMD_PIO_DOWNLOAD = 0x06,
	CMD_PIO_UPLOAD = 0x07,
	CMD_RECEIVE_PACKET = 0x08,
	CMD_TRANSMIT_PACKET = 0x09,
	CMD_NETWORK_STATISTICS = 0x0a,
	CMD_LOAD_MULTICAST_LIST = 0x0b,
	CMD_CLEAR_PROGRAM = 0x0c,
	CMD_DOWNLOAD_PROGRAM = 0x0d,
	CMD_EXECUTE_PROGRAM = 0x0e,
	CMD_SELF_TEST = 0x0f,
	CMD_SET_STATION_ADDRESS = 0x10,
	CMD_ADAPTER_INFO = 0x11,

	CMD_MC_17 = 0x17,
	CMD_TRANSMIT_PACKET_18 = 0x18,
	CMD_MC_F8 =0xf8,
	CMD_MC_F9 =0xf9,
	CMD_MC_FA =0xfa,

	/*adapter PCB commands */
	CMD_RESET_RESPONSE = 0x30,
	CMD_CONFIGURE_ADAPTER_RESPONSE = 0x31,
	CMD_CONFIGURE_82586_RESPONSE = 0x32,
	CMD_ADDRESS_RESPONSE = 0x33,
	CMD_DOWNLOAD_DATA_REQUEST = 0x34,
	CMD_UPLOAD_DATA_REQUEST = 0x35,
	CMD_RECEIVE_PACKET_COMPLETE = 0x38,
	CMD_TRANSMIT_PACKET_COMPLETE = 0x39,
	CMD_NETWORK_STATISTICS_RESPONSE = 0x3a,
	CMD_LOAD_MULTICAST_RESPONSE = 0x3b,
	CMD_CLEAR_PROGRAM_RESPONSE = 0x3c,
	CMD_DOWNLOAD_PROGRAM_RESPONSE = 0x3d,
	CMD_EXECUTE_PROGRAM_RESPONSE = 0x3e,
	CMD_SELF_TEST_RESPONSE = 0x3f,
	CMD_SET_ADDRESS_RESPONSE = 0x40,
	CMD_ADAPTER_INFO_RESPONSE = 0x41,

	CMD_MC_17_COMPLETE = 0x47,
	CMD_TRANSMIT_PACKET_18_COMPLETE = 0x48,

	CMD_MC_E1_RESPONSE = 0xe1,
	CMD_MC_E2_RESPONSE = 0xe2
};

/* These defines for 'configure' */
#define RECV_STATION    0x00
#define RECV_BROAD      0x01
#define RECV_MULTI      0x02
#define RECV_PROMISC    0x04
#define NO_LOOPBACK     0x00
#define INT_LOOPBACK    0x08
#define EXT_LOOPBACK    0x10

// FIXME:
// #define lsb_first(a) (((a & 255) << 8) | ((a >>8) & 255))
#define lsb_first(a) (a)

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// device type definition
const device_type THREECOM3C505 = &device_creator<threecom3c505_device>;

//-------------------------------------------------
// sc499_device - constructor
//-------------------------------------------------

threecom3c505_device::threecom3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, THREECOM3C505, "Threecom 3C505", tag, owner, clock),
	  device_network_interface(mconfig, *this, 10.0f)
{
	memset(static_cast<threecom3c505_interface *>(this), 0, sizeof(threecom3c505_interface));
}

//-------------------------------------------------
//  static_set_interface - set the interface struct
//-------------------------------------------------

void threecom3c505_device::static_set_interface(device_t &device, const threecom3c505_interface &interface)
{
	threecom3c505_device &threecom3c505 = downcast<threecom3c505_device &>(device);
	static_cast<threecom3c505_interface &>(threecom3c505) = interface;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void threecom3c505_device::device_start()
{
	m_device = this;
	LOG1(("start 3COM 3C505"));

	 m_rx_fifo.start(this, RX_FIFO_SIZE, ETH_BUFFER_SIZE);
	 m_rx_data_buffer.start(this, ETH_BUFFER_SIZE);
	 m_tx_data_buffer.start(this, ETH_BUFFER_SIZE);
	 m_program_buffer.start(this, PGM_BUFFER_SIZE);

	if (tx_init != NULL)
	{
		(*tx_init)(this);
	}

	m_timer = m_device->machine().scheduler().timer_alloc(FUNC(static_set_interrupt), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void threecom3c505_device::device_reset()
{
	LOG1(("reset 3COM 3C505"));

	m_rx_fifo.reset();
	m_rx_data_buffer.reset();
	m_tx_data_buffer.reset();
	m_program_buffer.reset();

	memset(m_reg, 0, sizeof(m_reg));
	m_status = HCRE;
	m_control = 0;
	m_command_index = 0;
	m_command_pending = 0;
	m_mc_f9_pending = 0;
	m_wait_for_ack = 0;
	m_rx_data_index = 0;
	m_rx_pending = 0;
	m_tx_data_length = 0;
	m_response_length = 0;
	m_response_index = 0;
	m_microcode_running = 0;
	m_microcode_version = 0;
	m_i82586_config = 0;

	// FIXME: test data
	m_netstat.tot_recv = 1;
	m_netstat.tot_xmit = 2;
	m_netstat.err_CRC = 3;
	m_netstat.err_align = 4;
	m_netstat.err_res = 5;
	m_netstat.err_ovrrun = 6;

	memset(m_station_address, 0 , sizeof(m_station_address));
	memset(m_multicast_list, 0 , sizeof(m_multicast_list));
	set_filter_list();
	set_promisc(true);
}

/***************************************************************************
 cpu_context - return a string describing the current CPU context
 ***************************************************************************/

const char *threecom3c505_device::cpu_context()
{
	static char statebuf[64]; /* string buffer containing state description */

	device_t *cpu = machine().device(MAINCPU);
	osd_ticks_t t = osd_ticks();
	int s = t / osd_ticks_per_second();
	int ms = (t % osd_ticks_per_second()) / 1000;

	/* if we have an executing CPU, output data */
	if (cpu != NULL)
	{
		sprintf(statebuf, "%d.%03d %s pc=%08x - %s", s, ms, cpu->tag(),
				cpu->safe_pcbase(), tag());
	}
	else
	{
		sprintf(statebuf, "%d.%03d", s, ms);
	}
	return statebuf;
}

//**************************************************************************
//  data_buffer
//**************************************************************************

threecom3c505_device::data_buffer::data_buffer() :
	m_device(NULL),
	m_size(0L),
	m_data(NULL)
{
}

void threecom3c505_device::data_buffer::start(threecom3c505_device *device, INT32 size)
{
	m_device = device;
	LOG2(("start threecom3c505_device::data_buffer with size %0x", size));
	m_size = size;
	m_data = auto_alloc_array(device->machine(), UINT8, size);
}

void threecom3c505_device::data_buffer::reset()
{
	LOG2(("reset threecom3c505_device::data_buffer"));
	m_length = 0;
}

void threecom3c505_device::data_buffer::copy(data_buffer *db) const
{
	db->m_size = m_size;
	db->m_length = m_length;
	memcpy(db->m_data, m_data, m_size);
}

int threecom3c505_device::data_buffer::append(UINT8 data)
{
	if (m_length >= m_size)
	{
		return 0;
	}
	else
	{
		m_data[m_length++] = data;
		return 1;
	}
}

void threecom3c505_device::data_buffer::log(const char * title) const
{
	if (verbose > 0)
	{
		int i;
		logerror("%s: %s (length=%02x)", m_device->cpu_context(),	title, m_length);
		for (i = 0; i < m_length; i++)
		{
			logerror(" %02x", m_data[i]);
			if (i >= 1023)
			{
				logerror(" ...");
				break;
			}
		}
		logerror("\n");
	}
}

//**************************************************************************
//  data_buffer fifo
//**************************************************************************

threecom3c505_device::data_buffer_fifo::data_buffer_fifo() :
	m_device(NULL),
	m_size(0)
{
}

void threecom3c505_device::data_buffer_fifo::start(
		threecom3c505_device *device, INT32 size, INT32 db_size)
{
	m_device = device;
	LOG2(("start threecom3c505_device::data_buffer_fifo"));

	int i;
	// FIXME: fifo size is hardcoded
	m_size = RX_FIFO_SIZE; // size;
	for (i = 0; i < m_size; i++)
	{
		m_db[i] = new data_buffer();
		m_db[i]->start(device, db_size);
	}
}

void threecom3c505_device::data_buffer_fifo::reset()
{
	LOG2(("reset threecom3c505_device::data_buffer_fifo"));
	m_get_index = m_put_index = 0;
	m_count = 0;
}

int threecom3c505_device::data_buffer_fifo::put(const UINT8 data[], const int length)
{
	UINT16 next_index = (m_put_index + 1) % m_size;

	LOG2(("threecom3c505_device::data_buffer_fifo::put %d", m_count));

	if (next_index == m_get_index)
	{
		// overflow, fifo full
		return 0;
	}
	else if (length > ETH_BUFFER_SIZE)
	{
		// data size exceeds buffer size
		LOG(("threecom3c505_device::data_buffer_fifo::put %d: data size (%d) exceeds buffer size (%d)!!!", m_count, length,ETH_BUFFER_SIZE));
		return 0;
	}
	else
	{
		memcpy(m_db[m_put_index]->m_data, data, length);
		m_db[m_put_index]->m_length = length;
		m_put_index = next_index;
		m_count++;
		return 1;
	}
}

int threecom3c505_device::data_buffer_fifo::get(data_buffer *db)
{
	LOG2(("threecom3c505_device::data_buffer_fifo::get %d", m_count));

	if (m_get_index == m_put_index)
	{
		// fifo empty
		return 0;
	}
	else
	{
		m_db[m_get_index]->copy(db);
		m_get_index = (m_get_index + 1) % m_size;
		m_count--;
		return 1;
	}
}

/*-------------------------------------------------
 set_filter_list - set the ethernet packet filter list
 -------------------------------------------------*/

void threecom3c505_device::set_filter_list()
{
	memset(m_filter_list, 0 , sizeof(m_filter_list));
	memcpy(m_filter_list, m_station_address,  ETHERNET_ADDR_SIZE);
	memset(m_filter_list+ETHERNET_ADDR_SIZE,0xff, ETHERNET_ADDR_SIZE);
	memcpy(m_filter_list+ETHERNET_ADDR_SIZE*2, m_multicast_list, sizeof(m_multicast_list));

	if (setfilter != NULL)
	{
		int node_id = (((m_station_address[3] << 8) + m_station_address[4]) << 8) + m_station_address[5];
		LOG2(("set_filter_list node_id=%x",node_id));
		(*setfilter)(this, node_id);
	}

}

/*-------------------------------------------------
 set_interrupt - set the IRQ state
 -------------------------------------------------*/

void threecom3c505_device::set_interrupt(enum line_state state)
{
	if (state != irq_state)
	{
		LOG2(("set_interrupt(%d)",state));
		if (set_irq != NULL)
		{
			(*set_irq)(this, state);
		}
		irq_state = state;

		// FIXME: hack?
		if ( m_command_buffer[0] == CMD_EXECUTE_PROGRAM && state == 1)
		{
			m_status |= ASF_PCB_END;
		}
	}
}

TIMER_CALLBACK( threecom3c505_device::static_set_interrupt )
{
	reinterpret_cast<threecom3c505_device *> (ptr)->set_interrupt(ASSERT_LINE);
}

// -------------------------------------

void threecom3c505_device::log_command()
{
	if (verbose > 0)
	{
		int i;
		logerror("%s: Command ", cpu_context());
		switch (m_command_buffer[0])
		{
		case CMD_RESET: // 0x00
			logerror("CMD_RESET");
			break;
		case CMD_CONFIGURE_ADAPTER_MEMORY: // 0x01
			logerror("CMD_CONFIGURE_ADAPTER_MEMORY");
			break;
		case CMD_CONFIGURE_82586: // 0x02
			logerror("CMD_CONFIGURE_82586");
			break;
		case CMD_RECEIVE_PACKET: // 0x08
			logerror("CMD_RECEIVE_PACKET");
			break;
		case CMD_TRANSMIT_PACKET: // 0x09
			logerror("CMD_TRANSMIT_PACKET");
			break;
		case CMD_NETWORK_STATISTICS: // 0x0a
			logerror("CMD_NETWORK_STATISTICS");
			break;
		case CMD_LOAD_MULTICAST_LIST: // 0x0b,
			logerror("CMD_LOAD_MULTICAST_LIST");
			break;
		case CMD_CLEAR_PROGRAM: // 0x0c
			logerror("!!! Unexpected CMD_CLEAR_PROGRAM");
			break;
		case CMD_DOWNLOAD_PROGRAM: // 0x0d
			logerror("CMD_DOWNLOAD_PROGRAM");
			break;
		case CMD_EXECUTE_PROGRAM: // 0x0e
			logerror("CMD_EXECUTE_PROGRAM");
			break;
		case CMD_SET_STATION_ADDRESS: // 0x10
			logerror("CMD_SET_STATION_ADDRESS");
			break;
		case CMD_ADAPTER_INFO: // 0x11
			logerror("CMD_ADAPTER_INFO");
			break;
		case CMD_MC_17: // 0x17
			logerror("CMD_MC_17");
			break;
		case CMD_TRANSMIT_PACKET_18: // 0x18
			logerror("CMD_TRANSMIT_PACKET_18");
			break;

		case CMD_MC_F8: // 0xf8
			logerror("!!! CMD_MC_F8");
			break;
		case CMD_MC_F9: // 0xf9
			logerror("CMD_MC_F9");
			break;
		case CMD_MC_FA: // 0xfa
			logerror("!!! CMD_MC_FA");
			break;

		default:
			logerror("!!! Unexpected Command !!!");
		}

		switch (m_command_buffer[0])
		{
		case CMD_MC_F9: // 0xf9
           logerror(" (%02x, length=00)", m_command_buffer[0]);
			break;

		default:
			logerror(" (%02x, length=%02x)", m_command_buffer[0],
					m_command_buffer[1]);
			for (i = 2; i < m_command_index; i++)
			{
				logerror(" %02x", m_command_buffer[i]);
			}
			break;
		}
		logerror("\n");
	}
}

void threecom3c505_device::log_response()
{
	if (verbose > 0)
	{
		int i;
		logerror("%s: Response ", cpu_context());
		switch (m_response.command)
		{
		case CMD_RESET_RESPONSE: // 0x30
			logerror("CMD_RESET_RESPONSE");
			break;
		case CMD_CONFIGURE_ADAPTER_RESPONSE: // 0x31
			logerror("CMD_CONFIGURE_ADAPTER_RESPONSE");
			break;
		case CMD_CONFIGURE_82586_RESPONSE: // 0x32
			logerror("CMD_CONFIGURE_82586_RESPONSE");
			break;
		case CMD_RECEIVE_PACKET_COMPLETE: // 0x38
			logerror("CMD_RECEIVE_PACKET_COMPLETE");
			break;
		case CMD_TRANSMIT_PACKET_COMPLETE: // 0x39
			logerror("CMD_TRANSMIT_PACKET_COMPLETE");
			break;
		case CMD_NETWORK_STATISTICS_RESPONSE: // 0x3a
			logerror("CMD_NETWORK_STATISTICS_RESPONSE");
			break;
		case CMD_LOAD_MULTICAST_RESPONSE: // 0x3b
			logerror("CMD_LOAD_MULTICAST_RESPONSE");
			break;
		case CMD_DOWNLOAD_PROGRAM_RESPONSE: // 0x3d
			logerror("CMD_DOWNLOAD_PROGRAM_RESPONSE");
			break;
		case CMD_EXECUTE_PROGRAM_RESPONSE: // 0x3e
			logerror("CMD_EXECUTE_PROGRAM_RESPONSE");
			break;
		case CMD_SET_ADDRESS_RESPONSE: // 0x40
			logerror("CMD_SET_ADDRESS_RESPONSE");
			break;
		case CMD_ADAPTER_INFO_RESPONSE: // 0x41
			logerror("CMD_ADAPTER_INFO_RESPONSE");
			break;
		case CMD_MC_17_COMPLETE: // 0x47
			logerror("CMD_MC_17_COMPLETE");
			break;
		case CMD_TRANSMIT_PACKET_18_COMPLETE: // 0x48
			logerror("CMD_TRANSMIT_PACKET_18_COMPLETE");
			break;
		case CMD_MC_E1_RESPONSE: // 0xe1
			logerror("!!! CMD_MC_E1_RESPONSE");
			break;
		case CMD_MC_E2_RESPONSE: // 0xe2
			logerror("!!! CMD_MC_E2_RESPONSE");
			break;
		default:
			logerror("!!! Unexpected Response !!!");
		}
		logerror(" (%02x, length=%02x)", m_response.command, m_response.length);
		for (i = 0; i < m_response.length; i++)
		{
			logerror(" %02x", m_response.data.raw[i]);
		}
		logerror("\n");
	}
}

void threecom3c505_device::do_receive_command()
{
	LOG2(("do_receive_command - data_length=%x rx_pending=%d",
					m_rx_data_buffer.get_length(), m_rx_pending));
#if 0
	if (m_microcode_running && m_microcode_version == APOLLO_MC_VERSION_SR10_4)
	{
		m_response.command = CMD_MC_E1_RESPONSE;
		m_response.length = 0;
		m_response_length = 0;
		m_response_index = 0;

		m_rx_pending++;
	}
	else
#endif

	{
		m_response.command = CMD_RECEIVE_PACKET_COMPLETE; // 0x38
		m_response.length = 16;
		m_response.data.rcv_resp.buf_ofs = 0;
		m_response.data.rcv_resp.buf_seg = 0;
		m_response.data.rcv_resp.buf_len = m_rx_data_buffer.get_length();
		m_response.data.rcv_resp.pkt_len = m_rx_data_buffer.get_length(); // (?)
		m_response.data.rcv_resp.timeout = 0; // successful completion
		m_response.data.rcv_resp.status = m_rx_data_buffer.get_length() > 0 ? 0 : 0xffff;
		m_response.data.rcv_resp.timetag = 0L; // TODO

		m_response_length = m_response.length + 2;
		m_response_index = 0;
	}

	m_status |= ACRF; /* adapter command register full */

	if (m_control & CMDE)
	{
		LOG2(("do_receive_command - set_interrupt(%d)", ASSERT_LINE));
		// set_interrupt(ASSERT_LINE);

		// interrupt later
		m_timer->adjust( attotime::from_usec(10));
	}
}

void threecom3c505_device::set_command_pending(int onoff)
{
	m_command_pending = onoff;
	LOG2(("set_command_pending %d m_rx_pending=%d m_rx_data_buffer.get_length()=%x%s",
			m_command_pending, m_rx_pending, m_rx_data_buffer.get_length(), onoff ? "" :"\n"));

//- verbose = onoff ? 1 : 2;

	if (!m_command_pending)
	{
		// clear previous command byte
		m_command_buffer[0] = 0;

		m_mc_f9_pending = 0;
		m_wait_for_ack = 0;

		if (m_rx_pending > 0 && m_rx_data_buffer.get_length() > 0)
		{
			m_rx_pending--;
			m_command_pending = 1;

			LOG2(("set_command_pending %d m_rx_pending=%d", m_command_pending, m_rx_pending));

			do_receive_command();
		}
	}
}

/***************************************************************************
 do_command
 ***************************************************************************/

void threecom3c505_device::do_command()
{
	// default to successful completion
	m_response.command = m_command_buffer[0] + CMD_RESPONSE_OFFSET;
	m_response.length = 1;
	m_response.data.failed = 0; // successful completion

	switch (m_command_buffer[0])
	{
	case CMD_RESET: // 0x00
//      FIXME:
//      device_reset();
		break;

	case CMD_CONFIGURE_ADAPTER_MEMORY: // 0x01
		m_i82586_config = m_program_buffer.get_word(1);
		break;

	case CMD_RECEIVE_PACKET: // 0x08
		if (m_rx_data_buffer.get_length() == 0 && !m_rx_fifo.get(&m_rx_data_buffer))
		{
			// receive data not yet available
			m_rx_pending++;
			set_command_pending(0);
		}
		else
		{
			do_receive_command();
		}
		return;
		// break;

	case CMD_MC_F9:
		m_response.command = CMD_TRANSMIT_PACKET_COMPLETE;
		// fall through

	case CMD_TRANSMIT_PACKET: // 0x09
	case CMD_TRANSMIT_PACKET_18: // 0x18
		m_response.length = 8;
		m_response.data.xmit_resp.buf_ofs = 0;
		m_response.data.xmit_resp.buf_seg = 0;
		m_response.data.xmit_resp.c_stat = 0; // successful completion
		m_response.data.xmit_resp.status = 0;
		break;

	case CMD_EXECUTE_PROGRAM: // 0x0e
		m_response.length = 0;
		m_microcode_running = 1;
		m_microcode_version = m_program_buffer.get_word(1);
		break;

	case CMD_NETWORK_STATISTICS: // 0x0a
		m_response.length = sizeof(struct Netstat);
		// response data must be LSB first!
		m_response.data.netstat.tot_recv = lsb_first(m_netstat.tot_recv);
		m_response.data.netstat.tot_xmit = lsb_first(m_netstat.tot_xmit);
		m_response.data.netstat.err_CRC = lsb_first(m_netstat.err_CRC);
		m_response.data.netstat.err_align = lsb_first(m_netstat.err_align);
		m_response.data.netstat.err_res = lsb_first(m_netstat.err_res);
		m_response.data.netstat.err_ovrrun = lsb_first(m_netstat.err_ovrrun);
		break;

	case CMD_ADAPTER_INFO: // 0x11
		m_response.length = sizeof(struct Info);
		// FIXME: replace test data; must be LSB first!
		m_response.data.info.minor_vers = 1;
		m_response.data.info.major_vers = 2;
		m_response.data.info.ROM_cksum = lsb_first(3);
		m_response.data.info.RAM_sz = lsb_first(4);
		m_response.data.info.free_ofs = lsb_first(5);
		m_response.data.info.free_seg = lsb_first(6);
		break;

	case CMD_LOAD_MULTICAST_LIST:// 0x0b
		if (m_command_buffer[1] > sizeof(m_multicast_list) ||
				(m_command_buffer[1] % ETHERNET_ADDR_SIZE) != 0)
		{
			LOG(("CMD_LOAD_MULTICAST_LIST - !!! unexpected data size %d !!!", m_command_buffer[1]));
		}
		else
		{
			memset(m_multicast_list, 0 , sizeof(m_multicast_list));
			memcpy(m_multicast_list, m_command_buffer+2 , m_command_buffer[1]-2);
			set_filter_list();
		}
		break;

	case CMD_SET_STATION_ADDRESS: // 0x10
		if (m_command_buffer[1] != sizeof(m_station_address))
		{
			LOG(("CMD_SET_STATION_ADDRESS - !!! unexpected data size %d !!!", m_command_buffer[1]));
			memset(m_station_address, 0 , sizeof(m_station_address));
		}
		else
		{
			memcpy(m_station_address, m_command_buffer+2 , m_command_buffer[1]);
		}
		set_filter_list();
		set_mac((char *)m_station_address);
		break;

	case CMD_CONFIGURE_82586: // 0x02
	case CMD_DOWNLOAD_PROGRAM: // 0x0d
	case CMD_MC_17: // 0x17
	default:
		break;
	}

	m_response_index = 0;
	m_response_length = m_response.length + 2;
	m_status |= ACRF; /* adapter command register full */

	if (m_control & CMDE)
	{
		switch (m_command_buffer[0])
		{
		case CMD_NETWORK_STATISTICS: // 0x0a
		case CMD_DOWNLOAD_PROGRAM: // 0xd
		case CMD_EXECUTE_PROGRAM: // 0x0e
		case CMD_ADAPTER_INFO: // 0x11
			// interrupt later
			m_timer->adjust( attotime::from_msec(5));
			break;

		default:
			// interrupt at once
			set_interrupt(ASSERT_LINE);
			break;
		}
	}
}

/***************************************************************************
 ethernet_packet_is_for_me - check if ethernet address is for me
 ***************************************************************************/

int threecom3c505_device::ethernet_packet_is_for_me(const UINT8 mac_address[])
{
	// tcpdump -i eth0 -q ether host 08:00:1e:01:ae:a5 or ether broadcast or ether dst 09:00:1e:00:00:00 or ether dst 09:00:1e:00:00:01
	// wireshark filter: eth.addr eq 08:00:1e:01:ae:a5 or eth.dst eq ff:ff:ff:ff:ff:ff or eth.dst eq 09:00:1e:00:00:00 or eth.dst eq 09:00:1e:00:00:01

	int i;
	for (i = 0; i + ETHERNET_ADDR_SIZE < sizeof(m_filter_list); i += ETHERNET_ADDR_SIZE)
	{
		if (memcmp(mac_address, m_filter_list + i, ETHERNET_ADDR_SIZE) == 0)
		{
			return 1;
		}
	}
	return  0;
}

/***************************************************************************
 recv_cb - receive callback - receive and process an ethernet packet
 ***************************************************************************/

void threecom3c505_device::recv_cb(UINT8 *data, int length)
{

	if (length < ETHERNET_ADDR_SIZE || !ethernet_packet_is_for_me(data))
	{
		// skip packet
	}
	else if (!m_rx_fifo.put(data, length))
	{
		m_netstat.tot_recv++;
		m_netstat.err_ovrrun++;
		// fifo overrun
		LOG1(("recv_cb: data_length=%x !!! RX FIFO OVERRUN !!!", length));

	}
	else
	{
		m_netstat.tot_recv++;
		LOG2(("recv_cb: data_length=%x m_rx_pending=%d", length, m_rx_pending));

		if (m_rx_data_buffer.get_length() == 0)
		{
			m_rx_fifo.get(&m_rx_data_buffer);
		}

		if (!m_command_pending && m_rx_pending > 0
				&& m_rx_data_buffer.get_length() > 0)
		{
			m_rx_pending--;
			set_command_pending(1);
			do_receive_command();
		}
	}
}

bool threecom3c505_device::mcast_chk(const UINT8 *buf, int len) {
	int i;
	for (i = 0; i + ETHERNET_ADDR_SIZE < sizeof(m_multicast_list); i += ETHERNET_ADDR_SIZE)
	{
		if (memcmp(buf, m_multicast_list + i, ETHERNET_ADDR_SIZE) == 0)
		{
			LOG2(("threecom3c505_device::mcast_chk: true (len=%d)", len));
			return true;
		}
	}
	return  false;
}

void threecom3c505_device::write_command_port( UINT8 data)
{
	LOG2(("writing 3C505 command port %02x - m_status=%02x m_control=%02x m_command_index=%02x", data, m_status, m_control, m_command_index));

	if ((m_control & HSF_PCB_MASK) != HSF_PCB_END)
	{
		m_command_buffer[m_command_index++] = data;
		m_status |= HCRE; /* command register empty */

		if (m_command_index == 1)
		{
			switch (data)
			{
			case 0:
				// spurious data; set command register empty
				m_command_index = 0;
				m_status |= HCRE;
				break;

			case CMD_MC_F9:
//          case CMD_MC_FA:
				// read data length from data port (not from command port)
				m_tx_data_buffer.reset();
				m_status |= HRDY; /* data register ready */
				set_command_pending(1);
				break;

			default:
				set_command_pending(1);
				break;
			}
		}
	}
	else
	{
		m_status &= ~ASF_PCB_MASK;
		m_status |= (data == m_command_index) ? ASF_PCB_END : ASF_PCB_NAK;

		log_command();

		switch (m_command_buffer[0])
		{
		case CMD_TRANSMIT_PACKET_18:
			// read transmit data into m_tx_data_buffer
			m_tx_data_buffer.reset();
			m_tx_data_length = m_command_buffer[2] + (m_command_buffer[3] << 8);
			m_status |= HRDY; /* data register ready */
			break;

		case CMD_TRANSMIT_PACKET:
			// read transmit data into m_tx_data_buffer
			m_tx_data_buffer.reset();
			m_tx_data_length = m_command_buffer[6] + (m_command_buffer[7] << 8);
			m_status |= HRDY; /* data register ready */
			break;

		case CMD_DOWNLOAD_PROGRAM:
			// read program data into m_program_buffer
			m_program_buffer.reset();
			m_program_length = m_command_buffer[2] + (m_command_buffer[3] << 8);
			m_status |= HRDY; /* data register ready */
			break;

		default:
			do_command();
			break;
		}
	}
}

UINT8 threecom3c505_device::read_command_port()
{
	UINT8 data;
	if (m_response_index == 0)
	{
//      set_command_pending(1);
		data = m_response.command;
	}
	else if (m_response_index == 1)
	{
		data = m_response.length;
	}
	else if (m_response_index < m_response_length)
	{
		data = m_response.data.raw[m_response_index - 2];
	}
	else if (m_response_index == m_response_length)
	{
		data = m_response.length + 2;
	}
	else if (m_response_index == m_response_length+1 /*&& m_microcode_running*/)
	{
		// FIXME: special for SR10.4 microcode, content doesn't matter?
		data = 0; // ?
		m_response_index++;

		m_status &= ~ACRF; /* adapter command register no longer full */

		LOG2(("FIXME: reading 3C505 Command Register at offset %02x with index %02x = %02x",
				PORT_COMMAND, m_response_index, data));

		if (m_mc_f9_pending == 1)
		{
			m_mc_f9_pending++;
		}
	}
	else
	{
		// should never happen
		data = 0; // 0xff;
		LOG(("UNEXPECTED: reading 3C505 Command Register at offset %02x = %02x", PORT_COMMAND, data));
	}

	if (m_response_index <= m_response_length + 1)
	{
		if (++m_response_index == m_response_length)
		{
			m_status = (m_status & ~ASF_PCB_MASK) | ASF_PCB_END;
		}
		else if (m_response_index == m_response_length + 1)
		{
			// FIXME:
			if (!(m_status & DIR_))
			{
				m_status &= ~HRDY; /* data register no longer ready */
			}

			log_response();

			switch (m_response.command)
			{
			case CMD_MC_E1_RESPONSE:
				m_status |= HRDY; /* data register ready */
				// prepend data length
				m_rx_data_index = -2;
				break;

			case CMD_RECEIVE_PACKET_COMPLETE:
				m_status |= HRDY; /* data register ready */
				m_rx_data_index = 0;
				break;

			case CMD_TRANSMIT_PACKET_COMPLETE:
			case CMD_TRANSMIT_PACKET_18_COMPLETE:
				m_netstat.tot_xmit++;

				if(!send(m_tx_data_buffer.get_data(), m_tx_data_buffer.get_length()))
				{
					// FIXME: failed to send the ethernet packet
					LOG(("read_command_port(): failed to send ethernet packet"));
				}

				if (tx_data != NULL && //
					(*tx_data)(this, m_tx_data_buffer.get_data(), m_tx_data_buffer.get_length()) == 0)
				{
					// FIXME: failed to transmit the ethernet packet
					LOG(("read_command_port(): failed to transmit ethernet packet"));
				}

				m_tx_data_buffer.reset();
				if (m_command_buffer[0] != CMD_MC_F9)
				{
//                  set_command_pending(0);
					m_wait_for_ack = 1;
				}
				break;

			case CMD_DOWNLOAD_PROGRAM_RESPONSE:
				m_program_buffer.reset();
//              set_command_pending(0);
				m_wait_for_ack = 1;
				break;

			default:
//              set_command_pending(0);
				m_wait_for_ack = 1;
				break;
			}
		}
	}
	return data;
}

void threecom3c505_device::write_data_port( UINT8 data)
{
	if (m_control & FLSH)
	{
		// flush input data
	}
	else if ((m_status & HRDY) == 0)
	{
		// this will happen in ether.dex Test 20/1
		LOG(("write_data_port: failed to write tx data (data register not ready), data length=%x status=%x",
						m_tx_data_buffer.get_length(), m_status));
	}

#if 0
	else if (m_command_buffer[0] == CMD_MC_F8)
	{
		// FIXME: what does it do?
		LOG(("write_data_port: !!! TODO: CMD_MC_F8 !!! command=%x data=%02x", m_command_buffer[0], data));
	}
#endif

	else if (m_command_buffer[0] == CMD_MC_F9)
	{
		switch (m_command_index) {
		case 1:
			m_command_buffer[m_command_index++] = data;
			break;
		case 2:
			m_command_buffer[m_command_index++] = data;
			m_tx_data_length = m_command_buffer[1] + (m_command_buffer[2] << 8);
			log_command();
			break;
		default:
			if (!m_tx_data_buffer.append(data))
			{
				LOG(("write_data_port: failed to write tx data (buffer size exceeded), data length=%x",
								m_tx_data_buffer.get_length()));
			}
			if (m_tx_data_buffer.get_length() == m_tx_data_length)
			{
				// CMD_TRANSMIT_PACKET_COMPLETE
#if 0
				if (m_command_buffer[0] == CMD_MC_F8)
				{
					// FIXME: what does it do?
					LOG(("write_data_port: !!! TODO: CMD_MC_F8 !!! command=%x data length=%x", m_command_buffer[0], m_tx_data_length));
				}
#endif
				m_tx_data_buffer.log("Tx Data");
				do_command();
			}
			break;
		}
	}
	else if (m_command_buffer[0] == CMD_TRANSMIT_PACKET ||
			m_command_buffer[0] == CMD_TRANSMIT_PACKET_18)
	{
		if (!m_tx_data_buffer.append(data))
		{
			LOG(("write_data_port: failed to write tx data (buffer size exceeded), data length=%x",
							m_tx_data_buffer.get_length()));
		}

		if (m_tx_data_buffer.get_length() == m_tx_data_length)
		{
			// CMD_TRANSMIT_PACKET_COMPLETE
			m_tx_data_buffer.log("Tx Data");
			do_command();
		}
	}
	else if (m_command_buffer[0] == CMD_DOWNLOAD_PROGRAM)
	{
		if (!m_program_buffer.append(data))
		{
			LOG(("write_data_port: failed to write program data (buffer size exceeded), data length=%x",
							m_program_buffer.get_length()));
		}

		if (m_program_buffer.get_length() == m_program_length)
		{
			m_program_buffer.log("Program Data");
			do_command();
		}
	}
	else if (m_tx_data_buffer.get_length() < PORT_DATA_FIFO_SIZE)
	{
		// write to data fifo
		if (!m_tx_data_buffer.append(data))
		{
			LOG(("write_data_port: failed to write tx data (buffer size exceeded), data length=%x",
							m_tx_data_buffer.get_length()));
		}
	}
	else
	{
		LOG(("write_data_port: !!! UNEXPECTED !!! command=%x data=%02x", m_command_buffer[0], data));
	}

	if (m_tx_data_buffer.get_length() >= PORT_DATA_FIFO_SIZE
			&& m_tx_data_buffer.get_length() >= m_tx_data_length)
	{
		m_status &= ~HRDY; /* data register not ready */
		if (m_tx_data_buffer.get_length() > PORT_DATA_FIFO_SIZE
				&& m_tx_data_buffer.get_length() > m_tx_data_length)
		{
			LOG(("write_data_port: port_data tx fifo exhausted, data length=%x status=%x",
							m_tx_data_buffer.get_length(), m_status));
		}
	}
}

UINT8 threecom3c505_device::read_data_port(){
	UINT8 data;
	UINT16 data_length = m_rx_data_buffer.get_length();
	// DomainOS will read  words (i.e. even number of bytes); must handle packets with odd byte length
	UINT16 even_data_length = (data_length+1) & ~1;

	if (m_rx_data_index < even_data_length)
	{
		// eventually prepend data length (for CMD_MC_E1_RESPONSE)
		data = m_rx_data_index == -2 ? (data_length & 0xff) : //
				m_rx_data_index == -1 ? (data_length << 8) : //
				m_rx_data_buffer.get(m_rx_data_index);

		m_rx_data_index++;

		if (m_rx_data_index == even_data_length)
		{
			m_status &= ~HRDY; /* data register no longer ready */
			m_rx_data_buffer.log("Rx Data");
			m_rx_data_buffer.reset();
//          set_command_pending(0);
			m_wait_for_ack = 1;
		}
	}

#if 0
	else if (m_rx_data_index == data_length && (data_length & 1))
	{
		// data length is odd, pad with 0 to next even size
		data = 0;
		m_rx_data_index++;
		LOG(("!!!: padding 3C505 Rx Data with odd data length %2x with value %02x", data_length, data));
	}
#endif

	else
	{
		// FIXME: should never happen
		data = 0xff;
		m_status &= ~HRDY; /* data register no longer ready */
		set_command_pending(0);
		LOG(("UNEXPECTED: reading 3C505 data Register as %02x (data_length=%02x)", data, data_length));
	}
	return data;
}

void threecom3c505_device::write_control_port( UINT8 data)
{
	// if (m_device->machine->firstcpu->safe_pcbase() == 0x3C4BAD48) verbose = 3;
	// if (m_device->machine->firstcpu->safe_pcbase() == 0x010464DC) verbose = 3;

	if (verbose <= 2 && (data & (DMAE | TCEN /*| CMDE*/)) != 0)
	{
		// dma or interrupts enabled
		LOG1(("writing 3C505 Control Register at offset %02x = %02x", PORT_CONTROL, data));
	}

	m_status = (m_status & ~DIR_) | (data & DIR_);

	switch (data & HSF_PCB_MASK)
	{
	case HSF_PCB_ACK: // HSF1
		if (m_wait_for_ack)
		{
			set_command_pending(0);
		}
		break;
	case HSF_PCB_NAK: // HSF2
		if (m_microcode_running)
		{
			m_status = (m_status & ~ASF_PCB_MASK) | ASF_PCB_ACK;
		}
		break;
	case HSF_PCB_END: // (HSF2|HSF1)
		m_status &= ~(ATTN | DMAE | ASF_PCB_MASK);
		if (m_microcode_running)
		{
			m_status = (m_status & ~ASF_PCB_MASK) | ASF_PCB_ACK;
		}
		break;
	default: // 0
		m_command_index = 0;
		m_status |= HCRE; /* command register empty */

		if (m_command_buffer[0] == CMD_MC_F9)
		{
			m_mc_f9_pending = 1;
//-         verbose = 2;
		}

		if (data == 0x00)
		{
			// hardware reset
			m_status |= HRDY; /* data register ready */
			// flush data register (reset tx data fifo)
			m_tx_data_length = 0;
			// FIXME: Don't do this (but why?)
			// m_tx_data_buffer.reset();
		}
		else if (data == DIR_)
		{
			// why?? dex ether 20 expects HRDY
			// m_status &= ~HRDY; /* data register not ready */
			m_status |= HRDY; /* data register ready */
		}
		else if (data & FLSH)
		{
			if (data & DIR_)
			{
				m_status &= ~HRDY; /* data register not ready */
			}
			else
			{
				// flush data register (reset tx data fifo)
//              m_tx_data_length = 0;
				m_tx_data_buffer.reset();
				m_status |= HRDY; /* data register ready */
			}
		}
		break;
	}

	if ((m_control & (ATTN | FLSH)) == (ATTN | FLSH) && (data & (ATTN | FLSH)) == 0)
	{
		m_status |= ASF_PCB_END;
	}

	if (m_mc_f9_pending == 2)
	{
		m_mc_f9_pending = 0;
		set_command_pending(0);
	}

	m_control = data;
}

UINT8 threecom3c505_device::read_status_port()
{
	UINT8 data = m_status;
	m_status &= ~ASF_PCB_MASK;

	switch (data & ASF_PCB_MASK) {
	case ASF_PCB_END:
		m_status |= ASF_PCB_ACK;
		break;
	}
	return data;
}

void threecom3c505_device::write_port(offs_t offset, UINT8 data)
{
	m_reg[offset & 0x0f] = data;
	LOG2(("writing 3C505 Register at offset %02x = %02x", offset, data));

	switch (offset)
	{
	case PORT_COMMAND: /* 0x00 read/write, 8-bit */
		write_command_port(data);
		break;
	case PORT_AUXDMA: /* 0x02 write only, 8-bit */
		break;
	case PORT_DATA: /* 0x04 read/write, 16-bit */
	case PORT_DATA + 1: /* 0x04 read/write, 16-bit */
		write_data_port(data);
		break;
	case PORT_CONTROL: /* 0x06 read/write, 8-bit */
		write_control_port(data);
		break;
	default:
		break;
	}
}

//READ8_DEVICE_HANDLER( threecom3c505_r )
UINT8 threecom3c505_device::read_port(offs_t offset)
{
	// data to omit excessive logging
	static UINT8 last_data = 0xff;
	static UINT32 last_pc = 0;

	UINT8 data = m_reg[offset & 0x0f];
	switch (offset)
	{
	case PORT_COMMAND: /* 0x00 read/write, 8-bit */
		data = read_command_port();
		break;
	case PORT_STATUS: /* 0x02 read only, 8-bit */
		data=read_status_port();

		set_interrupt(CLEAR_LINE);

		// omit excessive logging
		if (data == last_data)
		{
			UINT32 pc = machine().device(MAINCPU)->safe_pcbase();
			if (pc == last_pc) {
				return data;
			}
			last_pc = pc;
		}
		last_data = data;

		break;
	case PORT_DATA: /* 0x04 read/write, 16-bit */
	case PORT_DATA + 1: /* 0x04 read/write, 16-bit */
		data = read_data_port();
		break;
	case PORT_CONTROL: /* 0x06 read/write, 8-bit */
		// just return current value
		break;
	default:
		break;
	}

	LOG2(("reading 3C505 Register at offset %02x = %02x", offset, data));
	return data;
}

//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_DEVICE_HANDLER( threecom3c505_r )
{
	return downcast<threecom3c505_device *> (device)->read_port(offset);
}

WRITE8_DEVICE_HANDLER( threecom3c505_w )
{
	downcast<threecom3c505_device *> (device)->write_port(offset, data);
}

int threecom3c505_receive(device_t *device, const UINT8 data[], int length)
{
	 downcast<threecom3c505_device *> (device)->recv_cb((UINT8 *)data,length);
	 return 1;
}

void threecom3c505_set_verbose(int on_off)
{
	verbose = on_off == 0 ? 0 : VERBOSE > 1 ? VERBOSE : 1;
}
