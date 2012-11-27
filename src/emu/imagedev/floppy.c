/*********************************************************************



*********************************************************************/

#include "emu.h"
#include "zippath.h"
#include "floppy.h"
#include "formats/imageutl.h"
#include "uiimage.h"

// device type definition
const device_type FLOPPY_CONNECTOR = &device_creator<floppy_connector>;
const device_type FLOPPY_3_SSDD = &device_creator<floppy_3_ssdd>;
const device_type FLOPPY_3_DSDD = &device_creator<floppy_3_dsdd>;
const device_type FLOPPY_35_DD = &device_creator<floppy_35_dd>;
const device_type FLOPPY_35_DD_NOSD = &device_creator<floppy_35_dd_nosd>;
const device_type FLOPPY_35_HD = &device_creator<floppy_35_hd>;
const device_type FLOPPY_35_ED = &device_creator<floppy_35_ed>;
const device_type FLOPPY_525_SSSD_35T = &device_creator<floppy_525_sssd_35t>;
const device_type FLOPPY_525_SSSD = &device_creator<floppy_525_sssd>;
const device_type FLOPPY_525_SSDD = &device_creator<floppy_525_ssdd>;
const device_type FLOPPY_525_DD = &device_creator<floppy_525_dd>;
const device_type FLOPPY_525_QD = &device_creator<floppy_525_qd>;
const device_type FLOPPY_525_HD = &device_creator<floppy_525_hd>;
const device_type FLOPPY_8_SSSD = &device_creator<floppy_8_sssd>;
const device_type FLOPPY_8_SSDD = &device_creator<floppy_8_ssdd>;
const device_type FLOPPY_8_DSDD = &device_creator<floppy_8_dsdd>;

const floppy_format_type floppy_image_device::default_floppy_formats[] = {
	FLOPPY_D88_FORMAT,
	FLOPPY_DFI_FORMAT,
	FLOPPY_IMD_FORMAT,
	FLOPPY_IPF_FORMAT,
	FLOPPY_MFI_FORMAT,
	FLOPPY_MFM_FORMAT,
	NULL
};

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

floppy_image_format_t *floppy_image_device::get_formats() const
{
	return fif_list;
}

floppy_image_format_t *floppy_image_device::get_load_format() const
{
	return input_format;
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

void floppy_image_device::setup_write(floppy_image_format_t *_output_format)
{
	output_format = _output_format;
	commit_image();
}

void floppy_image_device::commit_image()
{
	image_dirty = false;
	if(!output_format)
		return;
	io_generic io;
	// Do _not_ remove this cast otherwise the pointer will be incorrect when used by the ioprocs.
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0xff;
	output_format->save(&io, image);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_image_device::device_start()
{
	rpm = 0;
	motor_always_on = false;
	setup_characteristics();

	idx = 0;

	/* motor off */
	mon = 1;

	cyl = 0;
	ss  = 0;
	stp = 1;
	dskchg = 0;
	index_timer = timer_alloc(0);
	image_dirty = false;
}

void floppy_image_device::device_reset()
{
	revolution_start_time = attotime::never;
	revolution_count = 0;
	mon = 1;
	if(motor_always_on)
		mon_w(0);
}

void floppy_image_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	index_resync();
}

floppy_image_format_t *floppy_image_device::identify(astring filename) const
{
	FILE *fd;
	fd = fopen(filename.cstr(), "r");
	if(!fd)
		return 0;

	io_generic io;
	io.file = fd;
	io.procs = &stdio_ioprocs_noclose;
	io.filler = 0xff;
	int best = 0;
	floppy_image_format_t *best_format = 0;
	for(floppy_image_format_t *format = fif_list; format; format = format->next) {
		int score = format->identify(&io, form_factor);
		if(score > best) {
			best = score;
			best_format = format;
		}
	}
	fclose(fd);
	return best_format;
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
		int score = format->identify(&io, form_factor);
		if(score > best) {
			best = score;
			best_format = format;
		}
	}

	if(!best_format)
	{
		seterror(IMAGE_ERROR_INVALIDIMAGE, "Unable to identify the image format");
		return IMAGE_INIT_FAIL;
	}

	image = global_alloc(floppy_image(tracks, sides, form_factor));
	best_format->load(&io, form_factor, image);

	revolution_start_time = motor_always_on ? machine().time() : attotime::never;
	revolution_count = 0;

	index_resync();
	image_dirty = false;
	output_format = 0;

	if (!cur_load_cb.isnull())
		return cur_load_cb(this);
	return IMAGE_INIT_PASS;
}

