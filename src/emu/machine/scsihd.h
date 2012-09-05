/***************************************************************************

 scsihd.h

***************************************************************************/

#ifndef _SCSIHD_H_
#define _SCSIHD_H_

#include "machine/scsidev.h"
#include "harddisk.h"

class scsihd_device : public scsidev_device
{
public:
	// construction/destruction
	scsihd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	scsihd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void SetDevice( void *device );
	virtual void GetDevice( void **device );
	virtual void ExecCommand( int *transferLength );
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();

private:
	UINT32 lba;
	UINT32 blocks;
	int sectorbytes;
	hard_disk_file *disk;
	bool is_image_device;
};

// device type definition
extern const device_type SCSIHD;

#endif
