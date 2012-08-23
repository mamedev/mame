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
#include "machine/scsidev.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static scsidev_device *devices[8];	// SCSI IDs 0-7
static const struct WD33C93interface *intf;

/* wd register names */
#define WD_OWN_ID					0x00
#define WD_CONTROL					0x01
#define WD_TIMEOUT_PERIOD			0x02
#define WD_CDB_1					0x03
#define WD_CDB_2					0x04
#define WD_CDB_3					0x05
#define WD_CDB_4					0x06
#define WD_CDB_5					0x07
#define WD_CDB_6					0x08
#define WD_CDB_7					0x09
#define WD_CDB_8					0x0a
#define WD_CDB_9					0x0b
#define WD_CDB_10					0x0c
#define WD_CDB_11					0x0d
#define WD_CDB_12					0x0e
#define WD_TARGET_LUN				0x0f
#define WD_COMMAND_PHASE			0x10
#define WD_SYNCHRONOUS_TRANSFER		0x11
#define WD_TRANSFER_COUNT_MSB		0x12
#define WD_TRANSFER_COUNT			0x13
#define WD_TRANSFER_COUNT_LSB		0x14
#define WD_DESTINATION_ID			0x15
#define WD_SOURCE_ID				0x16
#define WD_SCSI_STATUS				0x17
#define WD_COMMAND					0x18
#define WD_DATA						0x19
#define WD_QUEUE_TAG				0x1a
#define WD_AUXILIARY_STATUS			0x1f

/* WD commands */
#define WD_CMD_RESET				0x00
#define WD_CMD_ABORT				0x01
#define WD_CMD_ASSERT_ATN			0x02
#define WD_CMD_NEGATE_ACK			0x03
#define WD_CMD_DISCONNECT			0x04
#define WD_CMD_RESELECT				0x05
#define WD_CMD_SEL_ATN				0x06
#define WD_CMD_SEL					0x07
#define WD_CMD_SEL_ATN_XFER			0x08
#define WD_CMD_SEL_XFER				0x09
#define WD_CMD_RESEL_RECEIVE		0x0a
#define WD_CMD_RESEL_SEND			0x0b
#define WD_CMD_WAIT_SEL_RECEIVE		0x0c
#define WD_CMD_SSCC					0x0d
#define WD_CMD_SND_DISC				0x0e
#define WD_CMD_SET_IDI				0x0f
#define WD_CMD_RCV_CMD				0x10
#define WD_CMD_RCV_DATA				0x11
#define WD_CMD_RCV_MSG_OUT			0x12
#define WD_CMD_RCV					0x13
#define WD_CMD_SND_STATUS			0x14
#define WD_CMD_SND_DATA				0x15
#define WD_CMD_SND_MSG_IN			0x16
#define WD_CMD_SND					0x17
#define WD_CMD_TRANS_ADDR			0x18
#define WD_CMD_XFER_PAD				0x19
#define WD_CMD_TRANS_INFO			0x20
#define WD_CMD_TRANSFER_PAD			0x21
#define WD_CMD_SBT_MODE				0x80

/* ASR register */
#define ASR_INT						0x80
#define ASR_LCI						0x40
#define ASR_BSY						0x20
#define ASR_CIP						0x10
#define ASR_PE						0x02
#define ASR_DBR						0x01

/* SCSI Bus Phases */
#define PHS_DATA_OUT				0x00
#define PHS_DATA_IN					0x01
#define PHS_COMMAND					0x02
#define PHS_STATUS					0x03
#define PHS_MESS_OUT				0x06
#define PHS_MESS_IN					0x07

/* Command Status Register definitions */

  /* reset state interrupts */
#define CSR_RESET					0x00
#define CSR_RESET_AF				0x01

  /* successful completion interrupts */
#define CSR_RESELECT				0x10
#define CSR_SELECT					0x11
#define CSR_SEL_XFER_DONE			0x16
#define CSR_XFER_DONE				0x18

  /* paused or aborted interrupts */
