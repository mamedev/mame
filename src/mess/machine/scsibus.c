/*
    SCSIBus.c

    Implementation of a raw SCSI/SASI bus for machines that don't use a SCSI
    controler chip such as the RM Nimbus, which implements it as a bunch of
    74LS series chips.

*/

#include "emu.h"
#include "machine/scsi.h"
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

#define LOGLEVEL			0
#define ENABLE_DATA_DUMP	1

#define LOG(level,...)      if(LOGLEVEL>=level) logerror(__VA_ARGS__)

typedef struct _scsibus_t scsibus_t;
struct _scsibus_t
{
	scsidev_device          *devices[8];
	const SCSIBus_interface *interface;

	devcb_resolved_write_line out_bsy_func;
	devcb_resolved_write_line out_sel_func;
	devcb_resolved_write_line out_cd_func;
	devcb_resolved_write_line out_io_func;
	devcb_resolved_write_line out_msg_func;
	devcb_resolved_write_line out_req_func;
	devcb_resolved_write_line out_rst_func;

	UINT8       linestate;
	UINT8       last_id;
	UINT8       phase;

	UINT8       command[CMD_BUF_SIZE];
	UINT8       cmd_idx;
	UINT8       is_linked;

	UINT8		status;
	UINT8		sense;

	UINT8       data[ADAPTEC_BUF_SIZE];
	UINT16      data_idx;
	int         xfer_count;
	int         bytes_left;
	int         data_last;
	int			sectorbytes;

	emu_timer   *req_timer;
	emu_timer   *ack_timer;
	emu_timer   *sel_timer;
	emu_timer   *dataout_timer;
};

static void set_scsi_line_now(device_t *device, UINT8 line, UINT8 state);
static void set_scsi_line_ack(device_t *device, UINT8 state);
static void scsi_in_line_changed(device_t *device, UINT8 line, UINT8 state);
static void scsi_out_line_change(device_t *device, UINT8 line, UINT8 state);
static void scsi_out_line_change_now(device_t *device, UINT8 line, UINT8 state);
static void scsi_out_line_req(device_t *device, UINT8 state);
static TIMER_CALLBACK(req_timer_callback);
static TIMER_CALLBACK(ack_timer_callback);
static TIMER_CALLBACK(sel_timer_callback);
static TIMER_CALLBACK(dataout_timer_callback);

static void scsi_change_phase(device_t *device, UINT8 newphase);

static const char *const linenames[] =
{
    "select", "busy", "request", "acknoledge", "C/D", "I/O", "message", "reset"
};

static const char *const phasenames[] =
{
    "data out", "data in", "command", "status", "none", "none", "message out", "message in", "bus free","select"
};

INLINE scsibus_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SCSIBUS);

	return (scsibus_t *) downcast<legacy_device_base *>(device)->token();
}

static void dump_bytes(UINT8 *buff, int count)
{
    int byteno;

    for(byteno=0; byteno<count; byteno++)
    {
        logerror("%02X ",buff[byteno]);
    }
}

static void dump_command_bytes(scsibus_t   *bus)
{

    logerror("sending command 0x%02X to ScsiID %d\n",bus->command[0],bus->last_id);
	dump_bytes(bus->command,bus->cmd_idx);
    logerror("\n\n");
}

#if ENABLE_DATA_DUMP
static void dump_data_bytes(scsibus_t   *bus, int count)
{

    logerror("Data buffer[0..%d]\n",count);
	dump_bytes(bus->data,count);
    logerror("\n\n");
}
#endif

static void scsibus_read_data(scsibus_t   *bus)
{
    bus->data_last = (bus->bytes_left >= bus->sectorbytes) ? bus->sectorbytes : bus->bytes_left;

    LOG(2,"SCSIBUS:scsibus_read_data bus->bytes_left=%04X, bus->data_last=%04X, bus->xfer_count=%04X\n",bus->bytes_left,bus->data_last,bus->xfer_count);

    if (bus->data_last > 0)
    {
        bus->devices[bus->last_id]->ReadData(bus->data, bus->data_last);
        bus->bytes_left-=bus->data_last;
        bus->data_idx=0;
    }
}

static void scsibus_write_data(scsibus_t   *bus)
{
    if(bus->bytes_left >= bus->sectorbytes)
    {
        bus->devices[bus->last_id]->WriteData(bus->data, bus->sectorbytes);

        bus->bytes_left-=bus->sectorbytes;
        bus->data_idx=0;
    }
}

