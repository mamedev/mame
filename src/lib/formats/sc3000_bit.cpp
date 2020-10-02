// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/sc3000_bit.c

    Cassette code for Sega SC-3000 *.bit files

*********************************************************************/
#include "sc3000_bit.h"

#include <cassert>


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 0

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    CassetteModulation sc3000_bit_modulation
-------------------------------------------------*/

static const cassette_image::Modulation sc3000_bit_modulation =
{
	cassette_image::MODULATION_SINEWAVE,
	1200.0 - 300, 1200.0, 1200.0 + 300,
	2400.0 - 600, 2400.0, 2400.0 + 600
};

/*-------------------------------------------------
    sc3000_bit_identify - identify cassette
-------------------------------------------------*/

static cassette_image::error sc3000_bit_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->modulation_identify(sc3000_bit_modulation, opts);
}

/*-------------------------------------------------
    sc3000_bit_load - load cassette
-------------------------------------------------*/

#define MODULATE(_value) \
	for (int i = 0; i < (_value ? 2 : 1); i++) { \
		err = cassette->put_modulated_data_bit(0, time_index, _value, sc3000_bit_modulation, &time_displacement);\
		if (err != cassette_image::error::SUCCESS) return err;\
		time_index += time_displacement;\
	}

static cassette_image::error sc3000_bit_load(cassette_image *cassette)
{
	cassette_image::error err;
	uint64_t image_size = cassette->image_size();
	uint64_t image_pos = 0;
	double time_index = 0.0;
	double time_displacement;
	uint8_t data;

	while (image_pos < image_size)
	{
		cassette->image_read(&data, image_pos, 1);

		switch (data)
		{
		case '1':
			MODULATE(1);
			break;

		case '0':
			MODULATE(0);
			break;

		case ' ':
			err = cassette->put_sample(0, time_index, 1/1200.0, 0);
			if (err != cassette_image::error::SUCCESS) return err;
			time_index += 1/1200.0;
			break;
		}

		image_pos++;
	}

	return cassette_image::error::SUCCESS;
}

/*-------------------------------------------------
    CassetteFormat sc3000_bit_cassette_format
-------------------------------------------------*/

const cassette_image::Format sc3000_bit_format =
{
	"bit",
	sc3000_bit_identify,
	sc3000_bit_load,
	nullptr
};

CASSETTE_FORMATLIST_START( sc3000_cassette_formats )
	CASSETTE_FORMAT(sc3000_bit_format)
CASSETTE_FORMATLIST_END
