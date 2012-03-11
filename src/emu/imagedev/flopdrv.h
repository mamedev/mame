/* flopdrv provides simple emulation of a disc drive */
/* the 8271, upd765 and wd179x use this */

#ifndef __FLOPDRV_H__
#define __FLOPDRV_H__

#include "devcb.h"
#include "image.h"
#include "formats/flopimg.h"

#define FLOPPY_TYPE_REGULAR 0
#define FLOPPY_TYPE_APPLE	1
#define FLOPPY_TYPE_SONY	2

#define FLOPPY_DRIVE_2_8_INCH	1
#define FLOPPY_DRIVE_3_INCH		2
#define FLOPPY_DRIVE_3_5_INCH	3
#define FLOPPY_DRIVE_5_25_INCH	4
#define FLOPPY_DRIVE_8_INCH		5

// Maximum supported density
#define FLOPPY_DRIVE_SD 1
#define FLOPPY_DRIVE_DD 2
#define FLOPPY_DRIVE_QD 3
#define FLOPPY_DRIVE_HD 4
#define FLOPPY_DRIVE_ED 5

#define FLOPPY_STANDARD_3_SSDD       { FLOPPY_DRIVE_3_INCH,    1, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_DSDD       { FLOPPY_DRIVE_3_INCH,    2, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_5_SSDD     { FLOPPY_DRIVE_3_5_INCH,  1, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_5_DSDD     { FLOPPY_DRIVE_3_5_INCH,  2, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_5_DSHD     { FLOPPY_DRIVE_3_5_INCH,  2, 83, FLOPPY_DRIVE_HD }
#define FLOPPY_STANDARD_3_5_DSED     { FLOPPY_DRIVE_3_5_INCH,  2, 83, FLOPPY_DRIVE_ED }
#define FLOPPY_STANDARD_5_25_SSSD_35 { FLOPPY_DRIVE_5_25_INCH, 1, 37, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_DSSD_35 { FLOPPY_DRIVE_5_25_INCH, 2, 37, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_SSSD    { FLOPPY_DRIVE_5_25_INCH, 1, 42, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_DSSD    { FLOPPY_DRIVE_5_25_INCH, 2, 42, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_SSDD_40 { FLOPPY_DRIVE_5_25_INCH, 1, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_DSDD_40 { FLOPPY_DRIVE_5_25_INCH, 2, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_SSDD    { FLOPPY_DRIVE_5_25_INCH, 1, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_DSDD    { FLOPPY_DRIVE_5_25_INCH, 2, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_DSQD    { FLOPPY_DRIVE_5_25_INCH, 2, 83, FLOPPY_DRIVE_QD }
#define FLOPPY_STANDARD_5_25_DSHD    { FLOPPY_DRIVE_5_25_INCH, 2, 83, FLOPPY_DRIVE_HD }
#define FLOPPY_STANDARD_8_SSSD       { FLOPPY_DRIVE_8_INCH,    1, 77, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_8_DSSD       { FLOPPY_DRIVE_8_INCH,    2, 77, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_8_DSDD       { FLOPPY_DRIVE_8_INCH,    2, 77, FLOPPY_DRIVE_DD }

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> floppy_type_t

struct floppy_type_t
{
	UINT8 media_size;
	UINT8 head_number;
	UINT8 max_track_number;
	UINT8 max_density;
};

// ======================> floppy_interface

struct floppy_interface
{
	devcb_write_line out_idx_func;  /* index */
	devcb_read_line  in_mon_func;   /* motor on */
	devcb_write_line out_tk00_func; /* track 00 */
	devcb_write_line out_wpt_func;  /* write protect */
	devcb_write_line out_rdy_func;  /* ready */
//  devcb_write_line out_dskchg_func;  /* disk changed */

	floppy_type_t floppy_type;
	const struct FloppyFormat *formats;
	const char *interface;
	device_image_display_info_func	device_displayinfo;
};

struct chrn_id
{
	unsigned char C;
	unsigned char H;
	unsigned char R;
	unsigned char N;
	int data_id;			// id for read/write data command
	unsigned long flags;
};

/* set if drive is ready */
#define FLOPPY_DRIVE_READY						0x0010
/* set if index has just occurred */
#define FLOPPY_DRIVE_INDEX						0x0020

/* a callback which will be executed if the ready state of the drive changes e.g. not ready->ready, ready->not ready */
void floppy_drive_set_ready_state_change_callback(device_t *img, void (*callback)(device_t *controller,device_t *img, int state));

void floppy_drive_set_index_pulse_callback(device_t *img, void (*callback)(device_t *controller,device_t *image, int state));

/* set flag state */
int floppy_drive_get_flag_state(device_t *img, int flag);
/* get flag state */
void floppy_drive_set_flag_state(device_t *img, int flag, int state);
/* get current physical track drive is on */
int floppy_drive_get_current_track(device_t *img);
/* get current physical track size */
UINT64 floppy_drive_get_current_track_size(device_t *img, int head);

/* get next id from track, 1 if got a id, 0 if no id was got */
int floppy_drive_get_next_id(device_t *img, int side, chrn_id *);
/* set ready state of drive. If flag == 1, set ready state only if drive present,
disk is in drive, and motor is on. Otherwise set ready state to the state passed */
void floppy_drive_set_ready_state(device_t *img, int state, int flag);

/* seek up or down */
void floppy_drive_seek(device_t *img, signed int signed_tracks);

