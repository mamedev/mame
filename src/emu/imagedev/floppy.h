/*********************************************************************

    floppy.h

*********************************************************************/

#ifndef FLOPPY_H
#define FLOPPY_H

#include "formats/flopimg.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
// ======================> floppy_interface

struct floppy_interface
{
	devcb_write_line				m_out_idx_cb;  /* index */

	const floppy_format_type 		*m_formats;
	const char *					m_interface;
	device_image_display_info_func	m_device_displayinfo;
	device_image_load_func			m_load_func;
	device_image_unload_func		m_unload_func;
};

// ======================> cdrom_image_device

class floppy_image_device :	public device_t,
							public floppy_interface,
							public device_image_interface
{
public:
	typedef delegate<int (floppy_image_device *)> load_cb;
	typedef delegate<void (floppy_image_device *)> unload_cb;
	typedef delegate<void (floppy_image_device *, int)> index_pulse_cb;

	// construction/destruction
	floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }

	virtual iodevice_t image_type() const { return IO_FLOPPY; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_extension_list; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);
	void setup_index_pulse_cb(index_pulse_cb cb);

	UINT32* get_buffer() { return m_image->get_buffer(m_cyl,m_ss ^ 1); }
	UINT32 get_len() { return m_image->get_track_size(m_cyl,m_ss ^ 1); }

	void mon_w(int state);
	void index_func();
	int  ready_r();
	double get_pos();

	int wpt_r() { return m_wpt; }
	int dskchg_r() { return m_dskchg; }
	int trk00_r() { return (m_cyl==0) ? 0 : 1; }

	void stp_w(int state);
	void dir_w(int state) { m_dir = state; }
	void ss_w(int state) { m_ss = state; }

protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();

	image_device_format m_format;
	floppy_image		*m_image;
	char				m_extension_list[256];
	floppy_image_format_t *m_fif_list;

	/* index pulse timer */
	emu_timer	*m_index_timer;

	/* state of input lines */
	int m_dir;  /* direction */
	int m_stp;  /* step */
	int m_wtg;  /* write gate */
	int m_mon;  /* motor on */
	int m_ss;	/* side select */

	/* state of output lines */
	int m_idx;  /* index pulse */
	int m_tk00; /* track 00 */
	int m_wpt;  /* write protect */
	int m_rdy;  /* ready */
	int m_dskchg;		/* disk changed */

	/* rotation per minute => gives index pulse frequency */
	float m_rpm;

	int m_cyl;
	devcb_resolved_write_line m_out_idx_func;

	load_cb cur_load_cb;
	unload_cb cur_unload_cb;
	index_pulse_cb cur_index_pulse_cb;
};

// device type definition
extern const device_type FLOPPY;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define FLOPPY_0 "floppy0"
#define FLOPPY_1 "floppy1"
#define FLOPPY_2 "floppy2"
#define FLOPPY_3 "floppy3"

#define MCFG_FLOPPY_DRIVE_ADD(_config) \
	MCFG_DEVICE_ADD(FLOPPY_0, FLOPPY, 0) \
	MCFG_DEVICE_CONFIG(_config)	\

#define MCFG_FLOPPY_2_DRIVES_ADD(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_0, FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_1, FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)

#endif /* FLOPPY_H */
