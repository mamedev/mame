// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*****************************************************************************
 *
 * includes/poisk1.h
 *
 ****************************************************************************/

#ifndef POISK1_H_
#define POISK1_H_

#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "bus/isa/isa.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "bus/isa/xsu_cards.h"
#include "sound/speaker.h"

#define POISK1_UPDATE_ROW(name) \
	void name(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 *videoram, UINT16 ma, UINT8 ra, UINT8 stride)

class p1_state : public driver_device
{
public:
	p1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_pit8253(*this, "pit8253"),
		m_ppi8255n1(*this, "ppi8255n1"),
		m_ppi8255n2(*this, "ppi8255n2"),
		m_isabus(*this, "isa"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette") { }

	required_device<cpu_device>  m_maincpu;
	required_device<pic8259_device>  m_pic8259;
	required_device<pit8253_device>  m_pit8253;
	required_device<i8255_device>  m_ppi8255n1;
	required_device<i8255_device>  m_ppi8255n2;
	required_device<isa8_device>  m_isabus;
	required_device<speaker_sound_device>  m_speaker;
	required_device<cassette_image_device>  m_cassette;
	required_device<ram_device> m_ram;

	DECLARE_DRIVER_INIT(poisk1);
	DECLARE_MACHINE_START(poisk1);
	DECLARE_MACHINE_RESET(poisk1);

	DECLARE_PALETTE_INIT(p1);
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void set_palette_luts();
	POISK1_UPDATE_ROW(cga_gfx_2bpp_update_row);
	POISK1_UPDATE_ROW(cga_gfx_1bpp_update_row);
	POISK1_UPDATE_ROW(poisk1_gfx_1bpp_update_row);

	DECLARE_WRITE_LINE_MEMBER(p1_pit8253_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(p1_speaker_set_spkrdata);
	UINT8 m_p1_spkrdata;
	UINT8 m_p1_input;

	UINT8 m_kbpoll_mask;

	struct
	{
		UINT8 trap[4];
		std::unique_ptr<UINT8[]> videoram_base;
		UINT8 *videoram;
		UINT8 mode_control_6a;
		UINT8 color_select_68;
		UINT8 palette_lut_2bpp[4];
		int stride;
		void *update_row(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 *videoram, UINT16 ma, UINT8 ra, UINT8 stride);
	} m_video;

	DECLARE_READ8_MEMBER(p1_trap_r);
	DECLARE_WRITE8_MEMBER(p1_trap_w);
	DECLARE_READ8_MEMBER(p1_cga_r);
	DECLARE_WRITE8_MEMBER(p1_cga_w);
	DECLARE_WRITE8_MEMBER(p1_vram_w);

	DECLARE_READ8_MEMBER(p1_ppi_r);
	DECLARE_WRITE8_MEMBER(p1_ppi_w);
	DECLARE_WRITE8_MEMBER(p1_ppi_porta_w);
	DECLARE_READ8_MEMBER(p1_ppi_porta_r);
	DECLARE_READ8_MEMBER(p1_ppi_portb_r);
	DECLARE_READ8_MEMBER(p1_ppi_portc_r);
	DECLARE_WRITE8_MEMBER(p1_ppi_portc_w);
	DECLARE_WRITE8_MEMBER(p1_ppi2_porta_w);
	DECLARE_WRITE8_MEMBER(p1_ppi2_portb_w);
	DECLARE_READ8_MEMBER(p1_ppi2_portc_r);
	required_device<palette_device> m_palette;
	const char *m_cputag;
};

#endif /* POISK1_H_ */
