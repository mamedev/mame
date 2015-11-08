// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
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
#include "image.h"

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


floppy_image_legacy *legacy_floppy_image_device::flopimg_get_image()
{
	return m_floppy;
}

int legacy_floppy_image_device::flopimg_get_sectors_per_track(int side)
{
	floperr_t err;
	int sector_count;

	if (!m_floppy)
		return 0;

	err = floppy_get_sector_count(m_floppy, side, m_track, &sector_count);
	if (err)
		return 0;
	return sector_count;
}

void legacy_floppy_image_device::flopimg_get_id_callback(chrn_id *id, int id_index, int side)
{
	int cylinder, sector, N;
	unsigned long flags;
	UINT32 sector_length;

	if (!m_floppy)
		return;

	floppy_get_indexed_sector_info(m_floppy, side, m_track, id_index, &cylinder, &side, &sector, &sector_length, &flags);

	N = compute_log2(sector_length);

	id->C = cylinder;
	id->H = side;
	id->R = sector;
	id->data_id = id_index;
	id->flags = flags;
	id->N = ((N >= 7) && (N <= 10)) ? N - 7 : 0;
}

void legacy_floppy_image_device::log_readwrite(const char *name, int head, int track, int sector, const char *buf, int length)
{
	char membuf[1024];
	int i;
	for (i = 0; i < length; i++)
		sprintf(membuf + i*2, "%02x", (int) (UINT8) buf[i]);
	logerror("%s:  head=%i track=%i sector=%i buffer='%s'\n", name, head, track, sector, membuf);
}

void legacy_floppy_image_device::floppy_drive_set_geometry_absolute(int tracks, int sides)
{
	m_max_track = tracks;
	m_num_sides = sides;
}

void legacy_floppy_image_device::floppy_drive_set_geometry(floppy_type_t type)
{
	floppy_drive_set_geometry_absolute(type.max_track_number, type.head_number);
}

/* this is called on device init */
void legacy_floppy_image_device::floppy_drive_init()
{
	/* initialise flags */
	m_flags = 0;
	m_index_pulse_callback = NULL;
	m_index_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(legacy_floppy_image_device::floppy_drive_index_callback),this));
	m_idx = 0;

	floppy_drive_set_geometry(((floppy_interface*)static_config())->floppy_type);

	/* initialise id index - not so important */
	m_id_index = 0;
	/* initialise track */
	m_current_track = 0;

	/* default RPM */
	m_rpm = 300;

	m_controller = NULL;

	m_floppy_drive_type = FLOPPY_TYPE_REGULAR;
}

/* index pulses at rpm/60 Hz, and stays high 1/20th of time */
void legacy_floppy_image_device::floppy_drive_index_func()
{
	double ms = 1000.0 / ((double) m_rpm / 60.0);

	if (m_idx)
	{
		m_idx = 0;
		m_index_timer->adjust(attotime::from_double(ms*19/20/1000.0));
	}
	else
	{
		m_idx = 1;
		m_index_timer->adjust(attotime::from_double(ms/20/1000.0));
	}

	m_out_idx_func(m_idx);

	if (m_index_pulse_callback)
		m_index_pulse_callback(m_controller, this, m_idx);
}

TIMER_CALLBACK_MEMBER(legacy_floppy_image_device::floppy_drive_index_callback)
{
	floppy_drive_index_func();
}

/*************************************************************************/
/* IO_FLOPPY device functions */

