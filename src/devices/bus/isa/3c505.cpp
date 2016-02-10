// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer,R. Belmont
/*
 * 3c505.c - 3COM 3C505 ethernet controller (for Apollo DN3x00)
 *
 *  Created on: August 27, 2010
 *      Author: Hans Ostermeyer
 *      ISA conversion by R. Belmont
 *
 *  see also:
 *  - http://lxr.free-electrons.com/source/drivers/net/3c505.h
 *  - http://lxr.free-electrons.com/source/drivers/net/3c505.c
 *  - http://stason.org/TULARC/pc/network-cards/O/OLIVETTI-Ethernet-NPU-9144-3C505.html
 *  - http://www.bitsavers.org/pdf/3Com/3c505_Etherlink_Plus_Developers_Guide_May86.pdf'
 *  - http://www.bitsavers.org/pdf/3Com/1569-03_EtherLink_Plus_Technical_Reference_Jan89.pdf
 *
 */

#include "3c505.h"

#define VERBOSE 0

static int verbose = VERBOSE;

#define LOG(d,x)  { d->logerror ("%s: ", cpu_context()); d->logerror x; d->logerror ("\n"); }
#define LOG1(d,x) { if (verbose > 0) LOG(d,x)}
#define LOG2(d,x) { if (verbose > 1) LOG(d,x)}

#define  MAINCPU "maincpu"

#ifdef LSB_FIRST
static UINT16 uint16_to_le(UINT16 value)
{
	return value;
}

static UINT16 uint16_from_le(UINT16 value)
{
	return value;
}
#else
static UINT16 uint16_to_le(UINT16 value)
{
	return ((value&0x00ff)<<8)|((value&0xff00)>>8);
}

static UINT16 uint16_from_le(UINT16 value)
{
	return ((value&0x00ff)<<8)|((value&0xff00)>>8);
}
#endif

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
	CMD_MC_F8 = 0xf8,
	CMD_TRANSMIT_PACKET_F9 = 0xf9,
	CMD_MC_FA = 0xfa,

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

/* defines for 'configure' */

#define RECV_STATION    0x00
#define RECV_BROAD      0x01
#define RECV_MULTI      0x02
#define RECV_PROMISC    0x04
#define NO_LOOPBACK     0x00
#define INT_LOOPBACK    0x08
#define EXT_LOOPBACK    0x10

ROM_START( threecom3c505 )
	ROM_REGION( 0x02000, "threecom3c505", 0 )
	ROM_LOAD_OPTIONAL( "3000_3c505_010728-00.bin", 0x00000, 0x02000, CRC(69b77ec6) SHA1(7ac36cc6fc90b90ddfc56c45303b514cbe18ae58) )
	// see http://www.bitsavers.org/bits/Apollo/firmware/
ROM_END

