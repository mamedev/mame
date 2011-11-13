/*********************************************************************



*********************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/imageutl.h"

// device type definition
const device_type FLOPPY_CONNECTOR = &device_creator<floppy_connector>;
const device_type FLOPPY_35_DD = &device_creator<floppy_35_dd>;
const device_type FLOPPY_35_HD = &device_creator<floppy_35_hd>;
const device_type FLOPPY_525_DD = &device_creator<floppy_525_dd>;

floppy_connector::floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, FLOPPY_CONNECTOR, "Floppy drive connector abstraction", tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}

floppy_connector::~floppy_connector()
{
}

void floppy_connector::set_formats(const floppy_format_type *_formats)
{
	formats = _formats;
}

void floppy_connector::device_start()
{
}

void floppy_connector::device_config_complete()
{
	floppy_image_device *dev = dynamic_cast<floppy_image_device *>(get_card_device());
	if(dev)
		dev->set_formats(formats);
}

floppy_image_device *floppy_connector::get_device()
{
	return dynamic_cast<floppy_image_device *>(get_card_device());
}

//-------------------------------------------------
//  floppy_image_device - constructor
//-------------------------------------------------

floppy_image_device::floppy_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_image_interface(mconfig, *this),
	  device_slot_card_interface(mconfig, *this),
	  image(NULL)
{
	extension_list[0] = '\0';
}

//-------------------------------------------------
//  floppy_image_device - destructor
//-------------------------------------------------

floppy_image_device::~floppy_image_device()
{
}

void floppy_image_device::setup_load_cb(load_cb cb)
{
	cur_load_cb = cb;
}

void floppy_image_device::setup_unload_cb(unload_cb cb)
{
	cur_unload_cb = cb;
}

void floppy_image_device::setup_index_pulse_cb(index_pulse_cb cb)
{
	cur_index_pulse_cb = cb;
}

void floppy_image_device::set_formats(const floppy_format_type *formats)
{
	image_device_format **formatptr;
    image_device_format *format;
    formatptr = &m_formatlist;
	extension_list[0] = '\0';
	fif_list = 0;
	for(int cnt=0; formats[cnt]; cnt++)
	{
		// allocate a new format
		floppy_image_format_t *fif = formats[cnt]();
		if(!fif_list)
			fif_list = fif;
		else
			fif_list->append(fif);

		format = global_alloc_clear(image_device_format);
		format->m_index       = cnt;
		format->m_name        = fif->name();
		format->m_description = fif->description();
		format->m_extensions  = fif->extensions();
		format->m_optspec     = "";

		image_specify_extension( extension_list, 256, fif->extensions() );
		// and append it to the list
		*formatptr = format;
		formatptr = &format->m_next;
	}

	// set brief and instance name
	update_names();
}

void floppy_image_device::device_config_complete()
{
	update_names();
}

void floppy_image_device::set_rpm(float _rpm)
{
	if(rpm == _rpm)
		return;

	rpm = _rpm;
	rev_time = attotime::from_double(60/rpm);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_image_device::device_start()
{
	rpm = 0;
	setup_limits();

	idx = 0;

	/* motor off */
	mon = 1;
	/* set write protect on */
	wpt = 0;

	cyl = 0;
	ss  = 1;
	stp = 1;
	dskchg = 0;
	index_timer = timer_alloc(0);
}

void floppy_image_device::device_reset()
{
	revolution_start_time = attotime::never;
	revolution_count = 0;
	mon = 1;
}

void floppy_image_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	index_resync();
}

bool floppy_image_device::call_load()
{
	io_generic io;
	// Do _not_ remove this cast otherwise the pointer will be incorrect when used by the ioprocs.
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0xff;
	int best = 0;
	floppy_image_format_t *best_format = 0;
	for(floppy_image_format_t *format = fif_list; format; format = format->next) {
		int score = format->identify(&io);
		if(score > best) {
			best = score;
			best_format = format;
		}
	}

	if(!best_format)
		return IMAGE_INIT_FAIL;

	image = global_alloc(floppy_image(tracks, sides));
	best_format->load(&io, image);

	revolution_start_time = attotime::never;
	revolution_count = 0;

	index_resync();

	if (!cur_load_cb.isnull())
		return cur_load_cb(this);
	return IMAGE_INIT_PASS;
}

void floppy_image_device::call_unload()
{
	dskchg = 0;

	if (image) {
		global_free(image);
		image = 0;
	}
	if (!cur_unload_cb.isnull())
		cur_unload_cb(this);
}

/* motor on, active low */
void floppy_image_device::mon_w(int state)
{
	if(mon == state)
		return;

	mon = state;

	/* off -> on */
	if (!mon && image)
	{
		revolution_start_time = machine().time();
		index_resync();
	}

	/* on -> off */
	else {
		revolution_start_time = attotime::never;
		index_timer->adjust(attotime::zero);
	}
}

