// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/**************************************************************************

    coco_cas.c

    Format code for CoCo CAS (*.cas) files

    This is the actual code that processes the CAS data.  CAS files are a
    problem because they are a legacy of previous CoCo emulators, and most
    of these emulators patch the system as a shortcut.  Thus, the ones and
    zeroes in the file do not truly represent modulated pulses in the
    waveform, but what the BIOS reads and writes.

    In doing so, they make the CoCo more tolerant of short headers and lack
    of delays between blocks.  This legacy is reflected in most CAS files in
    use, and thus presents a problem for a pure hardware emulation like MESS.

    One alternative is to preprocess the data on the CAS, file by file, but
    this proves to be problematic because in the process, legitimate data
    that is unrecognized by the preprocessor may get dropped.

    The approach taken here is a hybrid approach - it retrieves the data
    block by block until an error occurs; be it the end of the CAS or a
    corrupt (?!) block.  When "legitimate" blocks are done processing, the
    remainder of the data is added to the waveform in a traditional manner.
    The result has proven to work quite well.

    2005-May-04, P.Harvey-Smith, improved handling of some "copy protected"
    Dragon games that have odd blocks in odd places, this has slightly
    increased loading time but does mean games like "Rommel's Revenge" will
    now load.

**************************************************************************/

#include "coco_cas.h"

#include <cassert>


#define COCO_WAVESAMPLES_HEADER     (1.0)
#define COCO_WAVESAMPLES_TRAILER    (1.0)
#define COCO_LONGSILENCE            (5.0)

//some games load with only 5s, but most games need 15s
#define ALICE32_WAVESAMPLES_HEADER  (15.0)

const cassette_image::Modulation coco_cas_modulation =
{
	cassette_image::MODULATION_SINEWAVE,
	600.0,  1200.0, 1500.0,
	1500.0, 2400.0, 3000.0
};



static cassette_image::error coco_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->modulation_identify(coco_cas_modulation, opts);
}



static bool get_cas_block(cassette_image *cassette, uint64_t &offset, uint8_t *block, int &block_len, int &synccount)
{
	uint8_t block_length = 0;
	uint8_t block_checksum = 0;
	int state = 0;
	int phase = 0;

	synccount = 0;
	uint16_t p = 0;
	uint16_t image_size = cassette->image_size();

	for(uint64_t current_offset = offset; current_offset < image_size; )
	{
		assert((p & 0xFF00) == 0);
		p |= cassette->image_read_byte(current_offset) << 8;
		current_offset++;

		for (int i = 0; i < 8; i++)
		{
			p >>= 1;

			if (state == 0)
			{
				/* searching for a block */
				if ((p & 0xFF) == 0x3C)
				{
					/* found one! */
					phase = i;
					state++;
				}
				else if ((p & 0xFF) == 0x55)
				{
					synccount++;
				}
			}
			else if (i == phase)
			{
				uint8_t b = p & 0xFF;
				*(block++) = b;
				switch(state) {
				case 1:
					/* found file type */
					block_checksum = b;
					state++;
					break;
				case 2:
					/* found file size */
					block_length = b;
					block_len = ((int) block_length) + 3;
					block_checksum += b;
					state++;
					break;

				case 3:
					/* data byte */
					if (block_length)
					{
						block_length--;
						block_checksum += b;
					}
					else
					{
						/* end of block */
						if (b != block_checksum)
						{
							/* checksum failure */
							return false;
						}
						else
						{
							/* checksum success */
							offset = current_offset;
							return true;
						}
					}
				}
			}
		}
	}

	/* no more blocks */
	return false;
}



