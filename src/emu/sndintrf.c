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

typedef struct _sound_interface sound_interface;
struct _sound_interface
{
	/* table of core functions */
	void		(*get_info)(void *token, UINT32 state, sndinfo *info);
	void		(*set_info)(void *token, UINT32 state, sndinfo *info);
	void * 		(*start)(int index, int clock, const void *config);
	void		(*stop)(void *token);
	void		(*reset)(void *token);
};


typedef struct _sndintrf_data sndintrf_data;
struct _sndintrf_data
{
	sound_interface	intf;	 		/* copy of the interface data */
	sound_type			sndtype; 		/* type index of this sound chip */
	sound_type			aliastype;		/* aliased type index of this sound chip */
	int				index; 			/* index of this sound chip */
	int				clock; 			/* clock for this sound chip */
	void *			token;			/* dynamically allocated token data */
};



/***************************************************************************
    EXTERNAL PROTOTYPES
***************************************************************************/

static void dummy_sound_get_info(void *token, UINT32 state, sndinfo *info);
void custom_get_info(void *token, UINT32 state, sndinfo *info);
void samples_get_info(void *token, UINT32 state, sndinfo *info);
void dac_get_info(void *token, UINT32 state, sndinfo *info);
void dmadac_get_info(void *token, UINT32 state, sndinfo *info);
void discrete_get_info(void *token, UINT32 state, sndinfo *info);
void ay8910_get_info(void *token, UINT32 state, sndinfo *info);
void ay8912_get_info(void *token, UINT32 state, sndinfo *info);
void ay8913_get_info(void *token, UINT32 state, sndinfo *info);
void ay8930_get_info(void *token, UINT32 state, sndinfo *info);
void ym2149_get_info(void *token, UINT32 state, sndinfo *info);
void ym3439_get_info(void *token, UINT32 state, sndinfo *info);
void ymz284_get_info(void *token, UINT32 state, sndinfo *info);
void ymz294_get_info(void *token, UINT32 state, sndinfo *info);
void ym2203_get_info(void *token, UINT32 state, sndinfo *info);
void ym2151_get_info(void *token, UINT32 state, sndinfo *info);
void ym2608_get_info(void *token, UINT32 state, sndinfo *info);
void ym2610_get_info(void *token, UINT32 state, sndinfo *info);
void ym2610b_get_info(void *token, UINT32 state, sndinfo *info);
void ym2612_get_info(void *token, UINT32 state, sndinfo *info);
void ym3438_get_info(void *token, UINT32 state, sndinfo *info);
void ym2413_get_info(void *token, UINT32 state, sndinfo *info);
void ym3812_get_info(void *token, UINT32 state, sndinfo *info);
void ym3526_get_info(void *token, UINT32 state, sndinfo *info);
void ymz280b_get_info(void *token, UINT32 state, sndinfo *info);
void y8950_get_info(void *token, UINT32 state, sndinfo *info);
void sn76477_get_info(void *token, UINT32 state, sndinfo *info);
void sn76489_get_info(void *token, UINT32 state, sndinfo *info);
void sn76489a_get_info(void *token, UINT32 state, sndinfo *info);
void sn76494_get_info(void *token, UINT32 state, sndinfo *info);
void sn76496_get_info(void *token, UINT32 state, sndinfo *info);
void gamegear_get_info(void *token, UINT32 state, sndinfo *info);
void smsiii_get_info(void *token, UINT32 state, sndinfo *info);
void pokey_get_info(void *token, UINT32 state, sndinfo *info);
void nesapu_get_info(void *token, UINT32 state, sndinfo *info);
void astrocade_get_info(void *token, UINT32 state, sndinfo *info);
void namco_get_info(void *token, UINT32 state, sndinfo *info);
void namco_15xx_get_info(void *token, UINT32 state, sndinfo *info);
void namco_cus30_get_info(void *token, UINT32 state, sndinfo *info);
void namco_52xx_get_info(void *token, UINT32 state, sndinfo *info);
void namco_63701x_get_info(void *token, UINT32 state, sndinfo *info);
void namcona_get_info(void *token, UINT32 state, sndinfo *info);
void tms36xx_get_info(void *token, UINT32 state, sndinfo *info);
void tms3615_get_info(void *token, UINT32 state, sndinfo *info);
void tms5110_get_info(void *token, UINT32 state, sndinfo *info);
void tms5220_get_info(void *token, UINT32 state, sndinfo *info);
void vlm5030_get_info(void *token, UINT32 state, sndinfo *info);
void adpcm_get_info(void *token, UINT32 state, sndinfo *info);
void okim6295_get_info(void *token, UINT32 state, sndinfo *info);
void msm5205_get_info(void *token, UINT32 state, sndinfo *info);
void msm5232_get_info(void *token, UINT32 state, sndinfo *info);
void upd7759_get_info(void *token, UINT32 state, sndinfo *info);
void hc55516_get_info(void *token, UINT32 state, sndinfo *info);
void k005289_get_info(void *token, UINT32 state, sndinfo *info);
void k007232_get_info(void *token, UINT32 state, sndinfo *info);
void k051649_get_info(void *token, UINT32 state, sndinfo *info);
void k053260_get_info(void *token, UINT32 state, sndinfo *info);
void k054539_get_info(void *token, UINT32 state, sndinfo *info);
void segapcm_get_info(void *token, UINT32 state, sndinfo *info);
void rf5c68_get_info(void *token, UINT32 state, sndinfo *info);
void cem3394_get_info(void *token, UINT32 state, sndinfo *info);
void c140_get_info(void *token, UINT32 state, sndinfo *info);
void qsound_get_info(void *token, UINT32 state, sndinfo *info);
void saa1099_get_info(void *token, UINT32 state, sndinfo *info);
void iremga20_get_info(void *token, UINT32 state, sndinfo *info);
void es5503_get_info(void *token, UINT32 state, sndinfo *info);
void es5505_get_info(void *token, UINT32 state, sndinfo *info);
void es5506_get_info(void *token, UINT32 state, sndinfo *info);
void bsmt2000_get_info(void *token, UINT32 state, sndinfo *info);
void ymf262_get_info(void *token, UINT32 state, sndinfo *info);
void ymf278b_get_info(void *token, UINT32 state, sndinfo *info);
void gaelco_cg1v_get_info(void *token, UINT32 state, sndinfo *info);
void gaelco_gae1_get_info(void *token, UINT32 state, sndinfo *info);
void x1_010_get_info(void *token, UINT32 state, sndinfo *info);
void multipcm_get_info(void *token, UINT32 state, sndinfo *info);
void c6280_get_info(void *token, UINT32 state, sndinfo *info);
void tia_get_info(void *token, UINT32 state, sndinfo *info);
void sp0250_get_info(void *token, UINT32 state, sndinfo *info);
void scsp_get_info(void *token, UINT32 state, sndinfo *info);
void psxspu_get_info(void *token, UINT32 state, sndinfo *info);
void ymf271_get_info(void *token, UINT32 state, sndinfo *info);
void cdda_get_info(void *token, UINT32 state, sndinfo *info);
void ics2115_get_info(void *token, UINT32 state, sndinfo *info);
void st0016_get_info(void *token, UINT32 state, sndinfo *info);
void c352_get_info(void *token, UINT32 state, sndinfo *info);
void vrender0_get_info(void *token, UINT32 state, sndinfo *info);
void votrax_get_info(void *token, UINT32 state, sndinfo *info);
void es8712_get_info(void *token, UINT32 state, sndinfo *info);
void rf5c400_get_info(void *token, UINT32 state, sndinfo *info);
void speaker_get_info(void *token, UINT32 state, sndinfo *info);
void cdp1869_get_info(void *token, UINT32 state, sndinfo *info);
void beep_get_info(void *token, UINT32 state, sndinfo *info);
void wave_get_info(void *token, UINT32 state, sndinfo *info);
void sid6581_get_info(void *token, UINT32 state, sndinfo *info);
void sid8580_get_info(void *token, UINT32 state, sndinfo *info);
void sp0256_get_info(void *token, UINT32 state, sndinfo *info);
void s14001a_get_info(void *token, UINT32 state, sndinfo *info);

