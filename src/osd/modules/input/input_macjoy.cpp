// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  input_macjoy.cpp - "traditional" Mac joystick support via
//  IOHIDManager.
//
//  Mac OSD by R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_MAC)

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>

#include <set>
#include <string>
#include <vector>

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
//  macjoy_device
//============================================================

class macjoy_device : public device_info, protected joystick_assignment_helper
{
public:
	static constexpr unsigned MAX_BUTTONS = 32;
	static constexpr unsigned MAX_HATS = 4;

	macjoy_device(std::string &&name, std::string &&id, input_module &module, IOHIDDeviceRef device) :
		device_info(std::move(name), std::move(id), module),
		m_device(device),
		m_connected(true)
	{
		CFRetain(m_device);
		enumerate_elements();
	}

	virtual ~macjoy_device()
	{
		for (auto &axis : m_axes)
		{
			CFRelease(axis.element);
		}
		for (auto &button : m_buttons)
		{
			CFRelease(button.element);
		}
		for (auto &hat : m_hats)
		{
			CFRelease(hat.element);
		}
		CFRelease(m_device);
	}

	bool has_device(IOHIDDeviceRef device) const
	{
		return m_device == device;
	}

	void set_connected(bool connected)
	{
		m_connected = connected;
		if (!connected)
		{
			reset();
		}
	}

	virtual void poll(bool relative_reset) override
	{
		if (!m_connected)
		{
			return;
		}

		IOHIDValueRef valref;

		for (auto &axis : m_axes)
		{
			if (IOHIDDeviceGetValue(m_device, axis.element, &valref) == kIOReturnSuccess)
			{
				axis.value = normalize_absolute_axis(IOHIDValueGetIntegerValue(valref), axis.min, axis.max);
			}
		}

		for (auto &button : m_buttons)
		{
			if (IOHIDDeviceGetValue(m_device, button.element, &valref) == kIOReturnSuccess)
			{
				button.value = (IOHIDValueGetIntegerValue(valref) != 0) ? 0x80 : 0x00;
			}
		}

		for (auto &hat : m_hats)
		{
			if (IOHIDDeviceGetValue(m_device, hat.element, &valref) == kIOReturnSuccess)
			{
				// eight-way hat values run clockwise from north; anything
				// outside the logical range is the centered null state
				const CFIndex value = IOHIDValueGetIntegerValue(valref) - hat.min;
				hat.up    = ((value == 0) || (value == 1) || (value == 7)) ? 0x80 : 0x00;
				hat.right = ((value >= 1) && (value <= 3)) ? 0x80 : 0x00;
				hat.down  = ((value >= 3) && (value <= 5)) ? 0x80 : 0x00;
				hat.left  = ((value >= 5) && (value <= 7)) ? 0x80 : 0x00;
			}
		}
	}

	virtual void reset() override
	{
		for (auto &axis : m_axes)
		{
			axis.value = 0;
		}
		for (auto &button : m_buttons)
		{
			button.value = 0;
		}
		for (auto &hat : m_hats)
		{
			hat.up = hat.down = hat.left = hat.right = 0;
		}
	}

	virtual void configure(input_device &device) override
	{
		input_device::assignment_vector assignments;

		// add axes, falling back to additional absolute items on collisions
		std::set<input_item_id> used;
		input_item_id xaxis = ITEM_ID_INVALID;
		input_item_id yaxis = ITEM_ID_INVALID;
		for (auto &axis : m_axes)
		{
			input_item_id item = axis.preferred;
			if (used.count(item))
			{
				item = ITEM_ID_ADD_ABSOLUTE1;
				while (used.count(item))
				{
					item = input_item_id(item + 1);
				}
			}
			used.insert(item);

			const input_item_id actual = device.add_item(
					axis.name,
					std::string_view(),
					item,
					generic_axis_get_state<int32_t>,
					&axis.value);

			if ((ITEM_ID_INVALID == xaxis) && (ITEM_ID_XAXIS == axis.preferred))
			{
				xaxis = actual;
			}
			if ((ITEM_ID_INVALID == yaxis) && (ITEM_ID_YAXIS == axis.preferred))
			{
				yaxis = actual;
			}
		}

		// add buttons and default button assignments
		unsigned buttoncount = 0;
		input_item_id firstbutton = ITEM_ID_INVALID;
		for (auto &button : m_buttons)
		{
			const input_item_id actual = device.add_item(
					button.name,
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + buttoncount),
					generic_button_get_state<std::uint8_t>,
					&button.value);

			if (ITEM_ID_INVALID == firstbutton)
			{
				firstbutton = actual;
			}
			add_button_assignment(assignments, ioport_type(IPT_BUTTON1 + buttoncount), { actual });
			buttoncount++;
		}

