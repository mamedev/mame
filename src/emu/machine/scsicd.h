/***************************************************************************

 scsicd.h

***************************************************************************/

#ifndef _SCSICD_H_
#define _SCSICD_H_

#include "machine/scsihle.h"
#include "cdrom.h"

class scsicd_device : public scsihle_device
{
public:
	// construction/destruction
	scsicd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	scsicd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void SetDevice( void *device );
	virtual void GetDevice( void **device );
	virtual void ExecCommand( int *transferLength );
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );
	virtual int GetSectorBytes();

	static struct cdrom_interface cd_intf;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	UINT32 lba;
	UINT32 blocks;
	UINT32 last_lba;
	UINT32 bytes_per_sector;
	UINT32 num_subblocks;
	UINT32 cur_subblock;
	UINT32 play_err_flag;
	cdrom_file *cdrom;
};

// device type definition
extern const device_type SCSICD;

#endif
