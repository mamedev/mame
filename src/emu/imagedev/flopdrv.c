/*
    This code handles the floppy drives.
    All FDD actions should be performed using these functions.

    The functions are emulated and a disk image is used.

  Real disk operation:
  - set unit id

  TODO:
    - Override write protect if disk image has been opened in read mode
*/

#include "emu.h"
#include "formats/imageutl.h"
#include "flopdrv.h"

#define VERBOSE     0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define FLOPDRVTAG  "flopdrv"
#define LOG_FLOPPY      0

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct floppy_drive
{
	/* callbacks */
	devcb_resolved_write_line out_idx_func;
	devcb_resolved_read_line in_mon_func;
	devcb_resolved_write_line out_tk00_func;
	devcb_resolved_write_line out_wpt_func;
	devcb_resolved_write_line out_rdy_func;
	devcb_resolved_write_line out_dskchg_func;

	/* state of input lines */
	int drtn; /* direction */
	int stp;  /* step */
	int wtg;  /* write gate */
	int mon;  /* motor on */

	/* state of output lines */
	int idx;  /* index pulse */
	int tk00; /* track 00 */
	int wpt;  /* write protect */
	int rdy;  /* ready */
	int dskchg;     /* disk changed */

	/* drive select logic */
	int drive_id;
	int active;

	const floppy_interface  *config;

	/* flags */
	int flags;
	/* maximum track allowed */
	int max_track;
	/* num sides */
	int num_sides;
	/* current track - this may or may not relate to the present cylinder number
	stored by the fdc */
	int current_track;

	/* index pulse timer */
	emu_timer   *index_timer;
	/* index pulse callback */
	void    (*index_pulse_callback)(device_t *controller,device_t *image, int state);
	/* rotation per minute => gives index pulse frequency */
	float rpm;

	void    (*ready_state_change_callback)(device_t *controller,device_t *img, int state);

	int id_index;

	device_t *controller;

	floppy_image_legacy *floppy;
	int track;
	void (*load_proc)(device_image_interface &image);
	void (*unload_proc)(device_image_interface &image);
	int floppy_drive_type;
};


