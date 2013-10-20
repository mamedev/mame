/*****************************************************************************
 *
 * includes/compis.h
 *
 * machine driver header
 *
 * Per Ola Ingvarsson
 * Tomas Karlsson
 *
 ****************************************************************************/

#ifndef COMPIS_H_
#define COMPIS_H_

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "formats/cpis_dsk.h"
#include "imagedev/cassette.h"
#include "machine/compiskb.h"
#include "machine/ctronics.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/mm58274c.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "video/upd7220.h"

#define CASSETTE_TAG "cassette"

class compis_state : public driver_device
{
public:
	compis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_8253(*this, "pit8253"),
		  m_8254(*this, "pit8254"),
		  m_8259m(*this, "pic8259_master"),
		  m_8255(*this, "ppi8255"),
		  m_centronics(*this, "centronics"),
		  m_uart(*this, "uart"),
		  m_rtc(*this, "mm58274c"),
		  m_fdc(*this, "i8272a"),
		  m_crtc(*this, "upd7220"),
		  m_cassette(*this, CASSETTE_TAG),
		  m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_8253;
	required_device<pit8254_device> m_8254;
	required_device<pic8259_device> m_8259m;
	required_device<i8255_device> m_8255;
	required_device<centronics_device> m_centronics;
	required_device<i8251_device> m_uart;
	required_device<mm58274c_device> m_rtc;
	required_device<i8272a_device> m_fdc;
	required_device<upd7220_device> m_crtc;
	required_device<cassette_image_device> m_cassette;

	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(compis_ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(compis_ppi_port_c_w);
	UINT8 *m_p_videoram;
	void compis_fdc_tc(int state);
	DECLARE_READ8_MEMBER(fdc_mon_r);
	DECLARE_WRITE8_MEMBER(fdc_mon_w);
	bool m_mon;
	DECLARE_WRITE_LINE_MEMBER(tmr0_w);
	DECLARE_WRITE_LINE_MEMBER(tmr2_w);
	DECLARE_WRITE8_MEMBER(tape_mon_w);

	void fdc_irq(bool state);
	void fdc_drq(bool state);

	required_shared_ptr<UINT8> m_video_ram;
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(compis_vblank_int);
	DECLARE_READ8_MEMBER(compis_irq_callback);
	void compis_fdc_reset();

	int m_tmr0;
};


/*----------- defined in machine/compis.c -----------*/

extern const i8255_interface compis_ppi_interface;
extern const struct pit8253_interface compis_pit8253_config;
extern const struct pit8253_interface compis_pit8254_config;
extern const i8251_interface compis_usart_interface;

#endif /* COMPIS_H_ */
