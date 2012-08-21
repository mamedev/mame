/*****************************************************************************
 *
 * includes/special.h
 *
 ****************************************************************************/

#ifndef SPECIAL_H_
#define SPECIAL_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "formats/rk_cas.h"
#include "formats/smx_dsk.h"
#include "machine/wd17xx.h"
#include "machine/ram.h"


class special_state : public driver_device
{
public:
	special_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ppi(*this, "ppi8255"),
	m_fdc(*this, "wd1793"),
	m_dac(*this, "dac"),
	m_cass(*this, CASSETTE_TAG),
	m_ram(*this, RAM_TAG),
	m_p_videoram(*this, "p_videoram")
	{ }

	DECLARE_WRITE8_MEMBER(specimx_select_bank);
	DECLARE_WRITE8_MEMBER(video_memory_w);
	DECLARE_WRITE8_MEMBER(specimx_video_color_w);
	DECLARE_READ8_MEMBER(specimx_video_color_r);
	DECLARE_READ8_MEMBER(specimx_disk_ctrl_r);
	DECLARE_WRITE8_MEMBER(specimx_disk_ctrl_w);
	DECLARE_READ8_MEMBER(erik_rr_reg_r);
	DECLARE_WRITE8_MEMBER(erik_rr_reg_w);
	DECLARE_READ8_MEMBER(erik_rc_reg_r);
	DECLARE_WRITE8_MEMBER(erik_rc_reg_w);
	DECLARE_READ8_MEMBER(erik_disk_reg_r);
	DECLARE_WRITE8_MEMBER(erik_disk_reg_w);
	DECLARE_READ8_MEMBER(specialist_8255_porta_r);
	DECLARE_READ8_MEMBER(specialist_8255_portb_r);
	DECLARE_READ8_MEMBER(specialist_8255_portc_r);
	DECLARE_WRITE8_MEMBER(specialist_8255_porta_w);
	DECLARE_WRITE8_MEMBER(specialist_8255_portb_w);
	DECLARE_WRITE8_MEMBER(specialist_8255_portc_w);
	DECLARE_WRITE_LINE_MEMBER(specimx_pit8253_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(specimx_pit8253_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(specimx_pit8253_out2_changed);
	void specimx_set_bank(offs_t i, UINT8 data);
	void erik_set_bank();
	UINT8 *m_specimx_colorram;
	UINT8 m_erik_color_1;
	UINT8 m_erik_color_2;
	UINT8 m_erik_background;
	UINT8 m_specimx_color;
	device_t *m_specimx_audio;
	int m_specialist_8255_porta;
	int m_specialist_8255_portb;
	int m_specialist_8255_portc;
	UINT8 m_RR_register;
	UINT8 m_RC_register;
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device> m_ppi;
	optional_device<device_t> m_fdc;
	optional_device<dac_device> m_dac;
	optional_device<cassette_image_device> m_cass;
	optional_device<ram_device> m_ram;
	optional_shared_ptr<UINT8> m_p_videoram;
	DECLARE_DRIVER_INIT(erik);
	DECLARE_DRIVER_INIT(special);
};


/*----------- defined in machine/special.c -----------*/

extern const struct pit8253_config specimx_pit8253_intf;
extern const i8255_interface specialist_ppi8255_interface;

MACHINE_RESET( special );

MACHINE_RESET( specimx );
MACHINE_START ( specimx );

MACHINE_RESET( erik );

/*----------- defined in video/special.c -----------*/

VIDEO_START( special );
SCREEN_UPDATE_IND16( special );

VIDEO_START( specialp );
SCREEN_UPDATE_IND16( specialp );

VIDEO_START( specimx );
SCREEN_UPDATE_IND16( specimx );

VIDEO_START( erik );
SCREEN_UPDATE_IND16( erik );
PALETTE_INIT( erik );

PALETTE_INIT( specimx );
extern const rgb_t specimx_palette[16];

/*----------- defined in audio/special.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(SPECIMX, specimx_sound);

void specimx_set_input(device_t *device, int index, int state);

#endif /* SPECIAL_H_ */
