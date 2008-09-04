/*************************************************************************

    ldcore.c

    Private core laserdisc player implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#include "driver.h"
#include "ldcore.h"
#include "avcomp.h"
#include "streams.h"
#include "vbiparse.h"
#include "config.h"
#include "sound/custom.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_POSITION(x)				/*printf x*/



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* we simulate extra lead-in and lead-out tracks */
#define VIRTUAL_LEAD_IN_TRACKS		200
#define VIRTUAL_LEAD_OUT_TRACKS		200



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* core-specific data */
struct _ldcore_data
{
	/* general config */
	laserdisc_config	config;					/* copy of the inline config */
	ldplayer_interface	intf;					/* interface to the player */		

	/* disc parameters */
	chd_file *			disc;					/* handle to the disc itself */
	av_codec_decompress_config avconfig;		/* decompression configuration */
	UINT8				readpending;			/* true if a read is pending */
	UINT32				maxtrack;				/* maximum track number */

	/* core states */
	UINT8				audiosquelch;			/* audio squelch state: bit 0 = audio 1, bit 1 = audio 2 */
	UINT8				videosquelch;			/* video squelch state: bit 0 = on/off */
	UINT8				fieldnum;				/* field number (0 or 1) */
	INT32				curtrack;				/* current track */

	/* video data */
	bitmap_t *			videoframe[3];			/* currently cached frames */
	bitmap_t *			videovisframe[3];		/* wrapper around videoframe with only visible lines */
	UINT8				videofields[3];			/* number of fields in each frame */
	UINT32				videoframenum[3];		/* frame number contained in each frame */
	UINT8				videoindex;				/* index of the current video buffer */
	bitmap_t			videotarget;			/* fake target bitmap for decompression */
	bitmap_t *			emptyframe;				/* blank frame */

	/* audio data */
	INT16 *				audiobuffer[2];			/* buffer for audio samples */
	UINT32				audiobufsize;			/* size of buffer */
	UINT32				audiobufin;				/* input index */
	UINT32				audiobufout;			/* output index */
	UINT32				audiocursamples;		/* current samples this track */
	UINT32				audiomaxsamples;		/* maximum samples per track */
	int					audiocustom;			/* custom sound index */
	int					samplerate;				/* playback samplerate */

	/* metadata */
	vbi_metadata		metadata[2];			/* metadata parsed from the stream, for each field */
	int					last_frame;				/* last seen frame number */
	int					last_chapter;			/* last seen chapter number */

	/* I/O data */
	UINT8				datain;					/* current input data value */
	UINT8				linein[LASERDISC_INPUT_LINES]; /* current input line state */
	UINT8				dataout;				/* current output data value */
	UINT8				lineout[LASERDISC_OUTPUT_LINES]; /* current output line state */

	/* video updating */
	UINT8				videoenable;			/* is video enabled? */
	render_texture *	videotex;				/* texture for the video */
	UINT8				overenable;				/* is the overlay enabled? */
	bitmap_t *			overbitmap[2];			/* overlay bitmaps */
	int					overindex;				/* index of the overlay bitmap */
	render_texture *	overtex;				/* texture for the overlay */
};