/* SCSI Bus read/write */

UINT8 scsi_data_r(device_t *device)
{
    scsibus_t   *bus = get_token(device);
    UINT8       result = 0;

    switch (bus->phase)
    {
        case SCSI_PHASE_DATAIN :
            result=bus->data[bus->data_idx++];

            // check to see if we have reached the end of the block buffer
            // and that there is more data to read from the scsi disk
            if((bus->data_idx==bus->sectorbytes) && (bus->bytes_left>0) && IS_READ_COMMAND())
            {
                scsibus_read_data(bus);
            }
            break;

        case SCSI_PHASE_STATUS :
            result=bus->status;		// return command status
            break;

        case SCSI_PHASE_MESSAGE_IN :
            result=0;              // no errors for the time being !
            break;
    }

    LOG(2,"scsi_data_r : %02x phase=%s, data_idx=%d, cmd_idx=%d\n",result,phasenames[bus->phase],bus->data_idx,bus->cmd_idx);
    return result;
}

READ8_DEVICE_HANDLER( scsi_data_r )
{
	return scsi_data_r(device);
}

void scsi_data_w(device_t *device, UINT8 data)
{
    scsibus_t   *bus = get_token(device);

    switch (bus->phase)
    {
        // Note this assumes we only have one initiator and therefore
        // only one line active.
        case SCSI_PHASE_BUS_FREE :
            bus->last_id=scsibus_driveno(data);
            break;

        case SCSI_PHASE_COMMAND :
            bus->command[bus->cmd_idx++]=data;
            break;

        case SCSI_PHASE_DATAOUT :

            //LOG(1,"SCSIBUS:xfer_count=%02X, bytes_left=%02X data_idx=%02X\n",bus->xfer_count,bus->bytes_left,bus->data_idx);

            if(IS_COMMAND(SCSI_CMD_FORMAT_UNIT))
            {
                // Only store the first 4 bytes of the bad block list (the header)
                //if(bus->data_idx<4)
                    bus->data[bus->data_idx++]=data;
					dump_data_bytes(bus,4);
                //else
                //   bus->data_idx++;

				// If we have the first byte, then cancel the dataout timout
				if(bus->data_idx==1)
					bus->dataout_timer->adjust(attotime::never);

                // When we have the first 3 bytes, calculate how many more are in the
                // bad block list.
                if(bus->data_idx==3)
                {
                    bus->xfer_count+=((bus->data[2]<<8)+bus->data[3]);
                    bus->data_last=bus->xfer_count;
					LOG(1,"format_unit reading an extra %d bytes\n",bus->xfer_count-4);
					dump_data_bytes(bus,4);
                }
            }
			else
			{
				bus->data[bus->data_idx++]=data;
			}

            // If the data buffer is full, and we are writing blocks flush it to the SCSI disk
            if((bus->data_idx == bus->sectorbytes) && IS_WRITE_COMMAND())
                scsibus_write_data(bus);
            break;
    }
}

WRITE8_DEVICE_HANDLER( scsi_data_w )
{
	scsi_data_w(device, data);
}

/* Get/Set lines */

UINT8 get_scsi_lines(device_t *device)
{
    scsibus_t   *bus = get_token(device);

    return bus->linestate;
}

UINT8 get_scsi_line(device_t *device, UINT8 lineno)
{
    scsibus_t   *bus = get_token(device);
    UINT8       result=0;

    switch (lineno)
    {
        case SCSI_LINE_SEL      : result=(bus->linestate & (1<<SCSI_LINE_SEL)) >> SCSI_LINE_SEL; break;
        case SCSI_LINE_BSY      : result=(bus->linestate & (1<<SCSI_LINE_BSY)) >> SCSI_LINE_BSY; break;
        case SCSI_LINE_REQ      : result=(bus->linestate & (1<<SCSI_LINE_REQ)) >> SCSI_LINE_REQ; break;
        case SCSI_LINE_ACK      : result=(bus->linestate & (1<<SCSI_LINE_ACK)) >> SCSI_LINE_ACK; break;
        case SCSI_LINE_CD       : result=(bus->linestate & (1<<SCSI_LINE_CD )) >> SCSI_LINE_CD; break;
        case SCSI_LINE_IO       : result=(bus->linestate & (1<<SCSI_LINE_IO )) >> SCSI_LINE_IO; break;
        case SCSI_LINE_MSG      : result=(bus->linestate & (1<<SCSI_LINE_MSG)) >> SCSI_LINE_MSG; break;
        case SCSI_LINE_RESET    : result=(bus->linestate & (1<<SCSI_LINE_RESET)) >> SCSI_LINE_RESET; break;
    }

    LOG(3,"get_scsi_line(%s)=%d\n",linenames[lineno],result);

    return result;
}

