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
	void init(chd_file* chdfile, const char* tag, int maxcyl, int maxhead, int trackslots, mfmhd_enc_t encoding);
	UINT16* get_trackimage(int cylinder, int head);

private:
	void        mfm_encode(mfmhd_trackimage* slot, int& position, UINT8 byte, int count=1);
	void        mfm_encode_a1(mfmhd_trackimage* slot, int& position);
	void        mfm_encode_mask(mfmhd_trackimage* slot, int& position, UINT8 byte, int count, int mask);
	chd_error   load_track(mfmhd_trackimage* slot, int cylinder, int head, int sectorcount, int size, int interleave);
	void        write_back(mfmhd_trackimage* timg);
	int         chs_to_lba(int cylinder, int head, int sector);
	chd_file*   m_chd;

	const char* m_tagdev;
	mfmhd_trackimage* m_tracks;
	mfmhd_enc_t m_encoding;
	bool        m_lastbit;
	int         m_current_crc;
	int         m_cylinders;
	int         m_heads;
	int         m_sectors_per_track;
	int         m_sectorsize;
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

	mfmhd_enc_t get_encoding() { return m_encoding; }

	// Active low lines. We're using ASSERT=0 / CLEAR=1
	line_state      ready_r() { return m_ready? ASSERT_LINE : CLEAR_LINE; }
	line_state      seek_complete_r() { return m_seek_complete? ASSERT_LINE : CLEAR_LINE; } ;
	line_state      trk00_r() { return m_current_cylinder==0? ASSERT_LINE : CLEAR_LINE; }

	// Data output towards controller
	bool            read(attotime &from_when, const attotime &limit, UINT16 &data);

	// Step
	void            step_w(line_state line);
	void            direction_in_w(line_state line);

	// Head select
	void            headsel_w(int head) { m_current_head = head & 0x0f; }

	bool            call_load();

	// Tells us the time when the track ends (next index pulse)
	attotime        track_end_time();

protected:
	void                device_start();
	void                device_stop();
	void                device_reset();
	void                device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void        setup_characteristics() = 0;
	std::string         tts(const attotime &t);

	emu_timer           *m_index_timer, *m_spinup_timer, *m_seek_timer;
	index_pulse_cb      m_index_pulse_cb;
	ready_cb            m_ready_cb;
	seek_complete_cb    m_seek_complete_cb;

	int m_max_cylinder;
	int m_max_heads;

private:
	mfmhd_enc_t m_encoding;
	int         m_spinupms;
	int         m_cachelines;
	bool        m_ready;
	int         m_current_cylinder;
	int         m_current_head;
	int         m_track_delta;
	int         m_step_phase;
	bool        m_seek_complete;
	bool        m_seek_inward;
	//bool      m_seeking;
	bool        m_autotruncation;
	bool        m_recalibrated;
	line_state  m_step_line;    // keep the last state

	attotime    m_spinup_time;
	attotime    m_revolution_start_time;
	attotime    m_rev_time;

	mfmhd_trackimage_cache* m_cache;

	void        prepare_track(int cylinder, int head);
	void        head_move();
	void        recalibrate();
};

class mfm_hd_generic_device : public mfm_harddisk_device
{
public:
	mfm_hd_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	void setup_characteristics();
};

extern const device_type MFMHD_GENERIC;

class mfm_hd_st225_device : public mfm_harddisk_device
{
public:
	mfm_hd_st225_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	void setup_characteristics();
};

extern const device_type MFMHD_ST225;

/* Connector for a MFM hard disk. See also floppy.c */
class mfm_harddisk_connector : public device_t,
								public device_slot_interface
{
public:
	mfm_harddisk_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mfm_harddisk_connector();

	mfm_harddisk_device *get_device();

	void configure(mfmhd_enc_t encoding, int spinupms, int cache);

protected:
	void device_start() { };
	void device_config_complete();

private:
	mfmhd_enc_t m_encoding;
	int m_spinupms;
	int m_cachesize;
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
#define MCFG_MFM_HARDDISK_CONN_ADD(_tag, _slot_intf, _def_slot, _enc, _spinupms, _cache)  \
	MCFG_DEVICE_ADD(_tag, MFM_HD_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	static_cast<mfm_harddisk_connector *>(device)->configure(_enc, _spinupms, _cache);

// ===========================================================================
// Legacy implementation
// ===========================================================================
#define MFMHD_0 "mfmhd0"
#define MFMHD_1 "mfmhd1"
#define MFMHD_2 "mfmhd2"

extern const device_type TI99_MFMHD_LEG;

/*
    Needed to adapt to higher cylinder numbers. Floppies do not have such
    high numbers.
*/
struct chrn_id_hd
{
	UINT16 C;
	UINT8 H;
	UINT8 R;
	UINT8 N;
	int data_id;            // id for read/write data command
	unsigned long flags;
};

class mfm_harddisk_legacy_device : public device_t
{
public:
	mfm_harddisk_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void    read_sector(int cylinder, int head, int sector, UINT8 *buf);
	void    write_sector(int cylinder, int head, int sector, UINT8 *buf);
	void    read_track(int head, UINT8 *buffer);
	void    write_track(int head, UINT8 *buffer, int data_count);
	UINT8   get_status();
	void    seek(int direction);
	void    get_next_id(int head, chrn_id_hd *id);
	int     get_track_length();

protected:
	void    device_start();
	void    device_reset();
	machine_config_constructor device_mconfig_additions() const;

private:
	int     find_block(const UINT8 *buffer, int start, int stop, UINT8 byte, size_t number);
	UINT8   cylinder_to_ident(int cylinder);
	bool    harddisk_chs_to_lba(hard_disk_file *hdfile, int cylinder, int head, int sector, UINT32 *lba);

	int     m_current_cylinder;
	int     m_current_head;
	bool    m_seeking;
	int     m_status;
	int     m_id_index; /* position in track for seeking the sector; counts the sector number */

	harddisk_image_device *m_drive;
};

class ide_harddisk_legacy_device : public device_t
{
public:
	ide_harddisk_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	virtual void    device_start() { };
	virtual void    device_reset() { };
	virtual machine_config_constructor device_mconfig_additions() const;
};

#define MCFG_MFMHD_3_DRIVES_ADD()           \
	MCFG_DEVICE_ADD(MFMHD_0, TI99_MFMHD_LEG, 0)     \
	MCFG_DEVICE_ADD(MFMHD_1, TI99_MFMHD_LEG, 0)     \
	MCFG_DEVICE_ADD(MFMHD_2, TI99_MFMHD_LEG, 0)

#endif
