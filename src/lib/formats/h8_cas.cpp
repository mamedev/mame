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

constexpr double ONE_FREQ              = 1200.0;
constexpr double ONE_FREQ_VARIANCE     = 300.0;
constexpr double ZERO_FREQ             = 2400.0;
constexpr double ZERO_FREQ_VARIANCE    = 600.0;

const cassette_image::Modulation heath_h8t_modulation =
{
	cassette_image::MODULATION_SINEWAVE,
	ONE_FREQ - ONE_FREQ_VARIANCE,   ONE_FREQ,  ONE_FREQ + ONE_FREQ_VARIANCE,
	ZERO_FREQ - ZERO_FREQ_VARIANCE, ZERO_FREQ, ZERO_FREQ + ZERO_FREQ_VARIANCE
};

cassette_image::error heath_h8t_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->modulation_identify(heath_h8t_modulation, opts);
}


cassette_image::error heath_h8t_load(cassette_image *cassette)
{
	cassette_image::error err = cassette_image::error::SUCCESS;
	uint64_t image_size = cassette->image_size();
	double time_index = 0.0;
	double time_displacement;

	auto const MODULATE =
			[&cassette, &err, &time_index, &time_displacement] (unsigned value)
			{
				for (int i = 0; (i < (value ? 8 : 4)); i++)
				{
					err = cassette->put_modulated_data_bit(0, time_index, value, heath_h8t_modulation, &time_displacement);
					if (cassette_image::error::SUCCESS == err)
						time_index += time_displacement;
					else
						return;
				}
			};

	// leader - 1 second
	while ((cassette_image::error::SUCCESS == err) && (time_index < 1.0))
		MODULATE(1);

	for (uint64_t image_pos = 0; (cassette_image::error::SUCCESS == err) && (image_pos < image_size); image_pos++)
	{
		uint8_t data = cassette->image_read_byte(image_pos);

		// start bit
		MODULATE(0);

		// data bits
		for (int bit = 0; (cassette_image::error::SUCCESS == err) && (bit < 8); bit++)
			MODULATE(util::BIT(data, bit));

		// stop bit
		if (cassette_image::error::SUCCESS == err)
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
