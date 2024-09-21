// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Olivier Galibert, Miodrag Milanovic
/*********************************************************************



*********************************************************************/

#include "emu.h"
#include "floppy.h"

#include "formats/d88_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/cqm_dsk.h"
#include "formats/dsk_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/ipf_dsk.h"

#include "formats/fs_unformatted.h"
#include "formats/fsblk_vec.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/imageutl.h"

#include "util/ioprocs.h"
#include "util/ioprocsfilter.h"
#include "util/zippath.h"

#include <algorithm>

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
DEFINE_DEVICE_TYPE(FLOPPY_3_SSSD, floppy_3_sssd, "floppy_3_sssd", "3\" single-sided single density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_3_DSSD, floppy_3_dssd, "floppy_3_dssd", "3\" double-sided single density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_3_SSDD, floppy_3_ssdd, "floppy_3_ssdd", "3\" single-sided double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_3_DSDD, floppy_3_dsdd, "floppy_3_dsdd", "3\" double-sided double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_3_DSQD, floppy_3_dsqd, "floppy_3_dsqd", "3\" double-sided quad density floppy drive")

// generic 3.5" drives
DEFINE_DEVICE_TYPE(FLOPPY_35_SSDD, floppy_35_ssdd, "floppy_35_ssdd", "3.5\" single-sided double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_35_DD,   floppy_35_dd,   "floppy_35_dd",   "3.5\" double density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_35_HD,   floppy_35_hd,   "floppy_35_hd",   "3.5\" high density floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_35_ED,   floppy_35_ed,   "floppy_35_ed",   "3.5\" extended density floppy drive")

// generic 5.25" drives
DEFINE_DEVICE_TYPE(FLOPPY_525_SSSD_35T, floppy_525_sssd_35t, "floppy_525_sssd_35t", "5.25\" single-sided single density 35-track floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_SD_35T,   floppy_525_sd_35t,   "floppy_525_sd_35t",   "5.25\" double-sided single density 35-track floppy drive")
DEFINE_DEVICE_TYPE(FLOPPY_525_VTECH,    floppy_525_vtech,    "floppy_525_vtech",    "5.25\" single-sided single density VTECH floppy drive")
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

// Panasonic 3.5" drive
DEFINE_DEVICE_TYPE(PANA_JU_363, pana_ju_363, "pana_ju_363", "Panasonic JU-363 Flexible Disk Drive")

// Sony 3.5" drives
DEFINE_DEVICE_TYPE(SONY_OA_D31V, sony_oa_d31v, "sony_oa_d31v", "Sony OA-D31V Micro Floppydisk Drive")
DEFINE_DEVICE_TYPE(SONY_OA_D32W, sony_oa_d32w, "sony_oa_d32w", "Sony OA-D32W Micro Floppydisk Drive")
DEFINE_DEVICE_TYPE(SONY_OA_D32V, sony_oa_d32v, "sony_oa_d32v", "Sony OA-D32V Micro Floppydisk Drive")

// TEAC 3" drives
DEFINE_DEVICE_TYPE(TEAC_FD_30A, teac_fd_30a, "teac_fd_30a", "TEAC FD-30A FDD")

// TEAC 5.25" drives
DEFINE_DEVICE_TYPE(TEAC_FD_55A, teac_fd_55a, "teac_fd_55a", "TEAC FD-55A FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55B, teac_fd_55b, "teac_fd_55b", "TEAC FD-55B FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55E, teac_fd_55e, "teac_fd_55e", "TEAC FD-55E FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55F, teac_fd_55f, "teac_fd_55f", "TEAC FD-55F FDD")
DEFINE_DEVICE_TYPE(TEAC_FD_55G, teac_fd_55g, "teac_fd_55g", "TEAC FD-55G FDD")

// ALPS 5.25" drives
DEFINE_DEVICE_TYPE(ALPS_3255190X, alps_3255190x, "alps_3255190x", "ALPS 32551901/32551902 Floppy Drive")

// IBM 8" drives
DEFINE_DEVICE_TYPE(IBM_6360, ibm_6360, "ibm_6360", "IBM 6360 8\" single-sided single density floppy drive")

// Mac 3.5" drives
DEFINE_DEVICE_TYPE(OAD34V, oa_d34v_device, "oa_d34v", "Apple/Sony 3.5 SD (400K GCR)")
DEFINE_DEVICE_TYPE(MFD51W, mfd51w_device,  "mfd51w",  "Apple/Sony 3.5 DD (400/800K GCR)")
DEFINE_DEVICE_TYPE(MFD75W, mfd75w_device,  "mfd75w",  "Apple/Sony 3.5 HD (Superdrive)")


format_registration::format_registration()
{
	add(FLOPPY_MFI_FORMAT); // Our generic format
	add(FLOPPY_DFI_FORMAT); // Flux format, dying

	add(fs::UNFORMATTED);
}

void format_registration::add_fm_containers()
{
	add(FLOPPY_MFM_FORMAT);
	add(FLOPPY_TD0_FORMAT);
	add(FLOPPY_IMD_FORMAT);
}

void format_registration::add_mfm_containers()
{
	add_fm_containers();

	add(FLOPPY_D88_FORMAT);
	add(FLOPPY_CQM_FORMAT);
	add(FLOPPY_DSK_FORMAT);
}

void format_registration::add_pc_formats()
{
	add_mfm_containers();

	add(FLOPPY_PC_FORMAT);
	add(FLOPPY_IPF_FORMAT);
}

void format_registration::add(const floppy_image_format_t &format)
{
	m_formats.push_back(&format);
}

void format_registration::add(const fs::manager_t &fs)
{
	m_fs.push_back(&fs);
}

void floppy_image_device::default_fm_floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
}

void floppy_image_device::default_mfm_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
}

void floppy_image_device::default_pc_floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
}

floppy_connector::floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FLOPPY_CONNECTOR, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	formats(nullptr),
	m_enable_sound(false),
	m_sectoring_type(floppy_image::SOFT)
{
}

