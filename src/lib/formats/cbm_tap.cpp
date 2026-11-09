// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Curt Coder
/*

    Commodore TAP format

    Based on http://ist.uwaterloo.ca/~schepers/formats/TAP.TXT

    $0000-$000B  signature "C64-TAPE-RAW" or "C16-TAPE-RAW"
    $000C        version
    $000D        platform: 0 = C64, 1 = VIC-20, 2 = C16/Plus/4, 3 = PET,
                 4 = CBM-II P series, 5 = CBM-II B series
    $000E        video: 0 = PAL, 1 = NTSC, 2 = old NTSC, 3 = PAL-N
    $000F        reserved
    $0010-$0013  data size, little endian
    $0014-       data

    Version 0: each byte is the length of a full pulse in units of
    8 clock cycles, with $00 meaning an overflow longer than 255 * 8.

    Version 1: as version 0, but $00 is followed by the exact pulse
    length in clock cycles as a 24-bit little endian value.

    Version 2: as version 1, but each value is the length of a half
    pulse, as used by the C16/Plus/4 tape routines.

    Pulse ends are stored in ticks of half a clock cycle.

*/

#include "cbm_tap.h"

#include "multibyte.h"

#include <cstring>

namespace {

enum : uint8_t
{
	TAP_HEADER_VERSION = 0x0c,
	TAP_HEADER_PLATFORM = 0x0d,
	TAP_HEADER_VIDEO = 0x0e,
	TAP_HEADER_SIZE = 0x10,
	TAP_HEADER_LENGTH = 0x14
};

enum : uint8_t
{
	TAP_PLATFORM_C64 = 0,
	TAP_PLATFORM_VIC20,
	TAP_PLATFORM_C16,
	TAP_PLATFORM_PET,
	TAP_PLATFORM_C5X0,
	TAP_PLATFORM_C6X0
};

enum : uint8_t
{
	TAP_VIDEO_PAL = 0,
	TAP_VIDEO_NTSC,
	TAP_VIDEO_OLD_NTSC,
	TAP_VIDEO_PALN
};

constexpr uint8_t TAP_VERSION_HALF_PULSES = 2;

uint32_t tap_clock(uint8_t platform, uint8_t video)
{
	switch (platform)
	{
	case TAP_PLATFORM_C64:
	case TAP_PLATFORM_C5X0:
		switch (video)
		{
		case TAP_VIDEO_PAL: return 985'248;
		case TAP_VIDEO_NTSC:
		case TAP_VIDEO_OLD_NTSC: return 1'022'730;
		case TAP_VIDEO_PALN: return 1'023'440;
		default: return 0;
		}
	case TAP_PLATFORM_VIC20:
		return (video == TAP_VIDEO_PAL) ? 1'108'405 : (video == TAP_VIDEO_NTSC) ? 1'022'727 : 0;
	case TAP_PLATFORM_C16:
		return (video == TAP_VIDEO_PAL) ? 886'724 : (video == TAP_VIDEO_NTSC) ? 894'886 : 0;
	case TAP_PLATFORM_PET:
		return (video <= TAP_VIDEO_NTSC) ? 1'000'000 : 0;
	case TAP_PLATFORM_C6X0:
		return (video <= TAP_VIDEO_NTSC) ? 2'000'000 : 0;
	default:
		return 0;
	}
}

cassette_image::error cbm_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	uint8_t header[TAP_HEADER_LENGTH];
	if (cassette->image_size() < sizeof(header))
		return cassette_image::error::INVALID_IMAGE;
	cassette->image_read(header, 0, sizeof(header));
	if ((std::memcmp(header, "C64-TAPE-RAW", 12) && std::memcmp(header, "C16-TAPE-RAW", 12)) || header[TAP_HEADER_VERSION] > TAP_VERSION_HALF_PULSES)
		return cassette_image::error::INVALID_IMAGE;
	uint32_t const clock = tap_clock(header[TAP_HEADER_PLATFORM], header[TAP_HEADER_VIDEO]);
	if (!clock || uint64_t(get_u32le(&header[TAP_HEADER_SIZE])) + sizeof(header) > cassette->image_size())
		return cassette_image::error::INVALID_IMAGE;
	opts->channels = 1;
	opts->bits_per_sample = 16;
	opts->sample_frequency = clock * 2;
	return cassette_image::error::SUCCESS;
}

cassette_image::error cbm_cassette_load(cassette_image *cassette)
{
	uint8_t header[TAP_HEADER_LENGTH];
	cassette->image_read(header, 0, sizeof(header));
	std::vector<uint8_t> data(get_u32le(&header[TAP_HEADER_SIZE]));
	cassette->image_read(data.data(), sizeof(header), data.size());
	std::vector<cassette_image::Pulse> pulses;
	uint64_t end = 0;
	int32_t const amplitude = (0x5a9e >> 1) * 65536;
	int32_t value = (header[TAP_HEADER_VERSION] == TAP_VERSION_HALF_PULSES) ? amplitude : -amplitude;
	for (size_t i = 0; i < data.size(); ++i)
	{
		uint32_t cycles = uint32_t(data[i]) * 8;
		if (!cycles)
		{
			if (!header[TAP_HEADER_VERSION])
				cycles = 256 * 8;
			else
			{
				if (data.size() - i < 4)
					return cassette_image::error::INVALID_IMAGE;
				cycles = get_u24le(&data[i + 1]);
				i += 3;
				if (!cycles)
					return cassette_image::error::INVALID_IMAGE;
			}
		}
		if (header[TAP_HEADER_VERSION] == TAP_VERSION_HALF_PULSES)
		{
			end += uint64_t(cycles) * 2;
			pulses.push_back({ end, value });
			value = -value;
		}
		else
		{
			end += cycles;
			pulses.push_back({ end, -amplitude });
			end += cycles;
			pulses.push_back({ end, amplitude });
		}
	}
	if (pulses.empty())
		return cassette_image::error::INVALID_IMAGE;
	cassette->set_pulses(std::move(pulses));
	return cassette_image::error::SUCCESS;
}

const cassette_image::Format cbm_tap_cassette_format = {
	"tap",
	cbm_cassette_identify,
	cbm_cassette_load,
	nullptr
};

} // anonymous namespace

CASSETTE_FORMATLIST_START(cbm_cassette_formats)
	CASSETTE_FORMAT(cbm_tap_cassette_format)
CASSETTE_FORMATLIST_END
