// license:MAME
// copyright-holders:smf
/*

scsihle.h

Base class for HLE'd SCSI devices.

*/

#ifndef _SCSIHLE_H_
#define _SCSIHLE_H_

#include "machine/scsibus.h"
#include "machine/scsidev.h"
#include "machine/t10spc.h"

class scsihle_device : public scsidev_device,
	public virtual t10spc
{
public:
	// construction/destruction
	scsihle_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual int GetDeviceID();

	virtual void scsi_in( UINT32 data, UINT32 mask );

	// configuration helpers
	static void static_set_deviceid(device_t &device, int _scsiID);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	void scsi_out_req_delay(UINT8 state);
	void scsi_change_phase(UINT8 newphase);
	int get_scsi_cmd_len(int cbyte);
	UINT8 scsibus_driveno(UINT8  drivesel);
	void scsibus_read_data();
	void scsibus_write_data();
	void scsibus_exec_command();
	void dump_command_bytes();
	void dump_data_bytes(int count);
	void dump_bytes(UINT8 *buff, int count);

	emu_timer *req_timer;
	emu_timer *sel_timer;
	emu_timer *dataout_timer;

	UINT8 cmd_idx;
	UINT8 is_linked;

	UINT8 buffer[ 1024 ];
	UINT16 data_idx;
	int bytes_left;
	int data_last;

	int scsiID;
};

//
// Status / Sense data taken from Adaptec ACB40x0 documentation.
//

// SCSI IDs
enum
{
	SCSI_ID_0 = 0,
	SCSI_ID_1,
	SCSI_ID_2,
	SCSI_ID_3,
	SCSI_ID_4,
	SCSI_ID_5,
	SCSI_ID_6,
	SCSI_ID_7
};

#define MCFG_SCSIDEV_ADD(_tag, _type, _id) \
	MCFG_DEVICE_ADD(_tag, _type, 0) \
	scsihle_device::static_set_deviceid(*device, _id);

#endif