struct floppy_error_map
{
	floperr_t ferr;
	image_error_t ierr;
	const char *message;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const floppy_error_map errmap[] =
{
	{ FLOPPY_ERROR_SUCCESS,         IMAGE_ERROR_SUCCESS },
	{ FLOPPY_ERROR_INTERNAL,        IMAGE_ERROR_INTERNAL },
	{ FLOPPY_ERROR_UNSUPPORTED,     IMAGE_ERROR_UNSUPPORTED },
	{ FLOPPY_ERROR_OUTOFMEMORY,     IMAGE_ERROR_OUTOFMEMORY },
	{ FLOPPY_ERROR_INVALIDIMAGE,    IMAGE_ERROR_INVALIDIMAGE }
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

INLINE floppy_drive *get_safe_token(device_t *device)
{
	assert( device != NULL );
	return (floppy_drive *) downcast<legacy_floppy_image_device *>(device)->token();
}

floppy_image_legacy *flopimg_get_image(device_t *image)
{
	return get_safe_token(image)->floppy;
}

static int flopimg_get_sectors_per_track(device_t *image, int side)
{
	floperr_t err;
	int sector_count;
	floppy_drive *flopimg = get_safe_token( image );

	if (!flopimg || !flopimg->floppy)
		return 0;

	err = floppy_get_sector_count(flopimg->floppy, side, flopimg->track, &sector_count);
	if (err)
		return 0;
	return sector_count;
}

static void flopimg_get_id_callback(device_t *image, chrn_id *id, int id_index, int side)
{
	floppy_drive *flopimg;
	int cylinder, sector, N;
	unsigned long flags;
	UINT32 sector_length;

	flopimg = get_safe_token( image );
	if (!flopimg || !flopimg->floppy)
		return;

	floppy_get_indexed_sector_info(flopimg->floppy, side, flopimg->track, id_index, &cylinder, &side, &sector, &sector_length, &flags);

	N = compute_log2(sector_length);

	id->C = cylinder;
	id->H = side;
	id->R = sector;
	id->data_id = id_index;
	id->flags = flags;
	id->N = ((N >= 7) && (N <= 10)) ? N - 7 : 0;
}

static void log_readwrite(const char *name, int head, int track, int sector, const char *buf, int length)
{
	char membuf[1024];
	int i;
	for (i = 0; i < length; i++)
		sprintf(membuf + i*2, "%02x", (int) (UINT8) buf[i]);
	logerror("%s:  head=%i track=%i sector=%i buffer='%s'\n", name, head, track, sector, membuf);
}

static void floppy_drive_set_geometry_absolute(device_t *img, int tracks, int sides)
{
	floppy_drive *pDrive = get_safe_token( img );
	pDrive->max_track = tracks;
	pDrive->num_sides = sides;
}

void floppy_drive_set_geometry(device_t *img, floppy_type_t type)
{
	floppy_drive_set_geometry_absolute(img, type.max_track_number, type.head_number);
}

static TIMER_CALLBACK(floppy_drive_index_callback);

/* this is called on device init */
static void floppy_drive_init(device_t *img)
{
	floppy_drive *pDrive = get_safe_token( img );

	/* initialise flags */
	pDrive->flags = 0;
	pDrive->index_pulse_callback = NULL;
	pDrive->ready_state_change_callback = NULL;
	pDrive->index_timer = img->machine().scheduler().timer_alloc(FUNC(floppy_drive_index_callback), (void *) img);
	pDrive->idx = 0;

	floppy_drive_set_geometry(img, ((floppy_interface*)img->static_config())->floppy_type);

	/* initialise id index - not so important */
	pDrive->id_index = 0;
	/* initialise track */
	pDrive->current_track = 0;

	/* default RPM */
	pDrive->rpm = 300;

	pDrive->controller = NULL;

	pDrive->floppy_drive_type = FLOPPY_TYPE_REGULAR;
}

/* index pulses at rpm/60 Hz, and stays high 1/20th of time */
static void floppy_drive_index_func(device_t *img)
{
	floppy_drive *drive = get_safe_token( img );

	double ms = 1000.0 / (drive->rpm / 60.0);

	if (drive->idx)
	{
		drive->idx = 0;
		drive->index_timer->adjust(attotime::from_double(ms*19/20/1000.0));
	}
	else
	{
		drive->idx = 1;
		drive->index_timer->adjust(attotime::from_double(ms/20/1000.0));
	}

	drive->out_idx_func(drive->idx);

	if (drive->index_pulse_callback)
		drive->index_pulse_callback(drive->controller, img, drive->idx);
}

static TIMER_CALLBACK(floppy_drive_index_callback)
{
	device_t *image = (device_t *) ptr;
	floppy_drive_index_func(image);
}

/*************************************************************************/
/* IO_FLOPPY device functions */

/* set flag state */
void floppy_drive_set_flag_state(device_t *img, int flag, int state)
{
	floppy_drive *drv = get_safe_token( img );
	int prev_state;
	int new_state;

	/* get old state */
	prev_state = drv->flags & flag;

	/* set new state */
	drv->flags &= ~flag;
	if (state)
		drv->flags |= flag;

	/* get new state */
	new_state = drv->flags & flag;

	/* changed state? */
	if (prev_state ^ new_state)
	{
		if (flag & FLOPPY_DRIVE_READY)
		{
			/* trigger state change callback */
			drv->out_rdy_func(new_state ? ASSERT_LINE : CLEAR_LINE);

			if (drv->ready_state_change_callback)
				drv->ready_state_change_callback(drv->controller, img, new_state);
		}
	}
}

/* for pc, drive is always ready, for amstrad,pcw,spectrum it is only ready under
a fixed set of circumstances */
/* use this to set ready state of drive */
void floppy_drive_set_ready_state(device_t *img, int state, int flag)
{
	floppy_drive *drive = get_safe_token(img);
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);
	if (flag)
	{
		/* set ready only if drive is present, disk is in the drive,
		and disk motor is on - for Amstrad, Spectrum and PCW*/

		/* drive present? */
		/* disk inserted? */
		if (image->exists())
		{
			if (drive->mon == CLEAR_LINE)
			{
				/* set state */
				floppy_drive_set_flag_state(img, FLOPPY_DRIVE_READY, state);
				return;
			}
		}
		floppy_drive_set_flag_state(img, FLOPPY_DRIVE_READY, 0);
	}
	else
	{
		/* force ready state - for PC driver */
		floppy_drive_set_flag_state(img, FLOPPY_DRIVE_READY, state);
	}
}

/* get flag state */
int floppy_drive_get_flag_state(device_t *img, int flag)
{
	floppy_drive *drv = get_safe_token( img );
	int drive_flags;
	int flags;

	flags = 0;

	drive_flags = drv->flags;

	/* these flags are independent of a real drive/disk image */
	flags |= drive_flags & (FLOPPY_DRIVE_READY | FLOPPY_DRIVE_INDEX);

	flags &= flag;

	return flags;
}


void floppy_drive_seek(device_t *img, signed int signed_tracks)
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);
	floppy_drive *pDrive;

	pDrive = get_safe_token( img );

	LOG(("seek from: %d delta: %d\n",pDrive->current_track, signed_tracks));

	/* update position */
	pDrive->current_track+=signed_tracks;

	if (pDrive->current_track<0)
	{
		pDrive->current_track = 0;
	}
	else
	if (pDrive->current_track>=pDrive->max_track)
	{
		pDrive->current_track = pDrive->max_track-1;
	}

	/* set track 0 flag */
	pDrive->tk00 = (pDrive->current_track == 0) ? CLEAR_LINE : ASSERT_LINE;
	pDrive->out_tk00_func(pDrive->tk00);

	/* clear disk changed flag */
	pDrive->dskchg = ASSERT_LINE;
	//flopimg->out_dskchg_func(flopimg->dskchg);

	/* inform disk image of step operation so it can cache information */
	if (image->exists())
		pDrive->track = pDrive->current_track;

	pDrive->id_index = 0;
}


