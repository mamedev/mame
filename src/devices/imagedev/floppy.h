// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Olivier Galibert, Miodrag Milanovic
/*********************************************************************

    floppy.h

*********************************************************************/

#ifndef FLOPPY_H
#define FLOPPY_H

#include "formats/flopimg.h"
#include "formats/d88_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/ipf_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/cqm_dsk.h"
#include "formats/dsk_dsk.h"
#include "ui/imgcntrl.h"
#include "sound/samples.h"

#define MCFG_FLOPPY_DRIVE_ADD(_tag, _slot_intf, _def_slot, _formats)  \
	MCFG_DEVICE_ADD(_tag, FLOPPY_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	static_cast<floppy_connector *>(device)->set_formats(_formats);

#define MCFG_FLOPPY_DRIVE_SOUND(_doit) \
	static_cast<floppy_connector *>(device)->enable_sound(_doit);

#define DECLARE_FLOPPY_FORMATS(_name) \
	static const floppy_format_type _name []

#define FLOPPY_FORMATS_MEMBER(_member) \
	const floppy_format_type _member [] = {
#define FLOPPY_FORMATS_END \
		, \
		FLOPPY_D88_FORMAT, \
		FLOPPY_DFI_FORMAT, \
		FLOPPY_IMD_FORMAT, \
		FLOPPY_IPF_FORMAT, \
		FLOPPY_MFI_FORMAT, \
		FLOPPY_MFM_FORMAT, \
		FLOPPY_TD0_FORMAT, \
		FLOPPY_CQM_FORMAT, \
		FLOPPY_DSK_FORMAT, \
		NULL };

class floppy_sound_device;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class floppy_image_device : public device_t,
							public device_image_interface,
							public device_slot_card_interface
{
public:
	typedef delegate<int (floppy_image_device *)> load_cb;
	typedef delegate<void (floppy_image_device *)> unload_cb;
	typedef delegate<void (floppy_image_device *, int)> index_pulse_cb;
	typedef delegate<void (floppy_image_device *, int)> ready_cb;
	typedef delegate<void (floppy_image_device *, int)> wpt_cb;

	// construction/destruction
	floppy_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~floppy_image_device();

	virtual void handled_variants(UINT32 *variants, int &var_count) const = 0;

	void set_formats(const floppy_format_type *formats);
	floppy_image_format_t *get_formats() const;
	floppy_image_format_t *get_load_format() const;
	floppy_image_format_t *identify(std::string filename);
	void set_rpm(float rpm);

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }
	virtual const char *image_interface() const = 0;
	virtual iodevice_t image_type() const { return IO_FLOPPY; }

	virtual bool is_readable()  const { return true; }
	virtual bool is_writeable() const { return true; }
	virtual bool is_creatable() const { return true; }
	virtual bool must_be_loaded() const { return false; }
	virtual bool is_reset_on_load() const { return false; }
	virtual const char *file_extensions() const { return extension_list; }
	virtual const option_guide *create_option_guide() const { return NULL; }
	void setup_write(floppy_image_format_t *output_format);

	void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);
	void setup_index_pulse_cb(index_pulse_cb cb);
	void setup_ready_cb(ready_cb cb);
	void setup_wpt_cb(wpt_cb cb);

	std::vector<UINT32> &get_buffer() { return image->get_buffer(cyl, ss, subcyl); }
	int get_cyl() { return cyl; }

	void mon_w(int state);
	bool ready_r();
	double get_pos();

	bool wpt_r() { return wpt; }
	int dskchg_r() { return dskchg; }
	bool trk00_r() { return cyl != 0; }
	int idx_r() { return idx; }
	int mon_r() { return mon; }
	bool ss_r() { return ss; }
	bool twosid_r();

	void seek_phase_w(int phases);
	void stp_w(int state);
	void dir_w(int state) { dir = state; }
	void ss_w(int state) { ss = state; }
	void inuse_w(int state) { }

	void index_resync();
	attotime time_next_index();
	attotime get_next_transition(const attotime &from_when);
	void write_flux(const attotime &start, const attotime &end, int transition_count, const attotime *transitions);
	void set_write_splice(const attotime &when);
	int get_sides() { return sides; }
	UINT32 get_form_factor() const;
	UINT32 get_variant() const;

	virtual ui_menu *get_selection_menu(running_machine &machine, class render_container *container);

	static const floppy_format_type default_floppy_formats[];

	// Enable sound
	void    enable_sound(bool doit) { m_make_sound = doit; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void setup_characteristics() = 0;

	floppy_image_format_t *input_format;
	floppy_image_format_t *output_format;
	floppy_image          *image;
	char                  extension_list[256];
	floppy_image_format_t *fif_list;
	emu_timer             *index_timer;

	/* Physical characteristics, filled by setup_characteristics */
	int tracks; /* addressable tracks */
	int sides;  /* number of heads */
	UINT32 form_factor; /* 3"5, 5"25, etc */
	bool motor_always_on;

	/* state of input lines */
	int dir;  /* direction */
	int stp;  /* step */
	int wtg;  /* write gate */
	int mon;  /* motor on */
	int ss; /* side select */

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
	UINT32 revolution_count;
	int cyl, subcyl;

	bool image_dirty;
	int ready_counter;

	load_cb cur_load_cb;
	unload_cb cur_unload_cb;
	index_pulse_cb cur_index_pulse_cb;
	ready_cb cur_ready_cb;
	wpt_cb cur_wpt_cb;

	UINT32 find_position(attotime &base, const attotime &when);
	int find_index(UINT32 position, const std::vector<UINT32> &buf);
	void write_zone(UINT32 *buf, int &cells, int &index, UINT32 spos, UINT32 epos, UINT32 mg);
	void commit_image();
	attotime get_next_index_time(std::vector<UINT32> &buf, int index, int delta, attotime base);

	// Sound
	bool    m_make_sound;
	floppy_sound_device* m_sound_out;
};

