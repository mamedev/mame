// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Olivier Galibert, Miodrag Milanovic
/*********************************************************************

    floppy.h

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_FLOPPY_H
#define MAME_DEVICES_IMAGEDEV_FLOPPY_H

#pragma once

#include "formats/flopimg.h"
#include "formats/d88_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/ipf_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/cqm_dsk.h"
#include "formats/dsk_dsk.h"
#include "sound/samples.h"
#include "softlist_dev.h"

#define DECLARE_FLOPPY_FORMATS(_name) \
	static const floppy_format_type _name []

#define FLOPPY_FORMATS_MEMBER(_member) \
	const floppy_format_type _member [] = {
#define FLOPPY_FORMATS_END0 \
		, \
		nullptr };
#define FLOPPY_FORMATS_END \
		, \
		FLOPPY_D88_FORMAT, \
		FLOPPY_DFI_FORMAT, \
		FLOPPY_HFE_FORMAT, \
		FLOPPY_IMD_FORMAT, \
		FLOPPY_IPF_FORMAT, \
		FLOPPY_MFI_FORMAT, \
		FLOPPY_MFM_FORMAT, \
		FLOPPY_TD0_FORMAT, \
		FLOPPY_CQM_FORMAT, \
		FLOPPY_DSK_FORMAT \
	FLOPPY_FORMATS_END0

class floppy_sound_device;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class floppy_image_device : public device_t,
							public device_image_interface
{
public:
	typedef delegate<image_init_result (floppy_image_device *)> load_cb;
	typedef delegate<void (floppy_image_device *)> unload_cb;
	typedef delegate<void (floppy_image_device *, int)> index_pulse_cb;
	typedef delegate<void (floppy_image_device *, int)> ready_cb;
	typedef delegate<void (floppy_image_device *, int)> wpt_cb;
	typedef delegate<void (floppy_image_device *, int)> led_cb;

	// construction/destruction
	virtual ~floppy_image_device();

	virtual void handled_variants(uint32_t *variants, int &var_count) const = 0;

	void set_formats(const floppy_format_type *formats);
	floppy_image_format_t *get_formats() const;
	floppy_image_format_t *get_load_format() const;
	floppy_image_format_t *identify(std::string filename);
	void set_rpm(float rpm);

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual const char *image_interface() const noexcept override = 0;
	virtual iodevice_t image_type() const noexcept override { return IO_FLOPPY; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return extension_list; }
	void setup_write(floppy_image_format_t *output_format);

	void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);
	void setup_index_pulse_cb(index_pulse_cb cb);
	void setup_ready_cb(ready_cb cb);
	void setup_wpt_cb(wpt_cb cb);
	void setup_led_cb(led_cb cb);

	std::vector<uint32_t> &get_buffer() { return image->get_buffer(cyl, ss, subcyl); }
	int get_cyl() { return cyl; }

	void mon_w(int state);
	bool ready_r();
	void set_ready(bool state);
	double get_pos();

	bool wpt_r() { return wpt; }
	int dskchg_r() { return dskchg; }
	bool trk00_r() { return (has_trk00_sensor ? (cyl != 0) : 1); }
	int idx_r() { return idx; }
	int mon_r() { return mon; }
	bool ss_r() { return ss; }
	bool twosid_r();

	void seek_phase_w(int phases);
	void stp_w(int state);
	void dir_w(int state) { dir = state; }
	void ss_w(int state) { if (sides > 1) ss = state; }
	void inuse_w(int state) { }
	void dskchg_w(int state) { if (dskchg_writable) dskchg = state; }
	void ds_w(int state) { ds = state; check_led(); }

	void index_resync();
	attotime time_next_index();
	attotime get_next_transition(const attotime &from_when);
	void write_flux(const attotime &start, const attotime &end, int transition_count, const attotime *transitions);
	void set_write_splice(const attotime &when);
	int get_sides() { return sides; }
	uint32_t get_form_factor() const;
	uint32_t get_variant() const;

	static const floppy_format_type default_floppy_formats[];

	// Enable sound
	void    enable_sound(bool doit) { m_make_sound = doit; }

protected:
	floppy_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void device_add_mconfig(machine_config &config) override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return image_software_list_loader::instance(); }

	virtual void setup_characteristics() = 0;

	void init_floppy_load(bool write_supported);

	floppy_image_format_t *input_format;
	floppy_image_format_t *output_format;
	floppy_image          *image;
	char                  extension_list[256];
	floppy_image_format_t *fif_list;
	emu_timer             *index_timer;

	/* Physical characteristics, filled by setup_characteristics */
	int tracks; /* addressable tracks */
	int sides;  /* number of heads */
	uint32_t form_factor; /* 3"5, 5"25, etc */
	bool motor_always_on;
	bool dskchg_writable;
	bool has_trk00_sensor;

	int drive_index;

	/* state of input lines */
	int dir;  /* direction */
	int stp;  /* step */
	int wtg;  /* write gate */
	int mon;  /* motor on */
	int ss; /* side select */
	int ds; /* drive select */

	/* state of output lines */
	int idx;  /* index pulse */
	int wpt;  /* write protect */
	int rdy;  /* ready */
	int dskchg;     /* disk changed */
	bool ready;

	/* rotation per minute => gives index pulse frequency */
	float rpm;
	int floppy_ratio_1; // rpm/300*1000

	attotime revolution_start_time, rev_time;
	uint32_t revolution_count;
	int cyl, subcyl;

	/* Current floppy zone cache */
	attotime cache_start_time, cache_end_time, cache_weak_start;
	int cache_index;
	u32 cache_entry;
	bool cache_weak;

	bool image_dirty;
	int ready_counter;

	load_cb cur_load_cb;
	unload_cb cur_unload_cb;
	index_pulse_cb cur_index_pulse_cb;
	ready_cb cur_ready_cb;
	wpt_cb cur_wpt_cb;
	led_cb cur_led_cb;

	void check_led();
	uint32_t find_position(attotime &base, const attotime &when);
	int find_index(uint32_t position, const std::vector<uint32_t> &buf) const;
	bool test_track_last_entry_warps(const std::vector<uint32_t> &buf) const;
	attotime position_to_time(const attotime &base, int position) const;

	void write_zone(uint32_t *buf, int &cells, int &index, uint32_t spos, uint32_t epos, uint32_t mg);
	void commit_image();

	u32 hash32(u32 val) const;

	void cache_clear();
	void cache_fill_index(const std::vector<uint32_t> &buf, int &index, attotime &base);
	void cache_fill(const attotime &when);
	void cache_weakness_setup();

	// Sound
	bool    m_make_sound;
	floppy_sound_device* m_sound_out;
};

