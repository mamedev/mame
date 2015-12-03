// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    TOC parser for CHD compression frontend
    Handles CDRDAO .toc, CDRWIN .cue, Nero .nrg, and Sega GDROM .gdi

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "osdcore.h"
#include "chd.h"
#include "chdcd.h"
#include "corefile.h"



/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

/**
 * @def TOKENIZE();
 *
 * @brief   A macro that defines tokenize.
 *
 * @param   linebuffer          The linebuffer.
 * @param   i                   Zero-based index of the.
 * @param   sizeof(linebuffer)  The sizeof(linebuffer)
 * @param   token               The token.
 * @param   sizeof(token)       The sizeof(token)
 */

#define TOKENIZE i = tokenize( linebuffer, i, sizeof(linebuffer), token, sizeof(token) );



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/** @brief  The linebuffer[ 512]. */
static char linebuffer[512];



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/**
 * @fn  static std::string get_file_path(std::string &path)
 *
 * @brief   Gets file path.
 *
 * @param [in,out]  path    Full pathname of the file.
 *
 * @return  The file path.
 */

static std::string get_file_path(std::string &path)
{
	int pos = path.find_last_of('\\');
	if (pos!=-1) {
		path = path.substr(0,pos+1);
	} else {
		pos = path.find_last_of('/');
		path = path.substr(0,pos+1);
	}
	return path;
}
/*-------------------------------------------------
    get_file_size - get the size of a file
-------------------------------------------------*/

/**
 * @fn  static UINT64 get_file_size(const char *filename)
 *
 * @brief   Gets file size.
 *
 * @param   filename    Filename of the file.
 *
 * @return  The file size.
 */

static UINT64 get_file_size(const char *filename)
{
	osd_file *file;
	UINT64 filesize = 0;
	file_error filerr;

	filerr = osd_open(filename, OPEN_FLAG_READ, &file, &filesize);
	if (filerr == FILERR_NONE)
		osd_close(file);

	return filesize;
}


/*-------------------------------------------------
    tokenize - get a token from the line buffer
-------------------------------------------------*/

/**
 * @fn  static int tokenize( const char *linebuffer, int i, int linebuffersize, char *token, int tokensize )
 *
 * @brief   Tokenizes.
 *
 * @param   linebuffer      The linebuffer.
 * @param   i               Zero-based index of the.
 * @param   linebuffersize  The linebuffersize.
 * @param [in,out]  token   If non-null, the token.
 * @param   tokensize       The tokensize.
 *
 * @return  An int.
 */

static int tokenize( const char *linebuffer, int i, int linebuffersize, char *token, int tokensize )
{
	int j = 0;
	int singlequote = 0;
	int doublequote = 0;

	while ((i < linebuffersize) && isspace((UINT8)linebuffer[i]))
	{
		i++;
	}

	while ((i < linebuffersize) && (j < tokensize))
	{
		if (!singlequote && linebuffer[i] == '"' )
		{
			doublequote = !doublequote;
		}
		else if (!doublequote && linebuffer[i] == '\'')
		{
			singlequote = !singlequote;
		}
		else if (!singlequote && !doublequote && isspace((UINT8)linebuffer[i]))
		{
			break;
		}
		else
		{
			token[j] = linebuffer[i];
			j++;
		}

		i++;
	}

	token[j] = '\0';

	return i;
}


/*-------------------------------------------------
    msf_to_frames - convert m:s:f into a number of frames
-------------------------------------------------*/

/**
 * @fn  static int msf_to_frames( char *token )
 *
 * @brief   Msf to frames.
 *
 * @param [in,out]  token   If non-null, the token.
 *
 * @return  An int.
 */

static int msf_to_frames( char *token )
{
	int m = 0;
	int s = 0;
	int f = 0;

	if( sscanf( token, "%d:%d:%d", &m, &s, &f ) == 1 )
	{
		f = m;
	}
	else
	{
		/* convert to just frames */
		s += (m * 60);
		f += (s * 75);
	}

	return f;
}

/*-------------------------------------------------
    parse_wav_sample - takes a .WAV file, verifies
    that the file is 16 bits, and returns the
    length in bytes of the data and the offset in
    bytes to where the data starts in the file.
-------------------------------------------------*/

/**
 * @fn  static UINT32 parse_wav_sample(const char *filename, UINT32 *dataoffs)
 *
 * @brief   Parse WAV sample.
 *
 * @param   filename            Filename of the file.
 * @param [in,out]  dataoffs    If non-null, the dataoffs.
 *
 * @return  An UINT32.
 */