floppy_connector::~floppy_connector()
{
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
		dev->set_sectoring_type(m_sectoring_type);
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
		m_input_format(nullptr),
		m_output_format(nullptr),
		m_image(),
		m_index_timer(nullptr),
		m_tracks(0),
		m_sides(0),
		m_form_factor(0),
		m_sectoring_type(floppy_image::SOFT),
		m_motor_always_on(false),
		m_dskchg_writable(false),
		m_has_trk00_sensor(true),
		m_dir(0), m_stp(0), m_wtg(0), m_mon(0), m_ss(0), m_ds(-1), m_idx(0), m_wpt(0), m_rdy(0), m_dskchg(0),
		m_ready(false),
		m_rpm(0),
		m_angular_speed(0),
		m_revolution_count(0),
		m_cyl(0),
		m_subcyl(0),
		m_amplifier_freakout_time(attotime::from_usec(16)),
		m_image_dirty(false),
		m_track_dirty(false),
		m_ready_counter(0),
		m_make_sound(false),
		m_sound_out(nullptr)
{
	m_extension_list[0] = '\0';
}

//-------------------------------------------------
//  floppy_image_device - destructor
//-------------------------------------------------

floppy_image_device::~floppy_image_device()
{
}

void floppy_image_device::setup_load_cb(load_cb cb)
{
	m_cur_load_cb = cb;
}

void floppy_image_device::setup_unload_cb(unload_cb cb)
{
	m_cur_unload_cb = cb;
}

void floppy_image_device::setup_index_pulse_cb(index_pulse_cb cb)
{
	m_cur_index_pulse_cb = cb;
}

void floppy_image_device::setup_ready_cb(ready_cb cb)
{
	m_cur_ready_cb = cb;
}

void floppy_image_device::setup_wpt_cb(wpt_cb cb)
{
	m_cur_wpt_cb = cb;
}

void floppy_image_device::setup_led_cb(led_cb cb)
{
	m_cur_led_cb = cb;
}

struct floppy_image_device::fs_enum : public fs::manager_t::floppy_enumerator {
	floppy_image_device *m_fid;
	const fs::manager_t *m_manager;

	fs_enum(floppy_image_device *fid);

	virtual void add_raw(const char *name, u32 key, const char *description) override;
protected:
	virtual void add_format(const floppy_image_format_t &type, u32 image_size, const char *name, const char *description) override;
};

floppy_image_device::fs_enum::fs_enum(floppy_image_device *fid)
	: fs::manager_t::floppy_enumerator(fid->m_form_factor, fid->m_variants)
	, m_fid(fid)
{
}

void floppy_image_device::fs_enum::add_format(const floppy_image_format_t &type, u32 image_size, const char *name, const char *description)
{
	m_fid->m_fs.emplace_back(fs_info(m_manager, &type, image_size, name, description));
}

void floppy_image_device::fs_enum::add_raw(const char *name, u32 key, const char *description)
{
	m_fid->m_fs.emplace_back(fs_info(name, key, description));
}

void floppy_image_device::register_formats()
{
	format_registration fr;
	if(m_format_registration_cb)
		m_format_registration_cb(fr);

	m_extension_list[0] = '\0';
	m_fif_list = std::move(fr.m_formats);
	for(const floppy_image_format_t *fif : m_fif_list)
	{
		add_format(fif->name(), fif->description(), fif->extensions(), "");
		image_specify_extension( m_extension_list, 256, fif->extensions() );
	}

	fs_enum fse(this);
	for(const fs::manager_t *fmt : fr.m_fs)
	{
		fse.m_manager = fmt;
		fmt->enumerate_f(fse);
		m_fs_managers.push_back(fmt);
	}
}

void floppy_image_device::add_variant(uint32_t variant)
{
	uint32_t actual_variant = variant;

	if (m_sectoring_type == floppy_image::H10) {
		switch (variant) {
		case floppy_image::SSSD:
			actual_variant = floppy_image::SSSD10;
			break;
		case floppy_image::SSDD:
			actual_variant = floppy_image::SSDD10;
			break;
		case floppy_image::SSQD:
			actual_variant = floppy_image::SSQD10;
			break;
		case floppy_image::DSSD:
			actual_variant = floppy_image::DSSD10;
			break;
		case floppy_image::DSDD:
			actual_variant = floppy_image::DSDD10;
			break;
		case floppy_image::DSQD:
			actual_variant = floppy_image::DSQD10;
			break;
		}
	} else if (m_sectoring_type == floppy_image::H16) {
		switch (variant) {
		case floppy_image::SSSD:
			actual_variant = floppy_image::SSDD16;
			break;
		case floppy_image::SSDD:
			actual_variant = floppy_image::SSSD16;
			break;
		case floppy_image::SSQD:
			actual_variant = floppy_image::SSQD16;
			break;
		case floppy_image::DSSD:
			actual_variant = floppy_image::DSSD16;
			break;
		case floppy_image::DSDD:
			actual_variant = floppy_image::DSDD16;
			break;
		case floppy_image::DSQD:
			actual_variant = floppy_image::DSQD16;
			break;
		}
	} else if (m_sectoring_type == floppy_image::H32) {
		switch (variant) {
		case floppy_image::SSSD:
			actual_variant = floppy_image::SSSD32;
			break;
		case floppy_image::SSDD:
			actual_variant = floppy_image::SSDD32;
			break;
		case floppy_image::DSSD:
			actual_variant = floppy_image::DSSD32;
			break;
		case floppy_image::DSDD:
			actual_variant = floppy_image::DSDD32;
			break;
		}
	}

	m_variants.push_back(actual_variant);
}

void floppy_image_device::set_formats(std::function<void (format_registration &fr)> formats)
{
	m_format_registration_cb = formats;
}

const std::vector<const floppy_image_format_t *> &floppy_image_device::get_formats() const
{
	return m_fif_list;
}

const floppy_image_format_t *floppy_image_device::get_load_format() const
{
	return m_input_format;
}

void floppy_image_device::set_rpm(float _rpm)
{
	if(m_rpm == _rpm)
		return;

	m_rpm = _rpm;
	m_rev_time = attotime::from_double(60/m_rpm);
	m_angular_speed = m_rpm/60.0*2e8;
}

void floppy_image_device::set_sectoring_type(uint32_t sectoring_type)
{
	m_sectoring_type = sectoring_type;
}

uint32_t floppy_image_device::get_sectoring_type()
{
	return m_sectoring_type;
}

void floppy_image_device::setup_write(const floppy_image_format_t *_output_format)
{
	m_output_format = _output_format;
	if(m_image)
		commit_image();
}

void floppy_image_device::commit_image()
{
	m_image_dirty = false;
	if(!m_output_format || !m_output_format->supports_save())
		return;

	check_for_file();
	auto io = util::random_read_write_fill(image_core_file(), 0xff);
	if(!io) {
		popmessage("Error, out of memory");
		return;
	}

	std::error_condition const err = image_core_file().truncate(0);
	if (err)
		popmessage("Error, unable to truncate image: %s", err.message());

	m_output_format->save(*io, m_variants, *m_image);
}

void floppy_image_device::device_config_complete()
{
	m_rpm = 0;
	m_motor_always_on = false;
	m_dskchg_writable = false;
	m_has_trk00_sensor = true;

	setup_characteristics();
	register_formats();
}

const software_list_loader &floppy_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_image_device::device_start()
{
	// better would be an extra parameter in the MCFG macro
	m_drive_index = atoi(owner()->basetag());

	m_idx = 0;

	/* motor off */
	m_mon = 1;

	m_cyl = 0;
	m_subcyl = 0;
	m_ss  = 0;
	m_actual_ss = 0;
	m_ds = -1;
	m_stp = 1;
	m_wpt = 0;
	m_dskchg = exists() ? 1 : 0;
	m_index_timer = timer_alloc(FUNC(floppy_image_device::index_resync), this);
	m_image_dirty = false;
	m_ready = true;
	m_ready_counter = 0;
	m_phases = 0;


	if (m_make_sound) m_sound_out = subdevice<floppy_sound_device>(FLOPSND_TAG);

	save_item(NAME(m_dir));
	save_item(NAME(m_stp));
	save_item(NAME(m_wtg));
	save_item(NAME(m_mon));
	save_item(NAME(m_ss));
	save_item(NAME(m_actual_ss));
	save_item(NAME(m_ds));
	save_item(NAME(m_idx));
	save_item(NAME(m_wpt));
	save_item(NAME(m_rdy));
	save_item(NAME(m_dskchg));
	save_item(NAME(m_ready));
	save_item(NAME(m_rpm));
	save_item(NAME(m_angular_speed));
	save_item(NAME(m_revolution_start_time));
	save_item(NAME(m_rev_time));
	save_item(NAME(m_revolution_count));
	save_item(NAME(m_cyl));
	save_item(NAME(m_subcyl));
	save_item(NAME(m_cache_start_time));
	save_item(NAME(m_cache_end_time));
	save_item(NAME(m_cache_index));
	save_item(NAME(m_cache_entry));
	save_item(NAME(m_cache_weak));
	save_item(NAME(m_image_dirty));
	save_item(NAME(m_ready_counter));
	save_item(NAME(m_phases));
}

void floppy_image_device::device_reset()
{
	if (m_make_sound)
	{
		// Have we loaded all samples? Otherwise mute the floppy.
		m_make_sound = m_sound_out->samples_loaded();
	}

	m_revolution_start_time = attotime::never;
	m_revolution_count = 0;
	m_mon = 1;
	set_ready(true);
	if(m_motor_always_on && m_image)
		mon_w(0);
	cache_clear();
}

std::pair<std::error_condition, const floppy_image_format_t *> floppy_image_device::identify(std::string_view filename)
{
	util::core_file::ptr fd;
	std::string revised_path;
	std::error_condition err = util::zippath_fopen(filename, OPEN_FLAG_READ, fd, revised_path);
	if(err)
		return{ err, nullptr };

	auto io = util::random_read_fill(std::move(fd), 0xff);
	if(!io)
		return{ std::errc::not_enough_memory, nullptr };

	int best = 0;
	const floppy_image_format_t *best_format = nullptr;
	for(const floppy_image_format_t *format : m_fif_list) {
		int score = format->identify(*io, m_form_factor, m_variants);
		if(score > best) {
			best = score;
			best_format = format;
		}
	}

	return{ std::error_condition(), best_format };
}

void floppy_image_device::init_floppy_load(bool write_supported)
{
	cache_clear();
	m_revolution_start_time = m_mon ? attotime::never : machine().time();
	m_revolution_count = 0;

	index_resync(0);

	m_wpt = 1; // disk sleeve is covering the sensor
	if (!m_cur_wpt_cb.isnull())
		m_cur_wpt_cb(this, m_wpt);

	m_wpt = is_readonly() || (!write_supported);
	if (!m_cur_wpt_cb.isnull())
		m_cur_wpt_cb(this, m_wpt);

	if (m_motor_always_on) {
		// When disk is inserted, start motor
		mon_w(0);
	} else if(!m_mon)
		m_ready_counter = 2;

	if (m_dskchg_writable)
		m_dskchg = 1;
}

std::pair<std::error_condition, std::string> floppy_image_device::call_load()
{
	check_for_file();
	auto io = util::random_read_fill(image_core_file(), 0xff);
	if(!io)
		return std::make_pair(std::errc::not_enough_memory, std::string());

	int best = 0;
	const floppy_image_format_t *best_format = nullptr;
	for (const floppy_image_format_t *format : m_fif_list) {
		int score = format->identify(*io, m_form_factor, m_variants);
		if(score && format->extension_matches(filename()))
			score |= floppy_image_format_t::FIFID_EXT;
		if(score > best) {
			best = score;
			best_format = format;
		}
	}

	if (!best_format)
		return std::make_pair(image_error::INVALIDIMAGE, "Unable to identify image file format");

	m_image = std::make_unique<floppy_image>(m_tracks, m_sides, m_form_factor);
	if (!best_format->load(*io, m_form_factor, m_variants, *m_image)) {
		m_image.reset();
		return std::make_pair(image_error::INVALIDIMAGE, "Incompatible image file format or corrupted data");
	}
	m_output_format = is_readonly() ? nullptr : best_format;

	m_image_dirty = false;

	init_floppy_load(m_output_format != nullptr);

	if (!m_cur_load_cb.isnull())
		m_cur_load_cb(this);

	return std::make_pair(std::error_condition(), std::string());
}

void floppy_image_device::call_unload()
{
	cache_clear();
	m_dskchg = 0;

	if (m_image) {
		if(m_image_dirty)
			commit_image();
		m_image.reset();
	}

	m_wpt = 1; // disk sleeve is covering the sensor
	if (!m_cur_wpt_cb.isnull())
		m_cur_wpt_cb(this, m_wpt);

	m_wpt = 0; // sensor is uncovered
	if (!m_cur_wpt_cb.isnull())
		m_cur_wpt_cb(this, m_wpt);

	if (!m_cur_unload_cb.isnull())
		m_cur_unload_cb(this);

	if (m_motor_always_on) {
		// When disk is removed, stop motor
		mon_w(1);
	}

	set_ready(true);
}

std::pair<std::error_condition, std::string> floppy_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	m_image = std::make_unique<floppy_image>(m_tracks, m_sides, m_form_factor);
	m_output_format = nullptr;

	// search for a suitable format based on the extension
	for(const floppy_image_format_t *i : m_fif_list)
	{
		// only consider formats that actually support saving
		if(!i->supports_save())
			continue;

		if (i->extension_matches(basename()))
		{
			m_output_format = i;
			break;
		}

		// Use MFI as a default.
		if (!strcmp(i->name(), "mfi"))
			m_output_format = i;
	}

	init_floppy_load(true);

	return std::make_pair(std::error_condition(), std::string());
}

