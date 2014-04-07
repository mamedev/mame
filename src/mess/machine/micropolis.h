/*********************************************************************

    micropolis.h

    Implementations of the Micropolis
    floppy disk controller for the Sorcerer

*********************************************************************/

#ifndef __MICROPOLIS_H__
#define __MICROPOLIS_H__

#include "devcb.h"
#include "imagedev/flopdrv.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define MCFG_MICROPOLIS_DDEN_CALLBACK(_read) \
	devcb = &micropolis_device::set_dden_rd_callback(*device, DEVCB2_##_read);

#define MCFG_MICROPOLIS_INTRQ_CALLBACK(_write) \
	devcb = &micropolis_device::set_intrq_wr_callback(*device, DEVCB2_##_write);

#define MCFG_MICROPOLIS_DRQ_CALLBACK(_write) \
	devcb = &micropolis_device::set_drq_wr_callback(*device, DEVCB2_##_write);

/* Interface */
struct micropolis_interface
{
	const char *m_floppy_drive_tags[4];
};

/***************************************************************************
    MACROS
***************************************************************************/

class micropolis_device : public device_t,
							public micropolis_interface
{
public:
	micropolis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~micropolis_device() {}

	template<class _Object> static devcb2_base &set_dden_rd_callback(device_t &device, _Object object) { return downcast<micropolis_device &>(device).m_read_dden.set_callback(object); }
	template<class _Object> static devcb2_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<micropolis_device &>(device).m_write_intrq.set_callback(object); }
	template<class _Object> static devcb2_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<micropolis_device &>(device).m_write_drq.set_callback(object); }

	void set_drive(UINT8 drive); // set current drive (0-3)

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( data_r );

	DECLARE_WRITE8_MEMBER( command_w );
	DECLARE_WRITE8_MEMBER( data_w );

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state

	devcb2_read_line m_read_dden;
	devcb2_write_line m_write_intrq;
	devcb2_write_line m_write_drq;

	/* register */
	UINT8 m_data;
	UINT8 m_drive_num;
	UINT8 m_track;
	UINT8 m_sector;
	UINT8 m_command;
	UINT8 m_status;

	UINT8   m_write_cmd;              /* last write command issued */

	UINT8   m_buffer[6144];           /* I/O buffer (holds up to a whole track) */
	UINT32  m_data_offset;            /* offset into I/O buffer */
	INT32   m_data_count;             /* transfer count from/into I/O buffer */

	UINT32  m_sector_length;          /* sector length (byte) */

	/* this is the drive currently selected */
	legacy_floppy_image_device *m_drive;

	void read_sector();
	void write_sector();
};

extern const device_type MICROPOLIS;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

extern const micropolis_interface default_micropolis_interface;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MICROPOLIS_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MICROPOLIS, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __MICROPOLIS_H__ */
