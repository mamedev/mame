// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_x11.cpp - X11 XLib/XInput routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(SDLMAME_SDL2) && !defined(SDLMAME_WIN32) && defined(USE_XINPUT) && USE_XINPUT

// for X11 xinput
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/Xutil.h>

// standard sdl header
#include "sdlinc.h"
#include <ctype.h>
#include <stddef.h>
#include <mutex>
#include <memory>
#include <queue>
#include <string>

// MAME headers
#include "emu.h"
#include "osdepend.h"

// MAMEOS headers
#include "../lib/osdobj_common.h"
#include "input_common.h"
#include "../../sdl/osdsdl.h"
#include "input_sdlcommon.h"

#define MAX_DEVMAP_ENTRIES  16

#define INVALID_EVENT_TYPE     -1
static int motion_type         = INVALID_EVENT_TYPE;
static int button_press_type   = INVALID_EVENT_TYPE;
static int button_release_type = INVALID_EVENT_TYPE;
static int key_press_type      = INVALID_EVENT_TYPE;
static int key_release_type    = INVALID_EVENT_TYPE;
static int proximity_in_type   = INVALID_EVENT_TYPE;
static int proximity_out_type  = INVALID_EVENT_TYPE;

// state information for a lightgun
struct lightgun_state
{
	INT32 lX, lY;
	INT32 buttons[MAX_BUTTONS];
};

struct x11_api_state
{
	XID deviceid; // X11 device id
	INT32 maxx, maxy;
	INT32 minx, miny;
};

//============================================================
//  DEBUG MACROS
//============================================================

#if defined(XINPUT_DEBUG) && XINPUT_DEBUG
#define XI_DBG(format, ...) osd_printf_verbose(format, __VA_ARGS__)

#define print_motion_event(motion) print_motion_event_impl(motion)
static inline void print_motion_event_impl(XDeviceMotionEvent *motion)
{
	/*
	* print a lot of debug informations of the motion event(s).
	*/
	osd_printf_verbose(
		"XDeviceMotionEvent:\n"
		"  type: %d\n"
		"  serial: %lu\n"
		"  send_event: %d\n"
		"  display: %p\n"
		"  window: --\n"
		"  deviceid: %lu\n"
		"  root: --\n"
		"  subwindow: --\n"
		"  time: --\n"
		"  x: %d, y: %d\n"
		"  x_root: %d, y_root: %d\n"
		"  state: %u\n"
		"  is_hint: %2.2X\n"
		"  same_screen: %d\n"
		"  device_state: %u\n"
		"  axes_count: %2.2X\n"
		"  first_axis: %2.2X\n"
		"  axis_data[6]: {%d,%d,%d,%d,%d,%d}\n",
		motion->type,
		motion->serial,
		motion->send_event,
		motion->display,
		/* motion->window, */
		motion->deviceid,
		/* motion->root */
		/* motion->subwindow */
		/* motion->time, */
		motion->x, motion->y,
		motion->x_root, motion->y_root,
		motion->state,
		motion->is_hint,
		motion->same_screen,
		motion->device_state,
		motion->axes_count,
		motion->first_axis,
		motion->axis_data[0], motion->axis_data[1], motion->axis_data[2], motion->axis_data[3], motion->axis_data[4], motion->axis_data[5]
		);
}
#else
#define XI_DBG(format, ...) while(0) {}
#define print_motion_event(motion) while(0) {}
#endif

//============================================================
//  lightgun helpers: copy-past from xinfo
//============================================================

XDeviceInfo*
find_device_info(Display    *display,
			const char       *name,
			bool       only_extended)
{
	XDeviceInfo *devices;
	XDeviceInfo *found = NULL;
	int     loop;
	int     num_devices;
	int     len = strlen(name);
	bool    is_id = true;
	XID     id = (XID)-1;

	for(loop = 0; loop < len; loop++)
	{
		if (!isdigit(name[loop]))
		{
			is_id = false;
			break;
		}
	}

	if (is_id)
	{
		id = atoi(name);
	}

	devices = XListInputDevices(display, &num_devices);

	for(loop = 0; loop < num_devices; loop++)
	{
		osd_printf_verbose("Evaluating device with name: %s\n", devices[loop].name);

		// if only extended devices and our device isn't extended, skip
		if (only_extended && devices[loop].use < IsXExtensionDevice)
			continue;

		// Adjust name to remove spaces for accurate comparison
		std::string name_no_space = remove_spaces(devices[loop].name);
		if ((!is_id && strcmp(name_no_space.c_str(), name) == 0)
			|| (is_id && devices[loop].id == id))
		{
			if (found)
			{
				osd_printf_verbose(
					"Warning: There are multiple devices named \"%s\".\n"
					"To ensure the correct one is selected, please use "
					"the device ID instead.\n\n", name);
			}
			else
			{
				found = &devices[loop];
			}
		}
	}

	return found;
}

