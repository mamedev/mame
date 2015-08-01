// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*************************************************************************

    Hard disk emulation

    Michael Zapf
    April 2010
    February 2012: Rewritten as class
    April 2015: Rewritten with deeper emulation detail

    References:
    [1] ST225 OEM Manual, Seagate

**************************************************************************/

// TODO: Define format by a file

#include "emu.h"
#include "formats/imageutl.h"
#include "harddisk.h"

#include "ti99_hd.h"

#define TRACE_STEPS 0
#define TRACE_SIGNALS 0
#define TRACE_READ 0
#define TRACE_WRITE 0
#define TRACE_CACHE 0
#define TRACE_RWTRACK 0
#define TRACE_BITS 0
#define TRACE_DETAIL 0
#define TRACE_TIMING 0
#define TRACE_IMAGE 0
#define TRACE_STATE 1
#define TRACE_CONFIG 1
#define TRACE_LAYOUT 0
#define TRACE_FORMAT 1

enum
{
	INDEX_TM = 0,
	SPINUP_TM,
	SEEK_TM,
	CACHE_TM
};

enum
{
	STEP_COLLECT = 0,
	STEP_MOVING,
	STEP_SETTLE
};

#define TRACKSLOTS 5

#define OFFLIMIT -1

std::string mfm_harddisk_device::tts(const attotime &t)
{
	char buf[256];
	int nsec = t.attoseconds / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%4d.%03d,%03d,%03d", int(t.seconds), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

mfm_harddisk_device::mfm_harddisk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: harddisk_image_device(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_card_interface(mconfig, *this)
{
	m_spinupms = 10000;
	m_cachelines = TRACKSLOTS;
	m_max_cylinders = 0;
	m_phys_cylinders = 0;   // We will get this value for generic drives from the image
	m_max_heads = 0;
	m_cell_size = 100;
	m_rpm = 3600;           // MFM drives have a revolution rate of 3600 rpm (i.e. 60/sec)
	m_trackimage_size = (int)((60000000000L / (m_rpm * m_cell_size)) / 16 + 1);
	m_cache = NULL;
	// We will calculate default values from the time specs later.
	m_seeknext_time = 0;
	m_maxseek_time = 0;
	m_actual_cylinders = 0;
	m_landing_zone = 0;
	m_interleave = 0;
}

mfm_harddisk_device::~mfm_harddisk_device()
{
}

void mfm_harddisk_device::device_start()
{
	m_index_timer = timer_alloc(INDEX_TM);
	m_spinup_timer = timer_alloc(SPINUP_TM);
	m_seek_timer = timer_alloc(SEEK_TM);
	m_cache_timer = timer_alloc(CACHE_TM);

	m_rev_time = attotime::from_hz(m_rpm/60);
	m_index_timer->adjust(attotime::from_hz(m_rpm/60), 0, attotime::from_hz(m_rpm/60));

	m_current_cylinder = m_landing_zone; // Park position
	m_spinup_timer->adjust(attotime::from_msec(m_spinupms));

	m_cache = global_alloc(mfmhd_trackimage_cache);

	// In 5 second periods, check whether the cache has dirty lines
	m_cache_timer->adjust(attotime::from_msec(5000), 0, attotime::from_msec(5000));
}

void mfm_harddisk_device::device_reset()
{
	m_autotruncation = false;
	m_ready = false;
	m_seek_complete = true;
	m_seek_inward = false;
	m_track_delta = 0;
	m_step_line = CLEAR_LINE;
	m_recalibrated = false;
}

void mfm_harddisk_device::device_stop()
{
	if (m_cache!=NULL) global_free(m_cache);
}

/*
    Load the image from the CHD. We also calculate the head timing here
    because we need the number of cylinders, and for generic drives we get
    them from the CHD.
*/
bool mfm_harddisk_device::call_load()
{
	bool loaded = harddisk_image_device::call_load();
	if (loaded==IMAGE_INIT_PASS)
	{
		std::string metadata;
		chd_file* chdfile = get_chd_file();

		if (chdfile==NULL)
		{
			logerror("%s: chdfile is null\n", tag());
			return IMAGE_INIT_FAIL;
		}

		// Read the hard disk metadata
		chd_error state = chdfile->read_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
		if (state != CHDERR_NONE)
		{
			logerror("%s: Failed to read CHD metadata\n", tag());
			return IMAGE_INIT_FAIL;
		}

		if (TRACE_CONFIG) logerror("%s: CHD metadata: %s\n", tag(), metadata.c_str());

		// Parse the metadata
		mfmhd_layout_params param;
		param.encoding = m_encoding;
		if (TRACE_CONFIG) logerror("%s: Set encoding to %d\n", tag(), m_encoding);

		if (sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &param.cylinders, &param.heads, &param.sectors_per_track, &param.sector_size) != 4)
		{
			logerror("%s: Invalid CHD metadata\n", tag());
			return IMAGE_INIT_FAIL;
		}

		if (TRACE_CONFIG) logerror("%s: CHD image has geometry cyl=%d, head=%d, sect=%d, size=%d\n", tag(), param.cylinders, param.heads, param.sectors_per_track, param.sector_size);

		if (m_max_cylinders != 0 && (param.cylinders != m_max_cylinders || param.heads != m_max_heads))
		{
			throw emu_fatalerror("Image geometry does not fit this kind of hard drive: drive=(%d,%d), image=(%d,%d)", m_max_cylinders, m_max_heads, param.cylinders, param.heads);
		}

		// MDM format specs
		param.interleave = 0;
		param.cylskew = 0;
		param.headskew = 0;
		param.write_precomp_cylinder = -1;
		param.reduced_wcurr_cylinder = -1;

		state = chdfile->read_metadata(MFM_HARD_DISK_METADATA_TAG, 0, metadata);
		if (state != CHDERR_NONE)
		{
			logerror("%s: Failed to read CHD sector arrangement/recording specs, applying defaults\n", tag());
		}
		else
		{
			sscanf(metadata.c_str(), MFMHD_REC_METADATA_FORMAT, &param.interleave, &param.cylskew, &param.headskew, &param.write_precomp_cylinder, &param.reduced_wcurr_cylinder);
		}

		if (!param.sane_rec())
		{
			if (TRACE_CONFIG) logerror("%s: Sector arrangement/recording specs have invalid values, applying defaults\n", tag());
			param.reset_rec();
		}
		else
			if (TRACE_CONFIG) logerror("%s: MFM HD rec specs: interleave=%d, cylskew=%d, headskew=%d, wpcom=%d, rwc=%d\n",
				tag(), param.interleave, param.cylskew, param.headskew, param.write_precomp_cylinder, param.reduced_wcurr_cylinder);

		state = chdfile->read_metadata(MFM_HARD_DISK_METADATA_TAG, 1, metadata);
		if (state != CHDERR_NONE)
		{
			logerror("%s: Failed to read CHD track gap specs, applying defaults\n", tag());
		}
		else
		{
			sscanf(metadata.c_str(), MFMHD_GAP_METADATA_FORMAT, &param.gap1, &param.gap2, &param.gap3, &param.sync, &param.headerlen, &param.ecctype);
		}

		if (!param.sane_gap())
		{
			if (TRACE_CONFIG) logerror("%s: MFM HD gap specs have invalid values, applying defaults\n", tag());
			param.reset_gap();
		}
		else
			if (TRACE_CONFIG) logerror("%s: MFM HD gap specs: gap1=%d, gap2=%d, gap3=%d, sync=%d, headerlen=%d, ecctype=%d\n",
				tag(), param.gap1, param.gap2, param.gap3, param.sync, param.headerlen, param.ecctype);

		m_format->set_layout_params(param);

		m_cache->init(this, m_trackimage_size, m_cachelines);

		// Head timing
		// We assume that the real times are 80% of the max times
		// The single-step time includes the settle time, so does the max time
		// From that we calculate the actual cylinder-by-cylinder time and the settle time

		m_actual_cylinders = param.cylinders;

		if (m_phys_cylinders == 0) m_phys_cylinders = m_actual_cylinders+1;
		if (m_landing_zone == 0) m_landing_zone = m_phys_cylinders-1;

		float realnext = (m_seeknext_time==0)? 10 : (m_seeknext_time * 0.8);
		float realmax = (m_maxseek_time==0)? (m_actual_cylinders * 0.2) : (m_maxseek_time * 0.8);
		float settle_us = ((m_actual_cylinders-1.0) * realnext - realmax) / (m_actual_cylinders-2.0) * 1000;
		float step_us = realnext * 1000 - settle_us;
		if (TRACE_CONFIG) logerror("%s: Calculated settle time: %0.2f ms, step: %d us\n", tag(), settle_us/1000, (int)step_us);

		m_settle_time = attotime::from_usec((int)settle_us);
		m_step_time = attotime::from_usec((int)step_us);

		m_current_cylinder = m_landing_zone;
	}
	else
	{
		logerror("%s: Could not load CHD\n", tag());
	}
	return loaded;
}

const char *MFMHD_REC_METADATA_FORMAT = "IL:%d,CSKEW:%d,HSKEW:%d,WPCOM:%d,RWC:%d";
const char *MFMHD_GAP_METADATA_FORMAT = "GAP1:%d,GAP2:%d,GAP3:%d,SYNC:%d,HLEN:%d,ECC:%d";

void mfm_harddisk_device::call_unload()
{
	mfmhd_layout_params* params = m_format->get_current_params();
	mfmhd_layout_params* oldparams = m_format->get_initial_params();

	if (m_cache!=NULL)
	{
		m_cache->cleanup();

		if (m_format->save_param(MFMHD_IL) && !params->equals_rec(oldparams))
		{
			logerror("%s: MFM HD sector arrangement and recording specs have changed; updating CHD metadata\n", tag());
			chd_file* chdfile = get_chd_file();
			std::string metadata;

			strprintf(metadata, MFMHD_REC_METADATA_FORMAT, params->interleave, params->cylskew, params->headskew, params->write_precomp_cylinder, params->reduced_wcurr_cylinder);

			chd_error err = chdfile->write_metadata(MFM_HARD_DISK_METADATA_TAG, 0, metadata, 0);
			if (err != CHDERR_NONE)
			{
				logerror("%s: Failed to save MFM HD sector arrangement/recording specs to CHD\n", tag());
			}
		}

		if (m_format->save_param(MFMHD_GAP1) && !params->equals_gap(oldparams))
		{
			logerror("%s: MFM HD track gap specs have changed; updating CHD metadata\n", tag());
			chd_file* chdfile = get_chd_file();
			std::string metadata;

			strprintf(metadata, MFMHD_GAP_METADATA_FORMAT, params->gap1, params->gap2, params->gap3, params->sync, params->headerlen, params->ecctype);

			chd_error err = chdfile->write_metadata(MFM_HARD_DISK_METADATA_TAG, 1, metadata, 0);
			if (err != CHDERR_NONE)
			{
				logerror("%s: Failed to save MFM HD track gap specs to CHD\n", tag());
			}
		}
	}
	harddisk_image_device::call_unload();
}

void mfm_harddisk_device::setup_index_pulse_cb(index_pulse_cb cb)
{
	m_index_pulse_cb = cb;
}

void mfm_harddisk_device::setup_ready_cb(ready_cb cb)
{
	m_ready_cb = cb;
}

void mfm_harddisk_device::setup_seek_complete_cb(seek_complete_cb cb)
{
	m_seek_complete_cb = cb;
}

attotime mfm_harddisk_device::track_end_time()
{
	// We back up two microseconds before the track end to avoid the
	// index pulse to appear earlier (because of rounding effects)
	attotime nexttime = m_rev_time - attotime::from_nsec(2000);
	attotime endtime = attotime::never;

	if (!m_ready)
	{
		// estimate the next index time; during power-up we assume half the rotational speed
		// Should never be relevant, though, because READY is false.
		nexttime = nexttime * 2;
	}

	if (!m_revolution_start_time.is_never())
	{
		endtime = m_revolution_start_time + nexttime;
		if (TRACE_TIMING) logerror("%s: Track start time = %s, end time = %s\n", tag(), tts(m_revolution_start_time).c_str(), tts(endtime).c_str());
	}
	return endtime;
}

void mfm_harddisk_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case INDEX_TM:
		// Simple index hole handling. We assume that there is only a short pulse.
		m_revolution_start_time = machine().time();
		if (!m_index_pulse_cb.isnull())
		{
			m_index_pulse_cb(this, ASSERT_LINE);
			m_index_pulse_cb(this, CLEAR_LINE);
		}
		break;

	case SPINUP_TM:
		recalibrate();
		break;

	case CACHE_TM:
		m_cache->write_back_one();
		break;

	case SEEK_TM:
		switch (m_step_phase)
		{
		case STEP_COLLECT:
			// Collect timer has expired; start moving head
			head_move();
			break;
		case STEP_MOVING:
			// Head has reached final position
			// Check whether we have a new delta
			if (m_track_delta == 0)
			{
				// Start the settle timer
				m_step_phase = STEP_SETTLE;
				m_seek_timer->adjust(m_settle_time);
				if (TRACE_STEPS && TRACE_DETAIL) logerror("%s: Arrived at target cylinder %d, settling ...\n", tag(), m_current_cylinder);
			}
			else
			{
				// need to move the head again
				head_move();
			}
			break;
		case STEP_SETTLE:
			// Do we have new step pulses?
			if (m_track_delta != 0) head_move();
			else
			{
				// Seek completed
				if (!m_recalibrated)
				{
					m_ready = true;
					m_recalibrated = true;
					if (TRACE_STATE) logerror("%s: Spinup complete, drive recalibrated and positioned at cylinder %d; drive is READY\n", tag(), m_current_cylinder);
					if (!m_ready_cb.isnull()) m_ready_cb(this, ASSERT_LINE);
				}
				else
				{
					if (TRACE_SIGNALS) logerror("%s: Settling done at cylinder %d, seek complete\n", tag(), m_current_cylinder);
				}
				m_seek_complete = true;
				if (!m_seek_complete_cb.isnull()) m_seek_complete_cb(this, ASSERT_LINE);
				m_step_phase = STEP_COLLECT;
			}
			break;
		}
	}
}

