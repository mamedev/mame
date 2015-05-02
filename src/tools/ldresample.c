// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ldresample.c

    Laserdisc audio synchronizer and resampler.

****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <new>
#include <assert.h>
#include "bitmap.h"
#include "chd.h"
#include "avhuff.h"
#include "vbiparse.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// size of window where we scan ahead to find maximum; this should be large enough to
// catch peaks of even slow waves
const UINT32 MAXIMUM_WINDOW_SIZE = 40;

// number of standard deviations away from silence that we consider a real signal
const UINT32 SIGNAL_DEVIATIONS = 100;

// number of standard deviations away from silence that we consider the start of a signal
const UINT32 SIGNAL_START_DEVIATIONS = 5;

// number of consecutive entries of signal before we consider that we found it
const UINT32 MINIMUM_SIGNAL_COUNT = 20;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct movie_info
{
	double          framerate;
	int             iframerate;
	int             numfields;
	int             width;
	int             height;
	int             samplerate;
	int             channels;
	int             interlaced;
	bitmap_yuy16    bitmap;
	std::vector<INT16>   lsound;
	std::vector<INT16>   rsound;
	UINT32          samples;
};


// ======================> chd_resample_compressor

class chd_resample_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_resample_compressor(chd_file &source, movie_info &info, INT64 ioffset, INT64 islope)
		: m_source(source),
			m_info(info),
			m_ioffset(ioffset),
			m_islope(islope) { }

	// read interface
	virtual UINT32 read_data(void *_dest, UINT64 offset, UINT32 length)
	{
		assert(offset % m_source.hunk_bytes() == 0);
		assert(length % m_source.hunk_bytes() == 0);

		UINT32 startfield = offset / m_source.hunk_bytes();
		UINT32 endfield = startfield + length / m_source.hunk_bytes();
		UINT8 *dest = reinterpret_cast<UINT8 *>(_dest);

		for (UINT32 fieldnum = startfield; fieldnum < endfield; fieldnum++)
		{
			generate_one_frame(dest, m_source.hunk_bytes(), fieldnum);
			dest += m_source.hunk_bytes();
		}
		return length;
	}

private:
	// internal helpers
	void generate_one_frame(UINT8 *dest, UINT32 datasize, UINT32 fieldnum);

	// internal state
	chd_file &                  m_source;
	movie_info &                m_info;
	INT64                       m_ioffset;
	INT64                       m_islope;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  field_to_sample_number - given a field number
//  compute the absolute sample number for the
//  first sample of that field
//-------------------------------------------------

inline UINT32 field_to_sample_number(const movie_info &info, UINT32 field)
{
	return (UINT64(info.samplerate) * UINT64(field) * UINT64(1000000) + info.iframerate - 1) / UINT64(info.iframerate);
}


//-------------------------------------------------
//  sample_number_to_field - given a sample number
//  compute the field where it is located and
//  the offset within the field
//-------------------------------------------------

inline UINT32 sample_number_to_field(const movie_info &info, UINT32 samplenum, UINT32 &offset)
{
	UINT32 guess = (UINT64(samplenum) * UINT64(info.iframerate) + (UINT64(info.samplerate) * UINT64(1000000) - 1)) / (UINT64(info.samplerate) * UINT64(1000000));
	while (1)
	{
		UINT32 fieldstart = field_to_sample_number(info, guess);
		UINT32 fieldend = field_to_sample_number(info, guess + 1);
		if (samplenum >= fieldstart && samplenum < fieldend)
		{
			offset = samplenum - fieldstart;
			return guess;
		}
		else if (samplenum < fieldstart)
			guess--;
		else
			guess++;
	}
}



//**************************************************************************
//  CHD HANDLING
//**************************************************************************

//-------------------------------------------------
//  open_chd - open a CHD file and return
//  information about it
//-------------------------------------------------

static chd_error open_chd(chd_file &file, const char *filename, movie_info &info)
{
	// open the file
	chd_error chderr = file.open(filename);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file: %s\n", chd_file::error_string(chderr));
		return chderr;
	}

	// get the metadata
	std::string metadata;
	chderr = file.read_metadata(AV_METADATA_TAG, 0, metadata);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting A/V metadata: %s\n", chd_file::error_string(chderr));
		return chderr;
	}

	// extract the info
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	if (sscanf(metadata.c_str(), AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
	{
		fprintf(stderr, "Improperly formatted metadata\n");
		return CHDERR_INVALID_DATA;
	}

	// extract movie info
	info.iframerate = fps * 1000000 + fpsfrac;
	info.framerate = info.iframerate / 1000000.0;
	info.numfields = file.hunk_count();
	info.width = width;
	info.height = height;
	info.interlaced = interlaced;
	info.samplerate = rate;
	info.channels = channels;

	// allocate buffers
	info.bitmap.resize(info.width, info.height);
	info.lsound.resize(info.samplerate);
	info.rsound.resize(info.samplerate);
	return CHDERR_NONE;
}


//-------------------------------------------------
//  create_chd - create a new CHD file
//-------------------------------------------------

static chd_error create_chd(chd_file_compressor &file, const char *filename, chd_file &source, const movie_info &info)
{
	// create the file
	chd_codec_type compression[4] = { CHD_CODEC_AVHUFF };
	chd_error chderr = file.create(filename, source.logical_bytes(), source.hunk_bytes(), source.unit_bytes(), compression);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating new CHD file: %s\n", chd_file::error_string(chderr));
		return chderr;
	}

	// clone the metadata
	chderr = file.clone_all_metadata(source);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error cloning metadata: %s\n", chd_file::error_string(chderr));
		return chderr;
	}

	// begin compressing
	file.compress_begin();
	return CHDERR_NONE;
}


