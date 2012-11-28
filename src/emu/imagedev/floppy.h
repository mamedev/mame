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

#define MCFG_FLOPPY_DRIVE_ADD(_tag, _slot_intf, _def_slot, _def_inp, _formats)  \
	MCFG_DEVICE_ADD(_tag, FLOPPY_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	static_cast<floppy_connector *>(device)->set_formats(_formats);

#define DECLARE_FLOPPY_FORMATS(_name) \
	static const floppy_format_type _name [];

#define FLOPPY_FORMATS_MEMBER(_member) \
	const floppy_format_type _member [] = { \

#define FLOPPY_FORMATS_END \
		, \
		FLOPPY_D88_FORMAT, \
		FLOPPY_DFI_FORMAT, \
		FLOPPY_IMD_FORMAT, \
		FLOPPY_IPF_FORMAT, \
		FLOPPY_MFI_FORMAT, \
		FLOPPY_MFM_FORMAT, \
		NULL };


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

	// construction/destruction
	floppy_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_image_device();

	virtual void handled_variants(UINT32 *variants, int &var_count) const = 0;

	void set_formats(const floppy_format_type *formats);
	floppy_image_format_t *get_formats() const;
	floppy_image_format_t *get_load_format() const;
	floppy_image_format_t *identify(astring filename) const;
	void set_rpm(float rpm);

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual void call_display_info() {}
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }
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

	UINT32* get_buffer() { return image->get_buffer(cyl, ss); }
	UINT32 get_len() { return image->get_track_size(cyl, ss); }

	void mon_w(int state);
	bool ready_r();
	double get_pos();

	bool wpt_r() { return output_format == 0; }
	int dskchg_r() { return dskchg; }
	bool trk00_r() { return cyl != 0 || !image; }
	int idx_r() { return idx; }
	bool ss_r() { return ss; }
	bool twosid_r();

	void stp_w(int state);
	void dir_w(int state) { dir = state; }
	void ss_w(int state) { ss = state; }

	void index_resync();
	attotime time_next_index();
	attotime get_next_transition(attotime from_when);
	void write_flux(attotime start, attotime end, int transition_count, const attotime *transitions);
	void set_write_splice(attotime when);
	UINT32 get_form_factor() const;
	UINT32 get_variant() const;

	virtual ui_menu *get_selection_menu(running_machine &machine, class render_container *container);

	static const floppy_format_type default_floppy_formats[];

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void setup_characteristics() = 0;

	image_device_format   format;
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

	/* rotation per minute => gives index pulse frequency */
	float rpm;

	attotime revolution_start_time, rev_time;
	UINT32 revolution_count;
	int cyl;

	bool image_dirty;

	load_cb cur_load_cb;
	unload_cb cur_unload_cb;
	index_pulse_cb cur_index_pulse_cb;

	UINT32 find_position(attotime &base, attotime when);
	int find_index(UINT32 position, const UINT32 *buf, int buf_size);
	void write_zone(UINT32 *buf, int &cells, int &index, UINT32 spos, UINT32 epos, UINT32 mg);
	void commit_image();
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
	astring input_filename, output_filename;

	void do_load_create();
	virtual void hook_load(astring filename, bool softlist);
};

class floppy_3_ssdd : public floppy_image_device {
public:
	floppy_3_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_3_ssdd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_3_ssdd"; }
	virtual const char *image_interface() const { return "floppy_3"; }
protected:
	virtual void setup_characteristics();
};

class floppy_3_dsdd : public floppy_image_device {
public:
	floppy_3_dsdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_3_dsdd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_3_dsdd"; }
	virtual const char *image_interface() const { return "floppy_3"; }
protected:
	virtual void setup_characteristics();
};

class floppy_35_dd : public floppy_image_device {
public:
	floppy_35_dd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_35_dd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_35_dd"; }
	virtual const char *image_interface() const { return "floppy_3_5"; }
protected:
	virtual void setup_characteristics();
};