void mfm_harddisk_device::recalibrate()
{
	if (TRACE_STEPS) logerror("%s: Recalibrate to track 0\n", tag());
	direction_in_w(CLEAR_LINE);
	while (-m_track_delta  < m_phys_cylinders)
	{
		step_w(ASSERT_LINE);
		step_w(CLEAR_LINE);
	}
}

void mfm_harddisk_device::head_move()
{
	int steps = m_track_delta;
	if (steps < 0) steps = -steps;
	if (TRACE_STEPS) logerror("%s: Moving head by %d step(s) %s\n", tag(), steps, (m_track_delta<0)? "outward" : "inward");

	// We simulate the head movement by pausing for n*step_time with n being the cylinder delta
	m_step_phase = STEP_MOVING;
	m_seek_timer->adjust(m_step_time * steps);

	if (TRACE_TIMING) logerror("%s: Head movement takes %s time\n", tag(), tts(m_step_time * steps).c_str());
	// We pretend that we already arrived
	// TODO: Check auto truncation?
	m_current_cylinder += m_track_delta;
	if (m_current_cylinder < 0) m_current_cylinder = 0;
	if (m_current_cylinder >= m_actual_cylinders) m_current_cylinder = m_actual_cylinders-1;
	m_track_delta = 0;
}

void mfm_harddisk_device::direction_in_w(line_state line)
{
	m_seek_inward = (line == ASSERT_LINE);
	if (TRACE_STEPS && TRACE_DETAIL) logerror("%s: Setting seek direction %s\n", tag(), m_seek_inward? "inward" : "outward");
}