/* sound callback info */
typedef struct _sound_token sound_token;
struct _sound_token
{
	sound_stream *		stream;
	laserdisc_state *	ld;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* generic helper functions */
static void read_track_data(laserdisc_state *ld);
static void process_track_data(const device_config *device);
static void fake_metadata(UINT32 track, UINT8 which, vbi_metadata *metadata);
//static void render_display(UINT16 *videodata, UINT32 rowpixels, UINT32 width, int frame);
static void *custom_start(int clock, const custom_sound_interface *config);
static void custom_stream_callback(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
static void configuration_load(running_machine *machine, int config_type, xml_data_node *parentnode);
static void configuration_save(running_machine *machine, int config_type, xml_data_node *parentnode);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const ldplayer_interface *player_interfaces[] = 
{
//	&pr7820_interface,
	&pr8210_interface,
	&simutrek_interface,
	&ldv1000_interface,
//	&ldp1450_interface,
//	&vp932_interface
};

const custom_sound_interface laserdisc_custom_interface =
{
	custom_start
};

static const UINT8 numberfont[10][8] =
{
	{
		0x30,	// ..xx....
		0x48,	// .x..x...
		0x84,	// x....x..
		0x84,	// x....x..
		0x84,	// x....x..
		0x48,	// .x..x...
		0x30	// ..xx....
	},
	{
		0x10,	// ...x....
		0x30,	// ..xx....
		0x50,	// .x.x....
		0x10,	// ...x....
		0x10,	// ...x....
		0x10,	// ...x....
		0x7c	// .xxxxx..
	},
	{
		0x78,	// .xxxx...
		0x84,	// x....x..
		0x04,	// .....x..
		0x38,	// ..xxx...
		0x40,	// .x......
		0x80,	// x.......
		0xfc	// xxxxxx..
	},
	{
		0x78,	// .xxxx...
		0x84,	// x....x..
		0x04,	// .....x..
		0x38,	// ..xxx...
		0x04,	// .....x..
		0x84,	// x....x..
		0x78	// .xxxx...
	},
	{
		0x08,	// ....x...
		0x18,	// ...xx...
		0x28,	// ..x.x...
		0x48,	// .x..x...
		0xfc,	// xxxxxx..
		0x08,	// ....x...
		0x08	// ....x...
	},
	{
		0xfc,	// xxxxxx..
		0x80,	// x.......
		0x80,	// x.......
		0xf8,	// xxxxx...
		0x04,	// .....x..
		0x84,	// x....x..
		0x78	// .xxxx...
	},
	{
		0x38,	// ..xxx...
		0x40,	// .x......
		0x80,	// x.......
		0xf8,	// xxxxx...
		0x84,	// x....x..
		0x84,	// x....x..
		0x78	// .xxxx...
	},
	{
		0xfc,	// xxxxxx..
		0x04,	// .....x..
		0x08,	// ....x...
		0x10,	// ...x....
		0x20,	// ..x.....
		0x20,	// ..x.....
		0x20	// ..x.....
	},
	{
		0x78,	// .xxxx...
		0x84,	// x....x..
		0x84,	// x....x..
		0x78,	// .xxxx...
		0x84,	// x....x..
		0x84,	// x....x..
		0x78	// .xxxx...
	},
	{
		0x78,	// .xxxx..
		0x84,	// x....x.
		0x84,	// x....x.
		0x7c,	// .xxxxx.
		0x04,	// .....x.
		0x08,	// ....x..
		0x70,	// .xxx...
	}
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a laserdisc device
-------------------------------------------------*/

INLINE laserdisc_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == LASERDISC);

	return (laserdisc_state *)device->token;
}


/*-------------------------------------------------
    audio_channel_active - return TRUE if the
    given audio channel should be output
-------------------------------------------------*/

INLINE int audio_channel_active(ldcore_data *ldcore, int channel)
{
	return (~ldcore->audiosquelch >> channel) & 1;
}


/*-------------------------------------------------
    video_active - return TRUE if the video should
    be output
-------------------------------------------------*/

INLINE int video_active(ldcore_data *ldcore)
{
	return !ldcore->videosquelch;
}


/*-------------------------------------------------
    add_to_current_track - add a value to the
    current track, stopping if we hit the min or
    max
-------------------------------------------------*/

INLINE int add_to_current_track(ldcore_data *ldcore, INT32 delta)
{
	ldcore->curtrack += delta;
	if (ldcore->curtrack < 1)
	{
		ldcore->curtrack = 1;
		return TRUE;
	}
	else if (ldcore->curtrack >= ldcore->maxtrack - 1)
	{
		ldcore->curtrack = ldcore->maxtrack - 1;
		return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    fillbitmap_yuy16 - fill a YUY16 bitmap with a
    given color pattern
-------------------------------------------------*/

INLINE void fillbitmap_yuy16(bitmap_t *bitmap, UINT8 yval, UINT8 cr, UINT8 cb)
{
	UINT16 color0 = (yval << 8) | cb;
	UINT16 color1 = (yval << 8) | cr;
	int x, y;

	/* write 32 bits of color (2 pixels at a time) */
	for (y = 0; y < bitmap->height; y++)
	{
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		for (x = 0; x < bitmap->width / 2; x++)
		{
			*dest++ = color0;
			*dest++ = color1;
		}
	}
}



/***************************************************************************
    GENERIC IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    laserdisc_vsync - call this once per field
    on the VSYNC signal
-------------------------------------------------*/

void laserdisc_vsync(const device_config *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	/* wait for previous read and decode to finish */
	process_track_data(device);
	
	/* update the state */
	if (ldcore->intf.update != NULL)
	{
		INT32 advanceby = (*ldcore->intf.update)(ld, &ldcore->metadata[ldcore->fieldnum], ldcore->fieldnum, timer_get_time());
		add_to_current_track(ldcore, advanceby);
	}

	/* flush any audio before we read more */
	if (ldcore->audiocustom != -1)
	{
		sound_token *token = custom_get_token(ldcore->audiocustom);
		stream_update(token->stream);
	}

	/* start reading the track data for the next round */
	ldcore->fieldnum ^= 1;
	read_track_data(ld);
}


/*-------------------------------------------------
    laserdisc_data_w - write data to the given
    laserdisc player
-------------------------------------------------*/

void laserdisc_data_w(const device_config *device, UINT8 data)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT8 prev = ldcore->datain;
	ldcore->datain = data;

	/* call through to the player-specific write handler */
	if (ldcore->intf.writedata != NULL)
		(*ldcore->intf.writedata)(ld, prev, data);
}


/*-------------------------------------------------
    laserdisc_line_w - control an input line
-------------------------------------------------*/

void laserdisc_line_w(const device_config *device, UINT8 line, UINT8 newstate)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	assert(line < LASERDISC_INPUT_LINES);
	assert(newstate == ASSERT_LINE || newstate == CLEAR_LINE || newstate == PULSE_LINE);

	/* assert */
	if (newstate == ASSERT_LINE || newstate == PULSE_LINE)
	{
		if (ldcore->linein[line] != ASSERT_LINE)
		{
			/* call through to the player-specific line handler */
			if (ldcore->intf.writeline[line] != NULL)
				(*ldcore->intf.writeline[line])(ld, CLEAR_LINE, ASSERT_LINE);
		}
		ldcore->linein[line] = ASSERT_LINE;
	}

	/* deassert */
	if (newstate == CLEAR_LINE || newstate == PULSE_LINE)
	{
		if (ldcore->linein[line] != CLEAR_LINE)
		{
			/* call through to the player-specific line handler */
			if (ldcore->intf.writeline[line] != NULL)
				(*ldcore->intf.writeline[line])(ld, ASSERT_LINE, CLEAR_LINE);
		}
		ldcore->linein[line] = CLEAR_LINE;
	}
}


/*-------------------------------------------------
    laserdisc_data_r - return the current
    data byte
-------------------------------------------------*/

UINT8 laserdisc_data_r(const device_config *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT8 result = ldcore->dataout;

	/* call through to the player-specific data handler */
	if (ldcore->intf.readdata != NULL)
		result = (*ldcore->intf.readdata)(ld);

	return result;
}


/*-------------------------------------------------
    laserdisc_line_r - return the current state
    of an output line
-------------------------------------------------*/

UINT8 laserdisc_line_r(const device_config *device, UINT8 line)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT8 result;

	assert(line < LASERDISC_OUTPUT_LINES);
	result = ldcore->lineout[line];

	/* call through to the player-specific data handler */
	if (ldcore->intf.readline[line] != NULL)
		result = (*ldcore->intf.readline[line])(ld);

	return result;
}


/*-------------------------------------------------
    laserdisc_get_video - return the current
    video frame
-------------------------------------------------*/

