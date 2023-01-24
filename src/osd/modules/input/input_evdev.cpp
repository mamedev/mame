// license:BSD-3-Clause
// copyright-holders:Marc A. Pelletier
//============================================================
//
//  input_evdev.cpp - Linux evdev input module
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if (USE_EVDEV)

#include <cctype>
#include <cstddef>
#include <mutex>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

// MAME headers
#include "emu.h"
#include "osdepend.h"

// MAMEOS headers
#include "../lib/osdobj_common.h"
#include "input_common.h"

#include <linux/version.h>
#include <linux/input.h>

#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <poll.h>

namespace {


struct idmap {
	int evdev_id;
	input_item_id mame_id;
	const char* name;
};


idmap key_map[] = {
	{ KEY_ESC,		ITEM_ID_ESC,		"ESC" },
	{ KEY_1,		ITEM_ID_1,		"1" },
	{ KEY_2,		ITEM_ID_2,		"2" },
	{ KEY_3,		ITEM_ID_3,		"3" },
	{ KEY_4,		ITEM_ID_4,		"4" },
	{ KEY_5,		ITEM_ID_5,		"5" },
	{ KEY_6,		ITEM_ID_6,		"6" },
	{ KEY_7,		ITEM_ID_7,		"7" },
	{ KEY_8,		ITEM_ID_8,		"8" },
	{ KEY_9,		ITEM_ID_9,		"9" },
	{ KEY_0,		ITEM_ID_0,		"0" },
	{ KEY_MINUS,		ITEM_ID_MINUS,		"MINUS" },
	{ KEY_EQUAL,		ITEM_ID_EQUALS,		"EQUAL" },
	{ KEY_BACKSPACE,	ITEM_ID_BACKSPACE,	"BACKSPACE" },
	{ KEY_TAB,		ITEM_ID_TAB,		"TAB" },
	{ KEY_Q,		ITEM_ID_Q,		"Q" },
	{ KEY_W,		ITEM_ID_W,		"W" },
	{ KEY_E,		ITEM_ID_E,		"E" },
	{ KEY_R,		ITEM_ID_R,		"R" },
	{ KEY_T,		ITEM_ID_T,		"T" },
	{ KEY_Y,		ITEM_ID_Y,		"Y" },
	{ KEY_U,		ITEM_ID_U,		"U" },
	{ KEY_I,		ITEM_ID_I,		"I" },
	{ KEY_O,		ITEM_ID_O,		"O" },
	{ KEY_P,		ITEM_ID_P,		"P" },
	{ KEY_LEFTBRACE,	ITEM_ID_OPENBRACE,	"OPENBRACE" },
	{ KEY_RIGHTBRACE,	ITEM_ID_CLOSEBRACE,	"CLOSEBRACE" },
	{ KEY_ENTER,		ITEM_ID_ENTER,		"ENTER" },
	{ KEY_LEFTCTRL,		ITEM_ID_LCONTROL,	"LCONTROL" },
	{ KEY_A,		ITEM_ID_A,		"A" },
	{ KEY_S,		ITEM_ID_S,		"S" },
	{ KEY_D,		ITEM_ID_D,		"D" },
	{ KEY_F,		ITEM_ID_F,		"F" },
	{ KEY_G,		ITEM_ID_G,		"G" },
	{ KEY_H,		ITEM_ID_H,		"H" },
	{ KEY_J,		ITEM_ID_J,		"J" },
	{ KEY_K,		ITEM_ID_K,		"K" },
	{ KEY_L,		ITEM_ID_L,		"L" },
	{ KEY_SEMICOLON,	ITEM_ID_COLON,		"COLON" },
	{ KEY_APOSTROPHE,	ITEM_ID_QUOTE,		"QUOTE" },
	{ KEY_GRAVE,		ITEM_ID_TILDE,		"TILDE" },
	{ KEY_LEFTSHIFT,	ITEM_ID_LSHIFT,		"LSHIFT" },
	{ KEY_BACKSLASH,	ITEM_ID_BACKSLASH,	"BACKSLASH" },
	{ KEY_Z,		ITEM_ID_Z,		"Z" },
	{ KEY_X,		ITEM_ID_X,		"X" },
	{ KEY_C,		ITEM_ID_C,		"C" },
	{ KEY_V,		ITEM_ID_V,		"V" },
	{ KEY_B,		ITEM_ID_B,		"B" },
	{ KEY_N,		ITEM_ID_N,		"N" },
	{ KEY_M,		ITEM_ID_M,		"M" },
	{ KEY_COMMA,		ITEM_ID_COMMA,		"COMMA" },
	{ KEY_DOT,		ITEM_ID_STOP,		"STOP" },
	{ KEY_SLASH,		ITEM_ID_SLASH,		"SLASH" },
	{ KEY_RIGHTSHIFT,	ITEM_ID_RSHIFT,		"RSHIFT" },
	{ KEY_KPASTERISK,	ITEM_ID_ASTERISK,	"ASTERISK" },
	{ KEY_LEFTALT,		ITEM_ID_LALT,		"LALT" },
	{ KEY_SPACE,		ITEM_ID_SPACE,		"SPACE" },
	{ KEY_CAPSLOCK,		ITEM_ID_CAPSLOCK,	"CAPSLOCK" },
	{ KEY_F1,		ITEM_ID_F1,		"F1" },
	{ KEY_F2,		ITEM_ID_F2,		"F2" },
	{ KEY_F3,		ITEM_ID_F3,		"F3" },
	{ KEY_F4,		ITEM_ID_F4,		"F4" },
	{ KEY_F5,		ITEM_ID_F5,		"F5" },
	{ KEY_F6,		ITEM_ID_F6,		"F6" },
	{ KEY_F7,		ITEM_ID_F7,		"F7" },
	{ KEY_F8,		ITEM_ID_F8,		"F8" },
	{ KEY_F9,		ITEM_ID_F9,		"F9" },
	{ KEY_F10,		ITEM_ID_F10,		"F10" },
	{ KEY_NUMLOCK,		ITEM_ID_NUMLOCK,	"NUMLOCK" },
	{ KEY_SCROLLLOCK,	ITEM_ID_SCRLOCK,	"SCRLOCK" },
	{ KEY_KP7,		ITEM_ID_7_PAD,		"KP 7" },
	{ KEY_KP8,		ITEM_ID_8_PAD,		"KP 8" },
	{ KEY_KP9,		ITEM_ID_9_PAD,		"KP 9" },
	{ KEY_KPMINUS,		ITEM_ID_MINUS_PAD,	"KP MINUS" },
	{ KEY_KP4,		ITEM_ID_4_PAD,		"KP 4" },
	{ KEY_KP5,		ITEM_ID_5_PAD,		"KP 5" },
	{ KEY_KP6,		ITEM_ID_6_PAD,		"KP 6" },
	{ KEY_KPPLUS,		ITEM_ID_PLUS_PAD,	"KP PLUS" },
	{ KEY_KP1,		ITEM_ID_1_PAD,		"KP 1" },
	{ KEY_KP2,		ITEM_ID_2_PAD,		"KP 2" },
	{ KEY_KP3,		ITEM_ID_3_PAD,		"KP 3" },
	{ KEY_KP0,		ITEM_ID_0_PAD,		"KP 0" },
	{ KEY_KPDOT,		ITEM_ID_DEL_PAD,	"KP DEL" },
	{ KEY_F11,		ITEM_ID_F11,		"F11" },
	{ KEY_F12,		ITEM_ID_F12,		"F12" },
	{ KEY_KPENTER,		ITEM_ID_ENTER_PAD,	"KP ENTER" },
	{ KEY_RIGHTCTRL,	ITEM_ID_RCONTROL,	"RCONTROL" },
	{ KEY_KPSLASH,		ITEM_ID_SLASH_PAD,	"KP SLASH" },
	{ KEY_SYSRQ,		ITEM_ID_PRTSCR,		"PRTSCR" },
	{ KEY_RIGHTALT,		ITEM_ID_RALT,		"RALT" },
	{ KEY_HOME,		ITEM_ID_HOME,		"HOME" },
	{ KEY_UP,		ITEM_ID_UP,		"UP" },
	{ KEY_PAGEUP,		ITEM_ID_PGUP,		"PGUP" },
	{ KEY_LEFT,		ITEM_ID_LEFT,		"LEFT" },
	{ KEY_RIGHT,		ITEM_ID_RIGHT,		"RIGHT" },
	{ KEY_END,		ITEM_ID_END,		"END" },
	{ KEY_DOWN,		ITEM_ID_DOWN,		"DOWN" },
	{ KEY_PAGEDOWN,		ITEM_ID_PGDN,		"PGDN" },
	{ KEY_INSERT,		ITEM_ID_INSERT,		"INSERT" },
	{ KEY_DELETE,		ITEM_ID_DEL,		"DEL" },
	{ KEY_KPEQUAL,		ITEM_ID_EQUALS_PAD,	"KP EQUALS" },
	{ KEY_PAUSE,		ITEM_ID_PAUSE,		"PAUSE" },
	{ KEY_LEFTMETA,		ITEM_ID_LWIN,		"LWIN" },
	{ KEY_RIGHTMETA,	ITEM_ID_RWIN,		"RWIN" },
	{ KEY_COMPOSE,		ITEM_ID_MENU,		"MENU" },
	{ KEY_F13,		ITEM_ID_F13,		"F13" },
	{ KEY_F14,		ITEM_ID_F14,		"F14" },
	{ KEY_F15,		ITEM_ID_F15,		"F15" },
	{ KEY_F16,		ITEM_ID_F16,		"F16" },
	{ KEY_F17,		ITEM_ID_F17,		"F17" },
	{ KEY_F18,		ITEM_ID_F18,		"F18" },
	{ KEY_F19,		ITEM_ID_F19,		"F19" },
	{ KEY_F20,		ITEM_ID_F20,		"F20" },

	{ BTN_LEFT,		ITEM_ID_INVALID,	"LEFT" },
	{ BTN_RIGHT,		ITEM_ID_INVALID,	"RIGHT" },
	{ BTN_MIDDLE,		ITEM_ID_INVALID,	"MIDDLE" },
	{ BTN_SIDE,		ITEM_ID_INVALID,	"SIDE" },
	{ BTN_EXTRA,		ITEM_ID_INVALID,	"EXTRA" },
	{ BTN_FORWARD,		ITEM_ID_INVALID,	"FORWARD" },
	{ BTN_BACK,		ITEM_ID_INVALID,	"BACK" },
	{ BTN_TASK,		ITEM_ID_INVALID,	"TASK" },
	{ BTN_TRIGGER,		ITEM_ID_INVALID,	"TRIGGER" },
	{ BTN_THUMB,		ITEM_ID_INVALID,	"THUMB" },
	{ BTN_THUMB2,		ITEM_ID_INVALID,	"THUMB2" },
	{ BTN_TOP,		ITEM_ID_INVALID,	"TOP" },
	{ BTN_TOP2,		ITEM_ID_INVALID,	"TOP2" },
	{ BTN_PINKIE,		ITEM_ID_INVALID,	"PINKIE" },
	{ BTN_BASE,		ITEM_ID_INVALID,	"BASE" },
	{ BTN_BASE2,		ITEM_ID_INVALID,	"BASE2" },
	{ BTN_BASE3,		ITEM_ID_INVALID,	"BASE3" },
	{ BTN_BASE4,		ITEM_ID_INVALID,	"BASE4" },
	{ BTN_BASE5,		ITEM_ID_INVALID,	"BASE5" },
	{ BTN_BASE6,		ITEM_ID_INVALID,	"BASE6" },
	{ BTN_DEAD,		ITEM_ID_INVALID,	"DEAD" },
	{ BTN_SOUTH,		ITEM_ID_INVALID,	"A" },
	{ BTN_EAST,		ITEM_ID_INVALID,	"B" },
	{ BTN_C,		ITEM_ID_INVALID,	"C" },
	{ BTN_NORTH,		ITEM_ID_INVALID,	"X" },
	{ BTN_WEST,		ITEM_ID_INVALID,	"Y" },
	{ BTN_Z,		ITEM_ID_INVALID,	"Z" },
	{ BTN_TL,		ITEM_ID_INVALID,	"LEFT TRIGGER" },
	{ BTN_TR,		ITEM_ID_INVALID,	"RIGHT TRIGGER" },
	{ BTN_TL2,		ITEM_ID_INVALID,	"LEFT TRIGGER 2" },
	{ BTN_TR2,		ITEM_ID_INVALID,	"RIGHT TRIGGER 2" },
	{ BTN_SELECT,		ITEM_ID_INVALID,	"SELECT" },
	{ BTN_START,		ITEM_ID_INVALID,	"START" },
	{ BTN_MODE,		ITEM_ID_INVALID,	"MODE" },
	{ BTN_THUMBL,		ITEM_ID_INVALID,	"LEFT THUMB" },
	{ BTN_THUMBR,		ITEM_ID_INVALID,	"RIGHT THUMB" },
	{ BTN_GEAR_DOWN,	ITEM_ID_INVALID,	"GEAR DOWN" },
	{ BTN_GEAR_UP,		ITEM_ID_INVALID,	"GEAR UP" },
	{ 0, ITEM_ID_INVALID, 0 },
};

idmap relaxis_map[] = {
	{ REL_X,		ITEM_ID_XAXIS,		"X" },
	{ REL_Y,		ITEM_ID_YAXIS,		"Y" },
	{ REL_Z,		ITEM_ID_ZAXIS,		"Z" },
	{ REL_RX,		ITEM_ID_RXAXIS,		"RX" },
	{ REL_RY,		ITEM_ID_RYAXIS,		"RY" },
	{ REL_RZ,		ITEM_ID_RZAXIS,		"RZ" },
	{ REL_HWHEEL,		ITEM_ID_ADD_RELATIVE1,	"HWHEEL" },
	{ REL_DIAL,		ITEM_ID_SLIDER2,	"DIAL" },
	{ REL_WHEEL,		ITEM_ID_SLIDER1,	"WHEEL" },
	{ REL_MISC,		ITEM_ID_ADD_RELATIVE2,	"EXTRA AXIS" },
	{ REL_WHEEL_HI_RES,	ITEM_ID_SLIDER1,	"WHEEL" },
	{ REL_HWHEEL_HI_RES,	ITEM_ID_ADD_RELATIVE1,	"HWHEEL" },
	{ 0, ITEM_ID_INVALID, 0 },
};

idmap absaxis_map[] = {
	{ ABS_X,		ITEM_ID_XAXIS,		"X" },
	{ ABS_Y,		ITEM_ID_YAXIS,		"Y" },
	{ ABS_Z,		ITEM_ID_ZAXIS,		"Z" },
	{ ABS_RX,		ITEM_ID_RXAXIS,		"RX" },
	{ ABS_RY,		ITEM_ID_RYAXIS,		"RY" },
	{ ABS_RZ,		ITEM_ID_RZAXIS,		"RZ" },
	{ ABS_THROTTLE,		ITEM_ID_SLIDER1,	"THROTTLE" },
	{ ABS_RUDDER,		ITEM_ID_SLIDER2,	"RUDDER" },
	{ ABS_WHEEL,		ITEM_ID_ADD_ABSOLUTE1,	"WHEEL" },
	{ ABS_GAS,		ITEM_ID_ADD_ABSOLUTE2,	"GAS" },
	{ ABS_BRAKE,		ITEM_ID_ADD_ABSOLUTE3,	"BRAKE" },
	{ ABS_HAT0X,		ITEM_ID_ADD_ABSOLUTE4,	"HAT1 X" },
	{ ABS_HAT0Y,		ITEM_ID_ADD_ABSOLUTE5,	"HAT1 Y" },
	{ ABS_HAT1X,		ITEM_ID_ADD_ABSOLUTE6,	"HAT2 X" },
	{ ABS_HAT1Y,		ITEM_ID_ADD_ABSOLUTE7,	"HAT2 Y" },
	{ ABS_HAT2X,		ITEM_ID_ADD_ABSOLUTE8,	"HAT3 X" },
	{ ABS_HAT2Y,		ITEM_ID_ADD_ABSOLUTE9,	"HAT3 Y" },
	{ ABS_HAT3X,		ITEM_ID_ADD_ABSOLUTE10,	"HAT4 X" },
	{ ABS_HAT3Y,		ITEM_ID_ADD_ABSOLUTE11,	"HAT4 Y" },
	{ ABS_PRESSURE,		ITEM_ID_ADD_ABSOLUTE12,	"PRESSURE" },
	{ ABS_DISTANCE,		ITEM_ID_ADD_ABSOLUTE13,	"DISTANCE" },
	{ ABS_TILT_X,		ITEM_ID_ADD_ABSOLUTE14,	"TILT X" },
	{ ABS_TILT_Y,		ITEM_ID_ADD_ABSOLUTE15,	"TILT Y" },
	{ ABS_TOOL_WIDTH,	ITEM_ID_ADD_ABSOLUTE16,	"WIDTH" },
	{ 0, ITEM_ID_INVALID, 0 },
};

enum device_type {
	EVDEV_NONE,
	EVDEV_KEYBOARD,
	EVDEV_MOUSE,
	EVDEV_JOYSTICK,
	EVDEV_LIGHTGUN,
};


constexpr size_t octets_for(unsigned int bits)
{
	return ((bits+7)>>3);
}

class evdev_device;
class evdev_input;

struct evdev_device_info
{
	std::string name;
	std::string id;
	mutable evdev_device* mapped;
	device_type type;
	unsigned char buttons[octets_for(KEY_MAX)];
	unsigned char relaxes[octets_for(REL_MAX)];
	unsigned char absaxes[octets_for(ABS_MAX)];

