// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*********************************************************************

    53c7xx.c

    NCR 53C700 SCSI I/O Processor


    TODO:
    * Low-level register accesses
    * Remove arbitrary delays
    * Add unimplemented SCRIPTS opcodes

*********************************************************************/

#include "emu.h"
#include "53c7xx.h"



//**************************************************************************
//  DEBUGGERY
//**************************************************************************

#define LOG_UNHANDLED       (1U << 1)
#define LOG_HOST            (1U << 2)
#define LOG_STATE           (1U << 3)
#define LOG_SCRIPTS         (1U << 4)
#define VERBOSE             (0)

#include "logmacro.h"


//**************************************************************************
//  REGISTER DEFINES (INCOMPLETE)
//**************************************************************************

#define SCNTL0_TRG          0x01
#define SCNTL0_AAP          0x02
#define SCNTL0_EPG          0x04
#define SCNTL0_EPC          0x08
#define SCNTL0_WATN         0x10
#define SCNTL0_START        0x20
#define SCNTL0_ARB_MASK     3
#define SCNTL0_ARB_SHIFT    6

#define SSTAT0_PAR          0x01
#define SSTAT0_RST          0x02
#define SSTAT0_UDC          0x04
#define SSTAT0_SGE          0x08
#define SSTAT0_SEL          0x10
#define SSTAT0_STO          0x20
#define SSTAT0_CMP          0x40
#define SSTAT0_MA           0x80

#define SSTAT1_SDP          0x01
#define SSTAT1_RST          0x02
#define SSTAT1_WOA          0x04
#define SSTAT1_LOA          0x08
#define SSTAT1_AIP          0x10
#define SSTAT1_ORF          0x20
#define SSTAT1_OLF          0x40
#define SSTAT1_ILF          0x80

#define ISTAT_DIP           0x01
#define ISTAT_SIP           0x02
#define ISTAT_PRE           0x04
#define ISTAT_CON           0x08
#define ISTAT_ABRT          0x80

#define DSTAT_OPC           0x01
#define DSTAT_WTD           0x02
#define DSTAT_SIR           0x04
#define DSTAT_SSI           0x08
#define DSTAT_ABRT          0x10
#define DSTAT_DFE           0x80


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(NCR53C7XX, ncr53c7xx_device, "ncr537xx", "NCR 53C7xx SCSI")

//-------------------------------------------------
//  ncr53c7xx_device - constructor/destructor
//-------------------------------------------------

