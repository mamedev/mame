/*********************************************************************

    floppy.h

*********************************************************************/

#ifndef FLOPPY_H
#define FLOPPY_H

#include "formats/flopimg.h"

#define MCFG_FLOPPY_DRIVE_ADD(_tag, _type, _tracks, _sides, _formats)	\
	MCFG_DEVICE_ADD(_tag, FLOPPY, 0) \
	downcast<floppy_image_device *>(device)->set_info(_type, _tracks, _sides, _formats);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


class floppy_image_device :	public device_t,
							public device_image_interface
{
public:
	enum {
		TYPE_35_SD, TYPE_35_DD, TYPE_35_HD, TYPE_35_ED,
		TYPE_525_SD, TYPE_525_DD, TYPE_525_HD
	};

	typedef delegate<int (floppy_image_device *)> load_cb;
	typedef delegate<void (floppy_image_device *)> unload_cb;
	typedef delegate<void (floppy_image_device *, int)> index_pulse_cb;

	// construction/destruction
	floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_image_device();

	void set_info(int type, int tracks, int sides, const floppy_format_type *formats);
	void set_rpm(float rpm);

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual void call_display_info() {}
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }

	virtual iodevice_t image_type() const { return IO_FLOPPY; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *file_extensions() const { return extension_list; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);
	void setup_index_pulse_cb(index_pulse_cb cb);

	UINT32* get_buffer() { return image->get_buffer(cyl, ss ^ 1); }
	UINT32 get_len() { return image->get_track_size(cyl, ss ^ 1); }

	void mon_w(int state);
	int  ready_r();
	double get_pos();

	int wpt_r() { return wpt; }
	int dskchg_r() { return dskchg; }
	bool trk00_r() { return cyl != 0; }
	int idx_r() { return idx; }

	void stp_w(int state);
	void dir_w(int state) { dir = state; }
	void ss_w(int state) { ss = state; }

	void index_resync();
	attotime time_next_index();
	attotime get_next_transition(attotime from_when);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	image_device_format   format;
	floppy_image		  *image;
	char				  extension_list[256];
	floppy_image_format_t *fif_list;
	emu_timer             *index_timer;

	/* Physical characteristics */
	int type;   /* reader type */
	int tracks; /* addressable tracks */
	int sides;  /* number of heads */

	/* state of input lines */
	int dir;  /* direction */
	int stp;  /* step */
	int wtg;  /* write gate */
	int mon;  /* motor on */
	int ss;	/* side select */

	/* state of output lines */
	int idx;  /* index pulse */
	int wpt;  /* write protect */
	int rdy;  /* ready */
	int dskchg;		/* disk changed */

	/* rotation per minute => gives index pulse frequency */
	float rpm;

	attotime revolution_start_time, rev_time;
	UINT32 revolution_count;
	int cyl;

	load_cb cur_load_cb;
	unload_cb cur_unload_cb;
	index_pulse_cb cur_index_pulse_cb;

	int find_position(int position, const UINT32 *buf, int buf_size);
};

// device type definition
extern const device_type FLOPPY;

#endif /* FLOPPY_H */
