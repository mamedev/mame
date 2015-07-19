// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hard disk support
    See ti99_hd.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TI99_HD__
#define __TI99_HD__

#include "emu.h"
#include "imagedev/harddriv.h"

/*
    Determine how data are passed from the hard disk to the controller. We
    allow for different degrees of hardware emulation.
*/
enum mfmhd_enc_t
{
	MFM_BITS,               // One bit at a time
	MFM_BYTE,               // One data byte with interleaved clock bits
	SEPARATED,              // 8 clock bits (most sig byte), one data byte (least sig byte)
	SEPARATED_SIMPLE        // MSB: 00/FF (standard / mark) clock, LSB: one data byte
};

class mfmhd_image_format_t;

// Pointer to its alloc function
typedef mfmhd_image_format_t *(*mfmhd_format_type)();

template<class _FormatClass>
mfmhd_image_format_t *mfmhd_image_format_creator()
{
	return new _FormatClass();
}

class mfmhd_trackimage
{
public:
	bool    dirty;
	int     cylinder;
	int     head;
	UINT16* encdata;            // MFM encoding per byte
	mfmhd_trackimage* next;
};

class mfmhd_trackimage_cache
{
public:
	mfmhd_trackimage_cache();
	~mfmhd_trackimage_cache();
	void        init(chd_file* chdfile, const char* tag, int tracksize, int imagecyls, int imageheads, int imagesecpt, int trackslots, mfmhd_enc_t encoding, mfmhd_image_format_t* format);
	UINT16*     get_trackimage(int cylinder, int head);
	void        mark_current_as_dirty();
	void        cleanup();
	void        write_back_one();
	int         get_cylinders() { return m_cylinders; }

private:
	chd_file*   m_chd;

	const char*             m_tagdev;
	mfmhd_trackimage*       m_tracks;
	mfmhd_enc_t             m_encoding;
	mfmhd_image_format_t*   m_format;

	bool        m_lastbit;
	int         m_current_crc;
	int         m_cylinders;
	int         m_heads;
	int         m_sectors_per_track;
	int         m_sectorsize;
	int         m_tracksize;

	void        showtrack(UINT16* enctrack, int length);
	const char* tag() { return m_tagdev; }
};

class mfm_harddisk_device : public harddisk_image_device,
							public device_slot_card_interface
{
public:
	mfm_harddisk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~mfm_harddisk_device();

	typedef delegate<void (mfm_harddisk_device*, int)> index_pulse_cb;
	typedef delegate<void (mfm_harddisk_device*, int)> ready_cb;
	typedef delegate<void (mfm_harddisk_device*, int)> seek_complete_cb;

	void setup_index_pulse_cb(index_pulse_cb cb);
	void setup_ready_cb(ready_cb cb);
	void setup_seek_complete_cb(seek_complete_cb cb);

	// Configuration
	void set_encoding(mfmhd_enc_t encoding) { m_encoding = encoding; }
	void set_spinup_time(int spinupms) { m_spinupms = spinupms; }
	void set_cache_size(int tracks) { m_cachelines = tracks;    }
	void set_format(mfmhd_image_format_t* format) { m_format = format; }

	mfmhd_enc_t get_encoding() { return m_encoding; }

	// Active low lines. We're using ASSERT=0 / CLEAR=1
	line_state      ready_r() { return m_ready? ASSERT_LINE : CLEAR_LINE; }
	line_state      seek_complete_r() { return m_seek_complete? ASSERT_LINE : CLEAR_LINE; } ;
	line_state      trk00_r() { return m_current_cylinder==0? ASSERT_LINE : CLEAR_LINE; }

	// Common routine for read/write
	bool            find_position(attotime &from_when, const attotime &limit, int &bytepos, int &bitpos);

	// Data output towards controller
	bool            read(attotime &from_when, const attotime &limit, UINT16 &data);

	// Data input from controller
	bool            write(attotime &from_when, const attotime &limit, UINT16 data);

	// Step
	void            step_w(line_state line);
	void            direction_in_w(line_state line);

	// Head select
	void            headsel_w(int head) { m_current_head = head & 0x0f; }

	bool            call_load();
	void            call_unload();

	// Tells us the time when the track ends (next index pulse)
	attotime        track_end_time();

protected:
	void                device_start();
	void                device_stop();
	void                device_reset();
	void                device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	std::string         tts(const attotime &t);

	emu_timer           *m_index_timer, *m_spinup_timer, *m_seek_timer, *m_cache_timer;
	index_pulse_cb      m_index_pulse_cb;
	ready_cb            m_ready_cb;
	seek_complete_cb    m_seek_complete_cb;

	int         m_max_cylinders;
	int         m_phys_cylinders;
	int         m_actual_cylinders;  // after reading the CHD
	int         m_max_heads;
	int         m_park_pos;
	int         m_maxseek_time;
	int         m_seeknext_time;

private:
	mfmhd_enc_t m_encoding;
	int         m_cell_size;    // nanoseconds
	int         m_trackimage_size;  // number of 16-bit cell blocks (data bytes)
	int         m_spinupms;
	int         m_rpm;
	int         m_interleave;
	int         m_cachelines;
	bool        m_ready;
	int         m_current_cylinder;
	int         m_current_head;
	int         m_track_delta;
	int         m_step_phase;
	bool        m_seek_complete;
	bool        m_seek_inward;
	bool        m_autotruncation;
	bool        m_recalibrated;
	line_state  m_step_line;    // keep the last state

	attotime    m_spinup_time;
	attotime    m_revolution_start_time;
	attotime    m_rev_time;

	attotime    m_settle_time;
	attotime    m_step_time;

	mfmhd_trackimage_cache* m_cache;
	mfmhd_image_format_t*   m_format;

	void        prepare_track(int cylinder, int head);
	void        head_move();
	void        recalibrate();
};