static UINT32 parse_wav_sample(const char *filename, UINT32 *dataoffs)
{
	unsigned long offset = 0;
	UINT32 length, rate, filesize;
	UINT16 bits, temp16;
	char buf[32];
	osd_file *file;
	file_error filerr;
	UINT64 fsize = 0;
	UINT32 actual;

	filerr = osd_open(filename, OPEN_FLAG_READ, &file, &fsize);
	if (filerr != FILERR_NONE)
	{
		printf("ERROR: could not open (%s)\n", filename);
		return 0;
	}

	/* read the core header and make sure it's a WAVE file */
	osd_read(file, buf, 0, 4, &actual);
	offset += actual;
	if (offset < 4)
	{
		osd_close(file);
		printf("ERROR: unexpected RIFF offset %lu (%s)\n", offset, filename);
		return 0;
	}
	if (memcmp(&buf[0], "RIFF", 4) != 0)
	{
		osd_close(file);
		printf("ERROR: could not find RIFF header (%s)\n", filename);
		return 0;
	}

	/* get the total size */
	osd_read(file, &filesize, offset, 4, &actual);
	offset += actual;
	if (offset < 8)
	{
		osd_close(file);
		printf("ERROR: unexpected size offset %lu (%s)\n", offset, filename);
		return 0;
	}
	filesize = LITTLE_ENDIANIZE_INT32(filesize);

	/* read the RIFF file type and make sure it's a WAVE file */
	osd_read(file, buf, offset, 4, &actual);
	offset += actual;
	if (offset < 12)
	{
		osd_close(file);
		printf("ERROR: unexpected WAVE offset %lu (%s)\n", offset, filename);
		return 0;
	}
	if (memcmp(&buf[0], "WAVE", 4) != 0)
	{
		osd_close(file);
		printf("ERROR: could not find WAVE header (%s)\n", filename);
		return 0;
	}

	/* seek until we find a format tag */
	while (1)
	{
		osd_read(file, buf, offset, 4, &actual);
		offset += actual;
		osd_read(file, &length, offset, 4, &actual);
		offset += actual;
		length = LITTLE_ENDIANIZE_INT32(length);
		if (memcmp(&buf[0], "fmt ", 4) == 0)
			break;

		/* seek to the next block */
		offset += length;
		if (offset >= filesize)
		{
			osd_close(file);
			printf("ERROR: could not find fmt tag (%s)\n", filename);
			return 0;
		}
	}

	/* read the format -- make sure it is PCM */
	osd_read(file, &temp16, offset, 2, &actual);
	offset += actual;
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
	{
		osd_close(file);
		printf("ERROR: unsupported format %u - only PCM is supported (%s)\n", temp16, filename);
		return 0;
	}

	/* number of channels -- only stereo is supported */
	osd_read(file, &temp16, offset, 2, &actual);
	offset += actual;
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 2)
	{
		osd_close(file);
		printf("ERROR: unsupported number of channels %u - only stereo is supported (%s)\n", temp16, filename);
		return 0;
	}

	/* sample rate */
	osd_read(file, &rate, offset, 4, &actual);
	offset += actual;
	rate = LITTLE_ENDIANIZE_INT32(rate);
	if (rate != 44100)
	{
		osd_close(file);
		printf("ERROR: unsupported samplerate %u - only 44100 is supported (%s)\n", rate, filename);
		return 0;
	}

	/* bytes/second and block alignment are ignored */
	osd_read(file, buf, offset, 6, &actual);
	offset += actual;

	/* bits/sample */
	osd_read(file, &bits, offset, 2, &actual);
	offset += actual;
	bits = LITTLE_ENDIANIZE_INT16(bits);
	if (bits != 16)
	{
		osd_close(file);
		printf("ERROR: unsupported bits/sample %u - only 16 is supported (%s)\n", bits, filename);
		return 0;
	}

	/* seek past any extra data */
	offset += length - 16;

	/* seek until we find a data tag */
	while (1)
	{
		osd_read(file, buf, offset, 4, &actual);
		offset += actual;
		osd_read(file, &length, offset, 4, &actual);
		offset += actual;
		length = LITTLE_ENDIANIZE_INT32(length);
		if (memcmp(&buf[0], "data", 4) == 0)
			break;

		/* seek to the next block */
		offset += length;
		if (offset >= filesize)
		{
			osd_close(file);
			printf("ERROR: could not find data tag (%s)\n", filename);
			return 0;
		}
	}

	osd_close(file);

	/* if there was a 0 length data block, we're done */
	if (length == 0)
	{
		printf("ERROR: empty data block (%s)\n", filename);
		return 0;
	}

	*dataoffs = offset;

	return length;
}

