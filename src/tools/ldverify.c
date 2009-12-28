/***************************************************************************

    ldverify.c

    Laserdisc AVI/CHD verifier.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "aviio.h"
#include "bitmap.h"
#include "chd.h"
#include "vbiparse.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define REPORT_BLANKS_THRESHOLD		50



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _movie_info movie_info;
struct _movie_info
{
	double	framerate;
	int		numframes;
	int		width;
	int		height;
	int		samplerate;
	int		channels;
};

typedef struct _video_info video_info;
struct _video_info
{
	int first_whitefield;
	int saw_leadin;
	int saw_leadout;
	int last_frame;
	int last_chapter;
	int cadence;
	UINT32 cadence_history;
	int prev_whitefield;
	int min_overall;
	int max_overall;
	int first_blank_frame;
	int first_blank_field;
	int num_blank_fields;
	int first_low_frame;
	int first_low_field;
	int num_low_fields;
	int first_high_frame;
	int first_high_field;
	int num_high_fields;
};

typedef struct _audio_info audio_info;
struct _audio_info
{
	int min_lsample;
	int min_rsample;
	int max_lsample;
	int max_rsample;
	int min_lsample_count;
	int min_rsample_count;
	int max_lsample_count;
	int max_rsample_count;
	int sample_count;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 chdinterlaced;



/***************************************************************************
    AVI HANDLING
***************************************************************************/

/*-------------------------------------------------
    open_avi - open an AVI file and return
    information about it
-------------------------------------------------*/

static void *open_avi(const char *filename, movie_info *info)
{
	const avi_movie_info *aviinfo;
	avi_error avierr;
	avi_file *avi;

	/* open the file */
	avierr = avi_open(filename, &avi);
	if (avierr != AVIERR_NONE)
	{
		fprintf(stderr, "Error opening AVI file: %s\n", avi_error_string(avierr));
		return NULL;
	}

	/* extract movie info */
	aviinfo = avi_get_movie_info(avi);
	info->framerate = (double)aviinfo->video_timescale / (double)aviinfo->video_sampletime;
	info->numframes = aviinfo->video_numsamples;
	info->width = aviinfo->video_width;
	info->height = aviinfo->video_height;
	info->samplerate = aviinfo->audio_samplerate;
	info->channels = aviinfo->audio_channels;

	return avi;
}


/*-------------------------------------------------
    read_avi - read a frame from an AVI file
-------------------------------------------------*/

static int read_avi(void *file, int frame, bitmap_t *bitmap, INT16 *lsound, INT16 *rsound, int *samples)
{
	const avi_movie_info *aviinfo = avi_get_movie_info((avi_file *)file);
	UINT32 firstsample = ((UINT64)aviinfo->audio_samplerate * (UINT64)frame * (UINT64)aviinfo->video_sampletime + aviinfo->video_timescale - 1) / (UINT64)aviinfo->video_timescale;
	UINT32 lastsample = ((UINT64)aviinfo->audio_samplerate * (UINT64)(frame + 1) * (UINT64)aviinfo->video_sampletime + aviinfo->video_timescale - 1) / (UINT64)aviinfo->video_timescale;
	avi_error avierr;

	/* read the frame */
	avierr = avi_read_video_frame_yuy16((avi_file *)file, frame, bitmap);
	if (avierr != AVIERR_NONE)
		return FALSE;

	/* read the samples */
	avierr = avi_read_sound_samples((avi_file *)file, 0, firstsample, lastsample - firstsample, lsound);
	avierr = avi_read_sound_samples((avi_file *)file, 1, firstsample, lastsample - firstsample, rsound);
	if (avierr != AVIERR_NONE)
		return FALSE;
	*samples = lastsample - firstsample;
	return TRUE;
}


/*-------------------------------------------------
    close_avi - close an AVI file
-------------------------------------------------*/

static void close_avi(void *file)
{
	avi_close((avi_file *)file);
}