class ui_menu_control_floppy_image : public ui_menu_control_device_image {
public:
	ui_menu_control_floppy_image(running_machine &machine, render_container *container, device_image_interface *image);
	virtual ~ui_menu_control_floppy_image();

	virtual void handle();

protected:
	enum { SELECT_FORMAT = LAST_ID, SELECT_MEDIA, SELECT_RW };

	floppy_image_format_t **format_array;
	floppy_image_format_t *input_format, *output_format;
	std::string input_filename, output_filename;

	void do_load_create();
	virtual void hook_load(std::string filename, bool softlist);
};


#define DECLARE_FLOPPY_IMAGE_DEVICE(_name, _interface) \
	class _name : public floppy_image_device { \
	public: \
		_name(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock); \
		virtual ~_name(); \
		virtual void handled_variants(UINT32 *variants, int &var_count) const; \
		virtual const char *image_interface() const { return _interface; } \
	protected: \
		virtual void setup_characteristics(); \
	};

DECLARE_FLOPPY_IMAGE_DEVICE(floppy_3_ssdd, "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_3_dsdd, "floppy_3")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_35_ssdd, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_35_dd, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_35_hd, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_35_ed, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_sssd_35t, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_sd_35t, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_sssd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_sd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_ssdd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_dd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_ssqd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_qd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_525_hd, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_8_sssd, "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_8_dssd, "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_8_ssdd, "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(floppy_8_dsdd, "floppy_8")
DECLARE_FLOPPY_IMAGE_DEVICE(epson_smd_165, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(epson_sd_320, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(epson_sd_321, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(sony_oa_d31v, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(sony_oa_d32w, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(sony_oa_d32v, "floppy_3_5")
DECLARE_FLOPPY_IMAGE_DEVICE(teac_fd_55e, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(teac_fd_55f, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(teac_fd_55g, "floppy_5_25")
DECLARE_FLOPPY_IMAGE_DEVICE(alps_3255190x, "floppy_5_25")

extern const device_type FLOPPYSOUND;


/*
    Floppy drive sound
*/

#define MAX_STEP_SAMPLES 5

class floppy_sound_device : public samples_device
{
public:
	floppy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void motor(bool on);
	void step();
	bool samples_loaded() { return m_loaded; }
	void register_for_save_states();

protected:
	void device_start();

private:
	// device_sound_interface overrides
	void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	sound_stream*   m_sound;
	bool            m_loaded;
	bool			m_is525; // true if this is a 5.25" floppy drive

	int				m_sampleindex_motor_start;
	int				m_sampleindex_motor_loop;
	int				m_sampleindex_motor_end;
	int             m_samplesize_motor_start;
	int             m_samplesize_motor_loop;
	int             m_samplesize_motor_end;
	int             m_samplepos_motor;
	int             m_motor_playback_state;
	bool            m_motor_on;

	int				m_step_samples;
	int				m_sampleindex_step1;
	int             m_samplesize_step[MAX_STEP_SAMPLES];
	int             m_samplepos_step;
	int             m_step_playback_state;
};


class floppy_connector: public device_t,
						public device_slot_interface
{
public:
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_connector();

	void set_formats(const floppy_format_type *formats);
	floppy_image_device *get_device();
	void enable_sound(bool doit) { m_enable_sound = doit; }

protected:
	virtual void device_start();
	virtual void device_config_complete();

private:
	const floppy_format_type *formats;
	bool m_enable_sound;
};


// device type definition
extern const device_type FLOPPY_CONNECTOR;
extern const device_type FLOPPY_3_SSDD;
extern const device_type FLOPPY_3_DSDD;
extern const device_type FLOPPY_35_SSDD;
extern const device_type FLOPPY_35_DD;
extern const device_type FLOPPY_35_HD;
extern const device_type FLOPPY_35_ED;
extern const device_type FLOPPY_525_SSSD_35T;
extern const device_type FLOPPY_525_SD_35T;
extern const device_type FLOPPY_525_SSSD;
extern const device_type FLOPPY_525_SD;
extern const device_type FLOPPY_525_SSDD;
extern const device_type FLOPPY_525_DD;
extern const device_type FLOPPY_525_SSQD;
extern const device_type FLOPPY_525_QD;
extern const device_type FLOPPY_525_HD;
extern const device_type FLOPPY_8_SSSD;
extern const device_type FLOPPY_8_DSSD;
extern const device_type FLOPPY_8_SSDD;
extern const device_type FLOPPY_8_DSDD;
extern const device_type EPSON_SMD_165;
extern const device_type EPSON_SD_320;
extern const device_type EPSON_SD_321;
extern const device_type SONY_OA_D31V;
extern const device_type SONY_OA_D32W;
extern const device_type SONY_OA_D32V;
extern const device_type TEAC_FD_55E;
extern const device_type TEAC_FD_55F;
extern const device_type TEAC_FD_55G;
extern const device_type ALPS_3255190x;

#endif /* FLOPPY_H */