static INPUT_PORTS_START( tc3c505_port )
	PORT_START("IO_BASE")
	PORT_DIPNAME( 0x3f0, 0x300, "3C505 I/O base")
	PORT_DIPSETTING(     0x010, "010h" )
	PORT_DIPSETTING(     0x020, "020h" )
	PORT_DIPSETTING(     0x030, "030h" )
	PORT_DIPSETTING(     0x040, "040h" )
	PORT_DIPSETTING(     0x050, "050h" )
	PORT_DIPSETTING(     0x060, "060h" )
	PORT_DIPSETTING(     0x070, "070h" )
	PORT_DIPSETTING(     0x080, "080h" )
	PORT_DIPSETTING(     0x090, "090h" )
	PORT_DIPSETTING(     0x0a0, "0a0h" )
	PORT_DIPSETTING(     0x0b0, "0b0h" )
	PORT_DIPSETTING(     0x0c0, "0c0h" )
	PORT_DIPSETTING(     0x0d0, "0d0h" )
	PORT_DIPSETTING(     0x0e0, "0e0h" )
	PORT_DIPSETTING(     0x0f0, "0f0h" )
	PORT_DIPSETTING(     0x100, "0100h" )
	PORT_DIPSETTING(     0x110, "0110h" )
	PORT_DIPSETTING(     0x120, "0120h" )
	PORT_DIPSETTING(     0x130, "0130h" )
	PORT_DIPSETTING(     0x140, "0140h" )
	PORT_DIPSETTING(     0x150, "0150h" )
	PORT_DIPSETTING(     0x160, "0160h" )
	PORT_DIPSETTING(     0x170, "0170h" )
	PORT_DIPSETTING(     0x180, "0180h" )
	PORT_DIPSETTING(     0x190, "0190h" )
	PORT_DIPSETTING(     0x1a0, "01a0h" )
	PORT_DIPSETTING(     0x1b0, "01b0h" )
	PORT_DIPSETTING(     0x1c0, "01c0h" )
	PORT_DIPSETTING(     0x1d0, "01d0h" )
	PORT_DIPSETTING(     0x1e0, "01e0h" )
	PORT_DIPSETTING(     0x1f0, "01f0h" )
	PORT_DIPSETTING(     0x200, "0200h" )
	PORT_DIPSETTING(     0x210, "0210h" )
	PORT_DIPSETTING(     0x220, "0220h" )
	PORT_DIPSETTING(     0x230, "0230h" )
	PORT_DIPSETTING(     0x240, "0240h" )
	PORT_DIPSETTING(     0x250, "0250h" )
	PORT_DIPSETTING(     0x260, "0260h" )
	PORT_DIPSETTING(     0x270, "0270h" )
	PORT_DIPSETTING(     0x280, "0280h" )
	PORT_DIPSETTING(     0x290, "0290h" )
	PORT_DIPSETTING(     0x2a0, "02a0h" )
	PORT_DIPSETTING(     0x2b0, "02b0h" )
	PORT_DIPSETTING(     0x2c0, "02c0h" )
	PORT_DIPSETTING(     0x2d0, "02d0h" )
	PORT_DIPSETTING(     0x2e0, "02e0h" )
	PORT_DIPSETTING(     0x2f0, "02f0h" )
	PORT_DIPSETTING(     0x300, "0300h" )
	PORT_DIPSETTING(     0x310, "0310h" )
	PORT_DIPSETTING(     0x320, "0320h" )
	PORT_DIPSETTING(     0x330, "0330h" )
	PORT_DIPSETTING(     0x340, "0340h" )
	PORT_DIPSETTING(     0x350, "0350h" )
	PORT_DIPSETTING(     0x360, "0360h" )
	PORT_DIPSETTING(     0x370, "0370h" )
	PORT_DIPSETTING(     0x380, "0380h" )
	PORT_DIPSETTING(     0x390, "0390h" )
	PORT_DIPSETTING(     0x3a0, "03a0h" )
	PORT_DIPSETTING(     0x3b0, "03b0h" )
	PORT_DIPSETTING(     0x3c0, "03c0h" )
	PORT_DIPSETTING(     0x3d0, "03d0h" )
	PORT_DIPSETTING(     0x3e0, "03e0h" )
	PORT_DIPSETTING(     0x3f0, "03f0h" )

	PORT_START("IRQ_DRQ")
	PORT_DIPNAME( 0x0f, 0x0a, "3C505 IRQ")
	PORT_DIPSETTING(    0x03, "IRQ 3" )
	PORT_DIPSETTING(    0x04, "IRQ 4" )
	PORT_DIPSETTING(    0x05, "IRQ 5" )
	PORT_DIPSETTING(    0x06, "IRQ 6" )
	PORT_DIPSETTING(    0x07, "IRQ 7" )
	PORT_DIPSETTING(    0x09, "IRQ 9" )
	PORT_DIPSETTING(    0x0a, "IRQ 10" )
	PORT_DIPSETTING(    0x0b, "IRQ 11" )
	PORT_DIPSETTING(    0x0c, "IRQ 12" )
	PORT_DIPSETTING(    0x0e, "IRQ 14" )
	PORT_DIPSETTING(    0x0f, "IRQ 15" )

	PORT_DIPNAME( 0x70, 0x60, "3C505 DMA")
	PORT_DIPSETTING(    0x00, "none" )
	PORT_DIPSETTING(    0x10, "DRQ 1" )
	PORT_DIPSETTING(    0x30, "DRQ 3" )
	PORT_DIPSETTING(    0x50, "DRQ 5" )
	PORT_DIPSETTING(    0x60, "DRQ 6" )
	PORT_DIPSETTING(    0x70, "DRQ 7" )

	PORT_START("ROM_OPTS")
	PORT_DIPNAME( 0x01, 0x01, "ROM control")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x01, "Enabled" )
	PORT_DIPNAME( 0x06, 0x00, "ROM base")
	PORT_DIPSETTING(    0x00, "80000h" )
	PORT_DIPSETTING(    0x02, "82000h" )
	PORT_DIPSETTING(    0x04, "84000h" )
	PORT_DIPSETTING(    0x06, "86000h" )

INPUT_PORTS_END

/***************************************************************************
 IMPLEMENTATION
 ***************************************************************************/

// device type definition
const device_type ISA16_3C505 = &device_creator<threecom3c505_device> ;

//-------------------------------------------------
// threecom3c505_device - constructor
//-------------------------------------------------

threecom3c505_device::threecom3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISA16_3C505, "3Com 3C505 Network Adaptor", tag, owner, clock, "3c505", __FILE__),
	device_network_interface(mconfig, *this, 10.0f),
	device_isa16_card_interface(mconfig, *this),
	m_iobase(*this, "IO_BASE"),
	m_irqdrq(*this, "IRQ_DRQ"),
	m_romopts(*this, "ROM_OPTS"), m_status(0), m_control(0), m_command_index(0), m_command_pending(0), m_wait_for_ack(0), m_wait_for_nak(0), m_rx_data_index(0), m_rx_pending(0), m_tx_data_length(0), m_program_length(0), m_response_length(0), m_response_index(0), m_microcode_version(0), m_microcode_running(0), m_i82586_config(0), irq_state(), m_do_command_timer(nullptr), m_installed(false), m_irq(0), m_drq(0)
{
}

threecom3c505_device::threecom3c505_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, "3Com 3C505 Network Adaptor", tag, owner, clock, "3c505", __FILE__),
	device_network_interface(mconfig, *this, 10.0f),
	device_isa16_card_interface(mconfig, *this),
	m_iobase(*this, "IO_BASE"),
	m_irqdrq(*this, "IRQ_DRQ"),
	m_romopts(*this, "ROM_OPTS"), m_status(0), m_control(0), m_command_index(0), m_command_pending(0), m_wait_for_ack(0), m_wait_for_nak(0), m_rx_data_index(0), m_rx_pending(0), m_tx_data_length(0), m_program_length(0), m_response_length(0), m_response_index(0), m_microcode_version(0), m_microcode_running(0), m_i82586_config(0), irq_state(), m_do_command_timer(nullptr), m_installed(false), m_irq(0), m_drq(0)
{
}