/*
    According to the specs [1]:

    "4.3.1 BUFFERED SEEK: To minimize access time, pulses may be issued at an
    accelerated rate and buffered in a counter. Initiation of a seek starts
    immediately after the first pulse is received. Head motion occurs during
    pulse accumulation, and the seek is completed following receipt of all pulses."

    "8.1.3 SEEKING: Upon receiving a Step pulse, the MPU (microprocessor unit)
    pauses for 250 usec to allow for additional pulses before executing the seek
    operation. Every incoming pulse resets the 250 usec timer. The seek will
    not begin until the last pulse is received."

    WTF? Oh come on, Seagate, be consistent at least in a single document.

    ================================

    Step behaviour:
    During all waiting times, further step_w invocations increase the counter

    - Leading edge increments the counter c and sets the timer to 250us (mode=collect)
    - When the timer expires (mode=collect):
   (1)- Calculate the stepping time: time = c*200us; save the counter
      - Start the timer (mode=move)
      - When the timer expires (mode=move)
        - Add the track delta to the current track position
        - Subtract the delta from the current counter
        - When the counter is not zero (pulses arrived in the meantime), go to (1)
        - When the counter is zero, set the timer to 16.8 ms (mode=settle)
        - When the timer expires (mode=settle)
          - When the counter is not zero, go to (1)
          - When the counter is zero, signal seek_complete; done
*/

void mfm_harddisk_device::step_w(line_state line)
{
	// Leading edge
	if (line == ASSERT_LINE && m_step_line == CLEAR_LINE)
	{
		if (m_seek_complete)
		{
			m_step_phase = STEP_COLLECT;
			m_seek_complete = false;
			if (!m_seek_complete_cb.isnull()) m_seek_complete_cb(this, CLEAR_LINE);
		}

		// Counter will be adjusted according to the direction (+-1)
		m_track_delta += (m_seek_inward)? +1 : -1;
		if (TRACE_STEPS && TRACE_DETAIL) logerror("%s: Got seek pulse; track delta %d\n", tag(), m_track_delta);
		if (m_track_delta < -m_phys_cylinders || m_track_delta > m_phys_cylinders)
		{
			if (TRACE_STEPS) logerror("%s: Excessive step pulses - doing auto-truncation\n", tag());
			m_autotruncation = true;
		}
		m_seek_timer->adjust(attotime::from_usec(250));  // Start step collect timer
	}
	m_step_line = line;
}

/*
    Find the position of the cell.
    Returns true when the current time exceeds the limit.
    Returns the position as an index into the track array and the bit number.
*/
bool mfm_harddisk_device::find_position(attotime &from_when, const attotime &limit, int &bytepos, int &bit)
{
	// Frequency
	UINT32 freq = 1000000000/m_cell_size;

	// As we stop some few cells early each track, we adjust our position
	// to the track start
	if (from_when < m_revolution_start_time) from_when = m_revolution_start_time;

	// Calculate the position in the track, given the from_when time and the revolution_start_time.
	int cell = (from_when - m_revolution_start_time).as_ticks(freq);

	from_when += attotime::from_nsec((m_encoding==MFM_BITS)? m_cell_size : (16*m_cell_size));
	if (from_when > limit) return true;

	bytepos = cell / 16;

	// Reached the end
	if (bytepos >= m_trackimage_size)
	{
		if (TRACE_TIMING) logerror("%s: Reached end: rev_start = %s, live = %s\n", tag(), tts(m_revolution_start_time).c_str(), tts(from_when).c_str());
		m_revolution_start_time += m_rev_time;
		cell = (from_when - m_revolution_start_time).as_ticks(freq);
		bytepos = cell / 16;
	}

	if (bytepos < 0)
	{
		if (TRACE_TIMING) logerror("%s: Negative cell number: rev_start = %s, live = %s\n", tag(), tts(m_revolution_start_time).c_str(), tts(from_when).c_str());
		bytepos = 0;
	}
	bit = cell % 16;

	return false;
}

