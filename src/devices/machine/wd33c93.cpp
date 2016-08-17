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

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

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
UINT8 wd33c93_device::getunit( void )
{
	/* return the destination unit id */
	return regs[WD_DESTINATION_ID] & SRCID_MASK;
}

void wd33c93_device::set_xfer_count( int count )
{
	/* set the count */
	regs[ WD_TRANSFER_COUNT_LSB ] = count & 0xff;
	regs[ WD_TRANSFER_COUNT ] = ( count >> 8 ) & 0xff;
	regs[ WD_TRANSFER_COUNT_MSB ] = ( count >> 16 ) & 0xff;
}

int wd33c93_device::get_xfer_count( void )
{
	/* get the count */
	int count = regs[ WD_TRANSFER_COUNT_MSB ];

	count <<= 8;
	count |= regs[ WD_TRANSFER_COUNT ];
	count <<= 8;
	count |= regs[ WD_TRANSFER_COUNT_LSB ];

	return count;
}

void wd33c93_device::complete_immediate( int status )
{
	/* reset our timer */
	cmd_timer->reset();

	/* set the new status */
	regs[WD_SCSI_STATUS] = status & 0xff;

	/* set interrupt pending */
	regs[WD_AUXILIARY_STATUS] |= ASR_INT;

	/* check for error conditions */
	if ( get_xfer_count() > 0 )
	{
		/* set data buffer ready */
		regs[WD_AUXILIARY_STATUS] |= ASR_DBR;
	}
	else
	{
		/* clear data buffer ready */
		regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
	}

	/* clear command in progress and bus busy */
	regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);

	/* if we have a callback, call it */
	if (!m_irq_cb.isnull())
	{
		m_irq_cb(1);
	}
}

void wd33c93_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch( tid )
	{
	case 0:
		complete_immediate( param );
		break;

	case 1:
		complete_immediate(CSR_SRV_REQ | busphase);
		break;

	case 2:
		regs[WD_AUXILIARY_STATUS] &= ~ASR_CIP;
		break;
	}
}

void wd33c93_device::complete_cmd( UINT8 status )
{
	/* fire off a timer to complete the command */
	cmd_timer->adjust( attotime::from_usec(1), status );
}

/* command handlers */
void wd33c93_device::unimplemented_cmd()
{
	logerror( "%s:Unimplemented SCSI controller command: %02x\n", machine().describe_context(), regs[WD_COMMAND] );

	/* complete the command */
	complete_cmd( CSR_INVALID );
}

void wd33c93_device::invalid_cmd()
{
	logerror( "%s:Invalid SCSI controller command: %02x\n", machine().describe_context(), regs[WD_COMMAND] );

	/* complete the command */
	complete_cmd( CSR_INVALID );
}

void wd33c93_device::reset_cmd()
{
	int advanced = 0;

	/* see if it wants us to reset with advanced features */
	if ( regs[WD_OWN_ID] & OWNID_EAF )
	{
		advanced = 1;
	}

	/* clear out all registers */
	memset( regs, 0, sizeof( regs ) );

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
	regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);
}