ioport_constructor threecom3c505_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tc3c505_port );
}

const rom_entry *threecom3c505_device::device_rom_region() const
{
	return ROM_NAME( threecom3c505 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void threecom3c505_device::device_start()
{
	set_isa_device();

	LOG1(this,("start 3COM 3C505"));

	m_installed = false;

	m_rx_fifo.start(this, RX_FIFO_SIZE, ETH_BUFFER_SIZE);
	m_rx_data_buffer.start(this, ETH_BUFFER_SIZE);
	m_tx_data_buffer.start(this, ETH_BUFFER_SIZE);
	m_program_buffer.start(this, PGM_BUFFER_SIZE);

	m_do_command_timer = timer_alloc(0, nullptr);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void threecom3c505_device::device_reset()
{
	LOG1(this,("reset 3COM 3C505"));

	m_rx_fifo.reset();
	m_rx_data_buffer.reset();
	m_tx_data_buffer.reset();
	m_program_buffer.reset();

	memset(m_reg, 0, sizeof(m_reg));
	m_status = HCRE | DIR_;
	m_control = 0;
	m_command_index = 0;
	m_command_pending = 0;
	m_wait_for_nak = 0;
	m_wait_for_ack = 0;
	m_rx_data_index = 0;
	m_rx_pending = 0;
	m_tx_data_length = 0;
	m_response_length = 0;
	m_response_index = 0;
	m_microcode_running = 0;
	m_microcode_version = 0;
	m_i82586_config = 0;

	// these will appear in /etc/nodestat -l
	m_netstat.tot_recv = 0;
	m_netstat.tot_xmit = 0;
	m_netstat.err_CRC = 0;
	m_netstat.err_align = 0;
	m_netstat.err_res = 0;
	m_netstat.err_ovrrun = 0;

	memset(m_station_address, 0, sizeof(m_station_address));
	memset(m_multicast_list, 0, sizeof(m_multicast_list));
	set_filter_list();
	set_promisc(true);

	if (!m_installed)
	{
		int base = m_iobase->read();

		m_irq = m_irqdrq->read() & 0xf;
		m_drq = (m_irqdrq->read() >> 4) & 0x7;

		m_isa->install16_device(base, base + ELP_IO_EXTENT - 1, 0, 0, read16_delegate(FUNC(threecom3c505_device::read), this), write16_delegate(FUNC(threecom3c505_device::write), this));

		if (m_romopts->read() & 1)
		{
			// host ROM is enabled, get base address
			static const int rom_bases[4] = { 0x0000, 0x2000, 0x4000, 0x6000 };
			int rom_base = rom_bases[(m_romopts->read() >> 1) & 3];
			m_isa->install_rom(this, rom_base, rom_base + 0x01fff, 0, 0, "threecom3c505", "threecom3c505");
		}

		m_installed = true;
	}
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
	if (cpu != nullptr)
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
	m_device(nullptr), m_length(0)
{
}

void threecom3c505_device::data_buffer::start(threecom3c505_device *device, INT32 size)
{
	m_device = device;
	LOG2(device,("start threecom3c505_device::data_buffer with size %0x", size));
	m_data.resize(size);
}

void threecom3c505_device::data_buffer::reset()
{
	LOG2(m_device,("reset threecom3c505_device::data_buffer"));
	m_length = 0;
}

void threecom3c505_device::data_buffer::copy(data_buffer *db) const
{
	db->m_data.resize(m_data.size());
	db->m_length = m_length;
	memcpy(&db->m_data[0], &m_data[0], m_data.size());
}

int threecom3c505_device::data_buffer::append(UINT8 data)
{
	if (m_length >= m_data.size())
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
		m_device->logerror("%s: %s (length=%02x)", m_device->cpu_context(), title, m_length);
		for (i = 0; i < m_length; i++)
		{
			m_device->logerror(" %02x", m_data[i]);
			if (i >= 1023)
			{
				m_device->logerror(" ...");
				break;
			}
		}
		m_device->logerror("\n");
	}
}

//**************************************************************************
//  data_buffer fifo
//**************************************************************************

threecom3c505_device::data_buffer_fifo::data_buffer_fifo() :
	m_device(nullptr),
	m_size(0), m_count(0), m_get_index(0), m_put_index(0)
{
}

void threecom3c505_device::data_buffer_fifo::start(
		threecom3c505_device *device, INT32 size, INT32 db_size)
{
	m_device = device;
	LOG2(m_device,("start threecom3c505_device::data_buffer_fifo"));

	int i;
	// FIXME: fifo size is hardcoded
	m_size = RX_FIFO_SIZE; // size;
	for (i = 0; i < m_size; i++)
	{
		m_db[i] = global_alloc(data_buffer());
		m_db[i]->start(device, db_size);
	}
}

void threecom3c505_device::data_buffer_fifo::reset()
{
	LOG2(m_device,("reset threecom3c505_device::data_buffer_fifo"));
	m_get_index = m_put_index = 0;
	m_count = 0;
}

threecom3c505_device::data_buffer_fifo::~data_buffer_fifo()
{
	for (int i = 0; i < m_size; i++)
	{
		global_free(m_db[i]);
	}
}

int threecom3c505_device::data_buffer_fifo::put(const UINT8 data[], const int length)
{
	UINT16 next_index = (m_put_index + 1) % m_size;

	LOG2(m_device,("threecom3c505_device::data_buffer_fifo::put %d", m_count));

	if (next_index == m_get_index)
	{
		// overflow, fifo full
		return 0;
	}
	else if (length > ETH_BUFFER_SIZE)
	{
		// data size exceeds buffer size
		LOG(m_device,("threecom3c505_device::data_buffer_fifo::put %d: data size (%d) exceeds buffer size (%d)!!!", m_count, length,ETH_BUFFER_SIZE));
		return 0;
	}
	else
	{
		memcpy(&m_db[m_put_index]->m_data[0], data, length);
		m_db[m_put_index]->m_length = length;
		m_put_index = next_index;
		m_count++;
		return 1;
	}
}

int threecom3c505_device::data_buffer_fifo::get(data_buffer *db)
{
	LOG2(m_device,("threecom3c505_device::data_buffer_fifo::get %d", m_count));

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
	memset(m_filter_list, 0, sizeof(m_filter_list));
	memcpy(m_filter_list, m_station_address, ETHERNET_ADDR_SIZE);
	memset(m_filter_list + ETHERNET_ADDR_SIZE, 0xff, ETHERNET_ADDR_SIZE);
	memcpy(m_filter_list + ETHERNET_ADDR_SIZE * 2, m_multicast_list, sizeof(m_multicast_list));

	int node_id = (((m_station_address[3] << 8) + m_station_address[4]) << 8) + m_station_address[5];
	LOG2(this,("set_filter_list node_id=%x",node_id));

	setfilter(this, node_id);
}

/*-------------------------------------------------
 set_interrupt - set the IRQ state
 -------------------------------------------------*/

void threecom3c505_device::set_interrupt(enum line_state state)
{
	if (state != irq_state)
	{
		LOG2(this,("set_interrupt(%d)",state));
		switch (m_irq)
		{
			case 3: m_isa->irq3_w(state); break;
			case 4: m_isa->irq4_w(state); break;
			case 5: m_isa->irq5_w(state); break;
			case 6: m_isa->irq6_w(state); break;
			case 7: m_isa->irq7_w(state); break;
			case 9: m_isa->irq2_w(state); break;    // IRQ 9 on ISA16 goes to IRQ 2
			case 10: m_isa->irq10_w(state); break;
			case 11: m_isa->irq11_w(state); break;
			case 12: m_isa->irq12_w(state); break;
			case 14: m_isa->irq14_w(state); break;
			case 15: m_isa->irq15_w(state); break;
			default: logerror("3c505: invalid IRQ %d\n", m_irq); break;
		}
		irq_state = state;
	}
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
			logerror("!!! unexpected CMD_RESET");
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
			logerror("!!! unexpected CMD_CLEAR_PROGRAM");
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
		case CMD_TRANSMIT_PACKET_F9: // 0xf9
			logerror("CMD_TRANSMIT_PACKET_F9");
			break;
		case CMD_MC_FA: // 0xfa
			logerror("!!! CMD_MC_FA");
			break;

		default:
			logerror("!!! unexpected Command");
		}

		switch (m_command_buffer[0])
		{
		case CMD_TRANSMIT_PACKET_F9: // 0xf9
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
			logerror("!!! unexpected Response");
		}
		logerror(" (%02x, length=%02x)", m_response.command, m_response.length);
		for (i = 0; i < m_response.length; i++)
		{
			logerror(" %02x", m_response.data.raw[i]);
		}
		logerror("\n");
	}
}

/***************************************************************************
 do_receive_command
 ***************************************************************************/

void threecom3c505_device::do_receive_command()
{
	// receive pending and no other command is pending
	if (m_rx_pending > 0 && !m_command_pending)
	{
		if (m_rx_data_buffer.get_length() == 0 && !m_rx_fifo.is_empty())
		{
			m_rx_fifo.get(&m_rx_data_buffer);
		}

		// receive data available ?
		if (m_rx_data_buffer.get_length() > 0)
		{
			LOG2(this,("do_receive_command - data_length=%x rx_pending=%d",
							m_rx_data_buffer.get_length(), m_rx_pending));

			m_rx_pending--;
			set_command_pending(1);

			// preset receive response PCB
			memcpy(&m_response, &m_rcv_response, sizeof(m_rcv_response));

//          m_response.command = CMD_RECEIVE_PACKET_COMPLETE; // 0x38
//          m_response.length = 16;
//          m_response.data.rcv_resp.buf_ofs = htole16(0);
//          m_response.data.rcv_resp.buf_seg = htole16(0);
//          m_response.data.rcv_resp.buf_len = htole16(buf_len);

			// htole16 and friends are not portable beyond Linux.  It's named differently on *BSD and differently again on OS X.  Avoid!
			m_response.data.rcv_resp.pkt_len = uint16_to_le(m_rx_data_buffer.get_length());
			m_response.data.rcv_resp.timeout = 0; // successful completion
			m_response.data.rcv_resp.status  = uint16_to_le(m_rx_data_buffer.get_length() > 0 ? 0 : 0xffff);
			m_response.data.rcv_resp.timetag = 0; // TODO: time tag

			// compute and check no of bytes to be DMA'ed (must be even)
			UINT16 buf_len = uint16_from_le(m_response.data.rcv_resp.buf_len) & ~1;
			if (m_rx_data_buffer.get_length() > buf_len)
			{
				LOG1(this,("do_receive_command !!! buffer size too small (%d < %d)", buf_len, m_rx_data_buffer.get_length()));
				m_response.data.rcv_resp.pkt_len = uint16_to_le(buf_len);
				m_response.data.rcv_resp.status = 0xffff;
			}
			else
			{
				buf_len = (m_rx_data_buffer.get_length() + 1) & ~1;
				m_response.data.rcv_resp.buf_len = uint16_to_le(buf_len);
			}

			m_response_length = m_response.length + 2;
			m_response_index = 0;

			m_status |= ACRF; /* set adapter command register full */
			if (m_control & CMDE)
			{
				set_interrupt(ASSERT_LINE);
			}
		}
	}
}

/***************************************************************************
 set_command_pending onoff
 ***************************************************************************/

void threecom3c505_device::set_command_pending(int state)
{
	LOG2(this,("set_command_pending %d -> %d m_wait_for_ack=%d m_wait_for_nak=%d m_rx_pending=%d%s",
					m_command_pending, state, m_wait_for_ack, m_wait_for_nak, m_rx_pending, state ? "" :"\n"));

//- verbose = onoff ? 1 : 2;

	switch (state)
	{
	case 0:
		// command is no longer pending
		m_command_pending = 0;

		// clear previous command byte
		m_command_buffer[0] = 0;

		m_wait_for_ack = 0;
		m_wait_for_nak = 0;

		do_receive_command();
		break;
	case 1:
		// command is pending
		m_command_pending = 1;
		break;
	case 2:
		// wait for nak/ack
		if (m_microcode_running && m_microcode_version == APOLLO_MC_VERSION_SR10_4)
		{
			m_wait_for_nak = 1;
		}
		else
		{
			m_wait_for_ack = 1;
		}
		break;
	default:
		LOG(this,("set_command_pending %d unexpected" , state));
		break;
	}

}

/***************************************************************************
   execute the commands (formerly do_command)
 ***************************************************************************/

void threecom3c505_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	do_command();
}

