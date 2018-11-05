// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont, Ryan Holtz
/*
 * wd33c93.c
 *
 * WD/AMD 33c93 SCSI controller, as seen in
 * early PCs, some MSX add-ons, NEC PC-88, and SGI
 * Indigo, Indigo2, and Indy systems.
 *
 * References:
 * WD 33c93 manual
 * NetBSD 33c93 driver
 *
 */

#include "emu.h"
#include "wd33c93.h"

#define LOG_READS		(1 << 0)
#define LOG_WRITES		(1 << 1)
#define LOG_COMMANDS	(1 << 2)
#define LOG_ERRORS		(1 << 3)
#define LOG_MISC		(1 << 4)

#define VERBOSE			(0)
#include "logmacro.h"


/* WD commands */
#define WD_CMD_RESET                0x00
#define WD_CMD_ABORT                0x01
#define WD_CMD_ASSERT_ATN           0x02
#define WD_CMD_NEGATE_ACK           0x03
#define WD_CMD_DISCONNECT           0x04
#define WD_CMD_RESELECT             0x05
#define WD_CMD_SEL_ATN              0x06
#define WD_CMD_SEL                  0x07
#define WD_CMD_SEL_ATN_XFER         0x08
#define WD_CMD_SEL_XFER             0x09
#define WD_CMD_RESEL_RECEIVE        0x0a
#define WD_CMD_RESEL_SEND           0x0b
#define WD_CMD_WAIT_SEL_RECEIVE     0x0c
#define WD_CMD_SSCC                 0x0d
#define WD_CMD_SND_DISC             0x0e
#define WD_CMD_SET_IDI              0x0f
#define WD_CMD_RCV_CMD              0x10
#define WD_CMD_RCV_DATA             0x11
#define WD_CMD_RCV_MSG_OUT          0x12
#define WD_CMD_RCV                  0x13
#define WD_CMD_SND_STATUS           0x14
#define WD_CMD_SND_DATA             0x15
#define WD_CMD_SND_MSG_IN           0x16
#define WD_CMD_SND                  0x17
#define WD_CMD_TRANS_ADDR           0x18
#define WD_CMD_XFER_PAD             0x19
#define WD_CMD_TRANS_INFO           0x20
#define WD_CMD_TRANSFER_PAD         0x21
#define WD_CMD_SBT_MODE             0x80

/* ASR register */
#define ASR_INT                     0x80
#define ASR_LCI                     0x40
#define ASR_BSY                     0x20
#define ASR_CIP                     0x10
#define ASR_PE                      0x02
#define ASR_DBR                     0x01

/* SCSI Bus Phases */
#define PHS_DATA_OUT                0x00
#define PHS_DATA_IN                 0x01
#define PHS_COMMAND                 0x02
#define PHS_STATUS                  0x03
#define PHS_MESS_OUT                0x06
#define PHS_MESS_IN                 0x07

/* Command Status Register definitions */

	/* reset state interrupts */
#define CSR_RESET                   0x00
#define CSR_RESET_AF                0x01

	/* successful completion interrupts */
#define CSR_RESELECT                0x10
#define CSR_SELECT                  0x11
#define CSR_SEL_XFER_DONE           0x16
#define CSR_XFER_DONE               0x18

	/* paused or aborted interrupts */
#define CSR_MSGIN                   0x20
#define CSR_SDP                     0x21
#define CSR_SEL_ABORT               0x22
#define CSR_RESEL_ABORT             0x25
#define CSR_RESEL_ABORT_AM          0x27
#define CSR_ABORT                   0x28

	/* terminated interrupts */
#define CSR_INVALID                 0x40
#define CSR_UNEXP_DISC              0x41
#define CSR_TIMEOUT                 0x42
#define CSR_PARITY                  0x43
#define CSR_PARITY_ATN              0x44
#define CSR_BAD_STATUS              0x45
#define CSR_UNEXP                   0x48

	/* service required interrupts */