class floppy_35_dd_nosd : public floppy_image_device {
public:
	floppy_35_dd_nosd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_35_dd_nosd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_35_dd_nosd"; }
	virtual const char *image_interface() const { return "floppy_3_5"; }
protected:
	virtual void setup_characteristics();
};

class floppy_35_hd : public floppy_image_device {
public:
	floppy_35_hd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_35_hd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_35_hd"; }
	virtual const char *image_interface() const { return "floppy_3_5"; }
protected:
	virtual void setup_characteristics();
};

class floppy_35_ed : public floppy_image_device {
public:
	floppy_35_ed(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_35_ed();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_35_ed"; }
	virtual const char *image_interface() const { return "floppy_3_5"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_sssd_35t : public floppy_image_device {
public:
	floppy_525_sssd_35t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_sssd_35t();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_sssd_35t"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_sd_35t : public floppy_image_device {
public:
	floppy_525_sd_35t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_sd_35t();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_sd_35t"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_sssd : public floppy_image_device {
public:
	floppy_525_sssd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_sssd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_sssd"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_sd : public floppy_image_device {
public:
	floppy_525_sd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_sd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_sd"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_ssdd : public floppy_image_device {
public:
	floppy_525_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_ssdd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_ssdd"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_dd : public floppy_image_device {
public:
	floppy_525_dd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_dd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_dd"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_qd : public floppy_image_device {
public:
	floppy_525_qd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_qd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_qd"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_525_hd : public floppy_image_device {
public:
	floppy_525_hd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_525_hd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_525_hd"; }
	virtual const char *image_interface() const { return "floppy_5_25"; }
protected:
	virtual void setup_characteristics();
};

class floppy_8_sssd : public floppy_image_device {
public:
	floppy_8_sssd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_8_sssd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_8_sssd"; }
	virtual const char *image_interface() const { return "floppy_8"; }
protected:
	virtual void setup_characteristics();
};

class floppy_8_ssdd : public floppy_image_device {
public:
	floppy_8_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_8_ssdd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_8_ssdd"; }
	virtual const char *image_interface() const { return "floppy_8"; }
protected:
	virtual void setup_characteristics();
};

class floppy_8_dsdd : public floppy_image_device {
public:
	floppy_8_dsdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_8_dsdd();
	virtual void handled_variants(UINT32 *variants, int &var_count) const;
	virtual void device_config_complete() { m_shortname = "floppy_8_dsdd"; }
	virtual const char *image_interface() const { return "floppy_8"; }
protected:
	virtual void setup_characteristics();
};

class floppy_connector: public device_t,
						public device_slot_interface
{
public:
	floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_connector();

	void set_formats(const floppy_format_type *formats);
	floppy_image_device *get_device();

protected:
	virtual void device_start();
	virtual void device_config_complete();

private:
	const floppy_format_type *formats;
};


// device type definition
extern const device_type FLOPPY_CONNECTOR;
extern const device_type FLOPPY_3_SSDD;
extern const device_type FLOPPY_3_DSDD;
extern const device_type FLOPPY_35_DD;
extern const device_type FLOPPY_35_DD_NOSD;
extern const device_type FLOPPY_35_HD;
extern const device_type FLOPPY_35_ED;
extern const device_type FLOPPY_525_SSSD_35T;
extern const device_type FLOPPY_525_SD_35T;
extern const device_type FLOPPY_525_SSSD;
extern const device_type FLOPPY_525_SD;
extern const device_type FLOPPY_525_SSDD;
extern const device_type FLOPPY_525_DD;
extern const device_type FLOPPY_525_QD;
extern const device_type FLOPPY_525_HD;
extern const device_type FLOPPY_8_SSSD;
extern const device_type FLOPPY_8_SSDD;
extern const device_type FLOPPY_8_DSDD;

#endif /* FLOPPY_H */
