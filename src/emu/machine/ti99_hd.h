// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hard disk support
    See mfm_hd.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TI99_HD__
#define __TI99_HD__

#include "emu.h"
#include "imagedev/harddriv.h"

const chd_metadata_tag MFM_HARD_DISK_METADATA_TAG = CHD_MAKE_TAG('G','D','D','I');

extern const char *MFMHD_REC_METADATA_FORMAT;
extern const char *MFMHD_GAP_METADATA_FORMAT;

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
class mfm_harddisk_device;

// Pointer to its alloc function
typedef mfmhd_image_format_t *(*mfmhd_format_type)();

template<class _FormatClass>
mfmhd_image_format_t *mfmhd_image_format_creator()
{
	return new _FormatClass();
}

/*
    Parameters for the track layout
*/
class mfmhd_layout_params
{
public:
	// Geometry params. These are fixed for the device. However, sector sizes
	// could be changed, but we do not support this (yet). These are defined
	// in the CHD and must match those of the device. They are stored by the GDDD tag.
	// The encoding is not stored in the CHD but is also supposed to be immutable.
	int     cylinders;
	int     heads;
	int     sectors_per_track;
	int     sector_size;
	mfmhd_enc_t     encoding;

	// Parameters like interleave, precompensation, write current can be changed
	// on every write operation. They are stored by the GDDI tag (first record).
	int     interleave;
	int     cylskew;
	int     headskew;
	int     write_precomp_cylinder;     // if -1, no wpcom on the disks
	int     reduced_wcurr_cylinder;     // if -1, no rwc on the disks

	// Parameters for the track layout that are supposed to be the same for
	// all tracks and that do not change (until the next reformat).
	// Also, they do not have any influence on the CHD file.
	// They are stored by the GDDI tag (second record).
	int     gap1;
	int     gap2;
	int     gap3;
	int     sync;
	int     headerlen;
	int     ecctype;        // -1 is CRC

	bool sane_rec()
	{
		return ((interleave >= 0 && interleave < 32) && (cylskew >= 0 && cylskew < 32) && (headskew >= 0 && headskew < 32)
			&& (write_precomp_cylinder >= -1 && write_precomp_cylinder < 100000)
			&& (reduced_wcurr_cylinder >= -1 && reduced_wcurr_cylinder < 100000));
	}

	void reset_rec()
	{
		interleave = cylskew = headskew = 0;
		write_precomp_cylinder = reduced_wcurr_cylinder = -1;
	}

	bool sane_gap()
	{
		return ((gap1 >= 1 && gap1 < 1000) && (gap2 >= 1 && gap2 < 20) && (gap3 >= 1 && gap3 < 1000)
			&& (sync >= 10 && sync < 20)
			&& (headerlen >= 4 && headerlen<=5) && (ecctype>=-1 && ecctype < 10));
	}

	void reset_gap()
	{
		gap1 = gap2 = gap3 = sync = headerlen = ecctype = 0;
	}

	bool equals_rec(mfmhd_layout_params* other)
	{
		return ((interleave == other->interleave) &&
				(cylskew == other->cylskew) &&
				(headskew == other->headskew) &&
				(write_precomp_cylinder == other->write_precomp_cylinder) &&
				(reduced_wcurr_cylinder == other->reduced_wcurr_cylinder));
	}

	bool equals_gap(mfmhd_layout_params* other)
	{
		return ((gap1 == other->gap1) &&
				(gap2 == other->gap2) &&
				(gap3 == other->gap3) &&
				(sync == other->sync) &&
				(headerlen == other->headerlen) &&
				(ecctype == other->ecctype));
	}
};

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
	void        init(mfm_harddisk_device* mfmhd, int tracksize, int trackslots);
	UINT16*     get_trackimage(int cylinder, int head);
	void        mark_current_as_dirty();
	void        cleanup();
	void        write_back_one();