void filter_volume_get_info(void *token, UINT32 state, sndinfo *info);
void filter_rc_get_info(void *token, UINT32 state, sndinfo *info);



/***************************************************************************
    CORE INTERFACE LIST
***************************************************************************/

static sound_interface sndintrf[SOUND_COUNT];

static const struct
{
	sound_type	sndtype;
	void	(*get_info)(void *token, UINT32 state, sndinfo *info);
} sndintrf_map[] =
{
	{ SOUND_DUMMY, dummy_sound_get_info },
#if (HAS_CUSTOM)
	{ SOUND_CUSTOM, custom_get_info },
#endif
#if (HAS_SAMPLES)
	{ SOUND_SAMPLES, samples_get_info },
#endif
#if (HAS_DAC)
	{ SOUND_DAC, dac_get_info },
#endif
#if (HAS_DMADAC)
	{ SOUND_DMADAC, dmadac_get_info },
#endif
#if (HAS_DISCRETE)
	{ SOUND_DISCRETE, discrete_get_info },
#endif
#if (HAS_AY8910)
	{ SOUND_AY8910, ay8910_get_info },
	{ SOUND_AY8912, ay8912_get_info },
	{ SOUND_AY8913, ay8913_get_info },
	{ SOUND_AY8930, ay8930_get_info },
	{ SOUND_YM2149, ym2149_get_info },
	{ SOUND_YM3439, ym3439_get_info },
	{ SOUND_YMZ284, ymz284_get_info },
	{ SOUND_YMZ294, ymz294_get_info },
#endif
#if (HAS_YM2203)
	{ SOUND_YM2203, ym2203_get_info },
#endif
#if (HAS_YM2151)
	{ SOUND_YM2151, ym2151_get_info },
#endif
#if (HAS_YM2608)
	{ SOUND_YM2608, ym2608_get_info },
#endif
#if (HAS_YM2610)
	{ SOUND_YM2610, ym2610_get_info },
#endif
#if (HAS_YM2610B)
	{ SOUND_YM2610B, ym2610b_get_info },
#endif
#if (HAS_YM2612)
	{ SOUND_YM2612, ym2612_get_info },
#endif
#if (HAS_YM3438)
	{ SOUND_YM3438, ym3438_get_info },
#endif
#if (HAS_YM2413)
	{ SOUND_YM2413, ym2413_get_info },
#endif
#if (HAS_YM3812)
	{ SOUND_YM3812, ym3812_get_info },
#endif
#if (HAS_YM3526)
	{ SOUND_YM3526, ym3526_get_info },
#endif
#if (HAS_YMZ280B)
	{ SOUND_YMZ280B, ymz280b_get_info },
#endif
#if (HAS_Y8950)
	{ SOUND_Y8950, y8950_get_info },
#endif
#if (HAS_SN76477)
	{ SOUND_SN76477, sn76477_get_info },
#endif
#if (HAS_SN76496)
	{ SOUND_SN76489, sn76489_get_info },
	{ SOUND_SN76489A, sn76489a_get_info },
	{ SOUND_SN76494, sn76494_get_info },
	{ SOUND_SN76496, sn76496_get_info },
	{ SOUND_GAMEGEAR, gamegear_get_info },
	{ SOUND_SMSIII, smsiii_get_info },
#endif
#if (HAS_POKEY)
	{ SOUND_POKEY, pokey_get_info },
#endif
#if (HAS_NES)
	{ SOUND_NES, nesapu_get_info },
#endif
#if (HAS_ASTROCADE)
	{ SOUND_ASTROCADE, astrocade_get_info },
#endif
#if (HAS_NAMCO)
	{ SOUND_NAMCO, namco_get_info },
#endif
#if (HAS_NAMCO_15XX)
	{ SOUND_NAMCO_15XX, namco_15xx_get_info },
#endif
#if (HAS_NAMCO_CUS30)
	{ SOUND_NAMCO_CUS30, namco_cus30_get_info },
#endif
#if (HAS_NAMCO_52XX)
	{ SOUND_NAMCO_52XX, namco_52xx_get_info },
#endif
#if (HAS_NAMCO_63701X)
	{ SOUND_NAMCO_63701X, namco_63701x_get_info },
#endif
#if (HAS_NAMCONA)
	{ SOUND_NAMCONA, namcona_get_info },
#endif
#if (HAS_TMS36XX)
	{ SOUND_TMS36XX, tms36xx_get_info },
#endif
#if (HAS_TMS3615)
	{ SOUND_TMS3615, tms3615_get_info },
#endif
#if (HAS_TMS5110)
	{ SOUND_TMS5110, tms5110_get_info },
#endif
#if (HAS_TMS5220)
	{ SOUND_TMS5220, tms5220_get_info },
#endif
#if (HAS_VLM5030)
	{ SOUND_VLM5030, vlm5030_get_info },
#endif
#if (HAS_OKIM6295)
	{ SOUND_OKIM6295, okim6295_get_info },
#endif
#if (HAS_MSM5205)
	{ SOUND_MSM5205, msm5205_get_info },
#endif
#if (HAS_MSM5232)
	{ SOUND_MSM5232, msm5232_get_info },
#endif
#if (HAS_UPD7759)
	{ SOUND_UPD7759, upd7759_get_info },
#endif
#if (HAS_HC55516)
	{ SOUND_HC55516, hc55516_get_info },
#endif
#if (HAS_K005289)
	{ SOUND_K005289, k005289_get_info },
#endif
#if (HAS_K007232)
	{ SOUND_K007232, k007232_get_info },
#endif
#if (HAS_K051649)
	{ SOUND_K051649, k051649_get_info },
#endif
#if (HAS_K053260)
	{ SOUND_K053260, k053260_get_info },
#endif
#if (HAS_K054539)
	{ SOUND_K054539, k054539_get_info },
#endif
#if (HAS_SEGAPCM)
	{ SOUND_SEGAPCM, segapcm_get_info },
#endif
#if (HAS_RF5C68)
	{ SOUND_RF5C68, rf5c68_get_info },
#endif
#if (HAS_CEM3394)
	{ SOUND_CEM3394, cem3394_get_info },
#endif
#if (HAS_C140)
	{ SOUND_C140, c140_get_info },
#endif
#if (HAS_QSOUND)
	{ SOUND_QSOUND, qsound_get_info },
#endif
#if (HAS_SAA1099)
	{ SOUND_SAA1099, saa1099_get_info },
#endif
#if (HAS_IREMGA20)
	{ SOUND_IREMGA20, iremga20_get_info },
#endif
#if (HAS_ES5503)
	{ SOUND_ES5503, es5503_get_info },
#endif
#if (HAS_ES5505)
	{ SOUND_ES5505, es5505_get_info },
#endif
#if (HAS_ES5506)
	{ SOUND_ES5506, es5506_get_info },
#endif
#if (HAS_BSMT2000)
	{ SOUND_BSMT2000, bsmt2000_get_info },
#endif
#if (HAS_YMF262)
	{ SOUND_YMF262, ymf262_get_info },
#endif
#if (HAS_YMF278B)
	{ SOUND_YMF278B, ymf278b_get_info },
#endif
#if (HAS_GAELCO_CG1V)
	{ SOUND_GAELCO_CG1V, gaelco_cg1v_get_info },
#endif
#if (HAS_GAELCO_GAE1)
	{ SOUND_GAELCO_GAE1, gaelco_gae1_get_info },
#endif
#if (HAS_X1_010)
	{ SOUND_X1_010, x1_010_get_info },
#endif
#if (HAS_MULTIPCM)
	{ SOUND_MULTIPCM, multipcm_get_info },
#endif
#if (HAS_C6280)
	{ SOUND_C6280, c6280_get_info },
#endif
#if (HAS_TIA)
	{ SOUND_TIA, tia_get_info },
#endif
#if (HAS_SP0250)
	{ SOUND_SP0250, sp0250_get_info },
#endif
#if (HAS_SCSP)
	{ SOUND_SCSP, scsp_get_info },
#endif
#if (HAS_PSXSPU)
	{ SOUND_PSXSPU, psxspu_get_info },
#endif
#if (HAS_YMF271)
	{ SOUND_YMF271, ymf271_get_info },
#endif
#if (HAS_CDDA)
	{ SOUND_CDDA, cdda_get_info },
#endif
#if (HAS_ICS2115)
	{ SOUND_ICS2115, ics2115_get_info },
#endif
#if (HAS_ST0016)
	{ SOUND_ST0016, st0016_get_info },
#endif
#if (HAS_C352)
	{ SOUND_C352, c352_get_info },
#endif
#if (HAS_VRENDER0)
	{ SOUND_VRENDER0, vrender0_get_info },
#endif
#if (HAS_VOTRAX)
	{ SOUND_VOTRAX, votrax_get_info },
#endif
#if (HAS_ES8712)
	{ SOUND_ES8712, es8712_get_info },
#endif
#if (HAS_RF5C400)
	{ SOUND_RF5C400, rf5c400_get_info },
#endif
#if (HAS_SPEAKER)
	{ SOUND_SPEAKER, speaker_get_info },
#endif
#if (HAS_CDP1869)
	{ SOUND_CDP1869, cdp1869_get_info },
#endif
#if (HAS_S14001A)
	{ SOUND_S14001A, s14001a_get_info },
#endif
#if (HAS_BEEP)
	{ SOUND_BEEP, beep_get_info },
#endif
#if (HAS_WAVE)
	{ SOUND_WAVE, wave_get_info },
#endif
#if (HAS_SID6581)
	{ SOUND_SID6581, sid6581_get_info },
#endif
#if (HAS_SID8580)
	{ SOUND_SID8580, sid8580_get_info },
#endif
#if (HAS_SP0256)
	{ SOUND_SP0256, sp0256_get_info },
#endif

	{ SOUND_FILTER_VOLUME, filter_volume_get_info },
	{ SOUND_FILTER_RC, filter_rc_get_info },
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
    (type,index) pairs for the current Machine
-------------------------------------------------*/

void sndintrf_init(running_machine *machine)
{
	int mapindex;

	/* reset the sndintrf array */
	memset(sndintrf, 0, sizeof(sndintrf));

	/* build the sndintrf array */
	for (mapindex = 0; mapindex < sizeof(sndintrf_map) / sizeof(sndintrf_map[0]); mapindex++)
	{
		sound_type sndtype = sndintrf_map[mapindex].sndtype;
		sound_interface *intf = &sndintrf[sndtype];
		sndinfo info;

		/* start with the get_info routine */
		intf->get_info = sndintrf_map[mapindex].get_info;

		/* bootstrap the rest of the function pointers */
		info.set_info = NULL;
		(*intf->get_info)(NULL, SNDINFO_PTR_SET_INFO, &info);
		intf->set_info = info.set_info;

		info.start = NULL;
		(*intf->get_info)(NULL, SNDINFO_PTR_START, &info);
		intf->start = info.start;

		info.stop = NULL;
		(*intf->get_info)(NULL, SNDINFO_PTR_STOP, &info);
		intf->stop = info.stop;

		info.reset = NULL;
		(*intf->get_info)(NULL, SNDINFO_PTR_RESET, &info);
		intf->reset = info.reset;
	}

	/* fill in any empty entries with the dummy sound */
	for (mapindex = 0; mapindex < SOUND_COUNT; mapindex++)
		if (sndintrf[mapindex].get_info == NULL)
			sndintrf[mapindex] = sndintrf[SOUND_DUMMY];

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

int sndintrf_init_sound(int sndnum, sound_type sndtype, int clock, const void *config)
{
	sndintrf_data *info = &sound[sndnum];
	int index;

	/* fill in the type and interface */
	info->intf = sndintrf[sndtype];
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
	info->token = (*info->intf.start)(index, clock, config);
	current_sound_start = NULL;
	VPRINTF(("  token = %p\n", info->token));

	/* if that failed, die */
	if (!info->token)
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
		(*sound[sndnum].intf.stop)(sound[sndnum].token);
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
		current_sound_start->token = token;
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
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
	return info.i;
}

void *sndnum_get_info_ptr(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_ptr);
	info.p = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
	return info.p;
}

genf *sndnum_get_info_fct(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_fct);
	info.f = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
	return info.f;
}