#define CSR_RESEL                   0x80
#define CSR_RESEL_AM                0x81
#define CSR_DISC                    0x85
#define CSR_SRV_REQ                 0x88

	/* Own ID/CDB Size register */
#define OWNID_EAF                   0x08
#define OWNID_EHP                   0x10
#define OWNID_RAF                   0x20
#define OWNID_FS_8                  0x00
#define OWNID_FS_12                 0x40
#define OWNID_FS_16                 0x80

	/* Control register */
#define CTRL_HSP                    0x01
#define CTRL_HA                     0x02
#define CTRL_IDI                    0x04
#define CTRL_EDI                    0x08
#define CTRL_HHP                    0x10
#define CTRL_POLLED                 0x00
#define CTRL_BURST                  0x20
#define CTRL_BUS                    0x40
#define CTRL_DMA                    0x80

	/* Synchronous Transfer Register */
#define STR_FSS                     0x80

	/* Destination ID register */
#define DSTID_DPD                   0x40
#define DATA_OUT_DIR                0
#define DATA_IN_DIR                 1
#define DSTID_SCC                   0x80

	/* Source ID register */
#define SRCID_MASK                  0x07
#define SRCID_SIV                   0x08
#define SRCID_DSP                   0x20
#define SRCID_ES                    0x40
#define SRCID_ER                    0x80

/* convernience functions */
uint8_t wd33c93_device::getunit()
{
	/* return the destination unit id */
	return m_regs[WD_DESTINATION_ID] & SRCID_MASK;
}

void wd33c93_device::set_xfer_count( int count )
{
	/* set the count */
	m_regs[WD_TRANSFER_COUNT_LSB] = count & 0xff;
	m_regs[WD_TRANSFER_COUNT] = (count >> 8) & 0xff;
	m_regs[WD_TRANSFER_COUNT_MSB] = (count >> 16) & 0xff;
}

int wd33c93_device::get_xfer_count()
{
	/* get the count */
	int count = m_regs[WD_TRANSFER_COUNT_MSB];

	count <<= 8;
	count |= m_regs[WD_TRANSFER_COUNT];
	count <<= 8;
	count |= m_regs[WD_TRANSFER_COUNT_LSB];

	return count;
}

void wd33c93_device::complete_immediate(int status)
{
	/* reset our timer */
	m_cmd_timer->reset();

	/* set the new status */
	m_regs[WD_SCSI_STATUS] = status & 0xff;

	/* set interrupt pending */
	m_regs[WD_AUXILIARY_STATUS] |= ASR_INT;

	/* check for error conditions */
	if (get_xfer_count() > 0)
	{
		/* set data buffer ready */
		m_regs[WD_AUXILIARY_STATUS] |= ASR_DBR;
	}
	else
	{
		/* clear data buffer ready */
		m_regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
	}

	/* clear command in progress and bus busy */
	m_regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);

	/* if we have a callback, call it */
	if (!m_irq_cb.isnull())
	{
		m_irq_cb(1);
	}
}

void wd33c93_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	case 0:
		complete_immediate(param);
		break;

	case 1:
		complete_immediate(CSR_SRV_REQ | m_busphase);
		break;

	case 2:
		m_regs[WD_AUXILIARY_STATUS] &= ~ASR_CIP;
		break;
	}
}

void wd33c93_device::complete_cmd(uint8_t status)
{
	/* fire off a timer to complete the command */
	m_cmd_timer->adjust(attotime::from_usec(1), status);
}

/* command handlers */
void wd33c93_device::unimplemented_cmd()
{
	LOGMASKED(LOG_COMMANDS | LOG_ERRORS, "%s: Unimplemented SCSI controller command: %02x\n", machine().describe_context(), m_regs[WD_COMMAND]);

	/* complete the command */
	complete_cmd(CSR_INVALID);
}