UINT32 laserdisc_get_video(const device_config *device, bitmap_t **bitmap)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	int frameindex;

	/* determine the most recent live set of frames */
	frameindex = ldcore->videoindex;
	if (ldcore->videofields[frameindex] < 2)
		frameindex = (frameindex + ARRAY_LENGTH(ldcore->videofields) - 1) % ARRAY_LENGTH(ldcore->videofields);

	/* if no video present, return the empty frame */
	if (!video_active(ldcore) || ldcore->videofields[frameindex] < 2)
	{
		*bitmap = ldcore->emptyframe;
		return 0;
	}
	else
	{
		*bitmap = ldcore->videovisframe[frameindex];
		return ldcore->videoframenum[frameindex];
	}
}


/*-------------------------------------------------
    laserdisc_get_field_code - return raw field
    information read from the disc
-------------------------------------------------*/

UINT32 laserdisc_get_field_code(const device_config *device, UINT8 code)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	int field = ldcore->fieldnum ^ 1;

	/* if no video present, return */
	if (!video_active(ldcore))
		return 0;

	switch (code)
	{
		case LASERDISC_CODE_WHITE_FLAG:
			return ldcore->metadata[field].white;

		case LASERDISC_CODE_LINE16:
			return ldcore->metadata[field].line16;

		case LASERDISC_CODE_LINE17:
			return ldcore->metadata[field].line17;

		case LASERDISC_CODE_LINE18:
			return ldcore->metadata[field].line18;
	}

	return 0;
}



/***************************************************************************
    PLAYER-TO-CORE INTERFACES
***************************************************************************/

/*-------------------------------------------------
    ldcore_get_safe_token - return a token with 
    type checking from a device
-------------------------------------------------*/

laserdisc_state *ldcore_get_safe_token(const device_config *device)
{
	return get_safe_token(device);
}


/*-------------------------------------------------
    ldcore_set_audio_squelch - set the left/right 
    audio squelch states
-------------------------------------------------*/

void ldcore_set_audio_squelch(laserdisc_state *ld, UINT8 squelchleft, UINT8 squelchright)
{
	ld->core->audiosquelch = (squelchleft ? 1 : 0) | (squelchright ? 2 : 0);
}


/*-------------------------------------------------
    ldcore_set_video_squelch - set the video 
    squelch state
-------------------------------------------------*/

void ldcore_set_video_squelch(laserdisc_state *ld, UINT8 squelch)
{
	ld->core->videosquelch = squelch;
}



/***************************************************************************
    GENERIC HELPER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ldcore_generic_update - generically update in 
    a way that works for most situations
-------------------------------------------------*/

INT32 ldcore_generic_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime, ldplayer_state *newstate)
{
	INT32 advanceby = 0;
	int frame;
	
	/* start by assuming the state doesn't change */
	*newstate = ld->state;
	
	/* handle things based on the state */
	switch (ld->state.state)
	{
		case LDSTATE_EJECTING:
			/* when time expires, switch to the ejected state */
			if (attotime_compare(curtime, ld->state.endtime) >= 0)
			    newstate->state = LDSTATE_EJECTED;
			break;
		
		case LDSTATE_EJECTED:
			/* do nothing */
			break;
		
		case LDSTATE_PARKED:
			/* do nothing */
			break;
		
		case LDSTATE_LOADING:
			/* when time expires, switch to the spinup state */
			if (attotime_compare(curtime, ld->state.endtime) >= 0)
			    newstate->state = LDSTATE_SPINUP;
			advanceby = -GENERIC_SEARCH_SPEED;
			break;
		
		case LDSTATE_SPINUP:
			/* when time expires, switch to the playing state */
			if (attotime_compare(curtime, ld->state.endtime) >= 0)
			    newstate->state = LDSTATE_PLAYING;
			advanceby = -GENERIC_SEARCH_SPEED;
			break;
		
		case LDSTATE_PAUSING:
			/* if he hit the start of a frame, switch to paused state */
			if (is_start_of_frame(vbi))
			{
			    newstate->state = LDSTATE_PAUSED;
			    newstate->param = fieldnum;
			}
			
			/* else advance until we hit it */
			else if (fieldnum == 1)
				advanceby = 1;
			break;
		
		case LDSTATE_PAUSED:
			/* if we paused on field 1, we must flip back and forth */
			if (ld->state.param == 1)
				advanceby = (fieldnum == 1) ? 1 : -1;
			break;
		
		case LDSTATE_PLAYING:
			/* if we hit the target frame, switch to the paused state */
			if (ld->state.param > 0 && is_start_of_frame(vbi) && frame_from_metadata(vbi) == ld->state.param)
			{
			    newstate->state = LDSTATE_PAUSED;
			    newstate->param = fieldnum;
			}

			/* otherwise after the second field of each frame */
			else if (fieldnum == 1)
				advanceby = 1;
			break;
		
		case LDSTATE_PLAYING_SLOW_REVERSE:
			/* after the second field of each frame, see if we need to advance */
			if (fieldnum == 1 && ++ld->state.substate > ld->state.param)
			{
				advanceby = -1;
				ld->state.substate = 0;
			}
			break;
		
		case LDSTATE_PLAYING_SLOW_FORWARD:
			/* after the second field of each frame, see if we need to advance */
			if (fieldnum == 1 && ++ld->state.substate > ld->state.param)
			{
				advanceby = 1;
				ld->state.substate = 0;
			}
			break;
		
		case LDSTATE_PLAYING_FAST_REVERSE:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
				advanceby = -ld->state.param;
			break;

		case LDSTATE_PLAYING_FAST_FORWARD:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
				advanceby = ld->state.param;
			break;
		
		case LDSTATE_SCANNING:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
				advanceby = ld->state.param >> 8;
			
			/* after we run out of vsyncs, revert to the saved state */
			if (++ld->state.substate >= (ld->state.param & 0xff))
				*newstate = ld->savestate;
			break;
		
		case LDSTATE_STEPPING_REVERSE:
			/* wait for the first field of the frame and then leap backwards */
			if (is_start_of_frame(vbi))
			{
				advanceby = (fieldnum == 1) ? -1 : -2;
			    newstate->state = LDSTATE_PAUSING;
			}
			break;
		
		case LDSTATE_STEPPING_FORWARD:
			/* wait for the first field of the frame and then switch to pausing state */
			if (is_start_of_frame(vbi))
			    newstate->state = LDSTATE_PAUSING;
			break;
		
		case LDSTATE_SEEKING:
			/* if we're in the final state, look for a matching frame and pause there */
			frame = frame_from_metadata(vbi);
			if (ld->state.substate == 1 && is_start_of_frame(vbi) && frame == ld->state.param)
			{
			    newstate->state = LDSTATE_PAUSED;
			    newstate->param = fieldnum;
			}
			
			/* otherwise, if we got frame data from the VBI, update our seeking logic */
			else if (ld->state.substate == 0 && frame != FRAME_NOT_PRESENT)
			{
				INT32 delta = (ld->state.param - 2) - frame;

				/* if we're within a couple of frames, just play until we hit it */
				if (delta >= 0 && delta <= 2)
					ld->state.substate++;
				
				/* otherwise, compute the delta assuming 1:1 track to frame; this will correct eventually */
				else
				{
					if (delta < 0)
						delta--;
					advanceby = delta;
					advanceby = MIN(advanceby, GENERIC_SEARCH_SPEED);
					advanceby = MAX(advanceby, -GENERIC_SEARCH_SPEED);
				}
			}
			
			/* otherwise, keep advancing until we know what's up */
			else
			{
				if (fieldnum == 1)
					advanceby = 1;
			}
			break;
	}
		
	return advanceby;
}


