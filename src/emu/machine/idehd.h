#include "emu.h"
#include "imagedev/harddriv.h"

#define IDE_DISK_SECTOR_SIZE            512

#define IDE_STATUS_ERROR                    0x01
#define IDE_STATUS_HIT_INDEX                0x02
#define IDE_STATUS_BUFFER_READY             0x08
#define IDE_STATUS_SEEK_COMPLETE            0x10
#define IDE_STATUS_DRIVE_READY              0x40
#define IDE_STATUS_BUSY                     0x80

#define IDE_ERROR_NONE                      0x00
#define IDE_ERROR_DEFAULT                   0x01
#define IDE_ERROR_TRACK0_NOT_FOUND          0x02
#define IDE_ERROR_UNKNOWN_COMMAND           0x04
#define IDE_ERROR_BAD_LOCATION              0x10
#define IDE_ERROR_BAD_SECTOR                0x80

// ======================> ide_device_interface

class ide_device_interface
{
public:
	ide_device_interface(const machine_config &mconfig, device_t &device);

	virtual void set_irq(int state);
	virtual void set_dmarq(int state);

	virtual UINT16 read_dma() = 0;
	virtual DECLARE_READ16_MEMBER(read_cs0) = 0;
	virtual DECLARE_READ16_MEMBER(read_cs1) = 0;
	virtual void write_dma(UINT16 data) = 0;
	virtual DECLARE_WRITE16_MEMBER(write_cs0) = 0;
	virtual DECLARE_WRITE16_MEMBER(write_cs1) = 0;

	virtual UINT8 *get_features() = 0;

	UINT8           m_master_password_enable;
	UINT8           m_user_password_enable;
	const UINT8 *   m_master_password;
	const UINT8 *   m_user_password;

	int m_csel;
	int m_dasp;

	devcb2_write_line m_irq_handler;
	devcb2_write_line m_dmarq_handler;
};

class ide_mass_storage_device : public device_t,
	public ide_device_interface,
	public device_slot_card_interface
{
public:
	ide_mass_storage_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname = "", const char *source = __FILE__);

	virtual int  read_sector(UINT32 lba, void *buffer) = 0;
	virtual int  write_sector(UINT32 lba, const void *buffer) = 0;

	virtual void set_irq(int state);
	virtual UINT16 read_dma();
	virtual DECLARE_READ16_MEMBER(read_cs0);
	virtual DECLARE_READ16_MEMBER(read_cs1);
	virtual void write_dma(UINT16 data);
	virtual DECLARE_WRITE16_MEMBER(write_cs0);
	virtual DECLARE_WRITE16_MEMBER(write_cs1);

	virtual UINT8 *get_features() { return m_features; }
	
protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual bool is_ready() = 0;
	virtual void read_key(UINT8 key[]) = 0;

	UINT8           m_gnetreadlock;

	UINT8           m_features[IDE_DISK_SECTOR_SIZE];
	UINT16          m_num_cylinders;
	UINT8           m_num_sectors;
	UINT8           m_num_heads;

private:
	UINT32 lba_address();
	void set_geometry(UINT8 sectors, UINT8 heads) { m_num_sectors = sectors; m_num_heads = heads; }
	void read_sector_done();
	void write_sector_done();
	void read_next_sector();
	void continue_write();
	void signal_delayed_interrupt(attotime time, int buffer_ready);
	void next_sector();
	void security_error();
	void continue_read();
	void read_first_sector();
	void handle_command(UINT8 _command);
	void read_buffer_empty();
	void write_buffer_full();

	int             m_cur_drive;
	UINT16          m_cur_cylinder;
	UINT8           m_cur_sector;
	UINT8           m_cur_head;
	UINT8           m_cur_head_reg;
	UINT32          m_cur_lba;

	UINT8           m_status;
	UINT8           m_error;
	UINT8           m_command;

	UINT8           m_buffer[IDE_DISK_SECTOR_SIZE];
	UINT16          m_buffer_offset;

	UINT8           m_adapter_control;
	UINT8           m_precomp_offset;
	UINT16          m_sector_count;
	UINT16          m_block_count;

	UINT8           m_interrupt_pending;
	UINT16          m_sectors_until_int;

	UINT8           m_dma_active;
	UINT8           m_verify_only;

	emu_timer *     m_last_status_timer;
	emu_timer *     m_reset_timer;
};

// ======================> ide_hdd_device

class ide_hdd_device : public ide_mass_storage_device
{
public:
	// construction/destruction
	ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual int  read_sector(UINT32 lba, void *buffer) { return hard_disk_read(m_disk, lba, buffer); }
	virtual int  write_sector(UINT32 lba, const void *buffer) { return hard_disk_write(m_disk, lba, buffer); }

protected:
	// device-level overrides
	virtual void device_reset();

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual bool is_ready() { return (m_disk != NULL); }
	virtual void read_key(UINT8 key[]);

	void ide_build_features();

	chd_file       *m_handle;
	hard_disk_file *m_disk;
};

// device type definition
extern const device_type IDE_HARDDISK;
