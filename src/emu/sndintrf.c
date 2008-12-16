/***************************************************************************

    sndintrf.c

    Core sound interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Still to do:
        * fix drivers that used to use ADPCM
        * many cores do their own resampling; they should stop
        * many cores mix to a separate buffer; no longer necessary

***************************************************************************/

#include "driver.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE			(0)

#define VPRINTF(x) do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sndintrf_data sndintrf_data;
struct _sndintrf_data
{
	snd_class_header	intf;	 		/* copy of the interface data */
	sound_type		sndtype; 		/* type index of this sound chip */
	sound_type		aliastype;		/* aliased type index of this sound chip */
	device_config *	device;			/* dummy device for now */
	const char *	tag; 			/* tag this sound chip */
	int				index; 			/* index of this sound chip */
	int				clock; 			/* clock for this sound chip */
};



/***************************************************************************
    PROTOTYPES FOR ALL SND ENTRY POINTS
***************************************************************************/

static SND_GET_INFO( dummy_sound );
SND_GET_INFO( custom );
SND_GET_INFO( samples );
SND_GET_INFO( dac );
SND_GET_INFO( dmadac );
SND_GET_INFO( discrete );
SND_GET_INFO( ay8910 );
SND_GET_INFO( ay8912 );
SND_GET_INFO( ay8913 );
SND_GET_INFO( ay8930 );
SND_GET_INFO( ym2149 );
SND_GET_INFO( ym3439 );
SND_GET_INFO( ymz284 );
SND_GET_INFO( ymz294 );
SND_GET_INFO( ym2203 );
SND_GET_INFO( ym2151 );
SND_GET_INFO( ym2608 );
SND_GET_INFO( ym2610 );
SND_GET_INFO( ym2610b );
SND_GET_INFO( ym2612 );
SND_GET_INFO( ym3438 );
SND_GET_INFO( ym2413 );
SND_GET_INFO( ym3812 );
SND_GET_INFO( ym3526 );
SND_GET_INFO( ymz280b );
SND_GET_INFO( y8950 );
SND_GET_INFO( sn76477 );
SND_GET_INFO( sn76489 );
SND_GET_INFO( sn76489a );
SND_GET_INFO( sn76494 );
SND_GET_INFO( sn76496 );
SND_GET_INFO( gamegear );
SND_GET_INFO( smsiii );
SND_GET_INFO( pokey );
SND_GET_INFO( nesapu );
SND_GET_INFO( astrocade );
SND_GET_INFO( namco );
SND_GET_INFO( namco_15xx );
SND_GET_INFO( namco_cus30 );
SND_GET_INFO( namco_52xx );
SND_GET_INFO( namco_63701x );
SND_GET_INFO( snkwave );
SND_GET_INFO( tms36xx );
SND_GET_INFO( tms3615 );
SND_GET_INFO( tms5100 );
SND_GET_INFO( tms5110 );
SND_GET_INFO( tms5110a );
SND_GET_INFO( cd2801 );
SND_GET_INFO( tmc0281 );
SND_GET_INFO( cd2802 );
SND_GET_INFO( m58817 );
SND_GET_INFO( tmc0285 );
SND_GET_INFO( tms5200 );
SND_GET_INFO( tms5220 );
SND_GET_INFO( vlm5030 );
SND_GET_INFO( adpcm );
SND_GET_INFO( okim6295 );
SND_GET_INFO( okim6258 );
SND_GET_INFO( msm5205 );
SND_GET_INFO( msm5232 );
SND_GET_INFO( upd7759 );
SND_GET_INFO( hc55516 );
SND_GET_INFO( mc3417 );
SND_GET_INFO( mc3418 );
SND_GET_INFO( k005289 );
SND_GET_INFO( k007232 );
SND_GET_INFO( k051649 );
SND_GET_INFO( k053260 );
SND_GET_INFO( k054539 );
SND_GET_INFO( segapcm );
SND_GET_INFO( rf5c68 );
SND_GET_INFO( cem3394 );
SND_GET_INFO( c140 );
SND_GET_INFO( qsound );
SND_GET_INFO( saa1099 );
SND_GET_INFO( iremga20 );
SND_GET_INFO( es5503 );
SND_GET_INFO( es5505 );
SND_GET_INFO( es5506 );
SND_GET_INFO( bsmt2000 );
SND_GET_INFO( ymf262 );
SND_GET_INFO( ymf278b );
SND_GET_INFO( gaelco_cg1v );
SND_GET_INFO( gaelco_gae1 );
SND_GET_INFO( x1_010 );
SND_GET_INFO( multipcm );
SND_GET_INFO( c6280 );
SND_GET_INFO( tia );
SND_GET_INFO( sp0250 );
SND_GET_INFO( scsp );
SND_GET_INFO( psxspu );
SND_GET_INFO( ymf271 );
SND_GET_INFO( cdda );
SND_GET_INFO( ics2115 );
SND_GET_INFO( st0016 );
SND_GET_INFO( nile );
SND_GET_INFO( c352 );
SND_GET_INFO( vrender0 );
SND_GET_INFO( votrax );
SND_GET_INFO( es8712 );
SND_GET_INFO( rf5c400 );
SND_GET_INFO( speaker );
SND_GET_INFO( cdp1869 );
SND_GET_INFO( beep );
SND_GET_INFO( wave );
SND_GET_INFO( sid6581 );
SND_GET_INFO( sid8580 );
SND_GET_INFO( sp0256 );
SND_GET_INFO( s14001a );
SND_GET_INFO( aica );