void floppy_image_device::call_unload()
{
	dskchg = 0;

	if (image) {
		if(image_dirty)
			commit_image();
		global_free(image);
		image = 0;
	}
	if (!cur_unload_cb.isnull())
		cur_unload_cb(this);
}

bool floppy_image_device::call_create(int format_type, option_resolution *format_options)
{
	image = global_alloc(floppy_image(tracks, sides, form_factor));
	output_format = 0;
	return IMAGE_INIT_PASS;
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
		if(image_dirty)
			commit_image();
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

bool floppy_image_device::ready_r()
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

bool floppy_image_device::twosid_r()
{
	int tracks = 0, heads = 0;

	if (image) image->get_actual_geometry(tracks, heads);

	return heads == 2;
}

void floppy_image_device::stp_w(int state)
{
    if ( stp != state ) {
		stp = state;
    	if ( stp == 0 ) {
			int ocyl = cyl;
			if ( dir ) {
				if ( cyl ) cyl--;
			} else {
				if ( cyl < tracks-1 ) cyl++;
			}
			if(ocyl != cyl)
				logerror("%s: track %d\n", tag(), cyl);
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

	return (delta*rpm/300).as_ticks(1000000000);
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

	//  logerror("Floppy: cuspos=%d nextpos=%d\n", position, next_position);
	return base + attotime::from_nsec(UINT64(next_position)*300/rpm);
}

void floppy_image_device::write_flux(attotime start, attotime end, int transition_count, const attotime *transitions)
{
	image_dirty = true;

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
		image->set_track_size(cyl, ss, 1);
		buf = image->get_buffer(cyl, ss);
		buf[cells++] = floppy_image::MG_N;
	}

	if(index && (buf[index] & floppy_image::TIME_MASK) == start_pos)
		index--;

	UINT32 cur_mg = buf[index] & floppy_image::MG_MASK;
	if(cur_mg == floppy_image::MG_N || cur_mg == floppy_image::MG_D)
		cur_mg = floppy_image::MG_A;

	UINT32 pos = start_pos;
	int ti = 0;
	while(pos != end_pos) {
		if(image->get_track_size(cyl, ss) < cells+10) {
			image->set_track_size(cyl, ss, cells+200);
			buf = image->get_buffer(cyl, ss);
		}
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

UINT32 floppy_image_device::get_form_factor() const
{
	return form_factor;
}

UINT32 floppy_image_device::get_variant() const
{
	return image ? image->get_variant() : 0;
}

ui_menu *floppy_image_device::get_selection_menu(running_machine &machine, render_container *container)
{
	return auto_alloc_clear(machine, ui_menu_control_floppy_image(machine, container, this));
}

ui_menu_control_floppy_image::ui_menu_control_floppy_image(running_machine &machine, render_container *container, device_image_interface *_image) : ui_menu_control_device_image(machine, container, _image)
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(image);
	const floppy_image_format_t *fif_list = fd->get_formats();
	int fcnt = 0;
	for(const floppy_image_format_t *i = fif_list; i; i = i->next)
		fcnt++;

	format_array = global_alloc_array(floppy_image_format_t *, fcnt);
	input_format = output_format = 0;
	input_filename = output_filename = "";
}

ui_menu_control_floppy_image::~ui_menu_control_floppy_image()
{
	global_free(format_array);
}

void ui_menu_control_floppy_image::do_load_create()
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(image);
	if(input_filename == "") {
		int err = fd->create(output_filename, 0, NULL);
		if (err != 0) {
			popmessage("Error: %s", fd->error());
			return;
		}
		fd->setup_write(output_format);
	} else {
		int err = fd->load(input_filename);
		if(!err && output_filename != "")
			err = fd->reopen_for_write(output_filename);
		if(err != 0) {
			popmessage("Error: %s", fd->error());
			return;
		}
		if(output_format)
			fd->setup_write(output_format);
	}
}

void ui_menu_control_floppy_image::hook_load(astring filename, bool softlist)
{
	input_filename = filename;
	input_format = static_cast<floppy_image_device *>(image)->identify(filename);
	if(!input_format) {
		popmessage("Error: %s\n", image->error());
		ui_menu::stack_pop(machine());
		return;
	}

	bool can_in_place = input_format->supports_save();
	if(can_in_place) {
		file_error filerr = FILERR_NOT_FOUND;
		astring tmp_path;
		core_file *tmp_file;
		/* attempt to open the file for writing but *without* create */
		filerr = zippath_fopen(filename, OPEN_FLAG_READ|OPEN_FLAG_WRITE, tmp_file, tmp_path);
		if(!filerr)
			core_fclose(tmp_file);
		else
			can_in_place = false;
	}
	submenu_result = -1;
	ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_rw(machine(), container, can_in_place, &submenu_result)));
	state = SELECT_RW;
}