	evdev_device_info(): mapped(0), type(EVDEV_NONE)
	{
	}
};


class evdev_device: public device_info
{
	friend class evdev_input;
private:
	const evdev_device_info& info;
	int fd;

	// This is _way_ too conservative, but memory is cheap
	uint8_t	keystate[KEY_MAX];
	int32_t	relaxis[REL_MAX];
	struct {
		int32_t	normalized;
		input_absinfo absinfo;
	} absaxis[ABS_MAX];

public:
	evdev_device(running_machine& machine, std::string&& name, std::string&& id, input_module& module,
			input_device_class deviceclass, const evdev_device_info& deviceinfo):
		device_info(machine, std::move(name), std::move(id), deviceclass, module),
		info(deviceinfo)
	{
		fd = -1;
		reset();
	}

	~evdev_device()
	{
		info.mapped = 0;
		if(fd >= 0)
			close(fd);
	}

	void start(int dirfd)
	{
		int numkeys = 0;
		int numbuttons = 0;
		int mapi = 0;
		for(int i=1; i<KEY_MAX; i++)
			if(info.buttons[i>>3] & (1<<(i&7))) {
				while(key_map[mapi].evdev_id && key_map[mapi].evdev_id<i)
					mapi++;
				char name[32];
				input_item_id id = ITEM_ID_INVALID;
				name[0] = 0;
				if(key_map[mapi].evdev_id == i) {
					strncpy(name, key_map[mapi].name, 31);
					name[31] = 0;
					if((id = key_map[mapi].mame_id) != ITEM_ID_INVALID)
						numkeys++;
				} else {
					if(i<BTN_MISC)
						snprintf(name, 32, "KEYCODE %d", i);
					else
						snprintf(name, 32, "BUTTON %d", numbuttons+1);
				}
				if(id == ITEM_ID_INVALID && numbuttons<32)
					id = static_cast<input_item_id>(ITEM_ID_BUTTON1 + numbuttons++);
				if(id != ITEM_ID_INVALID) {
					device()->add_item(name, id, generic_button_get_state<uint8_t>, keystate+i);
				}
			}

		if(fd < 0) {
			fd = openat(dirfd, info.id.c_str(), O_RDONLY);
			if(fd < 0) {
				osd_printf_warning("evdev: unable to open %s: %s\n", info.id.c_str(), strerror(errno));
			}
		}

		mapi = 0;
		for(int i=0; i<REL_MAX; i++)
			if(info.relaxes[i>>3] & (1<<(i&7))) {
				while((mapi==0 || relaxis_map[mapi].evdev_id) && relaxis_map[mapi].evdev_id<i)
					mapi++;
				if(relaxis_map[mapi].evdev_id == i) {
					device()->add_item(relaxis_map[mapi].name, relaxis_map[mapi].mame_id,
						generic_axis_get_state<int32_t>, relaxis+i);
				}
			}

		mapi = 0;
		for(int i=0; i<ABS_MAX; i++)
			if(info.absaxes[i>>3] & (1<<(i&7))) {
				while((mapi==0 || absaxis_map[mapi].evdev_id) && absaxis_map[mapi].evdev_id<i)
					mapi++;
				if(absaxis_map[mapi].evdev_id == i) {
					if(fd >= 0)
						ioctl(fd, EVIOCGABS(i), &absaxis[i].absinfo);
					device()->add_item(absaxis_map[mapi].name, absaxis_map[mapi].mame_id,
						generic_axis_get_state<int32_t>, &absaxis[i].normalized);
				}
			}

		info.mapped = this;
	}