#define CSR_MSGIN					0x20
#define CSR_SDP						0x21
#define CSR_SEL_ABORT				0x22
#define CSR_RESEL_ABORT				0x25
#define CSR_RESEL_ABORT_AM			0x27
#define CSR_ABORT					0x28

  /* terminated interrupts */
#define CSR_INVALID					0x40
#define CSR_UNEXP_DISC				0x41
#define CSR_TIMEOUT					0x42
#define CSR_PARITY					0x43
#define CSR_PARITY_ATN				0x44
#define CSR_BAD_STATUS				0x45
#define CSR_UNEXP					0x48

  /* service required interrupts */
#define CSR_RESEL					0x80
#define CSR_RESEL_AM				0x81
#define CSR_DISC					0x85
#define CSR_SRV_REQ					0x88

   /* Own ID/CDB Size register */
#define OWNID_EAF					0x08
#define OWNID_EHP					0x10
#define OWNID_RAF					0x20
#define OWNID_FS_8					0x00
#define OWNID_FS_12					0x40
#define OWNID_FS_16					0x80

   /* Control register */
#define CTRL_HSP					0x01
#define CTRL_HA						0x02
#define CTRL_IDI					0x04
#define CTRL_EDI					0x08
#define CTRL_HHP					0x10
#define CTRL_POLLED					0x00
#define CTRL_BURST					0x20
#define CTRL_BUS					0x40
#define CTRL_DMA					0x80

   /* Synchronous Transfer Register */
#define STR_FSS						0x80

   /* Destination ID register */
#define DSTID_DPD					0x40
#define DATA_OUT_DIR				0
#define DATA_IN_DIR					1
#define DSTID_SCC					0x80

   /* Source ID register */
#define SRCID_MASK					0x07
#define SRCID_SIV					0x08
#define SRCID_DSP					0x20
#define SRCID_ES					0x40
#define SRCID_ER					0x80

/* command handler definition */
typedef void (*cmd_handler)(running_machine &machine);
#define CMD_HANDLER(name) void name(running_machine &machine)

#define TEMP_INPUT_LEN	262144
#define FIFO_SIZE		12

/* internal controller data definition */
typedef struct
{
	UINT8		sasr;
	UINT8		regs[WD_AUXILIARY_STATUS+1];
	UINT8		fifo[FIFO_SIZE];
	int			fifo_pos;
	UINT8		*temp_input;
	int			temp_input_pos;
	UINT8		busphase;
	UINT8		identify;
	int			read_pending;
	emu_timer *cmd_timer;
} _wd33c93_data;

/* local instance of controller data */
static _wd33c93_data scsi_data;


/* convernience functions */
static UINT8 wd33c93_getunit( void )
{
	/* return the destination unit id */
	return scsi_data.regs[WD_DESTINATION_ID] & SRCID_MASK;
}

static void wd33c93_set_xfer_count( int count )
{
	/* set the count */
	scsi_data.regs[ WD_TRANSFER_COUNT_LSB ] = count & 0xff;
	scsi_data.regs[ WD_TRANSFER_COUNT ] = ( count >> 8 ) & 0xff;
	scsi_data.regs[ WD_TRANSFER_COUNT_MSB ] = ( count >> 16 ) & 0xff;
}

static int wd33c93_get_xfer_count( void )
{
	/* get the count */
	int		count = scsi_data.regs[ WD_TRANSFER_COUNT_MSB ];

	count <<= 8;
	count |= scsi_data.regs[ WD_TRANSFER_COUNT ];
	count <<= 8;
	count |= scsi_data.regs[ WD_TRANSFER_COUNT_LSB ];

	return count;
}

static void wd33c93_read_data(int bytes, UINT8 *pData)
{
	UINT8	unit = wd33c93_getunit();

	if ( devices[unit] )
	{
		devices[unit]->ReadData( pData, bytes );
	}
	else
	{
		logerror("wd33c93: request for unknown device SCSI ID %d\n", unit);
	}
}