/***************************************************************************
    CHD HANDLING
***************************************************************************/

/*-------------------------------------------------
    open_chd - open a CHD file and return
    information about it
-------------------------------------------------*/

static void *open_chd(const char *filename, movie_info *info)
{
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	char metadata[256];
	chd_error chderr;
	chd_file *chd;

	/* open the file */
	chderr = chd_open(filename, CHD_OPEN_READ, NULL, &chd);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file: %s\n", chd_error_string(chderr));
		return NULL;
	}

	/* get the metadata */
	chderr = chd_get_metadata(chd, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL, NULL);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting A/V metadata: %s\n", chd_error_string(chderr));
		chd_close(chd);
		return NULL;
	}

	/* extract the info */
	if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
	{
		fprintf(stderr, "Improperly formatted metadata\n");
		chd_close(chd);
		return NULL;
	}

	/* extract movie info */
	info->framerate = (fps * 1000000 + fpsfrac) / 1000000.0;
	info->numframes = chd_get_header(chd)->totalhunks;
	info->width = width;
	info->height = height;
	info->samplerate = rate;
	info->channels = channels;

	/* convert to an interlaced frame */
	chdinterlaced = interlaced;
	if (interlaced)
	{
		info->framerate /= 2;
		info->numframes = (info->numframes + 1) / 2;
		info->height *= 2;
	}

	return chd;
}


/*-------------------------------------------------
    read_chd - read a frame from a CHD file
-------------------------------------------------*/

static int read_chd(void *file, int frame, bitmap_t *bitmap, INT16 *lsound, INT16 *rsound, int *samples)
{
	av_codec_decompress_config avconfig = { 0 };
	int interlace_factor = chdinterlaced ? 2 : 1;
	bitmap_t fakebitmap;
	UINT32 numsamples;
	chd_error chderr;
	int fieldnum;

	/* loop over fields */
	*samples = 0;
	for (fieldnum = 0; fieldnum < interlace_factor; fieldnum++)
	{
		/* make a fake bitmap for this field */
		fakebitmap = *bitmap;
		fakebitmap.base = BITMAP_ADDR16(&fakebitmap, fieldnum, 0);
		fakebitmap.rowpixels *= interlace_factor;
		fakebitmap.height /= interlace_factor;

		/* configure the codec */
		avconfig.video = &fakebitmap;
		avconfig.maxsamples = 48000;
		avconfig.actsamples = &numsamples;
		avconfig.audio[0] = &lsound[*samples];
		avconfig.audio[1] = &rsound[*samples];

		/* configure the decompressor for this frame */
		chd_codec_config((chd_file *)file, AV_CODEC_DECOMPRESS_CONFIG, &avconfig);

		/* read the frame */
		chderr = chd_read((chd_file *)file, frame * interlace_factor + fieldnum, NULL);
		if (chderr != CHDERR_NONE)
			return FALSE;

		/* account for samples read */
		*samples += numsamples;
	}
	return TRUE;
}


/*-------------------------------------------------
    close_chd - close a CHD file
-------------------------------------------------*/