	void fail(void)
	{
		osd_printf_warning("evdev: unable to read from device %s, disabling\n", info.id.c_str());
		if(fd >= 0) {
			close(fd);
			fd = -1;
		}
		info.mapped = 0;
	}

	void process(const input_event* events, size_t num_events)
	{
		for(int i=0; i<num_events; i++) {
			switch(events[i].type) {
				case EV_KEY:
					keystate[events[i].code] = (events[i].value>0)? 0x80: 0;
					break;
				case EV_REL:
					relaxis[events[i].code] += events[i].value;
					break;
				case EV_ABS: {
						auto& axis = absaxis[events[i].code];
						axis.normalized = normalize_absolute_axis(events[i].value, axis.absinfo.minimum, axis.absinfo.maximum);
					}
					break;
			}
		}
	}

	void poll() override
	{
	}

	void reset() override
	{
		std::fill(std::begin(keystate), std::end(keystate), 0);
		std::fill(std::begin(relaxis), std::end(relaxis), 0);
		for(int i=0; i<ABS_MAX; i++)
			absaxis[i].normalized = 0;
	}
};

#define MAX_EVDEV_DEVICES 64

class evdev_input
{
private:
	bool initialized;

public:
	std::vector<evdev_device_info> dev;
	int device_dir_fd;
	std::thread* reader_thread;
	int reader_thread_pipe[2];

public:
	evdev_input(): initialized(false), device_dir_fd(-1), reader_thread(0)
	{
	}