/*
    Reading bytes from the hard disk.

    Returns true if the time limit will be exceeded before reading the bit or complete byte.
    Otherwise returns the bit at the given position, or the complete data byte with the clock byte.
*/
bool mfm_harddisk_device::read(attotime &from_when, const attotime &limit, UINT16 &cdata)
{
	UINT16* track = m_cache->get_trackimage(m_current_cylinder, m_current_head);

	if (track==NULL)
	{
		// What shall we do in this case?
		throw emu_fatalerror("Cannot read CHD image");
	}

	// Get a copy for later debug output
	attotime fw = from_when;

	int bytepos = 0;
	int bitpos = 0;

	bool offlimit = find_position(from_when, limit, bytepos, bitpos);
	if (offlimit) return true;

	if (m_encoding == MFM_BITS)
	{
		// We will deliver a single bit
		cdata = ((track[bytepos] << bitpos) & 0x8000) >> 15;
		if (TRACE_BITS) logerror("%s: Reading (c=%d,h=%d,bit=%d) at cell %d [%s] = %d\n", tag(), m_current_cylinder, m_current_head, bitpos, ((bytepos<<4) + bitpos), tts(fw).c_str(), cdata);
	}
	else
	{
		// We will deliver a whole byte
		if (TRACE_READ) logerror("%s: Reading (c=%d,h=%d) at position %d\n", tag(), m_current_cylinder, m_current_head, bytepos);
		cdata = track[bytepos];
	}
	return false;
}

/*
    Writing bytes to the hard disk.

    Returns true if the time limit will be exceeded before writing the bit or complete byte.
*/
bool mfm_harddisk_device::write(attotime &from_when, const attotime &limit, UINT16 cdata, bool wpcom, bool reduced_wc)
{
	UINT16* track = m_cache->get_trackimage(m_current_cylinder, m_current_head);

	if (track==NULL)
	{
		// What shall we do in this case?
		throw emu_fatalerror("Cannot read CHD image");
	}

	int bytepos = 0;
	int bitpos = 0;

	bool offlimit = find_position(from_when, limit, bytepos, bitpos);
	if (offlimit) return true;

	m_cache->mark_current_as_dirty();

	if (m_encoding == MFM_BITS)
	{
		// We will write a single bit
		if (cdata != 0) track[bytepos] |= (0x8000 >> bitpos);
		else track[bytepos] &= ~(0x8000 >> bitpos);
		bitpos++;
	}
	else
	{
		// We will write a whole byte
		track[bytepos] = cdata;
	}

	// Update our cylinders for reduced write current and write precompensation.
	// We assume that write precompensation and reduced write current occur
	// at some cylinder and continue up to the innermost cylinder.
	mfmhd_layout_params* params = m_format->get_current_params();

	if (reduced_wc && (params->reduced_wcurr_cylinder == -1 || m_current_cylinder < params->reduced_wcurr_cylinder))
		params->reduced_wcurr_cylinder = m_current_cylinder;

	if (wpcom && (params->write_precomp_cylinder == -1 || m_current_cylinder < params->write_precomp_cylinder))
		params->write_precomp_cylinder = m_current_cylinder;

	if (TRACE_WRITE) if ((bitpos&0x0f)==0) logerror("%s: Wrote data=%04x (c=%d,h=%d) at position %04x, wpcom=%d, rwc=%d\n", tag(), track[bytepos], m_current_cylinder, m_current_head, bytepos, wpcom, reduced_wc);
	return false;
}

chd_error mfm_harddisk_device::load_track(UINT16* data, int cylinder, int head)
{
	chd_error state = m_format->load(m_chd, data, m_trackimage_size, cylinder, head);
	return state;
}

void mfm_harddisk_device::write_track(UINT16* data, int cylinder, int head)
{
	m_format->save(m_chd, data, m_trackimage_size, cylinder, head);
}

int mfm_harddisk_device::get_actual_heads()
{
	return m_format->get_current_params()->heads;
}

/*
    The generic HD takes any kind of CHD HD image and magically creates enough heads and cylinders.
*/
mfm_hd_generic_device::mfm_hd_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: mfm_harddisk_device(mconfig, MFMHD_GENERIC, "Generic MFM hard disk", tag, owner, clock, "mfm_harddisk", __FILE__)
{
}

const device_type MFMHD_GENERIC = &device_creator<mfm_hd_generic_device>;

/*
    Various models.
*/
mfm_hd_st213_device::mfm_hd_st213_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: mfm_harddisk_device(mconfig, MFMHD_ST213, "Seagate ST-213 MFM hard disk", tag, owner, clock, "mfm_hd_st213", __FILE__)
{
	m_phys_cylinders = 670;
	m_max_cylinders = 615;      // 0..614
	m_landing_zone = 620;
	m_max_heads = 2;
	m_seeknext_time = 20;       // time for one step including settle time
	m_maxseek_time = 150;
}

const device_type MFMHD_ST213 = &device_creator<mfm_hd_st213_device>;

mfm_hd_st225_device::mfm_hd_st225_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: mfm_harddisk_device(mconfig, MFMHD_ST225, "Seagate ST-225 MFM hard disk", tag, owner, clock, "mfm_hd_st225", __FILE__)
{
	m_phys_cylinders = 670;
	m_max_cylinders = 615;
	m_landing_zone = 620;
	m_max_heads = 4;
	m_seeknext_time = 20;
	m_maxseek_time = 150;
}

const device_type MFMHD_ST225 = &device_creator<mfm_hd_st225_device>;

mfm_hd_st251_device::mfm_hd_st251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: mfm_harddisk_device(mconfig, MFMHD_ST251, "Seagate ST-251 MFM hard disk", tag, owner, clock, "mfm_hd_st251", __FILE__)
{
	m_phys_cylinders = 821;
	m_max_cylinders = 820;
	m_landing_zone = 820;
	m_max_heads = 6;
	m_seeknext_time = 8;
	m_maxseek_time = 70;
}

const device_type MFMHD_ST251 = &device_creator<mfm_hd_st251_device>;

// ===========================================================
//   Track cache
//   The cache holds track images to be read by the controller.
//   This is a write-back LRU cache.
// ===========================================================

mfmhd_trackimage_cache::mfmhd_trackimage_cache():
	m_tracks(NULL)
{
}

mfmhd_trackimage_cache::~mfmhd_trackimage_cache()
{
	mfmhd_trackimage* current = m_tracks;
	if (TRACE_CACHE) logerror("%s: MFM HD cache destroy\n", m_mfmhd->tag());

	while (current != NULL)
	{
		global_free_array(current->encdata);
		mfmhd_trackimage* currenttmp = current->next;
		global_free(current);
		current = currenttmp;
	}
}