static void close_chd(void *file)
{
	chd_close((chd_file *)file);
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    init_video - init video info structure
-------------------------------------------------*/

static void init_video(video_info *video)
{
	video->first_whitefield = -1;
	video->saw_leadin = FALSE;
	video->saw_leadout = FALSE;
	video->last_frame = -1;
	video->last_chapter = -1;
	video->cadence = -1;
	video->cadence_history = 0;
	video->prev_whitefield = -1;
	video->min_overall = 255;
	video->max_overall = 0;
	video->first_blank_frame = -1;
	video->first_blank_field = -1;
	video->num_blank_fields = -1;
	video->first_low_frame = -1;
	video->first_low_field = -1;
	video->num_low_fields = -1;
	video->first_high_frame = -1;
	video->first_high_field = -1;
	video->num_high_fields = -1;
}


/*-------------------------------------------------
    verify_video - verify video frame
-------------------------------------------------*/

static void verify_video(video_info *video, int frame, bitmap_t *bitmap)
{
	const int fields_per_frame = 2;
	int fieldnum;

	/* loop over fields */
	for (fieldnum = 0; fieldnum < fields_per_frame; fieldnum++)
	{
		int yminval, ymaxval, cbminval, cbmaxval, crminval, crmaxval;
		int field = frame * fields_per_frame + fieldnum;
		int x, y, pixels, remaining;
		UINT32 yhisto[256] = { 0 };
		UINT32 crhisto[256] = { 0 };
		UINT32 cbhisto[256] = { 0 };
		vbi_metadata metadata;

		/* output status */
		if (frame % 10 == 0 && fieldnum == 0)
			fprintf(stderr, "%6d.%d...\r", frame, fieldnum);

		/* parse the VBI data */
		vbi_parse_all(BITMAP_ADDR16(bitmap, fieldnum, 0), bitmap->rowpixels * 2, bitmap->width, 8, &metadata);

		/* if we have data in both 17 and 18, it should match */
		if (metadata.line17 != 0 && metadata.line18 != 0 && metadata.line17 != metadata.line18)
		{
			printf("%6d.%d: line 17 and 18 data does not match (17=%06X 18=%06X) (WARNING)\n", frame, fieldnum, metadata.line17, metadata.line18);
			printf("%6d.%d: selected %06X based on bit confidence\n", frame, fieldnum, metadata.line1718);
		}

		/* is this a lead-in code? */
		if (metadata.line1718 == VBI_CODE_LEADIN)
		{
			/* if we haven't seen lead-in yet, detect it */
			if (!video->saw_leadin)
			{
				video->saw_leadin = TRUE;
				printf("%6d.%d: lead-in code detected\n", frame, fieldnum);
			}

			/* if we've previously seen chapters/frames, that's weird */
			if (video->last_frame != -1 || video->last_chapter != -1)
				printf("%6d.%d: lead-in code detected after frame/chapter data (WARNING)\n", frame, fieldnum);
		}

		/* is this a lead-out code? */
		if (metadata.line1718 == VBI_CODE_LEADOUT)
		{
			/* if we haven't seen lead-in yet, detect it */
			if (!video->saw_leadout)
			{
				video->saw_leadout = TRUE;
				printf("%6d.%d: lead-out code detected\n", frame, fieldnum);
				if (video->last_frame != -1)
					printf("%6d.%d: final frame number was %d\n", frame, fieldnum, video->last_frame);
				else
					printf("%6d.%d: never detected any frame numbers (ERROR)\n", frame, fieldnum);
			}

			/* if we've previously seen chapters/frames, that's weird */
			if (video->last_frame == -1)
				printf("%6d.%d: lead-out code detected with no frames detected beforehand (WARNING)\n", frame, fieldnum);
		}

		/* is this a frame code? */
		if ((metadata.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
		{
			int framenum = VBI_CAV_PICTURE(metadata.line1718);

			/* did we see any leadin? */
			if (!video->saw_leadin)
			{
				printf("%6d.%d: detected frame number but never saw any lead-in (WARNING)\n", frame, fieldnum);
				video->saw_leadin = TRUE;
			}

			/* if this is the first frame, make sure it's 1 */
			if (video->last_frame == -1)
			{
				if (framenum == 0)
					printf("%6d.%d: detected frame 0\n", frame, fieldnum);
				else if (framenum == 1)
					printf("%6d.%d: detected frame 1\n", frame, fieldnum);
				else
					printf("%6d.%d: first frame number is not 0 or 1 (%d) (ERROR)\n", frame, fieldnum, framenum);
			}

			/* print an update every 10000 frames */
			if (framenum != 0 && framenum % 10000 == 0)
				printf("%6d.%d: detected frame %d\n", frame, fieldnum, framenum);

			/* if this frame is not consecutive, it's an error */
			if (video->last_frame != -1 && framenum != video->last_frame + 1)
				printf("%6d.%d: gap in frame number sequence (%d->%d) (ERROR)\n", frame, fieldnum, video->last_frame, framenum);

			/* remember the frame number */
			video->last_frame = framenum;

			/* if we've seen a white flag before, but it's not here, warn */
			if (video->first_whitefield != -1 && !metadata.white)
				printf("%6d.%d: detected frame number but no white flag (WARNING)\n", frame, fieldnum);
		}

		/* is the whiteflag set? */
		if (metadata.white)
		{
			/* if this is the first white flag we see, count it */
			if (video->first_whitefield == -1)
			{
				video->first_whitefield = field;
				printf("%6d.%d: first white flag seen\n", frame, fieldnum);
			}

			/* if we've seen frame numbers before, but not here, warn */
			if (video->last_frame != -1 && (metadata.line1718 & VBI_MASK_CAV_PICTURE) != VBI_CODE_CAV_PICTURE)
				printf("%6d.%d: detected white flag but no frame number (WARNING)\n", frame, fieldnum);
		}

		/* if this is the start of a frame, handle cadence */
		if (metadata.white || (metadata.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
		{
			/* if we've seen frames, but we're not yet to the lead-out, check the cadence */
			if (video->last_frame != -1 && !video->saw_leadout)
			{
				/* make sure we have a proper history */
				if (video->prev_whitefield != -1)
					video->cadence_history = (video->cadence_history << 4) | ((field - video->prev_whitefield) & 0x0f);
				video->prev_whitefield = field;

				/* if we don't know our cadence yet, determine it */
				if (video->cadence == -1 && (video->cadence_history & 0xf00) != 0)
				{
					if ((video->cadence_history & 0xfff) == 0x222)
					{
						printf("%6d.%d: detected 2:2 cadence\n", frame, fieldnum);
						video->cadence = 4;
					}
					else if ((video->cadence_history & 0xfff) == 0x323)
					{
						printf("%6d.%d: detected 3:2 cadence\n", frame, fieldnum);
						video->cadence = 5;
					}
					else if ((video->cadence_history & 0xfff) == 0x232)
					{
						printf("%6d.%d: detected 2:3 cadence\n", frame, fieldnum);
						video->cadence = 5;
					}
					else
					{
						printf("%6d.%d: unknown cadence (history %d:%d:%d) (WARNING)\n", frame, fieldnum,
								(video->cadence_history >> 8) & 15, (video->cadence_history >> 4) & 15, video->cadence_history & 15);
					}
				}

				/* if we know our cadence, make sure we stick to it */
				if (video->cadence != -1)
				{
					if (video->cadence == 4 && (video->cadence_history & 0xfff) != 0x222)
					{
						printf("%6d.%d: missed cadence (history %d:%d:%d) (WARNING)\n", frame, fieldnum,
								(video->cadence_history >> 8) & 15, (video->cadence_history >> 4) & 15, video->cadence_history & 15);
						video->cadence = -1;
						video->cadence_history = 0;
					}
					else if (video->cadence == 5 && (video->cadence_history & 0xfff) != 0x323 && (video->cadence_history & 0xfff) != 0x232)
					{
						printf("%6d.%d: missed cadence (history %d:%d:%d) (WARNING)\n", frame, fieldnum,
								(video->cadence_history >> 8) & 15, (video->cadence_history >> 4) & 15, video->cadence_history & 15);
						video->cadence = -1;
						video->cadence_history = 0;
					}
				}
			}
		}

		/* now examine the active video signal */
		pixels = 0;
		for (y = 22*2 + fieldnum; y < bitmap->height; y += 2)
		{
			for (x = 16; x < 720 - 16; x++)
			{
				yhisto[*BITMAP_ADDR16(bitmap, y, x) >> 8]++;
				if (x % 2 == 0)
					cbhisto[*BITMAP_ADDR16(bitmap, y, x) & 0xff]++;
				else
					crhisto[*BITMAP_ADDR16(bitmap, y, x) & 0xff]++;
			}
			pixels += 720 - 16 - 16;
		}

		/* remove the top/bottom 0.1% of Y */
		remaining = pixels / 1000;
		for (yminval = 0; (remaining -= yhisto[yminval]) >= 0; yminval++) ;
		remaining = pixels / 1000;
		for (ymaxval = 255; (remaining -= yhisto[ymaxval]) >= 0; ymaxval--) ;

		/* remove the top/bottom 0.1% of Cb */
		remaining = pixels / 500;
		for (cbminval = 0; (remaining -= cbhisto[cbminval]) >= 0; cbminval++) ;
		remaining = pixels / 500;
		for (cbmaxval = 255; (remaining -= cbhisto[cbmaxval]) >= 0; cbmaxval--) ;

		/* remove the top/bottom 0.1% of Cr */
		remaining = pixels / 500;
		for (crminval = 0; (remaining -= crhisto[crminval]) >= 0; crminval++) ;
		remaining = pixels / 500;
		for (crmaxval = 255; (remaining -= crhisto[crmaxval]) >= 0; crmaxval--) ;

		/* track blank frames */
		if (ymaxval - yminval < 10 && cbmaxval - cbminval < 10 && crmaxval - cbmaxval < 10)
		{
			if (video->first_blank_frame == -1)
			{
				video->first_blank_frame = frame;
				video->first_blank_field = fieldnum;
				video->num_blank_fields = 0;
			}
			video->num_blank_fields++;
		}
		else if (video->num_blank_fields > 0)
		{
			if (video->num_blank_fields >= REPORT_BLANKS_THRESHOLD)
				printf("%6d.%d-%6d.%d: blank frames for %d fields (INFO)\n", video->first_blank_frame, video->first_blank_field, frame, fieldnum, video->num_blank_fields);
			video->first_blank_frame = video->first_blank_field = video->num_blank_fields = -1;
		}

		/* update the overall min/max */
		video->min_overall = MIN(yminval, video->min_overall);
		video->max_overall = MAX(ymaxval, video->max_overall);

		/* track low fields */
		if (yminval <= 0)
		{
			if (video->first_low_frame == -1)
			{
				video->first_low_frame = frame;
				video->first_low_field = fieldnum;
				video->num_low_fields = 0;
			}
			video->num_low_fields++;
		}
		else if (video->num_low_fields > 0)
		{
			printf("%6d.%d-%6d.%d: active video signal level low for %d fields (WARNING)\n", video->first_low_frame, video->first_low_field, frame, fieldnum, video->num_low_fields);
			video->first_low_frame = video->first_low_field = video->num_low_fields = -1;
		}

		/* track high fields */
		if (ymaxval >= 255)
		{
			if (video->first_high_frame == -1)
			{
				video->first_high_frame = frame;
				video->first_high_field = fieldnum;
				video->num_high_fields = 0;
			}
			video->num_high_fields++;
		}
		else if (video->num_high_fields > 0)
		{
			printf("%6d.%d-%6d.%d: active video signal level high for %d fields (WARNING)\n", video->first_high_frame, video->first_high_field, frame, fieldnum, video->num_high_fields);
			video->first_high_frame = video->first_high_field = video->num_high_fields = -1;
		}
	}
}


/*-------------------------------------------------
    verify_video_final - final verification
-------------------------------------------------*/

static void verify_video_final(video_info *video, int frame, bitmap_t *bitmap)
{
	int fields_per_frame = (bitmap->height >= 288) ? 2 : 1;
	int field = frame * fields_per_frame;

	/* did we ever see any white flags? */
	if (video->first_whitefield == -1)
		printf("Track %6d.%d: never saw any white flags (WARNING)\n", field / fields_per_frame, 0);

	/* did we ever see any lead-out? */
	if (!video->saw_leadout)
		printf("Track %6d.%d: never saw any lead-out (WARNING)\n", field / fields_per_frame, 0);

	/* any remaining high/low reports? */
	if (video->num_blank_fields >= REPORT_BLANKS_THRESHOLD)
		printf("%6d.%d-%6d.%d: blank frames for %d fields (INFO)\n", video->first_blank_frame, video->first_blank_field, frame, 0, video->num_blank_fields);
	if (video->num_low_fields > 0)
		printf("%6d.%d-%6d.%d: active video signal level low for %d fields (WARNING)\n", video->first_low_frame, video->first_low_field, frame, 0, video->num_low_fields);
	if (video->num_high_fields > 0)
		printf("%6d.%d-%6d.%d: active video signal level high for %d fields (WARNING)\n", video->first_high_frame, video->first_high_field, frame, 0, video->num_high_fields);

	/* summary info */
	printf("\nVideo summary:\n");
	printf("  Overall video range: %d-%d (%02X-%02X)\n", video->min_overall, video->max_overall, video->min_overall, video->max_overall);
}


/*-------------------------------------------------
    init_audio - init audio info structure
-------------------------------------------------*/

static void init_audio(audio_info *audio)
{
	audio->min_lsample = 32767;
	audio->min_rsample = 32767;
	audio->max_lsample = -32768;
	audio->max_rsample = -32768;
	audio->min_lsample_count = 0;
	audio->min_rsample_count = 0;
	audio->max_lsample_count = 0;
	audio->max_rsample_count = 0;
	audio->sample_count = 0;
}


/*-------------------------------------------------
    verify_audio - verify audio data
-------------------------------------------------*/

static void verify_audio(audio_info *audio, const INT16 *lsound, const INT16 *rsound, int samples)
{
	int sampnum;

	/* count the overall samples */
	audio->sample_count += samples;

	/* iterate over samples, tracking min/max */
	for (sampnum = 0; sampnum < samples; sampnum++)
	{
		/* did we hit a minimum on the left? */
		if (lsound[sampnum] < audio->min_lsample)
		{
			audio->min_lsample = lsound[sampnum];
			audio->min_lsample_count = 1;
		}
		else if (lsound[sampnum] == audio->min_lsample)
			audio->min_lsample_count++;

		/* did we hit a maximum on the left? */
		if (lsound[sampnum] > audio->max_lsample)
		{
			audio->max_lsample = lsound[sampnum];
			audio->max_lsample_count = 1;
		}
		else if (lsound[sampnum] == audio->max_lsample)
			audio->max_lsample_count++;

		/* did we hit a minimum on the right? */
		if (rsound[sampnum] < audio->min_rsample)
		{
			audio->min_rsample = rsound[sampnum];
			audio->min_rsample_count = 1;
		}
		else if (rsound[sampnum] == audio->min_rsample)
			audio->min_rsample_count++;

		/* did we hit a maximum on the right? */
		if (rsound[sampnum] > audio->max_rsample)
		{
			audio->max_rsample = rsound[sampnum];
			audio->max_rsample_count = 1;
		}
		else if (rsound[sampnum] == audio->max_rsample)
			audio->max_rsample_count++;
	}
}


/*-------------------------------------------------
    verify_audio_final - final verification
-------------------------------------------------*/

static void verify_audio_final(audio_info *audio)
{
	printf("\nAudio summary:\n");
	printf("  Overall channel 0 range: %d-%d (%04X-%04X)\n", audio->min_lsample, audio->max_lsample, (UINT16)audio->min_lsample, (UINT16)audio->max_lsample);
	printf("  Overall channel 1 range: %d-%d (%04X-%04X)\n", audio->min_rsample, audio->max_rsample, (UINT16)audio->min_rsample, (UINT16)audio->max_rsample);
}


/*-------------------------------------------------
    usage - display program usage
-------------------------------------------------*/

static int usage(void)
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "  ldverify <avifile.avi|chdfile.chd>\n");
	return 1;
}


/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	movie_info info = { 0 };
	INT16 *lsound, *rsound;
	const char *srcfile;
	bitmap_t *bitmap;
	int srcfilelen;
	int samples = 0;
	void *file;
	int isavi;
	int frame;
	audio_info audio;
	video_info video;

	/* init globals */
	init_audio(&audio);
	init_video(&video);

	/* verify arguments */
	if (argc < 2)
		return usage();
	srcfile = argv[1];

	/* check extension of file */
	srcfilelen = strlen(srcfile);
	if (srcfilelen < 4)
		return usage();
	if (tolower((UINT8)srcfile[srcfilelen-3]) == 'a' && tolower((UINT8)srcfile[srcfilelen-2]) == 'v' && tolower((UINT8)srcfile[srcfilelen-1]) == 'i')
		isavi = TRUE;
	else if (tolower((UINT8)srcfile[srcfilelen-3]) == 'c' && tolower((UINT8)srcfile[srcfilelen-2]) == 'h' && tolower((UINT8)srcfile[srcfilelen-1]) == 'd')
		isavi = FALSE;
	else
		return usage();

	/* open the file */
	printf("Processing file: %s\n", srcfile);
	file = isavi ? open_avi(srcfile, &info) : open_chd(srcfile, &info);
	if (file == NULL)
	{
		fprintf(stderr, "Unable to open file '%s'\n", srcfile);
		return 1;
	}

	/* comment on the video dimensions */
	printf("Video dimensions: %dx%d\n", info.width, info.height);
	if (info.width != 720)
		printf("WARNING: Unexpected video width (should be 720)\n");
	if (info.height != 524)
		printf("WARNING: Unexpected video height (should be 262 or 524)\n");

	/* comment on the video frame rate */
	printf("Video frame rate: %.2fHz\n", info.framerate);
	if ((int)(info.framerate * 100.0 + 0.5) != 2997)
		printf("WARNING: Unexpected frame rate (should be 29.97Hz)\n");

	/* comment on the sample rate */
	printf("Sample rate: %dHz\n", info.samplerate);
	if (info.samplerate != 48000)
		printf("WARNING: Unexpected sampele rate (should be 48000Hz)\n");

	/* allocate a bitmap */
	bitmap = bitmap_alloc(info.width, info.height, BITMAP_FORMAT_YUY16);
	if (bitmap == NULL)
	{
		isavi ? close_avi(file) : close_chd(file);
		fprintf(stderr, "Out of memory creating %dx%d bitmap\n", info.width, info.height);
		return 1;
	}

	/* allocate sound buffers */
	lsound = (INT16 *)malloc(info.samplerate * sizeof(*lsound));
	rsound = (INT16 *)malloc(info.samplerate * sizeof(*rsound));
	if (lsound == NULL || rsound == NULL)
	{
		isavi ? close_avi(file) : close_chd(file);
		bitmap_free(bitmap);
		if (rsound != NULL)
			free(rsound);
		if (lsound != NULL)
			free(lsound);
		fprintf(stderr, "Out of memory allocating sound buffers of %d bytes\n", (INT32)(info.samplerate * sizeof(*rsound)));
		return 1;
	}

	/* loop over frames */
	frame = 0;
	while (isavi ? read_avi(file, frame, bitmap, lsound, rsound, &samples) : read_chd(file, frame, bitmap, lsound, rsound, &samples))
	{
		verify_video(&video, frame, bitmap);
		verify_audio(&audio, lsound, rsound, samples);
		frame++;
	}

	/* close the files */
	isavi ? close_avi(file) : close_chd(file);

	/* final output */
	verify_video_final(&video, frame, bitmap);
	verify_audio_final(&audio);

	/* free memory */
	bitmap_free(bitmap);
	free(lsound);
	free(rsound);

	return 0;
}
