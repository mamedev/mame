#include "emu.h"
#include "imagedev/harddriv.h"

#define IDE_DISK_SECTOR_SIZE            512

// ======================> ide_device_interface

class ide_device_interface : public device_slot_card_interface
{
public:
	ide_device_interface(const machine_config &mconfig, device_t &device);
public:
	virtual int  read_sector(UINT32 lba, void *buffer) = 0;
	virtual int  write_sector(UINT32 lba, const void *buffer) = 0;

	UINT8 *get_features() { return m_features;}

	UINT16 get_cylinders() { return m_num_cylinders; }
	UINT16 get_sectors() { return m_num_sectors; }
	UINT16 get_heads() { return m_num_heads; }
	void set_geometry(UINT8 sectors, UINT8 heads) { m_num_sectors= sectors; m_num_heads=heads; }
	UINT32 lba_address();
	virtual bool is_ready() { return true; }
	virtual void read_key(UINT8 key[]) { }
	UINT16          cur_cylinder;
	UINT8           cur_sector;
	UINT8           cur_head;
	UINT8           cur_head_reg;
	UINT32          cur_lba;

	UINT8           buffer[IDE_DISK_SECTOR_SIZE];
	UINT16          buffer_offset;

	UINT8           adapter_control;
	UINT8           precomp_offset;
	UINT16          sector_count;
	UINT16          block_count;

	UINT8           dma_active;
	UINT8           verify_only;

	UINT8           master_password_enable;
	UINT8           user_password_enable;
	const UINT8 *   master_password;
	const UINT8 *   user_password;

	UINT8           gnetreadlock;

protected:
	UINT8           m_features[IDE_DISK_SECTOR_SIZE];
	UINT16          m_num_cylinders;
	UINT8           m_num_sectors;
	UINT8           m_num_heads;
};

// ======================> ide_hdd_device

class ide_hdd_device : public device_t,
						public ide_device_interface
{
public:
	// construction/destruction
	ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual int  read_sector(UINT32 lba, void *buffer) { return hard_disk_read(m_disk, lba, buffer); }
	virtual int  write_sector(UINT32 lba, const void *buffer) { return hard_disk_write(m_disk, lba, buffer); }
	virtual void read_key(UINT8 key[]);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	void ide_build_features();
	virtual bool is_ready() { return (m_disk != NULL); }
protected:
	chd_file       *m_handle;
	hard_disk_file *m_disk;
};
// device type definition
extern const device_type IDE_HARDDISK;
