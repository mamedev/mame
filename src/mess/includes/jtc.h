#pragma once

#ifndef __JTC__
#define __JTC__


#include "emu.h"
#include "cpu/z8/z8.h"
#include "imagedev/cassette.h"
#include "machine/ctronics.h"
#include "machine/ram.h"
#include "sound/speaker.h"
#include "sound/wave.h"

#define SCREEN_TAG      "screen"
#define UB8830D_TAG     "ub8830d"
#define CENTRONICS_TAG  "centronics"

#define JTC_ES40_VIDEORAM_SIZE  0x2000

class jtc_state : public driver_device
{
public:
	jtc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, UB8830D_TAG),
			m_cassette(*this, CASSETTE_TAG),
			m_speaker(*this, SPEAKER_TAG),
			m_centronics(*this, CENTRONICS_TAG),
		m_video_ram(*this, "video_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;

	virtual void machine_start();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( p3_r );
	DECLARE_WRITE8_MEMBER( p3_w );
	DECLARE_PALETTE_INIT(jtc_es40);
	optional_shared_ptr<UINT8> m_video_ram;
};

class jtces88_state : public jtc_state
{
public:
	jtces88_state(const machine_config &mconfig, device_type type, const char *tag)
		: jtc_state(mconfig, type, tag)
	{ }
};

class jtces23_state : public jtc_state
{
public:
	jtces23_state(const machine_config &mconfig, device_type type, const char *tag)
		: jtc_state(mconfig, type, tag)
	{ }

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class jtces40_state : public jtc_state
{
public:
	jtces40_state(const machine_config &mconfig, device_type type, const char *tag)
		: jtc_state(mconfig, type, tag)
	{ }

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( videoram_r );
	DECLARE_WRITE8_MEMBER( videoram_w );
	DECLARE_WRITE8_MEMBER( banksel_w );

	UINT8 m_video_bank;
	UINT8 *m_color_ram_r;
	UINT8 *m_color_ram_g;
	UINT8 *m_color_ram_b;
};

#endif