/*-------------------------------------------------
    read_track_data - read and process data for
    a particular video track
-------------------------------------------------*/

static void read_track_data(laserdisc_state *ld)
{
	ldcore_data *ldcore = ld->core;
	UINT32 tracknum = ldcore->curtrack;
	UINT32 fieldnum = ldcore->fieldnum;
	chd_error err;
	
	/* if the previous field had the white flag, force the new field to pair with it */
	if (ldcore->metadata[fieldnum ^ 1].white)
		ldcore->videofields[ldcore->videoindex] = 1;

	/* if we already have both fields on the current videoindex, advance */
	if (ldcore->videofields[ldcore->videoindex] >= 2)
	{
		ldcore->videoindex = (ldcore->videoindex + 1) % ARRAY_LENGTH(ldcore->videofields);
		ldcore->videofields[ldcore->videoindex] = 0;
	}

	/* set the video target information */
	ldcore->videotarget = *ldcore->videoframe[ldcore->videoindex];
	ldcore->videotarget.base = BITMAP_ADDR16(&ldcore->videotarget, fieldnum, 0);
	ldcore->videotarget.height /= 2;
	ldcore->videotarget.rowpixels *= 2;
	ldcore->avconfig.video = &ldcore->videotarget;

	/* set the audio target information */
	if (ldcore->audiobufin + ldcore->audiomaxsamples <= ldcore->audiobufsize)
	{
		/* if we can fit without wrapping, just read the data directly */
		ldcore->avconfig.audio[0] = &ldcore->audiobuffer[0][ldcore->audiobufin];
		ldcore->avconfig.audio[1] = &ldcore->audiobuffer[1][ldcore->audiobufin];
	}
	else
	{
		/* otherwise, read to the beginning of the buffer */
		ldcore->avconfig.audio[0] = &ldcore->audiobuffer[0][0];
		ldcore->avconfig.audio[1] = &ldcore->audiobuffer[1][0];
	}

	/* override if we're not decoding */
	ldcore->avconfig.maxsamples = ldcore->audiomaxsamples;
	ldcore->avconfig.actsamples = &ldcore->audiocursamples;
	ldcore->audiocursamples = 0;
	if (!audio_channel_active(ldcore, 0))
		ldcore->avconfig.audio[0] = NULL;
	if (!audio_channel_active(ldcore, 1))
		ldcore->avconfig.audio[1] = NULL;

	/* configure the codec and then read */
	if (ldcore->disc != NULL)
	{
		err = chd_codec_config(ldcore->disc, AV_CODEC_DECOMPRESS_CONFIG, &ldcore->avconfig);
		if (err == CHDERR_NONE)
		{
			INT32 maxtrack = chd_get_header(ldcore->disc)->totalhunks / 2;
			INT32 chdtrack = tracknum - 1 - VIRTUAL_LEAD_IN_TRACKS;

			/* clamp the track */
			chdtrack = MAX(chdtrack, 0);
			chdtrack = MIN(chdtrack, maxtrack - 1);
			err = chd_read_async(ldcore->disc, chdtrack * 2 + fieldnum, NULL);
			ldcore->readpending = TRUE;
		}
	}
}


/*-------------------------------------------------
    process_track_data - process data from a
    track after it has been read
-------------------------------------------------*/