void floppy_image_device::init_fs(const fs_info *fs, const fs::meta_data &meta)
{
	assert(m_image);
	if (fs->m_type) {
		std::vector<u8> img(fs->m_image_size);
		fs::fsblk_vec_t blockdev(img);
		auto cfs = fs->m_manager->mount(blockdev);
		cfs->format(meta);

		auto io = util::ram_read(img.data(), img.size(), 0xff);
		fs->m_type->load(*io, floppy_image::FF_UNKNOWN, m_variants, *m_image);
	} else {
		fs::unformatted_image::format(fs->m_key, m_image.get());
	}

	// intializing a file system makes the floppy dirty
	m_image_dirty = true;
}

/* write protect, active high
   phase 1 can force it to 1 for drive detection
   on the rare drives that actually use m_phases.
 */
bool floppy_image_device::wpt_r()
{
	return m_wpt || (m_phases & 2);
}

/* motor on, active low */
void floppy_image_device::mon_w(int state)
{
	if(m_mon == state)
		return;

	m_mon = state;

	/* off -> on */
	if (!m_mon && m_image)
	{
		m_revolution_start_time = machine().time();
		cache_clear();
		if (m_motor_always_on) {
			// Drives with motor that is always spinning are immediately ready when a disk is loaded
			// because there is no spin-up time
			set_ready(false);
		} else {
			m_ready_counter = 2;
		}
		index_resync(0);
	}

	/* on -> off */
	else {
		if(m_image_dirty)
			commit_image();
		cache_clear();
		m_revolution_start_time = attotime::never;
		m_index_timer->adjust(attotime::zero);
		set_ready(true);
	}

	// Create a motor sound (loaded or empty)
	if (m_make_sound) m_sound_out->motor(state==0, exists());
}

