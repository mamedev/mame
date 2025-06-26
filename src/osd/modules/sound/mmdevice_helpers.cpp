// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "mmdevice_helpers.h"

#if defined(_WIN32)

#include "osdcore.h"
#include "strconv.h"

#include "util/coretmpl.h"

#include <array>
#include <string_view>
#include <utility>
#include <vector>

#include <combaseapi.h>
#include <oleauto.h>

#include <mmreg.h>

#include <functiondiscoverykeys_devpkey.h>


namespace osd {

namespace {

using mm_endpoint_ptr = Microsoft::WRL::ComPtr<IMMEndpoint>;

char const *const f_speaker_names[] = {
		"FL",       // SPEAKER_FRONT_LEFT
		"FR",       // SPEAKER_FRONT_RIGHT
		"FC",       // SPEAKER_FRONT_CENTER
		"LFE",      // SPEAKER_LOW_FREQUENCY
		"BL",       // SPEAKER_BACK_LEFT
		"BR",       // SPEAKER_BACK_RIGHT
		"FCL",      // SPEAKER_FRONT_LEFT_OF_CENTER
		"FCR",      // SPEAKER_FRONT_RIGHT_OF_CENTER
		"BC",       // SPEAKER_BACK_CENTER
		"SL",       // SPEAKER_SIDE_LEFT
		"SR",       // SPEAKER_SIDE_RIGHT
		"TC",       // SPEAKER_TOP_CENTER
		"TFL",      // SPEAKER_TOP_FRONT_LEFT
		"TFC",      // SPEAKER_TOP_FRONT_CENTER
		"TFR",      // SPEAKER_TOP_FRONT_RIGHT
		"TBL",      // SPEAKER_TOP_BACK_LEFT
		"TBC",      // SPEAKER_TOP_BACK_CENTER
		"TBR" };    // SPEAKER_TOP_BACK_RIGHT

channel_position const f_speaker_positions[] = {
		channel_position::FL(),                 // SPEAKER_FRONT_LEFT
		channel_position::FR(),                 // SPEAKER_FRONT_RIGHT
		channel_position::FC(),                 // SPEAKER_FRONT_CENTER
		channel_position::LFE(),                // SPEAKER_LOW_FREQUENCY
		channel_position::RL(),                 // SPEAKER_BACK_LEFT
		channel_position::RR(),                 // SPEAKER_BACK_RIGHT
		channel_position( -0.1,  0.0,  1.0 ),   // SPEAKER_FRONT_LEFT_OF_CENTER
		channel_position(  0.1,  0.0,  1.0 ),   // SPEAKER_FRONT_RIGHT_OF_CENTER
		channel_position::RC(),                 // SPEAKER_BACK_CENTER
		channel_position( -0.2,  0.0,  0.0 ),   // SPEAKER_SIDE_LEFT
		channel_position(  0.2,  0.0,  0.0 ),   // SPEAKER_SIDE_RIGHT
		channel_position(  0.0,  0.5,  0.0 ),   // SPEAKER_TOP_CENTER
		channel_position( -0.2,  0.5,  1.0 ),   // SPEAKER_TOP_FRONT_LEFT
		channel_position(  0.0,  0.5,  1.0 ),   // SPEAKER_TOP_FRONT_CENTER
		channel_position(  0.2,  0.5,  1.0 ),   // SPEAKER_TOP_FRONT_RIGHT
		channel_position( -0.2,  0.5, -0.5 ),   // SPEAKER_TOP_BACK_LEFT
		channel_position(  0.0,  0.5, -0.5 ),   // SPEAKER_TOP_BACK_CENTER
		channel_position(  0.2,  0.5, -0.5 ) }; // SPEAKER_TOP_BACK_RIGHT

} // anonymous namespace


HRESULT populate_audio_node_info(
		IMMDevice &device,
		std::wstring &device_id,
		audio_info::node_info &info)
{
	HRESULT result;

	// get the device ID
	co_task_wstr_ptr device_id_w;
	std::string id_string;
	{
		LPWSTR id_raw = nullptr;
		result = device.GetId(&id_raw);
		if (FAILED(result) || !id_raw)
		{
			osd_printf_error("Error getting ID for audio device. Error: 0x%X\n", result);
			return FAILED(result) ? result : E_POINTER;
		}
		device_id_w.reset(std::exchange(id_raw, nullptr));
		try
		{
			osd::text::from_wstring(id_string, device_id_w.get());
		}
		catch (std::bad_alloc const &)
		{
			return E_OUTOFMEMORY;
		}
	}

	// get the property store (needed for various important things)
	property_store_ptr properties;
	result = device.OpenPropertyStore(STGM_READ, properties.GetAddressOf());
	if (FAILED(result) || !properties)
	{
		osd_printf_error(
				"Error opening property store for audio device %s. Error: 0x%X\n",
				id_string,
				result);
		return FAILED(result) ? result : E_POINTER;
	}

	// get the display name
	std::string device_name;
	{
		std::optional<std::string> name_string;
		result = get_string_property_value(*properties.Get(), PKEY_Device_FriendlyName, name_string);
		if (FAILED(result) || !name_string)
		{
			// fall back to using device ID
			osd_printf_error(
					"Error getting display name for audio device %s. Error: 0x%X\n",
					id_string,
					result);
			try
			{
				device_name = id_string;
			}
			catch (std::bad_alloc const &)
			{
				return E_OUTOFMEMORY;
			}
		}
		else
		{
			device_name = std::move(*name_string);
		}
	}

	// see whether it's an input or output
	EDataFlow data_flow;
	{
		mm_endpoint_ptr endpoint;
		result = device.QueryInterface(endpoint.GetAddressOf());
		if (FAILED(result) || !endpoint)
		{
			osd_printf_error(
					"Error getting endpoint information for audio device %s. Error: 0x%X\n",
					device_name,
					result);
			return FAILED(result) ? result : E_POINTER;
		}

		result = endpoint->GetDataFlow(&data_flow);
		if (FAILED(result))
		{
			osd_printf_error(
					"Error getting data flow direction for audio device %s. Error: 0x%X\n",
					device_name,
					result);
			return result;
		}

		if ((eRender != data_flow) && (eCapture != data_flow))
		{
			osd_printf_error(
					"Invalid data flow direction for audio device %s. Value: %u\n",
					device_name,
					std::underlying_type_t<EDataFlow>(data_flow));
			return E_INVALIDARG;
		}
	}

	// get format information
	prop_variant_helper format_property;
	result = properties->GetValue(PKEY_AudioEngine_DeviceFormat, &format_property.value);
	if (FAILED(result))
	{
		osd_printf_error(
				"Error getting stream format for audio device %s. Error: 0x%X\n",
				device_name,
				result);
		return result;
	}
	else if (VT_BLOB != format_property.value.vt)
	{
		// you can get VT_EMPTY when a device is initially added - don't warn about it
		if (VT_EMPTY != format_property.value.vt)
		{
			osd_printf_error(
					"Stream format has invalid data type for audio device %s. Type: %u\n",
					device_name,
					std::underlying_type_t<VARENUM>(format_property.value.vt));
		}
		return E_INVALIDARG;
	}
	auto const format = reinterpret_cast<WAVEFORMATEX const *>(format_property.value.blob.pBlobData);

	// get the channel mask for speaker positions
	std::optional<std::uint32_t> channel_mask;
	{
		prop_variant_helper speakers_property;
		result = properties->GetValue(PKEY_AudioEndpoint_PhysicalSpeakers, &speakers_property.value);
		if (FAILED(result))
		{
			osd_printf_error(
					"Error getting speaker arrangement for audio device %s. Error: 0x%X\n",
					device_name,
					result);
		}
		else switch (speakers_property.value.vt)
		{
		case VT_EMPTY:
			break;
		case VT_UI4:
			channel_mask = speakers_property.value.ulVal;
			break;
		case VT_UINT:
			channel_mask = speakers_property.value.uintVal;
			break;
		default:
			osd_printf_error(
					"Speaker arrangement has invalid data type for audio device %s. Type: %u\n",
					device_name,
					std::underlying_type_t<VARENUM>(speakers_property.value.vt));
		}
	}
	if (!channel_mask && (WAVE_FORMAT_EXTENSIBLE == format->wFormatTag))
	{
		auto const extensible_format = reinterpret_cast<WAVEFORMATEXTENSIBLE const *>(format);
		channel_mask = extensible_format->dwChannelMask;
	}

	// set up channel info
	std::vector<std::string> channel_names;
	std::vector<channel_position> channel_positions;
	try
	{
		channel_names.reserve(format->nChannels);
		channel_positions.reserve(format->nChannels);
		DWORD i = 0;
		if ((eRender == data_flow) && channel_mask)
		{
			static_assert(std::size(f_speaker_names) == std::size(f_speaker_positions));
			DWORD b = 0;
			while ((format->nChannels > i) && (std::size(f_speaker_names) > b))
			{
				if (util::BIT(*channel_mask, b))
				{
					channel_names.emplace_back(f_speaker_names[b]);
					channel_positions.emplace_back(f_speaker_positions[b]);
					++i;
				}
				++b;
			}
		}
		while (format->nChannels > i)
		{
			channel_names.emplace_back(util::string_format("Channel %u", i + 1));
			++i;
		}
		channel_positions.resize(format->nChannels, channel_position::UNKNOWN());
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}

	// set results
	try
	{
		device_id = device_id_w.get();
		info.m_name = osd::text::from_wstring(device_id);
		info.m_display_name = std::move(device_name);
		info.m_rate.m_default_rate = format->nSamplesPerSec;
		info.m_rate.m_min_rate = format->nSamplesPerSec;
		info.m_rate.m_max_rate = format->nSamplesPerSec;
		info.m_port_names = std::move(channel_names);
		info.m_port_positions = std::move(channel_positions);
		info.m_sinks = (eRender == data_flow) ? format->nChannels : 0;
		info.m_sources = (eCapture == data_flow) ? format->nChannels : 0;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


HRESULT get_default_audio_device_id(
		IMMDeviceEnumerator &enumerator,
		EDataFlow data_flow,
		ERole role,
		co_task_wstr_ptr &value)
{
	HRESULT result;

	mm_device_ptr device;
	LPWSTR id_raw = nullptr;
	result = enumerator.GetDefaultAudioEndpoint(data_flow, role, device.GetAddressOf());
	if (SUCCEEDED(result) && device)
		result = device->GetId(&id_raw);

	value.reset(std::exchange(id_raw, nullptr));
	return result;
}


HRESULT get_string_property_value(
		IPropertyStore &properties,
		REFPROPERTYKEY key,
		std::optional<std::string> &value)
{
	prop_variant_helper property_value;
	HRESULT const result = properties.GetValue(key, &property_value.value);
	if (FAILED(result))
		return result;

	try
	{
		switch (property_value.value.vt)
		{
		case VT_EMPTY:
			value = std::nullopt;
			return result;

		case VT_BSTR:
			value = osd::text::from_wstring(std::wstring_view(property_value.value.bstrVal, SysStringLen(property_value.value.bstrVal)));
			return result;

		case VT_LPSTR:
			value = osd::text::from_astring(property_value.value.pszVal);
			return result;

		case VT_LPWSTR:
			value = osd::text::from_wstring(property_value.value.pwszVal);
			return result;

		default:
			return E_INVALIDARG;
		}
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}

} // namespace osd

#endif // defined(_WIN32)
