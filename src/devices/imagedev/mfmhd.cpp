// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*************************************************************************

    MFM Hard disk emulation
    -----------------------

    This is a low-level emulation of a hard disk drive. Unlike high-level
    emulations which just deliver the data bytes, this implementation
    considers all bytes on a track, including gaps, CRC, interleave, and more.

    The actual data are stored on a CHD file, however, only as a sequence
    of sector contents. The other metadata (like gap information, interleave)
    are stored as metadata information in the CHD.

    To provide the desired low-level emulation grade, the tracks must be
    reconstructed from the sector contents in the CHD. This is done in
    the call_load method.

    Usually, more than one sector of a track is read, so when a track is
    reconstructed, the track image is retained for later accesses.

    This implementation also features a LRU cache for the track images,
    implemented by the class mfmhd_trackimage_cache, also contained in this
    source file. The LRU cache stores the most recently accessed track images.
    When lines must be evicted, they are stored back into the CHD file. This
    is done in the call_unload method. Beside the sector contents, call_unload
    also saves track layout metadata which have been detected during usage.

    The architecture can be imagined like this:

    [host system] ---- [controller] --- [harddisk] --- [track cache]
                                             |
                                         [format] ---- [CHD]

    Encodings
    ---------
    The goal of this implementation is to provide an emulation that is very
    close to the original, similar to the grade achieved for the floppy
    emulation. This means that the track image does not contain a byte
    sequence but a sequence of MFM cell values. Unlike the floppy emulation,
    we do not define the cells by time intervals but simply by a sequence of
    bits which represent the MFM cell contents; this is also due to the fact
    that the cell rate is more than 10 times higher than with floppy media.

    There are four options for encodings which differ by overhead and
    emulation precision.

    - MFM_BITS: MFM cells are transferred bit by bit for reading and writing
    - MFM_BYTE: MFM cells are transferred in clusters of 16 cells, thus
                encoding a full data byte (with 8 clock bits interleaved)
    - SEPARATED: 16 bits are transferred in one go, with the 8 clock bits in the
                first byte, and the 8 data bits in the second byte
    - SEPARATED_SIMPLE: Similar to SEPARATED, but instead of the clock bits,
                0x00 is used for normal data, and 0xff is used for marks.

    Following the specification of MFM, the data byte 0x67 is encoded as follows:

    MFM_BITS / MFM_BYTE: 1001010010010101 = 0x9495
    SEPARATED:           10001000 01100111 = 0x8867
    SEPARATED_SIMPLE:    00000000 01100111 = 0x0067

    The ID Address Mark (0xA1) is encoded this way:

    MFM_BITS / MFM_BYTE: 0100010010001001 = 0x4489
    SEPARATED:           00001010 10100001 = 0x0AA1
    SEPARATED_SIMPLE:    11111111 10100001 = 0xFFA1

    If the CPU load by the emulation is already very high, the
    SEPARATED(_SIMPLE) options are recommended. The MFM_BITS option is closest
    to the real processing, but causes a high load. MFM_BYTE is a good
    compromise between speed and precision.


    Drive definition
    ----------------
    Hard disk drives are defined by subclassing mfm_harddisk_device, as can
    be seen for the offered Seagate drive implementations.

    The following parameters must be set in the constructor of the subclass:

    - Number of physical cylinders. These can be more than the number of
      cylinders that are used for data. Some drives have cylinders near the
      spindle that are used as park positions.

    - Number of cylinders used for data. This is the number of the highest
      cylinder plus 1 (counting from 0).

    - Landing zone: Cylinder number where the head is parked. Should be higher
      than the number of usable cylinders.

    - Heads: Number of heads.

    - Time for one cylinder seek step in milliseconds: This is the time the
      drive heads need to step one cylinder inwards or outwards. This time
      includes the settling time.

    - Maximum seek time in milliseconds: This is the time the drive needs to
      seek from cylinder 0 to the maximum cylinder. This time includes the
      settling time and is typically far less than the one cylinder time
      multiplied by the number of cylinders, because the settling time only
      occurs once. These delay values are calculated in call_load.

    If the number of physical cylinders is set to 0, the cylinder and head
    count in taken from the metadata of the mounted CHD file. This allows for
    using all kinds of CHD images that can be handled by the controller,
    without having to define a proper drive for them.

    The predefined drives are

    ST-213: Seagate hard disk drive, 10 MB capacity
    ST-225: Seagate hard disk drive, 20 MB capacity
    ST-251: Seagate hard disk drive, 40 MB capacity

    generic: Hard disk with 0 physical cylinders, which can be used for
             all CHDs that can be handled by the controller.

    The ST-xxx drives require to mount a CHD that exactly matches their
    geometry.


    Track image cache
    -----------------
    Since the reconstruction of the track takes some time, and we don't want
    to create unnecessary effort, track images (sector contents plus all
    preambles and gaps encoded as selected) are kept in a cache. Whenever a
    track shall be read, the cache is consulted first to retrieve a copy.
    If no recent copy is available, the track is loaded from the CHD, set up
    by the format implementation (see lib/formats/mfm_hd.c). The least
    recently used track is evicted from the cache and written back to the CHD
    (also by means of the format implementation).

    When the emulation is stopped, all cache lines are evicted and written back.
    If the emulation is killed before, cache contents may possibly not be
    written back, so changes may be lost. To alleviate this issue, the cache
    writes back one line every 5 seconds so that changes are automatically
    committed after some time.

    This cache is not related to caches on real hard drives. It is a pure
    emulation artifact, intended to keep conversion efforts as low as possible.


    Interface
    ---------
    There are three outgoing lines, used as callbacks to the controller:
    - READY: asserted when the drive has completed its spinup.
    - INDEX: asserted when the index hole passes by. Unlike the floppy
             implementation, this hard disk implementation produces a
             zero-length pulse (assert/clear). This must be considered for
             the controller emulation.
    - SEEK COMPLETE: asserted when the read/write heads have settled over the
             target cylinder. This line is important for controller that want
             to employ buffered steps.

    There are two data transfer methods:

    - read(attotime &from_when, const attotime &limit, UINT16 &data)

      Delivers the MFM cells at the given point in time. The cells are returned
      in the data parameter. The behavior depends on the chosen encoding:

      MFM_BITS: data contains 0x0000 or 0x0001
      MFM_BYTE: data contains a set of 16 consecutive cells at the given time
      SEPARATED: data contains the clock bits in the MSB, the data bits in the LSB
      SEPARATED_SIMPLE: data contains 0x00 or 0xFF in the MSB (normal or mark)
                        and the data bits in the LSB.

      When the limit is exceeded, the method returns true, otherwise false.

    - write(attotime &from_when, const attotime &limit, UINT16 cdata, bool wpcom=false, bool reduced_wc=false)

      Writes the MFM cells at the given point in time. cdata contains the
      cells according to the encoding (see above). The controller also has
      to set wpcom to true to indicate write precompensation, and reduced_wc
      to true to indicate a reduced write current; otherwise, these settings
      are assumed to be false. The wpcom and rwc settings do not affect the
      recording of the data bytes in this emulation, but the drive will store
      the cylinder with the lowest number where wpcom (or rwc) occured and
      store this in the CHD.

    These methods are used to move and select the heads:

    - step_w(line_state line)
    - direction_in_w(line_state line)
    - headsel_w(int head)

    Some status lines:

    - ready_r
    - seek_complete_r
    - trk00_r

    These reflect the values that are also passed by the callback routines
    listed above and can be used for a polling scheme. Track00 is not available
    as a callback. It indicates whether track 0 has been reached.


    Configuration
    -------------
    For a working example please refer to emu/bus/ti99_peb/hfdc.c.

    According to the MAME/MESS concept of slot devices, the settings are
    passed over the slot to the slot device, in this case, the hard disk drive.

    This means that when we add a slot (the connector), we also have to
    pass the desired parameters for the drive.

    MCFG_MFM_HARDDISK_CONN_ADD(_tag, _slot_intf, _def_slot, _enc, _spinupms, _cache, _format)

    Specific parameters:

    _enc: Select an encoding from the values as listed above.
    _spinupms: Number of milliseconds until the drive announces READY. Many
          drives like the included Seagate drives require a pretty long
          powerup time (10-20 seconds). In some computer systems, the
          user is therefore asked to turn on the drive first so that
          on first access by the system, the drive may have completed
          its powerup. In MAME/MESS we cannot turn on components earlier,
          thus we do not define the spinup time inside the drive
          implementation but at this point.
    _cache: Number of tracks to be stored in the LRU track cache
    _format: Format to be used for the drive. Must be a subclass of
          mfmhd_image_format_t, for example, mfmhd_generic_format. The format
          is specified by its format creator identifier (e.g. MFMHD_GEN_FORMAT).

    Metadata
    --------
    We have three sets of metadata information. The first one is the
    declaration of cylinders, heads, sectors, and sector size. It is stored
    by the tag GDDD in the CHD.
    The second is the declaration of interleave, skew, write precompensation,
    and reduced write current.

       Write precompensation is a modification of the timing used for the inner
       cylinders. Although write precompensation (wpcom for short) can be applied
       at every write operation, it is usually only used starting from some
       cylinder, going towards the spindle, applied to the whole tracks.
       The value defined here is the first cylinder where wpcom is applied.
       Reduced write current (rwc) is a modification of the electrical current
       used for writing data. The value defined here is the first cylinder where
       rwc is applied.

       Both wpcom and rwc have an effect on the physical device, but this is not
       emulated. For that reason we store the information as additional metadata
       inside the CHD. It is not relevant for the functionality of the emulated
       hard disk. The write operations

       When wpcom or rwc are not used, their value is defined to be -1.

    Interleave affects the order how sectors are arranged on the track; skew
    is the number of sectors that the sector sequence is shifted on the
    next cylinder (cylinder skew) or head (head skew). These values are used
    to compensate for the delay that occurs when the read/write heads are moved
    from one cylinder to the next, or switched from one head to the next.

    These parameters are stored by the tag GDDI on the CHD.

    The third set refers to the specification of gaps and sync fields on the
    track. These values may change only on first use (when undefined) or when
    the hard disk is reformatted with a different controller or driver. These
    parameters are also stored by the GDDI tag as a second record.


    Michael Zapf
    August 2015

    References:
    [1] ST225 OEM Manual, Seagate