	~evdev_input()
	{
		if(initialized && reader_thread) {
			write(reader_thread_pipe[1], "x", 1);
			reader_thread->join();
		}
	}

	static void reader_function(evdev_input* self)
	{
		self->do_reader_function();
	}

	void do_reader_function()
	{
		osd_printf_verbose("evdev: start monitoring devices\n");
		std::vector<pollfd> poll_fd;
		std::vector<evdev_device_info*> devices;
		poll_fd.reserve(dev.size()+1);
		devices.reserve(dev.size()+1);

		for(;;) {
			poll_fd.clear();
			devices.clear();

			devices.emplace_back();
			poll_fd.emplace_back();
			pollfd& pfd = poll_fd.back();
			pfd.fd = reader_thread_pipe[0];
			pfd.events = POLLIN;

			for(auto& d : dev)
				if(d.mapped && d.mapped->fd>=0) {
					devices.emplace_back(&d);
					poll_fd.emplace_back();
					pollfd& pfd = poll_fd.back();
					pfd.fd = d.mapped->fd;
					pfd.events = POLLIN;
				}

			int result = poll(poll_fd.data(), poll_fd.size(), -1);
			if(result < 0) {
				osd_printf_warning("evdev: poll() failed: %s\n", strerror(errno));
				continue;
			}

			if(result > 0 && (poll_fd[0].revents & POLLIN)) {
				char octet;
				if(read(reader_thread_pipe[0], &octet, 1)==1) {
					if(octet == 'x')
						break;
					continue;
				}
			}

			if(result != 0) {
				input_event events[256];

				for(int i=1; i<poll_fd.size(); i++) {
					if(poll_fd[i].revents & (POLLERR|POLLHUP|POLLNVAL)) {
						// file descriptor went away, did someone yank the device?
						if(devices[i]->mapped)
							devices[i]->mapped->fail();
						continue;
					}
					if(poll_fd[i].revents & POLLIN) {
						// we try to reduce the number of system calls by reading
						// as many events as reasonable in one read() call
						ssize_t rdlen = read(poll_fd[i].fd, events, 256*sizeof(input_event));
						if(rdlen<0 && (errno==EWOULDBLOCK || errno==EAGAIN))
							continue;
						if(devices[i]->mapped) {
							if(rdlen < sizeof(input_event))
								devices[i]->mapped->fail();
							else
								devices[i]->mapped->process(events, rdlen/sizeof(input_event));
						}
					}
				}
			}
		}
	}