/*
    The Generic drive is a MFM drive that has just enough heads and cylinders
    to handle the CHD image.

    Specific Seagate models:

    ST-213: 10 MB
    ST-225: 20 MB
    ST-251: 40 MB
*/
class mfm_hd_generic_device : public mfm_harddisk_device
{
public:
	mfm_hd_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MFMHD_GENERIC;

class mfm_hd_st213_device : public mfm_harddisk_device
{
public:
	mfm_hd_st213_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MFMHD_ST213;

class mfm_hd_st225_device : public mfm_harddisk_device
{
public:
	mfm_hd_st225_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MFMHD_ST225;

class mfm_hd_st251_device : public mfm_harddisk_device
{
public:
	mfm_hd_st251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MFMHD_ST251;


/* Connector for a MFM hard disk. See also floppy.c */
class mfm_harddisk_connector : public device_t,
								public device_slot_interface
{
public:
	mfm_harddisk_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mfm_harddisk_connector();

	mfm_harddisk_device *get_device();

	void configure(mfmhd_enc_t encoding, int spinupms, int cache, mfmhd_format_type format);

protected:
	void device_start() { };
	void device_config_complete();

private:
	mfmhd_enc_t m_encoding;
	int m_spinupms;
	int m_cachesize;
	mfmhd_image_format_t* m_format;
};

extern const device_type MFM_HD_CONNECTOR;

/*
    Add a harddisk connector.
    Parameters:
    _tag = Tag of the connector
    _slot_intf = Selection of hard drives
    _def_slot = Default hard drive
    _enc = Encoding (see comments in ti99_hd.c)
    _spinupms = Spinup time in milliseconds (some configurations assume that the
    user has turned on the hard disk before turning on the system. We cannot
    emulate this, so we allow for shorter times)
    _cache = number of cached MFM tracks
*/
#define MCFG_MFM_HARDDISK_CONN_ADD(_tag, _slot_intf, _def_slot, _enc, _spinupms, _cache, _format)  \
	MCFG_DEVICE_ADD(_tag, MFM_HD_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	static_cast<mfm_harddisk_connector *>(device)->configure(_enc, _spinupms, _cache, _format);


/*
    Hard disk format
*/
class mfmhd_image_format_t
{
public:
	mfmhd_image_format_t();
	virtual ~mfmhd_image_format_t();

	// Load the image.
	virtual chd_error load(const char* tagdev, chd_file* chdfile, UINT16* trackimage, mfmhd_enc_t encoding, int tracksize, int cylinder, int head, int cylcnt, int headcnt, int sect_per_track) = 0;

	// Save the image.
	virtual chd_error save(const char* tagdev, chd_file* chdfile, UINT16* trackimage, mfmhd_enc_t encoding, int tracksize, int cylinder, int head, int cylcnt, int headcnt, int sect_per_track) = 0;

	// Return the recent interleave of the image
	int get_interleave() { return m_interleave; }

protected:
	bool        m_lastbit;
	int         m_current_crc;
	mfmhd_enc_t m_encoding;
	const char* m_tagdev;
	int         m_cylinders;
	int         m_heads;
	int         m_sectors_per_track;
	int         m_interleave;

	void    mfm_encode(UINT16* trackimage, int& position, UINT8 byte, int count=1);
	void    mfm_encode_a1(UINT16* trackimage, int& position);
	void    mfm_encode_mask(UINT16* trackimage, int& position, UINT8 byte, int count, int mask);
	UINT8   mfm_decode(UINT16 raw);
	const char* tag() { return m_tagdev; }
	void    showtrack(UINT16* enctrack, int length);
};

class ti99_mfmhd_format : public mfmhd_image_format_t
{
public:
	ti99_mfmhd_format() {};
	chd_error load(const char* tagdev, chd_file* chdfile, UINT16* trackimage, mfmhd_enc_t encoding, int tracksize, int cylinder, int head, int cylcnt, int headcnt, int sect_per_track);
	chd_error save(const char* tagdev, chd_file* chdfile, UINT16* trackimage, mfmhd_enc_t encoding, int tracksize, int cylinder, int head, int cylcnt, int headcnt, int sect_per_track);
private:
	UINT8   cylinder_to_ident(int cylinder);
	int     chs_to_lba(int cylinder, int head, int sector);
};

extern const mfmhd_format_type MFMHD_TI99_FORMAT;

#endif
