// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,???
/*

TZX (currently spectrum only) and spectrum TAP cassette format support by Wilbert Pol

TODO:
    Add support for the remaining block types:
        case 0x15:  Direct Recording
        case 0x18:  CSW Recording
        case 0x19:  Generalized Data Block
        case 0x21:  Group Start
        case 0x22:  Group End
        case 0x23:  Jump To Block
        case 0x24:  Loop Start
        case 0x25:  Loop End
        case 0x26:  Call Sequence
        case 0x27:  Return From Sequence
        case 0x28:  Select Block
        case 0x2A:  Stop Tape if in 48K Mode
        case 0x2B:  Set signal level
        case 0x5A:  Merge Block
    Add support for the deprecated block types? Only if there is some image which need them:
        case 0x16:  C64 ROM type data block
        case 0x17:  C64 turbo tape data block
        case 0x34:  Emulation info
        case 0x40:  Snapshot block

Notes:

TZX format specification lists
8064 pulses for a header block and 3220 for a data block

but the documentaiton on worldofspectrum lists
8063 pulses for a header block and 3223 for a data block

see http://www.worldofspectrum.org/faq/reference/48kreference.htm#TapeDataStructure

We are currently using the numbers from the TZX specification...

*/

#include <assert.h>

#include "tzx_cas.h"
#include "formats/imageutl.h"
#include "emu.h"

#define TZX_WAV_FREQUENCY   44100
#define WAVE_LOW        -0x5a9e
#define WAVE_HIGH       0x5a9e
#define WAVE_NULL       0

#define SUPPORTED_VERSION_MAJOR 0x01

#define INITIAL_MAX_BLOCK_COUNT 256
#define BLOCK_COUNT_INCREMENTS  256

static const UINT8 TZX_HEADER[8] = { 'Z','X','T','a','p','e','!',0x1a };

/*
  Global variables

  Initialized by tzx_cas_get_wave_size, used (and cleaned up) by tzx_cas_fill_wave
 */

static INT16    wave_data = 0;
static int  block_count = 0;
static UINT8**  blocks = nullptr;
static float t_scale = 1;  /* for scaling T-states to the 4MHz CPC */

static void toggle_wave_data(void)
{
	if (wave_data == WAVE_LOW)
	{
		wave_data = WAVE_HIGH;
	}
	else
	{
		wave_data = WAVE_LOW;
	}
}

