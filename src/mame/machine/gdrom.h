/***************************************************************************

 gdrom.h

***************************************************************************/

#ifndef _GDROM_H_
#define _GDROM_H_

#include "machine/scsihle.h"

// Sega GD-ROM handler
class gdrom_device : public scsihle_device
{
public:
	// construction/destruction
	gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
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
	virtual void device_stop();

private:
	UINT32 lba;
	UINT32 blocks;
	UINT32 last_lba;
	UINT32 bytes_per_sector;
	UINT32 num_subblocks;
	UINT32 cur_subblock;
	UINT32 play_err_flag;
	UINT32 read_type;   // for command 0x30 only
	UINT32 data_select; // for command 0x30 only
	cdrom_file *cdrom;
	bool is_file;
};

// device type definition
extern const device_type GDROM;

#endif
