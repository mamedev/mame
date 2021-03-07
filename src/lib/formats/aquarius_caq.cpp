// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    formats/aquarius_caq.cpp

    Cassette code for Aquarius .CAQ files

*********************************************************************/

#include "aquarius_caq.h"

#include "coretmpl.h" // BIT

#include <cassert>


/*-------------------------------------------------
    CassetteModulation aquarius_caq_modulation
-------------------------------------------------*/

static const cassette_image::Modulation aquarius_caq_modulation =
{
	cassette_image::MODULATION_SINEWAVE,
	900.0 - 300, 900.0, 900.0 + 300,
	1800.0 - 600, 1800.0, 1800.0 + 600


};

/*-------------------------------------------------
    aquarius_caq_identify - identify cassette
-------------------------------------------------*/

static cassette_image::error aquarius_caq_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->modulation_identify(aquarius_caq_modulation, opts);
}

/*-------------------------------------------------
    aquarius_caq_load - load cassette
-------------------------------------------------*/

#define MODULATE(_value) \
	for (int i = 0; i < 2; i++) { \
		err = cassette->put_modulated_data_bit(0, time_index, _value, aquarius_caq_modulation, &time_displacement); \
		if (err != cassette_image::error::SUCCESS) return err; \
		time_index += time_displacement; \
	}

static cassette_image::error aquarius_caq_load(cassette_image *cassette)
{
	cassette_image::error err;
	uint64_t image_size = cassette->image_size();
	double time_index = 0.0;
	double time_displacement;

	/* silence */
	err = cassette->put_sample(0, time_index, 0.5, 0);
	if (err != cassette_image::error::SUCCESS) return err;
	time_index += 0.5;

	for (uint64_t image_pos = 0; image_pos < image_size; image_pos++)
	{
		uint8_t data = cassette->image_read_byte(image_pos);

		/* start bit */
		MODULATE(0);

		/* data bits */
		for (int bit = 7; bit >= 0; bit--)
		{
			MODULATE(util::BIT(data, bit));
		}

		/* stop bits */
		MODULATE(1);
		MODULATE(1);
	}

	/* silence */
	err = cassette->put_sample(0, time_index, 0.5, 0);
	if (err != cassette_image::error::SUCCESS) return err;
	time_index += 0.5;

	return cassette_image::error::SUCCESS;
}

/*-------------------------------------------------
    CassetteFormat aquarius_caq_cassette_format
-------------------------------------------------*/

const cassette_image::Format aquarius_caq_format =
{
	"caq",
	aquarius_caq_identify,
	aquarius_caq_load,
	nullptr
};

CASSETTE_FORMATLIST_START( aquarius_cassette_formats )
	CASSETTE_FORMAT(aquarius_caq_format)
CASSETTE_FORMATLIST_END