	void init()
	{
		if(initialized)
			return;
		initialized = true;
		osd_printf_verbose("evdev: Enumerating devices\n");
		device_dir_fd = open("/dev/input/by-path", O_RDONLY|O_CLOEXEC|O_DIRECTORY);
		if(device_dir_fd < 0) {
			osd_printf_warning("evdev: unable to open device directory: %s\n", strerror(errno));
			return;
		}

		DIR* idir = fdopendir(device_dir_fd);
		while(idir) {
			dirent* de = readdir(idir);
			if(!de)
				break;

			char linkbuf[16];
			ssize_t rl = readlinkat(device_dir_fd, de->d_name, linkbuf, sizeof(linkbuf)-1);
			if(rl < 0)
				continue;
			linkbuf[rl] = 0;
			if(strncmp(linkbuf, "../event", 8))
				continue;

			int ed = openat(device_dir_fd, de->d_name, O_RDONLY);
			if(ed < 0) {
				osd_printf_warning("evdev: unable to open device '%s', ignored: %s\n", de->d_name, strerror(errno));
				continue;
			}

			char namebuf[64];
			if(ioctl(ed, EVIOCGNAME(sizeof(namebuf)), namebuf)<0) {
				// If the device doesn't report a name, use the evdev short link
				// as its name "eventXX".  It's not _informative_ but it's
				// distinctive enough
				strncpy(namebuf, linkbuf+3, 63);
				namebuf[63] = 0;
			}

			dev.emplace_back();
			evdev_device_info& device = dev.back(); // C++17 would return back() from emplace_back()
			
			device.name = namebuf;
			device.id = de->d_name;

			std::fill(std::begin(device.buttons), std::end(device.buttons), 0);
			std::fill(std::begin(device.relaxes), std::end(device.relaxes), 0);
			std::fill(std::begin(device.absaxes), std::end(device.absaxes), 0);

			unsigned long	evbits = 0;

			ioctl(ed, EVIOCGBIT(0, sizeof(evbits)), &evbits);

			if(evbits & (1<<EV_KEY)) {
				ioctl(ed, EVIOCGBIT(EV_KEY, octets_for(KEY_MAX)), device.buttons);
				for(int i=0; device.type==EVDEV_NONE && i<KEY_MAX; i++)
					if(device.buttons[i>>3] & (1<<(i&7)))
						switch(i & ~15) {
							case KEY_Q: // sane? heuristic
								device.type = EVDEV_KEYBOARD;
								break;
							case BTN_MOUSE:
								device.type = EVDEV_MOUSE;
								break;
							case BTN_GAMEPAD:
							case BTN_JOYSTICK:
								device.type = EVDEV_JOYSTICK;
								break;
						}
			}
			if(evbits & (1<<EV_ABS)) {
				ioctl(ed, EVIOCGBIT(EV_ABS, octets_for(ABS_MAX)), device.absaxes);
				if(device.type == EVDEV_NONE)
					device.type = EVDEV_JOYSTICK;
			}
			if(evbits & (1<<EV_REL)) {
				ioctl(ed, EVIOCGBIT(EV_REL, octets_for(REL_MAX)), device.relaxes);
				if(device.type == EVDEV_NONE)
					device.type = EVDEV_MOUSE;
			}

			osd_printf_verbose("evdev: found \"%s\" (%s)\n", device.name.c_str(), device.id.c_str());
			close(ed);
		}

		if(dev.size() > 0)
			pipe2(reader_thread_pipe, O_DIRECT);
	}