void wd33c93_device::select_cmd()
{
	UINT8 unit = getunit();
	UINT8 newstatus;

	/* see if we can select that device */
	if (select(unit))
	{
		/* device is available - signal selection done */
		newstatus = CSR_SELECT;

		/* determine the next bus phase depending on the command */
		if ( (regs[WD_COMMAND] & 0x7f) == WD_CMD_SEL_ATN )
		{
			/* /ATN asserted during select: Move to Message Out Phase to read identify */
			busphase = PHS_MESS_OUT;
		}
		else
		{
			/* No /ATN asserted: Move to Command Phase */
			busphase = PHS_COMMAND;
		}

		/* queue up a service request out in the future */
		service_req_timer->adjust( attotime::from_usec(50) );
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
	UINT8 unit = getunit();
	UINT8 newstatus;

	/* see if we can select that device */
	if (select(unit))
	{
		if ( regs[WD_COMMAND_PHASE] < 0x45 )
		{
			/* device is available */
			int phase;

			/* do the request */
			send_command(&regs[WD_CDB_1], 12);
			phase = get_phase();

			/* set transfer count */
			if ( get_xfer_count() > TEMP_INPUT_LEN )
			{
				logerror( "WD33C93: Transfer count too big. Please increase TEMP_INPUT_LEN (size=%d)\n", get_xfer_count() );
				set_xfer_count( TEMP_INPUT_LEN );
			}

			switch( phase )
			{
				case SCSI_PHASE_DATAIN:
					read_pending = 1;
					break;
			}
		}

		if ( read_pending )
		{
			int len = TEMP_INPUT_LEN;

			if ( get_xfer_count() < len ) len = get_xfer_count();

			memset( &temp_input[0], 0, TEMP_INPUT_LEN );
			read_data(&temp_input[0], len);
			temp_input_pos = 0;
			read_pending = 0;
		}

		regs[WD_TARGET_LUN] = 0;
		regs[WD_CONTROL] |= CTRL_EDI;
		regs[WD_COMMAND_PHASE] = 0x60;

		/* signal transfer ready */
		newstatus = CSR_SEL_XFER_DONE;

		/* if allowed disconnect, queue a service request */
		if ( identify & 0x40 )
		{
			/* queue disconnect message in */
			busphase = PHS_MESS_IN;

			/* queue up a service request out in the future */
			service_req_timer->adjust( attotime::from_usec(50) );
		}
	}
	else
	{
		/* device is not available */
		newstatus = CSR_TIMEOUT;

		set_xfer_count( 0 );
	}

	/* complete the command */
	complete_cmd(newstatus);
}

void wd33c93_device::negate_ack()
{
	logerror( "WD33C93: ACK Negated\n" );

	/* complete the command */
	regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);
}

void wd33c93_device::xferinfo_cmd()
{
	/* make the buffer available right away */
	regs[WD_AUXILIARY_STATUS] |= ASR_DBR;
	regs[WD_AUXILIARY_STATUS] |= ASR_CIP;

	/* the command will be completed once the data is transferred */
	deassert_cip_timer->adjust( attotime::from_msec(1) );
}

