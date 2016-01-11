// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * sc499.c - ARCHIVE SC-499 cartridge tape controller (for Apollo DN3x00)
 *  Created on: April 17, 2011
 *      Author: Hans Ostermeyer
 *      ISA conversion by R. Belmont
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf
 *  - http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 *  - http://www.bitsavers.org/pdf/archive/20271-001_scorpPrDesc_Mar84.pdf
 *  - http://www.bitsavers.org/pdf/archive/SidewinderProdDescr.pdf
 *
 */

#include "sc499.h"
#include "formats/ioprocs.h"

#define VERBOSE 0

static int verbose = VERBOSE;

#define LOG(x)  { logerror ("%s: ", cpu_context()); logerror x; logerror ("\n"); }
#define LOG1(x) { if (verbose > 0) LOG(x)}
#define LOG2(x) { if (verbose > 1) LOG(x)}
#define LOG3(x) { if (verbose > 2) LOG(x)}

#define SC499_CTAPE_TAG "sc499_ctape"
extern const device_type SC499_CTAPE;

static INPUT_PORTS_START( sc499_port )
	PORT_START("IO_BASE")
	PORT_DIPNAME( 0x3f8, 0x200, "SC499 I/O base (jumpers A3-A9)")
	PORT_DIPSETTING(     0x008, "008h" )
	PORT_DIPSETTING(     0x010, "010h" )
	PORT_DIPSETTING(     0x018, "018h" )
	PORT_DIPSETTING(     0x020, "020h" )
	PORT_DIPSETTING(     0x028, "028h" )
	PORT_DIPSETTING(     0x030, "030h" )
	PORT_DIPSETTING(     0x038, "038h" )
	PORT_DIPSETTING(     0x040, "040h" )
	PORT_DIPSETTING(     0x048, "048h" )
	PORT_DIPSETTING(     0x050, "050h" )
	PORT_DIPSETTING(     0x058, "058h" )
	PORT_DIPSETTING(     0x060, "060h" )
	PORT_DIPSETTING(     0x068, "068h" )
	PORT_DIPSETTING(     0x070, "070h" )
	PORT_DIPSETTING(     0x078, "078h" )
	PORT_DIPSETTING(     0x080, "080h" )
	PORT_DIPSETTING(     0x088, "088h" )
	PORT_DIPSETTING(     0x090, "090h" )
	PORT_DIPSETTING(     0x098, "098h" )
	PORT_DIPSETTING(     0x0a0, "0a0h" )
	PORT_DIPSETTING(     0x0a8, "0a8h" )
	PORT_DIPSETTING(     0x0b0, "0b0h" )
	PORT_DIPSETTING(     0x0b8, "0b8h" )
	PORT_DIPSETTING(     0x0c0, "0c0h" )
	PORT_DIPSETTING(     0x0c8, "0c8h" )
	PORT_DIPSETTING(     0x0d0, "0d0h" )
	PORT_DIPSETTING(     0x0d8, "0d8h" )
	PORT_DIPSETTING(     0x0e0, "0e0h" )
	PORT_DIPSETTING(     0x0e8, "0e8h" )
	PORT_DIPSETTING(     0x0f0, "0f0h" )
	PORT_DIPSETTING(     0x0f8, "0f8h" )
	PORT_DIPSETTING(     0x100, "0100h" )
	PORT_DIPSETTING(     0x108, "0108h" )
	PORT_DIPSETTING(     0x110, "0110h" )
	PORT_DIPSETTING(     0x118, "0118h" )
	PORT_DIPSETTING(     0x120, "0120h" )
	PORT_DIPSETTING(     0x128, "0128h" )
	PORT_DIPSETTING(     0x130, "0130h" )
	PORT_DIPSETTING(     0x138, "0138h" )
	PORT_DIPSETTING(     0x140, "0140h" )
	PORT_DIPSETTING(     0x148, "0148h" )
	PORT_DIPSETTING(     0x150, "0150h" )
	PORT_DIPSETTING(     0x158, "0158h" )
	PORT_DIPSETTING(     0x160, "0160h" )
	PORT_DIPSETTING(     0x168, "0168h" )
	PORT_DIPSETTING(     0x170, "0170h" )
	PORT_DIPSETTING(     0x178, "0178h" )
	PORT_DIPSETTING(     0x180, "0180h" )
	PORT_DIPSETTING(     0x188, "0188h" )
	PORT_DIPSETTING(     0x190, "0190h" )
	PORT_DIPSETTING(     0x198, "0198h" )
	PORT_DIPSETTING(     0x1a0, "01a0h" )
	PORT_DIPSETTING(     0x1a8, "01a8h" )
	PORT_DIPSETTING(     0x1b0, "01b0h" )
	PORT_DIPSETTING(     0x1b8, "01b8h" )
	PORT_DIPSETTING(     0x1c0, "01c0h" )
	PORT_DIPSETTING(     0x1c8, "01c8h" )
	PORT_DIPSETTING(     0x1d0, "01d0h" )
	PORT_DIPSETTING(     0x1d8, "01d8h" )
	PORT_DIPSETTING(     0x1e0, "01e0h" )
	PORT_DIPSETTING(     0x1e8, "01e8h" )
	PORT_DIPSETTING(     0x1f0, "01f0h" )
	PORT_DIPSETTING(     0x1f8, "01f8h" )
	PORT_DIPSETTING(     0x200, "0200h" )
	PORT_DIPSETTING(     0x208, "0208h" )
	PORT_DIPSETTING(     0x210, "0210h" )
	PORT_DIPSETTING(     0x218, "0218h" )
	PORT_DIPSETTING(     0x220, "0220h" )
	PORT_DIPSETTING(     0x228, "0228h" )
	PORT_DIPSETTING(     0x230, "0230h" )
	PORT_DIPSETTING(     0x238, "0238h" )
	PORT_DIPSETTING(     0x240, "0240h" )
	PORT_DIPSETTING(     0x248, "0248h" )
	PORT_DIPSETTING(     0x250, "0250h" )
	PORT_DIPSETTING(     0x258, "0258h" )
	PORT_DIPSETTING(     0x260, "0260h" )
	PORT_DIPSETTING(     0x268, "0268h" )
	PORT_DIPSETTING(     0x270, "0270h" )
	PORT_DIPSETTING(     0x278, "0278h" )
	PORT_DIPSETTING(     0x280, "0280h" )
	PORT_DIPSETTING(     0x288, "0288h" )
	PORT_DIPSETTING(     0x290, "0290h" )
	PORT_DIPSETTING(     0x298, "0298h" )
	PORT_DIPSETTING(     0x2a0, "02a0h" )
	PORT_DIPSETTING(     0x2a8, "02a8h" )
	PORT_DIPSETTING(     0x2b0, "02b0h" )
	PORT_DIPSETTING(     0x2b8, "02b8h" )
	PORT_DIPSETTING(     0x2c0, "02c0h" )
	PORT_DIPSETTING(     0x2c8, "02c8h" )
	PORT_DIPSETTING(     0x2d0, "02d0h" )
	PORT_DIPSETTING(     0x2d8, "02d8h" )
	PORT_DIPSETTING(     0x2e0, "02e0h" )
	PORT_DIPSETTING(     0x2e8, "02e8h" )
	PORT_DIPSETTING(     0x2f0, "02f0h" )
	PORT_DIPSETTING(     0x2f8, "02f8h" )
	PORT_DIPSETTING(     0x300, "0300h" )
	PORT_DIPSETTING(     0x308, "0308h" )
	PORT_DIPSETTING(     0x310, "0310h" )
	PORT_DIPSETTING(     0x318, "0318h" )
	PORT_DIPSETTING(     0x320, "0320h" )
	PORT_DIPSETTING(     0x328, "0328h" )
	PORT_DIPSETTING(     0x330, "0330h" )
	PORT_DIPSETTING(     0x338, "0338h" )
	PORT_DIPSETTING(     0x340, "0340h" )
	PORT_DIPSETTING(     0x348, "0348h" )
	PORT_DIPSETTING(     0x350, "0350h" )
	PORT_DIPSETTING(     0x358, "0358h" )
	PORT_DIPSETTING(     0x360, "0360h" )
	PORT_DIPSETTING(     0x368, "0368h" )
	PORT_DIPSETTING(     0x370, "0370h" )
	PORT_DIPSETTING(     0x378, "0378h" )
	PORT_DIPSETTING(     0x380, "0380h" )
	PORT_DIPSETTING(     0x388, "0388h" )
	PORT_DIPSETTING(     0x390, "0390h" )
	PORT_DIPSETTING(     0x398, "0398h" )
	PORT_DIPSETTING(     0x3a0, "03a0h" )
	PORT_DIPSETTING(     0x3a8, "03a8h" )
	PORT_DIPSETTING(     0x3b0, "03b0h" )
	PORT_DIPSETTING(     0x3b8, "03b8h" )
	PORT_DIPSETTING(     0x3c0, "03c0h" )
	PORT_DIPSETTING(     0x3c8, "03c8h" )
	PORT_DIPSETTING(     0x3d0, "03d0h" )
	PORT_DIPSETTING(     0x3d8, "03d8h" )
	PORT_DIPSETTING(     0x3e0, "03e0h" )
	PORT_DIPSETTING(     0x3e8, "03e8h" )
	PORT_DIPSETTING(     0x3f0, "03f0h" )
	PORT_DIPSETTING(     0x3f8, "03f8h" )

	PORT_START("IRQ_DRQ")
	PORT_DIPNAME( 0x07, 0x05, "SC499 IRQ (jumpers IRQ2-IRQ7)")
	PORT_DIPSETTING(    0x02, "IRQ 2" )
	PORT_DIPSETTING(    0x03, "IRQ 3" )
	PORT_DIPSETTING(    0x04, "IRQ 4" )
	PORT_DIPSETTING(    0x05, "IRQ 5" )
	PORT_DIPSETTING(    0x06, "IRQ 6" )
	PORT_DIPSETTING(    0x07, "IRQ 7" )

	PORT_DIPNAME( 0x30, 0x10, "SC499 DMA (jumpers DRQ1-DRQ3)")
	PORT_DIPSETTING(    0x10, "DRQ 1" )
	PORT_DIPSETTING(    0x20, "DRQ 2" )
	PORT_DIPSETTING(    0x30, "DRQ 3" )