/* set flag state */
void legacy_floppy_image_device::floppy_drive_set_flag_state(int flag, int state)
{
	int prev_state;
	int new_state;

	/* get old state */
	prev_state = m_flags & flag;

	/* set new state */
	m_flags &= ~flag;
	if (state)
		m_flags |= flag;

	/* get new state */
	new_state = m_flags & flag;

	/* changed state? */
	if (prev_state ^ new_state)
	{
		if (flag & FLOPPY_DRIVE_READY)
		{
			/* trigger state change callback */
			//m_out_rdy_func(new_state ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

/* for pc, drive is always ready, for amstrad,pcw,spectrum it is only ready under
a fixed set of circumstances */
/* use this to set ready state of drive */
void legacy_floppy_image_device::floppy_drive_set_ready_state(int state, int flag)
{
	if (flag)
	{
		/* set ready only if drive is present, disk is in the drive,
		and disk motor is on - for Amstrad, Spectrum and PCW*/

		/* drive present? */
		/* disk inserted? */
		if (exists())
		{
			if (m_mon == CLEAR_LINE)
			{
				/* set state */
				floppy_drive_set_flag_state(FLOPPY_DRIVE_READY, state);
				return;
			}
		}
		floppy_drive_set_flag_state(FLOPPY_DRIVE_READY, 0);
	}
	else
	{
		/* force ready state - for PC driver */
		floppy_drive_set_flag_state(FLOPPY_DRIVE_READY, state);
	}
}

/* get flag state */
int legacy_floppy_image_device::floppy_drive_get_flag_state(int flag)
{
	int drive_flags;
	int flags;

	flags = 0;

	drive_flags = m_flags;

	/* these flags are independent of a real drive/disk image */
	flags |= drive_flags & (FLOPPY_DRIVE_READY | FLOPPY_DRIVE_INDEX);

	flags &= flag;

	return flags;
}


void legacy_floppy_image_device::floppy_drive_seek(signed int signed_tracks)
{
	LOG(("seek from: %d delta: %d\n",m_current_track, signed_tracks));

	/* update position */
	m_current_track+=signed_tracks;

	if (m_current_track<0)
	{
		m_current_track = 0;
	}
	else
	if (m_current_track>=m_max_track)
	{
		m_current_track = m_max_track-1;
	}

	/* set track 0 flag */
	m_tk00 = (m_current_track == 0) ? CLEAR_LINE : ASSERT_LINE;
	//m_out_tk00_func(m_tk00);

	/* clear disk changed flag */
	m_dskchg = ASSERT_LINE;
	//m_out_dskchg_func(m_dskchg);

	/* inform disk image of step operation so it can cache information */
	if (exists())
		m_track = m_current_track;

	m_id_index = 0;
}


/* this is not accurate. But it will do for now */
int legacy_floppy_image_device::floppy_drive_get_next_id(int side, chrn_id *id)
{
	int spt;

	/* get sectors per track */
	spt = flopimg_get_sectors_per_track(side);

	/* set index */
	if ((m_id_index==(spt-1)) || (spt==0))
	{
		floppy_drive_set_flag_state(FLOPPY_DRIVE_INDEX, 1);
	}
	else
	{
		floppy_drive_set_flag_state(FLOPPY_DRIVE_INDEX, 0);
	}

	/* get id */
	if (spt!=0)
	{
		flopimg_get_id_callback(id, m_id_index, side);
	}

	m_id_index++;
	if (spt!=0)
		m_id_index %= spt;
	else
		m_id_index = 0;

	return (spt == 0) ? 0 : 1;
}

void legacy_floppy_image_device::floppy_drive_read_track_data_info_buffer(int side, void *ptr, int *length )
{
	if (exists())
	{
		if (!m_floppy)
			return;

		floppy_read_track_data(m_floppy, side, m_track, ptr, *length);
	}
}

void legacy_floppy_image_device::floppy_drive_write_track_data_info_buffer(int side, const void *ptr, int *length )
{
	if (exists())
	{
		if (!m_floppy)
			return;

		floppy_write_track_data(m_floppy, side, m_track, ptr, *length);
	}
}

void legacy_floppy_image_device::floppy_drive_format_sector(int side, int sector_index,int c,int h, int r, int n, int filler)
{
	if (exists())
	{
/*      if (m_interface_.format_sector)
            m_interface_.format_sector(img, side, sector_index,c, h, r, n, filler);*/
	}
}

void legacy_floppy_image_device::floppy_drive_read_sector_data(int side, int index1, void *ptr, int length)
{
	if (exists())
	{
		if (!m_floppy)
			return;

		floppy_read_indexed_sector(m_floppy, side, m_track, index1, 0, ptr, length);

		if (LOG_FLOPPY)
			log_readwrite("sector_read", side, m_track, index1, (const char *)ptr, length);

	}
}

void legacy_floppy_image_device::floppy_drive_write_sector_data(int side, int index1, const void *ptr,int length, int ddam)
{
	if (exists())
	{
		if (!m_floppy)
			return;

		if (LOG_FLOPPY)
			log_readwrite("sector_write", side, m_track, index1, (const char *)ptr, length);

		floppy_write_indexed_sector(m_floppy, side, m_track, index1, 0, ptr, length, ddam);
	}
}

void legacy_floppy_image_device::floppy_install_load_proc(void (*proc)(device_image_interface &image))
{
	m_load_proc = proc;
}

void legacy_floppy_image_device::floppy_install_unload_proc(void (*proc)(device_image_interface &image))
{
	m_unload_proc = proc;
}

/* set the callback for the index pulse */
void legacy_floppy_image_device::floppy_drive_set_index_pulse_callback(void (*callback)(device_t *controller, device_t *img, int state))
{
	m_index_pulse_callback = callback;
}

int legacy_floppy_image_device::floppy_drive_get_current_track()
{
	return m_current_track;
}

UINT64 legacy_floppy_image_device::floppy_drive_get_current_track_size(int head)
{
	int size = 0;
	if (exists())
	{
		size = floppy_get_track_size(m_floppy, head, m_current_track);
	}

	return size;
}

void legacy_floppy_image_device::floppy_drive_set_rpm(float rpm)
{
	m_rpm = rpm;
}

void legacy_floppy_image_device::floppy_drive_set_controller(device_t *controller)
{
	m_controller = controller;
}

int legacy_floppy_image_device::internal_floppy_device_load(int create_format, option_resolution *create_args)
{
	floperr_t err;
	const struct FloppyFormat *floppy_options;
	int floppy_flags, i;
	const char *extension;

	device_image_interface *image = NULL;
	interface(image);   /* figure out the floppy options */
	floppy_options = ((floppy_interface*)static_config())->formats;

	if (has_been_created())
	{
		/* creating an image */
		assert(create_format >= 0);
		err = floppy_create((void *) image, &image_ioprocs, &floppy_options[create_format], create_args, &m_floppy);
		if (err)
			goto error;
	}
	else
	{
		/* opening an image */
		floppy_flags = !is_readonly() ? FLOPPY_FLAGS_READWRITE : FLOPPY_FLAGS_READONLY;
		extension = filetype();
		err = floppy_open_choices((void *) image, &image_ioprocs, extension, floppy_options, floppy_flags, &m_floppy);
		if (err)
			goto error;
	}
	if (floppy_callbacks(m_floppy)->get_heads_per_disk && floppy_callbacks(m_floppy)->get_tracks_per_disk)
	{
		floppy_drive_set_geometry_absolute(floppy_get_tracks_per_disk(m_floppy),floppy_get_heads_per_disk(m_floppy));
	}
	/* disk changed */
	m_dskchg = CLEAR_LINE;

	return IMAGE_INIT_PASS;

error:
	for (i = 0; i < ARRAY_LENGTH(errmap); i++)
	{
		if (err == errmap[i].ferr)
			seterror(errmap[i].ierr, errmap[i].message);
	}
	return IMAGE_INIT_FAIL;
}

TIMER_CALLBACK_MEMBER( legacy_floppy_image_device::set_wpt )
{
	m_wpt = param;
	//m_out_wpt_func(param);
}

legacy_floppy_image_device *floppy_get_device(running_machine &machine,int drive)
{
	switch(drive) {
		case 0 : return machine.device<legacy_floppy_image_device>(FLOPPY_0);
		case 1 : return machine.device<legacy_floppy_image_device>(FLOPPY_1);
		case 2 : return machine.device<legacy_floppy_image_device>(FLOPPY_2);
		case 3 : return machine.device<legacy_floppy_image_device>(FLOPPY_3);
	}
	return NULL;
}

int legacy_floppy_image_device::floppy_get_drive_type()
{
	return m_floppy_drive_type;
}

void legacy_floppy_image_device::floppy_set_type(int ftype)
{
	m_floppy_drive_type = ftype;
}

legacy_floppy_image_device *floppy_get_device_by_type(running_machine &machine,int ftype,int drive)
{
	int i;
	int cnt = 0;
	for (i=0;i<4;i++) {
		legacy_floppy_image_device *disk = floppy_get_device(machine,i);
		if (disk->floppy_get_drive_type()==ftype) {
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

int floppy_get_drive_by_type(legacy_floppy_image_device *image,int ftype)
{
	int i,drive =0;
	for (i=0;i<4;i++) {
		legacy_floppy_image_device *disk = floppy_get_device(image->machine(),i);
		if (disk->floppy_get_drive_type()==ftype) {
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
	if (machine.device<legacy_floppy_image_device>(FLOPPY_0)) cnt++;
	if (machine.device<legacy_floppy_image_device>(FLOPPY_1)) cnt++;
	if (machine.device<legacy_floppy_image_device>(FLOPPY_2)) cnt++;
	if (machine.device<legacy_floppy_image_device>(FLOPPY_3)) cnt++;
	return cnt;
}


/* drive select 0 */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_ds0_w )
{
	if (state == CLEAR_LINE)
		m_active = (m_drive_id == 0);
}

/* drive select 1 */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_ds1_w )
{
	if (state == CLEAR_LINE)
		m_active = (m_drive_id == 1);
}

/* drive select 2 */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_ds2_w )
{
	if (state == CLEAR_LINE)
		m_active = (m_drive_id == 2);
}

/* drive select 3 */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_ds3_w )
{
	if (state == CLEAR_LINE)
		m_active = (m_drive_id == 3);
}

/* shortcut to write all four ds lines */
WRITE8_MEMBER( legacy_floppy_image_device::floppy_ds_w )
{
	floppy_ds0_w(BIT(data, 0));
	floppy_ds1_w(BIT(data, 1));
	floppy_ds2_w(BIT(data, 2));
	floppy_ds3_w(BIT(data, 3));
}

/* motor on, active low */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_mon_w )
{
	/* force off if there is no attached image */
	if (!exists())
		state = ASSERT_LINE;

	/* off -> on */
	if (m_mon && state == CLEAR_LINE)
	{
		m_idx = 0;
		floppy_drive_index_func();
	}

	/* on -> off */
	else if (m_mon == CLEAR_LINE && state)
		m_index_timer->adjust(attotime::zero);

	m_mon = state;
}

/* direction */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_drtn_w )
{
	m_drtn = state;
}

/* write data */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_wtd_w )
{
}

/* step */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_stp_w )
{
	/* move head one track when going from high to low and write gate is high */
	if (m_active && m_stp && state == CLEAR_LINE && m_wtg)
	{
		/* move head according to the direction line */
		if (m_drtn)
		{
			/* move head outward */
			if (m_current_track > 0)
				m_current_track--;

			/* are we at track 0 now? */
			m_tk00 = (m_current_track == 0) ? CLEAR_LINE : ASSERT_LINE;
		}
		else
		{
			/* move head inward */
			if (m_current_track < m_max_track)
				m_current_track++;

			/* we can't be at track 0 here, so reset the line */
			m_tk00 = ASSERT_LINE;
		}

		/* update track 0 line with new status */
		//m_out_tk00_func(m_tk00);
	}

	m_stp = state;
}