READ_LINE_DEVICE_HANDLER( scsi_bsy_r ) { return get_scsi_line(device, SCSI_LINE_BSY); }
READ_LINE_DEVICE_HANDLER( scsi_sel_r ) { return get_scsi_line(device, SCSI_LINE_SEL); }
READ_LINE_DEVICE_HANDLER( scsi_cd_r ) { return get_scsi_line(device, SCSI_LINE_CD); }
READ_LINE_DEVICE_HANDLER( scsi_io_r ) { return get_scsi_line(device, SCSI_LINE_IO); }
READ_LINE_DEVICE_HANDLER( scsi_msg_r ) { return get_scsi_line(device, SCSI_LINE_MSG); }
READ_LINE_DEVICE_HANDLER( scsi_req_r ) { return get_scsi_line(device, SCSI_LINE_REQ); }
READ_LINE_DEVICE_HANDLER( scsi_ack_r ) { return get_scsi_line(device, SCSI_LINE_ACK); }
READ_LINE_DEVICE_HANDLER( scsi_rst_r ) { return get_scsi_line(device, SCSI_LINE_RESET); }

void set_scsi_line(device_t *device, UINT8 line, UINT8 state)
{
    scsibus_t   *bus = get_token(device);
    UINT8       changed;

    changed = ((bus->linestate & (1<<line)) != (state << line));

    LOG(3,"set_scsi_line(%s,%d), changed=%d, linestate=%02X\n",linenames[line],state,changed,bus->linestate);

    if(changed)
    {
        if (line==SCSI_LINE_ACK)
            set_scsi_line_ack(device,state);
        else
            set_scsi_line_now(device,line,state);
    }
}