attotime floppy_image_device::time_next_index()
{
	if(m_revolution_start_time.is_never())
		return attotime::never;
	return m_revolution_start_time + m_rev_time;
}

/* index pulses at m_rpm/60 Hz, and stays high for ~2ms at 300rpm */
TIMER_CALLBACK_MEMBER(floppy_image_device::index_resync)
{
	if(m_revolution_start_time.is_never()) {
		if(m_idx) {
			m_idx = 0;
			if (!m_cur_index_pulse_cb.isnull())
				m_cur_index_pulse_cb(this, m_idx);
		}
		return;
	}

	attotime delta = machine().time() - m_revolution_start_time;
	while(delta >= m_rev_time) {
		delta -= m_rev_time;
		m_revolution_start_time += m_rev_time;
		m_revolution_count++;
	}
	int position = int(delta.as_double()*m_angular_speed + 0.5);

	uint32_t last_index = 0, next_index = 200000000;
	// if hard-sectored floppy, has extra IDX pulses
	if(m_image)
		m_image->find_index_hole(position, last_index, next_index);
	int new_idx = position - last_index < 2000000;

	if(new_idx) {
		uint32_t index_up = last_index + 2000000;
		attotime index_up_time = attotime::from_double(index_up/m_angular_speed);
		m_index_timer->adjust(index_up_time - delta);
	} else {
		attotime next_index_time = next_index >= 200000000 ? m_rev_time : attotime::from_double(next_index/m_angular_speed);
		m_index_timer->adjust(next_index_time - delta);
	}

	if(new_idx != m_idx) {
		m_idx = new_idx;
		if(m_idx && m_ready) {
			m_ready_counter--;
			if(!m_ready_counter) {
				// logerror("Drive spun up\n");
				set_ready(false);
			}
		}
		if (!m_cur_index_pulse_cb.isnull())
			m_cur_index_pulse_cb(this, m_idx);
	}
}

bool floppy_image_device::ready_r()
{
	return m_ready;
}

void floppy_image_device::set_ready(bool state)
{
	if (state != m_ready)
	{
		m_ready = state;
		check_led();
		if (!m_cur_ready_cb.isnull())
			m_cur_ready_cb(this, m_ready);
	}
}

void floppy_image_device::check_led()
{
	if(!m_cur_led_cb.isnull())
		m_cur_led_cb(this, (m_ds == m_drive_index) && !m_ready ? 1 : 0);
}

bool floppy_image_device::twosid_r()
{
	int tracks = 0, heads = 0;

	if (m_image) m_image->get_actual_geometry(tracks, heads);

	return heads == 1;
}

bool floppy_image_device::floppy_is_hd()
{
	if (!m_image)
		return false;
	u32 const variant = m_image->get_variant();
	return variant == floppy_image::DSHD;
}

bool floppy_image_device::floppy_is_ed()
{
	if (!m_image)
		return false;
	u32 const variant = m_image->get_variant();
	return variant == floppy_image::DSED;
}

void floppy_image_device::track_changed()
{
}

void floppy_image_device::stp_w(int state)
{
	// Before spin-up is done, ignore step pulses
	// TODO: There are reports about drives supporting step operation with
	// stopped spindle. Need to check that on real drives.
	// if (m_ready_counter > 0) return;

	if ( m_stp != state ) {
		cache_clear();
		m_stp = state;
		if ( m_stp == 0 ) {
			int ocyl = m_cyl;
			if ( m_dir ) {
				if ( m_cyl ) m_cyl--;
			} else {
				if ( m_cyl < m_tracks-1 ) m_cyl++;
			}
			if(ocyl != m_cyl)
			{
				if (TRACE_STEP) logerror("track %d\n", m_cyl);
				// Do we want a stepper sound?
				// We plan for 5 zones with possibly specific sounds
				if (m_make_sound) m_sound_out->step(m_cyl*5/m_tracks);
				track_changed();
			}
			/* Update disk detection if applicable */
			if (exists() && !m_dskchg_writable)
			{
				if (m_dskchg==0) m_dskchg = 1;
			}
		}
		m_subcyl = 0;
	}
}

