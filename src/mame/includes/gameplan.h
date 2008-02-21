/***************************************************************************

GAME PLAN driver

driver by Chris Moore

***************************************************************************/


#define GAMEPLAN_MAIN_MASTER_CLOCK		(XTAL_3_579545MHz)
#define GAMEPLAN_AUDIO_MASTER_CLOCK		(XTAL_3_579545MHz)
#define GAMEPLAN_MAIN_CPU_CLOCK			(GAMEPLAN_MAIN_MASTER_CLOCK / 4)
#define GAMEPLAN_AUDIO_CPU_CLOCK		(GAMEPLAN_AUDIO_MASTER_CLOCK / 4)
#define GAMEPLAN_AY8910_CLOCK			(GAMEPLAN_AUDIO_MASTER_CLOCK / 2)
#define GAMEPLAN_PIXEL_CLOCK			(XTAL_11_6688MHz / 2)


typedef struct _gameplan_state gameplan_state;
struct _gameplan_state
{
	/* machine state */
	UINT8   current_port;
	UINT8   audio_cmd;
	UINT8  *trvquest_question;

	/* video state */
	UINT8  *videoram;
	size_t  videoram_size;
	UINT8   video_x;
	UINT8   video_y;
	UINT8   video_command;
	UINT8   video_data;
	emu_timer *via_0_ca1_timer;
};


/*----------- defined in video/gameplan.c -----------*/

MACHINE_DRIVER_EXTERN( gameplan_video );
MACHINE_DRIVER_EXTERN( leprechn_video );
MACHINE_DRIVER_EXTERN( trvquest_video );