/* write gate */
WRITE_LINE_MEMBER( legacy_floppy_image_device::floppy_wtg_w )
{
	m_wtg = state;
}

/* write protect signal, active low */
READ_LINE_MEMBER( legacy_floppy_image_device::floppy_wpt_r )
{
	return m_wpt;
}

/* track 0 detect */
READ_LINE_MEMBER( legacy_floppy_image_device::floppy_tk00_r )
{
	return m_tk00;
}

/* disk changed */
READ_LINE_MEMBER( legacy_floppy_image_device::floppy_dskchg_r )
{
	return m_dskchg;
}

/* 2-sided disk */
READ_LINE_MEMBER( legacy_floppy_image_device::floppy_twosid_r )
{
	if (m_floppy == NULL)
		return ASSERT_LINE;
	else
		return !floppy_get_heads_per_disk(m_floppy);
}

READ_LINE_MEMBER( legacy_floppy_image_device::floppy_index_r )
{
	return m_idx;
}

READ_LINE_MEMBER( legacy_floppy_image_device::floppy_ready_r )
{
	return !(floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) == FLOPPY_DRIVE_READY);
}

// device type definition
const device_type LEGACY_FLOPPY = &device_creator<legacy_floppy_image_device>;

//-------------------------------------------------
//  legacy_floppy_image_device - constructor
//-------------------------------------------------