ncr53c7xx_device::ncr53c7xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:   nscsi_device(mconfig, NCR53C7XX, tag, owner, clock),
		nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_irq_handler(*this),
		m_host_write(*this),
		m_host_read(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ncr53c7xx_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// resolve line callbacks
	m_irq_handler.resolve_safe();
	m_host_read.resolve_safe(0);
	m_host_write.resolve_safe();

	m_tm = timer_alloc(0);

	// The SCRIPTS processor runs at ~2 MIPS so approximate this
	set_unscaled_clock(2000000);

	// savestate support
	save_item(NAME(m_scntl));
	save_item(NAME(m_sdid));
	save_item(NAME(m_sien));
	save_item(NAME(m_scid));
	save_item(NAME(m_sxfer));
	save_item(NAME(m_sodl));
	save_item(NAME(m_socl));
	save_item(NAME(m_sfbr));
	save_item(NAME(m_sidl));
	save_item(NAME(m_sbdl));
	save_item(NAME(m_sbcl));
	save_item(NAME(m_dstat));
	save_item(NAME(m_sstat));
	save_item(NAME(m_ctest));
	save_item(NAME(m_temp));
	save_item(NAME(m_dfifo));
	save_item(NAME(m_istat));
	save_item(NAME(m_dbc));
	save_item(NAME(m_dcmd));
	save_item(NAME(m_dnad));
	save_item(NAME(m_dsp));
	save_item(NAME(m_dsps));
	save_item(NAME(m_dmode));
	save_item(NAME(m_dien));
	save_item(NAME(m_dwt));
	save_item(NAME(m_dcntl));

	// other state
	save_item(NAME(m_scsi_state));
	save_item(NAME(m_connected));
	save_item(NAME(m_finished));
	save_item(NAME(m_last_data));
	save_item(NAME(m_xfr_phase));

	save_item(NAME(m_scripts_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ncr53c7xx_device::device_reset()
{
	// Reset registers to defaults
	m_scntl[0]  = 3 << SCNTL0_ARB_SHIFT;
	m_scntl[1]  = 0;
	m_sdid      = 0;
	m_sien      = 0;
	m_scid      = 0;
	m_sxfer     = 0;
	m_sodl      = 0;
	m_socl      = 0;
	m_sfbr      = 0;
	m_sidl      = 0;
	m_sbdl      = 0;
	m_sbcl      = 0;
	m_dstat     = DSTAT_DFE;
	m_sstat[0]  = 0;
	m_sstat[1]  = 0;
	m_sstat[2]  = 0;
	m_ctest[0]  = 0;
	m_ctest[1]  = 0xf0;
	m_ctest[2]  = 0x21;
	m_ctest[3]  = 0;
	m_ctest[4]  = 0;
	m_ctest[5]  = 0;
	m_ctest[6]  = 0;
	m_ctest[7]  = 0;
	m_dfifo     = 0;
	m_istat     = 0;//ISTAT_PRE;
	m_dmode     = 0;
	m_dien      = 0;
	m_dcntl     = 0;

	m_finished = false;
	m_connected = false;

	scsi_bus->ctrl_wait(scsi_refid, S_SEL | S_BSY | S_RST, S_ALL);
	set_scripts_state(SCRIPTS_IDLE);
	set_scsi_state(IDLE);

	m_irq_handler(CLEAR_LINE);
}


//**************************************************************************
//  MEMORY HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read - Host read handler
//-------------------------------------------------

uint32_t ncr53c7xx_device::read(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_HOST, "%s: REG R: [%x] (%08X)\n", machine().describe_context(), offset, mem_mask);

	uint32_t ret = 0;

	switch (offset)
	{
		case 0x0:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_scntl[0];
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_scntl[1] << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sdid << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_sien << 24;
			}

			break;
		}

		case 0x1:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_scid;
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_sxfer << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sodl << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_socl << 24;
			}

			break;
		}

		case 0x2:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_sfbr;
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_sidl << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sbdl << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_sbcl << 24;
			}

			break;
		}

		case 0x3:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_dstat;
				m_dstat = 0;
				update_irqs();
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_sstat[0] << 8;
				m_sstat[0] = 0;
				update_irqs();
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_sstat[1] << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_sstat[2] << 24;
			}

			break;
		}

		case 0x5:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_ctest[0];
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_ctest[1] << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_ctest[2] << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_ctest[3] << 24;
			}

			break;
		}

		case 0x6:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_ctest[4];
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_ctest[5] << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_ctest[6] << 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_ctest[7] << 24;
			}

			break;
		}

		case 0x7:
		{
			ret = m_temp;

			break;
		}

		case 0x8:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_dfifo;
			}
			if (ACCESSING_BITS_8_15)
			{
				ret |= m_istat << 8;
			}

			break;
		}

		case 0x9:
		{
			if (ACCESSING_BITS_0_7 || ACCESSING_BITS_8_15 || ACCESSING_BITS_16_23)
			{
				ret = m_dbc;
			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_dcmd << 24;
			}

			break;
		}

		case 0xa:
		{
			ret = m_dnad;

			break;
		}

		case 0xb:
		{
			ret = m_dsp;

			break;
		}

		case 0xc:
		{
			ret = m_dsps;

			break;
		}

		case 0xd:
		{
			if (ACCESSING_BITS_0_7)
			{
				ret = m_dmode;
			}

			break;
		}

		case 0xe:
		{
			if (ACCESSING_BITS_8_15)
			{
				ret = m_dien << 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				ret |= m_dwt << 16;

			}
			if (ACCESSING_BITS_24_31)
			{
				ret |= m_dcntl << 24;
			}

			break;
		}

		default:
		{
			LOGMASKED(LOG_UNHANDLED, "%s: Unhandled register access", machine().describe_context());
		}
	}

	return ret;
}


//-------------------------------------------------
//  write - Host write handler
//-------------------------------------------------