WRITE_LINE_DEVICE_HANDLER( scsi_bsy_w ) { return set_scsi_line(device, SCSI_LINE_BSY, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_sel_w ) { return set_scsi_line(device, SCSI_LINE_SEL, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_cd_w ) { return set_scsi_line(device, SCSI_LINE_CD, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_io_w ) { return set_scsi_line(device, SCSI_LINE_IO, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_msg_w ) { return set_scsi_line(device, SCSI_LINE_MSG, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_req_w ) { return set_scsi_line(device, SCSI_LINE_REQ, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_ack_w ) { return set_scsi_line(device, SCSI_LINE_ACK, state); }
WRITE_LINE_DEVICE_HANDLER( scsi_rst_w ) { return set_scsi_line(device, SCSI_LINE_RESET, state); }

static void set_scsi_line_now(device_t *device, UINT8 line, UINT8 state)
{
    scsibus_t   *bus = get_token(device);

    if(state)
        bus->linestate |= (1<<line);
    else
        bus->linestate &= ~(1<<line);

    scsi_in_line_changed(device,line,state);
}

void set_scsi_line_ack(device_t *device, UINT8 state)
{
    scsibus_t   *bus = get_token(device);

    bus->ack_timer->adjust(attotime::from_nsec(ACK_DELAY_NS),state);
}

static void scsibus_exec_command(device_t *device)
{
    scsibus_t   *bus = get_token(device);
    int         command_local = 0;
    int         newphase;

    if(LOGLEVEL)
        dump_command_bytes(bus);

    //bus->is_linked=bus->command[bus->cmd_idx-1] & 0x01;
	bus->is_linked=0;

	// Assume command will succeed, if not set sytatus below.
	if (bus->command[0]!=SCSI_CMD_REQUEST_SENSE)
		SET_STATUS_SENSE(SCSI_STATUS_OK,SCSI_SENSE_NO_SENSE);

    // Check for locally executed commands, and if found execute them
    switch (bus->command[0])
    {
		// Test ready
		case SCSI_CMD_TEST_READY :
			LOG(1,"SCSIBUS: test_ready\n");
            command_local=1;
			bus->xfer_count=0;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Recalibrate drive
		case SCSI_CMD_RECALIBRATE :
			LOG(1,"SCSIBUS: Recalibrate drive\n");
            command_local=1;
			bus->xfer_count=0;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Request sense, return previous error codes
		case SCSI_CMD_REQUEST_SENSE :
			LOG(1,"SCSIBUS: request_sense\n");
            command_local=1;
			bus->xfer_count=SCSI_SENSE_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->data[0]=bus->sense;
			bus->data[1]=0x00;
			bus->data[2]=0x00;
			bus->data[3]=0x00;
			SET_STATUS_SENSE(SCSI_STATUS_OK,SCSI_SENSE_NO_SENSE);
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
			break;

        // Format unit
        case SCSI_CMD_FORMAT_UNIT :
            LOG(1,"SCSIBUS: format unit bus->command[1]=%02X & 0x10\n",(bus->command[1] & 0x10));
            command_local=1;
            if((bus->command[1] & 0x10)==0x10)
                bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            else
                bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);

            bus->xfer_count=4;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->dataout_timer->adjust(attotime::from_seconds(FORMAT_UNIT_TIMEOUT));
            break;

		// Check track format Xebec
		case SCSI_CMD_CHECK_TRACK_FORMAT :
			LOG(1,"SCSIBUS: check track format\n");
            command_local=1;
			bus->xfer_count=0;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Setup drive parameters Xebec
		case SCSI_CMD_INIT_DRIVE_PARAMS :
			LOG(1,"SCSIBUS: init_drive_params: Xebec S1410\n");
            command_local=1;
			bus->xfer_count=XEBEC_PARAMS_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            break;

		// Format bad track Xebec
		case SCSI_CMD_FORMAT_ALT_TRACK :
			LOG(1,"SCSIBUS: format_alt_track: Xebec S1410\n");
            command_local=1;
			bus->xfer_count=XEBEC_ALT_TRACK_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            break;

		// Write buffer Xebec S1410 specific
		case SCSI_CMD_WRITE_SEC_BUFFER :
			LOG(1,"SCSIBUS: write_sector_buffer: Xebec S1410\n");
            command_local=1;
			bus->xfer_count=XEBEC_SECTOR_BUFFER_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            break;

		// Read buffer Xebec S1410 specific
		case SCSI_CMD_READ_SEC_BUFFER :
            LOG(1,"SCSIBUS: read_sector_buffer: Xebec S1410\n");
            command_local=1;
			bus->xfer_count=XEBEC_SECTOR_BUFFER_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
            bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAIN);
            break;

		// Write buffer, Adaptec ACB40x0 specific
		case SCSI_CMD_WRITE_DATA_BUFFER :
			LOG(1,"SCSIBUS: write_buffer: Adaptec ACB40x0\n");
            command_local=1;
			bus->xfer_count=ADAPTEC_DATA_BUFFER_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            break;

		// Read buffer, Adaptec ACB40x0 specific
		case SCSI_CMD_READ_DATA_BUFFER :
            LOG(1,"SCSIBUS: read_data_buffer: Adaptec ACB40x0\n");
            command_local=1;
			bus->xfer_count=ADAPTEC_DATA_BUFFER_SIZE;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
            bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAIN);
            break;

		// Send diagnostic info
		case SCSI_CMD_SEND_DIAGNOSTIC :
            LOG(1,"SCSIBUS: send_diagnostic\n");
            command_local=1;
			bus->xfer_count=(bus->command[3]<<8)+bus->command[4];
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            break;

		case SCSI_CMD_SEARCH_DATA_EQUAL :
            LOG(1,"SCSIBUS: Search_data_equal ACB40x0\n");
            command_local=1;
			bus->xfer_count=0;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

        case SCSI_CMD_READ_DEFECT :
            LOG(1,"SCSIBUS: read defect list\n");
            command_local=1;

            bus->data[0] = 0x00;
            bus->data[1] = bus->command[2];
            bus->data[3] = 0x00;         // defect list len msb
            bus->data[4] = 0x00;         // defect list len lsb

            bus->xfer_count=4;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
            bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAIN);
            break;

        // write buffer
        case SCSI_CMD_BUFFER_WRITE :
            LOG(1,"SCSIBUS: write_buffer\n");
            command_local=1;
			bus->xfer_count=(bus->command[7]<<8)+bus->command[8];
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAOUT);
            break;

        // read buffer
        case SCSI_CMD_BUFFER_READ   :
            LOG(1,"SCSIBUS: read_buffer\n");
            command_local=1;
			bus->xfer_count=(bus->command[7]<<8)+bus->command[8];
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
            bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_DATAIN);
            break;

		// Xebec S1410
		case SCSI_CMD_RAM_DIAGS :
		case SCSI_CMD_DRIVE_DIAGS :
		case SCSI_CMD_CONTROLER_DIAGS :
			LOG(1,"SCSIBUS: Xebec RAM, disk or Controler diags [%02X]\n",bus->command[0]);
            command_local=1;
			bus->xfer_count=0;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;

		// Commodore D9060/9090
		case SCSI_CMD_PHYSICAL_DEVICE_ID :
			LOG(1,"SCSIBUS: physical device ID\n");
            command_local=1;
			bus->xfer_count=0;
            bus->data_last=bus->xfer_count;
            bus->bytes_left=0;
			bus->devices[bus->last_id]->SetPhase(SCSI_PHASE_STATUS);
			break;
	}


    // Check for locally executed command, if not then pass it on
    // to the disk driver
    if(!command_local)
    {
        bus->devices[bus->last_id]->SetCommand(bus->command, bus->cmd_idx);
        bus->devices[bus->last_id]->ExecCommand(&bus->xfer_count);
        bus->bytes_left=bus->xfer_count;
        bus->data_last=bus->xfer_count;
        bus->data_idx=0;
    }

    bus->devices[bus->last_id]->GetPhase(&newphase);

    scsi_change_phase(device,newphase);

    LOG(1,"SCSIBUS:xfer_count=%02X, bytes_left=%02X data_idx=%02X\n",bus->xfer_count,bus->bytes_left,bus->data_idx);

    // This is correct as we need to read from disk for commands other than just read data
    if ((bus->phase == SCSI_PHASE_DATAIN) && (!command_local))
        scsibus_read_data(bus);
}

