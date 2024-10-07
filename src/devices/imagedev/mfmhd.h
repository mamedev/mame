// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    MFM hard disk emulation

    See mfmhd.c for documentation

    Michael Zapf
    August 2015

*****************************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_MFMHD_H
#define MAME_DEVICES_IMAGEDEV_MFMHD_H

#pragma once

#include "imagedev/harddriv.h"

#include "formats/mfm_hd.h"

#include <string>
#include <system_error>
#include <utility>


class mfm_harddisk_device;

class mfmhd_trackimage
{
public:
	bool    dirty;
	int     cylinder;
	int     head;
	std::unique_ptr<uint16_t []> encdata;            // MFM encoding per byte
	mfmhd_trackimage* next;
};

class mfmhd_trackimage_cache
{
public:
	mfmhd_trackimage_cache(running_machine &machine);
	~mfmhd_trackimage_cache();
	void        init(mfm_harddisk_device* mfmhd, int tracksize, int trackslots);
	uint16_t*     get_trackimage(int cylinder, int head);
	void        mark_current_as_dirty();
	void        cleanup();
	void        write_back_one();

private:
	mfm_harddisk_device*        m_mfmhd;
	mfmhd_trackimage*           m_tracks;
	running_machine &           m_machine;
};

class mfm_harddisk_device : public device_t, public device_image_interface
{
public:
	~mfm_harddisk_device();

	typedef delegate<void (mfm_harddisk_device*, int)> index_pulse_cb;
	typedef delegate<void (mfm_harddisk_device*, int)> ready_cb;
	typedef delegate<void (mfm_harddisk_device*, int)> seek_complete_cb;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_type_name() const noexcept override { return "harddisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "hard"; }
	virtual const char *file_extensions() const noexcept override { return "chd"; }

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
	line_state      seek_complete_r() { return m_seek_complete? ASSERT_LINE : CLEAR_LINE; }
	line_state      trk00_r() { return m_current_cylinder==0? ASSERT_LINE : CLEAR_LINE; }

	// Data output towards controller
	bool            read(attotime &from_when, const attotime &limit, uint16_t &data);

	// Data input from controller
	bool            write(attotime &from_when, const attotime &limit, uint16_t cdata, bool wpcom=false, bool reduced_wc=false);

	// Step
	void            step_w(line_state line);
	void            direction_in_w(line_state line);

	// Head select
	void            headsel_w(int head) { m_current_head = head & 0x0f; }

	std::pair<std::error_condition, std::string> call_load() override;
	void            call_unload() override;

	// Tells us the time when the track ends (next index pulse). Needed by the controller.
	attotime        track_end_time();

	// Access the tracks on the image. Used as a callback from the cache.
	std::error_condition load_track(uint16_t* data, int cylinder, int head);
	void            write_track(uint16_t* data, int cylinder, int head);

	// Delivers the number of heads according to the loaded image
	int             get_actual_heads();

protected:
	mfm_harddisk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void        device_start() override;
	virtual void        device_stop() override;
	virtual void        device_reset() override;

	TIMER_CALLBACK_MEMBER(index_timer);
	TIMER_CALLBACK_MEMBER(recalibrate);
	TIMER_CALLBACK_MEMBER(seek_update);
	TIMER_CALLBACK_MEMBER(cache_update);

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

	std::unique_ptr<mfmhd_trackimage_cache> m_cache;
	mfmhd_image_format_t*   m_format;
	chd_file   *m_chd;
	void        head_move();

	// Common routine for read/write
	bool            find_position(attotime &from_when, const attotime &limit, int &bytepos, int &bitpos);
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
	mfm_hd_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MFMHD_GENERIC, mfm_hd_generic_device)

class mfm_hd_st213_device : public mfm_harddisk_device
{
public:
	mfm_hd_st213_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MFMHD_ST213, mfm_hd_st213_device)

class mfm_hd_st225_device : public mfm_harddisk_device
{
public:
	mfm_hd_st225_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MFMHD_ST225, mfm_hd_st225_device)

class mfm_hd_st251_device : public mfm_harddisk_device
{
public:
	mfm_hd_st251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MFMHD_ST251, mfm_hd_st251_device)


/*
    Connector for a MFM hard disk. Similar concept as in floppy.cpp.
*/
class mfm_harddisk_connector : public device_t,
								public device_slot_interface
{
public:
	template <typename T>
	mfm_harddisk_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts,
		const char *dflt, mfmhd_enc_t enc, int spinup, int cache, const mfmhd_format_type format, bool fixed = false)
		: mfm_harddisk_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		configure(enc, spinup, cache, format);
	}

	mfm_harddisk_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~mfm_harddisk_connector();

	mfm_harddisk_device *get_device();

	/*
	    Configuration parameters:
	    encoding = Encoding (see comments in mfm_hd.c)
	    spinupms = Spinup time in milliseconds. Even though this is a property
	        of the physical device, we need a way to configure it per system;
	        some systems expect the hard disk to be turned on before the
	        main system, and expect it to be ready when they try to access it
	    cache = number of cached MFM tracks
	    format = MFMHD_GEN_FORMAT (see formats/mfm_hd.h; currently the only value)
	*/
	void configure(mfmhd_enc_t encoding, int spinupms, int cache, mfmhd_format_type format);

protected:
	void device_start() override ATTR_COLD;
	void device_config_complete() override;

private:
	mfmhd_enc_t m_encoding;
	int m_spinupms;
	int m_cachesize;
	mfmhd_image_format_t *m_format;
};

DECLARE_DEVICE_TYPE(MFM_HD_CONNECTOR, mfm_harddisk_connector)

#endif // MAME_DEVICES_IMAGEDEV_MFMHD_H
