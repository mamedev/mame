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

#include "input_common.h"

#include "osdsdl.h"

// MAME headers
#include "inpttype.h"

// standard SDL header
#include <SDL2/SDL.h>

// for X11 xinput
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>


namespace osd {

namespace {

#define MAX_DEVMAP_ENTRIES  16

#define INVALID_EVENT_TYPE     -1
static int motion_type         = INVALID_EVENT_TYPE;
static int button_press_type   = INVALID_EVENT_TYPE;
static int button_release_type = INVALID_EVENT_TYPE;
static int key_press_type      = INVALID_EVENT_TYPE;
static int key_release_type    = INVALID_EVENT_TYPE;
static int proximity_in_type   = INVALID_EVENT_TYPE;
static int proximity_out_type  = INVALID_EVENT_TYPE;

//============================================================
//  DEBUG MACROS
//============================================================

#if defined(XINPUT_DEBUG) && XINPUT_DEBUG
#define XI_DBG(format, ...) osd_printf_verbose(format, __VA_ARGS__)

#define print_motion_event(motion) print_motion_event_impl(motion)
inline void print_motion_event_impl(XDeviceMotionEvent const *motion)
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
			motion->axis_data[0], motion->axis_data[1], motion->axis_data[2], motion->axis_data[3], motion->axis_data[4], motion->axis_data[5]);
}
#else
#define XI_DBG(format, ...) do { } while (false)
#define print_motion_event(motion) do { } while (false)
#endif


inline std::string remove_spaces(const char *s)
{
	std::string output(s);
	output.erase(std::remove_if(output.begin(), output.end(), isspace), output.end());
	return output;
}


//============================================================
//  lightgun helpers: copy-past from xinfo
//============================================================

XDeviceInfo*
find_device_info(Display    *display,
			const char       *name,
			bool       only_extended)
{
	XDeviceInfo *devices;
	XDeviceInfo *found = nullptr;
	int     loop;
	int     num_devices;
	int     len = strlen(name);
	bool    is_id = true;
	XID     id = static_cast<XID>(-1);

	for (loop = 0; loop < len; loop++)
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

	for (loop = 0; loop < num_devices; loop++)
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
int
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


struct device_map
{
	static inline constexpr unsigned MAX_ENTRIES = 16;

	struct {
		std::string    name;
		int            physical;
	} map[MAX_ENTRIES];
	int     logical[MAX_ENTRIES];
	int     initialized;

	void init(osd_options const &options, const char *opt, int max_devices, const char *label)
	{
		// initialize based on an input option prefix and max number of devices
		char defname[20];

		// The max devices the user specified, better not be bigger than the max the arrays can old
		assert(max_devices <= MAX_ENTRIES);

		// Initialize the map to default uninitialized values
		for (int dev = 0; dev < MAX_ENTRIES; dev++)
		{
			map[dev].name.clear();
			map[dev].physical = -1;
			logical[dev] = -1;
		}
		initialized = 0;

		// populate the device map up to the max number of devices
		for (int dev = 0; dev < max_devices; dev++)
		{
			const char *dev_name;

			// derive the parameter name from the option name and index. For instance: lightgun_index1 to lightgun_index8
			sprintf(defname, "%s%d", opt, dev + 1);

			// Get the user-specified name that matches the parameter
			dev_name = options.value(defname);

			// If they've specified a name and it's not "auto", treat it as a custom mapping
			if (dev_name && *dev_name && strcmp(dev_name, OSDOPTVAL_AUTO))
			{
				// remove the spaces from the name store it in the index
				map[dev].name = remove_spaces(dev_name);
				osd_printf_verbose("%s: Logical id %d: %s\n", label, dev + 1, map[dev].name);
				initialized = 1;
			}
		}
	}
};


//============================================================
//  x11_event_manager
//============================================================

class x11_event_manager : public event_subscription_manager<XEvent, int>
{
private:
	struct x_cleanup
	{
		void operator()(Display *ptr) const
		{
			if (ptr)
				XCloseDisplay(ptr);
		}
		void operator()(XExtensionVersion *ptr) const
		{
			if (ptr)
				XFree(ptr);
		}
	};

	template <typename T> using x_ptr = std::unique_ptr<T, x_cleanup>;

	x_ptr<Display> m_display;

	x11_event_manager() = default;

public:
	Display *display() const { return m_display.get(); }

	static x11_event_manager &instance()
	{
		static x11_event_manager s_instance;
		return s_instance;
	}

	int initialize()
	{
		std::lock_guard<std::mutex> scope_lock(subscription_mutex());

		if (m_display)
			return 0;

		m_display.reset(XOpenDisplay(nullptr));
		if (!m_display)
		{
			osd_printf_verbose("Unable to connect to X server\n");
			return -1;
		}

		x_ptr<XExtensionVersion> version(XGetExtensionVersion(m_display.get(), INAME));
		if (!version || (version.get() == reinterpret_cast<XExtensionVersion *>(NoSuchExtension)))
		{
			osd_printf_verbose("xinput extension not available!\n");
			return -1;
		}

		return 0;
	}

	void process_events()
	{
		std::lock_guard<std::mutex> scope_lock(subscription_mutex());

		// If X11 has become invalid for some reason, XPending will crash. Assert instead.
		assert(m_display);

		// Get XInput events
		while (XPending(m_display.get()) != 0)
		{
			XEvent event;
			XNextEvent(m_display.get(), &event);
			dispatch_event(event.type, event);
		}
	}
};


//============================================================
//  x11_input_device
//============================================================

class x11_input_device : public event_based_device<XEvent>
{
public:
	x11_input_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			XDeviceInfo const *info) :
		event_based_device(std::move(name), std::move(id), module),
		m_device_id(info ? info->id : 0)
	{
	}

	XID const &device_id() const { return m_device_id; }

protected:
	XID const m_device_id; // X11 device ID
};


