// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  input_macgame.mm - Mac game controller input via the
//  GameController framework.  Gives higher-level functionality
//  similar to sdlgame.
//
//  Mac OSD by R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_MAC)

#import <GameController/GameController.h>

#include <cstring>
#include <string>

// MAME headers
#include "emu.h"
#include "inpttype.h"

#include "../../mac/osdmac.h"
#include "assignmenthelper.h"
#include "input_common.h"

#include "util/strformat.h"


namespace osd {

namespace {

//============================================================
//  macgame_joystick_device
//============================================================

class macgame_joystick_device : public device_info, protected joystick_assignment_helper
{
public:
	enum
	{
		AXIS_LSX, AXIS_LSY,
		AXIS_RSX, AXIS_RSY,
		AXIS_LT, AXIS_RT,
		AXIS_TOTAL
	};

	enum
	{
		BTN_A, BTN_B, BTN_X, BTN_Y,
		BTN_LB, BTN_RB,
		BTN_LSB, BTN_RSB,
		BTN_HOME, BTN_TOUCHPAD,
		BTN_OPTIONS, BTN_MENU,
		BTN_TOTAL
	};

	enum
	{
		HAT_UP, HAT_DOWN, HAT_LEFT, HAT_RIGHT,
		HAT_TOTAL
	};

	macgame_joystick_device(std::string &&name, std::string &&id, input_module &module, GCController *controller) :
		device_info(std::move(name), std::move(id), module),
		m_controller([controller retain]),
		m_touchpad(nil),
		m_connected(true),
		m_state{}
	{
		// the touchpad button only exists on DualSense controllers
		if (@available(macOS 11.3, *))
		{
			GCExtendedGamepad *pad = [m_controller extendedGamepad];
			if ([pad isKindOfClass:[GCDualSenseGamepad class]])
			{
				m_touchpad = [(GCDualSenseGamepad *)pad touchpadButton];
			}
		}
	}

	virtual ~macgame_joystick_device()
	{
		[m_controller release];
	}

	bool has_controller(GCController *controller) const
	{
		return m_controller == controller;
	}

	void set_connected(bool connected)
	{
		m_connected = connected;
		if (!connected)
		{
			memset(&m_state, 0, sizeof(m_state));
		}
	}

	virtual void poll(bool relative_reset) override
	{
		GCExtendedGamepad *pad = [m_controller extendedGamepad];
		if (!m_connected || (pad == nil))
		{
			return;
		}

		// thumbsticks are -1..1 with +1 up; MAME wants negative up
		m_state.axes[AXIS_LSX] = int32_t([[[pad leftThumbstick] xAxis] value] * input_device::ABSOLUTE_MAX);
		m_state.axes[AXIS_LSY] = int32_t([[[pad leftThumbstick] yAxis] value] * -input_device::ABSOLUTE_MAX);
		m_state.axes[AXIS_RSX] = int32_t([[[pad rightThumbstick] xAxis] value] * input_device::ABSOLUTE_MAX);
		m_state.axes[AXIS_RSY] = int32_t([[[pad rightThumbstick] yAxis] value] * -input_device::ABSOLUTE_MAX);

		// triggers are 0..1; MAME wants negative values for triggers
		m_state.axes[AXIS_LT] = -int32_t([[pad leftTrigger] value] * input_device::ABSOLUTE_MAX);
		m_state.axes[AXIS_RT] = -int32_t([[pad rightTrigger] value] * input_device::ABSOLUTE_MAX);

		// messaging nil elements safely returns NO
		m_state.buttons[BTN_A] = [[pad buttonA] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_B] = [[pad buttonB] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_X] = [[pad buttonX] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_Y] = [[pad buttonY] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_LB] = [[pad leftShoulder] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_RB] = [[pad rightShoulder] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_LSB] = [[pad leftThumbstickButton] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_RSB] = [[pad rightThumbstickButton] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_OPTIONS] = [[pad buttonOptions] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_MENU] = [[pad buttonMenu] isPressed] ? 0x80 : 0x00;
		m_state.buttons[BTN_TOUCHPAD] = [m_touchpad isPressed] ? 0x80 : 0x00;
		if (@available(macOS 11.0, *))
		{
			m_state.buttons[BTN_HOME] = [[pad buttonHome] isPressed] ? 0x80 : 0x00;
		}

		GCControllerDirectionPad *dpad = [pad dpad];
		m_state.dpad[HAT_UP] = [[dpad up] isPressed] ? 0x80 : 0x00;
		m_state.dpad[HAT_DOWN] = [[dpad down] isPressed] ? 0x80 : 0x00;
		m_state.dpad[HAT_LEFT] = [[dpad left] isPressed] ? 0x80 : 0x00;
		m_state.dpad[HAT_RIGHT] = [[dpad right] isPressed] ? 0x80 : 0x00;
	}