void wd33c93_device::invalid_cmd()
{
	LOGMASKED(LOG_COMMANDS | LOG_ERRORS, "%s: Invalid SCSI controller command: %02x\n", machine().describe_context(), m_regs[WD_COMMAND]);

	/* complete the command */
	complete_cmd(CSR_INVALID);
}

void wd33c93_device::reset_cmd()
{
	int advanced = 0;

	/* see if it wants us to reset with advanced features */
	if (m_regs[WD_OWN_ID] & OWNID_EAF)
	{
		advanced = 1;
	}

	/* clear out all registers */
	memset(m_regs, 0, sizeof(m_regs));

	/* complete the command */
	complete_cmd(advanced ? CSR_RESET_AF : CSR_RESET);
}

void wd33c93_device::abort_cmd()
{
	/* complete the command */
	complete_cmd(CSR_ABORT);
}

void wd33c93_device::disconnect_cmd()
{
	/* complete the command */
	m_regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);
}

void wd33c93_device::select_cmd()
{
	uint8_t unit = getunit();
	uint8_t newstatus;

	/* see if we can select that device */
	if (select(unit))
	{
		/* device is available - signal selection done */
		newstatus = CSR_SELECT;

		/* determine the next bus phase depending on the command */
		if ((m_regs[WD_COMMAND] & 0x7f) == WD_CMD_SEL_ATN)
		{
			/* /ATN asserted during select: Move to Message Out Phase to read identify */
			m_busphase = PHS_MESS_OUT;
		}
		else
		{
			/* No /ATN asserted: Move to Command Phase */
			m_busphase = PHS_COMMAND;
		}

		/* queue up a service request out in the future */
		m_service_req_timer->adjust( attotime::from_usec(50) );
	}
	else
	{
		/* device is not available */
		newstatus = CSR_TIMEOUT;
	}

	/* complete the command */
	complete_cmd(newstatus);
}

void wd33c93_device::selectxfer_cmd()
{
	uint8_t unit = getunit();
	uint8_t newstatus;

	/* see if we can select that device */
	if (select(unit))
	{
		if (m_regs[WD_COMMAND_PHASE] < 0x45)
		{
			/* device is available */

			/* do the request */
			send_command(&m_regs[WD_CDB_1], 12);
			int phase = get_phase();

			/* set transfer count */
			if (get_xfer_count() > TEMP_INPUT_LEN)
			{
				LOGMASKED(LOG_ERRORS, "WD33C93: Transfer count too big. Please increase TEMP_INPUT_LEN (size=%d)\n", get_xfer_count());
				set_xfer_count(TEMP_INPUT_LEN);
			}

			switch (phase)
			{
			case SCSI_PHASE_DATAIN:
				m_read_pending = true;
				break;
			}
		}

		if (m_read_pending)
		{
			int len = TEMP_INPUT_LEN;

			if (get_xfer_count() < len)
				len = get_xfer_count();

			memset(&m_temp_input[0], 0, TEMP_INPUT_LEN);
			read_data(&m_temp_input[0], len);
			m_temp_input_pos = 0;
			m_read_pending = false;
		}

		m_regs[WD_TARGET_LUN] = 0;
		m_regs[WD_CONTROL] |= CTRL_EDI;
		m_regs[WD_COMMAND_PHASE] = 0x60;

		/* signal transfer ready */
		newstatus = CSR_SEL_XFER_DONE;

		/* if allowed disconnect, queue a service request */
		if (m_identify & 0x40)
		{
			/* queue disconnect message in */
			m_busphase = PHS_MESS_IN;

			/* queue up a service request out in the future */
			m_service_req_timer->adjust(attotime::from_usec(50));
		}
	}
	else
	{
		/* device is not available */
		newstatus = CSR_TIMEOUT;

		set_xfer_count(0);
	}

	/* complete the command */
	complete_cmd(newstatus);
}

void wd33c93_device::negate_ack()
{
	LOGMASKED(LOG_MISC, "WD33C93: ACK Negated\n");

	/* complete the command */
	m_regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);
}