void floppy_image_device::seek_phase_w(int _phases)
{
	m_phases = _phases;

	int cur_pos = (m_cyl << 2) | m_subcyl;
	int req_pos;
	switch(m_phases) {
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
	else if(next_pos > (m_tracks-1)*4)
		next_pos = (m_tracks-1)*4;

	m_cyl = next_pos >> 2;
	m_subcyl = next_pos & 3;

	cache_clear();

	if(next_pos != cur_pos) {
		if (TRACE_STEP) logerror("track %d.%d\n", m_cyl, m_subcyl);
		if (m_make_sound) m_sound_out->step(m_subcyl);
	}

	/* Update disk detection if applicable */
	if (exists() && !m_dskchg_writable)
		if (m_dskchg==0)
			m_dskchg = 1;
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

uint32_t floppy_image_device::find_position(attotime &base, const attotime &when)
{
	base = m_revolution_start_time;
	attotime delta = when - base;

	while(delta >= m_rev_time) {
		delta -= m_rev_time;
		base += m_rev_time;
	}
	while(delta < attotime::zero) {
		delta += m_rev_time;
		base -= m_rev_time;
	}

	uint32_t res = uint32_t(delta.as_double()*m_angular_speed+0.5);
	if (res >= 200000000) {
		// Due to rounding errors in the previous operation,
		// 'res' sometimes overflows 2E+8
		res -= 200000000;
		base += m_rev_time;
	}
	return res;
}

attotime floppy_image_device::position_to_time(const attotime &base, int position) const
{
	return base + attotime::from_double(position/m_angular_speed);
}

void floppy_image_device::cache_fill_index(const std::vector<uint32_t> &buf, int &index, attotime &base)
{
	int cells = buf.size();

	m_cache_index = index;
	m_cache_start_time = position_to_time(base, buf[index] & floppy_image::TIME_MASK);
	m_cache_entry = buf[m_cache_index];

	index ++;
	if(index >= cells) {
		index = 0;
		base += m_rev_time;
	}

	m_cache_end_time = position_to_time(base, buf[index] & floppy_image::TIME_MASK);
}

void floppy_image_device::cache_clear()
{
	m_cache_start_time = m_cache_end_time = m_cache_weak_start = attotime::zero;
	m_cache_index = 0;
	m_cache_entry = 0;
	m_cache_weak = false;
}

void floppy_image_device::cache_fill(const attotime &when)
{
	std::vector<uint32_t> &buf = m_image->get_buffer(m_cyl, m_ss, m_subcyl);
	uint32_t const cells = buf.size();
	if(cells <= 1) {
		m_cache_start_time = attotime::zero;
		m_cache_end_time = attotime::never;
		m_cache_index = 0;
		m_cache_entry = (cells == 1) ? buf[0] : floppy_image::MG_N;
		cache_weakness_setup();
		return;
	}

	attotime base;
	uint32_t position = find_position(base, when);

	auto const it = std::upper_bound(
			buf.begin(), buf.end(), position,
			[] (uint32_t a, uint32_t b) { return a < (b & floppy_image::TIME_MASK); });

	int index;
	if(buf.begin() == it) {
		base -= m_rev_time;
		index = buf.size() - 1;
	} else {
		index = int(it - buf.begin()) - 1;
	}

	for(;;) {
		cache_fill_index(buf, index, base);
		if(m_cache_end_time > when) {
			cache_weakness_setup();
			break;
		}
	}
}

void floppy_image_device::cache_weakness_setup()
{
	u32 type = m_cache_entry & floppy_image::MG_MASK;
	if(type == floppy_image::MG_N || type == floppy_image::MG_D) {
		m_cache_weak = true;
		m_cache_weak_start = m_cache_start_time;
		return;
	}

	m_cache_weak = m_cache_end_time.is_never() || (m_cache_end_time - m_cache_start_time >= m_amplifier_freakout_time);
	if(!m_cache_weak) {
		m_cache_weak_start = attotime::never;
		return;
	}
	m_cache_weak_start = m_cache_start_time + attotime::from_usec(16);
}

attotime floppy_image_device::get_next_transition(const attotime &from_when)
{
	if(!m_image || m_mon)
		return attotime::never;

	if(from_when < m_cache_start_time || m_cache_start_time.is_zero() || (!m_cache_end_time.is_never() && from_when >= m_cache_end_time))
		cache_fill(from_when);

	if(!m_cache_weak)
		return m_cache_end_time;

	// Put a flux transition in the middle of a 4us interval with a 50% probability
	uint64_t interval_index = (from_when < m_cache_weak_start) ? 0 : (from_when - m_cache_weak_start).as_ticks(250000);
	attotime weak_time = m_cache_weak_start + attotime::from_ticks(interval_index*2+1, 500000);
	for(;;) {
		if(weak_time >= m_cache_end_time)
			return m_cache_end_time;
		if(weak_time > from_when) {
			u32 test = hash32(hash32(hash32(hash32(m_revolution_count) ^ 0x4242) + m_cache_index) + interval_index);
			if(test & 1)
				return weak_time;
		}
		weak_time += attotime::from_usec(4);
		interval_index ++;
	}
}

bool floppy_image_device::writing_disabled() const
{
	// Disable writing when write protect is on or when, in the diskii
	// case, phase 1 is 1
	return m_wpt || (m_phases & 2);
}

void floppy_image_device::write_flux(const attotime &start, const attotime &end, int transition_count, const attotime *transitions)
{
	if(!m_image || m_mon)
		return;

	if(writing_disabled())
		return;

	m_image_dirty = true;
	m_track_dirty = true;
	cache_clear();

	std::vector<wspan> wspans(1);

	attotime base;
	wspans[0].start = find_position(base, start);
	wspans[0].end   = find_position(base, end);

	for(int i=0; i != transition_count; i++)
		wspans[0].flux_change_positions.push_back(find_position(base, transitions[i]));

	wspan_split_on_wrap(wspans);

	std::vector<uint32_t> &buf = m_image->get_buffer(m_cyl, m_ss, m_subcyl);

	if(buf.empty()) {
		buf.push_back(floppy_image::MG_N);
		buf.push_back(floppy_image::MG_E | 199999999);
	}

	wspan_remove_damaged(wspans, buf);
	wspan_write(wspans, buf);

	cache_clear();
}

void floppy_image_device::wspan_split_on_wrap(std::vector<wspan> &wspans)
{
	int ne = wspans.size();
	for(int i=0; i != ne; i++)
		if(wspans[i].end < wspans[i].start) {
			wspans.resize(wspans.size()+1);
			auto &ws = wspans[i];
			auto &we = wspans.back();
			we.start = 0;
			we.end = ws.end;
			ws.end = 200000000;
			int start = ws.start;
			int split_index;
			for(split_index = 0; split_index != ws.flux_change_positions.size(); split_index++)
				if(ws.flux_change_positions[split_index] < start)
					break;
			if(split_index == 0)
				std::swap(ws.flux_change_positions, we.flux_change_positions);

			else {
				we.flux_change_positions.resize(ws.flux_change_positions.size() - split_index);
				std::copy(ws.flux_change_positions.begin() + split_index, ws.flux_change_positions.end(), we.flux_change_positions.begin());
				ws.flux_change_positions.erase(ws.flux_change_positions.begin() + split_index, ws.flux_change_positions.end());
			}
		}
}

void floppy_image_device::wspan_remove_damaged(std::vector<wspan> &wspans, const std::vector<uint32_t> &track)
{
	for(size_t pos = 0; pos != track.size(); pos++)
		if((track[pos] & floppy_image::MG_MASK) == floppy_image::MG_D) {
			int start = track[pos] & floppy_image::TIME_MASK;
			int end = track[pos+1] & floppy_image::TIME_MASK;
			int ne = wspans.size();
			for(int i=0; i != ne; i++) {
				// D range outside of span range
				if(wspans[i].start > end || wspans[i].end <= start)
					continue;

				// D range covers span range
				if(wspans[i].start >= start && wspans[i].end-1 <= end) {
					wspans.erase(wspans.begin() + i);
					i --;
					ne --;
					continue;
				}

				// D range covers the start of the span range
				if(wspans[i].start >= start && wspans[i].end-1 > end) {
					wspans[i].start = end+1;
					while(!wspans[i].flux_change_positions.empty() && wspans[i].flux_change_positions[0] <= end)
						wspans[i].flux_change_positions.erase(wspans[i].flux_change_positions.begin());
					continue;
				}

				// D range covers the end of the span range
				if(wspans[i].start < start && wspans[i].end-1 <= end) {
					wspans[i].end = start;
					while(!wspans[i].flux_change_positions.empty() && wspans[i].flux_change_positions[wspans[i].flux_change_positions.size()-1] >= start)
						wspans[i].flux_change_positions.erase(wspans[i].flux_change_positions.end()-1);
					continue;
				}

				// D range is inside the span range, need to split
				int id = wspans.size();
				wspans.resize(id+1);
				wspans[id].start = end+1;
				wspans[id].end = wspans[i].end;
				wspans[id].flux_change_positions = wspans[i].flux_change_positions;
				wspans[i].end = start;
				while(!wspans[i].flux_change_positions.empty() && wspans[i].flux_change_positions[wspans[i].flux_change_positions.size()-1] >= start)
					wspans[i].flux_change_positions.erase(wspans[i].flux_change_positions.end()-1);
				while(!wspans[id].flux_change_positions.empty() && wspans[id].flux_change_positions[0] <= end)
					wspans[id].flux_change_positions.erase(wspans[id].flux_change_positions.begin());
			}
		}
}

void floppy_image_device::wspan_write(const std::vector<wspan> &wspans, std::vector<uint32_t> &track)
{
	for(const auto &ws : wspans) {
		unsigned si, ei;
		for(si = 0; si != track.size(); si++)
			if((track[si] & floppy_image::TIME_MASK) >= ws.start)
				break;
		for(ei = si; ei != track.size(); ei++)
			if((track[ei] & floppy_image::TIME_MASK) >= ws.end)
				break;

		// Reduce neutral zone at the start, if there's one
		if(si != track.size() && (track[si] & floppy_image::MG_MASK) == floppy_image::MG_E) {
			// Neutral zone is over the whole range, split it and adapt si/ei
			if(si == ei) {
				track.insert(track.begin() + si, floppy_image::MG_E | (ws.start-1));
				track.insert(track.begin() + si + 1, (track[si-1] & floppy_image::MG_MASK) | ws.end);
				si = ei = si+1;
			} else {
				// Reduce the zone size
				track[si] = floppy_image::MG_E | (ws.start-1);
				si ++;
			}
		}

		// Check for a neutral zone at the end and reduce it if needed
		if(ei != track.size() && (track[ei] & floppy_image::MG_MASK) == floppy_image::MG_E) {
			track[ei-1] = floppy_image::MG_N | ws.end;
			ei --;
		}

		// Clear the covered zone
		track.erase(track.begin() + si, track.begin() + ei);

		// Insert the flux changes
		for(auto f : ws.flux_change_positions) {
			track.insert(track.begin() + si, floppy_image::MG_F | f);
			si ++;
		}
	}
}

void floppy_image_device::set_write_splice(const attotime &when)
{
	if(m_image && !m_mon) {
		m_image_dirty = true;
		attotime base;
		int splice_pos = find_position(base, when);
		m_image->set_write_splice_position(m_cyl, m_ss, splice_pos, m_subcyl);
	}
}

uint32_t floppy_image_device::get_form_factor() const
{
	return m_form_factor;
}

uint32_t floppy_image_device::get_variant() const
{
	return m_image ? m_image->get_variant() : 0;
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
	set_samples_names(is525 ? floppy525_sample_names : floppy35_sample_names);

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
		m_sound = stream_alloc(0, 1, clock()); // per-floppy stream
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
			m_spin_playback_sample = withdisk ? SPIN_START_LOADED : SPIN_START_EMPTY; // (re)start the motor sound
		}
		else
		{
			// Motor has been running and is turned off now
			if ((m_spin_playback_sample == SPIN_EMPTY || m_spin_playback_sample == SPIN_LOADED) && !running)
			{
				m_spin_samplepos = 0;
				m_spin_playback_sample = SPIN_END; // go to spin down sound when loop is finished
			}
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

				// Start the new seek sound from the beginning.
				m_seek_samplepos = 0;
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

void floppy_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// We are using only one stream, unlike the parent class
	// Also, there is no need for interpolation, as we only expect
	// one sample rate of 44100 for all samples

	int16_t out;
	auto &samplebuffer = outputs[0];
	int m_idx = 0;
	int sampleend = 0;

	for (int sampindex = 0; sampindex < samplebuffer.samples(); sampindex++)
	{
		out = 0;

		// Motor sound
		if (m_spin_playback_sample != QUIET)
		{
			m_idx = m_spin_playback_sample;
			sampleend = m_sample[m_idx].data.size();
			out = m_sample[m_idx].data[m_spin_samplepos++];

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
						m_spin_playback_sample = m_with_disk ? SPIN_START_LOADED : SPIN_START_EMPTY;
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

			m_idx = m_step_base + m_seek_playback_sample;
			sampleend = m_sample[m_idx].data.size();
			// Mix it into the stream value
			out += m_sample[m_idx].data[(int)m_seek_samplepos];
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
				m_idx = m_step_base + m_step_playback_sample;
				sampleend = m_sample[m_idx].data.size();

				// Mix it into the stream value
				out += m_sample[m_idx].data[m_step_samplepos++];
				if (m_step_samplepos >= sampleend)
				{
					// Step sample done
					m_step_samplepos = 0;
					m_step_playback_sample = QUIET;
				}
			}
		}

		// Write to the stream buffer
		samplebuffer.put_int(sampindex, out, 32768);
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
//  3" single-sided single density
//-------------------------------------------------

floppy_3_sssd::floppy_3_sssd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_3_SSSD, tag, owner, clock)
{
}

floppy_3_sssd::~floppy_3_sssd()
{
}

void floppy_3_sssd::setup_characteristics()
{
	m_form_factor = floppy_image::FF_3;
	m_tracks = 42;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
}

//-------------------------------------------------
//  3" double-sided single density
//-------------------------------------------------

floppy_3_dssd::floppy_3_dssd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_3_DSSD, tag, owner, clock)
{
}

floppy_3_dssd::~floppy_3_dssd()
{
}

void floppy_3_dssd::setup_characteristics()
{
	m_form_factor = floppy_image::FF_3;
	m_tracks = 42;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::DSSD);
}

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
	m_form_factor = floppy_image::FF_3;
	m_tracks = 42;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
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
	m_form_factor = floppy_image::FF_3;
	m_tracks = 42;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::DSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
}

