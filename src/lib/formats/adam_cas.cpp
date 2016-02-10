// license:BSD-3-Clause
// copyright-holders:Curt Coder

#include <assert.h>

#include "cassimg.h"
#include "adam_cas.h"

// This code will reproduce the timing of an adam tape played back on a standard tape deck with a 1 7/8ips speed.
#define CASS_AMP 0x1fffffff
#define SPEED_MULTIPLIER 20.0/1.875
#define COL_ADAM_PERIOD  0.00007*SPEED_MULTIPLIER
#define COL_ADAM_PERIOD1 0.000031*SPEED_MULTIPLIER
#define COL_ADAM_PERIOD2 0.000039*SPEED_MULTIPLIER
#define TYPE_HE 0
#define TYPE_GW 1

/***************************************************************
Coleco Adam Digital Data Pack format:
The packs are the same shape as a standard audio cassette, but with different alignment and capstan holes
The tape only has one side and has two widely separated mono data tracks (second track readable on an audio
cassette deck by flipping the tape over and reversing the recorded signal).

Header format:
2 byte header id: 0x48 0x45 (central directory HE type) or 0x47 0x57 (GW type)
2 byte block number
2 byte NOT block number
1 byte number of blocks (0x80)
1 byte sum of the above (checksum)


track format:
leading zeros: ~2753

block format:
sync byte 0x16
header
pad zero bytes ~21
sync byte 0x16
data(1k)
pad zero bytes ~21
sync byte 0x16
2 byte checksum-16
pad zero bytes ~2
sync bytes 0xaa ~922
pad zero bytes ~2

Data is phase encoded, with 70us bit cells with a transition at 31us for a 1 and no transition for a 0

****************************************************************/


static casserr_t coladam_ddp_identify ( cassette_image *cass, struct CassetteOptions *opts )
{
	opts -> bits_per_sample = 16;
	opts -> channels = 2;
	opts -> sample_frequency = 44100;
	return CASSETTE_ERROR_SUCCESS;
}


// Store byte of data
casserr_t coladam_put_byte(cassette_image *cass, int channel, double *time_index, int byte, int *prev_sign)
{
	casserr_t err = CASSETTE_ERROR_SUCCESS;
	for (int i = 0; i < 8; i++)
	{
		if(byte & 0x80)
		{
			err = cassette_put_sample( cass, channel, *time_index, COL_ADAM_PERIOD1, -CASS_AMP*(*prev_sign) );
			(*time_index) += COL_ADAM_PERIOD1;
			err = cassette_put_sample( cass, channel, *time_index, COL_ADAM_PERIOD2,  CASS_AMP*(*prev_sign) );
			(*time_index) += COL_ADAM_PERIOD2;
		}
		else
		{
			err = cassette_put_sample( cass, channel, *time_index, COL_ADAM_PERIOD, -CASS_AMP*(*prev_sign) );
			(*prev_sign) *=-1;
			(*time_index) += COL_ADAM_PERIOD;
		}
		byte <<= 1;
	}
	return err;
}

casserr_t coladam_put_block(cassette_image *cass, int channel, double *time_index, int *prev_sign, int block_index, UINT8 *buffer, int layout_type)
{
	int i, checksum_16=0;
	UINT8 header[] = { 0x16, 0x48, 0x45, 0x00, static_cast<UINT8>(block_index), 0xff, static_cast<UINT8>(0xff - block_index), 0x00, 0x80, 0xf4 };
	casserr_t err;
	if (layout_type == TYPE_GW)
	{
		header[1] = 0x47;
		header[2] = 0x57;
		header[9] = 0xe3;
	}

	for (i = 0; i < 10; i++) // header
	{
		err = coladam_put_byte(cass, channel, time_index, header[i], prev_sign);
	}
	for (i = 0; i < 21; i++) // leading zero bytes
	{
		err = coladam_put_byte(cass, channel, time_index, 0x00, prev_sign);
	}
	err = coladam_put_byte(cass, channel, time_index, 0x16, prev_sign); // data start
	for ( i = 0; i < 0x400; i++ )
	{
		err = coladam_put_byte(cass, channel, time_index, buffer[i], prev_sign);
		checksum_16 += buffer[i];
	}
	for (i = 0; i < 21; i++) // trailing padding zeros
	{
		err = coladam_put_byte(cass, channel, time_index, 0x00, prev_sign);
	}
	err = coladam_put_byte(cass, channel, time_index, 0x16, prev_sign);
	err = coladam_put_byte(cass, channel, time_index, (checksum_16 & 0xff00) >> 8, prev_sign); // write checksum
	err = coladam_put_byte(cass, channel, time_index, (checksum_16 & 0xff), prev_sign);
	for (i = 0; i < 922; i++) // sync bytes
	{
		err = coladam_put_byte(cass, channel, time_index, 0xaa, prev_sign);
	}
	err = coladam_put_byte(cass, channel, time_index, 0x00, prev_sign);
	err = coladam_put_byte(cass, channel, time_index, 0x00, prev_sign);
	return err;
}