static void tzx_cas_get_blocks( const UINT8 *casdata, int caslen )
{
	int pos = sizeof(TZX_HEADER) + 2;
	int max_block_count = INITIAL_MAX_BLOCK_COUNT;
	int loopcount = 0, loopoffset = 0;
	blocks = (UINT8**)malloc(max_block_count * sizeof(UINT8*));
	memset(blocks,0,max_block_count);
	block_count = 0;

	while (pos < caslen)
	{
		UINT32 datasize;
		UINT8 blocktype = casdata[pos];

		if (block_count == max_block_count)
		{
			void *old_blocks = blocks;
			int old_max_block_count = max_block_count;
			max_block_count = max_block_count + BLOCK_COUNT_INCREMENTS;
			blocks = (UINT8**)malloc(max_block_count * sizeof(UINT8*)); // SHOULD NOT BE USING auto_alloc_array()
			memset(blocks, 0, max_block_count);
			memcpy(blocks, old_blocks, old_max_block_count * sizeof(UINT8*));
			free(old_blocks);
		}

		blocks[block_count] = (UINT8*)&casdata[pos];

		pos += 1;

		switch (blocktype)
		{
		case 0x10:
			pos += 2;
			datasize = casdata[pos] + (casdata[pos + 1] << 8);
			pos += 2 + datasize;
			break;
		case 0x11:
			pos += 0x0f;
			datasize = casdata[pos] + (casdata[pos + 1] << 8) + (casdata[pos + 2] << 16);
			pos += 3 + datasize;
			break;
		case 0x12:
			pos += 4;
			break;
		case 0x13:
			datasize = casdata[pos];
			pos += 1 + 2 * datasize;
			break;
		case 0x14:
			pos += 7;
			datasize = casdata[pos] + (casdata[pos + 1] << 8) + (casdata[pos + 2] << 16);
			pos += 3 + datasize;
			break;
		case 0x15:
			pos += 5;
			datasize = casdata[pos] + (casdata[pos + 1] << 8) + (casdata[pos + 2] << 16);
			pos += 3 + datasize;
			break;
		case 0x20: case 0x23:
			pos += 2;
			break;

		case 0x24:
			loopcount = casdata[pos] + (casdata[pos + 1] << 8);
			pos +=2;
			loopoffset = pos;
			break;

		case 0x21: case 0x30:
			datasize = casdata[pos];
			pos += 1 + datasize;
			break;
		case 0x22: case 0x27:
			break;

		case 0x25:
			if (loopcount>0)
			{
				pos = loopoffset;
				loopcount--;
			}
			break;

		case 0x26:
			datasize = casdata[pos] + (casdata[pos + 1] << 8);
			pos += 2 + 2 * datasize;
			break;
		case 0x28: case 0x32:
			datasize = casdata[pos] + (casdata[pos + 1] << 8);
			pos += 2 + datasize;
			break;
		case 0x31:
			pos += 1;
			datasize = casdata[pos];
			pos += 1 + datasize;
			break;
		case 0x33:
			datasize = casdata[pos];
			pos += 1 + 3 * datasize;
			break;
		case 0x34:
			pos += 8;
			break;
		case 0x35:
			pos += 0x10;
			datasize = casdata[pos] + (casdata[pos + 1] << 8) + (casdata[pos + 2] << 16) + (casdata[pos + 3] << 24);
			pos += 4 + datasize;
			break;
		case 0x40:
			pos += 1;
			datasize = casdata[pos] + (casdata[pos + 1] << 8) + (casdata[pos + 2] << 16);
			pos += 3 + datasize;
			break;
		case 0x5A:
			pos += 9;
			break;
		default:
			datasize = casdata[pos] + (casdata[pos + 1] << 8) + (casdata[pos + 2] << 16) + (casdata[pos + 3] << 24);
			pos += 4 + datasize;
			break;
		}

		block_count++;
	}
}

INLINE int millisec_to_samplecount( int millisec )
{
	return (int) (millisec * ((double)TZX_WAV_FREQUENCY / 1000.0));
}

INLINE int tcycles_to_samplecount( int tcycles )
{
	return (int) ((0.5 + (((double)TZX_WAV_FREQUENCY / 3500000) * (double)tcycles)) * (double) t_scale);
}

static void tzx_output_wave( INT16 **buffer, int length )
{
	if (buffer == nullptr)
	{
		return;
	}

	for ( ; length > 0; length--)
	{
		**buffer = wave_data;
		*buffer = *buffer + 1;
	}
}