void floppy_drive_read_track_data_info_buffer(device_t *img, int side, void *ptr, int *length );
void floppy_drive_write_track_data_info_buffer(device_t *img, int side, const void *ptr, int *length );
void floppy_drive_format_sector(device_t *img, int side, int sector_index, int c, int h, int r, int n, int filler);
void floppy_drive_read_sector_data(device_t *img, int side, int index1, void *pBuffer, int length);
void floppy_drive_write_sector_data(device_t *img, int side, int index1, const void *pBuffer, int length, int ddam);

/* set motor speed to get correct index pulses
   standard RPM are 300 RPM (common) and 360 RPM
   Note: this actually only works for soft sectored disks: one index pulse per
   track.
*/
void floppy_drive_set_rpm(device_t *image, float rpm);

void floppy_drive_set_controller(device_t *img, device_t *controller);

floppy_image_legacy *flopimg_get_image(device_t *image);

/* hack for apple II; replace this when we think of something better */
void floppy_install_unload_proc(device_t *image, void (*proc)(device_image_interface &image));

void floppy_install_load_proc(device_t *image, void (*proc)(device_image_interface &image));

device_t *floppy_get_device(running_machine &machine,int drive);
device_t *floppy_get_device_by_type(running_machine &machine,int ftype,int drive);
int floppy_get_drive_type(device_t *image);
void floppy_set_type(device_t *image,int ftype);
int floppy_get_count(running_machine &machine);

int floppy_get_drive(device_t *image);
int floppy_get_drive_by_type(device_t *image,int ftype);

void *flopimg_get_custom_data(device_t *image);
void flopimg_alloc_custom_data(device_t *image,void *custom);

void floppy_drive_set_geometry(device_t *img, floppy_type_t type);

/* drive select lines */
WRITE_LINE_DEVICE_HANDLER( floppy_ds0_w );
WRITE_LINE_DEVICE_HANDLER( floppy_ds1_w );
WRITE_LINE_DEVICE_HANDLER( floppy_ds2_w );
WRITE_LINE_DEVICE_HANDLER( floppy_ds3_w );
WRITE8_DEVICE_HANDLER( floppy_ds_w );

WRITE_LINE_DEVICE_HANDLER( floppy_mon_w );
WRITE_LINE_DEVICE_HANDLER( floppy_drtn_w );
WRITE_LINE_DEVICE_HANDLER( floppy_stp_w );
WRITE_LINE_DEVICE_HANDLER( floppy_wtd_w );
WRITE_LINE_DEVICE_HANDLER( floppy_wtg_w );

/* write-protect */
READ_LINE_DEVICE_HANDLER( floppy_wpt_r );

/* track 0 detect */
READ_LINE_DEVICE_HANDLER( floppy_tk00_r );

/* disk changed */
READ_LINE_DEVICE_HANDLER( floppy_dskchg_r );

/* 2-sided disk */
READ_LINE_DEVICE_HANDLER( floppy_twosid_r );

// index pulse
READ_LINE_DEVICE_HANDLER( floppy_index_r );

// drive ready
READ_LINE_DEVICE_HANDLER( floppy_ready_r );

class legacy_floppy_image_device :	public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	legacy_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	legacy_floppy_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~legacy_floppy_image_device();

	virtual bool call_load();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) {	return load_software(swlist, swname, start_entry); }
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual void call_unload();
	virtual void call_display_info();

	virtual iodevice_t image_type() const { return IO_FLOPPY; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const;
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const;
	virtual const char *file_extensions() const { return m_extension_list; }
	virtual const option_guide *create_option_guide() const { return floppy_option_guide; }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device overrides
	virtual void device_config_complete();
	virtual void device_start();

	void *m_token;
	char			m_extension_list[256];
};

// device type definition
extern const device_type LEGACY_FLOPPY;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define FLOPPY_0 "floppy0"
#define FLOPPY_1 "floppy1"
#define FLOPPY_2 "floppy2"
#define FLOPPY_3 "floppy3"


#define MCFG_LEGACY_FLOPPY_DRIVE_ADD(_tag, _config)	\
	MCFG_DEVICE_ADD(_tag, LEGACY_FLOPPY, 0)			\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_LEGACY_FLOPPY_DRIVE_MODIFY(_tag, _config)	\
	MCFG_DEVICE_MODIFY(_tag)		\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_0, LEGACY_FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_1, LEGACY_FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_2, LEGACY_FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_3, LEGACY_FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_LEGACY_FLOPPY_4_DRIVES_MODIFY(_config)	\
	MCFG_DEVICE_MODIFY(FLOPPY_0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_MODIFY(FLOPPY_1)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_MODIFY(FLOPPY_2)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_MODIFY(FLOPPY_3)		\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_LEGACY_FLOPPY_4_DRIVES_REMOVE()	\
	MCFG_DEVICE_REMOVE(FLOPPY_0)		\
	MCFG_DEVICE_REMOVE(FLOPPY_1)		\
	MCFG_DEVICE_REMOVE(FLOPPY_2)		\
	MCFG_DEVICE_REMOVE(FLOPPY_3)

#define MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_0, LEGACY_FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_1, LEGACY_FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_LEGACY_FLOPPY_2_DRIVES_MODIFY(_config)	\
	MCFG_DEVICE_MODIFY(FLOPPY_0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_MODIFY(FLOPPY_1)		\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_LEGACY_FLOPPY_2_DRIVES_REMOVE()	\
	MCFG_DEVICE_REMOVE(FLOPPY_0)		\
	MCFG_DEVICE_REMOVE(FLOPPY_1)

#endif /* __FLOPDRV_H__ */