void threecom3c505_device::do_command()
{
	pcb_struct &command_pcp = (pcb_struct &) m_command_buffer;

	// default to successful completion
	m_response.command = command_pcp.command + CMD_RESPONSE_OFFSET;
	m_response.length = 1;
	m_response.data.failed = 0; // successful completion

	switch (command_pcp.command)
	{
	case CMD_RESET: // 0x00
		// FIXME: should never occur
		break;

	case CMD_CONFIGURE_ADAPTER_MEMORY: // 0x01
		// TODO
		break;

	case CMD_CONFIGURE_82586: // 0x02
		m_i82586_config = command_pcp.data.raw[0] + (command_pcp.data.raw[1] << 8);
		break;

	case CMD_RECEIVE_PACKET: // 0x08
		// preset response PCB from the Receive Command PCB
		m_rcv_response.command = CMD_RECEIVE_PACKET_COMPLETE; // 0x38
		m_rcv_response.length = sizeof(struct Rcv_resp);
		m_rcv_response.data.rcv_resp.buf_ofs = command_pcp.data.rcv_pkt.buf_ofs;
		m_rcv_response.data.rcv_resp.buf_seg = command_pcp.data.rcv_pkt.buf_seg;
		m_rcv_response.data.rcv_resp.buf_len = command_pcp.data.rcv_pkt.buf_len;
		m_rcv_response.data.rcv_resp.pkt_len = 0;
		m_rcv_response.data.rcv_resp.timeout = 0;
		m_rcv_response.data.rcv_resp.status = 0;
		m_rcv_response.data.rcv_resp.timetag = 0L; // TODO

		m_rx_pending++;
		set_command_pending(0);
		return;
		// break;

	case CMD_TRANSMIT_PACKET_F9:
		m_response.command = CMD_TRANSMIT_PACKET_COMPLETE;
		// fall through

	case CMD_TRANSMIT_PACKET: // 0x09
	case CMD_TRANSMIT_PACKET_18: // 0x18
		m_response.length = sizeof(struct Xmit_resp);
		m_response.data.xmit_resp.buf_ofs = 0;
		m_response.data.xmit_resp.buf_seg = 0;
		m_response.data.xmit_resp.c_stat = 0; // successful completion
		m_response.data.xmit_resp.status = 0;
		break;

	case CMD_EXECUTE_PROGRAM: // 0x0e
		// m_response.length = 0;

		// FIXME: hack?
		m_status |= ASF_PCB_END;
		break;

	case CMD_NETWORK_STATISTICS: // 0x0a
		m_response.length = sizeof(struct Netstat);
		m_response.data.netstat.tot_recv = uint16_to_le(m_netstat.tot_recv);
		m_response.data.netstat.tot_xmit = uint16_to_le(m_netstat.tot_xmit);
		m_response.data.netstat.err_CRC = uint16_to_le(m_netstat.err_CRC);
		m_response.data.netstat.err_align = uint16_to_le(m_netstat.err_align);
		m_response.data.netstat.err_res = uint16_to_le(m_netstat.err_res);
		m_response.data.netstat.err_ovrrun = uint16_to_le(m_netstat.err_ovrrun);
		break;

	case CMD_ADAPTER_INFO: // 0x11
		m_response.length = sizeof(struct Info);
		// FIXME: using demo data
		m_response.data.info.minor_vers = 1;
		m_response.data.info.major_vers = 2;
		m_response.data.info.ROM_cksum = uint16_to_le(3);
		m_response.data.info.RAM_sz = uint16_to_le(4);
		m_response.data.info.free_ofs = uint16_to_le(5);
		m_response.data.info.free_seg = uint16_to_le(6);
		break;

	case CMD_LOAD_MULTICAST_LIST:// 0x0b
		if (command_pcp.length > sizeof(m_multicast_list)
				|| (command_pcp.length % ETHERNET_ADDR_SIZE) != 0)
		{
			LOG(this,("CMD_LOAD_MULTICAST_LIST - unexpected data size %d", command_pcp.length));
		}
		else
		{
			memset(m_multicast_list, 0, sizeof(m_multicast_list));
			memcpy(m_multicast_list, command_pcp.data.multicast, command_pcp.length- 2);
			set_filter_list();
		}
		break;

	case CMD_SET_STATION_ADDRESS: // 0x10
		if (command_pcp.length != sizeof(m_station_address))
		{
			LOG(this,("CMD_SET_STATION_ADDRESS - unexpected data size %d", command_pcp.length));
			memset(m_station_address, 0, sizeof(m_station_address));
		}
		else
		{
			memcpy(m_station_address, command_pcp.data.eth_addr, command_pcp.length);
		}
		set_filter_list();
		set_mac((char *) m_station_address);
		break;

	case CMD_MC_17: // 0x17
		m_microcode_running = 1;
		break;

	case CMD_DOWNLOAD_PROGRAM: // 0x0d
		UINT16 mc_version = m_program_buffer.get_word(1);
		switch (mc_version)
		{
		case APOLLO_MC_VERSION_SR10_2:
		case APOLLO_MC_VERSION_SR10_4:
			m_microcode_version = mc_version;
			break;
		default:
			m_microcode_version = 0;
			LOG(this,("CMD_DOWNLOAD_PROGRAM - unexpected microcode version %04x", mc_version));
			break;
		}
		// return microcode version as program id
		m_response.length = 2;
		m_response.data.raw[0] = m_microcode_version & 0xff;
		m_response.data.raw[1] = (m_microcode_version >> 8) & 0xff;
		break;
	}

	m_response_index = 0;
	m_response_length = m_response.length + 2;

	m_status |= ACRF; /* set adapter command register full */
	if (m_control & CMDE)
	{
		set_interrupt(ASSERT_LINE);
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
	static const UINT8 broadcast_address[ETHERNET_ADDR_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	// accept all packets if RECV_PROMISC is set
	if (m_i82586_config & RECV_PROMISC)
	{
		return 1;
	}

	// skip Ethernet broadcast packets if RECV_BROAD is not set
	if (!(m_i82586_config & RECV_BROAD) && memcmp(mac_address,
			broadcast_address, ETHERNET_ADDR_SIZE) == 0)
	{
		return 0;
	}

	// skip Ethernet multicast packets if RECV_MULTI is not set
	if (!(m_i82586_config & RECV_MULTI) && (mac_address[0] & 0x01) != 0)
	{
		return 0;
	}

	for (i = 0; i + ETHERNET_ADDR_SIZE < sizeof(m_filter_list); i += ETHERNET_ADDR_SIZE)
	{
		if (memcmp(mac_address, m_filter_list + i, ETHERNET_ADDR_SIZE) == 0)
		{
			return 1;
		}
	}
	for (i = 0; i + ETHERNET_ADDR_SIZE < sizeof(m_multicast_list); i += ETHERNET_ADDR_SIZE)
	{
		if (memcmp(mac_address, m_multicast_list + i, ETHERNET_ADDR_SIZE) == 0)
		{
			return 1;
		}
	}
	return 0;
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
		LOG1(this,("recv_cb: data_length=%x !!! RX FIFO OVERRUN !!!", length));

	}
	else
	{
		m_netstat.tot_recv++;
		LOG2(this,("recv_cb: data_length=%x m_rx_pending=%d", length, m_rx_pending));

		do_receive_command();
	}
}

/***************************************************************************
 write_command_port
***************************************************************************/

void threecom3c505_device::write_command_port(UINT8 data)
{
	LOG2(this,("writing 3C505 command port %02x - m_status=%02x m_control=%02x m_command_index=%02x",
			data, m_status, m_control, m_command_index));

	if (m_command_index == 0)
	{
		switch (data)
		{
		case 0:
			LOG2(this,("!!! writing 3C505 Command Register = %02x", data));
			// spurious data; reset?
			break;

		case CMD_TRANSMIT_PACKET_F9:
			// read data length from data port (not from command port)
			m_tx_data_buffer.reset();
			m_status |= HRDY; /* data register ready */
			m_command_buffer[m_command_index++] = data;
			set_command_pending(1);
			break;

		default:
			m_command_buffer[m_command_index++] = data;
			set_command_pending(1);
			break;
		}
	}
	else if ((m_control & HSF_PCB_MASK) != HSF_PCB_END)
	{
		m_command_buffer[m_command_index++] = data;
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

		case CMD_NETWORK_STATISTICS: // 0x0a
		case CMD_EXECUTE_PROGRAM: // 0x0e
		case CMD_ADAPTER_INFO: // 0x11
			// delay command execution
			m_do_command_timer->adjust(attotime::from_usec(100));
			break;

		default:
			do_command();
			break;
		}
	}

	m_status |= HCRE; /* command register empty */
}

