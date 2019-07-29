// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Olivier Galibert, Miodrag Milanovic
/*********************************************************************



*********************************************************************/

#include "emu.h"
#include "floppy.h"

#include "speaker.h"
#include "formats/imageutl.h"
#include "zippath.h"

/*
    Debugging flags. Set to 0 or 1.
*/

// Show step operation
#define TRACE_STEP 0
#define TRACE_AUDIO 0

#define PITCH_SEEK_SAMPLES 1

#define FLOPSND_TAG "floppysound"

// device type definition
DEFINE_DEVICE_TYPE(FLOPPY_CONNECTOR, floppy_connector, "floppy_connector", "Floppy drive connector abstraction")

// generic 3" drives
DEFINE_DEVICE_TYPE(FLOPPY_3_SSDD, floppy_3_ssdd, "floppy_3_ssdd", "3\" single-sided floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_3_DSDD, floppy_3_dsdd, "floppy_3_dsdd", "3\" double-sided floppy drive")

// generic 3.5" drives
DEFINE_DEVICE_TYPE(FLOPPY_35_SSDD, floppy_35_ssdd, "floppy_35_ssdd", "3.5\" single-sided double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_35_DD,   floppy_35_dd,   "floppy_35_dd",   "3.5\" double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_35_HD,   floppy_35_hd,   "floppy_35_hd",   "3.5\" high density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_35_ED,   floppy_35_ed,   "floppy_35_ed",   "3.5\" extended density floppy drive")

// generic 5.25" drives
DEFINE_DEVICE_TYPE(FLOPPY_525_SSSD_35T, floppy_525_sssd_35t, "floppy_525_sssd_35t", "5.25\" single-sided single density 35-track floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_SD_35T,   floppy_525_sd_35t,   "floppy_525_sd_35t",   "5.25\" double-sided single density 35-track floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_SSSD,     floppy_525_sssd,     "floppy_525_sssd",     "5.25\" single-sided single density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_SD,       floppy_525_sd,       "floppy_525_sd",       "5.25\" single density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_SSDD,     floppy_525_ssdd,     "floppy_525_ssdd",     "5.25\" single-sided double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_DD,       floppy_525_dd,       "floppy_525_dd",       "5.25\" double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_SSQD,     floppy_525_ssqd,     "floppy_525_ssqd",     "5.25\" single-sided quad density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_QD,       floppy_525_qd,       "floppy_525_qd",       "5.25\" quad density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_HD,       floppy_525_hd,       "floppy_525_hd",       "5.25\" high density floppy drive")

// generic 8" drives
DEFINE_DEVICE_TYPE(FLOPPY_8_SSSD, floppy_8_sssd, "floppy_8_sssd", "8\" single-sided single density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_8_DSSD, floppy_8_dssd, "floppy_8_dssd", "8\" double-sided single density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_8_SSDD, floppy_8_ssdd, "floppy_8_ssdd", "8\" single-sided double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_8_DSDD, floppy_8_dsdd, "floppy_8_dsdd", "8\" double-sided double density floppy drive")