void ncr53c7xx_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_HOST, "%s: REG W: [%x] (%08X) %x\n", offset, mem_mask, data, machine().describe_context());

	switch (offset)
	{
		case 0x0:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_scntl[0] = data;

				if (data & SCNTL0_TRG)
					fatalerror("53c7xx: Target mode unsupported!");

				if (data & SCNTL0_START)
				{
					// Start arbitration
					set_scsi_state(ARBITRATE_WAIT_FREE);
					step(true);
				}
			}
			if (ACCESSING_BITS_8_15)
			{
				m_scntl[1] = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_sdid = data >> 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_sien = data >> 24;
			}

			break;
		}

		case 0x1:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_scid = data;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_sxfer = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_sodl = data >> 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_socl = data >> 24;
			}

			break;
		}

		case 0x6:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_ctest[4] = data;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_ctest[5] = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_ctest[6] = data >> 16;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_ctest[7] = data >> 24;
			}

			break;
		}

		case 0x7:
		{
			m_temp = data;

			break;
		}

		case 0x8:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_dfifo = data;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_istat = data >> 8;
			}

			break;
		}

		case 0x9:
		{
			if (ACCESSING_BITS_0_7 || ACCESSING_BITS_8_15 || ACCESSING_BITS_16_23)
			{
				m_dbc = data & 0xffffff;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_dcmd = data >> 24;
			}

			break;
		}

		case 0xa:
		{
			m_dnad = data;

			break;
		}

		case 0xb:
		{
			// Write to the upper byte starts the fetch
			m_dsp = data;

			if (m_dmode & 1)
			{
				set_scripts_state(SCRIPTS_WAIT_MANUAL_START);
			}
			else
			{
				set_scripts_state(SCRIPTS_FETCH);
			}

			break;
		}

		case 0xc:
		{
			m_dsps = data;

			break;
		}

		case 0xd:
		{
			if (ACCESSING_BITS_0_7)
			{
				m_dmode = data;
			}

			break;
		}

		case 0xe:
		{
			if (ACCESSING_BITS_8_15)
			{
				m_dien = data >> 8;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_dwt = data >> 16;

				if (m_dwt)
					fatalerror("53c7xx: DMA Watchdog Timer enabled!");
			}
			if (ACCESSING_BITS_24_31)
			{
				m_dcntl = data >> 24;

				// Note: not self-clearing
				if (m_dcntl & 1) // RST
				{
					device_reset();
				}
				else if (m_dcntl & 2) // STD
				{
					// Only applies to these modes:
					// * Manual Start
					// * Single Step
					// * Pipeline
					fatalerror("53c7xx: Start DMA");
				}
				else if (m_dcntl & 4)
				{
					fatalerror("53c7xx: SCSI Low-Level Mode not supported!");
				}

				// TODO: Update clocking
			}

			break;
		}

		default:
		{
			LOGMASKED(LOG_UNHANDLED, "%s: Unhandled register access", machine().describe_context());
		}
	}
}



//**************************************************************************
//  SCSI STATE MACHINE
//**************************************************************************