const char *sndnum_get_info_string(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_string);
	info.s = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
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
	(*sound[sndnum].intf.set_info)(sound[sndnum].token, state, &info);
}

void sndnum_set_info_ptr(int sndnum, UINT32 state, void *data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_ptr);
	info.p = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].token, state, &info);
}

void sndnum_set_info_fct(int sndnum, UINT32 state, genf *data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_ptr);
	info.f = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].token, state, &info);
}


/*-------------------------------------------------
    Misc accessors
-------------------------------------------------*/

void sndnum_reset(int sndnum)
{
	VERIFY_SNDNUM(sndnum_reset);
	if (sound[sndnum].intf.reset)
		(*sound[sndnum].intf.reset)(sound[sndnum].token);
}

int sndnum_clock(int sndnum)
{
	VERIFY_SNDNUM(sndnum_clock);
	return sound[sndnum].clock;
}

void *sndnum_token(int sndnum)
{
	VERIFY_SNDNUM(sndnum_token);
	return sound[sndnum].token;
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
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
	return info.i;
}

void *sndti_get_info_ptr(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_ptr);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.p = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
	return info.p;
}

genf *sndti_get_info_fct(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_fct);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.f = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
	return info.f;
}

const char *sndti_get_info_string(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_get_info_string);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.s = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].token, state, &info);
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
	(*sound[sndnum].intf.set_info)(sound[sndnum].token, state, &info);
}