/**
 * @fn  UINT16 read_uint16(FILE *infile)
 *
 * @brief   Reads uint 16.
 *
 * @param [in,out]  infile  If non-null, the infile.
 *
 * @return  The uint 16.
 */

UINT16 read_uint16(FILE *infile)
{
	UINT16 res = 0;
	unsigned char buffer[2];

	fread(buffer, 2, 1, infile);

	res = buffer[1] | buffer[0]<<8;

	return res;
}

/**
 * @fn  UINT32 read_uint32(FILE *infile)
 *
 * @brief   Reads uint 32.
 *
 * @param [in,out]  infile  If non-null, the infile.
 *
 * @return  The uint 32.
 */

UINT32 read_uint32(FILE *infile)
{
	UINT32 res = 0;
	unsigned char buffer[4];

	fread(buffer, 4, 1, infile);

	res = buffer[3] | buffer[2]<<8 | buffer[1]<<16 | buffer[0]<<24;

	return res;
}

/**
 * @fn  UINT64 read_uint64(FILE *infile)
 *
 * @brief   Reads uint 64.
 *
 * @param [in,out]  infile  If non-null, the infile.
 *
 * @return  The uint 64.
 */

UINT64 read_uint64(FILE *infile)
{
	UINT64 res0 = U64(0), res1 = U64(0);
	UINT64 res;
	unsigned char buffer[8];

	fread(buffer, 8, 1, infile);

	res0 = buffer[3] | buffer[2]<<8 | buffer[1]<<16 | buffer[0]<<24;
	res1 = buffer[7] | buffer[6]<<8 | buffer[5]<<16 | buffer[4]<<24;

	res = res0<<32 | res1;

	return res;
}

/*-------------------------------------------------
    chdcd_parse_nero - parse a Nero .NRG file
-------------------------------------------------*/

/**
 * @fn  chd_error chdcd_parse_nero(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
 *
 * @brief   Chdcd parse nero.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A chd_error.
 */