// Epson 3.5" drives
#if 0
DEFINE_DEVICE_TYPE(EPSON_SMD_110,   epson_smd_110,   "epson_smd_110",   "EPSON SMD-110 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_120,   epson_smd_120,   "epson_smd_120",   "EPSON SMD-120 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_125,   epson_smd_125,   "epson_smd_125",   "EPSON SMD-125 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_130,   epson_smd_130,   "epson_smd_130",   "EPSON SMD-130 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_140,   epson_smd_140,   "epson_smd_140",   "EPSON SMD-140 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_150,   epson_smd_150,   "epson_smd_150",   "EPSON SMD-150 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_160,   epson_smd_160,   "epson_smd_160",   "EPSON SMD-160 Floppy Disk Drive")
#endif
DEFINE_DEVICE_TYPE(EPSON_SMD_165,   epson_smd_165,   "epson_smd_165",   "EPSON SMD-165 Floppy Disk Drive")
#if 0
DEFINE_DEVICE_TYPE(EPSON_SMD_170,   epson_smd_170,   "epson_smd_170",   "EPSON SMD-170 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_180,   epson_smd_180,   "epson_smd_180",   "EPSON SMD-180 Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_240L,  epson_smd_240l,  "epson_smd_240l",  "EPSON SMD-240L Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_280HL, epson_smd_280hl, "epson_smd_280hl", "EPSON SMD-280HL Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_440L,  epson_smd_440l,  "epson_smd_440l",  "EPSON SMD-440L Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_449L,  epson_smd_449l,  "epson_smd_449l",  "EPSON SMD-449L Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_480LM, epson_smd_480lm, "epson_smd_480lm", "EPSON SMD-480LM Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SMD_489M,  epson_smd_489m,  "epson_smd_489m",  "EPSON SMD-489M Floppy Disk Drive")
#endif

// Epson 5.25" drives
#if 0
DEFINE_DEVICE_TYPE(EPSON_SD_311,  epson_sd_311,  "epson_sd_311",  "EPSON SD-311 Mini-Floppy Disk Drive")
#endif
DEFINE_DEVICE_TYPE(EPSON_SD_320,  epson_sd_320,  "epson_sd_320",  "EPSON SD-320 Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_321,  epson_sd_321,  "epson_sd_321",  "EPSON SD-321 Mini-Floppy Disk Drive")
#if 0
DEFINE_DEVICE_TYPE(EPSON_SD_521L, epson_sd_531l, "epson_sd_531l", "EPSON SD-531L Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_525,  epson_sd_525,  "epson_sd_525",  "EPSON SD-525 Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_543,  epson_sd_543,  "epson_sd_543",  "EPSON SD-543 Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_545,  epson_sd_545,  "epson_sd_545",  "EPSON SD-545 Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_560,  epson_sd_560,  "epson_sd_560",  "EPSON SD-560 Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_580L, epson_sd_580l, "epson_sd_580l", "EPSON SD-580L Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_581L, epson_sd_581l, "epson_sd_581l", "EPSON SD-581L Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_621L, epson_sd_621l, "epson_sd_621l", "EPSON SD-621L Mini-Floppy Disk Drive")
DEFINE_DEVICE_TYPE(EPSON_SD_680L, epson_sd_680l, "epson_sd_680l", "EPSON SD-680L Mini-Floppy Disk Drive")
#endif

// Sony 3.5" drives
DEFINE_DEVICE_TYPE(SONY_OA_D31V, sony_oa_d31v, "sony_oa_d31v", "Sony OA-D31V Micro Floppydisk Drive")
DEFINE_DEVICE_TYPE(SONY_OA_D32W, sony_oa_d32w, "sony_oa_d32w", "Sony OA-D32W Micro Floppydisk Drive")
DEFINE_DEVICE_TYPE(SONY_OA_D32V, sony_oa_d32v, "sony_oa_d32v", "Sony OA-D32V Micro Floppydisk Drive")

// TEAC 3" drives
DEFINE_DEVICE_TYPE(TEAC_FD_30A, teac_fd_30a, "teac_fd_30a", "TEAC FD-30A FDD")

// TEAC 5.25" drives
#if 0
DEFINE_DEVICE_TYPE(TEAC_FD_55A, teac_fd_55a, "teac_fd_55a", "TEAC FD-55A FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55B, teac_fd_55b, "teac_fd_55b", "TEAC FD-55B FDD")
#endif
DEFINE_DEVICE_TYPE(TEAC_FD_55E, teac_fd_55e, "teac_fd_55e", "TEAC FD-55E FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55F, teac_fd_55f, "teac_fd_55f", "TEAC FD-55F FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55G, teac_fd_55g, "teac_fd_55g", "TEAC FD-55G FDD")

// ALPS 5.25" drives
DEFINE_DEVICE_TYPE(ALPS_3255190X, alps_3255190x, "alps_3255190x", "ALPS 32551901/32551902 Floppy Drive")

// IBM 8" drives
DEFINE_DEVICE_TYPE(IBM_6360, ibm_6360, "ibm_6360", "IBM 6360 8\" single-sided single density floppy drive")


const floppy_format_type floppy_image_device::default_floppy_formats[] = {
	FLOPPY_D88_FORMAT,
	FLOPPY_DFI_FORMAT,
	FLOPPY_HFE_FORMAT,
	FLOPPY_IMD_FORMAT,
	FLOPPY_IPF_FORMAT,
	FLOPPY_MFI_FORMAT,
	FLOPPY_MFM_FORMAT,
	FLOPPY_TD0_FORMAT,
	FLOPPY_CQM_FORMAT,
	FLOPPY_DSK_FORMAT,
	nullptr
};

floppy_connector::floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FLOPPY_CONNECTOR, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	formats(nullptr),
	m_enable_sound(false)
{
}

floppy_connector::~floppy_connector()
{
}

void floppy_connector::set_formats(const floppy_format_type _formats[])
{
	formats = _formats;
}

void floppy_connector::device_start()
{
}

void floppy_connector::device_config_complete()
{
	floppy_image_device *dev = dynamic_cast<floppy_image_device *>(get_card_device());
	if(dev)
	{
		dev->set_formats(formats);
		dev->enable_sound(m_enable_sound);
	}
}

floppy_image_device *floppy_connector::get_device()
{
	return dynamic_cast<floppy_image_device *>(get_card_device());
}

//-------------------------------------------------
//  floppy_image_device - constructor
//-------------------------------------------------

floppy_image_device::floppy_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_image_interface(mconfig, *this),
		device_slot_card_interface(mconfig, *this),
		input_format(nullptr),
		output_format(nullptr),
		image(nullptr),
		fif_list(nullptr),
		index_timer(nullptr),
		tracks(0),
		sides(0),
		form_factor(0),
		motor_always_on(false),
		dskchg_writable(false),
		has_trk00_sensor(true),
		dir(0), stp(0), wtg(0), mon(0), ss(0), ds(-1), idx(0), wpt(0), rdy(0), dskchg(0),
		ready(false),
		rpm(0),
		floppy_ratio_1(0),
		revolution_count(0),
		cyl(0),
		subcyl(0),
		image_dirty(false),
		ready_counter(0),
		m_make_sound(false),
		m_sound_out(nullptr)
{
	extension_list[0] = '\0';
	m_err = IMAGE_ERROR_INVALIDIMAGE;
}

//-------------------------------------------------
//  floppy_image_device - destructor
//-------------------------------------------------

floppy_image_device::~floppy_image_device()
{
	for(floppy_image_format_t *format = fif_list; format; ) {
		floppy_image_format_t* tmp_format = format;
		format = format->next;
		delete tmp_format;
	}
	fif_list = nullptr;
}

void floppy_image_device::setup_load_cb(load_cb cb)
{
	cur_load_cb = cb;
}

void floppy_image_device::setup_unload_cb(unload_cb cb)
{
	cur_unload_cb = cb;
}

void floppy_image_device::setup_index_pulse_cb(index_pulse_cb cb)
{
	cur_index_pulse_cb = cb;
}

void floppy_image_device::setup_ready_cb(ready_cb cb)
{
	cur_ready_cb = cb;
}

void floppy_image_device::setup_wpt_cb(wpt_cb cb)
{
	cur_wpt_cb = cb;
}

void floppy_image_device::setup_led_cb(led_cb cb)
{
	cur_led_cb = cb;
}

void floppy_image_device::set_formats(const floppy_format_type *formats)
{
	extension_list[0] = '\0';
	fif_list = nullptr;
	for(int cnt=0; formats[cnt]; cnt++)
	{
		// allocate a new format
		floppy_image_format_t *fif = formats[cnt]();
		if(!fif_list)
			fif_list = fif;
		else
			fif_list->append(fif);

		add_format(fif->name(), fif->description(), fif->extensions(), "");

		image_specify_extension( extension_list, 256, fif->extensions() );
	}
}

floppy_image_format_t *floppy_image_device::get_formats() const
{
	return fif_list;
}

floppy_image_format_t *floppy_image_device::get_load_format() const
{
	return input_format;
}

void floppy_image_device::set_rpm(float _rpm)
{
	if(rpm == _rpm)
		return;

	rpm = _rpm;
	rev_time = attotime::from_double(60/rpm);
	floppy_ratio_1 = int(1000.0f*rpm/300.0f+0.5f);
}

void floppy_image_device::setup_write(floppy_image_format_t *_output_format)
{
	output_format = _output_format;
	commit_image();
}

void floppy_image_device::commit_image()
{
	image_dirty = false;
	if(!output_format || !output_format->supports_save())
		return;
	io_generic io;
	// Do _not_ remove this cast otherwise the pointer will be incorrect when used by the ioprocs.
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0xff;

	osd_file::error err = image_core_file().truncate(0);
	if (err != osd_file::error::NONE)
		popmessage("Error, unable to truncate image: %d", int(err));

	output_format->save(&io, image);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_image_device::device_start()
{
	rpm = 0;
	motor_always_on = false;
	dskchg_writable = false;
	has_trk00_sensor = true;

	// better would be an extra parameter in the MCFG macro
	drive_index = atoi(owner()->basetag());

	idx = 0;

	/* motor off */
	mon = 1;

	cyl = 0;
	subcyl = 0;
	ss  = 0;
	ds = -1;
	stp = 1;
	wpt = 0;
	dskchg = exists() ? 1 : 0;
	index_timer = timer_alloc(0);
	image_dirty = false;
	ready = true;
	ready_counter = 0;

	setup_characteristics();

	if (m_make_sound) m_sound_out = subdevice<floppy_sound_device>(FLOPSND_TAG);

	save_item(NAME(dir));
	save_item(NAME(stp));
	save_item(NAME(wtg));
	save_item(NAME(mon));
	save_item(NAME(ss));
	save_item(NAME(ds));
	save_item(NAME(idx));
	save_item(NAME(wpt));
	save_item(NAME(rdy));
	save_item(NAME(dskchg));
	save_item(NAME(ready));
	save_item(NAME(rpm));
	save_item(NAME(floppy_ratio_1));
	save_item(NAME(revolution_start_time));
	save_item(NAME(rev_time));
	save_item(NAME(revolution_count));
	save_item(NAME(cyl));
	save_item(NAME(subcyl));
	save_item(NAME(cache_start_time));
	save_item(NAME(cache_end_time));
	save_item(NAME(cache_index));
	save_item(NAME(cache_entry));
	save_item(NAME(cache_weak));
	save_item(NAME(image_dirty));
	save_item(NAME(ready_counter));
}

void floppy_image_device::device_reset()
{
	if (m_make_sound)
	{
		// Have we loaded all samples? Otherwise mute the floppy.
		m_make_sound = m_sound_out->samples_loaded();
	}

	revolution_start_time = attotime::never;
	revolution_count = 0;
	mon = 1;
	set_ready(true);
	if(motor_always_on && image)
		mon_w(0);
	cache_clear();
}

void floppy_image_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	index_resync();
}

floppy_image_format_t *floppy_image_device::identify(std::string filename)
{
	util::core_file::ptr fd;
	std::string revised_path;

	osd_file::error err = util::zippath_fopen(filename, OPEN_FLAG_READ, fd, revised_path);
	if(err != osd_file::error::NONE) {
		seterror(IMAGE_ERROR_INVALIDIMAGE, "Unable to open the image file");
		return nullptr;
	}

	io_generic io;
	io.file = fd.get();
	io.procs = &corefile_ioprocs_noclose;
	io.filler = 0xff;
	int best = 0;
	floppy_image_format_t *best_format = nullptr;
	for (floppy_image_format_t *format = fif_list; format; format = format->next)
	{
		int score = format->identify(&io, form_factor);
		if(score > best) {
			best = score;
			best_format = format;
		}
	}
	fd.reset();
	return best_format;
}

void floppy_image_device::init_floppy_load(bool write_supported)
{
	cache_clear();
	revolution_start_time = mon ? attotime::never : machine().time();
	revolution_count = 0;

	index_resync();

	wpt = 1; // disk sleeve is covering the sensor
	if (!cur_wpt_cb.isnull())
		cur_wpt_cb(this, wpt);

	wpt = is_readonly() || (!write_supported);
	if (!cur_wpt_cb.isnull())
		cur_wpt_cb(this, wpt);

	if (motor_always_on) {
		// When disk is inserted, start motor
		mon_w(0);
	} else if(!mon)
		ready_counter = 2;

	if (dskchg_writable)
		dskchg = 1;
}

image_init_result floppy_image_device::call_load()
{
	io_generic io;

	// Do _not_ remove this cast otherwise the pointer will be incorrect when used by the ioprocs.
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0xff;
	int best = 0;
	floppy_image_format_t *best_format = nullptr;
	for(floppy_image_format_t *format = fif_list; format; format = format->next) {
		int score = format->identify(&io, form_factor);
		if(score > best) {
			best = score;
			best_format = format;
		}
	}

	if(!best_format)
	{
		seterror(IMAGE_ERROR_INVALIDIMAGE, "Unable to identify the image format");
		return image_init_result::FAIL;
	}

	image = global_alloc(floppy_image(tracks, sides, form_factor));
	if (!best_format->load(&io, form_factor, image))
	{
		seterror(IMAGE_ERROR_UNSUPPORTED, "Incompatible image format or corrupted data");
		global_free(image);
		image = nullptr;
		return image_init_result::FAIL;
	}
	output_format = is_readonly() ? nullptr : best_format;

	image_dirty = false;

	init_floppy_load(output_format != nullptr);

	if (!cur_load_cb.isnull())
		return cur_load_cb(this);

	return image_init_result::PASS;
}

void floppy_image_device::call_unload()
{
	cache_clear();
	dskchg = 0;

	if (image) {
		if(image_dirty)
			commit_image();
		global_free(image);
		image = nullptr;
	}

	wpt = 1; // disk sleeve is covering the sensor
	if (!cur_wpt_cb.isnull())
		cur_wpt_cb(this, wpt);

	wpt = 0; // sensor is uncovered
	if (!cur_wpt_cb.isnull())
		cur_wpt_cb(this, wpt);

	if (!cur_unload_cb.isnull())
		cur_unload_cb(this);

	if (motor_always_on) {
		// When disk is removed, stop motor
		mon_w(1);
	}

	set_ready(true);
}

image_init_result floppy_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	image = global_alloc(floppy_image(tracks, sides, form_factor));
	output_format = nullptr;

	// search for a suitable format based on the extension
	for(floppy_image_format_t *i = fif_list; i; i = i->next)
	{
		// only consider formats that actually support saving
		if(!i->supports_save())
			continue;

		if (i->extension_matches(basename()))
		{
			output_format = i;
			break;
		}
	}

	// did we find a suitable format?
	if (output_format == nullptr)
	{
		seterror(IMAGE_ERROR_INVALIDIMAGE, "Unable to identify the image format");
		return image_init_result::FAIL;
	}

	init_floppy_load(output_format != nullptr);

	return image_init_result::PASS;
}

/* motor on, active low */
void floppy_image_device::mon_w(int state)
{
	if(mon == state)
		return;

	mon = state;

	/* off -> on */
	if (!mon && image)
	{
		revolution_start_time = machine().time();
		cache_clear();
		if (motor_always_on) {
			// Drives with motor that is always spinning are immediately ready when a disk is loaded
			// because there is no spin-up time
			set_ready(false);
		} else {
			ready_counter = 2;
		}
		index_resync();
	}

	/* on -> off */
	else {
		if(image_dirty)
			commit_image();
		cache_clear();
		revolution_start_time = attotime::never;
		index_timer->adjust(attotime::zero);
		set_ready(true);
	}

	// Create a motor sound (loaded or empty)
	if (m_make_sound) m_sound_out->motor(state==0, exists());
}

attotime floppy_image_device::time_next_index()
{
	if(revolution_start_time.is_never())
		return attotime::never;
	return revolution_start_time + rev_time;
}

/* index pulses at rpm/60 Hz, and stays high for ~2ms at 300rpm */
void floppy_image_device::index_resync()
{
	if(revolution_start_time.is_never()) {
		if(idx) {
			idx = 0;
			if (!cur_index_pulse_cb.isnull())
				cur_index_pulse_cb(this, idx);
		}
		return;
	}

	attotime delta = machine().time() - revolution_start_time;
	while(delta >= rev_time) {
		delta -= rev_time;
		revolution_start_time += rev_time;
		revolution_count++;
	}
	int position = (delta*floppy_ratio_1).as_ticks(1000000000/1000);

	int new_idx = position < 20000;

	if(new_idx) {
		attotime index_up_time = attotime::from_nsec((2000000*1000)/floppy_ratio_1);
		index_timer->adjust(index_up_time - delta);
	} else
		index_timer->adjust(rev_time - delta);

	if(new_idx != idx) {
		idx = new_idx;
		if(idx && ready) {
			ready_counter--;
			if(!ready_counter) {
				// logerror("Drive spun up\n");
				set_ready(false);
			}
		}
		if (!cur_index_pulse_cb.isnull())
			cur_index_pulse_cb(this, idx);
	}
}

bool floppy_image_device::ready_r()
{
	return ready;
}

void floppy_image_device::set_ready(bool state)
{
	if (state != ready)
	{
		ready = state;
		check_led();
		if (!cur_ready_cb.isnull())
			cur_ready_cb(this, ready);
	}
}

void floppy_image_device::check_led()
{
	if(!cur_led_cb.isnull())
		cur_led_cb(this, (ds == drive_index) && !ready ? 1 : 0);
}

double floppy_image_device::get_pos()
{
	return index_timer->elapsed().as_double();
}

bool floppy_image_device::twosid_r()
{
	int tracks = 0, heads = 0;

	if (image) image->get_actual_geometry(tracks, heads);

	return heads == 1;
}

void floppy_image_device::stp_w(int state)
{
	// Before spin-up is done, ignore step pulses
	// TODO: There are reports about drives supporting step operation with
	// stopped spindle. Need to check that on real drives.
	// if (ready_counter > 0) return;

	if ( stp != state ) {
		cache_clear();
		stp = state;
		if ( stp == 0 ) {
			int ocyl = cyl;
			if ( dir ) {
				if ( cyl ) cyl--;
			} else {
				if ( cyl < tracks-1 ) cyl++;
			}
			if(ocyl != cyl)
			{
				if (TRACE_STEP) logerror("track %d\n", cyl);
				// Do we want a stepper sound?
				// We plan for 5 zones with possibly specific sounds
				if (m_make_sound) m_sound_out->step(cyl*5/tracks);
			}
			/* Update disk detection if applicable */
			if (exists() && !dskchg_writable)
			{
				if (dskchg==0) dskchg = 1;
			}
		}
		subcyl = 0;
	}
}

void floppy_image_device::seek_phase_w(int phases)
{
	int cur_pos = (cyl << 2) | subcyl;
	int req_pos;
	switch(phases) {
	case 0x1: req_pos = 0; break;
	case 0x3: req_pos = 1; break;
	case 0x2: req_pos = 2; break;
	case 0x6: req_pos = 3; break;
	case 0x4: req_pos = 4; break;
	case 0xc: req_pos = 5; break;
	case 0x8: req_pos = 6; break;
	case 0x9: req_pos = 7; break;
	default: return;
	}

	// Opposite phase, don't move
	if(((cur_pos ^ req_pos) & 7) == 4)
		return;

	int next_pos = (cur_pos & ~7) | req_pos;
	if(next_pos < cur_pos-4)
		next_pos += 8;
	else if(next_pos > cur_pos+4)
		next_pos -= 8;
	if(next_pos < 0)
		next_pos = 0;
	else if(next_pos > (tracks-1)*4)
		next_pos = (tracks-1)*4;
	cyl = next_pos >> 2;
	subcyl = next_pos & 3;

	cache_clear();

	if(TRACE_STEP && (next_pos != cur_pos))
		logerror("track %d.%d\n", cyl, subcyl);

	/* Update disk detection if applicable */
	if (exists() && !dskchg_writable)
		if (dskchg==0)
			dskchg = 1;
}

// From http://burtleburtle.net/bob/hash/integer.html
uint32_t floppy_image_device::hash32(uint32_t a) const
{
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

int floppy_image_device::find_index(uint32_t position, const std::vector<uint32_t> &buf)const
{
	int spos = (buf.size() >> 1)-1;
	int step;
	for(step=1; step<buf.size()+1; step<<=1) { }
	step >>= 1;

	for(;;) {
		if(spos >= int(buf.size()) || (spos > 0 && (buf[spos] & floppy_image::TIME_MASK) > position)) {
			spos -= step;
			step >>= 1;
		} else if(spos < 0 || (spos < int(buf.size())-1 && (buf[spos+1] & floppy_image::TIME_MASK) <= position)) {
			spos += step;
			step >>= 1;
		} else
			return spos;
	}
}

uint32_t floppy_image_device::find_position(attotime &base, const attotime &when)
{
	base = revolution_start_time;
	attotime delta = when - base;

	while(delta >= rev_time) {
		delta -= rev_time;
		base += rev_time;
	}
	while(delta < attotime::zero) {
		delta += rev_time;
		base -= rev_time;
	}

	uint32_t res = (delta*floppy_ratio_1).as_ticks(1000000000/1000);
	if (res >= 200000000) {
		// Due to rounding errors in the previous operation,
		// 'res' sometimes overflows 2E+8
		res -= 200000000;
		base += rev_time;
	}
	return res;
}

bool floppy_image_device::test_track_last_entry_warps(const std::vector<uint32_t> &buf) const
{
	return !((buf[buf.size() - 1]^buf[0]) & floppy_image::MG_MASK);
}

attotime floppy_image_device::position_to_time(const attotime &base, int position) const
{
	return base + attotime::from_nsec((int64_t(position)*2000/floppy_ratio_1+1)/2);
}

void floppy_image_device::cache_fill_index(const std::vector<uint32_t> &buf, int &index, attotime &base)
{
	int cells = buf.size();

	if(index != 0 || !test_track_last_entry_warps(buf)) {
		cache_index = index;
		cache_start_time = position_to_time(base, buf[index] & floppy_image::TIME_MASK);
	} else {
		cache_index = cells - 1;
		cache_start_time = position_to_time(base - rev_time, buf[cache_index] & floppy_image::TIME_MASK);
	}

	cache_entry = buf[cache_index];

	index ++;
	if(index >= cells) {
		index = test_track_last_entry_warps(buf) ? 1 : 0;
		base += rev_time;
	}

	cache_end_time = position_to_time(base, buf[index] & floppy_image::TIME_MASK);
}

void floppy_image_device::cache_clear()
{
	cache_start_time = cache_end_time = cache_weak_start = attotime::zero;
	cache_index = 0;
	cache_entry = 0;
	cache_weak = false;
}

void floppy_image_device::cache_fill(const attotime &when)
{
	std::vector<uint32_t> &buf = image->get_buffer(cyl, ss, subcyl);
	uint32_t cells = buf.size();
	if(cells <= 1) {
		cache_start_time = attotime::zero;
		cache_end_time = attotime::never;
		cache_index = 0;
		cache_entry = cells == 1 ? buf[0] : floppy_image::MG_N;
		cache_weakness_setup();
		return;
	}

	attotime base;
	uint32_t position = find_position(base, when);

	int index = find_index(position, buf);

	if(index == -1) {
		// I suspect this should be an abort(), to check...
		cache_start_time = attotime::zero;
		cache_end_time = attotime::never;
		cache_index = 0;
		cache_entry = buf[0];
		cache_weakness_setup();
		return;
	}

	for(;;) {
		cache_fill_index(buf, index, base);
		if(cache_end_time > when) {
			cache_weakness_setup();
			return;
		}
	}
}

void floppy_image_device::cache_weakness_setup()
{
	u32 type = cache_entry & floppy_image::MG_MASK;
	if(type == floppy_image::MG_N || type == floppy_image::MG_D) {
		cache_weak = true;
		cache_weak_start = cache_start_time;
		return;
	}

	cache_weak = cache_end_time.is_never() || (cache_end_time - cache_start_time >= attotime::from_usec(16));
	if(!cache_weak) {
		cache_weak_start = attotime::never;
		return;
	}
	cache_weak_start = cache_start_time + attotime::from_usec(16);
}

attotime floppy_image_device::get_next_transition(const attotime &from_when)
{
	if(!image || mon)
		return attotime::never;

	if(from_when < cache_start_time || (!cache_end_time.is_never() && from_when >= cache_end_time))
		cache_fill(from_when);

	if(!cache_weak)
		return cache_end_time;

	// Put a flux transition in the middle of a 4us interval with a 50% probability
	int interval_index = (from_when - cache_weak_start).as_ticks(250000);
	if(interval_index < 0)
		interval_index = 0;
	attotime weak_time = cache_weak_start + attotime::from_ticks(interval_index*2+1, 500000);
	for(;;) {
		if(weak_time >= cache_end_time)
			return cache_end_time;
		if(weak_time > from_when) {
			u32 test = hash32(hash32(hash32(hash32(revolution_count) ^ 0x4242) + cache_index) + interval_index);
			if(test & 1)
				return weak_time;
		}
		weak_time += attotime::from_usec(4);
		interval_index ++;
	}
}

void floppy_image_device::write_flux(const attotime &start, const attotime &end, int transition_count, const attotime *transitions)
{
	if(!image || mon)
		return;
	image_dirty = true;

	attotime base;
	int start_pos = find_position(base, start);
	int end_pos   = find_position(base, end);

	std::vector<int> trans_pos(transition_count);
	for(int i=0; i != transition_count; i++)
		trans_pos[i] = find_position(base, transitions[i]);

	std::vector<uint32_t> &buf = image->get_buffer(cyl, ss, subcyl);

	int index;
	if(!buf.empty())
		index = find_index(start_pos, buf);
	else {
		index = 0;
		buf.push_back(floppy_image::MG_N);
	}

	if(index && (buf[index] & floppy_image::TIME_MASK) == start_pos)
		index--;

	uint32_t cur_mg = buf[index] & floppy_image::MG_MASK;
	if(cur_mg == floppy_image::MG_N || cur_mg == floppy_image::MG_D)
		cur_mg = floppy_image::MG_A;

	uint32_t pos = start_pos;
	int ti = 0;
	int cells = buf.size();
	while(pos != end_pos) {
		if(buf.size() < cells+10)
			buf.resize(cells+200);
		uint32_t next_pos;
		if(ti != transition_count)
			next_pos = trans_pos[ti++];
		else
			next_pos = end_pos;
		if(next_pos > pos)
			write_zone(&buf[0], cells, index, pos, next_pos, cur_mg);
		else {
			write_zone(&buf[0], cells, index, pos, 200000000, cur_mg);
			index = 0;
			write_zone(&buf[0], cells, index, 0, next_pos, cur_mg);
		}
		pos = next_pos;
		cur_mg = cur_mg == floppy_image::MG_A ? floppy_image::MG_B : floppy_image::MG_A;
	}

	buf.resize(cells);
}

void floppy_image_device::write_zone(uint32_t *buf, int &cells, int &index, uint32_t spos, uint32_t epos, uint32_t mg)
{
	cache_clear();
	while(spos < epos) {
		while(index != cells-1 && (buf[index+1] & floppy_image::TIME_MASK) <= spos)
			index++;

		uint32_t ref_start = buf[index] & floppy_image::TIME_MASK;
		uint32_t ref_end   = index == cells-1 ? 200000000 : buf[index+1] & floppy_image::TIME_MASK;
		uint32_t ref_mg    = buf[index] & floppy_image::MG_MASK;

		// Can't overwrite a damaged zone
		if(ref_mg == floppy_image::MG_D) {
			spos = ref_end;
			continue;
		}

		// If the zone is of the type we want, we don't need to touch it
		if(ref_mg == mg) {
			spos = ref_end;
			continue;
		}

		//  Check the overlaps, act accordingly
		if(spos == ref_start) {
			if(epos >= ref_end) {
				// Full overlap, that cell is dead, we need to see which ones we can extend
				uint32_t prev_mg = index != 0       ? buf[index-1] & floppy_image::MG_MASK : ~0;
				uint32_t next_mg = index != cells-1 ? buf[index+1] & floppy_image::MG_MASK : ~0;
				if(prev_mg == mg) {
					if(next_mg == mg) {
						// Both match, merge all three in one
						memmove(buf+index, buf+index+2, (cells-index-2)*sizeof(uint32_t));
						cells -= 2;
						index--;

					} else {
						// Previous matches, drop the current cell
						memmove(buf+index, buf+index+1, (cells-index-1)*sizeof(uint32_t));
						cells --;
					}

				} else {
					if(next_mg == mg) {
						// Following matches, extend it
						memmove(buf+index, buf+index+1, (cells-index-1)*sizeof(uint32_t));
						cells --;
						buf[index] = mg | spos;
					} else {
						// None match, convert the current cell
						buf[index] = mg | spos;
						index++;
					}
				}
				spos = ref_end;

			} else {
				// Overlap at the start only
				// Check if we can just extend the previous cell
				if(index != 0 && (buf[index-1] & floppy_image::MG_MASK) == mg)
					buf[index] = ref_mg | epos;
				else {
					// Otherwise we need to insert a new cell
					if(index != cells-1)
						memmove(buf+index+1, buf+index, (cells-index)*sizeof(uint32_t));
					cells++;
					buf[index] = mg | spos;
					buf[index+1] = ref_mg | epos;
				}
				spos = epos;
			}

		} else {
			if(epos >= ref_end) {
				// Overlap at the end only
				// If we can't just extend the following cell, we need to insert a new one
				if(index == cells-1 || (buf[index+1] & floppy_image::MG_MASK) != mg) {
					if(index != cells-1)
						memmove(buf+index+2, buf+index+1, (cells-index-1)*sizeof(uint32_t));
					cells++;
				}
				buf[index+1] = mg | spos;
				index++;
				spos = ref_end;

			} else {
				// Full inclusion
				// We need to split the zone in 3
				if(index != cells-1)
					memmove(buf+index+3, buf+index+1, (cells-index-1)*sizeof(uint32_t));
				cells += 2;
				buf[index+1] = mg | spos;
				buf[index+2] = ref_mg | epos;
				spos = epos;
			}
		}

	}
}

void floppy_image_device::set_write_splice(const attotime &when)
{
	if(image) {
		image_dirty = true;
		attotime base;
		int splice_pos = find_position(base, when);
		image->set_write_splice_position(cyl, ss, splice_pos, subcyl);
	}
}

uint32_t floppy_image_device::get_form_factor() const
{
	return form_factor;
}

uint32_t floppy_image_device::get_variant() const
{
	return image ? image->get_variant() : 0;
}

//===================================================================
//   Floppy sound
//
//   In order to enable floppy sound you must call
//      enable_sound(true)
//   and you must put audio samples (44100Hz, mono) with names as
//   shown in floppy_sample_names into the directory samples/floppy
//   Sound will be disabled when these samples are missing.
//
//   MZ, Aug 2015
//===================================================================

enum
{
	QUIET=-1,
	SPIN_START_EMPTY=0,
	SPIN_START_LOADED,
	SPIN_EMPTY,
	SPIN_LOADED,
	SPIN_END
};

enum
{
	STEP_SINGLE=0,
	STEP_SEEK2,
	STEP_SEEK6,
	STEP_SEEK12,
	STEP_SEEK20
};

/*
    Unless labeled "constructed", all samples were recorded from real floppy drives.
    The 3.5" floppy drive is a Sony MPF420-1.
    The 5.25" floppy drive is a Chinon FZ502.
*/
static const char *const floppy35_sample_names[] =
{
// Subdirectory
	"*floppy",
// Spinning sounds
	"35_spin_start_empty",
	"35_spin_start_loaded",
	"35_spin_empty",
	"35_spin_loaded",
	"35_spin_end",
// Stepping sounds
	"35_step_1_1",
// Seeking sounds
	"35_seek_2ms",      // constructed
	"35_seek_6ms",
	"35_seek_12ms",
	"35_seek_20ms",
	nullptr
};

static const char *const floppy525_sample_names[] =
{
// Subdirectory
	"*floppy",
// Spinning sounds
	"525_spin_start_empty",
	"525_spin_start_loaded",
	"525_spin_empty",
	"525_spin_loaded",
	"525_spin_end",
// Stepping sounds
	"525_step_1_1",
// Seeking sounds
	"525_seek_2ms",    // unrealistically fast, but needed for 3.5 (constructed)
	"525_seek_6ms",
	"525_seek_12ms",
	"525_seek_20ms",
	nullptr
};

floppy_sound_device::floppy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: samples_device(mconfig, FLOPPYSOUND, tag, owner, clock),
		m_sound(nullptr),
		m_step_base(0),
		m_spin_samples(0),
		m_step_samples(0),
		m_spin_samplepos(0),
		m_step_samplepos(0),
		m_seek_sound_timeout(0),
		m_zones(0),
		m_spin_playback_sample(QUIET),
		m_step_playback_sample(QUIET),
		m_seek_playback_sample(QUIET),
		m_motor_on(false),
		m_with_disk(false),
		m_loaded(false),
		m_seek_pitch(1.0),
		m_seek_samplepos(0.0)
{
}

void floppy_sound_device::register_for_save_states()
{
	save_item(NAME(m_step_base));
	save_item(NAME(m_spin_samples));
	save_item(NAME(m_step_samples));
	save_item(NAME(m_spin_samplepos));
	save_item(NAME(m_step_samplepos));
	save_item(NAME(m_seek_samplepos));
	save_item(NAME(m_seek_sound_timeout));
	save_item(NAME(m_zones));
	save_item(NAME(m_spin_playback_sample));
	save_item(NAME(m_step_playback_sample));
	save_item(NAME(m_seek_playback_sample));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_with_disk));
	save_item(NAME(m_loaded));
	save_item(NAME(m_seek_pitch));
}

void floppy_sound_device::device_start()
{
	// What kind of drive do we have?
	bool is525 = strstr(tag(), "525") != nullptr;
	set_samples_names(is525? floppy525_sample_names : floppy35_sample_names);

	m_motor_on = false;

	// Offsets in the sample collection
	m_spin_samples = 5;
	m_step_base = 5;
	m_step_samples = 1;
	m_zones = 1;             // > 1 needs more than one step sample

	m_spin_samplepos = m_step_samplepos = m_seek_samplepos = 0;
	m_spin_playback_sample = m_step_playback_sample = QUIET;

	// Read audio samples. The samples are stored in the list m_samples.
	m_loaded = load_samples();

	// If we don't have all samples, don't allocate a stream or access sample data.
	if (m_loaded)
	{
		m_sound = machine().sound().stream_alloc(*this, 0, 1, clock()); // per-floppy stream
	}
	register_for_save_states();
}

/*
    Motor sound. Select appropriate sound sample, depending on whether the
    motor is started or keeps running. Motor samples are always fully
    played.
*/
void floppy_sound_device::motor(bool running, bool withdisk)
{
	if (samples_loaded())
	{
		m_sound->update(); // required

		if ((m_spin_playback_sample==QUIET || m_spin_playback_sample==SPIN_END) && running) // motor was either off or already spinning down
		{
			m_spin_samplepos = 0;
			m_spin_playback_sample = withdisk? SPIN_START_LOADED : SPIN_START_EMPTY; // (re)start the motor sound
		}
		else
		{
			// Motor has been running and is turned off now
			if ((m_spin_playback_sample == SPIN_EMPTY || m_spin_playback_sample == SPIN_LOADED) && !running)
				m_spin_playback_sample = SPIN_END; // go to spin down sound when loop is finished
		}
	}
	m_motor_on = running;
	m_with_disk = withdisk;
}

/*
    Activate the step sound.
    The zone parameter should be used to select specific samples for the
    current head position (if available). Its value should range from 0 to 4.
*/
void floppy_sound_device::step(int zone)
{
	if (samples_loaded())
	{
		m_sound->update();  // required

		// Pick one of the step samples
		// TODO: This is only preliminary, need to complete that.
		if (zone >= m_zones) zone = m_zones-1;
		m_step_playback_sample = (zone * m_step_samples) + (machine().rand() % m_step_samples);

		if (m_step_samplepos > 0)
		{
			if (m_seek_playback_sample == QUIET)
			{
				// The last step sample was not completed;
				// we need to find out the step rate
				// With a sample rate of 44100 Hz we can calculate the
				// rate from the sample position
				// 2ms = 88
				// 6ms = 265
				// 12ms = 529
				// 20ms = 882

				if (m_step_samplepos < 100)
				{
					// Should only used for 3.5 drives
					m_seek_playback_sample = STEP_SEEK2;
					m_seek_pitch = 1.0;  // don't use a pitch
				}
				else
				{
					if (m_step_samplepos < 400)       // Use this for 8 ms also
					{
						m_seek_playback_sample = STEP_SEEK6;
						m_seek_pitch = 265.0 / m_step_samplepos;
					}
					else
					{
						if (m_step_samplepos < 600)
						{
							m_seek_playback_sample = STEP_SEEK12;
							m_seek_pitch = 529.0 / m_step_samplepos;
						}
						else
						{
							if (m_step_samplepos < 1200)
							{
								m_seek_playback_sample = STEP_SEEK20;
								m_seek_pitch = 882.0 / m_step_samplepos;
							}
							else
								// For 30ms and longer we replay the step sound
								m_seek_playback_sample = QUIET;
						}
					}
				}
			}

			// Changing the pitch does not always sound convincing
			if (!PITCH_SEEK_SAMPLES) m_seek_pitch = 1;

			if (TRACE_AUDIO) logerror("Seek sample = %d, pitch = %f\n", m_seek_playback_sample, m_seek_pitch);

			// Set the timeout for the seek sound. When it expires,
			// we assume that the seek process is over, and we'll play the
			// rest of the step sound.
			// This will be retriggered with each step pulse.
			m_seek_sound_timeout = m_step_samplepos * 2;
		}
		else
		{
			// Last step sample was completed, this is not a seek process
			m_seek_playback_sample = QUIET;
		}

		// If we switch to the seek sample, let's keep the position of the
		// step sample; else reset the step sample position.
		if (m_seek_playback_sample == QUIET)
			m_step_samplepos = 0;
	}
}

//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void floppy_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// We are using only one stream, unlike the parent class
	// Also, there is no need for interpolation, as we only expect
	// one sample rate of 44100 for all samples

	int16_t out;
	stream_sample_t *samplebuffer = outputs[0];
	int idx = 0;
	int sampleend = 0;

	while (samples-- > 0)
	{
		out = 0;

		// Motor sound
		if (m_spin_playback_sample != QUIET)
		{
			idx = m_spin_playback_sample;
			sampleend = m_sample[idx].data.size();
			out = m_sample[idx].data[m_spin_samplepos++];

			if (m_spin_samplepos >= sampleend)
			{
				// Motor sample has completed
				switch (m_spin_playback_sample)
				{
				case SPIN_START_EMPTY:
					// After start, switch to the continued spinning sound
					m_spin_playback_sample = SPIN_EMPTY; // move to looping sound
					break;
				case SPIN_START_LOADED:
					// After start, switch to the continued spinning sound
					m_spin_playback_sample = SPIN_LOADED; // move to looping sound
					break;
				case SPIN_EMPTY:
					// As long as the motor pin is asserted, restart the sample
					// play the spindown sample
					if (!m_motor_on) m_spin_playback_sample = SPIN_END; // motor was turned off already (during spin-up maybe) -> spin down
					break;
				case SPIN_LOADED:
					if (!m_motor_on) m_spin_playback_sample = SPIN_END; // motor was turned off already (during spin-up maybe) -> spin down
					break;
				case SPIN_END:
					// Spindown sample over, be quiet or restart if the
					// motor has been restarted
					if (m_motor_on)
						m_spin_playback_sample = m_with_disk? SPIN_START_LOADED : SPIN_START_EMPTY;
					else
						m_spin_playback_sample = QUIET;
					break;
				}
				// Restart the selected sample
				m_spin_samplepos = 0;
			}
		}

		// Seek sound
		// As long as we have a seek sound, there is no step sound
		if (m_seek_sound_timeout == 1)
		{
			// Not retriggered; switch back to the last step sound
			m_seek_playback_sample = QUIET;
			m_seek_sound_timeout = 0;
			// Skip 1/100 sec to dampen the loudest pulse
			// yep, a somewhat dirty trick; we don't have to record yet another sample
			m_step_samplepos += 441;
		}

		if (m_seek_playback_sample != QUIET)
		{
			m_seek_sound_timeout--;

			idx = m_step_base + m_seek_playback_sample;
			sampleend = m_sample[idx].data.size();
			// Mix it into the stream value
			out += m_sample[idx].data[(int)m_seek_samplepos];
			// By adding different values than 1, we can change the playback speed
			// This will be used to adjust the seek sound
			m_seek_samplepos += m_seek_pitch;

			// The seek sample will be replayed without interrupt
			if (m_seek_samplepos >= sampleend)
				m_seek_samplepos = 0;
		}
		else
		{
			// Stepper sound
			if (m_step_playback_sample != QUIET)
			{
				idx = m_step_base + m_step_playback_sample;
				sampleend = m_sample[idx].data.size();

				// Mix it into the stream value
				out += m_sample[idx].data[m_step_samplepos++];
				if (m_step_samplepos >= sampleend)
				{
					// Step sample done
					m_step_samplepos = 0;
					m_step_playback_sample = QUIET;
				}
			}
		}

		// Write to the stream buffer
		*(samplebuffer++) = out;
	}
}

#define FLOPSPK "flopsndout"

void floppy_image_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, FLOPSPK).front_center();
	FLOPPYSOUND(config, FLOPSND_TAG, 44100).add_route(ALL_OUTPUTS, FLOPSPK, 0.5);
}


