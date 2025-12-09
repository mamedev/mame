// license:BSD-3-Clause
// copyright-holders: Brad Hughes
/*
* monitor_dxgi.cpp
*
*/

#include "monitor_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

// local headers
#include "monitor_common.h"

// OSD headers
#include "modules/lib/osdlib.h"
#include "osdcore.h"
#include "strconv.h"
#include "window.h"
#include "windows/video.h"

// standard windows headers
#include <windows.h>

#include <dxgi1_2.h>
#include <objbase.h>
#include <wrl/client.h>
#undef interface


namespace osd {

namespace {

using dxgi_factory_ptr = Microsoft::WRL::ComPtr<IDXGIFactory2>;
using dxgi_adapter_ptr = Microsoft::WRL::ComPtr<IDXGIAdapter>;
using dxgi_output_ptr  = Microsoft::WRL::ComPtr<IDXGIOutput>;


class dxgi_monitor_info : public osd_monitor_info
{
private:
	dxgi_output_ptr m_output;    // The output interface

public:
	dxgi_monitor_info(monitor_module& module, HMONITOR handle, std::string &&monitor_device, float aspect, dxgi_output_ptr output) :
		osd_monitor_info(module, reinterpret_cast<std::uint64_t>(handle), std::move(monitor_device), aspect),
		m_output(output)
	{
		refresh();
	}

	void refresh() override
	{
		DXGI_OUTPUT_DESC desc;
		m_output->GetDesc(&desc);

		// fetch the latest info about the monitor
		m_name = osd::text::from_wstring(desc.DeviceName);

		m_pos_size = RECT_to_osd_rect(desc.DesktopCoordinates);
		m_usuable_pos_size = RECT_to_osd_rect(desc.DesktopCoordinates);
		m_is_primary = desc.AttachedToDesktop;
	}
};


class dxgi_monitor_module : public monitor_module_base
{
private:
	OSD_DYNAMIC_API(dxgi, "dxgi.dll");
	OSD_DYNAMIC_API_FN(dxgi, DWORD, WINAPI, CreateDXGIFactory1, REFIID, void**);
public:
	dxgi_monitor_module()
		: monitor_module_base(OSD_MONITOR_PROVIDER, "dxgi")
	{
	}

	bool probe() override
	{
		if(!OSD_DYNAMIC_API_TEST(CreateDXGIFactory1))
			return false;

		return true;
	}

	// Currently this method implementation is duplicated from the win32 module
	// however it needs to also handle being able to do this for UWP
	std::shared_ptr<osd_monitor_info> monitor_from_rect(const osd_rect& rect) override
	{
		if (!m_initialized)
			return nullptr;

		RECT p;
		p.top = rect.top();
		p.left = rect.left();
		p.bottom = rect.bottom();
		p.right = rect.right();

		auto nearest = monitor_from_handle(reinterpret_cast<std::uintptr_t>(MonitorFromRect(&p, MONITOR_DEFAULTTONEAREST)));
		assert(nearest != nullptr);
		return nearest;
	}

	// Currently this method implementation is duplicated from the win32 module
	// however it needs to also handle being able to do this for UWP
	std::shared_ptr<osd_monitor_info> monitor_from_window(const osd_window& window) override
	{
		if (!m_initialized)
			return nullptr;

		auto nearest = monitor_from_handle(reinterpret_cast<std::uintptr_t>(MonitorFromWindow(static_cast<const win_window_info &>(window).platform_window(), MONITOR_DEFAULTTONEAREST)));
		assert(nearest != nullptr);
		return nearest;
	}

protected:
	int init_internal(const osd_options& options) override
	{
		HRESULT result;
		dxgi_factory_ptr factory;
		dxgi_adapter_ptr adapter;

		result = OSD_DYNAMIC_CALL(CreateDXGIFactory1, IID_PPV_ARGS(factory.GetAddressOf()));
		if (result != ERROR_SUCCESS)
		{
			osd_printf_error("CreateDXGIFactory1 failed with error 0x%x\n", static_cast<unsigned int>(result));
			return 1;
		}

		UINT iAdapter = 0;
		while (!factory->EnumAdapters(iAdapter, adapter.ReleaseAndGetAddressOf()))
		{
			UINT i = 0;
			dxgi_output_ptr output;
			DXGI_OUTPUT_DESC desc;
			while (!adapter->EnumOutputs(i, &output))
			{
				output->GetDesc(&desc);

				// guess the aspect ratio assuming square pixels
				RECT const &coords = desc.DesktopCoordinates;
				float aspect = float(coords.right - coords.left) / float(coords.bottom - coords.top);

				// allocate a new monitor info
				std::string devicename = osd::text::from_wstring(desc.DeviceName);

				// allocate a new monitor info
				auto monitor = std::make_shared<dxgi_monitor_info>(*this, desc.Monitor, std::move(devicename), aspect, output);

				// hook us into the list
				add_monitor(monitor);

				i++;
			}

			iAdapter++;
		}

		// if we're verbose, print the list of monitors
		for (const auto &monitor : list())
		{
			osd_printf_verbose("Video: Monitor %u = \"%s\" %s\n", monitor->oshandle(), monitor->devicename(), monitor->is_primary() ? "(primary)" : "");
		}

		return 0;
	}
};

} // anonymous namespace

} // namespace osd

#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(dxgi_monitor_module, OSD_MONITOR_PROVIDER, "dxgi") } }

#endif

MODULE_DEFINITION(MONITOR_DXGI, osd::dxgi_monitor_module)
