// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * includes/lviv.h
 *
 ****************************************************************************/

#ifndef LVIV_H_
#define LVIV_H_

#include "imagedev/snapquik.h"
#include "machine/i8255.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

class lviv_state : public driver_device
{
public:
	lviv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette")  { }

	unsigned char * m_video_ram;
	unsigned short m_colortable[1][4];
	UINT8 m_ppi_port_outputs[2][3];
	UINT8 m_startup_mem_map;
	DECLARE_READ8_MEMBER(lviv_io_r);
	DECLARE_WRITE8_MEMBER(lviv_io_w);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(lviv);
	UINT32 screen_update_lviv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(lviv_ppi_0_porta_r);
	DECLARE_READ8_MEMBER(lviv_ppi_0_portb_r);
	DECLARE_READ8_MEMBER(lviv_ppi_0_portc_r);
	DECLARE_WRITE8_MEMBER(lviv_ppi_0_porta_w);
	DECLARE_WRITE8_MEMBER(lviv_ppi_0_portb_w);
	DECLARE_WRITE8_MEMBER(lviv_ppi_0_portc_w);
	DECLARE_READ8_MEMBER(lviv_ppi_1_porta_r);
	DECLARE_READ8_MEMBER(lviv_ppi_1_portb_r);
	DECLARE_READ8_MEMBER(lviv_ppi_1_portc_r);
	DECLARE_WRITE8_MEMBER(lviv_ppi_1_porta_w);
	DECLARE_WRITE8_MEMBER(lviv_ppi_1_portb_w);
	DECLARE_WRITE8_MEMBER(lviv_ppi_1_portc_w);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	void lviv_update_palette(UINT8 pal);
	void lviv_update_memory ();
	void lviv_setup_snapshot (UINT8 * data);
	void dump_registers();
	int lviv_verify_snapshot (UINT8 * data, UINT32 size);
	DECLARE_SNAPSHOT_LOAD_MEMBER( lviv );
	DECLARE_INPUT_CHANGED_MEMBER(lviv_reset);
};

/*----------- defined in video/lviv.c -----------*/

extern const unsigned char lviv_palette[8*3];


#endif /* LVIV_H_ */
