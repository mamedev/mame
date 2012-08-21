#pragma once

#ifndef __PHC25__
#define __PHC25__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/ctronics.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"

#define SCREEN_TAG		"screen"
#define Z80_TAG			"z80"
#define AY8910_TAG		"ay8910"
#define MC6847_TAG		"mc6847"
#define CENTRONICS_TAG	"centronics"

#define PHC25_VIDEORAM_SIZE		0x1800

class phc25_state : public driver_device
{
public:
	phc25_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_vdg(*this, MC6847_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_cassette(*this, CASSETTE_TAG)
	,
		m_video_ram(*this, "video_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;

	virtual void video_start();

	DECLARE_READ8_MEMBER( port40_r );
	DECLARE_WRITE8_MEMBER( port40_w );
	DECLARE_READ8_MEMBER( video_ram_r );

	static UINT8 ntsc_char_rom_r(running_machine &machine, UINT8 ch, int line);
	static UINT8 pal_char_rom_r(running_machine &machine, UINT8 ch, int line);

	/* video state */
	required_shared_ptr<UINT8> m_video_ram;
	UINT8 *m_char_rom;
};

#endif
