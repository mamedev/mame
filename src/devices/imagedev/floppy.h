// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Olivier Galibert, Miodrag Milanovic
/*********************************************************************

    floppy.h

*********************************************************************/

#ifndef MAME_IMAGEDEV_FLOPPY_H
#define MAME_IMAGEDEV_FLOPPY_H

#pragma once

#include "sound/samples.h"

// forward declarations
class floppy_image;
class floppy_image_format_t;

namespace fs {
	class manager_t;
	class meta_data;
};

class floppy_sound_device;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class format_registration {
public:
	format_registration();

	void add(const floppy_image_format_t &format);
	void add(const fs::manager_t &fs);

	void add_fm_containers();
	void add_mfm_containers();
	void add_pc_formats();

	std::vector<const floppy_image_format_t *> m_formats;
	std::vector<const fs::manager_t *> m_fs;
};

class floppy_image_device : public device_t,
							public device_image_interface
{
public:
	typedef delegate<void (floppy_image_device *)> load_cb;
	typedef delegate<void (floppy_image_device *)> unload_cb;
	typedef delegate<void (floppy_image_device *, int)> index_pulse_cb;
	typedef delegate<void (floppy_image_device *, int)> ready_cb;
	typedef delegate<void (floppy_image_device *, int)> wpt_cb;
	typedef delegate<void (floppy_image_device *, int)> led_cb;

	struct fs_info {
		const fs::manager_t *m_manager;
		const floppy_image_format_t *m_type;
		u32 m_image_size;
		const char *m_name;
		u32 m_key;
		const char *m_description;

		fs_info(const fs::manager_t *manager, const floppy_image_format_t *type, u32 image_size, const char *name, const char *description) :
			m_manager(manager),
			m_type(type),
			m_image_size(image_size),
			m_name(name),
			m_key(0),
			m_description(description)
		{}

		fs_info(const char *name, u32 key, const char *description) :
			m_manager(nullptr),
			m_type(nullptr),
			m_image_size(0),
			m_name(name),
			m_key(key),
			m_description(description)
		{}
	};

	// construction/destruction
	virtual ~floppy_image_device();

	void set_formats(std::function<void (format_registration &fr)> formats);
	const std::vector<const floppy_image_format_t *> &get_formats() const;
	const std::vector<fs_info> &get_fs() const { return m_fs; }
	const floppy_image_format_t *get_load_format() const;
	std::pair<std::error_condition, const floppy_image_format_t *> identify(std::string_view filename);
	void set_rpm(float rpm);
	void set_sectoring_type(uint32_t sectoring_type);
	uint32_t get_sectoring_type();

	void init_fs(const fs_info *fs, const fs::meta_data &meta);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual const char *image_interface() const noexcept override = 0;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return m_extension_list; }
	virtual const char *image_type_name() const noexcept override { return "floppydisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "flop"; }
	void setup_write(const floppy_image_format_t *output_format);

	void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);
	void setup_index_pulse_cb(index_pulse_cb cb);
	void setup_ready_cb(ready_cb cb);
	void setup_wpt_cb(wpt_cb cb);
	void setup_led_cb(led_cb cb);

	int get_cyl() const { return m_cyl; }
	bool on_track() const { return !m_subcyl; }

	virtual void mon_w(int state);
	bool ready_r();
	void set_ready(bool state);
	virtual void tfsel_w(int state) { }    // 35SEL line for Apple Sony drives

	virtual bool wpt_r(); // Mac sony drives using this for various reporting
	int dskchg_r() { return m_dskchg; }
	bool trk00_r() { return (m_has_trk00_sensor ? (m_cyl != 0) : 1); }
	int idx_r() { return m_idx; }
	int mon_r() { return m_mon; }
	bool ss_r() { return m_ss; }
	bool twosid_r();
	bool floppy_is_hd();
	bool floppy_is_ed();

	virtual bool writing_disabled() const;

	virtual void seek_phase_w(int phases);
	void stp_w(int state);
	void dir_w(int state) { m_dir = state; }
	void ss_w(int state) { m_actual_ss = state; if (m_sides > 1) m_ss = state; }
	void inuse_w(int state) { }
	void dskchg_w(int state) { if (m_dskchg_writable) m_dskchg = state; }
	void ds_w(int state) { m_ds = state; check_led(); }

	attotime time_next_index();
	attotime get_next_transition(const attotime &from_when);
	void write_flux(const attotime &start, const attotime &end, int transition_count, const attotime *transitions);
	void set_write_splice(const attotime &when);
	int get_sides() { return m_sides; }
	uint32_t get_form_factor() const;
	uint32_t get_variant() const;

	static void default_fm_floppy_formats(format_registration &fr);
	static void default_mfm_floppy_formats(format_registration &fr);
	static void default_pc_floppy_formats(format_registration &fr);

	// Enable sound
	void    enable_sound(bool doit) { m_make_sound = doit; }

