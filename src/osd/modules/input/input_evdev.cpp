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

#if defined(SDLMAME_SDL2) && defined(SDLMAME_LINUX)

#include <cctype>
#include <cstddef>
#include <mutex>
#include <memory>
#include <queue>
#include <string>
#include <algorithm>

// MAME headers
#include "emu.h"
#include "osdepend.h"

// MAMEOS headers
#include "../lib/osdobj_common.h"
#include "input_common.h"
#include "../../sdl/osdsdl.h"
#include "input_sdlcommon.h"

#include <linux/version.h>
#include <linux/input.h>

#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

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
	EVDEV_GENERIC,
};


#define OCTETS_FOR(n) ((n+7)>>3)
struct evdev_device_info
{
	char name[64];
	char id[32];
	device_type type;
	unsigned char	num;
	unsigned char	buttons[OCTETS_FOR(KEY_MAX)];
	unsigned char	relaxes[OCTETS_FOR(REL_MAX)];
	unsigned char	absaxes[OCTETS_FOR(ABS_MAX)];
};


class evdev_device: public device_info
{
private:
	// This is _way_ too conservative, but memory is cheap
	const evdev_device_info& info;
	int	fd;
	uint8_t	keystate[KEY_MAX];
	int32_t	relaxis[REL_MAX];
	struct {
		int32_t	normalized;
		input_absinfo absinfo;
	}	absaxis[ABS_MAX];

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
			char fname[16];
			snprintf(fname, 16, "event%d", info.num);
			fd = openat(dirfd, fname, O_RDONLY|O_NONBLOCK);
			if(fd < 0) {
				osd_printf_verbose("evdev: unable to open %s: %s\n", fname, strerror(errno));
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
	}

	void poll(void) override
	{
		static input_event ie[256]; // try to consume as many as possible at once
		while(fd >= 0) {
			ssize_t rd = read(fd, ie, 256*sizeof(input_event));
			if(rd < 0) {
				if(errno==EWOULDBLOCK || errno==EAGAIN)
					return;
				// cope with errors here
			}
			if(rd < sizeof(input_event))
				return;
			int nie = rd/sizeof(input_event);
			for(int i=0; i<nie; i++) {
				switch(ie[i].type) {
					case EV_KEY:
						keystate[ie[i].code] = (ie[i].value>0)? 0x80: 0;
						break;
					case EV_REL:
						relaxis[ie[i].code] += ie[i].value;
						break;
					case EV_ABS: {
						  auto& axis = absaxis[ie[i].code];
						  axis.normalized = normalize_absolute_axis(ie[i].value, axis.absinfo.minimum, axis.absinfo.maximum);
						}
						break;
				}
			}
		}
	}

	void reset(void) override
	{
		memset(keystate, 0, KEY_MAX);
		memset(relaxis, 0, REL_MAX*sizeof(int32_t));
		memset(absaxis, 0, ABS_MAX*sizeof(absaxis[0]));
	}
};

#define MAX_EVDEV_DEVICES 64

class evdev_input
{
private:
	bool initialized;

public:
	int  devinput;
	evdev_device_info	dev[MAX_EVDEV_DEVICES]; // 640k should be enough for anyone
	int			max_dev;

public:
	evdev_input(): initialized(false)
	{
	}

