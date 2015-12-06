// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
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
#include "audio/specimx_snd.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "imagedev/cassette.h"
#include "formats/smx_dsk.h"
#include "formats/rk_cas.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"


class special_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET,
		TIMER_PIT8253_GATES
	};

	special_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi8255"),
		m_fdc(*this, "fd1793"),
		m_dac(*this, "dac"),
		m_pit(*this, "pit8253"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_p_videoram(*this, "p_videoram"),
		m_region_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_io_line8(*this, "LINE8"),
		m_io_line9(*this, "LINE9"),
		m_io_line10(*this, "LINE10"),
		m_io_line11(*this, "LINE11"),
		m_io_line12(*this, "LINE12"),
		m_palette(*this, "palette")  { }

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
	DECLARE_READ8_MEMBER(specimx_8255_portb_r);
	DECLARE_READ8_MEMBER(specialist_8255_portc_r);
	DECLARE_WRITE8_MEMBER(specialist_8255_porta_w);
	DECLARE_WRITE8_MEMBER(specialist_8255_portb_w);
	DECLARE_WRITE8_MEMBER(specialist_8255_portc_w);
	void specimx_set_bank(offs_t i, UINT8 data);
	void erik_set_bank();
	UINT8 *m_specimx_colorram;
	UINT8 m_erik_color_1;
	UINT8 m_erik_color_2;
	UINT8 m_erik_background;
	UINT8 m_specimx_color;
	int m_specialist_8255_porta;
	int m_specialist_8255_portb;
	int m_specialist_8255_portc;
	UINT8 m_RR_register;
	UINT8 m_RC_register;
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device> m_ppi;
	optional_device<fd1793_t> m_fdc;
	optional_device<dac_device> m_dac;
	optional_device<pit8253_device> m_pit;
	optional_device<cassette_image_device> m_cassette;
	optional_device<ram_device> m_ram;
	optional_shared_ptr<UINT8> m_p_videoram;
	int m_drive;
	DECLARE_DRIVER_INIT(erik);
	DECLARE_DRIVER_INIT(special);
	DECLARE_MACHINE_RESET(special);
	DECLARE_VIDEO_START(special);
	DECLARE_MACHINE_RESET(erik);
	DECLARE_VIDEO_START(erik);
	DECLARE_PALETTE_INIT(erik);
	DECLARE_VIDEO_START(specialp);
	DECLARE_MACHINE_START(specimx);
	DECLARE_MACHINE_RESET(specimx);
	DECLARE_VIDEO_START(specimx);
	DECLARE_PALETTE_INIT(specimx);
	UINT32 screen_update_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_erik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_specialp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_specimx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);
	DECLARE_FLOPPY_FORMATS( specimx_floppy_formats );

protected:
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	optional_memory_bank m_bank5;
	optional_memory_bank m_bank6;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_ioport m_io_line9;
	required_ioport m_io_line10;
	required_ioport m_io_line11;
	required_ioport m_io_line12;

	required_device<palette_device> m_palette;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /* SPECIAL_H_ */