attotime floppy_image_device::time_next_index()
{
	if(revolution_start_time.is_never())
		return attotime::never;
	return revolution_start_time + attotime::from_double(60/rpm);
}

/* index pulses at rpm/60 Hz, and stays high for ~2ms at 300rpm */
void floppy_image_device::index_resync()
{
	if(revolution_start_time.is_never()) {
		if(idx) {
			idx = 0;
			if (!cur_index_pulse_cb.isnull())
				cur_index_pulse_cb(this, idx);
		}
		return;
	}

	attotime delta = machine().time() - revolution_start_time;
	while(delta >= rev_time) {
		delta -= rev_time;
		revolution_start_time += rev_time;
		revolution_count++;
	}
	int position = (delta*(rpm/300)).as_ticks(1000000000);

	int new_idx = position <= 20000;

	if(new_idx) {
		attotime index_up_time = attotime::from_nsec(2000000*300.0/rpm+0.5);
		index_timer->adjust(index_up_time - delta);
	} else
		index_timer->adjust(rev_time - delta);

	if(new_idx != idx) {
		idx = new_idx;
		if (!cur_index_pulse_cb.isnull())
			cur_index_pulse_cb(this, idx);
	}
}

int floppy_image_device::ready_r()
{
	if (exists())
	{
		if (mon == 0)
		{
			return 0;
		}
	}
	return 1;
}

double floppy_image_device::get_pos()
{
	return index_timer->elapsed().as_double();
}

void floppy_image_device::stp_w(int state)
{
    if ( stp != state ) {
		stp = state;
    	if ( stp == 0 ) {
			if ( dir ) {
				if ( cyl ) cyl--;
			} else {
				if ( cyl < tracks-1 ) cyl++;
			}

			/* Update disk detection if applicable */
			if (exists())
			{
				if (dskchg==0) dskchg = 1;
			}
		}
	}
}

int floppy_image_device::find_index(UINT32 position, const UINT32 *buf, int buf_size)
{
	int spos = (buf_size >> 1)-1;
	int step;
	for(step=1; step<buf_size+1; step<<=1);
	step >>= 1;

	for(;;) {
		if(spos >= buf_size || (spos > 0 && (buf[spos] & floppy_image::TIME_MASK) > position)) {
			spos -= step;
			step >>= 1;
		} else if(spos < 0 || (spos < buf_size-1 && (buf[spos+1] & floppy_image::TIME_MASK) <= position)) {
			spos += step;
			step >>= 1;
		} else
			return spos;
	}
}

UINT32 floppy_image_device::find_position(attotime &base, attotime when)
{
	base = revolution_start_time;
	UINT32 revc = revolution_count;
	attotime delta = when - base;

	while(delta >= rev_time) {
		delta -= rev_time;
		base += rev_time;
		revc++;
	}

	return (delta*(rpm/300)).as_ticks(1000000000);
}

attotime floppy_image_device::get_next_transition(attotime from_when)
{
	if(!image || mon)
		return attotime::never;

	int cells = image->get_track_size(cyl, ss);
	if(cells <= 1)
		return attotime::never;

	attotime base;
	UINT32 position = find_position(base, from_when);

	const UINT32 *buf = image->get_buffer(cyl, ss);
	int index = find_index(position, buf, cells);

	if(index == -1)
		return attotime::never;

	UINT32 next_position;
	if(index < cells-1)
		next_position = buf[index+1] & floppy_image::TIME_MASK;
	else if((buf[index]^buf[0]) & floppy_image::MG_MASK)
		next_position = 200000000;
	else
		next_position = 200000000 + (buf[1] & floppy_image::TIME_MASK);


	return base + attotime::from_nsec(next_position*(300/rpm));
}

void floppy_image_device::write_flux(attotime start, attotime end, int transition_count, const attotime *transitions)
{
	attotime base;
	int start_pos = find_position(base, start);
	int end_pos   = find_position(base, end);

	int *trans_pos = transition_count ? global_alloc_array(int, transition_count) : 0;
	for(int i=0; i != transition_count; i++)
		trans_pos[i] = find_position(base, transitions[i]);

	int cells = image->get_track_size(cyl, ss);
	UINT32 *buf = image->get_buffer(cyl, ss);

	int index;
	if(cells)
		index = find_index(start_pos, buf, cells);
	else {
		index = 0;
		buf[cells++] = floppy_image::MG_N | 200000000;
	}

	if(index && (buf[index] & floppy_image::TIME_MASK) == start_pos)
		index--;

	UINT32 cur_mg = buf[index] & floppy_image::MG_MASK;
	if(cur_mg == floppy_image::MG_N || cur_mg == floppy_image::MG_D)
		cur_mg = floppy_image::MG_A;

	UINT32 pos = start_pos;
	int ti = 0;
	while(pos != end_pos) {
		UINT32 next_pos;
		if(ti != transition_count)
			next_pos = trans_pos[ti++];
		else
			next_pos = end_pos;
		if(next_pos > pos)
			write_zone(buf, cells, index, pos, next_pos, cur_mg);
		else {
			write_zone(buf, cells, index, pos, 200000000, cur_mg);
			write_zone(buf, cells, index, 0, next_pos, cur_mg);
		}
		pos = next_pos;
		cur_mg = cur_mg == floppy_image::MG_A ? floppy_image::MG_B : floppy_image::MG_A;
	}

	image->set_track_size(cyl, ss, cells);

	if(trans_pos)
		global_free(trans_pos);
}