private:
	mfm_harddisk_device*        m_mfmhd;
	mfmhd_trackimage*           m_tracks;
	void        showtrack(UINT16* enctrack, int length);
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
	bool            write(attotime &from_when, const attotime &limit, UINT16 cdata, bool wpcom=false, bool reduced_wc=false);

	// Step
	void            step_w(line_state line);
	void            direction_in_w(line_state line);

	// Head select
	void            headsel_w(int head) { m_current_head = head & 0x0f; }

	bool            call_load();
	void            call_unload();

	// Tells us the time when the track ends (next index pulse)
	attotime        track_end_time();

	// Access the tracks on the image. Used as a callback from the cache.
	chd_error       load_track(UINT16* data, int cylinder, int head);
	void            write_track(UINT16* data, int cylinder, int head);

	// Delivers the number of heads according to the loaded image
	int             get_actual_heads();

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
	int         m_landing_zone;
	int         m_precomp_cyl;
	int         m_redwc_cyl;

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
    _enc = Encoding (see comments in mfm_hd.c)
    _spinupms = Spinup time in milliseconds (some configurations assume that the
    user has turned on the hard disk before turning on the system. We cannot
    emulate this, so we allow for shorter times)
    _cache = number of cached MFM tracks
*/
#define MCFG_MFM_HARDDISK_CONN_ADD(_tag, _slot_intf, _def_slot, _enc, _spinupms, _cache, _format)  \
	MCFG_DEVICE_ADD(_tag, MFM_HD_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	static_cast<mfm_harddisk_connector *>(device)->configure(_enc, _spinupms, _cache, _format);


enum mfmhd_param_t
{
	MFMHD_IL,
	MFMHD_HSKEW,
	MFMHD_CSKEW,
	MFMHD_WPCOM,
	MFMHD_RWC,
	MFMHD_GAP1,
	MFMHD_GAP2,
	MFMHD_GAP3,
	MFMHD_SYNC,
	MFMHD_HLEN,
	MFMHD_ECC
};

/*
    Hard disk format
*/
class mfmhd_image_format_t
{
public:
	mfmhd_image_format_t() {};
	virtual ~mfmhd_image_format_t() {};

	// Load the image.
	virtual chd_error load(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head) = 0;

	// Save the image.
	virtual chd_error save(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head) = 0;

	// Return the original parameters of the image
	mfmhd_layout_params* get_initial_params() { return &m_param_old; }

	// Return the recent parameters of the image
	mfmhd_layout_params* get_current_params() { return &m_param; }

	// Set the track layout parameters (and reset the skew detection values)
	void set_layout_params(mfmhd_layout_params param);

	// Concrete format shall decide whether we want to save the retrieved parameters or not.
	virtual bool save_param(mfmhd_param_t type) =0;

protected:
	bool    m_lastbit;
	int     m_current_crc;
	int     m_secnumber[4];     // used to determine the skew values

	mfmhd_layout_params m_param, m_param_old;

	void    mfm_encode(UINT16* trackimage, int& position, UINT8 byte, int count=1);
	void    mfm_encode_a1(UINT16* trackimage, int& position);
	void    mfm_encode_mask(UINT16* trackimage, int& position, UINT8 byte, int count, int mask);
	UINT8   mfm_decode(UINT16 raw);
	void    showtrack(UINT16* enctrack, int length);

	// Deliver defaults.
	virtual int get_default(mfmhd_param_t type) =0;
};

class gen_mfmhd_format : public mfmhd_image_format_t
{
public:
	gen_mfmhd_format() {};
	chd_error load(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head);
	chd_error save(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head);

	// Yes, we want to save all parameters
	virtual bool save_param(mfmhd_param_t type) { return true; }
	virtual int get_default(mfmhd_param_t type);

private:
	UINT8   cylinder_to_ident(int cylinder);
	int     chs_to_lba(int cylinder, int head, int sector);
};

extern const mfmhd_format_type MFMHD_GEN_FORMAT;

#endif