void wd33c93_device::xferinfo_cmd()
{
	/* make the buffer available right away */
	m_regs[WD_AUXILIARY_STATUS] |= ASR_DBR;
	m_regs[WD_AUXILIARY_STATUS] |= ASR_CIP;

	/* the command will be completed once the data is transferred */
	m_deassert_cip_timer->adjust(attotime::from_msec(1));
}

/* Handle pending commands */
void wd33c93_device::dispatch_command()
{
	/* get the command */
	uint8_t cmd = m_regs[WD_COMMAND] & 0x7f;

	switch (cmd)
	{
	case WD_CMD_RESET:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Reset Command\n", machine().describe_context());
		reset_cmd();
		break;

	case WD_CMD_ABORT:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Abort Command\n", machine().describe_context());
		abort_cmd();
		break;

	case WD_CMD_NEGATE_ACK:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Negate ACK Command\n", machine().describe_context());
		negate_ack();
		break;

	case WD_CMD_DISCONNECT:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Disconnect Command\n", machine().describe_context());
		disconnect_cmd();
		break;

	case WD_CMD_SEL_ATN:
	case WD_CMD_SEL:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Select %sCommand\n", machine().describe_context(), cmd == WD_CMD_SEL_ATN ? "w/ ATN " : "");
		select_cmd();
		break;

	case WD_CMD_SEL_ATN_XFER:
	case WD_CMD_SEL_XFER:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Select %sand Xfer Command\n", machine().describe_context(), cmd == WD_CMD_SEL_ATN ? "w/ ATN " : "");
		selectxfer_cmd();
		break;

	case WD_CMD_TRANS_INFO:
		LOGMASKED(LOG_COMMANDS, "WD33C93: %s - Transfer Info Command\n", machine().describe_context());
		xferinfo_cmd();
		break;

	case WD_CMD_ASSERT_ATN:
	case WD_CMD_RESELECT:
	case WD_CMD_RESEL_RECEIVE:
	case WD_CMD_RESEL_SEND:
	case WD_CMD_WAIT_SEL_RECEIVE:
	case WD_CMD_SSCC:
	case WD_CMD_SND_DISC:
	case WD_CMD_SET_IDI:
	case WD_CMD_RCV_CMD:
	case WD_CMD_RCV_DATA:
	case WD_CMD_RCV_MSG_OUT:
	case WD_CMD_RCV:
	case WD_CMD_SND_STATUS:
	case WD_CMD_SND_DATA:
	case WD_CMD_SND_MSG_IN:
	case WD_CMD_SND:
	case WD_CMD_TRANS_ADDR:
	case WD_CMD_XFER_PAD:
	case WD_CMD_TRANSFER_PAD:
		unimplemented_cmd();
		break;

	default:
		invalid_cmd();
		break;
	}
}