//============================================================
//  x11_lightgun_device
//============================================================

class x11_lightgun_device : public x11_input_device
{
public:
	x11_lightgun_device(
			std::string &&name,
			std::string &&id,
			input_module
			&module,
			XDeviceInfo *info) :
		x11_input_device(std::move(name), std::move(id), module, info),
		m_axis_count(0),
		m_button_count(0),
		m_maxx(0),
		m_maxy(0),
		m_minx(0),
		m_miny(0),
		m_lightgun({ 0 })
	{
		if (info && (info->num_classes > 0))
		{
			// Grab device info and translate to stuff MAME can use
			XAnyClassPtr any = static_cast<XAnyClassPtr>(info->inputclassinfo);
			for (int i = 0; i < info->num_classes; i++)
			{
				switch (any->c_class)
				{
				// Set the axis min/max ranges if we got them
				case ValuatorClass:
					{
						auto const valuator_info = reinterpret_cast<XValuatorInfoPtr>(any);
						auto axis_info = reinterpret_cast<XAxisInfoPtr>(reinterpret_cast<char *>(valuator_info) + sizeof(XValuatorInfo));
						for (int j = 0; (j < valuator_info->num_axes) && (j < 2); j++, axis_info++)
						{
							if (j == 0)
							{
								XI_DBG("Set minx=%d, maxx=%d\n", axis_info->min_value, axis_info->max_value);
								m_axis_count = 1;
								m_maxx = axis_info->max_value;
								m_minx = axis_info->min_value;
							}

							if (j == 1)
							{
								XI_DBG("Set miny=%d, maxy=%d\n", axis_info->min_value, axis_info->max_value);
								m_axis_count = 2;
								m_maxy = axis_info->max_value;
								m_miny = axis_info->min_value;
							}
						}
					}
					break;

				// Count the lightgun buttons based on what we read
				case ButtonClass:
					{
						XButtonInfoPtr b = reinterpret_cast<XButtonInfoPtr>(any);
						if (b->num_buttons < 0)
							m_button_count = 0;
						else if (b->num_buttons <= MAX_BUTTONS)
							m_button_count = b->num_buttons;
						else
							m_button_count = MAX_BUTTONS;
					}
					break;
				}

				any = reinterpret_cast<XAnyClassPtr>(reinterpret_cast<char *>(any) + any->length);
			}
		}
	}

