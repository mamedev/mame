// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __MC1000__
#define __MC1000__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "video/mc6845.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"
#include "bus/centronics/ctronics.h"
#include "machine/rescap.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "u13"
#define AY8910_TAG      "u21"
#define MC6845_TAG      "mc6845"
#define MC6847_TAG      "u19"
#define CENTRONICS_TAG  "centronics"

#define MC1000_MC6845_VIDEORAM_SIZE     0x800
#define MC1000_MC6847_VIDEORAM_SIZE     0x1800

class mc1000_state : public driver_device
{
public:
	mc1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_vdg(*this, MC6847_TAG),
			m_crtc(*this, MC6845_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_cassette(*this, "cassette"),
			m_ram(*this, RAM_TAG),
			m_rom(*this, Z80_TAG),
			m_mc6845_video_ram(*this, "mc6845_vram"),
			m_mc6847_video_ram(*this, "mc6847_vram"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_joya(*this, "JOYA"),
			m_joyb(*this, "JOYB"),
			m_modifiers(*this, "MODIFIERS"),
			m_joyakeymap(*this, "JOYAKEYMAP"),
			m_joybkeymap(*this, "JOYBKEYMAP")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	optional_device<mc6845_device> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_shared_ptr<UINT8> m_mc6845_video_ram;
	required_shared_ptr<UINT8> m_mc6847_video_ram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_joya;
	required_ioport m_joyb;
	required_ioport m_modifiers;
	required_ioport m_joyakeymap;
	required_ioport m_joybkeymap;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( printer_r );
	DECLARE_WRITE8_MEMBER( printer_w );
	DECLARE_WRITE8_MEMBER( mc6845_ctrl_w );
	DECLARE_WRITE8_MEMBER( mc6847_attr_w );
	DECLARE_WRITE_LINE_MEMBER( fs_w );
	DECLARE_WRITE_LINE_MEMBER( hs_w );
	DECLARE_READ8_MEMBER( videoram_r );
	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_READ8_MEMBER( keydata_r );
	DIRECT_UPDATE_MEMBER(mc1000_direct_update_handler);

	void bankswitch();

	/* cpu state */
	int m_ne555_int;

	/* memory state */
	int m_rom0000;
	int m_mc6845_bank;
	int m_mc6847_bank;

	/* keyboard state */
	int m_keylatch;

	/* video state */
	int m_hsync;
	int m_vsync;
	UINT8 m_mc6847_attr;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	int m_centronics_busy;

	DECLARE_DRIVER_INIT(mc1000);
	TIMER_DEVICE_CALLBACK_MEMBER(ne555_tick);
};

#endif