//Copypasted from xinfo
static int
register_events(
	Display *dpy,
	XDeviceInfo *info,
	const char *dev_name,
	bool handle_proximity)
{
	int                number = 0; /* number of events registered */
	XEventClass        event_list[7];
	int                i;
	XDevice *          device;
	Window             root_win;
	unsigned long      screen;
	XInputClassInfo *  ip;

	screen = DefaultScreen(dpy);
	root_win = RootWindow(dpy, screen);

	device = XOpenDevice(dpy, info->id);
	if (device == nullptr)
	{
		osd_printf_verbose("unable to open device %s\n", dev_name);
		return 0;
	}

	if (device->num_classes > 0)
	{
		for (ip = device->classes, i = 0; i < info->num_classes; ip++, i++)
		{
			switch (ip->input_class)
			{
			case KeyClass:
				DeviceKeyPress(device, key_press_type, event_list[number]); number++;
				DeviceKeyRelease(device, key_release_type, event_list[number]); number++;
				break;

			case ButtonClass:
				DeviceButtonPress(device, button_press_type, event_list[number]); number++;
				DeviceButtonRelease(device, button_release_type, event_list[number]); number++;
				break;

			case ValuatorClass:
				DeviceMotionNotify(device, motion_type, event_list[number]); number++;
				osd_printf_verbose("Motion = %i\n",motion_type);
				if (handle_proximity)
				{
					ProximityIn(device, proximity_in_type, event_list[number]); number++;
					ProximityOut(device, proximity_out_type, event_list[number]); number++;
				}
				break;

			default:
				osd_printf_verbose("unknown class\n");
				break;
			}
		}

		if (XSelectExtensionEvent(dpy, root_win, event_list, number))
		{
			osd_printf_verbose("error selecting extended events\n");
			return 0;
		}
	}

	return number;
}

//============================================================
//  x11_event_manager
//============================================================

class x11_event_handler
{
public:
	virtual void handle_event(XEvent &xevent) = 0;
};

class x11_event_manager : public event_manager_t<x11_event_handler>
{
private:
	Display *            m_display;

	x11_event_manager()
		: event_manager_t(),
		m_display(nullptr)
	{
	}
public:
	Display *       display() { return m_display; }

	static x11_event_manager& instance()
	{
		static x11_event_manager s_instance;
		return s_instance;
	}

	int initialize()
	{
		std::lock_guard<std::mutex> scope_lock(m_lock);

		if (m_display != nullptr)
			return 0;

		m_display = XOpenDisplay(NULL);
		if (m_display == nullptr)
		{
			osd_printf_verbose("Unable to connect to X server\n");
			return -1;
		}

		XExtensionVersion *version = XGetExtensionVersion(m_display, INAME);
		if (!version || (version == (XExtensionVersion*)NoSuchExtension))
		{
			osd_printf_verbose("xinput extension not available!\n");
			return -1;
		}

		return 0;
	}

	void process_events(running_machine &machine) override
	{
		std::lock_guard<std::mutex> scope_lock(m_lock);
		XEvent xevent;

		//Get XInput events
		while (XPending(m_display) != 0)
		{
			XNextEvent(m_display, &xevent);

			// Find all subscribers for the event type
			auto subscribers = m_subscription_index.equal_range(xevent.type);

			// Dispatch the events
			for (auto iter = subscribers.first; iter != subscribers.second; iter++)
				iter->second->handle_event(xevent);
		}
	}
};

//============================================================
//  x11_input_device
//============================================================

class x11_input_device : public event_based_device<XEvent>
{
public:
	x11_api_state x11_state;

