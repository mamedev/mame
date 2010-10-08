/***************************************************************************

    TOC parser for CHD compression frontend
    Handles CDRDAO .toc, CDRWIN .cue, and Sega GDROM .gdi

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include "osdcore.h"
#include "chd.h"
#include "chdcd.h"
#include "corefile.h"



/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define TOKENIZE i = tokenize( linebuffer, i, sizeof(linebuffer), token, sizeof(token) );



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static char linebuffer[512];



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    get_file_size - get the size of a file
-------------------------------------------------*/

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
static UINT32 parse_wav_sample(char *filename, UINT32 *dataoffs)
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
		osd_close(file);
		return 0;
	}

	/* read the core header and make sure it's a WAVE file */
	osd_read(file, buf, 0, 4, &actual);
	offset += actual;
	if (offset < 4)
		return 0;
	if (memcmp(&buf[0], "RIFF", 4) != 0)
		return 0;

	/* get the total size */
	osd_read(file, &filesize, offset, 4, &actual);
	offset += actual;
	if (offset < 8)
		return 0;
	filesize = LITTLE_ENDIANIZE_INT32(filesize);

	/* read the RIFF file type and make sure it's a WAVE file */
	osd_read(file, buf, offset, 4, &actual);
	offset += actual;
	if (offset < 12)
		return 0;
	if (memcmp(&buf[0], "WAVE", 4) != 0)
		return 0;

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
			return 0;
	}

	/* read the format -- make sure it is PCM */
	osd_read(file, &temp16, offset, 2, &actual);
	offset += actual;
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
		return 0;

	/* number of channels -- only mono is supported */
	osd_read(file, &temp16, offset, 2, &actual);
	offset += actual;
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 2)
		return 0;

	/* sample rate */
	osd_read(file, &rate, offset, 4, &actual);
	offset += actual;
	rate = LITTLE_ENDIANIZE_INT32(rate);
	if (rate != 44100)
		return 0;

	/* bytes/second and block alignment are ignored */
	osd_read(file, buf, offset, 6, &actual);
	offset += actual;

	/* bits/sample */
	osd_read(file, &bits, offset, 2, &actual);
	offset += actual;
	if (bits != 16)
		return 0;

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
			return 0;
	}

	/* if there was a 0 length data block, we're done */
	if (length == 0)
		return 0;

	osd_close(file);

	*dataoffs = offset;

	return length;
}

/*-------------------------------------------------
    chdcd_parse_gdi - parse a Sega GD-ROM rip
-------------------------------------------------*/

