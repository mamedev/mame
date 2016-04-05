#ifndef INPUT_DINPUT_H_
#define INPUT_DINPUT_H_

#include "input_common.h"
#include "winutil.h"

//============================================================
//  dinput_device - base directinput device
//============================================================

// DirectInput-specific information about a device
struct dinput_api_state
{
	Microsoft::WRL::ComPtr<IDirectInputDevice>   device;
	Microsoft::WRL::ComPtr<IDirectInputDevice2>  device2;
	DIDEVCAPS                                    caps;
	LPCDIDATAFORMAT                              format;
};

class device_enum_interface
{
public:
	struct dinput_callback_context
	{
		device_enum_interface *     self;
		void *						state;
	};

	virtual ~device_enum_interface()
	{
	}

	static BOOL CALLBACK enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref)
	{
		auto context = static_cast<dinput_callback_context*>(ref);
		return context->self->device_enum_callback(instance, context->state);
	}

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) = 0;
};

typedef lazy_loaded_function_p4<HRESULT, HMODULE, int, IDirectInput **, LPUNKNOWN> pfn_dinput_create;

class dinput_api_helper
{
private:
	Microsoft::WRL::ComPtr<IDirectInput>  m_dinput;
	int                                   m_dinput_version;
	pfn_dinput_create                     m_pfn_DirectInputCreate;

public:
	dinput_api_helper(int version);
	virtual ~dinput_api_helper();
	int initialize();

	template<class TDevice>
	TDevice* create_device(
		running_machine &machine,
		input_module_base &module,
		LPCDIDEVICEINSTANCE instance,
		LPCDIDATAFORMAT format1,
		LPCDIDATAFORMAT format2,
		DWORD cooperative_level)
	{
		HRESULT result;

		// convert instance name to utf8
		auto osd_deleter = [](void *ptr) { osd_free(ptr); };
		auto utf8_instance_name = std::unique_ptr<char, decltype(osd_deleter)>(utf8_from_tstring(instance->tszInstanceName), osd_deleter);

		// allocate memory for the device object
		TDevice* devinfo = module.devicelist()->create_device<TDevice>(machine, utf8_instance_name.get(), module);

		// attempt to create a device
		result = m_dinput->CreateDevice(instance->guidInstance, devinfo->dinput.device.GetAddressOf(), nullptr);
		if (result != DI_OK)
			goto error;

		// try to get a version 2 device for it
		result = devinfo->dinput.device.CopyTo(IID_IDirectInputDevice2, reinterpret_cast<void**>(devinfo->dinput.device2.GetAddressOf()));
		if (result != DI_OK)
			devinfo->dinput.device2 = nullptr;

		// get the caps
		devinfo->dinput.caps.dwSize = sizeof(devinfo->dinput.caps);
		result = devinfo->dinput.device->GetCapabilities(&devinfo->dinput.caps);
		if (result != DI_OK)
			goto error;

		// attempt to set the data format
		devinfo->dinput.format = format1;
		result = devinfo->dinput.device->SetDataFormat(devinfo->dinput.format);
		if (result != DI_OK)
		{
			// use the secondary format if available
			if (format2 != nullptr)
			{
				devinfo->dinput.format = format2;
				result = devinfo->dinput.device->SetDataFormat(devinfo->dinput.format);
			}
			if (result != DI_OK)
				goto error;
		}

		// set the cooperative level
		result = devinfo->dinput.device->SetCooperativeLevel(win_window_list->m_hwnd, cooperative_level);
		if (result != DI_OK)
			goto error;

		return devinfo;

	error:
		module.devicelist()->free_device(devinfo);
		return nullptr;
	}

	HRESULT enum_attached_devices(int devclass, device_enum_interface *enumerate_interface, void *state) const;
};

class dinput_device : public device_info
{
public:
	dinput_api_state dinput;

	dinput_device(running_machine &machine, const char *name, input_device_class deviceclass, input_module &module);
	virtual ~dinput_device();

protected:
	HRESULT poll_dinput(LPVOID pState) const;
};

class dinput_keyboard_device : public dinput_device
{
private:
	std::mutex m_device_lock;

public:
	keyboard_state  keyboard;

	dinput_keyboard_device(running_machine &machine, const char *name, input_module &module);

	void poll() override;
	void reset() override;
};

class dinput_mouse_device : public dinput_device
{
public:
	mouse_state mouse;

public:
	dinput_mouse_device(running_machine &machine, const char *name, input_module &module);
	void poll() override;
	void reset() override;
};

// state information for a joystick; DirectInput state must be first element
struct dinput_joystick_state
{
	DIJOYSTATE              state;
	LONG                    rangemin[8];
	LONG                    rangemax[8];
};

class dinput_joystick_device : public dinput_device
{
public:
	dinput_joystick_state   joystick;
public:
	dinput_joystick_device(running_machine &machine, const char *name, input_module &module);
	void reset() override;
	void poll() override;
	int configure();
};


#endif
