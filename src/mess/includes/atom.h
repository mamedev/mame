#pragma once

#ifndef __ATOM__
#define __ATOM__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "formats/atom_atm.h"
#include "formats/atom_tap.h"
#include "formats/basicdsk.h"
#include "formats/uef_cas.h"
#include "machine/ctronics.h"
#include "machine/6522via.h"
#include "machine/i8255.h"
#include "machine/i8271.h"
#include "sound/speaker.h"
#include "video/mc6847.h"

#define SY6502_TAG      "ic22"
#define INS8255_TAG     "ic25"
#define MC6847_TAG      "ic31"
#define R6522_TAG       "ic1"
#define I8271_TAG       "ic13"
#define MC6854_TAG      "econet_ic1"
#define SCREEN_TAG      "screen"
#define CENTRONICS_TAG  "centronics"
#define BASERAM_TAG     "baseram"
#define EXTROM_TAG      "a000"
#define DOSROM_TAG      "e000"


#define X1  XTAL_3_579545MHz    // MC6847 Clock
#define X2  XTAL_4MHz           // CPU Clock - a divider reduces it to 1MHz

class atom_state : public driver_device
{
public:
	atom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, SY6502_TAG),
			m_vdg(*this, MC6847_TAG),
			m_cassette(*this, CASSETTE_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_speaker(*this, SPEAKER_TAG),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_y8(*this, "Y8"),
			m_y9(*this, "Y9"),
			m_y10(*this, "Y10"),
			m_rpt(*this, "RPT"),
			m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_rpt;

	virtual void machine_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bankswitch();

	DECLARE_READ8_MEMBER( eprom_r );
	DECLARE_WRITE8_MEMBER( eprom_w );
	DECLARE_WRITE8_MEMBER( ppi_pa_w );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_READ8_MEMBER( ppi_pc_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_READ8_MEMBER( printer_busy );
	DECLARE_WRITE8_MEMBER( printer_data );
	DECLARE_READ8_MEMBER( vdg_videoram_r );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

	/* eprom state */
	int m_eprom;

	/* video state */
	required_shared_ptr<UINT8> m_video_ram;

	/* keyboard state */
	int m_keylatch;

	/* cassette state */
	int m_hz2400;
	int m_pc0;
	int m_pc1;

	/* devices */
	int m_previous_i8271_int_state;
	TIMER_DEVICE_CALLBACK_MEMBER(cassette_output_tick);
};

class atomeb_state : public atom_state
{
public:
	atomeb_state(const machine_config &mconfig, device_type type, const char *tag)
		: atom_state(mconfig, type, tag)
	{ }

	virtual void machine_start();
};

#endif