//-------------------------------------------------
//  3" double-sided quad density
//-------------------------------------------------

floppy_3_dsqd::floppy_3_dsqd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_3_DSQD, tag, owner, clock)
{
}

floppy_3_dsqd::~floppy_3_dsqd()
{
}

void floppy_3_dsqd::setup_characteristics()
{
	m_form_factor = floppy_image::FF_3;
	m_tracks = 84;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::DSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSQD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 84;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 84;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 84;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSHD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 84;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSHD);
	add_variant(floppy_image::DSED);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 35;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 35;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::DSSD);
}

//-------------------------------------------------
//  5.25" single-sided single density, VTECH edition
//-------------------------------------------------

floppy_525_vtech::floppy_525_vtech(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, FLOPPY_525_VTECH, tag, owner, clock)
{
	m_amplifier_freakout_time = attotime::from_usec(64);
}

floppy_525_vtech::~floppy_525_vtech()
{
}

void floppy_525_vtech::setup_characteristics()
{
	m_form_factor = floppy_image::FF_525;
	m_tracks = 40;
	m_sides = 1;
	set_rpm(85);

	add_variant(floppy_image::SSSD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 42;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 42;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 42;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 42;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 84;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::SSQD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 84;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::SSQD);
	add_variant(floppy_image::DSSD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSQD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 84;
	m_sides = 2;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::SSQD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSQD);
	add_variant(floppy_image::DSHD);
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
	m_form_factor = floppy_image::FF_8;
	m_tracks = 77;
	m_sides = 1;
	m_motor_always_on = true;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
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
	m_form_factor = floppy_image::FF_8;
	m_tracks = 77;
	m_sides = 2;
	m_motor_always_on = true;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::DSSD);
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
	m_form_factor = floppy_image::FF_8;
	m_tracks = 77;
	m_sides = 1;
	m_motor_always_on = true;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
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
	m_form_factor = floppy_image::FF_8;
	m_tracks = 77;
	m_sides = 2;
	m_motor_always_on = true;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 40;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::DSSD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 40;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 40;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
}


