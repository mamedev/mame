// license:???
// copyright-holders:John Butler, Ed Mueller, Aaron Giles
/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/sn76496.h"
#include "sound/discrete.h"

#define MAIN_CLOCK_OSC          20000000    /* 20 MHz */
#define SLITHER_CLOCK_OSC       21300000    /* 21.3 MHz */
#define SOUND_CLOCK_OSC         7372800     /* 7.3728 MHz */
#define COIN_CLOCK_OSC          4000000     /* 4 MHz */
#define QIX_CHARACTER_CLOCK     (20000000/2/16)


class qix_state : public driver_device
{
public:
	qix_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_videocpu(*this, "videocpu"),
		m_mcu(*this, "mcu"),
		m_crtc(*this, "vid_u18"),
		m_pia0(*this, "pia0"),
		m_pia1(*this, "pia1"),
		m_pia2(*this, "pia2"),
		m_sndpia0(*this, "sndpia0"),
		m_sndpia1(*this, "sndpia1"),
		m_sndpia2(*this, "sndpia2"),
		m_sn1 (*this, "sn1"),
		m_sn2 (*this, "sn2"),
		m_discrete(*this, "discrete"),
		m_68705_port_out(*this, "68705_port_out"),
		m_68705_ddr(*this, "68705_ddr"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_videoram_address(*this, "videoram_addr"),
		m_videoram_mask(*this, "videoram_mask"),
		m_scanline_latch(*this, "scanline_latch"),
		m_bank0(*this, "bank0"),
		m_bank1(*this, "bank1"),
		m_screen(*this, "screen") { }

	/* devices */
	required_device<m6809_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<m6809_base_device> m_videocpu;
	optional_device<cpu_device> m_mcu;
	required_device<mc6845_device> m_crtc;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<pia6821_device> m_sndpia0;
	optional_device<pia6821_device> m_sndpia1;
	optional_device<pia6821_device> m_sndpia2;
	optional_device<sn76489_device> m_sn1;
	optional_device<sn76489_device> m_sn2;
	optional_device<discrete_device> m_discrete;

	/* machine state */
	optional_shared_ptr<UINT8> m_68705_port_out;
	optional_shared_ptr<UINT8> m_68705_ddr;
	UINT8  m_68705_port_in[3];
	UINT8  m_coinctrl;

	/* video state */
	required_shared_ptr<UINT8> m_paletteram;
	optional_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram_address;
	optional_shared_ptr<UINT8> m_videoram_mask;
	required_shared_ptr<UINT8> m_scanline_latch;
	UINT8  m_flip;
	UINT8  m_palette_bank;
	UINT8  m_leds;

	optional_memory_bank m_bank0;
	optional_memory_bank m_bank1;
	required_device<screen_device> m_screen;

	pen_t m_pens[0x400];
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
	virtual void machine_reset() override;
	DECLARE_MACHINE_START(qixmcu);
	DECLARE_VIDEO_START(qix);
	TIMER_CALLBACK_MEMBER(pia_w_callback);
	TIMER_CALLBACK_MEMBER(deferred_sndpia1_porta_w);
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
	DECLARE_WRITE_LINE_MEMBER(qix_flip_screen_w);
	DECLARE_WRITE8_MEMBER(qix_dac_w);
	DECLARE_WRITE8_MEMBER(qix_vol_w);
	DECLARE_WRITE8_MEMBER(sndpia_2_warning_w);
	DECLARE_WRITE8_MEMBER(sync_sndpia1_porta_w);
	DECLARE_WRITE8_MEMBER(slither_coinctl_w);
	DECLARE_WRITE_LINE_MEMBER(qix_pia_dint);
	DECLARE_WRITE_LINE_MEMBER(qix_pia_sint);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);
	void set_pen(int offs);
	int kram3_permut1(int idx, int value);
	int kram3_permut2(int tbl_index, int idx, const UINT8 *xor_table);
	int kram3_decrypt(int address, int value);
	DECLARE_WRITE_LINE_MEMBER(kram3_lic_maincpu_changed);
	DECLARE_WRITE_LINE_MEMBER(kram3_lic_videocpu_changed);
};

/*----------- defined in video/qix.c -----------*/

MACHINE_CONFIG_EXTERN( qix_video );
MACHINE_CONFIG_EXTERN( kram3_video );
MACHINE_CONFIG_EXTERN( zookeep_video );
MACHINE_CONFIG_EXTERN( slither_video );

/*----------- defined in audio/qix.c -----------*/

MACHINE_CONFIG_EXTERN( qix_audio );
MACHINE_CONFIG_EXTERN( slither_audio );