		// add hats
		input_item_id hat1[4] = { ITEM_ID_INVALID, ITEM_ID_INVALID, ITEM_ID_INVALID, ITEM_ID_INVALID };
		unsigned hatcount = 0;
		for (auto &hat : m_hats)
		{
			static const char *const hatdirs[4] = { "Up", "Down", "Left", "Right" };
			uint8_t *const values[4] = { &hat.up, &hat.down, &hat.left, &hat.right };

			for (int i = 0; i < 4; i++)
			{
				const input_item_id actual = device.add_item(
						util::string_format("Hat %u %s", hatcount + 1, hatdirs[i]),
						std::string_view(),
						input_item_id(ITEM_ID_HAT1UP + (hatcount * 4) + i),
						generic_button_get_state<std::uint8_t>,
						values[i]);
				if (0 == hatcount)
				{
					hat1[i] = actual;
				}
			}
			hatcount++;
		}

		// use the main stick and first hat for primary movement controls
		add_directional_assignments(assignments, xaxis, yaxis, hat1[2], hat1[3], hat1[0], hat1[1]);

		// the first button makes a reasonable UI select
		add_button_assignment(assignments, IPT_UI_SELECT, { firstbutton });

		device.set_default_assignments(std::move(assignments));
	}

private:
	struct axis_info
	{
		IOHIDElementRef element;
		input_item_id   preferred;
		std::string     name;
		CFIndex         min, max;
		int32_t         value;
	};

	struct button_info
	{
		IOHIDElementRef element;
		std::string     name;
		uint8_t         value;
	};

	struct hat_info
	{
		IOHIDElementRef element;
		CFIndex         min;
		uint8_t         up, down, left, right;
	};

	void enumerate_elements()
	{
		CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_device, nullptr, kIOHIDOptionsTypeNone);
		if (elements == nullptr)
		{
			return;
		}

		// the same control can appear in multiple collections
		std::set<IOHIDElementCookie> seen;

		const CFIndex count = CFArrayGetCount(elements);
		for (CFIndex i = 0; i < count; i++)
		{
			const IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);

			const IOHIDElementType type = IOHIDElementGetType(element);
			if ((type != kIOHIDElementTypeInput_Misc) && (type != kIOHIDElementTypeInput_Button) && (type != kIOHIDElementTypeInput_Axis))
			{
				continue;
			}

			if (!seen.insert(IOHIDElementGetCookie(element)).second)
			{
				continue;
			}

			const uint32_t page = IOHIDElementGetUsagePage(element);
			const uint32_t usage = IOHIDElementGetUsage(element);

			if (page == kHIDPage_Button)
			{
				if (m_buttons.size() < MAX_BUTTONS)
				{
					CFRetain(element);
					m_buttons.emplace_back(button_info{ element, default_button_name(usage - 1), 0 });
				}
			}
			else if (page == kHIDPage_GenericDesktop)
			{
				static const struct { uint32_t usage; input_item_id item; const char *name; } axisdefs[] =
				{
					{ kHIDUsage_GD_X,      ITEM_ID_XAXIS,   "X" },
					{ kHIDUsage_GD_Y,      ITEM_ID_YAXIS,   "Y" },
					{ kHIDUsage_GD_Z,      ITEM_ID_ZAXIS,   "Z" },
					{ kHIDUsage_GD_Rx,     ITEM_ID_RXAXIS,  "RX" },
					{ kHIDUsage_GD_Ry,     ITEM_ID_RYAXIS,  "RY" },
					{ kHIDUsage_GD_Rz,     ITEM_ID_RZAXIS,  "RZ" },
					{ kHIDUsage_GD_Slider, ITEM_ID_SLIDER1, "Slider" },
					{ kHIDUsage_GD_Dial,   ITEM_ID_SLIDER2, "Dial" },
					{ kHIDUsage_GD_Wheel,  ITEM_ID_SLIDER2, "Wheel" }
				};

				if (usage == kHIDUsage_GD_Hatswitch)
				{
					if (m_hats.size() < MAX_HATS)
					{
						CFRetain(element);
						m_hats.emplace_back(hat_info{ element, IOHIDElementGetLogicalMin(element), 0, 0, 0, 0 });
					}
				}
				else
				{
					for (const auto &def : axisdefs)
					{
						if (usage == def.usage)
						{
							CFRetain(element);
							m_axes.emplace_back(axis_info{
									element,
									def.item,
									def.name,
									IOHIDElementGetLogicalMin(element),
									IOHIDElementGetLogicalMax(element),
									0 });
							break;
						}
					}
				}
			}
		}

		CFRelease(elements);
	}

	IOHIDDeviceRef m_device;
	bool m_connected;

	std::vector<axis_info> m_axes;
	std::vector<button_info> m_buttons;
	std::vector<hat_info> m_hats;
};


//============================================================
//  joystick_input_macjoy - IOHID joystick module
//============================================================

