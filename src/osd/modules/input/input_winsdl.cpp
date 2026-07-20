// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "input_module.h"

#include "modules/osdmodule.h"

#if !defined(OSD_SDL) && defined(USE_SDL_JOYSTICK)

#include "assignmenthelper.h"
#include "input_common.h"
#include "input_sdlcommon.h"

#include "modules/lib/osdobj_common.h"

// standard SDL header
#if defined(USE_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL2/SDL.h>
#endif


namespace osd {

namespace {

class sdl_joystick_device : public device_info, public sdl_joystick_device_common
{
public:
	sdl_joystick_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			SDL_Joystick *joy,
			char const *serial) :
		device_info(
				std::move(name),
				std::move(id),
				module),
		m_joydevice(joy),
#if defined(USE_SDL3)
		m_hapdevice(SDL_OpenHapticFromJoystick(joy)),
		m_axiscount(std::min<int>(SDL_GetNumJoystickAxes(joy), MAX_AXES)),
		m_buttoncount(std::min<int>(SDL_GetNumJoystickButtons(joy), MAX_BUTTONS)),
		m_hatcount(std::min<int>(SDL_GetNumJoystickHats(joy), MAX_HATS)),
		m_ballcount(std::min<int>(SDL_GetNumJoystickBalls(joy), MAX_AXES / 2))
#else
		m_hapdevice(SDL_HapticOpenFromJoystick(joy)),
		m_axiscount(std::min<int>(SDL_JoystickNumAxes(joy), MAX_AXES)),
		m_buttoncount(std::min<int>(SDL_JoystickNumButtons(joy), MAX_BUTTONS)),
		m_hatcount(std::min<int>(SDL_JoystickNumHats(joy), MAX_HATS)),
		m_ballcount(std::min<int>(SDL_JoystickNumBalls(joy), MAX_AXES / 2))
#endif
	{
		if (serial)
			m_serial = serial;
	}

	~sdl_joystick_device()
	{
		if (m_joydevice)
		{
			if (m_hapdevice)
			{
#if defined(USE_SDL3)
				SDL_CloseHaptic(m_hapdevice);
#else
				SDL_HapticClose(m_hapdevice);
#endif
				m_hapdevice = nullptr;
			}
#if defined(USE_SDL3)
			SDL_CloseJoystick(m_joydevice);
#else
			SDL_JoystickClose(m_joydevice);
#endif
			m_joydevice = nullptr;
		}
	}

	virtual void configure(input_device &device) override
	{
		configure_common(device, m_axiscount, m_buttoncount, m_hatcount, m_ballcount);
	}

	virtual void poll(bool relative_reset) override
	{
		for (int i = 0; i < m_axiscount; ++i)
		{
#if defined(USE_SDL3)
			m_joystick.axes[i] = int32_t(SDL_GetJoystickAxis(m_joydevice, i)) * 2;
#else
			m_joystick.axes[i] = int32_t(SDL_JoystickGetAxis(m_joydevice, i)) * 2;
#endif
		}
		for (int i = 0; i < m_buttoncount; ++i)
		{
#if defined(USE_SDL3)
			m_joystick.buttons[i] = SDL_GetJoystickButton(m_joydevice, i) ? 0x80 : 0;
#else
			m_joystick.buttons[i] = SDL_JoystickGetButton(m_joydevice, i) ? 0x80 : 0;
#endif
		}
		for (int i = 0; i < m_hatcount; ++i)
		{
#if defined(USE_SDL3)
			auto const state = SDL_GetJoystickHat(m_joydevice, i);
#else
			auto const state = SDL_JoystickGetHat(m_joydevice, i);
#endif
			m_joystick.hatsU[i] = (state & SDL_HAT_UP) ? 0x80 : 0;
			m_joystick.hatsD[i] = (state & SDL_HAT_DOWN) ? 0x80 : 0;
			m_joystick.hatsL[i] = (state & SDL_HAT_LEFT) ? 0x80 : 0;
			m_joystick.hatsR[i] = (state & SDL_HAT_RIGHT) ? 0x80 : 0;
		}
		for (int i = 0; i < m_ballcount; ++i)
		{
			int x, y;
#if defined(USE_SDL3)
			if (SDL_GetJoystickBall(m_joydevice, i, &x, &y))
#else
			if (!SDL_JoystickGetBall(m_joydevice, i, &x, &y))
#endif
			{
				m_ball[i * 2] = x * input_device::RELATIVE_PER_PIXEL;
				m_ball[i * 2 + 1] = y * input_device::RELATIVE_PER_PIXEL;
			}
		}

		if (relative_reset)
		{
			for (unsigned i = 0; (m_ballcount * 2) > i; ++i)
				m_joystick.balls[i] = std::exchange(m_ball[i], 0);
		}
	}

	virtual void reset() override
	{
		clear_buffer();
	}

	bool has_haptic() const
	{
		return m_hapdevice != nullptr;
	}

private:
	SDL_Joystick *m_joydevice;
	SDL_Haptic *m_hapdevice;
	int m_axiscount, m_buttoncount, m_hatcount, m_ballcount;
	std::optional<std::string> m_serial;
};


class sdl_joystick_module : public input_module_impl<sdl_joystick_device, osd_common_t>
{
public:
	sdl_joystick_module() :
		input_module_impl(OSD_JOYSTICKINPUT_PROVIDER, "sdljoy"),
		m_initialized_joystick(false),
		m_initialized_haptic(false)
	{
	}

	virtual ~sdl_joystick_module()
	{
		assert(!m_initialized_joystick);
		assert(!m_initialized_haptic);
	}