void mfmhd_trackimage_cache::write_back_one()
{
	mfmhd_trackimage* current = m_tracks;

	while (current != NULL)
	{
		if (current->dirty)
		{
			m_mfmhd->write_track(current->encdata, current->cylinder, current->head);
			current->dirty = false;
			break;
		}
		mfmhd_trackimage* currenttmp = current->next;
		current = currenttmp;
	}
}

void mfmhd_trackimage_cache::cleanup()
{
	mfmhd_trackimage* current = m_tracks;
	if (TRACE_CACHE) logerror("%s: MFM HD cache cleanup\n", m_mfmhd->tag());

	// Still dirty?
	while (current != NULL)
	{
		if (TRACE_CACHE) logerror("%s: MFM HD cache: evict line cylinder=%d head=%d\n", m_mfmhd->tag(), current->cylinder, current->head);
		if (current->dirty)
		{
			m_mfmhd->write_track(current->encdata, current->cylinder, current->head);
			current->dirty = false;
		}
		mfmhd_trackimage* currenttmp = current->next;
		current = currenttmp;
	}
}

/*
    Marks the recently loaded track as dirty. As every writing operations
    is preceded by a lookup, writing will always be done on the first track in the list.
*/
void mfmhd_trackimage_cache::mark_current_as_dirty()
{
	m_tracks->dirty = true;
}

const char *encnames[] = { "MFM_BITS","MFM_BYTE","SEPARATE","SSIMPLE " };

/*
    Initialize the cache by loading the first <trackslots> tracks.
*/
void mfmhd_trackimage_cache::init(mfm_harddisk_device* mfmhd, int tracksize, int trackslots)
{
	if (TRACE_CACHE) logerror("%s: MFM HD cache init; cache size is %d tracks\n", mfmhd->tag(), trackslots);

	chd_error state = CHDERR_NONE;

	mfmhd_trackimage* previous = NULL;
	mfmhd_trackimage* current = NULL;

	m_mfmhd = mfmhd;

	// Load some tracks into the cache
	int track = 0;
	int head = 0;
	int cylinder = 0;

	while (track < trackslots)
	{
		if (TRACE_CACHE && TRACE_DETAIL) logerror("%s: MFM HD allocate cache slot\n", mfmhd->tag());
		previous = current;
		current = global_alloc(mfmhd_trackimage);
		current->encdata = global_alloc_array(UINT16, tracksize);

		// Load the first tracks into the slots
		state = m_mfmhd->load_track(current->encdata, cylinder, head);
		if (state != CHDERR_NONE) throw emu_fatalerror("Cannot load (c=%d,h=%d) from hard disk", cylinder, head);

		current->dirty = false;
		current->cylinder = cylinder;
		current->head = head;

		// We will read all heads per cylinder first, then go to the next cylinder.
		if (++head >= mfmhd->get_actual_heads())
		{
			head = 0;
			cylinder++;
		}
		current->next = NULL;

		if (previous != NULL)
			previous->next = current;
		else
			// Head
			m_tracks = current;

		// Count the number of loaded tracks
		track++;
	}
}

/*
    Delivers the track image.
    First look up the track image in the cache. If not present, load it from
    the CHD, convert it, and evict the least recently used line.
    The searched track will be the first in m_tracks.
*/
UINT16* mfmhd_trackimage_cache::get_trackimage(int cylinder, int head)
{
	// Search the cached track images
	mfmhd_trackimage* current = m_tracks;
	mfmhd_trackimage* previous = NULL;

	chd_error state = CHDERR_NONE;

	// Repeat the search. This loop should run at most twice; once for a direct hit,
	// and twice on miss, then the second iteration will be a hit.
	while (state == CHDERR_NONE)
	{
		// A simple linear search
		while (current != NULL)
		{
			if (current->cylinder == cylinder && current->head == head)
			{
				// found it
				// move it to the front of the chain for LRU
				if (previous != NULL)
				{
					previous->next = current->next;  // pull out here
					current->next = m_tracks;  // put the previous head into the next field
					m_tracks = current;        // set this line as new head
				}
				return current->encdata;
			}
			else
			{
				// Don't lose the pointer to the next tail
				if (current->next != NULL) previous = current;
				current = current->next;
			}
		}
		// If we are here, we have a cache miss.
		// Evict the last line
		// Load the new line into that line
		// Then look it up again, which will move it to the front

		// previous points to the second to last element
		current = previous->next;
		if (TRACE_CACHE) logerror("%s: MFM HD cache: evict line (c=%d,h=%d)\n", m_mfmhd->tag(), current->cylinder, current->head);

		if (current->dirty)
		{
			m_mfmhd->write_track(current->encdata, current->cylinder, current->head);
			current->dirty = false;
		}

		state = m_mfmhd->load_track(current->encdata, cylinder, head);

		current->dirty = false;
		current->cylinder = cylinder;
		current->head = head;
	}
	// If we are here we have a CHD error.
	return NULL;
}

// ================================================================

mfm_harddisk_connector::mfm_harddisk_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock):
	device_t(mconfig, MFM_HD_CONNECTOR, "MFM hard disk connector", tag, owner, clock, "mfm_hd_connector", __FILE__),
	device_slot_interface(mconfig, *this)
{
}

mfm_harddisk_connector::~mfm_harddisk_connector()
{
}

mfm_harddisk_device* mfm_harddisk_connector::get_device()
{
	return dynamic_cast<mfm_harddisk_device *>(get_card_device());
}

void mfm_harddisk_connector::configure(mfmhd_enc_t encoding, int spinupms, int cache, const mfmhd_format_type format)
{
	m_encoding = encoding;
	m_spinupms = spinupms;
	m_cachesize = cache;
	m_format = format();
}

void mfm_harddisk_connector::device_config_complete()
{
	mfm_harddisk_device *dev = get_device();
	if (dev != NULL)
	{
		dev->set_encoding(m_encoding);
		dev->set_spinup_time(m_spinupms);
		dev->set_cache_size(m_cachesize);
		dev->set_format(m_format);
	}
}

const device_type MFM_HD_CONNECTOR = &device_creator<mfm_harddisk_connector>;

// ================================================================

void mfmhd_image_format_t::set_layout_params(mfmhd_layout_params param)
{
	m_param = m_param_old = param;
	m_secnumber[0] = m_secnumber[1] = m_secnumber[2] = -1;
}

void mfmhd_image_format_t::mfm_encode(UINT16* trackimage, int& position, UINT8 byte, int count)
{
	mfm_encode_mask(trackimage, position, byte, count, 0x00);
}