static int tzx_cas_handle_block( INT16 **buffer, const UINT8 *bytes, int pause, int data_size, int pilot, int pilot_length, int sync1, int sync2, int bit0, int bit1, int bits_in_last_byte )
{
	int pilot_samples = tcycles_to_samplecount(pilot);
	int sync1_samples = tcycles_to_samplecount(sync1);
	int sync2_samples = tcycles_to_samplecount(sync2);
	int bit0_samples = tcycles_to_samplecount(bit0);
	int bit1_samples = tcycles_to_samplecount(bit1);
	int data_index;
	int size = 0;

	/* Uncomment this to include into error.log a fully detailed analysis of each block */
//  LOG_FORMATS("tzx_cas_block_size: pilot_length = %d, pilot_samples = %d, sync1_samples = %d, sync2_samples = %d, bit0_samples = %d, bit1_samples = %d\n", pilot_length, pilot_samples, sync1_samples, sync2_samples, bit0_samples, bit1_samples);

	/* PILOT */
	for ( ; pilot_length > 0; pilot_length--)
	{
		tzx_output_wave(buffer, pilot_samples);
		size += pilot_samples;
		toggle_wave_data();
	}
	/* SYNC1 */
	if (sync1_samples > 0)
	{
		tzx_output_wave(buffer, sync1_samples);
		size += sync1_samples;
		toggle_wave_data();
	}
	/* SYNC2 */
	if (sync2_samples > 0)
	{
		tzx_output_wave(buffer, sync2_samples);
		size += sync2_samples;
		toggle_wave_data();
	}
	/* data */
	for (data_index = 0; data_index < data_size; data_index++)
	{
		UINT8 byte = bytes[data_index];
		int bits_to_go = (data_index == (data_size - 1)) ? bits_in_last_byte : 8;

		for ( ; bits_to_go > 0; byte <<= 1, bits_to_go--)
		{
			int bit_samples = (byte & 0x80) ? bit1_samples : bit0_samples;
			tzx_output_wave(buffer, bit_samples);
			size += bit_samples;
			toggle_wave_data();
			tzx_output_wave(buffer, bit_samples);
			size += bit_samples;
			toggle_wave_data();
		}
	}
	/* pause */
	if (pause > 0)
	{
		int start_pause_samples = millisec_to_samplecount(1);
		int rest_pause_samples = millisec_to_samplecount(pause - 1);

		tzx_output_wave(buffer, start_pause_samples);
		size += start_pause_samples;
		wave_data = WAVE_LOW;
		tzx_output_wave(buffer, rest_pause_samples);
		size += rest_pause_samples;
	}
	return size;
}

static int tzx_handle_direct(INT16 **buffer, const UINT8 *bytes, int pause, int data_size, int tstates, int bits_in_last_byte)
{
	int size = 0;
	int samples = tcycles_to_samplecount(tstates);

	/* data */
	for (int data_index = 0; data_index < data_size; data_index++)
	{
		UINT8 byte = bytes[data_index];
		int bits_to_go = (data_index == (data_size - 1)) ? bits_in_last_byte : 8;

		for ( ; bits_to_go > 0; byte <<= 1, bits_to_go--)
		{
			if (byte & 0x80) wave_data = WAVE_HIGH;
			else wave_data = WAVE_LOW;

			tzx_output_wave(buffer, samples);
			size += samples;

		}
	}

	/* pause */
	if (pause > 0)
	{
		int start_pause_samples = millisec_to_samplecount(1);
		int rest_pause_samples = millisec_to_samplecount(pause - 1);

		tzx_output_wave(buffer, start_pause_samples);
		size += start_pause_samples;
		wave_data = WAVE_LOW;
		tzx_output_wave(buffer, rest_pause_samples);
		size += rest_pause_samples;
	}
	return size;
}


INLINE int tzx_handle_symbol(INT16 **buffer, const UINT8 *symtable, UINT8 symbol, int maxp)
{
	int size = 0;
	const UINT8 *cursymb = symtable + (2 * maxp + 1)*symbol;

	UINT8 starttype = cursymb[0];

//  printf("start polarity %01x (max number of symbols is %d)\n", starttype, maxp);

	switch (starttype)
	{
	case 0x00:
		toggle_wave_data();
		break;

	case 0x01:
		// don't change
		break;

	case 0x02:
		// force low
		wave_data = WAVE_LOW;
		break;

	case 0x03:
		// force high
		wave_data = WAVE_HIGH;
		break;

	default:
		printf("SYMDEF invalid - bad starting polarity");
	}

	for (int i = 0; i < maxp; i++)
	{
		UINT16 pulse_length = cursymb[1 + (i*2)] | (cursymb[2 + (i*2)] << 8);
	//  printf("pulse_length %04x\n", pulse_length);

		// shorter lists can be terminated with a pulse_length of 0
		if (pulse_length != 0)
		{
			int samples = tcycles_to_samplecount(pulse_length);
			tzx_output_wave(buffer, samples);
			size += samples;
			toggle_wave_data();

		}
		else
		{
			toggle_wave_data();
			i = maxp;
			continue;
		}
	}

	//toggle_wave_data();

	return size;
}