/* this is not accurate. But it will do for now */
int floppy_drive_get_next_id(device_t *img, int side, chrn_id *id)
{
	floppy_drive *pDrive;
	int spt;

	pDrive = get_safe_token( img );

	/* get sectors per track */
	spt = flopimg_get_sectors_per_track(img, side);

	/* set index */
	if ((pDrive->id_index==(spt-1)) || (spt==0))
	{
		floppy_drive_set_flag_state(img, FLOPPY_DRIVE_INDEX, 1);
	}
	else
	{
		floppy_drive_set_flag_state(img, FLOPPY_DRIVE_INDEX, 0);
	}

	/* get id */
	if (spt!=0)
	{
		flopimg_get_id_callback(img, id, pDrive->id_index, side);
	}

	pDrive->id_index++;
	if (spt!=0)
		pDrive->id_index %= spt;
	else
		pDrive->id_index = 0;

	return (spt == 0) ? 0 : 1;
}

void floppy_drive_read_track_data_info_buffer(device_t *img, int side, void *ptr, int *length )
{
	floppy_drive *flopimg;
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	if (image->exists())
	{
		flopimg = get_safe_token( img );
		if (!flopimg || !flopimg->floppy)
			return;

		floppy_read_track_data(flopimg->floppy, side, flopimg->track, ptr, *length);
	}
}

void floppy_drive_write_track_data_info_buffer(device_t *img, int side, const void *ptr, int *length )
{
	floppy_drive *flopimg;
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	if (image->exists())
	{
		flopimg = get_safe_token( img );
		if (!flopimg || !flopimg->floppy)
			return;

		floppy_write_track_data(flopimg->floppy, side, flopimg->track, ptr, *length);
	}
}

void floppy_drive_format_sector(device_t *img, int side, int sector_index,int c,int h, int r, int n, int filler)
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	if (image->exists())
	{
/*      if (drv->interface_.format_sector)
            drv->interface_.format_sector(img, side, sector_index,c, h, r, n, filler);*/
	}
}

void floppy_drive_read_sector_data(device_t *img, int side, int index1, void *ptr, int length)
{
	floppy_drive *flopimg;
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	if (image->exists())
	{
		flopimg = get_safe_token( img );
		if (!flopimg || !flopimg->floppy)
			return;

		floppy_read_indexed_sector(flopimg->floppy, side, flopimg->track, index1, 0, ptr, length);

		if (LOG_FLOPPY)
			log_readwrite("sector_read", side, flopimg->track, index1, (const char *)ptr, length);

	}
}

