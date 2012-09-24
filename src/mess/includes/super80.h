#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/wave.h"
#include "imagedev/snapquik.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "machine/ctronics.h"
#include "video/mc6845.h"
#include "machine/z80pio.h"

/* Bits in shared variable:
    d5 cassette LED
    d4 super80v rom or pcg bankswitch (1=pcg ram, 0=char gen rom)
    d2 super80v video or colour bankswitch (1=video ram, 0=colour ram)
    d2 super80 screen off (=2mhz) or on (bursts of 2mhz at 50hz = 1mhz) */



class super80_state : public driver_device
{
public:
	super80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_pio(*this, "z80pio"),
		  m_cass(*this, CASSETTE_TAG),
		  m_wave(*this, WAVE_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_centronics(*this, "centronics"),
		  m_6845(*this, "crtc")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cass;
	required_device<device_t> m_wave;
	required_device<device_t> m_speaker;
	required_device<centronics_device> m_centronics;
	optional_device<mc6845_device> m_6845;
	READ8_MEMBER( super80v_low_r );
	READ8_MEMBER( super80v_high_r );
	WRITE8_MEMBER( super80v_low_w );
	WRITE8_MEMBER( super80v_high_w );
	WRITE8_MEMBER( super80v_10_w );
	WRITE8_MEMBER( super80v_11_w );
	WRITE8_MEMBER( super80_f1_w );
	READ8_MEMBER( super80_dc_r );
	READ8_MEMBER( super80_f2_r );
	WRITE8_MEMBER( super80_dc_w );
	WRITE8_MEMBER( super80_f0_w );
	WRITE8_MEMBER( super80r_f0_w );
	READ8_MEMBER( super80_read_ff );
	WRITE8_MEMBER( pio_port_a_w );
	//READ8_MEMBER( pio_port_b_r );
	virtual void machine_reset();
	UINT8 m_shared;
	UINT8 m_keylatch;
	UINT8 m_cass_data[4];
	UINT8 m_int_sw;
	UINT8 m_last_data;
	UINT16 m_vidpg;
	UINT8 m_current_palette;
	UINT8 m_current_charset;
	const UINT8 *m_p_chargen;
	UINT8 m_s_options;
	UINT8 m_mc6845_cursor[16];
	UINT8 m_mc6845_reg[32];
	UINT8 m_mc6845_ind;
	UINT8 m_framecnt;
	UINT8 m_speed;
	UINT8 m_flash;
	UINT16 m_cursor;
	UINT8 *m_p_videoram;
	UINT8 *m_p_colorram;
	UINT8 *m_p_pcgram;
	void mc6845_cursor_configure();
	DECLARE_DRIVER_INIT(super80);
	DECLARE_DRIVER_INIT(super80v);
	DECLARE_VIDEO_START(super80);
	DECLARE_VIDEO_START(super80v);
	DECLARE_PALETTE_INIT(super80m);
	UINT32 screen_update_super80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80e(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_super80m(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(super80_timer);
	TIMER_CALLBACK_MEMBER(super80_reset);
	TIMER_CALLBACK_MEMBER(super80_halfspeed);
};


/*----------- defined in video/super80.c -----------*/
MC6845_UPDATE_ROW( super80v_update_row );

/*----------- defined in machine/super80.c -----------*/

extern const z80pio_interface super80_pio_intf;