static int datain_done(scsibus_t   *bus)
{
    int result=0;

    // Read data commands
    if(IS_READ_COMMAND() && (bus->data_idx == bus->data_last) && (bus->bytes_left == 0))
        result=1;
    else if (bus->data_idx==bus->data_last)
        result=1;

    return result;
}

static int dataout_done(scsibus_t   *bus)
{
    int result=0;

    // Write data commands
    if(IS_WRITE_COMMAND() && (bus->data_idx == 0) && (bus->bytes_left == 0))
        result=1;
    else if (bus->data_idx==bus->data_last)
        result=1;

    return result;
}

static void check_process_dataout(device_t *device)
{
	scsibus_t   	*bus = get_token(device);
    int				capacity=0;
	int				tracks;
	adaptec_sense_t	*sense;

	LOG(1,"SCSIBUS:check_process_dataout cmd=%02X\n",bus->command[0]);

	switch (bus->command[0])
	{
		case SCSI_CMD_INIT_DRIVE_PARAMS :
			tracks=((bus->data[0]<<8)+bus->data[1]);
			capacity=(tracks * bus->data[2]) * 17;
			LOG(1,"Tracks=%d, Heads=%d\n",tracks,bus->data[2]);
			LOG(1,"Setting disk capacity to %d blocks\n",capacity);
			//debugger_break(device->machine());
			break;

		case SCSI_CMD_MODE_SELECT :
			sense=(adaptec_sense_t *)bus->data;
			tracks=(sense->cylinder_count[0]<<8)+sense->cylinder_count[1];
			capacity=(tracks * sense->head_count * 17);
			LOG(1,"Tracks=%d, Heads=%d sec/track=%d\n",tracks,sense->head_count,sense->sectors_per_track);
			LOG(1,"Setting disk capacity to %d blocks\n",capacity);
			dump_data_bytes(bus,0x16);
			//debugger_break(device->machine());
			break;
	}
}