static cassette_image::error cas_load(cassette_image *cassette, uint8_t silence)
{
	cassette_image::error err;
	uint64_t offset;
	uint64_t image_size;
	uint8_t block[258];   /* 255 bytes per block + 3 (type, length, checksum) */
	int block_length = 0;
	uint8_t last_blocktype;
	double time_index = 0.0;
	double time_displacement;
	static const uint8_t magic_bytes[2] = { 0x55, 0x3C };
	int synccount;

#if 0
	{
		static const uint8_t dummy_bytes[] =
		{
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x3C, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A
		};
		time_index = 10.0;
		return cassette_put_modulated_data(cassette, 0, time_index, dummy_bytes, sizeof(dummy_bytes), coco_cas_modulation, &time_displacement);
	}
#endif

	err = cassette->put_sample(0, time_index, COCO_WAVESAMPLES_HEADER, 0);
	if (err != cassette_image::error::SUCCESS)
		return err;
	time_index += COCO_WAVESAMPLES_HEADER;

	offset = 0;
	last_blocktype = 0;
	image_size = cassette->image_size();

	/* try to find a block that we can untangle */
	while(get_cas_block(cassette, offset, block, block_length, synccount))
	{
		/* Forcing a silence before a filename block, improves the ability to load some */
		/* copy protected Dragon games, e.g. Rommel's Revenge */
		/* was the last block a filename block? */
		if ((last_blocktype == 0) || (last_blocktype == 0xFF) || (block[0] == 0))
		{
			/* silence */
			err = cassette->put_sample(0, time_index, silence, 0);
			if (err != cassette_image::error::SUCCESS)
				return err;
			time_index += silence;

			/* sync data */
			err = cassette->put_modulated_filler(0, time_index, 0x55, 128, coco_cas_modulation, &time_displacement);
			if (err != cassette_image::error::SUCCESS)
				return err;
			time_index += time_displacement;
		}
		else if (synccount != 0)        /* If we have multiple sync bytes in cas file, make sure they */
		{               /* are passed through */
			/* sync data */
			err = cassette->put_modulated_filler(0, time_index, 0x55, synccount, coco_cas_modulation, &time_displacement);
			if (err != cassette_image::error::SUCCESS)
				return err;
			time_index += time_displacement;
		}

		/* now fill in the magic bytes */
		err = cassette->put_modulated_data(0, time_index, magic_bytes, sizeof(magic_bytes), coco_cas_modulation, &time_displacement);
		if (err != cassette_image::error::SUCCESS)
			return err;
		time_index += time_displacement;

		/* now fill in the block */
		err = cassette->put_modulated_data(0, time_index, block, block_length, coco_cas_modulation, &time_displacement);
		if (err != cassette_image::error::SUCCESS)
			return err;
		time_index += time_displacement;

		/* and the last magic byte */
		err = cassette->put_modulated_filler(0, time_index, 0x55, 1, coco_cas_modulation, &time_displacement);
		if (err != cassette_image::error::SUCCESS)
			return err;
		time_index += time_displacement;

		last_blocktype = block[0];
	}

	/* all futher data is undecipherable, so output it verbatim */
	err = cassette->read_modulated_data(0, time_index, offset, image_size - offset, coco_cas_modulation, &time_displacement);
	if (err != cassette_image::error::SUCCESS)
		return err;
	time_index += time_displacement;

	return cassette_image::error::SUCCESS;
}

static cassette_image::error coco_cas_load(cassette_image *cassette)
{
	return cas_load(cassette, COCO_WAVESAMPLES_HEADER);
}

static cassette_image::error alice32_cas_load(cassette_image *cassette)
{
	return cas_load(cassette, ALICE32_WAVESAMPLES_HEADER);
}

const cassette_image::Format coco_cas_format =
{
	"cas",
	coco_cas_identify,
	coco_cas_load,
	nullptr
};

const cassette_image::Format alice32_cas_format =
{
	"cas,c10,k7",
	coco_cas_identify,
	alice32_cas_load,
	nullptr
};


CASSETTE_FORMATLIST_START(coco_cassette_formats)
	CASSETTE_FORMAT(coco_cas_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(alice32_cassette_formats)
	CASSETTE_FORMAT(alice32_cas_format)
CASSETTE_FORMATLIST_END
