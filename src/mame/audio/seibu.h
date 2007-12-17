/***************************************************************************

    Seibu Sound System v1.02, games using this include:

    Cross Shooter    1987   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Cabal            1988   * "Michel/Seibu    sound 11/04/88" (YM2151 substituted for YM3812, unknown ADPCM)
    Dead Angle       1988   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (2xYM2203 substituted for YM3812, unknown ADPCM)
    Dynamite Duke    1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden           1990   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Blood Brothers   1990     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    D-Con            1992     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    Related sound programs (not implemented yet):

    Zero Team                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Legionaire                "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Raiden 2                  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Raiden DX                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Cup Soccer                "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    SD Gundam Psycho Salamander "Copyright by King Bee Sol 1991"
    * = encrypted

***************************************************************************/

#include "sound/3812intf.h"
#include "sound/2151intf.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "sound/custom.h"

ADDRESS_MAP_EXTERN(seibu_sound_readmem);
ADDRESS_MAP_EXTERN(seibu_sound_writemem);
ADDRESS_MAP_EXTERN(seibu2_sound_readmem);
ADDRESS_MAP_EXTERN(seibu2_sound_writemem);
ADDRESS_MAP_EXTERN(seibu3_sound_readmem);
ADDRESS_MAP_EXTERN(seibu3_sound_writemem);
ADDRESS_MAP_EXTERN(seibu3_adpcm_sound_writemem);

READ16_HANDLER( seibu_main_word_r );
READ8_HANDLER( seibu_main_v30_r );
WRITE16_HANDLER( seibu_main_word_w );
WRITE8_HANDLER( seibu_main_v30_w );

WRITE16_HANDLER( seibu_main_mustb_w );

WRITE8_HANDLER( seibu_irq_clear_w );
WRITE8_HANDLER( seibu_rst10_ack_w );
WRITE8_HANDLER( seibu_rst18_ack_w );
WRITE8_HANDLER( seibu_bank_w );
WRITE8_HANDLER( seibu_coin_w );
void seibu_ym3812_irqhandler(int linestate);
void seibu_ym2151_irqhandler(int linestate);
void seibu_ym2203_irqhandler(int linestate);
READ8_HANDLER( seibu_soundlatch_r );
READ8_HANDLER( seibu_main_data_pending_r );
WRITE8_HANDLER( seibu_main_data_w );
MACHINE_RESET( seibu_sound_1 );
MACHINE_RESET( seibu_sound_2 );
void seibu_sound_decrypt(int cpu_region,int length);

void *seibu_adpcm_start(int clock, const struct CustomSound_interface *config);
void seibu_adpcm_stop(void *token);
void seibu_adpcm_decrypt(int region);
WRITE8_HANDLER( seibu_adpcm_adr_1_w );
WRITE8_HANDLER( seibu_adpcm_ctl_1_w );
WRITE8_HANDLER( seibu_adpcm_adr_2_w );
WRITE8_HANDLER( seibu_adpcm_ctl_2_w );

/**************************************************************************/

#define SEIBU_COIN_INPUTS											\
	PORT_START														\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(4)			\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(4)

#define SEIBU_SOUND_SYSTEM_YM3812_HARDWARE							\
																	\
static struct YM3812interface ym3812_interface =					\
{																	\
	seibu_ym3812_irqhandler											\
};

#define SEIBU_SOUND_SYSTEM_ADPCM_HARDWARE							\
																	\
static struct CustomSound_interface adpcm_interface =				\
{																	\
	seibu_adpcm_start,												\
	seibu_adpcm_stop												\
};

#define SEIBU_SOUND_SYSTEM_YM2151_HARDWARE							\
																	\
static struct YM2151interface ym2151_interface =					\
{																	\
	seibu_ym2151_irqhandler											\
};

#define SEIBU_SOUND_SYSTEM_YM2203_HARDWARE							\
																	\
static struct YM2203interface ym2203_interface =					\
{																	\
	0,0,0,0,seibu_ym2203_irqhandler									\
};

#define SEIBU_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD(Z80, freq)											\
	/* audio CPU */									\
	MDRV_CPU_PROGRAM_MAP(seibu_sound_readmem,seibu_sound_writemem)		\

#define SEIBU2_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD(Z80, freq)											\
	/* audio CPU */									\
	MDRV_CPU_PROGRAM_MAP(seibu2_sound_readmem,seibu2_sound_writemem)		\

#define SEIBU3_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD(Z80, freq)											\
	/* audio CPU */									\
	MDRV_CPU_PROGRAM_MAP(seibu3_sound_readmem,seibu3_sound_writemem)		\

#define SEIBU3A_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD(Z80, freq)											\
	/* audio CPU */									\
	MDRV_CPU_PROGRAM_MAP(seibu3_sound_readmem,seibu3_adpcm_sound_writemem)		\

#define SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(freq1,freq2,region)		\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD(YM3812, freq1)									\
	MDRV_SOUND_CONFIG(ym3812_interface)								\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)						\
																	\
	MDRV_SOUND_ADD(OKIM6295, freq2)									\
	MDRV_SOUND_CONFIG(okim6295_interface_region_##region##_pin7low)	\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\

#define SEIBU_SOUND_SYSTEM_YM3812_RAIDEN_INTERFACE(freq1,freq2,region) \
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD(YM3812, freq1)									\
	MDRV_SOUND_CONFIG(ym3812_interface)								\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)						\
																	\
	MDRV_SOUND_ADD(OKIM6295, freq2)									\
	MDRV_SOUND_CONFIG(okim6295_interface_region_##region##_pin7low)	\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)						\

#define SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(freq1,freq2,region)		\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD(YM2151, freq1)									\
	MDRV_SOUND_CONFIG(ym2151_interface)								\
	MDRV_SOUND_ROUTE(0, "mono", 0.50)								\
	MDRV_SOUND_ROUTE(1, "mono", 0.50)								\
																	\
	MDRV_SOUND_ADD(OKIM6295, freq2)									\
	MDRV_SOUND_CONFIG(okim6295_interface_region_##region##_pin7low)	\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\

#define SEIBU_SOUND_SYSTEM_YM2203_INTERFACE(freq)					\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD(YM2203, freq)									\
	MDRV_SOUND_CONFIG(ym2203_interface)								\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)						\
																	\
	MDRV_SOUND_ADD(YM2203, freq)									\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)						\

#define SEIBU_SOUND_SYSTEM_ADPCM_INTERFACE							\
	MDRV_SOUND_ADD(CUSTOM, 8000)	 								\
	MDRV_SOUND_CONFIG(adpcm_interface)								\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40) 					\
																	\
	MDRV_SOUND_ADD(CUSTOM, 8000) 									\
	MDRV_SOUND_CONFIG(adpcm_interface)								\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\


/**************************************************************************/

