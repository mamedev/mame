#ifndef __MC1000__
#define __MC1000__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"
#include "machine/ctronics.h"
#include "machine/rescap.h"
#include "machine/ram.h"

#define SCREEN_TAG		"screen"
#define Z80_TAG			"u13"
#define AY8910_TAG		"u21"
#define MC6845_TAG		"mc6845"
#define MC6847_TAG		"u19"
#define CENTRONICS_TAG	"centronics"

#define MC1000_MC6845_VIDEORAM_SIZE		0x800
#define MC1000_MC6847_VIDEORAM_SIZE		0x1800

class mc1000_state : public driver_device
{
public:
	mc1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_vdg(*this, MC6847_TAG),
		  m_crtc(*this, MC6845_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_ram(*this, RAM_TAG)
	,
		m_mc6845_video_ram(*this, "mc6845_vram"),
		m_mc6847_video_ram(*this, "mc6847_vram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	optional_device<device_t> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	required_shared_ptr<UINT8> m_mc6845_video_ram;
	required_shared_ptr<UINT8> m_mc6847_video_ram;
	UINT8 m_mc6847_attr;
	DECLARE_DRIVER_INIT(mc1000);
	TIMER_DEVICE_CALLBACK_MEMBER(ne555_tick);
};

#endif