static chd_error chdcd_parse_gdi(const char *tocfname, cdrom_toc *outtoc, chdcd_track_input_info *outinfo)
{
	FILE *infile;
	int i, numtracks;
	//int chdpos=0;

	infile = fopen(tocfname, "rt");

	if (infile == (FILE *)NULL)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(outtoc, 0, sizeof(cdrom_toc));
	memset(outinfo, 0, sizeof(chdcd_track_input_info));


	fgets(linebuffer,511,infile);
	numtracks=atoi(linebuffer);

	for(i=0;i<numtracks;++i)
	{
		char *tok;
		int trknum;
		int trksize,trktype;
		int sz;
		int hunks;


		fgets(linebuffer,511,infile);

		tok=strtok(linebuffer," ");

		trknum=atoi(tok)-1;

		outinfo->swap[trknum]=0;
		outinfo->offset[trknum]=0;

		//outtoc->tracks[trknum].trktype = CD_TRACK_MODE1;
		outtoc->tracks[trknum].datasize = 0;
		outtoc->tracks[trknum].subtype = CD_SUB_NONE;
		outtoc->tracks[trknum].subsize = 0;

		tok=strtok(NULL," ");
		outtoc->tracks[trknum].physframeofs=atoi(tok);

		tok=strtok(NULL," ");
		trktype=atoi(tok);

		tok=strtok(NULL," ");
		trksize=atoi(tok);

		if(trktype==4 && trksize==2352)
		{
			outtoc->tracks[trknum].trktype=CD_TRACK_MODE1_RAW;
			outtoc->tracks[trknum].datasize=2352;
		}
		if(trktype==4 && trksize==2048)
		{
			outtoc->tracks[trknum].trktype=CD_TRACK_MODE1;
			outtoc->tracks[trknum].datasize=2048;
		}
		if(trktype==0)
		{
			//assert(trksize==2352);
			outtoc->tracks[trknum].trktype=CD_TRACK_AUDIO;
			outtoc->tracks[trknum].datasize=2352;
		}

		tok=strtok(NULL," ");
		strcpy(&(outinfo->fname[trknum][0]),tok);
		sz=get_file_size(outinfo->fname[trknum]);

		outtoc->tracks[trknum].frames=sz/trksize;
		outtoc->tracks[trknum].extraframes=0;

		if(trknum!=0)
		{
			int dif=outtoc->tracks[trknum].physframeofs-(outtoc->tracks[trknum-1].frames+outtoc->tracks[trknum-1].physframeofs);
			outtoc->tracks[trknum-1].frames+=dif;
		}

/*
        if(trknum!=0)
        {
            outtoc->tracks[trknum-1].extraframes=outtoc->tracks[trknum].physframeofs-(outtoc->tracks[trknum-1].frames+outtoc->tracks[trknum-1].physframeofs);
        }
*/
		hunks = (outtoc->tracks[trknum].frames+CD_FRAMES_PER_HUNK - 1) / CD_FRAMES_PER_HUNK;
		outtoc->tracks[trknum].extraframes = hunks * CD_FRAMES_PER_HUNK - outtoc->tracks[trknum].frames;

		//chdpos+=outtoc->tracks[trknum].frames+outtoc->tracks[trknum].extraframes;

	}
	/*
    for(i=0;i<numtracks;++i)
    {
        printf("%s %d %d %d\n",outinfo->fname[i],outtoc->tracks[i].frames,outtoc->tracks[i].extraframes,outtoc->tracks[i].physframeofs);
    }
    */
	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc->numtrks = numtracks;

	return CHDERR_NONE;
}

/*-------------------------------------------------
    chdcd_parse_toc - parse a CDRWin format CUE file
-------------------------------------------------*/

