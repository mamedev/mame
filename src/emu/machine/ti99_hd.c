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

#include "emu.h"
#include "formats/imageutl.h"
#include "harddisk.h"

#include "ti99_hd.h"

#define TI99HD_BLOCKNOTFOUND -1

#define LOG logerror
#define VERBOSE 0

#define GAP1 16
#define GAP2 8
#define GAP3 15
#define GAP4 340
#define SYNC 13

enum
{
	INDEX_TM = 0,
	SPINUP_TM,
	SEEK_TM
};

enum
{
	STEP_COLLECT = 0,
	STEP_MOVING,
	STEP_SETTLE
};

#define TRACKSLOTS 5
#define TRACKIMAGE_SIZE 10416       // Provide the buffer for a complete track, including preambles and gaps

mfm_harddisk_device::mfm_harddisk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: harddisk_image_device(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_card_interface(mconfig, *this)
{
}

mfm_harddisk_device::~mfm_harddisk_device()
{
}

void mfm_harddisk_device::device_start()
{
	m_index_timer = timer_alloc(INDEX_TM);
	m_spinup_timer = timer_alloc(SPINUP_TM);
	m_seek_timer = timer_alloc(SEEK_TM);
	m_spinup_time = attotime::from_msec(8000);
	m_rev_time = attotime::from_hz(60);

	// MFM drives have a revolution rate of 3600 rpm (i.e. 60/sec)
	m_index_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));

	// Spinup may take up to 24 seconds
	m_spinup_timer->adjust(m_spinup_time);

	m_current_cylinder = 10; // for test purpose

	m_cache = global_alloc(mfmhd_trackimage_cache);
}

void mfm_harddisk_device::device_reset()
{
	m_autotruncation = false;
	m_ready = false;
	m_seek_complete = true;
	m_seek_inward = false;
	m_track_delta = 0;
	m_step_line = CLEAR_LINE;
}

void mfm_harddisk_device::device_stop()
{
	global_free(m_cache);
}

bool mfm_harddisk_device::call_load()
{
	logerror("call_load\n");
	bool loaded = harddisk_image_device::call_load();
	if (loaded==IMAGE_INIT_PASS)
	{
		m_cache->init(get_chd_file(), tag(), TRACKSLOTS, m_encoding);
	}
	else
	{
		logerror("Could not load CHD\n");
	}
	return loaded;
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
	if (!m_ready)
	{
		// estimate the next index time; during power-up we assume half the rotational speed
		// Should never be relevant, though, because READY is false.
		nexttime = nexttime * 2;
	}
	return (m_revolution_start_time.is_never())? attotime::never : (m_revolution_start_time + nexttime);
}

void mfm_harddisk_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case INDEX_TM:
		/* Simple index hole handling. We assume that there is only a short pulse. */
		if (!m_index_pulse_cb.isnull())
		{
			m_revolution_start_time = machine().time();
			m_index_pulse_cb(this, ASSERT_LINE);
			m_index_pulse_cb(this, CLEAR_LINE);
		}
		break;
	case SPINUP_TM:
		m_ready = true;
		logerror("%s: Spinup complete, drive is ready\n", tag());
		if (!m_ready_cb.isnull()) m_ready_cb(this, ASSERT_LINE);
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
				m_seek_timer->adjust(attotime::from_usec(16800));
				logerror("%s: Arrived at target track %d, settling ...\n", tag(), m_current_cylinder);
			}
			break;
		case STEP_SETTLE:
			// Do we have new step pulses?
			if (m_track_delta != 0) head_move();
			else
			{
				// Seek completed
				logerror("%s: Settling done at cylinder %d, seek complete\n", tag(), m_current_cylinder);
				m_seek_complete = true;
				if (!m_seek_complete_cb.isnull()) m_seek_complete_cb(this, ASSERT_LINE);
				m_step_phase = STEP_COLLECT;
			}
			break;
		}
	}
}