#define DECLARE_FLOPPY_IMAGE_DEVICE(Type, Name, Interface) \
	class Name : public floppy_image_device { \
	public: \
		Name(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); \
		virtual ~Name(); \
		virtual void handled_variants(uint32_t *variants, int &var_count) const override; \
		virtual const char *image_interface() const noexcept override { return Interface; } \
	protected: \
		virtual void setup_characteristics() override; \
	}; \
	DECLARE_DEVICE_TYPE(Type, Name)

DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_SSDD,       floppy_3_ssdd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_DSDD,       floppy_3_dsdd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_SSDD,      floppy_35_ssdd,      "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_DD,        floppy_35_dd,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_HD,        floppy_35_hd,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_ED,        floppy_35_ed,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SSSD_35T, floppy_525_sssd_35t, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SD_35T,   floppy_525_sd_35t,   "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SSSD,     floppy_525_sssd,     "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SD,       floppy_525_sd,       "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SSDD,     floppy_525_ssdd,     "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_DD,       floppy_525_dd,       "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SSQD,     floppy_525_ssqd,     "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_QD,       floppy_525_qd,       "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_HD,       floppy_525_hd,       "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_8_SSSD,       floppy_8_sssd,       "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_8_DSSD,       floppy_8_dssd,       "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_8_SSDD,       floppy_8_ssdd,       "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_8_DSDD,       floppy_8_dsdd,       "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(EPSON_SMD_165,       epson_smd_165,       "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(EPSON_SD_320,        epson_sd_320,        "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(EPSON_SD_321,        epson_sd_321,        "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(SONY_OA_D31V,        sony_oa_d31v,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(SONY_OA_D32W,        sony_oa_d32w,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(SONY_OA_D32V,        sony_oa_d32v,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_30A,         teac_fd_30a,         "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55E,         teac_fd_55e,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55F,         teac_fd_55f,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55G,         teac_fd_55g,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(ALPS_3255190X,       alps_3255190x,       "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(IBM_6360,            ibm_6360,            "floppy_8")

DECLARE_DEVICE_TYPE(FLOPPYSOUND, floppy_sound_device)


/*
    Floppy drive sound
*/

class floppy_sound_device : public samples_device
{
public:
	floppy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void motor(bool on, bool withdisk);
	void step(int track);
	bool samples_loaded() { return m_loaded; }
	void register_for_save_states();

protected:
	void device_start() override;

private:
	// device_sound_interface overrides
	void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	sound_stream*   m_sound;

	int         m_step_base;
	int         m_spin_samples;
	int         m_step_samples;
	int         m_spin_samplepos;
	int         m_step_samplepos;
	int         m_seek_sound_timeout;
	int         m_zones;
	int         m_spin_playback_sample;
	int         m_step_playback_sample;
	int         m_seek_playback_sample;
	bool        m_motor_on;
	bool        m_with_disk;
	bool        m_loaded;
	double      m_seek_pitch;
	double      m_seek_samplepos;
};


class floppy_connector: public device_t,
						public device_slot_interface
{
public:
	template <typename T>
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, const floppy_format_type formats[], bool fixed = false)
		: floppy_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		set_formats(formats);
	}
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, const char *option, const device_type &devtype, bool is_default, const floppy_format_type formats[])
		: floppy_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		option_add(option, devtype);
		if(is_default)
			set_default_option(option);
		set_fixed(false);
		set_formats(formats);
	}
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~floppy_connector();

	void set_formats(const floppy_format_type formats[]);
	floppy_image_device *get_device();
	void enable_sound(bool doit) { m_enable_sound = doit; }

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

private:
	const floppy_format_type *formats;
	bool m_enable_sound;
};


// device type definition
DECLARE_DEVICE_TYPE(FLOPPY_CONNECTOR, floppy_connector)

extern template class device_finder<floppy_connector, false>;
extern template class device_finder<floppy_connector, true>;

#endif // MAME_DEVICES_IMAGEDEV_FLOPPY_H