void sndti_set_info_ptr(sound_type sndtype, int sndindex, UINT32 state, void *data)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_set_info_ptr);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.p = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].token, state, &info);
}

void sndti_set_info_fct(sound_type sndtype, int sndindex, UINT32 state, genf *data)
{
	sndinfo info;
	int sndnum;

	VERIFY_SNDTI(sndti_set_info_ptr);
	sndnum = sound_matrix[sndtype][sndindex] - 1;
	info.f = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].token, state, &info);
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
		(*sound[sndnum].intf.reset)(sound[sndnum].token);
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
	return sound[sndnum].token;
}



/***************************************************************************
    CHIP INTERFACES BY TYPE
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndtype_get_info_int(sound_type sndtype, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_int);
	info.i = 0;
	(*sndintrf[sndtype].get_info)(NULL, state, &info);
	return info.i;
}

void *sndtype_get_info_ptr(sound_type sndtype, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_ptr);
	info.p = NULL;
	(*sndintrf[sndtype].get_info)(NULL, state, &info);
	return info.p;
}

genf *sndtype_get_info_fct(sound_type sndtype, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_fct);
	info.f = NULL;
	(*sndintrf[sndtype].get_info)(NULL, state, &info);
	return info.f;
}

const char *sndtype_get_info_string(sound_type sndtype, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_string);
	info.s = NULL;
	(*sndintrf[sndtype].get_info)(NULL, state, &info);
	return info.s;
}



/***************************************************************************
    DUMMY INTERFACES
***************************************************************************/

static void *dummy_sound_start(int index, int clock, const void *config)
{
	logerror("Warning: starting a dummy sound core -- you are missing a hookup in sndintrf.c!\n");
	return auto_malloc(1);
}


static void dummy_sound_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


static void dummy_sound_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = dummy_sound_set_info;	break;
		case SNDINFO_PTR_START:							info->start = dummy_sound_start;		break;
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