static void wd33c93_complete_immediate( running_machine &machine, int status )
{
	/* reset our timer */
	scsi_data.cmd_timer->reset(  );

	/* set the new status */
	scsi_data.regs[WD_SCSI_STATUS] = status & 0xff;

	/* set interrupt pending */
	scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_INT;

	/* check for error conditions */
	if ( wd33c93_get_xfer_count() > 0 )
	{
		/* set data buffer ready */
		scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_DBR;
	}
	else
	{
		/* clear data buffer ready */
		scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
	}

	/* clear command in progress and bus busy */
	scsi_data.regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);

	/* if we have a callback, call it */
	if (intf && intf->irq_callback)
	{
		intf->irq_callback(machine, 1);
	}
}

static TIMER_CALLBACK(wd33c93_complete_cb)
{
	wd33c93_complete_immediate( machine, param );
}

static TIMER_CALLBACK(wd33c93_service_request)
{
	/* issue a message out request */
	wd33c93_complete_immediate(machine, CSR_SRV_REQ | scsi_data.busphase);
}

static TIMER_CALLBACK(wd33c93_deassert_cip)
{
	scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_CIP;
}

static void wd33c93_complete_cmd( UINT8 status )
{
	/* fire off a timer to complete the command */
	scsi_data.cmd_timer->adjust( attotime::from_usec(1), status );
}

/* command handlers */
static CMD_HANDLER( wd33c93_invalid_cmd )
{
	logerror( "%s:Unknown/Unimplemented SCSI controller command: %02x\n", machine.describe_context(), scsi_data.regs[WD_COMMAND] );

	/* complete the command */
	wd33c93_complete_cmd( CSR_INVALID );
}

static CMD_HANDLER( wd33c93_reset_cmd )
{
	int		advanced = 0;

	/* see if it wants us to reset with advanced features */
	if ( scsi_data.regs[WD_OWN_ID] & OWNID_EAF )
	{
		advanced = 1;
	}

	/* clear out all registers */
	memset( scsi_data.regs, 0, sizeof( scsi_data.regs ) );

	/* complete the command */
	wd33c93_complete_cmd(advanced ? CSR_RESET_AF : CSR_RESET);
}

static CMD_HANDLER( wd33c93_abort_cmd )
{
	/* complete the command */
	wd33c93_complete_cmd(CSR_ABORT);
}

static CMD_HANDLER( wd33c93_disconnect_cmd )
{
	/* complete the command */
	scsi_data.regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);
}

static CMD_HANDLER( wd33c93_select_cmd )
{
	UINT8	unit = wd33c93_getunit();
	UINT8	newstatus;

	/* see if we can select that device */
	if ( devices[unit] )
	{
		/* device is available - signal selection done */
		newstatus = CSR_SELECT;

		/* determine the next bus phase depending on the command */
		if ( (scsi_data.regs[WD_COMMAND] & 0x7f) == WD_CMD_SEL_ATN )
		{
			/* /ATN asserted during select: Move to Message Out Phase to read identify */
			scsi_data.busphase = PHS_MESS_OUT;
		}
		else
		{
			/* No /ATN asserted: Move to Command Phase */
			scsi_data.busphase = PHS_COMMAND;
		}

		/* queue up a service request out in the future */
		machine.scheduler().timer_set( attotime::from_usec(50), FUNC(wd33c93_service_request ));
	}
	else
	{
		/* device is not available */
		newstatus = CSR_TIMEOUT;
	}

	/* complete the command */
	wd33c93_complete_cmd(newstatus);
}

