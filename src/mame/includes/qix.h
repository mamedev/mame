/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/sn76496.h"
#include "sound/discrete.h"

#define MAIN_CLOCK_OSC			20000000	/* 20 MHz */
#define SLITHER_CLOCK_OSC		21300000	/* 21.3 MHz */
#define SOUND_CLOCK_OSC			7372800		/* 7.3728 MHz */
#define COIN_CLOCK_OSC			4000000		/* 4 MHz */
#define QIX_CHARACTER_CLOCK		(20000000/2/16)


#define NUM_PENS	(0x100)

class qix_state : public driver_device
{
public:
	qix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sn1 (*this, "sn1"),
		m_sn2 (*this, "sn2"),
		m_68705_port_out(*this, "68705_port_out"),
		m_68705_ddr(*this, "68705_ddr"),
		m_videoram(*this, "videoram"),
		m_videoram_address(*this, "videoram_addr"),
		m_videoram_mask(*this, "videoram_mask"),
		m_paletteram(*this, "paletteram"),
		m_scanline_latch(*this, "scanline_latch"),
		m_discrete(*this, "discrete") { }

	/* devices */
	optional_device<sn76489_device> m_sn1;
	optional_device<sn76489_device> m_sn2;

	/* machine state */
	optional_shared_ptr<UINT8> m_68705_port_out;
	optional_shared_ptr<UINT8> m_68705_ddr;
	UINT8  m_68705_port_in[3];
	UINT8  m_coinctrl;

	/* video state */
	optional_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram_address;
	optional_shared_ptr<UINT8> m_videoram_mask;
	required_shared_ptr<UINT8> m_paletteram;
	UINT8  m_flip;
	UINT8  m_palette_bank;
	UINT8  m_leds;
	required_shared_ptr<UINT8> m_scanline_latch;
	optional_device<discrete_device> m_discrete;
	pen_t m_pens[NUM_PENS];
	DECLARE_WRITE8_MEMBER(zookeep_bankswitch_w);
	DECLARE_WRITE8_MEMBER(qix_data_firq_w);
	DECLARE_WRITE8_MEMBER(qix_data_firq_ack_w);
	DECLARE_READ8_MEMBER(qix_data_firq_r);
	DECLARE_READ8_MEMBER(qix_data_firq_ack_r);
	DECLARE_WRITE8_MEMBER(qix_video_firq_w);
	DECLARE_WRITE8_MEMBER(qix_video_firq_ack_w);
	DECLARE_READ8_MEMBER(qix_video_firq_r);
	DECLARE_READ8_MEMBER(qix_video_firq_ack_r);
	DECLARE_READ8_MEMBER(qix_68705_portA_r);
	DECLARE_READ8_MEMBER(qix_68705_portB_r);
	DECLARE_READ8_MEMBER(qix_68705_portC_r);
	DECLARE_WRITE8_MEMBER(qix_68705_portA_w);
	DECLARE_WRITE8_MEMBER(qix_68705_portB_w);
	DECLARE_WRITE8_MEMBER(qix_68705_portC_w);
	DECLARE_READ8_MEMBER(qix_videoram_r);
	DECLARE_WRITE8_MEMBER(qix_videoram_w);
	DECLARE_WRITE8_MEMBER(slither_videoram_w);
	DECLARE_READ8_MEMBER(qix_addresslatch_r);
	DECLARE_WRITE8_MEMBER(qix_addresslatch_w);
	DECLARE_WRITE8_MEMBER(slither_addresslatch_w);
	DECLARE_WRITE8_MEMBER(qix_paletteram_w);
	DECLARE_WRITE8_MEMBER(qix_palettebank_w);
	DECLARE_DRIVER_INIT(slither);
	DECLARE_DRIVER_INIT(zookeep);
	DECLARE_DRIVER_INIT(kram3);
	virtual void machine_reset();
	DECLARE_MACHINE_START(qixmcu);
	DECLARE_VIDEO_START(qix);
	TIMER_CALLBACK_MEMBER(pia_w_callback);
	DECLARE_WRITE_LINE_MEMBER(qix_vsync_changed);
	DECLARE_READ8_MEMBER(qixmcu_coin_r);
	DECLARE_WRITE8_MEMBER(qixmcu_coin_w);
	DECLARE_WRITE8_MEMBER(qixmcu_coinctrl_w);
	DECLARE_WRITE8_MEMBER(qix_pia_w);
	DECLARE_WRITE8_MEMBER(qix_coinctl_w);
	DECLARE_WRITE8_MEMBER(slither_76489_0_w);
	DECLARE_WRITE8_MEMBER(slither_76489_1_w);
	DECLARE_READ8_MEMBER(slither_trak_lr_r);
	DECLARE_READ8_MEMBER(slither_trak_ud_r);
	DECLARE_WRITE_LINE_MEMBER(display_enable_changed);
	DECLARE_WRITE8_MEMBER(qix_flip_screen_w);
	DECLARE_WRITE8_MEMBER(qix_dac_w);
	DECLARE_WRITE8_MEMBER(qix_vol_w);
	DECLARE_WRITE8_MEMBER(sndpia_2_warning_w);
	DECLARE_WRITE8_MEMBER(sync_sndpia1_porta_w);
	DECLARE_WRITE8_MEMBER(slither_coinctl_w);
	DECLARE_WRITE_LINE_MEMBER(qix_pia_dint);
	DECLARE_WRITE_LINE_MEMBER(qix_pia_sint);
};


/*----------- defined in machine/qix.c -----------*/

extern const pia6821_interface qix_pia_0_intf;
extern const pia6821_interface qix_pia_1_intf;
extern const pia6821_interface qix_pia_2_intf;
extern const pia6821_interface qixmcu_pia_0_intf;
extern const pia6821_interface qixmcu_pia_2_intf;
extern const pia6821_interface slither_pia_1_intf;
extern const pia6821_interface slither_pia_2_intf;

/*----------- defined in video/qix.c -----------*/

MACHINE_CONFIG_EXTERN( qix_video );
MACHINE_CONFIG_EXTERN( zookeep_video );
MACHINE_CONFIG_EXTERN( slither_video );

/*----------- defined in audio/qix.c -----------*/

MACHINE_CONFIG_EXTERN( qix_audio );
MACHINE_CONFIG_EXTERN( slither_audio );