void mfmhd_image_format_t::mfm_encode_a1(UINT16* trackimage, int& position)
{
	m_current_crc = 0xffff;
	mfm_encode_mask(trackimage, position, 0xa1, 1, 0x04);
	// 0x443b; CRC for A1
}

void mfmhd_image_format_t::mfm_encode_mask(UINT16* trackimage, int& position, UINT8 byte, int count, int mask)
{
	UINT16 encclock = 0;
	UINT16 encdata = 0;
	UINT8 thisbyte = byte;
	bool mark = (mask != 0x00);

	m_current_crc = ccitt_crc16_one(m_current_crc, byte);

	for (int i=0; i < 8; i++)
	{
		encdata <<= 1;
		encclock <<= 1;

		if (m_param.encoding == MFM_BITS || m_param.encoding == MFM_BYTE)
		{
			// skip one position for later interleaving
			encdata <<= 1;
			encclock <<= 1;
		}

		if (thisbyte & 0x80)
		{
			// Encoding 1 => 01
			encdata |= 1;
			m_lastbit = true;
		}
		else
		{
			// Encoding 0 => x0
			// If the bit in the mask is set, suppress the clock bit
			// Also, if we use the simplified encoding, don't set the clock bits
			if (m_lastbit == false && m_param.encoding != SEPARATED_SIMPLE && (mask & 0x80) == 0) encclock |= 1;
			m_lastbit = false;
		}
		mask <<= 1;
		// For simplified encoding, set all clock bits to indicate a mark
		if (m_param.encoding == SEPARATED_SIMPLE && mark) encclock |= 1;
		thisbyte <<= 1;
	}

	if (m_param.encoding == MFM_BITS || m_param.encoding == MFM_BYTE)
		encclock <<= 1;
	else
		encclock <<= 8;

	trackimage[position++] = (encclock | encdata);

	// When we write the byte multiple times, check whether the next encoding
	// differs from the previous because of the last bit

	if (m_param.encoding == MFM_BITS || m_param.encoding == MFM_BYTE)
	{
		encclock &= 0x7fff;
		if ((byte & 0x80)==0 && m_lastbit==false) encclock |= 0x8000;
	}

	for (int j=1; j < count; j++)
	{
		trackimage[position++] = (encclock | encdata);
		m_current_crc = ccitt_crc16_one(m_current_crc, byte);
	}
}

UINT8 mfmhd_image_format_t::mfm_decode(UINT16 raw)
{
	unsigned int value = 0;

	for (int i=0; i < 8; i++)
	{
		value <<= 1;

		value |= (raw & 0x4000);
		raw <<= 2;
	}
	return (value >> 14) & 0xff;
}

/*
    For debugging. Outputs the byte array in a xxd-like way.
*/
void mfmhd_image_format_t::showtrack(UINT16* enctrack, int length)
{
	for (int i=0; i < length; i+=16)
	{
		logerror("%07x: ", i);
		for (int j=0; j < 16; j++)
		{
			logerror("%04x ", enctrack[i+j]);
		}
		logerror(" ");
		logerror("\n");
	}
}

// ======================================================================
//    General MFM HD format
// ======================================================================

	// According to MDM5 formatting:
	// gap0=16 gap1=16 gap2=3 gap3=22 sync=13 count=32 size=2

	// HFDC manual: When using the hard disk format, the values for GAP0 and GAP1 must
	// both be set to the same number and loaded in the appropriate registers.


/*
    Concerning pre-comp and reduced wc:
    The MDM5 formatter allows for specifying the cylinders for precompensation and reduced write current
    Default: red_wc = precomp_cyl = bottom(cylinders / 21) * 16

    Cylinder = 0 ... precomp_cyl-1: Format command = 0x62, Write command = 0xc0
    Cylinder = precomp_cyl ... end: Format command = 0x62, Write command = 0xca

    (format command unaffected; could be a bug)

    C0: no current reduction, disable EARLY/LATE
    CA: reduced write current, enable EARLY/LATE
*/

const mfmhd_format_type MFMHD_GEN_FORMAT = &mfmhd_image_format_creator<gen_mfmhd_format>;

enum
{
	SEARCH_A1=0,
	FOUND_A1,
	DAM_FOUND,
	CHECK_CRC
};

/*
    Calculate the ident byte from the cylinder. The specification does not
    define idents beyond cylinder 1023, but formatting programs seem to
    continue with 0xfd for cylinders between 1024 and 2047.
*/
UINT8 gen_mfmhd_format::cylinder_to_ident(int cylinder)
{
	if (cylinder < 256) return 0xfe;
	if (cylinder < 512) return 0xff;
	if (cylinder < 768) return 0xfc;
	return 0xfd;
}

/*
    Returns the linear sector number, given the CHS data.

      C,H,S
    | 0,0,0 | 0,0,1 | 0,0,2 | ...
    | 0,1,0 | 0,1,1 | 0,1,2 | ...
    ...
    | 1,0,0 | ...
    ...
*/
int gen_mfmhd_format::chs_to_lba(int cylinder, int head, int sector)
{
	if ((cylinder < m_param.cylinders) && (head < m_param.heads) && (sector < m_param.sectors_per_track))
	{
		return (cylinder * m_param.heads + head) * m_param.sectors_per_track + sector;
	}
	else return -1;
}