void mfm_harddisk_device::head_move()
{
	int steps = m_track_delta;
	if (steps < 0) steps = -steps;
	logerror("%s: Moving head by %d step(s) %s\n", tag(), steps, (m_track_delta<0)? "outward" : "inward");

	int disttime = steps*200;
	m_step_phase = STEP_MOVING;
	m_seek_timer->adjust(attotime::from_usec(disttime));
	// We pretend that we already arrived
	// TODO: Check auto truncation?
	m_current_cylinder += m_track_delta;
	if (m_current_cylinder < 0) m_current_cylinder = 0;
	if (m_current_cylinder > 670) m_current_cylinder = 670;
	m_track_delta = 0;
}

void mfm_harddisk_device::direction_in_w(line_state line)
{
	m_seek_inward = (line == ASSERT_LINE);
	logerror("%s: Setting seek direction %s\n", tag(), m_seek_inward? "inward" : "outward");
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

      Step timing:
        per track = 20 ms max, full seek: 150 ms max (615 tracks); both including settling time
        We assume t(1) = 17; t(615)=140
        t(i) = s+d*i
        s=(615*t(1)-t(615))/614
        d=t(1)-s
        s=16800 us, d=200 us
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
		logerror("%s: Got seek pulse; track delta %d\n", tag(), m_track_delta);
		if (m_track_delta < -670 || m_track_delta > 670)
		{
			logerror("%s: Excessive step pulses - doing auto-truncation\n", tag());
			m_autotruncation = true;
		}
		m_seek_timer->adjust(attotime::from_usec(250));
	}
	m_step_line = line;
}

/*
    Reading bytes from the hard disk.
    This is the byte-level access to the hard disk. We deliver the next data byte
    together with the clock byte.

    Returns true if the time limit will be exceeded before reading the complete byte.
    Otherwise returns the byte at the given position together with the clock byte.
*/
bool mfm_harddisk_device::read(attotime &from_when, const attotime &limit, UINT16 &cdata)
{
	UINT16* track = m_cache->get_trackimage(m_current_cylinder, m_current_head);

	if (track==NULL)
	{
		// What shall we do in this case?
		throw emu_fatalerror("Cannot read CHD image");
	}

	// Calculate the position in the track, given the from_when time and the revolution_start_time.
	// Each cell takes 100 ns (10 MHz)

	int cell = (from_when - m_revolution_start_time).as_ticks(10000000);

	from_when += attotime::from_nsec(1600);
	if (from_when > limit) return true;

	int position = cell / 16;

	// Reached the end
	if (position >= 10416)
	{
		m_revolution_start_time += m_rev_time;
		cell = (from_when - m_revolution_start_time).as_ticks(10000000);
		position = cell / 16;
	}

	// TODO: fix the actual issue
	if (position < 0) position = 0;

	logerror("%s: Reading track %d head %d at position %d\n", tag(), m_current_cylinder, m_current_head, position);
	cdata = track[position];

	return false;
}

mfm_hd_generic_device::mfm_hd_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: mfm_harddisk_device(mconfig, MFM_HD_GENERIC, "Generic MFM hard disk (byte level)", tag, owner, clock, "mfm_harddisk", __FILE__)
{
}

const device_type MFM_HD_GENERIC = &device_creator<mfm_hd_generic_device>;

// ===========================================================
//   Track cache
//   The cache holds track images to be read by the controller.
//   This is a write-back LRU cache.
// ===========================================================

mfmhd_trackimage_cache::mfmhd_trackimage_cache():
	m_tracks(NULL)
{
	m_current_crc = 0;
}

mfmhd_trackimage_cache::~mfmhd_trackimage_cache()
{
	mfmhd_trackimage* current = m_tracks;
	logerror("%s: MFM HD cache destroy\n", tag());

	// Still dirty?
	while (current != NULL)
	{
		logerror("%s: MFM HD cache: evict line cylinder=%d head=%d\n", tag(), current->cylinder, current->head);
		if (current->dirty) write_back(current);
		global_free_array(current->encdata);
		mfmhd_trackimage* currenttmp = current->next;
		global_free(current);
		current = currenttmp;
	}
}