static void scsi_in_line_changed(device_t *device, UINT8 line, UINT8 state)
{
    scsibus_t   *bus = get_token(device);
	void		*hdfile;

    // Reset aborts and returns to bus free
    if((line==SCSI_LINE_RESET) && (state==0))
    {
        scsi_change_phase(device,SCSI_PHASE_BUS_FREE);
        bus->cmd_idx=0;
        bus->data_idx=0;
        bus->is_linked=0;

        return;
    }

    switch (bus->phase)
    {
        case SCSI_PHASE_BUS_FREE :
            if((line==SCSI_LINE_SEL) && (bus->devices[bus->last_id]!=NULL))
            {
				// Check to see if device had image file mounted, if not, do not set busy,
				// and stay busfree.
				bus->devices[bus->last_id]->GetDevice(&hdfile);
				if(hdfile!=(void *)NULL)
				{
					if(state==0)
						bus->sel_timer->adjust(attotime::from_nsec(BSY_DELAY_NS));
					else
						scsi_change_phase(device,SCSI_PHASE_COMMAND);
				}
            }
            break;

        case SCSI_PHASE_COMMAND :
            if(line==SCSI_LINE_ACK)
            {
                if(state)
                {
                    // If the command is ready go and execute it
                    if(bus->cmd_idx==get_scsi_cmd_len(bus->command[0]))
                    {
                        scsibus_exec_command(device);
                    }
                    else
                        scsi_out_line_change(device,SCSI_LINE_REQ,0);
                }
                else
                    scsi_out_line_change(device,SCSI_LINE_REQ,1);
            }
            break;

        case SCSI_PHASE_DATAIN :
            if(line==SCSI_LINE_ACK)
            {
                if(state)
                {
                    if(datain_done(bus))
                        scsi_change_phase(device,SCSI_PHASE_STATUS);
                    else
                        scsi_out_line_change(device,SCSI_LINE_REQ,0);
                }
                else
                    scsi_out_line_change(device,SCSI_LINE_REQ,1);
            }
            break;

        case SCSI_PHASE_DATAOUT :
            if(line==SCSI_LINE_ACK)
            {
                if(state)
                {
                    if(dataout_done(bus))
					{
						check_process_dataout(device);
                        scsi_change_phase(device,SCSI_PHASE_STATUS);
					}
                    else
                        scsi_out_line_change(device,SCSI_LINE_REQ,0);
                }
                else
                    scsi_out_line_change(device,SCSI_LINE_REQ,1);
            }
            break;

        case SCSI_PHASE_STATUS :
            if(line==SCSI_LINE_ACK)
            {
                if(state)
                {
                    if(bus->cmd_idx > 0)
                    {
                        scsi_change_phase(device,SCSI_PHASE_MESSAGE_IN);
                    }
                    else
                        scsi_out_line_change(device,SCSI_LINE_REQ,0);
                }
				else
				{
					bus->cmd_idx++;
                    scsi_out_line_change(device,SCSI_LINE_REQ,1);
				}
            }
            break;

        case SCSI_PHASE_MESSAGE_IN :
            if(line==SCSI_LINE_ACK)
            {
                if(state)
                {
                    if(bus->cmd_idx > 0)
                    {
                        if(bus->is_linked)
                            scsi_change_phase(device,SCSI_PHASE_COMMAND);
                        else
                            scsi_change_phase(device,SCSI_PHASE_BUS_FREE);
                    }
                    else
                        scsi_out_line_change(device,SCSI_LINE_REQ,0);
                }
                else
				{
					bus->cmd_idx++;
                    scsi_out_line_change(device,SCSI_LINE_REQ,1);
				}
            }
            break;
    }
}

static void scsi_out_line_change(device_t *device, UINT8 line, UINT8 state)
{
    if(line==SCSI_LINE_REQ)
        scsi_out_line_req(device,state);
    else
        scsi_out_line_change_now(device,line,state);
}

static void scsi_out_line_change_now(device_t *device, UINT8 line, UINT8 state)
{
    scsibus_t   *bus = get_token(device);

    if(state)
        bus->linestate |= (1<<line);
    else
        bus->linestate &= ~(1<<line);

    LOG(3,"scsi_out_line_change(%s,%d)\n",linenames[line],state);

    if(bus->interface->line_change_cb!=NULL)
        bus->interface->line_change_cb(device,line,state);

	switch (line)
	{
	case SCSI_LINE_BSY: bus->out_bsy_func(state); break;
	case SCSI_LINE_SEL: bus->out_sel_func(state); break;
	case SCSI_LINE_CD: bus->out_cd_func(state); break;
	case SCSI_LINE_IO: bus->out_io_func(state); break;
	case SCSI_LINE_MSG: bus->out_msg_func(state); break;
	case SCSI_LINE_REQ: bus->out_req_func(state); break;
	case SCSI_LINE_RESET: bus->out_rst_func(state); break;
	}
}