chd_error gen_mfmhd_format::load(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head)
{
	chd_error state = CHDERR_NONE;
	UINT8 sector_content[1024];     // TODO: should be prepared for max 16K

	int sectorcount = m_param.sectors_per_track;
	int size = m_param.sector_size;
	int position = 0; // will be incremented by each encode call
	int sec_number = 0;
	int identfield = 0;
	int cylfield = 0;
	int headfield = 0;
	int sizefield = (size >> 7)-1;

	// If we don't have interleave data in the CHD, take a default
	if (m_param.interleave==0)
	{
		m_param.interleave = get_default(MFMHD_IL);
		m_param.cylskew = get_default(MFMHD_CSKEW);
		m_param.headskew = get_default(MFMHD_HSKEW);
	}

	int sec_il_start = (m_param.cylskew * cylinder + m_param.headskew * head) % sectorcount;
	int delta = (sectorcount + m_param.interleave-1) / m_param.interleave;

	if (TRACE_RWTRACK) logerror("gen_mfmhd_format: MFM HD cache: load track (c=%d,h=%d) from CHD, interleave=%d, cylskew=%d, headskew=%d\n", cylinder, head, m_param.interleave, m_param.cylskew, m_param.headskew);

	m_lastbit = false;

	if (m_param.sync==0)
	{
		m_param.gap1 = get_default(MFMHD_GAP1);
		m_param.gap2 = get_default(MFMHD_GAP2);
		m_param.gap3 = get_default(MFMHD_GAP3);
		m_param.sync = get_default(MFMHD_SYNC);
		m_param.headerlen = get_default(MFMHD_HLEN);
		m_param.ecctype = get_default(MFMHD_ECC);
	}

	// Gap 1
	mfm_encode(trackimage, position, 0x4e, m_param.gap1);

	if (TRACE_LAYOUT) logerror("gen_mfmhd_format: cyl=%d head=%d: sector sequence = ", cylinder, head);

	sec_number = sec_il_start;
	for (int sector = 0; sector < sectorcount; sector++)
	{
		if (TRACE_LAYOUT) logerror("%02d ", sec_number);

		// Sync gap
		mfm_encode(trackimage, position, 0x00, m_param.sync);

		// Write IDAM
		mfm_encode_a1(trackimage, position);

		// Write header
		identfield = cylinder_to_ident(cylinder);
		cylfield = cylinder & 0xff;
		headfield = head & 0x0f;
		if (m_param.headerlen==5)
			headfield |= ((cylinder & 0x700)>>4);

		mfm_encode(trackimage, position, identfield);
		mfm_encode(trackimage, position, cylfield);
		mfm_encode(trackimage, position, headfield);
		mfm_encode(trackimage, position, sec_number);
		if (m_param.headerlen==5)
			mfm_encode(trackimage, position, sizefield);

		// Write CRC for header.
		int crc = m_current_crc;
		mfm_encode(trackimage, position, (crc >> 8) & 0xff);
		mfm_encode(trackimage, position, crc & 0xff);

		// Gap 2
		mfm_encode(trackimage, position, 0x4e, m_param.gap2);

		// Sync
		mfm_encode(trackimage, position, 0x00, m_param.sync);

		// Write DAM
		mfm_encode_a1(trackimage, position);
		mfm_encode(trackimage, position, 0xfb);

		// Get sector content from CHD
		int lbaposition = chs_to_lba(cylinder, head, sec_number);
		if (lbaposition>=0)
		{
			chd_error state = chdfile->read_units(lbaposition, sector_content);
			if (state != CHDERR_NONE) break;
		}
		else
		{
			logerror("gen_mfmhd_format: Invalid CHS data (%d,%d,%d); not loading from CHD\n", cylinder, head, sector);
		}

		for (int i=0; i < size; i++)
			mfm_encode(trackimage, position, sector_content[i]);

		// Write CRC for content.
		crc = m_current_crc;
		mfm_encode(trackimage, position, (crc >> 8) & 0xff);
		mfm_encode(trackimage, position, crc & 0xff);

		// Gap 3
		mfm_encode(trackimage, position, 0x00, 3);
		mfm_encode(trackimage, position, 0x4e, m_param.gap3-3);

		// Calculate next sector number
		sec_number += delta;
		if (sec_number >= sectorcount)
		{
			sec_il_start = (sec_il_start+1) % delta;
			sec_number = sec_il_start;
		}
	}
	if (TRACE_LAYOUT) logerror("\n");

	// Gap 4
	if (state == CHDERR_NONE)
	{
		// Fill the rest with 0x4e
		mfm_encode(trackimage, position, 0x4e, tracksize-position);
		if (TRACE_IMAGE) showtrack(trackimage, tracksize);
	}
	return state;
}

