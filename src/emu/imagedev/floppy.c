/*********************************************************************



*********************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/imageutl.h"

// device type definition
const device_type FLOPPY = &device_creator<floppy_image_device>;

//-------------------------------------------------
//  floppy_image_device - constructor
//-------------------------------------------------

floppy_image_device::floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, FLOPPY, "Floppy drive", tag, owner, clock),
	  device_image_interface(mconfig, *this),
	  image(NULL)
{
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

void floppy_image_device::set_info(int _type, int _tracks, int _sides, const floppy_format_type *formats)
{
	type = _type;
	tracks = _tracks;
	sides = _sides;

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
	idx = 0;

	/* motor off */
	mon = 1;
	/* set write protect on */
	wpt = 0;

	rpm = 0;
	set_rpm(300);

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

int floppy_image_device::find_position(int position, const UINT32 *buf, int buf_size)
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

attotime floppy_image_device::get_next_transition(attotime from_when)
{
	if(!image || mon)
		return attotime::never;

	int cells = image->get_track_size(cyl, ss);
	if(cells <= 1)
		return attotime::never;

	attotime base = revolution_start_time;
	UINT32 revc = revolution_count;
	attotime delta = from_when - base;

	while(delta >= rev_time) {
		delta -= rev_time;
		base += rev_time;
		revc++;
	}
	int position = (delta*(rpm/300)).as_ticks(1000000000);

	const UINT32 *buf = image->get_buffer(cyl, ss);
	int index = find_position(position, buf, cells);

	if(index == -1)
		return attotime::never;

	int next_position;
	if(index < cells-1)
		next_position = buf[index+1] & floppy_image::TIME_MASK;
	else if((buf[index]^buf[0]) & floppy_image::MG_MASK)
		next_position = 200000000;
	else
		next_position = 200000000 + (buf[1] & floppy_image::TIME_MASK);

	return base + attotime::from_nsec(next_position*(300/rpm));
}
