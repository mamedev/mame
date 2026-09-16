// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_OSD_SOUND_MMDEVICE_HELPERS_H
#define MAME_OSD_SOUND_MMDEVICE_HELPERS_H

#pragma once

#if defined(_WIN32)

#include "interface/audio.h"

#include <cstring>
#include <memory>
#include <optional>
#include <string>

#include <windows.h>

#include <objbase.h>
#include <propidl.h>
#include <propsys.h>

#include <mmdeviceapi.h>
#include <mmreg.h>

#include <wrl/client.h>


namespace osd {

struct prop_variant_helper
{
	PROPVARIANT value;

	prop_variant_helper() { PropVariantInit(&value); }
	~prop_variant_helper() { PropVariantClear(&value); }
	prop_variant_helper(prop_variant_helper const &) = delete;
	prop_variant_helper &operator=(prop_variant_helper const &) = delete;
};

struct handle_deleter
{
	void operator()(HANDLE obj) const
	{
		if (obj)
			CloseHandle(obj);
	}
};

struct co_task_mem_deleter
{
	template <typename T>
	void operator()(T *obj) const
	{
		if (obj)
			CoTaskMemFree(obj);
	}
};

using handle_ptr               = std::unique_ptr<std::remove_pointer_t<HANDLE>, handle_deleter>;

using co_task_wstr_ptr         = std::unique_ptr<wchar_t, co_task_mem_deleter>;

using mm_device_ptr            = Microsoft::WRL::ComPtr<IMMDevice>;
using mm_device_collection_ptr = Microsoft::WRL::ComPtr<IMMDeviceCollection>;
using mm_device_enumerator_ptr = Microsoft::WRL::ComPtr<IMMDeviceEnumerator>;
using property_store_ptr       = Microsoft::WRL::ComPtr<IPropertyStore>;


template <typename T>
HRESULT enumerate_audio_endpoints(
		IMMDeviceEnumerator &enumerator,
		EDataFlow data_flow,
		DWORD state_mask,
		T &&action)
{
	HRESULT result;

	// get devices
	mm_device_collection_ptr devices;
	result = enumerator.EnumAudioEndpoints(data_flow, state_mask, devices.GetAddressOf());
	if (FAILED(result))
		return result;

	// count devices
	UINT count;
	result = devices->GetCount(&count);
	if (FAILED(result))
		return result;

	// enumerate devices
	for (UINT i = 0; count > i; ++i)
	{
		mm_device_ptr device;
		result = devices->Item(i, device.GetAddressOf());
		if (!action(result, device))
			break;
	}

	return S_OK;
}

HRESULT populate_audio_node_info(
		IMMDevice &device,
		std::wstring &device_id,
		audio_info::node_info &info);

HRESULT get_default_audio_device_id(
		IMMDeviceEnumerator &enumerator,
		EDataFlow data_flow,
		ERole role,
		co_task_wstr_ptr &value);

HRESULT get_string_property_value(
		IPropertyStore &properties,
		REFPROPERTYKEY key,
		std::optional<std::string> &value);

inline void populate_wave_format(
		WAVEFORMATEXTENSIBLE &format,
		DWORD channels,
		DWORD rate,
		std::optional<DWORD> positions)
{
	std::memset(&format, 0, sizeof(format));

	format.Format.wFormatTag = WAVE_FORMAT_PCM;
	format.Format.nChannels = channels;
	format.Format.nSamplesPerSec = rate;
	format.Format.nAvgBytesPerSec = 2 * channels * rate;
	format.Format.nBlockAlign = 2 * channels;
	format.Format.wBitsPerSample = 16;
	format.Format.cbSize = 0;

	if (positions || (2 < channels))
	{
		format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		format.Format.cbSize = sizeof(format) - sizeof(format.Format);
		format.Samples.wValidBitsPerSample = 16;
		format.dwChannelMask = positions ? *positions : 0;
		format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	}
}

} // namespace osd

#endif // defined(_WIN32)

#endif // MAME_OSD_SOUND_MMDEVICE_HELPERS_H