chd_error chdcd_parse_nero(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
{
	FILE *infile;
	unsigned char buffer[12];
	UINT32 chain_offs, chunk_size;
	int done = 0;

	std::string path = std::string(tocfname);

	infile = fopen(tocfname, "rb");
	path = get_file_path(path);

	if (infile == (FILE *)nullptr)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	// seek to 12 bytes before the end
	fseek(infile, -12, SEEK_END);
	fread(buffer, 12, 1, infile);

	if (memcmp(buffer, "NER5", 4))
	{
		printf("ERROR: Not a Nero 5.5 or later image!\n");
		fclose(infile);
		return CHDERR_UNSUPPORTED_VERSION;
	}

	chain_offs = buffer[11] | (buffer[10]<<8) | (buffer[9]<<16) | (buffer[8]<<24);

	if ((buffer[7] != 0) || (buffer[6] != 0) || (buffer[5] != 0) || (buffer[4] != 0))
	{
		printf("ERROR: File size is > 4GB, this version of CHDMAN cannot handle it.");
		fclose(infile);
		return CHDERR_UNSUPPORTED_FORMAT;
	}

//  printf("NER5 detected, chain offset: %x\n", chain_offs);

	while (!done)
	{
		UINT32 offset;
		UINT8 start, end;
		int track;

		fseek(infile, chain_offs, SEEK_SET);
		fread(buffer, 8, 1, infile);

		chunk_size = (buffer[7] | buffer[6]<<8 | buffer[5]<<16 | buffer[4]<<24);

//      printf("Chunk type: %c%c%c%c, size %x\n", buffer[0], buffer[1], buffer[2], buffer[3], chunk_size);

		// we want the DAOX chunk, which has the TOC information
		if (!memcmp(buffer, "DAOX", 4))
		{
			// skip second chunk size and UPC code
			fseek(infile, 20, SEEK_CUR);

			fread(&start, 1, 1, infile);
			fread(&end, 1, 1, infile);

//          printf("Start track %d  End track: %d\n", start, end);

			outtoc.numtrks = (end-start) + 1;

			offset = 0;
			for (track = start; track <= end; track++)
			{
				UINT32 size, mode;
				UINT64 index0, index1, track_end;

				fseek(infile, 12, SEEK_CUR);    // skip ISRC code
				size = read_uint16(infile);
				mode = read_uint16(infile);
				fseek(infile, 2, SEEK_CUR);
				index0 = read_uint64(infile);
				index1 = read_uint64(infile);
				track_end = read_uint64(infile);

//              printf("Track %d: sector size %d mode %x index0 %llx index1 %llx track_end %llx (pregap %d sectors, length %d sectors)\n", track, size, mode, index0, index1, track_end, (UINT32)(index1-index0)/size, (UINT32)(track_end-index1)/size);
				outinfo.track[track-1].fname.assign(tocfname);
				outinfo.track[track-1].offset = offset + (UINT32)(index1-index0);
				outinfo.track[track-1].idx0offs = 0;
				outinfo.track[track-1].idx1offs = 0;

				switch (mode)
				{
					case 0x0000:    // 2048 byte data
						outtoc.tracks[track-1].trktype = CD_TRACK_MODE1;
						outinfo.track[track-1].swap = false;
						break;

					case 0x0300:    // Mode 2 Form 1
						printf("ERROR: Mode 2 Form 1 tracks not supported\n");
						fclose(infile);
						return CHDERR_UNSUPPORTED_FORMAT;

					case 0x0500:    // raw data
						printf("ERROR: Raw data tracks not supported\n");
						fclose(infile);
						return CHDERR_UNSUPPORTED_FORMAT;

					case 0x0600:    // 2352 byte mode 2 raw
						outtoc.tracks[track-1].trktype = CD_TRACK_MODE2_RAW;
						outinfo.track[track-1].swap = false;
						break;

					case 0x0700:    // 2352 byte audio
						outtoc.tracks[track-1].trktype = CD_TRACK_AUDIO;
						outinfo.track[track-1].swap = true;
						break;

					case 0x0f00:    // raw data with sub-channel
						printf("ERROR: Raw data tracks with sub-channel not supported\n");
						fclose(infile);
						return CHDERR_UNSUPPORTED_FORMAT;

					case 0x1000:    // audio with sub-channel
						printf("ERROR: Audio tracks with sub-channel not supported\n");
						fclose(infile);
						return CHDERR_UNSUPPORTED_FORMAT;

					case 0x1100:    // raw Mode 2 Form 1 with sub-channel
						printf("ERROR: Raw Mode 2 Form 1 tracks with sub-channel not supported\n");
						fclose(infile);
						return CHDERR_UNSUPPORTED_FORMAT;

					default:
						printf("ERROR: Unknown track type %x, contact MAMEDEV!\n", mode);
						fclose(infile);
						return CHDERR_UNSUPPORTED_FORMAT;
				}

				outtoc.tracks[track-1].datasize = size;

				outtoc.tracks[track-1].subtype = CD_SUB_NONE;
				outtoc.tracks[track-1].subsize = 0;

				outtoc.tracks[track-1].pregap = (UINT32)(index1-index0)/size;
				outtoc.tracks[track-1].frames = (UINT32)(track_end-index1)/size;
				outtoc.tracks[track-1].postgap = 0;
				outtoc.tracks[track-1].pgtype = 0;
				outtoc.tracks[track-1].pgsub = CD_SUB_NONE;
				outtoc.tracks[track-1].pgdatasize = 0;
				outtoc.tracks[track-1].pgsubsize = 0;
				outtoc.tracks[track-1].padframes = 0;

				offset += (UINT32)track_end-index1;
			}
		}

		if (!memcmp(buffer, "END!", 4))
		{
			done = 1;
		}
		else
		{
			chain_offs += chunk_size + 8;
		}
	}

	fclose(infile);

	return CHDERR_NONE;
}

/*-------------------------------------------------
    chdcd_parse_iso - parse a .ISO file
-------------------------------------------------*/

/**
 * @fn  chd_error chdcd_parse_iso(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
 *
 * @brief   Chdcd parse ISO.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A chd_error.
 */

chd_error chdcd_parse_iso(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
{
	FILE *infile;
	std::string path = std::string(tocfname);

	infile = fopen(tocfname, "rb");
	path = get_file_path(path);

	if (infile == (FILE *)nullptr)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	fseek(infile, 0, SEEK_END);
	long size = ftell(infile);
	fclose(infile);


	outtoc.numtrks = 1;

	outinfo.track[0].fname = tocfname;
	outinfo.track[0].offset = 0;
	outinfo.track[0].idx0offs = 0;
	outinfo.track[0].idx1offs = 0;

	if ((size % 2048)==0 ) {
		outtoc.tracks[0].trktype = CD_TRACK_MODE1;
		outtoc.tracks[0].frames = size / 2048;
		outtoc.tracks[0].datasize = 2048;
		outinfo.track[0].swap = false;
	} else if ((size % 2352)==0 ) {
		// 2352 byte mode 2 raw
		outtoc.tracks[0].trktype = CD_TRACK_MODE2_RAW;
		outtoc.tracks[0].frames = size / 2352;
		outtoc.tracks[0].datasize = 2352;
		outinfo.track[0].swap = false;
	} else {
		printf("ERROR: Unrecognized track type\n");
		return CHDERR_UNSUPPORTED_FORMAT;
	}

	outtoc.tracks[0].subtype = CD_SUB_NONE;
	outtoc.tracks[0].subsize = 0;

	outtoc.tracks[0].pregap = 0;

	outtoc.tracks[0].postgap = 0;
	outtoc.tracks[0].pgtype = 0;
	outtoc.tracks[0].pgsub = CD_SUB_NONE;
	outtoc.tracks[0].pgdatasize = 0;
	outtoc.tracks[0].pgsubsize = 0;
	outtoc.tracks[0].padframes = 0;


	return CHDERR_NONE;
}

/*-------------------------------------------------
    chdcd_parse_gdi - parse a Sega GD-ROM rip
-------------------------------------------------*/

/**
 * @fn  static chd_error chdcd_parse_gdi(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
 *
 * @brief   Chdcd parse GDI.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A chd_error.
 */

static chd_error chdcd_parse_gdi(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
{
	FILE *infile;
	int i, numtracks;

	std::string path = std::string(tocfname);

	infile = fopen(tocfname, "rt");
	path = get_file_path(path);

	if (infile == (FILE *)nullptr)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	outtoc.flags = CD_FLAG_GDROM;

	fgets(linebuffer,511,infile);
	numtracks=atoi(linebuffer);

	for(i=0;i<numtracks;++i)
	{
		char *tok;
		int trknum;
		int trksize,trktype;
		int sz;

		fgets(linebuffer,511,infile);

		tok=strtok(linebuffer," ");

		trknum=atoi(tok)-1;

		outinfo.track[trknum].swap=false;
		outinfo.track[trknum].offset=0;

		outtoc.tracks[trknum].datasize = 0;
		outtoc.tracks[trknum].subtype = CD_SUB_NONE;
		outtoc.tracks[trknum].subsize = 0;
		outtoc.tracks[trknum].pgsub = CD_SUB_NONE;

		tok=strtok(nullptr," ");
		outtoc.tracks[trknum].physframeofs=atoi(tok);

		tok=strtok(nullptr," ");
		trktype=atoi(tok);

		tok=strtok(nullptr," ");
		trksize=atoi(tok);

		if(trktype==4 && trksize==2352)
		{
			outtoc.tracks[trknum].trktype=CD_TRACK_MODE1_RAW;
			outtoc.tracks[trknum].datasize=2352;
		}
		if(trktype==4 && trksize==2048)
		{
			outtoc.tracks[trknum].trktype=CD_TRACK_MODE1;
			outtoc.tracks[trknum].datasize=2048;
		}
		if(trktype==0)
		{
			outtoc.tracks[trknum].trktype=CD_TRACK_AUDIO;
			outtoc.tracks[trknum].datasize=2352;
			outinfo.track[trknum].swap = true;
		}

		std::string name;

		tok=strtok(nullptr," ");
		name = tok;
		if (tok[0]=='"') {
			do {
				tok=strtok(nullptr," ");
				if (tok!=nullptr) {
					name += " ";
					name += tok;
				}
			} while(tok!=nullptr && (strrchr(tok,'"')-tok !=(strlen(tok)-1)));
			strdelchr(name,'"');
		}
		outinfo.track[trknum].fname.assign(path).append(name);

		sz = get_file_size(outinfo.track[trknum].fname.c_str());

		outtoc.tracks[trknum].frames = sz/trksize;
		outtoc.tracks[trknum].padframes = 0;

		if (trknum != 0)
		{
			int dif=outtoc.tracks[trknum].physframeofs-(outtoc.tracks[trknum-1].frames+outtoc.tracks[trknum-1].physframeofs);
			outtoc.tracks[trknum-1].frames += dif;
			outtoc.tracks[trknum-1].padframes = dif;
		}
	}

	#if 0
	for(i=0; i < numtracks; i++)
	{
		printf("%s %d %d %d (true %d)\n", outinfo.track[i].fname.c_str(), outtoc.tracks[i].frames, outtoc.tracks[i].padframes, outtoc.tracks[i].physframeofs, outtoc.tracks[i].frames - outtoc.tracks[i].padframes);
	}
	#endif

	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc.numtrks = numtracks;

	return CHDERR_NONE;
}

/*-------------------------------------------------
    chdcd_parse_cue - parse a CDRWin format CUE file
-------------------------------------------------*/

/**
 * @fn  chd_error chdcd_parse_cue(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
 *
 * @brief   Chdcd parse cue.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A chd_error.
 */

chd_error chdcd_parse_cue(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
{
	FILE *infile;
	int i, trknum;
	static char token[512];
	std::string lastfname;
	UINT32 wavlen, wavoffs;
	std::string path = std::string(tocfname);

	infile = fopen(tocfname, "rt");
	path = get_file_path(path);
	if (infile == (FILE *)nullptr)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	trknum = -1;
	wavoffs = wavlen = 0;

	while (!feof(infile))
	{
		/* get the next line */
		fgets(linebuffer, 511, infile);

		/* if EOF didn't hit, keep going */
		if (!feof(infile))
		{
			i = 0;

			TOKENIZE

			if (!strcmp(token, "FILE"))
			{
				/* found the data file for a track */
				TOKENIZE

				/* keep the filename */
				lastfname.assign(path).append(token);

				/* get the file type */
				TOKENIZE

				if (!strcmp(token, "BINARY"))
				{
					outinfo.track[trknum+1].swap = false;
				}
				else if (!strcmp(token, "MOTOROLA"))
				{
					outinfo.track[trknum+1].swap = true;
				}
				else if (!strcmp(token, "WAVE"))
				{
					wavlen = parse_wav_sample(lastfname.c_str(), &wavoffs);
					if (!wavlen)
					{
						printf("ERROR: couldn't read [%s] or not a valid .WAV\n", lastfname.c_str());
						return CHDERR_INVALID_DATA;
					}
				}
				else
				{
					printf("ERROR: Unhandled track type %s\n", token);
					return CHDERR_UNSUPPORTED_FORMAT;
				}
			}
			else if (!strcmp(token, "TRACK"))
			{
				/* get the track number */
				TOKENIZE
				trknum = strtoul(token, nullptr, 10) - 1;

				/* next token on the line is the track type */
				TOKENIZE

				if (wavlen != 0)
				{
					outtoc.tracks[trknum].trktype = CD_TRACK_AUDIO;
					outtoc.tracks[trknum].frames = wavlen/2352;
					outinfo.track[trknum].offset = wavoffs;
					wavoffs = wavlen = 0;
				}
				else
				{
					outtoc.tracks[trknum].trktype = CD_TRACK_MODE1;
					outtoc.tracks[trknum].datasize = 0;
					outinfo.track[trknum].offset = 0;
				}
				outtoc.tracks[trknum].subtype = CD_SUB_NONE;
				outtoc.tracks[trknum].subsize = 0;
				outtoc.tracks[trknum].pregap = 0;
				outtoc.tracks[trknum].padframes = 0;
				outinfo.track[trknum].idx0offs = -1;
				outinfo.track[trknum].idx1offs = 0;

				outinfo.track[trknum].fname.assign(lastfname); // default filename to the last one

//              printf("trk %d: fname %s offset %d\n", trknum, outinfo.track[trknum].fname.c_str(), outinfo.track[trknum].offset);

				cdrom_convert_type_string_to_track_info(token, &outtoc.tracks[trknum]);
				if (outtoc.tracks[trknum].datasize == 0)
				{
					printf("ERROR: Unknown track type [%s].  Contact MAMEDEV.\n", token);
					return CHDERR_UNSUPPORTED_FORMAT;
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				cdrom_convert_subtype_string_to_track_info(token, &outtoc.tracks[trknum]);
			}
			else if (!strcmp(token, "INDEX"))   /* only in bin/cue files */
			{
				int idx, frames;

				/* get index number */
				TOKENIZE
				idx = strtoul(token, nullptr, 10);

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				if (idx == 0)
				{
					outinfo.track[trknum].idx0offs = frames;
				}
				else if (idx == 1)
				{
					outinfo.track[trknum].idx1offs = frames;
					if ((outtoc.tracks[trknum].pregap == 0) && (outinfo.track[trknum].idx0offs != -1))
					{
						outtoc.tracks[trknum].pregap = frames - outinfo.track[trknum].idx0offs;
						outtoc.tracks[trknum].pgtype = outtoc.tracks[trknum].trktype;
						switch (outtoc.tracks[trknum].pgtype)
						{
							case CD_TRACK_MODE1:
							case CD_TRACK_MODE2_FORM1:
								outtoc.tracks[trknum].pgdatasize = 2048;
								break;

							case CD_TRACK_MODE1_RAW:
							case CD_TRACK_MODE2_RAW:
							case CD_TRACK_AUDIO:
								outtoc.tracks[trknum].pgdatasize = 2352;
								break;

							case CD_TRACK_MODE2:
							case CD_TRACK_MODE2_FORM_MIX:
								outtoc.tracks[trknum].pgdatasize = 2336;
								break;

							case CD_TRACK_MODE2_FORM2:
								outtoc.tracks[trknum].pgdatasize = 2324;
								break;
						}
					}
					else    // pregap sectors not in file, but we're always using idx0ofs for track length calc now
					{
						outinfo.track[trknum].idx0offs = frames;
					}
				}
			}
			else if (!strcmp(token, "PREGAP"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				outtoc.tracks[trknum].pregap = frames;
			}
			else if (!strcmp(token, "POSTGAP"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				outtoc.tracks[trknum].postgap = frames;
			}
		}
	}

	/* close the input CUE */
	fclose(infile);

	/* store the number of tracks found */
	outtoc.numtrks = trknum + 1;

	/* now go over the files again and set the lengths */
	for (trknum = 0; trknum < outtoc.numtrks; trknum++)
	{
		UINT64 tlen = 0;

		// this is true for cue/bin and cue/iso, and we need it for cue/wav since .WAV is little-endian
		if (outtoc.tracks[trknum].trktype == CD_TRACK_AUDIO)
		{
			outinfo.track[trknum].swap = true;
		}

		// don't do this for .WAV tracks, we already have their length and offset filled out
		if (outinfo.track[trknum].offset == 0)
		{
			// is this the last track?
			if (trknum == (outtoc.numtrks-1))
			{
				/* if we have the same filename as the last track, do it that way */
				if (trknum != 0 && (outinfo.track[trknum].fname.compare(outinfo.track[trknum-1].fname)==0))
				{
					tlen = get_file_size(outinfo.track[trknum].fname.c_str());
					if (tlen == 0)
					{
						printf("ERROR: couldn't find bin file [%s]\n", outinfo.track[trknum-1].fname.c_str());
						return CHDERR_FILE_NOT_FOUND;
					}
					outinfo.track[trknum].offset = outinfo.track[trknum-1].offset + outtoc.tracks[trknum-1].frames * (outtoc.tracks[trknum-1].datasize + outtoc.tracks[trknum-1].subsize);
					outtoc.tracks[trknum].frames = (tlen - outinfo.track[trknum].offset) / (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
				}
				else    /* data files are different */
				{
					tlen = get_file_size(outinfo.track[trknum].fname.c_str());
					if (tlen == 0)
					{
						printf("ERROR: couldn't find bin file [%s]\n", outinfo.track[trknum-1].fname.c_str());
						return CHDERR_FILE_NOT_FOUND;
					}
					tlen /= (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
					outtoc.tracks[trknum].frames = tlen;
					outinfo.track[trknum].offset = 0;
				}
			}
			else
			{
				/* if we have the same filename as the next track, do it that way */
				if (outinfo.track[trknum].fname.compare(outinfo.track[trknum+1].fname)==0)
				{
					outtoc.tracks[trknum].frames = outinfo.track[trknum+1].idx0offs - outinfo.track[trknum].idx0offs;

					if (trknum == 0)    // track 0 offset is 0
					{
						outinfo.track[trknum].offset = 0;
					}
					else
					{
						outinfo.track[trknum].offset = outinfo.track[trknum-1].offset + outtoc.tracks[trknum-1].frames * (outtoc.tracks[trknum-1].datasize + outtoc.tracks[trknum-1].subsize);
					}

					if (!outtoc.tracks[trknum].frames)
					{
						printf("ERROR: unable to determine size of track %d, missing INDEX 01 markers?\n", trknum+1);
						return CHDERR_INVALID_DATA;
					}
				}
				else    /* data files are different */
				{
					tlen = get_file_size(outinfo.track[trknum].fname.c_str());
					if (tlen == 0)
					{
						printf("ERROR: couldn't find bin file [%s]\n", outinfo.track[trknum].fname.c_str());
						return CHDERR_FILE_NOT_FOUND;
					}
					tlen /= (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
					outtoc.tracks[trknum].frames = tlen;
					outinfo.track[trknum].offset = 0;
				}
			}
		}
		//printf("trk %d: %d frames @ offset %d\n", trknum+1, outtoc.tracks[trknum].frames, outinfo.track[trknum].offset);
	}

	return CHDERR_NONE;
}

/*-------------------------------------------------
    chdcd_parse_toc - parse a CDRDAO format TOC file
-------------------------------------------------*/

/**
 * @fn  chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
 *
 * @brief   Chdcd parse TOC.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A chd_error.
 */

chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo)
{
	FILE *infile;
	int i, trknum;
	static char token[512];
	char tocftemp[512];

	strcpy(tocftemp, tocfname);
	for (i = 0; i < strlen(tocfname); i++)
	{
		tocftemp[i] = tolower(tocftemp[i]);
	}

	if (strstr(tocftemp,".gdi"))
	{
		return chdcd_parse_gdi(tocfname, outtoc, outinfo);
	}

	if (strstr(tocftemp,".cue"))
	{
		return chdcd_parse_cue(tocfname, outtoc, outinfo);
	}

	if (strstr(tocftemp,".nrg"))
	{
		return chdcd_parse_nero(tocfname, outtoc, outinfo);
	}

	if (strstr(tocftemp,".iso") || strstr(tocftemp,".cdr"))
	{
		return chdcd_parse_iso(tocfname, outtoc, outinfo);
	}

	std::string path = std::string(tocfname);

	infile = fopen(tocfname, "rt");
	path = get_file_path(path);

	if (infile == (FILE *)nullptr)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	trknum = -1;

	while (!feof(infile))
	{
		/* get the next line */
		fgets(linebuffer, 511, infile);

		/* if EOF didn't hit, keep going */
		if (!feof(infile))
		{
			i = 0;

			TOKENIZE

			if ((!strcmp(token, "DATAFILE")) || (!strcmp(token, "AUDIOFILE")) || (!strcmp(token, "FILE")))
			{
				int f;

				/* found the data file for a track */
				TOKENIZE

				/* keep the filename */
				outinfo.track[trknum].fname.assign(path).append(token);

				/* get either the offset or the length */
				TOKENIZE

				if (!strcmp(token, "SWAP"))
				{
					TOKENIZE

					outinfo.track[trknum].swap = true;
				}
				else
				{
					outinfo.track[trknum].swap = false;
				}

				if (token[0] == '#')
				{
					/* it's a decimal offset, use it */
					f = strtoul(&token[1], nullptr, 10);
				}
				else if (isdigit((UINT8)token[0]))
				{
					/* convert the time to an offset */
					f = msf_to_frames( token );

					f *= (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
				}
				else
				{
					f = 0;
				}

				outinfo.track[trknum].offset = f;

				TOKENIZE

				if (isdigit((UINT8)token[0]))
				{
					// this could be the length or an offset from the previous field.
					f = msf_to_frames( token );

					TOKENIZE

					if (isdigit((UINT8)token[0]))
					{
						// it was an offset.
						f *= (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);

						outinfo.track[trknum].offset += f;

						// this is the length.
						f = msf_to_frames( token );
					}
				}
				else if( trknum == 0 && outinfo.track[trknum].offset != 0 )
				{
					/* the 1st track might have a length with no offset */
					f = outinfo.track[trknum].offset / (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
					outinfo.track[trknum].offset = 0;
				}
				else
				{
					/* guesstimate the track length? */
					f = 0;
				}

				outtoc.tracks[trknum].frames = f;
			}
			else if (!strcmp(token, "TRACK"))
			{
				trknum++;

				/* next token on the line is the track type */
				TOKENIZE

				outtoc.tracks[trknum].trktype = CD_TRACK_MODE1;
				outtoc.tracks[trknum].datasize = 0;
				outtoc.tracks[trknum].subtype = CD_SUB_NONE;
				outtoc.tracks[trknum].subsize = 0;
				outtoc.tracks[trknum].padframes = 0;

				cdrom_convert_type_string_to_track_info(token, &outtoc.tracks[trknum]);
				if (outtoc.tracks[trknum].datasize == 0)
				{
					printf("ERROR: Unknown track type [%s].  Contact MAMEDEV.\n", token);
					return CHDERR_UNSUPPORTED_FORMAT;
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				cdrom_convert_subtype_string_to_track_info(token, &outtoc.tracks[trknum]);
			}
			else if (!strcmp(token, "START"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				outtoc.tracks[trknum].pregap = frames;
			}
		}
	}

	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc.numtrks = trknum + 1;

	return CHDERR_NONE;
}