static void scsi_out_line_req(device_t *device, UINT8 state)
{
    scsibus_t   *bus = get_token(device);

    bus->req_timer->adjust(attotime::from_nsec(REQ_DELAY_NS),state);
}


static TIMER_CALLBACK(req_timer_callback)
{
    scsi_out_line_change_now((device_t *)ptr, SCSI_LINE_REQ, param);
}

static TIMER_CALLBACK(ack_timer_callback)
{
    set_scsi_line_now((device_t *)ptr, SCSI_LINE_ACK, param);
}

static TIMER_CALLBACK(sel_timer_callback)
{
    scsi_out_line_change_now((device_t *)ptr, SCSI_LINE_BSY, param);
}

static TIMER_CALLBACK(dataout_timer_callback)
{
    scsibus_t   *bus = get_token((device_t *)ptr);

	// Some drives, notably the ST225N and ST125N, accept fromat unit commands
	// with flags set indicating that bad block data should be transfered but
	// don't then implemnt a data in phase, this timeout it to catch these !
	if(IS_COMMAND(SCSI_CMD_FORMAT_UNIT) && (bus->data_idx==0))
		scsi_change_phase((device_t *)ptr,SCSI_PHASE_STATUS);

}

UINT8 get_scsi_phase(device_t *device)
{
    scsibus_t   *bus = get_token(device);

    return bus->phase;
}


static void scsi_change_phase(device_t *device, UINT8 newphase)
{
    scsibus_t   *bus = get_token(device);

    LOG(1,"scsi_change_phase() from=%s, to=%s\n",phasenames[bus->phase],phasenames[newphase]);

    bus->phase=newphase;
    bus->cmd_idx=0;
    bus->data_idx=0;

    switch(bus->phase)
    {
        case SCSI_PHASE_BUS_FREE :
            scsi_out_line_change(device,SCSI_LINE_CD,1);
            scsi_out_line_change(device,SCSI_LINE_IO,1);
            scsi_out_line_change(device,SCSI_LINE_MSG,1);
            scsi_out_line_change(device,SCSI_LINE_REQ,1);
            scsi_out_line_change(device,SCSI_LINE_BSY,1);
			LOG(1,"SCSIBUS: done\n\n");
			//if (IS_COMMAND(SCSI_CMD_READ_CAPACITY))
			//  debugger_break(device->machine());
            break;

        case SCSI_PHASE_COMMAND :
            scsi_out_line_change(device,SCSI_LINE_CD,0);
            scsi_out_line_change(device,SCSI_LINE_IO,1);
            scsi_out_line_change(device,SCSI_LINE_MSG,1);
            scsi_out_line_change(device,SCSI_LINE_REQ,0);
			LOG(1,"\nSCSIBUS: Command begin\n");
            break;

        case SCSI_PHASE_DATAOUT :
            scsi_out_line_change(device,SCSI_LINE_CD,1);
            scsi_out_line_change(device,SCSI_LINE_IO,1);
            scsi_out_line_change(device,SCSI_LINE_MSG,1);
            scsi_out_line_change(device,SCSI_LINE_REQ,0);
            break;

        case SCSI_PHASE_DATAIN :
            scsi_out_line_change(device,SCSI_LINE_CD,1);
            scsi_out_line_change(device,SCSI_LINE_IO,0);
            scsi_out_line_change(device,SCSI_LINE_MSG,1);
            scsi_out_line_change(device,SCSI_LINE_REQ,0);
            break;

        case SCSI_PHASE_STATUS :
            scsi_out_line_change(device,SCSI_LINE_CD,0);
            scsi_out_line_change(device,SCSI_LINE_IO,0);
            scsi_out_line_change(device,SCSI_LINE_MSG,1);
            scsi_out_line_change(device,SCSI_LINE_REQ,0);
            break;

        case SCSI_PHASE_MESSAGE_OUT :
            scsi_out_line_change(device,SCSI_LINE_CD,0);
            scsi_out_line_change(device,SCSI_LINE_IO,1);
            scsi_out_line_change(device,SCSI_LINE_MSG,0);
            scsi_out_line_change(device,SCSI_LINE_REQ,0);
            break;

        case SCSI_PHASE_MESSAGE_IN :
            scsi_out_line_change(device,SCSI_LINE_CD,0);
            scsi_out_line_change(device,SCSI_LINE_IO,0);
            scsi_out_line_change(device,SCSI_LINE_MSG,0);
            scsi_out_line_change(device,SCSI_LINE_REQ,0);
            break;
    }
}

