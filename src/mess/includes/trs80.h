/*****************************************************************************
 *
 * includes/trs80.h
 *
 ****************************************************************************/

#ifndef TRS80_H_
#define TRS80_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "machine/ay31015.h"
#include "machine/ctronics.h"
#include "machine/wd17xx.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "imagedev/snapquik.h"
#include "formats/trs_cas.h"
#include "formats/trs_cmd.h"
#include "formats/trs_dsk.h"


class trs80_state : public driver_device
{
public:
	trs80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_printer(*this, "centronics"),
	m_ay31015(*this, "tr1602"),
	m_fdc(*this, "wd179x"),
	m_speaker(*this, SPEAKER_TAG),
	m_cass(*this, CASSETTE_TAG),
	m_p_videoram(*this, "p_videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<centronics_device> m_printer;
	optional_device<device_t> m_ay31015;
	optional_device<device_t> m_fdc;
	required_device<device_t> m_speaker;
	required_device<cassette_image_device> m_cass;
	DECLARE_WRITE8_MEMBER ( trs80_ff_w );
	DECLARE_WRITE8_MEMBER ( lnw80_fe_w );
	DECLARE_WRITE8_MEMBER ( sys80_fe_w );
	DECLARE_WRITE8_MEMBER ( sys80_f8_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ff_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_f4_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ec_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_eb_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ea_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e9_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e8_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e4_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e0_w );
	DECLARE_WRITE8_MEMBER ( trs80m4p_9c_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_90_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_84_w );
	DECLARE_READ8_MEMBER ( lnw80_fe_r );
	DECLARE_READ8_MEMBER ( trs80_ff_r );
	DECLARE_READ8_MEMBER ( sys80_f9_r );
	DECLARE_READ8_MEMBER ( trs80m4_ff_r );
	DECLARE_READ8_MEMBER ( trs80m4_ec_r );
	DECLARE_READ8_MEMBER ( trs80m4_eb_r );
	DECLARE_READ8_MEMBER ( trs80m4_ea_r );
	DECLARE_READ8_MEMBER ( trs80m4_e8_r );
	DECLARE_READ8_MEMBER ( trs80m4_e4_r );
	DECLARE_READ8_MEMBER ( trs80m4_e0_r );
	DECLARE_READ8_MEMBER( trs80_irq_status_r );
	DECLARE_READ8_MEMBER( trs80_printer_r );
	DECLARE_WRITE8_MEMBER( trs80_printer_w );
	DECLARE_WRITE8_MEMBER( trs80_cassunit_w );
	DECLARE_WRITE8_MEMBER( trs80_motor_w );
	DECLARE_READ8_MEMBER( trs80_keyboard_r );
	DECLARE_WRITE8_MEMBER ( trs80m4_88_w );
	DECLARE_READ8_MEMBER( trs80_videoram_r );
	DECLARE_WRITE8_MEMBER( trs80_videoram_w );
	DECLARE_READ8_MEMBER( trs80_gfxram_r );
	DECLARE_WRITE8_MEMBER( trs80_gfxram_w );
	DECLARE_READ8_MEMBER (trs80_wd179x_r);
	const UINT8 *m_p_chargen;
	optional_shared_ptr<UINT8> m_p_videoram;
	UINT8 *m_p_gfxram;
	UINT8 m_model4;
	UINT8 m_mode;
	UINT8 m_irq;
	UINT8 m_mask;
	UINT8 m_nmi_mask;
	UINT8 m_port_ec;
	UINT8 m_tape_unit;
	UINT8 m_reg_load;
	UINT8 m_nmi_data;
#ifdef USE_TRACK
	UINT8 m_track[4];
#endif
	UINT8 m_head;
#ifdef USE_SECTOR
	UINT8 m_sector[4];
#endif
	UINT8 m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	UINT16 m_start_address;
	UINT8 m_crtc_reg;
	UINT8 m_size_store;
	DECLARE_DRIVER_INIT(trs80m4);
	DECLARE_DRIVER_INIT(trs80l2);
	DECLARE_DRIVER_INIT(trs80m4p);
	DECLARE_DRIVER_INIT(lnw80);
	DECLARE_DRIVER_INIT(trs80);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_MACHINE_RESET(trs80m4);
	DECLARE_MACHINE_RESET(lnw80);
	DECLARE_PALETTE_INIT(lnw80);
	UINT32 screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_trs80m4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_radionic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_meritum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(trs80_rtc_interrupt);
	INTERRUPT_GEN_MEMBER(trs80_fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
};


/*----------- defined in machine/trs80.c -----------*/

extern const wd17xx_interface trs80_wd17xx_interface;

#endif	/* TRS80_H_ */