DEFINE_DEVICE_TYPE(FLOPPYSOUND, floppy_sound_device, "flopsnd", "Floppy sound")


//**************************************************************************
//  GENERIC FLOPPY DRIVE DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  3" single-sided double density
//-------------------------------------------------

floppy_3_ssdd::floppy_3_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_3_SSDD, tag, owner, clock)
{
}

floppy_3_ssdd::~floppy_3_ssdd()
{
}

void floppy_3_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_3;
	tracks = 42;
	sides = 1;
	set_rpm(300);
}

void floppy_3_ssdd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSDD;
}

//-------------------------------------------------
//  3" double-sided double density
//-------------------------------------------------

floppy_3_dsdd::floppy_3_dsdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_3_DSDD, tag, owner, clock)
{
}

floppy_3_dsdd::~floppy_3_dsdd()
{
}

void floppy_3_dsdd::setup_characteristics()
{
	form_factor = floppy_image::FF_3;
	tracks = 42;
	sides = 2;
	set_rpm(300);
}

void floppy_3_dsdd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

//-------------------------------------------------
//  3.5" single-sided double density
//-------------------------------------------------

floppy_35_ssdd::floppy_35_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_35_SSDD, tag, owner, clock)
{
}

floppy_35_ssdd::~floppy_35_ssdd()
{
}

