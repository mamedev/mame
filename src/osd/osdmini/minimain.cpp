// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  minimain.c - Main function for mini OSD
//
//============================================================

#include "emu.h"
#include "osdepend.h"
#include "render.h"
#include "clifront.h"
#include "osdmini.h"


//============================================================
//  CONSTANTS
//============================================================

// we fake a keyboard with the following keys
enum
{
	KEY_ESCAPE,
	KEY_P1_START,
	KEY_BUTTON_1,
	KEY_BUTTON_2,
	KEY_BUTTON_3,
	KEY_JOYSTICK_U,
	KEY_JOYSTICK_D,
	KEY_JOYSTICK_L,
	KEY_JOYSTICK_R,
	KEY_TOTAL
};


//============================================================
//  GLOBALS
//============================================================

// a single rendering target
static render_target *our_target;

// a single input device
static input_device *keyboard_device;

// the state of each key
static UINT8 keyboard_state[KEY_TOTAL];


//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static INT32 keyboard_get_state(void *device_internal, void *item_internal);


//============================================================
//  mini_osd_options
//============================================================

mini_osd_options::mini_osd_options()
: osd_options()
{
	//add_entries(s_option_entries);
}

//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	// cli_frontend does the heavy lifting; if we have osd-specific options, we
	// create a derivative of cli_options and add our own
	mini_osd_options options;
	mini_osd_interface osd(options);
	osd.register_options();
	cli_frontend frontend(options, osd);
	return frontend.execute(argc, argv);
}


//============================================================
//  constructor
//============================================================

mini_osd_interface::mini_osd_interface(mini_osd_options &options)
: osd_common_t(options)
{
}


//============================================================
//  destructor
//============================================================

mini_osd_interface::~mini_osd_interface()
{
}


//============================================================
//  init
//============================================================

void mini_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

	// initialize the video system by allocating a rendering target
	our_target = machine.render().target_alloc();

	// nothing yet to do to initialize sound, since we don't have any
	// sound updates are handled by update_audio_stream() below

	// initialize the input system by adding devices
	// let's pretend like we have a keyboard device
	keyboard_device = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device("Keyboard");
	if (keyboard_device == nullptr)
		fatalerror("Error creating keyboard device\n");

	// our faux keyboard only has a couple of keys (corresponding to the
	// common defaults)
	keyboard_device->add_item("Esc", ITEM_ID_ESC, keyboard_get_state, &keyboard_state[KEY_ESCAPE]);
	keyboard_device->add_item("P1", ITEM_ID_1, keyboard_get_state, &keyboard_state[KEY_P1_START]);
	keyboard_device->add_item("B1", ITEM_ID_LCONTROL, keyboard_get_state, &keyboard_state[KEY_BUTTON_1]);
	keyboard_device->add_item("B2", ITEM_ID_LALT, keyboard_get_state, &keyboard_state[KEY_BUTTON_2]);
	keyboard_device->add_item("B3", ITEM_ID_SPACE, keyboard_get_state, &keyboard_state[KEY_BUTTON_3]);
	keyboard_device->add_item("JoyU", ITEM_ID_UP, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_U]);
	keyboard_device->add_item("JoyD", ITEM_ID_DOWN, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_D]);
	keyboard_device->add_item("JoyL", ITEM_ID_LEFT, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_L]);
	keyboard_device->add_item("JoyR", ITEM_ID_RIGHT, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_R]);

	// hook up the debugger log
//  add_logerror_callback(machine, output_oslog);
}


//============================================================
//  osd_update
//============================================================

void mini_osd_interface::update(bool skip_redraw)
{
	// get the minimum width/height for the current layout
	int minwidth, minheight;
	our_target->compute_minimum_size(minwidth, minheight);

	// make that the size of our target
	our_target->set_bounds(minwidth, minheight);

	// get the list of primitives for the target at the current size
	render_primitive_list &primlist = our_target->get_primitives();

	// lock them, and then render them
	primlist.acquire_lock();

	// do the drawing here
	primlist.release_lock();

	// after 5 seconds, exit
	if (machine().time() > attotime::from_seconds(5))
		machine().schedule_exit();
}


//============================================================
//  keyboard_get_state
//============================================================

static INT32 keyboard_get_state(void *device_internal, void *item_internal)
{
	// this function is called by the input system to get the current key
	// state; it is specified as a callback when adding items to input
	// devices
	UINT8 *keystate = (UINT8 *)item_internal;
	return *keystate;
}