//-------------------------------------------------
//  read_chd - read a field from a CHD file
//-------------------------------------------------

static bool read_chd(chd_file &file, UINT32 field, movie_info &info, UINT32 soundoffs)
{
	// configure the codec
	avhuff_decompress_config avconfig;
	avconfig.video.wrap(info.bitmap, info.bitmap.cliprect());
	avconfig.maxsamples = info.lsound.size();
	avconfig.actsamples = &info.samples;
	avconfig.audio[0] = &info.lsound[soundoffs];
	avconfig.audio[1] = &info.rsound[soundoffs];

	// configure the decompressor for this field
	file.codec_configure(CHD_CODEC_AVHUFF, AVHUFF_CODEC_DECOMPRESS_CONFIG, &avconfig);

	// read the field
	chd_error chderr = file.read_hunk(field, NULL);
	return (chderr == CHDERR_NONE);
}



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  find_edge_near_field - given a field number,
//  load +/- 1/2 second on either side and find
//  an audio edge
//-------------------------------------------------

static bool find_edge_near_field(chd_file &srcfile, UINT32 fieldnum, movie_info &info, bool report_best_field, INT32 &delta)
{
	// clear the sound buffers
	memset(&info.lsound[0], 0, info.lsound.size() * 2);
	memset(&info.rsound[0], 0, info.rsound.size() * 2);

	// read 1 second around the target area
	int fields_to_read = info.iframerate / 1000000;
	INT32 firstfield = fieldnum - (fields_to_read / 2);
	UINT32 targetsoundstart = 0;
	UINT32 firstfieldend = 0;
	UINT32 fieldstart[100];
	UINT32 soundend = 0;
	for (INT32 curfield = 0; curfield < fields_to_read; curfield++)
	{
		// remember the start of each field
		fieldstart[curfield] = soundend;

		// remember the sound offset where the initial fieldnum is
		if (firstfield + curfield == fieldnum)
			targetsoundstart = soundend;

		// read the frame and samples
		if (firstfield + curfield >= 0)
		{
			read_chd(srcfile, firstfield + curfield, info, soundend);
			soundend += info.samples;

			// also remember the offset at the end of the first field
			if (firstfieldend == 0)
				firstfieldend = soundend;
		}
	}

	// compute absolute deltas across the samples
	for (UINT32 sampnum = 0; sampnum < soundend; sampnum++)
	{
		info.lsound[sampnum] = labs(info.lsound[sampnum + 1] - info.lsound[sampnum]);
		info.rsound[sampnum] = labs(info.rsound[sampnum + 1] - info.rsound[sampnum]);
	}

	// for each sample in the collection, find the highest deltas over the
	// next few samples, and take the nth highest value (to remove outliers)
	for (UINT32 sampnum = 0; sampnum < soundend - MAXIMUM_WINDOW_SIZE; sampnum++)
	{
		// scan forward over the maximum window
		UINT32 lmax = 0, rmax = 0;
		for (UINT32 scannum = 0; scannum < MAXIMUM_WINDOW_SIZE; scannum++)
		{
			if (info.lsound[sampnum + scannum] > lmax)
				lmax = info.lsound[sampnum + scannum];
			if (info.rsound[sampnum + scannum] > rmax)
				rmax = info.rsound[sampnum + scannum];
		}

		// replace this sample with the maximum value found
		info.lsound[sampnum] = lmax;
		info.rsound[sampnum] = rmax;
	}

	// now compute the average over the first field, which is assumed to be silence
	UINT32 firstlavg = 0;
	UINT32 firstravg = 0;
	for (UINT32 sampnum = 0; sampnum < firstfieldend; sampnum++)
	{
		firstlavg += info.lsound[sampnum];
		firstravg += info.rsound[sampnum];
	}
	firstlavg /= firstfieldend;
	firstravg /= firstfieldend;

	// then compute the standard deviation over the first field
	UINT32 firstldev = 0;
	UINT32 firstrdev = 0;
	for (UINT32 sampnum = 0; sampnum < firstfieldend; sampnum++)
	{
		firstldev += (info.lsound[sampnum] - firstlavg) * (info.lsound[sampnum] - firstlavg);
		firstrdev += (info.rsound[sampnum] - firstravg) * (info.rsound[sampnum] - firstravg);
	}
	firstldev = sqrt(double(firstldev) / firstfieldend);
	firstrdev = sqrt(double(firstrdev) / firstfieldend);

	// scan forward through the samples, counting consecutive samples more than
	// SIGNAL_DEVIATIONS standard deviations away from silence
	UINT32 lcount = 0;
	UINT32 rcount = 0;
	UINT32 sampnum = 0;
	for (sampnum = 0; sampnum < soundend; sampnum++)
	{
		// left speaker
		if (info.lsound[sampnum] > firstlavg + SIGNAL_DEVIATIONS * firstldev)
			lcount++;
		else
			lcount = 0;

		// right speaker
		if (info.rsound[sampnum] > firstravg + SIGNAL_DEVIATIONS * firstrdev)
			rcount++;
		else
			rcount = 0;

		// stop if we find enough
		if (lcount > MINIMUM_SIGNAL_COUNT || rcount > MINIMUM_SIGNAL_COUNT)
			break;
	}

	// if we didn't find any, return failure
	if (sampnum >= soundend)
	{
		if (!report_best_field)
			printf("Field %5d: Unable to find edge\n", fieldnum);
		return false;
	}

	// scan backwards to find the start of the signal
	for ( ; sampnum > 0; sampnum--)
		if (info.lsound[sampnum - 1] < firstlavg + SIGNAL_START_DEVIATIONS * firstldev ||
			info.rsound[sampnum - 1] < firstravg + SIGNAL_START_DEVIATIONS * firstrdev)
			break;

	// if we're to report the best field, figure out which field we are in
	if (report_best_field)
	{
		INT32 curfield;
		for (curfield = 0; curfield < fields_to_read - 1; curfield++)
			if (sampnum < fieldstart[curfield + 1])
				break;
		printf("Field %5d: Edge found at offset %d (frame %.1f)\n", firstfield + curfield, sampnum - fieldstart[curfield], (double)(firstfield + curfield) * 0.5);
	}

	// otherwise, compute the delta from the provided field number
	else
	{
		printf("Field %5d: Edge at offset %d from expected (found at %d, expected %d)\n", fieldnum, sampnum - targetsoundstart, sampnum, targetsoundstart);
		delta = sampnum - targetsoundstart;
	}
	return true;
}