static void process_track_data(const device_config *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT32 tracknum = ldcore->curtrack;
	UINT32 fieldnum = ldcore->fieldnum;
	int frame, chapter;
	chd_error chderr;

	/* wait for the async operation to complete */
	if (ldcore->disc != NULL && ldcore->readpending)
	{
		/* complete the async operation */
		chderr = chd_async_complete(ldcore->disc);
		if (chderr != CHDERR_NONE && chderr != CHDERR_NO_ASYNC_OPERATION)
			ldcore->avconfig.video = NULL;
	}
	ldcore->readpending = FALSE;

	/* parse the metadata */
	if (ldcore->disc != NULL && ldcore->avconfig.video != NULL)
		vbi_parse_all((const UINT16 *)ldcore->avconfig.video->base, ldcore->avconfig.video->rowpixels, ldcore->avconfig.video->width, 8, &ldcore->metadata[fieldnum]);
	else
		fake_metadata(tracknum, fieldnum, &ldcore->metadata[fieldnum]);
//  printf("Track %5d: Metadata = %d %08X %08X %08X %08X\n", tracknum, ldcore->metadata[fieldnum].white, ldcore->metadata[fieldnum].line16, ldcore->metadata[fieldnum].line17, ldcore->metadata[fieldnum].line18, ldcore->metadata[fieldnum].line1718);

	/* update the last seen frame and chapter */
	frame = frame_from_metadata(&ldcore->metadata[fieldnum]);
	if (frame >= 1 && frame < 99999)
		ldcore->last_frame = frame;
	chapter = chapter_from_metadata(&ldcore->metadata[fieldnum]);
	if (chapter != -1)
		ldcore->last_chapter = chapter;

	/* render the display if present */
//	if (ldcore->display && ldcore->avconfig.video != NULL)
//		render_display((UINT16 *)ldcore->avconfig.video->base, ldcore->avconfig.video->rowpixels, ldcore->avconfig.video->width, ldcore->last_frame);

	/* update video field */
	if (ldcore->avconfig.video != NULL)
	{
		ldcore->videofields[ldcore->videoindex]++;
		ldcore->videoframenum[ldcore->videoindex] = ldcore->last_frame;
	}

	/* pass the audio to the callback */
	if (ldcore->config.audio != NULL)
		(*ldcore->config.audio)(device, ldcore->samplerate, ldcore->audiocursamples, ldcore->avconfig.audio[0], ldcore->avconfig.audio[1]);

	/* shift audio data if we read it into the beginning of the buffer */
	if (ldcore->audiocursamples != 0 && ldcore->audiobufin != 0)
	{
		int chnum;

		/* iterate over channels */
		for (chnum = 0; chnum < 2; chnum++)
			if (ldcore->avconfig.audio[chnum] == &ldcore->audiobuffer[chnum][0])
			{
				int samplesleft;

				/* move data to the end */
				samplesleft = ldcore->audiobufsize - ldcore->audiobufin;
				samplesleft = MIN(samplesleft, ldcore->audiocursamples);
				memmove(&ldcore->audiobuffer[chnum][ldcore->audiobufin], &ldcore->audiobuffer[chnum][0], samplesleft * 2);

				/* shift data at the beginning */
				if (samplesleft < ldcore->audiocursamples)
					memmove(&ldcore->audiobuffer[chnum][0], &ldcore->audiobuffer[chnum][samplesleft], (ldcore->audiocursamples - samplesleft) * 2);
			}
	}

	/* update the input buffer pointer */
	ldcore->audiobufin = (ldcore->audiobufin + ldcore->audiocursamples) % ldcore->audiobufsize;
}


/*-------------------------------------------------
    fake_metadata - fake metadata when there's
    no disc present
-------------------------------------------------*/

static void fake_metadata(UINT32 track, UINT8 which, vbi_metadata *metadata)
{
	if (which == 0)
	{
		metadata->white = 1;
		metadata->line16 = 0;
		metadata->line17 = metadata->line18 = 0xf80000 |
				(((track / 10000) % 10) << 16) |
				(((track / 1000) % 10) << 12) |
				(((track / 100) % 10) << 8) |
				(((track / 10) % 10) << 4) |
				(((track / 1) % 10) << 0);
	}
	else
		memset(metadata, 0, sizeof(*metadata));
}


/*-------------------------------------------------
    render_display - draw the frame display
-------------------------------------------------*/

#ifdef UNUSED_FUNCTION
static void render_display(UINT16 *videodata, UINT32 rowpixels, UINT32 width, int frame)
{
	const int xscale = 4, yscale = 2;
	char buffer[10];
	int x = width / 10;
	int y = 50;
	int ch;
	int delta;

	/* do nothing if no data */
	if (videodata == NULL)
		return;

	/* convert to a character string and render */
	sprintf(buffer, "%5d", frame);

	/* iterate over 5 positions: (-1,-1), (-1,1), (1,-1), (1,1) and (0,0) */
	/* render all in black except the last one */
	for (delta = 0; delta < 5; delta++)
	{
		int color = (delta < 4) ? 0x0080 : 0xff80;
		int dx = (delta < 4) ? ((delta & 1) ? -1 : 1) : 0;
		int dy = (delta < 4) ? ((delta & 2) ? -1 : 1) : 0;

		/* iterate over 5 characters */
		for (ch = 0; ch < 5; ch++)
			if (buffer[ch] >= '0' && buffer[ch] <= '9')
			{
				const UINT8 *fontdata = &numberfont[buffer[ch] - '0'][0];
				int cy, cx;

				/* iterate over the rows of the character */
				for (cy = 0; cy < 8; cy++)
				{
					UINT8 bits = *fontdata++;

					/* and over the columns */
					for (cx = 0; cx < 8; cx++)
						if (bits & (0x80 >> cx))
						{
							int ix, iy;

							/* fill in an xscale x yscale pixel */
							for (iy = 0; iy < yscale; iy++)
								for (ix = 0; ix < xscale; ix++)
									videodata[(y + cy * yscale + iy + dy) * rowpixels + (x + (ch * 9 + cx) * xscale + ix + dx)] = color;
						}
				}
			}
	}
}
#endif


/*-------------------------------------------------
    custom_start - custom audio start
    for laserdiscs
-------------------------------------------------*/

static void *custom_start(int clock, const custom_sound_interface *config)
{
	sound_token *token = auto_malloc(sizeof(*token));
	token->stream = stream_create(0, 2, 48000, token, custom_stream_callback);
	token->ld = NULL;
	return token;
}


/*-------------------------------------------------
    custom_stream_callback - audio streamer
    for laserdiscs
-------------------------------------------------*/