class joystick_input_macjoy : public input_module_impl<macjoy_device, mac_osd_interface>
{
public:
	joystick_input_macjoy() :
		input_module_impl<macjoy_device, mac_osd_interface>(OSD_JOYSTICKINPUT_PROVIDER, "macjoy"),
		m_manager(nullptr)
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		input_module_impl<macjoy_device, mac_osd_interface>::input_init(machine);

		m_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		if (m_manager == nullptr)
		{
			osd_printf_error("macjoy: could not create the HID manager\n");
			return;
		}

		// match joysticks, gamepads and multi-axis controllers
		static const uint32_t usages[] = { kHIDUsage_GD_Joystick, kHIDUsage_GD_GamePad, kHIDUsage_GD_MultiAxisController };
		CFMutableArrayRef matchers = CFArrayCreateMutable(kCFAllocatorDefault, std::size(usages), &kCFTypeArrayCallBacks);
		for (uint32_t usage : usages)
		{
			const uint32_t page = kHIDPage_GenericDesktop;
			CFNumberRef pageref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
			CFNumberRef usageref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);

			CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
					kCFAllocatorDefault, 2,
					&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), pageref);
			CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usageref);
			CFArrayAppendValue(matchers, dict);

			CFRelease(dict);
			CFRelease(usageref);
			CFRelease(pageref);
		}
		IOHIDManagerSetDeviceMatchingMultiple(m_manager, matchers);
		CFRelease(matchers);

		// hot-plug callbacks, serviced by the main run loop via the event pump
		IOHIDManagerRegisterDeviceMatchingCallback(m_manager, &joystick_input_macjoy::device_matched, this);
		IOHIDManagerRegisterDeviceRemovalCallback(m_manager, &joystick_input_macjoy::device_removed, this);
		IOHIDManagerScheduleWithRunLoop(m_manager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

		if (IOHIDManagerOpen(m_manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
		{
			osd_printf_error("macjoy: could not open the HID manager\n");
			return;
		}

		// add the devices that are already connected
		CFSetRef devices = IOHIDManagerCopyDevices(m_manager);
		if (devices != nullptr)
		{
			const CFIndex count = CFSetGetCount(devices);
			std::vector<const void *> refs(count);
			CFSetGetValues(devices, refs.data());
			for (const void *ref : refs)
			{
				add_hid_device((IOHIDDeviceRef)ref);
			}
			CFRelease(devices);
		}
	}

	virtual void exit() override
	{
		if (m_manager != nullptr)
		{
			IOHIDManagerUnscheduleFromRunLoop(m_manager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
			IOHIDManagerClose(m_manager, kIOHIDOptionsTypeNone);
			CFRelease(m_manager);
			m_manager = nullptr;
		}

		input_module_impl<macjoy_device, mac_osd_interface>::exit();
	}

private:
	void add_hid_device(IOHIDDeviceRef hiddevice)
	{
		if (hiddevice == nullptr)
		{
			return;
		}

		// reconnects get their existing device back
		bool found = false;
		devicelist().for_each_device(
				[hiddevice, &found] (auto &device)
				{
					if (device.has_device(hiddevice))
					{
						device.set_connected(true);
						found = true;
					}
				});
		if (found)
		{
			return;
		}

		std::string name = "HID Joystick";
		CFTypeRef product = IOHIDDeviceGetProperty(hiddevice, CFSTR(kIOHIDProductKey));
		if ((product != nullptr) && (CFGetTypeID(product) == CFStringGetTypeID()))
		{
			char buffer[256];
			if (CFStringGetCString((CFStringRef)product, buffer, sizeof(buffer), kCFStringEncodingUTF8))
			{
				name = buffer;
			}
		}
		std::string id = util::string_format("%s %u", name, unsigned(devicelist().size() + 1));

		osd_printf_verbose("macjoy: adding %s\n", name);
		create_device<macjoy_device>(DEVICE_CLASS_JOYSTICK, std::move(name), std::move(id), hiddevice);
	}

	void remove_hid_device(IOHIDDeviceRef hiddevice)
	{
		devicelist().for_each_device(
				[hiddevice] (auto &device)
				{
					if (device.has_device(hiddevice))
					{
						device.set_connected(false);
					}
				});
	}

	static void device_matched(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
	{
		if (result == kIOReturnSuccess)
		{
			static_cast<joystick_input_macjoy *>(context)->add_hid_device(device);
		}
	}

	static void device_removed(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
	{
		static_cast<joystick_input_macjoy *>(context)->remove_hid_device(device);
	}

	IOHIDManagerRef m_manager;
};

} // anonymous namespace

} // namespace osd

#else // defined(OSD_MAC)

#include "input_module.h"
#include "modules/osdmodule.h"

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(joystick_input_macjoy, OSD_JOYSTICKINPUT_PROVIDER, "macjoy")

} // anonymous namespace

} // namespace osd

#endif // defined(OSD_MAC)

MODULE_DEFINITION(JOYSTICKINPUT_MACJOY, osd::joystick_input_macjoy)
