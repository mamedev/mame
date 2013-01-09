/*
 * sc499.c - ARCHIVE SC-499 cartridge tape controller (for Apollo DN3x00)
 *_DeviceClass
 *  Created on: April 17, 2011
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf
 *  - http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 *  - http://www.bitsavers.org/pdf/archive/20271-001_scorpPrDesc_Mar84.pdf
 *  - http://www.bitsavers.org/pdf/archive/SidewinderProdDescr.pdf
 *
 */

#include "machine/sc499.h"

#define VERBOSE 0

static int verbose = VERBOSE;

#define LOG(x)	{ logerror ("%s: ", cpu_context()); logerror x; logerror ("\n"); }
#define LOG1(x)	{ if (verbose > 0) LOG(x)}
#define LOG2(x)	{ if (verbose > 1) LOG(x)}
#define LOG3(x)	{ if (verbose > 2) LOG(x)}

#define  MAINCPU "maincpu"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

//const device_type SC499 = sc499_device_config::static_alloc_device_config;

sc499_device *sc499_device::m_device;

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// I/O register offsets

#define SC499_PORT_DATA       0x00    // read data (status data)
#define SC499_PORT_COMMAND    0x00    // write command
#define SC499_PORT_STATUS     0x01    // read status
#define SC499_PORT_CONTROL    0x01    // write control
#define SC499_PORT_DMAGO      0x02    // write only, Start DMA
#define SC499_PORT_RSTDMA     0x03    // write only, Reset DMA

// Tape Status Byte 0 (LSB)

#define SC499_ST0     0x8000    // 0=> Status byte 0
#define SC499_ST0_NOC 0x4000    // 1=> Cartridge not in place
#define SC499_ST0_USD 0x2000    // 1=> Unselected drive
#define SC499_ST0_WP  0x1000    // 1=> Write protected cartridge
#define SC499_ST0_EOM 0x0800    // 1=> End of media (EOM)
#define SC499_ST0_UDE 0x0400    // 1=> Unrecoverable data error
#define SC499_ST0_BNL 0x0200    // 1=> Bad block not located
#define SC499_ST0_FM  0x0100    // 1=> Filemark detected
#define SC499_ST0_MASK 0x7f00

// Tape Status Byte 1 (MSB)

#define SC499_ST1     0x0080    // 1=> Status byte 1
#define SC499_ST1_ILL 0x0040    // 1=> Illegal command
#define SC499_ST1_NOD 0x0020    // 1=> No data detected
#define SC499_ST1_MGN 0x0010    // 1=> Marginal block detected
#define SC499_ST1_BOM 0x0008    // 1=> Beginning of media (BOM)
#define SC499_ST1_BPE 0x0004    // 1=> Reserved for bus parity error
#define SC499_ST1_ERM 0x0002    // 1=> Reserved for end of recorded media
#define SC499_ST1_POR 0x0001    // 1=> Power on/reset occurred
#define SC499_ST1_MASK 0x007f

#define SC499_ST_CLEAR_ALL      ~(SC499_ST0_NOC | SC499_ST0_USD | SC499_ST0_WP | SC499_ST0_EOM |  SC499_ST1_BOM)
#define SC499_ST_READ_ERROR      (SC499_ST0_UDE | SC499_ST0_BNL |  SC499_ST1_NOD)

// Tape Status word

//dc.w $EFFF,$C000 // 3a No cartridge
//dc.w $FFFF,$F000 // 39 No drive
//dc.w $FFFF,$8800 // 38 End of media
//dc.w $EFFF,$8488 // 37 Write abort
//dc.w $EFFF,$8400 // 36 Read error, bad blk xfer
//dc.w $EFFF,$8600 // 35 Read error, filler blk xfer
//dc.w $EFFF,$86A0 // 34 Read error, no data
//dc.w $EFFF,$8EA0 // 33 Read error, no data and EOM
//dc.w $EFE9,$86A8 // 32 Read error, no data and BOM
//dc.w $EFFF,$8100 // 31 Filemark read
//dc.w $0FF7,$00C0 // 30 Illegal command

// Command Register (write) [50000 | 3FF9C00 ]

#define SC499_CMD_SELECT         0x00   // Select
#define SC499_CMD_SEL_1          0x01   // select tape 1
#define SC499_CMD_SEL_2          0x02   // select tape 2
#define SC499_CMD_SEL_3          0x04   // select tape 3
#define SC499_CMD_SEL_4          0x08   // select tape 4
#define SC499_CMD_POSITION       0x20   // Position
#define SC499_CMD_REWIND         0x21   // rewind tape
#define SC499_CMD_ERASE          0x22   // erase tape
#define SC499_CMD_RETEN          0x24   // retension tape
#define SC499_CMD_WRITE_DATA     0x40   // Write Data
#define SC499_CMD_WRITE_FILEMARK 0x60   // Write File Mark
#define SC499_CMD_READ_DATA      0x80   // Read Data
#define SC499_CMD_READ_FILE_MARK 0xa0   // Read File Mark
#define SC499_CMD_READ_STATUS    0xc0   // Read Status
#define SC499_CMD_RESERVED       0xe0   // Reserved
#define SC499_CMD_NO_COMMAND     0xff   // no command