void floppy_drive_write_sector_data(device_t *img, int side, int index1, const void *ptr,int length, int ddam)
{
	floppy_drive *flopimg;
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	if (image->exists())
	{
		flopimg = get_safe_token( img );
		if (!flopimg || !flopimg->floppy)
			return;

		if (LOG_FLOPPY)
			log_readwrite("sector_write", side, flopimg->track, index1, (const char *)ptr, length);

		floppy_write_indexed_sector(flopimg->floppy, side, flopimg->track, index1, 0, ptr, length, ddam);
	}
}

void floppy_install_load_proc(device_t *image, void (*proc)(device_image_interface &image))
{
	floppy_drive *flopimg = get_safe_token( image );
	flopimg->load_proc = proc;
}

void floppy_install_unload_proc(device_t *image, void (*proc)(device_image_interface &image))
{
	floppy_drive *flopimg = get_safe_token( image );
	flopimg->unload_proc = proc;
}

/* set the callback for the index pulse */
void floppy_drive_set_index_pulse_callback(device_t *img, void (*callback)(device_t *controller,device_t *image, int state))
{
	floppy_drive *pDrive = get_safe_token( img );
	pDrive->index_pulse_callback = callback;
}


void floppy_drive_set_ready_state_change_callback(device_t *img, void (*callback)(device_t *controller,device_t *img, int state))
{
	floppy_drive *pDrive = get_safe_token( img );
	pDrive->ready_state_change_callback = callback;
}

int floppy_drive_get_current_track(device_t *img)
{
	floppy_drive *drv = get_safe_token( img );
	return drv->current_track;
}

UINT64 floppy_drive_get_current_track_size(device_t *img, int head)
{
	floppy_drive *drv = get_safe_token( img );
	int size = 0;
	device_image_interface *image = dynamic_cast<device_image_interface *>(img);

	if (image->exists())
	{
		size = floppy_get_track_size(drv->floppy, head, drv->current_track);
	}

	return size;
}

void floppy_drive_set_rpm(device_t *img, float rpm)
{
	floppy_drive *drv = get_safe_token( img );
	drv->rpm = rpm;
}

void floppy_drive_set_controller(device_t *img, device_t *controller)
{
	floppy_drive *drv = get_safe_token( img );
	drv->controller = controller;
}

static int internal_floppy_device_load(device_image_interface *image, int create_format, option_resolution *create_args)
{
	floperr_t err;
	floppy_drive *flopimg;
	const struct FloppyFormat *floppy_options;
	int floppy_flags, i;
	const char *extension;

	/* look up instance data */
	flopimg = get_safe_token( &image->device() );

	/* figure out the floppy options */
	floppy_options = ((floppy_interface*)image->device().static_config())->formats;

	if (image->has_been_created())
	{
		/* creating an image */
		assert(create_format >= 0);
		err = floppy_create((void *) image, &image_ioprocs, &floppy_options[create_format], create_args, &flopimg->floppy);
		if (err)
			goto error;
	}
	else
	{
		/* opening an image */
		floppy_flags = !image->is_readonly() ? FLOPPY_FLAGS_READWRITE : FLOPPY_FLAGS_READONLY;
		extension = image->filetype();
		err = floppy_open_choices((void *) image, &image_ioprocs, extension, floppy_options, floppy_flags, &flopimg->floppy);
		if (err)
			goto error;
	}
	if (floppy_callbacks(flopimg->floppy)->get_heads_per_disk && floppy_callbacks(flopimg->floppy)->get_tracks_per_disk)
	{
		floppy_drive_set_geometry_absolute(&image->device(),
			floppy_get_tracks_per_disk(flopimg->floppy),
			floppy_get_heads_per_disk(flopimg->floppy));
	}
	/* disk changed */
	flopimg->dskchg = CLEAR_LINE;

	return IMAGE_INIT_PASS;

error:
	for (i = 0; i < ARRAY_LENGTH(errmap); i++)
	{
		if (err == errmap[i].ferr)
			image->seterror(errmap[i].ierr, errmap[i].message);
	}
	return IMAGE_INIT_FAIL;
}