WRITE8_MEMBER(wd33c93_device::write)
{
	switch (offset)
	{
	case 0:
	{
		/* update register select */
		m_sasr = data & 0x1f;
	}
	break;

	case 1:
	{
		/* update the register */
		if (m_sasr != WD_SCSI_STATUS && m_sasr <= WD_QUEUE_TAG)
		{
			m_regs[m_sasr] = data;
		}

		switch (m_sasr)
		{
		case WD_OWN_ID:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Own ID Register (CDB Size) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CONTROL:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Control Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_TIMEOUT_PERIOD:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Timeout Period Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_1:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Total Sectors Register (CDB1) = %02x\n", machine().describe_context(), m_sasr, data);
			m_regs[WD_COMMAND_PHASE] = 0;
			break;
		case WD_CDB_2:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Total Heads Register (CDB2) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_3:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Total Cylinders Register MSB (CDB3) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_4:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Total Cylinders Register LSB (CDB4) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_5:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Logical Address Register MSB (CDB5) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_6:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Logical Address Register 2nd (CDB6) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_7:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Logical Address Register 3rd (CDB7) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_8:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Logical Address Register LSB (CDB8) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_9:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Sector Number Register (CDB9) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_10:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Head Number Register (CDB10) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_11:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Cylinder Number Register MSB (CDB11) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_CDB_12:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Cylinder Number Register LSB (CDB12) = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_TARGET_LUN:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Target LUN Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_COMMAND_PHASE:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Command Phase Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_SYNCHRONOUS_TRANSFER:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Synchronous Transfer Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_TRANSFER_COUNT_MSB:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Transfer Count Register MSB = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_TRANSFER_COUNT:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Transfer Count Register 2nd = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_TRANSFER_COUNT_LSB:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Transfer Count Register LSB = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_DESTINATION_ID:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Destination ID Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_SOURCE_ID:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Source ID Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_SCSI_STATUS:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, SCSI Status Register (read-only!) = %02x (ignored)\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_COMMAND:
			/* if we receive a command, schedule to process it */
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Command Register = %02x - unit %d\n", machine().describe_context(), m_sasr, data, getunit());

			/* signal we're processing it */
			m_regs[WD_AUXILIARY_STATUS] |= ASR_CIP;

			/* process the command */
			dispatch_command();
			break;
		case WD_DATA:
		{
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Data Register = %02x\n", machine().describe_context(), m_sasr, data);

			/* if data was written, and we have a count, send to device */
			int count = get_xfer_count();

			if (m_regs[WD_COMMAND] & 0x80)
				count = 1;

			if (count-- > 0)
			{
				/* write to FIFO */
				if (m_fifo_pos < FIFO_SIZE)
				{
					m_fifo[m_fifo_pos++] = data;
				}

				/* update count */
				set_xfer_count(count);

				/* if we're done with the write, see where we're at */
				if (count == 0)
				{
					m_regs[WD_AUXILIARY_STATUS] |= ASR_INT;
					m_regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;

					switch (m_busphase)
					{
						case PHS_MESS_OUT:
						{
							/* reset fifo */
							m_fifo_pos = 0;

							/* Message out phase. Data is probably SCSI Identify. Move to command phase. */
							m_busphase = PHS_COMMAND;

							m_identify = m_fifo[0];
						}
						break;

						case PHS_COMMAND:
						{
							/* Execute the command. Depending on the command, we'll move to data in or out */
							send_command(&m_fifo[0], 12);
							int xfercount = get_length();
							int phase = get_phase();

							/* reset fifo */
							m_fifo_pos = 0;

							/* set the new count */
							set_xfer_count(xfercount);

							switch (phase)
							{
							case SCSI_PHASE_STATUS:
								m_busphase = PHS_STATUS;
								break;

							case SCSI_PHASE_DATAIN:
								m_busphase = PHS_DATA_IN;
								m_read_pending = true;
								break;

							case SCSI_PHASE_DATAOUT:
								m_busphase = PHS_DATA_OUT;
								break;
							}
						}
						break;

						case PHS_DATA_OUT:
						{
							/* write data out to device */
							write_data(m_fifo, m_fifo_pos);

							/* reset fifo */
							m_fifo_pos = 0;

							/* move to status phase */
							m_busphase = PHS_STATUS;
						}
						break;
					}

					/* complete the command */
					complete_immediate(CSR_XFER_DONE | m_busphase);
				}
			}
			else
			{
				LOGMASKED(LOG_MISC | LOG_ERRORS, "WD33C93: Sending data to device with transfer count = 0!. Ignoring...\n");
			}
			break;
		}
		case WD_QUEUE_TAG:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Queue Tag Register = %02x\n", machine().describe_context(), m_sasr, data);
			break;
		case WD_AUXILIARY_STATUS:
			LOGMASKED(LOG_WRITES, "WD33C93: %s - Write Register %02x, Auxiliary Status Register (read-only!) = %02x (ignored)\n", machine().describe_context(), m_sasr, data);
			break;
		default:
			LOGMASKED(LOG_WRITES | LOG_ERRORS, "WD33C93: %s - Write Register %02x, Unknown = %02x (ignored)\n", machine().describe_context(), m_sasr, data);
			break;
		}

		/* auto-increment register select if not on special registers */
		if (m_sasr != WD_COMMAND && m_sasr != WD_DATA && m_sasr != WD_AUXILIARY_STATUS)
		{
			m_sasr = (m_sasr + 1) & 0x1f;
		}
	}
	break;

	default:
	{
		LOGMASKED(LOG_ERRORS, "WD33C93: Write to invalid offset %d (data=%02x)\n", offset, data);
	}
	break;
	}
}