static CMD_HANDLER( wd33c93_selectxfer_cmd )
{
	UINT8	unit = wd33c93_getunit();
	UINT8	newstatus;

	/* see if we can select that device */
	if ( devices[unit] )
	{
		if ( scsi_data.regs[WD_COMMAND_PHASE] < 0x45 )
		{
			/* device is available */
			int xfercount;
			int phase;

			/* do the request */
			devices[unit]->SetCommand( &scsi_data.regs[WD_CDB_1], 12 );
			devices[unit]->ExecCommand( &xfercount );
			devices[unit]->GetPhase( &phase );

			/* set transfer count */
			if ( wd33c93_get_xfer_count() > TEMP_INPUT_LEN )
			{
				logerror( "WD33C93: Transfer count too big. Please increase TEMP_INPUT_LEN (size=%d)\n", wd33c93_get_xfer_count() );
				wd33c93_set_xfer_count( TEMP_INPUT_LEN );
			}

			switch( phase )
			{
				case SCSI_PHASE_DATAIN:
					scsi_data.read_pending = 1;
					break;
			}
		}

		if ( scsi_data.read_pending )
		{
			int		len = TEMP_INPUT_LEN;

			if ( wd33c93_get_xfer_count() < len ) len = wd33c93_get_xfer_count();

			memset( &scsi_data.temp_input[0], 0, TEMP_INPUT_LEN );
			wd33c93_read_data( len, &scsi_data.temp_input[0] );
			scsi_data.temp_input_pos = 0;
			scsi_data.read_pending = 0;
		}

		scsi_data.regs[WD_TARGET_LUN] = 0;
		scsi_data.regs[WD_CONTROL] |= CTRL_EDI;
		scsi_data.regs[WD_COMMAND_PHASE] = 0x60;

		/* signal transfer ready */
		newstatus = CSR_SEL_XFER_DONE;

		/* if allowed disconnect, queue a service request */
		if ( scsi_data.identify & 0x40 )
		{
			/* queue disconnect message in */
			scsi_data.busphase = PHS_MESS_IN;

			/* queue up a service request out in the future */
			machine.scheduler().timer_set( attotime::from_msec(50), FUNC(wd33c93_service_request ));
		}
	}
	else
	{
		/* device is not available */
		newstatus = CSR_TIMEOUT;

		wd33c93_set_xfer_count( 0 );
	}

	/* complete the command */
	wd33c93_complete_cmd(newstatus);
}

static CMD_HANDLER( wd33c93_negate_ack )
{
	logerror( "WD33C93: ACK Negated\n" );

	/* complete the command */
	scsi_data.regs[WD_AUXILIARY_STATUS] &= ~(ASR_CIP | ASR_BSY);
}

static CMD_HANDLER( wd33c93_xferinfo_cmd )
{
	/* make the buffer available right away */
	scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_DBR;
	scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_CIP;

	/* the command will be completed once the data is transferred */
	machine.scheduler().timer_set( attotime::from_msec(1), FUNC(wd33c93_deassert_cip ));
}