void floppy_image_device::write_zone(UINT32 *buf, int &cells, int &index, UINT32 spos, UINT32 epos, UINT32 mg)
{
	while(spos < epos) {
		while(index != cells-1 && (buf[index+1] & floppy_image::TIME_MASK) <= spos)
			index++;

		UINT32 ref_start = buf[index] & floppy_image::TIME_MASK;
		UINT32 ref_end   = index == cells-1 ? 200000000 : buf[index+1] & floppy_image::TIME_MASK;
		UINT32 ref_mg    = buf[index] & floppy_image::MG_MASK;

		// Can't overwrite a damaged zone
		if(ref_mg == floppy_image::MG_D) {
			spos = ref_end;
			continue;
		}

		// If the zone is of the type we want, we don't need to touch it
		if(ref_mg == mg) {
			spos = ref_end;
			continue;
		}

		//  Check the overlaps, act accordingly
		if(spos == ref_start) {
			if(epos >= ref_end) {
				// Full overlap, that cell is dead, we need to see which ones we can extend
				UINT32 prev_mg = index != 0       ? buf[index-1] & floppy_image::MG_MASK : ~0;
				UINT32 next_mg = index != cells-1 ? buf[index+1] & floppy_image::MG_MASK : ~0;
				if(prev_mg == mg) {
					if(next_mg == mg) {
						// Both match, merge all three in one
						memmove(buf+index, buf+index+2, (cells-index-2)*sizeof(UINT32));
						cells -= 2;
						index--;

					} else {
						// Previous matches, drop the current cell
						memmove(buf+index, buf+index+1, (cells-index-1)*sizeof(UINT32));
						cells --;
					}

				} else {
					if(next_mg == mg) {
						// Following matches, extend it
						memmove(buf+index, buf+index+1, (cells-index-1)*sizeof(UINT32));
						cells --;
						buf[index] = mg | spos;
					} else {
						// None match, convert the current cell
						buf[index] = mg | spos;
						index++;
					}
				}
				spos = ref_end;

			} else {
				// Overlap at the start only
				// Check if we can just extend the previous cell
				if(index != 0 && (buf[index-1] & floppy_image::MG_MASK) == mg)
					buf[index] = ref_mg | epos;
				else {
					// Otherwise we need to insert a new cell
					if(index != cells-1)
						memmove(buf+index+1, buf+index, (cells-index)*sizeof(UINT32));
					cells++;
					buf[index] = mg | spos;
					buf[index+1] = ref_mg | epos;
				}
				spos = epos;
			}

		} else {
			if(epos >= ref_end) {
				// Overlap at the end only
				// If we can't just extend the following cell, we need to insert a new one
				if(index == cells-1 || (buf[index+1] & floppy_image::MG_MASK) != mg) {
					if(index != cells-1)
						memmove(buf+index+2, buf+index+1, (cells-index-1)*sizeof(UINT32));
					cells++;
				}
				buf[index+1] = mg | spos;
				index++;
				spos = ref_end;

			} else {
				// Full inclusion
				// We need to split the zone in 3
				if(index != cells-1)
					memmove(buf+index+3, buf+index+1, (cells-index-1)*sizeof(UINT32));
				cells += 2;
				buf[index+1] = mg | spos;
				buf[index+2] = ref_mg | epos;
				spos = epos;
			}
		}

	}
}

floppy_35_dd::floppy_35_dd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_35_DD, "3.5\" double density floppy drive", tag, owner, clock)
{
}

floppy_35_dd::~floppy_35_dd()
{
}

void floppy_35_dd::setup_limits()
{
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

floppy_35_hd::floppy_35_hd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_35_HD, "3.5\" high density floppy drive", tag, owner, clock)
{
}

floppy_35_hd::~floppy_35_hd()
{
}

void floppy_35_hd::setup_limits()
{
	tracks = 84;
	sides = 2;
	set_rpm(300);
}


floppy_525_dd::floppy_525_dd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_DD, "3.5\" high density floppy drive", tag, owner, clock)
{
}

floppy_525_dd::~floppy_525_dd()
{
}

void floppy_525_dd::setup_limits()
{
	tracks = 42;
	sides = 1;
	set_rpm(300);
}