static casserr_t coladam_ddp_load( cassette_image *cass )
{
	double time = 0.;
	int i, block, prev_sign=-1;
	UINT8 buffer[0x400];
	casserr_t err = CASSETTE_ERROR_SUCCESS;

	// It would appear that data packs that originally had the type GW data layout and headers work fine when converted to type
	// HE. Thus we set all tapes to type HE.

	int layout_type = TYPE_HE;

	// Track 0
	for ( i = 0; i < 2753; i++ ) // leading zero bytes
	{
		err = coladam_put_byte(cass, 0, &time, 0x00, &prev_sign);
	}

	for (block = 0; block < 128; block++)
	{
		cassette_image_read( cass, buffer, 0x20000+0x400*block, 0x400 );
		err = coladam_put_block(cass, 0, &time, &prev_sign, block, buffer, layout_type);
	}
	for (block = 128; block < 131; block++)
	{
		cassette_image_read( cass, buffer, 0x3f400+0x400*(block-128), 0x400 );
		err = coladam_put_block(cass, 0, &time, &prev_sign, block, buffer, layout_type);
	}

	// Track 1
	time = 0.;
	for ( i = 0; i < 2753; i++ ) // leading zero bytes
	{
		err = coladam_put_byte(cass, 1, &time, 0x00, &prev_sign);
	}
	if (layout_type == TYPE_HE)
	{
		for (block = 0; block < 64; block++)
		{
			cassette_image_read( cass, buffer, 0x10000+0x400*block, 0x400 );
			err = coladam_put_block(cass, 1, &time, &prev_sign, block, buffer, layout_type);
		}
		for (block = 64; block < 128; block++)
		{
			cassette_image_read( cass, buffer, 0x00000+0x400*(block-64), 0x400 );
			err = coladam_put_block(cass, 1, &time, &prev_sign, block, buffer, layout_type);
		}
		for (block = 128; block < 131; block++)
		{
			cassette_image_read( cass, buffer, 0x0f400+0x400*(block-128), 0x400 );
			err = coladam_put_block(cass, 1, &time, &prev_sign, block, buffer, layout_type);
		}
	}
	else
	{
		time = 0;
		for ( i = 0; i < 2753; i++ ) // leading zero bytes
		{
			err = coladam_put_byte(cass, 1, &time, 0x00, &prev_sign);
		}
		for (block = 0; block < 128; block++)
		{
			cassette_image_read( cass, buffer, 0x400*block, 0x400 );
			err = coladam_put_block(cass, 1, &time, &prev_sign, block, buffer, layout_type);
		}
		for (block = 128; block < 131; block++)
		{
			cassette_image_read( cass, buffer, 0x1f400+0x400*(block-128), 0x400 );
			err = coladam_put_block(cass, 1, &time, &prev_sign, block, buffer, layout_type);
		}
	}

	return err;
}



static const struct CassetteFormat coladam_ddp =
{ "ddp", coladam_ddp_identify, coladam_ddp_load, nullptr /* no save */ };

CASSETTE_FORMATLIST_START(coleco_adam_cassette_formats)
	CASSETTE_FORMAT(coladam_ddp)
CASSETTE_FORMATLIST_END