INPUT_PORTS_END

MACHINE_CONFIG_FRAGMENT( sc499_ctape )
	MCFG_DEVICE_ADD(SC499_CTAPE_TAG, SC499_CTAPE, 0)
MACHINE_CONFIG_END

machine_config_constructor sc499_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sc499_ctape );
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISA8_SC499 = &device_creator<sc499_device>;

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
	: device_t(mconfig, SC499, "Archive SC-499", tag, owner, clock, "sc499", __FILE__),
	device_isa8_card_interface(mconfig, *this),
	m_iobase(*this, "IO_BASE"),
	m_irqdrq(*this, "IRQ_DRQ"), m_data(0), m_command(0), m_status(0), m_control(0), m_has_cartridge(0), m_is_writable(0), m_current_command(0), m_first_block_hack(0), m_nasty_readahead(0), m_read_block_pending(0),
	m_data_index(0), m_tape_status(0), m_data_error_counter(0), m_underrun_counter(0), m_tape_pos(0), m_ctape_block_count(0), m_ctape_block_index(0), m_image_length(0),
	m_image(*this, SC499_CTAPE_TAG), irq_state(), dma_drq_state(), m_timer(nullptr), m_timer1(nullptr), m_irq(0), m_drq(0), m_installed(false)
{
}