INLINE int stream_get_bit(const UINT8 *bytes, UINT8 &stream_bit, UINT32 &stream_byte)
{
	// get bit here
	UINT8 retbit = 0;

	UINT8 byte = bytes[stream_byte];
	byte = byte << stream_bit;

	if (byte & 0x80) retbit = 1;


	stream_bit++;

	if (stream_bit == 8)
	{
		stream_bit = 0;
		stream_byte++;
	}

	return retbit;
}

static int tzx_handle_generalized(INT16 **buffer, const UINT8 *bytes, int pause, int data_size, UINT32 totp, int npp, int asp, UINT32 totd, int npd, int asd )
{
	int size = 0;

	if (totp > 0)
	{
	//  printf("pilot block table %04x\n", totp);

		const UINT8 *symtable = bytes;
		const UINT8 *table2 = symtable + (2 * npp + 1)*asp;

		// the Pilot and sync data stream has an RLE encoding
		for (int i = 0; i < totp; i+=3)
		{
			UINT8 symbol = table2[i + 0];
			UINT16 repetitions = table2[i + 1] + (table2[i + 2] << 8);
			//printf("symbol %02x repititions %04x\n", symbol, repetitions); // does 1 mean repeat once, or that it only occurs once?

			for (int j = 0; j < repetitions; j++)
			{
				size += tzx_handle_symbol(buffer, symtable, symbol, npp);
			//  toggle_wave_data();
			}


		}

		// advance to after this data
		bytes += ((2 * npp + 1)*asp) + totp * 3;
	}
	else
	{
		printf("no pilot block\n");
	}

	if (totd > 0)
	{
		printf("data block table %04x (has %0d symbols, max symbol length is %d)\n", totd, asd, npd);

		const UINT8 *symtable = bytes;
		const UINT8 *table2 = bytes + (2 * npd + 1)*asd;

		int NB = ceil(compute_log2(asd)); // number of bits needed to represent each symbol
		printf("NB is %d\n", NB);

		UINT8 stream_bit = 0;
		UINT32 stream_byte = 0;

		for (int i = 0; i < totd; i++)
		{
			UINT8 symbol = 0;

			for (int j = 0; j < NB; j++)
			{
				symbol |= stream_get_bit(table2, stream_bit, stream_byte) << j;
			}

			size += tzx_handle_symbol(buffer, symtable, symbol, npd);

			//toggle_wave_data();
		}
	}
	else
	{
		printf("no data block\n");
	}



	/* pause */
	if (pause > 0)
	{
		int start_pause_samples = millisec_to_samplecount(1);
		int rest_pause_samples = millisec_to_samplecount(pause - 1);

		tzx_output_wave(buffer, start_pause_samples);
		size += start_pause_samples;
		wave_data = WAVE_LOW;
		tzx_output_wave(buffer, rest_pause_samples);
		size += rest_pause_samples;
	}
	return size;
}



static void ascii_block_common_log( const char *block_type_string, UINT8 block_type )
{
	LOG_FORMATS("%s (type %02x) encountered.\n", block_type_string, block_type);
	LOG_FORMATS("This block contains info on the .tzx file you are loading.\n");
	LOG_FORMATS("Please include the following info in your bug reports, if the image has issue in M.E.S.S.\n");
}

static const char *const archive_ident[] =
{
	"Full title",
	"Software house/publisher",
	"Author(s)",
	"Year of publication",
	"Language",
	"Game/utility type",
	"Price",
	"Protection scheme/loader",
	"Origin",
};

static const char *const hw_info[] =
{
	"Tape runs on this machine / this hardware",
	"Tape needs this machine / this hardware",
	"Tape runs on this machine / this hardware, but does not require its special features",
	"Tape does not run on this machine / this hardware",
};