void floppy_35_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 1;
	set_rpm(300);
}

void floppy_35_ssdd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

//-------------------------------------------------
//  3.5" double-sided double density
//-------------------------------------------------

floppy_35_dd::floppy_35_dd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_35_DD, tag, owner, clock)
{
}

floppy_35_dd::~floppy_35_dd()
{
}

void floppy_35_dd::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_dd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

//-------------------------------------------------
//  3.5" high density
//-------------------------------------------------

floppy_35_hd::floppy_35_hd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_35_HD, tag, owner, clock)
{
}

floppy_35_hd::~floppy_35_hd()
{
}

void floppy_35_hd::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_hd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSHD;
}

//-------------------------------------------------
//  3.5" extended density
//-------------------------------------------------

floppy_35_ed::floppy_35_ed(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_35_ED, tag, owner, clock)
{
}

floppy_35_ed::~floppy_35_ed()
{
}

void floppy_35_ed::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_ed::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSHD;
	variants[var_count++] = floppy_image::DSED;
}

//-------------------------------------------------
//  5.25" single-sided single density 35 tracks
//-------------------------------------------------

floppy_525_sssd_35t::floppy_525_sssd_35t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSSD_35T, tag, owner, clock)
{
}

floppy_525_sssd_35t::~floppy_525_sssd_35t()
{
}

