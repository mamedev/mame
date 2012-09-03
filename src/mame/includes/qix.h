/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/sn76496.h"


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
		m_scanline_latch(*this, "scanline_latch") { }

	/* devices */
	optional_device<sn76489_new_device> m_sn1;
	optional_device<sn76489_new_device> m_sn2;
	
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
};


/*----------- defined in machine/qix.c -----------*/

extern const pia6821_interface qix_pia_0_intf;
extern const pia6821_interface qix_pia_1_intf;
extern const pia6821_interface qix_pia_2_intf;
extern const pia6821_interface qixmcu_pia_0_intf;
extern const pia6821_interface qixmcu_pia_2_intf;
extern const pia6821_interface slither_pia_1_intf;
extern const pia6821_interface slither_pia_2_intf;


MACHINE_START( qixmcu );
MACHINE_RESET( qix );





WRITE8_DEVICE_HANDLER( qix_pia_w );

WRITE_LINE_DEVICE_HANDLER( qix_vsync_changed );


/*----------- defined in video/qix.c -----------*/

MACHINE_CONFIG_EXTERN( qix_video );
MACHINE_CONFIG_EXTERN( zookeep_video );
MACHINE_CONFIG_EXTERN( slither_video );

WRITE8_DEVICE_HANDLER( qix_flip_screen_w );


/*----------- defined in audio/qix.c -----------*/

MACHINE_CONFIG_EXTERN( qix_audio );
MACHINE_CONFIG_EXTERN( slither_audio );