static void custom_stream_callback(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	sound_token *token = param;
	laserdisc_state *ld = token->ld;
	ldcore_data *ldcore = ld->core;
	stream_sample_t *dst0 = outputs[0];
	stream_sample_t *dst1 = outputs[1];
	int samples_avail = 0;

	/* see if we have enough samples to fill the buffer; if not, drop out */
	if (ld != NULL)
	{
		samples_avail = ldcore->audiobufin - ldcore->audiobufout;
		if (samples_avail < 0)
			samples_avail += ldcore->audiobufsize;
	}

	/* if no attached ld, just clear the buffers */
	if (samples_avail < samples)
	{
		memset(dst0, 0, samples * sizeof(dst0[0]));
		memset(dst1, 0, samples * sizeof(dst1[0]));
	}

	/* otherwise, stream from our buffer */
	else
	{
		INT16 *buffer0 = ldcore->audiobuffer[0];
		INT16 *buffer1 = ldcore->audiobuffer[1];
		int sampout = ldcore->audiobufout;

		/* copy samples, clearing behind us as we go */
		while (sampout != ldcore->audiobufin && samples-- > 0)
		{
			*dst0++ = buffer0[sampout];
			*dst1++ = buffer1[sampout];
			buffer0[sampout] = 0;
			buffer1[sampout] = 0;
			sampout++;
			if (sampout >= ldcore->audiobufsize)
				sampout = 0;
		}
		ldcore->audiobufout = sampout;

		/* clear out the rest of the buffer */
		if (samples > 0)
		{
			int sampout = (ldcore->audiobufout == 0) ? ldcore->audiobufsize - 1 : ldcore->audiobufout - 1;
			stream_sample_t fill0 = buffer0[sampout];
			stream_sample_t fill1 = buffer1[sampout];

			while (samples-- > 0)
			{
				*dst0++ = fill0;
				*dst1++ = fill1;
			}
		}
	}
}



/***************************************************************************
    CONFIG SETTINGS ACCESS
***************************************************************************/

/*-------------------------------------------------
    configuration_load - read and apply data from
    the configuration file
-------------------------------------------------*/

static void configuration_load(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *overnode;
	xml_data_node *ldnode;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == NULL)
		return;

	/* iterate over overlay nodes */
	for (ldnode = xml_get_sibling(parentnode->child, "device"); ldnode != NULL; ldnode = xml_get_sibling(ldnode->next, "device"))
	{
		const char *devtag = xml_get_attribute_string(ldnode, "tag", "");
		const device_config *device = device_list_find_by_tag(machine->config->devicelist, LASERDISC, devtag);
		if (device != NULL)
		{
			laserdisc_state *ld = get_safe_token(device);
			ldcore_data *ldcore = ld->core;

			/* handle the overlay node */
			overnode = xml_get_sibling(ldnode->child, "overlay");
			if (overnode != NULL)
			{
				/* fetch positioning controls */
				ldcore->config.overposx = xml_get_attribute_float(overnode, "hoffset", ldcore->config.overposx);
				ldcore->config.overscalex = xml_get_attribute_float(overnode, "hstretch", ldcore->config.overscalex);
				ldcore->config.overposy = xml_get_attribute_float(overnode, "voffset", ldcore->config.overposy);
				ldcore->config.overscaley = xml_get_attribute_float(overnode, "vstretch", ldcore->config.overscaley);
			}
		}
	}
}


/*-------------------------------------------------
    configuration_save - save data to the
    configuration file
-------------------------------------------------*/

static void configuration_save(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	const device_config *device;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* iterate over disc devices */
	for (device = device_list_first(machine->config->devicelist, LASERDISC); device != NULL; device = device_list_next(device, LASERDISC))
	{
		laserdisc_config *origconfig = device->inline_config;
		laserdisc_state *ld = get_safe_token(device);
		ldcore_data *ldcore = ld->core;
		xml_data_node *overnode;
		xml_data_node *ldnode;

		/* create a node */
		ldnode = xml_add_child(parentnode, "device", NULL);
		if (ldnode != NULL)
		{
			int changed = FALSE;

			/* output the basics */
			xml_set_attribute(ldnode, "tag", device->tag);

			/* add an overlay node */
			overnode = xml_add_child(ldnode, "overlay", NULL);
			if (overnode != NULL)
			{
				/* output the positioning controls */
				if (ldcore->config.overposx != origconfig->overposx)
				{
					xml_set_attribute_float(overnode, "hoffset", ldcore->config.overposx);
					changed = TRUE;
				}

				if (ldcore->config.overscalex != origconfig->overscalex)
				{
					xml_set_attribute_float(overnode, "hstretch", ldcore->config.overscalex);
					changed = TRUE;
				}

				if (ldcore->config.overposy != origconfig->overposy)
				{
					xml_set_attribute_float(overnode, "voffset", ldcore->config.overposy);
					changed = TRUE;
				}

				if (ldcore->config.overscaley != origconfig->overscaley)
				{
					xml_set_attribute_float(overnode, "vstretch", ldcore->config.overscaley);
					changed = TRUE;
				}
			}

			/* if nothing changed, kill the node */
			if (!changed)
				xml_delete_node(ldnode);
		}
	}
}



/***************************************************************************
    VIDEO INTERFACE
***************************************************************************/

/*-------------------------------------------------
    laserdisc_video_enable - enable/disable the
    video
-------------------------------------------------*/

void laserdisc_video_enable(const device_config *device, int enable)
{
	laserdisc_state *ld = get_safe_token(device);
	ld->core->videoenable = enable;
}


/*-------------------------------------------------
    laserdisc_video_enable - enable/disable the
    video
-------------------------------------------------*/

void laserdisc_overlay_enable(const device_config *device, int enable)
{
	laserdisc_state *ld = get_safe_token(device);
	ld->core->overenable = enable;
}


/*-------------------------------------------------
    video update callback
-------------------------------------------------*/