static TIMER_CALLBACK( set_wpt )
{
	floppy_drive *flopimg = (floppy_drive *)ptr;

	flopimg->wpt = param;
	flopimg->out_wpt_func(param);
}

device_t *floppy_get_device(running_machine &machine,int drive)
{
	switch(drive) {
		case 0 : return machine.device(FLOPPY_0);
		case 1 : return machine.device(FLOPPY_1);
		case 2 : return machine.device(FLOPPY_2);
		case 3 : return machine.device(FLOPPY_3);
	}
	return NULL;
}

int floppy_get_drive_type(device_t *image)
{
	floppy_drive *flopimg = get_safe_token( image );
	return flopimg->floppy_drive_type;
}

void floppy_set_type(device_t *image,int ftype)
{
	floppy_drive *flopimg = get_safe_token( image );
	flopimg->floppy_drive_type = ftype;
}

device_t *floppy_get_device_by_type(running_machine &machine,int ftype,int drive)
{
	int i;
	int cnt = 0;
	for (i=0;i<4;i++) {
		device_t *disk = floppy_get_device(machine,i);
		if (floppy_get_drive_type(disk)==ftype) {
			if (cnt==drive) {
				return disk;
			}
			cnt++;
		}
	}
	return NULL;
}

int floppy_get_drive(device_t *image)
{
	int drive =0;
	if (strcmp(image->tag(), ":" FLOPPY_0) == 0) drive = 0;
	if (strcmp(image->tag(), ":" FLOPPY_1) == 0) drive = 1;
	if (strcmp(image->tag(), ":" FLOPPY_2) == 0) drive = 2;
	if (strcmp(image->tag(), ":" FLOPPY_3) == 0) drive = 3;
	return drive;
}

int floppy_get_drive_by_type(device_t *image,int ftype)
{
	int i,drive =0;
	for (i=0;i<4;i++) {
		device_t *disk = floppy_get_device(image->machine(),i);
		if (floppy_get_drive_type(disk)==ftype) {
			if (image==disk) {
				return drive;
			}
			drive++;
		}
	}
	return drive;
}

int floppy_get_count(running_machine &machine)
{
	int cnt = 0;
	if (machine.device(FLOPPY_0)) cnt++;
	if (machine.device(FLOPPY_1)) cnt++;
	if (machine.device(FLOPPY_2)) cnt++;
	if (machine.device(FLOPPY_3)) cnt++;
	return cnt;
}


/* drive select 0 */
WRITE_LINE_DEVICE_HANDLER( floppy_ds0_w )
{
	floppy_drive *drive = get_safe_token(device);

	if (state == CLEAR_LINE)
		drive->active = (drive->drive_id == 0);
}

/* drive select 1 */
WRITE_LINE_DEVICE_HANDLER( floppy_ds1_w )
{
	floppy_drive *drive = get_safe_token(device);

	if (state == CLEAR_LINE)
		drive->active = (drive->drive_id == 1);
}

/* drive select 2 */
WRITE_LINE_DEVICE_HANDLER( floppy_ds2_w )
{
	floppy_drive *drive = get_safe_token(device);

	if (state == CLEAR_LINE)
		drive->active = (drive->drive_id == 2);
}

/* drive select 3 */
WRITE_LINE_DEVICE_HANDLER( floppy_ds3_w )
{
	floppy_drive *drive = get_safe_token(device);

	if (state == CLEAR_LINE)
		drive->active = (drive->drive_id == 3);
}

/* shortcut to write all four ds lines */
WRITE8_DEVICE_HANDLER( floppy_ds_w )
{
	floppy_ds0_w(device, BIT(data, 0));
	floppy_ds1_w(device, BIT(data, 1));
	floppy_ds2_w(device, BIT(data, 2));
	floppy_ds3_w(device, BIT(data, 3));
}

/* motor on, active low */
WRITE_LINE_DEVICE_HANDLER( floppy_mon_w )
{
	floppy_drive *drive = get_safe_token(device);
	device_image_interface *image = dynamic_cast<device_image_interface *>(device);
	/* force off if there is no attached image */
	if (!image->exists())
		state = ASSERT_LINE;

	/* off -> on */
	if (drive->mon && state == CLEAR_LINE)
	{
		drive->idx = 0;
		floppy_drive_index_func(device);
	}

	/* on -> off */
	else if (drive->mon == CLEAR_LINE && state)
		drive->index_timer->adjust(attotime::zero);

	drive->mon = state;
}

