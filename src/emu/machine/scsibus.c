/*
	SCSIBus.c

	Implementation of a raw SCSI/SASI bus for machines that don't use a SCSI
	controler chip such as the RM Nimbus, which implements it as a bunch of
	74LS series chips.

*/

#include "emu.h"
#include "machine/scsidev.h"
#include "machine/scsibus.h"
#include "debugger.h"
#include "debug/debugcpu.h"
#include "debug/debugcon.h"

/*
    LOGLEVEL
        0   no logging,
        1   just commands
        2   1 + data
        3   2 + line changes
*/

#define LOGLEVEL            0

#define LOG(level,...)      if(LOGLEVEL>=level) logerror(__VA_ARGS__)

static const char *const linenames[] =
{
	"select", "busy", "request", "acknoledge", "C/D", "I/O", "message", "reset"
};

static const char *const phasenames[] =
{
	"data out", "data in", "command", "status", "none", "none", "message out", "message in", "bus free","select"
};

void scsibus_device::dump_bytes(UINT8 *buff, int count)
{
	int byteno;

	for(byteno=0; byteno<count; byteno++)
	{
		logerror("%02X ",buff[byteno]);
	}
}

void scsibus_device::dump_command_bytes()
{
	logerror("sending command 0x%02X to ScsiID %d\n",command[0],last_id);
	dump_bytes(command,cmd_idx);
	logerror("\n\n");
}

void scsibus_device::dump_data_bytes(int count)
{
	logerror("Data buffer[0..%d]\n",count);
	dump_bytes(buffer,count);
	logerror("\n\n");
}

void scsibus_device::scsibus_read_data()
{
	data_last = (bytes_left >= sectorbytes) ? sectorbytes : bytes_left;

	LOG(2,"SCSIBUS:scsibus_read_data bytes_left=%04X, data_last=%04X, xfer_count=%04X\n",bytes_left,data_last,xfer_count);

	if (data_last > 0)
	{
		devices[last_id]->ReadData(buffer, data_last);
		bytes_left-=data_last;
		data_idx=0;
	}
}

void scsibus_device::scsibus_write_data()
{
	if(bytes_left >= sectorbytes)
	{
		devices[last_id]->WriteData(buffer, sectorbytes);

		bytes_left-=sectorbytes;
		data_idx=0;
	}
}

/* SCSI Bus read/write */

UINT8 scsibus_device::scsi_data_r()
{
	UINT8 result = 0;

	switch (phase)
	{
		case SCSI_PHASE_DATAIN:
			result=buffer[data_idx++];

			// check to see if we have reached the end of the block buffer
			// and that there is more data to read from the scsi disk
			if((data_idx==sectorbytes) && (bytes_left>0) && IS_READ_COMMAND())
			{
				scsibus_read_data();
			}
			break;

		case SCSI_PHASE_STATUS:
			result=status; // return command status
			break;

		case SCSI_PHASE_MESSAGE_IN:
			result=0; // no errors for the time being !
			break;
	}

	LOG(2,"scsi_data_r : %02x phase=%s, data_idx=%d, cmd_idx=%d\n",result,phasenames[phase],data_idx,cmd_idx);
	return result;
}

READ8_MEMBER( scsibus_device::scsi_data_r )
{
	return scsi_data_r();
}