	virtual void reset() override
	{
		memset(&m_lightgun, 0, sizeof(m_lightgun));
	}

	virtual void configure(input_device &device) override
	{
		// Add buttons
		for (int button = 0; button < m_button_count; button++)
		{
			input_item_id const itemid = input_item_id(ITEM_ID_BUTTON1 + button);
			device.add_item(default_button_name(button), std::string_view(), itemid, generic_button_get_state<std::int32_t>, &m_lightgun.buttons[button]);
		}

		// Add X and Y axis
		if (1 <= m_axis_count)
			device.add_item("X", std::string_view(), ITEM_ID_XAXIS, generic_axis_get_state<std::int32_t>, &m_lightgun.lX);
		if (2 <= m_axis_count)
			device.add_item("Y", std::string_view(), ITEM_ID_YAXIS, generic_axis_get_state<std::int32_t>, &m_lightgun.lY);
	}

	virtual void process_event(XEvent const &xevent) override
	{
		if (xevent.type == motion_type)
		{
			auto const motion = reinterpret_cast<XDeviceMotionEvent const *>(&xevent);
			print_motion_event(motion);

			// We have to check with axis will start on array index 0.
			// We also have to check the number of axes that are stored in the array.
			switch (motion->first_axis)
			{
			// Starting with x, check number of axes, if there is also the y axis stored.
			case 0:
				if (motion->axes_count >= 1)
					m_lightgun.lX = normalize_absolute_axis(motion->axis_data[0], m_minx, m_maxx);
				if (motion->axes_count >= 2)
					m_lightgun.lY = normalize_absolute_axis(motion->axis_data[1], m_miny, m_maxy);
				break;

			// Starting with y, ...
			case 1:
				if (motion->axes_count >= 1)
					m_lightgun.lY = normalize_absolute_axis(motion->axis_data[0], m_miny, m_maxy);
				break;
			}
		}
		else if (xevent.type == button_press_type || xevent.type == button_release_type)
		{
			auto const button = reinterpret_cast<XDeviceButtonEvent const *>(&xevent);

			/*
			 * SDL/X11 Number the buttons 1,2,3, while windows and other parts of MAME
			 * like offscreen_reload expect 0,2,1. Transpose buttons 2 and 3, and then
			 * -1 the button number to align the numbering schemes.
			*/
			int button_number = button->button;
			if (button_number <= MAX_BUTTONS)
			{
				switch (button_number)
				{
				case 2:
				case 3:
					button_number ^= 1;
					break;
				}
				m_lightgun.buttons[button_number - 1] = (xevent.type == button_press_type) ? 0x80 : 0;
			}
		}
	}

private:
	struct lightgun_state
	{
		int32_t lX, lY;
		int32_t buttons[MAX_BUTTONS];
	};

	int m_axis_count;
	int m_button_count;
	int32_t m_maxx;
	int32_t m_maxy;
	int32_t m_minx;
	int32_t m_miny;
	lightgun_state m_lightgun;
};


//============================================================
//  x11_lightgun_module
//============================================================

class x11_lightgun_module : public input_module_impl<x11_input_device, osd_common_t>, public x11_event_manager::subscriber
{
private:
	device_map m_lightgun_map;
	Display *m_display;

public:
	x11_lightgun_module() :
		input_module_impl<x11_input_device, osd_common_t>(OSD_LIGHTGUNINPUT_PROVIDER, "x11"),
		m_display(nullptr)
	{
	}

	virtual bool probe() override
	{
		// If there is no X server, X11 lightguns cannot be supported
		Display *const display = XOpenDisplay(nullptr);
		if (!display)
			return false;
		XCloseDisplay(display);

		return true;
	}