chd_error gen_mfmhd_format::save(chd_file* chdfile, UINT16* trackimage, int tracksize, int current_cylinder, int current_head)
{
	if (TRACE_CACHE) logerror("gen_mfmhd_format: write back (c=%d,h=%d) to CHD\n", current_cylinder, current_head);

	UINT8 buffer[1024]; // for header or sector content

	int bytepos = 0;
	int state = SEARCH_A1;
	int count = 0;
	int pos = 0;
	UINT16 crc = 0;
	UINT8 byte;
	bool search_header = true;

	int ident = 0;
	int cylinder = 0;
	int head = 0;
	int sector = 0;
	int size = 0;

	int headerpos = 0;

	int interleave = 0;
	int interleave_prec = -1;
	bool check_interleave = true;
	bool check_skew = true;

	int gap1 = 0;
	int ecctype = 0;

	// if (current_cylinder==0 && current_head==0) showtrack(trackimage, tracksize);

	// If we want to detect gaps, we only do it on cylinder 0, head 0
	// This makes it safer to detect the header length
	// (There is indeed some chance that we falsely assume a header length of 4
	// because the two bytes behind happen to be a valid CRC value)
	if (save_param(MFMHD_GAP1) && current_cylinder==0 && current_head==0)
	{
		m_param.gap1 = 0;
		m_param.gap2 = 0;
		m_param.gap3 = 0;
		m_param.sync = 0;
		// 4-byte headers are used for the IBM-AT format
		// 5-byte headers are used in other formats
		m_param.headerlen = 4;
		m_param.ecctype = 0;
	}

	// AT format implies 512 bytes per sector
	int sector_length = 512;

	// Only check once
	bool countgap1 = (m_param.gap1==0);
	bool countgap2 = false;
	bool countgap3 = false;
	bool countsync = false;

	chd_error chdstate = CHDERR_NONE;

	if (TRACE_IMAGE)
	{
		for (int i=0; i < tracksize; i++)
		{
			if ((i % 16)==0) logerror("\n%04x: ", i);
			logerror("%02x ", (m_param.encoding==MFM_BITS || m_param.encoding==MFM_BYTE)? mfm_decode(trackimage[i]) : (trackimage[i]&0xff));
		}
		logerror("\n");
	}

	// We have to go through the bytes of the track and save a sector as soon as one shows up

	while (bytepos < tracksize)
	{
		// Decode the next 16 bits
		if (m_param.encoding==MFM_BITS || m_param.encoding==MFM_BYTE)
		{
			byte = mfm_decode(trackimage[bytepos]);
		}
		else byte = (trackimage[bytepos] & 0xff);

		switch (state)
		{
		case SEARCH_A1:
			// Counting gaps and sync
			if (countgap2)
			{
				if (byte == 0x4e) m_param.gap2++;
				else if (byte == 0) { countsync = true; countgap2 = false; }
			}

			if (countsync)
			{
				if (byte == 0) m_param.sync++;
				else countsync = false;
			}

			if (countgap3)
			{
				if (byte != 0x00 || m_param.gap3 < 4) m_param.gap3++;
				else countgap3 = false;
			}

			if (((m_param.encoding==MFM_BITS || m_param.encoding==MFM_BYTE) && trackimage[bytepos]==0x4489)
				|| (m_param.encoding==SEPARATED && trackimage[bytepos]==0x0aa1)
				|| (m_param.encoding==SEPARATED_SIMPLE && trackimage[bytepos]==0xffa1))
			{
				state = FOUND_A1;
				count = (search_header? m_param.headerlen : (sector_length+1)) + 2;
				crc = 0x443b; // init value with a1
				pos = 0;
			}
			bytepos++;
			break;

		case FOUND_A1:
			crc = ccitt_crc16_one(crc, byte);
			// logerror("%s: MFM HD: Byte = %02x, CRC=%04x\n", tag(), byte, crc);

			// Put byte into buffer
			// but not the data mark and the CRC
			if (search_header || (count > 2 &&  count < sector_length+3)) buffer[pos++] = byte;

			// Stop counting gap1
			if (search_header && countgap1)
			{
				gap1 = bytepos-1;
				countgap1 = false;
			}

			if (--count == 0)
			{
				if (crc==0)
				{
					if (search_header)
					{
						// Found a header
						ident = buffer[0];
						cylinder = buffer[1];
						// For non-PC-AT formats, highest three bits are in the head field
						if (m_param.headerlen == 5) cylinder |= ((buffer[2]&0x70)<<4);
						else
						{
							logerror("gen_mfmhd_format: Unexpected header size: %d, cylinder=%d, position=%04x\n", m_param.headerlen, cylinder, bytepos);
							showtrack(trackimage, tracksize);
						}

						head = buffer[2] & 0x0f;
						sector = buffer[3];
						int identexp = cylinder_to_ident(cylinder);

						if (identexp != ident)
						{
							logerror("gen_mfmhd_format: Field error; ident = %02x (expected %02x) for sector chs=(%d,%d,%d)\n", ident, identexp, cylinder, head, sector);
						}

						if (cylinder != current_cylinder)
						{
							logerror("gen_mfmhd_format: Sector header of sector %d defines cylinder = %02x (should be %02x)\n", sector, cylinder, current_cylinder);
						}

						if (head != current_head)
						{
							logerror("gen_mfmhd_format: Sector header of sector %d defines head = %02x (should be %02x)\n", sector, head, current_head);
						}

						// Check skew
						// We compare the beginning of this track with the track on the next head and the track on the next cylinder
						if (check_skew && cylinder < 2 && head < 2)
						{
							m_secnumber[cylinder*2 + head] = sector;
							check_skew=false;
						}

						// Count the sectors for the interleave
						if (check_interleave)
						{
							if (interleave_prec == -1) interleave_prec = sector;
							else
							{
								if (sector == interleave_prec+1) check_interleave = false;
								interleave++;
							}
						}

						if (interleave == 0) interleave = sector - buffer[3];

						// When we have 4-byte headers, the sector length is 512 bytes
						if (m_param.headerlen == 5)
						{
							size = buffer[4];
							sector_length = 128 << (size&0x07);
							ecctype = (size&0xf0)>>4;
						}

						search_header = false;
						if (TRACE_DETAIL) logerror("gen_mfmhd_format: Found sector chs=(%d,%d,%d)\n", cylinder, head, sector);
						headerpos = pos;
						// Start the GAP2 counter (if not already determined)
						if (m_param.gap2==0) countgap2 = true;
					}
					else
					{
						// Sector contents
						// Write the sectors to the CHD
						int lbaposition = chs_to_lba(cylinder, head, sector);
						if (lbaposition>=0)
						{
							if (TRACE_DETAIL) logerror("gen_mfmhd_format: Writing sector chs=(%d,%d,%d) to CHD\n", current_cylinder, current_head, sector);
							chdstate = chdfile->write_units(chs_to_lba(current_cylinder, current_head, sector), buffer);

							if (chdstate != CHDERR_NONE)
							{
								logerror("gen_mfmhd_format: Write error while writing sector chs=(%d,%d,%d)\n", cylinder, head, sector);
							}
						}
						else
						{
							logerror("gen_mfmhd_format: Invalid CHS data in track image: (%d,%d,%d); not saving to CHD\n", cylinder, head, sector);
						}
						if (m_param.gap3==0) countgap3 = true;
						search_header = true;
					}
				}
				else
				{
					// Let's test for a 5-byte header
					if (search_header && m_param.headerlen==4 && current_cylinder==0 && current_head==0)
					{
						if (TRACE_DETAIL) logerror("gen_mfmhd_format: CRC error for 4-byte header; trying 5 bytes\n");
						m_param.headerlen=5;
						count = 1;
						bytepos++;
						break;
					}
					else
					{
						logerror("gen_mfmhd_format: CRC error in %s of (%d,%d,%d)\n", search_header? "header" : "data", cylinder, head, sector);
						search_header = true;
					}
				}
				// search next A1
				state = SEARCH_A1;

				if (!search_header && (pos - headerpos) > 30)
				{
					logerror("gen_mfmhd_format: Error; missing DAM; searching next header\n");
					search_header = true;
				}
			}
			bytepos++;
			break;
		}
	}

	if (check_interleave == false && save_param(MFMHD_IL))
	{
		// Successfully determined the interleave
		m_param.interleave = interleave;
	}

	if (check_skew == false)
	{
		if (m_secnumber[0] != -1)
		{
			if (m_secnumber[1] != -1)
			{
				if (save_param(MFMHD_HSKEW)) m_param.headskew = m_secnumber[1]-m_secnumber[0];
				if (TRACE_FORMAT) logerror("gen_mfmhd_format: Determined head skew = %d\n", m_param.headskew);
			}
			if (m_secnumber[2] != -1)
			{
				if (save_param(MFMHD_CSKEW)) m_param.cylskew = m_secnumber[2]-m_secnumber[0];
				if (TRACE_FORMAT) logerror("gen_mfmhd_format: Determined cylinder skew = %d\n", m_param.cylskew);
			}
		}
	}

	gap1 -= m_param.sync;
	ecctype = -1;   // lock to CRC until we have a support for ECC

	if (current_cylinder==0 && current_head==0)
	{
		if (save_param(MFMHD_GAP1)) m_param.gap1 = gap1;
		if (save_param(MFMHD_ECC)) m_param.ecctype = ecctype;
	}
	return chdstate;
}

int gen_mfmhd_format::get_default(mfmhd_param_t type)
{
	switch (type)
	{
	case MFMHD_IL: return 4;
	case MFMHD_HSKEW:
	case MFMHD_CSKEW: return 0;
	case MFMHD_WPCOM:
	case MFMHD_RWC: return -1;
	case MFMHD_GAP1: return 16;
	case MFMHD_GAP2: return 3;
	case MFMHD_GAP3: return 18;
	case MFMHD_SYNC: return 13;
	case MFMHD_HLEN: return 5;
	case MFMHD_ECC: return -1;
	}
	return -1;
}