	virtual void reset() override
	{
		memset(&m_state, 0, sizeof(m_state));
	}

	virtual void configure(input_device &device) override
	{
		GCExtendedGamepad *pad = [m_controller extendedGamepad];
		input_device::assignment_vector assignments;

		// all axes are always present on the extended gamepad profile
		static const struct { input_item_id item; const char *name; } axisdefs[AXIS_TOTAL] =
		{
			{ ITEM_ID_XAXIS,   "LSX" },
			{ ITEM_ID_YAXIS,   "LSY" },
			{ ITEM_ID_ZAXIS,   "RSX" },
			{ ITEM_ID_RZAXIS,  "RSY" },
			{ ITEM_ID_SLIDER1, "LT" },
			{ ITEM_ID_SLIDER2, "RT" }
		};

		input_item_id axisactual[AXIS_TOTAL];
		for (int i = 0; i < AXIS_TOTAL; i++)
		{
			axisactual[i] = device.add_item(
					axisdefs[i].name,
					std::string_view(),
					axisdefs[i].item,
					generic_axis_get_state<int32_t>,
					&m_state.axes[i]);
		}

		// numbered buttons, in the same order SDL's game controller support uses
		input_item_id btnactual[BTN_TOTAL];
		std::fill(std::begin(btnactual), std::end(btnactual), ITEM_ID_INVALID);

		input_item_id button_item = ITEM_ID_BUTTON1;
		unsigned buttoncount = 0;

		auto add_button =
				[&] (GCControllerButtonInput *element, const char *fallback, int slot)
				{
					if (element == nil)
					{
						return;
					}

					const char *name = [[element localizedName] UTF8String];
					btnactual[slot] = device.add_item(
							(name != nullptr) ? name : fallback,
							std::string_view(),
							button_item++,
							generic_button_get_state<std::uint8_t>,
							&m_state.buttons[slot]);
					add_button_assignment(assignments, ioport_type(IPT_BUTTON1 + buttoncount++), { btnactual[slot] });
				};

		add_button([pad buttonA], "A", BTN_A);
		add_button([pad buttonB], "B", BTN_B);
		add_button([pad buttonX], "X", BTN_X);
		add_button([pad buttonY], "Y", BTN_Y);
		add_button([pad leftShoulder], "LB", BTN_LB);
		add_button([pad rightShoulder], "RB", BTN_RB);
		add_button([pad leftThumbstickButton], "LSB", BTN_LSB);
		add_button([pad rightThumbstickButton], "RSB", BTN_RSB);
		if (@available(macOS 11.0, *))
		{
			add_button([pad buttonHome], "Guide", BTN_HOME);
		}
		add_button(m_touchpad, "Touchpad", BTN_TOUCHPAD);

		// buttons with fixed item IDs
		auto add_fixed_button =
				[&] (GCControllerButtonInput *element, const char *fallback, int slot, input_item_id item)
				{
					if (element == nil)
					{
						return;
					}

					const char *name = [[element localizedName] UTF8String];
					btnactual[slot] = device.add_item(
							(name != nullptr) ? name : fallback,
							std::string_view(),
							item,
							generic_button_get_state<std::uint8_t>,
							&m_state.buttons[slot]);
				};

		add_fixed_button([pad buttonOptions], "Options", BTN_OPTIONS, ITEM_ID_SELECT);
		add_fixed_button([pad buttonMenu], "Menu", BTN_MENU, ITEM_ID_START);

		// the D-pad becomes a hat switch
		static const struct { input_item_id item; const char *name; } hatdefs[HAT_TOTAL] =
		{
			{ ITEM_ID_HAT1UP,    "D-pad Up" },
			{ ITEM_ID_HAT1DOWN,  "D-pad Down" },
			{ ITEM_ID_HAT1LEFT,  "D-pad Left" },
			{ ITEM_ID_HAT1RIGHT, "D-pad Right" }
		};

		input_item_id hatactual[HAT_TOTAL];
		for (int i = 0; i < HAT_TOTAL; i++)
		{
			hatactual[i] = device.add_item(
					hatdefs[i].name,
					std::string_view(),
					hatdefs[i].item,
					generic_button_get_state<std::uint8_t>,
					&m_state.dpad[i]);
		}

		// use the best stick for primary movement controls, with the D-pad as backup
		input_item_id diraxis[2][2];
		choose_primary_stick(diraxis, axisactual[AXIS_LSX], axisactual[AXIS_LSY], axisactual[AXIS_RSX], axisactual[AXIS_RSY]);
		add_directional_assignments(
				assignments,
				diraxis[0][0],
				diraxis[0][1],
				hatactual[HAT_LEFT],
				hatactual[HAT_RIGHT],
				hatactual[HAT_UP],
				hatactual[HAT_DOWN]);

		// assign the secondary stick to joystick Z, or combined triggers failing that
		const bool zaxis = add_assignment(
				assignments,
				IPT_AD_STICK_Z,
				SEQ_TYPE_STANDARD,
				ITEM_CLASS_ABSOLUTE,
				ITEM_MODIFIER_NONE,
				{ diraxis[1][1], diraxis[1][0] });
		if (!zaxis)
		{
			assignments.emplace_back(
					IPT_AD_STICK_Z,
					SEQ_TYPE_STANDARD,
					input_seq(
							make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axisactual[AXIS_LT]),
							make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_REVERSE, axisactual[AXIS_RT])));
		}

