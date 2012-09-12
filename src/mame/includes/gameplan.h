/***************************************************************************

GAME PLAN driver

driver by Chris Moore

***************************************************************************/

#include "machine/6522via.h"

#define GAMEPLAN_MAIN_MASTER_CLOCK       (XTAL_3_579545MHz)
#define GAMEPLAN_AUDIO_MASTER_CLOCK      (XTAL_3_579545MHz)
#define GAMEPLAN_MAIN_CPU_CLOCK          (GAMEPLAN_MAIN_MASTER_CLOCK / 4)
#define GAMEPLAN_AUDIO_CPU_CLOCK         (GAMEPLAN_AUDIO_MASTER_CLOCK / 4)
#define GAMEPLAN_AY8910_CLOCK            (GAMEPLAN_AUDIO_MASTER_CLOCK / 2)
#define GAMEPLAN_PIXEL_CLOCK             (XTAL_11_6688MHz / 2)

/* Used Leprechaun/Pot of Gold (and Pirate Treasure) - as stated in manual for Pot Of Gold */

#define LEPRECHAUN_MAIN_MASTER_CLOCK     (XTAL_4MHz)
#define LEPRECHAUN_MAIN_CPU_CLOCK        (LEPRECHAUN_MAIN_MASTER_CLOCK / 4)


class gameplan_state : public driver_device
{
public:
	gameplan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_trvquest_question(*this, "trvquest_q"),
		  m_via_0(*this, "via6522_0"),
		  m_via_1(*this, "via6522_1"),
		  m_via_2(*this, "via6522_2") { }

	/* machine state */
	UINT8   m_current_port;
	optional_shared_ptr<UINT8> m_trvquest_question;

	/* video state */
	UINT8   *m_videoram;
	size_t   m_videoram_size;
	UINT8    m_video_x;
	UINT8    m_video_y;
	UINT8    m_video_command;
	UINT8    m_video_data;
	emu_timer *m_via_0_ca1_timer;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_riot;
	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;
	required_device<via6522_device> m_via_2;
	DECLARE_WRITE8_MEMBER(io_select_w);
	DECLARE_READ8_MEMBER(io_port_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(audio_reset_w);
	DECLARE_WRITE8_MEMBER(audio_cmd_w);
	DECLARE_WRITE8_MEMBER(audio_trigger_w);
	DECLARE_WRITE_LINE_MEMBER(r6532_irq);
	DECLARE_WRITE8_MEMBER(r6532_soundlatch_w);
};


/*----------- defined in video/gameplan.c -----------*/

extern const via6522_interface gameplan_via_0_interface;
extern const via6522_interface leprechn_via_0_interface;
extern const via6522_interface trvquest_via_0_interface;

MACHINE_CONFIG_EXTERN( gameplan_video );
MACHINE_CONFIG_EXTERN( leprechn_video );
MACHINE_CONFIG_EXTERN( trvquest_video );