void scsibus_device::scsi_data_w( UINT8 data )
{
	switch (phase)
	{
		// Note this assumes we only have one initiator and therefore
		// only one line active.
		case SCSI_PHASE_BUS_FREE:
			last_id=scsibus_driveno(data);
			break;

		case SCSI_PHASE_COMMAND:
			command[cmd_idx++]=data;
			break;

		case SCSI_PHASE_DATAOUT:

			//LOG(1,"SCSIBUS:xfer_count=%02X, bytes_left=%02X data_idx=%02X\n",xfer_count,bytes_left,data_idx);

			if(IS_COMMAND(SCSI_CMD_FORMAT_UNIT))
			{
				// Only store the first 4 bytes of the bad block list (the header)
				//if(data_idx<4)
					buffer[data_idx++]=data;
					dump_data_bytes(4);
				//else
				//   data_idx++;

				// If we have the first byte, then cancel the dataout timout
				if(data_idx==1)
					dataout_timer->adjust(attotime::never);

				// When we have the first 3 bytes, calculate how many more are in the
				// bad block list.
				if(data_idx==3)
				{
					xfer_count+=((buffer[2]<<8)+buffer[3]);
					data_last=xfer_count;
					LOG(1,"format_unit reading an extra %d bytes\n",xfer_count-4);
					dump_data_bytes(4);
				}
			}
			else
			{
				buffer[data_idx++]=data;
			}

			// If the data buffer is full, and we are writing blocks flush it to the SCSI disk
			if((data_idx == sectorbytes) && IS_WRITE_COMMAND())
				scsibus_write_data();
			break;
	}
}

WRITE8_MEMBER( scsibus_device::scsi_data_w )
{
	scsi_data_w( data );
}

/* Get/Set lines */

UINT8 scsibus_device::get_scsi_line(UINT8 lineno)
{
	UINT8 result=0;

	switch (lineno)
	{
		case SCSI_LINE_SEL:   result=(linestate & (1<<SCSI_LINE_SEL)) >> SCSI_LINE_SEL; break;
		case SCSI_LINE_BSY:   result=(linestate & (1<<SCSI_LINE_BSY)) >> SCSI_LINE_BSY; break;
		case SCSI_LINE_REQ:   result=(linestate & (1<<SCSI_LINE_REQ)) >> SCSI_LINE_REQ; break;
		case SCSI_LINE_ACK:   result=(linestate & (1<<SCSI_LINE_ACK)) >> SCSI_LINE_ACK; break;
		case SCSI_LINE_CD:    result=(linestate & (1<<SCSI_LINE_CD )) >> SCSI_LINE_CD; break;
		case SCSI_LINE_IO:    result=(linestate & (1<<SCSI_LINE_IO )) >> SCSI_LINE_IO; break;
		case SCSI_LINE_MSG:   result=(linestate & (1<<SCSI_LINE_MSG)) >> SCSI_LINE_MSG; break;
		case SCSI_LINE_RESET: result=(linestate & (1<<SCSI_LINE_RESET)) >> SCSI_LINE_RESET; break;
	}

	LOG(3,"get_scsi_line(%s)=%d\n",linenames[lineno],result);

	return result;
}

READ_LINE_MEMBER( scsibus_device::scsi_bsy_r ) { return get_scsi_line(SCSI_LINE_BSY); }
READ_LINE_MEMBER( scsibus_device::scsi_sel_r ) { return get_scsi_line(SCSI_LINE_SEL); }
READ_LINE_MEMBER( scsibus_device::scsi_cd_r ) { return get_scsi_line(SCSI_LINE_CD); }
READ_LINE_MEMBER( scsibus_device::scsi_io_r ) { return get_scsi_line(SCSI_LINE_IO); }
READ_LINE_MEMBER( scsibus_device::scsi_msg_r ) { return get_scsi_line(SCSI_LINE_MSG); }
READ_LINE_MEMBER( scsibus_device::scsi_req_r ) { return get_scsi_line(SCSI_LINE_REQ); }
READ_LINE_MEMBER( scsibus_device::scsi_ack_r ) { return get_scsi_line(SCSI_LINE_ACK); }
READ_LINE_MEMBER( scsibus_device::scsi_rst_r ) { return get_scsi_line(SCSI_LINE_RESET); }

void scsibus_device::set_scsi_line(UINT8 line, UINT8 state)
{
	UINT8 changed;

	changed = ((linestate & (1<<line)) != (state << line));

	LOG(3,"set_scsi_line(%s,%d), changed=%d, linestate=%02X\n",linenames[line],state,changed,linestate);

	if(changed)
	{
		if (line==SCSI_LINE_ACK)
			set_scsi_line_ack(state);
		else
			set_scsi_line_now(line,state);
	}
}

