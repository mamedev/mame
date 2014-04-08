/*********************************************************************

    wd17xx.h

    Implementations of the Western Digital 17xx and 27xx families of
    floppy disk controllers

*********************************************************************/

#ifndef __WD17XX_H__
#define __WD17XX_H__

#include "imagedev/flopdrv.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* Interface */
struct wd17xx_interface
{
	devcb_read_line in_dden_func;
	devcb_write_line out_intrq_func;
	devcb_write_line out_drq_func;
	const char *floppy_drive_tags[4];
};

extern const wd17xx_interface default_wd17xx_interface;
extern const wd17xx_interface default_wd17xx_interface_2_drives;

/***************************************************************************
    MACROS
***************************************************************************/

class wd1770_device : public device_t
{
public:
	wd1770_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	wd1770_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	/* the following are not strictly part of the wd179x hardware/emulation
	but will be put here for now until the flopdrv code has been finalised more */
	void set_drive(UINT8);     /* set drive wd179x is accessing */
	void set_side(UINT8);      /* set side wd179x is accessing */

	void set_pause_time(int usec);       /* default is 40 usec if not set */
	void index_pulse_callback(device_t *img, int state);

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( track_r );
	DECLARE_READ8_MEMBER( sector_r );
	DECLARE_READ8_MEMBER( data_r );

	DECLARE_WRITE8_MEMBER( command_w );
	DECLARE_WRITE8_MEMBER( track_w );
	DECLARE_WRITE8_MEMBER( sector_w );
	DECLARE_WRITE8_MEMBER( data_w );

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	WRITE_LINE_MEMBER( mr_w );
	WRITE_LINE_MEMBER( rdy_w );
	READ_LINE_MEMBER( mo_r );
	WRITE_LINE_MEMBER( tr00_w );
	WRITE_LINE_MEMBER( idx_w );
	WRITE_LINE_MEMBER( wprt_w );
	WRITE_LINE_MEMBER( dden_w );
	READ_LINE_MEMBER( drq_r );
	READ_LINE_MEMBER( intrq_r );
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
protected:
	int wd17xx_dden();
	void wd17xx_clear_drq();
	void wd17xx_set_drq();
	void wd17xx_clear_intrq();
	void wd17xx_set_intrq();
	TIMER_CALLBACK_MEMBER( wd17xx_command_callback );
	TIMER_CALLBACK_MEMBER( wd17xx_data_callback );
	void wd17xx_set_busy(attotime duration);
	void wd17xx_command_restore();
	void write_track();
	void read_track();
	void wd17xx_read_id();
	int wd17xx_locate_sector();
	int wd17xx_find_sector();
	void wd17xx_side_compare(UINT8 command);
	void wd17xx_read_sector();
	void wd17xx_complete_command(int delay);
	void wd17xx_write_sector();
	void wd17xx_verify_seek();
	TIMER_CALLBACK_MEMBER( wd17xx_read_sector_callback );
	TIMER_CALLBACK_MEMBER( wd17xx_write_sector_callback );
	void wd17xx_timed_data_request();
	void wd17xx_timed_read_sector_request();
	void wd17xx_timed_write_sector_request();

	// internal state
	/* callbacks */
	devcb_resolved_read_line m_in_dden_func;
	devcb_resolved_write_line m_out_intrq_func;
	devcb_resolved_write_line m_out_drq_func;

	/* input lines */
	int m_mr;            /* master reset */
	int m_rdy;           /* ready, enable precomp */
	int m_tr00;          /* track 00 */
	int m_idx;           /* index */
	int m_wprt;          /* write protect */
	int m_dden;          /* double density */

	/* output lines */
	int m_mo;            /* motor on */
	int m_dirc;          /* direction */
	int m_drq;           /* data request */
	int m_intrq;         /* interrupt request */

	/* register */
	UINT8 m_data_shift;
	UINT8 m_data;
	UINT8 m_track;
	UINT8 m_sector;
	UINT8 m_command;
	UINT8 m_status;
	UINT8 m_interrupt;

	int m_stepping_rate[4];  /* track stepping rate in ms */

	unsigned short  m_crc;    /* Holds the current CRC value for write_track CRC calculation */
	int     m_crc_active; /* Flag indicating that CRC calculation in write_track is active. */

	UINT8   m_track_reg;              /* value of track register */
	UINT8   m_command_type;           /* last command type */
	UINT8   m_head;                   /* current head # */