void floppy_525_sssd_35t::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 35;
	sides = 1;
	set_rpm(300);
}

void floppy_525_sssd_35t::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  5.25" double-sided single density 35 tracks
//-------------------------------------------------

floppy_525_sd_35t::floppy_525_sd_35t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_SD_35T, tag, owner, clock)
{
}

floppy_525_sd_35t::~floppy_525_sd_35t()
{
}

void floppy_525_sd_35t::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 35;
	sides = 2;
	set_rpm(300);
}

void floppy_525_sd_35t::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  5.25" single-sided single density
//-------------------------------------------------

floppy_525_sssd::floppy_525_sssd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSSD, tag, owner, clock)
{
}

floppy_525_sssd::~floppy_525_sssd()
{
}

void floppy_525_sssd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 1;
	set_rpm(300);
}

void floppy_525_sssd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  5.25" double-sided single density
//-------------------------------------------------

floppy_525_sd::floppy_525_sd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_SD, tag, owner, clock)
{
}

floppy_525_sd::~floppy_525_sd()
{
}

void floppy_525_sd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 2;
	set_rpm(300);
}

void floppy_525_sd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  5.25" single-sided double density
//-------------------------------------------------

floppy_525_ssdd::floppy_525_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSDD, tag, owner, clock)
{
}

