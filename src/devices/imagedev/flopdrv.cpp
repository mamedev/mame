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
#include "flopdrv.h"

#include "softlist_dev.h"

#include "formats/imageutl.h"

#include "util/ioprocs.h"
#include "util/ioprocsfilter.h"

//#define VERBOSE 1
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LOG_FLOPPY      0

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct floppy_error_map
{
	floperr_t ferr;
	std::error_condition ierr;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const floppy_error_map errmap[] =
{
	{ FLOPPY_ERROR_SUCCESS,         { } },
	{ FLOPPY_ERROR_INTERNAL,        { image_error::INTERNAL } },
	{ FLOPPY_ERROR_UNSUPPORTED,     { image_error::UNSUPPORTED } },
	{ FLOPPY_ERROR_OUTOFMEMORY,     { std::errc::not_enough_memory } },
	{ FLOPPY_ERROR_INVALIDIMAGE,    { image_error::INVALIDIMAGE } }
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void legacy_floppy_image_device::log_readwrite(const char *name, int head, int track, int sector, const char *buf, int length)
{
	char membuf[1024];
	int i;
	for (i = 0; i < length; i++)
		snprintf(membuf + i*2, 1024 - (i*2), "%02x", (int) (uint8_t) buf[i]);
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
	m_index_pulse_callback = nullptr;
	m_index_timer = timer_alloc(FUNC(legacy_floppy_image_device::floppy_drive_index_callback), this);
	m_idx = 0;

	floppy_drive_set_geometry(m_config->floppy_type);

	/* initialise track */
	m_current_track = 0;

	/* default RPM */
	m_rpm = 300;

	m_controller = nullptr;
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
	LOG("seek from: %d delta: %d\n", m_current_track, signed_tracks);

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
	m_tk00 = (m_current_track == 0) ? 0 : 1;
	//m_out_tk00_func(m_tk00);

	/* clear disk changed flag */
	m_dskchg = 1;
	//m_out_dskchg_func(m_dskchg);

	/* inform disk image of step operation so it can cache information */
	if (exists())
		m_track = m_current_track;
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

void legacy_floppy_image_device::floppy_install_load_proc(void (*proc)(device_image_interface &image, bool is_created))
{
	m_load_proc = proc;
}

void legacy_floppy_image_device::floppy_install_unload_proc(void (*proc)(device_image_interface &image))
{
	m_unload_proc = proc;
}

int legacy_floppy_image_device::floppy_drive_get_current_track()
{
	return m_current_track;
}

void legacy_floppy_image_device::floppy_drive_set_rpm(float rpm)
{
	m_rpm = rpm;
}

void legacy_floppy_image_device::floppy_drive_set_controller(device_t *controller)
{
	m_controller = controller;
}

std::error_condition legacy_floppy_image_device::internal_floppy_device_load(bool is_create, int create_format, util::option_resolution *create_args)
{
	const struct FloppyFormat *floppy_options = m_config->formats;

	floperr_t err;
	check_for_file();
	auto io = util::random_read_write_fill(image_core_file(), 0xff);
	if (!io)
	{
		err = FLOPPY_ERROR_OUTOFMEMORY;
	}
	else if (is_create)
	{
		/* creating an image */
		assert(create_format >= 0);
		err = floppy_create(std::move(io), &floppy_options[create_format], create_args, &m_floppy);
	}
	else
	{
		/* opening an image */
		int const floppy_flags = !is_readonly() ? FLOPPY_FLAGS_READWRITE : FLOPPY_FLAGS_READONLY;
		err = floppy_open_choices(std::move(io), filetype(), floppy_options, floppy_flags, &m_floppy);
	}

	if (!err)
	{
		if (floppy_callbacks(m_floppy)->get_heads_per_disk && floppy_callbacks(m_floppy)->get_tracks_per_disk)
		{
			floppy_drive_set_geometry_absolute(floppy_get_tracks_per_disk(m_floppy),floppy_get_heads_per_disk(m_floppy));
		}
		/* disk changed */
		m_dskchg = CLEAR_LINE;

		// If we have one of our hacky load procs, call it
		if (m_load_proc)
			m_load_proc(*this, is_create);

		return std::error_condition();
	}
	else
	{
		for (int i = 0; i < std::size(errmap); i++)
		{
			if (err == errmap[i].ferr)
				return errmap[i].ierr;
		}
		return image_error::UNSPECIFIED;
	}
}

TIMER_CALLBACK_MEMBER( legacy_floppy_image_device::set_wpt )
{
	m_wpt = param;
	//m_out_wpt_func(param);
}

/* drive select */
void legacy_floppy_image_device::floppy_ds_w(int state)
{
	m_active = (state == 0);
}

/* motor on, active low */
void legacy_floppy_image_device::floppy_mon_w(int state)
{
	/* force off if there is no attached image */
	if (!exists())
		state = 1;

	/* off -> on */
	if (m_mon && state == 0)
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
void legacy_floppy_image_device::floppy_drtn_w(int state)
{
	m_drtn = state;
}

/* step */
void legacy_floppy_image_device::floppy_stp_w(int state)
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
			m_tk00 = (m_current_track == 0) ? 0 : 1;
		}
		else
		{
			/* move head inward */
			if (m_current_track < m_max_track)
				m_current_track++;

			/* we can't be at track 0 here, so reset the line */
			m_tk00 = 1;
		}

		/* update track 0 line with new status */
		//m_out_tk00_func(m_tk00);

		/* inform disk image of step operation so it can cache information */
		if (exists())
			m_track = m_current_track;
	}

	m_stp = state;
}

/* write gate */
void legacy_floppy_image_device::floppy_wtg_w(int state)
{
	m_wtg = state;
}

/* track 0 detect */
int legacy_floppy_image_device::floppy_tk00_r()
{
	return m_tk00;
}

int legacy_floppy_image_device::floppy_index_r()
{
	return m_idx;
}

int legacy_floppy_image_device::floppy_ready_r()
{
	return !(floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) == FLOPPY_DRIVE_READY);
}