protected:
	struct fs_enum;

	floppy_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

	TIMER_CALLBACK_MEMBER(index_resync);

	virtual void track_changed();
	virtual void setup_characteristics() = 0;

	void init_floppy_load(bool write_supported);

	std::function<void (format_registration &fr)> m_format_registration_cb;
	const floppy_image_format_t *m_input_format;
	const floppy_image_format_t *m_output_format;
	std::vector<uint32_t> m_variants;
	std::unique_ptr<floppy_image> m_image;
	char                  m_extension_list[256];
	std::vector<const floppy_image_format_t *> m_fif_list;
	std::vector<fs_info>  m_fs;
	std::vector<const fs::manager_t *> m_fs_managers;
	emu_timer             *m_index_timer;

	/* Physical characteristics, filled by setup_characteristics */
	int m_tracks; /* addressable tracks */
	int m_sides;  /* number of heads */
	uint32_t m_form_factor; /* 3"5, 5"25, etc */
	uint32_t m_sectoring_type; /* SOFT, Hard 10/16/32 */
	bool m_motor_always_on;
	bool m_dskchg_writable;
	bool m_has_trk00_sensor;

	int m_drive_index;

	/* state of input lines */
	int m_dir;  /* direction */
	int m_stp;  /* step */
	int m_wtg;  /* write gate */
	int m_mon;  /* motor on */
	int m_ss, m_actual_ss; /* side select (forced to 0 if single-sided drive / actual value) */
	int m_ds; /* drive select */

	int m_phases; /* phases lines, when they exist */

	/* state of output lines */
	int m_idx;  /* index pulse */
	int m_wpt;  /* write protect */
	int m_rdy;  /* ready */
	int m_dskchg;     /* disk changed */
	bool m_ready;

	/* rotation per minute => gives index pulse frequency */
	float m_rpm;
	/* angular speed, where a full circle is 2e8 */
	double m_angular_speed;

	attotime m_revolution_start_time, m_rev_time;
	uint32_t m_revolution_count;
	int m_cyl, m_subcyl;
	/* Current floppy zone cache */
	attotime m_cache_start_time, m_cache_end_time, m_cache_weak_start;
	attotime m_amplifier_freakout_time;
	int m_cache_index;
	u32 m_cache_entry;
	bool m_cache_weak;

	bool m_image_dirty, m_track_dirty;
	int m_ready_counter;

	load_cb m_cur_load_cb;
	unload_cb m_cur_unload_cb;
	index_pulse_cb m_cur_index_pulse_cb;
	ready_cb m_cur_ready_cb;
	wpt_cb m_cur_wpt_cb;
	led_cb m_cur_led_cb;


	// Temporary structure storing a write span
	struct wspan {
		int start, end;
		std::vector<int> flux_change_positions;
	};

	static void wspan_split_on_wrap(std::vector<wspan> &wspans);
	static void wspan_remove_damaged(std::vector<wspan> &wspans, const std::vector<uint32_t> &track);
	static void wspan_write(const std::vector<wspan> &wspans, std::vector<uint32_t> &track);

	void register_formats();

	void add_variant(uint32_t variant);

	void check_led();
	uint32_t find_position(attotime &base, const attotime &when);
	attotime position_to_time(const attotime &base, int position) const;

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
		virtual const char *image_interface() const noexcept override { return Interface; } \
	protected: \
		virtual void setup_characteristics() override; \
	}; \
	DECLARE_DEVICE_TYPE(Type, Name)

DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_SSSD,       floppy_3_sssd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_DSSD,       floppy_3_dssd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_SSDD,       floppy_3_ssdd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_DSDD,       floppy_3_dsdd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_3_DSQD,       floppy_3_dsqd,       "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_SSDD,      floppy_35_ssdd,      "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_DD,        floppy_35_dd,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_HD,        floppy_35_hd,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_35_ED,        floppy_35_ed,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SSSD_35T, floppy_525_sssd_35t, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_SD_35T,   floppy_525_sd_35t,   "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(FLOPPY_525_VTECH,    floppy_525_vtech,    "floppy_5_25")
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
DECLARE_FLOPPY_IMAGE_DEVICE(PANA_JU_363,         pana_ju_363,         "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(SONY_OA_D31V,        sony_oa_d31v,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(SONY_OA_D32W,        sony_oa_d32w,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(SONY_OA_D32V,        sony_oa_d32v,        "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_30A,         teac_fd_30a,         "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55A,         teac_fd_55a,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55B,         teac_fd_55b,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55E,         teac_fd_55e,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55F,         teac_fd_55f,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(TEAC_FD_55G,         teac_fd_55g,         "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(ALPS_3255190X,       alps_3255190x,       "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(IBM_6360,            ibm_6360,            "floppy_8")

DECLARE_DEVICE_TYPE(FLOPPYSOUND, floppy_sound_device)

class mac_floppy_device : public floppy_image_device {
public:
	virtual ~mac_floppy_device() = default;

	virtual bool wpt_r() override;
	virtual void mon_w(int) override;
	virtual void tfsel_w(int state) override;
	virtual void seek_phase_w(int phases) override;
	virtual const char *image_interface() const noexcept override { return "floppy_3_5"; }
	virtual bool writing_disabled() const override;

protected:
	u8 m_reg;
	bool m_strb;
	bool m_mfm, m_has_mfm;

	mac_floppy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void track_changed() override;

	virtual bool is_2m() const = 0;
};

// 400K GCR
class oa_d34v_device : public mac_floppy_device {
public:
	oa_d34v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~oa_d34v_device() = default;
protected:
	virtual void setup_characteristics() override;
	virtual void track_changed() override;

	virtual bool is_2m() const override;
};

// 400/800K GCR (e.g. dual-sided)
class mfd51w_device : public mac_floppy_device {
public:
	mfd51w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~mfd51w_device() = default;
protected:
	virtual void setup_characteristics() override;

	virtual bool is_2m() const override;
};

// 400/800K GCR + 1.44 MFM (Superdrive)
class mfd75w_device : public mac_floppy_device {
public:
	mfd75w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~mfd75w_device() = default;

protected:
	virtual void setup_characteristics() override;

	virtual bool is_2m() const override;
};

DECLARE_DEVICE_TYPE(OAD34V, oa_d34v_device)
DECLARE_DEVICE_TYPE(MFD51W, mfd51w_device)
DECLARE_DEVICE_TYPE(MFD75W, mfd75w_device)


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
	void device_start() override ATTR_COLD;

private:
	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
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

	template <typename T, typename U>
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, U &&formats, bool fixed = false)
		: floppy_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		set_formats(std::forward<U>(formats));
	}

	template <typename T>
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, const char *option, device_type drivetype, bool is_default, T &&formats)
		: floppy_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		option_add(option, drivetype);
		if(is_default)
			set_default_option(option);
		set_fixed(false);
		set_formats(std::forward<T>(formats));
	}

	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~floppy_connector();

	template <typename T> void set_formats(T &&_formats) { formats = std::forward<T>(_formats); }
	void enable_sound(bool doit) { m_enable_sound = doit; }
	void set_sectoring_type(uint32_t sectoring_type) { m_sectoring_type = sectoring_type; }

	floppy_image_device *get_device();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	std::function<void (format_registration &fr)> formats;
	bool m_enable_sound;
	uint32_t m_sectoring_type;
};


// device type definition
DECLARE_DEVICE_TYPE(FLOPPY_CONNECTOR, floppy_connector)

extern template class device_finder<floppy_connector, false>;
extern template class device_finder<floppy_connector, true>;

#endif // MAME_IMAGEDEV_FLOPPY_H