VIDEO_UPDATE( laserdisc )
{
	const device_config *laserdisc = device_list_first(screen->machine->config->devicelist, LASERDISC);
	if (laserdisc != NULL)
	{
		laserdisc_state *ld = laserdisc->token;
		ldcore_data *ldcore = ld->core;
		bitmap_t *overbitmap = ldcore->overbitmap[ldcore->overindex];
		bitmap_t *vidbitmap = NULL;

		/* handle the overlay if present */
		if (overbitmap != NULL && ldcore->config.overupdate != NULL)
		{
			rectangle clip = *cliprect;

			/* scale the cliprect to the overlay size and then call the update callback */
			clip.min_x = 0;
			clip.max_x = ldcore->config.overwidth - 1;
			clip.min_y = cliprect->min_y * overbitmap->height / bitmap->height;
			clip.max_y = (cliprect->max_y + 1) * overbitmap->height / bitmap->height - 1;
			(*ldcore->config.overupdate)(screen, overbitmap, &clip);
		}

		/* if this is the last update, do the rendering */
		if (cliprect->max_y == video_screen_get_visible_area(screen)->max_y)
		{
			float x0, y0, x1, y1;

			/* update the texture with the overlay contents */
			if (overbitmap != NULL)
			{
				if (overbitmap->format == BITMAP_FORMAT_INDEXED16)
					render_texture_set_bitmap(ldcore->overtex, overbitmap, &ldcore->config.overclip, 0, TEXFORMAT_PALETTEA16);
				else if (overbitmap->format == BITMAP_FORMAT_RGB32)
					render_texture_set_bitmap(ldcore->overtex, overbitmap, &ldcore->config.overclip, 0, TEXFORMAT_ARGB32);
			}

			/* get the laserdisc video */
			laserdisc_get_video(laserdisc, &vidbitmap);
			if (vidbitmap != NULL)
				render_texture_set_bitmap(ldcore->videotex, vidbitmap, NULL, 0, TEXFORMAT_YUY16);

			/* reset the screen contents */
			render_container_empty(render_container_get_screen(screen));

			/* add the video texture */
			if (ldcore->videoenable)
				render_screen_add_quad(screen, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), ldcore->videotex, PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));

			/* add the overlay */
			if (ldcore->overenable && overbitmap != NULL)
			{
				x0 = 0.5f - 0.5f * ldcore->config.overscalex + ldcore->config.overposx;
				y0 = 0.5f - 0.5f * ldcore->config.overscaley + ldcore->config.overposy;
				x1 = x0 + ldcore->config.overscalex;
				y1 = y0 + ldcore->config.overscaley;
				render_screen_add_quad(screen, x0, y0, x1, y1, MAKE_ARGB(0xff,0xff,0xff,0xff), ldcore->overtex, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_SCREENTEX(1));
			}

			/* swap to the next bitmap */
			ldcore->overindex = (ldcore->overindex + 1) % ARRAY_LENGTH(ldcore->overbitmap);
		}
	}

	return 0;
}



/***************************************************************************
    CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    laserdisc_get_config - return a copy of the
    current live configuration settings
-------------------------------------------------*/

void laserdisc_get_config(const device_config *device, laserdisc_config *config)
{
	laserdisc_state *ld = get_safe_token(device);
	*config = ld->core->config;
}


/*-------------------------------------------------
    laserdisc_get_config - change the current live
    configuration settings
-------------------------------------------------*/