SND_GET_INFO( filter_volume );
SND_GET_INFO( filter_rc );



/***************************************************************************
    MASTER SND LIST
***************************************************************************/

static const struct
{
	sound_type	sndtype;
	snd_get_info_func	get_info;
} sndintrf_map[] =
{
	{ SOUND_DUMMY, SND_GET_INFO_NAME( dummy_sound ) },
#if (HAS_CUSTOM)
	{ SOUND_CUSTOM, SND_GET_INFO_NAME( custom ) },
#endif
#if (HAS_SAMPLES)
	{ SOUND_SAMPLES, SND_GET_INFO_NAME( samples ) },
#endif
#if (HAS_DAC)
	{ SOUND_DAC, SND_GET_INFO_NAME( dac ) },
#endif
#if (HAS_DMADAC)
	{ SOUND_DMADAC, SND_GET_INFO_NAME( dmadac ) },
#endif
#if (HAS_DISCRETE)
	{ SOUND_DISCRETE, SND_GET_INFO_NAME( discrete ) },
#endif
#if (HAS_AY8910)
	{ SOUND_AY8910, SND_GET_INFO_NAME( ay8910 ) },
	{ SOUND_AY8912, SND_GET_INFO_NAME( ay8912 ) },
	{ SOUND_AY8913, SND_GET_INFO_NAME( ay8913 ) },
	{ SOUND_AY8930, SND_GET_INFO_NAME( ay8930 ) },
	{ SOUND_YM2149, SND_GET_INFO_NAME( ym2149 ) },
	{ SOUND_YM3439, SND_GET_INFO_NAME( ym3439 ) },
	{ SOUND_YMZ284, SND_GET_INFO_NAME( ymz284 ) },
	{ SOUND_YMZ294, SND_GET_INFO_NAME( ymz294 ) },
#endif
#if (HAS_YM2203)
	{ SOUND_YM2203, SND_GET_INFO_NAME( ym2203 ) },
#endif
#if (HAS_YM2151)
	{ SOUND_YM2151, SND_GET_INFO_NAME( ym2151 ) },
#endif
#if (HAS_YM2608)
	{ SOUND_YM2608, SND_GET_INFO_NAME( ym2608 ) },
#endif
#if (HAS_YM2610)
	{ SOUND_YM2610, SND_GET_INFO_NAME( ym2610 ) },
#endif
#if (HAS_YM2610B)
	{ SOUND_YM2610B, SND_GET_INFO_NAME( ym2610b ) },
#endif
#if (HAS_YM2612)
	{ SOUND_YM2612, SND_GET_INFO_NAME( ym2612 ) },
#endif
#if (HAS_YM3438)
	{ SOUND_YM3438, SND_GET_INFO_NAME( ym3438 ) },
#endif
#if (HAS_YM2413)
	{ SOUND_YM2413, SND_GET_INFO_NAME( ym2413 ) },
#endif
#if (HAS_YM3812)
	{ SOUND_YM3812, SND_GET_INFO_NAME( ym3812 ) },
#endif
#if (HAS_YM3526)
	{ SOUND_YM3526, SND_GET_INFO_NAME( ym3526 ) },
#endif
#if (HAS_YMZ280B)
	{ SOUND_YMZ280B, SND_GET_INFO_NAME( ymz280b ) },
#endif
#if (HAS_Y8950)
	{ SOUND_Y8950, SND_GET_INFO_NAME( y8950 ) },
#endif
#if (HAS_SN76477)
	{ SOUND_SN76477, SND_GET_INFO_NAME( sn76477 ) },
#endif
#if (HAS_SN76496)
	{ SOUND_SN76489, SND_GET_INFO_NAME( sn76489 ) },
	{ SOUND_SN76489A, SND_GET_INFO_NAME( sn76489a ) },
	{ SOUND_SN76494, SND_GET_INFO_NAME( sn76494 ) },
	{ SOUND_SN76496, SND_GET_INFO_NAME( sn76496 ) },
	{ SOUND_GAMEGEAR, SND_GET_INFO_NAME( gamegear ) },
	{ SOUND_SMSIII, SND_GET_INFO_NAME( smsiii ) },
#endif
#if (HAS_POKEY)
	{ SOUND_POKEY, SND_GET_INFO_NAME( pokey ) },
#endif
#if (HAS_NES)
	{ SOUND_NES, SND_GET_INFO_NAME( nesapu ) },
#endif
#if (HAS_ASTROCADE)
	{ SOUND_ASTROCADE, SND_GET_INFO_NAME( astrocade ) },
#endif
#if (HAS_NAMCO)
	{ SOUND_NAMCO, SND_GET_INFO_NAME( namco ) },
#endif
#if (HAS_NAMCO_15XX)
	{ SOUND_NAMCO_15XX, SND_GET_INFO_NAME( namco_15xx ) },
#endif
#if (HAS_NAMCO_CUS30)
	{ SOUND_NAMCO_CUS30, SND_GET_INFO_NAME( namco_cus30 ) },
#endif
#if (HAS_NAMCO_52XX)
	{ SOUND_NAMCO_52XX, SND_GET_INFO_NAME( namco_52xx ) },
#endif
#if (HAS_NAMCO_63701X)
	{ SOUND_NAMCO_63701X, SND_GET_INFO_NAME( namco_63701x ) },
#endif
#if (HAS_SNKWAVE)
	{ SOUND_SNKWAVE, SND_GET_INFO_NAME( snkwave ) },
#endif
#if (HAS_TMS36XX)
	{ SOUND_TMS36XX, SND_GET_INFO_NAME( tms36xx ) },
#endif
#if (HAS_TMS3615)
	{ SOUND_TMS3615, SND_GET_INFO_NAME( tms3615 ) },
#endif
#if (HAS_TMS5100)
	{ SOUND_TMS5100, SND_GET_INFO_NAME( tms5100 ) },
#endif
#if (HAS_TMS5110)
	{ SOUND_TMS5110, SND_GET_INFO_NAME( tms5110 ) },
#endif
#if (HAS_TMS5110A)
	{ SOUND_TMS5110A, SND_GET_INFO_NAME( tms5110a ) },
#endif
#if (HAS_CD2801)
	{ SOUND_CD2801, SND_GET_INFO_NAME( cd2801 ) },
#endif
#if (HAS_TMC0281)
	{ SOUND_TMC0281, SND_GET_INFO_NAME( tmc0281 ) },
#endif
#if (HAS_CD2802)
	{ SOUND_CD2802, SND_GET_INFO_NAME( cd2802 ) },
#endif
#if (HAS_M58817)
	{ SOUND_M58817, SND_GET_INFO_NAME( m58817 ) },
#endif
#if (HAS_TMC0285)
	{ SOUND_TMC0285, SND_GET_INFO_NAME( tmc0285 ) },
#endif
#if (HAS_TMS5200)
	{ SOUND_TMS5200, SND_GET_INFO_NAME( tms5200 ) },
#endif
#if (HAS_TMS5220)
	{ SOUND_TMS5220, SND_GET_INFO_NAME( tms5220 ) },
#endif
#if (HAS_VLM5030)
	{ SOUND_VLM5030, SND_GET_INFO_NAME( vlm5030 ) },
#endif
#if (HAS_OKIM6295)
	{ SOUND_OKIM6295, SND_GET_INFO_NAME( okim6295 ) },
#endif
#if (HAS_MSM5205)
	{ SOUND_MSM5205, SND_GET_INFO_NAME( msm5205 ) },
#endif
#if (HAS_MSM5232)
	{ SOUND_MSM5232, SND_GET_INFO_NAME( msm5232 ) },
#endif
#if (HAS_UPD7759)
	{ SOUND_UPD7759, SND_GET_INFO_NAME( upd7759 ) },
#endif
#if (HAS_HC55516)
	{ SOUND_HC55516, SND_GET_INFO_NAME( hc55516 ) },
	{ SOUND_MC3417, SND_GET_INFO_NAME( mc3417 ) },
	{ SOUND_MC3418, SND_GET_INFO_NAME( mc3418 ) },
#endif
#if (HAS_K005289)
	{ SOUND_K005289, SND_GET_INFO_NAME( k005289 ) },
#endif
#if (HAS_K007232)
	{ SOUND_K007232, SND_GET_INFO_NAME( k007232 ) },
#endif
#if (HAS_K051649)
	{ SOUND_K051649, SND_GET_INFO_NAME( k051649 ) },
#endif
#if (HAS_K053260)
	{ SOUND_K053260, SND_GET_INFO_NAME( k053260 ) },
#endif
#if (HAS_K054539)
	{ SOUND_K054539, SND_GET_INFO_NAME( k054539 ) },
#endif
#if (HAS_SEGAPCM)
	{ SOUND_SEGAPCM, SND_GET_INFO_NAME( segapcm ) },
#endif
#if (HAS_RF5C68)
	{ SOUND_RF5C68, SND_GET_INFO_NAME( rf5c68 ) },
#endif
#if (HAS_CEM3394)
	{ SOUND_CEM3394, SND_GET_INFO_NAME( cem3394 ) },
#endif
#if (HAS_C140)
	{ SOUND_C140, SND_GET_INFO_NAME( c140 ) },
#endif
#if (HAS_QSOUND)
	{ SOUND_QSOUND, SND_GET_INFO_NAME( qsound ) },
#endif
#if (HAS_SAA1099)
	{ SOUND_SAA1099, SND_GET_INFO_NAME( saa1099 ) },
#endif
#if (HAS_IREMGA20)
	{ SOUND_IREMGA20, SND_GET_INFO_NAME( iremga20 ) },
#endif
#if (HAS_ES5503)
	{ SOUND_ES5503, SND_GET_INFO_NAME( es5503 ) },
#endif
#if (HAS_ES5505)
	{ SOUND_ES5505, SND_GET_INFO_NAME( es5505 ) },
#endif
#if (HAS_ES5506)
	{ SOUND_ES5506, SND_GET_INFO_NAME( es5506 ) },
#endif
#if (HAS_BSMT2000)
	{ SOUND_BSMT2000, SND_GET_INFO_NAME( bsmt2000 ) },
#endif
#if (HAS_YMF262)
	{ SOUND_YMF262, SND_GET_INFO_NAME( ymf262 ) },
#endif
#if (HAS_YMF278B)
	{ SOUND_YMF278B, SND_GET_INFO_NAME( ymf278b ) },
#endif
#if (HAS_GAELCO_CG1V)
	{ SOUND_GAELCO_CG1V, SND_GET_INFO_NAME( gaelco_cg1v ) },
#endif
#if (HAS_GAELCO_GAE1)
	{ SOUND_GAELCO_GAE1, SND_GET_INFO_NAME( gaelco_gae1 ) },
#endif
#if (HAS_X1_010)
	{ SOUND_X1_010, SND_GET_INFO_NAME( x1_010 ) },
#endif
#if (HAS_MULTIPCM)
	{ SOUND_MULTIPCM, SND_GET_INFO_NAME( multipcm ) },
#endif
#if (HAS_C6280)
	{ SOUND_C6280, SND_GET_INFO_NAME( c6280 ) },
#endif
#if (HAS_TIA)
	{ SOUND_TIA, SND_GET_INFO_NAME( tia ) },
#endif
#if (HAS_SP0250)
	{ SOUND_SP0250, SND_GET_INFO_NAME( sp0250 ) },
#endif
#if (HAS_SCSP)
	{ SOUND_SCSP, SND_GET_INFO_NAME( scsp ) },
#endif
#if (HAS_PSXSPU)
	{ SOUND_PSXSPU, SND_GET_INFO_NAME( psxspu ) },
#endif
#if (HAS_YMF271)
	{ SOUND_YMF271, SND_GET_INFO_NAME( ymf271 ) },
#endif
#if (HAS_CDDA)
	{ SOUND_CDDA, SND_GET_INFO_NAME( cdda ) },
#endif
#if (HAS_ICS2115)
	{ SOUND_ICS2115, SND_GET_INFO_NAME( ics2115 ) },
#endif
#if (HAS_ST0016)
	{ SOUND_ST0016, SND_GET_INFO_NAME( st0016 ) },
#endif
#if (HAS_NILE)
	{ SOUND_NILE, SND_GET_INFO_NAME( nile ) },
#endif
#if (HAS_C352)
	{ SOUND_C352, SND_GET_INFO_NAME( c352 ) },
#endif
#if (HAS_VRENDER0)
	{ SOUND_VRENDER0, SND_GET_INFO_NAME( vrender0 ) },
#endif
#if (HAS_VOTRAX)
	{ SOUND_VOTRAX, SND_GET_INFO_NAME( votrax ) },
#endif
#if (HAS_ES8712)
	{ SOUND_ES8712, SND_GET_INFO_NAME( es8712 ) },
#endif
#if (HAS_RF5C400)
	{ SOUND_RF5C400, SND_GET_INFO_NAME( rf5c400 ) },
#endif
#if (HAS_SPEAKER)
	{ SOUND_SPEAKER, SND_GET_INFO_NAME( speaker ) },
#endif
#if (HAS_CDP1869)
	{ SOUND_CDP1869, SND_GET_INFO_NAME( cdp1869 ) },
#endif
#if (HAS_S14001A)
	{ SOUND_S14001A, SND_GET_INFO_NAME( s14001a ) },
#endif
#if (HAS_BEEP)
	{ SOUND_BEEP, SND_GET_INFO_NAME( beep ) },
#endif
#if (HAS_WAVE)
	{ SOUND_WAVE, SND_GET_INFO_NAME( wave ) },
#endif
#if (HAS_SID6581)
	{ SOUND_SID6581, SND_GET_INFO_NAME( sid6581 ) },
#endif
#if (HAS_SID8580)
	{ SOUND_SID8580, SND_GET_INFO_NAME( sid8580 ) },
#endif
#if (HAS_SP0256)
	{ SOUND_SP0256, SND_GET_INFO_NAME( sp0256 ) },
#endif
#if (HAS_AICA)
	{ SOUND_AICA, SND_GET_INFO_NAME( aica ) },
#endif

	{ SOUND_FILTER_VOLUME, SND_GET_INFO_NAME( filter_volume ) },
	{ SOUND_FILTER_RC, SND_GET_INFO_NAME( filter_rc ) },
};