#define SC499_CMD_TYPE_MASK      0xe0   // command type mask
#define SC499_CMD_DATA_MASK      0x1f   // command data mask

// Status Register (read only)

#define SC499_STAT_IRQ 0x80   // active high, Interrupt request flag ('or' of rdy and exc), and done if dni is set.
#define SC499_STAT_RDY 0x40   // active low, Ready from LSI chip.
#define SC499_STAT_EXC 0x20   // active low, Exception, from LSI chip.
#define SC499_STAT_DON 0x10   // active high, DMA done (from DMA logic)
#define SC499_STAT_DIR 0x08   // active high, Direction, indicates direction of bus is from controller to DN3000.

// Control Register (write only) [50001 | 3FF9C01 ]

// Note: All of these bits are cleared automatically when writing to SC499_PORT_RSTDMA.
// So SC499_CTR_IEN and SC499_CTR_DNI must be reprogrammed before the write to AR_START_DMA_PORT.

#define SC499_CTR_RST 0x80    // 1=> Reset controller microprocessor.
#define SC499_CTR_REQ 0x40    // 1=> Request to LSI chip.
#define SC499_CTR_IEN 0x20    // 1=> Enables interrupts.
#define SC499_CTR_DNI 0x10    // 1=> Enables DONE int, dni = 0 masks DONE int

// ctape device data

#define SC499_CTAPE_TAG "sc499_ctape"
#define SC499_CTAPE_BLOCK_SIZE 512
#define SC499_CTAPE_MAX_BLOCK_COUNT (60*1024*1024/SC499_CTAPE_BLOCK_SIZE)

// mean time to read one block from ctape (should be 5-6 msec)
#define SC499_CTAPE_READ_BLOCK_TIME 6

// Timer type
#define SC499_TIMER_1 1
#define SC499_TIMER_2 2
#define SC499_TIMER_3 3
#define SC499_TIMER_4 4
#define SC499_TIMER_5 5
#define SC499_TIMER_6 6
#define SC499_TIMER_7 7


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// device type definition
const device_type SC499 = &device_creator<sc499_device>;

//-------------------------------------------------
// sc499_device - constructor
//-------------------------------------------------

sc499_device::sc499_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SC499, "Archive SC-499", tag, owner, clock)
{
	memset(static_cast<sc499_interface *>(this), 0, sizeof(sc499_interface));
}

//-------------------------------------------------
//  static_set_interface - set the interface struct
//-------------------------------------------------

