// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Robbbert
#pragma once

#ifndef __APRICOTF__
#define __APRICOTF__


#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80daisy.h"
#include "formats/basicdsk.h"
#include "imagedev/flopdrv.h"
#include "machine/apricotkb.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "rendlay.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SCREEN_TAG      "screen"
#define I8086_TAG       "10d"
#define Z80CTC_TAG      "13d"
#define Z80SIO2_TAG     "15d"
#define WD2797_TAG      "5f"
#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> f1_state

class f1_state : public driver_device
{
public:
	f1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8086_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_sio(*this, Z80SIO2_TAG),
			m_fdc(*this, WD2797_TAG),
			m_floppy0(*this, WD2797_TAG ":0"),
			m_floppy1(*this, WD2797_TAG ":1"),
			m_centronics(*this, CENTRONICS_TAG),
			m_cent_data_out(*this, "cent_data_out"),
			m_ctc_int(CLEAR_LINE),
			m_sio_int(CLEAR_LINE),
			m_p_scrollram(*this, "p_scrollram"),
			m_p_paletteram(*this, "p_paletteram"),
			m_palette(*this, "palette")
	{ }

	virtual void machine_start();

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio2_device> m_sio;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	int m_ctc_int;
	int m_sio_int;
	required_shared_ptr<UINT16> m_p_scrollram;
	required_shared_ptr<UINT16> m_p_paletteram;
	required_device<palette_device> m_palette;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER( palette_r );
	DECLARE_WRITE16_MEMBER( palette_w );
	DECLARE_WRITE8_MEMBER( system_w );
	DECLARE_WRITE_LINE_MEMBER( sio_int_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_int_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );

	int m_40_80;
	int m_200_256;
};



#endif