//-------------------------------------------------
//  generate_one_frame - generate a single
//  resampled frame
//-------------------------------------------------

void chd_resample_compressor::generate_one_frame(UINT8 *dest, UINT32 datasize, UINT32 fieldnum)
{
	// determine the first field needed to cover this range of samples
	UINT32 srcbegin = field_to_sample_number(m_info, fieldnum);
	INT64 dstbegin = (INT64(srcbegin) << 24) + m_ioffset + m_islope * fieldnum;
	UINT32 dstbeginoffset;
	INT32 dstbeginfield;
	if (dstbegin >= 0)
		dstbeginfield = sample_number_to_field(m_info, dstbegin >> 24, dstbeginoffset);
	else
	{
		dstbeginfield = -1 - sample_number_to_field(m_info, -dstbegin >> 24, dstbeginoffset);
		dstbeginoffset = (field_to_sample_number(m_info, -dstbeginfield) - field_to_sample_number(m_info, -dstbeginfield - 1)) - dstbeginoffset;
	}

	// determine the last field needed to cover this range of samples
	UINT32 srcend = field_to_sample_number(m_info, fieldnum + 1);
	INT64 dstend = (INT64(srcend) << 24) + m_ioffset + m_islope * (fieldnum + 1);
	UINT32 dstendoffset;
	INT32 dstendfield;
	if (dstend >= 0)
		dstendfield = sample_number_to_field(m_info, dstend >> 24, dstendoffset);
	else
	{
		dstendfield = -1 - -sample_number_to_field(m_info, -dstend >> 24, dstendoffset);
		dstendoffset = (field_to_sample_number(m_info, -dstendfield) - field_to_sample_number(m_info, -dstendfield - 1)) - dstendoffset;
	}
/*
printf("%5d: start=%10d (%5d.%03d) end=%10d (%5d.%03d)\n",
        fieldnum,
        (INT32)(dstbegin >> 24), dstbeginfield, dstbeginoffset,
        (INT32)(dstend >> 24), dstendfield, dstendoffset);
*/
	// read all samples required into the end of the sound buffers
	UINT32 dstoffset = srcend - srcbegin;
	for (INT32 dstfield = dstbeginfield; dstfield <= dstendfield; dstfield++)
	{
		if (dstfield >= 0)
			read_chd(m_source, dstfield, m_info, dstoffset);
		else
		{
			m_info.samples = field_to_sample_number(m_info, -dstfield) - field_to_sample_number(m_info, -dstfield - 1);
			memset(&m_info.lsound[dstoffset], 0, m_info.samples * sizeof(m_info.lsound[0]));
			memset(&m_info.rsound[dstoffset], 0, m_info.samples * sizeof(m_info.rsound[0]));
		}
		dstoffset += m_info.samples;
	}

	// resample the destination samples to the source
	dstoffset = srcend - srcbegin;
	INT64 dstpos = dstbegin;
	INT64 dststep = (dstend - dstbegin) / INT64(srcend - srcbegin);
	for (UINT32 srcoffset = 0; srcoffset < srcend - srcbegin; srcoffset++)
	{
		m_info.lsound[srcoffset] = m_info.lsound[(int)(dstoffset + dstbeginoffset + (dstpos >> 24) - (dstbegin >> 24))];
		m_info.rsound[srcoffset] = m_info.rsound[(int)(dstoffset + dstbeginoffset + (dstpos >> 24) - (dstbegin >> 24))];
		dstpos += dststep;
	}

	// read the original frame, pointing the sound buffer past where we've calculated
	read_chd(m_source, fieldnum, m_info, srcend - srcbegin);

	// assemble the final frame
	dynamic_buffer buffer;
	INT16 *sampledata[2] = { &m_info.lsound[0], &m_info.rsound[0] };
	avhuff_encoder::assemble_data(buffer, m_info.bitmap, m_info.channels, m_info.samples, sampledata);
	memcpy(dest, &buffer[0], MIN(buffer.size(), datasize));
	if (buffer.size() < datasize)
		memset(&dest[buffer.size()], 0, datasize - buffer.size());
}