/* Handle pending commands */
void wd33c93_device::dispatch_command()
{
	/* get the command */
	UINT8 cmd = regs[WD_COMMAND] & 0x7f;

	switch(cmd)
	{
	case WD_CMD_RESET:
		reset_cmd();
		break;

	case WD_CMD_ABORT:
		abort_cmd();
		break;

	case WD_CMD_NEGATE_ACK:
		negate_ack();
		break;

	case WD_CMD_DISCONNECT:
		disconnect_cmd();
		break;

	case WD_CMD_SEL_ATN:
	case WD_CMD_SEL:
		select_cmd();
		break;

	case WD_CMD_SEL_ATN_XFER:
	case WD_CMD_SEL_XFER:
		selectxfer_cmd();
		break;

	case WD_CMD_TRANS_INFO:
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
	switch( offset )
	{
		case 0:
		{
			/* update register select */
			sasr = data & 0x1f;
		}
		break;

		case 1:
		{
			LOG(( "WD33C93: PC=%08x - Write REG=%02x, data = %02x\n", space.device().safe_pc(), sasr, data ));

			/* update the register */
			regs[sasr] = data;

			/* if we receive a command, schedule to process it */
			if ( sasr == WD_COMMAND )
			{
				LOG(( "WDC33C93: PC=%08x - Executing command %08x - unit %d\n", space.device().safe_pc(), data, getunit() ));

				/* signal we're processing it */
				regs[WD_AUXILIARY_STATUS] |= ASR_CIP;

				/* process the command */
				dispatch_command();
			}
			else if ( sasr == WD_CDB_1 )
			{
				regs[WD_COMMAND_PHASE] = 0;
			}
			else if ( sasr == WD_DATA )
			{
				/* if data was written, and we have a count, send to device */
				int count = get_xfer_count();

				if ( regs[WD_COMMAND] & 0x80 )
					count = 1;

				if ( count-- > 0 )
				{
					/* write to FIFO */
					if ( fifo_pos < FIFO_SIZE )
					{
						fifo[fifo_pos++] = data;
					}

					/* update count */
					set_xfer_count( count );

					/* if we're done with the write, see where we're at */
					if ( count == 0 )
					{
						regs[WD_AUXILIARY_STATUS] |= ASR_INT;
						regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;

						switch( busphase )
						{
							case PHS_MESS_OUT:
							{
								/* reset fifo */
								fifo_pos = 0;

								/* Message out phase. Data is probably SCSI Identify. Move to command phase. */
								busphase = PHS_COMMAND;

								identify = fifo[0];
							}
							break;

							case PHS_COMMAND:
							{
								int xfercount;
								int phase;

								/* Execute the command. Depending on the command, we'll move to data in or out */
								send_command(&fifo[0], 12);
								xfercount = get_length();
								phase = get_phase();

								/* reset fifo */
								fifo_pos = 0;

								/* set the new count */
								set_xfer_count( xfercount );

								switch( phase )
								{
								case SCSI_PHASE_STATUS:
									busphase = PHS_STATUS;
									break;

								case SCSI_PHASE_DATAIN:
									busphase = PHS_DATA_IN;
									read_pending = 1;
									break;

								case SCSI_PHASE_DATAOUT:
									busphase = PHS_DATA_OUT;
									break;
								}
							}
							break;

							case PHS_DATA_OUT:
							{
								/* write data out to device */
								write_data(fifo, fifo_pos);

								/* reset fifo */
								fifo_pos = 0;

								/* move to status phase */
								busphase = PHS_STATUS;
							}
							break;
						}

						/* complete the command */
						complete_immediate(CSR_XFER_DONE | busphase);
					}
				}
				else
				{
					logerror( "WD33C93: Sending data to device with transfer count = 0!. Ignoring...\n" );
				}
			}

			/* auto-increment register select if not on special registers */
			if ( sasr != WD_COMMAND && sasr != WD_DATA && sasr != WD_AUXILIARY_STATUS )
			{
				sasr = ( sasr + 1 ) & 0x1f;
			}
		}
		break;

		default:
		{
			logerror( "WD33C93: Write to invalid offset %d (data=%02x)\n", offset, data );
		}
		break;
	}
}

READ8_MEMBER(wd33c93_device::read)
{
	switch( offset )
	{
		case 0:
		{
			/* read aux status */
			return regs[WD_AUXILIARY_STATUS];
		}

		case 1:
		{
			UINT8 ret;

			/* if reading status, clear irq flag */
			if ( sasr == WD_SCSI_STATUS )
			{
				regs[WD_AUXILIARY_STATUS] &= ~ASR_INT;

				if (!m_irq_cb.isnull())
				{
					m_irq_cb(0);
				}

				LOG(( "WD33C93: PC=%08x - Status read (%02x)\n", space.device().safe_pc(), regs[WD_SCSI_STATUS] ));
			}
			else if ( sasr == WD_DATA )
			{
				/* we're going to be doing synchronous reads */

				/* get the transfer count */
				int count = get_xfer_count();

				/* initialize the return value */
				regs[WD_DATA] = 0;

				if ( count <= 0 && busphase == PHS_MESS_IN )
				{
					/* move to disconnect */
					complete_cmd(CSR_DISC);
				}
				else if ( count == 1 && busphase == PHS_STATUS )
				{
					/* update the count */
					set_xfer_count( 0 );

					/* move to message in phase */
					busphase = PHS_MESS_IN;

					/* complete the command */
					complete_cmd(CSR_XFER_DONE | busphase);
				}
				else if ( count-- > 0 ) /* make sure we still have data to send */
				{
					if ( read_pending )
					{
						int len = TEMP_INPUT_LEN;

						if ( (count+1) < len ) len = count+1;
						read_data(&temp_input[0], len);
						temp_input_pos = 0;
						read_pending = 0;
					}

					regs[WD_AUXILIARY_STATUS] &= ~ASR_INT;

					/* read in one byte */
					if ( temp_input_pos < TEMP_INPUT_LEN )
						regs[WD_DATA] = temp_input[temp_input_pos++];

					/* update the count */
					set_xfer_count( count );

					/* transfer finished, see where we're at */
					if ( count == 0 )
					{
						if ( regs[WD_COMMAND_PHASE] != 0x60 )
						{
							/* move to status phase */
							busphase = PHS_STATUS;

							/* complete the command */
							complete_cmd(CSR_XFER_DONE | busphase);
						}
						else
						{
							regs[WD_AUXILIARY_STATUS] |= ASR_INT;
							regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
						}
					}
				}
			}

			LOG(( "WD33C93: PC=%08x - Data read (%02x)\n", space.device().safe_pc(), regs[WD_DATA] ));

			/* get the register value */
			ret = regs[sasr];

			/* auto-increment register select if not on special registers */
			if ( sasr != WD_COMMAND && sasr != WD_DATA && sasr != WD_AUXILIARY_STATUS )
			{
				sasr = ( sasr + 1 ) & 0x1f;
			}

			return ret;
		}

		default:
		{
			logerror( "WD33C93: Read from invalid offset %d\n", offset );
		}
		break;
	}

	return 0;
}

wd33c93_device::wd33c93_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	legacy_scsi_host_adapter(mconfig, WD33C93, "33C93 SCSI", tag, owner, clock, "wd33c93", __FILE__),
	m_irq_cb(*this)
{
}

void wd33c93_device::device_start()
{
	legacy_scsi_host_adapter::device_start();

	memset(regs, 0, sizeof(regs));
	memset(fifo, 0, sizeof(fifo));
	memset(temp_input, 0, sizeof(temp_input));

	sasr = 0;
	fifo_pos = 0;
	temp_input_pos = 0;
	busphase = 0;
	identify = 0;
	read_pending = 0;

	m_irq_cb.resolve();

	/* allocate a timer for commands */
	cmd_timer = timer_alloc(0);
	service_req_timer = timer_alloc(1);
	deassert_cip_timer = timer_alloc(2);

	save_item( NAME( sasr ) );
	save_item( NAME( regs ) );
	save_item( NAME( fifo ) );
	save_item( NAME( fifo_pos ) );
	save_item( NAME( temp_input ) );
	save_item( NAME( temp_input_pos ) );
	save_item( NAME( busphase ) );
	save_item( NAME( identify ) );
	save_item( NAME( read_pending ) );
}

void wd33c93_device::dma_read_data( int bytes, UINT8 *pData )
{
	int len = bytes;

	if ( len >= get_xfer_count() )
		len = get_xfer_count();

	if ( len == 0 )
		return;

	if ( (temp_input_pos+len) >= TEMP_INPUT_LEN )
	{
		logerror( "Reading past end of buffer, increase TEMP_INPUT_LEN size\n" );
		len = TEMP_INPUT_LEN - len;
	}

	assert(len);

	memcpy( pData, &temp_input[temp_input_pos], len );

	temp_input_pos += len;
	len = get_xfer_count() - len;
	set_xfer_count(len);
}

void wd33c93_device::dma_write_data(int bytes, UINT8 *pData)
{
	write_data(pData, bytes);
}

void wd33c93_device::clear_dma()
{
	/* indicate DMA completed by clearing the transfer count */
	set_xfer_count(0);
	regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
}

int wd33c93_device::get_dma_count()
{
	return get_xfer_count();
}

const device_type WD33C93 = &device_creator<wd33c93_device>;
