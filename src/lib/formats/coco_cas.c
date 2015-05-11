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

#include "emu.h" // PAIR
#include "coco_cas.h"

#define COCO_WAVESAMPLES_HEADER     (1.0)
#define COCO_WAVESAMPLES_TRAILER    (1.0)
#define COCO_LONGSILENCE            (5.0)

//some games load with only 5s, but most games need 15s
#define ALICE32_WAVESAMPLES_HEADER  (15.0)

static int synccount;

const struct CassetteModulation coco_cas_modulation =
{
	CASSETTE_MODULATION_SINEWAVE,
	600.0,  1200.0, 1500.0,
	1500.0, 2400.0, 3000.0
};



static casserr_t coco_cas_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_modulation_identify(cassette, &coco_cas_modulation, opts);
}



static int get_cas_block(cassette_image *cassette, UINT64 *offset, UINT8 *block, int *block_len)
{
	UINT8 block_length = 0;
	UINT8 block_checksum = 0;
	UINT64 current_offset;
	UINT64 image_size;
	PAIR p;
	int i;
	int state = 0;
	int phase = 0;

	synccount = 0;
	p.w.l = 0;
	image_size = cassette_image_size(cassette);
	current_offset = *offset;

	while(current_offset < image_size)
	{
		cassette_image_read(cassette, &p.b.h, current_offset, 1);
		current_offset++;

		for (i = 0; i < 8; i++)
		{
			p.w.l >>= 1;

			if (state == 0)
			{
				/* searching for a block */
				if (p.b.l == 0x3C)
				{
					/* found one! */
					phase = i;
					state++;
				}
				else if (p.b.l == 0x55)
				{
					synccount++;
				}
			}
			else if (i == phase)
			{
				*(block++) = p.b.l;
				switch(state) {
				case 1:
					/* found file type */
					block_checksum = p.b.l;
					state++;
					break;
				case 2:
					/* found file size */
					block_length = p.b.l;
					*block_len = ((int) block_length) + 3;
					block_checksum += p.b.l;
					state++;
					break;

				case 3:
					/* data byte */
					if (block_length)
					{
						block_length--;
						block_checksum += p.b.l;
					}
					else
					{
						/* end of block */
						if (p.b.l != block_checksum)
						{
							/* checksum failure */
							return FALSE;
						}
						else
						{
							/* checksum success */
							*offset = current_offset;
							return TRUE;
						}
					}
				}
			}
		}
	}

	/* no more blocks */
	return FALSE;
}



static casserr_t cas_load(cassette_image *cassette, UINT8 silence)
{
	casserr_t err;
	UINT64 offset;
	UINT64 image_size;
	UINT8 block[258];   /* 255 bytes per block + 3 (type, length, checksum) */
	int block_length = 0;
	UINT8 last_blocktype;
	double time_index = 0.0;
	double time_displacement;
	static const UINT8 magic_bytes[2] = { 0x55, 0x3C };

#if 0
	{
		static const UINT8 dummy_bytes[] =
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
		return cassette_put_modulated_data(cassette, 0, time_index, dummy_bytes, sizeof(dummy_bytes), &coco_cas_modulation, &time_displacement);
	}
#endif

	err = cassette_put_sample(cassette, 0, time_index, COCO_WAVESAMPLES_HEADER, 0);
	if (err)
		return err;
	time_index += COCO_WAVESAMPLES_HEADER;

	offset = 0;
	last_blocktype = 0;
	image_size = cassette_image_size(cassette);

	/* try to find a block that we can untangle */
	while(get_cas_block(cassette, &offset, block, &block_length))
	{
		/* Forcing a silence before a filename block, improves the ability to load some */
		/* copy protected Dragon games, e.g. Rommel's Revenge */
		/* was the last block a filename block? */
		if ((last_blocktype == 0) || (last_blocktype == 0xFF) || (block[0] == 0))
		{
			/* silence */
			err = cassette_put_sample(cassette, 0, time_index, silence, 0);
			if (err)
				return err;
			time_index += silence;

			/* sync data */
			err = cassette_put_modulated_filler(cassette, 0, time_index, 0x55, 128, &coco_cas_modulation, &time_displacement);
			if (err)
				return err;
			time_index += time_displacement;
		}
		else if (synccount != 0)        /* If we have multiple sync bytes in cas file, make sure they */
		{               /* are passed through */
			/* sync data */
			err = cassette_put_modulated_filler(cassette, 0, time_index, 0x55, synccount, &coco_cas_modulation, &time_displacement);
			if (err)
				return err;
			time_index += time_displacement;
		}

		/* now fill in the magic bytes */
		err = cassette_put_modulated_data(cassette, 0, time_index, magic_bytes, sizeof(magic_bytes), &coco_cas_modulation, &time_displacement);
		if (err)
			return err;
		time_index += time_displacement;

		/* now fill in the block */
		err = cassette_put_modulated_data(cassette, 0, time_index, block, block_length, &coco_cas_modulation, &time_displacement);
		if (err)
			return err;
		time_index += time_displacement;

		/* and the last magic byte */
		err = cassette_put_modulated_filler(cassette, 0, time_index, 0x55, 1, &coco_cas_modulation, &time_displacement);
		if (err)
			return err;
		time_index += time_displacement;

		last_blocktype = block[0];
	}

	/* all futher data is undecipherable, so output it verbatim */
	err = cassette_read_modulated_data(cassette, 0, time_index, offset, image_size - offset, &coco_cas_modulation, &time_displacement);
	if (err)
		return err;
	time_index += time_displacement;

	return CASSETTE_ERROR_SUCCESS;
}

static casserr_t coco_cas_load(cassette_image *cassette)
{
	return cas_load(cassette, COCO_WAVESAMPLES_HEADER);
}

static casserr_t alice32_cas_load(cassette_image *cassette)
{
	return cas_load(cassette, ALICE32_WAVESAMPLES_HEADER);
}

const struct CassetteFormat coco_cas_format =
{
	"cas",
	coco_cas_identify,
	coco_cas_load,
	NULL
};

const struct CassetteFormat alice32_cas_format =
{
	"cas,c10,k7",
	coco_cas_identify,
	alice32_cas_load,
	NULL
};


CASSETTE_FORMATLIST_START(coco_cassette_formats)
	CASSETTE_FORMAT(coco_cas_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(alice32_cassette_formats)
	CASSETTE_FORMAT(alice32_cas_format)
CASSETTE_FORMATLIST_END