/*  Will go through blocks and calculate number of samples needed.
If buffer is not NULL the sample data will also be written. */
static int tzx_cas_do_work( INT16 **buffer )
{
	int current_block = 0;
	int size = 0;

	wave_data = WAVE_LOW;

	int loopcount = 0, loopoffset = 0;

	while (current_block < block_count)
	{
		int pause_time;
		UINT32 data_size;
		int text_size, total_size, i;
		int pilot, pilot_length, sync1, sync2;
		int bit0, bit1, bits_in_last_byte;
		UINT8 *cur_block = blocks[current_block];
		UINT8 block_type = cur_block[0];
		UINT16 tstates = 0;

	/* Uncomment this to include into error.log a list of the types each block */
	LOG_FORMATS("tzx_cas_fill_wave: block %d, block_type %02x\n", current_block, block_type);

		switch (block_type)
		{
		case 0x10:  /* Standard Speed Data Block (.TAP block) */
			pause_time = cur_block[1] + (cur_block[2] << 8);
			data_size = cur_block[3] + (cur_block[4] << 8);
			pilot_length = (cur_block[5] == 0x00) ?  8064 : 3220;
			size += tzx_cas_handle_block(buffer, &cur_block[5], pause_time, data_size, 2168, pilot_length, 667, 735, 855, 1710, 8);
			current_block++;
			break;
		case 0x11:  /* Turbo Loading Data Block */
			pilot = cur_block[1] + (cur_block[2] << 8);
			sync1 = cur_block[3] + (cur_block[4] << 8);
			sync2 = cur_block[5] + (cur_block[6] << 8);
			bit0 = cur_block[7] + (cur_block[8] << 8);
			bit1 = cur_block[9] + (cur_block[10] << 8);
			pilot_length = cur_block[11] + (cur_block[12] << 8);
			bits_in_last_byte = cur_block[13];
			pause_time = cur_block[14] + (cur_block[15] << 8);
			data_size = cur_block[16] + (cur_block[17] << 8) + (cur_block[18] << 16);
			size += tzx_cas_handle_block(buffer, &cur_block[19], pause_time, data_size, pilot, pilot_length, sync1, sync2, bit0, bit1, bits_in_last_byte);
			current_block++;
			break;
		case 0x12:  /* Pure Tone */
			pilot = cur_block[1] + (cur_block[2] << 8);
			pilot_length = cur_block[3] + (cur_block[4] << 8);
			size += tzx_cas_handle_block(buffer, cur_block, 0, 0, pilot, pilot_length, 0, 0, 0, 0, 0);
			current_block++;
			break;
		case 0x13:  /* Sequence of Pulses of Different Lengths */
			for (data_size = 0; data_size < cur_block[1]; data_size++)
			{
				pilot = cur_block[2 + 2 * data_size] + (cur_block[3 + 2 * data_size] << 8);
				size += tzx_cas_handle_block(buffer, cur_block, 0, 0, pilot, 1, 0, 0, 0, 0, 0);
			}
			current_block++;
			break;
		case 0x14:  /* Pure Data Block */
			bit0 = cur_block[1] + (cur_block[2] << 8);
			bit1 = cur_block[3] + (cur_block[4] << 8);
			bits_in_last_byte = cur_block[5];
			pause_time = cur_block[6] + (cur_block[7] << 8);
			data_size = cur_block[8] + (cur_block[9] << 8) + (cur_block[10] << 16);
			size += tzx_cas_handle_block(buffer, &cur_block[11], pause_time, data_size, 0, 0, 0, 0, bit0, bit1, bits_in_last_byte);
			current_block++;
			break;
		case 0x20:  /* Pause (Silence) or 'Stop the Tape' Command */
			pause_time = cur_block[1] + (cur_block[2] << 8);
			if (pause_time == 0)
			{
				/* pause = 0 is used to let an emulator automagically stop the tape
				   in MESS we do not do that, so we insert a 5 second pause. */
				pause_time = 5000;
			}
			size += tzx_cas_handle_block(buffer, cur_block, pause_time, 0, 0, 0, 0, 0, 0, 0, 0);
			current_block++;
			break;
		case 0x16:  /* C64 ROM Type Data Block */       // Deprecated in TZX 1.20
		case 0x17:  /* C64 Turbo Tape Data Block */     // Deprecated in TZX 1.20
		case 0x34:  /* Emulation Info */                // Deprecated in TZX 1.20
		case 0x40:  /* Snapshot Block */                // Deprecated in TZX 1.20
			LOG_FORMATS("Deprecated block type (%02x) encountered.\n", block_type);
			LOG_FORMATS("Please look for an updated .tzx file.\n");
			current_block++;
			break;
		case 0x30:  /* Text Description */
			ascii_block_common_log("Text Description Block", block_type);
			for (data_size = 0; data_size < cur_block[1]; data_size++)
				LOG_FORMATS("%c", cur_block[2 + data_size]);
			LOG_FORMATS("\n");
			current_block++;
			break;
		case 0x31:  /* Message Block */
			ascii_block_common_log("Message Block", block_type);
			LOG_FORMATS("Expected duration of the message display: %02x\n", cur_block[1]);
			LOG_FORMATS("Message: \n");
			for (data_size = 0; data_size < cur_block[2]; data_size++)
			{
				LOG_FORMATS("%c", cur_block[3 + data_size]);
				if (cur_block[3 + data_size] == 0x0d)
					LOG_FORMATS("\n");
			}
			LOG_FORMATS("\n");
			current_block++;
			break;
		case 0x32:  /* Archive Info */
			ascii_block_common_log("Archive Info Block", block_type);
			total_size = cur_block[1] + (cur_block[2] << 8);
			text_size = 0;
			for (data_size = 0; data_size < cur_block[3]; data_size++)  // data_size = number of text blocks, in this case
			{
				if (cur_block[4 + text_size] < 0x09) {
					LOG_FORMATS("%s: \n", archive_ident[cur_block[4 + text_size]]);
				}
				else {
					LOG_FORMATS("Comment(s): \n");
				}

				for (i = 0; i < cur_block[4 + text_size + 1]; i++)
				{
					LOG_FORMATS("%c", cur_block[4 + text_size + 2 + i]);
				}
				text_size += 2 + i;
			}
			LOG_FORMATS("\n");
			if (text_size != total_size)
				LOG_FORMATS("Malformed Archive Info Block (Text length different from the declared one).\n Please verify your tape image.\n");
			current_block++;
			break;
		case 0x33:  /* Hardware Type */
			ascii_block_common_log("Hardware Type Block", block_type);
			for (data_size = 0; data_size < cur_block[1]; data_size++)  // data_size = number of hardware blocks, in this case
			{
				LOG_FORMATS("Hardware Type %02x - Hardware ID %02x - ", cur_block[2 + data_size * 3], cur_block[2 + data_size * 3 + 1]);
				LOG_FORMATS("%s \n ", hw_info[cur_block[2 + data_size * 3 + 2]]);
			}
			current_block++;
			break;
		case 0x35:  /* Custom Info Block */
			ascii_block_common_log("Custom Info Block", block_type);
			for (data_size = 0; data_size < 10; data_size++)
			{
				LOG_FORMATS("%c", cur_block[1 + data_size]);
			}
			LOG_FORMATS(":\n");
			text_size = cur_block[11] + (cur_block[12] << 8) + (cur_block[13] << 16) + (cur_block[14] << 24);
			for (data_size = 0; data_size < text_size; data_size++)
				LOG_FORMATS("%c", cur_block[15 + data_size]);
			LOG_FORMATS("\n");
			current_block++;
			break;
		case 0x5A:  /* "Glue" Block */
			LOG_FORMATS("Glue Block (type %02x) encountered.\n", block_type);
			LOG_FORMATS("Please use a .tzx handling utility to split the merged tape files.\n");
			current_block++;
			break;
		case 0x24:  /* Loop Start */
			loopcount = cur_block[1] + (cur_block[2] << 8);
			current_block++;
			loopoffset = current_block;

			LOG_FORMATS("loop start %d %d\n",  loopcount, current_block);
			break;
		case 0x25:  /* Loop End */
			if (loopcount>0)
			{
				current_block = loopoffset;
				loopcount--;
				LOG_FORMATS("do loop\n");
			}
			else
			{
				current_block++;
			}
			break;

		case 0x21:  /* Group Start */
		case 0x22:  /* Group End */
		case 0x23:  /* Jump To Block */
		case 0x26:  /* Call Sequence */
		case 0x27:  /* Return From Sequence */
		case 0x28:  /* Select Block */
		case 0x2A:  /* Stop Tape if in 48K Mode */
		case 0x2B:  /* Set signal level */
		default:
			LOG_FORMATS("Unsupported block type (%02x) encountered.\n", block_type);
			current_block++;
			break;

		case 0x15:  /* Direct Recording */ // used on 'bombscar' in the cpc_cass list
			// having this missing is fatal
			tstates = cur_block[1] + (cur_block[2] << 8);
			pause_time= cur_block[3] + (cur_block[4] << 8);
			bits_in_last_byte = cur_block[5];
			data_size = cur_block[6] + (cur_block[7] << 8) + (cur_block[8] << 16);
			size += tzx_handle_direct(buffer, &cur_block[9], pause_time, data_size, tstates, bits_in_last_byte);
			current_block++;
			break;

		case 0x18:  /* CSW Recording */
			// having this missing is fatal
			printf("Unsupported block type (0x15 - CSW Recording) encountered.\n");
			current_block++;
			break;

		case 0x19:  /* Generalized Data Block */
			{
				// having this missing is fatal
				// used crudely by batmanc in spectrum_cass list (which is just a redundant encoding of batmane ?)
				printf("Unsupported block type (0x19 - Generalized Data Block) encountered.\n");

				data_size = cur_block[1] + (cur_block[2] << 8) + (cur_block[3] << 16) + (cur_block[4] << 24);
				pause_time= cur_block[5] + (cur_block[6] << 8);

				UINT32 totp = cur_block[7] + (cur_block[8] << 8) + (cur_block[9] << 16) + (cur_block[10] << 24);
				int npp = cur_block[11];
				int asp = cur_block[12];
				if (asp == 0) asp = 256;

				UINT32 totd = cur_block[13] + (cur_block[14] << 8) + (cur_block[15] << 16) + (cur_block[16] << 24);
				int npd = cur_block[17];
				int asd = cur_block[18];
				if (asd == 0) asd = 256;

				size += tzx_handle_generalized(buffer, &cur_block[19], pause_time, data_size, totp, npp, asp, totd, npd, asd);

				current_block++;
			}
			break;

		}
	}
	return size;
}