**************************************************************************/

#include "emu.h"
#include "harddisk.h"
#include "mfmhd.h"

#define TRACE_STEPS 0
#define TRACE_SIGNALS 0
#define TRACE_READ 0
#define TRACE_WRITE 0
#define TRACE_CACHE 0
#define TRACE_BITS 0
#define TRACE_DETAIL 0
#define TRACE_TIMING 0
#define TRACE_STATE 1
#define TRACE_CONFIG 1

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

std::string mfm_harddisk_device::tts(const attotime &t)
{
	char buf[256];
	int nsec = t.attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%4d.%03d,%03d,%03d", int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

mfm_harddisk_device::mfm_harddisk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: harddisk_image_device(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_card_interface(mconfig, *this), 
		m_index_timer(NULL),
		m_spinup_timer(NULL),
		m_seek_timer(NULL),
		m_cache_timer(NULL),
		m_precomp_cyl(0), 
		m_redwc_cyl(0), 
		m_encoding(), 
		m_ready(false), 
		m_current_cylinder(0), 
		m_current_head(0), 
		m_track_delta(0), 
		m_step_phase(0), 
		m_seek_complete(false), 
		m_seek_inward(false), 
		m_autotruncation(false), 
		m_recalibrated(false), 
		m_step_line(), 
		m_format(NULL)
{
	m_spinupms = 10000;
	m_cachelines = 5;
	m_max_cylinders = 0;
	m_phys_cylinders = 0;   // We will get this value for generic drives from the image
	m_max_heads = 0;
	m_cell_size = 100;
	m_rpm = 3600;           // MFM drives have a revolution rate of 3600 rpm (i.e. 60/sec)
	m_trackimage_size = (int)((60000000000ULL / (m_rpm * m_cell_size)) / 16 + 1);
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

	m_cache = global_alloc(mfmhd_trackimage_cache(machine()));

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

	std::string devtag(tag());
	devtag += ":format";

	m_format->set_tag(devtag);

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

mfmhd_trackimage_cache::mfmhd_trackimage_cache(running_machine &machine): 
	m_mfmhd(NULL),
	m_tracks(NULL),
	m_machine(machine)
{
}

mfmhd_trackimage_cache::~mfmhd_trackimage_cache()
{
	mfmhd_trackimage* current = m_tracks;
	if (TRACE_CACHE) m_machine.logerror("%s: MFM HD cache destroy\n", m_mfmhd->tag());

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
	if (TRACE_CACHE) m_machine.logerror("%s: MFM HD cache cleanup\n", m_mfmhd->tag());

	// Still dirty?
	while (current != NULL)
	{
		if (TRACE_CACHE) m_machine.logerror("%s: MFM HD cache: evict line cylinder=%d head=%d\n", m_mfmhd->tag(), current->cylinder, current->head);
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
	if (TRACE_CACHE) m_machine.logerror("%s: MFM HD cache init; cache size is %d tracks\n", mfmhd->tag(), trackslots);

	chd_error state;

	mfmhd_trackimage* previous;
	mfmhd_trackimage* current = NULL;

	m_mfmhd = mfmhd;

	// Load some tracks into the cache
	int track = 0;
	int head = 0;
	int cylinder = 0;

	while (track < trackslots)
	{
		if (TRACE_CACHE && TRACE_DETAIL) m_machine.logerror("%s: MFM HD allocate cache slot\n", mfmhd->tag());
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
		if (TRACE_CACHE) m_machine.logerror("%s: MFM HD cache: evict line (c=%d,h=%d)\n", m_mfmhd->tag(), current->cylinder, current->head);

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
	device_slot_interface(mconfig, *this), 
	m_encoding(), 
	m_spinupms(0), 
	m_cachesize(0),
	m_format(NULL)
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