void laserdisc_set_config(const device_config *device, const laserdisc_config *config)
{
	laserdisc_state *ld = get_safe_token(device);
	ld->core->config = *config;
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( laserdisc )
{
	int fps = 30, fpsfrac = 0, width = 720, height = 240, interlaced = 1, channels = 2, rate = 44100;
	const laserdisc_config *config = device->inline_config;
	laserdisc_state *ld = get_safe_token(device);
	UINT32 fps_times_1million;
	ldcore_data *ldcore;
	char metadata[256];
	int sndnum, index;
	int statesize;
	chd_error err;
	
	/* save a copy of the device pointer */
	ld->device = device;
	
	/* allocate memory for the core state */
	ld->core = auto_malloc(sizeof(*ld->core));
	memset(ld->core, 0, sizeof(*ld->core));
	ldcore = ld->core;

	/* determine the maximum player-specific state size and allocate it */
	statesize = 0;
	for (index = 0; index < ARRAY_LENGTH(player_interfaces); index++)
		statesize = MAX(statesize, player_interfaces[index]->statesize);
	ld->player = auto_malloc(statesize);
	memset(ld->player, 0, statesize);
	
	/* copy config data to the live state */
	ldcore->config = *config;
	if (ldcore->config.overclip.max_x == ldcore->config.overclip.min_x || ldcore->config.overclip.max_y == ldcore->config.overclip.min_y)
	{
		ldcore->config.overclip.min_x = ldcore->config.overclip.min_y = 0;
		ldcore->config.overclip.max_x = ldcore->config.overwidth - 1;
		ldcore->config.overclip.max_y = ldcore->config.overheight - 1;
	}
	if (ldcore->config.overscalex == 0)
		ldcore->config.overscalex = 1.0f;
	if (ldcore->config.overscaley == 0)
		ldcore->config.overscaley = 1.0f;

	/* find the disc */
	ldcore->disc = get_disk_handle(device->tag);
	ldcore->audiocustom = 0;
	for (sndnum = 0; sndnum < MAX_SOUND; sndnum++)
	{
		if (device->machine->config->sound[sndnum].tag != NULL && strcmp(device->machine->config->sound[sndnum].tag, device->tag) == 0)
			break;
		if (device->machine->config->sound[sndnum].type == SOUND_CUSTOM)
			ldcore->audiocustom++;
	}
	if (sndnum == MAX_SOUND)
		ldcore->audiocustom = -1;

	/* get the disc metadata and extract the ld */
	if (ldcore->disc != NULL)
	{
		/* require the A/V codec */
		if (chd_get_header(ldcore->disc)->compression != CHDCOMPRESSION_AV)
			fatalerror("Laserdisc video must be compressed with the A/V codec!");

		/* read and extract the metadata */
		err = chd_get_metadata(ldcore->disc, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL);
		if (err != CHDERR_NONE)
			fatalerror("Non-A/V CHD file specified");
		if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
			fatalerror("Invalid metadata in CHD file");

		/* require interlaced video */
		if (!interlaced)
			fatalerror("Laserdisc video must be interlaced!");

		/* determine the maximum track and allocate a frame buffer */
		ldcore->maxtrack = chd_get_header(ldcore->disc)->totalhunks / 2;
	}
	else
		ldcore->maxtrack = 54000;
	ldcore->maxtrack += VIRTUAL_LEAD_IN_TRACKS + VIRTUAL_LEAD_OUT_TRACKS;

	/* allocate video frames */
	for (index = 0; index < ARRAY_LENGTH(ldcore->videofields); index++)
	{
		/* first allocate a YUY16 bitmap at 2x the height */
		ldcore->videoframe[index] = auto_bitmap_alloc(width, height * 2, BITMAP_FORMAT_YUY16);
		fillbitmap_yuy16(ldcore->videoframe[index], 40, 109, 240);
		
		/* make a copy of the bitmap that clips out the VBI and horizontal blanking areas */
		ldcore->videovisframe[index] = auto_malloc(sizeof(*ldcore->videovisframe[index]));
		*ldcore->videovisframe[index] = *ldcore->videoframe[index];
		ldcore->videovisframe[index]->base = BITMAP_ADDR16(ldcore->videovisframe[index], 44, ldcore->videoframe[index]->width * 8 / 720);
		ldcore->videovisframe[index]->height -= 44;
		ldcore->videovisframe[index]->width -= 2 * ldcore->videoframe[index]->width * 8 / 720;
	}
	
	/* allocate an empty frame of the same size */
	ldcore->emptyframe = auto_bitmap_alloc(width, height * 2, BITMAP_FORMAT_YUY16);
	fillbitmap_yuy16(ldcore->emptyframe, 0, 128, 128);

	/* allocate audio buffers */
	fps_times_1million = fps * 1000000 + fpsfrac;
	ldcore->audiomaxsamples = ((UINT64)rate * 1000000 + fps_times_1million - 1) / fps_times_1million;
	ldcore->audiobufsize = ldcore->audiomaxsamples * 4;
	ldcore->audiobuffer[0] = auto_malloc(ldcore->audiobufsize * sizeof(ldcore->audiobuffer[0][0]));
	ldcore->audiobuffer[1] = auto_malloc(ldcore->audiobufsize * sizeof(ldcore->audiobuffer[1][0]));
	ldcore->samplerate = rate;

	/* allocate texture for rendering */
	ldcore->videoenable = TRUE;
	ldcore->videotex = render_texture_alloc(NULL, NULL);
	if (ldcore->videotex == NULL)
		fatalerror("Out of memory allocating video texture");

	/* allocate overlay */
	if (ldcore->config.overwidth > 0 && ldcore->config.overheight > 0 && ldcore->config.overupdate != NULL)
	{
		ldcore->overenable = TRUE;
		ldcore->overbitmap[0] = auto_bitmap_alloc(ldcore->config.overwidth, ldcore->config.overheight, ldcore->config.overformat);
		ldcore->overbitmap[1] = auto_bitmap_alloc(ldcore->config.overwidth, ldcore->config.overheight, ldcore->config.overformat);
		ldcore->overtex = render_texture_alloc(NULL, NULL);
		if (ldcore->overtex == NULL)
			fatalerror("Out of memory allocating overlay texture");
	}

	/* register callbacks */
	config_register(device->machine, "laserdisc", configuration_load, configuration_save);
}


/*-------------------------------------------------
    device exit callback
-------------------------------------------------*/

static DEVICE_STOP( laserdisc )
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	/* make sure all async operations have completed */
	if (ldcore->disc != NULL)
		chd_async_complete(ldcore->disc);

	/* free any textures */
	if (ldcore->videotex != NULL)
		render_texture_free(ldcore->videotex);
	if (ldcore->overtex != NULL)
		render_texture_free(ldcore->overtex);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( laserdisc )
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	int pltype, line;

	/* find our interface */
	for (pltype = 0; pltype < ARRAY_LENGTH(player_interfaces); pltype++)
		if (player_interfaces[pltype]->type == ldcore->config.type)
			break;
	if (pltype == ARRAY_LENGTH(player_interfaces))
		fatalerror("No interface found for laserdisc player type %d\n", ldcore->config.type);
	ldcore->intf = *player_interfaces[pltype];

	/* attempt to wire up the audio */
	if (ldcore->audiocustom != -1)
	{
		sound_token *token = custom_get_token(ldcore->audiocustom);
		token->ld = ld;
		stream_set_sample_rate(token->stream, ldcore->samplerate);
	}

	/* set up the general ld */
	ldcore->videosquelch = 1;
	ldcore->audiosquelch = 3;

	/* default to track 1 */
	ldcore->curtrack = 1;
	ldcore->last_frame = 0;
	ldcore->last_chapter = 0;

	/* reset the I/O lines */
	for (line = 0; line < LASERDISC_INPUT_LINES; line++)
		ldcore->linein[line] = CLEAR_LINE;
	for (line = 0; line < LASERDISC_OUTPUT_LINES; line++)
		ldcore->lineout[line] = CLEAR_LINE;
	
	/* call the initialization */
	if (ldcore->intf.init != NULL)
		(*ldcore->intf.init)(ld);
}


/*-------------------------------------------------
    device set info callback
-------------------------------------------------*/

static DEVICE_SET_INFO( laserdisc )
{
	laserdisc_state *ld = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case LDINFO_INT_TYPE:
			if (ld != NULL && ld->core != NULL && ld->core->config.type != info->i)
			{
				ld->core->config.type = info->i;
				device_reset(device);
			}
			break;
	}
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( laserdisc )
{
	const laserdisc_config *config = NULL;
	int pltype;

	/* if we have a device, figure out where our config lives */
	if (device != NULL)
	{
		laserdisc_state *ld = device->token;
		config = (ld == NULL || ld->core == NULL) ? device->inline_config : &ld->core->config;
	}

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(laserdisc_state);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(laserdisc_config);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;						break;
		case LDINFO_INT_TYPE:					info->i = config->type;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:				info->set_info = DEVICE_SET_INFO_NAME(laserdisc); 	break;
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(laserdisc); 		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(laserdisc); 			break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(laserdisc);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:
			info->s = "Unknown Laserdisc Player";
			for (pltype = 0; pltype < ARRAY_LENGTH(player_interfaces); pltype++)
				if (player_interfaces[pltype]->type == config->type)
					info->s = player_interfaces[pltype]->name;
			break;
		case DEVINFO_STR_FAMILY:				info->s = "Laserdisc Player";			break;
		case DEVINFO_STR_VERSION:				info->s = "1.0";						break;
		case DEVINFO_STR_SOURCE_FILE:			info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:				info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