/* direction */
WRITE_LINE_DEVICE_HANDLER( floppy_drtn_w )
{
	floppy_drive *drive = get_safe_token(device);
	drive->drtn = state;
}

/* write data */
WRITE_LINE_DEVICE_HANDLER( floppy_wtd_w )
{

}

/* step */
WRITE_LINE_DEVICE_HANDLER( floppy_stp_w )
{
	floppy_drive *drive = get_safe_token(device);

	/* move head one track when going from high to low and write gate is high */
	if (drive->active && drive->stp && state == CLEAR_LINE && drive->wtg)
	{
		/* move head according to the direction line */
		if (drive->drtn)
		{
			/* move head outward */
			if (drive->current_track > 0)
				drive->current_track--;

			/* are we at track 0 now? */
			drive->tk00 = (drive->current_track == 0) ? CLEAR_LINE : ASSERT_LINE;
		}
		else
		{
			/* move head inward */
			if (drive->current_track < drive->max_track)
				drive->current_track++;

			/* we can't be at track 0 here, so reset the line */
			drive->tk00 = ASSERT_LINE;
		}

		/* update track 0 line with new status */
		drive->out_tk00_func(drive->tk00);
	}

	drive->stp = state;
}

/* write gate */
WRITE_LINE_DEVICE_HANDLER( floppy_wtg_w )
{
	floppy_drive *drive = get_safe_token(device);
	drive->wtg = state;
}

/* write protect signal, active low */
READ_LINE_DEVICE_HANDLER( floppy_wpt_r )
{
	floppy_drive *drive = get_safe_token(device);
	return drive->wpt;
}

/* track 0 detect */
READ_LINE_DEVICE_HANDLER( floppy_tk00_r )
{
	floppy_drive *drive = get_safe_token(device);
	return drive->tk00;
}

/* disk changed */
READ_LINE_DEVICE_HANDLER( floppy_dskchg_r )
{
	floppy_drive *drive = get_safe_token(device);
	return drive->dskchg;
}

/* 2-sided disk */
READ_LINE_DEVICE_HANDLER( floppy_twosid_r )
{
	floppy_drive *drive = get_safe_token(device);

	if (drive->floppy == NULL)
		return ASSERT_LINE;
	else
		return !floppy_get_heads_per_disk(drive->floppy);
}

READ_LINE_DEVICE_HANDLER( floppy_index_r )
{
	floppy_drive *drive = get_safe_token(device);
	return drive->idx;
}

READ_LINE_DEVICE_HANDLER( floppy_ready_r )
{
	return !(floppy_drive_get_flag_state(device, FLOPPY_DRIVE_READY) == FLOPPY_DRIVE_READY);
}

// device type definition
const device_type LEGACY_FLOPPY = &device_creator<legacy_floppy_image_device>;

//-------------------------------------------------
//  legacy_floppy_image_device - constructor
//-------------------------------------------------

legacy_floppy_image_device::legacy_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LEGACY_FLOPPY, "Floppy Disk", tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_token(NULL)
{
	m_token = global_alloc_clear(floppy_drive);
}

legacy_floppy_image_device::legacy_floppy_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_token(NULL)
{
	m_token = global_alloc_clear(floppy_drive);
}

//-------------------------------------------------
//  legacy_floppy_image_device - destructor
//-------------------------------------------------

