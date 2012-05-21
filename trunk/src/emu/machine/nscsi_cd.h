#ifndef __NSCSI_CD_H__
#define __NSCSI_CD_H__

#include "machine/nscsi_bus.h"
#include "cdrom.h"

#define MCFG_NSCSI_CDROM_ADD(_tag, _subtag)						\
	MCFG_NSCSI_FULL_DEVICE_ADD(_tag, _subtag, NSCSI_CDROM, 0)

class nscsi_cdrom_device : public nscsi_full_device
{
public:
	nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;

	static struct cdrom_interface cd_intf;

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "scsi_cdrom"; }

	virtual void scsi_command();
	virtual UINT8 scsi_get_data(int id, int pos);

private:
	UINT8 block[2048];
	cdrom_file *cdrom;
	int bytes_per_sector;
	int lba, cur_lba, blocks;
};

extern const device_type NSCSI_CDROM;

#endif