/***************************************************************************
 read_command_port
 ***************************************************************************/

UINT8 threecom3c505_device::read_command_port()
{
	UINT8 data;

	// the interrupt request is cleared when the Command Register is read
	set_interrupt(CLEAR_LINE);

	if (m_response_index == 0)
	{
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
	else if (m_response_index == m_response_length + 1 /*&& m_microcode_running*/)
	{
		// FIXME: special for SR10.4 microcode, content doesn't matter?
		data = 0; // ?
		m_response_index++;

		m_status &= ~ACRF; /* the adapter command register is no longer full */

		LOG2(this,("read_command_port: !!! reading 3C505 Command Register = %02x - m_status=%02x m_control=%02x",
						data, m_status, m_control));

		// wait for nak in control register
		set_command_pending(2);
	}
	else
	{
		// should never happen
		data = 0; // 0xff;
		LOG(this,("read_command_port: unexpected reading Command Register at index %04x", m_response_index));
	}

	if (m_response_index <= m_response_length + 1)
	{
		if (++m_response_index == m_response_length)
		{
			m_status = (m_status & ~ASF_PCB_MASK) | ASF_PCB_END;
		}
		else if (m_response_index == m_response_length + 1)
		{
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

				if (!send(m_tx_data_buffer.get_data(),  m_tx_data_buffer.get_length()))
				{
					// FIXME: failed to send the Ethernet packet
					LOG(this,("read_command_port(): !!! failed to send Ethernet packet"));
				}

				if (!tx_data(this, m_tx_data_buffer.get_data(), m_tx_data_buffer.get_length()))
				{
					// FIXME: failed to transmit the Ethernet packet
					LOG(this,("read_command_port(): !!! failed to transmit Ethernet packet"));
				}

				m_tx_data_buffer.reset();
				if (m_command_buffer[0] != CMD_TRANSMIT_PACKET_F9)
				{
					set_command_pending(2);
				}
				break;

			case CMD_DOWNLOAD_PROGRAM_RESPONSE:
				m_program_buffer.reset();
				set_command_pending(2);
				break;

			default:
				set_command_pending(2);
				break;
			}
		}
	}
	return data;
}