	virtual int init(osd_interface &osd, osd_options const &options) override
	{
		// If the X server has become invalid, a crash can occur
		x11_event_manager::instance().initialize();
		m_display = x11_event_manager::instance().display();
		if (!m_display)
			return -1;

		return input_module_impl<x11_input_device, osd_common_t>::init(osd, options);
	}

	virtual void input_init(running_machine &machine) override
	{
		assert(m_display);

		input_module_impl<x11_input_device, osd_common_t>::input_init(machine);

		osd_printf_verbose("Lightgun: Begin initialization\n");

		m_lightgun_map.init(*options(), SDLOPTION_LIGHTGUNINDEX, 8, "Lightgun mapping");

		// Loop through all 8 possible devices
		for (int index = 0; index < 8; index++)
		{
			// Skip if the name is empty
			if (m_lightgun_map.map[index].name.empty())
				continue;

			// Find the device info associated with the name
			std::string const &name = m_lightgun_map.map[index].name;
			osd_printf_verbose("%i: %s\n", index, name);
			XDeviceInfo *const info = find_device_info(m_display, name.c_str(), 0);
			if (!info)
				osd_printf_verbose("Lightgun: Can't find device %s!\n", name);

			// previously had code to use "NC%d" format if name was empty
			// but that couldn't happen because device creation would be skipped

			// Register and add the device
			create_device<x11_lightgun_device>(
					DEVICE_CLASS_LIGHTGUN,
					std::string(name),
					std::string(name),
					info);

			// Register this device to receive event notifications
			if (info)
			{
				int const events_registered = register_events(m_display, info, name.c_str(), 0);
				osd_printf_verbose("Device %i: Registered %i events.\n", int(info->id), events_registered);
			}
		}

		// register ourself to handle events from event manager
		int const event_types[] = { motion_type, button_press_type, button_release_type };
		osd_printf_verbose("Events types to register: motion:%d, press:%d, release:%d\n", motion_type, button_press_type, button_release_type);
		subscribe(x11_event_manager::instance(), event_types);

		osd_printf_verbose("Lightgun: End initialization\n");
	}

	virtual void exit() override
	{
		// unsubscribe from events
		unsubscribe();

		input_module_impl<x11_input_device, osd_common_t>::exit();
	}

	virtual bool should_poll_devices() override
	{
		return osd().has_focus();
	}

	virtual void before_poll() override
	{
		// trigger the SDL event manager so it can process window events
		input_module_impl<x11_input_device, osd_common_t>::before_poll();

		// Tell the event manager to process events and push them to the devices
		if (should_poll_devices())
			x11_event_manager::instance().process_events();
	}

	virtual void handle_event(XEvent const &xevent) override
	{
		XID deviceid;
		if (xevent.type == motion_type)
		{
			auto const motion = reinterpret_cast<XDeviceMotionEvent const *>(&xevent);
			deviceid = motion->deviceid;
		}
		else if (xevent.type == button_press_type || xevent.type == button_release_type)
		{
			auto const button = reinterpret_cast<XDeviceButtonEvent const *>(&xevent);
			deviceid = button->deviceid;
		}
		else
		{
			return;
		}

		// Figure out which lightgun this event id destined for
		auto target_device = std::find_if(
				devicelist().begin(),
				devicelist().end(),
				[&deviceid] (auto &device) { return device->device_id() == deviceid; });

		// If we find a matching lightgun, dispatch the event to the lightgun
		if (target_device != devicelist().end())
			(*target_device)->queue_events(&xevent, 1);
	}
};

} // anonymous namespace

} // namespace osd


#else // defined(SDLMAME_SDL2) && !defined(SDLMAME_WIN32) && defined(USE_XINPUT) && USE_XINPUT

namespace osd { namespace { MODULE_NOT_SUPPORTED(x11_lightgun_module, OSD_LIGHTGUNINPUT_PROVIDER, "x11") } }

#endif //  defined(SDLMAME_SDL2) && !defined(SDLMAME_WIN32) && defined(USE_XINPUT) && USE_XINPUT


MODULE_DEFINITION(LIGHTGUN_X11, osd::x11_lightgun_module)