	x11_input_device(running_machine &machine, const char* name, input_device_class devclass, input_module &module)
		: event_based_device(machine, name, devclass, module),
			x11_state({0})
	{
	}
};

//============================================================
//  x11_lightgun_device
//============================================================

class x11_lightgun_device : public x11_input_device
{
public:
	lightgun_state lightgun;

	x11_lightgun_device(running_machine &machine, const char* name, input_module &module)
		: x11_input_device(machine, name, DEVICE_CLASS_LIGHTGUN, module),
			lightgun({0})
	{
	}

	void process_event(XEvent &xevent) override
	{
		if (xevent.type == motion_type)
		{
			XDeviceMotionEvent *motion = (XDeviceMotionEvent *)&xevent;
			print_motion_event(motion);

			/*
			* We have to check with axis will start on array index 0.
			* We have also to check the number of axes that are stored in the array.
			*/
			switch (motion->first_axis)
			{
				/*
				* Starting with x, check number of axis, if there is also the y axis stored.
				*/
			case 0:
				if (motion->axes_count >= 1)
				{
					lightgun.lX = normalize_absolute_axis(motion->axis_data[0], x11_state.minx, x11_state.maxx);
					if (motion->axes_count >= 2)
					{
						lightgun.lY = normalize_absolute_axis(motion->axis_data[1], x11_state.miny, x11_state.maxy);
					}
				}
				break;

				/*
				* Starting with y, ...
				*/
			case 1:
				if (motion->axes_count >= 1)
				{
					lightgun.lY = normalize_absolute_axis(motion->axis_data[0], x11_state.miny, x11_state.maxy);
				}
				break;
			}
		}
		else if (xevent.type == button_press_type || xevent.type == button_release_type)
		{
			XDeviceButtonEvent *button = (XDeviceButtonEvent *)&xevent;
			lightgun.buttons[button->button] = (xevent.type == button_press_type) ? 0x80 : 0;
		}
	}

	void reset() override
	{
		memset(&lightgun, 0, sizeof(lightgun));
	}
};

//============================================================
//  x11_lightgun_module
//============================================================

class x11_lightgun_module : public input_module_base, public x11_event_handler
{
private:
	device_map_t   m_lightgun_map;
	Display *      m_display;
public:
	x11_lightgun_module()
		: input_module_base(OSD_LIGHTGUNINPUT_PROVIDER, "x11")
	{
	}

	void input_init(running_machine &machine) override
	{
		int index;

		osd_printf_verbose("Lightgun: Begin initialization\n");

		devmap_init(machine, &m_lightgun_map, SDLOPTION_LIGHTGUNINDEX, 8, "Lightgun mapping");

		x11_event_manager::instance().initialize();
		m_display = x11_event_manager::instance().display();

		// Loop through all 8 possible devices
		for (index = 0; index < 8; index++)
		{
			XDeviceInfo *info;

			// Skip if the name is empty
			if (m_lightgun_map.map[index].name.length() == 0)
				continue;

			x11_lightgun_device *devinfo;
			std::string const &name = m_lightgun_map.map[index].name;
			char defname[512];

			// Register and add the device
			devinfo = create_lightgun_device(machine, index);
			osd_printf_verbose("%i: %s\n", index, name.c_str());

			// Find the device info associated with the name
			info = find_device_info(m_display, name.c_str(), 0);

			// If we couldn't find the device, skip
			if (info == nullptr)
			{
				osd_printf_verbose("Can't find device %s!\n", name.c_str());
				continue;
			}

			//Grab device info and translate to stuff mame can use
			if (info->num_classes > 0)
			{
				// Add the lightgun buttons based on what we read
				add_lightgun_buttons((XAnyClassPtr)info->inputclassinfo, info->num_classes, devinfo);

				// Also, set the axix min/max ranges if we got them
				set_lightgun_axis_props((XAnyClassPtr)info->inputclassinfo, info->num_classes, devinfo);
			}

			// Add X and Y axis
			sprintf(defname, "X %s", devinfo->name());
			devinfo->device()->add_item(defname, ITEM_ID_XAXIS, generic_axis_get_state, &devinfo->lightgun.lX);

			sprintf(defname, "Y %s", devinfo->name());
			devinfo->device()->add_item(defname, ITEM_ID_YAXIS, generic_axis_get_state, &devinfo->lightgun.lY);

			// Save the device id
			devinfo->x11_state.deviceid = info->id;

			// Register this device to receive event notifications
			int events_registered = register_events(m_display, info, m_lightgun_map.map[index].name.c_str(), 0);
			osd_printf_verbose("Device %i: Registered %i events.\n", (int)info->id, events_registered);

			// register ourself to handle events from event manager
			int event_types[] = { motion_type, button_press_type, button_release_type };
			osd_printf_verbose("Events types to register: motion:%d, press:%d, release:%d\n", motion_type, button_press_type, button_release_type);
			x11_event_manager::instance().subscribe(event_types, ARRAY_LENGTH(event_types), this);
		}

		osd_printf_verbose("Lightgun: End initialization\n");
	}