/***************************************************************************
 write_data_port
 ***************************************************************************/

void threecom3c505_device::write_data_port(UINT8 data)
{
	if (m_control & FLSH)
	{
		// flush input data
	}
	else if ((m_status & HRDY) == 0)
	{
		// this happened in ether.dex Test 20/1
		LOG(this,("write_data_port: failed to write tx data (data register not ready), data length=%x status=%x",
						m_tx_data_buffer.get_length(), m_status));
	}

#if 0
	else if (m_command_buffer[0] == CMD_MC_F8)
	{
		// FIXME: what does it do?
		LOG(this,("write_data_port: !!! TODO: CMD_MC_F8 !!! command=%x data=%02x", m_command_buffer[0], data));
	}
#endif

	else if (m_command_buffer[0] == CMD_TRANSMIT_PACKET_F9)
	{
		switch (m_command_index)
		{
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
				LOG(this,("write_data_port: failed to write tx data (buffer size exceeded), data length=%x",
								m_tx_data_buffer.get_length()));
			}
			if (m_tx_data_buffer.get_length() == m_tx_data_length)
			{
				// CMD_TRANSMIT_PACKET_COMPLETE
				m_tx_data_buffer.log("Tx Data");
				do_command();
			}
			break;
		}
	}
	else if (m_command_buffer[0] == CMD_TRANSMIT_PACKET || //
			m_command_buffer[0] == CMD_TRANSMIT_PACKET_18)
	{
		if (!m_tx_data_buffer.append(data))
		{
			LOG(this,("write_data_port: failed to write tx data (buffer size exceeded), data length=%x",
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
			LOG(this,("write_data_port: failed to write program data (buffer size exceeded), data length=%x",
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
			LOG(this,("write_data_port: failed to write tx data (buffer size exceeded), data length=%x",
							m_tx_data_buffer.get_length()));
		}
	}
	else
	{
		LOG(this,("write_data_port: unexpected command %02x data=%02x", m_command_buffer[0], data));
	}

	if (m_command_buffer[0] != CMD_DOWNLOAD_PROGRAM &&
			m_tx_data_buffer.get_length() >= PORT_DATA_FIFO_SIZE
			&& m_tx_data_buffer.get_length() >= m_tx_data_length)
	{
		m_status &= ~HRDY; /* data register not ready */
		if (m_tx_data_buffer.get_length() > PORT_DATA_FIFO_SIZE
				&& m_tx_data_buffer.get_length() > m_tx_data_length)
		{
			LOG(this,("write_data_port: port_data tx fifo exhausted, data length=%x status=%x",
							m_tx_data_buffer.get_length(), m_status));
		}
	}
}

/***************************************************************************
 read_data_port
 ***************************************************************************/

UINT8 threecom3c505_device::read_data_port()
{
	UINT8 data;
	UINT16 data_length = m_rx_data_buffer.get_length();
	// DomainOS will read  words (i.e. even number of bytes); must handle packets with odd byte length
	UINT16 even_data_length = (data_length + 1) & ~1;

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
			set_command_pending(2);
		}
	}
	else
	{
		// FIXME: should never happen
		data = 0xff;
		LOG(this,("read_data_port: unexpected reading data at index %04x)", m_rx_data_index));
	}
	return data;
}

/***************************************************************************
 write_control_port
 ***************************************************************************/

void threecom3c505_device::write_control_port(UINT8 data)
{
	switch (data & (ATTN | FLSH))
	{
	case ATTN:
		LOG2(this,("write_control_port %02x - Soft Reset", data));
		// TODO: soft reset
		break;

	case FLSH:
		LOG2(this,("write_control_port %02x - Flush Data Register", data));
		// flush data register
		if (data & DIR_)
		{
			m_status &= ~HRDY; /* data register not ready */
		}
		else
		{
			// download to adapter
			m_status |= HRDY; /* data register ready */

			// flush data register (reset tx data fifo)
			// m_tx_data_length = 0;
			m_tx_data_buffer.reset();
		}
		break;

	case ATTN | FLSH:
		LOG2(this,("write_control_port %02x - Reset Adapter", data));
		device_reset();
		break;

	case 0:
		LOG2(this,("write_control_port %02x", data));

		// end reset
		if ((m_control & (ATTN | FLSH)) == (ATTN | FLSH))
		{
			m_status |= ASF_PCB_END;
			m_status |= HRDY; /* 20 byte data fifo is empty */
		}

		if (data == DIR_)
		{
			// why?? dex ether 20 expects HRDY
			m_status |= HRDY; /* data register ready */
		}
		break;
	}

	// propagate DIR_ from Control to Status register
	m_status = (m_status & ~DIR_) | (data & DIR_);

	switch (data & HSF_PCB_MASK)
	{
	case HSF_PCB_ACK: // HSF1
		if (m_wait_for_ack)
		{
			set_command_pending(0);
		}
		break;

	case HSF_PCB_END: // (HSF2|HSF1)
		m_status &= ~ACRF; /* adapter command register is not full */
		// fall through

	case HSF_PCB_NAK: // HSF2
		if (m_microcode_running)
		{
			if (m_wait_for_nak)
			{
				set_command_pending(0);
			}
			m_status = (m_status & ~ASF_PCB_MASK) | ASF_PCB_ACK;
		}
		break;

	default: // 0
		m_command_index = 0;
		m_status |= HCRE; /* host command register is empty */
		break;
	}

	m_control = data;
}

/***************************************************************************
 read_status_port
 ***************************************************************************/

UINT8 threecom3c505_device::read_status_port()
{
	UINT8 data = m_status;
	m_status &= ~ASF_PCB_MASK;

	switch (data & ASF_PCB_MASK)
	{
	case ASF_PCB_END:
		m_status |= ASF_PCB_ACK;
		break;
	}
	return data;
}


/***************************************************************************
 write_port
 ***************************************************************************/

WRITE16_MEMBER(threecom3c505_device::write)
{
	// make byte offset
	offset *= 2;

	m_reg[offset & 0x0f] = data;
	LOG2(this,("writing 3C505 Register at offset=%02x with mem_mask=%04x = %04x", offset, mem_mask, data));

	switch (offset)
	{
	case PORT_COMMAND: /* 0x00 read/write, 8-bit */
		write_command_port(data);
		break;
	case PORT_AUXDMA: /* 0x02 write only, 8-bit */
		break;
	case PORT_DATA: /* 0x04 read/write, 16-bit */
		write_data_port(data & 0xff);
		write_data_port(data >> 8);
		break;
	case PORT_CONTROL: /* 0x06 read/write, 8-bit */
		write_control_port(data);
		break;
	default:
		break;
	}
}

/***************************************************************************
 read_port
 ***************************************************************************/

READ16_MEMBER(threecom3c505_device::read)
{
	// data to omit excessive logging
	static UINT16 last_data = 0xff;
	static UINT32 last_pc = 0;

	// make byte offset
	offset *= 2;

	UINT16 data = m_reg[offset & 0x0f];
	switch (offset)
	{
	case PORT_COMMAND: /* 0x00 read/write, 8-bit */
		data = read_command_port();
		break;
	case PORT_STATUS: /* 0x02 read only, 8-bit */
		data = read_status_port();

		// omit excessive logging
		if (data == last_data)
		{
			UINT32 pc = space.device().safe_pcbase();
			if (pc == last_pc)
			{
				return data;
			}
			last_pc = pc;
		}
		last_data = data;
		break;
	case PORT_DATA: /* 0x04 read/write, 16-bit */
		data = read_data_port();
		data |= (read_data_port() << 8);
		break;
	case PORT_CONTROL: /* 0x06 read/write, 8-bit */
		data = m_control;
		break;
	default:
		break;
	}

	LOG2(this,("reading 3C505 Register at offset=%02x with mem_mask=%04x = %04x", offset, mem_mask, data));

	return data;
}

void threecom3c505_device::set_verbose(int on_off)
{
	verbose = on_off == 0 ? 0 : VERBOSE > 1 ? VERBOSE : 1;
}

int threecom3c505_device::tx_data(device_t *device, const UINT8 data[], int length)
{
	LOG1(device,("threecom3c505_device::tx_data length=%d", length));
	return 1;
}

int threecom3c505_device::setfilter(device_t *device, int node_id)
{
	return 0;
}
