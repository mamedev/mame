/*
    SCSIBus.h

*/

#pragma once

#ifndef _SCSIBUS_H_
#define _SCSIBUS_H_

#include "machine/scsicb.h"
#include "machine/scsihle.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define MCFG_SCSIBUS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SCSIBUS, 0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SCSI_LINE_SEL   0
#define SCSI_LINE_BSY   1
#define SCSI_LINE_REQ   2
#define SCSI_LINE_ACK   3
#define SCSI_LINE_CD    4
#define SCSI_LINE_IO    5
#define SCSI_LINE_MSG   6
#define SCSI_LINE_RESET 7

#define REQ_DELAY_NS    90
#define ACK_DELAY_NS    90
#define BSY_DELAY_NS	50

#define CMD_BUF_SIZE    			32
#define ADAPTEC_BUF_SIZE			1024

// scsidev
#define SCSI_CMD_BUFFER_WRITE ( 0x3b )
#define SCSI_CMD_BUFFER_READ ( 0x3c )

// scsihd
#define SCSI_CMD_FORMAT_UNIT    	0x04
#define SCSI_CMD_SEARCH_DATA_EQUAL	0x31
#define SCSI_CMD_READ_DEFECT    	0x37


#define IS_COMMAND(cmd)         	(command[0]==cmd)
#define IS_READ_COMMAND()       	((command[0]==0x08) || (command[0]==0x28) || (command[0]==0xa8))
#define IS_WRITE_COMMAND()      	((command[0]==0x0a) || (command[0]==0x2a))

#define FORMAT_UNIT_TIMEOUT			5

struct adaptec_sense_t
{
	// parameter list
	UINT8		reserved1[3];
	UINT8		length;

	// descriptor list
	UINT8		density;
	UINT8		reserved2[4];
	UINT8		block_size[3];

	// drive parameter list
	UINT8		format_code;
	UINT8		cylinder_count[2];
	UINT8		head_count;
	UINT8		reduced_write[2];
	UINT8		write_precomp[2];
	UINT8		landing_zone;
	UINT8		step_pulse_code;
	UINT8		bit_flags;
	UINT8		sectors_per_track;
};

class scsibus_device : public device_t
{
public:
	// construction/destruction
	scsibus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	/* SCSI Bus read/write */

	UINT8 scsi_data_r();
	void scsi_data_w( UINT8 data );

	/* Get/Set lines */

	UINT8 get_scsi_line(UINT8 lineno);
	void set_scsi_line(UINT8 line, UINT8 state);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	int get_scsi_cmd_len(int cbyte);
	UINT8 scsibus_driveno(UINT8  drivesel);
	void scsi_change_phase(UINT8 newphase);
	void set_scsi_line_now(UINT8 line, UINT8 state);
	void set_scsi_line_ack(UINT8 state);
	void scsi_in_line_changed(UINT8 line, UINT8 state);
	void scsi_out_line_change(UINT8 line, UINT8 state);
	void scsi_out_line_change_now(UINT8 line, UINT8 state);
	void scsi_out_line_req(UINT8 state);
	void scsibus_read_data();
	void scsibus_write_data();
	void scsibus_exec_command();
	void check_process_dataout();
	void dump_command_bytes();
	void dump_data_bytes(int count);
	void dump_bytes(UINT8 *buff, int count);

	scsihle_device          *devices[8];
	scsicb_device *m_scsicb;

	UINT8       linestate;
	UINT8       last_id;
	UINT8       phase;

	UINT8       command[CMD_BUF_SIZE];
	UINT8       cmd_idx;
	UINT8       is_linked;

	UINT8       buffer[ADAPTEC_BUF_SIZE];
	UINT16      data_idx;
	int         bytes_left;
	int         data_last;
	int         sectorbytes;

	emu_timer *req_timer;
	emu_timer *ack_timer;
	emu_timer *sel_timer;
	emu_timer *dataout_timer;
};

// device type definition
extern const device_type SCSIBUS;

#endif