static int tzx_cas_to_wav_size( const UINT8 *casdata, int caslen )
{
	int size = 0;

	/* Header size plus major and minor version number */
	if (caslen < 10)
	{
		LOG_FORMATS("tzx_cas_to_wav_size: cassette image too small\n");
		goto cleanup;
	}

	/* Check for correct header */
	if (memcmp(casdata, TZX_HEADER, sizeof(TZX_HEADER)))
	{
		LOG_FORMATS("tzx_cas_to_wav_size: cassette image has incompatible header\n");
		goto cleanup;
	}

	/* Check major version number in header */
	if (casdata[0x08] > SUPPORTED_VERSION_MAJOR)
	{
		LOG_FORMATS("tzx_cas_to_wav_size: unsupported version\n");
		goto cleanup;
	}

	tzx_cas_get_blocks(casdata, caslen);

	LOG_FORMATS("tzx_cas_to_wav_size: %d blocks found\n", block_count);

	if (block_count == 0)
	{
		LOG_FORMATS("tzx_cas_to_wav_size: no blocks found!\n");
		goto cleanup;
	}

	size = tzx_cas_do_work(nullptr);

	return size;

cleanup:
	return -1;
}

static int tzx_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes )
{
	INT16 *p = buffer;
	int size = 0;
	t_scale = 1.0;
	size = tzx_cas_do_work(&p);
	return size;
}