chd_error chdcd_parse_cue(const char *tocfname, cdrom_toc *outtoc, chdcd_track_input_info *outinfo)
{
	FILE *infile;
	int i, trknum;
	static char token[128];
	static char lastfname[128];
	UINT32 wavlen, wavoffs;

	infile = fopen(tocfname, "rt");

	if (infile == (FILE *)NULL)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(outtoc, 0, sizeof(cdrom_toc));
	memset(outinfo, 0, sizeof(chdcd_track_input_info));

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
				strncpy(lastfname, token, 128);

				/* get the file type */
				TOKENIZE

				if (!strcmp(token, "BINARY"))
				{
					outinfo->swap[trknum] = 0;
				}
				else if (!strcmp(token, "MOTOROLA"))
				{
					outinfo->swap[trknum] = 1;
				}
				else if (!strcmp(token, "WAVE"))
				{
					wavlen = parse_wav_sample(lastfname, &wavoffs);
					if (!wavlen)
					{
						file_error err;
						core_file *fhand;

						err = core_fopen(lastfname, OPEN_FLAG_READ, &fhand);
						if (err != FILERR_NONE) printf("holy moley!\n");
						else core_fclose(fhand);

						printf("ERROR: couldn't read [%s] or not a valid .WAV\n", lastfname);
						return CHDERR_FILE_NOT_FOUND;
					}
				}
				else
				{
					printf("ERROR: Unhandled track type %s\n", token);
					return CHDERR_FILE_NOT_FOUND;
				}
			}
			else if (!strcmp(token, "TRACK"))
			{
				/* get the track number */
				TOKENIZE
				trknum = strtoul(token, NULL, 10) - 1;

				/* next token on the line is the track type */
				TOKENIZE

				if (wavlen != 0)
				{
					outtoc->tracks[trknum].trktype = CD_TRACK_AUDIO;
					outtoc->tracks[trknum].frames = wavlen/2352;
					outinfo->offset[trknum] = wavoffs;
					wavoffs = wavlen = 0;
				}
				else
				{
					outtoc->tracks[trknum].trktype = CD_TRACK_MODE1;
					outtoc->tracks[trknum].datasize = 0;
					outinfo->offset[trknum] = 0;
				}
				outtoc->tracks[trknum].subtype = CD_SUB_NONE;
				outtoc->tracks[trknum].subsize = 0;
				outtoc->tracks[trknum].pregap = 0;
				outinfo->idx0offs[trknum] = -1;
				outinfo->idx1offs[trknum] = 0;
				strcpy(&outinfo->fname[trknum][0], lastfname);	// default filename to the last one
//              printf("trk %d: fname %s offset %d\n", trknum, &outinfo->fname[trknum][0], outinfo->offset[trknum]);

				cdrom_convert_type_string_to_track_info(token, &outtoc->tracks[trknum]);
				if (outtoc->tracks[trknum].datasize == 0)
				{
					printf("ERROR: Unknown track type [%s].  Contact MAMEDEV.\n", token);
					return CHDERR_FILE_NOT_FOUND;
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				cdrom_convert_subtype_string_to_track_info(token, &outtoc->tracks[trknum]);
			}
			else if (!strcmp(token, "INDEX"))	/* only in bin/cue files */
			{
				int idx, frames;

				/* get index number */
				TOKENIZE
				idx = strtoul(token, NULL, 10);

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				if (idx == 0)
				{
					outinfo->idx0offs[trknum] = frames;
				}
				else if (idx == 1)
				{
					outinfo->idx1offs[trknum] = frames;
					if ((outtoc->tracks[trknum].pregap == 0) && (outinfo->idx0offs[trknum] != -1))
					{
						outtoc->tracks[trknum].pregap = frames - outinfo->idx0offs[trknum];
					}
				}
			}
			else if (!strcmp(token, "PREGAP"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				outtoc->tracks[trknum].pregap = frames;
			}
			else if (!strcmp(token, "POSTGAP"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				outtoc->tracks[trknum].postgap = frames;
			}
		}
	}

	/* close the input CUE */
	fclose(infile);

	/* store the number of tracks found */
	outtoc->numtrks = trknum + 1;

	/* now go over the files again and set the lengths */
	for (trknum = 0; trknum < outtoc->numtrks; trknum++)
	{
		UINT64 tlen = 0;

		// this is true for cue/bin and cue/iso, and we need it for cue/wav since .WAV is little-endian
		if (outtoc->tracks[trknum].trktype == CD_TRACK_AUDIO)
		{
			outinfo->swap[trknum] = 1;
		}

		// don't do this for .WAV tracks, we already have their length and offset filled out
		if (outinfo->offset[trknum] == 0)
		{
			// is this the last track?
			if (trknum == (outtoc->numtrks-1))
			{
				/* if we have the same filename as the last track, do it that way */
				if (!strcmp(&outinfo->fname[trknum][0], &outinfo->fname[trknum-1][0]))
				{
					tlen = get_file_size(outinfo->fname[trknum]);
					if (tlen == 0)
					{
						printf("ERROR: couldn't find bin file [%s]\n", outinfo->fname[trknum-1]);
						return CHDERR_FILE_NOT_FOUND;
					}
					outinfo->offset[trknum] = outinfo->offset[trknum-1] + outtoc->tracks[trknum-1].frames * (outtoc->tracks[trknum-1].datasize + outtoc->tracks[trknum-1].subsize);
					outtoc->tracks[trknum].frames = (tlen - outinfo->offset[trknum]) / (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);
				}
				else	/* data files are different */
				{
					tlen = get_file_size(outinfo->fname[trknum]);
					if (tlen == 0)
					{
						printf("ERROR: couldn't find bin file [%s]\n", outinfo->fname[trknum-1]);
						return CHDERR_FILE_NOT_FOUND;
					}
					tlen /= (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);
					outtoc->tracks[trknum].frames = tlen;
					outinfo->offset[trknum] = 0;
				}
			}
			else
			{
				/* if we have the same filename as the next track, do it that way */
				if (!strcmp(&outinfo->fname[trknum][0], &outinfo->fname[trknum+1][0]))
				{
					outtoc->tracks[trknum].frames = outinfo->idx1offs[trknum+1] - outinfo->idx1offs[trknum];

					if (trknum == 0)	// track 0 offset is 0
					{
						outinfo->offset[trknum] = 0;
					}
					else
					{
						outinfo->offset[trknum] = outinfo->offset[trknum-1] + outtoc->tracks[trknum-1].frames * (outtoc->tracks[trknum-1].datasize + outtoc->tracks[trknum-1].subsize);
					}

					if (!outtoc->tracks[trknum].frames)
					{
						printf("ERROR: unable to determine size of track %d, missing INDEX 01 markers?\n", trknum+1);
						return CHDERR_FILE_NOT_FOUND;
					}
				}
				else	/* data files are different */
				{
					tlen = get_file_size(outinfo->fname[trknum]);
					if (tlen == 0)
					{
						printf("ERROR: couldn't find bin file [%s]\n", outinfo->fname[trknum]);
						return CHDERR_FILE_NOT_FOUND;
					}
					tlen /= (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);
					outtoc->tracks[trknum].frames = tlen;
					outinfo->offset[trknum] = 0;
				}
			}
		}
		printf("trk %d: %d frames @ offset %d\n", trknum+1, outtoc->tracks[trknum].frames, outinfo->offset[trknum]);
	}

	return CHDERR_NONE;
}