		// triggers make good pedals
		add_assignment(assignments, IPT_PEDAL, SEQ_TYPE_STANDARD, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, { axisactual[AXIS_RT] });
		add_assignment(assignments, IPT_PEDAL2, SEQ_TYPE_STANDARD, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, { axisactual[AXIS_LT] });

		// twin stick controls
		add_twin_stick_assignments(
				assignments,
				axisactual[AXIS_LSX],
				axisactual[AXIS_LSY],
				axisactual[AXIS_RSX],
				axisactual[AXIS_RSY],
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID);

		// select/start and UI shortcuts
		add_button_assignment(assignments, IPT_SELECT, { btnactual[BTN_OPTIONS] });
		add_button_assignment(assignments, IPT_START, { btnactual[BTN_MENU] });
		add_button_assignment(assignments, IPT_UI_MENU, { btnactual[BTN_HOME] });
		add_button_assignment(assignments, IPT_UI_SELECT, { btnactual[BTN_A] });
		add_button_assignment(assignments, IPT_UI_CLEAR, { btnactual[BTN_X] });

		// previous/next group on the triggers
		input_item_id lt = axisactual[AXIS_LT];
		input_item_id rt = axisactual[AXIS_RT];
		consume_trigger_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, lt, rt);

		device.set_default_assignments(std::move(assignments));
	}

