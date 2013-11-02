// license:MAME
// copyright-holders:smf
/*

scsidev.h

Base class for SCSI devices.

*/

#ifndef _SCSIDEV_H_
#define _SCSIDEV_H_

#include "emu.h"

#define SCSI_MASK_DATA  ( 0x00000ff )
#define SCSI_MASK_DATAH ( 0x000ff00 )
#define SCSI_MASK_DATAP ( 0x0010000 )
#define SCSI_MASK_BSY   ( 0x0020000 )
#define SCSI_MASK_SEL   ( 0x0040000 )
#define SCSI_MASK_CD    ( 0x0080000 )
#define SCSI_MASK_IO    ( 0x0100000 )
#define SCSI_MASK_MSG   ( 0x0200000 )
#define SCSI_MASK_REQ   ( 0x0400000 )
#define SCSI_MASK_ACK   ( 0x0800000 )
#define SCSI_MASK_ATN   ( 0x1000000 )
#define SCSI_MASK_RST   ( 0x2000000 )
#define SCSI_MASK_ALL   ( 0x3ffffff )

class scsibus_device;

// base handler
class scsidev_device : public device_t
{
	friend class scsibus_device;

public:
	// construction/destruction
	scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start();

	void scsi_out( UINT32 data, UINT32 mask );

private:
	virtual void scsi_in( UINT32 data, UINT32 mask ) = 0;

	UINT32 data_out;
	scsibus_device *m_scsibus;
};

#endif