READ8_MEMBER(wd33c93_device::read)
{
	switch (offset)
	{
	case 0:
		/* read aux status */
		return m_regs[WD_AUXILIARY_STATUS];

	case 1:
	{
		switch (m_sasr)
		{
		case WD_OWN_ID:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Own ID Register (CDB Size) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CONTROL:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Control Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_TIMEOUT_PERIOD:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Timeout Period Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_1:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Total Sectors Register (CDB1) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			m_regs[WD_COMMAND_PHASE] = 0;
			break;
		case WD_CDB_2:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Total Heads Register (CDB2) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_3:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Total Cylinders Register MSB (CDB3) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_4:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Total Cylinders Register LSB (CDB4) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_5:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Logical Address Register MSB (CDB5) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_6:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Logical Address Register 2nd (CDB6) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_7:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Logical Address Register 3rd (CDB7) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_8:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Logical Address Register LSB (CDB8) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_9:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Sector Number Register (CDB9) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_10:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Head Number Register (CDB10) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_11:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Cylinder Number Register MSB (CDB11) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_CDB_12:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Cylinder Number Register LSB (CDB12) (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_TARGET_LUN:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Target LUN Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_COMMAND_PHASE:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Command Phase Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_SYNCHRONOUS_TRANSFER:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Synchronous Transfer Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_TRANSFER_COUNT_MSB:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Transfer Count Register MSB (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_TRANSFER_COUNT:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Transfer Count Register 2nd (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_TRANSFER_COUNT_LSB:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Transfer Count Register LSB (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_DESTINATION_ID:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Destination ID Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_SOURCE_ID:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Source ID Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_SCSI_STATUS:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, SCSI Status Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			m_regs[WD_AUXILIARY_STATUS] &= ~ASR_INT;

			/* if reading status, clear irq flag */
			if (!m_irq_cb.isnull())
			{
				m_irq_cb(0);
			}
			break;
		case WD_COMMAND:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Command Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_DATA:
		{
			/* we're going to be doing synchronous reads */

			/* get the transfer count */
			int count = get_xfer_count();

			/* initialize the return value */
			m_regs[WD_DATA] = 0;

			if (count <= 0 && m_busphase == PHS_MESS_IN)
			{
				/* move to disconnect */
				complete_cmd(CSR_DISC);
			}
			else if (count == 1 && m_busphase == PHS_STATUS)
			{
				/* update the count */
				set_xfer_count(0);

				/* move to message in phase */
				m_busphase = PHS_MESS_IN;

				/* complete the command */
				complete_cmd(CSR_XFER_DONE | m_busphase);
			}
			else if (count-- > 0) /* make sure we still have data to send */
			{
				if (m_read_pending)
				{
					int len = TEMP_INPUT_LEN;

					if ((count + 1) < len )
						len = count + 1;
					read_data(&m_temp_input[0], len);
					m_temp_input_pos = 0;
					m_read_pending = false;
				}

				m_regs[WD_AUXILIARY_STATUS] &= ~ASR_INT;

				/* read in one byte */
				if (m_temp_input_pos < TEMP_INPUT_LEN)
					m_regs[WD_DATA] = m_temp_input[m_temp_input_pos++];

				/* update the count */
				set_xfer_count(count);

				/* transfer finished, see where we're at */
				if (count == 0)
				{
					if (m_regs[WD_COMMAND_PHASE] != 0x60)
					{
						/* move to status phase */
						m_busphase = PHS_STATUS;

						/* complete the command */
						complete_cmd(CSR_XFER_DONE | m_busphase);
					}
					else
					{
						m_regs[WD_AUXILIARY_STATUS] |= ASR_INT;
						m_regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
					}
				}
				LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Data Register (%02x)\n", machine().describe_context(), WD_DATA, m_regs[WD_DATA]);
			}
			break;
		}
		case WD_QUEUE_TAG:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Queue Tag Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		case WD_AUXILIARY_STATUS:
			LOGMASKED(LOG_READS, "WD33C93: %s - Read Register %02x, Auxiliary Status Register (%02x)\n", machine().describe_context(), m_sasr, m_regs[m_sasr]);
			break;
		default:
			LOGMASKED(LOG_READS | LOG_ERRORS, "WD33C93: %s - Read Register %02x, Unknown\n", machine().describe_context(), m_sasr);
			break;
		}

		/* get the register value */
		uint8_t ret = 0xff;
		if (m_sasr == WD_AUXILIARY_STATUS || m_sasr <= WD_QUEUE_TAG)
			ret = m_regs[m_sasr];

		/* auto-increment register select if not on special registers */
		if (m_sasr != WD_COMMAND && m_sasr != WD_DATA && m_sasr != WD_AUXILIARY_STATUS)
		{
			m_sasr = (m_sasr + 1) & 0x1f;
		}

		return ret;
	}

	default:
		LOGMASKED(LOG_READS | LOG_ERRORS, "WD33C93: Read from invalid offset %d\n", offset);
		break;
	}

	return 0;
}

wd33c93_device::wd33c93_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	legacy_scsi_host_adapter(mconfig, WD33C93, tag, owner, clock),
	m_irq_cb(*this)
{
}

void wd33c93_device::device_start()
{
	legacy_scsi_host_adapter::device_start();

	memset(m_regs, 0, sizeof(m_regs));
	memset(m_fifo, 0, sizeof(m_fifo));
	memset(m_temp_input, 0, sizeof(m_temp_input));

	m_sasr = 0;
	m_fifo_pos = 0;
	m_temp_input_pos = 0;
	m_busphase = 0;
	m_identify = 0;
	m_read_pending = 0;

	m_irq_cb.resolve();

	/* allocate a timer for commands */
	m_cmd_timer = timer_alloc(0);
	m_service_req_timer = timer_alloc(1);
	m_deassert_cip_timer = timer_alloc(2);

	save_item(NAME(m_sasr));
	save_item(NAME(m_regs));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_pos));
	save_item(NAME(m_temp_input));
	save_item(NAME(m_temp_input_pos));
	save_item(NAME(m_busphase));
	save_item(NAME(m_identify));
	save_item(NAME(m_read_pending));
}

void wd33c93_device::dma_read_data(int bytes, uint8_t *data)
{
	int len = bytes;

	if (len >= get_xfer_count())
		len = get_xfer_count();

	if (len == 0)
		return;

	if ((m_temp_input_pos + len) >= TEMP_INPUT_LEN)
	{
		LOGMASKED(LOG_ERRORS, "Reading past end of buffer, increase TEMP_INPUT_LEN size\n");
		len = TEMP_INPUT_LEN - len;
	}

	assert(len);

	memcpy(data, &m_temp_input[m_temp_input_pos], len);

	m_temp_input_pos += len;
	len = get_xfer_count() - len;
	set_xfer_count(len);
}

void wd33c93_device::dma_write_data(int bytes, uint8_t *data)
{
	write_data(data, bytes);
}

void wd33c93_device::clear_dma()
{
	/* indicate DMA completed by clearing the transfer count */
	set_xfer_count(0);
	m_regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
}

int wd33c93_device::get_dma_count()
{
	return get_xfer_count();
}

DEFINE_DEVICE_TYPE(WD33C93, wd33c93_device, "wd33c93", "Western Digital WD33C93 SCSI")