//-------------------------------------------------
//  3.5" Panasonic Flexible Disk Drive JU-363
//
//  track to track: 3 ms
//  settling time: 15 ms
//  motor start time: 500 ms
//  transfer rate: 250 Kbits/s
//
//-------------------------------------------------

pana_ju_363::pana_ju_363(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, PANA_JU_363, tag, owner, clock)
{
}

pana_ju_363::~pana_ju_363()
{
}

void pana_ju_363::setup_characteristics()
{
	m_form_factor = floppy_image::FF_35;
	m_tracks = 84;
	m_sides = 2;
	m_dskchg_writable = true;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 70;
	m_sides = 1;
	m_dskchg_writable = true;
	set_rpm(600);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 80;
	m_sides = 2;
	m_dskchg_writable = true;
	set_rpm(600);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_35;
	m_tracks = 80;
	m_sides = 1;
	m_dskchg_writable = true;
	set_rpm(600);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
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
	m_form_factor = floppy_image::FF_3;
	m_tracks = 40;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSDD);
}

//-------------------------------------------------
//  TEAC FD-55A
//
//  track to track: 6 ms
//  average: 93 ms
//  setting time: 15 ms
//  motor start time: 400 ms
//
//-------------------------------------------------

teac_fd_55a::teac_fd_55a(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: floppy_image_device(mconfig, TEAC_FD_55A, tag, owner, clock)
{
}

teac_fd_55a::~teac_fd_55a()
{
}

void teac_fd_55a::setup_characteristics()
{
	m_form_factor = floppy_image::FF_525;
	m_tracks = 40;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
}

//-------------------------------------------------
//  TEAC FD-55B
//
//  track to track: 6 ms
//  average: 93 ms
//  setting time: 15 ms
//  motor start time: 400 ms
//
//-------------------------------------------------

teac_fd_55b::teac_fd_55b(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: floppy_image_device(mconfig, TEAC_FD_55B, tag, owner, clock)
{
}

teac_fd_55b::~teac_fd_55b()
{
}

void teac_fd_55b::setup_characteristics()
{
	m_form_factor = floppy_image::FF_525;
	m_tracks = 40;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSSD);
	add_variant(floppy_image::DSDD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 80;
	m_sides = 1;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::SSQD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 80;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::SSQD);
	add_variant(floppy_image::DSSD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSQD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 77;
	m_sides = 2;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::SSQD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSQD);
	add_variant(floppy_image::DSHD);
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
	m_form_factor = floppy_image::FF_525;
	m_tracks = 84;
	m_sides = 1;
	set_rpm(300);
	m_cyl = 34;

	add_variant(floppy_image::SSSD);
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
	m_form_factor = floppy_image::FF_8;
	m_tracks = 77;
	m_sides = 1;
	m_motor_always_on = true;
	m_has_trk00_sensor = false;
	set_rpm(360);

	add_variant(floppy_image::SSSD);
}


//-------------------------------------------------
//  Variable-speed Macintosh drives
//-------------------------------------------------

mac_floppy_device::mac_floppy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) : floppy_image_device(mconfig, type, tag, owner, clock)
{
	m_has_mfm = false;
	m_dskchg_writable = true;
}

void mac_floppy_device::device_start()
{
	floppy_image_device::device_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_strb));
}

void mac_floppy_device::device_reset()
{
	floppy_image_device::device_reset();
	m_reg = 0;
	m_strb = 0;
	m_mfm = m_has_mfm;
}

// Initial state of bits f-c (2M, ready, MFM, rd1):
//    0000 - 400K GCR drive
//    0001 - 4MB Typhoon drive
//    x011 - Superdrive (x depends on the HD hole of the inserted disk, if any)
//    1010 - 800K GCR drive
//    1110 - HD-20 drive
//    1111 - No drive (pull-up on the sense line)

bool mac_floppy_device::writing_disabled() const
{
	return m_wpt;
}