void ui_menu_control_floppy_image::handle()
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(image);
	switch(state) {
	case DO_CREATE: {
		floppy_image_format_t *fif_list = fd->get_formats();
		int ext_match = 0, total_usable = 0;
		for(floppy_image_format_t *i = fif_list; i; i = i->next) {
			if(!i->supports_save())
				continue;
			if(i->extension_matches(current_file))
				format_array[total_usable++] = i;
		}
		ext_match = total_usable;
		for(floppy_image_format_t *i = fif_list; i; i = i->next) {
			if(!i->supports_save())
				continue;
			if(!i->extension_matches(current_file))
				format_array[total_usable++] = i;
		}
		submenu_result = -1;
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_format(machine(), container, format_array, ext_match, total_usable, &submenu_result)));

		state = SELECT_FORMAT;
		break;
	}

	case SELECT_FORMAT:
		if(submenu_result == -1) {
			state = START_FILE;
			handle();
		} else {
			zippath_combine(output_filename, current_directory, current_file);
			output_format = format_array[submenu_result];
			do_load_create();
			ui_menu::stack_pop(machine());
		}
		break;

	case SELECT_RW:
		switch(submenu_result) {
		case ui_menu_select_rw::READONLY:
			do_load_create();
			ui_menu::stack_pop(machine());
			break;

		case ui_menu_select_rw::READWRITE:
			output_format = input_format;
			do_load_create();
			ui_menu::stack_pop(machine());
			break;

		case ui_menu_select_rw::WRITE_DIFF:
			popmessage("Sorry, diffs are not supported yet\n");
			ui_menu::stack_pop(machine());
			break;

		case ui_menu_select_rw::WRITE_OTHER:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_file_create(machine(), container, image, current_directory, current_file)));
			state = CREATE_FILE;
			break;

		case -1:
			ui_menu::stack_pop(machine());
			break;
		}
		break;

	default:
		ui_menu_control_device_image::handle();
	}
}

floppy_3_ssdd::floppy_3_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_3_SSDD, "3\" double sided floppy drive", tag, owner, clock)
{
}

floppy_3_ssdd::~floppy_3_ssdd()
{
}

void floppy_3_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_3;
	tracks = 42;
	sides = 2;
	set_rpm(300);
}

void floppy_3_ssdd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSDD;
}

floppy_3_dsdd::floppy_3_dsdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_3_DSDD, "3\" single sided floppy drive", tag, owner, clock)
{
}

floppy_3_dsdd::~floppy_3_dsdd()
{
}

void floppy_3_dsdd::setup_characteristics()
{
	form_factor = floppy_image::FF_3;
	tracks = 42;
	sides = 1;
	set_rpm(300);
}

void floppy_3_dsdd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

floppy_35_dd::floppy_35_dd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_35_DD, "3.5\" double density floppy drive", tag, owner, clock)
{
}

floppy_35_dd::~floppy_35_dd()
{
}

void floppy_35_dd::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_dd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

floppy_35_dd_nosd::floppy_35_dd_nosd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_35_DD_NOSD, "3.5\" double density floppy drive", tag, owner, clock)
{
}

floppy_35_dd_nosd::~floppy_35_dd_nosd()
{
}