//-------------------------------------------------
//  update_irqs -
//-------------------------------------------------
void ncr53c7xx_device::update_irqs()
{
	if (m_sstat[0] & m_sien)
		m_istat |= ISTAT_SIP;
	else
		m_istat &= ~ISTAT_SIP;

	if (m_dstat & m_dien)
		m_istat |= ISTAT_DIP;
	else
		m_istat &= ~ISTAT_DIP;

	m_irq_handler(m_istat ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  set_scsi_state - change SCSI state
//-------------------------------------------------

void ncr53c7xx_device::set_scsi_state(int state)
{
	LOGMASKED(LOG_STATE, "SCSI state change: %x to %x\n", m_scsi_state, state);

	m_scsi_state = state;
}


//-------------------------------------------------
//  delay - step the SCSI state machine following
//  a time delay
//-------------------------------------------------

void ncr53c7xx_device::delay(const attotime &delay)
{
	m_tm->adjust(delay);
}


//-------------------------------------------------
//  scsi_ctrl_changed - callback from nscsi_device
//-------------------------------------------------

void ncr53c7xx_device::scsi_ctrl_changed()
{
	step(false);
}


//-------------------------------------------------
//  send_byte - send data to a SCSI device
//-------------------------------------------------

void ncr53c7xx_device::send_byte()
{
	if (m_dbc == 0)
		fatalerror("53C7XX: send_byte() - DBC should not be 0\n");

	set_scsi_state( (m_scsi_state & STATE_MASK) | (SEND_WAIT_SETTLE << SUB_SHIFT) );

	uint32_t data = m_host_read(m_dnad & ~3, 0xffffffff);
	data = data >> ((m_dnad & 3) * 8) & 0xff;

	++m_dnad;
	--m_dbc;

	scsi_bus->data_w(scsi_refid, data);
	scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	delay(attotime::from_nsec(5));
}


//-------------------------------------------------
//  recv_byte - receive data from a SCSI device
//-------------------------------------------------

void ncr53c7xx_device::recv_byte()
{
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	set_scsi_state( (m_scsi_state & STATE_MASK) | (RECV_WAIT_REQ_1 << SUB_SHIFT) );
	step(false);
}


//-------------------------------------------------
//  device_timer - callback to step the SCSI
//  state machine
//-------------------------------------------------

void ncr53c7xx_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	step(true);
}


//-------------------------------------------------
//  step - advance the SCSI state machine
//-------------------------------------------------

void ncr53c7xx_device::step(bool timeout)
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	uint32_t data = scsi_bus->data_r();

	LOGMASKED(LOG_STATE, "Step: CTRL:%x DATA:%x (%d.%d) Timeout:%d\n", ctrl, data, m_scsi_state & STATE_MASK, m_scsi_state >> SUB_SHIFT, timeout);

	// Check for disconnect from target
	if (!(m_scntl[0] & SCNTL0_TRG) && m_connected && !(ctrl & S_BSY))
	{
		set_scsi_state(FINISHED);
		m_connected = false;
//      istatus |= I_DISCONNECT;
//      reset_disconnect();
//      check_irq();
	}

	switch (m_scsi_state & SUB_MASK ? m_scsi_state & SUB_MASK : m_scsi_state & STATE_MASK)
	{
		case IDLE:
		{
			break;
		}

		case FINISHED:
		{
			m_finished = true;
			set_scsi_state(IDLE);
			step(true);

			break;
		}

		case ARBITRATE_WAIT_FREE:
		{
			if (!timeout)
				break;

			// Is the bus free?
			if (ctrl & (S_BSY | S_SEL))
			{
				// Keep trying until it is
				delay(attotime::from_nsec(800));
			}
			else
			{
				// Bus is free; next phase
				delay(attotime::from_nsec(800));
				set_scsi_state(ARBITRATE_CHECK_FREE);
			}

			break;
		}

		case ARBITRATE_CHECK_FREE:
		{
			if ((ctrl & (S_BSY | S_SEL)) == 0)
			{
				// Bus is free - assert the controller SCSI ID and BUSY
				scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);

				if (((m_scntl[0] >> SCNTL0_ARB_SHIFT) & SCNTL0_ARB_MASK) == 3)
				{
					// Full arbitration
					scsi_bus->data_w(scsi_refid, m_scid);
				}
				else
				{
					// Simple arbitration
					scsi_bus->data_w(scsi_refid, m_sodl);
				}

				set_scsi_state(ARBITRATE_EXAMINE_BUS);
				delay(attotime::from_nsec(2400));
			}

			break;
		}

		case ARBITRATE_EXAMINE_BUS:
		{
			if (!timeout)
				break;

			if (ctrl & S_SEL)
			{
				scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
				scsi_bus->data_w(scsi_refid, 0);

				if (((m_scntl[0] >> SCNTL0_ARB_SHIFT) & SCNTL0_ARB_MASK) == 3)
				{
					// Try again
					set_scsi_state(ARBITRATE_WAIT_FREE);
					delay(attotime::from_nsec(2400));
				}
				else
				{
					// TODO: Is this right?
					m_sstat[1] |= SSTAT1_LOA;
					m_scntl[0] &= ~SCNTL0_START;
					m_sstat[0] |= SSTAT0_CMP;
					update_irqs();

					set_scsi_state(FINISHED);
					step(true);
				}

				break;
			}

			// Full arbitration?
			if (((m_scntl[0] >> SCNTL0_ARB_SHIFT) & SCNTL0_ARB_MASK) == 3)
			{
				int win;
				for (win = 7; win >=0 && !(data & (1 << win)); win--) {};

				if ((1 << win) != m_scid)
				{
					scsi_bus->data_w(scsi_refid, 0);
					scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);

					delay(attotime::from_nsec(2400));
					break;
				}

				// Begin the select phase; assert SEL
				m_sstat[1] |= SSTAT1_WOA;
				scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
				set_scsi_state(ARBITRATE_ASSERT_SEL);
				delay(attotime::from_nsec(1200));
			}
			else
			{
				// TODO: Worth adding another state here?
				m_sstat[0] |= SSTAT0_CMP;
				m_scntl[0] &= ~SCNTL0_START;
				update_irqs();
				set_scsi_state(FINISHED);
				step(true);
			}

			break;
		}

		case ARBITRATE_ASSERT_SEL:
		{
			if (!timeout)
				break;

			// Activate data line of the thing
			scsi_bus->data_w(scsi_refid, m_sdid);

			set_scsi_state(ARBITRATE_SELECT_DEST);
			delay(attotime::from_nsec(2));

			break;
		}

		case ARBITRATE_SELECT_DEST:
		{
			if (!timeout)
				break;

			scsi_bus->ctrl_w(scsi_refid, m_scntl[0] & SCNTL0_WATN ? S_ATN : 0, S_ATN | S_BSY);

			set_scsi_state(ARBITRATE_RELEASE_BSY);
			delay(attotime::from_nsec(20));

			break;
		}

		case ARBITRATE_RELEASE_BSY:
		{
			if (!timeout)
				break;

			set_scsi_state(ARBITRATE_DESKEW_WAIT);
			delay(attotime::from_nsec(500));

			break;
		}

		case ARBITRATE_DESKEW_WAIT:
		{
			if (!timeout)
				break;

			// Clear everything
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);

			// Done?
			m_sstat[0] |= SSTAT0_CMP;
			m_scntl[0] &= ~SCNTL0_START;
			//update_irqs();
			set_scsi_state(FINISHED);
			m_connected = true;

			step(true);
			break;
		}


		// Note this is actually block transfers
		case INIT_XFER:
		{
			if (ctrl & S_INP)
			{
				set_scsi_state(m_dbc ? INIT_XFER_RECV_BYTE_ACK : INIT_XFER_RECV_BYTE_NACK);
				recv_byte();
			}
			else
			{
				if (m_dbc == 1)
					scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);

				set_scsi_state(INIT_XFER_SEND_BYTE);
				send_byte();
			}

			break;
		}

		case INIT_XFER_SEND_BYTE:
		{
			if (m_dbc == 0)
			{
				set_scsi_state(FINISHED);
				step(true);
			}
			else
			{
				set_scsi_state(INIT_XFER_WAIT_REQ);
			}

			break;
		}

		case INIT_XFER_RECV_BYTE_ACK:
		{
			set_scsi_state(INIT_XFER_WAIT_REQ);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);

			break;
		}

		case INIT_XFER_RECV_BYTE_NACK:
		{
			set_scsi_state(FINISHED);
			step(true);

			break;
		}

		case INIT_XFER_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
				break;

			if ((ctrl & S_PHASE_MASK) != m_xfr_phase)
			{
				set_scsi_state(FINISHED);
				step(true);
			}
			else
			{
				set_scsi_state(INIT_XFER);
				step(false);
			}

			break;
		}

		case SEND_WAIT_SETTLE << SUB_SHIFT:
		{
			if (!timeout)
				break;

			set_scsi_state( (m_scsi_state & STATE_MASK) | (SEND_WAIT_REQ_0 << SUB_SHIFT) );
			step(false);

			break;
		}

		case SEND_WAIT_REQ_0 << SUB_SHIFT:
		{
			if (ctrl & S_REQ)
				break;

			set_scsi_state(m_scsi_state & STATE_MASK);
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);

			step(false);

			break;
		}

		case RECV_WAIT_REQ_1 << SUB_SHIFT:
		{
			if (!(ctrl & S_REQ))
				break;

			set_scsi_state( (m_scsi_state & STATE_MASK) | (RECV_WAIT_SETTLE << SUB_SHIFT) );
			delay(attotime::from_nsec(5));

			break;
		}

		case RECV_WAIT_SETTLE << SUB_SHIFT:
		{
			if (!timeout)
				break;

			if ((m_scsi_state & STATE_MASK) != INIT_XFER_RECV_PAD)
			{
				m_last_data = scsi_bus->data_r();

				uint32_t shift = (8 * (m_dnad & 3));
				uint32_t mem_mask = 0xff << shift;
				m_host_write(m_dnad & ~3, data << shift, mem_mask);

				++m_dnad;
				--m_dbc;
			}

			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			set_scsi_state( (m_scsi_state & STATE_MASK) | (RECV_WAIT_REQ_0 << SUB_SHIFT) );
			step(false);

			break;
		}

		case RECV_WAIT_REQ_0 << SUB_SHIFT:
		{
			if (ctrl & S_REQ)
				break;

			set_scsi_state(m_scsi_state & STATE_MASK);
			step(false);

			break;
		}

		default:
			fatalerror("Unknown state! (%x)\n", m_scsi_state);

	}
}


