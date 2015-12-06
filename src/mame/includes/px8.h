// license:GPL-2.0+
// copyright-holders:Curt Coder,Dirk Best
#pragma once

#ifndef __PX8__
#define __PX8__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "bus/epson_sio/pf10.h"
#include "sound/wave.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define UPD70008_TAG    "4a"
#define UPD7508_TAG     "2e"
#define HD6303_TAG      "13d"
#define SED1320_TAG     "7c"
#define I8251_TAG       "13e"
#define UPD7001_TAG     "1d"
#define SCREEN_TAG      "screen"

#define PX8_VIDEORAM_MASK   0x17ff

class px8_state : public driver_device
{
public:
	px8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, UPD70008_TAG),
			m_cassette(*this, "cassette"),
			m_ram(*this, RAM_TAG),
		m_video_ram(*this, "video_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	/* video state */
	required_shared_ptr<UINT8> m_video_ram;         /* LCD video RAM */

	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( gah40m_r );
	DECLARE_WRITE8_MEMBER( gah40m_w );
	DECLARE_READ8_MEMBER( gah40s_r );
	DECLARE_WRITE8_MEMBER( gah40s_w );
	DECLARE_WRITE8_MEMBER( gah40s_ier_w );
	DECLARE_READ8_MEMBER( krtn_0_3_r );
	DECLARE_READ8_MEMBER( krtn_4_7_r );
	DECLARE_WRITE8_MEMBER( ksc_w );

	void bankswitch();
	UINT8 krtn_read();

	/* GAH40M state */
	UINT16 m_icr;               /* input capture register */
	UINT16 m_frc;               /* free running counter */
	UINT8 m_ier;                /* interrupt acknowledge register */
	UINT8 m_isr;                /* interrupt status register */
	UINT8 m_sio;                /* serial I/O register */
	int m_bank0;                /* */

	/* GAH40S state */
	UINT16 m_cnt;               /* microcassette tape counter */
	int m_swpr;             /* P-ROM power switch */
	UINT16 m_pra;               /* P-ROM address */
	UINT8 m_prd;                /* P-ROM data */

	/* memory state */
	int m_bk2;              /* */

	/* keyboard state */
	int m_ksc;              /* keyboard scan column */
	DECLARE_PALETTE_INIT(px8);
	UINT32 screen_update_px8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif
