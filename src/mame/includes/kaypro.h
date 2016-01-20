// license:BSD-3-Clause
// copyright-holders:Robbbert
#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/com8116.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/snapquik.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "machine/wd_fdc.h"

struct kay_kbd_t;

class kaypro_state : public driver_device
{
public:
	enum
	{
		TIMER_FLOPPY
	};

	kaypro_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_pio_g(*this, "z80pio_g")
		, m_pio_s(*this, "z80pio_s")
		, m_sio(*this, "z80sio")
		, m_sio2x(*this, "z80sio_2x")
		, m_centronics(*this, "centronics")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_crtc(*this, "crtc")
		, m_beep(*this, "beeper")
	{}

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_READ8_MEMBER(kaypro2x_87_r);
	DECLARE_READ8_MEMBER(kaypro2x_system_port_r);
	DECLARE_READ8_MEMBER(kaypro2x_status_r);
	DECLARE_READ8_MEMBER(kaypro2x_videoram_r);
	DECLARE_WRITE8_MEMBER(kaypro2x_system_port_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_index_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_register_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_videoram_w);
	DECLARE_READ8_MEMBER(pio_system_r);
	DECLARE_WRITE8_MEMBER(kayproii_pio_system_w);
	DECLARE_WRITE8_MEMBER(kaypro4_pio_system_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_READ8_MEMBER(kaypro_videoram_r);
	DECLARE_WRITE8_MEMBER(kaypro_videoram_w);
	DECLARE_MACHINE_START(kayproii);
	DECLARE_MACHINE_RESET(kaypro);
	DECLARE_VIDEO_START(kaypro);
	DECLARE_PALETTE_INIT(kaypro);
	DECLARE_MACHINE_RESET(kay_kbd);
	DECLARE_DRIVER_INIT(kaypro);
	DECLARE_FLOPPY_FORMATS(kayproii_floppy_formats);
	DECLARE_FLOPPY_FORMATS(kaypro2x_floppy_formats);
	UINT32 screen_update_kayproii(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kaypro2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_omni2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kay_kbd_interrupt);
	DECLARE_READ8_MEMBER(kaypro_sio_r);
	DECLARE_WRITE8_MEMBER(kaypro_sio_w);
	MC6845_UPDATE_ROW(kaypro2x_update_row);
	DECLARE_QUICKLOAD_LOAD_MEMBER(kaypro);
	const UINT8 *m_p_chargen;
	UINT8 m_mc6845_cursor[16];
	UINT8 m_mc6845_reg[32];
	UINT8 m_mc6845_ind;
	UINT8 m_framecnt;
	UINT8 *m_p_videoram;
	kay_kbd_t *m_kbd;
	int m_centronics_busy;
	required_device<palette_device> m_palette;

private:
	bool m_is_motor_off;
	UINT8 m_fdc_rq;
	UINT8 m_system_port;
	UINT16 m_mc6845_video_address;
	floppy_image_device *m_floppy;
	void mc6845_cursor_configure();
	void mc6845_screen_configure();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<cpu_device> m_maincpu;
	optional_device<z80pio_device> m_pio_g;
	optional_device<z80pio_device> m_pio_s;
	required_device<z80sio0_device> m_sio;
	optional_device<z80sio0_device> m_sio2x;
	required_device<centronics_device> m_centronics;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<mc6845_device> m_crtc;
	required_device<beep_device> m_beep;
};


/*----------- defined in machine/kay_kbd.c -----------*/

UINT8 kay_kbd_c_r( running_machine &machine );
UINT8 kay_kbd_d_r( running_machine &machine );
void kay_kbd_d_w( running_machine &machine, UINT8 data );

INPUT_PORTS_EXTERN( kay_kbd );