//**************************************************************************
//  SCSI SCRIPTS
//**************************************************************************

#define     UNIMPLEMENTED   fatalerror("%s is unimplemented\n", __FUNCTION__)

//-------------------------------------------------
//  set_scripts_state -
//-------------------------------------------------

void ncr53c7xx_device::set_scripts_state(scripts_state state)
{
	m_scripts_state = state;
}


//-------------------------------------------------
//  scripts_yield - suspend execution
//-------------------------------------------------
void ncr53c7xx_device::scripts_yield()
{
	m_icount = 0;
}


//-------------------------------------------------
//  execute_run - SCRIPTS execution loop
//-------------------------------------------------

void ncr53c7xx_device::execute_run()
{
	// Not processing anything so bail
	if (m_scripts_state < SCRIPTS_FETCH)
	{
		m_icount = 0;
		return;
	}

	do
	{
		switch (m_scripts_state)
		{
			case SCRIPTS_FETCH:
			{
				m_finished = false;

				// Fetch the instruction
				uint32_t inst = m_host_read(m_dsp, 0xffffffff);

				m_dcmd = inst >> 24;
				m_dbc = inst & 0xffffff;

				// Unless we encounter an illegal instruction...
				set_scripts_state(SCRIPTS_EXECUTE);

				// Decode the relevant group
				switch ((m_dcmd >> 6) & 3)
				{
					case 0:
						scripts_decode_bm();
						break;

					case 1:
						scripts_decode_io();
						break;

					case 2:
						scripts_decode_tc();
						break;

					case 3:
						illegal();
				}

				LOGMASKED(LOG_SCRIPTS, "%s", disassemble_scripts());
				break;
			}

			case SCRIPTS_EXECUTE:
			{
				(*this.*m_scripts_op)();
				break;
			}
		}

		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  scripts_decode_bm - decode block move
//-------------------------------------------------

void ncr53c7xx_device::scripts_decode_bm(void)
{
	// Decode our instruction
	if (m_scntl[0] & SCNTL0_TRG)
	{
		// Target mode
		switch ((m_dcmd >> 3) & 3)
		{
			case 0:
				m_scripts_op = &ncr53c7xx_device::bm_t_move;
				break;

			default:
				illegal();
				return;
		}
	}
	else
	{
		// Initiator mode
		switch ((m_dcmd >> 3) & 3)
		{
			case 0:
				m_scripts_op = &ncr53c7xx_device::bm_i_move;
				break;

			case 1:
				m_scripts_op = &ncr53c7xx_device::bm_i_wmov;
				break;

			default:
				illegal();
				return;
		}
	}

	m_dnad = m_host_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;
}


//-------------------------------------------------
//  scripts_decode_io - decode IO
//-------------------------------------------------

void ncr53c7xx_device::scripts_decode_io(void)
{
	// Set Target Mode?
	if (m_dbc & (1 << 9))
		m_scntl[0] |= SCNTL0_TRG;

	// Decode our instruction
	if (m_scntl[0] & SCNTL0_TRG)
	{
		// Initiator mode
		switch ((m_dcmd >> 3) & 7)
		{
			case 0:
				m_scripts_op = &ncr53c7xx_device::io_t_reselect;
				break;

			case 1:
				m_scripts_op = &ncr53c7xx_device::io_t_disconnect;
				break;

			case 2:
				m_scripts_op = &ncr53c7xx_device::io_t_waitselect;
				break;

			case 3:
				m_scripts_op = &ncr53c7xx_device::io_t_set;
				break;

			case 4:
				m_scripts_op = &ncr53c7xx_device::io_t_clear;
				break;

			default:
				illegal();
				return;
		}
	}
	else
	{
		// Initiator mode
		switch ((m_dcmd >> 3) & 7)
		{
			case 0:
				m_scripts_op = &ncr53c7xx_device::io_i_select;
				break;

			case 1:
				m_scripts_op = &ncr53c7xx_device::io_i_waitdisconnect;
				break;

			case 2:
				m_scripts_op = &ncr53c7xx_device::io_i_waitreselect;
				break;

			case 3:
				m_scripts_op = &ncr53c7xx_device::io_i_set;
				break;

			case 4:
				m_scripts_op = &ncr53c7xx_device::io_i_clear;
				break;

			default:
				illegal();
				return;
		}
	}

	// Set some additional registers
	m_dnad = m_dsps = m_host_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;
}


//-------------------------------------------------
//  scripts_decode_tc - decode transfer control
//-------------------------------------------------

void ncr53c7xx_device::scripts_decode_tc(void)
{
	// Decode our instruction
	switch ((m_dcmd >> 3) & 7)
	{
		case 0:
			m_scripts_op = &ncr53c7xx_device::tc_jump;
			break;

		case 1:
			m_scripts_op = &ncr53c7xx_device::tc_call;
			break;

		case 2:
			m_scripts_op = &ncr53c7xx_device::tc_return;
			break;

		case 3:
			m_scripts_op = &ncr53c7xx_device::tc_int;
			break;

		default:
			illegal();
			break;
	}

	m_dnad = m_dsps = m_host_read(m_dsp + 4, 0xffffffff);
	m_dsp += 8;
}

//**************************************************************************
//  SCSI SCRIPTS INSTRUCTIONS
//**************************************************************************

//-------------------------------------------------
//  illegal - illegal instruction
//-------------------------------------------------

void ncr53c7xx_device::illegal()
{
	m_dstat |= DSTAT_OPC;
	update_irqs();
	set_scripts_state(SCRIPTS_IDLE);

}


//-------------------------------------------------
//  bm_t_move - block move (target)
//-------------------------------------------------

void ncr53c7xx_device::bm_t_move()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  bm_i_move - block move (initiator)
//-------------------------------------------------

void ncr53c7xx_device::bm_i_move()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  bm_i_wmov - wait block move (initiator)
//-------------------------------------------------

void ncr53c7xx_device::bm_i_wmov()
{
	if (!m_finished)
	{
		if (m_scsi_state == IDLE)
		{
			if (m_dbc == 0)
			{
				LOGMASKED(LOG_UNHANDLED, "DBC should not be 0\n");
				illegal();
			}

			// Indirect addressing
			if (m_dcmd & (1 << 5))
				m_dnad = m_host_read(m_dnad, 0xffffffff);

			// Compare the phase bits
			if ((scsi_bus->ctrl_r() & 7) == (m_dcmd & 7))
			{
				// Transfer bytes
				set_scsi_state(INIT_XFER);
				m_xfr_phase = m_dcmd & 7;
				step(false);
			}
			else
			{
				fatalerror("Phase mismatch\n");
			}
		}
		else
		{
			scripts_yield();
		}
	}
	else
	{
		// TODO: We should see what happened here; different behaviour
		// depending on whether or not we won arbitration
		set_scripts_state(SCRIPTS_FETCH);
	}
}


//-------------------------------------------------
//  io_t_reselect -
//-------------------------------------------------

void ncr53c7xx_device::io_t_reselect()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  io_t_disconnect -
//-------------------------------------------------

void ncr53c7xx_device::io_t_disconnect()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  io_t_waitselect -
//-------------------------------------------------

void ncr53c7xx_device::io_t_waitselect()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  io_t_set -
//-------------------------------------------------

void ncr53c7xx_device::io_t_set()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  io_t_clear -
//-------------------------------------------------

void ncr53c7xx_device::io_t_clear()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  io_i_select -
//-------------------------------------------------

void ncr53c7xx_device::io_i_select()
{
	if (!m_finished)
	{
		if (m_scsi_state == IDLE)
		{
			m_sdid = m_dbc >> 16;
			m_scntl[0] |= (3 << SCNTL0_ARB_SHIFT) | SCNTL0_START;

			// Set select with ATN bit
			if (m_dcmd & 1)
				m_scntl[0] |= SCNTL0_WATN;

			// Start the arbitration
			set_scsi_state(ARBITRATE_WAIT_FREE);
			step(true);
		}

		scripts_yield();
	}
	else
	{
		// TODO: We should see what happened here; different behaviour
		// depending on whether or not we won arbitration
		set_scripts_state(SCRIPTS_FETCH);
	}
}


//-------------------------------------------------
//  io_i_waitdisconnect -
//-------------------------------------------------

void ncr53c7xx_device::io_i_waitdisconnect()
{
	if (scsi_bus->ctrl_r() & (S_BSY | S_SEL))
		scripts_yield();
	else
		set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  io_i_waitreselect -
//-------------------------------------------------

void ncr53c7xx_device::io_i_waitreselect()
{
	UNIMPLEMENTED;
}


//-------------------------------------------------
//  io_i_set -
//-------------------------------------------------

void ncr53c7xx_device::io_i_set()
{
	uint32_t mask = 0;

	if (m_dbc & (1 << 3))
		mask |= S_ATN;

	if (m_dbc & (1 << 6))
		mask |= S_ACK;

	scsi_bus->ctrl_w(scsi_refid, mask, mask);

	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  io_i_clear -
//-------------------------------------------------

void ncr53c7xx_device::io_i_clear()
{
	uint32_t mask = 0;

	if (m_dbc & (1 << 3))
		mask |= S_ATN;

	if (m_dbc & (1 << 6))
		mask |= S_ACK;

	scsi_bus->ctrl_w(scsi_refid, 0, mask);

	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_jump -
//-------------------------------------------------

void ncr53c7xx_device::tc_jump()
{
//  if (m_dbc & (1 << 16))
//      printf("Must wait for valid phase?\n");

	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= (m_dbc & 0xff) == m_last_data;
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_dsp = m_dsps;
	}
	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_call -
//-------------------------------------------------

void ncr53c7xx_device::tc_call()
{
	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= (m_dbc & 0xff) == m_last_data;
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_temp = m_dsp;
		m_dsp = m_dsps;
	}
	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_return -
//-------------------------------------------------

void ncr53c7xx_device::tc_return()
{
	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= (m_dbc & 0xff) == m_last_data;
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_dsp = m_temp;
	}
	set_scripts_state(SCRIPTS_FETCH);
}


//-------------------------------------------------
//  tc_int -
//-------------------------------------------------

void ncr53c7xx_device::tc_int()
{
	bool jump = true;

	if (m_dbc & (1 << 17))
	{
		// Phase
		jump &= (m_dcmd & 7) == (scsi_bus->ctrl_r() & 7);
	}
	if (m_dbc & (1 << 18))
	{
		// Data
		jump &= (m_dbc & 0xff) == m_last_data;
	}

	if (!(m_dbc & (1 << 19)))
		jump = !jump;

	if (jump)
	{
		m_dstat |= DSTAT_SIR;
		update_irqs();
		set_scripts_state(SCRIPTS_IDLE);
	}
	else
	{
		set_scripts_state(SCRIPTS_FETCH);
	}
}


//**************************************************************************
//  SCSI SCRIPTS DISASSEMBLY
//**************************************************************************

//-------------------------------------------------
//  disassemble_scripts -
//-------------------------------------------------

std::string ncr53c7xx_device::disassemble_scripts()
{
	static char const *const phases[] =
	{
		"Data Out",
		"Data In",
		"Command",
		"Status",
		"Reserved",
		"Reserved",
		"Message Out",
		"Message In"
	};

	std::string opstring;

	switch ((m_dcmd >> 6) & 3)
	{
		case 0:
		{
			opstring = util::string_format("BMOV: %s [%x] %d bytes\n", phases[m_dcmd & 7], m_dnad, m_dbc);
			break;
		}
		case 1:
		{
			static char const *const ops[] =
			{
				"SELECT",
				"DISCONNECT",
				"RESELECT",
				"SET",
				"CLEAR",
				"ILLEGAL",
				"ILLEGAL",
				"ILLEGAL",
			};

			opstring = util::string_format("IO: %s (%x)\n", ops[(m_dcmd >> 3) & 7], m_dnad);
			break;
		}
		case 2:
		{
			static char const *const ops[] =
			{
				"JUMP",
				"CALL",
				"RETURN",
				"INT",
				"ILLEGAL",
				"ILLEGAL",
				"ILLEGAL",
				"ILLEGAL",
			};

			opstring = util::string_format("TC: %s %c (%s) (%x)\n", ops[(m_dcmd >> 3) & 7], m_dbc & (1 << 19) ? 'T' : 'F', phases[m_dcmd & 7], m_dnad);
			break;
		}
		case 3:
		{
			opstring = "ILLEGAL";
			break;
		}
	}

	return util::string_format("SCRIPTS [%08x]: %s", m_dsp - 8, opstring);
}