	void start()
	{
		if(initialized && dev.size() > 0) {
			if(reader_thread)
				write(reader_thread_pipe[1], "r", 1);
			else
				reader_thread = new std::thread(reader_function, this);
		}
	}

};

class evdev_input_module: public input_module_base
{
protected:
	static evdev_input evdev;

public:
	evdev_input_module(const char* type): input_module_base(type, "evdev")
	{
	}

	void input_setup(running_machine& machine, const char* devname, input_device_class deviceclass, device_type type)
	{
		evdev.init();

		osd_printf_verbose("evdev: Setting up %s devices\n", devname);

		// Note: at this time, no lightgun devices can possibly be mapped because they
		// pretend to be mice.  There may or may not be a mechanism to force it in the future.
		if(type != EVDEV_NONE) {
			for(auto& dev : evdev.dev) {
				if(dev.type==type && !dev.mapped) {
					auto& di = devicelist().create_device<evdev_device>(machine,
						std::string(dev.name), std::string(dev.id),
						*this, deviceclass, dev);
					di.start(evdev.device_dir_fd);
					evdev.start();
				}
			}
		}
	}

	void exit() override
	{
	}

	void before_poll(running_machine& machine) override
	{
	}

	bool should_poll_devices(running_machine& machine) override
	{
		return input_enabled();
	}

};

// Singleton shared for all evdev input devices
evdev_input evdev_input_module::evdev;

class evdev_keyboard_module: public evdev_input_module
{
public:
	evdev_keyboard_module(): evdev_input_module(OSD_KEYBOARDINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Keyboard", DEVICE_CLASS_KEYBOARD, EVDEV_KEYBOARD);
	}

};

class evdev_mouse_module: public evdev_input_module
{
public:
	evdev_mouse_module(): evdev_input_module(OSD_MOUSEINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Mouse", DEVICE_CLASS_MOUSE, EVDEV_MOUSE);
	}
};

class evdev_joystick_module: public evdev_input_module
{
public:
	evdev_joystick_module(): evdev_input_module(OSD_JOYSTICKINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Joystick", DEVICE_CLASS_JOYSTICK, EVDEV_JOYSTICK);
	}
};

class evdev_lightgun_module: public evdev_input_module
{
public:
	evdev_lightgun_module(): evdev_input_module(OSD_LIGHTGUNINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Lightgun", DEVICE_CLASS_LIGHTGUN, EVDEV_LIGHTGUN);
	}
};


} // anonymous namespace

#else // (USE_EVDEV)

MODULE_NOT_SUPPORTED(evdev_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "evdev")
MODULE_NOT_SUPPORTED(evdev_mouse_module,    OSD_MOUSEINPUT_PROVIDER,    "evdev")
MODULE_NOT_SUPPORTED(evdev_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "evdev")
MODULE_NOT_SUPPORTED(evdev_lightgun_module, OSD_LIGHTGUNINPUT_PROVIDER, "evdev")

#endif // (USE_EVDEV)

MODULE_DEFINITION(KEYBOARDINPUT_EVDEV, evdev_keyboard_module)
MODULE_DEFINITION(MOUSEINPUT_EVDEV,    evdev_mouse_module)
MODULE_DEFINITION(JOYSTICKINPUT_EVDEV, evdev_joystick_module)
MODULE_DEFINITION(LIGHTGUNINPUT_EVDEV, evdev_lightgun_module)
