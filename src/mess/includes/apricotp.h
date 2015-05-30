// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Dirk Best
#pragma once

#ifndef __APRICOTP__
#define __APRICOTP__


#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/m6800/m6800.h"
#include "machine/am9517a.h"
#include "machine/apricotkb.h"
#include "bus/centronics/ctronics.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80dart.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"
#include "rendlay.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8086_TAG       "ic7"
#define I8284_TAG       "ic30"
#define I8237_TAG       "ic17"
#define I8259A_TAG      "ic51"
#define I8253A5_TAG     "ic20"
#define TMS4500_TAG     "ic42"
#define MC6845_TAG      "ic69"
#define HD63B01V1_TAG   "ic29"
#define AD7574_TAG      "ic34"
#define AD1408_TAG      "ic37"
#define Z80SIO0_TAG     "ic6"
#define WD2797_TAG      "ic5"
#define SN76489AN_TAG   "ic13"
#define CENTRONICS_TAG  "centronics"
#define SCREEN_LCD_TAG  "screen0"
#define SCREEN_CRT_TAG  "screen1"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fp_state

class fp_state : public driver_device
{
public:
	fp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8086_TAG),
			m_soundcpu(*this, HD63B01V1_TAG),
			m_dmac(*this, I8237_TAG),
			m_pic(*this, I8259A_TAG),
			m_pit(*this, I8253A5_TAG),
			m_sio(*this, Z80SIO0_TAG),
			m_fdc(*this, WD2797_TAG),
			m_crtc(*this, MC6845_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, WD2797_TAG":0"),
			m_floppy1(*this, WD2797_TAG":1"),
			m_floppy(NULL),
			m_centronics(*this, CENTRONICS_TAG),
			m_work_ram(*this, "work_ram"),
			m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<z80dart_device> m_sio;
	required_device<wd2797_t> m_fdc;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_device<centronics_device> m_centronics;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	MC6845_UPDATE_ROW(update_row);
	DECLARE_READ16_MEMBER( mem_r );
	DECLARE_WRITE16_MEMBER( mem_w );
	DECLARE_READ8_MEMBER( prtr_snd_r );
	DECLARE_WRITE8_MEMBER( pint_clr_w );
	DECLARE_WRITE8_MEMBER( ls_w );
	DECLARE_WRITE8_MEMBER( contrast_w );
	DECLARE_WRITE8_MEMBER( palette_w );
	DECLARE_WRITE16_MEMBER( video_w );
	DECLARE_WRITE8_MEMBER( lat_w );

	void lat_ls259_w(offs_t offset, int state);

	optional_shared_ptr<UINT16> m_work_ram;

	// video state
	optional_shared_ptr<UINT16> m_video_ram;
	UINT8 m_video;

	int m_centronics_busy;
	int m_centronics_select;
	int m_centronics_fault;
	int m_centronics_perror;

	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_select );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_fault );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_perror );
};



#endif