legacy_floppy_image_device::legacy_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LEGACY_FLOPPY, "Floppy Disk", tag, owner, clock, "legacy_floppy_image", __FILE__),
		device_image_interface(mconfig, *this),
		m_out_idx_func(*this),
		m_drtn(0),
		m_stp(0),
		m_wtg(0),
		m_mon(0),
		m_idx(0),
		m_tk00(0),
		m_wpt(0),
		m_rdy(0),
		m_dskchg(0),
		m_drive_id(0),
		m_active(0),
		m_flags(0),
		m_max_track(0),
		m_num_sides(0),
		m_current_track(0),
		m_index_pulse_callback(NULL),
		m_rpm(0.0f),
		m_id_index(0),
		m_controller(NULL),
		m_floppy(NULL),
		m_track(0),
		m_load_proc(NULL),
		m_unload_proc(NULL),
		m_floppy_drive_type(0)
{
	memset(&m_extension_list,0,sizeof(m_extension_list));
}

legacy_floppy_image_device::legacy_floppy_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_image_interface(mconfig, *this),
		m_out_idx_func(*this),
		m_drtn(0),
		m_stp(0),
		m_wtg(0),
		m_mon(0),
		m_idx(0),
		m_tk00(0),
		m_wpt(0),
		m_rdy(0),
		m_dskchg(0),
		m_drive_id(0),
		m_active(0),
		m_flags(0),
		m_max_track(0),
		m_num_sides(0),
		m_current_track(0),
		m_index_pulse_callback(NULL),
		m_rpm(0.0f),
		m_id_index(0),
		m_controller(NULL),
		m_floppy(NULL),
		m_track(0),
		m_load_proc(NULL),
		m_unload_proc(NULL),
		m_floppy_drive_type(0)
{
	memset(&m_extension_list,0,sizeof(m_extension_list));
}

