/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "sound/discrete.h"


#define VICDUAL_MASTER_CLOCK				(15468480)
#define VICDUAL_MAIN_CPU_CLOCK				(VICDUAL_MASTER_CLOCK/8)
#define VICDUAL_PIXEL_CLOCK					(VICDUAL_MASTER_CLOCK/3)
#define VICDUAL_HTOTAL						(0x148)
#define VICDUAL_HBEND						(0x000)
#define VICDUAL_HBSTART						(0x100)
#define VICDUAL_HSSTART						(0x110)
#define VICDUAL_HSEND						(0x130)
#define VICDUAL_VTOTAL						(0x106)
#define VICDUAL_VBEND						(0x000)
#define VICDUAL_VBSTART						(0x0e0)
#define VICDUAL_VSSTART						(0x0ec)
#define VICDUAL_VSEND						(0x0f0)


class vicdual_state : public driver_device
{
public:
	vicdual_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_characterram(*this, "characterram"){ }

	UINT32 m_coin_status;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_characterram;
	UINT8 m_samurai_protection_data;
	UINT8 m_palette_bank;
	DECLARE_WRITE8_MEMBER(vicdual_videoram_w);
	DECLARE_WRITE8_MEMBER(vicdual_characterram_w);
	DECLARE_READ8_MEMBER(depthch_io_r);
	DECLARE_WRITE8_MEMBER(depthch_io_w);
	DECLARE_READ8_MEMBER(safari_io_r);
	DECLARE_WRITE8_MEMBER(safari_io_w);
	DECLARE_READ8_MEMBER(frogs_io_r);
	DECLARE_WRITE8_MEMBER(frogs_io_w);
	DECLARE_READ8_MEMBER(headon_io_r);
	DECLARE_READ8_MEMBER(sspaceat_io_r);
	DECLARE_WRITE8_MEMBER(headon_io_w);
	DECLARE_READ8_MEMBER(headon2_io_r);
	DECLARE_WRITE8_MEMBER(headon2_io_w);
	DECLARE_WRITE8_MEMBER(digger_io_w);
	DECLARE_WRITE8_MEMBER(invho2_io_w);
	DECLARE_WRITE8_MEMBER(invds_io_w);
	DECLARE_WRITE8_MEMBER(sspacaho_io_w);
	DECLARE_WRITE8_MEMBER(tranqgun_io_w);
	DECLARE_WRITE8_MEMBER(spacetrk_io_w);
	DECLARE_WRITE8_MEMBER(carnival_io_w);
	DECLARE_WRITE8_MEMBER(brdrline_io_w);
	DECLARE_WRITE8_MEMBER(pulsar_io_w);
	DECLARE_WRITE8_MEMBER(heiankyo_io_w);
	DECLARE_WRITE8_MEMBER(alphaho_io_w);
	DECLARE_WRITE8_MEMBER(samurai_protection_w);
	DECLARE_WRITE8_MEMBER(samurai_io_w);
	DECLARE_READ8_MEMBER(nsub_io_r);
	DECLARE_WRITE8_MEMBER(nsub_io_w);
	DECLARE_READ8_MEMBER(invinco_io_r);
	DECLARE_WRITE8_MEMBER(invinco_io_w);
	DECLARE_WRITE8_MEMBER(vicdual_palette_bank_w);
	DECLARE_CUSTOM_INPUT_MEMBER(vicdual_read_coin_status);
	DECLARE_CUSTOM_INPUT_MEMBER(vicdual_get_64v);
	DECLARE_CUSTOM_INPUT_MEMBER(vicdual_get_vblank_comp);
	DECLARE_CUSTOM_INPUT_MEMBER(vicdual_get_composite_blank_comp);
	DECLARE_CUSTOM_INPUT_MEMBER(vicdual_get_timer_value);
	DECLARE_CUSTOM_INPUT_MEMBER(brdrline_lives);
	DECLARE_CUSTOM_INPUT_MEMBER(samurai_protection_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);
	DECLARE_MACHINE_START(frogs_audio);
	UINT32 screen_update_vicdual_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vicdual_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vicdual_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*----------- defined in drivers/vicdual.c -----------*/

int vicdual_is_cabinet_color(running_machine &machine);


/*----------- defined in video/vicdual.c -----------*/







/*----------- defined in audio/vicdual.c -----------*/


MACHINE_CONFIG_EXTERN( frogs_audio );
MACHINE_CONFIG_EXTERN( headon_audio );
DECLARE_WRITE8_HANDLER( frogs_audio_w );
DECLARE_WRITE8_HANDLER( headon_audio_w );
DECLARE_WRITE8_HANDLER( invho2_audio_w );


/*----------- defined in audio/depthch.c -----------*/

MACHINE_CONFIG_EXTERN( depthch_audio );
DECLARE_WRITE8_HANDLER( depthch_audio_w );


/*----------- defined in audio/carnival.c -----------*/

MACHINE_CONFIG_EXTERN( carnival_audio );
DECLARE_WRITE8_HANDLER( carnival_audio_1_w );
DECLARE_WRITE8_HANDLER( carnival_audio_2_w );


/*----------- defined in audio/invinco.c -----------*/

MACHINE_CONFIG_EXTERN( invinco_audio );
DECLARE_WRITE8_HANDLER( invinco_audio_w );


/*----------- defined in audio/pulsar.c -----------*/

MACHINE_CONFIG_EXTERN( pulsar_audio );
DECLARE_WRITE8_HANDLER( pulsar_audio_1_w );
DECLARE_WRITE8_HANDLER( pulsar_audio_2_w );