/* Command handlers */
static const cmd_handler wd33c93_cmds[0x22] =
{
	&wd33c93_reset_cmd,		/* 0x00 - WD_CMD_RESET */
	&wd33c93_abort_cmd,		/* 0x01 - WD_CMD_ABORT */
	&wd33c93_invalid_cmd,	/* 0x02 - WD_CMD_ASSERT_ATN (uninmplemented) */
	&wd33c93_negate_ack,	/* 0x03 - WD_CMD_NEGATE_ACK */
	&wd33c93_disconnect_cmd,/* 0x04 - WD_CMD_DISCONNECT */
	&wd33c93_invalid_cmd,	/* 0x05 - WD_CMD_RESELECT (uninmplemented) */
	&wd33c93_select_cmd,	/* 0x06 - WD_CMD_SEL_ATN */
	&wd33c93_select_cmd,	/* 0x07 - WD_CMD_SEL */
	&wd33c93_selectxfer_cmd,/* 0x08 - WD_CMD_SEL_ATN_XFER */
	&wd33c93_selectxfer_cmd,/* 0x09 - WD_CMD_SEL_XFER */
	&wd33c93_invalid_cmd,	/* 0x0a - WD_CMD_RESEL_RECEIVE (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x0b - WD_CMD_RESEL_SEND (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x0c - WD_CMD_WAIT_SEL_RECEIVE (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x0d - WD_CMD_SSCC (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x0e - WD_CMD_SND_DISC (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x0f - WD_CMD_SET_IDI (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x10 - WD_CMD_RCV_CMD (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x11 - WD_CMD_RCV_DATA (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x12 - WD_CMD_RCV_MSG_OUT (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x13 - WD_CMD_RCV (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x14 - WD_CMD_SND_STATUS (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x15 - WD_CMD_SND_DATA (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x16 - WD_CMD_SND_MSG_IN (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x17 - WD_CMD_SND (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x18 - WD_CMD_TRANS_ADDR (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x19 - WD_CMD_XFER_PAD (uninmplemented) */
	&wd33c93_invalid_cmd,	/* 0x1a - invalid */
	&wd33c93_invalid_cmd,	/* 0x1b - invalid */
	&wd33c93_invalid_cmd,	/* 0x1c - invalid */
	&wd33c93_invalid_cmd,	/* 0x1d - invalid */
	&wd33c93_invalid_cmd,	/* 0x1e - invalid */
	&wd33c93_invalid_cmd,	/* 0x1f - invalid */
	&wd33c93_xferinfo_cmd,	/* 0x20 - WD_CMD_TRANS_INFO) */
	&wd33c93_invalid_cmd	/* 0x21 - WD_CMD_TRANSFER_PAD (uninmplemented) */
};

/* Handle pending commands */
static void wd33c93_command( running_machine &machine )
{
	/* get the command */
	UINT8 cmd = scsi_data.regs[WD_COMMAND];

	/* check if its within valid bounds */
	if ( (cmd & 0x7F) > WD_CMD_TRANSFER_PAD )
	{
		wd33c93_invalid_cmd(machine);
		return;
	}

	/* call the command handler */
	(*wd33c93_cmds[cmd & 0x7F])(machine);
}