ioport_constructor sc499_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sc499_port );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sc499_device::device_start()
{
	set_isa_device();

	LOG1(("start sc499"));

	m_timer = timer_alloc(0, nullptr);
	m_timer1 = timer_alloc(1, nullptr);

	m_installed = false;

	if (m_image->image_core_file() == nullptr)
	{
		LOG2(("start sc499: no cartridge tape"));
	}
	else
	{
		LOG2(("start sc499: cartridge tape image is %s", m_image->filename()));
	}

	m_ctape_block_buffer.resize(SC499_CTAPE_BLOCK_SIZE);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sc499_device::device_reset()
{
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

	if (!m_installed)
	{
		int base = m_iobase->read();

		m_irq = m_irqdrq->read() & 7;
		m_drq = m_irqdrq->read()>>4;

		m_isa->install_device(base, base+7, 0, 0, read8_delegate(FUNC(sc499_device::read), this), write8_delegate(FUNC(sc499_device::write), this));
		m_isa->set_dma_channel(m_drq, this, true);

		m_installed = true;
	}
}

/*-------------------------------------------------
 cpu_context - return a string describing the current CPU context
 -------------------------------------------------*/

const char *sc499_device::cpu_context()
{
	static char statebuf[64]; /* string buffer containing state description */

	osd_ticks_t t = osd_ticks();
	int s = t / osd_ticks_per_second();
	int ms = (t % osd_ticks_per_second()) / 1000;

	sprintf(statebuf, "%d.%03d%s:", s, ms, tag());

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

		if (m_image_length != m_image->tapelen())
		{
			// tape has changed, get new size
			m_image_length = m_image->tapelen();
			m_ctape_block_count = (UINT32)((m_image_length + SC499_CTAPE_BLOCK_SIZE - 1) / SC499_CTAPE_BLOCK_SIZE);
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

void sc499_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG2(("timer_func param=%d status=%x", param, m_status));

	switch (param)
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
		case 2: m_data = m_tape_status & 0xff;   break;
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
				m_timer1->adjust(attotime::never, param, attotime::never);
				m_status &= ~SC499_STAT_EXC;
				m_status &= ~SC499_STAT_DIR;
				tape_status_clear(SC499_ST_CLEAR_ALL);
				tape_status_set(SC499_ST_READ_ERROR);

				LOG(("timer_func param=%d status=%x tape_pos=%d - read data underrun aborted at %d",
						param, m_status, m_tape_pos, m_underrun_counter));
			}
			else
			{
				LOG2(("timer_func param=%d status=%x tape_pos=%d - read data underrun %d",
						param, m_status, m_tape_pos, m_underrun_counter));
			}
			break;
		}
		else if ( m_tape_pos > m_ctape_block_count || !(m_status & SC499_STAT_RDY))
		{
			LOG1(("timer_func param=%d status=%x tape_pos=%d - end-of-tape or not ready",
								param, m_status, m_tape_pos));
			m_timer1->adjust(attotime::never, param, attotime::never);
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
						param, m_status, m_tape_pos, m_underrun_counter));
			}

			read_block();
			m_underrun_counter = 0;
			if (block_is_filemark())
			{
				m_timer1->adjust(attotime::never, param, attotime::never);
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
		LOG(("timer_func param=%d UNEXPECTED", param));
		m_timer->reset();
		break;
	}

}