/*
    Initialize the cache by loading the first <trackslots> tracks.
*/
void mfmhd_trackimage_cache::init(chd_file* chdfile, const char* dtag, int trackslots, mfmhd_enc_t encoding)
{
	m_encoding = encoding;
	m_tagdev = dtag;

	logerror("%s: MFM HD cache init; using encoding %d\n", m_tagdev, encoding);
	chd_error state = CHDERR_NONE;
	mfmhd_trackimage* previous = NULL;
	mfmhd_trackimage* current = NULL;
	std::string metadata;

	m_chd = chdfile;

	if (chdfile==NULL)
	{
		logerror("%s: chdfile is null\n", tag());
		return;
	}

	// Read the hard disk metadata
	state = chdfile->read_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
	if (state != CHDERR_NONE)
	{
		throw emu_fatalerror("Failed to read CHD metadata");
	}

	// Parse the metadata
	if (sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &m_cylinders, &m_heads, &m_sectors_per_track, &m_sectorsize) != 4)
	{
		throw emu_fatalerror("Invalid metadata");
	}

	// Load some tracks into the cache
	for (int i=0; i < trackslots; i++)
	{
		logerror("%s: MFM HD allocate cache slot\n", tag());
		previous = current;
		current = global_alloc(mfmhd_trackimage);
		current->encdata = global_alloc_array(UINT16, TRACKIMAGE_SIZE);

		// Load the first tracks into the slots
		state = load_track(current, i, 0, 32, 256, 4);
		current->next = NULL;

		if (state != CHDERR_NONE) throw emu_fatalerror("Cannot load cylinder %d head %d from hard disk", i, 0);

		if (previous != NULL)
			previous->next = current;
		else
			// Head
			m_tracks = current;
	}

	current = m_tracks;

	while (current != NULL)
	{
		logerror("%s: MFM HD cache: containing line cylinder=%d head=%d\n", tag(), current->cylinder, current->head);
		mfmhd_trackimage* currenttmp = current->next;
		current = currenttmp;
	}
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
int mfmhd_trackimage_cache::chs_to_lba(int cylinder, int head, int sector)
{
	if ((cylinder < m_cylinders) && (head < m_heads) && (sector < m_sectors_per_track))
	{
		return (cylinder * m_heads + head) * m_sectors_per_track + sector;
	}
	else return -1;
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
		logerror("%s: MFM HD cache: evict line cylinder=%d head=%d\n", tag(), current->cylinder, current->head);

		if (current->dirty) write_back(current);
		state = load_track(current, cylinder, head, 32, 256, 4);
	}
	// If we are here we have a CHD error.
	return NULL;
}

/*
    Create MFM encoding.
*/
void mfmhd_trackimage_cache::mfm_encode(mfmhd_trackimage* slot, int& position, UINT8 byte, int count)
{
	mfm_encode_mask(slot, position, byte, count, 0x00);
}

void mfmhd_trackimage_cache::mfm_encode_a1(mfmhd_trackimage* slot, int& position)
{
	m_current_crc = 0xffff;
	mfm_encode_mask(slot, position, 0xa1, 1, 0x04);
	// 0x443b; CRC for A1
}

void mfmhd_trackimage_cache::mfm_encode_mask(mfmhd_trackimage* slot, int& position, UINT8 byte, int count, int mask)
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

		if (m_encoding == MFM_BITS || m_encoding == MFM_BYTE)
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
			if (m_lastbit == false && m_encoding != SEPARATED_SIMPLE && (mask & 0x80) == 0) encclock |= 1;
			m_lastbit = false;
		}
		mask <<= 1;
		// For simplified encoding, set all clock bits to indicate a mark
		if (m_encoding == SEPARATED_SIMPLE && mark) encclock |= 1;
		thisbyte <<= 1;
	}

	if (m_encoding == MFM_BITS || m_encoding == MFM_BYTE)
		encclock <<= 1;
	else
		encclock <<= 8;

	slot->encdata[position++] = (encclock | encdata);

	// When we write the byte multiple times, check whether the next encoding
	// differs from the previous because of the last bit

	if (m_encoding == MFM_BITS || m_encoding == MFM_BYTE)
	{
		encclock &= 0x7fff;
		if ((byte & 0x80)==0 && m_lastbit==false) encclock |= 0x8000;
	}

	for (int j=1; j < count; j++)
	{
		slot->encdata[position++] = (encclock | encdata);
		m_current_crc = ccitt_crc16_one(m_current_crc, byte);
	}
}