WRITE8_HANDLER(wd33c93_w)
{
	switch( offset )
	{
		case 0:
		{
			/* update register select */
			scsi_data.sasr = data & 0x1f;
		}
		break;

		case 1:
		{
			LOG(( "WD33C93: PC=%08x - Write REG=%02x, data = %02x\n", cpu_get_pc(&space->device()), scsi_data.sasr, data ));

			/* update the register */
			scsi_data.regs[scsi_data.sasr] = data;

			/* if we receive a command, schedule to process it */
			if ( scsi_data.sasr == WD_COMMAND )
			{
				LOG(( "WDC33C93: PC=%08x - Executing command %08x - unit %d\n", cpu_get_pc(&space->device()), data, wd33c93_getunit() ));

				/* signal we're processing it */
				scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_CIP;

				/* process the command */
				wd33c93_command(space->machine());
			}
			else if ( scsi_data.sasr == WD_CDB_1 )
			{
				scsi_data.regs[WD_COMMAND_PHASE] = 0;
			}
			else if ( scsi_data.sasr == WD_DATA )
			{
				/* if data was written, and we have a count, send to device */
				int		count = wd33c93_get_xfer_count();

				if ( scsi_data.regs[WD_COMMAND] & 0x80 )
					count = 1;

				if ( count-- > 0 )
				{
					/* write to FIFO */
					if ( scsi_data.fifo_pos < FIFO_SIZE )
					{
						scsi_data.fifo[scsi_data.fifo_pos++] = data;
					}

					/* update count */
					wd33c93_set_xfer_count( count );

					/* if we're done with the write, see where we're at */
					if ( count == 0 )
					{
						scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_INT;
						scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;

						switch( scsi_data.busphase )
						{
							case PHS_MESS_OUT:
							{
								/* reset fifo */
								scsi_data.fifo_pos = 0;

								/* Message out phase. Data is probably SCSI Identify. Move to command phase. */
								scsi_data.busphase = PHS_COMMAND;

								scsi_data.identify = scsi_data.fifo[0];
							}
							break;

							case PHS_COMMAND:
							{
								UINT8	unit = wd33c93_getunit();
								int		xfercount;
								int phase;

								/* Execute the command. Depending on the command, we'll move to data in or out */
								devices[unit]->SetCommand( &scsi_data.fifo[0], 12 );
								devices[unit]->ExecCommand( &xfercount );
								devices[unit]->GetPhase( &phase );

								/* reset fifo */
								scsi_data.fifo_pos = 0;

								/* set the new count */
								wd33c93_set_xfer_count( xfercount );

								switch( phase )
								{
								case SCSI_PHASE_STATUS:
									scsi_data.busphase = PHS_STATUS;
									break;

								case SCSI_PHASE_DATAIN:
									scsi_data.busphase = PHS_DATA_IN;
									scsi_data.read_pending = 1;
									break;

								case SCSI_PHASE_DATAOUT:
									scsi_data.busphase = PHS_DATA_OUT;
									break;
								}
							}
							break;

							case PHS_DATA_OUT:
							{
								/* write data out to device */
								wd33c93_write_data( scsi_data.fifo_pos, scsi_data.fifo );

								/* reset fifo */
								scsi_data.fifo_pos = 0;

								/* move to status phase */
								scsi_data.busphase = PHS_STATUS;
							}
							break;
						}

						/* complete the command */
						wd33c93_complete_immediate(space->machine(), CSR_XFER_DONE | scsi_data.busphase);
					}
				}
				else
				{
					logerror( "WD33C93: Sending data to device with transfer count = 0!. Ignoring...\n" );
				}
			}

			/* auto-increment register select if not on special registers */
			if ( scsi_data.sasr != WD_COMMAND && scsi_data.sasr != WD_DATA && scsi_data.sasr != WD_AUXILIARY_STATUS )
			{
				scsi_data.sasr = ( scsi_data.sasr + 1 ) & 0x1f;
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

READ8_HANDLER(wd33c93_r)
{
	switch( offset )
	{
		case 0:
		{
			/* read aux status */
			return scsi_data.regs[WD_AUXILIARY_STATUS];
		}
		break;

		case 1:
		{
			UINT8	ret;

			/* if reading status, clear irq flag */
			if ( scsi_data.sasr == WD_SCSI_STATUS )
			{
				scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_INT;

				if (intf && intf->irq_callback)
				{
					intf->irq_callback(space->machine(), 0);
				}

				LOG(( "WD33C93: PC=%08x - Status read (%02x)\n", cpu_get_pc(&space->device()), scsi_data.regs[WD_SCSI_STATUS] ));
			}
			else if ( scsi_data.sasr == WD_DATA )
			{
				/* we're going to be doing synchronous reads */

				/* get the transfer count */
				int		count = wd33c93_get_xfer_count();

				/* initialize the return value */
				scsi_data.regs[WD_DATA] = 0;

				if ( count <= 0 && scsi_data.busphase == PHS_MESS_IN )
				{
					/* move to disconnect */
					wd33c93_complete_cmd(CSR_DISC);
				}
				else if ( count == 1 && scsi_data.busphase == PHS_STATUS )
				{
					/* update the count */
					wd33c93_set_xfer_count( 0 );

					/* move to message in phase */
					scsi_data.busphase = PHS_MESS_IN;

					/* complete the command */
					wd33c93_complete_cmd(CSR_XFER_DONE | scsi_data.busphase);
				}
				else if ( count-- > 0 ) /* make sure we still have data to send */
				{
					if ( scsi_data.read_pending )
					{
						int		len = TEMP_INPUT_LEN;

						if ( (count+1) < len ) len = count+1;
						wd33c93_read_data( len, &scsi_data.temp_input[0] );
						scsi_data.temp_input_pos = 0;
						scsi_data.read_pending = 0;
					}

					scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_INT;

					/* read in one byte */
					if ( scsi_data.temp_input_pos < TEMP_INPUT_LEN )
						scsi_data.regs[WD_DATA] = scsi_data.temp_input[scsi_data.temp_input_pos++];

					/* update the count */
					wd33c93_set_xfer_count( count );

					/* transfer finished, see where we're at */
					if ( count == 0 )
					{
						if ( scsi_data.regs[WD_COMMAND_PHASE] != 0x60 )
						{
							/* move to status phase */
							scsi_data.busphase = PHS_STATUS;

							/* complete the command */
							wd33c93_complete_cmd(CSR_XFER_DONE | scsi_data.busphase);
						}
						else
						{
							scsi_data.regs[WD_AUXILIARY_STATUS] |= ASR_INT;
							scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
						}
					}
				}
			}

			LOG(( "WD33C93: PC=%08x - Data read (%02x)\n", cpu_get_pc(&space->device()), scsi_data.regs[WD_DATA] ));

			/* get the register value */
			ret = scsi_data.regs[scsi_data.sasr];

			/* auto-increment register select if not on special registers */
			if ( scsi_data.sasr != WD_COMMAND && scsi_data.sasr != WD_DATA && scsi_data.sasr != WD_AUXILIARY_STATUS )
			{
				scsi_data.sasr = ( scsi_data.sasr + 1 ) & 0x1f;
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

void wd33c93_init( running_machine &machine, const struct WD33C93interface *interface )
{
	int i;

	// save interface pointer for later
	intf = interface;

	memset(&scsi_data, 0, sizeof(scsi_data));
	memset(devices, 0, sizeof(devices));

	// try to open the devices
	for (i = 0; i < interface->scsidevs->devs_present; i++)
	{
		scsidev_device *device = machine.device<scsidev_device>( interface->scsidevs->devices[i].tag );
		devices[device->GetDeviceID()] = device;
	}

	/* allocate a timer for commands */
	scsi_data.cmd_timer = machine.scheduler().timer_alloc(FUNC(wd33c93_complete_cb));

	scsi_data.temp_input = auto_alloc_array( machine, UINT8, TEMP_INPUT_LEN );

//  state_save_register_item_array(machine, "wd33c93", NULL, 0, scsi_data);
}

void wd33c93_get_dma_data( int bytes, UINT8 *pData )
{
	int	len = bytes;

	if ( len >= wd33c93_get_xfer_count() )
		len = wd33c93_get_xfer_count();

	if ( len == 0 )
		return;

	if ( (scsi_data.temp_input_pos+len) >= TEMP_INPUT_LEN )
	{
		logerror( "Reading past end of buffer, increase TEMP_INPUT_LEN size\n" );
		len = TEMP_INPUT_LEN - len;
	}

	assert(len);

	memcpy( pData, &scsi_data.temp_input[scsi_data.temp_input_pos], len );

	scsi_data.temp_input_pos += len;
	len = wd33c93_get_xfer_count() - len;
	wd33c93_set_xfer_count(len);
}

void wd33c93_write_data(int bytes, UINT8 *pData)
{
	UINT8	unit = wd33c93_getunit();

	if (devices[unit])
	{
		devices[unit]->WriteData( pData, bytes );
	}
	else
	{
		logerror("wd33c93: request for unknown device SCSI ID %d\n", unit);
	}
}

void wd33c93_clear_dma(void)
{
	/* indicate DMA completed by clearing the transfer count */
	wd33c93_set_xfer_count(0);
	scsi_data.regs[WD_AUXILIARY_STATUS] &= ~ASR_DBR;
}

int wd33c93_get_dma_count(void)
{
	return wd33c93_get_xfer_count();
}