static int cdt_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes )
{
	INT16 *p = buffer;
	int size = 0;
	t_scale = (40 / 35);  /* scale to 4MHz */
	size = tzx_cas_do_work(&p);
	return size;
}

static int tap_cas_to_wav_size( const UINT8 *casdata, int caslen )
{
	int size = 0;
	const UINT8 *p = casdata;

	while (p < casdata + caslen)
	{
		int data_size = p[0] + (p[1] << 8);
		int pilot_length = (p[2] == 0x00) ? 8064 : 3220;    /* TZX specification */
//      int pilot_length = (p[2] == 0x00) ? 8063 : 3223;    /* worldofspectrum */
		LOG_FORMATS("tap_cas_to_wav_size: Handling TAP block containing 0x%X bytes", data_size);
		p += 2;
		size += tzx_cas_handle_block(nullptr, p, 1000, data_size, 2168, pilot_length, 667, 735, 855, 1710, 8);
		LOG_FORMATS(", total size is now: %d\n", size);
		p += data_size;
	}
	return size;
}

static int tap_cas_fill_wave( INT16 *buffer, int length, UINT8 *bytes )
{
	INT16 *p = buffer;
	int size = 0;

	while (size < length)
	{
		int data_size = bytes[0] + (bytes[1] << 8);
		int pilot_length = (bytes[2] == 0x00) ? 8064 : 3220;    /* TZX specification */
//      int pilot_length = (bytes[2] == 0x00) ? 8063 : 3223;    /* worldofspectrum */
		LOG_FORMATS("tap_cas_fill_wave: Handling TAP block containing 0x%X bytes\n", data_size);
		bytes += 2;
		size += tzx_cas_handle_block(&p, bytes, 1000, data_size, 2168, pilot_length, 667, 735, 855, 1710, 8);
		bytes += data_size;
	}
	return size;
}