private:
	struct pad_state
	{
		int32_t axes[AXIS_TOTAL];
		uint8_t buttons[BTN_TOTAL];
		uint8_t dpad[HAT_TOTAL];
	};

	GCController *m_controller;
	GCControllerButtonInput *m_touchpad;    // owned by the controller's profile
	bool m_connected;
	pad_state m_state;
};


//============================================================
//  joystick_input_macgame - GameController joystick module
//============================================================

class joystick_input_macgame : public input_module_impl<macgame_joystick_device, mac_osd_interface>
{
public:
	joystick_input_macgame() :
		input_module_impl<macgame_joystick_device, mac_osd_interface>(OSD_JOYSTICKINPUT_PROVIDER, "macgame"),
		m_connect_observer(nil),
		m_disconnect_observer(nil)
	{
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		// wake the framework up early so the controller list is populated
		// by the time devices are created
		[GCController controllers];

		return input_module_impl<macgame_joystick_device, mac_osd_interface>::init(osd, options);
	}

	virtual void input_init(running_machine &machine) override
	{
		input_module_impl<macgame_joystick_device, mac_osd_interface>::input_init(machine);

		for (GCController *controller in [GCController controllers])
		{
			controller_connected(controller);
		}

		// the controller list populates asynchronously, so a controller
		// present at startup can arrive after machine start; this also
		// gives us hot-plugging
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		m_connect_observer = [[center
				addObserverForName:GCControllerDidConnectNotification
				object:nil
				queue:[NSOperationQueue mainQueue]
				usingBlock:^(NSNotification *note) { this->controller_connected((GCController *)[note object]); }] retain];
		m_disconnect_observer = [[center
				addObserverForName:GCControllerDidDisconnectNotification
				object:nil
				queue:[NSOperationQueue mainQueue]
				usingBlock:^(NSNotification *note) { this->controller_disconnected((GCController *)[note object]); }] retain];
	}

	virtual void exit() override
	{
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		if (m_connect_observer != nil)
		{
			[center removeObserver:m_connect_observer];
			[m_connect_observer release];
			m_connect_observer = nil;
		}
		if (m_disconnect_observer != nil)
		{
			[center removeObserver:m_disconnect_observer];
			[m_disconnect_observer release];
			m_disconnect_observer = nil;
		}

		input_module_impl<macgame_joystick_device, mac_osd_interface>::exit();
	}

private:
	void controller_connected(GCController *controller)
	{
		// only controllers with the standardized extended gamepad profile
		if ((controller == nil) || ([controller extendedGamepad] == nil))
		{
			return;
		}

		// a reconnected controller gets its existing device back
		bool found = false;
		devicelist().for_each_device(
				[controller, &found] (auto &device)
				{
					if (device.has_controller(controller))
					{
						device.set_connected(true);
						found = true;
					}
				});
		if (found)
		{
			return;
		}

		const char *vendor = [[controller vendorName] UTF8String];
		std::string name = (vendor != nullptr) ? vendor : "Game Controller";
		std::string id = util::string_format("%s %u", name, unsigned(devicelist().size() + 1));

		osd_printf_verbose("Game Controller: adding %s\n", name);
		create_device<macgame_joystick_device>(DEVICE_CLASS_JOYSTICK, std::move(name), std::move(id), controller);
	}

	void controller_disconnected(GCController *controller)
	{
		devicelist().for_each_device(
				[controller] (auto &device)
				{
					if (device.has_controller(controller))
					{
						device.set_connected(false);
					}
				});
	}

	id m_connect_observer;
	id m_disconnect_observer;
};

} // anonymous namespace

} // namespace osd

#else // defined(OSD_MAC)

#include "input_module.h"
#include "modules/osdmodule.h"

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(joystick_input_macgame, OSD_JOYSTICKINPUT_PROVIDER, "macgame")

} // anonymous namespace

} // namespace osd

#endif // defined(OSD_MAC)

MODULE_DEFINITION(JOYSTICKINPUT_MACGAME, osd::joystick_input_macgame)