/*-------------------------------------------------
    chdcd_parse_toc - parse a CDRDAO format TOC file
-------------------------------------------------*/

chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc *outtoc, chdcd_track_input_info *outinfo)
{
	FILE *infile;
	int i, trknum;
	static char token[128];

	if (strstr(tocfname,".gdi"))
	{
		return chdcd_parse_gdi(tocfname, outtoc, outinfo);
	}

	if (strstr(tocfname,".cue"))
	{
		return chdcd_parse_cue(tocfname, outtoc, outinfo);
	}

	infile = fopen(tocfname, "rt");

	if (infile == (FILE *)NULL)
	{
		return CHDERR_FILE_NOT_FOUND;
	}

	/* clear structures */
	memset(outtoc, 0, sizeof(cdrom_toc));
	memset(outinfo, 0, sizeof(chdcd_track_input_info));

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
				strncpy(&outinfo->fname[trknum][0], token, strlen(token));

				/* get either the offset or the length */
				TOKENIZE

				if (!strcmp(token, "SWAP"))
				{
					TOKENIZE

					outinfo->swap[trknum] = 1;
				}
				else
				{
					outinfo->swap[trknum] = 0;
				}

				if (token[0] == '#')
				{
					/* it's a decimal offset, use it */
					f = strtoul(&token[1], NULL, 10);
				}
				else if (isdigit((UINT8)token[0]))
				{
					/* convert the time to an offset */
					f = msf_to_frames( token );

					f *= (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);
				}
				else
				{
					f = 0;
				}

				outinfo->offset[trknum] = f;

				TOKENIZE

				if (isdigit((UINT8)token[0]))
				{
					// this could be the length or an offset from the previous field.
					f = msf_to_frames( token );

					TOKENIZE

					if (isdigit((UINT8)token[0]))
					{
						// it was an offset.
						f *= (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);

						outinfo->offset[trknum] += f;

						// this is the length.
						f = msf_to_frames( token );
					}
				}
				else if( trknum == 0 && outinfo->offset[trknum] != 0 )
				{
					/* the 1st track might have a length with no offset */
					f = outinfo->offset[trknum] / (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);
					outinfo->offset[trknum] = 0;
				}
				else
				{
					/* guesstimate the track length? */
					f = 0;
				}

				outtoc->tracks[trknum].frames = f;
			}
			else if (!strcmp(token, "TRACK"))
			{
				trknum++;

				/* next token on the line is the track type */
				TOKENIZE

				outtoc->tracks[trknum].trktype = CD_TRACK_MODE1;
				outtoc->tracks[trknum].datasize = 0;
				outtoc->tracks[trknum].subtype = CD_SUB_NONE;
				outtoc->tracks[trknum].subsize = 0;

				cdrom_convert_type_string_to_track_info(token, &outtoc->tracks[trknum]);
				if (outtoc->tracks[trknum].datasize == 0)
				{
					printf("ERROR: Unknown track type [%s].  Contact MAMEDEV.\n", token);
					return CHDERR_FILE_NOT_FOUND;
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				cdrom_convert_subtype_string_to_track_info(token, &outtoc->tracks[trknum]);
			}
			else if (!strcmp(token, "START"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames( token );

				outtoc->tracks[trknum].pregap = frames;
			}
		}
	}

	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc->numtrks = trknum + 1;

	return CHDERR_NONE;
}