//-------------------------------------------------
//  legacy_floppy_image_device - destructor
//-------------------------------------------------

legacy_floppy_image_device::~legacy_floppy_image_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void legacy_floppy_image_device::device_start()
{
	m_config = (const floppy_interface*)static_config();
	floppy_drive_init();

	m_drive_id = floppy_get_drive(this);
	m_active = FALSE;

	/* resolve callbacks */
	m_out_idx_func.resolve_safe();
	//m_in_mon_func.resolve(m_config->in_mon_func, *this);
	//m_out_tk00_func.resolve(m_config->out_tk00_func, *this);
	//m_out_wpt_func.resolve(m_config->out_wpt_func, *this);
	//m_out_rdy_func.resolve(m_config->out_rdy_func, *this);
//  m_out_dskchg_func.resolve(m_config->out_dskchg_func, *this);

	/* by default we are not write-protected */
	m_wpt = ASSERT_LINE;
	//m_out_wpt_func(m_wpt);

	/* not at track 0 */
	m_tk00 = ASSERT_LINE;
	//m_out_tk00_func(m_tk00);

	/* motor off */
	m_mon = ASSERT_LINE;

	/* disk changed */
	m_dskchg = CLEAR_LINE;
//  m_out_dskchg_func(m_dskchg);
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void legacy_floppy_image_device::device_config_complete()
{
	m_extension_list[0] = '\0';
	const struct FloppyFormat *floppy_options = ((floppy_interface*)static_config())->formats;
	for (int i = 0; floppy_options[i].construct; i++)
	{
		// only add if creatable
		if (floppy_options[i].param_guidelines) {
			// allocate a new format and append it to the list
			m_formatlist.append(*global_alloc(image_device_format(floppy_options[i].name, floppy_options[i].description, floppy_options[i].extensions, floppy_options[i].param_guidelines)));
		}
		image_specify_extension( m_extension_list, 256, floppy_options[i].extensions );
	}

	// set brief and instance name
	update_names();
}

bool legacy_floppy_image_device::call_create(int format_type, option_resolution *format_options)
{
	return internal_floppy_device_load(format_type, format_options);
}

bool legacy_floppy_image_device::call_load()
{
	int retVal = internal_floppy_device_load(-1, NULL);
	if (retVal==IMAGE_INIT_PASS) {
		/* if we have one of our hacky unload procs, call it */
		if (m_load_proc)
			m_load_proc(*this);
	}

	/* push disk halfway into drive */
	m_wpt = CLEAR_LINE;
	//m_out_wpt_func(m_wpt);

	/* set timer for disk load */
	int next_wpt;

	if (!is_readonly())
		next_wpt = ASSERT_LINE;
	else
		next_wpt = CLEAR_LINE;

	machine().scheduler().timer_set(attotime::from_msec(250), timer_expired_delegate(FUNC(legacy_floppy_image_device::set_wpt),this), next_wpt);

	return retVal;
}

void legacy_floppy_image_device::call_unload()
{
	if (m_unload_proc)
		m_unload_proc(*this);

	floppy_close(m_floppy);
	m_floppy = NULL;

	/* disk changed */
	m_dskchg = CLEAR_LINE;
	//m_out_dskchg_func(m_dskchg);

	/* pull disk halfway out of drive */
	m_wpt = CLEAR_LINE;
	//m_out_wpt_func(m_wpt);

	/* set timer for disk eject */
	machine().scheduler().timer_set(attotime::from_msec(250), timer_expired_delegate(FUNC(legacy_floppy_image_device::set_wpt),this), ASSERT_LINE);
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