/*-------------------------------------------------
 set_interrupt - set the IRQ state
 -------------------------------------------------*/

void sc499_device::set_interrupt(enum line_state state)
{
	if (state != irq_state)
	{
		LOG2(("set_interrupt(%d)",state));
		switch (m_irq)
		{
			case 2: m_isa->irq2_w(state); break;
			case 3: m_isa->irq3_w(state); break;
			case 4: m_isa->irq4_w(state); break;
			case 5: m_isa->irq5_w(state); break;
			case 6: m_isa->irq6_w(state); break;
			case 7: m_isa->irq7_w(state); break;
			default: logerror("sc499: invalid IRQ %d\n", m_irq); break;
		}
		irq_state = state;
	}
}

/*-------------------------------------------------
 set_dma_drq - set dma request output
 -------------------------------------------------*/

void sc499_device::set_dma_drq(enum line_state state)
{
	if (state != dma_drq_state)
	{
		LOG2(("set_dma_drq(%d)",state));

		switch (m_drq)
		{
			case 1: m_isa->drq1_w(state); break;
			case 2: m_isa->drq2_w(state); break;
			case 3: m_isa->drq3_w(state); break;
			default: logerror("sc499: invalid DRQ %d\n", m_drq); break;
		}

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

	m_first_block_hack = 1;
	m_nasty_readahead = 0;
	m_read_block_pending = 0;

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

WRITE8_MEMBER(sc499_device::write)
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

READ8_MEMBER(sc499_device::read)
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

void sc499_device::eop_w(int state)
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

UINT8 sc499_device::dack_r(int line)
{
	UINT8 data;

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

void sc499_device::dack_w(int line, UINT8 data)
{
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
	UINT8 *tape;

	if (m_tape_pos == 0)
	{
		// check if tape has been replaced or removed
		check_tape();
	}

	tape = m_image->read_block(m_tape_pos);

	if (tape == nullptr)
	{
		// either there is no tape or m_tape_pos goes beyond end-of-tape
		m_status &= ~SC499_STAT_EXC;
		m_status &= ~SC499_STAT_DIR;
		m_status &= ~SC499_STAT_DON;
		tape_status_clear(SC499_ST_CLEAR_ALL);
		tape_status_set(SC499_ST_READ_ERROR);
	}
	else
	{
		memcpy(&m_ctape_block_buffer[0], tape, SC499_CTAPE_BLOCK_SIZE);

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

	m_image->write_block(m_tape_pos, &m_ctape_block_buffer[0]);
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

	int is_filemark = memcmp(&m_ctape_block_buffer[0], fm_pattern, 4) == 0 &&
			memcmp(&m_ctape_block_buffer[0], &m_ctape_block_buffer[4], SC499_CTAPE_BLOCK_SIZE-4) == 0;

	LOG3(("block_is_filemark for block %d = %d", m_tape_pos-1, is_filemark));
	return is_filemark;
}

/*-------------------------------------------------
 block_set_filemark - set current block buffer data to filemark
 -------------------------------------------------*/

void sc499_device::block_set_filemark()
{
	static const UINT8 fm_pattern[] = {0xDE, 0xAF, 0xFA, 0xED};
	for (int i = 0; i < SC499_CTAPE_BLOCK_SIZE; i += 4)
	{
		memcpy(&m_ctape_block_buffer[i], fm_pattern, 4);
	}
}

//##########################################################################

const device_type SC499_CTAPE = &device_creator<sc499_ctape_image_device>;

sc499_ctape_image_device::sc499_ctape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SC499_CTAPE, "Cartridge Tape", tag, owner, clock, "sc499_ctape", __FILE__),
		device_image_interface(mconfig, *this)
{
}

void sc499_ctape_image_device::device_config_complete()
{
	update_names(SC499_CTAPE, "ctape", "ct");
}


UINT8 *sc499_ctape_image_device::read_block(int block_num)
{
	// access beyond end of tape cart
	if (m_ctape_data.size() <= (block_num + 1) * SC499_CTAPE_BLOCK_SIZE)
		return nullptr;
	else
		return &m_ctape_data[block_num * SC499_CTAPE_BLOCK_SIZE];
}

void sc499_ctape_image_device::write_block(int block_num, UINT8 *ptr)
{
	if (!(m_ctape_data.size() <= (block_num + 1) * SC499_CTAPE_BLOCK_SIZE))
		memcpy(&m_ctape_data[block_num * SC499_CTAPE_BLOCK_SIZE], ptr, SC499_CTAPE_BLOCK_SIZE);
}

bool sc499_ctape_image_device::call_load()
{
	UINT32 size;
	io_generic io;
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0xff;

	size = io_generic_size(&io);
	m_ctape_data.resize(size);

	io_generic_read(&io, &m_ctape_data[0], 0, size);

	return IMAGE_INIT_PASS;
}

void sc499_ctape_image_device::call_unload()
{
	m_ctape_data.resize(0);
	// TODO: add save tape on exit?
	//if (software_entry() == NULL)
	//{
	//    fseek(0, SEEK_SET);
	//    fwrite(m_ctape_data, m_ctape_data.size);
	//}
}