/*
    Load a track from the CHD.
*/
chd_error mfmhd_trackimage_cache::load_track(mfmhd_trackimage* slot, int cylinder, int head, int sectorcount, int size, int interleave)
{
	chd_error state = CHDERR_NONE;

	UINT8 sector_content[1024];

	logerror("%s: MFM HD cache: load cylinder=%d head=%d from CHD\n", tag(), cylinder, head);

	m_lastbit = false;
	int position = 0; // will be incremented by each encode call

	// Gap 1
	mfm_encode(slot, position, 0x4e, 16);

	int sec_il_start = 0;
	int sec_number = 0;

	// Round up
	int delta = (sectorcount + interleave-1) / interleave;

	logerror("%02x %02x: ", cylinder&0xff, head&0xff);
	for (int sector = 0; sector < sectorcount; sector++)
	{
		logerror("%02d ", sec_number);
		// Sync gap
		mfm_encode(slot, position, 0x00, 13);

		// Write IDAM
		mfm_encode_a1(slot, position);
		mfm_encode(slot, position, 0xfe);           // ID field?

		// Write header
		mfm_encode(slot, position, cylinder & 0xff);
		mfm_encode(slot, position, head & 0xff);
		mfm_encode(slot, position, sec_number);
		mfm_encode(slot, position, (size >> 7)-1);

		// Write CRC for header.
		int crc = m_current_crc;
		mfm_encode(slot, position, (crc >> 8) & 0xff);
		mfm_encode(slot, position, crc & 0xff);

		// Gap 2
		mfm_encode(slot, position, 0x4e, 3);

		// Sync
		mfm_encode(slot, position, 0x00, 13);

		// Write DAM
		mfm_encode_a1(slot, position);
		mfm_encode(slot, position, 0xfb);

		// Get sector content from CHD
		chd_error state = m_chd->read_units(chs_to_lba(cylinder, head, sec_number), sector_content);
		if (state != CHDERR_NONE)
			break;

		for (int i=0; i < size; i++)
			mfm_encode(slot, position, sector_content[i]);

		// Write CRC for content.
		crc = m_current_crc;
		mfm_encode(slot, position, (crc >> 8) & 0xff);
		mfm_encode(slot, position, crc & 0xff);

		// Gap 3
		mfm_encode(slot, position, 0x00, 3);
		mfm_encode(slot, position, 0x4e, 19);

		// Calculate next sector number
		sec_number += delta;
		if (sec_number >= sectorcount) sec_number = ++sec_il_start;
	}

	// Gap 4
	if (state == CHDERR_NONE)
	{
		// Fill the rest with 0x4e
		mfm_encode(slot, position, 0x4e, TRACKIMAGE_SIZE-position);
		logerror("\n");
		showtrack(slot->encdata, TRACKIMAGE_SIZE);
	}

	slot->dirty = false;
	slot->cylinder = cylinder;
	slot->head = head;

	return state;
}

/*
    TODO: Maybe use a scheduled write-back in addition to the eviction.
*/
void mfmhd_trackimage_cache::write_back(mfmhd_trackimage* slot)
{
	logerror("%s: MFM HD cache: write back cylinder=%d head=%d to CHD\n", tag(), slot->cylinder, slot->head);
	slot->dirty = false;
}

/*
    For debugging. Outputs the byte array in a xxd-like way.
*/
void mfmhd_trackimage_cache::showtrack(UINT16* enctrack, int length)
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

void mfm_harddisk_connector::device_config_complete()
{
	mfm_harddisk_device *dev = get_device();
	if (dev != NULL)
		dev->set_encoding(m_encoding);
}

const device_type MFM_HD_CONNECTOR = &device_creator<mfm_harddisk_connector>;

// ================================================================

// ===========================================================================
// Legacy implementation
// ===========================================================================

#include "smc92x4.h"

mfm_harddisk_legacy_device::mfm_harddisk_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, TI99_MFMHD_LEG, "MFM Harddisk LEGACY", tag, owner, clock, "mfm_harddisk_leg", __FILE__)
{
}

/*
    Calculate the ident byte from the cylinder. The specification does not
    define idents beyond cylinder 1023, but formatting programs seem to
    continue with 0xfd for cylinders between 1024 and 2047.
*/
UINT8 mfm_harddisk_legacy_device::cylinder_to_ident(int cylinder)
{
	if (cylinder < 256) return 0xfe;
	if (cylinder < 512) return 0xff;
	if (cylinder < 768) return 0xfc;
	if (cylinder < 1024) return 0xfd;
	return 0xfd;
}

