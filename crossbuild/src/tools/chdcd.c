/***************************************************************************

    CDRDAO TOC parser for CHD compression frontend

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "osdcore.h"
#include "chd.h"
#include "chdcd.h"
#include <ctype.h>



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

	while ((i < linebuffersize) && isspace(linebuffer[i]))
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
		else if (!singlequote && !doublequote && isspace(linebuffer[i]))
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
    chdcd_parse_toc - parse a CDRDAO format TOC file
-------------------------------------------------*/

chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc *outtoc, chdcd_track_input_info *outinfo)
{
	FILE *infile;
	int i, trknum;
	static char token[128];

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
				else if (isdigit(token[0]))
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

				if (isdigit(token[0]))
				{
					// this could be the length or an offset from the previous field.
					f = msf_to_frames( token );

					TOKENIZE

					if (isdigit(token[0]))
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
					/* guesstimate the track length */
					UINT64 tlen;
					printf("Warning: Estimating length of track %d.  If this is not the final or only track\n on the disc, the estimate may be wrong.\n", trknum+1);

					tlen = get_file_size(outinfo->fname[trknum]) - outinfo->offset[trknum];

					tlen /= (outtoc->tracks[trknum].datasize + outtoc->tracks[trknum].subsize);

					f = tlen;
				}

				outtoc->tracks[trknum].frames = f;
			}
			else if (!strcmp(token, "TRACK"))
			{
				/* found a new track */
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
				}
				else if (outtoc->tracks[trknum].trktype != CD_TRACK_MODE1_RAW &&
					outtoc->tracks[trknum].trktype != CD_TRACK_MODE2_RAW &&
					outtoc->tracks[trknum].trktype != CD_TRACK_AUDIO)
				{
					printf("Note: MAME now prefers and can accept RAW format images.\n");
					printf("At least one track of this CDRDAO rip is not either RAW or AUDIO.\n");
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				cdrom_convert_subtype_string_to_track_info(token, &outtoc->tracks[trknum]);
			}
		}
	}

	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc->numtrks = trknum + 1;

	return CHDERR_NONE;
}