void floppy_35_dd_nosd::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_dd_nosd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

floppy_35_hd::floppy_35_hd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_35_HD, "3.5\" high density floppy drive", tag, owner, clock)
{
}

floppy_35_hd::~floppy_35_hd()
{
}

void floppy_35_hd::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_hd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSHD;
}

floppy_35_ed::floppy_35_ed(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_35_ED, "3.5\" extended density floppy drive", tag, owner, clock)
{
}

floppy_35_ed::~floppy_35_ed()
{
}

void floppy_35_ed::setup_characteristics()
{
	form_factor = floppy_image::FF_35;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_35_ed::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSHD;
	variants[var_count++] = floppy_image::DSED;
}

floppy_525_sssd_35t::floppy_525_sssd_35t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSSD_35T, "5.25\" single-sided single density 35-track floppy drive", tag, owner, clock)
{
}

floppy_525_sssd_35t::~floppy_525_sssd_35t()
{
}

void floppy_525_sssd_35t::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 35;
	sides = 1;
	set_rpm(300);
}

void floppy_525_sssd_35t::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

floppy_525_sssd::floppy_525_sssd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSSD, "5.25\" single-sided single density floppy drive", tag, owner, clock)
{
}

floppy_525_sssd::~floppy_525_sssd()
{
}

void floppy_525_sssd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 1;
	set_rpm(300);
}

void floppy_525_sssd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

floppy_525_ssdd::floppy_525_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_SSDD, "5.25\" single-sided double density floppy drive", tag, owner, clock)
{
}

floppy_525_ssdd::~floppy_525_ssdd()
{
}

void floppy_525_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 1;
	set_rpm(300);
}

void floppy_525_ssdd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

floppy_525_dd::floppy_525_dd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_DD, "5.25\" double density floppy drive", tag, owner, clock)
{
}

floppy_525_dd::~floppy_525_dd()
{
}

void floppy_525_dd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 42;
	sides = 2;
	set_rpm(300);
}

void floppy_525_dd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}

floppy_525_qd::floppy_525_qd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_QD, "5.25\" quad density floppy drive", tag, owner, clock)
{
}

floppy_525_qd::~floppy_525_qd()
{
}

void floppy_525_qd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 84;
	sides = 2;
	set_rpm(300);
}

void floppy_525_qd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSQD;
}

floppy_525_hd::floppy_525_hd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_525_HD, "5.25\" high density floppy drive", tag, owner, clock)
{
}

floppy_525_hd::~floppy_525_hd()
{
}

void floppy_525_hd::setup_characteristics()
{
	form_factor = floppy_image::FF_525;
	tracks = 84;
	sides = 2;
	set_rpm(360);
}

void floppy_525_hd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::SSQD;
	variants[var_count++] = floppy_image::DSDD;
	variants[var_count++] = floppy_image::DSQD;
	variants[var_count++] = floppy_image::DSHD;
}

floppy_8_sssd::floppy_8_sssd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_8_SSSD, "8\" single density floppy drive", tag, owner, clock)
{
}

floppy_8_sssd::~floppy_8_sssd()
{
}

void floppy_8_sssd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 1;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_sssd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
}

floppy_8_ssdd::floppy_8_ssdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_8_DSDD, "8\" double density single sided floppy drive", tag, owner, clock)
{
}

floppy_8_ssdd::~floppy_8_ssdd()
{
}

void floppy_8_ssdd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 1;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_ssdd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
}

floppy_8_dsdd::floppy_8_dsdd(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	floppy_image_device(mconfig, FLOPPY_8_DSDD, "8\" double density double sided floppy drive", tag, owner, clock)
{
}

floppy_8_dsdd::~floppy_8_dsdd()
{
}

void floppy_8_dsdd::setup_characteristics()
{
	form_factor = floppy_image::FF_8;
	tracks = 77;
	sides = 2;
	motor_always_on = true;
	set_rpm(360);
}

void floppy_8_dsdd::handled_variants(UINT32 *variants, int &var_count) const
{
	var_count = 0;
	variants[var_count++] = floppy_image::SSSD;
	variants[var_count++] = floppy_image::SSDD;
	variants[var_count++] = floppy_image::DSDD;
}