	virtual void input_init(running_machine &machine) override
	{
		if (!options()->debug() && options()->background_input())
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#if !defined(USE_SDL3)
		SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif

#if defined(USE_SDL3)
		m_initialized_joystick = SDL_InitSubSystem(SDL_INIT_JOYSTICK);
#else
		m_initialized_joystick = !SDL_InitSubSystem(SDL_INIT_JOYSTICK);
#endif
		if (!m_initialized_joystick)
		{
			osd_printf_error("Could not initialize SDL Joystick subsystem: %s.\n", SDL_GetError());
			return;
		}

#if defined(USE_SDL3)
		m_initialized_haptic = SDL_InitSubSystem(SDL_INIT_HAPTIC);
#else
		m_initialized_haptic = !SDL_InitSubSystem(SDL_INIT_HAPTIC);
#endif
		if (!m_initialized_haptic)
			osd_printf_verbose("Could not initialize SDL Haptic subsystem: %s.\n", SDL_GetError());

		input_module_impl::input_init(machine);

		osd_printf_verbose("Joystick: Start initialization\n");
#if defined(USE_SDL3)
		int stick_count = 0;
		auto const joysticks = SDL_GetJoysticks(&stick_count);
		for (int physical_stick = 0; physical_stick < stick_count; physical_stick++)
			create_joystick_device(joysticks[physical_stick]);
		SDL_free(joysticks);
#else
		for (int physical_stick = 0; physical_stick < SDL_NumJoysticks(); physical_stick++)
			create_joystick_device(physical_stick);
#endif

		osd_printf_verbose("Joystick: End initialization\n");
	}

	virtual void exit() override
	{
		input_module_impl::exit();

		if (m_initialized_joystick)
		{
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
			m_initialized_joystick = false;
		}

		if (m_initialized_haptic)
		{
			SDL_QuitSubSystem(SDL_INIT_HAPTIC);
			m_initialized_haptic = false;
		}
	}

	virtual void before_poll() override
	{
		input_module_impl::before_poll();

#if defined(USE_SDL3)
		SDL_UpdateJoysticks();
#else
		SDL_JoystickUpdate();
#endif
	}

private:
#if defined(USE_SDL3)
	sdl_joystick_device *create_joystick_device(SDL_JoystickID index)
#else
	sdl_joystick_device *create_joystick_device(int index)
#endif
	{
		// open the joystick device
#if defined(USE_SDL3)
		SDL_Joystick *const joy = SDL_OpenJoystick(index);
#else
		SDL_Joystick *const joy = SDL_JoystickOpen(index);
#endif
		if (!joy)
		{
			osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", index, SDL_GetError());
			return nullptr;
		}

		// get basic info
		char guid_str[256];
		guid_str[0] = '\0';
#if defined(USE_SDL3)
		char const *const name = SDL_GetJoystickName(joy);
		SDL_GUID guid = SDL_GetJoystickGUID(joy);
		SDL_GUIDToString(guid, guid_str, sizeof(guid_str) - 1);
		char const *const serial = SDL_GetJoystickSerial(joy);
#else
		char const *const name = SDL_JoystickName(joy);
		SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
		SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str) - 1);
		char const *const serial = SDL_JoystickGetSerial(joy);
#endif
		std::string id(guid_str);
		if (serial)
			id.append(1, '-').append(serial);

		// print some diagnostic info
		osd_printf_verbose("Joystick: %s [GUID %s] Vendor ID %04X, Product ID %04X, Revision %04X, Serial %s\n",
				name ? name : "<nullptr>",
				guid_str,
#if defined(USE_SDL3)
				SDL_GetJoystickVendor(joy),
				SDL_GetJoystickProduct(joy),
				SDL_GetJoystickProductVersion(joy),
#else
				SDL_JoystickGetVendor(joy),
				SDL_JoystickGetProduct(joy),
				SDL_JoystickGetProductVersion(joy),
#endif
				serial ? serial : "<nullptr>");
#if defined(USE_SDL3)
		auto const axes = SDL_GetNumJoystickAxes(joy);
		auto const buttons = SDL_GetNumJoystickButtons(joy);
		auto const hats = SDL_GetNumJoystickHats(joy);
		auto const balls = SDL_GetNumJoystickBalls(joy);
#else
		auto const axes = SDL_JoystickNumAxes(joy);
		auto const buttons = SDL_JoystickNumButtons(joy);
		auto const hats = SDL_JoystickNumHats(joy);
		auto const balls = SDL_JoystickNumBalls(joy);
#endif
		osd_printf_verbose("Joystick:   ...  %d axes, %d buttons %d hats %d balls\n", axes, buttons, hats, balls);
		if (buttons > sdl_joystick_device::MAX_BUTTONS)
			osd_printf_verbose("Joystick:   ...  Has %d buttons which exceeds supported %d buttons\n", buttons, sdl_joystick_device::MAX_BUTTONS);

		// instantiate device
		sdl_joystick_device &devinfo = create_device<sdl_joystick_device>(DEVICE_CLASS_JOYSTICK, name ? name : guid_str, guid_str, joy, serial);

		if (devinfo.has_haptic())
			osd_printf_verbose("Joystick:   ...  Has haptic capability\n");
		else
			osd_printf_verbose("Joystick:   ...  Does not have haptic capability\n");

		return &devinfo;
	}

	bool m_initialized_joystick;
	bool m_initialized_haptic;
};

} // anonymous namespace

} // namespace osd


MODULE_DEFINITION(JOYSTICKINPUT_SDLJOY, osd::sdl_joystick_module)

#endif // !defined(OSD_SDL) && defined(USE_SDL_JOYSTICK)