// device type definition
DEFINE_DEVICE_TYPE(LEGACY_FLOPPY, legacy_floppy_image_device, "legacy_floppy_image", "Floppy Disk")

//-------------------------------------------------
//  legacy_floppy_image_device - constructor
//-------------------------------------------------

legacy_floppy_image_device::legacy_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: legacy_floppy_image_device(mconfig, LEGACY_FLOPPY, tag, owner, clock)
{
}

legacy_floppy_image_device::legacy_floppy_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_out_idx_func(*this),
		m_drtn(0),
		m_stp(0),
		m_wtg(0),
		m_mon(0),
		m_idx(0),
		m_tk00(0),
		m_wpt(0),
		m_dskchg(0),
		m_active(0),
		m_config(nullptr),
		m_flags(0),
		m_max_track(0),
		m_num_sides(0),
		m_current_track(0),
		m_index_timer(nullptr),
		m_index_pulse_callback(nullptr),
		m_rpm(0.0f),
		m_controller(nullptr),
		m_floppy(nullptr),
		m_track(0),
		m_load_proc(nullptr),
		m_unload_proc(nullptr)
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
	floppy_drive_init();

	m_active = false;

	/* resolve callbacks */
	//m_in_mon_func.resolve(m_config->in_mon_func, *this);
	//m_out_tk00_func.resolve(m_config->out_tk00_func, *this);
	//m_out_wpt_func.resolve(m_config->out_wpt_func, *this);
	//m_out_rdy_func.resolve(m_config->out_rdy_func, *this);
//  m_out_dskchg_func.resolve(m_config->out_dskchg_func, *this);

	/* by default we are not write-protected */
	m_wpt = 1;
	//m_out_wpt_func(m_wpt);

	/* not at track 0 */
	m_tk00 = 1;
	//m_out_tk00_func(m_tk00);

	/* motor off */
	m_mon = 1;

	/* disk changed */
	m_dskchg = CLEAR_LINE;
//  m_out_dskchg_func(m_dskchg);

	/* write-protect callback */
	m_wpt_timer = timer_alloc(FUNC(legacy_floppy_image_device::set_wpt), this);
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void legacy_floppy_image_device::device_config_complete()
{
	m_extension_list[0] = '\0';
	if (m_config)
	{
		const struct FloppyFormat *floppy_options = m_config->formats;
		for (int i = 0; floppy_options && floppy_options[i].construct; i++)
		{
			// only add if creatable
			if (floppy_options[i].param_guidelines)
			{
				// allocate a new format and append it to the list
				add_format(floppy_options[i].name, floppy_options[i].description, floppy_options[i].extensions, floppy_options[i].param_guidelines);
			}
			image_specify_extension(m_extension_list, 256, floppy_options[i].extensions);
		}
	}
}

const software_list_loader &legacy_floppy_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}

std::pair<std::error_condition, std::string> legacy_floppy_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	return std::make_pair(internal_floppy_device_load(true, format_type, format_options), std::string());
}

std::pair<std::error_condition, std::string> legacy_floppy_image_device::call_load()
{
	std::error_condition retVal = internal_floppy_device_load(false, -1, nullptr);

	/* push disk halfway into drive */
	m_wpt = CLEAR_LINE;
	//m_out_wpt_func(m_wpt);

	/* set timer for disk load */
	int next_wpt;

	if (!is_readonly())
		next_wpt = 1;
	else
		next_wpt = 0;

	m_wpt_timer->adjust(attotime::from_msec(250), next_wpt);

	return std::make_pair(retVal, std::string());
}

void legacy_floppy_image_device::call_unload()
{
	if (m_unload_proc)
		m_unload_proc(*this);

	floppy_close(m_floppy);
	m_floppy = nullptr;

	/* disk changed */
	m_dskchg = 0;
	//m_out_dskchg_func(m_dskchg);

	/* pull disk halfway out of drive */
	m_wpt = 0;
	//m_out_wpt_func(m_wpt);

	/* set timer for disk eject */
	m_wpt_timer->adjust(attotime::from_msec(250), 1);
}

bool legacy_floppy_image_device::is_creatable() const noexcept
{
	if (m_config)
	{
		const struct FloppyFormat *floppy_options = m_config->formats;
		for (int i = 0; floppy_options[i].construct; i++)
		{
			if (floppy_options[i].param_guidelines)
				return true;
		}
	}
	return false;
}

const char *legacy_floppy_image_device::image_interface() const noexcept
{
	return m_config->interface;
}