	bool should_poll_devices(running_machine &machine) override
	{
		return sdl_event_manager::instance().has_focus();
	}

	void before_poll(running_machine &machine) override
	{
		if (!should_poll_devices(machine))
			return;

		// Tell the event manager to process events and push them to the devices
		x11_event_manager::instance().process_events(machine);

		// Also trigger the SDL event manager so it can process window events
		sdl_event_manager::instance().process_events(machine);
	}

	void handle_event(XEvent &xevent) override
	{
		for (int i = 0; i < devicelist()->size(); i++)
		{
			downcast<x11_input_device*>(devicelist()->at(i))->queue_events(&xevent, 1);
		}
	}

private:
	x11_lightgun_device* create_lightgun_device(running_machine &machine, int index)
	{
		x11_lightgun_device *devinfo = nullptr;
		char tempname[20];

		if (m_lightgun_map.map[index].name.length() == 0)
		{
			if (m_lightgun_map.initialized)
			{
				snprintf(tempname, ARRAY_LENGTH(tempname), "NC%d", index);
				devinfo = devicelist()->create_device<x11_lightgun_device>(machine, tempname, *this);
			}

			return nullptr;
		}
		else
		{
			devinfo = devicelist()->create_device<x11_lightgun_device>(machine, m_lightgun_map.map[index].name.c_str(), *this);
		}

		return devinfo;
	}

	void add_lightgun_buttons(XAnyClassPtr first_info_class, int num_classes, x11_lightgun_device *devinfo)
	{
		XAnyClassPtr any = first_info_class;

		for (int i = 0; i < num_classes; i++)
		{
			switch (any->c_class)
			{
			case ButtonClass:
				XButtonInfoPtr b = (XButtonInfoPtr)any;
				for (int button = 0; button < b->num_buttons; button++)
				{
					input_item_id itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
					devinfo->device()->add_item(default_button_name(button), itemid, generic_button_get_state, &devinfo->lightgun.buttons[button]);
				}
				break;
			}

			any = (XAnyClassPtr)((char *)any + any->length);
		}
	}

	void set_lightgun_axis_props(XAnyClassPtr first_info_class, int num_classes, x11_lightgun_device *devinfo)
	{
		XAnyClassPtr any = first_info_class;

		for (int i = 0; i < num_classes; i++)
		{
			switch (any->c_class)
			{
			case ValuatorClass:
				XValuatorInfoPtr valuator_info = (XValuatorInfoPtr)any;
				XAxisInfoPtr axis_info = (XAxisInfoPtr)((char *)valuator_info + sizeof(XValuatorInfo));
				for (int j = 0; j < valuator_info->num_axes; j++, axis_info++)
				{
					if (j == 0)
					{
						XI_DBG("Set minx=%d, maxx=%d\n", axis_info->min_value, axis_info->max_value);
						devinfo->x11_state.maxx = axis_info->max_value;
						devinfo->x11_state.minx = axis_info->min_value;
					}

					if (j == 1)
					{
						XI_DBG("Set miny=%d, maxy=%d\n", axis_info->min_value, axis_info->max_value);
						devinfo->x11_state.maxy = axis_info->max_value;
						devinfo->x11_state.miny = axis_info->min_value;
					}
				}
				break;
			}

			any = (XAnyClassPtr)((char *)any + any->length);
		}
	}
};

#else
MODULE_NOT_SUPPORTED(x11_lightgun_module, OSD_LIGHTGUNINPUT_PROVIDER, "x11")
#endif

MODULE_DEFINITION(LIGHTGUN_X11, x11_lightgun_module)