/***************************************************************************
    VALIDATION MACROS
***************************************************************************/

#define VERIFY_SNDNUM(name) \
	assert_always(sndnum >= 0 && sndnum < totalsnd, #name "() called with invalid sound num!")

#define VERIFY_SNDTI(name) \
	assert_always(sndtype >= 0 && sndtype < SOUND_COUNT, #name "() called with invalid sound type!"); \
	assert_always(sndindex >= 0 && sndindex < totalsnd && sound_matrix[sndtype][sndindex] != 0, #name "() called with invalid (type,index) pair!")

#define VERIFY_SNDTYPE(name) \
	assert_always(sndtype >= 0 && sndtype < SOUND_COUNT, #name "() called with invalid sound type!")



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static snd_class_header snd_type_header[SOUND_COUNT];

static sndintrf_data sound[MAX_SOUND];
static sndintrf_data *current_sound_start;
static UINT8 sound_matrix[SOUND_COUNT][MAX_SOUND];
static int totalsnd;



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    sndintrf_init - discover all linked sound
    systems and build a matrix for sound
    (type,index) pairs for the current machine
-------------------------------------------------*/

void sndintrf_init(running_machine *machine)
{
	int mapindex;

	/* reset the sndintrf array */
	memset(snd_type_header, 0, sizeof(snd_type_header));

	/* build the sndintrf array */
	for (mapindex = 0; mapindex < ARRAY_LENGTH(sndintrf_map); mapindex++)
	{
		sound_type sndtype = sndintrf_map[mapindex].sndtype;
		snd_class_header *header = &snd_type_header[sndtype];
		sndinfo info;

		/* start with the get_info routine */
		header->sndtype = sndtype;
		header->get_info = sndintrf_map[mapindex].get_info;

		/* bootstrap the rest of the function pointers */
		info.set_info = NULL;
		(*header->get_info)(NULL, SNDINFO_PTR_SET_INFO, &info);
		header->set_info = info.set_info;

		info.start = NULL;
		(*header->get_info)(NULL, SNDINFO_PTR_START, &info);
		header->start = info.start;

		info.stop = NULL;
		(*header->get_info)(NULL, SNDINFO_PTR_STOP, &info);
		header->stop = info.stop;

		info.reset = NULL;
		(*header->get_info)(NULL, SNDINFO_PTR_RESET, &info);
		header->reset = info.reset;
	}

	/* fill in any empty entries with the dummy sound */
	for (mapindex = 0; mapindex < SOUND_COUNT; mapindex++)
		if (snd_type_header[mapindex].get_info == NULL)
			snd_type_header[mapindex] = snd_type_header[SOUND_DUMMY];

	/* zap the sound data structures */
	memset(sound, 0, sizeof(sound));
	totalsnd = 0;

	/* reset the (type,index) matrix */
	memset(sound_matrix, 0, sizeof(sound_matrix));
}


/*-------------------------------------------------
    sndintrf_init_sound - initialize data for a
    particular sndnum
-------------------------------------------------*/

static DEVICE_GET_INFO( sndclass )
{
	sndintrf_data *snddata = device->inline_config;
	(*snddata->intf.get_info)(device, state, (sndinfo *)info);
}

int sndintrf_init_sound(running_machine *machine, int sndnum, const char *tag, sound_type sndtype, int clock, const void *config)
{
	sndintrf_data *info = &sound[sndnum];
	int index;

	info->device = auto_malloc(sizeof(*info->device) + strlen(tag));
	memset(info->device, 0, sizeof(*info->device) + strlen(tag));
	info->device->type = DEVICE_GET_INFO_NAME(sndclass);
	info->device->machine = machine;
	strcpy(info->device->tag, tag);
	info->device->static_config = config;
	info->device->region = memory_region(info->device->machine, info->device->tag);
	info->device->regionbytes = memory_region_length(info->device->machine, info->device->tag);

	/* hack: stash the info pointer in the inline_config */
	info->device->inline_config = info;

	/* fill in the type and interface */
	info->intf = snd_type_header[sndtype];
	info->tag = tag;
	info->sndtype = sndtype;
	info->aliastype = sndtype_get_info_int(sndtype, SNDINFO_INT_ALIAS);
	if (info->aliastype == 0)
		info->aliastype = sndtype;
	info->clock = clock;

	/* find an empty slot in the matrix and add it */
	totalsnd++;
	for (index = 0; index < MAX_SOUND; index++)
		if (sound_matrix[info->aliastype][index] == 0)
		{
			sound_matrix[info->aliastype][index] = totalsnd;
			break;
		}
	info->index = index;

	/* start the chip, tagging all its streams */
	current_sound_start = &sound[sndnum];
	info->device->token = (*info->intf.start)(info->device, clock, config, info->index);
	current_sound_start = NULL;
	VPRINTF(("  token = %p\n", info->device->token));

	/* if that failed, die */
	if (info->device->token == NULL)
		return 1;

	return 0;
}


/*-------------------------------------------------
    sndintrf_exit_sound - tear down data for a
    particular sndnum
-------------------------------------------------*/

void sndintrf_exit_sound(int sndnum)
{
	/* stop the chip */
	if (sound[sndnum].intf.stop)
		(*sound[sndnum].intf.stop)(sound[sndnum].device);
}



/***************************************************************************
    HELPERS
***************************************************************************/

/*-------------------------------------------------
    sndintrf_register_token - register a token
    from within the sound_start routine
-------------------------------------------------*/

void sndintrf_register_token(void *token)
{
	if (current_sound_start)
		current_sound_start->device->token = token;
}


/*-------------------------------------------------
    sndti_exists - return TRUE if a (type,index)
    pair describes an existing chip
-------------------------------------------------*/

int sndti_exists(sound_type type, int index)
{
	return (sound_matrix[type][index] != 0);
}


/*-------------------------------------------------
    sndti_to_sndnum - map a (type,index) pair to
    a sound number
-------------------------------------------------*/

int sndti_to_sndnum(sound_type type, int index)
{
	return sound_matrix[type][index] - 1;
}


/*-------------------------------------------------
    sndnum_to_sndti - map a sound number to a
    (type,index) pair
-------------------------------------------------*/

sound_type sndnum_to_sndti(int sndnum, int *index)
{
	if (index != NULL)
		*index = sound[sndnum].index;
	return sound[sndnum].aliastype;
}


/*-------------------------------------------------
    sndtype_count - count the number of a
    given type
-------------------------------------------------*/

int sndtype_count(sound_type sndtype)
{
	int index;
	int count = 0;

	for (index = 0; index < totalsnd; index++)
		if (sound[index].sndtype == sndtype)
			count++;

	return count;
}



/***************************************************************************
    CHIP INTERFACES BY INDEX
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndnum_get_info_int(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_int);
	info.i = 0;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.i;
}

void *sndnum_get_info_ptr(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_ptr);
	info.p = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.p;
}

genf *sndnum_get_info_fct(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_fct);
	info.f = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.f;
}

const char *sndnum_get_info_string(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_string);
	info.s = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.s;
}


/*-------------------------------------------------
    Set info accessors
-------------------------------------------------*/

void sndnum_set_info_int(int sndnum, UINT32 state, INT64 data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_int);
	info.i = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndnum_set_info_ptr(int sndnum, UINT32 state, void *data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_ptr);
	info.p = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndnum_set_info_fct(int sndnum, UINT32 state, genf *data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_ptr);
	info.f = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}


/*-------------------------------------------------
    Misc accessors
-------------------------------------------------*/

void sndnum_reset(int sndnum)
{
	VERIFY_SNDNUM(sndnum_reset);
	if (sound[sndnum].intf.reset)
		(*sound[sndnum].intf.reset)(sound[sndnum].device);
}

int sndnum_clock(int sndnum)
{
	VERIFY_SNDNUM(sndnum_clock);
	return sound[sndnum].clock;
}

void *sndnum_token(int sndnum)
{
	VERIFY_SNDNUM(sndnum_token);
	return sound[sndnum].device->token;
}



/***************************************************************************
    CHIP INTERFACES BY (TYPE,INDEX) PAIR
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndti_get_info_int(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_int);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.i = 0;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.i;
}

void *sndti_get_info_ptr(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_ptr);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.p = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.p;
}

genf *sndti_get_info_fct(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_fct);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.f = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.f;
}

const char *sndti_get_info_string(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_string);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.s = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.s;
}


/*-------------------------------------------------
    Set info accessors
-------------------------------------------------*/

void sndti_set_info_int(sound_type sndtype, int sndindex, UINT32 state, INT64 data)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_set_info_int);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.i = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndti_set_info_ptr(sound_type sndtype, int sndindex, UINT32 state, void *data)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_set_info_ptr);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.p = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndti_set_info_fct(sound_type sndtype, int sndindex, UINT32 state, genf *data)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_set_info_ptr);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.f = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}