floppy_525_ssdd::~floppy_525_ssdd()
{
}

void floppy_525_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 1;
	set_rpm(300);
}

void floppy_525_ssdd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

//-------------------------------------------------
//  5.25" double-sided double density
//-------------------------------------------------

floppy_525_dd::floppy_525_dd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_DD, tag, owner, clock)
{
}

floppy_525_dd::~floppy_525_dd()
{
}

void floppy_525_dd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 2;
	set_rpm(300);
}

void floppy_525_dd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

//-------------------------------------------------
//  5.25" single-sided quad density
//-------------------------------------------------

floppy_525_ssqd::floppy_525_ssqd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSQD, tag, owner, clock)
{
}

floppy_525_ssqd::~floppy_525_ssqd()
{
}

void floppy_525_ssqd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 84;
	sides = 1;
	set_rpm(300);
}

void floppy_525_ssqd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
}

//-------------------------------------------------
//  5.25" double-sided quad density
//-------------------------------------------------

floppy_525_qd::floppy_525_qd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_QD, tag, owner, clock)
{
}

floppy_525_qd::~floppy_525_qd()
{
}

void floppy_525_qd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_525_qd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
	variants[var_count++] = floppy_image::DSSD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSQD;
}

//-------------------------------------------------
//  5.25" high density
//-------------------------------------------------