	UINT8   m_read_cmd;               /* last read command issued */
	UINT8   m_write_cmd;              /* last write command issued */
	INT8    m_direction;              /* last step direction */
	UINT8   m_last_command_data;      /* last command data */

	UINT8   m_status_drq;             /* status register data request bit */
	UINT8   m_busy_count;             /* how long to keep busy bit set */

	UINT8   m_buffer[6144];           /* I/O buffer (holds up to a whole track) */
	UINT32  m_data_offset;            /* offset into I/O buffer */
	INT32   m_data_count;             /* transfer count from/into I/O buffer */

	UINT8   *m_fmt_sector_data[256];  /* pointer to data after formatting a track */

	UINT8   m_dam_list[256][4];       /* list of data address marks while formatting */
	int     m_dam_data[256];          /* offset to data inside buffer while formatting */
	int     m_dam_cnt;                /* valid number of entries in the dam_list */
	UINT16  m_sector_length;          /* sector length (byte) */

	UINT8   m_ddam;                   /* ddam of sector found - used when reading */
	UINT8   m_sector_data_id;
	int     m_data_direction;

	int     m_hld_count;              /* head loaded counter */

	/* timers to delay execution/completion of commands */
	emu_timer *m_timer_cmd, *m_timer_data, *m_timer_rs, *m_timer_ws;

	/* this is the drive currently selected */
	legacy_floppy_image_device *m_drive;

	/* this is the head currently selected */
	UINT8 m_hd;

	/* pause time when writing/reading sector */
	int m_pause_time;

	/* Were we busy when we received a FORCE_INT command */
	UINT8   m_was_busy;

	/* Pointer to interface */
	const wd17xx_interface *m_intf;
};

extern ATTR_DEPRECATED const device_type WD1770;

class fd1771_device : public wd1770_device
{
public:
	fd1771_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1771;

class fd1781_device : public wd1770_device
{
public:
	fd1781_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1781;

class fd1791_device : public wd1770_device
{
public:
	fd1791_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1791;

class fd1792_device : public wd1770_device
{
public:
	fd1792_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1792;

class fd1793_device : public wd1770_device
{
public:
	fd1793_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1793;

class fd1794_device : public wd1770_device
{
public:
	fd1794_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1794;

class fd1795_device : public wd1770_device
{
public:
	fd1795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1795;

class fd1797_device : public wd1770_device
{
public:
	fd1797_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1797;

class fd1761_device : public wd1770_device
{
public:
	fd1761_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1761;

class fd1762_device : public wd1770_device
{
public:
	fd1762_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1762;

class fd1763_device : public wd1770_device
{
public:
	fd1763_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1763;

class fd1764_device : public wd1770_device
{
public:
	fd1764_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1764;

class fd1765_device : public wd1770_device
{
public:
	fd1765_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1765;

class fd1767_device : public wd1770_device
{
public:
	fd1767_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type FD1767;

class wd2791_device : public wd1770_device
{
public:
	wd2791_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type WD2791;

class wd2793_device : public wd1770_device
{
public:
	wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type WD2793;

class wd2795_device : public wd1770_device
{
public:
	wd2795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type WD2795;

class wd2797_device : public wd1770_device
{
public:
	wd2797_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type WD2797;

class wd1772_device : public wd1770_device
{
public:
	wd1772_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern ATTR_DEPRECATED const device_type WD1772;

class wd1773_device : public wd1770_device
{
public:
	wd1773_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type WD1773;

class mb8866_device : public wd1770_device
{
public:
	mb8866_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type MB8866;

class mb8876_device : public wd1770_device
{
public:
	mb8876_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type MB8876;

class mb8877_device : public wd1770_device
{
public:
	mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern ATTR_DEPRECATED const device_type MB8877;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_FD1771_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1771, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1781_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1781, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1791_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1791, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1792_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1792, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1793_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1793, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1794_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1794, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1795_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1795, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1797_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1797, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1761_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1761, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1762_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1762, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1763_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1763, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1764_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1764, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1765_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1765, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_FD1767_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, FD1767, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD2793_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD2793, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD2795_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD2795, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD2797_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD2797, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1770_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1770, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1772_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1772, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_WD1773_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, WD1773, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8866_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8866, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8876_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8876, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MB8877_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MB8877, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __WD17XX_H__ */
