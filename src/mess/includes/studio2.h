#pragma once

#ifndef __STUDIO2__
#define __STUDIO2__


#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cartslot.h"
#include "formats/studio2_st2.h"
#include "sound/beep.h"
#include "sound/cdp1864.h"
#include "sound/discrete.h"
#include "video/cdp1861.h"

#define CDP1802_TAG     "ic1"
#define CDP1861_TAG     "ic2"
#define CDP1864_TAG     "cdp1864"
#define SCREEN_TAG      "screen"

class studio2_state : public driver_device
{
public:
	studio2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_speaker(*this, BEEPER_TAG),
			m_vdc(*this, CDP1861_TAG)
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<beep_device> m_speaker;
	optional_device<cdp1861_device> m_vdc;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( dispon_r );
	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_WRITE8_MEMBER( dispon_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_INPUT_CHANGED_MEMBER( reset_w );

	/* keyboard state */
	UINT8 m_keylatch;
	DECLARE_DRIVER_INIT(studio2);
	TIMER_CALLBACK_MEMBER(setup_beep);
};

class visicom_state : public studio2_state
{
public:
	visicom_state(const machine_config &mconfig, device_type type, const char *tag)
		: studio2_state(mconfig, type, tag),
			m_color_ram(*this, "color_ram"),
			m_color_ram1(*this, "color_ram1")
	{ }

	required_shared_ptr<UINT8> m_color_ram;
	required_shared_ptr<UINT8> m_color_ram1;
};

class mpt02_state : public studio2_state
{
public:
	mpt02_state(const machine_config &mconfig, device_type type, const char *tag)
		: studio2_state(mconfig, type, tag),
			m_cti(*this, CDP1864_TAG),
			m_color_ram(*this, "color_ram")
	{ }

	required_device<cdp1864_device> m_cti;

	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ_LINE_MEMBER( rdata_r );
	DECLARE_READ_LINE_MEMBER( bdata_r );
	DECLARE_READ_LINE_MEMBER( gdata_r );

	/* video state */
	required_shared_ptr<UINT8> m_color_ram;
	UINT8 m_color;
};

#endif
