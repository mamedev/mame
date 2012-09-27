/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "sound/discrete.h"


class vicdual_state : public driver_device
{
public:
	vicdual_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_coinstate_timer(*this, "coinstate"),
		m_nsub_coinage_timer(*this, "nsub_coin"),
		m_videoram(*this, "videoram"),
		m_characterram(*this, "characterram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_coinstate_timer;
	optional_device<timer_device> m_nsub_coinage_timer;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_characterram;

	UINT8 m_coin_status;
	UINT8 m_palette_bank;
	UINT8 m_samurai_protection_data;
	int m_nsub_coin_counter;
	int m_nsub_play_counter;

	void coin_in();
	void assert_coin_status();
	
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
	DECLARE_CUSTOM_INPUT_MEMBER(vicdual_fake_lives_r);
	DECLARE_CUSTOM_INPUT_MEMBER(samurai_protection_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);
	DECLARE_INPUT_CHANGED_MEMBER(nsub_coin_in);

	TIMER_DEVICE_CALLBACK_MEMBER(clear_coin_status);
	TIMER_DEVICE_CALLBACK_MEMBER(nsub_coin_pulse);

	DECLARE_MACHINE_START(samurai);
	DECLARE_MACHINE_START(nsub);
	DECLARE_MACHINE_RESET(nsub);
	DECLARE_MACHINE_START(frogs_audio);

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update_vicdual_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vicdual_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vicdual_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/*----------- defined in drivers/vicdual.c -----------*/

int vicdual_is_cabinet_color(running_machine &machine);

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