/*
    Returns the linear sector number, given the CHS data.
*/
bool mfm_harddisk_legacy_device::harddisk_chs_to_lba(hard_disk_file *hdfile, int cylinder, int head, int sector, UINT32 *lba)
{
	const hard_disk_info *info;

	if ( hdfile != NULL)
	{
		info = hard_disk_get_info(hdfile);

		if (    (cylinder >= info->cylinders) ||
				(head >= info->heads) ||
				(sector >= info->sectors))
		return false;

		*lba = (cylinder * info->heads + head)
				* info->sectors
				+ sector;
		return true;
	}
	return false;
}

/* Accessor functions */
void mfm_harddisk_legacy_device::read_sector(int cylinder, int head, int sector, UINT8 *buf)
{
	UINT32 lba;
	if (VERBOSE>5) LOG("ti99_hd: read_sector(%d, %d, %d)\n", cylinder, head, sector);
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (cylinder != m_current_cylinder)
	{
		return;
	}

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	if (!harddisk_chs_to_lba(file, cylinder, head, sector, &lba))
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	if (!hard_disk_read(file, lba, buf))
	{
		m_status &= ~MFMHD_READY;
		return;
	}
	/* printf("ti99_hd read sector  c=%04d h=%02d s=%02d\n", cylinder, head, sector); */
	m_status |= MFMHD_READY;
}

void mfm_harddisk_legacy_device::write_sector(int cylinder, int head, int sector, UINT8 *buf)
{
	UINT32 lba;
	if (VERBOSE>5) LOG("ti99_hd: write_sector(%d, %d, %d)\n", cylinder, head, sector);
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	if (!harddisk_chs_to_lba(file, cylinder, head, sector, &lba))
	{
		m_status &= ~MFMHD_READY;
		m_status |= MFMHD_WRFAULT;
		return;
	}

	if (!hard_disk_write(file, lba, buf))
	{
		m_status &= ~MFMHD_READY;
		m_status |= MFMHD_WRFAULT;
		return;
	}

	m_status |= MFMHD_READY;
}

/*
    Searches a block containing number * byte, starting at the given
    position. Returns the position of the first byte of the block.
*/
int mfm_harddisk_legacy_device::find_block(const UINT8 *buffer, int start, int stop, UINT8 byte, size_t number)
{
	int i = start;
	size_t current = number;
	while (i < stop && current > 0)
	{
		if (buffer[i++] != byte)
		{
			current = number;
		}
		else
		{
			current--;
		}
	}
	if (current==0)
	{
		return i - number;
	}
	else
		return TI99HD_BLOCKNOTFOUND;
}

int mfm_harddisk_legacy_device::get_track_length()
{
	int count;
	int size;
	hard_disk_file *file = m_drive->get_hard_disk_file();
	const hard_disk_info *info;

	if (file==NULL) return 0;
	info = hard_disk_get_info(file);

	count = info->sectors;
	size = info->sectorbytes/128;
	return GAP1 + count*(SYNC+12+GAP2+SYNC+size*128+GAP3)+GAP4;
}