void sc499_device::static_set_interface(device_t &device, const sc499_interface &interface)
{
	sc499_device &sc499 = downcast<sc499_device &>(device);
	static_cast<sc499_interface &>(sc499) = interface;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sc499_device::device_start()
{
	// FIXME: temporary hack: we allow only one device:
	sc499_device::m_device = this;

	LOG1(("start sc499"));

	m_timer = m_device->machine().scheduler().timer_alloc(FUNC(static_timer_func), this);
	m_timer1 = m_device->machine().scheduler().timer_alloc(FUNC(static_timer_func), this);

	device_t *ctape_device = machine().device(SC499_CTAPE_TAG);
	m_image = dynamic_cast<device_image_interface *> (ctape_device);

	if (m_image->image_core_file() == NULL)
	{
		LOG2(("start sc499: no cartridge tape"));
	}
	else
	{
		LOG2(("start sc499: cartridge tape image is %s",m_image->filename()));
	}

	m_ctape_block_buffer = auto_alloc_array(machine(), UINT8, SC499_CTAPE_BLOCK_SIZE);
	assert(m_ctape_block_buffer!= NULL);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sc499_device::device_reset()
{
	LOG1(("reset sc499"));

	m_data = 0;
	m_command = SC499_CMD_NO_COMMAND;
	m_status  = SC499_STAT_RDY; // | SC499_STAT_EXC;
	m_control = 0;

	m_first_block_hack = 1;
	m_nasty_readahead = 0;
	m_read_block_pending = 0;

	m_current_command = m_command;

	m_data_error_counter = 0;
	m_underrun_counter = 0;

	m_image_length = 0;

	check_tape();

	m_data_index = 0;
	m_ctape_block_index = 0;
	m_tape_pos = 0;

	irq_state = CLEAR_LINE;
	dma_drq_state = CLEAR_LINE;
}

/*-------------------------------------------------
 cpu_context - return a string describing the current CPU context
 -------------------------------------------------*/

const char *sc499_device::cpu_context()
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

/*-------------------------------------------------
 tape_status_clear - clear bits in tape status
 -------------------------------------------------*/

void sc499_device::tape_status_clear(UINT16 value)
{
	m_tape_status &= ~value;
	tape_status_set(0);
}

/*-------------------------------------------------
 tape_status_set - set bits in tape status
 -------------------------------------------------*/

void sc499_device::tape_status_set(UINT16 value)
{
	m_tape_status |= value;
	m_tape_status &= ~(SC499_ST0 | SC499_ST1);
	if (m_tape_status & SC499_ST0_MASK)
	{
		m_tape_status |= SC499_ST0;
	}
	if (m_tape_status & SC499_ST1_MASK)
	{
		m_tape_status |= SC499_ST1;
	}
}

/*-------------------------------------------------
 check_tape - check tape
 -------------------------------------------------*/

void sc499_device::check_tape()
{
	m_tape_status = 0;
	m_is_writable = m_image->exists() && m_image->is_writeable();

	if (m_image->exists())
	{
		m_has_cartridge = 1; // we have a cartridge
		tape_status_set(SC499_ST1_BOM); // Beginning of media (BOM)
		if (!m_is_writable)
		{
			tape_status_set(SC499_ST0_WP);
		}

		if (m_image_length != m_image->length())
		{
			// tape has changed, get new size
			m_image_length = m_image->length();
			m_ctape_block_count = (UINT32)((m_image_length+SC499_CTAPE_BLOCK_SIZE-1) / SC499_CTAPE_BLOCK_SIZE);
		}

		LOG1(("check_tape: tape image is %s with %d blocks", m_image->filename(), m_ctape_block_count));
	}
	else
	{
		m_has_cartridge = 0; // we have no cartridge
		tape_status_set(SC499_ST0_NOC); // Cartridge not in place
		m_image_length = 0;
		m_ctape_block_count = 0;

		LOG1(("check_tape: no cartridge tape"));
	}
}

/*-------------------------------------------------
 timer_func - handle timer interrupts
 -------------------------------------------------*/

void sc499_device::timer_func(int timer_type)
{
	LOG2(("timer_func param=%d status=%x",timer_type, m_status));

	switch (timer_type)
	{
	case SC499_TIMER_1: // set ready
		m_status &= ~SC499_STAT_RDY;
		if (m_control & SC499_CTR_IEN)
		{
			set_interrupt(ASSERT_LINE);
			m_status |= SC499_STAT_IRQ;
		}
		break;

	case SC499_TIMER_2: // set exception, clear interrupt (?)
		m_status &= ~SC499_STAT_EXC;
		m_status &= ~SC499_STAT_IRQ;
		break;

	case SC499_TIMER_3: // start dma to read data
		set_dma_drq(ASSERT_LINE);
		m_status |= SC499_STAT_RDY;
		break;

	case SC499_TIMER_4: // ready to read next status byte
		m_status &= ~SC499_STAT_RDY;

		switch (++m_data_index)
		{
		case 1: m_data = m_tape_status >> 8;  break;
		case 2: m_data = m_tape_status & 0xff;	 break;
		case 3: m_data = m_data_error_counter >> 8; break;
		case 4: m_data = m_data_error_counter & 0xff; break;
		case 5: m_data = m_underrun_counter >> 8; break;
		case 6: m_data = m_underrun_counter & 0xff; break;
		default:
			m_data_index=0;
			m_command = SC499_CMD_NO_COMMAND;
			m_current_command = m_command;
			m_status &= ~SC499_STAT_DIR;
			tape_status_clear(SC499_ST_CLEAR_ALL);
			break;
		}

		if (m_control & SC499_CTR_IEN)
		{
			set_interrupt(ASSERT_LINE);
			m_status |= SC499_STAT_IRQ;
		}
		break;

	case SC499_TIMER_5: // read next data block for SC499_CMD_READ_DATA
	case SC499_TIMER_6: // read next data block for SC499_CMD_READ_FILE_MARK
		if (m_read_block_pending && (m_status & SC499_STAT_DIR))
		{
			// handle data underruns in a loaded Apollo emulation
			// the real ctape will stop, go back and restart reading if appropriate
			if (++m_underrun_counter >= 5000)
			{
				// stop tape (after 30 seconds) - probably the DMA handshake failed.
				m_timer1->adjust(attotime::never, timer_type, attotime::never);
				m_status &= ~SC499_STAT_EXC;
				m_status &= ~SC499_STAT_DIR;
				tape_status_clear(SC499_ST_CLEAR_ALL);
				tape_status_set(SC499_ST_READ_ERROR);

				LOG(("timer_func param=%d status=%x tape_pos=%d - read data underrun aborted at %d",
						timer_type, m_status, m_tape_pos, m_underrun_counter));
			}
			else
			{
				LOG2(("timer_func param=%d status=%x tape_pos=%d - read data underrun %d",
						timer_type, m_status, m_tape_pos, m_underrun_counter));
			}
			break;
		}
		else if ( m_tape_pos > m_ctape_block_count || !(m_status & SC499_STAT_RDY))
		{
			LOG1(("timer_func param=%d status=%x tape_pos=%d - end-of-tape or not ready",
								timer_type, m_status, m_tape_pos));
			m_timer1->adjust(attotime::never, timer_type, attotime::never);
			m_status &= ~SC499_STAT_EXC;
			m_status &= ~SC499_STAT_DIR;
			tape_status_clear(SC499_ST_CLEAR_ALL);
			tape_status_set(SC499_ST_READ_ERROR);
		}
		else if (m_nasty_readahead > 0 )
		{
			m_nasty_readahead = 0;
		}
		else
		{
			if  (m_underrun_counter > 0)
			{
				LOG1(("timer_func param=%d status=%x tape_pos=%d - read data underrun ended at %d",
						timer_type, m_status, m_tape_pos, m_underrun_counter));
			}

			read_block();
			m_underrun_counter = 0;
			if (block_is_filemark())
			{
				m_timer1->adjust(attotime::never, timer_type, attotime::never);
			}
			else if (m_current_command == SC499_CMD_READ_DATA)
			{
				m_read_block_pending = 1;
				m_status &= ~SC499_STAT_RDY;
			}
		}

		if (m_control & SC499_CTR_IEN)
		{
			if (m_current_command == SC499_CMD_READ_DATA || (m_status & SC499_STAT_EXC) == 0)
			{
				set_interrupt(ASSERT_LINE);
				m_status |= SC499_STAT_IRQ;
			}
		}

		break;

	case SC499_TIMER_7: // reset
		 // set exception
		m_status &= ~SC499_STAT_EXC;
		if (m_control & SC499_CTR_IEN)
		{
			set_interrupt(ASSERT_LINE);
			m_status |= SC499_STAT_IRQ;
		}
		break;

	default:
		LOG(("timer_func param=%d UNEXPECTED",timer_type));
		m_timer->reset();
		break;
	}

}

TIMER_CALLBACK( sc499_device::static_timer_func )
{
	reinterpret_cast<sc499_device *> (ptr)->timer_func(param);
}

/*-------------------------------------------------
 set_interrupt - set the IRQ state
 -------------------------------------------------*/

void sc499_device::set_interrupt(enum line_state state)
{
	if (set_irq != NULL && state != irq_state)
	{
		LOG2(("set_interrupt(%d)",state));
		(*set_irq)(this, state);
		irq_state = state;
	}
}

/*-------------------------------------------------
 set_dma_drq - set dma request output
 -------------------------------------------------*/

void sc499_device::set_dma_drq(enum line_state state)
{
	if (dma_drq != NULL && state != dma_drq_state)
	{
		LOG2(("set_dma_drq(%d)",state));
		(*dma_drq)(this, state);
		dma_drq_state = state;
	}
}

// -------------------------------------

void sc499_device::log_command(UINT8 data)
{
	switch (data)
	{
	case SC499_CMD_SELECT:
	case SC499_CMD_SEL_1:
	case SC499_CMD_SEL_2:
	case SC499_CMD_SEL_3:
	case SC499_CMD_SEL_4:
		LOG1(("write_command_port: %02x Select %x", data, data & 0x1f));
		break;
	case SC499_CMD_REWIND: // rewind tape
		LOG1(("write_command_port: %02x Rewind Tape", data));
		break;
	case SC499_CMD_ERASE:  // erase tape
		LOG1(("write_command_port: %02x Erase Tape", data));
		break;
	case SC499_CMD_RETEN:  // retention tape
		LOG1(("write_command_port: %02x Retention Tape", data));
		break;
	case SC499_CMD_WRITE_DATA:
		LOG1(("write_command_port: %02x Write Data %d ...", data, m_tape_pos));
		break;
	case SC499_CMD_WRITE_FILEMARK:
		LOG1(("write_command_port: %02x Write File Mark %d", data, m_tape_pos));
		break;
	case SC499_CMD_READ_DATA:
		LOG1(("write_command_port: %02x Read Data %d ...", data, m_tape_pos));
		break;
	case SC499_CMD_READ_FILE_MARK:
		LOG1(("write_command_port: %02x Read File Mark %d", data, m_tape_pos));
		break;
	case SC499_CMD_READ_STATUS:
		LOG1(("write_command_port: %02x Read Status (%04x %04x %04x)", data,
				m_tape_status, m_data_error_counter, m_underrun_counter));
		break;
	case SC499_CMD_RESERVED: // Reserved
	default:
		LOG(("write_command_port: %02x Unexpected Command!", data));
		break;
	}
}

void sc499_device::do_command(UINT8 data)
{
	m_status |= SC499_STAT_RDY;
	m_status &= ~SC499_STAT_DON;
	m_read_block_pending = 0;

	switch (data & SC499_CMD_TYPE_MASK)
	{
	case SC499_CMD_SELECT:
		m_timer->adjust(attotime::from_usec(100), SC499_TIMER_1);
		break;

	case SC499_CMD_POSITION:
		switch (data)
		{
		case SC499_CMD_REWIND: // rewind tape
			break;
		case SC499_CMD_ERASE:  // erase tape
			// TODO:
			break;
		case SC499_CMD_RETEN:  // retention tape
			break;
		}
		m_first_block_hack = 0;
		m_tape_pos = 0;
		tape_status_clear(SC499_ST_CLEAR_ALL);
		tape_status_set(SC499_ST1_BOM);

		m_timer->adjust(attotime::from_msec(500), SC499_TIMER_4);
		break;

	case SC499_CMD_WRITE_DATA:
		if (!m_is_writable)
		{
			m_status &= ~SC499_STAT_EXC;
		}
		m_timer->adjust(attotime::from_usec(100), SC499_TIMER_1);
		break;

	case SC499_CMD_WRITE_FILEMARK:
		if (!m_is_writable)
		{
			m_status &= ~SC499_STAT_EXC;
		}
		else
		{
			block_set_filemark();
			write_block();
		}
		m_timer->adjust( attotime::from_msec(100), SC499_TIMER_1);
		break;

	case SC499_CMD_READ_DATA:
		m_status |= SC499_STAT_DIR;
		// start reading blocks from tape
		m_nasty_readahead = 0;
		m_timer1->adjust( attotime::from_msec(200), SC499_TIMER_5, attotime::from_msec(SC499_CTAPE_READ_BLOCK_TIME));
		break;

	case SC499_CMD_READ_FILE_MARK:
		if (m_current_command == SC499_CMD_READ_DATA)
		{
			// SC499_CMD_READ_DATA pending
			m_status &= ~SC499_STAT_DIR;
			LOG1(("do_command: Read data -> Read File Mark at %d", m_tape_pos));
		}
		else
		{
			// start reading blocks from tape
			m_timer1->adjust( attotime::from_msec(200), SC499_TIMER_6, attotime::from_msec(SC499_CTAPE_READ_BLOCK_TIME));
		}
		break;

	case SC499_CMD_READ_STATUS:
		m_status |= SC499_STAT_DIR;
		m_status |= SC499_STAT_EXC;

		set_interrupt(CLEAR_LINE);
		set_dma_drq(CLEAR_LINE);
		m_timer1->adjust( attotime::never, SC499_TIMER_6, attotime::never);

		m_data_index = 0;
		m_timer->adjust( attotime::from_usec(100), SC499_TIMER_4);
		break;

	default:
		tape_status_clear(SC499_ST_CLEAR_ALL);
		tape_status_set(SC499_ST1 | SC499_ST1_ILL);
		m_status &= ~SC499_STAT_EXC;
		// Note: 100 usec is too fast for DN3000
		m_timer->adjust( attotime::from_usec(200), SC499_TIMER_1);
		break;
	}
}

// -------------------------------------

void sc499_device::do_reset()
{
	LOG1(("do_reset: Reset controller microprocessor"));

	m_data = 0;
	m_command = SC499_CMD_NO_COMMAND;
	m_current_command = m_command;
	m_status = ~(SC499_STAT_DIR | SC499_STAT_EXC);
	m_control = 0;

	if (m_has_cartridge)
	{
		tape_status_set(SC499_ST1_POR | SC499_ST1_BOM);
	}

	m_tape_pos = 0;
	m_data_error_counter = 0;
	m_underrun_counter = 0;

	set_interrupt(CLEAR_LINE);
	set_dma_drq(CLEAR_LINE);

	m_timer1->adjust( attotime::never, SC499_TIMER_6, attotime::never);
}

// -------------------------------------

void sc499_device::write_command_port(UINT8 data)
{
	m_command = data;
	log_command(data);
}

UINT8 sc499_device::read_data_port()
{
	static UINT8 m_last_data = 0xff;

	// omit excessive logging
	if (m_last_data != m_data)
	{
		LOG2(("read_data_port: %02x", m_data));
		m_last_data = m_data;
	}

	if (m_control & SC499_CTR_IEN)
	{
		set_interrupt(CLEAR_LINE);
		m_status &= ~SC499_STAT_IRQ;
	}

	return m_data;
}

void sc499_device::write_control_port( UINT8 data)
{
	LOG2(("write_control_port: %02x", data));

	if ((data ^ m_control) & SC499_CTR_RST)
	{
		if (data & SC499_CTR_RST)
		{
			// SC499_CTR_RST has changed to 1
			do_reset();
		}
		else
		{
			m_status |= SC499_STAT_EXC;
			m_timer->adjust(attotime::from_msec(200), SC499_TIMER_7);
		}
	}

	if ((data ^ m_control) & SC499_CTR_REQ)
	{
		// Request to LSI chip has changed
		if (data & SC499_CTR_REQ)
		{
			LOG3(("write_control_port: Request to LSI chip = On"));

			if (m_command == SC499_CMD_READ_STATUS) {
				m_status |= SC499_STAT_EXC;
			}

			if (m_command == SC499_CMD_READ_FILE_MARK && m_current_command == SC499_CMD_READ_DATA)
			{
				m_current_command= SC499_CMD_READ_FILE_MARK;
				m_status &= ~SC499_STAT_DIR;
			}

			if (!(m_status & SC499_STAT_DIR))
			{
				// write command and command data
				// (SC499_STAT_RDY follows SC499_CTR_REQ)
				m_status &= ~SC499_STAT_RDY;
			}
			else
			{
				m_status |= SC499_STAT_RDY;
			}
		}
		else
		{
			LOG2(("write_control_port: Request to LSI chip = Off (%d)", m_data_index));

			if (!(m_status & SC499_STAT_DIR))
			{
				do_command(m_command);
				m_current_command = m_command;
			}
			else if (m_command == SC499_CMD_READ_STATUS)
			{
//              if (m_data_index > 0)
				{
					m_timer->adjust( attotime::from_usec(20), SC499_TIMER_4);
				}
			}
		}
	}
	m_control = data;
}

UINT8 sc499_device::read_status_port()
{
	static UINT8 m_last_status = 0xff;

	// omit excessive logging
	if (m_last_status != m_status)
	{
		LOG2(("read_status_port: %02x", m_status));
		m_last_status = m_status;
	}

	// reset pending interrupts
	set_interrupt(CLEAR_LINE);
	m_status &= ~SC499_STAT_IRQ;

	return m_last_status;
}

// Start DMA (DMAGO). Any write to this register will cause DMAGO to be active.

void sc499_device::write_dma_go( UINT8 data)
{
	LOG2(("write_dma_go: %02x", data));

	m_status &= ~SC499_STAT_DON;

	switch (data)
	{
	case 0:
		// delay set_dma_drq(ASSERT_LINE);
		// Note: 200 usec may fail for MD LD; 400 usec may fail for RBAK_SHELL; 500 usec may fail for DN3000
		m_timer->adjust( attotime::from_msec(1), SC499_TIMER_3);

// Note: proper sequence is:
//      pc=3c4ad714 - ctape: write_dma_go: 00
//      pc=3c4ad718 - ctape: write_control_port: 30
//      pc=3c40ebda: apollo_dma_1_w: writing DMA Controller 1 at offset 02 = 00
//      pc=3c40ebe8: apollo_dma_1_w: writing DMA Controller 1 at offset 02 = 22
//      pc=3c40ebf4: apollo_dma_1_w: writing DMA Controller 1 at offset 0c = 00
//      pc=3c40ec04: apollo_dma_1_w: writing DMA Controller 1 at offset 03 = ff
//      pc=3c40ec12: apollo_dma_1_w: writing DMA Controller 1 at offset 03 = 01
//      pc=3c40ec1e: apollo_dma_1_w: writing DMA Controller 1 at offset 0b = 45
//      pc=3c40ec2e: apollo_dma_1_w: writing DMA Controller 1 at offset 0a = 01
//      pc=3c41a154 - ctape: timer_func param=3 status=2f
//      pc=3c41a154 - ctape: set_dma_drq(1)
//      pc=3c41a154 - maincpu: apollo_dma_ctape_drq: state=1
//      pc=3c41a154 - ctape: dack_read: data[0]=55 status=6f
//      pc=3c41a154: dma write byte at offset 17f9c00+200 = 55
//      pc=3c41a154: dma write byte at offset 17f9c00+2ff = 6d
//      pc=3c41a154: dma write byte at offset 17f9c00+300 = 65
//      pc=3c41a154 - ctape: dack_read: data[511]=60 status=6f
//      pc=3c41a154: dma write byte at offset 17f9c00+3ff = 60
//      pc=3c41a154 - dma8237_1: dma out eop state 00
//      pc=3c41a154 - ctape: set_tc_state: block=21 state=0
//      pc=3c41a154 - ctape: set_dma_drq(0)
//      pc=3c41a154 - maincpu: apollo_dma_ctape_drq: state=0
//      pc=3c41a154 - dma8237_1: dma out eop state 01
//      pc=3c41a154 - ctape: set_tc_state: block=21 state=1

		break;
	}
}

// Reset DMA (RSTDMA). Any write to this register will cause RSTDMA to be active.

void sc499_device::write_dma_reset( UINT8 data)
{
	LOG2(("write_dma_reset: %02x", data));

	m_status &= ~SC499_STAT_DON;
	m_control = 0;
}

void sc499_device::write_port(offs_t offset, UINT8 data)
{
	switch (offset)
	{
	case SC499_PORT_COMMAND: // write command
		write_command_port(data);
		break;
	case SC499_PORT_CONTROL: // write control
		write_control_port(data);
		break;
	case SC499_PORT_DMAGO: // write only, Start DMA
		write_dma_go(data);
		break;
	case SC499_PORT_RSTDMA: // write only, Reset DMA
		write_dma_reset(data);
		break;
	default:
		LOG(("writing sc499 Register at offset %02x = %02x", offset, data));
		break;
	}
}

//READ8_DEVICE_HANDLER( sc499_r )
UINT8 sc499_device::read_port(offs_t offset)
{
	UINT8 data = 0xff;

	switch (offset)
	{
	case SC499_PORT_DATA: // read data (status data)
		data = read_data_port();
		break;
	case SC499_PORT_STATUS: // read status
		data=read_status_port();
//      set_interrupt(CLEAR_LINE);
		break;
	default:
		LOG(("reading sc499 Register at offset %02x = %02x", offset, data));
		break;
	}

	return data;
}

void sc499_device::set_tc_state(int state)
{
	LOG2(("set_tc_state: block=%d state=%x", m_tape_pos-1, state));
	if (state == 0)
	{
		m_status |= SC499_STAT_DON; // 37ec
		set_dma_drq(CLEAR_LINE);

		switch (m_current_command & SC499_CMD_TYPE_MASK)
		{
		case SC499_CMD_READ_DATA:
			m_read_block_pending = 0;
			break;

		case SC499_CMD_WRITE_DATA:
			m_status &= ~SC499_STAT_RDY;
			if ((m_control & SC499_CTR_IEN) && (m_control & SC499_CTR_DNI))
			{
				set_interrupt(ASSERT_LINE);
				m_status |= SC499_STAT_IRQ;
			}
			break;
		}
	}
}

UINT8 sc499_device::dack_read() {
	UINT8 data = 0xff;

//  set_dma_drq(CLEAR_LINE);

	if (m_ctape_block_index >= SC499_CTAPE_BLOCK_SIZE)
	{
		LOG3(("dack_read: read_block"));
		read_block();
		m_nasty_readahead++;

		if (block_is_filemark())
		{
			set_dma_drq(CLEAR_LINE);
			m_status &= ~SC499_STAT_EXC;
			m_status &= ~SC499_STAT_DIR;
		}
	}

	data = m_ctape_block_buffer[m_ctape_block_index++];
	if (m_ctape_block_index < 2 || m_ctape_block_index > 511)
	{
		LOG2(("dack_read: data[%d]=%x status=%x",  m_ctape_block_index-1, data, m_status));
	}

//  if (m_ctape_block_index < SC499_CTAPE_BLOCK_SIZE)
//  {
//      set_dma_drq(ASSERT_LINE);
//  }

	return data;
}

void sc499_device::dack_write(UINT8 data) {
	LOG3(("dack_write: data=%x", data));

	if (m_ctape_block_index < SC499_CTAPE_BLOCK_SIZE)
	{
		m_ctape_block_buffer[m_ctape_block_index++] = data;
	}

	if (m_ctape_block_index == SC499_CTAPE_BLOCK_SIZE)
	{
		LOG3(("dack_write: write_block"));
		write_block();
	}
}

/*-------------------------------------------------
 log_block - log block data
 -------------------------------------------------*/

void sc499_device::log_block(const char *text)
{
	int data_length = 16;

	if (verbose > 0) {
		int i;
		logerror("%s: %s %d -", cpu_context(), text, m_tape_pos);
		for (i = 0; i < data_length && i < SC499_CTAPE_BLOCK_SIZE; i++) {
			logerror(" %02x", m_ctape_block_buffer[i]);
		}

		if (i < SC499_CTAPE_BLOCK_SIZE) {
			logerror(" ...");
		}

		if (m_ctape_block_index > 0 && m_ctape_block_index != SC499_CTAPE_BLOCK_SIZE)
		{
			logerror(" block_index = %d !!!", m_ctape_block_index);
		}

		logerror("\n");
	}
}

/*-------------------------------------------------
 read_block - read block from m_tape_pos to m_ctape_block_buffer
 -------------------------------------------------*/

void sc499_device::read_block()
{
	if (m_tape_pos == 0)
	{
		// check if tape has been replaced or removed
		check_tape();
	}

	m_image->fseek((UINT64) m_tape_pos * SC499_CTAPE_BLOCK_SIZE, SEEK_SET);

	if (m_image->image_feof())
	{
		m_status &= ~SC499_STAT_EXC;
		m_status &= ~SC499_STAT_DIR;
		m_status &= ~SC499_STAT_DON;
		tape_status_clear(SC499_ST_CLEAR_ALL);
		tape_status_set(SC499_ST_READ_ERROR);
	}
	else
	{
		m_image->fread(m_ctape_block_buffer, SC499_CTAPE_BLOCK_SIZE);

		//  if (verbose > 1 || m_tape_pos % 100 == 0)
		{
			log_block("read_block");
		}

		m_ctape_block_index = 0;
		m_tape_pos++;

		if (m_first_block_hack)
		{
			// FIXME: we must read first block twice (in MD for 'di c' and 'ld' or 'ex ...')
			// why is this necessary???
			m_tape_pos = 0;
			LOG(("read_block - duplicating block %d", m_tape_pos));
		}
		m_first_block_hack = 0;

		// we are no longer at Beginning of media (BOM)
		tape_status_clear(SC499_ST1_BOM);

		if (block_is_filemark())
		{
			m_status &= ~SC499_STAT_EXC;
			m_status &= ~SC499_STAT_DIR;
			tape_status_clear(SC499_ST_CLEAR_ALL);
			tape_status_set(SC499_ST0_FM);
		}
		else
		{
			tape_status_clear(SC499_ST0_FM);
		}
	}
	//  if (m_tape_pos == 69400) verbose = 2;
}

/*-------------------------------------------------
 write_block - write block from m_ctape_block_buffer to m_tape_pos
 -------------------------------------------------*/

void sc499_device::write_block()
{
	log_block("write_block");

	if (m_tape_pos == 0)
	{
		// check if tape has been replaced or removed
		check_tape();
	}

	m_image->fseek((UINT64) m_tape_pos * SC499_CTAPE_BLOCK_SIZE, SEEK_SET);
	m_image->fwrite(m_ctape_block_buffer, SC499_CTAPE_BLOCK_SIZE);
	m_ctape_block_count = m_tape_pos;
	m_ctape_block_index = 0;
	m_tape_pos++;

	// we are no longer at Beginning of media (BOM)
	tape_status_clear(SC499_ST1_BOM);
}

/*-------------------------------------------------
 block_is_filemark - returns 1 is current block buffer data is filemark
 -------------------------------------------------*/

int sc499_device::block_is_filemark()
{
	static const UINT8 fm_pattern[] = {0xDE, 0xAF, 0xFA, 0xED};

	int is_filemark = memcmp(m_ctape_block_buffer, fm_pattern, 4) == 0 &&
			memcmp(m_ctape_block_buffer, m_ctape_block_buffer+4, SC499_CTAPE_BLOCK_SIZE-4) == 0;

	LOG3(("block_is_filemark for block %d = %d", m_tape_pos-1, is_filemark));
	return is_filemark;
}

/*-------------------------------------------------
 block_set_filemark - set current block buffer data to filemark
 -------------------------------------------------*/

void sc499_device::block_set_filemark()
{
	static const UINT8 fm_pattern[] = {0xDE, 0xAF, 0xFA, 0xED};
	int i;

	for (i = 0; i < SC499_CTAPE_BLOCK_SIZE; i += 4)
	{
		memcpy(m_ctape_block_buffer + i, fm_pattern, 4);
	}
}

//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_DEVICE_HANDLER( sc499_r )
{
	return downcast<sc499_device *> (device)->read_port(offset);
}

WRITE8_DEVICE_HANDLER( sc499_w )
{
	downcast<sc499_device *> (device)->write_port(offset, data);
}

void sc499_set_tc_state(running_machine *machine, int state)
{
	sc499_device::m_device->set_tc_state(state);
}

UINT8 sc499_dack_r(running_machine *machine)
{
	return sc499_device::m_device->dack_read();
}

void sc499_dack_w(running_machine *machine, UINT8 data)
{
	sc499_device::m_device->dack_write(data);
}

void sc499_set_verbose(int on_off)
{
	verbose = on_off == 0 ? 0 : VERBOSE > 1 ? VERBOSE : 1;
}

//##########################################################################
class sc499_ctape_image_device :	public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	sc499_ctape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual iodevice_t image_type() const { return IO_MAGTAPE; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return "act"; }
	virtual const option_guide *create_option_guide() const { return NULL; }
protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start() { };
};

// device type definition
extern const device_type SC499_CTAPE;

const device_type SC499_CTAPE = &device_creator<sc499_ctape_image_device>;

sc499_ctape_image_device::sc499_ctape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SC499_CTAPE, "Cartridge Tape", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{
}

void sc499_ctape_image_device::device_config_complete()
{
	update_names(SC499_CTAPE, "ctape", "ct");
};

MACHINE_CONFIG_FRAGMENT( sc499_ctape )
	MCFG_DEVICE_ADD(SC499_CTAPE_TAG, SC499_CTAPE, 0)
MACHINE_CONFIG_END
