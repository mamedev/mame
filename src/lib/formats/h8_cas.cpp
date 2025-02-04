// license:BSD-3-Clause
// copyright-holders:Robbbert,Mark Garlanger
/********************************************************************

Support for Heathkit H8/H88 H8T cassette images


Standard Kansas City format (300 baud)

TODO - investigate 1200 buad support, H8 should support it, but H88 does not.

We output a leader, followed by the contents of the H8T file.

********************************************************************/

#include "h8_cas.h"

#include "coretmpl.h" // BIT


namespace {

static constexpr double ONE_FREQ              = 1200.0;
static constexpr double ONE_FREQ_VARIANCE     = 300.0;
static constexpr double ZERO_FREQ             = 2400.0;
static constexpr double ZERO_FREQ_VARIANCE    = 600.0;

static const cassette_image::Modulation heath_h8t_modulation =
{
	cassette_image::MODULATION_SINEWAVE,
	ONE_FREQ - ONE_FREQ_VARIANCE,   ONE_FREQ,  ONE_FREQ + ONE_FREQ_VARIANCE,
	ZERO_FREQ - ZERO_FREQ_VARIANCE, ZERO_FREQ, ZERO_FREQ + ZERO_FREQ_VARIANCE
};

static cassette_image::error heath_h8t_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->modulation_identify(heath_h8t_modulation, opts);
}


#define MODULATE(_value) \
	for (int i = 0; i < (_value ? 8 : 4); i++) { \
		err = cassette->put_modulated_data_bit(0, time_index, _value, heath_h8t_modulation, &time_displacement); \
		if (err != cassette_image::error::SUCCESS) return err; \
		time_index += time_displacement; \
	}

static cassette_image::error heath_h8t_load(cassette_image *cassette)
{
	cassette_image::error err = cassette_image::error::SUCCESS;
	uint64_t image_size = cassette->image_size();
	double time_index = 0.0;
	double time_displacement;

	// leader - 1 second
	while (time_index < 1.0)
	{
		MODULATE(1);
	}

	for (uint64_t image_pos = 0; image_pos < image_size; image_pos++)
	{
		uint8_t data = cassette->image_read_byte(image_pos);

		// start bit
		MODULATE(0);

		// data bits
		for (int bit = 0; bit < 8; bit++)
		{
			MODULATE(util::BIT(data, bit));
		}

		// stop bit
		MODULATE(1);
	}

	return err;
}

const cassette_image::Format heath_h8t_format =
{
	"h8t",
	heath_h8t_identify,
	heath_h8t_load,
	nullptr
};

}

CASSETTE_FORMATLIST_START( h8_cassette_formats )
	CASSETTE_FORMAT( heath_h8t_format )
CASSETTE_FORMATLIST_END
