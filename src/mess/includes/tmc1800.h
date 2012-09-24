#pragma once

#ifndef __TMC1800__
#define __TMC1800__


#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/beep.h"
#include "sound/cdp1864.h"
#include "video/cdp1861.h"

#define TMC2000_COLORRAM_SIZE	0x200

#define SCREEN_TAG		"screen"
#define CDP1802_TAG		"cdp1802"
#define CDP1861_TAG		"cdp1861"
#define CDP1864_TAG		"m3"

class tmc1800_state : public driver_device
{
public:
	tmc1800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, CDP1802_TAG),
		  m_vdc(*this, CDP1861_TAG),
		  m_cassette(*this, CASSETTE_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cdp1861_device> m_vdc;
	required_device<cassette_image_device> m_cassette;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_READ8_MEMBER( dispon_r );
	DECLARE_WRITE8_MEMBER( dispoff_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	/* keyboard state */
	int m_keylatch;			/* key latch */
	DECLARE_DRIVER_INIT(tmc1800);
	TIMER_CALLBACK_MEMBER(setup_beep);
};

class osc1000b_state : public driver_device
{
public:
	osc1000b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, CDP1802_TAG),
		  m_cassette(*this, CASSETTE_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	/* keyboard state */
	int m_keylatch;			/* key latch */
};

class tmc2000_state : public driver_device
{
public:
	tmc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, CDP1802_TAG),
		  m_cti(*this, CDP1864_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ_LINE_MEMBER( rdata_r );
	DECLARE_READ_LINE_MEMBER( bdata_r );
	DECLARE_READ_LINE_MEMBER( gdata_r );
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );

	void bankswitch();

	// memory
	int m_rac;
	int m_roc;

	/* video state */
	UINT8 *m_colorram;		/* color memory */
	UINT8 m_color;

	/* keyboard state */
	int m_keylatch;			/* key latch */
};

class nano_state : public driver_device
{
public:
	nano_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, CDP1802_TAG),
		  m_cti(*this, CDP1864_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_start();
	virtual void machine_reset();

	enum
	{
		TIMER_ID_EF4
	};

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );
	DECLARE_INPUT_CHANGED_MEMBER( monitor_pressed );

	/* keyboard state */
	int m_keylatch;			/* key latch */
};

/* ---------- defined in video/tmc1800.c ---------- */

MACHINE_CONFIG_EXTERN( tmc1800_video );
MACHINE_CONFIG_EXTERN( osc1000b_video );
MACHINE_CONFIG_EXTERN( tmc2000_video );
MACHINE_CONFIG_EXTERN( nano_video );

#endif