/*
    Reads a complete track. We have to rebuild the gaps. This is basically
    done in the same way as in the SDF format in ti99_dsk.

    WARNING: This function is untested! We need to create a suitable
    application program for the TI which makes use of it.
*/
void mfm_harddisk_legacy_device::read_track(int head, UINT8 *trackdata)
{
	/* We assume an interleave of 3 for 32 sectors. */
	int step = 3;
	UINT32 lba;
	int i;

	int size;
	int position = 0;
	int sector;
	int crc;
	int count;

	const hard_disk_info *info;
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	info = hard_disk_get_info(file);

	count = info->sectors;
	size = info->sectorbytes/128;

	/* Write lead-in. */
	memset(trackdata, 0x4e, GAP1);

	position += GAP1;
	for (i=0; i < count; i++)
	{
		sector = (i*step) % info->sectors;
		memset(&trackdata[position], 0x00, SYNC);
		position += SYNC;

		/* Write sync */
		trackdata[position++] = 0xa1;

		/* Write IDAM */
		trackdata[position++] = cylinder_to_ident(m_current_cylinder);
		trackdata[position++] = m_current_cylinder;
		trackdata[position++] = head;
		trackdata[position++] = sector;
		trackdata[position++] = (size==1)? 0x00 : (0x01 << (size-1));

		/* Set CRC16 */
		crc = ccitt_crc16(0xffff, &trackdata[position-5], 5);
		trackdata[position++] = (crc>>8)&0xff;
		trackdata[position++] = crc & 0xff;

		/* Write Gap2 */
		memset(&trackdata[position], 0x4e, GAP2);
		position += GAP2;

		/* Write sync */
		memset(&trackdata[position], 0x00, SYNC);
		position += SYNC;
		trackdata[position++] = 0xa1;

		/* Write DAM */
		trackdata[position++] = 0xfb;

		/* Locate the sector content in the image and load it. */

		if (!harddisk_chs_to_lba(file, m_current_cylinder, head, sector, &lba))
		{
			m_status &= ~MFMHD_READY;
			return;
		}

		if (!hard_disk_read(file, lba, &trackdata[position]))
		{
			m_status &= ~MFMHD_READY;
			return;
		}

		position += info->sectorbytes;

		/* Set CRC16. Includes the address mark. */
		crc = ccitt_crc16(0xffff, &trackdata[position-size-1], size+1);
		trackdata[position++] = (crc>>8)&0xff;
		trackdata[position++] = crc & 0xff;

		/* Write remaining 3 bytes which would have been used for ECC. */
		memset(&trackdata[position], 0x00, 3);
		position += 3;

		/* Write Gap3 */
		memset(&trackdata[position], 0x4e, GAP3);
		position += GAP3;
	}
	/* Write Gap 4 */
	memset(&trackdata[position], 0x4e, GAP4);
	position += GAP4;

	m_status |= MFMHD_READY;
}

/*
    Writes a track to the image. We need to isolate the sector contents.
    This is basically done in the same way as in the SDF format in ti99_dsk.
*/
void mfm_harddisk_legacy_device::write_track(int head, UINT8 *track_image, int data_count)
{
	int current_pos = 0;
	bool found;
	// UINT8 wident;
	UINT8 whead = 0, wsector = 0;
	// UINT8 wsize;
	UINT16 wcyl = 0;
	int state;

	/* Only search in the first 100 bytes for the start. */

	UINT32 lba;
	hard_disk_file *file = m_drive->get_hard_disk_file();

	/* printf("ti99_hd write track c=%d h=%d\n", m_current_cylinder, head); */

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	current_pos = find_block(track_image, 0, 100, 0x4e, GAP1);

	// In case of defect formats, we continue as far as possible. This
	// may lead to sectors not being written. */
	if (current_pos==TI99HD_BLOCKNOTFOUND)
	{
		logerror("ti99_hd error: write track: Cannot find GAP1 for cylinder %d, head %d.\n", m_current_cylinder, head);
		/* What now? The track data are illegal, so we refuse to continue. */
		m_status |= MFMHD_WRFAULT;
		return;
	}

	/* Get behind GAP1 */
	current_pos += GAP1;
	found = false;

	while (current_pos < data_count)
	{
		/* We must find the address block to determine the sector. */
		int new_pos = find_block(track_image, current_pos, data_count, 0x00, SYNC);
		if (new_pos==TI99HD_BLOCKNOTFOUND)
		{
			/* Forget about the rest; we're done. */
			if (found) break;  /* we were already successful, so all ok */
			logerror("ti99_hd error: write track: Cannot find sync for track %d, head %d.\n", m_current_cylinder, head);
			m_status |= MFMHD_WRFAULT;
			return;
		}
		found = true;

		new_pos = new_pos + SYNC; /* skip sync bytes */

		if (track_image[new_pos]==0xa1)
		{
			/* IDAM found. */
			current_pos = new_pos + 1;
			// wident = track_image[current_pos];  // unused
			wcyl = track_image[current_pos+1] + ((track_image[current_pos+2]&0x70)<<4);
			whead = track_image[current_pos+2]&0x0f;
			wsector = track_image[current_pos+3];
			// wsize = track_image[current_pos+4];  // unused

			if (wcyl == m_current_cylinder && whead == head)
			{
				if (!harddisk_chs_to_lba(file, wcyl, whead, wsector, &lba))
				{
					m_status |= MFMHD_WRFAULT;
					return;
				}

				/* Skip to the sector content. */
				new_pos = find_block(track_image, current_pos, data_count, 0x00, SYNC);
				current_pos = new_pos + SYNC;
				if (track_image[current_pos]==0xa1)
				{
					current_pos += 2;
					state = hard_disk_write(file, lba, track_image+current_pos);
					if (state==0)
					{
						logerror("ti99_hd error: write track: Write error during formatting cylinder %d, head %d\n", wcyl, whead);
						m_status |= MFMHD_WRFAULT;
						return;
					}
					current_pos = current_pos+256+2; /* Skip contents and CRC */
				}
				else
				{
					logerror("ti99_hd error: write track: Cannot find DAM for cylinder %d, head %d, sector %d.\n", wcyl, whead, wsector);
					m_status |= MFMHD_WRFAULT;
					return;
				}
			}
			else
			{
				logerror("ti99_hd error: write track: Cylinder/head mismatch. Drive is on cyl=%d, head=%d, track data say cyl=%d, head=%d\n", m_current_cylinder, head, wcyl, whead);
				m_status |= MFMHD_WRFAULT;
			}
		}
		else
		{
			logerror("ti99_hd error: write track: Invalid track image for cyl=%d, head=%d. Cannot find any IDAM in track data.\n",  m_current_cylinder, head);
			m_status |= MFMHD_WRFAULT;
			return;
		}
	}
}