static const struct CassetteLegacyWaveFiller tzx_legacy_fill_wave =
{
	tzx_cas_fill_wave,          /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	tzx_cas_to_wav_size,            /* chunk_sample_calc */
	TZX_WAV_FREQUENCY,          /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static const struct CassetteLegacyWaveFiller tap_legacy_fill_wave =
{
	tap_cas_fill_wave,          /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	tap_cas_to_wav_size,            /* chunk_sample_calc */
	TZX_WAV_FREQUENCY,          /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static const struct CassetteLegacyWaveFiller cdt_legacy_fill_wave =
{
	cdt_cas_fill_wave,          /* fill_wave */
	-1,                 /* chunk_size */
	0,                  /* chunk_samples */
	tzx_cas_to_wav_size,            /* chunk_sample_calc */
	TZX_WAV_FREQUENCY,          /* sample_frequency */
	0,                  /* header_samples */
	0                   /* trailer_samples */
};

static casserr_t tzx_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts )
{
	return cassette_legacy_identify(cassette, opts, &tzx_legacy_fill_wave);
}

static casserr_t tap_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts )
{
	return cassette_legacy_identify(cassette, opts, &tap_legacy_fill_wave);
}

static casserr_t cdt_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts )
{
	return cassette_legacy_identify(cassette, opts, &cdt_legacy_fill_wave);
}

static casserr_t tzx_cassette_load( cassette_image *cassette )
{
	return cassette_legacy_construct(cassette, &tzx_legacy_fill_wave);
}

static casserr_t tap_cassette_load( cassette_image *cassette )
{
	return cassette_legacy_construct(cassette, &tap_legacy_fill_wave);
}

static casserr_t cdt_cassette_load( cassette_image *cassette )
{
	return cassette_legacy_construct(cassette, &cdt_legacy_fill_wave);
}

static const struct CassetteFormat tzx_cassette_format =
{
	"tzx",
	tzx_cassette_identify,
	tzx_cassette_load,
	nullptr
};

static const struct CassetteFormat tap_cassette_format =
{
	"tap,blk",
	tap_cassette_identify,
	tap_cassette_load,
	nullptr
};

static const struct CassetteFormat cdt_cassette_format =
{
	"cdt",
	cdt_cassette_identify,
	cdt_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(tzx_cassette_formats)
	CASSETTE_FORMAT(tzx_cassette_format)
	CASSETTE_FORMAT(tap_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(cdt_cassette_formats)
	CASSETTE_FORMAT(cdt_cassette_format)
CASSETTE_FORMATLIST_END