bool mac_floppy_device::wpt_r()
{
	static const char *const regnames[16] = {
		"Dir", "Step", "Motor", "Eject",
		"RdData0", "Superdrive", "DoubleSide", "NoDrive",
		"NoDiskInPl", "NoWrProtect", "NotTrack0", "NoTachPulse",
		"RdData1", "MFMModeOn", "NoReady", "HD"
	};

	// m_actual_ss may have changed after the m_phases were set
	m_reg = (m_reg & 7) | (m_actual_ss ? 8 : 0);

	if(0 && (m_reg != 4 && m_reg != 12 && m_reg != 5 && m_reg != 13))
		logerror("fdc disk sense reg %x %s %p\n", m_reg, regnames[m_reg], m_image.get());

	switch(m_reg) {
	case 0x0: // Step direction
		return m_dir;

	case 0x1: // Step signal
		// We don't do the delay
		return true;

	case 0x2: // Is the motor on?
		return m_mon;

	case 0x3: // Disk change signal
		return !m_dskchg;

	case 0x4:
	case 0xc: // Index pulse, probably only in mfm mode and while writing though
		return !m_has_mfm ? false : !m_image || m_mon ? true : !m_idx;

	case 0x5: // Is it a superdrive (supports 1.4M MFM) ?
		return m_has_mfm;

	case 0x6: // Is the drive double-sided?
		return m_sides == 2;

	case 0x7: // Does the drive exist?
		return false;

	case 0x8: // Is there a disk in the drive?
		return m_image.get() == nullptr;

	case 0x9: // Is the disk write-protected?
		return !m_wpt;

	case 0xa: // Not on track 0?
		return m_cyl != 0;

	case 0xb:{// Tachometer, 60 pulses/rotation
		if(m_image.get() != nullptr && !m_mon) {
			attotime base;
			uint32_t pos = find_position(base, machine().time());
			uint32_t subpos = pos % 3333334;
			return subpos < 20000;
		} else
			return false;
	}

	case 0xd: // Is the current mode GCR or MFM?
		return m_mfm;

	case 0xe: // Is the floppy ready?
		return m_ready;

	case 0xf: // Does it implement the new interface *or* is the current disk is 1.4M MFM (superdrive only)
		return is_2m();

	default:
		return false;
	}
}

void mac_floppy_device::seek_phase_w(int phases)
{
	static const char *const regnames[16] = {
		"DirNext", "StepOn", "MotorOn", "EjectOff",
		"DirPrev", "StepOff", "MotorOff", "EjectOn",
		"-", "MFMModeOn", "-", "-",
		"DskchgClear", "GCRModeOn", "-", "-"
	};

	bool prev_strb = m_strb;

	m_reg = (phases & 7) | (m_actual_ss ? 8 : 0);
	m_strb = (phases >> 3) & 1;

	if(m_strb && !prev_strb) {
		switch(m_reg) {
		case 0x0: // Step to cylinder + 1
			logerror("cmd step dir +1\n");
			dir_w(0);
			break;

		case 0x1: // Step on
			logerror("cmd step on\n");
			stp_w(0);
			// There should be a delay, but it's not necessary
			stp_w(1);
			break;

		case 0x2: // Motor on
			logerror("cmd motor on\n");
			floppy_image_device::mon_w(0);
			break;

		case 0x3: // End eject
			logerror("cmd end eject\n");
			break;

		case 0x4: // Step to cylinder - 1
			logerror("cmd step dir -1\n");
			dir_w(1);
			break;

		case 0x6: // Motor off
			logerror("cmd motor off\n");
			floppy_image_device::mon_w(1);
			break;

		case 0x7: // Start eject
			logerror("cmd start eject\n");
			unload();
			break;

		case 0x9: // MFM mode on
			logerror("cmd mfm on\n");
			if(m_has_mfm) {
				m_mfm = true;
				track_changed();
			}
			break;

		case 0xc: // Clear m_dskchg
			logerror("cmd clear m_dskchg\n");
			m_dskchg = 1;
			break;

		case 0xd: // GCR mode on
			logerror("cmd gcr on\n");
			m_mfm = false;
			track_changed();
			break;

		default:
			logerror("cmd reg %x %s\n", m_reg, regnames[m_reg]);
			break;
		}
	}
}

void mac_floppy_device::track_changed()
{
	floppy_image_device::track_changed();

	float new_rpm;
	if(m_mfm)
		new_rpm = is_2m() ? 600 : 300;
	else if(m_cyl <= 15)
		new_rpm = 394;
	else if(m_cyl <= 31)
		new_rpm = 429;
	else if(m_cyl <= 47)
		new_rpm = 472;
	else if(m_cyl <= 63)
		new_rpm = 525;
	else
		new_rpm = 590;

	if(m_rpm != new_rpm)
		set_rpm(new_rpm);
}

void mac_floppy_device::mon_w(int)
{
	// Motor control is through commands
}

void mac_floppy_device::tfsel_w(int state)
{
	// if 35SEL line is clear and the motor is on, turn off the motor
	if ((state == CLEAR_LINE) && (!floppy_image_device::mon_r()))
	{
		floppy_image_device::mon_w(1);
	}
}

oa_d34v_device::oa_d34v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : mac_floppy_device(mconfig, OAD34V, tag, owner, clock)
{
}

void oa_d34v_device::setup_characteristics()
{
	m_form_factor = floppy_image::FF_35;
	m_tracks = 80;
	m_sides = 1;
	set_rpm(394);

	add_variant(floppy_image::SSDD);
}

bool oa_d34v_device::is_2m() const
{
	return false;
}

void oa_d34v_device::track_changed()
{
	// Skip the m_rpm-setting mac generic version, the single-sided
	// drive's m_rpm is externally controlled through a PWM signal.

	floppy_image_device::track_changed();
}

mfd51w_device::mfd51w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : mac_floppy_device(mconfig, MFD51W, tag, owner, clock)
{
}
void mfd51w_device::setup_characteristics()
{
	m_form_factor = floppy_image::FF_35;
	m_tracks = 80;
	m_sides = 2;
	set_rpm(394);

	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
}

bool mfd51w_device::is_2m() const
{
	return true;
}

mfd75w_device::mfd75w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : mac_floppy_device(mconfig, MFD75W, tag, owner, clock)
{
	m_has_mfm = true;
}

void mfd75w_device::setup_characteristics()
{
	m_form_factor = floppy_image::FF_35;
	m_tracks = 80;
	m_sides = 2;
	set_rpm(300);

	add_variant(floppy_image::SSDD);
	add_variant(floppy_image::DSDD);
	add_variant(floppy_image::DSHD);
}

bool mfd75w_device::is_2m() const
{
	if(!m_image)
		return false;

	if(m_image->get_variant() == floppy_image::SSDD || m_image->get_variant() == floppy_image::DSDD)
		return true;

	return false;
}