	void init(void)
	{
		if(initialized)
			return;
		initialized = true;
		osd_printf_verbose("evdev: enumerating devices\n");
		devinput = open("/dev/input", O_RDONLY|O_CLOEXEC|O_DIRECTORY);
		if(devinput < 0) {
			return;
		}

		max_dev = 0;
		for(int i=0; i<MAX_EVDEV_DEVICES; i++)
			dev[i].type = EVDEV_NONE;

		DIR* idir = fdopendir(devinput);
		while(idir) {
			dirent* de = readdir(idir);
			if(!de)
				break;
			if(strncmp(de->d_name, "event", 5))
				continue;
			int dnum = atoi(de->d_name+5);
			if(dnum >= MAX_EVDEV_DEVICES) {
				// too many to handle
				continue;
			}
			if(dnum < MAX_EVDEV_DEVICES) {
				if(dev[dnum].type != EVDEV_NONE) {
					// the same device twice?
					continue;
				}
			}
			int ed = openat(devinput, de->d_name, O_RDONLY);
			if(ed < 0) {
				osd_printf_verbose("evdev: unable to open de->d_name: %s\n", strerror(errno));
				continue;
			}
			input_id iid;
			if(ioctl(ed, EVIOCGID, &iid)<0) {
				osd_printf_verbose("evdev: event%d doesn't looks usable\n", dnum);
				close(ed);
				continue;
			} else
				snprintf(dev[dnum].id, 32, "%hu:%04hx:%04hx", iid.bustype, iid.vendor, iid.product);

			if(ioctl(ed, EVIOCGNAME(sizeof(dev[dnum].name)), dev[dnum].name)<0) {
				osd_printf_verbose("evdev: event%d doesn't have a name\n", dnum);
				close(ed);
				continue;
			}

			dev[dnum].type = EVDEV_NONE;
			memset(dev[dnum].buttons, 0, OCTETS_FOR(KEY_MAX));
			memset(dev[dnum].relaxes, 0, OCTETS_FOR(REL_MAX));
			memset(dev[dnum].absaxes, 0, OCTETS_FOR(ABS_MAX));

			unsigned long	evbits = 0;
			unsigned char	bits[(KEY_MAX+7)>>3];

			ioctl(ed, EVIOCGBIT(0, sizeof(evbits)), &evbits);

			if(evbits & (1<<EV_KEY)) {
				memset(bits, 0, sizeof(bits));
				ioctl(ed, EVIOCGBIT(EV_KEY, OCTETS_FOR(KEY_MAX)), dev[dnum].buttons);
				for(int i=0; dev[dnum].type==EVDEV_NONE && i<KEY_MAX; i++)
					if(dev[dnum].buttons[i>>3] & (1<<(i&7)))
						switch(i & ~15) {
							case KEY_Q: // sane? heuristic
								dev[dnum].type = EVDEV_KEYBOARD;
								break;
							case BTN_MOUSE:
								if(evbits & (1<<EV_REL))
									dev[dnum].type = EVDEV_MOUSE;
								break;
							case BTN_GAMEPAD:
							case BTN_JOYSTICK:
								if(evbits & (1<<EV_ABS))
									dev[dnum].type = EVDEV_JOYSTICK;
								break;
						}
			}
			if(evbits & (1<<EV_ABS)) {
				ioctl(ed, EVIOCGBIT(EV_ABS, OCTETS_FOR(ABS_MAX)), dev[dnum].absaxes);
				if(dev[dnum].type == EVDEV_NONE)
					dev[dnum].type = EVDEV_GENERIC;
			}
			if(evbits & (1<<EV_REL)) {
				ioctl(ed, EVIOCGBIT(EV_REL, OCTETS_FOR(REL_MAX)), dev[dnum].relaxes);
				if(dev[dnum].type == EVDEV_NONE)
					dev[dnum].type = EVDEV_GENERIC;
			}

			const char* const dname[] = { 0, "keyboard", "mouse", "joystick", "generic HID device" };

			if(dev[dnum].type != EVDEV_NONE) {
				dev[dnum].num = dnum;
				if(max_dev <= dnum)
					max_dev = dnum+1;
				osd_printf_verbose("evdev: found %s (%s) \"%s\"\n", dname[dev[dnum].type], dev[dnum].id, dev[dnum].name);
			}
			close(ed);
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

	void input_setup(running_machine& machine, const char* devname, input_device_class deviceclass, device_type type, const char* devmap)
	{
		evdev.init();

		osd_printf_verbose("evdev: Setting up %s devices\n", devname);

		char defname[20];
		int  devindex[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

		// first try to map explicitly requested devices
		// this _specifically_ allows matching against type since
		// one might want to treat something that looks like a joystick
		// as a mouse, or map a generic or unusual HID that nevertheless
		// provides buttons or axes.
		for(int dev=0; dev<8; dev++) {
			sprintf(defname, "%s%d", devmap, dev+1);
			const char* dev_name = machine.options().value(defname);
			if (dev_name && *dev_name && strcmp(dev_name, OSDOPTVAL_AUTO))
				for(int i=0; i<evdev.max_dev; i++) {
					if(!strcmp(dev_name, evdev.dev[i].name) || !strcmp(dev_name, evdev.dev[i].id)) {
						devindex[dev] = i;
						break;
					}
				}
		}
		// then try to map all unmapped matching devices that look
		// like the correct type (so remaining mice are mapped as such
		// and so on)
		if(type != EVDEV_NONE) {
			for(int i=0; i<evdev.max_dev; i++) {
				if(evdev.dev[i].type == type) {
					int absent = 1;
					int free = -1;
					for(int di=0; di<8; di++) {
						if(devindex[di]<0 && free<0)
							free = di;
						if(devindex[di]==i) {
							absent = 0;
							break;
						}
					}
					if(absent && free>=0)
						devindex[free] = i;
				}
			}
		}
		// create devices that ended up mapped
		for(int i=0; i<8; i++)
			if(devindex[i] >= 0) {
				auto& dev = evdev.dev[devindex[i]];
				auto& di = devicelist().create_device<evdev_device>(machine, dev.name, dev.id, *this, deviceclass, dev);
				di.start(evdev.devinput);
			}
	}

	void exit(void) override
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
	evdev_keyboard_module(void): evdev_input_module(OSD_KEYBOARDINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Keyboard", DEVICE_CLASS_KEYBOARD, EVDEV_KEYBOARD, SDLOPTION_KEYBINDEX);
	}

};

class evdev_mouse_module: public evdev_input_module
{
public:
	evdev_mouse_module(void): evdev_input_module(OSD_MOUSEINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Mouse", DEVICE_CLASS_MOUSE, EVDEV_MOUSE, SDLOPTION_MOUSEINDEX);
	}
};

class evdev_joystick_module: public evdev_input_module
{
public:
	evdev_joystick_module(void): evdev_input_module(OSD_JOYSTICKINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Joystick", DEVICE_CLASS_JOYSTICK, EVDEV_JOYSTICK, SDLOPTION_JOYINDEX);
	}
};

class evdev_lightgun_module: public evdev_input_module
{
public:
	evdev_lightgun_module(void): evdev_input_module(OSD_LIGHTGUNINPUT_PROVIDER)
	{
	}

	void input_init(running_machine& machine) override
	{
		input_setup(machine, "Lightgun", DEVICE_CLASS_LIGHTGUN, EVDEV_NONE, SDLOPTION_LIGHTGUNINDEX);
	}
};


} // anonymous namespace

#else // defined(SDLMAME_SDL2) && defined(SDLMAME_LINUX)

MODULE_NOT_SUPPORTED(evdev_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "evdev")
MODULE_NOT_SUPPORTED(evdev_mouse_module,    OSD_MOUSEINPUT_PROVIDER,    "evdev")
MODULE_NOT_SUPPORTED(evdev_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "evdev")
MODULE_NOT_SUPPORTED(evdev_lightgun_module, OSD_LIGHTGUNINPUT_PROVIDER, "evdev")

#endif //  defined(SDLMAME_SDL2) && !defined(SDLMAME_WIN32) && defined(USE_XINPUT) && USE_XINPUT

MODULE_DEFINITION(KEYBOARDINPUT_EVDEV, evdev_keyboard_module)
MODULE_DEFINITION(MOUSEINPUT_EVDEV,    evdev_mouse_module)
MODULE_DEFINITION(JOYSTICKINPUT_EVDEV, evdev_joystick_module)
MODULE_DEFINITION(LIGHTGUNINPUT_EVDEV, evdev_lightgun_module)
