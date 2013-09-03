/***************************************************************************

 gdrom.h

***************************************************************************/

#ifndef _GDROM_H_
#define _GDROM_H_

#include "machine/atapihle.h"
#include "machine/scsihle.h"
#include "sound/cdda.h"
#include "cdrom.h"

// Sega GD-ROM handler
class scsi_gdrom_device : public scsihle_device
{
public:
	// construction/destruction
	scsi_gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
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
	UINT32 transferOffset;
	cdrom_file *cdrom;
	required_device<cdda_device> m_cdda;
	bool is_file;
	UINT8 GDROM_Cmd11_Reply[32];
};

// device type definition
extern const device_type SCSI_GDROM;

class gdrom_device : public atapi_hle_device
{
public:
	gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();

	virtual void perform_diagnostic();
	virtual void identify_packet_device();
	virtual void process_buffer();
};

// device type definition
extern const device_type GDROM;

#endif
