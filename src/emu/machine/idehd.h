#include "emu.h"
#include "atadev.h"
#include "harddisk.h"
#include "imagedev/harddriv.h"

#define IDE_DISK_SECTOR_SIZE            512

// Error
#define IDE_STATUS_ERR  (0x01)

// Index
#define IDE_STATUS_IDX  (0x02)

// Corrected Data
#define IDE_STATUS_CORR (0x04)

// Data Request
#define IDE_STATUS_DRQ  (0x08)

// Drive Seek Complete
#define IDE_STATUS_DSC  (0x10)

// Drive Write Fault
#define IDE_STATUS_DWF  (0x20)

// Drive Ready
#define IDE_STATUS_DRDY (0x40)

// Busy
#define IDE_STATUS_BSY  (0x80)

#define IDE_ERROR_NONE                      0x00
#define IDE_ERROR_DIAGNOSTIC_OK             0x01
#define IDE_ERROR_TRACK0_NOT_FOUND          0x02
#define IDE_ERROR_UNKNOWN_COMMAND           0x04
#define IDE_ERROR_BAD_LOCATION              0x10
#define IDE_ERROR_BAD_SECTOR                0x80

#define IDE_ERROR_DIAGNOSTIC_FAILED 0x00
#define IDE_ERROR_DIAGNOSTIC_PASSED 0x01
#define IDE_ERROR_DIAGNOSTIC_DEVICE1_FAILED 0x81

#define IDE_DEVICE_HEAD_HS   0x0f
#define IDE_DEVICE_HEAD_DRV  0x10
#define IDE_DEVICE_HEAD_L    0x40

class ata_mass_storage_device : public device_t,
	public ata_device_interface,
	public device_slot_card_interface
{
public:
	ata_mass_storage_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname = "", const char *source = __FILE__);

	virtual UINT16 read_dma();
	virtual DECLARE_READ16_MEMBER(read_cs0);
	virtual DECLARE_READ16_MEMBER(read_cs1);

	virtual void write_dma(UINT16 data);
	virtual DECLARE_WRITE16_MEMBER(write_cs0);
	virtual DECLARE_WRITE16_MEMBER(write_cs1);
	virtual DECLARE_WRITE_LINE_MEMBER(write_csel);
	virtual DECLARE_WRITE_LINE_MEMBER(write_dasp);
	virtual DECLARE_WRITE_LINE_MEMBER(write_dmack);

	virtual UINT8 *identify_device_buffer() { return m_identify_device; }
	
protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual bool is_ready() { return true; }
	virtual int read_sector(UINT32 lba, void *buffer) = 0;
	virtual int write_sector(UINT32 lba, const void *buffer) = 0;

	int dev() { return (m_device_head & IDE_DEVICE_HEAD_DRV) >> 4; }
	bool device_selected() { return m_csel == dev(); }
	bool single_device() { return m_csel == 0 && m_dasp == 0; }

	void set_irq(int state);
	void set_dmarq(int state);
	void ide_build_identify_device();

	virtual bool process_command();
	virtual void process_buffer();
	virtual void fill_buffer();

	UINT8           m_buffer[IDE_DISK_SECTOR_SIZE];
	UINT16          m_buffer_offset;
	UINT8           m_error;
	UINT8           m_feature;
	UINT16          m_sector_count;
	UINT8           m_sector_number;
	UINT8           m_cylinder_low;
	UINT8           m_cylinder_high;
	UINT8           m_device_head;
	UINT8           m_status;
	UINT8           m_command;
	UINT8           m_device_control;

	int m_can_identify_device;
	UINT8           m_identify_device[IDE_DISK_SECTOR_SIZE];
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
	void next_sector();
	void security_error();
	void continue_read();
	void read_first_sector();
	void read_buffer_empty();
	void write_buffer_full();
	void update_irq();

	int m_csel;
	int m_dasp;
	int m_dmack;
	int m_dmarq;
	int m_irq;

	UINT32          m_cur_lba;
	UINT16          m_block_count;
	UINT16          m_sectors_until_int;

	emu_timer *     m_last_status_timer;
	emu_timer *     m_reset_timer;
};

// ======================> ide_hdd_device

class ide_hdd_device : public ata_mass_storage_device
{
public:
	// construction/destruction
	ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual int read_sector(UINT32 lba, void *buffer) { if (m_disk == NULL) return 0; return hard_disk_read(m_disk, lba, buffer); }
	virtual int write_sector(UINT32 lba, const void *buffer) { if (m_disk == NULL) return 0; return hard_disk_write(m_disk, lba, buffer); }

protected:
	// device-level overrides
	virtual void device_reset();

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	chd_file       *m_handle;
	hard_disk_file *m_disk;

private:
	required_device<harddisk_image_device> m_image;
};

// device type definition
extern const device_type IDE_HARDDISK;