UINT8 scsibus_driveno(UINT8  drivesel)
{
    switch (drivesel)
    {
        case 0x01   : return 0;
        case 0x02   : return 1;
        case 0x04   : return 2;
        case 0x08   : return 3;
        case 0x10   : return 4;
        case 0x20   : return 5;
        case 0x40   : return 6;
        case 0x80   : return 7;
        default : return 0;
    }
}

// get the length of a SCSI command based on it's command byte type
int get_scsi_cmd_len(int cbyte)
{
	int group;

	group = (cbyte>>5) & 7;

	if (group == 0 || group == 3 || group == 6 || group == 7) return 6;
	if (group == 1 || group == 2) return 10;
	if (group == 5) return 12;

	fatalerror("scsibus: Unknown SCSI command group %d, command byte=%02X", group,cbyte);

	return 6;
}

void init_scsibus(device_t *device, int sectorbytes)
{
    scsibus_t               *bus = get_token(device);
    const SCSIConfigTable   *scsidevs = bus->interface->scsidevs;
	int                     devno;

    // try to open the devices
    for (devno = 0; devno < scsidevs->devs_present; devno++)
    {
        LOG(1,"SCSIBUS:init devno=%d \n",devno);
        bus->devices[scsidevs->devices[devno].scsiID] = device->machine().device<scsidev_device>( scsidevs->devices[devno].tag );
    }

	bus->sectorbytes = sectorbytes;
}

static DEVICE_START( scsibus )
{
    scsibus_t               *bus = get_token(device);

	assert(device->static_config() != NULL);
    bus->interface = (const SCSIBus_interface*)device->static_config();

	memset(bus->devices, 0, sizeof(bus->devices));

	bus->out_bsy_func.resolve(bus->interface->out_bsy_func, *device);
	bus->out_sel_func.resolve(bus->interface->out_sel_func, *device);
	bus->out_cd_func.resolve(bus->interface->out_cd_func, *device);
	bus->out_io_func.resolve(bus->interface->out_io_func, *device);
	bus->out_msg_func.resolve(bus->interface->out_msg_func, *device);
	bus->out_req_func.resolve(bus->interface->out_req_func, *device);
	bus->out_rst_func.resolve(bus->interface->out_rst_func, *device);

    // All lines start high - inactive
    bus->linestate=0xFF;

    // Start with bus free
    bus->phase=SCSI_PHASE_BUS_FREE;

    // Setup req/ack/sel timers
    bus->req_timer=device->machine().scheduler().timer_alloc(FUNC(req_timer_callback), (void *)device);
    bus->ack_timer=device->machine().scheduler().timer_alloc(FUNC(ack_timer_callback), (void *)device);
	bus->sel_timer=device->machine().scheduler().timer_alloc(FUNC(sel_timer_callback), (void *)device);
	bus->dataout_timer=device->machine().scheduler().timer_alloc(FUNC(dataout_timer_callback), (void *)device);

}

static DEVICE_STOP( scsibus )
{
}

static DEVICE_RESET( scsibus )
{
}

DEVICE_GET_INFO( scsibus )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(scsibus_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = 0;				    		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:				    info->start = DEVICE_START_NAME(scsibus);	break;
		case DEVINFO_FCT_STOP:				    info->stop  = DEVICE_STOP_NAME(scsibus);	break;
		case DEVINFO_FCT_RESET:				    info->reset = DEVICE_RESET_NAME(scsibus);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:				    strcpy(info->s, "SCSI bus");				break;
		case DEVINFO_STR_FAMILY:			    strcpy(info->s, "SCSI");				    break;
		case DEVINFO_STR_VERSION:			    strcpy(info->s, "1.0");				    break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				    break;
		case DEVINFO_STR_CREDITS:			    strcpy(info->s, "Copyright the MAME and MESS Teams"); break;
	}
}

DEFINE_LEGACY_DEVICE(SCSIBUS, scsibus);
