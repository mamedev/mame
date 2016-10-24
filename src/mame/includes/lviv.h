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
	uint8_t m_ppi_port_outputs[2][3];
	uint8_t m_startup_mem_map;
	uint8_t lviv_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lviv_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_lviv(palette_device &palette);
	uint32_t screen_update_lviv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t lviv_ppi_0_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lviv_ppi_0_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lviv_ppi_0_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lviv_ppi_0_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lviv_ppi_0_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lviv_ppi_0_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lviv_ppi_1_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lviv_ppi_1_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lviv_ppi_1_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lviv_ppi_1_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lviv_ppi_1_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lviv_ppi_1_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	void lviv_update_palette(uint8_t pal);
	void lviv_update_memory ();
	void lviv_setup_snapshot (uint8_t * data);
	void dump_registers();
	image_verify_result lviv_verify_snapshot(uint8_t * data, uint32_t size);
	DECLARE_SNAPSHOT_LOAD_MEMBER( lviv );
	void lviv_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
};

/*----------- defined in video/lviv.c -----------*/

extern const unsigned char lviv_palette[8*3];


#endif /* LVIV_H_ */