legacy_floppy_image_device::~legacy_floppy_image_device()
{
	global_free(m_token);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void legacy_floppy_image_device::device_start()
{
	floppy_drive *floppy = get_safe_token( this );
	floppy->config = (const floppy_interface*)static_config();
	floppy_drive_init(this);

	floppy->drive_id = floppy_get_drive(this);
	floppy->active = FALSE;

	/* resolve callbacks */
	floppy->out_idx_func.resolve(floppy->config->out_idx_func, *this);
	floppy->in_mon_func.resolve(floppy->config->in_mon_func, *this);
	floppy->out_tk00_func.resolve(floppy->config->out_tk00_func, *this);
	floppy->out_wpt_func.resolve(floppy->config->out_wpt_func, *this);
	floppy->out_rdy_func.resolve(floppy->config->out_rdy_func, *this);
//  floppy->out_dskchg_func.resolve(floppy->config->out_dskchg_func, *this);

	/* by default we are not write-protected */
	floppy->wpt = ASSERT_LINE;
	floppy->out_wpt_func(floppy->wpt);

	/* not at track 0 */
	floppy->tk00 = ASSERT_LINE;
	floppy->out_tk00_func(floppy->tk00);

	/* motor off */
	floppy->mon = ASSERT_LINE;

	/* disk changed */
	floppy->dskchg = CLEAR_LINE;
//  floppy->out_dskchg_func(floppy->dskchg);
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void legacy_floppy_image_device::device_config_complete()
{
	image_device_format **formatptr;
	image_device_format *format;
	formatptr = &m_formatlist;
	int cnt = 0;

	m_extension_list[0] = '\0';
	const struct FloppyFormat *floppy_options = ((floppy_interface*)static_config())->formats;
	for (int i = 0; floppy_options[i].construct; i++)
	{
		// only add if creatable
		if (floppy_options[i].param_guidelines) {
			// allocate a new format
			format = global_alloc_clear(image_device_format);

			// populate it
			format->m_index       = cnt;
			format->m_name        = floppy_options[i].name;
			format->m_description = floppy_options[i].description;
			format->m_extensions  = floppy_options[i].extensions;
			format->m_optspec     = floppy_options[i].param_guidelines;

			// and append it to the list
			*formatptr = format;
			formatptr = &format->m_next;
			cnt++;
		}
		image_specify_extension( m_extension_list, 256, floppy_options[i].extensions );
	}

	// set brief and instance name
	update_names();
}

bool legacy_floppy_image_device::call_create(int format_type, option_resolution *format_options)
{
	return internal_floppy_device_load(this, format_type, format_options);
}

bool legacy_floppy_image_device::call_load()
{
	floppy_drive *flopimg;
	int retVal = internal_floppy_device_load(this, -1, NULL);
	flopimg = get_safe_token( this);
	if (retVal==IMAGE_INIT_PASS) {
		/* if we have one of our hacky unload procs, call it */
		if (flopimg->load_proc)
			flopimg->load_proc(*this);
	}

	/* push disk halfway into drive */
	flopimg->wpt = CLEAR_LINE;
	flopimg->out_wpt_func(flopimg->wpt);

	/* set timer for disk load */
	int next_wpt;

	if (!is_readonly())
		next_wpt = ASSERT_LINE;
	else
		next_wpt = CLEAR_LINE;

	machine().scheduler().timer_set(attotime::from_msec(250), FUNC(set_wpt), next_wpt, flopimg);

	return retVal;
}

void legacy_floppy_image_device::call_unload()
{
	floppy_drive *flopimg = get_safe_token( this);
	if (flopimg->unload_proc)
		flopimg->unload_proc(*this);

	floppy_close(flopimg->floppy);
	flopimg->floppy = NULL;

	/* disk changed */
	flopimg->dskchg = CLEAR_LINE;
	//flopimg->out_dskchg_func(flopimg->dskchg);

	/* pull disk halfway out of drive */
	flopimg->wpt = CLEAR_LINE;
	flopimg->out_wpt_func(flopimg->wpt);

	/* set timer for disk eject */
	machine().scheduler().timer_set(attotime::from_msec(250), FUNC(set_wpt), ASSERT_LINE, flopimg);
}

void legacy_floppy_image_device::call_display_info()
{
	if (((floppy_interface*)(this)->static_config())->device_displayinfo) {
		((floppy_interface*)(this)->static_config())->device_displayinfo(*this);
	}
}

bool legacy_floppy_image_device::is_creatable() const
{
	int cnt = 0;
	if (static_config() )
	{
		const struct FloppyFormat *floppy_options = ((floppy_interface*)static_config())->formats;
		int i;
		for ( i = 0; floppy_options[i].construct; i++ ) {
			if(floppy_options[i].param_guidelines) cnt++;
		}
	}
	return (cnt>0) ? 1 : 0;
}

const char *legacy_floppy_image_device::image_interface() const
{
	return ((floppy_interface *)static_config())->interface;
}