UINT8 mfm_harddisk_legacy_device::get_status()
{
	UINT8 status = 0;
	hard_disk_file *file = m_drive->get_hard_disk_file();
	if (file!=NULL)
		status |= MFMHD_READY;

	if (m_current_cylinder==0)
		status |= MFMHD_TRACK00;

	if (!m_seeking)
		status |= MFMHD_SEEKCOMP;

	if (m_id_index == 0)
		status |= MFMHD_INDEX;

	m_status = status;
	if (VERBOSE>5) LOG("ti99_hd: request status reply = %02x\n", status);
	return status;
}

void mfm_harddisk_legacy_device::seek(int direction)
{
	const hard_disk_info *info;
	hard_disk_file *file = m_drive->get_hard_disk_file();

	if (file==NULL) return;

	info = hard_disk_get_info(file);

	m_seeking = true;

	if (direction<0)
	{
		if (m_current_cylinder>0)
			m_current_cylinder--;
	}
	else
	{
		if (m_current_cylinder < info->cylinders)
			m_current_cylinder++;
	}

	// TODO: Requires timer

	m_seeking = false;
}

void mfm_harddisk_legacy_device::get_next_id(int head, chrn_id_hd *id)
{
	const hard_disk_info *info;
	hard_disk_file *file;
	int interleave = 3;

	file = m_drive->get_hard_disk_file();

	if (file==NULL)
	{
		m_status &= ~MFMHD_READY;
		return;
	}

	info = hard_disk_get_info(file);

	m_current_head = head;

	/* TODO: implement an interleave suitable for the number of sectors in the track. */
	m_id_index = (m_id_index + interleave) % info->sectors;

	/* build a new info block. */
	id->C = m_current_cylinder;
	id->H = m_current_head;
	id->R = m_id_index;
	id->N = 1;
	id->data_id = m_id_index;
	id->flags = 0;
}

void mfm_harddisk_legacy_device::device_start()
{
	m_current_cylinder = 0;
	m_current_head = 0;
}

void mfm_harddisk_legacy_device::device_reset()
{
	m_drive = static_cast<harddisk_image_device *>(subdevice("drive"));
	m_seeking = false;
	m_status = 0;
	m_id_index = 0;
}

MACHINE_CONFIG_FRAGMENT( mfmhd )
	MCFG_HARDDISK_ADD("drive")
MACHINE_CONFIG_END

machine_config_constructor mfm_harddisk_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mfmhd );
}

const device_type TI99_MFMHD_LEG = &device_creator<mfm_harddisk_legacy_device>;