floppy_525_hd::floppy_525_hd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_HD, tag, owner, clock)
{
}

floppy_525_hd::~floppy_525_hd()
{
}

void floppy_525_hd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 84;
	sides = 2;
	set_rpm(360);
}

void floppy_525_hd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSQD;
	variants[var_count++] = floppy_image::DSHD;
}

//-------------------------------------------------
//  8" single-sided single density
//-------------------------------------------------

floppy_8_sssd::floppy_8_sssd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_8_SSSD, tag, owner, clock)
{
}

floppy_8_sssd::~floppy_8_sssd()
{
}

void floppy_8_sssd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 1;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_sssd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  8" double-sided single density
//-------------------------------------------------

floppy_8_dssd::floppy_8_dssd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_8_DSSD, tag, owner, clock)
{
}

floppy_8_dssd::~floppy_8_dssd()
{
}

void floppy_8_dssd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 2;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_dssd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::DSSD;
}

//-------------------------------------------------
//  8" single-sided double density
//-------------------------------------------------

floppy_8_ssdd::floppy_8_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_8_SSDD, tag, owner, clock)
{
}

floppy_8_ssdd::~floppy_8_ssdd()
{
}

void floppy_8_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 1;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_ssdd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

//-------------------------------------------------
//  8" double-sided double density
//-------------------------------------------------