WRITE_LINE_MEMBER( scsibus_device::scsi_bsy_w ) { set_scsi_line(SCSI_LINE_BSY, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_sel_w ) { set_scsi_line(SCSI_LINE_SEL, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_cd_w ) { set_scsi_line(SCSI_LINE_CD, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_io_w ) { set_scsi_line(SCSI_LINE_IO, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_msg_w ) { set_scsi_line(SCSI_LINE_MSG, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_req_w ) { set_scsi_line(SCSI_LINE_REQ, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_ack_w ) { set_scsi_line(SCSI_LINE_ACK, state); }
WRITE_LINE_MEMBER( scsibus_device::scsi_rst_w ) { set_scsi_line(SCSI_LINE_RESET, state); }

void scsibus_device::set_scsi_line_now(UINT8 line, UINT8 state)
{
	if(state)
		linestate |= (1<<line);
	else
		linestate &= ~(1<<line);

	scsi_in_line_changed(line,state);
}

void scsibus_device::set_scsi_line_ack(UINT8 state)
{
	ack_timer->adjust(attotime::from_nsec(ACK_DELAY_NS),state);
}

void scsibus_device::scsibus_exec_command()
{
	int command_local = 0;
	int newphase;

	if(LOGLEVEL)
		dump_command_bytes();

	//is_linked=command[cmd_idx-1] & 0x01;
	is_linked=0;

	// Assume command will succeed, if not set sytatus below.
	if (command[0]!=SCSI_CMD_REQUEST_SENSE)
		SET_STATUS_SENSE(SCSI_STATUS_OK,SCSI_SENSE_NO_SENSE);

	// Check for locally executed commands, and if found execute them
	switch (command[0])
	{
		// Test ready
		case SCSI_CMD_TEST_READY:
			LOG(1,"SCSIBUS: test_ready\n");
			command_local=1;
			xfer_count=0;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Recalibrate drive
		case SCSI_CMD_RECALIBRATE:
			LOG(1,"SCSIBUS: Recalibrate drive\n");
			command_local=1;
			xfer_count=0;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Request sense, return previous error codes
		case SCSI_CMD_REQUEST_SENSE:
			LOG(1,"SCSIBUS: request_sense\n");
			command_local=1;
			xfer_count=SCSI_SENSE_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			buffer[0]=sense;
			buffer[1]=0x00;
			buffer[2]=0x00;
			buffer[3]=0x00;
			SET_STATUS_SENSE(SCSI_STATUS_OK,SCSI_SENSE_NO_SENSE);
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		// Format unit
		case SCSI_CMD_FORMAT_UNIT:
			LOG(1,"SCSIBUS: format unit command[1]=%02X & 0x10\n",(command[1] & 0x10));
			command_local=1;
			if((command[1] & 0x10)==0x10)
				devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			else
				devices[last_id]->SetPhase(SCSI_PHASE_STATUS);

			xfer_count=4;
			data_last=xfer_count;
			bytes_left=0;
			dataout_timer->adjust(attotime::from_seconds(FORMAT_UNIT_TIMEOUT));
			break;

		// Check track format Xebec
		case SCSI_CMD_CHECK_TRACK_FORMAT:
			LOG(1,"SCSIBUS: check track format\n");
			command_local=1;
			xfer_count=0;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Setup drive parameters Xebec
		case SCSI_CMD_INIT_DRIVE_PARAMS:
			LOG(1,"SCSIBUS: init_drive_params: Xebec S1410\n");
			command_local=1;
			xfer_count=XEBEC_PARAMS_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		// Format bad track Xebec
		case SCSI_CMD_FORMAT_ALT_TRACK:
			LOG(1,"SCSIBUS: format_alt_track: Xebec S1410\n");
			command_local=1;
			xfer_count=XEBEC_ALT_TRACK_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		// Write buffer Xebec S1410 specific
		case SCSI_CMD_WRITE_SEC_BUFFER:
			LOG(1,"SCSIBUS: write_sector_buffer: Xebec S1410\n");
			command_local=1;
			xfer_count=XEBEC_SECTOR_BUFFER_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		// Read buffer Xebec S1410 specific
		case SCSI_CMD_READ_SEC_BUFFER:
			LOG(1,"SCSIBUS: read_sector_buffer: Xebec S1410\n");
			command_local=1;
			xfer_count=XEBEC_SECTOR_BUFFER_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAIN);
			break;

		// Write buffer, Adaptec ACB40x0 specific
		case SCSI_CMD_WRITE_DATA_BUFFER:
			LOG(1,"SCSIBUS: write_buffer: Adaptec ACB40x0\n");
			command_local=1;
			xfer_count=ADAPTEC_DATA_BUFFER_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		// Read buffer, Adaptec ACB40x0 specific
		case SCSI_CMD_READ_DATA_BUFFER:
			LOG(1,"SCSIBUS: read_data_buffer: Adaptec ACB40x0\n");
			command_local=1;
			xfer_count=ADAPTEC_DATA_BUFFER_SIZE;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAIN);
			break;

		// Send diagnostic info
		case SCSI_CMD_SEND_DIAGNOSTIC:
			LOG(1,"SCSIBUS: send_diagnostic\n");
			command_local=1;
			xfer_count=(command[3]<<8)+command[4];
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		case SCSI_CMD_SEARCH_DATA_EQUAL:
			LOG(1,"SCSIBUS: Search_data_equal ACB40x0\n");
			command_local=1;
			xfer_count=0;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		case SCSI_CMD_READ_DEFECT:
			LOG(1,"SCSIBUS: read defect list\n");
			command_local=1;

			buffer[0] = 0x00;
			buffer[1] = command[2];
			buffer[3] = 0x00; // defect list len msb
			buffer[4] = 0x00; // defect list len lsb

			xfer_count=4;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAIN);
			break;

		// write buffer
		case SCSI_CMD_BUFFER_WRITE:
			LOG(1,"SCSIBUS: write_buffer\n");
			command_local=1;
			xfer_count=(command[7]<<8)+command[8];
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

		// read buffer
		case SCSI_CMD_BUFFER_READ:
			LOG(1,"SCSIBUS: read_buffer\n");
			command_local=1;
			xfer_count=(command[7]<<8)+command[8];
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_DATAIN);
			break;

		// Xebec S1410
		case SCSI_CMD_RAM_DIAGS:
		case SCSI_CMD_DRIVE_DIAGS:
		case SCSI_CMD_CONTROLER_DIAGS:
			LOG(1,"SCSIBUS: Xebec RAM, disk or Controler diags [%02X]\n",command[0]);
			command_local=1;
			xfer_count=0;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Commodore D9060/9090
		case SCSI_CMD_PHYSICAL_DEVICE_ID:
			LOG(1,"SCSIBUS: physical device ID\n");
			command_local=1;
			xfer_count=0;
			data_last=xfer_count;
			bytes_left=0;
			devices[last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;
	}


	// Check for locally executed command, if not then pass it on
	// to the disk driver
	if(!command_local)
	{
		devices[last_id]->SetCommand(command, cmd_idx);
		devices[last_id]->ExecCommand(&xfer_count);
		bytes_left=xfer_count;
		data_last=xfer_count;
		data_idx=0;
	}

	devices[last_id]->GetPhase(&newphase);

	scsi_change_phase(newphase);

	LOG(1,"SCSIBUS:xfer_count=%02X, bytes_left=%02X data_idx=%02X\n",xfer_count,bytes_left,data_idx);

	// This is correct as we need to read from disk for commands other than just read data
	if ((phase == SCSI_PHASE_DATAIN) && (!command_local))
		scsibus_read_data();
}

int scsibus_device::datain_done()
{
	int result=0;

	// Read data commands
	if(IS_READ_COMMAND() && (data_idx == data_last) && (bytes_left == 0))
		result=1;
	else if (data_idx==data_last)
		result=1;

	return result;
}

int scsibus_device::dataout_done()
{
	int result=0;

	// Write data commands
	if(IS_WRITE_COMMAND() && (data_idx == 0) && (bytes_left == 0))
		result=1;
	else if (data_idx==data_last)
		result=1;

	return result;
}

void scsibus_device::check_process_dataout()
{
	int capacity=0;
	int tracks;
	adaptec_sense_t *sense;

	LOG(1,"SCSIBUS:check_process_dataout cmd=%02X\n",command[0]);

	switch (command[0])
	{
		case SCSI_CMD_INIT_DRIVE_PARAMS:
			tracks=((buffer[0]<<8)+buffer[1]);
			capacity=(tracks * buffer[2]) * 17;
			LOG(1,"Tracks=%d, Heads=%d\n",tracks,buffer[2]);
			LOG(1,"Setting disk capacity to %d blocks\n",capacity);
			//debugger_break(device->machine());
			break;

		case SCSI_CMD_MODE_SELECT:
			sense=(adaptec_sense_t *)buffer;
			tracks=(sense->cylinder_count[0]<<8)+sense->cylinder_count[1];
			capacity=(tracks * sense->head_count * 17);
			LOG(1,"Tracks=%d, Heads=%d sec/track=%d\n",tracks,sense->head_count,sense->sectors_per_track);
			LOG(1,"Setting disk capacity to %d blocks\n",capacity);
			dump_data_bytes(0x16);
			//debugger_break(device->machine());
			break;
	}
}


void scsibus_device::scsi_in_line_changed(UINT8 line, UINT8 state)
{
	void *hdfile;

	// Reset aborts and returns to bus free
	if((line==SCSI_LINE_RESET) && (state==0))
	{
		scsi_change_phase(SCSI_PHASE_BUS_FREE);
		cmd_idx=0;
		data_idx=0;
		is_linked=0;

		return;
	}

	switch (phase)
	{
		case SCSI_PHASE_BUS_FREE:
			if((line==SCSI_LINE_SEL) && (devices[last_id]!=NULL))
			{
				// Check to see if device had image file mounted, if not, do not set busy,
				// and stay busfree.
				devices[last_id]->GetDevice(&hdfile);
				if(hdfile!=(void *)NULL)
				{
					if(state==0)
						sel_timer->adjust(attotime::from_nsec(BSY_DELAY_NS));
					else
						scsi_change_phase(SCSI_PHASE_COMMAND);
				}
			}
			break;

		case SCSI_PHASE_COMMAND:
			if(line==SCSI_LINE_ACK)
			{
				if(state)
				{
					// If the command is ready go and execute it
					if(cmd_idx==get_scsi_cmd_len(command[0]))
					{
						scsibus_exec_command();
					}
					else
						scsi_out_line_change(SCSI_LINE_REQ,0);
				}
				else
					scsi_out_line_change(SCSI_LINE_REQ,1);
			}
			break;

		case SCSI_PHASE_DATAIN:
			if(line==SCSI_LINE_ACK)
			{
				if(state)
				{
					if(datain_done())
						scsi_change_phase(SCSI_PHASE_STATUS);
					else
						scsi_out_line_change(SCSI_LINE_REQ,0);
				}
				else
					scsi_out_line_change(SCSI_LINE_REQ,1);
			}
			break;

		case SCSI_PHASE_DATAOUT:
			if(line==SCSI_LINE_ACK)
			{
				if(state)
				{
					if(dataout_done())
					{
						check_process_dataout();
						scsi_change_phase(SCSI_PHASE_STATUS);
					}
					else
						scsi_out_line_change(SCSI_LINE_REQ,0);
				}
				else
					scsi_out_line_change(SCSI_LINE_REQ,1);
			}
			break;

		case SCSI_PHASE_STATUS:
			if(line==SCSI_LINE_ACK)
			{
				if(state)
				{
					if(cmd_idx > 0)
					{
						scsi_change_phase(SCSI_PHASE_MESSAGE_IN);
					}
					else
						scsi_out_line_change(SCSI_LINE_REQ,0);
				}
				else
				{
					cmd_idx++;
					scsi_out_line_change(SCSI_LINE_REQ,1);
				}
			}
			break;

		case SCSI_PHASE_MESSAGE_IN:
			if(line==SCSI_LINE_ACK)
			{
				if(state)
				{
					if(cmd_idx > 0)
					{
						if(is_linked)
							scsi_change_phase(SCSI_PHASE_COMMAND);
						else
							scsi_change_phase(SCSI_PHASE_BUS_FREE);
					}
					else
						scsi_out_line_change(SCSI_LINE_REQ,0);
				}
				else
				{
					cmd_idx++;
					scsi_out_line_change(SCSI_LINE_REQ,1);
				}
			}
			break;
	}
}

void scsibus_device::scsi_out_line_change(UINT8 line, UINT8 state)
{
	if(line==SCSI_LINE_REQ)
		scsi_out_line_req(state);
	else
		scsi_out_line_change_now(line,state);
}

void scsibus_device::scsi_out_line_change_now(UINT8 line, UINT8 state)
{
	if(state)
		linestate |= (1<<line);
	else
		linestate &= ~(1<<line);

	LOG(3,"scsi_out_line_change(%s,%d)\n",linenames[line],state);

	if(line_change_cb!=NULL)
		line_change_cb(this, line,state);

	switch (line)
	{
	case SCSI_LINE_BSY: out_bsy_func(state); break;
	case SCSI_LINE_SEL: out_sel_func(state); break;
	case SCSI_LINE_CD: out_cd_func(state); break;
	case SCSI_LINE_IO: out_io_func(state); break;
	case SCSI_LINE_MSG: out_msg_func(state); break;
	case SCSI_LINE_REQ: out_req_func(state); break;
	case SCSI_LINE_RESET: out_rst_func(state); break;
	}
}

void scsibus_device::scsi_out_line_req(UINT8 state)
{
	req_timer->adjust(attotime::from_nsec(REQ_DELAY_NS),state);
}

void scsibus_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch( tid )
	{
	case 0:
		scsi_out_line_change_now(SCSI_LINE_REQ, param);
		break;

	case 1:
		set_scsi_line_now(SCSI_LINE_ACK, param);
		break;

	case 2:
		scsi_out_line_change_now(SCSI_LINE_BSY, param);
		break;

	case 3:
		// Some drives, notably the ST225N and ST125N, accept fromat unit commands
		// with flags set indicating that bad block data should be transfered but
		// don't then implemnt a data in phase, this timeout it to catch these !
		if(IS_COMMAND(SCSI_CMD_FORMAT_UNIT) && (data_idx==0))
			scsi_change_phase(SCSI_PHASE_STATUS);
		break;
	}
}

void scsibus_device::scsi_change_phase(UINT8 newphase)
{
	LOG(1,"scsi_change_phase() from=%s, to=%s\n",phasenames[phase],phasenames[newphase]);

	phase=newphase;
	cmd_idx=0;
	data_idx=0;

	switch(phase)
	{
		case SCSI_PHASE_BUS_FREE:
			scsi_out_line_change(SCSI_LINE_CD,1);
			scsi_out_line_change(SCSI_LINE_IO,1);
			scsi_out_line_change(SCSI_LINE_MSG,1);
			scsi_out_line_change(SCSI_LINE_REQ,1);
			scsi_out_line_change(SCSI_LINE_BSY,1);
			LOG(1,"SCSIBUS: done\n\n");
			//if (IS_COMMAND(SCSI_CMD_READ_CAPACITY))
			//  debugger_break(device->machine());
			break;

		case SCSI_PHASE_COMMAND:
			scsi_out_line_change(SCSI_LINE_CD,0);
			scsi_out_line_change(SCSI_LINE_IO,1);
			scsi_out_line_change(SCSI_LINE_MSG,1);
			scsi_out_line_change(SCSI_LINE_REQ,0);
			LOG(1,"\nSCSIBUS: Command begin\n");
			break;

		case SCSI_PHASE_DATAOUT:
			scsi_out_line_change(SCSI_LINE_CD,1);
			scsi_out_line_change(SCSI_LINE_IO,1);
			scsi_out_line_change(SCSI_LINE_MSG,1);
			scsi_out_line_change(SCSI_LINE_REQ,0);
			break;

		case SCSI_PHASE_DATAIN:
			scsi_out_line_change(SCSI_LINE_CD,1);
			scsi_out_line_change(SCSI_LINE_IO,0);
			scsi_out_line_change(SCSI_LINE_MSG,1);
			scsi_out_line_change(SCSI_LINE_REQ,0);
			break;

		case SCSI_PHASE_STATUS:
			scsi_out_line_change(SCSI_LINE_CD,0);
			scsi_out_line_change(SCSI_LINE_IO,0);
			scsi_out_line_change(SCSI_LINE_MSG,1);
			scsi_out_line_change(SCSI_LINE_REQ,0);
			break;

		case SCSI_PHASE_MESSAGE_OUT:
			scsi_out_line_change(SCSI_LINE_CD,0);
			scsi_out_line_change(SCSI_LINE_IO,1);
			scsi_out_line_change(SCSI_LINE_MSG,0);
			scsi_out_line_change(SCSI_LINE_REQ,0);
			break;

		case SCSI_PHASE_MESSAGE_IN:
			scsi_out_line_change(SCSI_LINE_CD,0);
			scsi_out_line_change(SCSI_LINE_IO,0);
			scsi_out_line_change(SCSI_LINE_MSG,0);
			scsi_out_line_change(SCSI_LINE_REQ,0);
			break;
	}
}

UINT8 scsibus_device::scsibus_driveno(UINT8 drivesel)
{
	switch (drivesel)
	{
		case 0x01: return 0;
		case 0x02: return 1;
		case 0x04: return 2;
		case 0x08: return 3;
		case 0x10: return 4;
		case 0x20: return 5;
		case 0x40: return 6;
		case 0x80: return 7;
		default: return 0;
	}
}

// get the length of a SCSI command based on it's command byte type
int scsibus_device::get_scsi_cmd_len(int cbyte)
{
	int group;

	group = (cbyte>>5) & 7;

	if (group == 0 || group == 3 || group == 6 || group == 7) return 6;
	if (group == 1 || group == 2) return 10;
	if (group == 5) return 12;

	fatalerror("scsibus: Unknown SCSI command group %d, command byte=%02X", group,cbyte);

	return 6;
}

void scsibus_device::init_scsibus(int _sectorbytes)
{
	sectorbytes = _sectorbytes;
}

scsibus_device::scsibus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SCSIBUS, "SCSI bus", tag, owner, clock)
{
}

void scsibus_device::device_config_complete()
{
	// inherit a copy of the static data
	const SCSIBus_interface *intf = reinterpret_cast<const SCSIBus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<SCSIBus_interface *>(this) = *intf;
	}
}

void scsibus_device::device_start()
{
	memset(devices, 0, sizeof(devices));

	out_bsy_func.resolve(_out_bsy_func, *this);
	out_sel_func.resolve(_out_sel_func, *this);
	out_cd_func.resolve(_out_cd_func, *this);
	out_io_func.resolve(_out_io_func, *this);
	out_msg_func.resolve(_out_msg_func, *this);
	out_req_func.resolve(_out_req_func, *this);
	out_rst_func.resolve(_out_rst_func, *this);

	// All lines start high - inactive
	linestate=0xFF;

	// Start with bus free
	phase=SCSI_PHASE_BUS_FREE;

	// Setup req/ack/sel timers
	req_timer=timer_alloc(0);
	ack_timer=timer_alloc(1);
	sel_timer=timer_alloc(2);
	dataout_timer=timer_alloc(3);

	for( device_t *device = first_subdevice(); device != NULL; device = device->next() )
	{
		scsidev_device *scsidev = dynamic_cast<scsidev_device *>(device);
		if( scsidev != NULL )
		{
			devices[scsidev->GetDeviceID()] = scsidev;
		}
	}
}

const device_type SCSIBUS = &device_creator<scsibus_device>;