/*-------------------------------------------------
    Misc accessors
-------------------------------------------------*/

void sndti_reset(sound_type sndtype, int sndindex)
{
	int sndnum;

	VERIFY_SNDTI(sndti_reset);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	if (sound[sndnum].intf.reset)
		(*sound[sndnum].intf.reset)(sound[sndnum].device);
}

int sndti_clock(sound_type sndtype, int sndindex)
{
	int sndnum;
	VERIFY_SNDTI(sndti_clock);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	return sound[sndnum].clock;
}

void *sndti_token(sound_type sndtype, int sndindex)
{
	int sndnum;
	VERIFY_SNDTI(sndti_token);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	return sound[sndnum].device->token;
}



/***************************************************************************
    CHIP INTERFACES BY TYPE
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndtype_get_info_int(sound_type sndtype, UINT32 state)
{
	snd_class_header *classheader = &snd_type_header[sndtype];
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_int);
	info.i = 0;
	(*classheader->get_info)(NULL, state, &info);
	return info.i;
}

void *sndtype_get_info_ptr(sound_type sndtype, UINT32 state)
{
	snd_class_header *classheader = &snd_type_header[sndtype];
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_ptr);
	info.p = NULL;
	(*classheader->get_info)(NULL, state, &info);
	return info.p;
}

genf *sndtype_get_info_fct(sound_type sndtype, UINT32 state)
{
	snd_class_header *classheader = &snd_type_header[sndtype];
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_fct);
	info.f = NULL;
	(*classheader->get_info)(NULL, state, &info);
	return info.f;
}

const char *sndtype_get_info_string(sound_type sndtype, UINT32 state)
{
	snd_class_header *classheader = &snd_type_header[sndtype];
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_string);
	info.s = NULL;
	(*classheader->get_info)(NULL, state, &info);
	return info.s;
}



/***************************************************************************
    DUMMY INTERFACES
***************************************************************************/

static SND_START( dummy_sound )
{
	logerror("Warning: starting a dummy sound core -- you are missing a hookup in sndintrf.c!\n");
	return auto_malloc(1);
}


static SND_SET_INFO( dummy_sound )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


static SND_GET_INFO( dummy_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( dummy_sound );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( dummy_sound );		break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Dummy";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Dummy";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