floppy_8_dsdd::floppy_8_dsdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_8_DSDD, tag, owner, clock)
{
}

floppy_8_dsdd::~floppy_8_dsdd()
{
}

void floppy_8_dsdd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 2;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_dsdd::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}


//**************************************************************************
//  SPECIFIC FLOPPY DRIVE DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  epson smd-165
//
//  track to track: 6 ms
//  average: 97 ms
//  setting time: 15 ms
//  motor start time: 1 s
//
//-------------------------------------------------

epson_smd_165::epson_smd_165(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, EPSON_SMD_165, tag, owner, clock)
{
}

epson_smd_165::~epson_smd_165()
{
}

void epson_smd_165::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 40;
	sides = 2;
	set_rpm(300);
}

void epson_smd_165::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::DSSD;
}

//-------------------------------------------------
//  epson sd-320
//
//  track to track: 15 ms
//  average: 220 ms
//  setting time: 15 ms
//  head load time: 35 ms
//  motor start time: 0.5 s
//
//  dip switch ss1
//  1 = drive select 0
//  2 = drive select 1
//  3 = drive select 2
//  4 = drive select 3
//  5 = head load from pin 4
//  6 = head load from drive select
//
//  dic switch ss2
//  hs = load controlled by head-load
//  ms = load controlled by motor enable
//
//  dic switch ss3
//  ds = in-use led by drive select
//  hl = in-use led by head load
//
//-------------------------------------------------

epson_sd_320::epson_sd_320(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, EPSON_SD_320, tag, owner, clock)
{
}

epson_sd_320::~epson_sd_320()
{
}

void epson_sd_320::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 40;
	sides = 2;
	set_rpm(300);
}

void epson_sd_320::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

//-------------------------------------------------
//  epson sd-321
//
//  same as sd-320, but no head-load selenoid
//
//-------------------------------------------------

epson_sd_321::epson_sd_321(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, EPSON_SD_321, tag, owner, clock)
{
}

epson_sd_321::~epson_sd_321()
{
}

void epson_sd_321::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 40;
	sides = 2;
	set_rpm(300);
}

void epson_sd_321::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

//-------------------------------------------------
//  Sony OA-D31V
//
//  track to track: 15 ms
//  average: 365 ms
//  setting time: 15 ms
//  head load time: 60 ms
//
//-------------------------------------------------

sony_oa_d31v::sony_oa_d31v(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, SONY_OA_D31V, tag, owner, clock)
{
}

sony_oa_d31v::~sony_oa_d31v()
{
}

void sony_oa_d31v::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 70;
	sides = 1;
	dskchg_writable = true;
	set_rpm(600);
}

void sony_oa_d31v::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

//-------------------------------------------------
//  Sony OA-D32W
//
//  track to track: 12 ms
//  average: 350 ms
//  setting time: 30 ms
//  head load time: 60 ms
//  average latency: 50 ms
//
//-------------------------------------------------

sony_oa_d32w::sony_oa_d32w(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, SONY_OA_D32W, tag, owner, clock)
{
}

sony_oa_d32w::~sony_oa_d32w()
{
}

void sony_oa_d32w::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 80;
	sides = 2;
	dskchg_writable = true;
	set_rpm(600);
}

void sony_oa_d32w::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

//-------------------------------------------------
//  Sony OA-D32V
//
//  track to track: 12 ms
//  average: 350 ms
//  setting time: 30 ms
//  head load time: 60 ms
//  average latency: 50 ms
//
//-------------------------------------------------

sony_oa_d32v::sony_oa_d32v(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, SONY_OA_D32V, tag, owner, clock)
{
}

sony_oa_d32v::~sony_oa_d32v()
{
}

void sony_oa_d32v::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 80;
	sides = 1;
	dskchg_writable = true;
	set_rpm(600);
}

void sony_oa_d32v::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

//-------------------------------------------------
//  TEAC FD-30A
//
//  track to track: 12 ms
//  average: 171 ms
//  setting time: 15 ms
//  motor start time: 400 ms
//
//-------------------------------------------------

teac_fd_30a::teac_fd_30a(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, TEAC_FD_30A, tag, owner, clock)
{
}

teac_fd_30a::~teac_fd_30a()
{
}

void teac_fd_30a::setup_characteristics()
{
	form_factor = floppy_image::FF_3;
	tracks = 40;
	sides = 1;
	set_rpm(300);
}

void teac_fd_30a::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  TEAC FD-55E
//
//  track to track: 3 ms
//  average: 94 ms
//  setting time: 15 ms
//  motor start time: 400 ms
//
//-------------------------------------------------

teac_fd_55e::teac_fd_55e(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, TEAC_FD_55E, tag, owner, clock)
{
}

teac_fd_55e::~teac_fd_55e()
{
}

void teac_fd_55e::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 80;
	sides = 1;
	set_rpm(300);
}

void teac_fd_55e::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
}

//-------------------------------------------------
//  TEAC FD-55F
//
//  track to track: 3 ms
//  average: 94 ms
//  setting time: 15 ms
//  motor start time: 400 ms
//
//-------------------------------------------------

teac_fd_55f::teac_fd_55f(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, TEAC_FD_55F, tag, owner, clock)
{
}

teac_fd_55f::~teac_fd_55f()
{
}

void teac_fd_55f::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 80;
	sides = 2;
	set_rpm(300);
}

void teac_fd_55f::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
	variants[var_count++] = floppy_image::DSSD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSQD;
}

//-------------------------------------------------
//  TEAC FD-55G
//
//  track to track: 3 ms
//  average: 91 ms
//  setting time: 15 ms
//  motor start time: 400 ms
//
//-------------------------------------------------

teac_fd_55g::teac_fd_55g(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, TEAC_FD_55G, tag, owner, clock)
{
}

teac_fd_55g::~teac_fd_55g()
{
}

void teac_fd_55g::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 77;
	sides = 2;
	set_rpm(360);
}

void teac_fd_55g::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSQD;
	variants[var_count++] = floppy_image::DSHD;
}

//-------------------------------------------------
//  ALPS 32551901 (black) / 32551902 (brown)
//
//  used in the Commodoere 1541 disk drive
//-------------------------------------------------

alps_3255190x::alps_3255190x(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, ALPS_3255190X, tag, owner, clock)
{
}

alps_3255190x::~alps_3255190x()
{
}

void alps_3255190x::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 84;
	sides = 1;
	set_rpm(300);
	cyl = 34;
}

void alps_3255190x::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

//-------------------------------------------------
//  IBM 6360 -- 8" single-sided single density
//-------------------------------------------------

ibm_6360::ibm_6360(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, IBM_6360, tag, owner, clock)
{
}

ibm_6360::~ibm_6360()
{
}

void ibm_6360::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 1;
	motor_always_on = true;
	has_trk00_sensor = false;
	set_rpm(360);
}

void ibm_6360::handled_variants(uint32_t *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}