//-------------------------------------------------
//  usage - display program usage
//-------------------------------------------------

static int usage(void)
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "  ldresample source.chd\n");
	fprintf(stderr, "  ldresample source.chd output.chd offset [slope]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where offset and slope make a linear equation f(x) which\n");
	fprintf(stderr, "describes the sample offset from the source as a function\n");
	fprintf(stderr, "of field number.\n");
	return 1;
}


//-------------------------------------------------
//  main - main entry point
//-------------------------------------------------

int main(int argc, char *argv[])
{
	// verify arguments
	if (argc < 2)
		return usage();
	const char *srcfilename = argv[1];
	const char *dstfilename = (argc < 3) ? NULL : argv[2];
	double offset = (argc < 4) ? 0.0 : atof(argv[3]);
	double slope = (argc < 5) ? 1.0 : atof(argv[4]);

	// print basic information
	printf("Input file: %s\n", srcfilename);
	if (dstfilename != NULL)
	{
		printf("Output file: %s\n", dstfilename);
		printf("Offset: %f\n", offset);
		printf("Slope: %f\n", slope);
	}

	// open the source file
	chd_file srcfile;
	movie_info info;
	chd_error err = open_chd(srcfile, srcfilename, info);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Unable to open file '%s'\n", srcfilename);
		return 1;
	}

	// output some basics
	printf("Video dimensions: %dx%d\n", info.width, info.height);
	printf("Video frame rate: %.2fHz\n", info.framerate);
	printf("Sample rate: %dHz\n", info.samplerate);
	printf("Total fields: %d\n", info.numfields);

	// if we don't have a destination file, scan for edges
	if (dstfilename == NULL)
	{
		for (UINT32 fieldnum = 60; fieldnum < info.numfields - 60; fieldnum += 30)
		{
			fprintf(stderr, "Field %5d\r", fieldnum);
			INT32 delta;
			find_edge_near_field(srcfile, fieldnum, info, true, delta);
		}
	}

	// otherwise, resample the source to the destination
	else
	{
		// open the destination file
		chd_resample_compressor dstfile(srcfile, info, INT64(offset * 65536.0 * 256.0), INT64(slope * 65536.0 * 256.0));
		err = create_chd(dstfile, dstfilename, srcfile, info);
		if (dstfile == NULL)
		{
			fprintf(stderr, "Unable to create file '%s'\n", dstfilename);
			return 1;
		}

		// loop over all the fields in the source file
		double progress, ratio;
		osd_ticks_t last_update = 0;
		while (dstfile.compress_continue(progress, ratio) == CHDERR_COMPRESSING)
			if (osd_ticks() - last_update > osd_ticks_per_second() / 4)
			{
				last_update = osd_ticks();
				printf("Processing, %.1f%% complete....\r", progress * 100.0);
			}
	}
	return 0;
}
